/******************************************************************************
 *
 * Project:  OpenCPN
 * Authors:  David Register
 *           Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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

#include "viewport.h"
#include "glTexCache.h"
#include "glTextureDescriptor.h"

#include "chcanv.h"
#include "glChartCanvas.h"
#include "Quilt.h"
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

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(JobList);


extern int g_mipmap_max_level;
extern GLuint g_raster_format;
extern int          g_nCacheLimit;
extern int          g_memCacheLimit;
extern ChartDB      *ChartData;
extern ocpnGLOptions    g_GLOptions;
extern long g_tex_mem_used;
extern int              g_tile_size;
extern int              g_uncompressed_tile_size;

extern ChartCanvas *cc1;
extern ChartBase *Current_Ch;

extern PFNGLGETCOMPRESSEDTEXIMAGEPROC s_glGetCompressedTexImage;
extern bool GetMemoryStatus( int *mem_total, int *mem_used );

bool bthread_debug;
bool g_throttle_squish;

class CompressionPoolThread;
class JobTicket
{
public:
    JobTicket();
    ~JobTicket() { free(level0_bits); }
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
                     unsigned char *tex_data, volatile bool &b_abort)
{
    wxASSERT(dim*dim == 2*size); // must be 4bpp
    uint64_t *tex_data64 = (uint64_t*)tex_data;
    
    int mbrow = wxMin(4, dim), mbcol = wxMin(4, dim);
    uint8_t block[48] = {};
    for(int row=0; row<dim; row+=4) {
        for(int col=0; col<dim; col+=4) {
            for(int brow=0; brow<mbrow; brow++)
                for(int bcol=0; bcol<mbcol; bcol++)
                    memcpy(block + (bcol*4+brow)*3,
                           data + ((row+brow)*dim + col+bcol)*3, 3);
                    
            extern uint64_t ProcessRGB( const uint8_t* src );
            *tex_data64++ = ProcessRGB( block );
        }
        if(b_abort)
            break;
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

    // find next lower level with map_array
    int first_level;
    for(first_level = level; first_level; first_level--)
        if( ptd->map_array[first_level - 1] )
            break;

    //Get level 0 bits from chart?
    if( !first_level ) {
        GetLevel0Map( ptd, rect, chart_path );
        first_level = 1;
    }

    int dim = g_GLOptions.m_iTextureDimension;
    for(int i=0; i<=level; i++) {
        if(i >= first_level) {
            ptd->map_array[i] = (unsigned char *) malloc( dim * dim * 3 );
            MipMap_24( 2*dim, 2*dim, ptd->map_array[i - 1], ptd->map_array[i] );
        }
        dim /= 2;
    }
}

int TextureTileSize(int level)
{
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    
    for(int i=0 ; i < level ; i++){
        dim /= 2;
        size /= 4;
        if(size < 8)
            size = 8;
    }

    return size;
}

bool DoCompress(JobTicket *pticket, glTextureDescriptor *ptd, int level)
{
    int dim = g_GLOptions.m_iTextureDimension;
    int size = TextureTileSize(level);
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

        bool a = false;
        squish::CompressImageRGBpow2_Flatten_Throttle_Abort( ptd->map_array[level], dim, dim, tex_data, flags,
                                                             true, pticket->b_throttle, a );
 
    }
    else if(g_raster_format == GL_ETC1_RGB8_OES) {
        bool a = false;
        CompressDataETC(ptd->map_array[level], dim, size, tex_data, a);
    }
        
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

    bit_array[0] = m_pticket->level0_bits;
    m_pticket->level0_bits = NULL;
    if(!bit_array[0]) {
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
    }
    
    //OK, got the bits?
    int ssize, dim;
    if(!bit_array[0] ){
        m_pticket->b_isaborted = true;
        goto SendEvtAndReturn;
    }
    
    //  Fill in the rest of the private uncompressed array

    dim = g_GLOptions.m_iTextureDimension;
    dim /= 2;
    for( int i = 1 ; i < g_mipmap_max_level+1 ; i++ ){
        bit_array[i] = (unsigned char *) malloc( dim * dim * 3 );
        MipMap_24( 2*dim, 2*dim, bit_array[i - 1], bit_array[i] );
        dim /= 2;
    }
        
    //  Do the compression
        
    dim = g_GLOptions.m_iTextureDimension;
    ssize = g_tile_size;
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
            
            squish::CompressImageRGBpow2_Flatten_Throttle_Abort( bit_array[i], dim, dim, tex_data, flags,
                                                                 true, m_pticket->b_throttle, m_pticket->b_abort );
            
        }
        else if(g_raster_format == GL_ETC1_RGB8_OES) 
            CompressDataETC(bit_array[i], dim, ssize, tex_data, m_pticket->b_abort);
            
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
            // shrink buffer to actual size.
            // This will greatly reduce ram usage, ratio usually 10:1
            // there might be a more efficient way than realloc...
//                if(!g_GLOptions.m_bTextureCompressionCaching)
            compressed_data = (unsigned char*)realloc(compressed_data, compressed_size);
            m_pticket->compcomp_bits_array[i] = compressed_data;
            m_pticket->compcomp_size_array[i] = compressed_size;
                
            csize /= 4;
            if(csize < 8)
                csize = 8;
        }
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

bool CompressUsingGPU( glTextureDescriptor *ptd, int level, bool b_post_comp, bool inplace)
{
    if(!ptd)
        return false;
    
    if( !s_glGetCompressedTexImage )
        return false;

    bool ret = false;
    GLuint comp_tex;
    if(!inplace) {
        glGenTextures(1, &comp_tex);
        glBindTexture(GL_TEXTURE_2D, comp_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    int dim = g_GLOptions.m_iTextureDimension;
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
                                                           (char *)compressed_data, TextureTileSize(level), 4);
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


//      glTextureManager Implementation
glTextureManager::glTextureManager()
{
    m_njobs_running = 0;
    int nCPU =  wxMax(1, wxThread::GetCPUCount());
    m_max_jobs =  nCPU;
    m_prevMemUsed = 0;    

//    bthread_debug = true;

    if(bthread_debug)
        printf(" nCPU: %d    m_max_jobs :%d\n", nCPU, m_max_jobs);
    
    //  Create/connect a dynamic event handler slot for messages from the worker threads
    Connect( wxEVT_OCPN_COMPRESSIONTHREAD,
             (wxObjectEventFunction) (wxEventFunction) &glTextureManager::OnEvtThread );
    
    m_ticks = 0;
    
//    m_timer.SetOwner(this, FACTORY_TIMER);
    m_timer.Connect(wxEVT_TIMER, wxTimerEventHandler( glTextureManager::OnTimer ), NULL, this);
    m_timer.Start(500);
}

glTextureManager::~glTextureManager()
{
//    ClearAllRasterTextures();
    ClearJobList();
}

void glTextureManager::OnEvtThread( OCPN_CompressionThreadEvent & event )
{
    JobTicket *ticket = event.GetTicket();
    m_njobs_running--;

    if(ticket->b_isaborted){

        for(int i=0 ; i < g_mipmap_max_level+1 ; i++) {
            free(ticket->comp_bits_array[i]);
            free( ticket->compcomp_bits_array[i] );
        }
        
        if(bthread_debug)
            printf( "    Abort job: %08X  Jobs running: %d             Job count: %lu   \n",
                    ticket->ident, m_njobs_running, (unsigned long)todo_list.GetCount());
    } else {
    
        //   Normal completion from here
        glTextureDescriptor *ptd = ticket->pFact->GetpTD( ticket->rect );

        if(ptd){
            for(int i=0 ; i < g_mipmap_max_level+1 ; i++) {
                if(ptd->CompressedArrayAccess( CA_READ, NULL, i))
                    printf("already have comp bits\n");
                ptd->CompressedArrayAccess( CA_WRITE, ticket->comp_bits_array[i], i);
            }

            // We need to force a refresh to replace the uncompressed texture
            // This frees video memory and is also really required if we had
            // gone up a mipmap level
            extern ChartCanvas *cc1;
            if(cc1) {
                glChartCanvas::Invalidate(); // ensure we refresh
                cc1->Refresh();
            }
            ptd->compdata_ticks = 10;

            if(ticket->bpost_zip_compress){
                for(int i=0 ; i < g_mipmap_max_level+1 ; i++){
                    ptd->CompCompArrayAccess( CA_WRITE, ticket->compcomp_bits_array[i], i);
                    ptd->compcomp_size[i] = ticket->compcomp_size_array[i];
                }
            }
        }
    
    
        if(bthread_debug)
            printf( "    Finished job: %08X  Jobs running: %d             Job count: %lu   \n",
                    ticket->ident, m_njobs_running, (unsigned long)todo_list.GetCount());
    }

    running_list.DeleteObject(ticket);

    delete ticket;
    StartTopJob();
}

void glTextureManager::OnTimer(wxTimerEvent &event)
{
    m_ticks++;
    
    //  Scrub all the TD's, looking for any completed compression jobs
    //  that have finished
    //  In the interest of not disturbing the GUI, process only one TD per tick

    if(g_GLOptions.m_bTextureCompression) {
        for(ChartPathHashTexfactType::iterator itt = m_chart_texfactory_hash.begin();
            itt != m_chart_texfactory_hash.end(); ++itt ) {
            glTexFactory *ptf = itt->second;
            if(ptf && ptf->OnTimer())
                break;
        }
    }

    // Once every minute, do more extensive garbage collection
#if 0    
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
                for(ChartPathHashTexfactType::iterator itt = m_chart_texfactory_hash.begin();
                    itt != m_chart_texfactory_hash.end(); ++itt ) {
                    glTexFactory *ptf = itt->second;
                    ptf->FreeIfCached();
                }
            }
        }
    }
#endif
#if 0
    if((m_ticks % 4/*120*/) == 0){
    
    // inventory
    int mem_total, mem_used;
    GetMemoryStatus(&mem_total, &mem_used);

    int map_size = 0;
    int comp_size = 0;
    int compcomp_size = 0;
    
    for(ChartPathHashTexfactType::iterator itt = m_chart_texfactory_hash.begin();
        itt != m_chart_texfactory_hash.end(); ++itt ) {
        glTexFactory *ptf = itt->second;

        ptf->AccumulateMemStatistics(map_size, comp_size, compcomp_size);
    }

    int m1 = 1024 * 1024;
//    wxString path = wxFileName(m_ChartPath).GetName();
    printf("%6d %6ld Map: %10d  Comp:%10d  CompComp: %10d \n", mem_used/1024, g_tex_mem_used/m1, map_size, comp_size, compcomp_size);//, path.mb_str().data());
  
///    qDebug() << "inv" << map_size/m1 << comp_size/m1 << compcomp_size/m1 << g_tex_mem_used/m1 << mem_used/1024;
    }
#endif

}


bool glTextureManager::ScheduleJob(glTexFactory* client, const wxRect &rect, int level,
                                        bool b_throttle_thread, bool b_immediate, bool b_postZip)
{
    if(!b_immediate && (todo_list.GetCount() >= 50) ){
//        int mem_used;
//        GetMemoryStatus(0, &mem_used);
        
///        qDebug() << "Could not add, count now" << (unsigned long)todo_list.GetCount() << mem_used << g_tex_mem_used;
        
//        return false;

        // remove last job which is least important
        wxJobListNode *node = todo_list.GetLast();
        JobTicket *ticket = node->GetData();
        todo_list.DeleteNode(node);
        delete ticket;
    }
    
    wxString chart_path = client->GetChartPath();

    //  Avoid adding duplicate jobs, i.e. the same chart_path, and the same rectangle
    wxJobListNode *node = todo_list.GetFirst();
    while(node){
        JobTicket *ticket = node->GetData();
        if( (ticket->m_ChartPath == chart_path) && (ticket->rect == rect)) {
            // bump to front
            todo_list.DeleteNode(node);
            todo_list.Insert(ticket);
            return false;
        }
        
        node = node->GetNext();
    }

    // avoid duplicate worker jobs
    wxJobListNode *tnode = running_list.GetFirst();
    while(tnode){
        JobTicket *ticket = tnode->GetData();
        if(ticket->rect == rect &&
           ticket->m_ChartPath == chart_path) {
            return false;
        }
        tnode = tnode->GetNext();
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

    pt->level0_bits = NULL;
    pt->b_abort = false;
    pt->b_isaborted = false;
    pt->bpost_zip_compress = b_postZip;

    if(!b_immediate){
        todo_list.Insert(pt); // push to front as a stack
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

bool glTextureManager::StartTopJob()
{
        //  Is it possible to start another job?
    if(m_njobs_running >= m_max_jobs)
        return false;

    wxJobListNode *node = todo_list.GetFirst();
    if(!node)
        return false;

    JobTicket *ticket = node->GetData();
    todo_list.DeleteNode(node);

    glTextureDescriptor *ptd = ticket->pFact->GetpTD( ticket->rect );
    // don't need the job if we already have the compressed data
    if(ptd->CompressedArrayAccess( CA_READ, NULL, 0)) {
        printf("already have copmressed data\n");
        delete ticket;
        return StartTopJob();
    }

    // give level 0 buffer to the ticket
    ticket->level0_bits = ptd->map_array[0];
    ptd->map_array[0] = NULL;
    
    m_njobs_running ++;
    running_list.Append(ticket);
    DoThreadJob(ticket);

    return true;
}


bool glTextureManager::DoThreadJob(JobTicket* pticket)
{
    if(bthread_debug)
        printf( "  Starting job: %08X  Jobs running: %d Jobs left: %lu\n", pticket->ident, m_njobs_running, (unsigned long)todo_list.GetCount());
    
///    qDebug() << "Starting job" << m_njobs_running <<  (unsigned long)todo_list.GetCount() << g_tex_mem_used;
    CompressionPoolThread *t = new CompressionPoolThread( pticket, this);
    pticket->pthread = t;
    
    t->Run();
    
    return true;
    
}



bool glTextureManager::DoJob(JobTicket* pticket)
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

bool glTextureManager::AsJob( wxString const &chart_path ) const
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

void glTextureManager::PurgeJobList( wxString chart_path )
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
            ticket->b_abort = true;
            node = node->GetNext();
        }
    }        
}

void glTextureManager::ClearJobList()
{
    wxJobListNode *node = todo_list.GetFirst();
    while(node){
        JobTicket *ticket = node->GetData();
        delete ticket;
        node = node->GetNext();
    }
    todo_list.Clear();
}


void glTextureManager::ClearAllRasterTextures( void )
{
    
    //     Delete all the TexFactory instances
    ChartPathHashTexfactType::iterator itt;
    for( itt = m_chart_texfactory_hash.begin(); itt != m_chart_texfactory_hash.end(); ++itt ) {
        glTexFactory *ptf = itt->second;
        
        delete ptf;
    }
    m_chart_texfactory_hash.clear();
    
    if(g_tex_mem_used != 0)
        wxLogMessage(_T("Texture memory use calculation error\n"));
}

bool glTextureManager::PurgeChartTextures( ChartBase *pc, bool b_purge_factory )
{
    //    Look for the texture factory for this chart
    ChartPathHashTexfactType::iterator ittf = m_chart_texfactory_hash.find( pc->GetFullPath() );
    
    //    Found ?
    if( ittf != m_chart_texfactory_hash.end() ) {
        glTexFactory *pTexFact = ittf->second;
        
        if(pTexFact){

            if( b_purge_factory){
                m_chart_texfactory_hash.erase(ittf);                // This chart  becoming invalid
            
                delete pTexFact;
            }      
            
            return true;
        }
        else {
            m_chart_texfactory_hash.erase(ittf);
            return false;
        }
    }
    else
        return false;
}

bool glTextureManager::TextureCrunch(double factor)
{
    
    double hysteresis = 0.90;

    bool bGLMemCrunch = g_tex_mem_used > (double)(g_GLOptions.m_iTextureMemorySize * 1024 * 1024) * factor;
    if( ! bGLMemCrunch )
        return false;
    
    
    ChartPathHashTexfactType::iterator it0;
    for( it0 = m_chart_texfactory_hash.begin(); it0 != m_chart_texfactory_hash.end(); ++it0 ) {
        wxString chart_full_path = it0->first;
        glTexFactory *ptf = it0->second;
        if(!ptf)
            continue;
        
        bGLMemCrunch = g_tex_mem_used > (double)(g_GLOptions.m_iTextureMemorySize * 1024 * 1024) * factor *hysteresis;
        if(!bGLMemCrunch)
            break;

        if( cc1->GetVP().b_quilt )          // quilted
        {
                if( cc1->m_pQuilt && cc1->m_pQuilt->IsComposed() &&
                    !cc1->m_pQuilt->IsChartInQuilt( chart_full_path ) ) {
                    ptf->DeleteSomeTextures( g_GLOptions.m_iTextureMemorySize * 1024 * 1024 * factor *hysteresis);
                    }
        }
        else      // not quilted
        {
                if( !Current_Ch->GetFullPath().IsSameAs(chart_full_path))
                {
                    ptf->DeleteSomeTextures( g_GLOptions.m_iTextureMemorySize * 1024 * 1024 * factor  *hysteresis);
                }
        }
    }
    
    return true;
}

#define MAX_CACHE_FACTORY 50
bool glTextureManager::FactoryCrunch(double factor)
{
    if (m_chart_texfactory_hash.size() == 0) {
        /* nothing to free */
        return false;
    }

    int mem_used, mem_start;
    GetMemoryStatus(0, &mem_used);
    double hysteresis = 0.90;
    mem_start = mem_used;
    
    bool bGLMemCrunch = mem_used > (double)(g_memCacheLimit) * factor && mem_used > (double)(m_prevMemUsed) *factor;
    if( ! bGLMemCrunch && (m_chart_texfactory_hash.size() <= MAX_CACHE_FACTORY))
        return false;
    
    ChartPathHashTexfactType::iterator it0;
    if( bGLMemCrunch) {
        for( it0 = m_chart_texfactory_hash.begin(); it0 != m_chart_texfactory_hash.end(); ++it0 ) {
            wxString chart_full_path = it0->first;
            glTexFactory *ptf = it0->second;
            bool mem_freed = false;
            if(!ptf)
                continue;
        
            if( cc1->GetVP().b_quilt )          // quilted
            {
                if( cc1->m_pQuilt && cc1->m_pQuilt->IsComposed() &&
                    !cc1->m_pQuilt->IsChartInQuilt( chart_full_path ) ) 
                {
                    ptf->FreeSome( g_memCacheLimit * factor * hysteresis);
                    mem_freed = true;
                }
            }
            else      // not quilted
            {
                if( !Current_Ch->GetFullPath().IsSameAs(chart_full_path))
                {
//                    ptf->DeleteSomeTextures( g_GLOptions.m_iTextureMemorySize * 1024 * 1024 * factor * hysteresis);
                    ptf->FreeSome( g_memCacheLimit * factor * hysteresis);
                    mem_freed = true;
                }
            }
            if (mem_freed) {
                GetMemoryStatus(0, &mem_used);
                bGLMemCrunch = mem_used > (double)(g_memCacheLimit) * factor * hysteresis;
                m_prevMemUsed = mem_used;
                if(!bGLMemCrunch)
                    break;
            }
        }
    }

    bGLMemCrunch = (mem_used > (double)(g_memCacheLimit) * factor *hysteresis && 
                    mem_used > (double)(m_prevMemUsed) * factor *hysteresis
                    )  || (m_chart_texfactory_hash.size() > MAX_CACHE_FACTORY);
    //  Need more, so delete the oldest factory
    if(bGLMemCrunch){
        
        //      Find the oldest unused factory
        int lru_oldest = 2147483647;
        glTexFactory *ptf_oldest = NULL;
        
        for( it0 = m_chart_texfactory_hash.begin(); it0 != m_chart_texfactory_hash.end(); ++it0 ) {
            wxString chart_full_path = it0->first;
            glTexFactory *ptf = it0->second;
            if(!ptf)
                continue;
            
            // we better have to find one because glTexFactory keep cache texture open
            // and ocpn will eventually run out of file descriptors
            if( cc1->GetVP().b_quilt )          // quilted
            {
                if( cc1->m_pQuilt && cc1->m_pQuilt->IsComposed() &&
                    !cc1->m_pQuilt->IsChartInQuilt( chart_full_path ) ) {
                    
                    int lru = ptf->GetLRUTime();
                    if(lru < lru_oldest && !ptf->BackgroundCompressionAsJob()){
                        lru_oldest = lru;
                        ptf_oldest = ptf;
                    }
                }
            }
            else {
                if( !Current_Ch->GetFullPath().IsSameAs(chart_full_path)) {
                    int lru = ptf->GetLRUTime();
                    if(lru < lru_oldest && !ptf->BackgroundCompressionAsJob()){
                        lru_oldest = lru;
                        ptf_oldest = ptf;
                    }
                }
            }
        }
                    
        //      Found one?
        if(ptf_oldest){
            m_chart_texfactory_hash.erase(ptf_oldest->GetChartPath());                // This chart  becoming invalid
                
            delete ptf_oldest;
                
//            int mem_now;
//            GetMemoryStatus(0, &mem_now);
//            printf("-------------FactoryDelete\n");
       }                
    }
    
//    int mem_now;
//    GetMemoryStatus(0, &mem_now);
//    printf(">>>>FactoryCrunch  was: %d  is:%d \n", mem_start, mem_now);
    
    return true;
}
