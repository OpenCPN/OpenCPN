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

extern bool g_bopengl;
extern bool g_bDebugOGL;
extern long g_tex_mem_used;
extern int g_mipmap_max_level;
extern GLuint g_raster_format;
extern int          g_nCacheLimit;
extern int          g_memCacheLimit;

extern ChartCanvas *cc1;
extern ChartBase *Current_Ch;
extern ColorScheme global_color_scheme;

extern ChartDB      *ChartData;
extern ocpnGLOptions    g_GLOptions;
extern wxString         g_PrivateDataDir;

extern int              g_tile_size;
extern int              g_uncompressed_tile_size;

extern PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
extern PFNGLGENERATEMIPMAPEXTPROC          s_glGenerateMipmap;
extern bool GetMemoryStatus( int *mem_total, int *mem_used );

extern wxString CompressedCachePath(wxString path);
extern glTextureManager   *g_glTextureManager;

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
enum TextureDataType {COMPRESSED_BUFFER_OK, MAP_BUFFER_OK};

glTexFactory::glTexFactory(ChartBase *chart, int raster_format)
{
//    m_pchart = chart;
    n_catalog_entries = 0;
    m_catalog_offset = sizeof(CompressedCacheHeader);
    wxDateTime ed = chart->GetEditionDate();
    m_chart_date_binary = (uint32_t)ed.GetTicks();
    m_chartfile_date_binary = ::wxFileModificationTime(chart->GetFullPath());
    m_chartfile_size = (uint32_t)wxFileName::GetSize(chart->GetFullPath()).GetLo();
    m_ChartPath = chart->GetFullPath();
    
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
        
    //  Calculate the number of textures needed
    m_tex_dim = g_GLOptions.m_iTextureDimension;
    m_nx_tex = ( m_size_X / m_tex_dim ) + ((m_size_X % m_tex_dim) == 0 ? 0:1);
    m_ny_tex = ( m_size_Y / m_tex_dim ) + ((m_size_Y % m_tex_dim) == 0 ? 0:1);
    
    m_stride = m_nx_tex;
    m_ntex = m_nx_tex * m_ny_tex;
    m_td_array = (glTextureDescriptor **)calloc(m_ntex, sizeof(glTextureDescriptor *));

    m_prepared_projection_type = 0;
    m_tiles = NULL;
}

glTexFactory::~glTexFactory()
{
    delete m_fs;

    PurgeBackgroundCompressionPool();
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

bool glTexFactory::OnTimer()
{
    for(int i=0 ; i < m_ntex ; i++) {
        glTextureDescriptor *ptd = m_td_array[i] ;
        // sometimes compressed data is produced but by the time
        // it arrives it is no longer needed, so with a timeout
        // of 5 seconds free this memory to avoid ram use buildup
        if(ptd && ptd->compdata_ticks) {
            ptd->compdata_ticks--;
            ptd->FreeComp();
        }
    }

#if 1
    // if we have the data in the catalog of level 0 or doubly compressed
    // for an entire row of tiles, then we can free rows from the linebuffer
    if(g_GLOptions.m_bTextureCompression) {
        ChartBase *pChart = ChartData->OpenChartFromDB( m_ChartPath, FULL_INIT );
        ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pChart );

        if(pBSBChart) {
            for(int y = 0; y<m_ny_tex; y++) {
                int dim = g_GLOptions.m_iTextureDimension;

                if(!pBSBChart->HaveLineCacheRow(y*dim))
                    continue;

                for(int x = 0; x<m_nx_tex; x++) {
                    int i = ArrayIndex(x, y);
                    glTextureDescriptor *ptd = m_td_array[i];
                    
                    if( !ptd )
                        goto keeplines;
                    
                    if( ptd->compcomp_array[0] )
                        continue; // ok
                    
                    CatalogEntryValue *p = GetCacheEntryValue(0, x*dim, y*dim, ptd->m_colorscheme);
                    if(!p)
                        goto keeplines;
                }

                pBSBChart->FreeLineCacheRows(y*dim, (y+1)*dim);
            }
        keeplines:;
        }
    }
#endif
    
    // write doubly compressed data to disk
    if(g_GLOptions.m_bTextureCompressionCaching)
        for(int i=0 ; i < m_ntex ; i++) {
            glTextureDescriptor *ptd = m_td_array[i];
            if(ptd && ptd->IsCompCompArrayComplete( 0 )) {
                int dim = g_GLOptions.m_iTextureDimension;
                UpdateCacheAllLevels
                    (wxRect(ptd->x, ptd->y, dim, dim),
                     ptd->m_colorscheme, ptd->compcomp_array, ptd->compcomp_size );

                // no longer need to store the compressed compressed data
                ptd->FreeCompComp();
//                return true;
            }
        }

    return false;
}

#ifdef __OCPN__ANDROID__
    // delete any uncompressed textures if texture memory is more than 30
    // on android??   Maybe this should be removed now
    bool bGLMemCrunch = g_tex_mem_used > 30/*g_GLOptions.m_iTextureMemorySize*/ * 1024 * 1024;
    
    if( bGLMemCrunch ){
        for(wxTextureListNode *node = m_texture_list.GetFirst(); node;
            node = node->GetNext()) {
            glTextureDescriptor *ptd = node->GetData();
            if(ptd->nGPU_compressed == GPU_TEXTURE_UNCOMPRESSED){
                DeleteSingleTexture(ptd);
            }
        }
    }
#endif    

void glTexFactory::AccumulateMemStatistics(int &map_size, int &comp_size, int &compcomp_size)
{
    for(int i=0 ; i < m_ntex ; i++) {
        glTextureDescriptor *ptd = m_td_array[i] ;
        if(ptd) {
            map_size += ptd->GetMapArrayAlloc();
            comp_size += ptd->GetCompArrayAlloc();
            compcomp_size += ptd->GetCompCompArrayAlloc();
        }
    }
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
        
            if(g_tex_mem_used <= target)
                break;
        }
    }
}

void glTexFactory::FreeSome( long target )
{
    for(int i=0 ; i < m_ntex ; i++){
        glTextureDescriptor *ptd = m_td_array[i] ;
        
        if( ptd )
            ptd->FreeMap();
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
    return g_glTextureManager->AsJob( m_ChartPath );
}

void glTexFactory::PurgeBackgroundCompressionPool()
{
    //  Purge the "todo" list, and allow any running jobs to complete normally
    g_glTextureManager->PurgeJobList( m_ChartPath );
}
        
void glTexFactory::DeleteSingleTexture( glTextureDescriptor *ptd )
{
    if(!ptd->tex_name)
        return;

    g_tex_mem_used -= ptd->tex_mem_used;
    ptd->level_min = g_mipmap_max_level + 1;  // default, nothing loaded

    glDeleteTextures( 1, &ptd->tex_name );
    ptd->tex_name = 0;
    ptd->tex_mem_used = 0;
    ptd->nGPU_compressed = GPU_TEXTURE_UNKNOWN;
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

glTextureDescriptor *glTexFactory::GetOrCreateTD(const wxRect &rect)
{
    int array_index = ArrayIndex(rect.x, rect.y);
    if( !m_td_array[array_index] ){
        glTextureDescriptor *p = new glTextureDescriptor();
        
        p->x = rect.x;
        p->y = rect.y;
        p->level_min = g_mipmap_max_level + 1;  // default, nothing loaded
        p->m_colorscheme = global_color_scheme;
        m_td_array[array_index] = p;
    }
    return m_td_array[array_index];
}

static void CreateTexture(GLuint &tex_name, bool b_use_mipmaps)
{
    glGenTextures( 1, &tex_name );
        
//        printf("gentex  %d   rect:  %d %d   index %d\n", ptd->tex_name, rect.x, rect.y, array_index);
    glBindTexture( GL_TEXTURE_2D, tex_name );
        
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


    if(b_use_mipmaps)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#ifdef __OCPN__ANDROID__
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
}

bool glTexFactory::BuildTexture(glTextureDescriptor *ptd, int base_level, const wxRect &rect)
{
    // the quality is only slightly worse because linear_mipmap_linear
    // is impossible, but still replace the texture data with the
    // correct level data and using only texture level 0

#ifdef ocpnUSE_GLES
    bool b_use_compressed_mipmaps = false;
    bool b_use_uncompressed_mipmaps = false;
#else
    bool b_use_compressed_mipmaps = true; // best possible quality
    // don't use uncompressed mipmaps as they are temporary
    // we upload only the correct level so the image quality is good anyways
    bool b_use_uncompressed_mipmaps = !g_GLOptions.m_bTextureCompression;
#endif    

    // on systems with little memory we can go up a mipmap level
    // here so that the uncompressed size without mipmaps (level+1)
    // is nearly the compressed size with all the compressed mipmaps
    // this way we won't require 5x more video memory than normal while we
    // are generating the compressed textures, when the cache is complete the image
    // becomes clearer as it is replaces with the higher resolution compressed version
    bool b_lowmem = false; // maybe instead decide based on how much texture memory we have
#ifdef ocpnUSE_GLES
    b_lowmem = g_GLOptions.m_bTextureCompression;
#endif
    if(g_GLOptions.m_bTextureCompression &&
       ptd->nGPU_compressed == GPU_TEXTURE_UNCOMPRESSED) {
        // if compressed data became available we blow away the texture
        if(ptd->comp_array[base_level])
            DeleteSingleTexture(ptd);
    }

    // we are done if the data is already in the texture
    if(base_level == ptd->level_min)
        return false;

    if(base_level > ptd->level_min) {
        // if we already have the mipmaps then we can return,
        // but if we need memory we should free the texture and
        // re-upload just the higher levels needed
        bool b_use_mipmaps = ptd->nGPU_compressed == GPU_TEXTURE_COMPRESSED ?
            b_use_compressed_mipmaps : b_use_uncompressed_mipmaps;
        if(b_use_mipmaps) {
            double factor = 0.5; // we should free uncompressed textures earliest
            bool bGLMemCrunch = g_tex_mem_used > (double)(g_GLOptions.m_iTextureMemorySize * 1024 * 1024) * factor;
            if(!bGLMemCrunch)
                return false; // we already have the data in vram
        }
    }    
    
    int status = GetTextureLevel( ptd, rect, base_level, ptd->m_colorscheme );

    bool b_use_mipmaps = COMPRESSED_BUFFER_OK == status ?
        b_use_compressed_mipmaps : b_use_uncompressed_mipmaps;

    DeleteSingleTexture(ptd);
    CreateTexture(ptd->tex_name, b_use_mipmaps);
    ptd->nGPU_compressed = COMPRESSED_BUFFER_OK == status ?
        GPU_TEXTURE_COMPRESSED : GPU_TEXTURE_UNCOMPRESSED;

    if( COMPRESSED_BUFFER_OK == status) {
        int texture_level = 0;
        for(int level = base_level; level < ptd->level_min; level++ ) {
            int size = TextureTileSize(level, true);
            int status = GetTextureLevel( ptd, rect, level, ptd->m_colorscheme );
            int dim = TextureDim(level);
            s_glCompressedTexImage2D( GL_TEXTURE_2D, texture_level,
                                      g_raster_format, dim, dim, 0, size,
                                      ptd->comp_array[level]);

            ptd->tex_mem_used += size;
            g_tex_mem_used += size;
            texture_level++;
            
            if(!b_use_mipmaps)
                break;
        }

        //   Free bitmap memory that has already been uploaded to the GPU
        ptd->FreeMap();
        ptd->FreeComp();
    } else { // COMPRESSED_BUFFER_OK == status
        if (m_newCatalog) {
            // it's an empty catalog or it's not used, odds it's going to be slow
            OCPNPlatform::ShowBusySpinner();
            m_newCatalog = false;
        }

        //  This level has not been compressed yet, and is not in the cache
#if 1   // perhaps we should eliminate this case
        // and build compressed fxt1 textures one per tick
        if( GL_COMPRESSED_RGB_FXT1_3DFX == g_raster_format &&
            g_GLOptions.m_bTextureCompression) {
            // this version avoids re-uploading the data
            g_glTextureManager->ScheduleJob( this, rect, base_level, true, false, true, true);
            ptd->FreeMap();
            ptd->nGPU_compressed = GPU_TEXTURE_COMPRESSED;
            b_use_mipmaps = b_use_compressed_mipmaps;
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        } else
#endif
        {
            int uc_base_level = base_level;
            if(b_lowmem)
                uc_base_level++;
            int texture_level = 0;
            for(int level = uc_base_level; level < ptd->level_min + b_lowmem; level++) {
                int status = GetTextureLevel( ptd, rect, level, ptd->m_colorscheme );
                int dim = TextureDim(level);
                glTexImage2D( GL_TEXTURE_2D, texture_level, GL_RGB,
                              dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                int size = TextureTileSize(level, false);
                ptd->tex_mem_used += size;
                g_tex_mem_used += size;
                texture_level++;
                
                if(!b_use_mipmaps)
                    break;
            }
        }
    }

    ptd->level_min = base_level;

    // free all mipmaps more than a level less than this
    for( int i = 0; i < base_level-1; i++ ) {
        free( ptd->map_array[i] );
        ptd->map_array[i] = 0;
    }
    
    return true;
}

bool glTexFactory::PrepareTexture( int base_level, const wxRect &rect, ColorScheme color_scheme )
{    
    glTextureDescriptor *ptd = NULL;

    try
    {
    ptd = GetOrCreateTD(rect);

    ptd->m_colorscheme = color_scheme;
    
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE ); // why?
    
    if(!BuildTexture(ptd, base_level, rect))
        glBindTexture( GL_TEXTURE_2D, ptd->tex_name );

    // should we schedule compression?
    if(g_GLOptions.m_bTextureCompression &&
       ptd->nGPU_compressed == GPU_TEXTURE_UNCOMPRESSED) {
        // scheduling at base_level reduces vram usage but is slower overall
        // probably shouldn't be used for caching until it can cache each level
        g_glTextureManager->ScheduleJob( this, rect, 0/*base_level*/,
                                         true, false, true, false);
        if( GL_COMPRESSED_RGB_FXT1_3DFX == g_raster_format )
            glBindTexture( GL_TEXTURE_2D, ptd->tex_name ); // reset texture binding

        // Free the map in ram
        ptd->FreeMap();
    }
    
    //   If global memory is getting short, we can crunch here.
    //   All mipmaps >= ptd->level_min have been uploaded to the GPU,
    //   so there is no reason to save the bits forever.
    //   Of course, this means that if the texture is deleted elsewhere, then the bits will need to be
    //   regenerated.  The price to pay for memory limits....
    
    int mem_used;
    GetMemoryStatus(0, &mem_used);
    //    qDebug() << mem_used;
    if((g_memCacheLimit > 0) && (mem_used > g_memCacheLimit * 7 / 10))
        ptd->FreeMap();

    if((g_memCacheLimit > 0) && (mem_used > g_memCacheLimit * 9 / 10))
        ptd->FreeAll();

#if 0
       // why is the entire chart+factory not freed instead?
    unsigned int nCache = 0;
    unsigned int lcache_limit = (unsigned int)g_nCacheLimit * 8 / 10;
        if(ChartData) {
        nCache = ChartData->GetChartCache()->GetCount();
        if(g_nCacheLimit && (nCache > lcache_limit))
            ptd->FreeAll();
        }
#endif
    
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


bool glTexFactory::UpdateCacheLevel( const wxRect &rect, int level, ColorScheme color_scheme, unsigned char *data, int size)
{
    if( !g_GLOptions.m_bTextureCompressionCaching)
        return false;

    if(!data)
        return false;

    //  Search for the requested texture
        //  Search the catalog for this particular texture
    CatalogEntryValue *v = GetCacheEntryValue(level, rect.x, rect.y, color_scheme) ;
        
    //      This texture is already done
    if(v != 0)
        return false;
    
    return UpdateCachePrecomp(data, size, rect, level, color_scheme);

}

bool glTexFactory::UpdateCacheAllLevels( const wxRect &rect, ColorScheme color_scheme, unsigned char **compcomp_array, int *compcomp_size)
{
    if( !g_GLOptions.m_bTextureCompressionCaching)
        return false;

    bool work = false;

    for (int level = 0; level < g_mipmap_max_level + 1; level++ )
        work |= UpdateCacheLevel( rect, level, color_scheme, compcomp_array[level], compcomp_size[level] );
    if (work) {
        WriteCatalogAndHeader();
    }    
    
    return work;
}

int glTexFactory::GetTextureLevel( glTextureDescriptor *ptd, const wxRect &rect,
                                   int level, ColorScheme color_scheme )
{
    //  Already available in the texture descriptor?
    if(g_GLOptions.m_bTextureCompression) {
        if( ptd->comp_array[level] )
            return COMPRESSED_BUFFER_OK;
        if( ptd->compcomp_array[level] ) {
            // If we have the compcomp bits in ram decompress them
            int size = TextureTileSize(level, true);
            unsigned char *cb = (unsigned char*)malloc(size);
            LZ4_decompress_fast((char*)ptd->compcomp_array[level], (char*)cb, size);
            ptd->comp_array[level] = cb;
            return COMPRESSED_BUFFER_OK;
        } else if(g_GLOptions.m_bTextureCompressionCaching) {
            //  If cacheing compressed textures, look in the cache
            //  Search for the requested texture
            //  Search the catalog for this particular texture
            CatalogEntryValue *p = GetCacheEntryValue(level, rect.x, rect.y, color_scheme);
        
            //      Requested texture level is found in the cache
            //      so go load it
            if( p != 0 ) {
                int size = TextureTileSize(level, true);

                if(m_fs->IsOpened()){
                    m_fs->Seek(p->texture_offset);
                    ptd->comp_array[level] = (unsigned char*)malloc(size);
                    int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
                    char *compressed_data = (char*)malloc(p->compressed_size);
                    m_fs->Read(compressed_data, p->compressed_size);
                    LZ4_decompress_fast(compressed_data, (char*)ptd->comp_array[level], size);
                    free(compressed_data);
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
#if 0
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
#endif

bool glTexFactory::UpdateCachePrecomp(unsigned char *data, int data_size, const wxRect &rect,
                                      int level, ColorScheme color_scheme, bool write_catalog)
{
    if (level < 0 || level >= MAX_TEX_LEVEL)
        return false;	// XXX BUG

    //  Search the catalog for this particular texture
    if (GetCacheEntryValue(level, rect.x, rect.y, color_scheme) != 0) 
        return false;
    
    // Make sure the file exists
    wxASSERT(m_fs != 0);
        
    if( ! m_fs->IsOpened() )
        return false;

    //      Create a new catalog entry
    CatalogEntry p( level, rect.x, rect.y, color_scheme);
    
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
