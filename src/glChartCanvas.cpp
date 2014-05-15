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
#include "TexFont.h"
#include "cutil.h"

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

extern PlugInManager* g_pi_manager;
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
PFNGLCOMPRESSEDTEXIMAGE2DPROC       s_glCompressedTexImage2D;
PFNGLGETCOMPRESSEDTEXIMAGEPROC      s_glGetCompressedTexImage;

#ifdef __WXMSW__
HINSTANCE s_hGL_DLL;                   // Handle to DLL
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( ArrayOfTexDescriptors );

#ifdef __WXMSW__
#define FORMAT_INTERNAL       GL_RGB
#define FORMAT_BITS           GL_BGR
#else
#define FORMAT_INTERNAL       GL_COMPRESSED_RGB
#define FORMAT_BITS           GL_RGB
#endif

bool glChartCanvas::s_b_useScissorTest;
bool glChartCanvas::s_b_useStencil;
static int s_nquickbind;

static bool UploadTexture( glTextureDescriptor *ptd, int n_basemult )
{
    if( g_bDebugOGL ) {
        wxString msg;
        msg.Printf( _T("  -->UploadTexture %d"), ptd->tex_name );
        wxLogMessage( msg );
    }

    glBindTexture( GL_TEXTURE_2D, ptd->tex_name );

    //    Calculate the effective base level
    int base_level = 0;
    switch( n_basemult ) {
    case 1:
        base_level = 0;
        break;
    case 2:
        base_level = 1;
        break;
    case 4:
        base_level = 2;
        break;
    case 8:
        base_level = 3;
        break;
    default:
        base_level = 0;
        break;
    }

    int width = ptd->base_size;
    int height = ptd->base_size;

    int level = 0;
    while( level < ptd->level_max ) {
        if( height == 2 ) break;                  // all done;

        if( g_bDebugOGL ) {
            wxString msg;
            msg.Printf( _T("     -->glTexImage2D...level:%d"), level );
            wxLogMessage( msg );
        }

        //    Upload to GPU?
        if( level >= base_level ) glTexImage2D( GL_TEXTURE_2D, level - base_level, FORMAT_INTERNAL,
                                                    width, height, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );

        width /= 2;
        height /= 2;

        level++;
    }

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, level - base_level - 1 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level - base_level - 1 );

    ptd->GPU_base = n_basemult;

    return true;
}

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

static void OCPNPopulateTD( glTextureDescriptor *ptd, int n_basemult, wxRect &rect, ChartBase *pchart )
{
    if( !pchart ) return;

    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( pchart );
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );

    if( !pPlugInWrapper && !pBSBChart ) return;

    bool b_plugin = false;
    if( pPlugInWrapper ) b_plugin = true;

    //    We do not need all possible mipmaps, since we can only zoom out so far....
    //    So, save some memory by limiting GL_TEXTURE_MAX_LEVEL

    int n_level_max = 3;
    //    Calculate the effective base level
    int base_level = 0;
    switch( n_basemult ) {
    case 1:
        base_level = 0;
        break;
    case 2:
        base_level = 1;
        break;
    case 4:
        base_level = 2;
        break;
    case 8:
        base_level = 3;
        break;
    default:
        base_level = 0;
        break;
    }

    //    Adjust the chart source rectangle to account for base multiplier
    wxRect rbits = rect;
    rbits.x *= n_basemult;
    rbits.y *= n_basemult;
    rbits.width *= n_basemult;
    rbits.height *= n_basemult;

    //    Get a buffer
    unsigned char *t_buf = (unsigned char *) malloc( rbits.width * rbits.height * 3 );

    //    Prime the pump with the "zero" level bits, ie. 1x native chart bits
    if( b_plugin ) pPlugInWrapper->GetChartBits( rbits, t_buf, 1 );
    else
        pBSBChart->GetChartBits( rbits, t_buf, 1 );

    //    and cache them here
    ptd->map_array[0] = t_buf;

    int last_height = rbits.height;
    int last_width = rbits.width;

    unsigned char *source;
    unsigned char *dest;

    int level_index = 1;
    while( level_index < n_level_max ) {
        if( last_height == 2 ) break;                  // all done;

        int newwidth = last_width / 2;
        int newheight = last_height / 2;

        dest = (unsigned char *) malloc( newwidth * newheight * 3 );
        source = ptd->map_array[level_index - 1];

        HalfScaleChartBits( last_width, last_height, source, dest );
        ptd->map_array[level_index] = dest;

        last_width = newwidth;
        last_height = newheight;

        level_index++;
    }

    ptd->level_min = 0;
    ptd->level_max = level_index - 1;
    ptd->base_size = rbits.width;
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
    snprintf(addrbuf, sizeof addrbuf, "%s%s", addr, extension);
    return (GenericFunction)systemGetProcAddress(addr);
}

static void GetglEntryPoints( void )
{
    // the following are all part of framebuffer object,
    // according to opengl spec, we cannot mix EXT and ARB extensions
    // (I don't know that it could ever happen, but if it did, bad things would happen)
    const char *extensions[] = {"", "ARB", "EXT" };

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
    EVT_ERASE_BACKGROUND(glChartCanvas::OnEraseBG)
END_EVENT_TABLE()

glChartCanvas::glChartCanvas( wxWindow *parent ) :
    wxGLCanvas( parent, wxID_ANY, wxDefaultPosition, wxSize( 256, 256 ),
                wxFULL_REPAINT_ON_RESIZE | wxBG_STYLE_CUSTOM, _T(""), attribs ), m_cacheinvalid(
                    1 ), m_data( NULL ), m_datasize( 0 ), m_bsetup( false )
{
    m_ntex = 0;
    m_b_paint_enable = true;

    m_b_BuiltFBO = false;
    m_b_DisableFBO = true; // disable until we merge the new fbo code

    ownship_tex = 0;
    ownship_large_scale_display_lists[0] = 0;
    ownship_large_scale_display_lists[1] = 0;
}

glChartCanvas::~glChartCanvas()
{
    free( m_data );

    ClearAllRasterTextures();
}

void glChartCanvas::OnEraseBG( wxEraseEvent& evt )
{
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
        if( m_bsetup && !m_b_BuiltFBO && !m_b_DisableFBO) {
            BuildFBO();
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );
        }

    }

}

void glChartCanvas::MouseEvent( wxMouseEvent& event )
{
    cc1->MouseEvent( event );
}

void glChartCanvas::BuildFBO( )
{
    if( m_bsetup && m_b_BuiltFBO ) {
        glDeleteTextures( 1, &m_cache_tex );
        ( s_glDeleteFramebuffers )( 1, &m_fb0 );
        ( s_glDeleteRenderbuffers )( 1, &m_depth_rb );
    }

    if( m_b_DisableFBO)
        return;

    m_cache_tex_x = GetSize().x;
    m_cache_tex_y = GetSize().y;

    ( s_glGenFramebuffers )( 1, &m_fb0 );
    glGenTextures( 1, &m_cache_tex );
    ( s_glGenRenderbuffers )( 1, &m_depth_rb );

    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );
    
    // initialize color texture
    
    glBindTexture( m_TEX_TYPE, m_cache_tex );
    glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, NULL );
    ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE,
                                  m_cache_tex, 0 );
    
    if( m_b_useFBOStencil ) {
        // initialize composite depth/stencil renderbuffer
        ( s_glBindRenderbuffer )( GL_RENDERBUFFER_EXT, m_depth_rb );
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT,
                                     m_cache_tex_x, m_cache_tex_y );
        
        // Can we attach to depth and stencil at once?  Maybe
        // it would be easier to not check for this extension and
        // always use 2 calls.
        if( QueryExtension( "GL_ARB_framebuffer_object" ) ) {
            ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT,
                                             GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, m_depth_rb );
        } else {
            ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_depth_rb );
            
            ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_depth_rb );
        }

    } else {
        // initialize depth renderbuffer
        ( s_glBindRenderbuffer )( GL_RENDERBUFFER_EXT, m_depth_rb );
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                                     m_cache_tex_x, m_cache_tex_y );
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                         GL_RENDERBUFFER_EXT, m_depth_rb );
    }
    
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
        m_b_DisableFBO = true;
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
}

int s_in_glpaint;

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
//          g_bDebugOGL = true;
    }

    //  Paint updates may have been externally disabled (temporarily, to avoid Yield() recursion performance loss)
    if(!m_b_paint_enable)
        return;
        
    //      Recursion test, sometimes seen on GTK systems when wxBusyCursor is activated
    if( s_in_glpaint ) return;
    s_in_glpaint++;

    cc1->DoTimedMovement();

    Render();

    s_in_glpaint--;

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

            if( ptd->tex_name > 0 ) {
                if( g_bDebugOGL ) printf( "glDeleteTextures in Purge...()\n" );
                glDeleteTextures( 1, &ptd->tex_name );
                m_ntex--;
            }

            delete ptd;
        }

        pTextureHash->clear();

        m_chart_hash.erase( it0 );            // erase the texture hash map for this chart

        delete pTextureHash;

        return true;
    } else
        return false;
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

void glChartCanvas::DrawFloatingOverlayObjects( ocpnDC &dc )
{
    ViewPort &vp = cc1->GetVP();

    //  Draw any active or selected routes now
    extern Routeman                  *g_pRouteMan;
    extern Track                     *g_pActiveTrack;
    Route *active_route = g_pRouteMan->GetpActiveRoute();

//    if( active_route ) active_route->DrawGL( vp );
//    if( g_pActiveTrack ) g_pActiveTrack->Draw( dc, vp );
//    if( cc1->m_pSelectedRoute ) cc1->m_pSelectedRoute->DrawGL( vp );

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

void glChartCanvas::RenderRasterChartRegionGL( ChartBase *chart, ViewPort &vp, OCPNRegion &region )
{
    if( !chart ) return;

    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( chart );
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );

    if( !pPlugInWrapper && !pBSBChart ) return;

    bool b_plugin = false;
    if( pPlugInWrapper ) b_plugin = true;

    int n_longbind = 0;

    /* setup texture parameters */
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    //  Make a special VP to account for rotations
    if( vp.b_MercatorProjectionOverride ) vp.SetProjectionType( PROJECTION_MERCATOR );
    ViewPort svp = vp;

    svp.pix_width = svp.rv_rect.width;
    svp.pix_height = svp.rv_rect.height;

    wxRect R;
    double scalefactor;
    int size_X, size_Y;
    if( b_plugin ) {
        pPlugInWrapper->ComputeSourceRectangle( svp, &R );
        scalefactor = pPlugInWrapper->GetRasterScaleFactor();
        size_X = pPlugInWrapper->GetSize_X();
        size_Y = pPlugInWrapper->GetSize_Y();
    } else {
        pBSBChart->ComputeSourceRectangle( svp, &R );
        scalefactor = pBSBChart->GetRasterScaleFactor();
        size_X = pBSBChart->GetSize_X();
        size_Y = pBSBChart->GetSize_Y();
    }

    int tex_dim = 512;                  //max_texture_dimension;
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

    int n_basemult = 1;
    if( scalefactor >= 2.0 ) {
        n_basemult = 2;
    }

    //    Iterate on the texture hashmap....
    //    Remove any textures whose tex_mult value does not match the target for this render (i.e. n_basemult)
    if( m_ntex > m_tex_max_res ) {
        ChartTextureHashType::iterator itt = pTextureHash->begin();
        while( itt != pTextureHash->end() ) {
            glTextureDescriptor *ptd = itt->second;

            if( ( ptd->tex_name > 0 ) && ( ptd->tex_mult != n_basemult ) ) // the texture known to the GPU does not match the target
            {
                if( g_bDebugOGL ) printf( "   glDeleteTexture on n_basemult mismatch\n" );
                glDeleteTextures( 1, &ptd->tex_name );
                m_ntex--;

                ptd->tex_name = 0;            // mark the ptd as unknown/unavailable to GPU

                //    Delete the chart data?
                if( m_b_mem_crunch ) {
                    pTextureHash->erase( itt );
                    delete ptd;
                    itt = pTextureHash->begin();              // reset the iterator
                }
            } else
                ++itt;
        }
    }

    //    Adjust the chart source rectangle by base multiplier
    R.x /= n_basemult;
    R.y /= n_basemult;
    R.width /= n_basemult;
    R.height /= n_basemult;

    //  Calculate the number of textures needed
    int nx_tex = ( ( size_X / n_basemult ) / tex_dim ) + 1;
    int ny_tex = ( ( size_Y / n_basemult ) / tex_dim ) + 1;

    glTextureDescriptor *ptd;
    wxRect rect( 0, 0, 1, 1 );

    //    For low memory systems, aggressively manage the textures in the GPU memory.
    //    Strategy:  Before loading any new textures,
    //               delete all textures from  the GPU that are not to be used
    //               in this particular VP render, on a per-chart basis.
    if( m_ntex > m_tex_max_res ) {
        rect.y = 0;
        for( int i = 0; i < ny_tex; i++ ) {
            rect.height = tex_dim;
            rect.x = 0;
            for( int j = 0; j < nx_tex; j++ ) {
                rect.width = tex_dim;

                wxASSERT(rect.x < 15383);
                wxASSERT(rect.y < 15383);

                int key = ( ( rect.x << 18 ) + ( rect.y << 4 ) ) + n_basemult;

                if( pTextureHash->find( key ) != pTextureHash->end() )  // found?
                {
                    //    Is this texture needed now?
                    wxRect ri = rect;
                    ri.Intersect( R );
                    if( !ri.width || !ri.height ) {
                        ptd = ( *pTextureHash )[key];
                        if( ptd->tex_name > 0 ) {
                            if( g_bDebugOGL ) printf( "   glDeleteTexture on m_ntex limit\n" );

                            glDeleteTextures( 1, &ptd->tex_name );
                            m_ntex--;

                            ptd->tex_name = 0;            // mark the ptd as not available to GPU

                            //    Delete the chart data?
                            if( m_b_mem_crunch ) {
                                pTextureHash->erase( key );
                                delete ptd;
                            }
                        }
                    }
                }
                rect.x += rect.width;
            }
            rect.y += rect.height;
        }
    }

    glPushMatrix();

    int spx = R.x + R.width / 2;
    int spy = R.y + R.height / 2;

    //    Calculate a sub-pixel bias for overzoom renders,
    //    anticipating and correcting for scaled up texture rendering
    double biasx = 0.;
    double biasy = 0.;
    if( scalefactor < 1.0 ) {
        double pixx, pixy;
        if( b_plugin ) pPlugInWrapper->latlong_to_chartpix( vp.clat, vp.clon, pixx, pixy );
        else
            pBSBChart->latlong_to_chartpix( vp.clat, vp.clon, pixx, pixy );

        biasy = pixy - spy;
        biasx = pixx - spx;
    }

    glScalef( 1. / scalefactor * n_basemult, 1. / scalefactor * n_basemult, 1 );

    double xt = 0.;
    double yt = 0.;

    if( ( ( fabs( vp.rotation ) > 0.01 ) ) || ( g_bskew_comp && ( fabs( vp.skew ) > 0.01 ) ) ) {

        //    Shift texture drawing positions to account for the larger chart rectangle
        //    needed to cover the screen on rotated images
        double w = vp.pix_width;
        double h = vp.pix_height;
        xt = ( R.width - ( w * scalefactor / n_basemult ) ) / 2;
        yt = ( R.height - ( h * scalefactor / n_basemult ) ) / 2;

        //    Rotations occur around 0,0, so calculate a post-rotate translation factor
        double angle = vp.rotation;
        angle -= vp.skew;
        double ddx = ( scalefactor / n_basemult ) * ( w * cos( -angle ) - h * sin( -angle ) - w )
                     / 2;
        double ddy = ( scalefactor / n_basemult ) * ( h * cos( -angle ) + w * sin( -angle ) - h )
                     / 2;

        glRotatef( angle * 180. / PI, 0, 0, 1 );

        glTranslatef( ddx, ddy, 0 );                 // post rotate translation
    }

#if 0
    Sum and Difference Formulas
    sin(A+B)=sin A cos B + cos A sin B
             sin(A-B)=sin A cos B - cos A sin B
                      cos(A+B)=cos A cos B - sin A sin B
                               cos(A-B)=cos A cos B + sin A sin B
#endif

                                        //    Using a 2D loop, iterate thru the texture tiles of the chart
                                        //    For each tile, is it (1) needed and (2) present?
                                        int n_chart_tex = 0;
    int key;

    rect.y = 0;
    for( int i = 0; i < ny_tex; i++ ) {
        rect.height = tex_dim;
        rect.x = 0;
        for( int j = 0; j < nx_tex; j++ ) {
            rect.width = tex_dim;

            //    Is this tile needed (i.e. does it intersect the chart source rectangle?)
            wxRect ri = rect;
            ri.Intersect( R );
            if( ri.width && ri.height ) {
                n_chart_tex++;

                // calculate the on-screen rectangle coordinates for this tile
                int w = ri.width, h = ri.height;
                int x1 = ri.x - rect.x;
                int y1 = ri.y - rect.y;
                double x2 = ( ri.x - R.x ) - xt;
                double y2 = ( ri.y - R.y ) - yt;

                y2 -= biasy;
                x2 -= biasx;

                wxRect rt( ( x2 ) / scalefactor * n_basemult, ( y2 ) / scalefactor * n_basemult,
                           w / scalefactor * n_basemult, h / scalefactor * n_basemult );
                rt.Offset( -vp.rv_rect.x, -vp.rv_rect.y ); // compensate for the adjustment made in quilt composition

                //    And does this tile intersect the desired render region?
                if( region.Contains( rt ) != wxOutRegion ) {
                    //    Is this texture tile already defined?

                    //    Create the hash key
                    wxASSERT(rect.x < 15383);
                    wxASSERT(rect.y < 15383);

                    key = ( ( rect.x << 18 ) + ( rect.y << 4 ) ) + n_basemult;
                    ChartTextureHashType::iterator it = pTextureHash->find( key );

                    // if not found in the hash map, then get the bits as a texture descriptor
                    if( it == pTextureHash->end() ) {

                        ptd = new glTextureDescriptor;
                        ptd->tex_mult = n_basemult;

//                                    printf("  -->PopulateTD\n");
                        OCPNPopulateTD( ptd, n_basemult, rect, chart );
                        ( *pTextureHash )[key] = ptd;
                    } else
                        ptd = ( *pTextureHash )[key];

                    //    If the GPU does not know about this texture, upload it
                    if( ptd->tex_name == 0 ) {
                        GLuint tex_name;
                        glGenTextures( 1, &tex_name );
                        ptd->tex_name = tex_name;

                        glBindTexture( GL_TEXTURE_2D, tex_name );

                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                         GL_LINEAR_MIPMAP_NEAREST );

                        UploadTexture( ptd, n_basemult );

                        m_ntex++;
                    }

                    //    The texture is known to be available to the GPU
                    //    So map it in

                    wxStopWatch sw;

                    if( g_bDebugOGL ) {
                        wxString msg;
                        msg.Printf( _T("  -->BindTexture %d"), ptd->tex_name );
                        wxLogMessage( msg );
                    }

                    glBindTexture( GL_TEXTURE_2D, ptd->tex_name );

                    GLint ltex;
                    glGetIntegerv( GL_TEXTURE_BINDING_2D, &ltex );
                    wxASSERT(ltex == (GLint)ptd->tex_name);

                    sw.Pause();
                    long tt = sw.Time();
                    if( tt > 10 ) n_longbind++;

                    double sx = rect.width;
                    double sy = rect.height;

                    if( g_bDebugOGL ) {
                        wxString msg;
                        msg.Printf( _T("     glQuads TexCoord (%g,%g) (%g,%g) (%g,%g) (%g,%g)"),
                                    x1 / sx, y1 / sy, ( x1 + w ) / sx, y1 / sy, ( x1 + w ) / sx,
                                    ( y1 + h ) / sy, x1 / sx, ( y1 + h ) / sy );
                        wxLogMessage( msg );

                        msg.Printf( _T("     glQuads Vertex2f (%g,%g) (%g,%g) (%g,%g) (%g,%g)"),
                                    ( x2 ), ( y2 ), ( w + x2 ), ( y2 ), ( w + x2 ), ( h + y2 ), ( x2 ),
                                    ( h + y2 ) );
                        wxLogMessage( msg );
                    }

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
//      printf("  basemult:%d  scalefactor:%g  chart_tex:%d\n", n_basemult, scalefactor, n_chart_tex);

    glPopMatrix();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_DEPTH_TEST );

    if( n_longbind ) m_tex_max_res--;
    else {
        if( s_nquickbind++ > 100 ) {
            s_nquickbind = 0;
            m_tex_max_res++;
        }
    }
    m_tex_max_res = wxMax(m_tex_max_res, 20);
    m_tex_max_res = wxMin(m_tex_max_res, m_tex_max_res_initial);

}

void glChartCanvas::RenderQuiltViewGL( ViewPort &vp, OCPNRegion Region, bool b_clear )
{
    m_gl_rendered_region.Clear();

    if( cc1->m_pQuilt->GetnCharts() && !cc1->m_pQuilt->IsBusy() ) {
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
            if( !clear_test_region.IsEmpty() )
                glClear( GL_COLOR_BUFFER_BIT );
        }

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

                if(b_rendered)
                    m_gl_rendered_region.Union(get_region);
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
    else if( !cc1->m_pQuilt->GetnCharts() ) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void glChartCanvas::ComputeRenderQuiltViewGLRegion( ViewPort &vp, OCPNRegion Region )
{
    m_gl_rendered_region.Clear();

    if( cc1->m_pQuilt->GetnCharts() && !cc1->m_pQuilt->IsBusy() ) {
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

                    //  Todo  If chart is cm93, and it happens not to render, then calculation will be wrong
                    //  need a "test render" method for cm93

                    m_gl_rendered_region.Union(get_region);
                }

                chart = cc1->m_pQuilt->GetNextChart();
            }
     }
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

//    DrawAllRoutesAndWaypoints( temp_vp, region );
    cc1->DrawAllRoutesInBBox( dc, temp_vp.GetBBox(), region );
    cc1->DrawAllWaypointsInBBox( dc, temp_vp.GetBBox(), region, true ); // true draws only isolated marks

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

    //    Take a look and see if memory is getting close to exceeding the user specified max
    m_b_mem_crunch = false;
    int mem_total, mem_used;
    GetMemoryStatus( &mem_total, &mem_used );
    if(mem_used > g_memCacheLimit * 8 / 10) m_b_mem_crunch = true;

    wxPaintDC( this );

    ViewPort VPoint = cc1->VPoint;
    ViewPort svp = VPoint;
    svp.pix_width = svp.rv_rect.width;
    svp.pix_height = svp.rv_rect.height;

    wxRegion ru = GetUpdateRegion();

    //  Is this viewpoint the same as the previously painted one?
    bool b_newview = true;
    ;
    if( ( m_gl_cache_vp.view_scale_ppm == VPoint.view_scale_ppm )
            && ( m_gl_cache_vp.rotation == VPoint.rotation ) && ( m_gl_cache_vp.clat == VPoint.clat )
            && ( m_gl_cache_vp.clon == VPoint.clon ) && m_gl_cache_vp.IsValid() ) {
        b_newview = false;
    }

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

    if((m_ntex > m_tex_max_res) || m_b_mem_crunch ) {
        ChartPointerHashType::iterator it0;
        for( it0 = m_chart_hash.begin(); it0 != m_chart_hash.end(); ++it0 ) {
            ChartBaseBSB *pc = (ChartBaseBSB *) it0->first;

            if( VPoint.b_quilt )          // quilted
            {
                if( cc1->m_pQuilt && cc1->m_pQuilt->IsComposed() ) {
                    if( !cc1->m_pQuilt->IsChartInQuilt( pc ) ) {
                        ChartTextureHashType *pTextureHash = m_chart_hash[pc];

                        // iterate over all the textures presently loaded
                        // and delete the OpenGL texture from the GPU
                        // but keep the private texture descriptor for now

                        ChartTextureHashType::iterator it = pTextureHash->begin();
                        while( it != pTextureHash->end() ) {
                            glTextureDescriptor *ptd = it->second;

                            if( ptd->tex_name > 0 ) {
                                if( g_bDebugOGL ) printf(
                                        "glDeleteTextures in Unused chart...()\n" );
                                glDeleteTextures( 1, &ptd->tex_name );
                                m_ntex--;

                                ptd->tex_name = 0;

                                //    Delete the chart data?
                                if( m_b_mem_crunch ) {
                                    pTextureHash->erase( it );
                                    delete ptd;
                                    it = pTextureHash->begin();              // reset the iterator
                                }

                            } else
                                ++it;
                        }
                    }
                }
            }
            else      // not quilted
            {
                if( Current_Ch != pc )
                {
                    ChartTextureHashType *pTextureHash = m_chart_hash[pc];

                    // iterate over all the textures presently loaded
                    // and delete the OpenGL texture from the GPU
                    // but keep the private texture descriptor for now

                    ChartTextureHashType::iterator it = pTextureHash->begin();
                    while( it != pTextureHash->end())
                    {
                        glTextureDescriptor *ptd = it->second;

                        if(ptd->tex_name > 0)
                        {
                            if(g_bDebugOGL) printf("glDeleteTextures in Unused chart...()\n");
                            glDeleteTextures( 1, &ptd->tex_name );
                            m_ntex--;

                            ptd->tex_name = 0;

                            //    Delete the chart data?
                            if(m_b_mem_crunch)
                            {
                                pTextureHash->erase(it);
                                delete ptd;
                                it = pTextureHash->begin();    // reset the iterator
                            }

                        }
                        else
                            ++it;
                    }
                }
            }
        }
    }

    int sx = GetSize().x;
    int sy = GetSize().y;

    if( VPoint.b_quilt )          // quilted
    {
        if( cc1->m_pQuilt && !cc1->m_pQuilt->IsComposed() ) return;

        //  TODO This may not be necessary, but nice for debugging
//              glClear(GL_COLOR_BUFFER_BIT);

        // Try to do accelerated pans
        if( m_b_BuiltFBO ) {
            if( m_gl_cache_vp.IsValid() && ( m_cache_tex > 0 ) && !g_bCourseUp ) {
                if( b_newview ) {

                    wxPoint c_old = VPoint.GetPixFromLL( VPoint.clat, VPoint.clon );
                    wxPoint c_new = m_gl_cache_vp.GetPixFromLL( VPoint.clat, VPoint.clon );

                    int dy = c_new.y - c_old.y;
                    int dx = c_new.x - c_old.x;

                    if( cc1->m_pQuilt->IsVPBlittable( VPoint, dx, dy, true ) ) // allow vector charts
                    {
                        if( dx || dy ) {
                            //    Render the reuseable portion of the cached texture
                            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

                            //      Make a new temporary texture, and bind to FBO
                            glGenTextures( 1, &m_blit_tex );

                            glBindTexture( m_TEX_TYPE, m_blit_tex );
                            glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                            glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                            glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0,
                                          GL_RGBA, GL_UNSIGNED_BYTE, NULL );
                            ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT,
                                                              GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE, m_blit_tex, 0 );

                            glBindTexture( m_TEX_TYPE, m_cache_tex );
                            glEnable( m_TEX_TYPE );
                            glColor3f( 0, .25, 0 );
                            glDisable( GL_BLEND );
                            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

                            glDisable( GL_DEPTH_TEST );
                            glClear( GL_COLOR_BUFFER_BIT );

                            // Render the cached texture as quad to FBO(m_blit_tex) with offsets

                            if( GL_TEXTURE_RECTANGLE_ARB == m_TEX_TYPE ) {
                                wxASSERT(sx == m_cache_tex_x);
                                wxASSERT(sy == m_cache_tex_y);
                                int ow = sx - abs( dx );
                                int oh = sy - abs( dy );
                                int x1, x2, y1, y2;
                                if( dx > 0 ) {
                                    x1 = dx;
                                    x2 = 0;
                                } else {
                                    x1 = 0;
                                    x2 = -dx;
                                }

                                if( dy > 0 ) {
                                    y1 = dy;
                                    y2 = 0;
                                } else {
                                    y1 = 0;
                                    y2 = -dy;
                                }

                                glBegin( GL_QUADS );
                                glTexCoord2f( x1, sy - y1 );
                                glVertex2f( x2, y2 );
                                glTexCoord2f( x1 + ow, sy - y1 );
                                glVertex2f( x2 + ow, y2 );
                                glTexCoord2f( x1 + ow, y2 );
                                glVertex2f( x2 + ow, y2 + oh );
                                glTexCoord2f( x1, y2 );
                                glVertex2f( x2, y2 + oh );
                                glEnd();
                            }

                            //calculate the new regions to render
                            OCPNRegion update_region;
                            if( dy ) {
                                if( dy > 0 ) update_region.Union(
                                        wxRect( 0, VPoint.pix_height - dy, VPoint.pix_width, dy ) );
                                else
                                    update_region.Union( wxRect( 0, 0, VPoint.pix_width, -dy ) );
                            }

                            if( dx ) {
                                if( dx > 0 ) update_region.Union(
                                        wxRect( VPoint.pix_width - dx, 0, dx, VPoint.pix_height ) );
                                else
                                    update_region.Union( wxRect( 0, 0, -dx, VPoint.pix_height ) );
                            }

                            //   Done with cached texture "blit"
                            glDisable( m_TEX_TYPE );
                            //      Delete the stale cached texture
                            glDeleteTextures( 1, &m_cache_tex );
                            //    And make the blit texture "become" the cached texture
                            m_cache_tex = m_blit_tex;

                            //   Attach the renamed "blit" texture to the FBO
                            ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT,
                                                              GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE, m_cache_tex, 0 );

                            //      Render the chart(s) in update region
                            RenderQuiltViewGL( VPoint, update_region, false );          // no clear wanted here
                            ComputeRenderQuiltViewGLRegion( VPoint, chart_get_region );

                        } else {
                            //    No sensible (dx, dy) change in the view, so use the cached member bitmap
                        }
                    }

                    else              // not blitable
                    {
                        ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

                        //      Delete the current cached texture
                        glDeleteTextures( 1, &m_cache_tex );

                        //      Make a new texture, and bind to FBO
                        glGenTextures( 1, &m_cache_tex );

                        glBindTexture( m_TEX_TYPE, m_cache_tex );
                        glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                        glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                        glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0,
                                      GL_RGBA, GL_UNSIGNED_BYTE, NULL );
                        ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT,
                                                          GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE, m_cache_tex, 0 );

                        //      Render the chart(s)
                        RenderQuiltViewGL( VPoint, chart_get_region );

                    }
                }           // newview
                else {
                    //    No change in the view, so use the cached member texture
                }
            } else      //cached texture is not valid
            {
                ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

                //      Delete the current cached texture
                glDeleteTextures( 1, &m_cache_tex );

                //      Make a new texture, and bind to FBO
                glGenTextures( 1, &m_cache_tex );

                glBindTexture( m_TEX_TYPE, m_cache_tex );
                glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                              GL_UNSIGNED_BYTE, NULL );
                ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                                  m_TEX_TYPE, m_cache_tex, 0 );

                //      Render the chart(s)
                RenderQuiltViewGL( VPoint, chart_get_region );

            }

            // Disable Render to FBO
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );
            glDisable( GL_DEPTH_TEST );

            // Render the cached texture as quad to screen
            glBindTexture( m_TEX_TYPE, m_cache_tex );
            glEnable( m_TEX_TYPE );
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

            if( GL_TEXTURE_RECTANGLE_ARB == m_TEX_TYPE ) {
                glBegin( GL_QUADS );
                glTexCoord2f( 0, m_cache_tex_y );
                glVertex2f( 0, 0 );
                glTexCoord2f( m_cache_tex_x, m_cache_tex_y );
                glVertex2f( sx, 0 );
                glTexCoord2f( m_cache_tex_x, 0 );
                glVertex2f( sx, sy );
                glTexCoord2f( 0, 0 );
                glVertex2f( 0, sy );
                glEnd();
            }
            glDisable( m_TEX_TYPE );

            m_gl_cache_vp = VPoint;
            cc1->m_pQuilt->SetRenderedVP( VPoint );

        }         // useFBO
        else {
            RenderQuiltViewGL( svp, chart_get_region );
        }

    }         // quilted
    else                  // not quilted
    {
        bool b_rendered = false;
        ChartBaseBSB *Current_Ch_BSB = dynamic_cast<ChartBaseBSB*>( Current_Ch );
        if( Current_Ch_BSB ) {
            glClear( GL_COLOR_BUFFER_BIT );
            RenderRasterChartRegionGL( Current_Ch, cc1->VPoint, chart_get_region );
            b_rendered = true;
        } else {
            ChartPlugInWrapper *Current_Ch_PlugInWrapper =
                dynamic_cast<ChartPlugInWrapper*>( Current_Ch );
            if( Current_Ch_PlugInWrapper ) {
                if( Current_Ch_PlugInWrapper->GetChartFamily() == CHART_FAMILY_RASTER ) {
                    RenderRasterChartRegionGL( Current_Ch, cc1->VPoint, chart_get_region );
                    b_rendered = true;
                }
            }
        }

        if( !b_rendered ) {
            if( !dynamic_cast<ChartDummy*>( Current_Ch ) ) {
                glClear( GL_COLOR_BUFFER_BIT );
                OCPNRegion full_region( cc1->VPoint.rv_rect );
                Current_Ch->RenderRegionViewOnGL( *m_pcontext, cc1->VPoint, full_region );
            }
        }
    }

//    Render the WorldChart

    OCPNRegion chartValidRegion;

    if(!VPoint.b_quilt)
        Current_Ch->GetValidCanvasRegion ( svp, &chartValidRegion );
    else
        chartValidRegion = m_gl_rendered_region;

    // Make a region covering the current chart on the canvas
        // growing the box to account for rotation
    OCPNRegion backgroundRegion( VPoint.rv_rect.x, VPoint.rv_rect.y, VPoint.rv_rect.width,
                                   VPoint.rv_rect.height );


        //    Remove the valid chart area
    if( chartValidRegion.IsOk() ) {
            chartValidRegion.Offset( wxPoint(VPoint.rv_rect.x, VPoint.rv_rect.y) );
            backgroundRegion.Subtract( chartValidRegion );
    }

        //    Draw the World Chart only in the areas NOT covered by the current chart view
        //    And, only if the region is ..not.. empty

    if( !backgroundRegion.IsEmpty() && ( fabs( cc1->GetVP().skew ) < .01 ) ) {
        ViewPort nvp = VPoint;
        nvp.rv_rect.x = 0;
        nvp.rv_rect.y = 0;

        SetClipRegion( nvp, backgroundRegion, true, true );       // clear background

        glPushMatrix();
        if( fabs( cc1->GetVP().rotation ) > .01 ) {
            double w2 = cc1->GetVP().rv_rect.width / 2;
            double h2 = cc1->GetVP().rv_rect.height / 2;

            double angle = cc1->GetVP().rotation;

            //    Rotations occur around 0,0, so calculate a post-rotate translation factor
            double ddx = ( w2 * cos( -angle ) - h2 * sin( -angle ) - w2 ) ;
            double ddy = ( h2 * cos( -angle ) + w2 * sin( -angle ) - h2 ) ;
            glRotatef( angle * 180. / PI, 0, 0, 1 );
            glTranslatef( ddx, ddy, 0 );                 // post rotate translation

                // WorldBackgroundChart renders in an offset rectangle,
                // So translate back to standard coordinates
            double x1 = cc1->GetVP().rv_rect.x;
            double y1 = cc1->GetVP().rv_rect.y;
            double x2 =  x1 * cos( angle ) + y1 * sin( angle );
            double y2 =  y1 * cos( angle ) - x1 * sin( angle );
            glTranslatef( x2, y2, 0 );
        }

        cc1->pWorldBackgroundChart->RenderViewOnDC( gldc, VPoint );

        glDisable( GL_STENCIL_TEST );
        glDisable( GL_DEPTH_TEST );

        glPopMatrix();
    }


//    Now render overlay objects
    wxRect rect = ru.GetBox();
    DrawGroundedOverlayObjectsRect( gldc, rect );
    DrawFloatingOverlayObjects( gldc );

    SwapBuffers();
    glFinish();

    cc1->PaintCleanup();

//     if(g_bDebugOGL)  printf("m_ntex: %d %d\n\n", m_ntex, m_tex_max_res);
    if( g_bDebugOGL ) {
        wxString msg;
        msg.Printf( _T("  -->m_ntex %d %d\n"), m_ntex, m_tex_max_res );
        wxLogMessage( msg );
    }

}

