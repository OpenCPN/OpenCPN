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

#include <stdint.h>

#include "GL/gl.h"

#include "glChartCanvas.h"
#include "glTextureDescriptor.h"
#include "chcanv.h"
#include "dychart.h"
#include "s52plib.h"
#include "Quilt.h"
#include "pluginmanager.h"
#include "routeman.h"
#include "chartbase.h"
#include "chartimg.h"
#include "s57chart.h"
#include "ChInfoWin.h"
#include "thumbwin.h"
#include "chartdb.h"
#include "navutil.h"
#include "TexFont.h"

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif

#ifdef USE_S57
#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#endif

#include "squish.h"
#include "lz4.h"
#include "lz4hc.h"

#ifdef USE_S57
#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#endif

extern bool GetMemoryStatus(int *mem_total, int *mem_used);

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#endif

extern ChartCanvas *cc1;
extern s52plib *ps52plib;
extern bool g_bopengl;
extern int g_GPU_MemSize;
extern bool g_bDebugOGL;
GLenum       g_texture_rectangle_format;

extern bool g_bskew_comp;
extern int g_memCacheLimit;
extern bool g_bCourseUp;
extern ChartBase *Current_Ch;
extern ColorScheme global_color_scheme;
extern bool g_bquiting;
extern ThumbWin         *pthumbwin;
extern bool             g_bDisplayGrid;

extern double           gLat, gLon, gCog, gSog, gHdt;

extern int              g_OwnShipIconType;
extern double           g_ownship_predictor_minutes;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;

extern double           gLat, gLon, gCog, gSog, gHdt;

extern int              g_OwnShipIconType;
extern double           g_ownship_predictor_minutes;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;

extern int              g_GroupIndex;
extern ChartDB          *ChartData;

extern PlugInManager* g_pi_manager;

extern WayPointman      *pWayPointMan;
extern RouteList        *pRouteList;

ocpnGLOptions g_GLOptions;


PFNGLGENFRAMEBUFFERSEXTPROC         s_glGenFramebuffers;
PFNGLGENRENDERBUFFERSEXTPROC        s_glGenRenderbuffers;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    s_glFramebufferTexture2D;
PFNGLBINDFRAMEBUFFEREXTPROC         s_glBindFramebuffer;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC s_glFramebufferRenderbuffer;
PFNGLRENDERBUFFERSTORAGEEXTPROC     s_glRenderbufferStorage;
PFNGLBINDRENDERBUFFEREXTPROC        s_glBindRenderbuffer;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  s_glCheckFramebufferStatus;
PFNGLDELETEFRAMEBUFFERSEXTPROC      s_glDeleteFramebuffers;
PFNGLDELETERENDERBUFFERSEXTPROC     s_glDeleteRenderbuffers;

PFNGLGENERATEMIPMAPEXTPROC          s_glGenerateMipmap;

PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
PFNGLGETCOMPRESSEDTEXIMAGEPROC s_glGetCompressedTexImage;

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( ArrayOfTexDescriptors );

GLuint g_raster_format = GL_RGB;

#ifdef __WXMSW__
#define FORMAT_BITS           GL_BGR
#else
#define FORMAT_BITS           GL_RGB
#endif

static       long g_tex_mem_used;

static int g_tile_size;

#if defined(__MSVC__) && !defined(ocpnUSE_GLES) /* this compiler doesn't support vla */
const
#endif
static int g_mipmap_max_level = 4;

bool glChartCanvas::s_b_useScissorTest;
bool glChartCanvas::s_b_useStencil;
bool glChartCanvas::s_b_UploadFullCompressedMipmaps;
//static int s_nquickbind;

long populate_tt_total, mipmap_tt_total, hwmipmap_tt_total, upload_tt_total;
long uploadcomp_tt_total, downloadcomp_tt_total, decompcomp_tt_total, readcomp_tt_total, writecomp_tt_total;

/* generate mipmap in software */
static void HalfScaleChartBits( int width, int height, unsigned char *source, unsigned char *target )
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

/* delete mipmaps less than base_level.  Delete the texture if all
   mipmaps are deleted, and return true if the texture descriptor is also deleted */
static void DeleteTexture(glTextureDescriptor *ptd, int base_level)
{
    wxASSERT(base_level >= 0 && base_level <= g_mipmap_max_level + 1);
    if( ptd->tex_name > 0 ) {
        /* compute space saved */
        int dim = g_GLOptions.m_iTextureDimension;
        int size = g_tile_size, orig_min = ptd->level_min;
        
        for(int level = 0; level < base_level; level++) {
            if(level == ptd->level_min) {
                g_tex_mem_used -= size;
                ptd->level_min++;
            }
            size /= 4;

            if(g_GLOptions.m_bTextureCompression && size < 8)
                size = 8;
        }
        
        if(ptd->level_min > g_mipmap_max_level) {
            if( g_bDebugOGL )
                wxLogMessage(wxString::Format(_T("glDeleteTextures entire tile %d"), ptd->tex_name ));
            glDeleteTextures( 1, &ptd->tex_name );
            ptd->tex_name = 0;
        } else {
            if( g_bDebugOGL ) wxLogMessage(wxString::Format(_T("glDeleteTextures up to %d level" ), base_level));

            /* free only unused mipmaps */
            for(int level = orig_min; level < ptd->level_min; level++) {
                glTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                              0, 0, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, NULL );
            }

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, ptd->level_min );
        }
    }
}

/* reduce pixel values to 5/6/5, because this is the format they are stored
   when compressed anyway, and this way the compression algorithm will use
   the exact same color in  adjacent 4x4 tiles and the result is nicer for our purpose.
   the lz4 compressed texture is smaller as well. */
static void FlattenColorsForCompression(unsigned char *data, int dim, bool swap_colors=true)
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
static void CompressDataETC(const unsigned char *data, int dim, int size,
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

extern wxString         g_PrivateDataDir;

wxString CompressedCachePath(wxString path)
{
#if defined(__WXMSW__)
    int colon = path.find(':', 0);
    path.Remove(colon, 1);
#endif

    /* replace path separators with ! */
    wxChar separator = wxFileName::GetPathSeparator();
    for(unsigned int pos = 0; pos < path.size();
        pos = path.find(separator, pos))
        path.replace(pos, 1, _T("!"));

    return g_PrivateDataDir + separator + _T("raster_texture_cache") + separator + path 
        + _T(".compressed_chart");
}

#define COMPRESSED_CACHE_MAGIC 0xf00f  // change this when the format changes
struct CompressedCacheHeader
{
    uint32_t magic, format, tiles;
};

/* load from base_level to needed levels of the texture */
void UploadTexture( glTextureDescriptor *ptd, int base_level,
                    const wxRect &rect, ChartBase *pchart, bool ramonly=false )
{
#ifdef ocpnUSE_GLES /* gles requires a complete set of mipmaps starting at 0 */
    base_level = 0;
#endif

    /* Also, some non-compliant OpenGL drivers need the complete mipmap set when using compressed textures */
    if( glChartCanvas::s_b_UploadFullCompressedMipmaps && g_GLOptions.m_bTextureCompression )
        base_level = 0;
    
    if(!ramonly) {
        //    If the GPU does not know about this texture, create it
        if( ptd->tex_name == 0 ) {
            glGenTextures( 1, &ptd->tex_name );

            if( g_bDebugOGL )
                wxLogMessage(wxString::Format( _T("  -->UploadTexture %d base_level %d"), ptd->tex_name, base_level ));

            glBindTexture( GL_TEXTURE_2D, ptd->tex_name );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

#ifdef ocpnUSE_GLES /* this is slightly faster */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
#else /* looks nicer */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
#endif
            ptd->level_min = (g_mipmap_max_level+1);
        } else {
            wxStopWatch sw;
            glBindTexture( GL_TEXTURE_2D, ptd->tex_name );
            sw.Pause();
            long tt = sw.Time();
            if( tt > 10 && g_GLOptions.m_iTextureMemorySize > 16 )
                g_GLOptions.m_iTextureMemorySize *= .8;
        }

#if 0
    if( n_longbind ) m_tex_max_res--;
    else {
        if( s_nquickbind++ > 100 ) {
            s_nquickbind = 0;
            m_tex_max_res++;
        }
    }
#endif


        /* this test is not required, but is a shortcut if everything is already loaded */
        if(base_level >= ptd->level_min)
            return;

    } else
        ptd->level_min = (g_mipmap_max_level+1);

    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( pchart );
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );

    if( !pPlugInWrapper && !pBSBChart ) return;

    bool b_plugin = pPlugInWrapper != NULL;
    int size;

    bool compressed = false;

    if(g_GLOptions.m_bTextureCompression &&
       g_GLOptions.m_bTextureCompressionCaching) {

        wxString CompressedCacheFilePath = CompressedCachePath(pchart->GetFullPath());

        if( g_bDebugOGL )
            wxLogMessage( wxString::Format(_T("  -->UploadTexture, compressedcachedpath: ")
                                           + CompressedCacheFilePath));

        if(wxFileName::FileExists(CompressedCacheFilePath)) {
            /* load needed levels into map_array */
            size = g_tile_size;

            ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );

            if(!pBSBChart) /* chart type not supported */
                goto load_base_level;

            wxStopWatch sw;
            wxFileInputStream fs(CompressedCacheFilePath);

            CompressedCacheHeader hdr;
            fs.Read(&hdr, sizeof hdr);

            int size_X = pBSBChart->GetSize_X();
            int size_Y = pBSBChart->GetSize_Y();
            int tex_dim = g_GLOptions.m_iTextureDimension;

            int nx_tex = ( size_X / tex_dim ) + 1;
            int ny_tex = ( size_Y / tex_dim ) + 1;

            /* test if cache is valid */
            if(hdr.magic != COMPRESSED_CACHE_MAGIC || hdr.format != g_raster_format ||
               hdr.tiles != (unsigned int)nx_tex*ny_tex*(g_mipmap_max_level+1)) {
                wxLogMessage(wxString::Format(_T("compressed cache miss, removing:\
 %s, suggest rebuild chart database"), (const char*)CompressedCacheFilePath.ToUTF8()));
                wxRemoveFile(CompressedCacheFilePath);

                /* should rebuild the file here instead  of skipping */
                goto load_base_level;
            }

            int offset_table = sizeof hdr;

            int j = rect.x / tex_dim, i = rect.y / tex_dim;

            int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
            char *compressed_data = new char[max_compressed_size];

            int offset_table_offset = ((i * nx_tex) + j) * (g_mipmap_max_level+1) + base_level;
            if(fs.SeekI(offset_table + offset_table_offset * 4, wxFromStart) == wxInvalidOffset) {
            corrupted:
                wxLogMessage(wxString::Format(_T("compressed cache file corrupted, removing:\
 %s, suggest rebuild chart database"), (const char*)CompressedCacheFilePath.ToUTF8()));
                wxRemoveFile(CompressedCacheFilePath);
                delete [] compressed_data;
                goto load_base_level;
            }

            uint32_t offsets[(g_mipmap_max_level+1) + 1];
            uint32_t offset_table_size = 4 * (ptd->level_min + 1 - base_level);
            fs.Read(offsets + base_level, offset_table_size );
            if(fs.LastRead() != offset_table_size)
                goto corrupted;
            if(fs.SeekI(offsets[base_level], wxFromStart) == wxInvalidOffset)
                goto corrupted;
            readcomp_tt_total += sw.Time();

            for(int level = base_level; level < ptd->level_min; level++) {
                uint32_t compressed_size = offsets[level+1] - offsets[level];
                wxStopWatch sw;
                fs.Read(compressed_data, compressed_size);
                if(fs.LastRead() != compressed_size)
                    goto corrupted;
                readcomp_tt_total += sw.Time();

                ptd->map_array[level] = (unsigned char *) malloc( size );
                wxStopWatch sw2;
                LZ4_decompress_fast(compressed_data, (char*)ptd->map_array[level], size);
                decompcomp_tt_total += sw2.Time();

                size /= 4;
                if(size < 8)
                   size = 8;
            }

            delete [] compressed_data;
            compressed = true;          
        } else /* need to build the cache */
            goto load_base_level;
    } else if(!ptd->map_array[0]) {
    load_base_level:

        if( g_bDebugOGL )
            wxLogMessage( wxString::Format(_T("  -->UploadTexture loading uncompressed base level")));

        // Load level 0 uncompressed data
        unsigned char *t_buf = (unsigned char *) malloc( rect.width * rect.height * 4 );
        
        wxRect nc_rect = rect;

        wxStopWatch sw;
        //    Prime the pump with the "zero" level bits, ie. 1x native chart bits
        if( b_plugin ) pPlugInWrapper->GetChartBits( nc_rect, t_buf, 1 );
        else
            pBSBChart->GetChartBits( nc_rect, t_buf, 1 );
        populate_tt_total += sw.Time();
        
        //    and cache them here
        ptd->map_array[0] = t_buf;
    }

    /* upload needed textures */
    int dim = g_GLOptions.m_iTextureDimension;

    size = g_tile_size;

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

    bool first = true;
    for(int level = 0; level < ptd->level_min; level++ ) {
        /* build mipmap in ram if needed */
        if(!compressed && !ptd->map_array[level]) {
            ptd->map_array[level] = (unsigned char *) malloc( dim * dim * 3 );

            wxStopWatch sw;
            HalfScaleChartBits( 2*dim, 2*dim, ptd->map_array[level - 1], ptd->map_array[level] );
            mipmap_tt_total += sw.Time();
        }
        
        //    Upload to GPU?
        if( level >= base_level && !ramonly) {
            if(compressed) { /* data is already compressed */
                if(size < 8)
                    size = 8;

                wxStopWatch sw;
                s_glCompressedTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                                          dim, dim, 0, size, ptd->map_array[level] );
                uploadcomp_tt_total += sw.Time();

                ptd->map_array[level] = NULL;

                g_tex_mem_used += size;
            } else {
                wxStopWatch sw;
                if(g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
                   g_raster_format == GL_ETC1_RGB8_OES)
                {
                    if(size < 8)
                        size = 8;

                    unsigned char *tex_data = (unsigned char*)malloc(size);
                    if(g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
                        FlattenColorsForCompression(ptd->map_array[level], dim, first);
                        first = false;

                        // color range fit is worse quality but twice as fast
                        int flags = squish::kDxt1 | squish::kColourRangeFit;
                        squish::CompressImageRGB(ptd->map_array[level], dim, dim,
                                                 tex_data, flags);
                    } else if(g_raster_format == GL_ETC1_RGB8_OES)
                        CompressDataETC(ptd->map_array[level], dim, size, tex_data);
                    s_glCompressedTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                                              dim, dim, 0, size, tex_data );
                    free(tex_data);
                } else /* use driver to do compression, or not compressed */
                    glTexImage2D( GL_TEXTURE_2D, level, g_raster_format,
                                  dim, dim, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );
                upload_tt_total += sw.Time();

                g_tex_mem_used += size;

                if(hw_mipmap) {
                    /* compute memory used for mipmaps */
                    int level;
                    dim /= 2;
                    size /= 4;

                    for(level = 1; level < ptd->level_min; level++ ) {
                        g_tex_mem_used += size;
                        dim /= 2;
                        size /= 4;
                    }
                    ptd->level_min = level;
        
#ifndef ocpnUSE_GLES
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level );
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, ptd->level_min-1 );
#endif
                    /* some ATI drivers require this, so to be safe... */
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

                    wxStopWatch sw;
                    if( g_bDebugOGL )
                        wxLogMessage( wxString::Format(_T("  -->UploadTexture Generate hw mipmaps")));

                    s_glGenerateMipmap( GL_TEXTURE_2D );
                    hwmipmap_tt_total += sw.Time();

                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
                    break;
                }
            }
        }

        dim /= 2;
        size /= 4;
    }

    if( g_bDebugOGL )
        wxLogMessage( wxString::Format(_T("  -->UploadTexture Setting texture parameters...")));

#ifndef ocpnUSE_GLES
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, g_mipmap_max_level );
#endif
    ptd->level_min = base_level;

    if( g_bDebugOGL ) {
        wxString msg;
        msg.Printf( _T("  -->tex_mem_used %.3f MB"), g_tex_mem_used / 1024.0 / 1024.0);
        wxLogMessage( msg );
    }
}

bool CompressChart(ChartBase *pchart, wxString CompressedCacheFilePath, wxString filename, bool ramonly)
{
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );
    int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
    char *compressed_data = new char[max_compressed_size];
        
    if(pBSBChart) {

        int size_X = pBSBChart->GetSize_X();
        int size_Y = pBSBChart->GetSize_Y();

        int tex_dim = g_GLOptions.m_iTextureDimension;
            
        int nx_tex = ( size_X / tex_dim ) + 1;
        int ny_tex = ( size_Y / tex_dim ) + 1;

        /* save the compressed data */
        wxFileOutputStream fs(CompressedCacheFilePath);

        if(!fs.IsOk()) {
            wxLogMessage( _T("BuildCompressedCache() Failed to write to: ") + CompressedCacheFilePath );
            goto fail;
        }

        CompressedCacheHeader hdr;
        hdr.magic = COMPRESSED_CACHE_MAGIC;
        hdr.format = g_raster_format;
        hdr.tiles = nx_tex * ny_tex * (g_mipmap_max_level+1);

        /* skip past where the header will go (write it last) */
        fs.SeekO(sizeof hdr, wxFromCurrent);

        int offset_table = fs.TellO();

        /* give offset after last tile to make computing size simple */
        int offset_table_count = hdr.tiles + 1;
        fs.SeekO(offset_table_count * 4, wxFromCurrent);

        wxRect rect;
        rect.y = 0;
        for( int y = 0; y < ny_tex; y++ ) {
            rect.height = tex_dim;
            rect.x = 0;
            for( int x = 0; x < nx_tex; x++ ) {
                rect.width = tex_dim;

                glTextureDescriptor ptd;

                UploadTexture( &ptd, 0, rect, pchart,
                               g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
                               g_raster_format == GL_ETC1_RGB8_OES);

                int size = g_tile_size;
                for(int level = 0; level <= g_mipmap_max_level; level++ ) {
                    unsigned char *tex_data = (unsigned char*)malloc(size);
                    int dim = tex_dim >> level;

                    wxStopWatch sw;
                    /* now compress map_array into tex_data */
                    if(g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
                        /* use slower cluster fit since we are building the cache for
                           better quality, this takes roughly 25% longer and uses about
                           10% more disk space (result doesn't compress as well with lz4) */
                        int flags = squish::kDxt1 | squish::kColourClusterFit;
                        FlattenColorsForCompression(ptd.map_array[level], dim);
                        squish::CompressImageRGB(ptd.map_array[level], dim, dim, tex_data,
                                                 flags);
                    } else if(g_raster_format == GL_ETC1_RGB8_OES)
                        CompressDataETC(ptd.map_array[level], dim, size, tex_data);
                    else {
                        /* the graphics driver does the work of compression,
                           just read the compressed data back */
                        /* uncompressed ram buffer is big enough and already allocated */
                        tex_data = ptd.map_array[level];
                        s_glGetCompressedTexImage(GL_TEXTURE_2D, level, tex_data);
                        g_tex_mem_used -= size;
                    }
                    downloadcomp_tt_total += sw.Time();

                    wxStopWatch sw2;
                    uint32_t offset = fs.TellO();

                    int compressed_size = LZ4_compressHC2((char*)tex_data, compressed_data, size, 4);
                    free(tex_data);

                    fs.Write(compressed_data, compressed_size);

                    /* update the offset table */
                    int offset_table_offset = ((y * nx_tex) + x) * (g_mipmap_max_level+1) + level;
                    fs.SeekO(offset_table + offset_table_offset * 4, wxFromStart);
                    fs.Write(&offset, 4);
                    fs.SeekO(0, wxFromEnd);

                    writecomp_tt_total += sw2.Time();

                    size /= 4;
                    if(size < 8)
                        size = 8;
                }

                glDeleteTextures(1, &ptd.tex_name);
                rect.x += rect.width;
            }
            rect.y += rect.height;
        }

        /* update final offset */
        uint32_t offset = fs.TellO();

        int offset_table_offset = ny_tex * nx_tex * (g_mipmap_max_level+1);
        fs.SeekO(offset_table + offset_table_offset * 4, wxFromStart);
        fs.Write(&offset, 4);

        /* write header now that all the data in the file is valid */
        fs.SeekO(0, wxFromStart);
        fs.Write(&hdr, sizeof hdr);

        if( g_bDebugOGL )
        {
            wxFileName fn = filename;
            printf("%s: %f -> %f  ratio:%4.2fx\n",
                   (const char*)fn.GetName().ToUTF8(),
                   (double)fn.GetSize().ToULong() / 1024.0 / 1024.0,
                   (double)offset / 1024.0 / 1024.0,
                   (double) offset / fn.GetSize().ToULong());
        }
    }

fail:
    delete [] compressed_data;

    return true;
}

class CompressedCacheWorkerThread : public wxThread
{
public:
    CompressedCacheWorkerThread(ChartBase *pc, wxString CCFP, wxString fn)
        : wxThread(wxTHREAD_JOINABLE), pchart(pc), CompressedCacheFilePath(CCFP), filename(fn)
        { Create(); }
    void *Entry() {
        CompressChart(pchart, CompressedCacheFilePath, filename, true);
        return 0;
    }

    ChartBase *pchart;
    wxString CompressedCacheFilePath;
    wxString filename;
};

void BuildCompressedCache()
{
    /* determine how many cached charts we will generate so the progress bar has the right count */
    int count = 0;
    for(int i = 0; i<ChartData->GetChartTableEntries(); i++) {
        /* skip if not kap */
        const ChartTableEntry &cte = ChartData->GetChartTableEntry(i);
        ChartTypeEnum chart_type = (ChartTypeEnum)cte.GetChartType();
        if(chart_type != CHART_TYPE_KAP)
            continue;

        wxString CompressedCacheFilePath = CompressedCachePath(ChartData->GetDBChartFileName(i));
        wxFileName fn(CompressedCacheFilePath);
        if(fn.FileExists()) /* skip if file exists */
            continue;
        count++;
    }

    if( g_bDebugOGL ) wxLogMessage(wxString::Format(_T("BuildCompressedCache() count = %d"), count ));

    if(count == 0)
        return;

    wxProgressDialog *pprog = new wxProgressDialog
        (_("OpenCPN Compressed Cache Update"), _T(""), count, GetOCPNCanvasWindow(), wxPD_SMOOTH
         | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME | wxPD_CAN_SKIP);

    //    Make sure the dialog is big enough to be readable
    pprog->Hide();
    wxSize sz = pprog->GetSize();
    wxSize csz = GetOCPNCanvasWindow()->GetClientSize();
    sz.x = csz.x * 7 / 10;
    pprog->SetSize( sz );
    pprog->Centre();
    pprog->Update( 0, _T("") ); // Sometimes this lock opencpn up because of recursive event loop
    pprog->Show();
    pprog->Raise();

    /* disable caching so we don't use the cache when we upload the texture */
    g_GLOptions.m_bTextureCompressionCaching = false;

    /* do we compress in ram using builtin libraries, or do we
       upload to the gpu and use the driver to perform compression?
       we have builtin libraries for DXT1 (squish) and ETC1 (etcpak)
       FXT1 must use the driver, ETC1 cannot, and DXT1 can use the driver
       but the results are worse and don't compress well.

    additionally, if we use the driver we must stay single threaded in this thread
    (unless we created multiple opengl contexts), but with with our own libraries,
    we can use multiple threads to take advantage of multiple cores */

    bool ramonly = false;
    if(g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
        ramonly = true;
#ifdef ocpnUSE_GLES
    if(g_raster_format == GL_ETC1_RGB8_OES)
        ramonly = true;
#endif

    int thread_count = 0;
    CompressedCacheWorkerThread **workers = NULL;
    if(ramonly) {
        thread_count = wxThread::GetCPUCount();
        workers = new CompressedCacheWorkerThread*[thread_count];
        for(int t = 0; t < thread_count; t++)
            workers[t] = NULL;
    }

    // build cached compressed charts
    count = 0;
    for(int i = 0; i<ChartData->GetChartTableEntries(); i++) {
        /* skip if not kap */
        const ChartTableEntry &cte = ChartData->GetChartTableEntry(i);
        ChartTypeEnum chart_type = (ChartTypeEnum)cte.GetChartType();
        if(chart_type != CHART_TYPE_KAP)
            continue;

        wxString filename = ChartData->GetDBChartFileName(i);
        wxString CompressedCacheFilePath = CompressedCachePath(filename);
        wxFileName fn(CompressedCacheFilePath);

        if(fn.FileExists()) /* skip if file exists */
            continue;

        if(!fn.DirExists())
            fn.Mkdir();

        if( g_bDebugOGL ) wxLogMessage(wxString::Format(_T("BuildCompressedCache() File:"
                                                           + CompressedCacheFilePath )));
        count++;

        ChartBase *pchart = ChartData->OpenChartFromDB( i, FULL_INIT );
        if(!pchart) /* probably a corrupt chart */
            continue;

        bool skip = false;
        pprog->Update(count-1, pchart->GetFullPath(), &skip );
        if(skip)
            break;

        if(ramonly) {
            int t = 0;
            for(;;) {
                if(!workers[t]) {
                    workers[t] = new CompressedCacheWorkerThread
                        (pchart, CompressedCacheFilePath, filename);

                    workers[t]->Run();
                    break;
                } else if(!workers[t]->IsRunning()) {
                    ChartData->DeleteCacheChart(workers[t]->pchart);
                    delete workers[t];
                    workers[t] = NULL;
                }
                if(++t == thread_count) {
                    wxThread::Sleep(10); /* wait for a worker to finish */
                    t = 0;
                }
            }
        } else
            CompressChart(pchart, CompressedCacheFilePath, filename, false);
    }
skip:

    /* wait for workers to finish, and clean up after then */
    if(ramonly) {
        for(int t = 0; t<thread_count; t++) {
            if(workers[t]) {
                if(workers[t]->IsRunning())
                    workers[t]->Wait();
                ChartData->DeleteCacheChart(workers[t]->pchart);
                delete workers[t];
            }
        }
        delete [] workers;
    }

    g_GLOptions.m_bTextureCompressionCaching = true;    // reenable

    delete pprog;
}

/* for debugging */
static void print_region(OCPNRegion &Region)
{
    OCPNRegionIterator upd ( Region );
    while ( upd.HaveRects() )
    {
        wxRect rect = upd.GetRect();
        printf("[(%d, %d) (%d, %d)] ", rect.x, rect.y, rect.width, rect.height);
        upd.NextRect();
    }
}

static GLboolean QueryExtension( const char *extName )
{
    /*
     ** Search for extName in the extensions string. Use of strstr()
     ** is not sufficient because extension names can be prefixes of
     ** other extension names. Could use strtok() but the constant
     ** string returned by glGetString might be in read-only memory.
     */
    char *p;
    char *end;
    int extNameLen;

    extNameLen = strlen( extName );

    p = (char *) glGetString( GL_EXTENSIONS );
    if( NULL == p ) {
        return GL_FALSE;
    }

    end = p + strlen( p );

    while( p < end ) {
        int n = strcspn( p, " " );
        if( ( extNameLen == n ) && ( strncmp( extName, p, n ) == 0 ) ) {
            return GL_TRUE;
        }
        p += ( n + 1 );
    }
    return GL_FALSE;
}

typedef void (*GenericFunction)(void);

#if defined(__WXMSW__)
#define systemGetProcAddress(ADDR) wglGetProcAddress(ADDR)
#elif defined(__WXOSX__)
#include <dlfcn.h>
#define systemGetProcAddress(ADDR) dlsym( RTLD_DEFAULT, ADDR)
#else
#define systemGetProcAddress(ADDR) glXGetProcAddress((const GLubyte*)ADDR)
#endif

GenericFunction ocpnGetProcAddress(const char *addr, const char *extension)
{
    char addrbuf[256];
    if(!extension)
        return (GenericFunction)NULL;

    snprintf(addrbuf, sizeof addrbuf, "%s%s", addr, extension);
    return (GenericFunction)systemGetProcAddress(addr);
}

static void GetglEntryPoints( void )
{
    // the following are all part of framebuffer object,
    // according to opengl spec, we cannot mix EXT and ARB extensions
    // (I don't know that it could ever happen, but if it did, bad things would happen)
    const char *extensions[] = {"", "ARB", "EXT", 0 };

    unsigned int i;
    for(i=0; i<(sizeof extensions) / (sizeof *extensions); i++) {
        if((s_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenFramebuffers", extensions[i])))
            break;
    }

    if(i<3){
        s_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenRenderbuffers", extensions[i]);
        s_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
            ocpnGetProcAddress( "glFramebufferTexture2D", extensions[i]);
        s_glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindFramebuffer", extensions[i]);
        s_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glFramebufferRenderbuffer", extensions[i]);
        s_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)
            ocpnGetProcAddress( "glRenderbufferStorage", extensions[i]);
        s_glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindRenderbuffer", extensions[i]);
        s_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
            ocpnGetProcAddress( "glCheckFramebufferStatus", extensions[i]);
        s_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteFramebuffers", extensions[i]);
        s_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteRenderbuffers", extensions[i]);
        s_glGenerateMipmap = (PFNGLGENERATEMIPMAPEXTPROC)
            ocpnGetProcAddress( "glGenerateMipmap", extensions[i]);
    }

    for(i=0; i<(sizeof extensions) / (sizeof *extensions); i++) {
        if((s_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)
            ocpnGetProcAddress( "glCompressedTexImage2D", extensions[i])))
            break;
    }

    if(i<3){
        s_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)
            ocpnGetProcAddress( "glGetCompressedTexImage", extensions[i]);
    }
}

// This attribute set works OK with vesa software only OpenGL renderer
int attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 8, 0 };
BEGIN_EVENT_TABLE ( glChartCanvas, wxGLCanvas ) EVT_PAINT ( glChartCanvas::OnPaint )
    EVT_ACTIVATE ( glChartCanvas::OnActivate )
    EVT_SIZE ( glChartCanvas::OnSize )
    EVT_MOUSE_EVENTS ( glChartCanvas::MouseEvent )
END_EVENT_TABLE()

glChartCanvas::glChartCanvas( wxWindow *parent ) :
    wxGLCanvas( parent, wxID_ANY, wxDefaultPosition, wxSize( 256, 256 ),
                wxFULL_REPAINT_ON_RESIZE | wxBG_STYLE_CUSTOM, _T(""), attribs ),
    m_data( NULL ), m_datasize( 0 ), m_bsetup( false )
{
    SetBackgroundStyle ( wxBG_STYLE_CUSTOM );  // on WXMSW, this prevents flashing
    
    m_cache_current_ch = NULL;

    m_b_paint_enable = true;
    m_in_glpaint = false;

    m_cache_tex[0] = m_cache_tex[1] = 0;
    m_cache_page = 0;

    m_b_BuiltFBO = false;
    m_b_DisableFBO = false;

    ownship_tex = 0;
    ownship_large_scale_display_lists[0] = 0;
    ownship_large_scale_display_lists[1] = 0;
}

glChartCanvas::~glChartCanvas()
{
    free( m_data );

    ClearAllRasterTextures();
}

void glChartCanvas::ClearAllRasterTextures( void )
{
    //     Clear and delete all the GPU textures presently loaded
    ChartPointerHashType::iterator it;
    for( it = m_chart_hash.begin(); it != m_chart_hash.end(); ++it ) {
        ChartBase *pc = (ChartBase *) it->first;

        ChartTextureHashType *pTextureHash = m_chart_hash[pc];

        // iterate over all the textures presently loaded
        // and delete the OpenGL texture and the private descriptor

        ChartTextureHashType::iterator it;
        for( it = pTextureHash->begin(); it != pTextureHash->end(); ++it ) {
            glTextureDescriptor *ptd = it->second;
            glDeleteTextures( 1, &ptd->tex_name );
            delete ptd;
        }

        pTextureHash->clear();
        delete pTextureHash;
    }
    m_chart_hash.clear();
    g_tex_mem_used = 0;
}

void glChartCanvas::OnActivate( wxActivateEvent& event )
{
    cc1->OnActivate( event );
}

void glChartCanvas::OnSize( wxSizeEvent& event )
{
    if( !g_bopengl ) {
        SetSize( cc1->GetVP().pix_width, cc1->GetVP().pix_height );
        event.Skip();
        return;
    }

    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize( event );

    /* expand opengl widget to fill viewport */
    ViewPort &VP = cc1->GetVP();
    if( GetSize().x != VP.pix_width || GetSize().y != VP.pix_height ) {
        SetSize( VP.pix_width, VP.pix_height );
        if( m_bsetup )
            BuildFBO();
    }
}

void glChartCanvas::MouseEvent( wxMouseEvent& event )
{
    cc1->MouseEvent( event );
}

void glChartCanvas::BuildFBO( )
{
    if( m_b_BuiltFBO ) {
        glDeleteTextures( 2, m_cache_tex );
        ( s_glDeleteFramebuffers )( 1, &m_fb0 );
        ( s_glDeleteRenderbuffers )( 1, &m_renderbuffer );
        m_b_BuiltFBO = false;
    }

    if( m_b_DisableFBO)
        return;

    ( s_glGenFramebuffers )( 1, &m_fb0 );
    ( s_glGenRenderbuffers )( 1, &m_renderbuffer );

    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

    // initialize color textures
    glGenTextures( 2, m_cache_tex );
    m_cache_tex_x = GetSize().x;
    m_cache_tex_y = GetSize().y;
    for(int i=0; i<2; i++) {
        glBindTexture( g_texture_rectangle_format, m_cache_tex[i] );
        glTexParameterf( g_texture_rectangle_format, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( g_texture_rectangle_format, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( g_texture_rectangle_format, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, NULL );
    }

    ( s_glBindRenderbuffer )( GL_RENDERBUFFER_EXT, m_renderbuffer );

    if( m_b_useFBOStencil ) {
        // initialize composite depth/stencil renderbuffer
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT,
                                         m_cache_tex_x, m_cache_tex_y );
        
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );

        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
    } else {
        // initialize depth renderbuffer
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                                         m_cache_tex_x, m_cache_tex_y );
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
    }
    
    // Disable Render to FBO
    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

    // Disable Render to FBO
    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

    /* invalidate cache */
    Invalidate();

    m_b_BuiltFBO = true;
}

void glChartCanvas::SetupOpenGL()
{
    char render_string[80];
    strncpy( render_string, (char *) glGetString( GL_RENDERER ), 79 );
    m_renderer = wxString( render_string, wxConvUTF8 );

    wxString msg;
    msg.Printf( _T("OpenGL-> Renderer String: ") );
    msg += m_renderer;
    wxLogMessage( msg );

    if( ps52plib ) ps52plib->SetGLRendererString( m_renderer );

    s_b_useScissorTest = true;
    // the radeon x600 driver has buggy scissor test
    if( GetRendererString().Find( _T("RADEON X600") ) != wxNOT_FOUND )
        s_b_useScissorTest = false;

    //  This little hack fixes a problem seen with some Intel 945 graphics chips
    //  We need to not do anything that requires (some) complicated stencil operations.
    // TODO: arrange this to use stencil, but depth for s52plib and eliminate display list

    bool bad_stencil_code = false;
    if( GetRendererString().Find( _T("Intel") ) != wxNOT_FOUND ) {
        wxLogMessage( _T("OpenGL-> Detected Intel renderer, disabling stencil buffer") );
        bad_stencil_code = true;
        wxLogMessage( _T("OpenGL-> Detected Intel renderer, disabling FBO") );
//        m_b_DisableFBO = true;
    }

    //      And for the lousy Unichrome drivers, too
    if( GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND ) {
        bad_stencil_code = true;
    }

    //      Stencil buffer test
    glEnable( GL_STENCIL_TEST );
    GLboolean stencil = glIsEnabled( GL_STENCIL_TEST );
    int sb;
    glGetIntegerv( GL_STENCIL_BITS, &sb );
    //        printf("Stencil Buffer Available: %d\nStencil bits: %d\n", stencil, sb);
    glDisable( GL_STENCIL_TEST );

    s_b_useStencil = false;
    if( !bad_stencil_code && stencil && ( sb == 8 ) ) s_b_useStencil = true;

    if( QueryExtension( "GL_ARB_texture_non_power_of_two" ) )
        g_texture_rectangle_format = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_OES_texture_npot" ) )
        g_texture_rectangle_format = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_ARB_texture_rectangle" ) )
        g_texture_rectangle_format = GL_TEXTURE_RECTANGLE_ARB;
    wxLogMessage( wxString::Format(_T("OpenGL-> Texture rectangle format: %x"),
                                   g_texture_rectangle_format));

    //      We require certain extensions to support FBO rendering
    if(!g_texture_rectangle_format)
        m_b_DisableFBO = true;
    
    if(!QueryExtension( "GL_EXT_framebuffer_object" ))
        m_b_DisableFBO = true;

    GetglEntryPoints();

    if( !s_glGenFramebuffers  || !s_glGenRenderbuffers        || !s_glFramebufferTexture2D ||
        !s_glBindFramebuffer  || !s_glFramebufferRenderbuffer || !s_glRenderbufferStorage  ||
        !s_glBindRenderbuffer || !s_glCheckFramebufferStatus  || !s_glDeleteFramebuffers   ||
        !s_glDeleteRenderbuffers )
        m_b_DisableFBO = true;

    //      Can we use the stencil buffer in a FBO?
#ifdef ocpnUSE_GLES /* gles requires all levels */
    m_b_useFBOStencil = QueryExtension( "GL_OES_packed_depth_stencil" );
#else
    m_b_useFBOStencil = QueryExtension( "GL_EXT_packed_depth_stencil" );
#endif

    //      Maybe build FBO(s)

    BuildFBO();
#if 0   /* this test sometimes failes when the fbo still works */
    if( m_b_BuiltFBO ) {
        // Check framebuffer completeness at the end of initialization.
        ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );
        GLenum fb_status = ( s_glCheckFramebufferStatus )( GL_FRAMEBUFFER_EXT );
        ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

        if( fb_status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
            wxString msg;
            msg.Printf( _T("    OpenGL-> Framebuffer Incomplete:  %08X"), fb_status );
            wxLogMessage( msg );
            m_b_DisableFBO = true;
            BuildFBO();
        }
    }
#endif

    if( m_b_BuiltFBO && !m_b_useFBOStencil )
        s_b_useStencil = false;

    if( m_b_BuiltFBO ) {
        wxLogMessage( _T("OpenGL-> Using Framebuffer Objects") );

        if( m_b_useFBOStencil && s_b_useStencil)
            wxLogMessage( _T("OpenGL-> Using FBO Stencil buffer") );
        else
            wxLogMessage( _T("OpenGL-> FBO Stencil buffer unavailable") );
    } else
        wxLogMessage( _T("OpenGL-> Framebuffer Objects unavailable") );

    if( s_b_useStencil ) wxLogMessage( _T("OpenGL-> Using Stencil buffer clipping") );
    else
        wxLogMessage( _T("OpenGL-> Using Depth buffer clipping") );

    /* we upload non-aligned memory */
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

#ifdef ocpnUSE_GLES /* gles requires all levels */
    int max_level = 0;
    int tex_dim = g_GLOptions.m_iTextureDimension;
    for(int dim=tex_dim; dim>0; dim/=2)
        max_level++;
    g_mipmap_max_level = max_level - 1;
#endif

    SetupCompression();

    s_b_UploadFullCompressedMipmaps = false;
#ifdef __WXOSX__    
    if( GetRendererString().Find( _T("Intel GMA 950") ) != wxNOT_FOUND )
        s_b_UploadFullCompressedMipmaps = true;
#endif    
    
}

void glChartCanvas::SetupCompression()
{
    int dim = g_GLOptions.m_iTextureDimension;
    int uncompressed_tile_size = dim*dim*3;
    if(g_GLOptions.m_bTextureCompression) {
        /* because s3tc is patented, many foss drivers disable
           support by default, however the extension dxt1 allows
           us to load this texture type which is enough because we
           compress in software using libsquish for superior quality anyway */

        if((QueryExtension("GL_EXT_texture_compression_s3tc") ||
            QueryExtension("GL_EXT_texture_compression_dxt1")) &&
           s_glCompressedTexImage2D) {
            /* buggy opensource nvidia driver, renders incorrectly,
               workaround is to use format with alpha... */
            if(GetRendererString().Find( _T("Gallium") ) != wxNOT_FOUND &&
               GetRendererString().Find( _T("NV") ) != wxNOT_FOUND )
                g_raster_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            else
                g_raster_format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

            wxLogMessage( _("OpenGL-> Using s3tc dxt1 compression") );
        } else if(QueryExtension("GL_3DFX_texture_compression_FXT1") &&
                  s_glCompressedTexImage2D && s_glGetCompressedTexImage) {
            g_raster_format = GL_COMPRESSED_RGB_FXT1_3DFX;

            wxLogMessage( _("OpenGL-> Using 3dfx fxt1 compression") );
#ifdef ocpnUSE_GLES
        } else if(QueryExtension("GL_OES_compressed_ETC1_RGB8_texture") &&
                  s_glCompressedTexImage2D) {
           g_raster_format = GL_ETC1_RGB8_OES;

            wxLogMessage( _("OpenGL-> Using oes etc1 compression") );
#endif
        } else {
            wxLogMessage( _("OpenGL-> No Useable compression format found") );
            goto no_compression;
        }

#ifdef ocpnUSE_GLES /* gles doesn't have GetTexLevelParameter */
        g_tile_size = 512*512/2; /* 4bpp */
#else
        /* determine compressed size of a level 0 single tile */
        GLuint texture;
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexImage2D( GL_TEXTURE_2D, 0, g_raster_format, dim, dim,
                      0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                 GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &g_tile_size);
        glDeleteTextures(1, &texture);
#endif

        /* disable texture compression if the tile size is 0 */
        if(g_tile_size == 0)
            goto no_compression;

        wxLogMessage( wxString::Format( _T("OpenGL-> Compressed tile size: %dkb (%d:1)"),
                                        g_tile_size / 1024,
                                        uncompressed_tile_size / g_tile_size));
    } else
    if(!g_GLOptions.m_bTextureCompression) {
    no_compression:
        g_tile_size = uncompressed_tile_size;
        g_raster_format = GL_RGB;
        wxLogMessage( wxString::Format( _T("OpenGL-> Not Using compression")));
    }
}

void glChartCanvas::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );

#if wxCHECK_VERSION(2, 9, 0)
    SetCurrent(*m_pcontext);
#else
    SetCurrent();
#endif

    Show( g_bopengl );
    if( !g_bopengl ) {
        event.Skip();
        return;
    }

    if( !m_bsetup ) {
        SetupOpenGL();
        m_bsetup = true;
//        g_bDebugOGL = true;
    }

    //  Paint updates may have been externally disabled (temporarily, to avoid Yield() recursion performance loss)
    if(!m_b_paint_enable)
        return;
        
    //      Recursion test, sometimes seen on GTK systems when wxBusyCursor is activated
    if( m_in_glpaint ) return;
    m_in_glpaint++;

    cc1->DoTimedMovement();

    Render();

    m_in_glpaint--;

}

bool glChartCanvas::PurgeChartTextures( ChartBase *pc )
{
    //    Look for the chart texture hashmap in the member chart hashmap
    ChartPointerHashType::iterator it0 = m_chart_hash.find( pc );

    ChartTextureHashType *pTextureHash;

    //    Found ?
    if( it0 != m_chart_hash.end() ) {
        pTextureHash = m_chart_hash[pc];

        // iterate over all the textures presently loaded
        // and delete the OpenGL texture and the private descriptor

        ChartTextureHashType::iterator it;
        for( it = pTextureHash->begin(); it != pTextureHash->end(); ++it ) {
            glTextureDescriptor *ptd = it->second;
            DeleteTexture(ptd, g_mipmap_max_level+1);
            delete ptd;
        }

        pTextureHash->clear();

        m_chart_hash.erase( it0 );            // erase the texture hash map for this chart

        delete pTextureHash;

        return true;
    }
    return false;
}

/* adjust the opengl transformation matrix so that
   points plotted using the identity viewport are correct.
   and all rotation translation and scaling is now done= in opengl */

/*   This is needed for building display lists */
#define NORM_FACTOR 16.0
void glChartCanvas::MultMatrixViewPort(const ViewPort &vp)
{
    wxPoint point;
    cc1->GetCanvasPointPix(0, 0, &point);
    glTranslatef(point.x, point.y, 0);
    glScalef(vp.view_scale_ppm/NORM_FACTOR, vp.view_scale_ppm/NORM_FACTOR, 1);
    double angle = vp.rotation;
//    if(!g_bskew_comp)
//        angle -= vp.skew;

    glRotatef(angle*180/PI, 0, 0, 1);
}

ViewPort glChartCanvas::NormalizedViewPort(const ViewPort &vp)
{
    ViewPort cvp = vp;
    cvp.clat = cvp.clon = 0;
    cvp.view_scale_ppm = NORM_FACTOR;
    cvp.rotation = cvp.skew = 0;
    return cvp;
}

void glChartCanvas::FixRenderIDL(int dl)
{
    //  Does current vp cross international dateline?
    // if so, call the display list again translated
    // to the other side of it..
    ViewPort vp = cc1->GetVP();
    if( vp.GetBBox().GetMinX() < -180. || vp.GetBBox().GetMaxX() > 180. ) {
        double ts = 40058986*NORM_FACTOR; /* 360 degrees in normalized viewport */

        glPushMatrix();
        if( vp.GetBBox().GetMinX() < -180. )
            glTranslated(-ts, 0, 0);
        else
            glTranslated(ts, 0, 0);
        glCallList(dl);
        glPopMatrix();
    }
}

void glChartCanvas::DrawAllRoutesAndWaypoints( ViewPort &vp, OCPNRegion &region )
{
    ocpnDC dc(*this);

    Route *active_route, *m_active_track;
    for(wxRouteListNode *node = pRouteList->GetFirst();
        node; node = node->GetNext() ) {
        Route *pRouteDraw = node->GetData();
        if( !pRouteDraw )
            continue;

        /* defer rendering active routes until later */
        if( pRouteDraw->IsActive() || pRouteDraw->IsSelected() )
            continue;

        if( pRouteDraw->IsTrack() ) {
            /* defer rendering active tracks until later */
            if( dynamic_cast<Track *>(pRouteDraw)->IsRunning() )
                continue;
        }

        /* this routine is called very often, so rather than using the
           wxBoundingBox::Intersect routine, do the comparisons directly
           to reduce the number of floating point comparisons */

        const wxBoundingBox &vp_box = vp.GetBBox(), &test_box = pRouteDraw->GetBBox();

        if(test_box.GetMaxY() < vp_box.GetMinY())
            continue;

        if(test_box.GetMinY() > vp_box.GetMaxY())
            continue;

        double vp_minx = vp_box.GetMinX(), vp_maxx = vp_box.GetMaxX();
        double test_minx = test_box.GetMinX(), test_maxx = test_box.GetMaxX();

        /* TODO: use DrawGL instead of Draw */

        // Route is not wholly outside viewport
        if(test_maxx >= vp_minx && test_minx <= vp_maxx) {
            pRouteDraw->DrawGL( vp, region );
        } else if( vp_maxx > 180. ) {
            if(test_minx + 360 <= vp_maxx && test_maxx + 360 >= vp_minx)
                pRouteDraw->DrawGL( vp, region );
        } else if( pRouteDraw->CrossesIDL() || vp_minx < -180. ) {
            if(test_maxx - 360 >= vp_minx && test_minx - 360 <= vp_maxx)
                pRouteDraw->DrawGL( vp, region );
        }
    }

    /* Waypoints not drawn as part of routes */
    if( vp.GetBBox().GetValid() ) {
        for(wxRoutePointListNode *pnode = pWayPointMan->GetWaypointList()->GetFirst(); pnode; pnode = pnode->GetNext() ) {
            RoutePoint *pWP = pnode->GetData();
            if( pWP && (!pWP->m_bIsInRoute && !pWP->m_bIsInTrack ) )
                pWP->DrawGL( vp, region );
        }
    }
    
}

void glChartCanvas::RenderChartOutline( int dbIndex, ViewPort &vp )
{
    /* quick bounds check */
    wxBoundingBox box, vpbox = vp.GetBBox();
    ChartData->GetDBBoundingBox( dbIndex, &box );

    float lon_bias;
    if( vpbox.IntersectOut( box ) ) {
        wxPoint2DDouble p = wxPoint2DDouble(360, 0);
        box.Translate( p );
        if( vpbox.IntersectOut( box ) ) {
            wxPoint2DDouble n = wxPoint2DDouble(-720, 0);
            box.Translate( n );
            if( vpbox.IntersectOut( box ) )
                return;
            lon_bias = -360;
        } else
            lon_bias = 360;
    } else
        lon_bias = 0;

    float plylat, plylon;

    wxColour color;
    switch( ChartData->GetDBChartType( dbIndex ) ) {
    case CHART_TYPE_S57:  color = GetGlobalColor( _T ( "UINFG" ) ); break;
    case CHART_TYPE_CM93: color = GetGlobalColor( _T ( "YELO1" ) ); break;
    default:              color = GetGlobalColor( _T ( "UINFR" ) ); break;
    }

    ChartTableEntry *entry = ChartData->GetpChartTableEntry(dbIndex);

    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glColor3ub(color.Red(), color.Green(), color.Blue());
    glLineWidth(1.3);

    //        Are there any aux ply entries?
    int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex ), nPly;
    int j=0;
    do {
        if(nAuxPlyEntries)
            nPly = ChartData->GetDBAuxPlyPoint( dbIndex, 0, j, 0, 0 );
        else
            nPly = ChartData->GetDBPlyPoint( dbIndex, 0, &plylat, &plylon );

        glBegin(GL_LINE_STRIP);
        for( int i = 0; i < nPly+1; i++ ) {
            if(nAuxPlyEntries)
                ChartData->GetDBAuxPlyPoint( dbIndex, i%nPly, j, &plylat, &plylon );
            else
                ChartData->GetDBPlyPoint( dbIndex, i%nPly, &plylat, &plylon );

            plylon += lon_bias;

            wxPoint r;
            cc1->GetCanvasPointPix( plylat, plylon, &r );
            glVertex2f( r.x + .5, r.y + .5 );
        }
        glEnd();
    } while(++j < nAuxPlyEntries );                 // There are no aux Ply Point entries
}

extern void CalcGridSpacing( float WindowDegrees, float& MajorSpacing, float&MinorSpacing );
extern void CalcGridText( float latlon, float spacing, bool bPostfix, char *text );
void glChartCanvas::GridDraw( )
{
    if( !g_bDisplayGrid ) return;

    bool b_rotated = fabs( cc1->GetVP().rotation ) > 1e-5 ||
        ( fabs( cc1->GetVP().skew ) < 1e-9 && !g_bskew_comp );

    double nlat, elon, slat, wlon;
    float lat, lon;
    float dlat, dlon;
    float gridlatMajor, gridlatMinor, gridlonMajor, gridlonMinor;
    wxCoord w, h;
    
    wxColour GridColor = GetGlobalColor( _T ( "SNDG1" ) );        

    static TexFont s_texfont;
    wxFont *font = wxTheFontList->FindOrCreateFont
        ( 8, wxFONTFAMILY_SWISS, wxNORMAL,
          wxFONTWEIGHT_NORMAL, FALSE, wxString( _T ( "Arial" ) ) );
    s_texfont.Build(*font);

    w = cc1->m_canvas_width;
    h = cc1->m_canvas_height;

    LLBBox llbbox = cc1->GetVP().GetBBox();
    nlat = llbbox.GetMaxY();
    slat = llbbox.GetMinY();
    elon = llbbox.GetMaxX();
    wlon = llbbox.GetMinX();

    /* base spacing off unexpanded viewport, so when rotating about a location
       the grid does not change. */
    double rotation = cc1->GetVP().rotation;
    cc1->GetVP().rotation = 0;

    double latp[2], lonp[2];
    cc1->GetCanvasPixPoint( 0, 0, latp[0], lonp[0] );
    cc1->GetCanvasPixPoint( w, h, latp[1], lonp[1] );
    cc1->GetVP().rotation = rotation;

    dlat = latp[0] - latp[1]; // calculate how many degrees of latitude are shown in the window
    dlon = lonp[1] - lonp[0]; // calculate how many degrees of longitude are shown in the window
    if( dlon < 0.0 ) // concider datum border at 180 degrees longitude
    {
        dlon = dlon + 360.0;
    }

    // calculate distance between latitude grid lines
    CalcGridSpacing( dlat, gridlatMajor, gridlatMinor );

    // calculate distance between grid lines
    CalcGridSpacing( dlon, gridlonMajor, gridlonMinor );

    // Draw Major latitude grid lines and text
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );

    glColor3ub(GridColor.Red(), GridColor.Green(), GridColor.Blue());

    // Render in two passes, lines then text is much more efficient for opengl
    for( int pass=0; pass<2; pass++ ) {
        if(pass == 0) {
            glLineWidth(1);
            glBegin(GL_LINES);
        }

        // calculate position of first major latitude grid line
        lat = ceil( slat / gridlatMajor ) * gridlatMajor;

        while( lat < nlat ) {
            wxPoint r, s;
            cc1->GetCanvasPointPix( lat, elon, &r );
            cc1->GetCanvasPointPix( lat, wlon, &s );
            if(pass == 0) {
                glVertex2i(r.x, r.y);
                glVertex2i(s.x, s.y);
            } else {
                char sbuf[12];
                CalcGridText( lat, gridlatMajor, true, sbuf ); // get text for grid line

                float x = 0, y = -1;
                y = (float)(r.y*s.x - s.y*r.x) / (s.x - r.x);
                if(y < 0 || y > h) {
                    int iy;
                    s_texfont.GetTextExtent(sbuf, strlen(sbuf), 0, &iy);
                    y = h - iy;
                    x = (float)(r.x*s.y - s.x*r.y + (s.x - r.x)*y) / (s.y - r.y);
                }

                glEnable(GL_TEXTURE_2D);
                s_texfont.RenderString(sbuf, x, y);
                glDisable(GL_TEXTURE_2D);
            }
            
            lat = lat + gridlatMajor;
            if( fabs( lat - wxRound( lat ) ) < 1e-5 ) lat = wxRound( lat );
        }

        if(pass == 0 && !b_rotated) {
            // calculate position of first minor latitude grid line
            lat = ceil( slat / gridlatMinor ) * gridlatMinor;
        
            // Draw minor latitude grid lines
            while( lat < nlat ) {
                wxPoint r;
                cc1->GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
                glVertex2i(0, r.y);
                glVertex2i(10, r.y);
                glVertex2i(w - 10, r.y);
                glVertex2i(w, r.y);
                lat = lat + gridlatMinor;
            }
        }

        // calculate position of first major latitude grid line
        lon = ceil( wlon / gridlonMajor ) * gridlonMajor;
        
        // draw major longitude grid lines
        for( int i = 0, itermax = (int) ( dlon / gridlonMajor ); i <= itermax; i++ ) {
            wxPoint r, s;
            cc1->GetCanvasPointPix( nlat, lon, &r );
            cc1->GetCanvasPointPix( slat, lon, &s );
            if(pass == 0) {
                glVertex2i(r.x, r.y);
                glVertex2i(s.x, s.y);
            } else {
                char sbuf[12];
                CalcGridText( lon, gridlonMajor, false, sbuf );

                float x = -1, y = 0;
                x = (float)(r.x*s.y - s.x*r.y) / (s.y - r.y);
                if(x < 0 || x > w) {
                    int ix;
                    s_texfont.GetTextExtent(sbuf, strlen(sbuf), &ix, 0);
                    x = w - ix;
                    y = (float)(r.y*s.x - s.y*r.x + (s.y - r.y)*x) / (s.x - r.x);
                }

                glEnable(GL_TEXTURE_2D);
                s_texfont.RenderString(sbuf, x, y);
                glDisable(GL_TEXTURE_2D);
            }

            lon = lon + gridlonMajor;
            if( lon > 180.0 )
                lon = lon - 360.0;

            if( fabs( lon - wxRound( lon ) ) < 1e-5 ) lon = wxRound( lon );
        }

        if(pass == 0 && !b_rotated) {
            // calculate position of first minor longitude grid line
            lon = ceil( wlon / gridlonMinor ) * gridlonMinor;
            // draw minor longitude grid lines
            for( int i = 0, itermax = (int) ( dlon / gridlonMinor ); i <= itermax; i++ ) {
                wxPoint r;
                cc1->GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
                glVertex2i(r.x, 0);
                glVertex2i(r.x, 10);
                glVertex2i(r.x, h-10);
                glVertex2i(r.x, h);
                lon = lon + gridlonMinor;
                if( lon > 180.0 ) {
                    lon = lon - 360.0;
                }
            }
        }

        if(pass == 0)
            glEnd();
    }
}

void glChartCanvas::DrawEmboss( emboss_data *emboss  )
{
    if( !emboss ) return;
    
    int w = emboss->width, h = emboss->height;
    
    glEnable( GL_TEXTURE_2D );
    
    // render using opengl and alpha blending
    if( !emboss->gltexind ) { /* upload to texture */

        emboss->glwidth = NextPow2(emboss->width);
        emboss->glheight = NextPow2(emboss->height);
                
        /* convert to luminance alpha map */
        int size = emboss->glwidth * emboss->glheight;
        char *data = new char[2 * size];
        for( int i = 0; i < h; i++ ) {
            for( int j = 0; j < emboss->glwidth; j++ ) {
                if( j < w ) {
                    data[2 * ( ( i * emboss->glwidth ) + j )] =
                        emboss->pmap[( i * w ) + j] > 0 ? 0 : 255;
                    data[2 * ( ( i * emboss->glwidth ) + j ) + 1] = abs(
                        emboss->pmap[( i * w ) + j] );
                }
            }
        }

        glGenTextures( 1, &emboss->gltexind );
        glBindTexture( GL_TEXTURE_2D, emboss->gltexind );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, emboss->glwidth, emboss->glheight, 0,
                      GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        
        delete[] data;
    }
    
    glBindTexture( GL_TEXTURE_2D, emboss->gltexind );
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
    
    const float factor = 200;
    glColor4f( 1, 1, 1, factor / 256 );
    
    int x = emboss->x, y = emboss->y;

    float wp = (float) w / emboss->glwidth;
    float hp = (float) h / emboss->glheight;
    
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 ), glVertex2i( x, y );
    glTexCoord2f( wp, 0 ), glVertex2i( x + w, y );
    glTexCoord2f( wp, hp ), glVertex2i( x + w, y + h );
    glTexCoord2f( 0, hp ), glVertex2i( x, y + h );
    glEnd();
    
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
}

void glChartCanvas::ShipDraw(ocpnDC& dc)
{
    if( !cc1->GetVP().IsValid() ) return;
    wxPoint lGPSPoint, lShipMidPoint, lPredPoint, lHeadPoint, GPSOffsetPixels(0,0);

    double pred_lat, pred_lon;

    //  COG/SOG may be undefined in NMEA data stream
    float pCog = gCog;
    if( wxIsNaN(pCog) )
        pCog = 0.0;
    float pSog = gSog;
    if( wxIsNaN(pSog) )
        pSog = 0.0;

    ll_gc_ll( gLat, gLon, pCog, pSog * g_ownship_predictor_minutes / 60., &pred_lat, &pred_lon );

    cc1->GetCanvasPointPix( gLat, gLon, &lGPSPoint );
    lShipMidPoint = lGPSPoint;
    cc1->GetCanvasPointPix( pred_lat, pred_lon, &lPredPoint );

    float cog_rad = atan2f( (float) ( lPredPoint.y - lShipMidPoint.y ),
                            (float) ( lPredPoint.x - lShipMidPoint.x ) );
    cog_rad += PI;

    float lpp = sqrtf( powf( (float) (lPredPoint.x - lShipMidPoint.x), 2) +
                       powf( (float) (lPredPoint.y - lShipMidPoint.y), 2) );

    //  Draw the icon rotated to the COG
    //  or to the Hdt if available
    float icon_hdt = pCog;
    if( !wxIsNaN( gHdt ) ) icon_hdt = gHdt;

    //  COG may be undefined in NMEA data stream
    if( wxIsNaN(icon_hdt) ) icon_hdt = 0.0;

//    Calculate the ownship drawing angle icon_rad using an assumed 10 minute predictor
    double osd_head_lat, osd_head_lon;
    wxPoint osd_head_point;

    ll_gc_ll( gLat, gLon, icon_hdt, pSog * 10. / 60., &osd_head_lat, &osd_head_lon );

    cc1->GetCanvasPointPix( gLat, gLon, &lShipMidPoint );
    cc1->GetCanvasPointPix( osd_head_lat, osd_head_lon, &osd_head_point );

    float icon_rad = atan2( (float) ( osd_head_point.y - lShipMidPoint.y ),
                            (float) ( osd_head_point.x - lShipMidPoint.x ) );
    icon_rad += PI;

    if( pSog < 0.2 ) icon_rad = ( ( icon_hdt + 90. ) * PI / 180. ) + cc1->GetVP().rotation;

//    Calculate ownship Heading pointer as a predictor
    double hdg_pred_lat, hdg_pred_lon;

    ll_gc_ll( gLat, gLon, icon_hdt, pSog * g_ownship_predictor_minutes / 60., &hdg_pred_lat,
              &hdg_pred_lon );

    cc1->GetCanvasPointPix( gLat, gLon, &lShipMidPoint );
    cc1->GetCanvasPointPix( hdg_pred_lat, hdg_pred_lon, &lHeadPoint );

//    Should we draw the Head vector?
//    Compare the points lHeadPoint and lPredPoint
//    If they differ by more than n pixels, and the head vector is valid, then render the head vector

    float ndelta_pix = 10.;
    bool b_render_hdt = false;
    if( !wxIsNaN( gHdt ) ) {
        float dist = sqrtf( powf( (float) (lHeadPoint.x - lPredPoint.x), 2) +
                            powf( (float) (lHeadPoint.y - lPredPoint.y), 2) );
        if( dist > ndelta_pix && !wxIsNaN(gSog) )
            b_render_hdt = true;
    }

    int img_height;

    if( cc1->GetVP().chart_scale > 300000 )             // According to S52, this should be 50,000
    {
        /* this test must match that in ShipDrawLargeScale */
        int list = cc1->m_ownship_state == SHIP_NORMAL;

        glPushMatrix();
        glTranslatef(lShipMidPoint.x, lShipMidPoint.y, 0);
            
        if(ownship_large_scale_display_lists[list])
            glCallList(ownship_large_scale_display_lists[list]);
        else {
            ownship_large_scale_display_lists[list] = glGenLists(1);
            glNewList(ownship_large_scale_display_lists[list], GL_COMPILE_AND_EXECUTE);
            lShipMidPoint.x = lShipMidPoint.y = 0;
                
            cc1->ShipDrawLargeScale(dc, lShipMidPoint);
            glEndList();
        }
        glPopMatrix();
        img_height = 20; /* is this needed? */
    } else {
        if(!ownship_tex) { /* initial run, create texture for ownship,
                              also needed at colorscheme changes (not implemented) */
            glGenTextures( 1, &ownship_tex );
            glBindTexture(GL_TEXTURE_2D, ownship_tex);

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

            wxImage &image = cc1->m_pos_image_user ? *cc1->m_pos_image_user : *cc1->m_pos_image_grey;
            
            int w = image.GetWidth(), h = image.GetHeight();
            int glw = NextPow2(w), glh = NextPow2(h);
            ownship_size = wxSize(w, h);
            ownship_tex_size = wxSize(glw, glh);
            
            unsigned char *d = image.GetData();
            unsigned char *a = image.GetAlpha();
            unsigned char *e = new unsigned char[4 * w * h];
            for( int p = 0; p < w*h; p++ ) {
                e[4*p+0] = d[3*p+0];
                e[4*p+1] = d[3*p+1];
                e[4*p+2] = d[3*p+2];
                e[4*p+3] = a[p];
            }
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                         glw, glh, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                            w, h, GL_RGBA, GL_UNSIGNED_BYTE, e);
            delete [] e;
        }

        /* establish ship color */
        if( cc1->m_pos_image_user )
            glColor4ub(255, 255, 255, 255);
        else if( SHIP_NORMAL == cc1->m_ownship_state )
            glColor4ub(255, 0, 0, 255);
        else if( SHIP_LOWACCURACY == cc1->m_ownship_state )
            glColor4ub(255, 255, 0, 255);
        else
            glColor4ub(128, 128, 128, 255);

        /* scaled ship? */
        float scale_factor_y = 1, scale_factor_x = 1;
        int ownShipWidth = 22; // Default values from s_ownship_icon
        int ownShipLength= 84;

        if( g_n_ownship_beam_meters > 0.0 &&
            g_n_ownship_length_meters > 0.0 &&
            g_OwnShipIconType > 0 )
        {            
            if( g_OwnShipIconType == 1 ) {
                ownShipWidth = ownship_size.x;
                ownShipLength= ownship_size.y;
            }

            cc1->ComputeShipScaleFactor
                (icon_hdt, ownShipWidth, ownShipLength, lShipMidPoint,
                 GPSOffsetPixels, lGPSPoint, scale_factor_x, scale_factor_y);
        }

        glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_HINT_BIT);
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_POLYGON_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

        glEnable(GL_BLEND);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            
        int x = lShipMidPoint.x, y = lShipMidPoint.y;
        glPushMatrix();
        glTranslatef(x, y, 0);

        float deg = 180/PI * ( icon_rad - PI/2 );
        glRotatef(deg, 0, 0, 1);

        glScalef(scale_factor_x, scale_factor_y, 1);

        if( g_OwnShipIconType < 2 ) { // Bitmap

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, ownship_tex);
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
                        
            float glw = ownship_tex_size.x, glh = ownship_tex_size.y;
            float u = ownship_size.x/glw, v = ownship_size.y/glh;
            float w = ownship_size.x, h = ownship_size.y;
            
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0), glVertex2f(-w/2, -h/2);
            glTexCoord2f(u, 0), glVertex2f(+w/2, -h/2);
            glTexCoord2f(u, v), glVertex2f(+w/2, +h/2);
            glTexCoord2f(0, v), glVertex2f(-w/2, +h/2);
            glEnd();

            glDisable(GL_TEXTURE_2D);
        } else if( g_OwnShipIconType == 2 ) { // Scaled Vector

            static const int s_ownship_icon[] = { 5, -42, 11, -28, 11, 42, -11, 42,
                                                  -11, -28, -5, -42, -11, 0, 11, 0,
                                                  0, 42, 0, -42       };
            glBegin(GL_POLYGON);
            for( int i = 0; i < 12; i+=2 )
                glVertex2f(s_ownship_icon[i], s_ownship_icon[i + 1] );
            glEnd();
            
            glColor3ub(0, 0, 0);
            glLineWidth(1);

            glBegin(GL_LINE_LOOP);
            for(int i=0; i<12; i+=2)
                glVertex2f( s_ownship_icon[i], s_ownship_icon[i+1] );
            glEnd();

            // draw reference point (midships) cross
            glBegin(GL_LINES);
            for(int i=12; i<20; i+=2)
                glVertex2f( s_ownship_icon[i], s_ownship_icon[i+1] );
            glEnd();
        }
        glPopMatrix();

        img_height = ownShipLength * scale_factor_y;
        
        //      Reference point, where the GPS antenna is
        int circle_rad = 3;
        if( cc1->m_pos_image_user ) circle_rad = 1;
               
        float cx = lGPSPoint.x, cy = lGPSPoint.y;
        float r = circle_rad+1;
        glColor3ub(0, 0, 0);
        glBegin(GL_POLYGON);
        for( float a = 0; a <= 2 * PI; a += 2 * PI / 12 )
            glVertex2f( cx + r * sinf( a ), cy + r * cosf( a ) );
        glEnd();

        r = circle_rad;
        glColor3ub(255, 255, 255);
        
        glBegin(GL_POLYGON);
        for( float a = 0; a <= 2 * M_PI; a += 2 * M_PI / 12 )
            glVertex2f( cx + r * sinf( a ), cy + r * cosf( a ) );
        glEnd();       
        glPopAttrib();            // restore state
    }

    cc1->ShipIndicatorsDraw(dc, lpp,  GPSOffsetPixels,
                            lGPSPoint,  lHeadPoint,
                            img_height, cog_rad,
                            lPredPoint,  b_render_hdt, lShipMidPoint);
}

void glChartCanvas::DrawFloatingOverlayObjects( ocpnDC &dc, OCPNRegion &region )
{
    ViewPort &vp = cc1->GetVP();

    //  Draw any active or selected routes now
    extern Routeman                  *g_pRouteMan;
    extern Track                     *g_pActiveTrack;
    Route *active_route = g_pRouteMan->GetpActiveRoute();

    if( active_route ) active_route->DrawGL( vp, region );
    if( g_pActiveTrack ) g_pActiveTrack->Draw( dc, vp );
    if( cc1->m_pSelectedRoute ) cc1->m_pSelectedRoute->DrawGL( vp, region );

    GridDraw( );

    if( g_pi_manager ) {
        g_pi_manager->SendViewPortToRequestingPlugIns( vp );
        g_pi_manager->RenderAllGLCanvasOverlayPlugIns( GetContext(), vp );
    }

    // all functions called with cc1-> are still slow because they go through ocpndc
    cc1->AISDrawAreaNotices( dc );
    DrawEmboss(cc1->EmbossDepthScale() );
    DrawEmboss(cc1->EmbossOverzoomIndicator( dc ) );

    cc1->DrawAnchorWatchPoints( dc );
    cc1->AISDraw( dc );
    ShipDraw( dc );
    cc1->AlertDraw( dc );

    cc1->RenderRouteLegs( dc );
    cc1->ScaleBarDraw( dc );
#ifdef USE_S57
    s57_DrawExtendedLightSectors( dc, cc1->VPoint, cc1->extendedSectorLegs );
#endif

    DisableClipRegion();

    /* This should be converted to opengl, it is currently caching screen
       outside render, so the viewport can change without updating, (incorrect)
       doing alpha blending in software with it and draw pixels (very slow) */
    if( cc1->m_pRouteRolloverWin && cc1->m_pRouteRolloverWin->IsActive() ) {
        dc.DrawBitmap( *(cc1->m_pRouteRolloverWin->GetBitmap()),
                       cc1->m_pRouteRolloverWin->GetPosition().x,
                       cc1->m_pRouteRolloverWin->GetPosition().y, false );
    }

    if( cc1->m_pAISRolloverWin && cc1->m_pAISRolloverWin->IsActive() ) {
        dc.DrawBitmap( *(cc1->m_pAISRolloverWin->GetBitmap()),
                       cc1->m_pAISRolloverWin->GetPosition().x,
                       cc1->m_pAISRolloverWin->GetPosition().y, false );
    }
}

void glChartCanvas::DrawQuiting()
{
    GLubyte pattern[4 * 32];
    for( int y = 0; y < 32; y++ ) {
        GLubyte mask = 1 << y % 8;
        for( int x = 0; x < 4; x++ )
            pattern[y * 4 + x] = mask;
    }
    
    glEnable( GL_POLYGON_STIPPLE );
    glPolygonStipple( pattern );
    glBegin( GL_QUADS );
    glColor3f( 0, 0, 0 );
    glVertex2i( 0, 0 );
    glVertex2i( 0, GetSize().y );
    glVertex2i( GetSize().x, GetSize().y );
    glVertex2i( GetSize().x, 0 );
    glEnd();
    glDisable( GL_POLYGON_STIPPLE );
}

void glChartCanvas::GrowData( int size )
{
    /* grow the temporary ram buffer used to load charts into textures */
    if( size > m_datasize ) {
        unsigned char* tmp = (unsigned char*) realloc( m_data, m_datasize );
        if( tmp != NULL ) {
            m_data = tmp;
            m_datasize = size;
            tmp = NULL;
        }
    }
}

void glChartCanvas::RotateToViewPort(const ViewPort &vp)
{
    float angle = vp.rotation;
    if(g_bskew_comp)
        angle -= vp.skew;

    if( fabs( angle ) > 0.0001 )
    {
        //    Rotations occur around 0,0, so translate to rotate around screen center
        float xt = vp.pix_width / 2, yt = vp.pix_height / 2;
        
        glTranslatef( xt, yt, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -xt, -yt, 0 );
    }
}

/* set stencil buffer to clip in this region, and optionally clear using the current color */
void glChartCanvas::SetClipRegion(const ViewPort &vp, const OCPNRegion &region,
                                  bool apply_rotation, bool b_clear )
{
    bool rotation = fabs( vp.rotation ) > 0.0001 || ( g_bskew_comp && fabs( vp.skew ) > 0.0001);

#if 1 /* optimization: use scissor test or no test at all if one is not needed */
    /* for some reason this causes an occasional bug in depth mode, I cannot
       seem to solve it yet, so for now: */
    if(!rotation && s_b_useStencil && s_b_useScissorTest) {
        int n_rect = 0;
        for(OCPNRegionIterator clipit( region ); clipit.HaveRects() && n_rect < 2; clipit.NextRect())
            n_rect++;

        if(n_rect == 1) {
            wxRect rect = OCPNRegionIterator( region ).GetRect();
            if(rect ==  vp.rv_rect) {
                /* no actual clipping need be done, common case */
            } else {
                glEnable(GL_SCISSOR_TEST);
                glScissor(rect.x, vp.rv_rect.height-rect.height-rect.y, rect.width, rect.height);
            }

            if(b_clear) { /* can glClear work in scissors instead? */
                glBegin( GL_QUADS );
                glVertex2i( rect.x, rect.y );
                glVertex2i( rect.x + rect.width, rect.y );
                glVertex2i( rect.x + rect.width, rect.y + rect.height );
                glVertex2i( rect.x, rect.y + rect.height );
                glEnd();
            }

            /* the code in s52plib depends on the depth buffer being
               initialized to this value, this code should go there instead and
               only a flag set here. */
            if(!s_b_useStencil) {
                glClearDepth( 0.25 );
                glDepthMask( GL_TRUE );    // to allow writes to the depth buffer
                glClear( GL_DEPTH_BUFFER_BIT );
                glDepthMask( GL_FALSE );
                glClearDepth( 1 ); // set back to default of 1
                glDepthFunc( GL_GREATER );                          // Set the test value
            }
            return;
        }
    }
#endif
    //    As a convenience, while we are creating the stencil or depth mask,
    //    also clear the background if selected
    if( !b_clear )
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

    if( s_b_useStencil ) {
        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
    } else              //  Use depth buffer for clipping
    {
        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer

        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start

        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
            // dep buffer clear = 1
            // 1 makes 0 in dep buffer, works
            // 0 make .5 in depth buffer
            // -1 makes 1 in dep buffer

            //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
            //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
            //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
        glTranslatef( 0, 0, .5 );
    }

    if(rotation && apply_rotation) {
        glPushMatrix();
        glChartCanvas::RotateToViewPort( vp );
    }

    //    Decompose the region into rectangles, and draw as quads
    OCPNRegionIterator clipit( region );
    while( clipit.HaveRects() ) {
        wxRect rect = clipit.GetRect();
        
        glBegin( GL_QUADS );
        glVertex2i( rect.x, rect.y );
        glVertex2i( rect.x + rect.width, rect.y );
        glVertex2i( rect.x + rect.width, rect.y + rect.height );
        glVertex2i( rect.x, rect.y + rect.height );
        glEnd();
        
        clipit.NextRect();
    }

    if(rotation && apply_rotation)
        glPopMatrix();
    
    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }

    if(!b_clear)
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
}

void glChartCanvas::DisableClipRegion()
{
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_DEPTH_TEST );
}

void glChartCanvas::Invalidate()
{
    /* should probably use a different flag for this */
    cc1->m_glcc->m_cache_vp.Invalidate();
}

void glChartCanvas::RenderRasterChartRegionGL( ChartBase *chart, ViewPort &vp, OCPNRegion &region )
{
    if( !chart ) return;

    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( chart );
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );

    if( !pPlugInWrapper && !pBSBChart ) return;

    bool b_plugin = false;
    if( pPlugInWrapper ) b_plugin = true;
 
    /* setup texture parameters */
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    //  Make a special VP to account for rotations
    if( vp.b_MercatorProjectionOverride ) vp.SetProjectionType( PROJECTION_MERCATOR );
    ViewPort svp = vp;

    svp.pix_width = svp.rv_rect.width;
    svp.pix_height = svp.rv_rect.height;

    wxRealPoint Rp, Rs;
    double scalefactor;
    int size_X, size_Y;
    if( b_plugin ) {
        /* TODO: plugins need floating point version */
        wxRect R;
        pPlugInWrapper->ComputeSourceRectangle( svp, &R );
        Rp.x = R.x, Rp.y = R.y, Rs.x = R.width, Rs.y = R.height;

        scalefactor = pPlugInWrapper->GetRasterScaleFactor();
        size_X = pPlugInWrapper->GetSize_X();
        size_Y = pPlugInWrapper->GetSize_Y();
    } else {
        pBSBChart->ComputeSourceRectangle( svp, &Rp, &Rs );
        scalefactor = pBSBChart->GetRasterScaleFactor();
        size_X = pBSBChart->GetSize_X();
        size_Y = pBSBChart->GetSize_Y();
    }

    int tex_dim = g_GLOptions.m_iTextureDimension;
    GrowData( 3 * tex_dim * tex_dim );

    /* clipping is relative to rv_rect */
    OCPNRegion clipregion(region);
    clipregion.Offset(vp.rv_rect.x, vp.rv_rect.y);
    SetClipRegion( vp, clipregion );

    //    Look for the chart texture hashmap in the member chart hashmap
    ChartPointerHashType::iterator it = m_chart_hash.find( chart );

    ChartTextureHashType *pTextureHash;

    //    Not Found ?
    if( it == m_chart_hash.end() ) {
        ChartTextureHashType *p = new ChartTextureHashType;
        m_chart_hash[chart] = p;
    }

    pTextureHash = m_chart_hash[chart];

    //    For underzoom cases, we will define the textures as having their base levels
    //    equivalent to a level "n" mipmap, where n is calculated, and is always binary
    //    This way we can avoid loading much texture memory
    int base_level = log(scalefactor) / log(2.0);
    if(base_level < 0) /* for overzoom */
        base_level = 0;
    if(base_level > g_mipmap_max_level)
        base_level = g_mipmap_max_level;

    wxRect R(floor(Rp.x), floor(Rp.y), ceil(Rs.x), ceil(Rs.y));

    //  Calculate the number of textures needed
    int nx_tex = ( size_X / tex_dim ) + 1;
    int ny_tex = ( size_Y / tex_dim ) + 1;

    glTextureDescriptor *ptd;
    wxRect rect( 0, 0, 1, 1 );

    glPushMatrix();

    glScalef( 1. / scalefactor, 1. / scalefactor, 1 );

    double xt = 0.;
    double yt = 0.;

    double angle = vp.rotation;
    if(g_bskew_comp)
        angle -= vp.skew;

    if( angle != 0 ) /* test not really needed, but maybe a little faster for north up? */
    {
        //    Shift texture drawing positions to account for the larger chart rectangle
        //    needed to cover the screen on rotated images
        double w = vp.pix_width;
        double h = vp.pix_height;

        double ddx = scalefactor * w / 2;
        double ddy = scalefactor * h / 2;

        xt = Rs.x/2.0 - ddx;
        yt = Rs.y/2.0 - ddy;

        glTranslatef( ddx, ddy, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -ddx, -ddy, 0 );
    }

    //    Using a 2D loop, iterate thru the texture tiles of the chart
    //    For each tile, is it (1) needed and (2) present?

    int key;

    rect.y = 0;
    for( int i = 0; i < ny_tex; i++ ) {
        rect.height = tex_dim;
        rect.x = 0;
        for( int j = 0; j < nx_tex; j++ ) {
            rect.width = tex_dim;

            //    Is this texture tile already defined?
            key = ( ( rect.x << 18 ) + ( rect.y << 4 ) );
            ChartTextureHashType::iterator it = pTextureHash->find( key );
            
            // compute position, end, and size
            wxRealPoint rip(wxMax(Rp.x, rect.x), wxMax(Rp.y, rect.y));
            wxRealPoint rie(wxMin(Rp.x+Rs.x, rect.x+rect.width), wxMin(Rp.y+Rs.y, rect.y+rect.height));
            wxRealPoint ris(rie.x - rip.x, rie.y - rip.y);

            //   Does this tile intersect the chart source rectangle?
            if( ris.x <= 0 || ris.y <= 0 ) {
                /*   user setting is in MB while we count exact bytes */
                bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;
                if( it != pTextureHash->end() && bGLMemCrunch) {
                    ptd = ( *pTextureHash )[key];
                    /* delete this unneeded tile if we need to free up memory */
                    DeleteTexture(ptd, g_mipmap_max_level+1);
                }
            } else {
                // calculate the on-screen rectangle coordinates for this tile
                double w = ris.x, h = ris.y;
                double x1 = rip.x - rect.x;
                double y1 = rip.y - rect.y;

                double x2 = ( rip.x - Rp.x ) - xt;
                double y2 = ( rip.y - Rp.y ) - yt;

                wxRect rt( floor( x2 / scalefactor),
                           floor( y2 / scalefactor),
                           ceil(w / scalefactor),
                           ceil(h / scalefactor));
                rt.Offset( -vp.rv_rect.x, -vp.rv_rect.y ); // compensate for the adjustment made in quilt composition

                // if not found in the hash map, then get the bits as a texture descriptor
                if( it == pTextureHash->end() )
                    ( *pTextureHash )[key] = new glTextureDescriptor;
                
                ptd = ( *pTextureHash )[key];

                //    And does this tile intersect the desired render region?
                if( region.Contains( rt ) == wxOutRegion ) {
                    /*   user setting is in MB while we count exact bytes */
                    bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;

                    /* delete this unneeded tile if we need to free up memory */
                    if( bGLMemCrunch )
                        DeleteTexture(ptd, g_mipmap_max_level+1);
                } else { // this tile is needed

                    //    The texture is known to be available to the GPU
                    UploadTexture( ptd, base_level, rect, chart ); 

                    /*   user setting is in MB while we count exact bytes,
                         recompute here because we may free some of the
                         mipmaps just created.    It could be useful to also
                         delete unused mipmaps from previous tiles in this frame... */
                    bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;

#if 0
                    /* delete unneeded mipmap levels when this tile is used */
                    if( bGLMemCrunch)
                        DeleteTexture(ptd, base_level);
#endif

                    double sx = rect.width;
                    double sy = rect.height;

                    glBegin( GL_QUADS );

                    glTexCoord2f( x1 / sx, y1 / sy );
                    glVertex2f( ( x2 ), ( y2 ) );
                    glTexCoord2f( ( x1 + w ) / sx, y1 / sy );
                    glVertex2f( ( w + x2 ), ( y2 ) );
                    glTexCoord2f( ( x1 + w ) / sx, ( y1 + h ) / sy );
                    glVertex2f( ( w + x2 ), ( h + y2 ) );
                    glTexCoord2f( x1 / sx, ( y1 + h ) / sy );
                    glVertex2f( ( x2 ), ( h + y2 ) );

                    glEnd();
                }
            }
            rect.x += rect.width;
        }

        rect.y += rect.height;
    }

    if( g_bDebugOGL ) {
        wxString msg;
        msg.Printf(_T("Timings: p:%ld m:%ld hwm:%ld u:%ld\tuc:%ld dc:%ld dcc: %ld rc:%ld wc:%ld   base:%d"),
                   populate_tt_total, mipmap_tt_total, hwmipmap_tt_total, upload_tt_total,
                   uploadcomp_tt_total, downloadcomp_tt_total, decompcomp_tt_total, readcomp_tt_total, writecomp_tt_total,
                   base_level);
            wxLogMessage(msg);

            printf("%s\n", (const char*)msg.ToUTF8());
            
            printf("texmem used: %.0fMB\n", g_tex_mem_used / 1024.0 / 1024.0);
    }

    glPopMatrix();

    glDisable( GL_TEXTURE_2D );

    DisableClipRegion();
}

void glChartCanvas::RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &Region, bool b_clear )
{
    if( cc1->m_pQuilt->GetnCharts() && !cc1->m_pQuilt->IsBusy() ) {
#if 0 /* clearing is probably faster than doing this calculation */
        //  Walk the region list to determine whether we need a clear before starting
        if( b_clear ) {
            OCPNRegion clear_test_region = Region;
            
            ChartBase *cchart = cc1->m_pQuilt->GetFirstChart();
            while( cchart ) {
                if( ! cc1->IsChartLargeEnoughToRender( cchart, vp ) ) {
                    cchart = cc1->m_pQuilt->GetNextChart();
                    continue;
                }

                QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
                if( pqp->b_Valid ) {
                    OCPNRegion get_region = pqp->ActiveRegion;

                    if( !get_region.IsEmpty() )
                        clear_test_region.Subtract( get_region );
                }
                cchart = cc1->m_pQuilt->GetNextChart();
            }

        //  We only need a screen clear if the test region is non-empty
            if( !clear_test_region.IsEmpty() ) {
                /* It is been reported on gallium drivers that this exposes
                   a bug so should not be used, in any case, we should not
                   really need a clear, and should fix the quilt logic so
                   it reports the rendered region correctly in all cases */
                wxColour clearcolor = GetGlobalColor ( _T ( "NODTA" ) );
                glClearColor(clearcolor.Red()/255.0,
                             clearcolor.Green()/255.0,
                             clearcolor.Blue()/255.0, 0);
                glClear( GL_COLOR_BUFFER_BIT );
            }
        }
#endif

        //  Now render the quilt
        ChartBase *chart = cc1->m_pQuilt->GetFirstChart();

        while( chart ) {
            if( ! cc1->IsChartLargeEnoughToRender( chart, vp ) ) {
                chart = cc1->m_pQuilt->GetNextChart();
                continue;
            }

            QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid ) {
                OCPNRegion get_region = pqp->ActiveRegion;
                get_region.Intersect( Region );

                bool b_rendered = false;

                if( !get_region.IsEmpty() ) {
                    if( !pqp->b_overlay ) {
                        ChartBaseBSB *Patch_Ch_BSB = dynamic_cast<ChartBaseBSB*>( chart );
                        if( Patch_Ch_BSB ) {
                            RenderRasterChartRegionGL( chart, cc1->VPoint, get_region );
                            b_rendered = true;
                        } else {
                            ChartPlugInWrapper *Patch_Ch_Plugin =
                                dynamic_cast<ChartPlugInWrapper*>( chart );
                            if( Patch_Ch_Plugin ) {
                                if( Patch_Ch_Plugin->GetChartFamily() == CHART_FAMILY_RASTER ) {
                                    RenderRasterChartRegionGL( chart, cc1->VPoint, get_region );
                                    b_rendered = true;
                                }
                            }
                        }

                        if( !b_rendered ) {
                            if( chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                                OCPNRegion rr = get_region;
                                rr.Offset( vp.rv_rect.x, vp.rv_rect.y );
                                b_rendered = chart->RenderRegionViewOnGL( *m_pcontext, cc1->VPoint, rr );
                            }
                        }
                    }
                }

                if(b_rendered) {
                    OCPNRegion get_region = pqp->ActiveRegion;
//                    get_region.Intersect( Region );  not technically required?
                    m_gl_rendered_region.Union(get_region);
                }
            }


            chart = cc1->m_pQuilt->GetNextChart();
        }

        //    Render any Overlay patches for s57 charts(cells)
        if( cc1->m_pQuilt->HasOverlays() ) {
            ChartBase *pch = cc1->m_pQuilt->GetFirstChart();
            while( pch ) {
                QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
                if( pqp->b_Valid ) {
                    OCPNRegion get_region = pqp->ActiveRegion;

                    get_region.Intersect( Region );
                    if( !get_region.IsEmpty() ) {
                        if( pqp->b_overlay ) {
                            if( pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                                s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                                if( pch ) {
                                    get_region.Offset( cc1->VPoint.rv_rect.x,
                                                       cc1->VPoint.rv_rect.y );
                                    Chs57->RenderOverlayRegionViewOnGL( *m_pcontext, cc1->VPoint,
                                                                        get_region );
                                }
                            }
                        }
                    }
                }

                pch = cc1->m_pQuilt->GetNextChart();
            }
        }

        // Hilite rollover patch
        OCPNRegion hiregion = cc1->m_pQuilt->GetHiliteRegion( vp );

        if( !hiregion.IsEmpty() ) {
            glPushAttrib( GL_COLOR_BUFFER_BIT );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

            double hitrans;
            switch( global_color_scheme ) {
            case GLOBAL_COLOR_SCHEME_DAY:
                hitrans = .4;
                break;
            case GLOBAL_COLOR_SCHEME_DUSK:
                hitrans = .2;
                break;
            case GLOBAL_COLOR_SCHEME_NIGHT:
                hitrans = .1;
                break;
            default:
                hitrans = .4;
                break;
            }

            glColor4f( (float) .8, (float) .4, (float) .4, (float) hitrans );

            OCPNRegionIterator upd ( hiregion );
            while ( upd.HaveRects() )
            {
                wxRect rect = upd.GetRect();

                glBegin( GL_QUADS );
                glVertex2i( rect.x, rect.y );
                glVertex2i( rect.x + rect.width, rect.y );
                glVertex2i( rect.x + rect.width, rect.y + rect.height );
                glVertex2i( rect.x, rect.y + rect.height );
                glEnd();

                upd.NextRect();
            }

            glDisable( GL_BLEND );
            glPopAttrib();
        }
        cc1->m_pQuilt->SetRenderedVP( vp );

        }
    else if( !cc1->m_pQuilt->GetnCharts() && b_clear ) {
//        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void glChartCanvas::RenderCharts(ocpnDC &dc, OCPNRegion &region)
{
    ViewPort VPoint = cc1->VPoint;
    m_gl_rendered_region.Clear();

    glPushMatrix();
    if(VPoint.b_quilt) {
        RenderQuiltViewGL( VPoint, region );
        if(m_gl_rendered_region.IsOk())
            m_gl_rendered_region.Offset(VPoint.rv_rect.x, VPoint.rv_rect.y);
    } else {
        ChartBaseBSB *Current_Ch_BSB = dynamic_cast<ChartBaseBSB*>( Current_Ch );
        if( Current_Ch_BSB ) {
            RenderRasterChartRegionGL( Current_Ch, VPoint, region );
        } else {
            ChartPlugInWrapper *Current_Ch_PlugInWrapper =
                dynamic_cast<ChartPlugInWrapper*>( Current_Ch );
            if( Current_Ch_PlugInWrapper ) {
                if( Current_Ch_PlugInWrapper->GetChartFamily() == CHART_FAMILY_RASTER ) {
                    RenderRasterChartRegionGL( Current_Ch, VPoint, region );
                }
            } else
                if( !dynamic_cast<ChartDummy*>( Current_Ch ) ) {
                    OCPNRegion rr = region;
                    if( Current_Ch->GetChartFamily() == CHART_FAMILY_VECTOR )
                        rr.Offset( VPoint.rv_rect.x, VPoint.rv_rect.y );
                    Current_Ch->RenderRegionViewOnGL( *m_pcontext, VPoint, rr );
                }
        }
        Current_Ch->GetValidCanvasRegion ( VPoint, &m_gl_rendered_region );
    }
    glPopMatrix();

    const int max_rect = 2;
    int n_rect = 0;
    for(OCPNRegionIterator clipit( region ); clipit.HaveRects() && n_rect<=max_rect; clipit.NextRect())
        n_rect++;

    if (n_rect > max_rect) {  // I don't expect this, and have never seen it
        wxLogMessage(wxString::Format(_T("warning: grounded nrect count: %d\n"), n_rect));
        region = OCPNRegion(region.GetBox()); /* flatten region to rectangle  */
    }

    /* now put in screen coords */
    region.Offset(VPoint.rv_rect.x, VPoint.rv_rect.y);

    OCPNRegion backgroundRegion(region);

    //    Remove the valid chart area
    //    Draw the World Chart only in the areas NOT covered by the charts
    if(!m_gl_rendered_region.IsEmpty())
        backgroundRegion.Subtract(m_gl_rendered_region);

    if( !backgroundRegion.IsEmpty() )
        RenderWorldChart(dc, backgroundRegion);

    /* render in each rectangle, the grounded overlay objects */
    for(OCPNRegionIterator upd( region ); upd.HaveRects(); upd.NextRect()) {
        wxRect rect = upd.GetRect();
        DrawGroundedOverlayObjectsRect(dc, rect);
    }
}

/* render world chart, but only in this rectangle */
void glChartCanvas::RenderWorldChart(ocpnDC &dc, OCPNRegion &region)
{
    ViewPort vp = cc1->VPoint;
  
    wxColour water = cc1->pWorldBackgroundChart->water;
    
    /* we are not going to benefit from multiple passes
       if we are zoomed in far enough to only have a few cells, or there are
       many rectangles */
    int n_rect = 0;
    for(OCPNRegionIterator clipit( region ); clipit.HaveRects(); clipit.NextRect())
        n_rect++;

    if(vp.view_scale_ppm > .003 || n_rect > 2)
    {
        glColor3ub(water.Red(), water.Green(), water.Blue());
        SetClipRegion( vp, region, true, true ); /* clear background, no rotation */

        vp.rv_rect.x = vp.rv_rect.y = 0;
        cc1->pWorldBackgroundChart->RenderViewOnDC( dc, vp );
    } else /* if there are (skinny) rectangles (common for panning)
              we can accelerate quite a bit here by doing two rendering passes
              with correct lat/lon bounding boxes for each, to eliminate
              most of the data to walk. */
    for(OCPNRegionIterator upd( region ); upd.HaveRects(); upd.NextRect())
    {
        wxRect rect = upd.GetRect();
        glColor3ub(water.Red(), water.Green(), water.Blue());
        SetClipRegion( vp, OCPNRegion(rect), true, true);
        ViewPort cvp = BuildClippedVP(vp, rect);
        cvp.rv_rect.x = cvp.rv_rect.y = 0;
        cc1->pWorldBackgroundChart->RenderViewOnDC( dc, cvp );
    }

    DisableClipRegion( );
}

void glChartCanvas::DeleteChartTextures(ChartBaseBSB *pc)
{
    ChartTextureHashType *pTextureHash = m_chart_hash[pc];
    
    // iterate over all the textures presently loaded
    // and delete the OpenGL texture from the GPU
    // but keep the private texture descriptor for now
    
    for(ChartTextureHashType::iterator it = pTextureHash->begin(), prev = it;
        it != pTextureHash->end(); prev = it++ )
        DeleteTexture(it->second, g_mipmap_max_level+1);
}

ViewPort glChartCanvas::BuildClippedVP(ViewPort &VP, wxRect &rect)
{
    //  Build synthetic ViewPort on this rectangle so that
    // it has a bounding box with lat and lon
    //  Especially, we want the BBox to be accurate in order to
    //  render only those objects actually visible in this region

    ViewPort temp_vp = VP;

    double lat1, lat2, lon2, lon1;

    /* clipping rectangles are not rotated */
    if(temp_vp.rotation)
        cc1->SetVPRotation(0);

    cc1->GetCanvasPixPoint( rect.x, rect.y + rect.height, lat1, lon1);
    cc1->GetCanvasPixPoint( rect.x + rect.width, rect.y, lat2, lon2);
    if( lon2 < lon1 )        // IDL fix
        lon2 += 360.;

    if(temp_vp.rotation)
        cc1->SetVPRotation(temp_vp.rotation);

    while( lon1 >= 180 ) {
        lon1 -= 360;
        lon2 -= 360;
    }
    
    while( lon2 <= -180 ) {
        lon1 += 360;
            lon2 += 360;
    }
        
    temp_vp.GetBBox().SetMin( lon1, lat1 );
    temp_vp.GetBBox().SetMax( lon2, lat2 );

    return temp_vp;
}

/* these are the overlay objects which move with the charts and
   are not frequently updated (not ships etc..) 

   many overlay objects are fixed to a geographical location or
   grounded as opposed to the floating overlay objects. */
void glChartCanvas::DrawGroundedOverlayObjectsRect(ocpnDC &dc, wxRect &rect)
{
    OCPNRegion region(rect);

    /* only draw in this rectangle */
    SetClipRegion( cc1->GetVP(), region);

   /* to allow each overlay item to use it's hash table and only fetch in
      these coordinates, construct a bounding box constrained to this rect */
    ViewPort temp_vp = BuildClippedVP(cc1->GetVP(), rect);
    cc1->RenderAllChartOutlines( dc, temp_vp );

    DrawAllRoutesAndWaypoints( temp_vp, region );

    if( cc1->m_bShowTide )
        cc1->DrawAllTidesInBBox( dc, temp_vp.GetBBox(), true, true );
    
    if( cc1->m_bShowCurrent )
        cc1->DrawAllCurrentsInBBox( dc, temp_vp.GetBBox(), true, true );

    DisableClipRegion();
}

void glChartCanvas::Render()
{
    if( !m_bsetup ) return;

    if( ( !cc1->VPoint.b_quilt ) && ( !Current_Ch ) ) return;

    m_last_render_time = wxDateTime::Now().GetTicks();

    wxPaintDC( this );

    ViewPort VPoint = cc1->VPoint;
    ViewPort svp = VPoint;
    svp.pix_width = svp.rv_rect.width;
    svp.pix_height = svp.rv_rect.height;

    OCPNRegion chart_get_region( 0, 0, cc1->VPoint.rv_rect.width, cc1->VPoint.rv_rect.height );

    ocpnDC gldc( *this );

    int w, h;
    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );

    glLoadIdentity();
    gluOrtho2D( 0, (GLint) w, (GLint) h, 0 );

    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    //  Delete any textures known to the GPU that
    //  belong to charts which will not be used in this render
    //  This is done chart-by-chart...later we will scrub for unused textures
    //  that belong to charts which ARE used in this render, if we need to....

    ChartPointerHashType::iterator it0;
    for( it0 = m_chart_hash.begin(); it0 != m_chart_hash.end(); ++it0 ) {
        ChartBaseBSB *pc = (ChartBaseBSB *) it0->first;

        bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;
        if(!bGLMemCrunch)
            break;
        
        if( VPoint.b_quilt )          // quilted
        {
            if( cc1->m_pQuilt && cc1->m_pQuilt->IsComposed() &&
                !cc1->m_pQuilt->IsChartInQuilt( pc ) )
                DeleteChartTextures(pc);
        }
        else      // not quilted
        {
            if( Current_Ch != pc )
                DeleteChartTextures(pc);
        }
    }

    if( VPoint.b_quilt && cc1->m_pQuilt && !cc1->m_pQuilt->IsComposed() ) return;

    // Try to use the framebuffer object's cache of the last frame
    // to accelerate drawing this frame (if overlapping)
    if( m_b_BuiltFBO ) {
        int sx = GetSize().x;
        int sy = GetSize().y;

        //  Is this viewpoint the same as the previously painted one?
        bool b_newview = true;

        // If the view is the same we do no updates, 
        // cached texture to the framebuffe
        if(    m_cache_vp.view_scale_ppm == VPoint.view_scale_ppm
               && m_cache_vp.rotation == VPoint.rotation
               && m_cache_vp.clat == VPoint.clat
               && m_cache_vp.clon == VPoint.clon
               && m_cache_vp.IsValid()
               && m_cache_current_ch == Current_Ch ) {
            b_newview = false;
        }

        if( b_newview ) {
            // enable rendering to texture in framebuffer object
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

            wxPoint c_old = VPoint.GetPixFromLL( VPoint.clat, VPoint.clon );
            wxPoint c_new = m_cache_vp.GetPixFromLL( VPoint.clat, VPoint.clon );

            int dy = c_new.y - c_old.y;
            int dx = c_new.x - c_old.x;

            bool rotation = fabs( VPoint.rotation ) != 0.0 ||
                (g_bskew_comp && fabs( VPoint.skew ) != 0.0 );

            // do we allow accelerated panning?  can we perform it here?
            if(g_GLOptions.m_bUseAcceleratedPanning &&
               (!VPoint.b_quilt ||
                cc1->m_pQuilt->IsVPBlittable( VPoint, dx, dy, true )) &&
               // only works for mercator without rotation
               VPoint.m_projection_type == PROJECTION_MERCATOR && !rotation &&
               m_cache_vp.IsValid() &&
               // there must be some overlap
               abs(dx) < m_cache_tex_x && abs(dy) < m_cache_tex_y ) {

                m_cache_page = !m_cache_page; /* page flip */

                /* perform accelerated pan rendering to the new framebuffer */
                ( s_glFramebufferTexture2D )
                    ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                      g_texture_rectangle_format, m_cache_tex[m_cache_page], 0 );

                /* using the old framebuffer */
                glBindTexture( g_texture_rectangle_format, m_cache_tex[!m_cache_page] );

                glEnable( g_texture_rectangle_format );
                glDisable( GL_BLEND );
                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
                            
                //    Render the reuseable portion of the cached texture
                            
                // Render the cached texture as quad to FBO(m_blit_tex) with offsets
                int x1, x2, y1, y2;

                wxASSERT(sx == m_cache_tex_x);
                wxASSERT(sy == m_cache_tex_y);
                int ow = sx - abs( dx );
                int oh = sy - abs( dy );
                if( dx > 0 )
                    x1 = dx,  x2 = 0;
                else
                    x1 = 0,   x2 = -dx;
                        
                if( dy > 0 )
                    y1 = dy,  y2 = 0;
                else
                    y1 = 0,   y2 = -dy;

                // normalize to texture coordinates range from 0 to 1
                float tx1 = x1, tx2 = x1 + ow, ty1 = sy - y1, ty2 = y2;
                if( GL_TEXTURE_RECTANGLE_ARB != g_texture_rectangle_format )
                    tx1 /= sx, tx2 /= sx, ty1 /= sy, ty2 /= sy;

                glBegin( GL_QUADS );
                glTexCoord2f( tx1, ty1 );  glVertex2f( x2, y2 );
                glTexCoord2f( tx2, ty1 );  glVertex2f( x2 + ow, y2 );
                glTexCoord2f( tx2, ty2 );  glVertex2f( x2 + ow, y2 + oh );
                glTexCoord2f( tx1, ty2 );  glVertex2f( x2, y2 + oh );
                glEnd();

                //   Done with cached texture "blit"
                glDisable( g_texture_rectangle_format );

                //calculate the new regions to render
                // add an extra pixel avoid coorindate rounding issues
                OCPNRegion update_region;

                if( dy > 0 && dy < VPoint.pix_height)
                    update_region.Union
                        (wxRect( 0, VPoint.pix_height - dy, VPoint.pix_width, dy ) );
                else if(dy < 0)
                    update_region.Union( wxRect( 0, 0, VPoint.pix_width, -dy ) );
                        
                if( dx > 0 && dx < VPoint.pix_width )
                    update_region.Union
                        (wxRect( VPoint.pix_width - dx, 0, dx, VPoint.pix_height ) );
                else if (dx < 0)
                    update_region.Union( wxRect( 0, 0, -dx, VPoint.pix_height ) );

                RenderCharts(gldc, update_region);
            } else { // must redraw the entire screen
                ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                              g_texture_rectangle_format,
                                              m_cache_tex[m_cache_page], 0 );
                RenderCharts(gldc, chart_get_region);
            } 
            // Disable Render to FBO
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );
        } // newview

        // Render the cached texture as quad to screen
        glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
        glEnable( g_texture_rectangle_format );
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

        float tx, ty;
        if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
            tx = sx, ty = sy;
        else
            tx = 1, ty = 1;

        glBegin( GL_QUADS );
            glTexCoord2f( 0, ty ); glVertex2f( 0,  0 );
            glTexCoord2f( tx,ty ); glVertex2f( sx, 0 );
            glTexCoord2f( tx, 0 ); glVertex2f( sx, sy );
            glTexCoord2f( 0,  0 ); glVertex2f( 0,  sy );
        glEnd();

        glDisable( g_texture_rectangle_format );
            
        m_cache_vp = VPoint;
        m_cache_current_ch = Current_Ch;

        if(VPoint.b_quilt)
            cc1->m_pQuilt->SetRenderedVP( VPoint );
    } else          // useFBO
        RenderCharts(gldc, chart_get_region);

    // Now draw all the objects which normally move around and are not
    // cached from the previous frame
    DrawFloatingOverlayObjects( gldc, chart_get_region );

    //  On some platforms, the opengl context window is always on top of any standard DC windows,
    //  so we need to draw the Chart Info Window and the Thumbnail as overlayed bmps.

#ifdef __WXOSX__    
    if(cc1->m_pCIWin && cc1->m_pCIWin->IsShown()) {
        int x, y, width, height;
        cc1->m_pCIWin->GetClientSize( &width, &height );
        cc1->m_pCIWin->GetPosition( &x, &y );
        wxBitmap bmp(width, height, -1);
        wxMemoryDC dc(bmp);
        if(bmp.IsOk()){
            dc.SetBackground( wxBrush(GetGlobalColor( _T ( "UIBCK" ) ) ));
            dc.Clear();
 
            dc.SetTextBackground( GetGlobalColor( _T ( "UIBCK" ) ) );
            dc.SetTextForeground( GetGlobalColor( _T ( "UITX1" ) ) );
            
            int yt = 0;
            int xt = 0;
            wxString s = cc1->m_pCIWin->GetString();
            int h = cc1->m_pCIWin->GetCharHeight();
            
            wxStringTokenizer tkz( s, _T("\n") );
            wxString token;
            
            while(tkz.HasMoreTokens()) {
                token = tkz.GetNextToken();
                dc.DrawText(token, xt, yt);
                yt += h;
            }
            dc.SelectObject(wxNullBitmap);
            
            gldc.DrawBitmap( bmp, x, y, false);
        }
    }

    if( pthumbwin && pthumbwin->IsShown()) {
        int thumbx, thumby;
        pthumbwin->GetPosition( &thumbx, &thumby );
        if( pthumbwin->GetBitmap().IsOk())
            gldc.DrawBitmap( pthumbwin->GetBitmap(), thumbx, thumby, false);
    }
    
#endif

    //quiting?
    if( g_bquiting ) DrawQuiting();

    SwapBuffers();

    // glFinish();   Should not be needed, and forces cpu to block which is slow

    cc1->PaintCleanup();
}
