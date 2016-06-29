/******************************************************************************
 *
 * Project:  OpenCPN
 * Authors:  David Register
 *           Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include <wx/wxprec.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include <stdint.h>

#include "dychart.h"

#include "viewport.h"
#include "glTexCache.h"
#include "glTextureDescriptor.h"

#include "chcanv.h"
#include "glChartCanvas.h"
#include "chartbase.h"
#include "chartimg.h"
#include "chartdb.h"
#include "OCPNPlatform.h"
#include "mipmap/mipmap.h"

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif


#include "squish.h"
#include "lz4.h"
#include "lz4hc.h"


extern OCPNPlatform *g_Platform;
extern bool g_bopengl;
extern int g_GPU_MemSize;
extern bool g_bDebugOGL;
extern long g_tex_mem_used;
extern int g_mipmap_max_level;
extern GLuint g_raster_format;
extern int          g_nCacheLimit;
extern int          g_memCacheLimit;
extern ChartDB      *ChartData;



extern ocpnGLOptions    g_GLOptions;
extern wxString         g_PrivateDataDir;

extern int              g_tile_size;
extern int              g_uncompressed_tile_size;

class CompressionWorkerPool;
CompressionWorkerPool   *g_CompressorPool;
static wxMutex 		s_MutexPool;


extern PFNGLGETCOMPRESSEDTEXIMAGEPROC s_glGetCompressedTexImage;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
extern PFNGLGENERATEMIPMAPEXTPROC          s_glGenerateMipmap;
extern bool GetMemoryStatus( int *mem_total, int *mem_used );

bool bthread_debug;
bool g_throttle_squish;

enum TextureDataType {COMPRESSED_BUFFER_OK, MAP_BUFFER_OK};

class CompressionPoolThread;
class JobTicket
{
public:
    JobTicket();
    glTexFactory *pFact;
    wxRect       rect;
    int         level_min_request;
    int         ident;
    bool        b_throttle;
    CompressionPoolThread *pthread;
    unsigned char *level0_bits;
    unsigned char *comp_bits_array[10];
    wxString    m_ChartPath;
    bool        b_abort;
    bool        b_isaborted;
    bool        bpost_zip_compress;
    unsigned char *compcomp_bits_array[10];
    int         compcomp_size_array[10];
    
};

JobTicket::JobTicket()
{
    for(int i=0 ; i < 10 ; i++) {
        compcomp_size_array[i] = 0;
        comp_bits_array[i] = NULL;
        compcomp_bits_array[i] = NULL;
    }
}


#include "ssl/sha1.h"

wxString CompressedCachePath(wxString path)
{
#if defined(__WXMSW__)
    int colon = path.find(':', 0);
    path.Remove(colon, 1);
#endif
    
    /* replace path separators with ! */
    wxChar separator = wxFileName::GetPathSeparator();
    for(unsigned int pos = 0; pos < path.size(); pos = path.find(separator, pos))
        path.replace(pos, 1, _T("!"));

    //  Obfuscate the compressed chart file name, to (slightly) protect some encrypted raster chart data.
    wxCharBuffer buf = path.ToUTF8();
    unsigned char sha1_out[20];
    sha1( (unsigned char *) buf.data(), strlen(buf.data()), sha1_out );

    wxString sha1;
    for (unsigned int i=0 ; i < 20 ; i++){
        wxString s;
        s.Printf(_T("%02X"), sha1_out[i]);
        sha1 += s;
    }
    
    return g_Platform->GetPrivateDataDir() + separator + _T("raster_texture_cache") + separator + sha1;
    
}

/* reduce pixel values to 5/6/5, because this is the format they are stored
 *   when compressed anyway, and this way the compression algorithm will use
 *   the exact same color in  adjacent 4x4 tiles and the result is nicer for our purpose.
 *   the lz4 compressed texture is smaller as well. */
void FlattenColorsForCompression(unsigned char *data, int dim, bool swap_colors=true)
{
    #ifdef __WXMSW__ /* undo BGR flip from ocpn_pixel (if ocpnUSE_ocpnBitmap is defined) */
    if(swap_colors)
        for(int i = 0; i<dim*dim; i++) {
            int off = 3*i;
            unsigned char t = data[off + 0];
            data[off + 0] = data[off + 2] & 0xfc;
            data[off + 1] &= 0xf8;
            data[off + 2] = t & 0xfc;
        }
        else
            #endif
            for(int i = 0; i<dim*dim; i++) {
                int off = 3*i;
                data[off + 0] &= 0xfc;
                data[off + 1] &= 0xf8;
                data[off + 2] &= 0xfc;
            }
}

/* return malloced data which is the etc compressed texture of the source */
void CompressDataETC(const unsigned char *data, int dim, int size,
                     unsigned char *tex_data)
{
    wxASSERT(dim*dim == 2*size); // must be 4bpp
    uint64_t *tex_data64 = (uint64_t*)tex_data;
    
    int mbrow = wxMin(4, dim), mbcol = wxMin(4, dim);
    uint8_t block[48] = {};
    for(int row=0; row<dim; row+=4)
        for(int col=0; col<dim; col+=4) {
            for(int brow=0; brow<mbrow; brow++)
                for(int bcol=0; bcol<mbcol; bcol++)
                    memcpy(block + (bcol*4+brow)*3,
                           data + ((row+brow)*dim + col+bcol)*3, 3);
                    
            extern uint64_t ProcessRGB( const uint8_t* src );
            *tex_data64++ = ProcessRGB( block );
        }
}

void GetLevel0Map( glTextureDescriptor *ptd,  const wxRect &rect, wxString &chart_path )
{
    // Load level 0 uncompressed data
    wxRect ncrect(rect);
    ptd->map_array[0] = 0;
    
    ChartBase *pChart = ChartData->OpenChartFromDB( chart_path, FULL_INIT );
    if( !pChart ) {
        ptd->map_array[0] = (unsigned char *) calloc( ncrect.width * ncrect.height * 4, 1 );
        return;
    }
    
        //    Prime the pump with the "zero" level bits, ie. 1x native chart bits
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pChart );
        
    if( pBSBChart ) {
        unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
        pBSBChart->GetChartBits( ncrect, t_buf, 1 );
        
        //    and cache them here
        ptd->map_array[0] = t_buf;
    } else {
        ptd->map_array[0] = (unsigned char *) calloc( ncrect.width * ncrect.height * 4, 1 );
        return;
    }
}



void GetFullMap( glTextureDescriptor *ptd,  const wxRect &rect, wxString chart_path, int level)
{
    
    //  Confirm that the uncompressed bits are all available, get them if not there yet
    if( ptd->map_array[level]) 
        return;

    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    
    for(int i=0 ; i < level ; i++){
        dim /= 2;
        size /= 4;
        if(size < 8)
            size = 8;
    }
        
    if( level > 0 && ptd->map_array[level - 1] ){
        ptd->map_array[level] = (unsigned char *) malloc( dim * dim * 3 );
        MipMap_24( 2*dim, 2*dim, ptd->map_array[level - 1], ptd->map_array[level] );
    }
    
    else {
        //      Any holes in the bit pointer array below the level requested?
        bool b_hole = false;
        for(int i=0 ; i < level ; i++){ 
            if( !ptd->map_array[i] ){
                b_hole = true;
                break;
            }
        }
        
        if( ( level == 0 ) || b_hole ){
            //Get level 0 bits from chart?
            if( !ptd->map_array[0] )
                GetLevel0Map( ptd, rect, chart_path );
                
            int i_lev = 1;
            int dimh = g_GLOptions.m_iTextureDimension / 2;         // starts at level 1
            while( i_lev <= level ){
                if( !ptd->map_array[i_lev] ) {
                    ptd->map_array[i_lev] = (unsigned char *) malloc( dimh * dimh * 3 );
                    MipMap_24( 2*dimh, 2*dimh, ptd->map_array[i_lev - 1], ptd->map_array[i_lev] );
                }
                dimh /= 2;
                i_lev++;
            }
        }
    }
}

bool DoCompress(JobTicket *pticket, glTextureDescriptor *ptd, int level)
{
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    
    for(int i=0 ; i < level ; i++){
        dim /= 2;
        size /= 4;
        if(size < 8)
            size = 8;
    }

    unsigned char *tex_data = (unsigned char*)malloc(size);
        
    if(g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
        // color range fit is worse quality but twice as fast
        int flags = squish::kDxt1 | squish::kColourRangeFit;
            
        if( g_GLOptions.m_bTextureCompressionCaching) {
            /* use slower cluster fit since we are building the cache for
             * better quality, this takes roughly 25% longer and uses about
             * 10% more disk space (result doesn't compress as well with lz4) */
            flags = squish::kDxt1 | squish::kColourClusterFit;
        }

        squish::CompressImageRGBpow2_Flatten_Throttle( ptd->map_array[level], dim, dim, tex_data, flags,
                                                            true, pticket->b_throttle );
 
    }
    else if(g_raster_format == GL_ETC1_RGB8_OES) 
        CompressDataETC(ptd->map_array[level], dim, size, tex_data);
        
    else if(g_raster_format == GL_COMPRESSED_RGB_FXT1_3DFX)
        CompressUsingGPU( ptd, level, false, false);    // no post compression

    //  Store the pointer to compressed data in the ptd
    ptd->CompressedArrayAccess( CA_WRITE, tex_data, level);
        
    if(pticket->bpost_zip_compress) {
        int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
        if(max_compressed_size){
            unsigned char *compressed_data = (unsigned char *)malloc(max_compressed_size);
            int compressed_size = LZ4_compressHC2( (char *)ptd->CompressedArrayAccess( CA_READ, NULL, level),
                                                   (char *)compressed_data, size, 4);
            ptd->CompCompArrayAccess( CA_WRITE, compressed_data, level);
            ptd->compcomp_size[level] = compressed_size;
        }
    }
        
    return true;
}



//  On Windows, we will use a translator to convert SEH exceptions (e.g. access violations),
//    into c++ standard exception handling method.
//  This class and helper function facilitate the conversion.

//  We only do this in the compression worker threads, as they are vulnerable due to possibly errant code in 
//  the chart database management class, especially on low memory systems where chart cahing is stressed heavily.

#ifdef __WXMSW__
class SE_Exception
{
private:
    unsigned int nSE;
public:
    SE_Exception() {}
    SE_Exception( unsigned int n ) : nSE( n ) {}
    ~SE_Exception() {}
    unsigned int getSeNumber() { return nSE; }
};

void my_translate(unsigned int code, _EXCEPTION_POINTERS *ep)
{
    throw SE_Exception();
}
#endif




OCPN_CompressionThreadEvent::OCPN_CompressionThreadEvent(wxEventType commandType, int id)
:wxEvent(id, commandType)
{
}

OCPN_CompressionThreadEvent::~OCPN_CompressionThreadEvent()
{
}

wxEvent* OCPN_CompressionThreadEvent::Clone() const
{
    OCPN_CompressionThreadEvent *newevent=new OCPN_CompressionThreadEvent(*this);
    newevent->m_ticket = this->m_ticket;
    return newevent;
}



class CompressionPoolThread : public wxThread
{
public:
    CompressionPoolThread(JobTicket *ticket, wxEvtHandler *message_target);
    void *Entry();
    
    wxEvtHandler        *m_pMessageTarget;
    JobTicket           *m_pticket;
};

CompressionPoolThread::CompressionPoolThread(JobTicket *ticket, wxEvtHandler *message_target)
{
    m_pMessageTarget = message_target;
    m_pticket = ticket;
    
    Create();
}

void * CompressionPoolThread::Entry()
{

#ifdef __MSVC__
    _set_se_translator(my_translate);

    //  On Windows, if anything in this thread produces a SEH exception (like access violation)
    //  we handle the exception locally, and simply alow the thread to exit smoothly with no results.
    //  Upstream will notice that nothing got done, and maybe try again later.
    
    try
#endif    
    {
    SetPriority( WXTHREAD_MIN_PRIORITY );
        
    unsigned char       *bit_array[10];
    for(int i=0 ; i < 10 ; i++)
        bit_array[i] = 0;
    
    wxRect ncrect(m_pticket->rect);

    //  Grab a copy of the level0 chart bits
    ChartBase *pchart;
    int index;
    
    if(ChartData){
        index =  ChartData->FinddbIndex( m_pticket->m_ChartPath );
        
        pchart = ChartData->OpenChartFromDBAndLock(index, FULL_INIT );
        
        if(pchart && ChartData->IsChartLocked( index )){
            ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );
            ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( pchart );
                
            if( pBSBChart ) {
                unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
                bit_array[0] = t_buf;

                pBSBChart->GetChartBits( ncrect, t_buf, 1 );
            }
            else if( pPlugInWrapper ){
                unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
                bit_array[0] = t_buf;
                    
                pPlugInWrapper->GetChartBits( ncrect, t_buf, 1 );
            }
            ChartData->UnLockCacheChart(index);
       }
       else
           bit_array[0] = NULL;
    }
    
    //OK, got the bits?
    if( bit_array[0] ){        
    
        //  Fill in the rest of the private uncompressed array

        int dim = g_GLOptions.m_iTextureDimension;
        dim /= 2;
        for( int i = 1 ; i < g_mipmap_max_level+1 ; i++ ){
            bit_array[i] = (unsigned char *) malloc( dim * dim * 3 );
            MipMap_24( 2*dim, 2*dim, bit_array[i - 1], bit_array[i] );
            dim /= 2;
        }
        
        //  Do the compression
        
        dim = g_GLOptions.m_iTextureDimension;
        int ssize = g_tile_size;
        for( int i = 0 ; i < g_mipmap_max_level+1 ; i++ ){
            unsigned char *tex_data = (unsigned char*)malloc(ssize);
            if(g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
            // color range fit is worse quality but twice as fast
                int flags = squish::kDxt1 | squish::kColourRangeFit;
            
                if( g_GLOptions.m_bTextureCompressionCaching) {
                /* use slower cluster fit since we are building the cache for
                * better quality, this takes roughly 25% longer and uses about
                * 10% more disk space (result doesn't compress as well with lz4) */
                    flags = squish::kDxt1 | squish::kColourClusterFit;
                }
            
                squish::CompressImageRGBpow2_Flatten_Throttle( bit_array[i], dim, dim, tex_data, flags,
                                                            true, m_pticket->b_throttle );
            
            }
            else if(g_raster_format == GL_ETC1_RGB8_OES) 
                CompressDataETC(bit_array[i], dim, ssize, tex_data);
            
            m_pticket->comp_bits_array[i] = tex_data;
            
            dim /= 2;
            ssize /= 4;
            if(ssize < 8)
                ssize = 8;

            if(m_pticket->b_abort){
                for( int i = 0; i < g_mipmap_max_level+1; i++ ){
                    free( bit_array[i] );
                    bit_array[i] = 0;
                }
                m_pticket->b_isaborted = true;
                goto SendEvtAndReturn;
            }
            
        }

        
        //  All done with the uncompressed data in the thread
        for( int i = 0; i < g_mipmap_max_level+1; i++ ){
            free( bit_array[i] );
            bit_array[i] = 0;
        }
       
        if(m_pticket->b_abort){
            m_pticket->b_isaborted = true;
            goto SendEvtAndReturn;
        }

        if(m_pticket->bpost_zip_compress) {
            
            int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
            int csize = g_tile_size;
            for( int i = 0 ; i < g_mipmap_max_level+1 ; i++ ){
                if(m_pticket->b_abort){
                    m_pticket->b_isaborted = true;
                    goto SendEvtAndReturn;
                }
                unsigned char *compressed_data = (unsigned char *)malloc(max_compressed_size);
                char *src = (char *)m_pticket->comp_bits_array[i];
                int compressed_size = LZ4_compressHC2( src, (char *)compressed_data, csize, 4);
                m_pticket->compcomp_bits_array[i] = compressed_data;
                m_pticket->compcomp_size_array[i] = compressed_size;
                
                csize /= 4;
                
            }
        }
    }
    else {
        m_pticket->b_isaborted = true;
    }

SendEvtAndReturn:
    
    if( m_pMessageTarget ) {
        OCPN_CompressionThreadEvent Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
        Nevent.SetTicket(m_pticket);
        
        m_pMessageTarget->AddPendingEvent(Nevent);
        // from here m_pticket is undefined (if deleted in event handler)
    }

    return 0;

    }           // try
    
#ifdef __MSVC__    
    catch (SE_Exception e)
    {
        if( m_pMessageTarget ) {
            OCPN_CompressionThreadEvent Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
            m_pticket->b_isaborted = true;
            Nevent.SetTicket(m_pticket);            
            m_pMessageTarget->AddPendingEvent(Nevent);
        }
        
        
        return 0;
    }
#endif    
    
}




#include <wx/listimpl.cpp>
WX_DEFINE_LIST(JobList);



//      CompressionWorkerPool Implementation
CompressionWorkerPool::CompressionWorkerPool()
{
    m_njobs_running = 0;
    int nCPU =  wxMax(1, wxThread::GetCPUCount());
    m_max_jobs =  nCPU;

    bthread_debug = false;

    if(bthread_debug)
        printf(" nCPU: %d    m_max_jobs :%d\n", nCPU, m_max_jobs);
    
    //  Create/connect a dynamic event handler slot for messages from the worker threads
    Connect( wxEVT_OCPN_COMPRESSIONTHREAD,
             (wxObjectEventFunction) (wxEventFunction) &CompressionWorkerPool::OnEvtThread );
    
}

CompressionWorkerPool::~CompressionWorkerPool()
{
}

void CompressionWorkerPool::OnEvtThread( OCPN_CompressionThreadEvent & event )
{
    JobTicket *ticket = event.GetTicket();
    
    if(ticket->b_abort){

        for(int i=0 ; i < g_mipmap_max_level+1 ; i++) {
            free(ticket->comp_bits_array[i]);
            free( ticket->compcomp_bits_array[i] );
        }
        
        m_njobs_running--;
        if(bthread_debug)
            printf( "    Abort job: %08X  Jobs running: %d             Job count: %lu   \n",
                    ticket->ident, m_njobs_running, (unsigned long)todo_list.GetCount());

        running_list.DeleteObject(ticket);
        delete ticket;
        StartTopJob();
        return;
    }
    
    //   Normal completion from here
    glTextureDescriptor *ptd = ticket->pFact->GetpTD( ticket->rect );

    if(ptd){
        ptd->FreeMap();

        for(int i=0 ; i < g_mipmap_max_level+1 ; i++)
            ptd->CompressedArrayAccess( CA_WRITE, ticket->comp_bits_array[i], i);
        
        if(ticket->bpost_zip_compress){
            for(int i=0 ; i < g_mipmap_max_level+1 ; i++){
                ptd->CompCompArrayAccess( CA_WRITE, ticket->compcomp_bits_array[i], i);
                ptd->compcomp_size[i] = ticket->compcomp_size_array[i];
            }
        }
    }
    
    m_njobs_running--;
    
    if(bthread_debug)
        printf( "    Finished job: %08X  Jobs running: %d             Job count: %lu   \n",
                ticket->ident, m_njobs_running, (unsigned long)todo_list.GetCount());

    running_list.DeleteObject(ticket);

//    int mem_used;
//    GetMemoryStatus(0, &mem_used);
    
///    qDebug() << "Finished" << m_njobs_running <<  (unsigned long)todo_list.GetCount() << mem_used << g_tex_mem_used;
    
    StartTopJob();

    delete ticket;
}

bool CompressionWorkerPool::ScheduleJob(glTexFactory* client, const wxRect &rect, int level,
                                        bool b_throttle_thread, bool b_immediate, bool b_postZip)
{
    if(!b_immediate && (todo_list.GetCount() >= 50) ){
//        int mem_used;
//        GetMemoryStatus(0, &mem_used);
        
///        qDebug() << "Could not add, count now" << (unsigned long)todo_list.GetCount() << mem_used << g_tex_mem_used;
        
        return false;
    }
    
    wxString chart_path = client->GetChartPath();

    //  Avoid adding duplicate jobs, i.e. the same chart_path, and the same rectangle
    wxJobListNode *node = todo_list.GetFirst();
    while(node){
        JobTicket *ticket = node->GetData();
        if( (ticket->m_ChartPath == chart_path) && (ticket->rect == rect)){
            return false;
        }
        
        node = node->GetNext();
    }
        
    JobTicket *pt = new JobTicket;
    pt->pFact = client;
    pt->rect = rect;
    pt->level_min_request = level;
    glTextureDescriptor *ptd = client->GetpTD( pt->rect );
    if(ptd)
        pt->ident = (ptd->tex_name << 16) + level;
    else
        pt->ident = -1;
    pt->b_throttle = b_throttle_thread;
    pt->m_ChartPath = chart_path;
    pt->b_abort = false;
    pt->bpost_zip_compress = b_postZip;

    if(!b_immediate){
        todo_list.Append(pt);
        if(bthread_debug){
            int mem_used;
            GetMemoryStatus(0, &mem_used);
            
            if(bthread_debug)
                printf( "Adding job: %08X  Job Count: %lu  mem_used %d\n", pt->ident, (unsigned long)todo_list.GetCount(), mem_used);
        }
 
//        int mem_used;
//        GetMemoryStatus(0, &mem_used);
 
///        qDebug() << "Added, count now" << (unsigned long)todo_list.GetCount() << mem_used << g_tex_mem_used;
        StartTopJob();
        return false;
    }
    else{
        DoJob(pt);
        delete pt;
        return true;
    }
}

bool CompressionWorkerPool::StartTopJob()
{
    //  Is it possible to start another job?
    if(m_njobs_running < m_max_jobs) {
        wxJobListNode *node = todo_list.GetFirst();
        if(node){
            JobTicket *ticket = node->GetData();
            todo_list.DeleteNode(node);
            m_njobs_running ++;
            running_list.Append(ticket);
            DoThreadJob(ticket);
            
            return true;
        }
        return false;
    }
    
    return false;
}


bool CompressionWorkerPool::DoThreadJob(JobTicket* pticket)
{
    if(bthread_debug)
        printf( "  Starting job: %08X  Jobs running: %d Jobs left: %lu\n", pticket->ident, m_njobs_running, (unsigned long)todo_list.GetCount());
    
///    qDebug() << "Starting job" << m_njobs_running <<  (unsigned long)todo_list.GetCount() << g_tex_mem_used;
    CompressionPoolThread *t = new CompressionPoolThread( pticket, this);
    pticket->pthread = t;
    
    t->Run();
    
    return true;
    
}



bool CompressionWorkerPool::DoJob(JobTicket* pticket)
{   
    bool ret = false;
    
    if(pticket->pFact){
        
        // Get the TextureDescriptor
        glTextureDescriptor *ptd = pticket->pFact->GetpTD( pticket->rect );
    
        if(ptd){
            //  Get the required chart bits for the requested compression
            GetFullMap( ptd, pticket->rect, pticket->pFact->GetChartPath(), pticket->level_min_request );
            
            DoCompress( pticket, ptd, pticket->level_min_request);
            ret = true;
            
        }
    }
    
    return ret;
}

bool CompressionWorkerPool::AsJob( wxString const &chart_path ) const
{
    if(chart_path.Len()){    
        wxJobListNode *tnode = running_list.GetFirst();
        while(tnode){
            JobTicket *ticket = tnode->GetData();
            if(ticket->m_ChartPath.IsSameAs(chart_path)){
                return true;
            }
            tnode = tnode->GetNext();
        }
    }
    return false;
}

void CompressionWorkerPool::PurgeJobList( wxString chart_path )
{
    if(chart_path.Len()){    
        //  Remove all pending jobs relating to the passed chart path
        wxJobListNode *tnode = todo_list.GetFirst();
        while(tnode){
            JobTicket *ticket = tnode->GetData();
            if(ticket->m_ChartPath.IsSameAs(chart_path)){
                if(bthread_debug)
                    printf("Pool:  Purge pending job for purged chart\n");
                todo_list.DeleteNode(tnode);
                delete ticket;
                tnode = todo_list.GetFirst();  // restart the list
            }
            else{
                tnode = tnode->GetNext();
            }
        }

        wxJobListNode *node = running_list.GetFirst();
        while(node){
            JobTicket *ticket = node->GetData();
            if(ticket->m_ChartPath.IsSameAs(chart_path)){
                ticket->b_isaborted = false;
                ticket->b_abort = true;
            }
            node = node->GetNext();
        }
            
        if(bthread_debug)
            printf("Pool:  Purge, todo count: %lu\n", (long unsigned)todo_list.GetCount());
    }
    else {
        wxJobListNode *node = todo_list.GetFirst();
        while(node){
            JobTicket *ticket = node->GetData();
            delete ticket;
            node = node->GetNext();
        }
        todo_list.Clear();
        //  Mark all running tasks for "abort"
        node = running_list.GetFirst();
        while(node){
            JobTicket *ticket = node->GetData();
            ticket->b_isaborted = false;
            ticket->b_abort = true;
            node = node->GetNext();
        }
    }        
}



//      CatalogEntry implementation
CatalogEntry::CatalogEntry()
{
}

CatalogEntry::~CatalogEntry()
{
}

CatalogEntry::CatalogEntry(int level, int x0, int y0, ColorScheme colorscheme)
{
    k.mip_level = level;
    k.x = x0;
    k.y = y0;
    k.tcolorscheme = colorscheme;
}

int CatalogEntry::GetSerialSize()
{
    return CATALOG_ENTRY_SERIAL_SIZE;
}

void CatalogEntry::Serialize( unsigned char *t)
{
    uint32_t *p = (uint32_t *)t;
    
    *p++ = k.mip_level;
    *p++ = k.x;
    *p++ = k.y;
    *p++ = k.tcolorscheme;
    *p++ = v.texture_offset;
    *p++ = v.compressed_size;
    
}


void CatalogEntry::DeSerialize( unsigned char *t)
{
    uint32_t *p = (uint32_t *)t;
    
     k.mip_level = *p++;
     k.x = *p++;
     k.y = *p++;
     k.tcolorscheme = (ColorScheme)*p++;
     v.texture_offset = *p++;
     v.compressed_size = *p++;
     
}
    



//      glTexFactory Implementation

BEGIN_EVENT_TABLE(glTexFactory, wxEvtHandler)
    EVT_TIMER(FACTORY_TIMER, glTexFactory::OnTimer)
END_EVENT_TABLE()

glTexFactory::glTexFactory(ChartBase *chart, int raster_format)
{
//    m_pchart = chart;
    n_catalog_entries = 0;
    m_catalog_offset = sizeof(CompressedCacheHeader);
    wxDateTime ed = chart->GetEditionDate();
    m_chart_date_binary = (uint32_t)ed.GetTicks();
    m_chartfile_date_binary = ::wxFileModificationTime(chart->GetFullPath());
    m_chartfile_size = (uint32_t)wxFileName::GetSize(chart->GetFullPath()).GetLo();
    
    m_CompressedCacheFilePath = CompressedCachePath(chart->GetFullPath());
    m_hdrOK = false;
    m_catalogOK = false;
    m_newCatalog = true;

    m_catalogCorrupted = false;

    m_fs = 0;
    m_LRUtime = 0;

    for (int i = 0; i < N_COLOR_SCHEMES; i++) {
        for (int j = 0; j < MAX_TEX_LEVEL; j++) {
            m_cache[i][j] = NULL;
        }
    }
    //  Initialize the TextureDescriptor array
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );
    
    if( !pBSBChart )
        return;
    
    m_size_X = pBSBChart->GetSize_X();
    m_size_Y = pBSBChart->GetSize_Y();
    
    m_ChartPath = chart->GetFullPath();
    
    //  Calculate the number of textures needed
    m_tex_dim = g_GLOptions.m_iTextureDimension;
    m_nx_tex = ( m_size_X / m_tex_dim ) + ((m_size_X % m_tex_dim) == 0 ? 0:1);
    m_ny_tex = ( m_size_Y / m_tex_dim ) + ((m_size_Y % m_tex_dim) == 0 ? 0:1);
    
    m_stride = m_nx_tex;
    m_ntex = m_nx_tex * m_ny_tex;
    m_td_array = (glTextureDescriptor **)calloc(m_ntex, sizeof(glTextureDescriptor *));
    
    {
        // we only want one pool but glTexFactory could be created from 
        // many concurrent threads in rebuildCache.
        wxMutexLocker lock(s_MutexPool);
         
        if(!g_CompressorPool)
            g_CompressorPool = new CompressionWorkerPool;
    }

    m_ticks = 0;
    // only the main thread can start timer
    if (wxThread::IsMain()) {
        m_timer.SetOwner(this, FACTORY_TIMER);
        m_timer.Start( 500 );
    }

    m_prepared_projection_type = 0;
    m_tiles = NULL;
}

glTexFactory::~glTexFactory()
{
    delete m_fs;

    if (wxThread::IsMain()) {
        PurgeBackgroundCompressionPool();
    }
    DeleteAllTextures();
    DeleteAllDescriptors();

    for (int i = 0; i < N_COLOR_SCHEMES; i++) {
        for (int j = 0; j < MAX_TEX_LEVEL; j++) {
            CatalogEntryValue *v = m_cache[i][j];
            if (v) {
                free(v);
            }
        }
    }
 
    free( m_td_array );         // array is empty

    if(m_tiles)
        for(int i=0 ; i < m_ntex; i++)
            delete m_tiles[i];
    delete [] m_tiles;
 }

glTextureDescriptor *glTexFactory::GetpTD( wxRect & rect )
{
    int array_index = ArrayIndex(rect.x, rect.y);
    return m_td_array[array_index];
}


void glTexFactory::DeleteTexture(const wxRect &rect)
{
    //    Is this texture tile defined?
    int array_index = ArrayIndex(rect.x, rect.y);
    glTextureDescriptor *ptd = m_td_array[array_index];

    
    if( ptd && ptd->tex_name > 0 ) {
        DeleteSingleTexture( ptd );
    }
}

void glTexFactory::DeleteAllTextures( void )
{
    // iterate over all the textures presently loaded
    // and delete the OpenGL texture from the GPU
    // but keep the private texture descriptor for now
  
    for(int i=0 ; i < m_ntex ; i++){
        glTextureDescriptor *ptd = m_td_array[i] ;
        
        if( ptd ) {
//            if(ptd->tex_name && bthread_debug)
//                printf("DAT::Delete Texture %d   resulting g_tex_mem_used, mb:  %ld\n", ptd->tex_name, g_tex_mem_used/(1024 * 1024));
            
            DeleteSingleTexture( ptd);
        }
    }
}


void glTexFactory::DeleteSomeTextures( long target )
{
    // iterate over all the textures presently loaded
    // and delete the OpenGL texture from the GPU
    // until the target g_tex_mem_used is reached
    // but keep the private texture descriptor for now
    
    for(int i=0 ; i < m_ntex ; i++){
        glTextureDescriptor *ptd = m_td_array[i] ;
        
        if( ptd ) {
//            if(ptd->tex_name && bthread_debug)
//                printf("DSoT::Delete Some Texture %d   resulting g_tex_mem_used, mb:  %ld\n", ptd->tex_name, g_tex_mem_used/(1024 * 1024));
            
            if(ptd->tex_name)
                DeleteSingleTexture( ptd);
        }
        
        if(g_tex_mem_used <= target)
            break;
    }
}

void glTexFactory::FreeSome( long target )
{
    for(int i=0 ; i < m_ntex ; i++){
        glTextureDescriptor *ptd = m_td_array[i] ;
        
        if( ptd ) {
            ptd->FreeAll();
        
            int mem_used;
            GetMemoryStatus(0, &mem_used);
        
            if(mem_used <= target)
                break;
        }
    }
}

void glTexFactory::DeleteAllDescriptors( void )
{
    // iterate over all the texture descriptors

    for(int i=0 ; i < m_ntex ; i++){
        glTextureDescriptor *ptd = m_td_array[i] ;
        delete ptd;
        m_td_array[i] = 0;
    }
    
}

bool glTexFactory::BackgroundCompressionAsJob() const
{
    if(g_CompressorPool) {
        return g_CompressorPool->AsJob( m_ChartPath );
    }
    return false;
}

void glTexFactory::PurgeBackgroundCompressionPool()
{
    //  Purge the "todo" list, and allow any running jobs to complete normally
    if(g_CompressorPool) {
        g_CompressorPool->PurgeJobList( m_ChartPath );
    }
}


void glTexFactory::DeleteSingleTexture( glTextureDescriptor *ptd )
{
    /* compute space saved */
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    if( ptd->nGPU_compressed == GPU_TEXTURE_UNCOMPRESSED)
        size = g_uncompressed_tile_size;
    
    for(int level = 0; level < g_mipmap_max_level + 1; level++) {
        if(ptd->miplevel_upload[level]) {
            g_tex_mem_used -= size;
            ptd->miplevel_upload[level] = false;
        }
        size /= 4;
        
        if(g_GLOptions.m_bTextureCompression && size < 8)
            size = 8;
    }
    
    ptd->level_min = g_mipmap_max_level + 1;  // default, nothing loaded
    

    if(ptd->tex_name) {
        glDeleteTextures( 1, &ptd->tex_name );
        ptd->tex_name = 0;
    }
    ptd->nGPU_compressed = GPU_TEXTURE_UNKNOWN;
    
    
    
}

bool glTexFactory::IsCompressedArrayComplete( int base_level, const wxRect &rect)
{
    int array_index = ArrayIndex(rect.x, rect.y);
    glTextureDescriptor *ptd = m_td_array[array_index];

    return IsCompressedArrayComplete( base_level, ptd);
}

bool glTexFactory::IsCompressedArrayComplete( int base_level, glTextureDescriptor *ptd)
{
    bool b_all_cmm_built = false;
    
    if(ptd) {
        
        //  It may be that the compressed mipmaps have been generated , uploaded, and free'ed
        if( ptd->nGPU_compressed == GPU_TEXTURE_COMPRESSED){
            b_all_cmm_built = true;
        }
        else {
            // are all required double-compressed levels available?
            b_all_cmm_built = true;
            for(int level = base_level; level < g_mipmap_max_level+1; level++ ) {
                if(NULL == ptd->CompCompArrayAccess( CA_READ, NULL, level)){
                    b_all_cmm_built = false;
                    break;
                }
            }
        }
    }
    
    return b_all_cmm_built;
}

void  glTexFactory::ArrayXY(wxRect *r, int index) const
{
    r->y = (index / m_stride)*m_tex_dim;
    r->x = (index -((r->y/m_tex_dim)*m_stride)) *m_tex_dim;
}

CatalogEntryValue *glTexFactory::GetCacheEntryValue(int level, int x, int y, ColorScheme color_scheme)
{
    if (level < 0 || level >= MAX_TEX_LEVEL)
        return 0;

    //  Look in the cache
    LoadCatalog();

    CatalogEntryValue *v = m_cache[color_scheme][level];
    if (v == 0)
        return 0;

    int array_index = ArrayIndex(x, y);
    if (array_index >= m_ntex)
        return 0;

    CatalogEntryValue *r = &v[array_index];
    if (r->compressed_size == 0)
        return 0;

    return r;    
}

bool glTexFactory::IsLevelInCache( int level, const wxRect &rect, ColorScheme color_scheme )
{
    bool b_ret = false;
    
    if(g_GLOptions.m_bTextureCompression &&
        g_GLOptions.m_bTextureCompressionCaching) {
         
    //  Search for the requested texture
        if (GetCacheEntryValue(level, rect.x, rect.y, color_scheme) != 0)
            b_ret = true;
    }
    
    return b_ret;
}

void glTexFactory::DoImmediateFullCompress(const wxRect &rect)
{
    int array_index = ArrayIndex(rect.x, rect.y);
    glTextureDescriptor *ptd = m_td_array[array_index];
    
    // if not found in the hash map, then get the bits as a texture descriptor
    if( !ptd ){
        glTextureDescriptor *p = new glTextureDescriptor;
        p->x = rect.x;
        p->y = rect.y;
        p->level_min = g_mipmap_max_level + 1;  // default, nothing loaded
        m_td_array[array_index] = p;
        ptd = p;
    }
    
    if(g_CompressorPool){
        for(int level = 0; level < g_mipmap_max_level + 1; level++ ) {
             g_CompressorPool->ScheduleJob( this, rect, level, false, true, false);  // immediate, no postZip
        }
    }
}

void glTexFactory::OnTimer(wxTimerEvent &event)
{
    m_ticks++;
    
    //  Scrub all the TD's, looking for any completed compression jobs that have
    //  not been written to disk
    //  In the interest of not disturbing the GUI, process only one TD per tick
    if(g_GLOptions.m_bTextureCompression && g_GLOptions.m_bTextureCompressionCaching) {
        for(int i=0 ; i < m_ntex ; i++){
            glTextureDescriptor *ptd = m_td_array[i];
            
            if( ptd && ptd->nCache_Color != m_colorscheme ){
                if( IsCompressedArrayComplete( 0, ptd) ){
                    bool work = UpdateCacheAllLevels( wxRect(ptd->x, ptd->y, g_GLOptions.m_iTextureDimension, g_GLOptions.m_iTextureDimension),
                                          m_colorscheme );
                    
                    // no longer need to store the compressed compressed data
                    ptd->FreeCompComp();
                    if (work) {
                        // we saved something reloaded the texture
                        // Now Delete the texture so it will be reloaded with compressed data
                        DeleteSingleTexture(ptd);

                        // We need to force a refresh to replace the uncompressed texture
                        // This frees video memory and is also really required if we had
                        // gone up a mipmap level
                        glChartCanvas::Invalidate(); // ensure we refresh
                        extern ChartCanvas *cc1;
                        cc1->Refresh();
                    }
                    else {
                        // XXX is that true?
                        ptd->nCache_Color = m_colorscheme;               // mark this TD as cached.
                    }
                    break;
                }
            }
        }
    }

#ifdef __OCPN__ANDROID__    
    bool bGLMemCrunch = g_tex_mem_used > 30/*g_GLOptions.m_iTextureMemorySize*/ * 1024 * 1024;
    
    if( bGLMemCrunch ){
        for(int i=0 ; i < m_ntex ; i++){
            glTextureDescriptor *ptd = m_td_array[i];
            if(ptd){
                if(ptd->nGPU_compressed == GPU_TEXTURE_UNCOMPRESSED){
                    DeleteSingleTexture(ptd);
                }
            }
        }
    }
#endif    
    
    // Once every minute, do more extensive garbage collection
    if(g_GLOptions.m_bTextureCompression && g_GLOptions.m_bTextureCompressionCaching) {
        if((m_ticks % 120) == 0){
            
            int mem_used;
            GetMemoryStatus(0, &mem_used);
            unsigned int nCache = 0;
            unsigned int lcache_limit = (unsigned int)g_nCacheLimit * 8 / 10;
            if(ChartData)
                nCache = ChartData->GetChartCache()->GetCount();
            
            bool mainMemCrunch = ( ((g_memCacheLimit > 0) && (mem_used > g_memCacheLimit * 7 / 10)) ||
            (g_nCacheLimit && (nCache > lcache_limit)) );
            
//            bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;

            if( mainMemCrunch ){
        //   Look for TDs where the compcomp array is full, and that the td has been marked as already cached
        //   This means that the TD was actually ready from cache, and uploaded to GPU already.
        //   It is thus safe to free all the memory for this TD.  If it is needed again, it will come
        //   quickly from the cache.
                for(int i=0 ; i < m_ntex ; i++){
                    glTextureDescriptor *ptd = m_td_array[i];
                    if(ptd){
                        if(ptd->GetCompCompArrayAlloc() && ( ptd->nCache_Color == m_colorscheme) ){
                            ptd->FreeAll();
                        }
                    }
                }
            }
        }
    }
    
#if 0
    if((m_ticks % 4/*120*/) == 0){
    
    // inventory
    int mem_total, mem_used;
    GetMemoryStatus(&mem_total, &mem_used);
    
    size_t map_size = 0;
    size_t comp_size = 0;
    size_t compcomp_size = 0;
    for(int i=0 ; i < m_ntex ; i++){
        glTextureDescriptor *ptd = m_td_array[i];
        if(ptd){
            map_size += ptd->GetMapArrayAlloc();
            comp_size += ptd->GetCompArrayAlloc();
            compcomp_size += ptd->GetCompCompArrayAlloc();
        }
    }

    size_t m1 = 1024 * 1024;
    printf("%6d %6ld Map: %6d  Comp:%6d  CompComp: %10d  %s\n", mem_used/1024, g_tex_mem_used/m1, map_size/m1, comp_size/m1, compcomp_size, m_ChartPath.mb_str().data());
  
///    qDebug() << "inv" << map_size/m1 << comp_size/m1 << compcomp_size/m1 << g_tex_mem_used/m1 << mem_used/1024;
    }
#endif

}

bool glTexFactory::PrepareTexture( int base_level, const wxRect &rect, ColorScheme color_scheme, bool b_throttle_thread)
{
    int array_index = ArrayIndex(rect.x, rect.y);
    glTextureDescriptor *ptd = m_td_array[array_index];

    m_colorscheme = color_scheme;
    
    try
    {
    
    // if not found in the hash map, then get the bits as a texture descriptor
    if( !ptd ){
        glTextureDescriptor *p = new glTextureDescriptor;
        p->x = rect.x;
        p->y = rect.y;
        p->level_min = g_mipmap_max_level + 1;  // default, nothing loaded
        m_td_array[array_index] = p;
        ptd = p;
    }
    else 
    {
        //  Now is a good time to update the cache, syncronously
        if(g_GLOptions.m_bTextureCompression && g_GLOptions.m_bTextureCompressionCaching) {
            if( ptd->nCache_Color != color_scheme ){
                if( IsCompressedArrayComplete( 0, ptd) ){
///                    g_Platform->ShowBusySpinner();
                
                    bool work = UpdateCacheAllLevels( rect, color_scheme );

                    // no longer need to store the compressed compressed data
                    ptd->FreeCompComp();
                    if (work) {
                        // Now Delete the texture so it will be reloaded with compressed data
                        DeleteSingleTexture(ptd);
                    }
                    ptd->nCache_Color = color_scheme;               // mark this TD as cached.
                }
            }
        }

        //  If we are not compressing/caching, We can do some housekeeping here, to recover some memory
        //   Free bitmap memory that has already been uploaded to the GPU
        if(!g_GLOptions.m_bTextureCompression) {
            for(int level = 0; level < g_mipmap_max_level+1; level++ ) {
                if(ptd->miplevel_upload[level]){
                    ptd->FreeAll();
                    break;
                }
            }
        }
    }
    
    //    If the GPU does not know about this texture, create it
    if( ptd->tex_name == 0 ) {
        glGenTextures( 1, &ptd->tex_name );
        
//        printf("gentex  %d   rect:  %d %d   index %d\n", ptd->tex_name, rect.x, rect.y, array_index);
        glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        
#ifdef ocpnUSE_GLES /* this is slightly faster */
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
#else /* looks nicer */
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
#endif
        
#ifdef __OCPN__ANDROID__
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif        
    }
    else
        glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
    
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    
        
    /* Some non-compliant OpenGL drivers need the complete mipmap set when using compressed textures */
    if( glChartCanvas::s_b_UploadFullMipmaps )
        base_level = 0;


    //  Texture requested has already been physically uploaded to the GPU
    //  so we are done
    if(base_level >= ptd->level_min){
//        if(ptd->miplevel_upload[0] && ptd->map_array[0])
        ptd->FreeAll();
        if(ptd->nGPU_compressed == GPU_TEXTURE_UNCOMPRESSED && g_GLOptions.m_bTextureCompression
                 && g_GLOptions.m_bTextureCompressionCaching) {
            
             if( (GL_COMPRESSED_RGBA_S3TC_DXT1_EXT == g_raster_format) ||
                 (GL_COMPRESSED_RGB_S3TC_DXT1_EXT == g_raster_format) ||
                 (GL_ETC1_RGB8_OES == g_raster_format) ){
                if(g_CompressorPool)
                    g_CompressorPool->ScheduleJob( this, rect, 0, b_throttle_thread, false, true);   // with postZip
             }
        }
        return true;
    }

///    g_Platform->ShowBusySpinner();
    
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    int uncompressed_size = g_uncompressed_tile_size;
    
    bool b_use_mipmaps = true;
    bool b_need_compress = false;

    for(int level = 0; level < g_mipmap_max_level+1; level++ ) {
        //    Upload to GPU?
        if( level >= base_level ) {
            int status = GetTextureLevel( ptd, rect, level, color_scheme );
            if (m_newCatalog) {
                 // it's an empty catalog or it's not used, odds it's going to be slow
                 OCPNPlatform::ShowBusySpinner();
                 m_newCatalog = false;
            }
            if(g_GLOptions.m_bTextureCompression) {
                if( (COMPRESSED_BUFFER_OK == status) && (ptd->nGPU_compressed != GPU_TEXTURE_UNCOMPRESSED ) ){
                    ptd->nGPU_compressed = GPU_TEXTURE_COMPRESSED;
                    if(!ptd->miplevel_upload[level]){
                        s_glCompressedTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                                          dim, dim, 0, size,
                                          ptd->CompressedArrayAccess( CA_READ, NULL, level));
                        ptd->miplevel_upload[level] = true;
                        g_tex_mem_used += size;
                        //if(bthread_debug)
                            //printf("Upload Compressed Texture %d  level: %d g_tex_mem_used: %ld \n", ptd->tex_name, level, g_tex_mem_used/(1024*1024));
                        
                        // We can safely discard this level's compressed data, since the GPU has it
                        ptd->FreeCompLevel( level );
                    }
                }                      
                else {
                    if(!ptd->miplevel_upload[level]){
                        
 //                       if(bthread_debug)
 //                                printf("UploadA Un-Compressed Texture %d  level: %d g_tex_mem_used: %ld\n", ptd->tex_name, level, g_tex_mem_used/(1024*1024));
 
                    //  This level has not been compressed yet, and is not in the cache
                    //  So, need to start a compression job 
                        
                        if( GL_COMPRESSED_RGB_FXT1_3DFX == g_raster_format ){
#if 1
                            // this version avoids re-uploading the data
                            ptd->nGPU_compressed = GPU_TEXTURE_COMPRESSED;
                            CompressUsingGPU( ptd, level, g_GLOptions.m_bTextureCompressionCaching, true);
                            g_tex_mem_used += size;
#else
                            ptd->nGPU_compressed = GPU_TEXTURE_UNCOMPRESSED;
                            CompressUsingGPU( ptd, level, g_GLOptions.m_bTextureCompressionCaching, false);
                            glTexImage2D( GL_TEXTURE_2D, level, GL_RGB,
                                          dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                            g_tex_mem_used += uncompressed_size;
#endif
                            ptd->miplevel_upload[level] = true;
                            
                        }
                        else {
                            ptd->nGPU_compressed = GPU_TEXTURE_UNCOMPRESSED;
                            b_need_compress = true;

#if 1
                        // in this version, we upload only the current level and don't use mipmaps
                        // temporarily while the compressed texture is being generated
                        // This is significant for memory consumption on opengles where a full mipmap
                        // stack is required in underzoom as we can avoid pushing  the uncompressed size
                        // into texture memory which is significant
                        // should be ok because the uncompressed data is only temporary anyway
                        b_use_mipmaps = false;

#ifdef ocpnUSE_GLES
                        // on systems with little memory we can go up a mipmap level
                        // here so that the uncompressed size without mipmaps (level+1)
                        // is nearly the compressed size with all the compressed mipmaps
                        // this way we won't require 5x more video memory than normal while we
                        // are generating the compressed textures, when the cache is complete the image
                        // becomes clearer as it is replaces with the higher resolution compressed version
                        base_level++;
                        level++;
                        if(ptd->miplevel_upload[level])
                            break;
                        GetTextureLevel( ptd, rect, level, color_scheme );
                        dim /= 2;
                        uncompressed_size /= 4;
#endif

                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
                                  dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                        ptd->miplevel_upload[level] = true;

                        g_tex_mem_used += uncompressed_size;
                    
                        // recompute texture memory if we replaced an existing uncompressed level
                        for(level++; level < g_mipmap_max_level+1; level++ ) {
                            uncompressed_size /= 4;
                            if(ptd->miplevel_upload[level]) {
                                ptd->miplevel_upload[level] = false;
                                g_tex_mem_used -= uncompressed_size;
                            }
                        }

                        //if(bthread_debug)
                            //printf("UploadB Un-Compressed Texture %d  level: %d g_tex_mem_used: %ld\n", ptd->tex_name, level, g_tex_mem_used/(1024*1024));
                        
                        break;
#else // this version uses mipmaps
                        glTexImage2D( GL_TEXTURE_2D, level, GL_RGB,
                                  dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                        ptd->miplevel_upload[level] = true;
#endif
                        }
                    }
                }
            }
            else {              // not compresssed
                if(!ptd->miplevel_upload[level]){
 //                   if(bthread_debug)
 //                       printf("Upload Un-Compressed Texture %d  level: %d g_tex_mem_used: %ld, data ptr: %p\n", ptd->tex_name, level, g_tex_mem_used/(1024*1024), ptd->map_array[level]);
                    ptd->nGPU_compressed = GPU_TEXTURE_UNCOMPRESSED;
                    glTexImage2D( GL_TEXTURE_2D, level, GL_RGB,
                                dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                    g_tex_mem_used += uncompressed_size;
                    ptd->miplevel_upload[level] = true;
                }
            }
        }

        dim /= 2;
        size /= 4;
        if(size < 8) size = 8;
        uncompressed_size /= 4;
    }

#ifndef ocpnUSE_GLES
    if(b_use_mipmaps) {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, g_mipmap_max_level );
    }
#endif
    ptd->level_min = base_level;
    
    if(b_need_compress){
         if( (GL_COMPRESSED_RGBA_S3TC_DXT1_EXT == g_raster_format) ||
             (GL_COMPRESSED_RGB_S3TC_DXT1_EXT == g_raster_format) ||
             (GL_ETC1_RGB8_OES == g_raster_format) ){
                if(g_CompressorPool)
                    g_CompressorPool->ScheduleJob( this, rect, 0, b_throttle_thread, false, true);   // with postZip
        }
    }
    
    //   If global memory is getting short, we can crunch here.
    //   All mipmaps >= ptd->level_min have been uploaded to the GPU,
    //   so there is no reason to save the bits forever.
    //   Of course, this means that if the texture is deleted elsewhere, then the bits will need to be
    //   regenerated.  The price to pay for memory limits....
    
    int mem_used;
    GetMemoryStatus(0, &mem_used);
    //    qDebug() << mem_used;
    
    unsigned int nCache = 0;
    unsigned int lcache_limit = (unsigned int)g_nCacheLimit * 8 / 10;
    if(ChartData)
        nCache = ChartData->GetChartCache()->GetCount();
    
    if( ((g_memCacheLimit > 0) && (mem_used > g_memCacheLimit * 7 / 10)) ||
        (g_nCacheLimit && (nCache > lcache_limit)) )
      
    {
        ptd->FreeAll();
    }
    
//    g_Platform->HideBusySpinner();
    
    return true;
    }   //try
    
    catch (...)
    {
        //      Clean up
        ptd->FreeAll();
        DeleteSingleTexture( ptd );
        return false;
    }
}

void glTexFactory::PrepareTiles(const ViewPort &vp, bool use_norm_vp, ChartBase *chart)
{
    ChartBaseBSB *pChartBSB = dynamic_cast<ChartBaseBSB*>( chart );
    ChartPlugInWrapper *cpw = NULL;
    if(chart->GetChartType() == CHART_TYPE_PLUGIN){
        cpw = dynamic_cast<ChartPlugInWrapper*> ( chart );
        if( !cpw) return;
    }
    else if( !pChartBSB ) return;
    
    // detect changing north/south polar
    if(vp.m_projection_type == PROJECTION_POLAR) {
        bool north = vp.clat > 0;
        if(m_north != north)
            m_prepared_projection_type = 0;
        m_north = north;
    }

    if(vp.m_projection_type == m_prepared_projection_type)
        return;

    m_prepared_projection_type = vp.m_projection_type;

    double native_scale;
    
    native_scale = pChartBSB->GetNativeScale();

    if(m_tiles)
        for(int i=0 ; i < m_ntex; i++)
            delete m_tiles[i];
    delete [] m_tiles;
    m_tiles = new glTexTile*[m_ntex];

    int tex_dim = g_GLOptions.m_iTextureDimension;

    // split cells for accuracy, much more with larger charts, and toward poles
    // depending on projection of viewport and chart
    // This is a very simplistic algorithm to determine split count, could be greatly improved

    double xsplits, ysplits;
    switch(vp.m_projection_type) {
    case PROJECTION_POLAR:
    case PROJECTION_STEREOGRAPHIC:
    case PROJECTION_ORTHOGRAPHIC:
    case PROJECTION_GNOMONIC:
    case PROJECTION_POLYCONIC:
        xsplits = native_scale / 1000000000.0 * tex_dim; // todo: fix this
//        xsplits /= (1 << base_level); // split less zoomed out
        
        // split more near poles
        if(vp.m_projection_type == PROJECTION_ORTHOGRAPHIC) {
            Extent e;
            if(cpw)
                cpw->GetChartExtent(&e);
            else
                pChartBSB->GetChartExtent(&e);
            xsplits = xsplits * wxMax(fabsf(e.NLAT), fabsf(e.SLAT)) / 90;
        }
        
        xsplits = round(xsplits);
        ysplits = 2*xsplits;
        
        xsplits = wxMin(wxMax(xsplits, 1), 8);
        ysplits = wxMin(wxMax(ysplits, 1), 8);
        break;
    case PROJECTION_EQUIRECTANGULAR:
        // needed for skewed charts?
//        xsplits = ysplits = 4;
//        break;
    default:
        xsplits = ysplits = 1; // TODO: is this good enough in all cases to reproject
        // non-mercator charts or even SM_ECC mercator charts in all cases?
    }

    ViewPort nvp;
    if(use_norm_vp) {
        if(cpw)
            cpw->chartpix_to_latlong(m_size_X/2, m_size_Y/2, &m_clat, &m_clon);
        else
            pChartBSB->chartpix_to_latlong(m_size_X/2, m_size_Y/2, &m_clat, &m_clon);
        nvp = glChartCanvas::NormalizedViewPort(vp, m_clat, m_clon);
    }

    //    Using a 2D loop, iterate thru the texture tiles of the chart
    wxRect rect;
    rect.y = 0;
    for( int i = 0; i < m_ny_tex; i++ ) {
        rect.height = tex_dim;
        rect.x = 0;
        for( int j = 0; j < m_nx_tex; j++ ) {
            rect.width = tex_dim;

            glTexTile *tile = m_tiles[i*m_nx_tex + j] = new glTexTile;
            tile->rect = rect;

            double lat, lon;
            float ll[8];
            int x[4] = {rect.x, rect.x, rect.x+rect.width, rect.x+rect.width};
            int y[4] = {rect.y+rect.height, rect.y, rect.y, rect.y+rect.height};

            for(int k=0; k<4; k++) {
                if(cpw)
                    cpw->chartpix_to_latlong(x[k], y[k], &lat, &lon);
                else
                    pChartBSB->chartpix_to_latlong(x[k], y[k], &lat, &lon);
                ll[2*k+0] = lon, ll[2*k+1] = lat;
            }

            // resolve idl
            float lonmin = ll[0], lonmax = ll[0];
            float latmin = ll[1], latmax = ll[1];
            for(int i=2; i<8; i+=2) {
                lonmin = wxMin(lonmin, ll[i]), lonmax = wxMax(lonmax, ll[i]);
                latmin = wxMin(latmin, ll[i+1]), latmax = wxMax(latmax, ll[i+1]);
            }

            if(fabsf(lonmin - lonmax) > 180) {
                lonmin = 540, lonmax = 0; 
                for(int i=0; i<8; i+=2) {
                    float lon = ll[i] < 0 ? ll[i]+360 : ll[i];
                    lonmin = wxMin(lonmin, lon), lonmax = wxMax(lonmax, lon);
                }
            }

            tile->box.Set(latmin, lonmin, latmax, lonmax);

            double sx = rect.width;
            double sy = rect.height;
                                
            double xs = rect.width / xsplits;
            double ys = rect.height / ysplits;
            double x1 = rect.x, u1 = 0;

            int maxncoords = 4*xsplits*ysplits;
            tile->m_coords = new float[2*maxncoords];
            tile->m_texcoords = new float[2*maxncoords];

            tile->m_ncoords = 0;

            int end = 0; // should be 1<<base_level but we have no way to know now

            for(int x = 0; x<xsplits; x++) {
                double x2 = wxMin(x1+xs, m_size_X - end);
                double u2 = (x2-rect.x)/rect.width;

                double y1 = rect.y, v1 = 0;
                for(int y = 0; y<ysplits; y++) {
                    double y2 = wxMin(y1+ys, m_size_Y - end);
                    double v2 = (y2-rect.y)/rect.height;

                    // todo avoid extra calls per loop and also caching from above
                    double xc[4] = {x1, x1, x2, x2}, yc[4] = {y2, y1, y1, y2};
                    double lat[4], lon[4];
                    for(int k=0; k<4; k++){
                        if(cpw)
                            cpw->chartpix_to_latlong(xc[k], yc[k], lat+k, lon+k);
                        else
                            pChartBSB->chartpix_to_latlong(xc[k], yc[k], lat+k, lon+k);
                    }

                    double u[4] = {u1, u1, u2, u2}, v[4] = {v2, v1, v1, v2};
                    for(int j=0; j<4; j++) {
                        int idx = 2*tile->m_ncoords;
                        tile->m_texcoords[idx+0] = u[j];
                        tile->m_texcoords[idx+1] = v[j];

                        if(use_norm_vp) {
                            wxPoint2DDouble p = nvp.GetDoublePixFromLL(lat[j], lon[j]);
                            tile->m_coords[idx+0] = p.m_x;
                            tile->m_coords[idx+1] = p.m_y;
                        } else {
                            tile->m_coords[idx+0] = lat[j];
                            tile->m_coords[idx+1] = lon[j];
                        }
                        tile->m_ncoords++;
                    }

                    if(y1 + ys > m_size_Y - end)
                        break;
                        
                    v1 = v2;
                    y1 = y2;
                }
                if(x1 + xs > m_size_X - end)
                    break;
                
                u1 = u2;
                x1 = x2;
            }
            rect.x += rect.width;
        }
        rect.y += rect.height;
    }
}


bool glTexFactory::UpdateCacheLevel( const wxRect &rect, int level, ColorScheme color_scheme, bool write_catalog )
{
    if( !g_GLOptions.m_bTextureCompressionCaching)
        return false;
    //  Search for the requested texture
        //  Search the catalog for this particular texture
    CatalogEntryValue *v = GetCacheEntryValue(level, rect.x, rect.y, color_scheme) ;
        
    //      This texture is already done
    if(v != 0)
        return false;
    
    //    Is this texture tile already defined?
    int array_index = ArrayIndex(rect.x, rect.y);
    glTextureDescriptor *ptd = m_td_array[array_index];
    
    if(!ptd)
        return false;
        

    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    
    for(int i=0 ; i < level ; i++){
        dim /= 2;
        size /= 4;
        if(size < 8)
            size = 8;
    }
    
    unsigned char *pd = ptd->CompCompArrayAccess(CA_READ, NULL, level);
    if(pd){
        return UpdateCachePrecomp(pd, ptd->compcomp_size[level], ptd, level, color_scheme, write_catalog);
    }
    else {
        unsigned char *source = ptd->CompressedArrayAccess( CA_READ, NULL, level);
        if(source)
            return UpdateCache(source, size, ptd, level, color_scheme, write_catalog);
    }
    return false;
}

bool glTexFactory::UpdateCacheAllLevels( const wxRect &rect, ColorScheme color_scheme )
{
    if( !g_GLOptions.m_bTextureCompressionCaching)
        return false;
    bool work = false;
    for (int level = 0; level < g_mipmap_max_level + 1; level++ )
        work |= UpdateCacheLevel( rect, level, color_scheme, false );
    if (work) {
        WriteCatalogAndHeader();
    }
    return work;
}

int glTexFactory::GetTextureLevel( glTextureDescriptor *ptd, const wxRect &rect,
                                   int level, ColorScheme color_scheme )
{
    //  Already available in the texture descriptor?
    if(g_GLOptions.m_bTextureCompression && ptd->nGPU_compressed != GPU_TEXTURE_UNCOMPRESSED) {
        if( ptd->CompressedArrayAccess( CA_READ, NULL, level))
            return COMPRESSED_BUFFER_OK;
        else if(!g_GLOptions.m_bTextureCompressionCaching) {
            if(g_CompressorPool){
                g_CompressorPool->ScheduleJob( this, rect, level, false, true, false);  // immediate, no postZip
                return COMPRESSED_BUFFER_OK;
            }
        } else {
            //  If cacheing compressed textures, look in the cache
            //  Search for the requested texture
            //  Search the catalog for this particular texture
            CatalogEntryValue *p = GetCacheEntryValue(level, rect.x, rect.y, color_scheme);
        
            //      Requested texture level is found in the cache
            //      so go load it
            if( p != 0 ) {
            
                int dim = g_GLOptions.m_iTextureDimension;
                int size = g_tile_size;
            
                for(int i=0 ; i < level ; i++){
                    dim /= 2;
                    size /= 4;
                    if(size < 8) size = 8;
                }
            
                if(m_fs->IsOpened()){
                    m_fs->Seek(p->texture_offset);
                    unsigned char *cb = (unsigned char*)malloc(size);
                    ptd->CompressedArrayAccess( CA_WRITE, cb, level);
                
                    int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
                    char *compressed_data = new char[max_compressed_size];
                    m_fs->Read(compressed_data, p->compressed_size);
                
                    LZ4_decompress_fast(compressed_data, (char*)cb, size);
                    delete [] compressed_data;    
                }
            
                return COMPRESSED_BUFFER_OK;
            }
        }
    }
        
    //  Requested Texture level is not in cache, and not already built
    //  So go build it
    if( !ptd->map_array[level] )
        GetFullMap( ptd, rect, m_ChartPath, level );
        
    return MAP_BUFFER_OK;
}


// return not used
// false? never
// true 
bool glTexFactory::LoadHeader(void)
{
    if(m_hdrOK)
        return true;

    bool need_new = false;

    if(wxFileName::FileExists(m_CompressedCacheFilePath)) {
        
        m_fs = new wxFFile(m_CompressedCacheFilePath, _T("rb+"));
        if(m_fs->IsOpened()){
        
            CompressedCacheHeader hdr;
            
            //  Header is located at the end of the file
            wxFileOffset hdr_offset = m_fs->Length() -sizeof( hdr);
            hdr_offset = m_fs->Seek( hdr_offset );
            
            if( sizeof( hdr) == m_fs->Read(&hdr, sizeof( hdr ))) {
                if( hdr.magic != COMPRESSED_CACHE_MAGIC ||
                    hdr.chartdate != m_chart_date_binary ||
                    hdr.chartfile_date != m_chartfile_date_binary ||
                    hdr.chartfile_size != m_chartfile_size ||
                    hdr.format != g_raster_format) {
                    
                    //  Bad header signature    
                    delete m_fs;
                    need_new = true;
                }
                else {      // good header
                    n_catalog_entries = hdr.m_nentries;
                    m_catalog_offset = hdr.catalog_offset;
                }
            }
            else{  // file exists, and is empty
                n_catalog_entries = 0;
                m_catalog_offset = 0;
                WriteCatalogAndHeader();
            }
        }  // is open
        
        else{               // some problem opening file, probably permissions on Win7
            delete m_fs;
            need_new = true;
            wxRemoveFile(m_CompressedCacheFilePath);
        }
        
    }   // exists
    
    else {   // File does not exist
        wxFileName fn(m_CompressedCacheFilePath);
        if(!fn.DirExists())
            fn.Mkdir();
        need_new = true;
    }

    if (need_new) {
        //  Create new file, with empty catalog, and correct header
        m_fs = new wxFFile(m_CompressedCacheFilePath, _T("wb"));
        n_catalog_entries = 0;
        m_catalog_offset = 0;
        WriteCatalogAndHeader();
        delete m_fs;

        m_fs = new wxFFile(m_CompressedCacheFilePath, _T("rb+"));
    }
    m_hdrOK = true;
    return true;
}

bool glTexFactory::AddCacheEntryValue(const CatalogEntry &p)
{
    if ((int)p.k.tcolorscheme < 0 || p.k.tcolorscheme >= N_COLOR_SCHEMES) 
        return false;

    if (p.k.mip_level < 0 || p.k.mip_level >= MAX_TEX_LEVEL) 
        return false;

    int array_index = ArrayIndex(p.k.x, p.k.y);
    if (array_index < 0 || array_index >= m_ntex) 
        return false;

    if (m_cache[p.k.tcolorscheme][p.k.mip_level] == 0) 
        m_cache[p.k.tcolorscheme][p.k.mip_level] = (CatalogEntryValue* )calloc(m_ntex, sizeof (CatalogEntryValue) );

    CatalogEntryValue *v = m_cache[p.k.tcolorscheme][p.k.mip_level];
    CatalogEntryValue *r = &v[array_index];
    *r = p.v; 
    return true;
}

bool glTexFactory::LoadCatalog(void)
{
    m_newCatalog = false;
    if(m_catalogOK)
        return true;

    if( !LoadHeader() )
        return false;
    
    if (n_catalog_entries == 0) {
        // new empty header
        m_catalogOK = true;
        m_newCatalog = true;
        return true;
    }
    
    m_fs->Seek(m_catalog_offset);
 
    CatalogEntry ps;
    int buf_size =  ps.GetSerialSize();
    unsigned char *buf = (unsigned char *)malloc(buf_size);
    
    CatalogEntry p;
    bool bad = false;
    for(int i=0 ; i < n_catalog_entries ; i++){
        m_fs->Read(buf, buf_size);
        p.DeSerialize(buf);
        if (!AddCacheEntryValue(p))
            bad = true;
    }
    
    free(buf);
    if (bad && !m_catalogCorrupted) {
        wxLogMessage(_T("Bad cache catalog %s %s"), m_ChartPath.c_str(), m_CompressedCacheFilePath.c_str());
        m_catalogCorrupted = true;
    }
    m_catalogOK = true;
    return true;
}


bool glTexFactory::WriteCatalogAndHeader()
{
    if(m_fs && m_fs->IsOpened()){

        m_fs->Seek(m_catalog_offset);
        
        CatalogEntry ps;
        int buf_size =  ps.GetSerialSize();
        unsigned char buf[CATALOG_ENTRY_SERIAL_SIZE];   // MSVC requires constant stack array size...
        int new_n_catalog_entries = 0;
        CatalogEntry p;
        wxRect rect;
        for (int i = 0; i < N_COLOR_SCHEMES; i++) {
            p.k.tcolorscheme = (ColorScheme)i;
            for (int j = 0; j < MAX_TEX_LEVEL; j++) {
                CatalogEntryValue *v = m_cache[i][j];
                if (!v) 
                    continue;
                p.k.mip_level = j;
                for (int k = 0; k < m_ntex; k++) {
                    ArrayXY(&rect, k);
                    p.k.y = rect.y;
                    p.k.x = rect.x;
                    CatalogEntryValue *r = &v[k];
                    if (r->compressed_size == 0)
                        continue;
                    p.v = *r;
                    new_n_catalog_entries++;
                    p.Serialize(buf);
                    m_fs->Write( buf, buf_size);

                }
            }
        }

        n_catalog_entries = new_n_catalog_entries;
        //   Write header at file end
        CompressedCacheHeader hdr;
        hdr.magic = COMPRESSED_CACHE_MAGIC;
        hdr.format = g_raster_format;
        hdr.m_nentries = n_catalog_entries;
        hdr.catalog_offset = m_catalog_offset;
        hdr.chartdate = m_chart_date_binary;
        hdr.chartfile_date = m_chartfile_date_binary;
        hdr.chartfile_size = m_chartfile_size;
        
        m_fs->Write( &hdr, sizeof(hdr));
        m_fs->Flush();
        
        return true;
    }
    else
        return false;
}

// return false on error (currently not used by callers)
bool glTexFactory::UpdateCache(unsigned char *data, int data_size, glTextureDescriptor *ptd, int level,
                               ColorScheme color_scheme, bool write_catalog)
{
    if (level < 0 || level >= MAX_TEX_LEVEL)
        return false;	// XXX BUG wrong level, assert ?

    //  Search the catalog for this particular texture
    if (GetCacheEntryValue(level, ptd->x, ptd->y, color_scheme) != 0) 
        return false;

    // not found, so add it
    // Make sure the file exists
    wxASSERT(m_fs != 0);
    
    if( !m_fs->IsOpened() )
        return false;

    //      Create a new catalog entry
    CatalogEntry p( level, ptd->x, ptd->y, color_scheme);
    
    n_catalog_entries++;
    
    //      Write the compressed data to disk
     p.v.texture_offset = m_catalog_offset;
    
    int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
    char *compressed_data = new char[max_compressed_size];
    
    int compressed_size = LZ4_compressHC2((char*)data, compressed_data, data_size, 4);
    p.v.compressed_size = compressed_size;
    AddCacheEntryValue(p);
    
    //      We write the new data at the current catalog offset, overwriting the old catalog
    m_fs->Seek( m_catalog_offset );
    m_fs->Write( compressed_data, compressed_size );
    
    delete [] compressed_data;
    
    //      Write the catalog and Header (which follows the catalog at the end of the file
    m_catalog_offset += compressed_size;
    if (write_catalog)
        WriteCatalogAndHeader();
    
    return true;
}

bool glTexFactory::UpdateCachePrecomp(unsigned char *data, int data_size, glTextureDescriptor *ptd, int level,
                                      ColorScheme color_scheme, bool write_catalog)
{
    if (level < 0 || level >= MAX_TEX_LEVEL)
        return false;	// XXX BUG

    //  Search the catalog for this particular texture
    if (GetCacheEntryValue(level, ptd->x, ptd->y, color_scheme) != 0) 
        return false;
    
    // Make sure the file exists
    wxASSERT(m_fs != 0);
        
    if( ! m_fs->IsOpened() )
        return false;

    //      Create a new catalog entry
    CatalogEntry p( level, ptd->x, ptd->y, color_scheme);
    
    //      Write the compressed data to disk
    p.v.texture_offset = m_catalog_offset;
    
    p.v.compressed_size = data_size;
    AddCacheEntryValue(p);            
    n_catalog_entries++;

    //      We write the new data at the current catalog offset, overwriting the old catalog
    m_fs->Seek( m_catalog_offset );
    m_fs->Write( data, data_size );
    
    //      Write the catalog and Header (which follows the catalog at the end of the file
    m_catalog_offset += data_size;
    if (write_catalog)
        WriteCatalogAndHeader();
    
    return true;
}


bool CompressUsingGPU( glTextureDescriptor *ptd, int level, bool b_post_comp, bool inplace)
{
    if(!ptd)
        return false;
    
    if( !s_glGetCompressedTexImage )
        return false;
    
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    
    for(int i=0 ; i < level ; i++){
        dim /= 2;
        size /= 4;
        if(size < 8)
            size = 8;
    }
    
    bool ret = false;
    GLuint comp_tex;
    if(!inplace) {
        glGenTextures(1, &comp_tex);
        glBindTexture(GL_TEXTURE_2D, comp_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    glTexImage2D(GL_TEXTURE_2D, level, GL_COMPRESSED_RGB_FXT1_3DFX,
                 dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, ptd->map_array[level]);
    
    GLint compressed;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
    
    /* if the compression has been successful */
    if (compressed == GL_TRUE){
        
        // If our compressed size is reasonable, save it.
        GLint compressedSize;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
                                 GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
                                 &compressedSize);
        
        
        if ((compressedSize > 0) && (compressedSize < 100000000)) {
            
            // Allocate a buffer to read back the compressed texture.
            unsigned char *compressedBytes = (unsigned char *)malloc(sizeof(GLubyte) * compressedSize);
            
            // Read back the compressed texture.
            s_glGetCompressedTexImage(GL_TEXTURE_2D, 0, compressedBytes);
            
            // Save the compressed texture pointer in the ptd
            ptd->CompressedArrayAccess( CA_WRITE, compressedBytes, level);
            
            
            // ZIP compress the data for disk storage
            if(b_post_comp){int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
                if(max_compressed_size){
                    unsigned char *compressed_data = (unsigned char *)malloc(max_compressed_size);
                    int compressed_size = LZ4_compressHC2( (char *)ptd->CompressedArrayAccess( CA_READ, NULL, level),
                                                        (char *)compressed_data, size, 4);
                    ptd->CompCompArrayAccess( CA_WRITE, compressed_data, level);
                    ptd->compcomp_size[level] = compressed_size;
                }
            }
        }
        
        ret = true;
    }

    if(!inplace)
        glDeleteTextures(1, &comp_tex);
    
    // Restore the old texture pointer
    glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
    
    return ret;
}
