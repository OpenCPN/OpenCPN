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

#ifdef __OCPN__ANDROID__
#include <qopengl.h>
#include "GL/gl_private.h"
#else
#include "GL/gl.h"
#endif


#include "glTexCache.h"

#include "glChartCanvas.h"
#include "chartbase.h"
#include "chartimg.h"
#include "chartdb.h"
#include "OCPNPlatform.h"

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
    int         m_raster_format;
    unsigned char **comp_bits_array;
    wxString    m_ChartPath;
    bool        b_abort;
    bool        b_isaborted;
    bool        bpost_zip_compress;
    unsigned char **compcomp_bits_array;
    int         compcomp_size_array[10];
    
};

JobTicket::JobTicket()
{
    for(int i=0 ; i < 10 ; i++)
        compcomp_size_array[i]=0;
    
    comp_bits_array = NULL;
    compcomp_bits_array = NULL;
}



/* generate mipmap in software */
void HalfScaleChartBits( int width, int height, unsigned char *source, unsigned char *target )
{
    int newwidth = width / 2;
    int newheight = height / 2;
    int stride = width * 3;
    
    unsigned char *s = target;
    unsigned char *t = source;
    // Average 4 pixels
    for( int i = 0; i < newheight; i++ ) {
        for( int j = 0; j < newwidth; j++ ) {
            
            for( int k = 0; k < 3; k++ ) {
                s[0] = ( *t + *( t + 3 ) + *( t + stride ) + *( t + stride + 3 ) ) / 4;
                s++;
                t += 1;
            }
            t += 3;
        }
        t += stride;
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
    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( pChart );
        
    if( !pPlugInWrapper && !pBSBChart ) {
        ptd->map_array[0] = (unsigned char *) calloc( ncrect.width * ncrect.height * 4, 1 );
        return;
    }
    
    if( pBSBChart ) {
        unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
        pBSBChart->GetChartBits( ncrect, t_buf, 1 );
        
        //    and cache them here
        ptd->map_array[0] = t_buf;
    }
    else if( pPlugInWrapper ){
        unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
        pPlugInWrapper->GetChartBits( ncrect, t_buf, 1 );
        
        //    and cache them here
        ptd->map_array[0] = t_buf;
    }
}



void GetFullMap( glTextureDescriptor *ptd,  const wxRect &rect, wxString chart_path, int level)
{
    
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    
    for(int i=0 ; i < level ; i++){
        dim /= 2;
        size /= 4;
        if(size < 8)
            size = 8;
        
    }
    
    
    //  Confirm that the uncompressed bits are all available, get them if not there yet
    if( !ptd->map_array[level]) {
        
        
        if( level > 0 && ptd->map_array[level - 1] ){
            ptd->map_array[level] = (unsigned char *) malloc( dim * dim * 3 );
            HalfScaleChartBits( 2*dim, 2*dim, ptd->map_array[level - 1], ptd->map_array[level] );
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
                        HalfScaleChartBits( 2*dimh, 2*dimh, ptd->map_array[i_lev - 1], ptd->map_array[i_lev] );
                    }
                    dimh /= 2;
                    i_lev++;
                }
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
        GLuint raster_format = pticket->pFact->GetRasterFormat();
    
        unsigned char *tex_data = (unsigned char*)malloc(size);
        
        if(raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
            // color range fit is worse quality but twice as fast
            int flags = squish::kDxt1 | squish::kColourRangeFit;
            
            if( g_GLOptions.m_bTextureCompressionCaching) {
                /* use slower cluster fit since we are building the cache for
                 * better quality, this takes roughly 25% longer and uses about
                 * 10% more disk space (result doesn't compress as well with lz4) */
                flags = squish::kDxt1 | squish::kColourClusterFit;
            }

            bool b_flip = false;
//#ifdef __WXMSW__ //ocpnUSE_ocpnBitmap
//            b_flip = true;
//#endif            
            squish::CompressImageRGB_Flatten_Flip_Throttle( ptd->map_array[level], dim, dim, tex_data, flags,
                                                            true, b_flip, pticket->b_throttle );
 
        }
        else if(raster_format == GL_ETC1_RGB8_OES) 
            CompressDataETC(ptd->map_array[level], dim, size, tex_data);
        
        else if(raster_format == GL_COMPRESSED_RGB_FXT1_3DFX) {
            CompressUsingGPU( ptd, raster_format, level, false);    // no post compression
            
        }

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
    unsigned char       **m_comp_bits;
    unsigned char       *m_bit_array[10];
    unsigned char       **m_compcomp_bits;
    
};

CompressionPoolThread::CompressionPoolThread(JobTicket *ticket, wxEvtHandler *message_target)
{
    m_pMessageTarget = message_target;
    m_pticket = ticket;
    
    Create();
}




void * CompressionPoolThread::Entry()
{

#ifdef __WXMSW__
    _set_se_translator(my_translate);

    //  On Windows, if anything in this thread produces a SEH exception (like access violation)
    //  we handle the exception locally, and simply alow the thread to exit smoothly with no results.
    //  Upstream will notice that nothing got done, and maybe try again later.
    
    try
#endif    
    {

        
    for(int i=0 ; i < 10 ; i++)
        m_bit_array[i] = 0;
    
    wxRect ncrect(m_pticket->rect);

    //  Grab a copy of the level0 chart bits
    ChartBase *pchart;
    int index;

    m_comp_bits = (unsigned char **)calloc(g_mipmap_max_level+1, sizeof(unsigned char *));
    m_pticket->comp_bits_array = m_comp_bits;

    m_compcomp_bits = (unsigned char **)calloc(g_mipmap_max_level+1, sizeof(unsigned char *));
    m_pticket->compcomp_bits_array = m_compcomp_bits;
    
    if(ChartData){
        index =  ChartData->FinddbIndex( m_pticket->m_ChartPath );
        
        pchart = ChartData->OpenChartFromDBAndLock(index, FULL_INIT );
        
        if(pchart && ChartData->IsChartLocked( index )){
                ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );
                ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( pchart );
                
                if( pBSBChart ) {
                    unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
                    m_bit_array[0] = t_buf;

                    pBSBChart->GetChartBits( ncrect, t_buf, 1 );
                }
                else if( pPlugInWrapper ){
                    unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
                    m_bit_array[0] = t_buf;
                    
                    pPlugInWrapper->GetChartBits( ncrect, t_buf, 1 );
                }
                ChartData->UnLockCacheChart(index);
       }
       else
           m_bit_array[0] = NULL;
    }
    
    //OK, got the bits?
    if( m_bit_array[0] ){        
    
        //  Fill in the rest of the private uncompressed array

        int dim = g_GLOptions.m_iTextureDimension;
        dim /= 2;
        for( int i = 1 ; i < g_mipmap_max_level+1 ; i++ ){
            m_bit_array[i] = (unsigned char *) malloc( dim * dim * 3 );
            HalfScaleChartBits( 2*dim, 2*dim, m_bit_array[i - 1], m_bit_array[i] );
            dim /= 2;
        }
        
        //  Do the compression
        
        dim = g_GLOptions.m_iTextureDimension;
        int ssize = g_tile_size;
        for( int i = 0 ; i < g_mipmap_max_level+1 ; i++ ){
            GLuint raster_format = m_pticket->m_raster_format;
        
            unsigned char *tex_data = (unsigned char*)malloc(ssize);
            if(raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
            // color range fit is worse quality but twice as fast
                int flags = squish::kDxt1 | squish::kColourRangeFit;
            
                if( g_GLOptions.m_bTextureCompressionCaching) {
                /* use slower cluster fit since we are building the cache for
                * better quality, this takes roughly 25% longer and uses about
                * 10% more disk space (result doesn't compress as well with lz4) */
                    flags = squish::kDxt1 | squish::kColourClusterFit;
                }
            
                bool b_flip = false;
//    #ifdef __WXMSW__ //ocpnUSE_ocpnBitmap
//                b_flip = true;
//    #endif            
                squish::CompressImageRGB_Flatten_Flip_Throttle( m_bit_array[i], dim, dim, tex_data, flags,
                                                            true, b_flip, m_pticket->b_throttle );
            
            }
            else if(raster_format == GL_ETC1_RGB8_OES) 
                CompressDataETC(m_bit_array[i], dim, ssize, tex_data);
            
            m_comp_bits[i] = tex_data;
            
            dim /= 2;
            ssize /= 4;
            if(ssize < 8)
                ssize = 8;

            if(m_pticket->b_abort){
                for( int i = 0; i < g_mipmap_max_level+1; i++ ){
                    free( m_bit_array[i] );
                    m_bit_array[i] = 0;
                }
                m_pticket->b_isaborted = true;
                goto SendEvtAndReturn;
            }
            
        }

        
        //  All done with the uncompressed data in the thread
        for( int i = 0; i < g_mipmap_max_level+1; i++ ){
            free( m_bit_array[i] );
            m_bit_array[i] = 0;
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
                char *src = (char *)(char *)m_comp_bits[i];
                int compressed_size = LZ4_compressHC2( src, (char *)compressed_data, csize, 4);
                m_compcomp_bits[i] = compressed_data;
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
    
#ifdef __WXMSW__    
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

    bthread_debug = false;;

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

        for(int i=0 ; i < g_mipmap_max_level+1 ; i++){
            free(ticket->comp_bits_array[i]);
        }
        free( ticket->comp_bits_array );
        
        if(ticket->bpost_zip_compress){
            for(int i=0 ; i < g_mipmap_max_level+1 ; i++){
                void *p = ticket->compcomp_bits_array[i];
                free( ticket->compcomp_bits_array[i] );
            }
            
            free( ticket->compcomp_bits_array );
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
        for(int i=0 ; i < g_mipmap_max_level+1 ; i++){
            ptd->CompressedArrayAccess( CA_WRITE, ticket->comp_bits_array[i], i);
        }
        
        free( ticket->comp_bits_array );
        
        if(ticket->bpost_zip_compress){
            for(int i=0 ; i < g_mipmap_max_level+1 ; i++){
                ptd->CompCompArrayAccess( CA_WRITE, ticket->compcomp_bits_array[i], i);
                ptd->compcomp_size[i] = ticket->compcomp_size_array[i];
            }
            
            free( ticket->compcomp_bits_array );
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
    
    delete ticket;
}

bool CompressionWorkerPool::ScheduleJob(glTexFactory* client, const wxRect &rect, int level,
                                        bool b_throttle_thread, bool b_immediate, bool b_postZip)
{
    if(!b_immediate && (todo_list.GetCount() >= 50) ){
//        int mem_used;
//        GetMemoryStatus(0, &mem_used);
        
///        qDebug() << "Could not add, count now" << (unsigned long)todo_list.GetCount() << mem_used << g_tex_mem_used;
        
        return false;;
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
    pt->m_raster_format = client->GetRasterFormat();
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
    
    t->SetPriority( WXTHREAD_MIN_PRIORITY );
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
    mip_level = level;
    x = x0;
    y = y0;
    tcolorscheme = colorscheme;
}

int CatalogEntry::GetSerialSize()
{
    return 6 * sizeof(uint32_t);
}

void CatalogEntry::Serialize( unsigned char *t)
{
    uint32_t *p = (uint32_t *)t;
    
    *p++ = mip_level;
    *p++ = x;
    *p++ = y;
    *p++ = tcolorscheme;
    *p++ = texture_offset;
    *p++ = compressed_size;
    
}


void CatalogEntry::DeSerialize( unsigned char *t)
{
    uint32_t *p = (uint32_t *)t;
    
     mip_level = *p++;
     x = *p++;
     y = *p++;
     tcolorscheme = (ColorScheme)*p++;
     texture_offset = *p++;
     compressed_size = *p++;
     
}
    






//      glTexFactory Implementation

BEGIN_EVENT_TABLE(glTexFactory, wxEvtHandler)
    EVT_TIMER(FACTORY_TIMER, glTexFactory::OnTimer)
END_EVENT_TABLE()

glTexFactory::glTexFactory(ChartBase *chart, GLuint raster_format)
{
//    m_pchart = chart;
    m_raster_format = raster_format;
    n_catalog_entries = 0;
    m_catalog_offset = sizeof(CompressedCacheHeader);
    wxDateTime ed = chart->GetEditionDate();
    m_chart_date_binary = (uint32_t)ed.GetTicks();
    
    
    m_CompressedCacheFilePath = CompressedCachePath(chart->GetFullPath());
    m_hdrOK = false;
    m_catalogOK = false;
    m_fs = 0;
    
    //  Initialize the TextureDescriptor array
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );
    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( chart );
    
    if( !pPlugInWrapper && !pBSBChart )
        return;
    
    bool b_plugin = false;
    if( pPlugInWrapper )
        b_plugin = true;
    
    if( b_plugin ) {
        m_size_X = pPlugInWrapper->GetSize_X();
        m_size_Y = pPlugInWrapper->GetSize_Y();
    } else {
        m_size_X = pBSBChart->GetSize_X();
        m_size_Y = pBSBChart->GetSize_Y();
    }
    
    m_ChartPath = chart->GetFullPath();
    
    //  Calculate the number of textures needed
    m_tex_dim = g_GLOptions.m_iTextureDimension;
    m_nx_tex = ( m_size_X / m_tex_dim ) + 1;
    m_ny_tex = ( m_size_Y / m_tex_dim ) + 1;
    
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
}

glTexFactory::~glTexFactory()
{
    delete m_fs;

    WX_CLEAR_ARRAY (m_catalog); 	 

    if (wxThread::IsMain()) {
        PurgeBackgroundCompressionPool();
    }
    DeleteAllTextures();
    DeleteAllDescriptors();
 
    free( m_td_array );         // array is empty
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
//                printf("DST::Delete Texture %d   resulting g_tex_mem_used, mb:  %ld\n", ptd->tex_name, g_tex_mem_used/(1024 * 1024));
            
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
        if(level == ptd->level_min) {
            g_tex_mem_used -= size;
            ptd->level_min++;
            ptd->miplevel_upload[level] = 0;
        }
        size /= 4;
        
        if(g_GLOptions.m_bTextureCompression && size < 8)
            size = 8;
    }
    
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


CatalogEntry *glTexFactory::GetCacheEntry(int level, int x, int y, ColorScheme color_scheme)
{

    LoadCatalog();
    
    //  Search the catalog for this particular texture
    for(int i=0 ; i < n_catalog_entries ; i++){
        CatalogEntry *p = m_catalog.Item(i);
        if( p && p->mip_level == level &&
                p->x == x && p->y == y &&
                p->tcolorscheme == color_scheme ) 
        {
            return p;
        }
    }
    return 0;
}

bool glTexFactory::IsLevelInCache( int level, const wxRect &rect, ColorScheme color_scheme )
{
    bool b_ret = false;
    
    if(g_GLOptions.m_bTextureCompression &&
        g_GLOptions.m_bTextureCompressionCaching) {
         
    //  Search for the requested texture
        if (GetCacheEntry(level, rect.x, rect.y, color_scheme) != 0)
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
                    for(int level = 0; level < g_mipmap_max_level + 1; level++ )
                        UpdateCacheLevel( wxRect(ptd->x, ptd->y, g_GLOptions.m_iTextureDimension, g_GLOptions.m_iTextureDimension),
                                          level, m_colorscheme );
                    
                    //      We can free all the ptd memory completely
                        //      and the texture will be reloaded from disk cache    
                        ptd->FreeAll();
                        ptd->nCache_Color = m_colorscheme;               // mark this TD as cached.
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
//    printf("%6d %6ld Map: %6d  Comp:%6d  CompComp: %10d  %s\n", mem_used/1024, g_tex_mem_used/m1, map_size/m1, comp_size/m1, compcomp_size, m_ChartPath.mb_str().data());
  
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
                    g_Platform->ShowBusySpinner();
                
                    for(int level = 0; level < g_mipmap_max_level + 1; level++ )
                        UpdateCacheLevel( rect, level, color_scheme );
                
                    //      We can free all the ptd memory completely
                    //      and the texture will be reloaded from disk cache    
                    ptd->FreeAll();
                
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
        if(ptd->miplevel_upload[0] && ptd->map_array[0]){
            ptd->FreeAll();
        }
            
        return true;
    }
    


    g_Platform->ShowBusySpinner();
    
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    int uncompressed_size = g_uncompressed_tile_size;
    
    
    /* optimization: when supported generate uncompressed mipmaps
       with hardware acceleration */
    //  I have never seen a case where GPU mipmap generation is faster than CPU generation.
    //  Also, HW generation uses a lot more texture memory in average cases, because we always need to upload 
    //  the level 0 bitmap.
    
    bool hw_mipmap = false;
    if(/*!g_GLOptions.m_bTextureCompression &&*/ s_glGenerateMipmap) {
//        base_level = 0;
//        hw_mipmap = true;
    }

#ifdef ocpnUSE_GLES /* glGenerateMipmaps is incredibly slow with mali drivers */
    hw_mipmap = false;
#endif
    
    
    bool b_need_compress = false;

    for(int level = 0; level < g_mipmap_max_level+1; level++ ) {
        //    Upload to GPU?
        if( level >= base_level ) {
            int status = GetTextureLevel( ptd, rect, level, color_scheme );
 
            if(g_GLOptions.m_bTextureCompression) {
                if( (COMPRESSED_BUFFER_OK == status) && (ptd->nGPU_compressed != GPU_TEXTURE_UNCOMPRESSED ) ){
                    ptd->nGPU_compressed = GPU_TEXTURE_COMPRESSED;
                    if(!ptd->miplevel_upload[level]){
                        //if(bthread_debug)
//                             printf("Upload Compressed Texture %d  level: %d \n", ptd->tex_name, level);
                        s_glCompressedTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                                          dim, dim, 0, size,
                                          ptd->CompressedArrayAccess( CA_READ, NULL, level));
                        ptd->miplevel_upload[level]++;
                        g_tex_mem_used += size;
                        
                        // We can safely discard this level's compressed data, since the GPU has it
                        ptd->FreeCompLevel( level );
                        
                    }
                    
                }                      
                else {
                    if(!ptd->miplevel_upload[level]){
                        
 //                       if(bthread_debug)
//                            printf("Upload Un-Compressed Texture %d  level: %d g_tex_mem_used: %ld\n", ptd->tex_name, level, g_tex_mem_used/(1024*1024));
                        ptd->nGPU_compressed = GPU_TEXTURE_UNCOMPRESSED;
                        glTexImage2D( GL_TEXTURE_2D, level, GL_RGB,
                                  dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                        ptd->miplevel_upload[level]++;
                        
                        g_tex_mem_used += uncompressed_size;
                    
                    //  This level has not been compressed yet, and is not in the cache
                    //  So, need to start a compression job 
                        
                        if( GL_COMPRESSED_RGB_FXT1_3DFX == g_raster_format ){
                            CompressUsingGPU( ptd, g_raster_format, level, true);
                        }
                        else                            
                            b_need_compress = true;
                    }
                }
                    
            }
            else {
                if(!ptd->miplevel_upload[level]){
 //                   if(bthread_debug)
 //                       printf("Upload Un-Compressed Texture %d  level: %d g_tex_mem_used: %ld, data ptr: %p\n", ptd->tex_name, level, g_tex_mem_used/(1024*1024), ptd->map_array[level]);
                    ptd->nGPU_compressed = GPU_TEXTURE_UNCOMPRESSED;
                    glTexImage2D( GL_TEXTURE_2D, level, GL_RGB,
                                dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                    g_tex_mem_used += uncompressed_size;
                    ptd->miplevel_upload[level]++;
                }
                
            }
                
        }
   
        if( hw_mipmap &&  (ptd->nGPU_compressed == GPU_TEXTURE_UNCOMPRESSED) ) {
   
            //  Base level has been loaded to GPU
            //  Use OGL driver to generate the rest of the mipmaps, and then break the loop
            /* compute memory used for mipmaps */
            dim /= 2;
            uncompressed_size /= 4;
       
            for(int slevel = base_level + 1; slevel < g_mipmap_max_level+1; slevel++ ) {
                g_tex_mem_used += uncompressed_size;
                dim /= 2;
                uncompressed_size /= 4;
            }
            
       
 #ifndef ocpnUSE_GLES
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, g_mipmap_max_level );
 #endif
       /* some ATI drivers require this, so to be safe... */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
       
            s_glGenerateMipmap( GL_TEXTURE_2D );
       
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
            break;  // done in one operation, so break out of the FOR loop
        }

        dim /= 2;
        size /= 4;
        if(size < 8) size = 8;
        uncompressed_size /= 4;
        
    }

 
#ifndef ocpnUSE_GLES
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, g_mipmap_max_level );
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



void glTexFactory::UpdateCacheLevel( const wxRect &rect, int level, ColorScheme color_scheme )
{
    //  Search for the requested texture
        //  Search the catalog for this particular texture
        CatalogEntry *v = GetCacheEntry(level, rect.x, rect.y, color_scheme) ;
        
        //      This texture is already done
        if(v != 0)
            return;
    
    
    
//    printf("Update cache level %d\n", level);
    
    //    Is this texture tile already defined?
    int array_index = ArrayIndex(rect.x, rect.y);
    glTextureDescriptor *ptd = m_td_array[array_index];
    
    if(ptd){
        
        int dim = g_GLOptions.m_iTextureDimension;
        int size = g_tile_size;
    
        for(int i=0 ; i < level ; i++){
            dim /= 2;
            size /= 4;
            if(size < 8)
                size = 8;
        }

        
    
       if( g_GLOptions.m_bTextureCompressionCaching){
           unsigned char *pd = ptd->CompCompArrayAccess(CA_READ, NULL, level);
           if(pd){
               UpdateCachePrecomp(pd, ptd->compcomp_size[level], ptd, level, color_scheme);
           }
           else {
               unsigned char *source = ptd->CompressedArrayAccess( CA_READ, NULL, level);
               if(source)
                    UpdateCache(source, size, ptd, level, color_scheme);
           }
       }
    }

    
}

int glTexFactory::GetTextureLevel( glTextureDescriptor *ptd, const wxRect &rect, int level, ColorScheme color_scheme )
{
    
    //  Already available in the texture descriptor?
    if(g_GLOptions.m_bTextureCompression && !g_GLOptions.m_bTextureCompressionCaching) {
        if( ptd->nGPU_compressed == GPU_TEXTURE_COMPRESSED){
            if( ptd->CompressedArrayAccess( CA_READ, NULL, level))
                return COMPRESSED_BUFFER_OK;
            else{
                if(g_CompressorPool){
                    g_CompressorPool->ScheduleJob( this, rect, level, false, true, false);  // immediate, no postZip
                    return COMPRESSED_BUFFER_OK;
                }
            }
        }
        else{
            if(g_CompressorPool){
                g_CompressorPool->ScheduleJob( this, rect, level, false, true, false);  // immediate, no postZip
                return COMPRESSED_BUFFER_OK;
            }
        }
    }
    else {
        if(ptd->nGPU_compressed == GPU_TEXTURE_UNCOMPRESSED){
            if( !ptd->map_array[ level ] ){
                GetFullMap( ptd, rect, m_ChartPath, level );
            }
            return MAP_BUFFER_OK;
        }
    }

    //  If cacheing compressed textures, look in the cache
    if(g_GLOptions.m_bTextureCompression &&
        g_GLOptions.m_bTextureCompressionCaching) {
        
    //  Search for the requested texture
        //  Search the catalog for this particular texture
        CatalogEntry *p = GetCacheEntry(level, rect.x, rect.y, color_scheme);
        
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
        
        //  Requested Texture level is not in cache, and not already built
        //  So go build it
    if( !ptd->map_array[level] ){
        GetFullMap( ptd, rect, m_ChartPath, level );
    }
        
    return MAP_BUFFER_OK;
        
            
}















bool glTexFactory::LoadHeader(void)
{
    bool ret = false;
    if( !m_hdrOK) {

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
                        hdr.format != m_raster_format) {
                        
                        //  Bad header signature    
                        m_fs->Close();
                        delete m_fs;
                    
                        m_fs = new wxFFile(m_CompressedCacheFilePath, _T("wb"));
                        n_catalog_entries = 0;
                        m_catalog_offset = 0;
                        WriteCatalogAndHeader();
                        m_fs->Close();
                        delete m_fs;
                    
                        m_fs = new wxFFile(m_CompressedCacheFilePath, _T("rb+"));
                    
                        m_hdrOK = true;
                    }
                    else {      // good header
                        n_catalog_entries = hdr.m_nentries;
                        m_catalog_offset = hdr.catalog_offset;
                        m_hdrOK = true;
                        ret = true;
                    }
                }
                else{  // file exists, and is empty
                    n_catalog_entries = 0;
                    m_catalog_offset = 0;
                    WriteCatalogAndHeader();
                    m_hdrOK = true;
                    ret = true;
                }
            }  // is open
            
            else{               // some problem opening file, probably permissions on Win7
                delete m_fs;
                wxRemoveFile(m_CompressedCacheFilePath);
                
                m_fs = new wxFFile(m_CompressedCacheFilePath, _T("wb"));
                n_catalog_entries = 0;
                m_catalog_offset = 0;
                WriteCatalogAndHeader();
                m_fs->Close();
                delete m_fs;
                
                m_fs = new wxFFile(m_CompressedCacheFilePath, _T("rb+"));
                
                m_hdrOK = true;
                ret = true;
                
                
            }
            
        }   // exists
       
        else {   // File does not exist
            wxFileName fn(m_CompressedCacheFilePath);
            if(!fn.DirExists())
                fn.Mkdir();
            
            //  Create new file, with empty catalog, and correct header
            m_fs = new wxFFile(m_CompressedCacheFilePath, _T("wb"));
            n_catalog_entries = 0;
            m_catalog_offset = 0;
            WriteCatalogAndHeader();
            m_fs->Close();
            delete m_fs;

            m_fs = new wxFFile(m_CompressedCacheFilePath, _T("rb+"));
            ret = true;
            
        }
    }
    else 
        ret = true;
        
    
    return ret;
}


bool glTexFactory::LoadCatalog(void)
{
    if(m_catalogOK)
        return true;
    if( LoadHeader() ){
        m_fs->Seek(m_catalog_offset);
 
        CatalogEntry ps;
        int buf_size =  ps.GetSerialSize();
        unsigned char *buf = (unsigned char *)malloc(buf_size);
        
        m_catalog.Clear();
        for(int i=0 ; i < n_catalog_entries ; i++){
            m_fs->Read(buf, buf_size);
            
            CatalogEntry *p = new CatalogEntry;
            p->DeSerialize(buf);
            m_catalog.Add(p);
        }
        
        n_catalog_entries = m_catalog.GetCount();
        
        free(buf);
        m_catalogOK = true;
        return true;
    }
    else
        return false;
}


bool glTexFactory::WriteCatalogAndHeader()
{
    if(m_fs && m_fs->IsOpened()){

        m_fs->Seek(m_catalog_offset);
        
        CatalogEntry ps;
        int buf_size =  ps.GetSerialSize();
        unsigned char *buf = (unsigned char *)malloc(buf_size);
        
        for(int i=0 ; i < n_catalog_entries ; i++){
            CatalogEntry *p = m_catalog.Item(i);
            p->Serialize(buf);
            m_fs->Write( buf, buf_size);
        }
        
        free(buf);

        //   Write header at file end
        CompressedCacheHeader hdr;
        hdr.magic = COMPRESSED_CACHE_MAGIC;
        hdr.format = m_raster_format;
        hdr.m_nentries = n_catalog_entries;
        hdr.catalog_offset = m_catalog_offset;
        hdr.chartdate = m_chart_date_binary;
        
        m_fs->Write( &hdr, sizeof(hdr));
        m_fs->Flush();
        
        return true;
    }
    else
        return false;
}
    
bool glTexFactory::UpdateCache(unsigned char *data, int data_size, glTextureDescriptor *ptd, int level,
                               ColorScheme color_scheme)
{
    bool b_found = false;
    //  Search the catalog for this particular texture
    if (GetCacheEntry(level, ptd->x, ptd->y, color_scheme) != 0) {
        b_found = true;
    }
    
    if( ! b_found ) {                           // not found, so add it

        // Make sure the file exists
        if(m_fs == 0){
            
            wxFileName fn(m_CompressedCacheFilePath);
            
            if(!fn.DirExists())
                fn.Mkdir();
            
            if(!fn.FileExists()){
                wxFFile new_file(m_CompressedCacheFilePath, _T("wb"));
                new_file.Close();
            }
            
            m_fs = new wxFFile(m_CompressedCacheFilePath, _T("rb+"));
            
            WriteCatalogAndHeader();
        }
        
        if(m_fs->IsOpened() ){
        //      Create a new catalog entry
            CatalogEntry *p = new CatalogEntry( level, ptd->x, ptd->y, color_scheme);
            
            m_catalog.Add(p);
            n_catalog_entries++;
        
        //      Write the compressed data to disk
             p->texture_offset = m_catalog_offset;
            
            int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
            char *compressed_data = new char[max_compressed_size];
            
            int compressed_size = LZ4_compressHC2((char*)data, compressed_data, data_size, 4);
            p->compressed_size = compressed_size;
            
            //      We write the new data at the current catalog offset, overwriting the old catalog
            m_fs->Seek( m_catalog_offset );
            m_fs->Write( compressed_data, compressed_size );
            
            delete [] compressed_data;
            
            
        //      Write the catalog and Header (which follows the catalog at the end of the file
            m_catalog_offset += compressed_size;
            WriteCatalogAndHeader();
        
        }
    }
    
    return true;
}

bool glTexFactory::UpdateCachePrecomp(unsigned char *data, int data_size, glTextureDescriptor *ptd, int level,
                               ColorScheme color_scheme)
{
    bool b_found = false;
    //  Search the catalog for this particular texture
    if (GetCacheEntry(level, ptd->x, ptd->y, color_scheme) != 0)  {
        b_found = true;
    }
    
    if( ! b_found ) {                           // not found, so add it

        // Make sure the file exists
        if(m_fs == 0){
            
            wxFileName fn(m_CompressedCacheFilePath);
            
            if(!fn.DirExists())
                fn.Mkdir();
            
            if(!fn.FileExists()){
                wxFFile new_file(m_CompressedCacheFilePath, _T("wb"));
                new_file.Close();
            }
            
            m_fs = new wxFFile(m_CompressedCacheFilePath, _T("rwb"));
            
            WriteCatalogAndHeader();
        }
        
        if(m_fs->IsOpened() ){
            //      Create a new catalog entry
            CatalogEntry *p = new CatalogEntry( level, ptd->x, ptd->y, color_scheme);
            
            m_catalog.Add(p);
            n_catalog_entries++;
            
            //      Write the compressed data to disk
            p->texture_offset = m_catalog_offset;
            
            p->compressed_size = data_size;
            
            //      We write the new data at the current catalog offset, overwriting the old catalog
            m_fs->Seek( m_catalog_offset );
            m_fs->Write( data, data_size );
            
            
            //      Write the catalog and Header (which follows the catalog at the end of the file
            m_catalog_offset += data_size;
            WriteCatalogAndHeader();
            
        }
    }
    
    return true;
}


bool CompressUsingGPU( glTextureDescriptor *ptd, GLuint raster_format, int level, bool b_post_comp)
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
    glGenTextures(1, &comp_tex);
    glBindTexture(GL_TEXTURE_2D, comp_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_FXT1_3DFX,
                 dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, ptd->map_array[level]);
    
    GLint compressed;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
    
    /* if the compression has been successful */
    if (compressed == GL_TRUE){
        
        // If our compressed size is reasonable, save it.
        GLint compressedSize;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
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
    
    // Restore the old texture pointer
    glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
    
    return ret;
}
        


