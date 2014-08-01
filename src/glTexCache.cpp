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

#include "GL/gl.h"


#include "glTexCache.h"

#include "glChartCanvas.h"
#include "chartbase.h"
#include "chartimg.h"
#include "chartdb.h"


#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif


#include "squish.h"
#include "lz4.h"
#include "lz4hc.h"




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

extern PFNGLGETCOMPRESSEDTEXIMAGEPROC s_glGetCompressedTexImage;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
extern PFNGLGENERATEMIPMAPEXTPROC          s_glGenerateMipmap;
extern bool GetMemoryStatus( int *mem_total, int *mem_used );

bool bthread_debug;
bool g_throttle_squish;

#include <wx/arrimpl.cpp> 
WX_DEFINE_OBJARRAY(ArrayOfCatalogEntries);

class CompressionPoolThread;
class JobTicket
{
public:
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
};

WX_DECLARE_LIST(JobTicket, JobList);

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
    
    return g_PrivateDataDir + separator + _T("raster_texture_cache") + separator + path 
    + _T(".compressed_chart");
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

void GetLevel0Map( glTextureDescriptor *ptd,  const wxRect &rect, ChartBase *pChart )
{
    // Load level 0 uncompressed data
    wxRect ncrect(rect);
    
    //    Prime the pump with the "zero" level bits, ie. 1x native chart bits
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pChart );
    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( pChart );
    
    if( !pPlugInWrapper && !pBSBChart )
        return;
    
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



void GetFullMap( glTextureDescriptor *ptd,  const wxRect &rect, ChartBase *pChart, int level)
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
                    GetLevel0Map( ptd, rect, pChart );
                    
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
#ifdef __WXMSW__ //ocpnUSE_ocpnBitmap
            b_flip = true;
#endif            
            squish::CompressImageRGB_Flatten_Flip_Throttle( ptd->map_array[level], dim, dim, tex_data, flags,
                                                            true, b_flip, pticket->b_throttle );
 
        }
        else if(raster_format == GL_ETC1_RGB8_OES) 
            CompressDataETC(ptd->map_array[level], dim, size, tex_data);
        
        
        ptd->CompressedArrayAccess( CA_WRITE, tex_data, level);
        
    
    return true;
    
}















const wxEventType wxEVT_OCPN_COMPRESSIONTHREAD = wxNewEventType();

class OCPN_CompressionThreadEvent: public wxEvent
{
public:
    OCPN_CompressionThreadEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );
    ~OCPN_CompressionThreadEvent( );
    
    // accessors
    void SetTicket( JobTicket *ticket ){m_ticket = ticket;}
    JobTicket *GetTicket(void){ return m_ticket; }
    
    // required for sending with wxPostEvent()
    wxEvent *Clone() const;
    
private:
    JobTicket  * m_ticket;
};

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
    
    
    
};

CompressionPoolThread::CompressionPoolThread(JobTicket *ticket, wxEvtHandler *message_target)
{
    m_pMessageTarget = message_target;
    m_pticket = ticket;
    
    Create();
}

void * CompressionPoolThread::Entry()
{
    m_bit_array[0] = 0;
    wxRect ncrect(m_pticket->rect);

    //  Grab a copy of the level0 chart bits
    
    if(ChartData){
        int index =  ChartData->FinddbIndex( m_pticket->m_ChartPath );
        
        ChartBase *pchart = ChartData->OpenChartFromDBAndLock(index, FULL_INIT );
        
             if(pchart){
                ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );
                ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( pchart );
                
                if( pBSBChart ) {
                    unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
                    pBSBChart->GetChartBits( ncrect, t_buf, 1 );
                    
                    //    and cache them here
                    m_bit_array[0] = t_buf;
                }
                else if( pPlugInWrapper ){
                    unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
                    pPlugInWrapper->GetChartBits( ncrect, t_buf, 1 );
                    
                    //    and cache them here
                    m_bit_array[0] = t_buf;
                }
                int index = ChartData->FinddbIndex(m_pticket->m_ChartPath);
                ChartData->UnLockCacheChart(index);
            }
    }
    
    //OK, got the bits?
    if( m_bit_array[0] ){        
    
        //  Fill in the rest of the private uncompressed array

        int dim = g_GLOptions.m_iTextureDimension;
        dim /= 2;
        for( int i = 1 ; i < 5 ; i++ ){
            m_bit_array[i] = (unsigned char *) malloc( dim * dim * 3 );
            HalfScaleChartBits( 2*dim, 2*dim, m_bit_array[i - 1], m_bit_array[i] );
            dim /= 2;
        }
        
        //  Do the compression
        m_comp_bits = (unsigned char **)malloc(5 * sizeof(unsigned char *));
        
        dim = g_GLOptions.m_iTextureDimension;
        int ssize = g_tile_size;
        for( int i = 0 ; i < 5 ; i++ ){
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
    #ifdef __WXMSW__ //ocpnUSE_ocpnBitmap
                b_flip = true;
    #endif            
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
            
        }

        //  All done with the uncompressed data in the thread
        for( int i = 0; i < 5; i++ ){
            free( m_bit_array[i] );
        }
            
        m_pticket->comp_bits_array = m_comp_bits;
    }
    
    if( m_pMessageTarget ) {
        OCPN_CompressionThreadEvent Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
        Nevent.SetTicket(m_pticket);
        
        m_pMessageTarget->AddPendingEvent(Nevent);
    }
    
    
    return 0;
}





#include <wx/listimpl.cpp>
WX_DEFINE_LIST(JobList);



//      CompressionWorkerPool Definition
class CompressionWorkerPool : public wxEvtHandler
{
public:
    CompressionWorkerPool();
    ~CompressionWorkerPool();
    
    bool ScheduleJob( glTexFactory *client, const wxRect &rect, int level_min,
                      bool b_throttle_thread = true, bool b_immediate = false);
    void OnEvtThread( OCPN_CompressionThreadEvent & event );
    int GetRunningJobCount(){ return m_njobs_running; }
    void PurgeJobList();
    bool FindTextureDescriptorInJoblist(glTextureDescriptor *ptd);
    
    
    unsigned int m_raster_format;
    JobList             running_list;
    
private:
    
    bool DoJob( JobTicket *pticket );
    bool DoThreadJob(JobTicket* pticket);
    bool StartTopJob();
    
    JobList             todo_list;
    int                 m_njobs_running;
    int                 m_max_jobs;
    
    
    
};

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
        running_list.DeleteObject(ticket);
        m_njobs_running--;
        
        if(bthread_debug)
            printf( "    Abort job: %08X  Jobs running: %d             Job count: %lu   \n",
                        ticket->ident, m_njobs_running, todo_list.GetCount());
        return;
    }
    
    glTextureDescriptor *ptd = ticket->pFact->GetpTD( ticket->rect );

    if(ptd){
        for(int i=0 ; i < 5 ; i++){
            ptd->CompressedArrayAccess( CA_WRITE, ticket->comp_bits_array[i], i);
        }
        
        free( ticket->comp_bits_array );
    }
    
    running_list.DeleteObject(ticket);
    m_njobs_running--;
    
    if(bthread_debug)
        printf( "    Finished job: %08X  Jobs running: %d             Job count: %lu   \n",
            ticket->ident, m_njobs_running, todo_list.GetCount());

    StartTopJob();
    
}

bool CompressionWorkerPool::FindTextureDescriptorInJoblist(glTextureDescriptor *ptd)
{
    //  Check the todo list
    wxJobListNode *node = todo_list.GetFirst();
    while(node){
        JobTicket *ticket = node->GetData();
        if(ticket->pFact){
            glTextureDescriptor *pcandidate = ticket->pFact->GetpTD( ticket->rect );
            if(pcandidate == ptd)
                return true;
        }
        node = node->GetNext();
    }
 
    //  Check the running list
    node = running_list.GetFirst();
    while(node){
        JobTicket *ticket = node->GetData();
        if(ticket->pFact){
            glTextureDescriptor *pcandidate = ticket->pFact->GetpTD( ticket->rect );
            if(pcandidate == ptd)
                return true;
        }
        node = node->GetNext();
    }
 
    return false;
    
}

bool CompressionWorkerPool::ScheduleJob(glTexFactory* client, const wxRect &rect, int level,
                                        bool b_throttle_thread, bool b_immediate)
{
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
    pt->m_ChartPath = client->GetChartPath();
    pt->b_abort = false;

    if(!b_immediate){
        todo_list.Append(pt);
        if(bthread_debug){
            int mem_total, mem_used;
            GetMemoryStatus(&mem_total, &mem_used);
            
            if(bthread_debug)
                printf( "Adding job: %08X  Job Count: %lu  mem_used %d\n", pt->ident, todo_list.GetCount(), mem_used);
        }
        
        StartTopJob();
        return false;
    }
    else{
        DoJob(pt);
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
        printf( "  Starting job: %08X  Jobs running: %d Jobs left: %lu\n", pticket->ident, m_njobs_running, todo_list.GetCount());
    
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
            GetFullMap( ptd, pticket->rect, pticket->pFact->GetpChart(), pticket->level_min_request );
            
            DoCompress( pticket, ptd, pticket->level_min_request);
            ret = true;
            
        }
    }
    
    return ret;
}

void CompressionWorkerPool::PurgeJobList()
{
    todo_list.Clear();
    
    //  Check the running list
    wxJobListNode *node = running_list.GetFirst();
    while(node){
        JobTicket *ticket = node->GetData();
        ticket->b_abort = true;
        node = node->GetNext();
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


glTexFactory::glTexFactory(ChartBase *chart, GLuint raster_format)
{
    m_pchart = chart;
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
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( m_pchart );
    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( m_pchart );
    
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
    
    if(!g_CompressorPool)
        g_CompressorPool = new CompressionWorkerPool;
    
}

glTexFactory::~glTexFactory()
{
    if(m_fs && m_fs->IsOpened()){
        m_fs->Close();
    }
    
    DeleteAllDescriptors();
    
    free( m_td_array );         // array is empty
}

glTextureDescriptor *glTexFactory::GetpTD( wxRect & rect )
{
    int array_index = ((rect.y / m_tex_dim) * m_stride) + (rect.x / m_tex_dim);
    return m_td_array[array_index];
}


void glTexFactory::DeleteTexture(const wxRect &rect)
{
    //    Is this texture tile defined?
    int array_index = ((rect.y / m_tex_dim) * m_stride) + (rect.x / m_tex_dim);
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
            if(ptd->tex_name && bthread_debug)
                printf("DAT::Delete Texture %d   resulting g_tex_mem_used, mb:  %ld\n", ptd->tex_name, g_tex_mem_used/(1024 * 1024));
            
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
            if(ptd->tex_name && bthread_debug)
                printf("DST::Delete Texture %d   resulting g_tex_mem_used, mb:  %ld\n", ptd->tex_name, g_tex_mem_used/(1024 * 1024));
            
            DeleteSingleTexture( ptd);
        }
        
        if(g_tex_mem_used <= target)
            break;
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

void glTexFactory::PurgeBackgroundCompressionPool()
{
    //  Purge the "todo" list, and allow any running jobs to complete normally
    //  This will of course block for some time....
    if(g_CompressorPool) {
        g_CompressorPool->PurgeJobList();
 
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
    int array_index = ((rect.y / m_tex_dim) * m_stride) + (rect.x / m_tex_dim);
    glTextureDescriptor *ptd = m_td_array[array_index];

    bool b_all_cmm_built = false;
    
    if(ptd) {
        
        //  It may be that the compressed mipmaps have been generated , uploaded, and free'ed
        if( ptd->nGPU_compressed == GPU_TEXTURE_COMPRESSED){
            b_all_cmm_built = true;
        }
        else {
        // are all required compressed levels available?
            b_all_cmm_built = true;
            for(int level = base_level; level < g_mipmap_max_level+1; level++ ) {
                if(NULL == ptd->CompressedArrayAccess( CA_READ, NULL, level)){
                    b_all_cmm_built = false;
                    break;
                }
            }
        }
    }
    
    return b_all_cmm_built;
}

bool glTexFactory::IsLevelInCache( int level, const wxRect &rect, ColorScheme color_scheme )
{
    bool b_ret = false;
    
    if(g_GLOptions.m_bTextureCompression &&
        g_GLOptions.m_bTextureCompressionCaching) {
    //  Look in the cache
         
        LoadCatalog();
    
    //  Search for the requested texture
        bool b_found = false;
        CatalogEntry *p;
        //  Search the catalog for this particular texture
        for(int i=0 ; i < n_catalog_entries ; i++){
            p = m_catalog.Item(i);
            if( (p->mip_level == level )  &&
                (p->x == rect.x) &&
                (p->y == rect.y) &&
                (p->tcolorscheme == color_scheme )) {
                b_found = true;
            break;
                }
        }
        
        b_ret = b_found;
    }
    
    return b_ret;
}

void glTexFactory::DoImmediateFullCompress(const wxRect &rect)
{
    int array_index = ((rect.y / m_tex_dim) * m_stride) + (rect.x / m_tex_dim);
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
             g_CompressorPool->ScheduleJob( this, rect, level, false, true);  // immediate
        }
    }
}

void glTexFactory::PrepareTexture( int base_level, const wxRect &rect, ColorScheme color_scheme, bool b_throttle_thread )
{
    int array_index = ((rect.y / m_tex_dim) * m_stride) + (rect.x / m_tex_dim);
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

         
#ifdef ocpnUSE_GLES /* gles requires a complete set of mipmaps starting at 0 */
    base_level = 0;
#endif

    //  On Windows, we need a color flip, so it should start at the base level
#ifdef __WXMSW__    
    if(g_GLOptions.m_bTextureCompression && (m_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) )
        base_level = 0;
#endif    
        
    /* Also, some non-compliant OpenGL drivers need the complete mipmap set when using compressed textures */
    if( glChartCanvas::s_b_UploadFullCompressedMipmaps && g_GLOptions.m_bTextureCompression )
        base_level = 0;

    
    //  If compression is to be used. and we have been building compressed mipmaps in worker threads..
    //  then see if we are done building all the mipmaps that we need.
    //  If we have all the compressed mipmaps available, then delete the texture, switch to compressed mode,
    //  and recover lots of memory from the uncompressed mipmaps.    
    if(g_GLOptions.m_bTextureCompression) {
        if(GPU_TEXTURE_UNCOMPRESSED == ptd->nGPU_compressed){

            int mem_total, mem_used;
            GetMemoryStatus(&mem_total, &mem_used);
            unsigned int nCache = 0;
            unsigned int lcache_limit = (unsigned int)g_nCacheLimit * 8 / 10;
            if(ChartData)
                nCache = ChartData->GetChartCache()->GetCount();
              
            bool mainMemCrunch = ( ((g_memCacheLimit > 0) && (mem_used > g_memCacheLimit * 8 / 10)) ||
                    (g_nCacheLimit && (nCache > lcache_limit)) );
                    
            bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;
            if( bGLMemCrunch || mainMemCrunch ) {
                
                // are all required compressed levels available?
                bool b_all_cmm_built = true;
                for(int level = base_level; level < g_mipmap_max_level+1; level++ ) {
                    if(NULL == ptd->CompressedArrayAccess( CA_READ, NULL, level)){
                        b_all_cmm_built = false;
                        break;
                    }
                }
                
                // Everything we need is available
                // So, free some unneeded uncompressed mipmaps, and prepare the TD for compressed use.
                if(b_all_cmm_built){
                    if(bthread_debug)
                        printf("Convert Texture %04X   resulting g_tex_mem_used, mb:  %ld\n", ptd->tex_name, g_tex_mem_used/(1024 * 1024));
                    
                    ptd->FreeMap();
                    
                    //  Delete and rebuild the actual GPU texture
                    DeleteSingleTexture(ptd);
                    
                    //  We know that the compressed mipmaps are now available, so...
                    ptd->nGPU_compressed = GPU_TEXTURE_COMPRESSED;
                    
                }
            }
        }
    }
                    
        
    //  Now is a good time to update the cache, syncronously
    if(g_GLOptions.m_bTextureCompression && g_GLOptions.m_bTextureCompressionCaching) {
        if( ptd->nCache_Color != color_scheme ){
            if( IsCompressedArrayComplete( 0, rect) ){
                for(int level = 0; level < g_mipmap_max_level + 1; level++ )
                    UpdateCacheLevel( rect, level, color_scheme );
                
                ptd->nCache_Color = color_scheme;               // mark this TD as cached.
                    
            }
        }
    }
    
            
    //    If the GPU does not know about this texture, create it
    if( ptd->tex_name == 0 ) {
        glGenTextures( 1, &ptd->tex_name );
        
        glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        
#ifdef ocpnUSE_GLES /* this is slightly faster */
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
#else /* looks nicer */
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
#endif
    }
       
        
        
        
        
        
        
        
    //  Texture requested has already been physically uploaded to the GPU
    //  so we merely need to bind it
    if(base_level >= ptd->level_min){
        glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
        return;
    }

    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    int uncompressed_size = g_uncompressed_tile_size;
    
    
    /* optimization: when supported generate uncompressed mipmaps
       with hardware acceleration */
    bool hw_mipmap = false;

    if(s_glGenerateMipmap) {
        base_level = 0;
        hw_mipmap = true;
    }
    
#ifdef ocpnUSE_GLES /* glGenerateMipmaps is incredibly slow with mali drivers */
    hw_mipmap = false;
#endif
    

    bool b_need_compress = false;
    
    for(int level = 0; level < g_mipmap_max_level+1; level++ ) {
        //    Upload to GPU?
        if( level >= base_level ) {
            int status = GetTextureLevel( ptd, rect, level, color_scheme, b_throttle_thread );
 
            if(g_GLOptions.m_bTextureCompression) {
                if( (COMPRESSED_BUFFER_OK == status) && (ptd->nGPU_compressed != GPU_TEXTURE_UNCOMPRESSED ) ){
                    
                    if(bthread_debug)
                        printf("Upload Compressed Texture %d  level: %d \n", ptd->tex_name, level);
                    
                    ptd->nGPU_compressed = GPU_TEXTURE_COMPRESSED;
                    s_glCompressedTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                                          dim, dim, 0, size,
                                          ptd->CompressedArrayAccess( CA_READ, NULL, level));
                    g_tex_mem_used += size;
                    
                }                      
                else {
                    if(bthread_debug)
                        printf("Upload Un-Compressed Texture %d  level: %d g_tex_mem_used: %ld\n", ptd->tex_name, level, g_tex_mem_used/(1024*1024));
                    ptd->nGPU_compressed = GPU_TEXTURE_UNCOMPRESSED;
                    glTexImage2D( GL_TEXTURE_2D, level, GL_RGB,
                                  dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                    
                    g_tex_mem_used += uncompressed_size;
                    
                    //  his level has not been compressed yet, and is not in the cache
                    //  So, need to start a compression job 
                    b_need_compress = true;
                }
                    
            }
            else {
                if(bthread_debug)
                    printf("Upload Un-Compressed Texture %d  level: %d g_tex_mem_used: %ld\n", ptd->tex_name, level, g_tex_mem_used/(1024*1024));
                ptd->nGPU_compressed = GPU_TEXTURE_UNCOMPRESSED;
                glTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                              dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                g_tex_mem_used += uncompressed_size;
                
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
        if(g_CompressorPool)
            g_CompressorPool->ScheduleJob( this, rect, 0, b_throttle_thread, false);
    }
    

    //   If global memory is getting short, we can crunch here.
    //   All mipmaps >= ptd->level_min have been uploaded to the GPU,
    //   so there is no reason to save the bits forever.
    //   Of course, this means that if the texture is deleted elsewhere, then the bits will need to be
    //   regenerated.  The price to pay for memory limits....
    
    int mem_total, mem_used;
    GetMemoryStatus(&mem_total, &mem_used);
    unsigned int nCache = 0;
    unsigned int lcache_limit = (unsigned int)g_nCacheLimit * 8 / 10;
    if(ChartData)
        nCache = ChartData->GetChartCache()->GetCount();
    
    if( ((g_memCacheLimit > 0) && (mem_used > g_memCacheLimit * 9 / 10)) ||
        (g_nCacheLimit && (nCache > lcache_limit)) )
      
    {
        ptd->FreeAll();
    }
   
}



void glTexFactory::UpdateCacheLevel( const wxRect &rect, int level, ColorScheme color_scheme )
{
    //  look in the cache
        LoadCatalog();
    
    //  Search for the requested texture
        bool b_found = false;
        CatalogEntry *p;
        //  Search the catalog for this particular texture
        for(int i=0 ; i < n_catalog_entries ; i++){
            p = m_catalog.Item(i);
            if( (p->mip_level == level )  &&
                (p->x == rect.x) &&
                (p->y == rect.y) &&
                (p->tcolorscheme == color_scheme )) {
                b_found = true;
            break;
                }
        }
        
        //      This texture is already done
        if(b_found)
            return;
    
    
    
    
    //    Is this texture tile already defined?
    int array_index = ((rect.y / m_tex_dim) * m_stride) + (rect.x / m_tex_dim);
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

        
    
       if( g_GLOptions.m_bTextureCompressionCaching)
            UpdateCache(ptd->CompressedArrayAccess( CA_READ, NULL, level), size, ptd, level, color_scheme);
    }

    
}

int glTexFactory::GetTextureLevel( glTextureDescriptor *ptd, const wxRect &rect, int level, ColorScheme color_scheme, bool b_throttle_thread )
{
    
    //  Already available in the texture descriptor?
    if(g_GLOptions.m_bTextureCompression) {
        if( ptd->nGPU_compressed == GPU_TEXTURE_COMPRESSED){
            if( ptd->CompressedArrayAccess( CA_READ, NULL, level))
                return COMPRESSED_BUFFER_OK;
        }
        else {
            if( ptd->map_array[ level ] )
                return MAP_BUFFER_OK;
        }
    }
    else {
        if( ptd->map_array[ level ] )
            return MAP_BUFFER_OK;
    }

    //  If cacheing compressed textures, look in the cache
    if(g_GLOptions.m_bTextureCompression &&
        g_GLOptions.m_bTextureCompressionCaching) {
        
        LoadCatalog();
    
    //  Search for the requested texture
        bool b_found = false;
        CatalogEntry *p;
        //  Search the catalog for this particular texture
        for(int i=0 ; i < n_catalog_entries ; i++){
            p = m_catalog.Item(i);
            if( (p->mip_level == level )  &&
                (p->x == ptd->x) &&
                (p->y == ptd->y) &&
                (p->tcolorscheme == color_scheme )) {
                b_found = true;
            break;
                }
        }
        
        //      Requested texture level is found in the cache
        //      so go load it
        if( b_found ) {
            
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
            
            ptd->level_min = wxMin(ptd->level_min, level);
            return COMPRESSED_BUFFER_OK;
        }
    }
        
        //  Requested Texture level is not in cache, and not already built
        //  So go build it
    if( !ptd->map_array[level] )
        GetFullMap( ptd, rect, m_pchart, level );
        
    return MAP_BUFFER_OK;
        
        
        
        
        
        //  Do the compression, if needed
            if(g_GLOptions.m_bTextureCompression) {
                if(m_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
                    m_raster_format == GL_ETC1_RGB8_OES)
                {
                    if(level == 0){
                        bool b_immediate = g_CompressorPool->ScheduleJob( this, rect, level, b_throttle_thread, false);
                        if(!b_immediate)
                            return COMPRESSED_BUFFER_PENDING;
                        else
                            return COMPRESSED_BUFFER_OK;
                    }
                    else
                        return COMPRESSED_BUFFER_PENDING;
                    
                    
                }
                
                return COMPRESSED_BUFFER_OK;
            }
            
            // No compression at all
            return MAP_BUFFER_OK;
            
            
}















bool glTexFactory::LoadHeader(void)
{
    bool ret = false;
    if( !m_hdrOK) {
        
        if(wxFileName::FileExists(m_CompressedCacheFilePath)) {
            
            m_fs = new wxFile(m_CompressedCacheFilePath, wxFile::read_write);
            
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
                
                    m_fs = new wxFile(m_CompressedCacheFilePath, wxFile::write);
                    n_catalog_entries = 0;
                    m_catalog_offset = 0;
                    WriteCatalogAndHeader();
                    m_fs->Close();
                    delete m_fs;
                
                    m_fs = new wxFile(m_CompressedCacheFilePath, wxFile::read_write);
                
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
        }
        else {   // File does not exist
            wxFileName fn(m_CompressedCacheFilePath);
            if(!fn.DirExists())
                fn.Mkdir();
            
            //  Create new file, with empty catalog, and correct header
            m_fs = new wxFile(m_CompressedCacheFilePath, wxFile::write);
            n_catalog_entries = 0;
            m_catalog_offset = 0;
            WriteCatalogAndHeader();
            m_fs->Close();
            delete m_fs;
            
            m_fs = new wxFile(m_CompressedCacheFilePath, wxFile::read_write);
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
        
        return true;
    }
    else
        return false;
}
    
bool glTexFactory::UpdateCache(unsigned char *data, int data_size, glTextureDescriptor *ptd, int level,
                               ColorScheme color_scheme)
{
    LoadCatalog();
    
    bool b_found = false;
    //  Search the catalog for this particular texture
    for(int i=0 ; i < n_catalog_entries ; i++){
        CatalogEntry *p = m_catalog.Item(i);
        if( (p->mip_level == level )  &&
            (p->x == ptd->x) &&
            (p->y == ptd->y) &&
            (p->tcolorscheme == color_scheme )) {
            b_found = true;
            break;
        }
    }
    
    if( ! b_found ) {                           // not found, so add it

        // Make sure the file exists
        if(m_fs == 0){
            
            wxFileName fn(m_CompressedCacheFilePath);
            
            if(!fn.DirExists())
                fn.Mkdir();
            
            if(!fn.FileExists()){
                wxFile new_file(m_CompressedCacheFilePath, wxFile::write);
                new_file.Close();
            }
            
            m_fs = new wxFile(m_CompressedCacheFilePath, wxFile::read_write);
            
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

        
        


