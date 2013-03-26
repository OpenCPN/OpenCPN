// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "glChartCanvas.h"
#include "glTextureDescriptor.h"
#include "chcanv.h"
#include "dychart.h"
#include "s52plib.h"

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#endif

extern ChartCanvas *cc1;
extern s52plib *ps52plib;
extern bool g_bopengl;
extern bool g_b_useStencil;
extern int g_GPU_MemSize;
extern bool g_bDebugOGL;
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

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( ArrayOfTexDescriptors );

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


