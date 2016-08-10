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

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/tokenzr.h>

#include <stdint.h>

#if defined( __UNIX__ ) && !defined(__WXOSX__)  // high resolution stopwatch for profiling
class OCPNStopWatch
{
public:
    OCPNStopWatch() { Reset(); }
    void Reset() { clock_gettime(CLOCK_REALTIME, &tp); }

    double GetTime() {
        timespec tp_end;
        clock_gettime(CLOCK_REALTIME, &tp_end);
        return (tp_end.tv_sec - tp.tv_sec) * 1.e3 + (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
    }

private:
    timespec tp;
};
#endif


#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#include "dychart.h"

#include "glChartCanvas.h"
#include "chcanv.h"
#include "s52plib.h"
#include "Quilt.h"
#include "pluginmanager.h"
#include "routeman.h"
#include "chartbase.h"
#include "chartimg.h"
#include "ChInfoWin.h"
#include "thumbwin.h"
#include "chartdb.h"
#include "navutil.h"
#include "TexFont.h"
#include "glTexCache.h"
#include "gshhs.h"
#include "ais.h"
#include "OCPNPlatform.h"
#include "toolbar.h"
#include "piano.h"
#include "tcmgr.h"
#include "compass.h"
#include "FontMgr.h"
#include "mipmap/mipmap.h"
#include "chartimg.h"

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif

#ifdef USE_S57
#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#endif

#include "lz4.h"

#ifdef __OCPN__ANDROID__
//  arm gcc compiler has a lot of trouble passing doubles as function aruments.
//  We don't really need double precision here, so fix with a (faster) macro.
extern "C" void glOrthof(float left,  float right,  float bottom,  float top,  float near,  float far);
#define glOrtho(a,b,c,d,e,f);     glOrthof(a,b,c,d,e,f);

#endif

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
extern bool g_bDebugOGL;
extern bool g_bShowFPS;
extern bool g_bSoftwareGL;
extern bool g_btouch;
extern OCPNPlatform *g_Platform;
extern ocpnFloatingToolbarDialog *g_FloatingToolbarDialog;
extern ocpnStyle::StyleManager* g_StyleManager;
extern bool             g_bShowChartBar;
extern Piano           *g_Piano;
extern ocpnCompass         *g_Compass;
extern ChartStack *pCurrentStack;
extern glTextureManager   *g_glTextureManager;
extern bool             b_inCompressAllCharts;

GLenum       g_texture_rectangle_format;

extern int g_memCacheLimit;
extern bool g_bCourseUp;
extern ChartBase *Current_Ch;
extern ColorScheme global_color_scheme;
extern bool g_bquiting;
extern ThumbWin         *pthumbwin;
extern bool             g_bDisplayGrid;
extern int g_mipmap_max_level;

extern double           gLat, gLon, gCog, gSog, gHdt;

extern int              g_OwnShipIconType;
extern double           g_ownship_predictor_minutes;
extern double           g_ownship_HDTpredictor_miles;

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
extern TrackList        *pTrackList;
extern bool             b_inCompressAllCharts;
extern bool             g_bGLexpert;
extern bool             g_bcompression_wait;
extern bool             g_bresponsive;
extern float            g_ChartScaleFactorExp;

float            g_GLMinSymbolLineWidth;
float            g_GLMinCartographicLineWidth;

extern bool             g_fog_overzoom;
extern double           g_overzoom_emphasis_base;
extern bool             g_oz_vector_scale;
extern TCMgr            *ptcmgr;


ocpnGLOptions g_GLOptions;

//    For VBO(s)
bool         g_b_EnableVBO;


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

//      Vertex Buffer Object (VBO) support
PFNGLGENBUFFERSPROC                 s_glGenBuffers;
PFNGLBINDBUFFERPROC                 s_glBindBuffer;
PFNGLBUFFERDATAPROC                 s_glBufferData;
PFNGLDELETEBUFFERSPROC              s_glDeleteBuffers;

#include <wx/arrimpl.cpp>
//WX_DEFINE_OBJARRAY( ArrayOfTexDescriptors );

GLuint g_raster_format = GL_RGB;
long g_tex_mem_used;

bool            b_timeGL;
wxStopWatch     g_glstopwatch;
double          g_gl_ms_per_frame;

int g_tile_size;
int g_uncompressed_tile_size;

bool glChartCanvas::s_b_useScissorTest;
bool glChartCanvas::s_b_useStencil;
bool glChartCanvas::s_b_useStencilAP;
//static int s_nquickbind;


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
#elif defined(__OCPN__ANDROID__)
#define systemGetProcAddress(ADDR) eglGetProcAddress(ADDR)
#else
#define systemGetProcAddress(ADDR) glXGetProcAddress((const GLubyte*)ADDR)
#endif

GenericFunction ocpnGetProcAddress(const char *addr, const char *extension)
{
    char addrbuf[256];
    if(!extension)
        return (GenericFunction)NULL;

#ifndef __OCPN__ANDROID__    
    //  If this is an extension entry point,
    //  We look explicitly in the extensions list to confirm
    //  that the request is actually supported.
    // This may be redundant, but is conservative, and only happens once per session.    
    if(extension && strlen(extension)){
        wxString s_extension(&addr[2], wxConvUTF8);
        wxString s_family;
        s_family = wxString(extension, wxConvUTF8);
        s_extension.Prepend(_T("_"));
        s_extension.Prepend(s_family);

        s_extension.Prepend(_T("GL_"));
        
        if(!QueryExtension( s_extension.mb_str() )){
            return (GenericFunction)NULL;
        }
    }
#endif    
    
    snprintf(addrbuf, sizeof addrbuf, "%s%s", addr, extension);
    return (GenericFunction)systemGetProcAddress(addrbuf);
    
}

bool  b_glEntryPointsSet;

static void GetglEntryPoints( void )
{
    b_glEntryPointsSet = true;
    
    // the following are all part of framebuffer object,
    // according to opengl spec, we cannot mix EXT and ARB extensions
    // (I don't know that it could ever happen, but if it did, bad things would happen)

#ifndef __OCPN__ANDROID__
    const char *extensions[] = {"", "ARB", "EXT", 0 };
#else
    const char *extensions[] = {"OES", 0 };
#endif
    
    unsigned int n_ext = (sizeof extensions) / (sizeof *extensions);

    unsigned int i;
    for(i=0; i<n_ext; i++) {
        if((s_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenFramebuffers", extensions[i])))
            break;
    }

    if(i<n_ext){
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
            
        //VBO
        s_glGenBuffers = (PFNGLGENBUFFERSPROC)
            ocpnGetProcAddress( "glGenBuffers", extensions[i]);
        s_glBindBuffer = (PFNGLBINDBUFFERPROC)
            ocpnGetProcAddress( "glBindBuffer", extensions[i]);
        s_glBufferData = (PFNGLBUFFERDATAPROC)
            ocpnGetProcAddress( "glBufferData", extensions[i]);
        s_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
            ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
            
    }

    //  Retry VBO entry points with all extensions
    if(0 == s_glGenBuffers){
        for( i=0; i<n_ext; i++) {
            if((s_glGenBuffers = (PFNGLGENBUFFERSPROC)ocpnGetProcAddress( "glGenBuffers", extensions[i])) )
                break;
        }
        
        if( i < n_ext ){
            s_glBindBuffer = (PFNGLBINDBUFFERPROC) ocpnGetProcAddress( "glBindBuffer", extensions[i]);
            s_glBufferData = (PFNGLBUFFERDATAPROC) ocpnGetProcAddress( "glBufferData", extensions[i]);
            s_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
        }
    }
            

#ifndef __OCPN__ANDROID__            
    for(i=0; i<n_ext; i++) {
        if((s_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)
            ocpnGetProcAddress( "glCompressedTexImage2D", extensions[i])))
            break;
    }

    if(i<n_ext){
        s_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)
            ocpnGetProcAddress( "glGetCompressedTexImage", extensions[i]);
    }
#else    
    s_glCompressedTexImage2D =          glCompressedTexImage2D;
#endif
    
}

// This attribute set works OK with vesa software only OpenGL renderer
int attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 8, 0 };
BEGIN_EVENT_TABLE ( glChartCanvas, wxGLCanvas ) EVT_PAINT ( glChartCanvas::OnPaint )
    EVT_ACTIVATE ( glChartCanvas::OnActivate )
    EVT_SIZE ( glChartCanvas::OnSize )
    EVT_MOUSE_EVENTS ( glChartCanvas::MouseEvent )
END_EVENT_TABLE()

glChartCanvas::glChartCanvas( wxWindow *parent ) :
#if !wxCHECK_VERSION(3,0,0)
    wxGLCanvas( parent, wxID_ANY, wxDefaultPosition, wxSize( 256, 256 ),
            wxFULL_REPAINT_ON_RESIZE | wxBG_STYLE_CUSTOM, _T(""), attribs ),
#else
    wxGLCanvas( parent, wxID_ANY, attribs, wxDefaultPosition, wxSize( 256, 256 ),
                        wxFULL_REPAINT_ON_RESIZE | wxBG_STYLE_CUSTOM, _T("") ),
#endif
                        
    m_bsetup( false )
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
    ownship_color = -1;

    m_piano_tex = 0;
    
    m_binPinch = false;
    m_binPan = false;
    m_bpinchGuard = false;
    
    b_timeGL = true;
    m_last_render_time = -1;

    m_LRUtime = 0;
    
    m_tideTex = 0;
    m_currentTex = 0;
    
#ifdef __OCPN__ANDROID__    
    //  Create/connect a dynamic event handler slot for gesture events
    Connect( wxEVT_QT_PANGESTURE,
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::OnEvtPanGesture, NULL, this );
    
    Connect( wxEVT_QT_PINCHGESTURE,
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::OnEvtPinchGesture, NULL, this );

    Connect( wxEVT_TIMER,
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::onGestureTimerEvent, NULL, this );
    
    m_gestureEeventTimer.SetOwner( this, GESTURE_EVENT_TIMER );
    m_bgestureGuard = false;
    
#endif    

    g_glTextureManager = new glTextureManager;
}

glChartCanvas::~glChartCanvas()
{
}

void glChartCanvas::FlushFBO( void ) 
{
    if(m_bsetup)
        BuildFBO();
}


void glChartCanvas::OnActivate( wxActivateEvent& event )
{
    cc1->OnActivate( event );
}

void glChartCanvas::OnSize( wxSizeEvent& event )
{
    if( !g_bopengl ) {
        SetSize( GetSize().x, GetSize().y );
        event.Skip();
        return;
    }

    // this is also necessary to update the context on some platforms
#if !wxCHECK_VERSION(3,0,0)    
    wxGLCanvas::OnSize( event );
#else
    // OnSize can be called with a different OpenGL context (when a plugin uses a different GL context).
    if( m_bsetup && m_pcontext && IsShown()) {
        SetCurrent(*m_pcontext);
    }
#endif
    
    /* expand opengl widget to fill viewport */
    if( GetSize() != cc1->GetSize() ) {
        SetSize( cc1->GetSize() );
        if( m_bsetup )
            BuildFBO();
    }

    GetClientSize( &cc1->m_canvas_width, &cc1->m_canvas_height );
}

void glChartCanvas::MouseEvent( wxMouseEvent& event )
{
    if(cc1->MouseEventOverlayWindows( event ))
        return;

#ifndef __OCPN__ANDROID__
    if(cc1->MouseEventSetup( event )) 
        return;                 // handled, no further action required
        
    bool obj_proc = cc1->MouseEventProcessObjects( event );
    
    if(!obj_proc && !cc1->singleClickEventIsValid ) 
        cc1->MouseEventProcessCanvas( event );
    
    if( !g_btouch )
        cc1->SetCanvasCursor( event );
 
#else

    if(m_bgestureGuard){
        cc1->r_rband.x = 0;             // turn off rubberband temporarily
        return;
    }
        
            
    if(cc1->MouseEventSetup( event, false )) {
        if(!event.LeftDClick()){
            return;                 // handled, no further action required
        }
    }

    cc1->MouseEventProcessObjects( event );
    

#endif    
        
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

    //  In CanvasPanning mode, we will build square POT textures for the FBO backing store
    //  We will make them as large as possible...
    if(g_GLOptions.m_bUseCanvasPanning){
        int rb_x = GetSize().x;
        int rb_y = GetSize().y;
        int i=1;
        while(i < rb_x) i <<= 1;
            rb_x = i;
            
        i=1;
        while(i < rb_y) i <<= 1;
            rb_y = i;
            
        m_cache_tex_x = wxMax(rb_x, rb_y);
        m_cache_tex_y = wxMax(rb_x, rb_y);
        m_cache_tex_x = wxMax(2048, m_cache_tex_x);
        m_cache_tex_y = wxMax(2048, m_cache_tex_y);
    } else {            
        m_cache_tex_x = GetSize().x;
        m_cache_tex_y = GetSize().y;
    }        
        
    ( s_glGenFramebuffers )( 1, &m_fb0 );
    ( s_glGenRenderbuffers )( 1, &m_renderbuffer );

    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );
    

    // initialize color textures
    glGenTextures( 2, m_cache_tex );
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
        
        GLenum depth_format = GL_DEPTH_COMPONENT24;
        
        //      Need to check for availability of 24 bit depth buffer extension on GLES
#ifdef ocpnUSE_GLES
        if( !QueryExtension("GL_OES_depth24") )
            depth_format = GL_DEPTH_COMPONENT16;
#endif        
        
        // initialize depth renderbuffer
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, depth_format,
                                         m_cache_tex_x, m_cache_tex_y );
        int err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> Framebuffer Depth Buffer Storage error:  %08X"), err );
            wxLogMessage(msg);
        }
                
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
        
        err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> Framebuffer Depth Buffer Attach error:  %08X"), err );
            wxLogMessage(msg);
        }
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
    if(g_bSoftwareGL)
        msg.Printf( _T("OpenGL-> Software OpenGL") );
    msg.Printf( _T("OpenGL-> Renderer String: ") );
    msg += m_renderer;
    wxLogMessage( msg );

    #ifdef USE_S57
    if( ps52plib ) ps52plib->SetGLRendererString( m_renderer );
    #endif
    
    char version_string[80];
    strncpy( version_string, (char *) glGetString( GL_VERSION ), 79 );
    msg.Printf( _T("OpenGL-> Version reported:  "));
    m_version = wxString( version_string, wxConvUTF8 );
    msg += m_version;
    wxLogMessage( msg );
    
    const GLubyte *ext_str = glGetString(GL_EXTENSIONS);
    m_extensions = wxString( (const char *)ext_str, wxConvUTF8 );
#ifdef __WXQT__    
    wxLogMessage( _T("OpenGL extensions available: ") );
    wxLogMessage(m_extensions );
#endif    
    
    //  Set the minimum line width
    GLint parms[2];
    glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0] );
    g_GLMinSymbolLineWidth = wxMax(parms[0], 1);
    g_GLMinCartographicLineWidth = wxMax(parms[0], 1);
    
    //    Some GL renderers do a poor job of Anti-aliasing very narrow line widths.
    //    This is most evident on rendered symbols which have horizontal or vertical line segments
    //    Detect this case, and adjust the render parameters.
    
    if( m_renderer.Upper().Find( _T("MESA") ) != wxNOT_FOUND ){
        GLfloat parf;
        glGetFloatv(  GL_SMOOTH_LINE_WIDTH_GRANULARITY, &parf );
        
        g_GLMinSymbolLineWidth = wxMax(((float)parms[0] + parf), 1);
    }
    
    s_b_useScissorTest = true;
    // the radeon x600 driver has buggy scissor test
    if( GetRendererString().Find( _T("RADEON X600") ) != wxNOT_FOUND )
        s_b_useScissorTest = false;

    //  This little hack fixes a problem seen with some Intel 945 graphics chips
    //  We need to not do anything that requires (some) complicated stencil operations.

    bool bad_stencil_code = false;
    if( GetRendererString().Find( _T("Intel") ) != wxNOT_FOUND ) {
        wxLogMessage( _T("OpenGL-> Detected Intel renderer, disabling stencil buffer") );
        bad_stencil_code = true;
    }

    //      And for the lousy Unichrome drivers, too
    if( GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND )
        bad_stencil_code = true;

    //      And for the lousy Mali drivers, too
    if( GetRendererString().Find( _T("Mali") ) != wxNOT_FOUND )
        bad_stencil_code = true;

    //XP  Generic Needs stencil buffer
    //W7 Generic Needs stencil buffer    
//      if( GetRendererString().Find( _T("Generic") ) != wxNOT_FOUND ) {
//          wxLogMessage( _T("OpenGL-> Detected Generic renderer, disabling stencil buffer") );
//          bad_stencil_code = true;
//      }
    
    //          Seen with intel processor on VBox Win7
    if( GetRendererString().Find( _T("Chromium") ) != wxNOT_FOUND ) {
        wxLogMessage( _T("OpenGL-> Detected Chromium renderer, disabling stencil buffer") );
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
    if( stencil && ( sb == 8 ) )
        s_b_useStencil = true;
     
    if( QueryExtension( "GL_ARB_texture_non_power_of_two" ) )
        g_texture_rectangle_format = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_OES_texture_npot" ) )
        g_texture_rectangle_format = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_ARB_texture_rectangle" ) )
        g_texture_rectangle_format = GL_TEXTURE_RECTANGLE_ARB;
    wxLogMessage( wxString::Format(_T("OpenGL-> Texture rectangle format: %x"),
                                   g_texture_rectangle_format));

#ifndef __OCPN__ANDROID__
        //      We require certain extensions to support FBO rendering
        if(!g_texture_rectangle_format)
            m_b_DisableFBO = true;
        
        if(!QueryExtension( "GL_EXT_framebuffer_object" ))
            m_b_DisableFBO = true;
#endif
 
#ifdef __OCPN__ANDROID__
            g_texture_rectangle_format = GL_TEXTURE_2D;
//        m_b_DisableFBO = false;
#endif
        
//    if(b_timeGL)
//        m_b_DisableFBO = true;
    
    GetglEntryPoints();
    
    //  ATI cards do not do glGenerateMipmap very well, or at all.
    if( GetRendererString().Upper().Find( _T("RADEON") ) != wxNOT_FOUND )
        s_glGenerateMipmap = 0;
    if( GetRendererString().Upper().Find( _T("ATI") ) != wxNOT_FOUND )
        s_glGenerateMipmap = 0;

    
    // Intel drivers on Windows may export glGenerateMipmap, but it doesn't work...
#ifdef __WXMSW__
        if( GetRendererString().Upper().Find( _T("INTEL") ) != wxNOT_FOUND )
            s_glGenerateMipmap = 0;
#endif        
            
            

    if( !s_glGenerateMipmap )
        wxLogMessage( _T("OpenGL-> glGenerateMipmap unavailable") );
    
    if( !s_glGenFramebuffers  || !s_glGenRenderbuffers        || !s_glFramebufferTexture2D ||
        !s_glBindFramebuffer  || !s_glFramebufferRenderbuffer || !s_glRenderbufferStorage  ||
        !s_glBindRenderbuffer || !s_glCheckFramebufferStatus  || !s_glDeleteFramebuffers   ||
        !s_glDeleteRenderbuffers )
        m_b_DisableFBO = true;

    g_b_EnableVBO = true;
    if( !s_glBindBuffer || !s_glBufferData || !s_glGenBuffers || !s_glDeleteBuffers )
        g_b_EnableVBO = false;

#ifdef __WXMSW__
    if( GetRendererString().Find( _T("Intel") ) != wxNOT_FOUND ) {
        wxLogMessage( _T("OpenGL-> Detected Windows Intel renderer, disabling Vertexbuffer Objects") );
        g_b_EnableVBO = false;
    }
#endif

#ifdef __OCPN__ANDROID__
    g_b_EnableVBO = false;
#endif

    if(g_b_EnableVBO)
        wxLogMessage( _T("OpenGL-> Using Vertexbuffer Objects") );
    else
        wxLogMessage( _T("OpenGL-> Vertexbuffer Objects unavailable") );
    
    
    //      Can we use the stencil buffer in a FBO?
#ifdef ocpnUSE_GLES 
    m_b_useFBOStencil = QueryExtension( "GL_OES_packed_depth_stencil" );
#else
    m_b_useFBOStencil = QueryExtension( "GL_EXT_packed_depth_stencil" ) == GL_TRUE;
#endif

#ifdef __OCPN__ANDROID__
    m_b_useFBOStencil = false;
#endif
    
    //  On Intel Graphics platforms, don't use stencil buffer at all
    if( bad_stencil_code)    
        s_b_useStencil = false;
    
    g_GLOptions.m_bUseCanvasPanning = false;
#ifdef __OCPN__ANDROID__
    g_GLOptions.m_bUseCanvasPanning = isPlatformCapable(PLATFORM_CAP_FASTPAN);
#endif
        
    //      Maybe build FBO(s)

    BuildFBO();
    
    
    
    
#if 1   /* this test sometimes fails when the fbo still works */
        //  But we need to be ultra-conservative here, so run all the tests we can think of
    
    
    //  But we cannot even run this test on some platforms
    //  So we simply have to declare FBO unavailable
#ifdef __WXMSW__
    if( GetRendererString().Upper().Find( _T("INTEL") ) != wxNOT_FOUND ) {
        if(GetRendererString().Upper().Find( _T("MOBILE") ) != wxNOT_FOUND ){
            wxLogMessage( _T("OpenGL-> Detected Windows Intel Mobile renderer, disabling Frame Buffer Objects") );
            m_b_DisableFBO = true;
            BuildFBO();
        }
    }
#endif
    
    if( m_b_BuiltFBO ) {
        // Check framebuffer completeness at the end of initialization.
        ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );
        
        ( s_glFramebufferTexture2D )
        ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
          g_texture_rectangle_format, m_cache_tex[0], 0 );
        
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

#ifdef __OCPN__ANDROID__
    g_GLOptions.m_bUseCanvasPanning = m_b_BuiltFBO;
    if(g_GLOptions.m_bUseCanvasPanning)
        wxLogMessage( _T("OpenGL-> Using FastCanvas Panning/Zooming") );
    
#endif

    if( m_b_BuiltFBO && !m_b_useFBOStencil )
        s_b_useStencil = false;

    //  If stencil seems to be a problem, force use of depth buffer clipping for Area Patterns
    s_b_useStencilAP = s_b_useStencil & !bad_stencil_code;

    if( m_b_BuiltFBO ) {
        wxLogMessage( _T("OpenGL-> Using Framebuffer Objects") );

        if( m_b_useFBOStencil )
            wxLogMessage( _T("OpenGL-> Using FBO Stencil buffer") );
        else
            wxLogMessage( _T("OpenGL-> FBO Stencil buffer unavailable") );
    } else
        wxLogMessage( _T("OpenGL-> Framebuffer Objects unavailable") );

    if( s_b_useStencil ) wxLogMessage( _T("OpenGL-> Using Stencil buffer clipping") );
    else
        wxLogMessage( _T("OpenGL-> Using Depth buffer clipping") );

    if(s_b_useScissorTest && s_b_useStencil)
        wxLogMessage( _T("OpenGL-> Using Scissor Clipping") );

    /* we upload non-aligned memory */
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    MipMap_ResolveRoutines();
    SetupCompression();

    wxString lwmsg;
    lwmsg.Printf(_T("OpenGL-> Minimum cartographic line width: %4.1f"), g_GLMinCartographicLineWidth);
    wxLogMessage(lwmsg);
    lwmsg.Printf(_T("OpenGL-> Minimum symbol line width: %4.1f"), g_GLMinSymbolLineWidth);
    wxLogMessage(lwmsg);
    
    m_benableFog = true;
    m_benableVScale = true;
#ifdef __OCPN__ANDROID__
    m_benableFog = false;
    m_benableVScale = false;
#endif    
        
    if(!g_bGLexpert)
        g_GLOptions.m_bUseAcceleratedPanning =  !m_b_DisableFBO && m_b_BuiltFBO;
    
    if(1)     // for now upload all levels
    {
        int max_level = 0;
        int tex_dim = g_GLOptions.m_iTextureDimension;
        for(int dim=tex_dim; dim>0; dim/=2)
            max_level++;
        g_mipmap_max_level = max_level - 1;
    }   
}

void glChartCanvas::SetupCompression()
{
    int dim = g_GLOptions.m_iTextureDimension;

#ifdef __WXMSW__    
    if(!::IsProcessorFeaturePresent( PF_XMMI64_INSTRUCTIONS_AVAILABLE )) {
        wxLogMessage( _("OpenGL-> SSE2 Instruction set not available") );
        goto no_compression;
    }
#endif

    g_uncompressed_tile_size = dim*dim*4; // stored as 32bpp in vram
    if(!g_GLOptions.m_bTextureCompression)
        goto no_compression;

    g_raster_format = GL_RGB;
    
    // On GLES, we prefer OES_ETC1 compression, if available
#ifdef ocpnUSE_GLES
    if(QueryExtension("GL_OES_compressed_ETC1_RGB8_texture") && s_glCompressedTexImage2D) {
        g_raster_format = GL_ETC1_RGB8_OES;
    
        wxLogMessage( _("OpenGL-> Using oes etc1 compression") );
    }
#endif
    
    if(GL_RGB == g_raster_format) {
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
        } else {
            wxLogMessage( _("OpenGL-> No Useable compression format found") );
            goto no_compression;
        }
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
                                    g_uncompressed_tile_size / g_tile_size));
    return;

no_compression:
    g_GLOptions.m_bTextureCompression = false;

    g_tile_size = g_uncompressed_tile_size;
    wxLogMessage( wxString::Format( _T("OpenGL-> Not Using compression")));    
}

void glChartCanvas::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );

    Show( g_bopengl );
    if( !g_bopengl ) {
        event.Skip();
        return;
    }

#if wxCHECK_VERSION(2, 9, 0)
    SetCurrent(*m_pcontext);
#else
    SetCurrent();
#endif
    
    if( !m_bsetup ) {
        SetupOpenGL();
        
        #ifdef USE_S57
        if( ps52plib )
            ps52plib->FlushSymbolCaches();
        #endif
        
        m_bsetup = true;
//        g_bDebugOGL = true;
    }

    //  Paint updates may have been externally disabled (temporarily, to avoid Yield() recursion performance loss)
    if(!m_b_paint_enable)
        return;
    //      Recursion test, sometimes seen on GTK systems when wxBusyCursor is activated
    if( m_in_glpaint ) return;
    m_in_glpaint++;
    Render();
    m_in_glpaint--;

}


//   These routines allow reusable coordinates
bool glChartCanvas::HasNormalizedViewPort(const ViewPort &vp)
{
    return vp.m_projection_type == PROJECTION_MERCATOR ||
        vp.m_projection_type == PROJECTION_POLAR ||
        vp.m_projection_type == PROJECTION_EQUIRECTANGULAR;
}

/* adjust the opengl transformation matrix so that
   points plotted using the identity viewport are correct.
   and all rotation translation and scaling is now done in opengl

   a central lat and lon of 0, 0 can be used, however objects on the far side of the world
   can be up to 3 meters off because limited floating point precision, and if the
   points cross 180 longitude then two passes will be required to render them correctly */
#define NORM_FACTOR 4096.0
void glChartCanvas::MultMatrixViewPort(ViewPort &vp, float lat, float lon)
{
    wxPoint2DDouble point;

    switch(vp.m_projection_type) {
    case PROJECTION_MERCATOR:
    case PROJECTION_EQUIRECTANGULAR:
        cc1->GetDoubleCanvasPointPixVP(vp, lat, lon, &point);
        glTranslated(point.m_x, point.m_y, 0);
        glScaled(vp.view_scale_ppm/NORM_FACTOR, vp.view_scale_ppm/NORM_FACTOR, 1);
        break;

    case PROJECTION_POLAR:
        cc1->GetDoubleCanvasPointPixVP(vp, vp.clat > 0 ? 90 : -90, vp.clon, &point);
        glTranslated(point.m_x, point.m_y, 0);
        glRotatef(vp.clon - lon, 0, 0, vp.clat);
        glScalef(vp.view_scale_ppm/NORM_FACTOR, vp.view_scale_ppm/NORM_FACTOR, 1);
        glTranslatef(-vp.pix_width/2, -vp.pix_height/2, 0);
        break;

    default:
        printf("ERROR: Unhandled projection\n");
    }

    double rotation = vp.rotation;

    if (rotation)
        glRotatef(rotation*180/PI, 0, 0, 1);
}

ViewPort glChartCanvas::NormalizedViewPort(const ViewPort &vp, float lat, float lon)
{
    ViewPort cvp = vp;

    switch(vp.m_projection_type) {
    case PROJECTION_MERCATOR:
    case PROJECTION_EQUIRECTANGULAR:
        cvp.clat = lat;
        break;

    case PROJECTION_POLAR:
        cvp.clat = vp.clat > 0 ? 90 : -90; // either north or south polar
        break;

    default:
        printf("ERROR: Unhandled projection\n");
    }

    cvp.clon = lon;
    cvp.view_scale_ppm = NORM_FACTOR;
    cvp.rotation = cvp.skew = 0;
    return cvp;
}

bool glChartCanvas::CanClipViewport(const ViewPort &vp)
{
    return vp.m_projection_type == PROJECTION_MERCATOR ||
        vp.m_projection_type == PROJECTION_EQUIRECTANGULAR;
}

ViewPort glChartCanvas::ClippedViewport(const ViewPort &vp, const LLRegion &region)
{
    if(!CanClipViewport(vp))
        return vp;

    ViewPort cvp = vp;
    LLBBox bbox = region.GetBox();

    /* region.GetBox() will always try to give coordinates from -180 to 180 but in
       the case where the viewport crosses the IDL, we actually want the clipped viewport
       to use coordinates outside this range to ensure the logic in the various rendering
       routines works the same here (with accelerated panning) as it does without, so we
       can adjust the coordinates here */

    if(bbox.GetMaxLon() < cvp.GetBBox().GetMinLon()) {
        bbox.Set(bbox.GetMinLat(), bbox.GetMinLon() + 360,
                 bbox.GetMaxLat(), bbox.GetMaxLon() + 360);
        cvp.SetBBoxDirect(bbox);
    } else if(bbox.GetMaxLon() > cvp.GetBBox().GetMaxLon()) {
        bbox.Set(bbox.GetMinLat(), bbox.GetMinLon() - 360,
                 bbox.GetMaxLat(), bbox.GetMaxLon() - 360);
        cvp.SetBBoxDirect(bbox);
    } else
        cvp.SetBBoxDirect(bbox);

    return cvp;
}


void glChartCanvas::DrawStaticRoutesTracksAndWaypoints( ViewPort &vp )
{
    ocpnDC dc(*this);

    for(wxTrackListNode *node = pTrackList->GetFirst();
        node; node = node->GetNext() ) {
        Track *pTrackDraw = node->GetData();
            /* defer rendering active tracks until later */
        ActiveTrack *pActiveTrack = dynamic_cast<ActiveTrack *>(pTrackDraw);
        if(pActiveTrack && pActiveTrack->IsRunning() )
            continue;

        pTrackDraw->Draw( dc, vp, vp.GetBBox() );
    }
    
    for(wxRouteListNode *node = pRouteList->GetFirst();
        node; node = node->GetNext() ) {
        Route *pRouteDraw = node->GetData();

        if( !pRouteDraw )
            continue;
    
        /* defer rendering active routes until later */
        if( pRouteDraw->IsActive() || pRouteDraw->IsSelected() )
            continue;
    
        /* defer rendering routes being edited until later */
        if( pRouteDraw->m_bIsBeingEdited )
            continue;
    
        pRouteDraw->DrawGL( vp );
    }
        
    /* Waypoints not drawn as part of routes, and not being edited */
    if( vp.GetBBox().GetValid() && pWayPointMan) {
        for(wxRoutePointListNode *pnode = pWayPointMan->GetWaypointList()->GetFirst(); pnode; pnode = pnode->GetNext() ) {
            RoutePoint *pWP = pnode->GetData();
            if( pWP && (!pWP->m_bIsBeingEdited) &&(!pWP->m_bIsInRoute ) )
                pWP->DrawGL( vp );
        }
    }
}

void glChartCanvas::DrawDynamicRoutesTracksAndWaypoints( ViewPort &vp )
{
    ocpnDC dc(*this);

    for(wxTrackListNode *node = pTrackList->GetFirst();
        node; node = node->GetNext() ) {
        Track *pTrackDraw = node->GetData();
            /* defer rendering active tracks until later */
        ActiveTrack *pActiveTrack = dynamic_cast<ActiveTrack *>(pTrackDraw);
        if(pActiveTrack && pActiveTrack->IsRunning() )
            pTrackDraw->Draw( dc, vp, vp.GetBBox() );     // We need Track::Draw() to dynamically render last (ownship) point.
    }

    for(wxRouteListNode *node = pRouteList->GetFirst(); node; node = node->GetNext() ) {
        Route *pRouteDraw = node->GetData();
        
        int drawit = 0;
        if( !pRouteDraw )
            continue;
        
        /* Active routes */
        if( pRouteDraw->IsActive() || pRouteDraw->IsSelected() )
            drawit++;
                
        /* Routes being edited */
        if( pRouteDraw->m_bIsBeingEdited )
            drawit++;
        
        /* Routes Selected */
        if( pRouteDraw->IsSelected() )
            drawit++;
        
        if(drawit) {
            const LLBBox &vp_box = vp.GetBBox(), &test_box = pRouteDraw->GetBBox();
            if(!vp_box.IntersectOut(test_box))
                pRouteDraw->DrawGL( vp );
        }
    }
    
    
    /* Waypoints not drawn as part of routes, which are being edited right now */
    if( vp.GetBBox().GetValid() && pWayPointMan) {
        
        for(wxRoutePointListNode *pnode = pWayPointMan->GetWaypointList()->GetFirst(); pnode; pnode = pnode->GetNext() ) {
            RoutePoint *pWP = pnode->GetData();
            if( pWP && pWP->m_bIsBeingEdited && !pWP->m_bIsInRoute )
                pWP->DrawGL( vp );
        }
    }
    
}

static void GetLatLonCurveDist(const ViewPort &vp, float &lat_dist, float &lon_dist)
{
    // This really could use some more thought, and possibly split at different
    // intervals based on chart skew and other parameters to optimize performance
    switch(vp.m_projection_type) {
    case PROJECTION_TRANSVERSE_MERCATOR:
        lat_dist = 4,   lon_dist = 1;        break;
    case PROJECTION_POLYCONIC:
        lat_dist = 2,   lon_dist = 1;        break;
    case PROJECTION_ORTHOGRAPHIC:
        lat_dist = 2,   lon_dist = 2;        break;
    case PROJECTION_POLAR:
        lat_dist = 180, lon_dist = 1;        break;
    case PROJECTION_STEREOGRAPHIC:
    case PROJECTION_GNOMONIC:
        lat_dist = 2, lon_dist = 1;          break;
    case PROJECTION_EQUIRECTANGULAR:
        // this is suboptimal because we don't care unless there is
        // a change in both lat AND lon (skewed chart)
        lat_dist = 2,   lon_dist = 360;      break;
    default:
        lat_dist = 180, lon_dist = 360;
    }
}

void glChartCanvas::RenderChartOutline( int dbIndex, ViewPort &vp )
{
    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_PLUGIN &&
        !ChartData->IsChartAvailable( dbIndex ) )
        return;
        
    /* quick bounds check */
    LLBBox box;
    ChartData->GetDBBoundingBox( dbIndex, box );
    if(!box.GetValid())
        return;

    
    // Don't draw an outline in the case where the chart covers the entire world */
    if(box.GetLonRange() == 360)
        return;

    LLBBox vpbox = vp.GetBBox();
    
    double lon_bias = 0;
    // chart is outside of viewport lat/lon bounding box
    if( box.IntersectOutGetBias( vp.GetBBox(), lon_bias ) )
        return;

    float plylat, plylon;

    wxColour color;

    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_CM93 )
        color = GetGlobalColor( _T ( "YELO1" ) );
    else if( ChartData->GetDBChartFamily( dbIndex ) == CHART_FAMILY_VECTOR )
        color = GetGlobalColor( _T ( "GREEN2" ) );
    else
        color = GetGlobalColor( _T ( "UINFR" ) );

//    glEnable( GL_BLEND );
    glEnable( GL_LINE_SMOOTH );

    glColor3ub(color.Red(), color.Green(), color.Blue());
    glLineWidth( g_GLMinSymbolLineWidth );

    float lat_dist, lon_dist;
    GetLatLonCurveDist(vp, lat_dist, lon_dist);

    //        Are there any aux ply entries?
    int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex ), nPly;
    int j=0;
    do {
        if(nAuxPlyEntries)
            nPly = ChartData->GetDBAuxPlyPoint( dbIndex, 0, j, 0, 0 );
        else
            nPly = ChartData->GetDBPlyPoint( dbIndex, 0, &plylat, &plylon );

        bool begin = false, sml_valid = false;
        double sml[2];
        float lastplylat = 0.0;
        float lastplylon = 0.0;
        for( int i = 0; i < nPly+1; i++ ) {
            if(nAuxPlyEntries)
                ChartData->GetDBAuxPlyPoint( dbIndex, i%nPly, j, &plylat, &plylon );
            else
                ChartData->GetDBPlyPoint( dbIndex, i%nPly, &plylat, &plylon );

            plylon += lon_bias;

            if(lastplylon - plylon > 180)
                lastplylon -= 360;
            else if(lastplylon - plylon < -180)
                lastplylon += 360;

            int splits;
            if(i==0)
                splits = 1;
            else {
                int lat_splits = floor(fabs(plylat-lastplylat) / lat_dist);
                int lon_splits = floor(fabs(plylon-lastplylon) / lon_dist);
                splits = wxMax(lat_splits, lon_splits) + 1;
            }
                
            double smj[2];
            if(splits != 1) {
                // must perform border interpolation in mercator space as this is what the charts use
                toSM(plylat, plylon, 0, 0, smj+0, smj+1);
                if(!sml_valid)
                    toSM(lastplylat, lastplylon, 0, 0, sml+0, sml+1);
            }

            for(double c=0; c<splits; c++) {
                double lat, lon;
                if(c == splits - 1)
                    lat = plylat, lon = plylon;
                else {
                    double d = (double)(c+1) / splits;
                    fromSM(d*smj[0] + (1-d)*sml[0], d*smj[1] + (1-d)*sml[1], 0, 0, &lat, &lon);
                }

                wxPoint2DDouble s;
                cc1->GetDoubleCanvasPointPix( lat, lon, &s );
                if(!wxIsNaN(s.m_x)) {
                    if(!begin) {
                        begin = true;
                        glBegin(GL_LINE_STRIP);
                    }
                    glVertex2f( s.m_x, s.m_y );
                } else if(begin) {
                    glEnd();
                    begin = false;
                }
            }
            if((sml_valid = splits != 1))
                memcpy(sml, smj, sizeof smj);
            lastplylat = plylat, lastplylon = plylon;
        }

        if(begin)
            glEnd();

    } while(++j < nAuxPlyEntries );                 // There are no aux Ply Point entries

    glDisable( GL_LINE_SMOOTH );
//    glDisable( GL_BLEND );
}

extern void CalcGridSpacing( float WindowDegrees, float& MajorSpacing, float&MinorSpacing );
extern wxString CalcGridText( float latlon, float spacing, bool bPostfix );
void glChartCanvas::GridDraw( )
{
    if( !g_bDisplayGrid ) return;

    ViewPort &vp = cc1->GetVP();

    // TODO: make minor grid work all the time
    bool minorgrid = fabs( vp.rotation ) < 0.0001 &&
        vp.m_projection_type == PROJECTION_MERCATOR;

    double nlat, elon, slat, wlon;
    float lat, lon;
    float gridlatMajor, gridlatMinor, gridlonMajor, gridlonMinor;
    wxCoord w, h;
    
    wxColour GridColor = GetGlobalColor( _T ( "SNDG1" ) );        

    if(!m_gridfont.IsBuilt()){
        wxFont *dFont = FontMgr::Get().GetFont( _("ChartTexts"), 0 );
        wxFont font = *dFont;
        font.SetPointSize(8);
        font.SetWeight(wxFONTWEIGHT_NORMAL);
        
        m_gridfont.Build(font);
    }

    w = vp.pix_width;
    h = vp.pix_height;

    LLBBox llbbox = vp.GetBBox();
    nlat = llbbox.GetMaxLat();
    slat = llbbox.GetMinLat();
    elon = llbbox.GetMaxLon();
    wlon = llbbox.GetMinLon();

    // calculate distance between latitude grid lines
    CalcGridSpacing( vp.view_scale_ppm, gridlatMajor, gridlatMinor );
    CalcGridSpacing( vp.view_scale_ppm, gridlonMajor, gridlonMinor );


    // if it is known the grid has straight lines it's a bit faster
    bool straight_latitudes =
        vp.m_projection_type == PROJECTION_MERCATOR ||
        vp.m_projection_type == PROJECTION_EQUIRECTANGULAR;
    bool straight_longitudes =
        vp.m_projection_type == PROJECTION_MERCATOR ||
        vp.m_projection_type == PROJECTION_POLAR ||
        vp.m_projection_type == PROJECTION_EQUIRECTANGULAR;

    double latmargin;
    if(straight_latitudes)
        latmargin = 0;
    else
        latmargin = gridlatMajor / 2; // don't draw on poles

    slat = wxMax(slat, -90 + latmargin);
    nlat = wxMin(nlat,  90 - latmargin);

    float startlat = ceil( slat / gridlatMajor ) * gridlatMajor;
    float startlon = ceil( wlon / gridlonMajor ) * gridlonMajor;
    float curved_step = wxMin(sqrt(5e-3 / vp.view_scale_ppm), 3);

    // Draw Major latitude grid lines and text
#ifndef __OCPN__ANDROID__
    glEnable( GL_BLEND );
    glEnable( GL_LINE_SMOOTH );
#endif
    
    glColor3ub(GridColor.Red(), GridColor.Green(), GridColor.Blue());

    glLineWidth( g_GLMinSymbolLineWidth );
    
    // First draw the grid then tphe text
    glBegin(GL_LINES);

    // calculate position of first major latitude grid line
    float lon_step = elon - wlon;
    if(!straight_latitudes)
        lon_step /= ceil(lon_step / curved_step);

    for(lat = startlat; lat < nlat; lat += gridlatMajor) {
        wxPoint2DDouble r, s;
        s.m_x = NAN;

        for(lon = wlon; lon < elon+lon_step/2; lon += lon_step) {
            cc1->GetDoubleCanvasPointPix( lat, lon, &r );
            if(!wxIsNaN(s.m_x) && !wxIsNaN(r.m_x)) {
                glVertex2d(s.m_x, s.m_y);
                glVertex2d(r.m_x, r.m_y);
            }
            s = r;
        }
    }

    if(minorgrid) {
        // draw minor latitude grid lines
        for(lat = ceil( slat / gridlatMinor ) * gridlatMinor; lat < nlat; lat += gridlatMinor) {
        
            wxPoint r;
            cc1->GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
            glVertex2i(0, r.y);
            glVertex2i(10, r.y);
            glVertex2i(w - 10, r.y);
            glVertex2i(w, r.y);
            lat = lat + gridlatMinor;
        }
    }

    // draw major longitude grid lines
    float lat_step = nlat - slat;
    if(!straight_longitudes)
        lat_step /= ceil(lat_step / curved_step);

    for(lon = startlon; lon < elon; lon += gridlonMajor) {
        wxPoint2DDouble r, s;
        s.m_x = NAN;
        for(lat = slat; lat < nlat+lat_step/2; lat+=lat_step) {
            cc1->GetDoubleCanvasPointPix( lat, lon, &r );

            if(!wxIsNaN(s.m_x) && !wxIsNaN(r.m_x)) {
                glVertex2d(s.m_x, s.m_y);
                glVertex2d(r.m_x, r.m_y);
                
            }
            s = r;
        }
    }

    if(minorgrid) {
        // draw minor longitude grid lines
        for(lon = ceil( wlon / gridlonMinor ) * gridlonMinor; lon < elon; lon += gridlonMinor) {
            wxPoint r;
            cc1->GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
            glVertex2i(r.x, 0);
            glVertex2i(r.x, 10);
            glVertex2i(r.x, h-10);
            glVertex2i(r.x, h);
        }
    }

    glEnd();

    glDisable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    
    // draw text labels
    glEnable(GL_TEXTURE_2D);
    for(lat = startlat; lat < nlat; lat += gridlatMajor) {
        if( fabs( lat - wxRound( lat ) ) < 1e-5 )
            lat = wxRound( lat );

        wxString st = CalcGridText( lat, gridlatMajor, true ); // get text for grid line
        int iy;
        m_gridfont.GetTextExtent(st, 0, &iy);

        if(straight_latitudes) {
            wxPoint r, s;
            cc1->GetCanvasPointPix( lat, elon, &r );
            cc1->GetCanvasPointPix( lat, wlon, &s );
        
            float x = 0, y = -1;
            y = (float)(r.y*s.x - s.y*r.x) / (s.x - r.x);
            if(y < 0 || y > h) {
                y = h - iy;
                x = (float)(r.x*s.y - s.x*r.y + (s.x - r.x)*y) / (s.y - r.y);
            }

            m_gridfont.RenderString(st, x, y);
        } else {
            // iteratively attempt to find where the latitude line crosses x=0
            wxPoint2DDouble r;
            double y1, y2, lat1, lon1, lat2, lon2;

            y1 = 0, y2 = vp.pix_height;
            double error = vp.pix_width, lasterror;
            int maxiters = 10;
            do {
                cc1->GetCanvasPixPoint(0, y1, lat1, lon1);
                cc1->GetCanvasPixPoint(0, y2, lat2, lon2);

                double y = y1 + (lat1 - lat) * (y2 - y1) / (lat1 - lat2);

                cc1->GetDoubleCanvasPointPix( lat, lon1 + (y1 - y) * (lon2 - lon1) / (y1 - y2), &r);

                if(fabs(y - y1) < fabs(y - y2))
                    y1 = y;
                else
                    y2 = y;

                lasterror = error;
                error = fabs(r.m_x);
                if(--maxiters == 0)
                    break;
            } while(error > 1 && error < lasterror);

            if(error < 1 && r.m_y >= 0 && r.m_y <= vp.pix_height - iy )
                r.m_x = 0;
            else
                // just draw at center longitude
                cc1->GetDoubleCanvasPointPix( lat, vp.clon, &r);

            m_gridfont.RenderString(st, r.m_x, r.m_y);
        }
    }


    for(lon = startlon; lon < elon; lon += gridlonMajor) {
        if( fabs( lon - wxRound( lon ) ) < 1e-5 )
            lon = wxRound( lon );

        wxPoint r, s;
        cc1->GetCanvasPointPix( nlat, lon, &r );
        cc1->GetCanvasPointPix( slat, lon, &s );

        float xlon = lon;
        if( xlon > 180.0 )
            xlon -= 360.0;
        else if( xlon <= -180.0 )
            xlon += 360.0;
        
        wxString st = CalcGridText( xlon, gridlonMajor, false );
        int ix;
        m_gridfont.GetTextExtent(st, &ix, 0);

        if(straight_longitudes) {
            float x = -1, y = 0;
            x = (float)(r.x*s.y - s.x*r.y) / (s.y - r.y);
            if(x < 0 || x > w) {
                x = w - ix;
                y = (float)(r.y*s.x - s.y*r.x + (s.y - r.y)*x) / (s.x - r.x);
            }
            
            m_gridfont.RenderString(st, x, y);
        } else {
            // iteratively attempt to find where the latitude line crosses x=0
            wxPoint2DDouble r;
            double x1, x2, lat1, lon1, lat2, lon2;

            x1 = 0, x2 = vp.pix_width;
            double error = vp.pix_height, lasterror;
            do {
                cc1->GetCanvasPixPoint(x1, 0, lat1, lon1);
                cc1->GetCanvasPixPoint(x2, 0, lat2, lon2);

                double x = x1 + (lon1 - lon) * (x2 - x1) / (lon1 - lon2);

                cc1->GetDoubleCanvasPointPix( lat1 + (x1 - x) * (lat2 - lat1) / (x1 - x2), lon, &r);

                if(fabs(x - x1) < fabs(x - x2))
                    x1 = x;
                else
                    x2 = x;

                lasterror = error;
                error = fabs(r.m_y);
            } while(error > 1 && error < lasterror);

            if(error < 1 && r.m_x >= 0 && r.m_x <= vp.pix_width - ix)
                r.m_y = 0;
            else
                // failure, instead just draw the text at center latitude
                cc1->GetDoubleCanvasPointPix( wxMin(wxMax(vp.clat, slat), nlat), lon, &r);

            m_gridfont.RenderString(st, r.m_x, r.m_y);
        }
    }

    glDisable(GL_TEXTURE_2D);

    glDisable( GL_BLEND );
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
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
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

    int drawit = 0;
    //    Is ship in Vpoint?
    if( cc1->GetVP().GetBBox().Contains( gLat,  gLon ) )
        drawit++;                             // yep

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
    cog_rad += (float)PI;

    float lpp = sqrtf( powf( (float) (lPredPoint.x - lShipMidPoint.x), 2) +
                       powf( (float) (lPredPoint.y - lShipMidPoint.y), 2) );

    //    Is predicted point in the VPoint?
    if( cc1->GetVP().GetBBox().Contains( pred_lat,  pred_lon ) )
        drawit++;      // yep

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
    
    cc1->GetCanvasPointPix( osd_head_lat, osd_head_lon, &osd_head_point );

    float icon_rad = atan2f( (float) ( osd_head_point.y - lShipMidPoint.y ),
                             (float) ( osd_head_point.x - lShipMidPoint.x ) );
    icon_rad += (float)PI;

    if( pSog < 0.2 ) icon_rad = ( ( icon_hdt + 90. ) * PI / 180. ) + cc1->GetVP().rotation;

//    Calculate ownship Heading pointer as a predictor
    double hdg_pred_lat, hdg_pred_lon;

    ll_gc_ll( gLat, gLon, icon_hdt, g_ownship_HDTpredictor_miles, &hdg_pred_lat,
              &hdg_pred_lon );
    
    cc1->GetCanvasPointPix( hdg_pred_lat, hdg_pred_lon, &lHeadPoint );

    //    Is head predicted point in the VPoint?
    if( cc1->GetVP().GetBBox().Contains( hdg_pred_lat,  hdg_pred_lon ) )
        drawit++;                     // yep

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

    //    Another draw test ,based on pixels, assuming the ship icon is a fixed nominal size
    //    and is just barely outside the viewport        ....
    wxBoundingBox bb_screen( 0, 0, cc1->GetVP().pix_width, cc1->GetVP().pix_height );
    if( bb_screen.PointInBox( lShipMidPoint, 20 ) ) drawit++;

    // And two more tests to catch the case where COG/HDG line crosses the screen,
    // but ownship and pred point are both off

    LLBBox box;
    box.SetFromSegment(gLon, gLat, pred_lon, pred_lat);
    if( !cc1->GetVP().GetBBox().IntersectOut( box ) )
        drawit++;
    box.SetFromSegment(gLon, gLat, hdg_pred_lon, hdg_pred_lat);
    if( !cc1->GetVP().GetBBox().IntersectOut(box))
        drawit++;
    
    //    Do the draw if either the ship or prediction is within the current VPoint
    if( !drawit )
        return;

    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_POLYGON_SMOOTH );
    glEnableClientState(GL_VERTEX_ARRAY);
    
    int img_height;

    if( cc1->GetVP().chart_scale > 300000 )             // According to S52, this should be 50,000
    {
        float scale =  g_ChartScaleFactorExp;
        
        const int v = 12;
        // start with cross
        float vertexes[4*v+8] = {-12, 0, 12, 0, 0, -12, 0, 12};

        // build two concentric circles
        for( int i=0; i<2*v; i+=2) {
            float a = i * (float)PI / v;
            float s = sinf( a ), c = cosf( a );
            vertexes[i+8] =  10 * s * scale;
            vertexes[i+9] =  10 * c * scale;
            vertexes[i+2*v+8] = 6 * s * scale;
            vertexes[i+2*v+9] = 6 * c * scale;
        }

        // apply translation
        for( int i=0; i<4*v+8; i+=2) {
            vertexes[i] += lShipMidPoint.x;
            vertexes[i+1] += lShipMidPoint.y;
        }

        glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), vertexes);

        wxColour c;
        if( SHIP_NORMAL != cc1->m_ownship_state ) {
            c = GetGlobalColor( _T ( "YELO1" ) );

            glColor4ub(c.Red(), c.Green(), c.Blue(), 255);
            glDrawArrays(GL_TRIANGLE_FAN, 4, v);
        }

        glLineWidth( 2 );
        c = cc1->PredColor();
        glColor4ub(c.Red(), c.Green(), c.Blue(), 255);

        glDrawArrays(GL_LINE_LOOP, 4, v);
        glDrawArrays(GL_LINE_LOOP, 4+v, v);

        glDrawArrays(GL_LINES, 0, 4);

        img_height = 20;
    } else {
        int draw_color = SHIP_INVALID;
        if( SHIP_NORMAL == cc1->m_ownship_state )
            draw_color = SHIP_NORMAL;
        else if( SHIP_LOWACCURACY == cc1->m_ownship_state )
            draw_color = SHIP_LOWACCURACY;

        if(!ownship_tex || (draw_color != ownship_color)) { /* initial run, create texture for ownship,
                              also needed at colorscheme changes (not implemented) */
                              
            ownship_color = draw_color;
            
            if(ownship_tex)
                glDeleteTextures(1, &ownship_tex);
            
            glGenTextures( 1, &ownship_tex );
            glBindTexture(GL_TEXTURE_2D, ownship_tex);

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

            wxImage image;
            if(cc1->m_pos_image_user) {
                switch (draw_color) {
                    case SHIP_INVALID: image = *cc1->m_pos_image_user_grey; break;
                    case SHIP_NORMAL: image = *cc1->m_pos_image_user; break;
                    case SHIP_LOWACCURACY: image = *cc1->m_pos_image_user_yellow; break;
                }
            }
            else {
                switch (draw_color) {
                    case SHIP_INVALID: image = *cc1->m_pos_image_grey; break;
                    case SHIP_NORMAL: image = *cc1->m_pos_image_red; break;
                    case SHIP_LOWACCURACY: image = *cc1->m_pos_image_yellow; break;
                }
            }
                
            int w = image.GetWidth(), h = image.GetHeight();
            int glw = NextPow2(w), glh = NextPow2(h);
            ownship_size = wxSize(w, h);
            ownship_tex_size = wxSize(glw, glh);
            
            unsigned char *d = image.GetData();
            unsigned char *a = image.GetAlpha();
            unsigned char *e = new unsigned char[4 * w * h];
            
            if(d && e && a){
                for( int p = 0; p < w*h; p++ ) {
                    e[4*p+0] = d[3*p+0];
                    e[4*p+1] = d[3*p+1];
                    e[4*p+2] = d[3*p+2];
                    e[4*p+3] = a[p];
                }
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
        float scale_factor_y = g_ChartScaleFactorExp, scale_factor_x = g_ChartScaleFactorExp;
        int ownShipWidth = 22; // Default values from s_ownship_icon
        int ownShipLength= 84;
        lShipMidPoint = lGPSPoint;

        if( g_n_ownship_beam_meters > 0.0 &&
            g_n_ownship_length_meters > 0.0 &&
            g_OwnShipIconType == 1 )
        {            
            ownShipWidth = ownship_size.x;
            ownShipLength= ownship_size.y;
        }

        if( g_OwnShipIconType != 0 )
            cc1->ComputeShipScaleFactor
                (icon_hdt, ownShipWidth, ownShipLength, lShipMidPoint,
                 GPSOffsetPixels, lGPSPoint, scale_factor_x, scale_factor_y);

        glEnable(GL_BLEND);

        glEnableClientState(GL_VERTEX_ARRAY);

        int x = lShipMidPoint.x, y = lShipMidPoint.y;
        glPushMatrix();
        glTranslatef(x, y, 0);

        float deg = 180/PI * ( icon_rad - PI/2 );
        glRotatef(deg, 0, 0, 1);

        
        if((g_ChartScaleFactorExp > 1.0) && ( g_OwnShipIconType < 2 )){
            scale_factor_x = (log(g_ChartScaleFactorExp) + 1.0) * 1.0;   // soften the scale factor a bit
            scale_factor_y = (log(g_ChartScaleFactorExp) + 1.0) * 1.0;
        }
        
        glScalef(scale_factor_x, scale_factor_y, 1);

        if( g_OwnShipIconType < 2 ) { // Bitmap

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, ownship_tex);
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            
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

            static const GLint s_ownship_icon[] = { 5, -42, 11, -28, 11, 42, -11, 42,
                                                  -11, -28, -5, -42, -11, 0, 11, 0,
                                                  0, 42, 0, -42       };

            glVertexPointer(2, GL_INT, 2*sizeof(GLint), s_ownship_icon);
            glDrawArrays(GL_POLYGON, 0, 6);

            glColor4ub(0, 0, 0, 255);
            glLineWidth(1);

            glDrawArrays(GL_LINE_LOOP, 0, 6);
            glDrawArrays(GL_LINES, 6, 4);
        }
        glPopMatrix();

        img_height = ownShipLength * scale_factor_y;
        
        //      Reference point, where the GPS antenna is
        int circle_rad = 3;
        if( cc1->m_pos_image_user ) circle_rad = 1;
               
        float cx = lGPSPoint.x, cy = lGPSPoint.y;
        // store circle coordinates at compile time
        const int v = 12;
        float circle[4*v];
        for( int i=0; i<2*v; i+=2) {
            float a = i * (float)PI / v;
            float s = sinf( a ), c = cosf( a );
            circle[i+0] = cx + (circle_rad+1) * s;
            circle[i+1] = cy + (circle_rad+1) * c;
            circle[i+2*v] = cx + circle_rad * s;
            circle[i+2*v+1] = cy + circle_rad * c;
        }

        glVertexPointer(2, GL_FLOAT, 2*sizeof(float), circle);

        glColor4ub(0, 0, 0, 255);
        glDrawArrays(GL_TRIANGLE_FAN, 0, v);
        glColor4ub(255, 255, 255, 255);
        glDrawArrays(GL_TRIANGLE_FAN, v, v);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POLYGON_SMOOTH );
    glDisable(GL_BLEND);

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

//    if( active_route ) active_route->DrawGL( vp, region );
//    if( g_pActiveTrack ) g_pActiveTrack->Draw( dc, vp );
//    if( cc1->m_pSelectedRoute ) cc1->m_pSelectedRoute->DrawGL( vp, region );

    GridDraw( );

    if( g_pi_manager ) {
        g_pi_manager->SendViewPortToRequestingPlugIns( vp );
        g_pi_manager->RenderAllGLCanvasOverlayPlugIns( NULL, vp );
    }

    // all functions called with cc1-> are still slow because they go through ocpndc
    AISDrawAreaNotices( dc );

    cc1->DrawAnchorWatchPoints( dc );
    AISDraw( dc );
    ShipDraw( dc );
    cc1->AlertDraw( dc );

    cc1->RenderRouteLegs( dc );
    cc1->ScaleBarDraw( dc );
#ifdef USE_S57
    s57_DrawExtendedLightSectors( dc, cc1->VPoint, cc1->extendedSectorLegs );
#endif
}

void glChartCanvas::DrawChartBar( ocpnDC &dc )
{
#if 0
    // this works but is inconsistent across drivers and really slow if there are icons
    g_Piano->Paint(cc1->m_canvas_height - g_Piano->GetHeight(), dc);
#else
    g_Piano->DrawGL(cc1->m_canvas_height - g_Piano->GetHeight());
#endif
}

void glChartCanvas::DrawQuiting()
{
    GLubyte pattern[8][8];
    for( int y = 0; y < 8; y++ )
        for( int x = 0; x < 8; x++ ) 
            pattern[y][x] = (y == x) * 255;

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glBindTexture(GL_TEXTURE_2D, 0);

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 8, 8,
                  0, GL_ALPHA, GL_UNSIGNED_BYTE, pattern );
    glColor3f( 0, 0, 0 );

    float x = GetSize().x, y = GetSize().y;
    float u = x / 8, v = y / 8;

    glBegin( GL_QUADS );
    glTexCoord2f(0, 0); glVertex2f( 0, 0 );
    glTexCoord2f(0, v); glVertex2f( 0, y );
    glTexCoord2f(u, v); glVertex2f( x, y );
    glTexCoord2f(u, 0); glVertex2f( x, 0 );
    glEnd();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void glChartCanvas::DrawCloseMessage(wxString msg)
{
    if(1){
        
        wxFont *pfont = FontMgr::Get().FindOrCreateFont(12, wxFONTFAMILY_DEFAULT,
                                                        wxFONTSTYLE_NORMAL,
                                                        wxFONTWEIGHT_NORMAL);
        
        TexFont texfont;
        
        texfont.Build(*pfont);
        int w, h;
        texfont.GetTextExtent( msg, &w, &h);
        h += 2;
        int yp = cc1->GetVP().pix_height/2;
        int xp = (cc1->GetVP().pix_width - w)/2;
        
        glColor3ub( 243, 229, 47 );
        
        glBegin(GL_QUADS);
        glVertex2i(xp, yp);
        glVertex2i(xp+w, yp);
        glVertex2i(xp+w, yp+h);
        glVertex2i(xp, yp+h);
        glEnd();
        
        glEnable(GL_BLEND);
        
        glColor3ub( 0, 0, 0 );
        glEnable(GL_TEXTURE_2D);
        texfont.RenderString( msg, xp, yp);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        
    }
}

void glChartCanvas::RotateToViewPort(const ViewPort &vp)
{
    float angle = vp.rotation;

    if( fabs( angle ) > 0.0001 )
    {
        //    Rotations occur around 0,0, so translate to rotate around screen center
        float xt = vp.pix_width / 2.0, yt = vp.pix_height / 2.0;
        
        glTranslatef( xt, yt, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -xt, -yt, 0 );
    }
}

static std::list<double*> combine_work_data;
static void combineCallbackD(GLdouble coords[3],
                             GLdouble *vertex_data[4],
                             GLfloat weight[4], GLdouble **dataOut )
{
    double *vertex = new double[3];
    combine_work_data.push_back(vertex);
    memcpy(vertex, coords, 3*(sizeof *coords)); 
    *dataOut = vertex;
}

void vertexCallbackD(GLvoid *vertex)
{
    glVertex3dv( (GLdouble *)vertex);
}

void beginCallbackD( GLenum mode)
{
    glBegin( mode );
}

void endCallbackD()
{
    glEnd();
}

void glChartCanvas::DrawRegion(ViewPort &vp, const LLRegion &region)
{
    float lat_dist, lon_dist;
    GetLatLonCurveDist(vp, lat_dist, lon_dist);

    GLUtesselator *tobj = gluNewTess();

    gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &vertexCallbackD  );
    gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &beginCallbackD  );
    gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &endCallbackD  );
    gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &combineCallbackD );
    
    gluTessNormal( tobj, 0, 0, 1);
    
    gluTessBeginPolygon(tobj, NULL);
    for(std::list<poly_contour>::const_iterator i = region.contours.begin(); i != region.contours.end(); i++) {
        gluTessBeginContour(tobj);
        contour_pt l = *i->rbegin();
        double sml[2];
        bool sml_valid = false;
        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++) {
            int lat_splits = floor(fabs(j->y - l.y) / lat_dist);
            int lon_splits = floor(fabs(j->x - l.x) / lon_dist);
            int splits = wxMax(lat_splits, lon_splits) + 1;

            double smj[2];
            if(splits != 1) {
                // must perform border interpolation in mercator space as this is what the charts use
                toSM(j->y, j->x, 0, 0, smj+0, smj+1);
                if(!sml_valid)
                    toSM(l.y, l.x, 0, 0, sml+0, sml+1);
            }

            for(int i = 0; i<splits; i++) {
                double lat, lon;
                if(i == splits - 1)
                    lat = j->y, lon = j->x;
                else {
                    double d = (double)(i+1) / splits;
                    fromSM(d*smj[0] + (1-d)*sml[0], d*smj[1] + (1-d)*sml[1], 0, 0, &lat, &lon);
                }
                wxPoint2DDouble q = vp.GetDoublePixFromLL(lat, lon);
                if(wxIsNaN(q.m_x))
                    continue;

                double *p = new double[6];
                p[0] = q.m_x, p[1] = q.m_y, p[2] = 0;
                gluTessVertex(tobj, p, p);
                combine_work_data.push_back(p);
            }
            l = *j;

            if((sml_valid = splits != 1))
                memcpy(sml, smj, sizeof smj);
        }
        gluTessEndContour(tobj);
    }
    gluTessEndPolygon(tobj);

    gluDeleteTess(tobj);

    for(std::list<double*>::iterator i = combine_work_data.begin(); i!=combine_work_data.end(); i++)
        delete [] *i;
    combine_work_data.clear();
}

/* set stencil buffer to clip in this region, and optionally clear using the current color */
void glChartCanvas::SetClipRegion(ViewPort &vp, const LLRegion &region)
{
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

    DrawRegion(vp, region);

    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
}

void glChartCanvas::SetClipRect(const ViewPort &vp, const wxRect &rect, bool b_clear)
{
    /* for some reason this causes an occasional bug in depth mode, I cannot
       seem to solve it yet, so for now: */
    if(s_b_useStencil && s_b_useScissorTest) {
        wxRect vp_rect(0, 0, vp.pix_width, vp.pix_height);
        if(rect != vp_rect) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(rect.x, cc1->m_canvas_height-rect.height-rect.y, rect.width, rect.height);
        }

        if(b_clear) {
            glBegin(GL_QUADS);
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

    // slower way if there is no scissor support
    if(!b_clear)
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

    glBegin(GL_QUADS);
    glVertex2i( rect.x, rect.y );
    glVertex2i( rect.x + rect.width, rect.y );
    glVertex2i( rect.x + rect.width, rect.y + rect.height );
    glVertex2i( rect.x, rect.y + rect.height );
    glEnd();
    
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

#if 0
/* set stencil buffer to clip in this region, and optionally clear using the current color */
void glChartCanvas::SetClipRegion(const ViewPort &vp, const OCPNRegion &region,
                                  bool apply_rotation, bool b_clear )
{
    bool rotation = apply_rotation && (fabs( vp.rotation ) > 0.0001 || ( g_bskew_comp && fabs( vp.skew ) > 0.0001));

#if 1 /* optimization: use scissor test or no test at all if one is not needed */
    /* for some reason this causes an occasional bug in depth mode, I cannot
       seem to solve it yet, so for now: */
    if(!rotation && s_b_useStencil && s_b_useScissorTest) {
        int n_rect = 0;
        for(OCPNRegionIterator clipit( region ); clipit.HaveRects() && n_rect < 2; clipit.NextRect())
            n_rect++;

        if(n_rect == 1) {
            wxRect rect = OCPNRegionIterator( region ).GetRect();
            if(rect == vp.rv_rect) {
                /* no actual clipping need be done, common case */
            } else {
                glEnable(GL_SCISSOR_TEST);
                glScissor(rect.x, rect.y, rect.width, rect.height);
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

    if(rotation) {
        glPushMatrix();
        glChartCanvas::RotateToViewPort( vp );
    }

    //    Decompose the region into rectangles, and draw as quads
    OCPNRegionIterator clipit( region );
    glBegin( GL_QUADS );
    while( clipit.HaveRects() ) {
        wxRect rect = clipit.GetRect();
        
        glVertex2i( rect.x, rect.y );
        glVertex2i( rect.x + rect.width, rect.y );
        glVertex2i( rect.x + rect.width, rect.y + rect.height );
        glVertex2i( rect.x, rect.y + rect.height );
        
        clipit.NextRect();
    }
    glEnd();

    if(rotation)
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
#endif

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

void glChartCanvas::RenderRasterChartRegionGL( ChartBase *chart, ViewPort &vp, LLRegion &region )
{
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );
    if( !pBSBChart ) return;

    if(b_inCompressAllCharts) return; // don't want multiple texfactories to exist
    
    double scalefactor = pBSBChart->GetRasterScaleFactor(vp);

    //    Look for the texture factory for this chart
    wxString key = chart->GetFullPath();
    glTexFactory *pTexFact;
    ChartPathHashTexfactType &hash = g_glTextureManager->m_chart_texfactory_hash;
    ChartPathHashTexfactType::iterator ittf = hash.find( key );
    
    //    Not Found ?
    if( ittf == hash.end() )
        hash[key] = new glTexFactory(chart, g_raster_format);
    
    pTexFact = hash[key];
    pTexFact->SetLRUTime(++m_LRUtime);
    
    // for small scales, don't use normalized coordinates for accuracy (difference is up to 3 meters error)
    bool use_norm_vp = glChartCanvas::HasNormalizedViewPort(vp) && pBSBChart->GetPPM() < 1;
    pTexFact->PrepareTiles(vp, use_norm_vp, pBSBChart);

    //    For underzoom cases, we will define the textures as having their base levels
    //    equivalent to a level "n" mipmap, where n is calculated, and is always binary
    //    This way we can avoid loading much texture memory

    int base_level;
    if(vp.m_projection_type == PROJECTION_MERCATOR &&
       chart->GetChartProjectionType() == PROJECTION_MERCATOR) {
        base_level = log(scalefactor) / log(2.0);

        if(base_level < 0) /* for overzoom */
            base_level = 0;
        if(base_level > g_mipmap_max_level)
            base_level = g_mipmap_max_level;
    } else
        base_level = 0; // base level should be computed per tile, for now load all

    /* setup opengl parameters */
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if(use_norm_vp) {
        glPushMatrix();
        double lat, lon;
        pTexFact->GetCenter(lat, lon);
        MultMatrixViewPort(vp, lat, lon);
    }

    LLBBox box = region.GetBox();
    int numtiles;
    glTexTile **tiles = pTexFact->GetTiles(numtiles);
    for(int i = 0; i<numtiles; i++) {
        glTexTile *tile = tiles[i];
        if(region.IntersectOut(tile->box)) {
            
            /*   user setting is in MB while we count exact bytes */
            bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;
            if( bGLMemCrunch)
                pTexFact->DeleteTexture( tile->rect );
        } else {
            bool texture = pTexFact->PrepareTexture( base_level, tile->rect, global_color_scheme );
            if(!texture) { // failed to load, draw red
                glDisable(GL_TEXTURE_2D);
                glColor3f(1, 0, 0);
            }

            float *coords;
            if(use_norm_vp)
                coords = tile->m_coords;
            else {
                coords = new float[2 * tile->m_ncoords];
                for(int i=0; i<tile->m_ncoords; i++) {
                    wxPoint2DDouble p = vp.GetDoublePixFromLL(tile->m_coords[2*i+0],
                                                              tile->m_coords[2*i+1]);
                    coords[2*i+0] = p.m_x;
                    coords[2*i+1] = p.m_y;
                }
            }

            glTexCoordPointer(2, GL_FLOAT, 2*sizeof(GLfloat), tile->m_texcoords);
            glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), coords);
            glDrawArrays(GL_QUADS, 0, tile->m_ncoords);

            if(!texture)
                glEnable(GL_TEXTURE_2D);

            if(!use_norm_vp)
                delete [] coords;
        }
    }

    glDisable(GL_TEXTURE_2D);

    if(use_norm_vp)
        glPopMatrix();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void glChartCanvas::RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &rect_region )
{
    if( !cc1->m_pQuilt->GetnCharts() || cc1->m_pQuilt->IsBusy() )
        return;

    //  render the quilt
    ChartBase *chart = cc1->m_pQuilt->GetFirstChart();
        
    //  Check the first, smallest scale chart
    if(chart) {
//            if( ! cc1->IsChartLargeEnoughToRender( chart, vp ) )
//            chart = NULL;
    }

    LLRegion region = vp.GetLLRegion(rect_region);

    LLRegion rendered_region;
    while( chart ) {
            
        //  This test does not need to be done for raster charts, since
        //  we can assume that texture binding is acceptably fast regardless of the render region,
        //  and that the quilt zoom methods choose a reasonable reference chart.
        if(chart->GetChartFamily() != CHART_FAMILY_RASTER)
        {
//                if( ! cc1->IsChartLargeEnoughToRender( chart, vp ) ) {
//                    chart = cc1->m_pQuilt->GetNextChart();
//                    continue;
//                }
        }

        QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
        if( pqp->b_Valid ) {
            LLRegion get_region = pqp->ActiveRegion;
            bool b_rendered = false;

            if( !pqp->b_overlay ) {
                get_region.Intersect( region );
                if( !get_region.Empty() ) {
                    if( chart->GetChartFamily() == CHART_FAMILY_RASTER ) {
                        ChartBaseBSB *Patch_Ch_BSB = dynamic_cast<ChartBaseBSB*>( chart );
                        if (Patch_Ch_BSB) {
                            SetClipRegion(vp, pqp->ActiveRegion/*pqp->quilt_region*/);
                            RenderRasterChartRegionGL( chart, vp, get_region );
                            DisableClipRegion();
                            b_rendered = true;
                        }
                    } else if(chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                        RenderNoDTA(vp, get_region);
                        b_rendered = chart->RenderRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                    }
                }
            }

            if(b_rendered) {
//                LLRegion get_region = pqp->ActiveRegion;
//                    get_region.Intersect( Region );  not technically required?
//                rendered_region.Union(get_region);
            }
        }


        chart = cc1->m_pQuilt->GetNextChart();
    }

    //    Render any Overlay patches for s57 charts(cells)
    if( cc1->m_pQuilt->HasOverlays() ) {
        ChartBase *pch = cc1->m_pQuilt->GetFirstChart();
        while( pch ) {
            QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid && pqp->b_overlay && pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                LLRegion get_region = pqp->ActiveRegion;

                get_region.Intersect( region );
#ifdef USE_S57
                if( !get_region.Empty()  ) {
                    s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                    if( Chs57 )
                        Chs57->RenderOverlayRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                }
#endif                
            }

            pch = cc1->m_pQuilt->GetNextChart();
        }
    }

    // Hilite rollover patch
    LLRegion hiregion = cc1->m_pQuilt->GetHiliteRegion();
//    hiregion.Intersect(region);

    if( !hiregion.Empty() ) {
        glEnable( GL_BLEND );

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

        DrawRegion(vp, hiregion);

        glDisable( GL_BLEND );
    }
    cc1->m_pQuilt->SetRenderedVP( vp );

    if(m_bfogit) {
        double scale_factor = vp.ref_scale/vp.chart_scale;
        float fog = ((scale_factor - g_overzoom_emphasis_base) * 255.) / 20.;
        fog = wxMin(fog, 200.);         // Don't blur completely
            
        if( !rendered_region.Empty() ) {
     
            int width = vp.pix_width; 
            int height = vp.pix_height;
                
            // Use MipMap LOD tweaking to produce a blurred, downsampling effect at reasonable speed.

            if( (s_glGenerateMipmap) && (g_texture_rectangle_format == GL_TEXTURE_2D)){       //nPOT texture supported

                //          Capture the rendered screen image to a texture
                glReadBuffer( GL_BACK);
                        
                GLuint screen_capture;
                glGenTextures( 1, &screen_capture );
                        
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, screen_capture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
                glCopyTexSubImage2D(GL_TEXTURE_2D,  0,  0,  0, 0,  0,  width, height);
                    
                glClear(GL_DEPTH_BUFFER_BIT);
                glDisable(GL_DEPTH_TEST);
                        
                //  Build MipMaps 
                int max_mipmap = 3;
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_mipmap );
                        
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, max_mipmap);
                        
                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
                        
                s_glGenerateMipmap(GL_TEXTURE_2D);

                // Render at reduced LOD (i.e. higher mipmap number)
                double bias = fog/70;
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias);
                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
                        

                glBegin(GL_QUADS);
                        
                glTexCoord2f(0 , 1 ); glVertex2i(0,     0);
                glTexCoord2f(0 , 0 ); glVertex2i(0,     height);
                glTexCoord2f(1 , 0 ); glVertex2i(width, height);
                glTexCoord2f(1 , 1 ); glVertex2i(width, 0);
                glEnd ();
                        
                glDeleteTextures(1, &screen_capture);

                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
                glDisable(GL_TEXTURE_2D);
            }
#if 0                    
            else if(scale_factor > 25)  { 
                // must use POT textures
                // and we cannot really trust the value that comes from GL_MAX_TEXTURE_SIZE
                // This method of fogging is very slow, so only activate it if the scale_factor is
                // very large.

                int tex_size = 512;  // reasonable assumption
                int ntx = (width / tex_size) + 1;
                int nty = (height / tex_size) + 1;

                GLuint *screen_capture = new GLuint[ntx * nty];
                glGenTextures( ntx * nty, screen_capture );
    
                // Render at reduced LOD (i.e. higher mipmap number)
                double bias = fog/70;
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS, bias);
                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
                        
                glClear(GL_DEPTH_BUFFER_BIT);
                int max_mipmap = 3;
                        
                for(int i=0 ; i < ntx ; i++){
                    for(int j=0 ; j < nty ; j++){
                                
                        int screen_x = i * tex_size;
                        int screen_y = j * tex_size;
                                
                        glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D, screen_capture[(i * ntx) + j]);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
 
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_mipmap );
                                
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, max_mipmap);
                                
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
                                
    
                        unsigned char *ps = (unsigned char *)malloc( tex_size * tex_size * 3 );
                        glReadPixels(screen_x, screen_y, tex_size, tex_size, GL_RGB, GL_UNSIGNED_BYTE, ps );
                        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex_size, tex_size, 0, GL_RGB, GL_UNSIGNED_BYTE, ps );

                        unsigned char *pd;
                        int dim = tex_size / 2;
                        for( int level = 1 ; level <= max_mipmap ; level++){
                            pd = (unsigned char *) malloc( dim * dim * 3 );
                            HalfScaleChartBits( 2*dim, 2*dim, ps, pd );

<<<<<<< HEAD
                                    MipMap_24( GL_TEXTURE_2D, level, GL_RGB, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, pd );
=======
                            glTexImage2D( GL_TEXTURE_2D, level, GL_RGB, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, pd );
>>>>>>> 90e80c0... Initial projections support commit
                                    
                            free(ps);
                            ps = pd;
                                    
                            dim /= 2;
                        }
                                
                        free(pd);
                    }
                }
                        
                for(int i=0 ; i < ntx ; i++){
                    int ybase =  height - tex_size; 
                    for(int j=0 ; j < nty ; j++){
                                
                        int screen_x = i * tex_size;
                        int screen_y = j * tex_size;
                                
                        glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D, screen_capture[(i * ntx) + j]);
                                
                        double bias = fog/70;
                        glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS, bias);
                        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
                                
                        glBegin(GL_QUADS);
                        glTexCoord2f(0 , 1 ); glVertex2i(screen_x,            ybase);
                        glTexCoord2f(0 , 0 ); glVertex2i(screen_x,            ybase + tex_size);
                        glTexCoord2f(1 , 0 ); glVertex2i(screen_x + tex_size, ybase + tex_size);
                        glTexCoord2f(1 , 1 ); glVertex2i(screen_x + tex_size, ybase);
                        glEnd ();
                        
                        ybase -= tex_size;
                    }
                }
                        
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS, 0);
                glDeleteTextures(ntx * nty, screen_capture);
                glDisable(GL_TEXTURE_2D);
                delete [] screen_capture;
            }
#endif
                    
#if 1
            else if(scale_factor > 20){ 
                // Fogging by alpha blending                
                fog = ((scale_factor - 20) * 255.) / 20.;
            
                glEnable( GL_BLEND );
                    
                fog = wxMin(fog, 150.);         // Don't fog out completely
                    
                wxColour color = cc1->GetFogColor(); 
                glColor4ub( color.Red(), color.Green(), color.Blue(), (int)fog );

                DrawRegion(vp, rendered_region);
                glDisable( GL_BLEND );
            }
#endif                
        }
    }
}

void glChartCanvas::RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region)
{
    ViewPort &vp = cc1->VPoint;

#ifdef USE_S57
    
    // Only for cm93 (not quilted), SetVPParms can change the valid region of the chart
    // we need to know this before rendering the chart so we can compute the background region
    // and nodta regions correctly.  I would prefer to just perform this here (or in SetViewPoint)
    // for all vector charts instead of in their render routine, but how to handle quilted cases?
    if(!vp.b_quilt && Current_Ch->GetChartType() == CHART_TYPE_CM93COMP)
        static_cast<cm93compchart*>( Current_Ch )->SetVPParms( vp );
#endif
        
    LLRegion chart_region;
    if(!vp.b_quilt && (Current_Ch->GetChartType() == CHART_TYPE_PLUGIN) ){
        // We do this the hard way, since PlugIn Raster charts do not understand LLRegion yet...
        double ll[8];
        ChartPlugInWrapper *cpw = dynamic_cast<ChartPlugInWrapper*> ( Current_Ch );
        if( !cpw) return;
        
        cpw->chartpix_to_latlong(0,                     0,              ll+0, ll+1);
        cpw->chartpix_to_latlong(0,                     cpw->GetSize_Y(), ll+2, ll+3);
        cpw->chartpix_to_latlong(cpw->GetSize_X(),      cpw->GetSize_Y(), ll+4, ll+5);
        cpw->chartpix_to_latlong(cpw->GetSize_X(),      0,              ll+6, ll+7);
        
        // for now don't allow raster charts to cross both 0 meridian and IDL (complicated to deal with)
        for(int i=1; i<6; i+=2)
            if(fabs(ll[i] - ll[i+2]) > 180) {
                // we detect crossing idl here, make all longitudes positive
                for(int i=1; i<8; i+=2)
                    if(ll[i] < 0)
                        ll[i] += 360;
                    break;
            }
            
        chart_region = LLRegion(4, ll);
        
    }
    else
        chart_region = vp.b_quilt ? cc1->m_pQuilt->GetFullQuiltRegion() : Current_Ch->GetValidRegion();

    bool world_view = false;
    for(OCPNRegionIterator upd ( rect_region ); upd.HaveRects(); upd.NextRect()) {
        wxRect rect = upd.GetRect();
        LLRegion background_region = vp.GetLLRegion(rect);
        //    Remove the valid chart area to find the region NOT covered by the charts
        background_region.Subtract(chart_region);

        if(!background_region.Empty()) {
            ViewPort cvp = ClippedViewport(vp, background_region);
            RenderWorldChart(dc, cvp, rect, world_view);
        }
    }

    if(vp.b_quilt)
        RenderQuiltViewGL( vp, rect_region );
    else {
        LLRegion region = vp.GetLLRegion(rect_region);
        if( Current_Ch->GetChartFamily() == CHART_FAMILY_RASTER )
            RenderRasterChartRegionGL( Current_Ch, vp, region );
        else if( Current_Ch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
            chart_region.Intersect(region);
            RenderNoDTA(vp, chart_region);
            Current_Ch->RenderRegionViewOnGL( *m_pcontext, vp, rect_region, region );
        } 
    }
        
    for(OCPNRegionIterator upd ( rect_region ); upd.HaveRects(); upd.NextRect()) {
        LLRegion region = vp.GetLLRegion(upd.GetRect()); // could cache this from above
        ViewPort cvp = ClippedViewport(vp, region);
        DrawGroundedOverlayObjects(dc, cvp);
    }
}

void glChartCanvas::RenderNoDTA(ViewPort &vp, const LLRegion &region)
{
    wxColour color = GetGlobalColor( _T ( "NODTA" ) );
    if( color.IsOk() )
        glColor3ub( color.Red(), color.Green(), color.Blue() );
    else
        glColor3ub( 163, 180, 183 );

    DrawRegion(vp, region);
}

void glChartCanvas::RenderNoDTA(ViewPort &vp, ChartBase *chart)
{
    wxColour color = GetGlobalColor( _T ( "NODTA" ) );
    if( color.IsOk() )
        glColor3ub( color.Red(), color.Green(), color.Blue() );
    else
        glColor3ub( 163, 180, 183 );

    int index = -1;
    ChartTableEntry *pt;
    for(int i=0; i<pCurrentStack->nEntry; i++) {
#if 0
        ChartBase *c = OpenChartFromStack(pCurrentStack, i, HEADER_ONLY);
        if(c == chart) {
            index = pCurrentStack->GetDBIndex(i);
            pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( index );
            break;
        }
#else
        int j = pCurrentStack->GetDBIndex(i);
        pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( j );
        if(pt->GetpsFullPath()->IsSameAs(chart->GetFullPath())){
            index = j;
            break;
        }
#endif
    }

    if(index == -1)
        return;

    if( ChartData->GetDBChartType( index ) != CHART_TYPE_CM93COMP ) {
        // Maybe it's a good idea to cache the glutesselator results to improve performance
        LLRegion region(pt->GetnPlyEntries(), pt->GetpPlyTable());
        DrawRegion(vp, region);
    } else {
        wxRect rect(0, 0, vp.pix_width, vp.pix_height);
        int x1 = rect.x, y1 = rect.y, x2 = x1 + rect.width, y2 = y1 + rect.height;
        glBegin( GL_QUADS );
        glVertex2i( x1, y1 );
        glVertex2i( x2, y1 );
        glVertex2i( x2, y2 );
        glVertex2i( x1, y2 );
        glEnd();
    }
}

/* render world chart, but only in this rectangle */
void glChartCanvas::RenderWorldChart(ocpnDC &dc, ViewPort &vp, wxRect &rect, bool &world_view)
{
    // set gl color to water
    wxColour water = cc1->pWorldBackgroundChart->water;
    glColor3ub(water.Red(), water.Green(), water.Blue());

    // clear background
    if(!world_view) {
        if(vp.m_projection_type == PROJECTION_ORTHOGRAPHIC) {
            // for this projection, if zoomed out far enough that the earth does
            // not fill the viewport we need to first clear the screen black and
            // draw a blue circle representing the earth

            ViewPort tvp = vp;
            tvp.clat = 0, tvp.clon = 0;
            tvp.rotation = 0;
            wxPoint2DDouble p = tvp.GetDoublePixFromLL( 89.99, 0);
            float w = ((float)tvp.pix_width)/2, h = ((float)tvp.pix_height)/2;
            double world_r = h - p.m_y;
            const float pi_ovr100 = float(M_PI)/100;
            if(world_r*world_r < w*w + h*h) {
                glClear( GL_COLOR_BUFFER_BIT );

                glBegin(GL_TRIANGLE_FAN);
                float w = ((float)vp.pix_width)/2, h = ((float)vp.pix_height)/2;
                for(float theta = 0; theta < 2*M_PI+.01f; theta+=pi_ovr100)
                    glVertex2f(w + world_r*sinf(theta), h + world_r*cosf(theta));
                glEnd();

                world_view = true;
            }
        } else if(vp.m_projection_type == PROJECTION_EQUIRECTANGULAR) {
            // for this projection we will draw black outside of the earth (beyond the pole)
            glClear( GL_COLOR_BUFFER_BIT );

            wxPoint2DDouble p[4] = {
                vp.GetDoublePixFromLL( 90, vp.clon - 170 ),
                vp.GetDoublePixFromLL( 90, vp.clon + 170 ),
                vp.GetDoublePixFromLL(-90, vp.clon + 170 ),
                vp.GetDoublePixFromLL(-90, vp.clon - 170 )};

            glBegin(GL_QUADS);
            for(int i = 0; i<4; i++)
                glVertex2f(p[i].m_x, p[i].m_y);
            glEnd();

            world_view = true;
        }

        if(!world_view) {
            int x1 = rect.x, y1 = rect.y, x2 = x1 + rect.width, y2 = y1 + rect.height;
            glBegin( GL_QUADS );
            glVertex2i( x1, y1 );
            glVertex2i( x2, y1 );
            glVertex2i( x2, y2 );
            glVertex2i( x1, y2 );
            glEnd();
        }
    }

    cc1->pWorldBackgroundChart->RenderViewOnDC( dc, vp );
}

/* these are the overlay objects which move with the charts and
   are not frequently updated (not ships etc..) 

   many overlay objects are fixed to a geographical location or
   grounded as opposed to the floating overlay objects. */
void glChartCanvas::DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp)
{
    cc1->RenderAllChartOutlines( dc, vp );

    DrawStaticRoutesTracksAndWaypoints( vp );

    if( cc1->m_bShowTide ) {
        LLBBox bbox = vp.GetBBox();

        // enlarge the bbox by half the width of the tide bitmap so that accelerated panning works
        if(CanClipViewport(vp))
            bbox.EnLarge(cc1->m_bmTideDay.GetWidth()/2 / vp.view_scale_ppm / 111274.96299695624);

        DrawGLTidesInBBox( dc, bbox );
    }
    
    if( cc1->m_bShowCurrent )
        DrawGLCurrentsInBBox( dc, vp.GetBBox() );

    DisableClipRegion();
}


void glChartCanvas::DrawGLTidesInBBox(ocpnDC& dc, LLBBox& BBox)
{
    // At small scale, we render the Tide icon as a texture for best performance
    if( cc1->GetVP().chart_scale > 500000 ) {
        
        // Prepare the texture if necessary
        
        if(!m_tideTex){
            wxBitmap bmp = cc1->GetTideBitmap();
            if(!bmp.Ok())
                return;
            
            wxImage image = bmp.ConvertToImage();
            int w = image.GetWidth(), h = image.GetHeight();
            
            int tex_w, tex_h;
            if(g_texture_rectangle_format == GL_TEXTURE_2D)
                tex_w = w, tex_h = h;
            else
                tex_w = NextPow2(w), tex_h = NextPow2(h);
            
            m_tideTexWidth = tex_w;
            m_tideTexHeight = tex_h;
            
            unsigned char *d = image.GetData();
            unsigned char *a = image.GetAlpha();
                
            unsigned char mr, mg, mb;
            image.GetOrFindMaskColour( &mr, &mg, &mb );
                
            unsigned char *e = new unsigned char[4 * w * h];
            if(e && d){
                for( int y = 0; y < h; y++ )
                    for( int x = 0; x < w; x++ ) {
                        unsigned char r, g, b;
                        int off = ( y * image.GetWidth() + x );
                        r = d[off * 3 + 0];
                        g = d[off * 3 + 1];
                        b = d[off * 3 + 2];
                            
                        e[off * 4 + 0] = r;
                        e[off * 4 + 1] = g;
                        e[off * 4 + 2] = b;
                            
                        e[off * 4 + 3] =
                        a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                    }
            }

            
            glGenTextures( 1, &m_tideTex );
            
            glBindTexture(GL_TEXTURE_2D, m_tideTex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
            
            if(g_texture_rectangle_format == GL_TEXTURE_2D)
                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, e );
            else {
                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
                glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, e );
            }
            
            delete [] e;
        }
        
        // Texture is ready
        
        glBindTexture( GL_TEXTURE_2D, m_tideTex);
        glEnable( GL_TEXTURE_2D );
        glEnable(GL_BLEND);
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
        
        for( int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++ ) {
            const IDX_entry *pIDX = ptcmgr->GetIDX_entry( i );
            
            char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
            if( ( type == 't' ) || ( type == 'T' ) )  // only Tides
            {
                double lon = pIDX->IDX_lon;
                double lat = pIDX->IDX_lat;
                
                if( BBox.Contains( lat,  lon ) ) {
                    wxPoint r;
                    cc1->GetCanvasPointPix( lat, lon, &r );
      
                    float xp = r.x;
                    float yp = r.y;
        
                    glBegin( GL_QUADS );
                    glTexCoord2f( 0,  0 );  glVertex2f( xp - m_tideTexWidth/2,  yp - m_tideTexHeight/2 );
                    glTexCoord2f( 0,  1 );  glVertex2f( xp - m_tideTexWidth/2,  yp + m_tideTexHeight/2);
                    glTexCoord2f( 1,  1 );  glVertex2f( xp + m_tideTexWidth/2,  yp + m_tideTexHeight/2 );
                    glTexCoord2f( 1,  0 );  glVertex2f( xp + m_tideTexWidth/2,  yp - m_tideTexHeight/2 );
                    glEnd();
                }
            } // type 'T"
        }       //loop
            
            
        glDisable( GL_TEXTURE_2D );
        glDisable(GL_BLEND);
        glBindTexture( GL_TEXTURE_2D, 0);
    }
    else
        cc1->DrawAllTidesInBBox( dc, BBox );
    cc1->RebuildTideSelectList(BBox);
}

void glChartCanvas::DrawGLCurrentsInBBox(ocpnDC& dc, LLBBox& BBox)
{
    cc1->DrawAllCurrentsInBBox(dc, BBox);
    cc1->RebuildCurrentSelectList(BBox);
}


void glChartCanvas::SetColorScheme(ColorScheme cs)
{
    glDeleteTextures(1, &m_tideTex);
    glDeleteTextures(1, &m_currentTex);
    m_tideTex = 0;
    m_currentTex = 0;
    ownship_color = -1;
    
}



int n_render;
void glChartCanvas::Render()
{
    if( !m_bsetup ||
        ( cc1->VPoint.b_quilt && cc1->m_pQuilt && !cc1->m_pQuilt->IsComposed() ) ||
        ( !cc1->VPoint.b_quilt && !Current_Ch ) ) {
#ifdef __WXGTK__  // for some reason in gtk, a swap is needed here to get an initial screen update
            SwapBuffers();
#endif
            return;
        }

    m_last_render_time = wxDateTime::Now().GetTicks();

    // we don't care about jobs that are now off screen
    // clear out and it will be repopulated during render
    if(g_GLOptions.m_bTextureCompression &&
       !g_GLOptions.m_bTextureCompressionCaching)
        g_glTextureManager->ClearJobList();

    if(b_timeGL && g_bShowFPS){
        if(n_render % 10){
            glFinish();   
            g_glstopwatch.Start();
        }
    }
    wxPaintDC( this );

    //  If we are in the middle of a fast pan, we don't want the FBO coordinates to be reset
    if(m_binPinch || m_binPan)
        return;
        
    ViewPort VPoint = cc1->VPoint;
    ocpnDC gldc( *this );

    int w, h;
    GetClientSize( &w, &h );

    OCPNRegion screen_region(wxRect(0, 0, VPoint.pix_width, VPoint.pix_height));

    glViewport( 0, 0, (GLint) w, (GLint) h );
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    // set opengl settings that don't normally change
    // this should be able to go in SetupOpenGL, but it's
    // safer here incase a plugin mangles these
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    //  Delete any textures known to the GPU that
    //  belong to charts which will not be used in this render
    //  This is done chart-by-chart...later we will scrub for unused textures
    //  that belong to charts which ARE used in this render, if we need to....

    g_glTextureManager->TextureCrunch(0.8);

    //  If we plan to post process the display, don't use accelerated panning
    double scale_factor = VPoint.ref_scale/VPoint.chart_scale;
    
    m_bfogit = m_benableFog && g_fog_overzoom && (scale_factor > g_overzoom_emphasis_base) && VPoint.b_quilt;
    bool scale_it  =  m_benableVScale && g_oz_vector_scale && (scale_factor > g_overzoom_emphasis_base) && VPoint.b_quilt;
    
    bool bpost_hilite = !cc1->m_pQuilt->GetHiliteRegion( ).Empty(), useFBO = false;
    int sx = GetSize().x;
    int sy = GetSize().y;

    // Try to use the framebuffer object's cache of the last frame
    // to accelerate drawing this frame (if overlapping)
    if(m_b_BuiltFBO && !m_bfogit && !scale_it && !bpost_hilite
       //&& VPoint.tilt == 0 // disabling fbo in tilt mode gives better quality but slower
        ) {
        //  Is this viewpoint the same as the previously painted one?
        bool b_newview = true;

        // If the view is the same we do no updates, 
        // cached texture to the framebuffer
        if(    m_cache_vp.view_scale_ppm == VPoint.view_scale_ppm
               && m_cache_vp.rotation == VPoint.rotation
               && m_cache_vp.clat == VPoint.clat
               && m_cache_vp.clon == VPoint.clon
               && m_cache_vp.IsValid()
               && m_cache_vp.pix_height == VPoint.pix_height
               && m_cache_current_ch == Current_Ch ) {
            b_newview = false;
        }

        if( b_newview ) {
            // enable rendering to texture in framebuffer object
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

            int dx, dy;
            bool accelerated_pan = false;
            if( g_GLOptions.m_bUseAcceleratedPanning && m_cache_vp.IsValid()
                && ( VPoint.m_projection_type == PROJECTION_MERCATOR
                || VPoint.m_projection_type == PROJECTION_EQUIRECTANGULAR )
                && m_cache_vp.pix_height == VPoint.pix_height )
            {
                wxPoint2DDouble c_old = VPoint.GetDoublePixFromLL( VPoint.clat, VPoint.clon );
                wxPoint2DDouble c_new = m_cache_vp.GetDoublePixFromLL( VPoint.clat, VPoint.clon );

//                printf("diff: %f %f\n", c_new.m_y - c_old.m_y, c_new.m_x - c_old.m_x);
                dy = wxRound(c_new.m_y - c_old.m_y);
                dx = wxRound(c_new.m_x - c_old.m_x);

                //   The math below using the previous frame's texture does not really
                //   work for sub-pixel pans.
                //   TODO is to rethink this.
                //   Meanwhile, require the accelerated pans to be whole pixel multiples only.
                //   This is not as bad as it sounds.  Keyboard and mouse pans are whole_pixel moves.
                //   However, autofollow at large scale is certainly not.
                
                double deltax = c_new.m_x - c_old.m_x;
                double deltay = c_new.m_y - c_old.m_y;
                
                bool b_whole_pixel = true;
                if( ( fabs( deltax - dx ) > 1e-2 ) || ( fabs( deltay - dy ) > 1e-2 ) )
                    b_whole_pixel = false;
                    
                accelerated_pan = b_whole_pixel && abs(dx) < m_cache_tex_x && abs(dy) < m_cache_tex_y
                                  && sx == m_cache_tex_x && sy == m_cache_tex_y;
            }

            // do we allow accelerated panning?  can we perform it here?
            if(accelerated_pan && !g_GLOptions.m_bUseCanvasPanning) {
                if((dx != 0) || (dy != 0)){   // Anything to do?
                    m_cache_page = !m_cache_page; /* page flip */

                    /* perform accelerated pan rendering to the new framebuffer */
                    ( s_glFramebufferTexture2D )
                        ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                        g_texture_rectangle_format, m_cache_tex[m_cache_page], 0 );

                    //calculate the new regions to render
                    // add an extra pixel avoid coorindate rounding issues
                    OCPNRegion update_region;

                    if( dy > 0 && dy < VPoint.pix_height)
                        update_region.Union(wxRect( 0, VPoint.pix_height - dy, VPoint.pix_width, dy ));
                    else if(dy < 0)
                        update_region.Union(wxRect( 0, 0, VPoint.pix_width, -dy ));
                            
                    if( dx > 0 && dx < VPoint.pix_width )
                        update_region.Union(wxRect( VPoint.pix_width - dx, 0, dx, VPoint.pix_height ));
                    else if (dx < 0)
                        update_region.Union(wxRect( 0, 0, -dx, VPoint.pix_height ));

                    RenderCharts(gldc, update_region);

                    // using the old framebuffer
                    glBindTexture( g_texture_rectangle_format, m_cache_tex[!m_cache_page] );
                    glEnable( g_texture_rectangle_format );
                    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
                                
                    //    Render the reuseable portion of the cached texture
                                
                    // Render the cached texture as quad to FBO(m_blit_tex) with offsets
                    int x1, x2, y1, y2;

                    int ow = VPoint.pix_width - abs( dx );
                    int oh = VPoint.pix_height - abs( dy );
                    if( dx > 0 )
                        x1 = dx,  x2 = 0;
                    else
                        x1 = 0,   x2 = -dx;
                            
                    if( dy > 0 )
                        y1 = dy,  y2 = 0;
                    else
                        y1 = 0,   y2 = -dy;

                    // normalize to texture coordinates range from 0 to 1
                    float tx1 = x1, tx2 = x1 + ow, ty1 = sy - y1, ty2 = sy - (y1 + oh);
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
                }

                } else { // must redraw the entire screen
                    ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                                g_texture_rectangle_format,
                                                m_cache_tex[m_cache_page], 0 );
                    
                    if(g_GLOptions.m_bUseCanvasPanning) {
                        bool b_reset = false;
                        if( (m_fbo_offsetx < 50) ||
                            ((m_cache_tex_x - (m_fbo_offsetx + sx)) < 50) ||
                            (m_fbo_offsety < 50) ||
                            ((m_cache_tex_y - (m_fbo_offsety + sy)) < 50))
                            b_reset = true;
    
                        if(m_cache_vp.view_scale_ppm != VPoint.view_scale_ppm )
                            b_reset = true;
                        if(!m_cache_vp.IsValid())
                            b_reset = true;
                            
                        if( b_reset ){
                            m_fbo_offsetx = (m_cache_tex_x - GetSize().x)/2;
                            m_fbo_offsety = (m_cache_tex_y - GetSize().y)/2;
                            m_fbo_swidth = sx;
                            m_fbo_sheight = sy;
                            
                            m_canvasregion = OCPNRegion( m_fbo_offsetx, m_fbo_offsety, sx, sy );
                            
                            if(m_cache_vp.view_scale_ppm != VPoint.view_scale_ppm )
                                OCPNPlatform::ShowBusySpinner();
                            
                            RenderCanvasBackingChart(gldc, m_canvasregion);
                        }
                        
                        

                        glPushMatrix();
                        
                        glViewport( m_fbo_offsetx, m_fbo_offsety, (GLint) sx, (GLint) sy );

                        //g_Platform->ShowBusySpinner();
                        RenderCharts(gldc, screen_region);
                        //g_Platform->HideBusySpinner();
                        
    /*                    
                        wxRect rect( 50, 50, cc1->VPoint.rv_rect.width-100, cc1->VPoint.rv_rect.height-100 );
                        glColor3ub(250, 0, 0);
                        
                        glBegin( GL_QUADS );
                        glVertex2f( rect.x,                     rect.y );
                        glVertex2f( rect.x + rect.width,        rect.y );
                        glVertex2f( rect.x + rect.width,        rect.y + rect.height );
                        glVertex2f( rect.x,                     rect.y + rect.height );
                        glEnd();
    */                    
                        glPopMatrix();

                        glViewport( 0, 0, (GLint) sx, (GLint) sy );
                    }
                    else{
                        m_fbo_offsetx = 0;
                        m_fbo_offsety = 0;
                        m_fbo_swidth = sx;
                        m_fbo_sheight = sy;
                        wxRect rect(m_fbo_offsetx, m_fbo_offsety, (GLint) sx, (GLint) sy);
                        RenderCharts(gldc, screen_region);
                    }
                    
                } 
            // Disable Render to FBO
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );
            
        } // newview

        useFBO = true;
    }

    if(VPoint.tilt) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(2*180/PI*atan2((double)h, (double)w), (GLfloat) w/(GLfloat) h, 1, w);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glScalef(1, -1, 1);
        glTranslatef(-w/2, -h/2, -w/2);
        glRotated(VPoint.tilt*180/PI, 1, 0, 0);

        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    }

    if(useFBO) {
        // Render the cached texture as quad to screen
        glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
        glEnable( g_texture_rectangle_format );

        float tx, ty, tx0, ty0, divx, divy;
        
        //  Normalize, or not?
        if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format ){
            divx = divy = 1.0f;
         }
        else{
            divx = m_cache_tex_x;
            divy = m_cache_tex_y;
        }

        tx0 = m_fbo_offsetx/divx;
        ty0 = m_fbo_offsety/divy;
        tx =  (m_fbo_offsetx + m_fbo_swidth)/divx;
        ty =  (m_fbo_offsety + m_fbo_sheight)/divy;
        
        glBegin( GL_QUADS );
        glTexCoord2f( tx0, ty );  glVertex2f( 0,  0 );
        glTexCoord2f( tx,  ty );  glVertex2f( sx, 0 );
        glTexCoord2f( tx,  ty0 ); glVertex2f( sx, sy );
        glTexCoord2f( tx0, ty0 ); glVertex2f( 0,  sy );
        glEnd();

        glDisable( g_texture_rectangle_format );

        m_cache_vp = VPoint;
        m_cache_current_ch = Current_Ch;

        if(VPoint.b_quilt)
            cc1->m_pQuilt->SetRenderedVP( VPoint );
        
    } else          // useFBO
        RenderCharts(gldc, screen_region);

    DrawDynamicRoutesTracksAndWaypoints( VPoint );
        
    // Now draw all the objects which normally move around and are not
    // cached from the previous frame
    DrawFloatingOverlayObjects( gldc );

    // from this point on don't use perspective
    if(VPoint.tilt) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    DrawEmboss(cc1->EmbossDepthScale() );
    DrawEmboss(cc1->EmbossOverzoomIndicator( gldc ) );

    if( cc1->m_pRouteRolloverWin )
        cc1->m_pRouteRolloverWin->Draw(gldc);

    if( cc1->m_pAISRolloverWin )
        cc1->m_pAISRolloverWin->Draw(gldc);

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
    
    if(g_FloatingToolbarDialog && g_FloatingToolbarDialog->m_pRecoverwin ){
        int recoverx, recovery;
        g_FloatingToolbarDialog->m_pRecoverwin->GetPosition( &recoverx, &recovery );
        if( g_FloatingToolbarDialog->m_pRecoverwin->GetBitmap().IsOk())
            gldc.DrawBitmap( g_FloatingToolbarDialog->m_pRecoverwin->GetBitmap(), recoverx, recovery, true);
    }
    
    
#endif
    // render the chart bar
    if(g_bShowChartBar)
        DrawChartBar(gldc);

    if (g_Compass)
        g_Compass->Paint(gldc);
    
    //quiting?
    if( g_bquiting )
        DrawQuiting();
    if( g_bcompression_wait)
        DrawCloseMessage( _("Waiting for raster chart compression thread exit."));

#ifdef __WXMSW__    
     //  MSW OpenGL drivers are generally very unstable.
     //  This helps...   
     glFinish();
#endif    
    
    SwapBuffers();
    if(b_timeGL && g_bShowFPS){
        if(n_render % 10){
            glFinish();
        
            double filter = .05;
        
        // Simple low pass filter
            g_gl_ms_per_frame = g_gl_ms_per_frame * (1. - filter) + ((double)(g_glstopwatch.Time()) * filter);
//            if(g_gl_ms_per_frame > 0)
            //                printf(" OpenGL frame time: %3.0f  %3.0f\n", g_gl_ms_per_frame, 1000./ g_gl_ms_per_frame);
        }
    }

    g_glTextureManager->TextureCrunch(0.8);
    g_glTextureManager->FactoryCrunch(0.6);
    
    cc1->PaintCleanup();
    OCPNPlatform::HideBusySpinner();
    
    n_render++;
}



void glChartCanvas::RenderCanvasBackingChart( ocpnDC dc, OCPNRegion valid_region)
{
 
    glPushMatrix();

    glLoadIdentity();
    
    glOrtho( 0, m_cache_tex_x, m_cache_tex_y, 0, -1, 1 );
    glViewport( 0, 0, (GLint) m_cache_tex_x, (GLint) m_cache_tex_y );
    
    // strategies:
    
    // 1:  Simple clear to color
    
    OCPNRegion texr( 0, 0, m_cache_tex_x,  m_cache_tex_y );
    texr.Subtract(valid_region);
   
    glViewport( 0, 0, (GLint) m_cache_tex_x, (GLint) m_cache_tex_y );
    
    glColor3ub(0, 250, 250);
    
    OCPNRegionIterator upd ( texr );
    while ( upd.HaveRects() )
    {
        wxRect rect = upd.GetRect();
        
        glBegin( GL_QUADS );
        glVertex2f( rect.x,                     rect.y );
        glVertex2f( rect.x + rect.width,        rect.y );
        glVertex2f( rect.x + rect.width,        rect.y + rect.height );
        glVertex2f( rect.x,                     rect.y + rect.height );
        glEnd();
        
        upd.NextRect();
    }        

    // 2:  Render World Background chart
    
    wxRect rtex( 0, 0, m_cache_tex_x,  m_cache_tex_y );
    ViewPort cvp = cc1->GetVP().BuildExpandedVP( m_cache_tex_x,  m_cache_tex_y );

    bool world_view = false;
    RenderWorldChart(dc, cvp, rtex, world_view);

/*    
    dc.SetPen(wxPen(wxColour(254,254,0), 3));
    dc.DrawLine( 0, 0, m_cache_tex_x, m_cache_tex_y);
    
    dc.DrawLine( 0, 0, 0, m_cache_tex_y);
    dc.DrawLine( 0, m_cache_tex_y, m_cache_tex_x, m_cache_tex_y);
    dc.DrawLine( m_cache_tex_x, m_cache_tex_y, m_cache_tex_x, 0);
    dc.DrawLine( m_cache_tex_x, 0, 0, 0);
*/    
  
    // 3:  Use largest scale chart in the current quilt candidate list (which is identical to chart bar)
    //          which covers the entire canvas
#if 0 

int sx = GetSize().x;
int sy = GetSize().y;
//glViewport( m_fbo_offsetx, m_fbo_offsety, (GLint) sx, (GLint) sy );
//glViewport( 0, 0, (GLint) sx, (GLint) sy );


//glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
//glEnable( g_texture_rectangle_format );
//glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );



glColor3ub(250, 0, 250);

    
    glBegin( GL_QUADS );
    glVertex2f( 0,                     0 );
    glVertex2f( 2048,        0 );
    glVertex2f( 2048,        2049 );
    glVertex2f( 0,                     2048 );
    glEnd();
    

    //  Make a ViewPort covering the entire canvas
    wxRect rtex( 0, 0, m_cache_tex_x,  m_cache_tex_y );
    ViewPort cvp = cc1->GetVP().BuildExpandedVP( m_cache_tex_x,  m_cache_tex_y );  //BuildClippedVP(cc1->GetVP(), rtex);

    int dbIndex = -1;
    ArrayOfInts candidates = cc1->GetQuiltCandidatedbIndexArray( );
    for(unsigned int i=0 ; i < candidates.GetCount(); i++){
        int index = candidates.Item(i);
        
        const ChartTableEntry &cte = ChartData->GetChartTableEntry(index);
        OCPNRegion testr = Quilt::GetChartQuiltRegion( cte, cvp );

        OCPNRegion texr( 0, 0, m_cache_tex_x,  m_cache_tex_y );
        texr.Subtract(testr);
        if(texr.IsEmpty()){
            dbIndex = index;
            break;
        }
    }
    
    if(dbIndex >= 0){
        ChartBase *target_chart = ChartData->OpenChartFromDB( dbIndex, FULL_INIT );
        if(target_chart){
            
            
//            OCPNRegion texr( 0, 0, m_cache_tex_x,  m_cache_tex_y );
            OCPNRegion texr( 10, 10, m_cache_tex_x-20,  m_cache_tex_y-20 );
            RenderRasterChartRegionGL( target_chart, cvp, texr, true );
        }
    }

    glBindTexture( g_texture_rectangle_format, 0);
    
#endif

    glPopMatrix();

}


void glChartCanvas::FastPan(int dx, int dy)
{
    int sx = GetSize().x;
    int sy = GetSize().y;
    
    //   ViewPort VPoint = cc1->VPoint;
    //   ViewPort svp = VPoint;
    //   svp.pix_width = svp.rv_rect.width;
    //   svp.pix_height = svp.rv_rect.height;
    
    //   OCPNRegion chart_get_region( 0, 0, cc1->VPoint.rv_rect.width, cc1->VPoint.rv_rect.height );
    
    //    ocpnDC gldc( *this );
    
    int w, h;
    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );
    
    glLoadIdentity();
    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );
    
    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }
    
    float vx0 = 0;
    float vy0 = 0;
    float vy = sy;
    float vx = sx;
    
    glBindTexture( g_texture_rectangle_format, 0);
    
    /*   glColor3ub(0, 0, 120);
     *    glBegin( GL_QUADS );
     *    glVertex2f( 0,  0 );
     *    glVertex2f( w, 0 );
     *    glVertex2f( w, h );
     *    glVertex2f( 0,  h );
     *    glEnd();
     */
    
    
    
    float tx, ty, tx0, ty0;
    //if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
    //  tx = sx, ty = sy;
    //else
    tx = 1, ty = 1;
    
    tx0 = ty0 = 0.;

    m_fbo_offsety += dy;
    m_fbo_offsetx += dx;
    
    tx0 = m_fbo_offsetx;
    ty0 = m_fbo_offsety;
    tx =  m_fbo_offsetx + sx;
    ty =  m_fbo_offsety + sy;
    
    
    if((m_fbo_offsety ) < 0){
        ty0 = 0;
        ty  =  m_fbo_offsety + sy;
        
        vy0 = 0;
        vy = sy + m_fbo_offsety;
        
        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  vy );
        glVertex2f( w, vy );
        glVertex2f( w, h );
        glVertex2f( 0,  h );
        glEnd();
        
    }
    else if((m_fbo_offsety + sy) > m_cache_tex_y){
        ty0 = m_fbo_offsety;
        ty  =  m_cache_tex_y;
        
        vy = sy;
        vy0 = (m_fbo_offsety + sy - m_cache_tex_y);
        
        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  0 );
        glVertex2f( w, 0 );
        glVertex2f( w, vy0 );
        glVertex2f( 0, vy0 );
        glEnd();
    }
    
    
    
    if((m_fbo_offsetx) < 0){
        tx0 = 0;
        tx  =  m_fbo_offsetx + sx;
        
        vx0 = -m_fbo_offsetx;
        vx = sx;
        
        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  0 );
        glVertex2f( vx0, 0 );
        glVertex2f( vx0, h );
        glVertex2f( 0,  h );
        glEnd();
    }
    else if((m_fbo_offsetx + sx) > m_cache_tex_x){
        tx0 = m_fbo_offsetx;
        tx  = m_cache_tex_x;
        
        vx0 = 0;
        vx = m_cache_tex_x - m_fbo_offsetx;
        
        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( vx,  0 );
        glVertex2f( w, 0 );
        glVertex2f( w, h );
        glVertex2f( vx,  h );
        glEnd();
    }
    

    // Render the cached texture as quad to screen
    glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
    glEnable( g_texture_rectangle_format );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    
    glBegin( GL_QUADS );
    glTexCoord2f( tx0/m_cache_tex_x, ty/m_cache_tex_y );  glVertex2f( vx0,  vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty/m_cache_tex_y );  glVertex2f( vx,   vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty0/m_cache_tex_y ); glVertex2f( vx,   vy );
    glTexCoord2f( tx0/m_cache_tex_x, ty0/m_cache_tex_y ); glVertex2f( vx0,  vy );
    glEnd();
    
    glDisable( g_texture_rectangle_format );
    glBindTexture( g_texture_rectangle_format, 0);
    
    
    SwapBuffers();
    
    m_canvasregion.Union(tx0, ty0, sx, sy);
}


void glChartCanvas::FastZoom(float factor)
{
    int sx = GetSize().x;
    int sy = GetSize().y;
   
    if(factor > 1.0f){
        int fbo_ctr_x = m_fbo_offsetx + (m_fbo_swidth / 2);
        int fbo_ctr_y = m_fbo_offsety + (m_fbo_sheight / 2);
        
        m_fbo_swidth  = m_fbo_swidth / factor;
        m_fbo_sheight = m_fbo_sheight / factor;
        
        m_fbo_offsetx = fbo_ctr_x - (m_fbo_swidth / 2.);
        m_fbo_offsety = fbo_ctr_y - (m_fbo_sheight / 2.);
        
    }
    
    if(factor < 1.0f){
        int fbo_ctr_x = m_fbo_offsetx + (m_fbo_swidth / 2);
        int fbo_ctr_y = m_fbo_offsety + (m_fbo_sheight / 2);
        
        m_fbo_swidth  = m_fbo_swidth / factor;
        m_fbo_sheight = m_fbo_sheight / factor;
        
        m_fbo_offsetx = fbo_ctr_x - (m_fbo_swidth / 2.);
        m_fbo_offsety = fbo_ctr_y - (m_fbo_sheight / 2.);
        
    }
    
    
    float tx, ty, tx0, ty0;
    //if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
    //  tx = sx, ty = sy;
    //else
    tx = 1, ty = 1;
    
    tx0 = ty0 = 0.;
    
    tx0 = m_fbo_offsetx;
    ty0 = m_fbo_offsety;
    tx =  m_fbo_offsetx + m_fbo_swidth;
    ty =  m_fbo_offsety + m_fbo_sheight;
    
    
    int w, h;
    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );
    
    glLoadIdentity();
    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );
    
    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    
    float vx0 = 0;
    float vy0 = 0;
    float vy = sy;
    float vx = sx;
    
    glBindTexture( g_texture_rectangle_format, 0);
    
    // Render the cached texture as quad to screen
    glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
    glEnable( g_texture_rectangle_format );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    
    
    glBegin( GL_QUADS );
    glTexCoord2f( tx0/m_cache_tex_x, ty/m_cache_tex_y );  glVertex2f( vx0,  vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty/m_cache_tex_y );  glVertex2f( vx,   vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty0/m_cache_tex_y ); glVertex2f( vx,   vy );
    glTexCoord2f( tx0/m_cache_tex_x, ty0/m_cache_tex_y ); glVertex2f( vx0,  vy );
    glEnd();
    
    glDisable( g_texture_rectangle_format );
    glBindTexture( g_texture_rectangle_format, 0);

    //  When zooming out, if we go too far, then the frame buffer is repeated on-screen due
    //  to address wrapping in the frame buffer.
    //  Detect this case, and render some simple solid covering quads to avoid a confusing display.
    if( (m_fbo_sheight > m_cache_tex_y) || (m_fbo_swidth > m_cache_tex_x) ){
        wxColour color = GetGlobalColor(_T("GREY1"));
        glColor3ub(color.Red(), color.Green(), color.Blue());
        
        if( m_fbo_sheight > m_cache_tex_y ){
            float h1 = sy * (1.0 - m_cache_tex_y/m_fbo_sheight) / 2.;
            
            glBegin( GL_QUADS );
            glVertex2f( 0,  0 );
            glVertex2f( w,  0 );
            glVertex2f( w, h1 );
            glVertex2f( 0, h1 );
            glEnd();

            glBegin( GL_QUADS );
            glVertex2f( 0,  sy );
            glVertex2f( w,  sy );
            glVertex2f( w, sy - h1 );
            glVertex2f( 0, sy - h1 );
            glEnd();
            
        }
 
         // horizontal axis
         if( m_fbo_swidth > m_cache_tex_x ){
             float w1 = sx * (1.0 - m_cache_tex_x/m_fbo_swidth) / 2.;
             
             glBegin( GL_QUADS );
             glVertex2f( 0,  0 );
             glVertex2f( w1,  0 );
             glVertex2f( w1, sy );
             glVertex2f( 0, sy );
             glEnd();
             
             glBegin( GL_QUADS );
             glVertex2f( sx,  0 );
             glVertex2f( sx - w1,  0 );
             glVertex2f( sx - w1, sy );
             glVertex2f( sx, sy );
             glEnd();
             
         }
    }
    
    SwapBuffers();
}

#ifdef __OCPN__ANDROID__

int panx, pany;

void glChartCanvas::OnEvtPanGesture( wxQT_PanGestureEvent &event)
{
   
    if( cc1->isRouteEditing() || cc1->isMarkEditing() )
        return;
    
    if(m_binPinch)
        return;
    if(m_bpinchGuard)
        return;
    
    int x = event.GetOffset().x;
    int y = event.GetOffset().y;
    
    int lx = event.GetLastOffset().x;
    int ly = event.GetLastOffset().y;
    
    int dx = lx - x;
    int dy = y - ly;
    
    switch(event.GetState()){
        case GestureStarted:
            if(m_binPan)
                break;
            
            panx = pany = 0;
            m_binPan = true;
            break;
            
        case GestureUpdated:
            if(m_binPan){
                if(!g_GLOptions.m_bUseCanvasPanning || m_bfogit)
                    cc1->PanCanvas( dx, -dy );
                else{
                    FastPan( dx, dy ); 
                }
                
                
                panx -= dx;
                pany -= dy;
                cc1->ClearbFollow();
            
            #ifdef __OCPN__ANDROID__
                androidSetFollowTool(false);
            #endif        
            }
            break;
            
        case GestureFinished:
            if(m_binPan){
                cc1->PanCanvas( -panx, pany );

            #ifdef __OCPN__ANDROID__
                androidSetFollowTool(false);
            #endif        
            
            }
            panx = pany = 0;
            m_binPan = false;
            
            break;
            
        case GestureCanceled:
            m_binPan = false; 
            break;
            
        default:
            break;
    }
    
    m_bgestureGuard = true;
    m_gestureEeventTimer.Start(500, wxTIMER_ONE_SHOT);
    
}


void glChartCanvas::OnEvtPinchGesture( wxQT_PinchGestureEvent &event)
{
    
    float zoom_gain = 1.0;
    float zoom_val;
    float total_zoom_val;

    if( event.GetScaleFactor() > 1)
        zoom_val = ((event.GetScaleFactor() - 1.0) * zoom_gain) + 1.0;
    else
        zoom_val = 1.0 - ((1.0 - event.GetScaleFactor()) * zoom_gain);

    if( event.GetTotalScaleFactor() > 1)
        total_zoom_val = ((event.GetTotalScaleFactor() - 1.0) * zoom_gain) + 1.0;
    else
        total_zoom_val = 1.0 - ((1.0 - event.GetTotalScaleFactor()) * zoom_gain);
 
    double projected_scale = cc1->GetVP().chart_scale / total_zoom_val;
    
    switch(event.GetState()){
        case GestureStarted:
            m_binPinch = true;
            m_binPan = false;   // cancel any tentative pan gesture, in case the "pan cancel" event was lost
            break;
            
        case GestureUpdated:
            if(g_GLOptions.m_bUseCanvasPanning){
                
                if( projected_scale < 3e8)
                    FastZoom(zoom_val);
                
            }
            break;
            
        case GestureFinished:{
                if( projected_scale < 3e8)
                    cc1->ZoomCanvas( total_zoom_val, false );
                else
                    cc1->ZoomCanvas(cc1->GetVP().chart_scale / 3e8, false);
            
             m_binPinch = false;
             break;
        }
            
        case GestureCanceled:
            m_binPinch = false;
            break;
            
        default:
            break;
    }

    m_bgestureGuard = true;
    m_bpinchGuard = true;
    m_gestureEeventTimer.Start(500, wxTIMER_ONE_SHOT);
    
}

void glChartCanvas::onGestureTimerEvent(wxTimerEvent &event)
{
    //  On some devices, the pan GestureFinished event fails to show up
    //  Watch for this case, and fix it.....
    if(m_binPan){
        m_binPan = false;
        Invalidate();
        Update();
    }
    m_bgestureGuard = false;
    m_bpinchGuard = false;
}


#endif

