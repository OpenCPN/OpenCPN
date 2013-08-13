/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include "wx/wxprec.h"
#include "GL/gl.h"

#include "glChartCanvas.h"
#include "glTextureDescriptor.h"
#include "chcanv.h"
#include "dychart.h"
#include "s52plib.h"
#include "Quilt.h"
#include "pluginmanager.h"
#include "chartbase.h"
#include "chartimg.h"
#include "s57chart.h"
#include "ChInfoWin.h"
#include "thumbwin.h"

extern bool GetMemoryStatus(int *mem_total, int *mem_used);

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#endif

extern ChartCanvas *cc1;
extern s52plib *ps52plib;
extern bool g_bopengl;
extern bool g_b_useStencil;
extern int g_GPU_MemSize;
extern bool g_bDebugOGL;
extern PlugInManager* g_pi_manager;
extern bool g_bskew_comp;
extern int g_memCacheLimit;
extern bool g_bCourseUp;
extern ChartBase *Current_Ch;
extern ColorScheme global_color_scheme;
extern bool g_bquiting;
extern ThumbWin         *pthumbwin;

extern PFNGLGENFRAMEBUFFERSEXTPROC         s_glGenFramebuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC        s_glGenRenderbuffersEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    s_glFramebufferTexture2DEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC         s_glBindFramebufferEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC s_glFramebufferRenderbufferEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC     s_glRenderbufferStorageEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC        s_glBindRenderbufferEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  s_glCheckFramebufferStatusEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC      s_glDeleteFramebuffersEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC     s_glDeleteRenderbuffersEXT;

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

static bool GetglEntryPoints( void )
{
#if defined(__WXMSW__)
    s_hGL_DLL = LoadLibrary( (LPCWSTR) "opengl32.dll" );
    if( NULL == s_hGL_DLL ) return false;

    s_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) GetProcAddress( s_hGL_DLL,
                             "glGenFramebuffersEXT" );
    s_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) GetProcAddress( s_hGL_DLL,
                              "glGenRenderbuffersEXT" );
    s_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) GetProcAddress( s_hGL_DLL,
                                  "glFramebufferTexture2DEXT" );
    s_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) GetProcAddress( s_hGL_DLL,
                             "glBindFramebufferEXT" );
    s_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) GetProcAddress(
                                         s_hGL_DLL, "glFramebufferRenderbufferEXT" );
    s_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) GetProcAddress( s_hGL_DLL,
                                 "glRenderbufferStorageEXT" );
    s_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) GetProcAddress( s_hGL_DLL,
                              "glBindRenderbufferEXT" );
    s_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) GetProcAddress( s_hGL_DLL,
                                    "glCheckFramebufferStatusEXT" );
    s_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) GetProcAddress( s_hGL_DLL,
                                "glDeleteFramebuffersEXT" );
    s_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) GetProcAddress( s_hGL_DLL,
                                 "glDeleteRenderbuffersEXT" );

#elif defined(__WXMAC__)
    return false;

#else

    s_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glGenFramebuffersEXT");
    s_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glGenRenderbuffersEXT");
    s_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)glXGetProcAddress((const GLubyte *)"glFramebufferTexture2DEXT");
    s_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glBindFramebufferEXT");
    s_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glFramebufferRenderbufferEXT");
    s_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)glXGetProcAddress((const GLubyte *)"glRenderbufferStorageEXT");
    s_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glBindRenderbufferEXT");
    s_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glXGetProcAddress((const GLubyte *)"glCheckFramebufferStatusEXT");
    s_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glDeleteFramebuffersEXT");
    s_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glDeleteRenderbuffersEXT");
#endif

    if( NULL == s_glGenFramebuffersEXT ) return false;
    if( NULL == s_glGenRenderbuffersEXT ) return false;
    if( NULL == s_glFramebufferTexture2DEXT ) return false;
    if( NULL == s_glBindFramebufferEXT ) return false;
    if( NULL == s_glFramebufferRenderbufferEXT ) return false;
    if( NULL == s_glRenderbufferStorageEXT ) return false;
    if( NULL == s_glBindRenderbufferEXT ) return false;
    if( NULL == s_glCheckFramebufferStatusEXT ) return false;
    if( NULL == s_glDeleteFramebuffersEXT ) return false;
    if( NULL == s_glDeleteRenderbuffersEXT ) return false;

    return true;
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
        if( m_bsetup && m_b_useFBO ) {
            BuildFBO();
            ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, 0 );
        }

    }

}

void glChartCanvas::MouseEvent( wxMouseEvent& event )
{
    cc1->MouseEvent( event );
}

void glChartCanvas::BuildFBO( void )
{
    if( m_bsetup && m_b_useFBO ) {
        glDeleteTextures( 1, &m_cache_tex );
        ( *s_glDeleteFramebuffersEXT )( 1, &m_fb0 );
        ( *s_glDeleteRenderbuffersEXT )( 1, &m_depth_rb );
    }

    if( m_b_useFBO ) {
        m_cache_tex_x = GetSize().x;
        m_cache_tex_y = GetSize().y;

        ( *s_glGenFramebuffersEXT )( 1, &m_fb0 );
        glGenTextures( 1, &m_cache_tex );
        ( *s_glGenRenderbuffersEXT )( 1, &m_depth_rb );

        ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );

// initialize color texture

        glBindTexture( m_TEX_TYPE, m_cache_tex );
        glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, NULL );
        ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE,
                                          m_cache_tex, 0 );

        if( m_b_useFBOStencil ) {
            // initialize composite depth/stencil renderbuffer
            ( *s_glBindRenderbufferEXT )( GL_RENDERBUFFER_EXT, m_depth_rb );
            ( *s_glRenderbufferStorageEXT )( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT,
                                             m_cache_tex_x, m_cache_tex_y );

            // Can we attach to depth and stencil at once?  Maybe
            // it would be easier to not check for this extension and
            // always use 2 calls.
            if( QueryExtension( "GL_ARB_framebuffer_object" ) ) {
                ( *s_glFramebufferRenderbufferEXT )( GL_FRAMEBUFFER_EXT,
                                                     GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, m_depth_rb );
            } else {
                ( *s_glFramebufferRenderbufferEXT )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                                     GL_RENDERBUFFER_EXT, m_depth_rb );

                ( *s_glFramebufferRenderbufferEXT )( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                                     GL_RENDERBUFFER_EXT, m_depth_rb );
            }

        } else {
            // initialize depth renderbuffer
            ( *s_glBindRenderbufferEXT )( GL_RENDERBUFFER_EXT, m_depth_rb );
            ( *s_glRenderbufferStorageEXT )( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                                             m_cache_tex_x, m_cache_tex_y );
            ( *s_glFramebufferRenderbufferEXT )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                                 GL_RENDERBUFFER_EXT, m_depth_rb );
        }

        ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, 0 );
    }

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

        char render_string[80];
        strncpy( render_string, (char *) glGetString( GL_RENDERER ), 79 );
        m_renderer = wxString( render_string, wxConvUTF8 );

        wxString msg;
        msg.Printf( _T("OpenGL-> Renderer String: ") );
        msg += m_renderer;
        wxLogMessage( msg );

        if( ps52plib ) ps52plib->SetGLRendererString( m_renderer );

        //  This little hack fixes a problem seen with some Intel 945 graphics chips
        //  We need to not do anything that requires (some) complicated stencil operations.

        bool bad_stencil_code = false;
        if( GetRendererString().Find( _T("Intel") ) != wxNOT_FOUND ) {
            wxLogMessage( _T("OpenGL-> Detected Intel renderer, disabling stencil buffer") );
            bad_stencil_code = true;
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

        g_b_useStencil = false;
        if( !bad_stencil_code && stencil && ( sb == 8 ) ) g_b_useStencil = true;

//          GLenum err = glewInit();
//           if (GLEW_OK != err)

        m_b_useFBO = false;              // default is false

        //      We require certain extensions to support FBO rendering
        if( QueryExtension( "GL_ARB_texture_rectangle" )
                && QueryExtension( "GL_EXT_framebuffer_object" ) ) {
            m_TEX_TYPE = GL_TEXTURE_RECTANGLE_ARB;
            m_b_useFBO = true;
        }

        if( GetRendererString().Find( _T("Intel") ) != wxNOT_FOUND ) {
            wxLogMessage( _T("OpenGL-> Detected Intel renderer, disabling FBO") );
            m_b_useFBO = false;
        }

        if( !GetglEntryPoints() ) m_b_useFBO = false;              // default is false

        //      Can we use the stencil buffer in a FBO?
        if( QueryExtension( "GL_EXT_packed_depth_stencil" ) ) m_b_useFBOStencil = true;
        else
            m_b_useFBOStencil = false;

        //      Maybe build FBO(s)
        if( m_b_useFBO ) {
            BuildFBO();
// Check framebuffer completeness at the end of initialization.
            ( s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );
            GLenum fb_status = ( *s_glCheckFramebufferStatusEXT )( GL_FRAMEBUFFER_EXT );
            ( s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, 0 );

            if( fb_status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
                wxString msg;
                msg.Printf( _T("    OpenGL-> Framebuffer Incomplete:  %08X"), fb_status );
                wxLogMessage( msg );
                m_b_useFBO = false;
            }
        }

        if( m_b_useFBO && !m_b_useFBOStencil ) g_b_useStencil = false;

        if( m_b_useFBO ) {
            wxLogMessage( _T("OpenGL-> Using Framebuffer Objects") );

            if( m_b_useFBOStencil ) wxLogMessage( _T("OpenGL-> Using FBO Stencil buffer") );
            else
                wxLogMessage( _T("OpenGL-> FBO Stencil buffer unavailable") );
        } else
            wxLogMessage( _T("OpenGL-> Framebuffer Objects unavailable") );

        if( g_b_useStencil ) wxLogMessage( _T("OpenGL-> Using Stencil buffer clipping") );
        else
            wxLogMessage( _T("OpenGL-> Using Depth buffer clipping") );

        /* we upload non-aligned memory */
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

        int n_tex_size = 512;
        double n_GPU_Mem = wxMax(64, g_GPU_MemSize) * ( 1 << 20 );
        m_tex_max_res = n_GPU_Mem / ( n_tex_size * n_tex_size * 4 * 1.3 ); // 1.3 multiplier allows for full mipmaps
        m_tex_max_res /= 2;
        m_tex_max_res_initial = m_tex_max_res;

        wxString str;
        str.Printf( _T("OpenGL-> Estimated Max Resident Textures: %d"), m_tex_max_res );
        wxLogMessage( str );

        m_bsetup = true;
//          g_bDebugOGL = true;
    }

    //      Recursion test, sometimes seen on GTK systems when wxBusyCursor is activated
    if( s_in_glpaint ) return;
    s_in_glpaint++;

    render();

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

void glChartCanvas::DrawGLOverLayObjects( void )
{
    if( g_pi_manager ) g_pi_manager->RenderAllGLCanvasOverlayPlugIns( m_pcontext, cc1->GetVP() );
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

void glChartCanvas::SetClipRegion( ViewPort &vp, OCPNRegion &region, bool b_clear )
{
    if( g_b_useStencil ) {
        glPushMatrix();

        if( ( ( fabs( vp.rotation ) > 0.01 ) ) || ( g_bskew_comp && ( fabs( vp.skew ) > 0.01 ) ) ) {

            //    Shift texture drawing positions to account for the larger chart rectangle
            //    needed to cover the screen on rotated images
            double w = vp.pix_width;
            double h = vp.pix_height;

            double angle = vp.rotation;
            angle -= vp.skew;

            //    Rotations occur around 0,0, so calculate a post-rotate translation factor
            double ddx = ( w * cos( -angle ) - h * sin( -angle ) - w ) / 2;
            double ddy = ( h * cos( -angle ) + w * sin( -angle ) - h ) / 2;

            glRotatef( angle * 180. / PI, 0, 0, 1 );

            glTranslatef( ddx, ddy, 0 );                 // post rotate translation
        }

        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    As a convenience, while we are creating the stencil or depth mask,
        //    also clear the background if selected
        if( b_clear ) {
            glColor3f( 0, 255, 0 );
            glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // enable color buffer
        } else
            glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

        //    Decompose the region into rectangles, and draw as quads
        OCPNRegionIterator clipit( region );
        while( clipit.HaveRects() ) {
            wxRect rect = clipit.GetRect();

            if(vp.b_quilt)
                rect.Offset(vp.rv_rect.x, vp.rv_rect.y); // undo the adjustment made in quilt composition
            else if(Current_Ch && Current_Ch->GetChartFamily() != CHART_FAMILY_VECTOR)
                rect.Offset(vp.rv_rect.x, vp.rv_rect.y);

            glBegin( GL_QUADS );

            glVertex2f( rect.x, rect.y );
            glVertex2f( rect.x + rect.width, rect.y );
            glVertex2f( rect.x + rect.width, rect.y + rect.height );
            glVertex2f( rect.x, rect.y + rect.height );
            glEnd();

            clipit.NextRect();
        }

        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer

        glPopMatrix();

    } else              //  Use depth buffer for clipping
    {
        glPushMatrix();

        if( ( ( fabs( vp.rotation ) > 0.01 ) ) || ( g_bskew_comp && ( fabs( vp.skew ) > 0.01 ) ) ) {

            //    Shift texture drawing positions to account for the larger chart rectangle
            //    needed to cover the screen on rotated images
            double w = vp.pix_width;
            double h = vp.pix_height;

            double angle = vp.rotation;
            angle -= vp.skew;

            //    Rotations occur around 0,0, so calculate a post-rotate translation factor
            double ddx = ( w * cos( -angle ) - h * sin( -angle ) - w ) / 2;
            double ddy = ( h * cos( -angle ) + w * sin( -angle ) - h ) / 2;

            glRotatef( angle * 180. / PI, 0, 0, 1 );

            glTranslatef( ddx, ddy, 0 );                 // post rotate translation
        }

        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer

        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start

        //    As a convenience, while we are creating the stencil or depth mask,
        //    also clear the background if selected
        if( b_clear ) {
            glColor3f( 255, 0, 0 );
            glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // enable color buffer
        } else
            glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
        OCPNRegionIterator clipit( region );
        while( clipit.HaveRects() ) {
            wxRect rect = clipit.GetRect();

            if(vp.b_quilt)
                rect.Offset(vp.rv_rect.x, vp.rv_rect.y); // undo the adjustment made in quilt composition
            else if(Current_Ch && Current_Ch->GetChartFamily() != CHART_FAMILY_VECTOR)
                rect.Offset(vp.rv_rect.x, vp.rv_rect.y);

            glBegin( GL_QUADS );

            // dep buffer clear = 1
            // 1 makes 0 in dep buffer, works
            // 0 make .5 in depth buffer
            // -1 makes 1 in dep buffer

            //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
            //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
            //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
            glVertex3f( rect.x, rect.y, 0.5 );
            glVertex3f( rect.x + rect.width, rect.y, 0.5 );
            glVertex3f( rect.x + rect.width, rect.y + rect.height, 0.5 );
            glVertex3f( rect.x, rect.y + rect.height, 0.5 );
            glEnd();

            clipit.NextRect();
        }

        glDepthFunc( GL_GREATER );                          // Set the test value
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
        glDepthMask( GL_FALSE );                            // disable depth buffer

        glPopMatrix();

    }
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

    SetClipRegion( vp, region, false );         // no need to clear

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

void glChartCanvas::render()
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

    if( g_b_useStencil ) {
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
        if( m_b_useFBO ) {
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
                            ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );

                            //      Make a new temporary texture, and bind to FBO
                            glGenTextures( 1, &m_blit_tex );

                            glBindTexture( m_TEX_TYPE, m_blit_tex );
                            glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                            glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                            glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0,
                                          GL_RGBA, GL_UNSIGNED_BYTE, NULL );
                            ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT,
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
                            ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT,
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
                        ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );

                        //      Delete the current cached texture
                        glDeleteTextures( 1, &m_cache_tex );

                        //      Make a new texture, and bind to FBO
                        glGenTextures( 1, &m_cache_tex );

                        glBindTexture( m_TEX_TYPE, m_cache_tex );
                        glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                        glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                        glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0,
                                      GL_RGBA, GL_UNSIGNED_BYTE, NULL );
                        ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT,
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
                ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );

                //      Delete the current cached texture
                glDeleteTextures( 1, &m_cache_tex );

                //      Make a new texture, and bind to FBO
                glGenTextures( 1, &m_cache_tex );

                glBindTexture( m_TEX_TYPE, m_cache_tex );
                glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                              GL_UNSIGNED_BYTE, NULL );
                ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                                  m_TEX_TYPE, m_cache_tex, 0 );

                //      Render the chart(s)
                RenderQuiltViewGL( VPoint, chart_get_region );

            }

            // Disable Render to FBO
            ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, 0 );
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

        SetClipRegion( nvp, backgroundRegion, true );       // clear background

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
    DrawGLOverLayObjects();
    cc1->DrawOverlayObjects( gldc, ru );

    if( cc1->m_bShowTide ) cc1->DrawAllTidesInBBox( gldc, cc1->GetVP().GetBBox(), true, true );

    if( cc1->m_bShowCurrent ) cc1->DrawAllCurrentsInBBox( gldc, cc1->GetVP().GetBBox(), true,
                true );
    
 
    //  On some platforms, the opengl context window is always on top of any standard DC windows,
    //  so we need to draw the Chart Info Window and the Thumbnail as overlayed bmps.

#ifdef __WXOSX__    
    if(cc1->m_pCIWin && cc1->m_pCIWin->IsShown()) {
        wxClientDC infodc(cc1->m_pCIWin);
        int x, y, width, height;
        cc1->m_pCIWin->GetClientSize( &width, &height );
        cc1->m_pCIWin->GetPosition( &x, &y );
        wxBitmap bmp(width, height, -1);
        wxMemoryDC dc(bmp);
        if(bmp.IsOk()){
            dc.Blit( 0, 0, width, height, &infodc, 0, 0 );
            dc.SelectObject(wxNullBitmap);
            
            gldc.DrawBitmap( bmp, x, y, false);
        }
    }

    if( pthumbwin && pthumbwin->IsShown()) {
        wxClientDC thumbdc(pthumbwin);
        int thumbx, thumby, thumbsx, thumbsy;
        pthumbwin->GetPosition( &thumbx, &thumby );
        pthumbwin->GetSize( &thumbsx, &thumbsy );
        wxBitmap thumbbmp(thumbsx, thumbsy, -1);
        wxMemoryDC dc(thumbbmp);
        if(thumbbmp.IsOk()){
            dc.Blit( 0, 0, thumbsx, thumbsy, &thumbdc, 0, 0 );
            dc.SelectObject(wxNullBitmap);
            
            gldc.DrawBitmap( thumbbmp, thumbx, thumby, false);
        }
    }
#endif

    //quiting?
    if( g_bquiting ) {
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

#if 0
    //  Debug
    OCPNRegionIterator upd ( ru );
    while ( upd )
    {
        wxRect rect = upd.GetRect();
        glBegin(GL_LINE_LOOP);
        glColor3f(0, 0, 0);
        glVertex2i(rect.x, rect.y);
        glVertex2i(rect.x, rect.y + rect.height);
        glVertex2i(rect.x + rect.width, rect.y + rect.height);
        glVertex2i(rect.x + rect.width, rect.y);
        glEnd();

        upd ++;
    }
#endif

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

