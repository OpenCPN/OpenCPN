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

void HalfScaleChartBits( int width, int height, unsigned char *source, unsigned char *target );
extern PFNGLGETCOMPRESSEDTEXIMAGEPROC s_glGetCompressedTexImage;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
extern PFNGLGENERATEMIPMAPEXTPROC          s_glGenerateMipmap;
extern bool GetMemoryStatus( int *mem_total, int *mem_used );


#include <wx/arrimpl.cpp> 
WX_DEFINE_OBJARRAY(ArrayOfCatalogEntries);

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
    
    m_size_X = pBSBChart->GetSize_X();
    m_size_Y = pBSBChart->GetSize_Y();
    
    //  Calculate the number of textures needed
    m_tex_dim = g_GLOptions.m_iTextureDimension;
    m_nx_tex = ( m_size_X / m_tex_dim ) + 1;
    m_ny_tex = ( m_size_Y / m_tex_dim ) + 1;
    
    m_stride = m_nx_tex;
    m_ntex = m_nx_tex * m_ny_tex;
    m_td_array = (glTextureDescriptor **)calloc(m_ntex, sizeof(glTextureDescriptor *));
    
    
}

glTexFactory::~glTexFactory()
{
    if(m_fs && m_fs->IsOpened()){
        m_fs->Close();
    }
    
    DeleteAllDescriptors();
    
    free( m_td_array );         // array is empty
}

void glTexFactory::DeleteTexture(const wxRect &rect)
{
    //    Is this texture tile defined?
    int array_index = ((rect.y / m_tex_dim) * m_stride) + (rect.x / m_tex_dim);
    glTextureDescriptor *ptd = m_td_array[array_index];
    
    if( ptd && ptd->tex_name > 0 )
        DeleteSingleTexture( ptd );
}

void glTexFactory::DeleteAllTextures( void )
{
    // iterate over all the textures presently loaded
    // and delete the OpenGL texture from the GPU
    // but keep the private texture descriptor for now
    
    for(int i=0 ; i < m_ntex ; i++){
        glTextureDescriptor *ptd = m_td_array[i] ;
        
        if( ptd )
            DeleteSingleTexture( ptd);
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

void glTexFactory::DeleteSingleTexture( glTextureDescriptor *ptd )
{
    /* compute space saved */
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size, orig_min = ptd->level_min;
    
    for(int level = 0; level < g_mipmap_max_level + 1; level++) {
        if(level == ptd->level_min) {
            g_tex_mem_used -= size;
            ptd->level_min++;
        }
        size /= 4;
        
        if(g_GLOptions.m_bTextureCompression && size < 8)
            size = 8;
    }
    
    glDeleteTextures( 1, &ptd->tex_name );
    ptd->tex_name = 0;
}
    
void glTexFactory::PrepareTexture( int base_level, const wxRect &rect, ColorScheme color_scheme )
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
        
    }
    ptd = m_td_array[array_index];
    
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

    //  Texture requested has already been physically uploaded to the GPU
    //  so we merely need to bind it
    if(base_level >= ptd->level_min){
        glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
        return;
    }

    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( m_pchart );
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( m_pchart );

    if( !pPlugInWrapper && !pBSBChart ) return;

    bool b_plugin = pPlugInWrapper != NULL;
    
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;

    
    /* optimization: when supported generate mipmaps
       with hardware acceleration if _not_ using texture compression.
       It may be faster on some hardware to use this with compression,
       but for my computers it is much slower. */
    bool hw_mipmap = false;
#ifndef ocpnUSE_GLES /* glGenerateMipmaps is incredibly slow with mali drivers */
    if(!g_GLOptions.m_bTextureCompression && s_glGenerateMipmap) {
        if( g_bDebugOGL )
            wxLogMessage( wxString::Format(_T("  -->UploadTexture Using hardware mipmaps")));
        base_level = 0;
        hw_mipmap = true;
    }
#endif


    for(int level = 0; level < g_mipmap_max_level+1; level++ ) {
        //    Upload to GPU?
        if( level >= base_level ) {
            unsigned char *tex_data = GetTextureLevel( rect, level, color_scheme );
 
            if(g_GLOptions.m_bTextureCompression) {
                s_glCompressedTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                                          dim, dim, 0, size, tex_data );
                
            }
            else {
                glTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                              dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, tex_data );
            }
                
            g_tex_mem_used += size;
        }
   
        if(hw_mipmap) {
            //  Base level has been loaded to GPU
            //  Use OGL driver to generate the rest of the mipmaps, and then break the loop
            /* compute memory used for mipmaps */
            dim /= 2;
            size /= 4;
       
            for(int slevel = base_level + 1; slevel < g_mipmap_max_level+1; slevel++ ) {
                g_tex_mem_used += size;
                dim /= 2;
                size /= 4;
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
        if(size < 8)
            size = 8;
    }

 
#ifndef ocpnUSE_GLES
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, g_mipmap_max_level );
#endif
    ptd->level_min = base_level;

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
    
    if( ((g_memCacheLimit > 0) && (mem_used > g_memCacheLimit * 8 / 10)) ||
        (g_nCacheLimit && (nCache > lcache_limit)) )
      
    {        
        for( int i = 0; i < 10; i++ ){
            free( ptd->map_array[i] );
            free( ptd->comp_array[i] );
        
            ptd->map_array[i] = 0;
            ptd->comp_array[i] = 0;
        }
    }
    
}



unsigned char *glTexFactory::GetTextureLevel( const wxRect &rect, int level, ColorScheme color_scheme )
{
    //    Is this texture tile already defined?
    int array_index = ((rect.y / m_tex_dim) * m_stride) + (rect.x / m_tex_dim);
    glTextureDescriptor *ptd = m_td_array[array_index];
    
    // if not found in the hash map, then get the bits as a texture descriptor
    if( !ptd ){
        glTextureDescriptor *p = new glTextureDescriptor;
        p->x = rect.x;
        p->y = rect.y;
        p->level_min = g_mipmap_max_level + 1;  // default, nothing loaded
        m_td_array[array_index] = p;
        
    }
    
    ptd = m_td_array[array_index];
    
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
    } else {
        
        //  maybe us glIsTexture() here??
        wxStopWatch sw;
        glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
        sw.Pause();
        long tt = sw.Time();
        if( tt > 10 && g_GLOptions.m_iTextureMemorySize > 16 )
            g_GLOptions.m_iTextureMemorySize *= .8;
    }
    
                         
                         
    
    //  Already available in the texture descriptor?
    if(g_GLOptions.m_bTextureCompression) {
        if( ptd->comp_array[ level ] )
            return ptd->comp_array[ level ];
    }
    else {
        if( ptd->map_array[ level ] )
            return ptd->map_array[ level ];
    }
        
    int dim = g_GLOptions.m_iTextureDimension;
    int size = g_tile_size;
    
    for(int i=0 ; i < level ; i++){
        dim /= 2;
        size /= 4;
        if(size < 8)
            size = 8;
        
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
 
            if(m_fs->IsOpened()){
                uint32_t off = m_fs->Tell();
                m_fs->Seek(p->texture_offset);
                ptd->comp_array[level] = (unsigned char*)malloc(size);
                
                int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
                char *compressed_data = new char[max_compressed_size];
                m_fs->Read(compressed_data, p->compressed_size);
                
                LZ4_decompress_fast(compressed_data, (char*)ptd->comp_array[level], size);
                delete [] compressed_data;    
            }
                
            ptd->level_min = wxMin(ptd->level_min, level);
            return ptd->comp_array[ level ];
        }
    }
 
    //  Requested Texture level is not in cache, and not already built
    //  So go build it
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
                if( !ptd->map_array[0] ){
                    // Load level 0 uncompressed data
                    wxRect ncrect(rect);
                    
                    //    Prime the pump with the "zero" level bits, ie. 1x native chart bits
                    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( m_pchart );
                    
                    if( pBSBChart ) {
                        unsigned char *t_buf = (unsigned char *) malloc( ncrect.width * ncrect.height * 4 );
                        pBSBChart->GetChartBits( ncrect, t_buf, 1 );
                        
                        //    and cache them here
                        ptd->map_array[0] = t_buf;
                    }
                }
                
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
        
    
    //  Do the compression, if needed
    if(g_GLOptions.m_bTextureCompression) {
        if(m_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
            m_raster_format == GL_ETC1_RGB8_OES)
        {
            unsigned char *tex_data = (unsigned char*)malloc(size);
            if(m_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
                bool first = true;
                for(int i=0 ; i < ptd->level_min ; i++) {
                    if(ptd->comp_array[i]) {
                        first = false;
                        break;
                    }
                }
                FlattenColorsForCompression(ptd->map_array[level], dim, first);
 
                // color range fit is worse quality but twice as fast
                int flags = squish::kDxt1 | squish::kColourRangeFit;
                
                if( g_GLOptions.m_bTextureCompressionCaching) {
                /* use slower cluster fit since we are building the cache for
                 * better quality, this takes roughly 25% longer and uses about
                 * 10% more disk space (result doesn't compress as well with lz4) */
                    flags = squish::kDxt1 | squish::kColourClusterFit;
                }
                
                squish::CompressImageRGB(ptd->map_array[level], dim, dim, tex_data, flags);
            } else if(m_raster_format == GL_ETC1_RGB8_OES) 
                CompressDataETC(ptd->map_array[level], dim, size, tex_data);
            
            ptd->comp_array[level] = tex_data;
            
        }
        else {
            //      We are using the OpenGL driver to do the compression
            //      Use a temporary texture to do the work so that it can be safely deleted and memory recovered 
            
            GLuint temp_tex_name;
            glGenTextures( 1, &temp_tex_name );
            glBindTexture( GL_TEXTURE_2D, temp_tex_name );
                    
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                    
#ifdef ocpnUSE_GLES /* this is slightly faster */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
#else /* looks nicer */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
#endif
                    
            glTexImage2D( GL_TEXTURE_2D, level, m_raster_format,
                              dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                
                //      Now read it back
            ptd->comp_array[ level ] = (unsigned char*)malloc(size);
            s_glGetCompressedTexImage(GL_TEXTURE_2D, level, ptd->comp_array[ level ]);

            glDeleteTextures( 1, &temp_tex_name );

            //  Re-Bind the method target texture
            glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
            
        }
            
        if( g_GLOptions.m_bTextureCompressionCaching)
            UpdateCache(ptd->comp_array[ level ], size, ptd, level, color_scheme);

        return ptd->comp_array[ level ];
    }
    
           // No compression at all
    return ptd->map_array[ level ];
    
    
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

        
        


