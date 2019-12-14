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

#include "config.h"

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


#if defined(__OCPN__ANDROID__)
#include "androidUTIL.h"
#elif defined(__WXQT__)
#include <GL/glx.h>
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
#include "Track.h"
#include "emboss_data.h"
#include "Route.h"
#include "mbtiles.h"
#include <vector>
#include <algorithm>

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif

#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"

#include "lz4.h"

#ifdef __OCPN__ANDROID__
//  arm gcc compiler has a lot of trouble passing doubles as function aruments.
//  We don't really need double precision here, so fix with a (faster) macro.
extern "C" void glOrthof(float left,  float right,  float bottom,  float top,  float near,  float far);
#define glOrtho(a,b,c,d,e,f);     glOrthof(a,b,c,d,e,f);

#endif

#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"

#ifdef USE_ANDROID_GLES2
#include <gl2.h>
#include "linmath.h"
#include "shaders.h"
#endif

extern bool GetMemoryStatus(int *mem_total, int *mem_used);

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#endif

extern s52plib *ps52plib;
extern bool g_bopengl;
extern bool g_bDebugOGL;
extern bool g_bShowFPS;
extern bool g_bSoftwareGL;
extern bool g_btouch;
extern ocpnFloatingToolbarDialog *g_MainToolbar;
extern bool             g_bShowChartBar;
extern glTextureManager   *g_glTextureManager;
extern bool             b_inCompressAllCharts;
extern std::vector<int> g_quilt_noshow_index_array;

GLenum       g_texture_rectangle_format;

extern int g_memCacheLimit;
extern ColorScheme global_color_scheme;
extern bool g_bquiting;
extern ThumbWin         *pthumbwin;
extern int g_mipmap_max_level;

extern double           gLat, gLon, gCog, gSog, gHdt;

extern int              g_OwnShipIconType;


extern ChartDB          *ChartData;

extern PlugInManager* g_pi_manager;

extern WayPointman      *pWayPointMan;
extern RouteList        *pRouteList;
extern TrackList        *pTrackList;
extern bool             b_inCompressAllCharts;
extern bool             g_bGLexpert;
extern bool             g_bcompression_wait;
extern float            g_ShipScaleFactorExp;

float            g_GLMinSymbolLineWidth;
float            g_GLMinCartographicLineWidth;

extern bool             g_fog_overzoom;
extern double           g_overzoom_emphasis_base;
extern bool             g_oz_vector_scale;
extern TCMgr            *ptcmgr;
extern int              g_nCPUCount;
extern bool             g_running;

extern unsigned int     g_canvasConfig;
extern ChartCanvas      *g_focusCanvas;
extern ChartCanvas      *g_overlayCanvas;

ocpnGLOptions g_GLOptions;

wxColor                 s_regionColor;

//    For VBO(s)
bool         g_b_EnableVBO;
bool         g_b_needFinish;  //Need glFinish() call on each frame?


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

PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
PFNGLGETCOMPRESSEDTEXIMAGEPROC s_glGetCompressedTexImage;

//      Vertex Buffer Object (VBO) support
PFNGLGENBUFFERSPROC                 s_glGenBuffers;
PFNGLBINDBUFFERPROC                 s_glBindBuffer;
PFNGLBUFFERDATAPROC                 s_glBufferData;
PFNGLDELETEBUFFERSPROC              s_glDeleteBuffers;

#ifndef USE_ANDROID_GLES2
#define glDeleteFramebuffers (s_glDeleteFramebuffers);
#define glDeleteRenderbuffers (s_glDeleteRenderbuffers);
#endif


typedef void (APIENTRYP PFNGLGETBUFFERPARAMETERIV) (GLenum target, GLenum value, GLint *data);
PFNGLGETBUFFERPARAMETERIV s_glGetBufferParameteriv;

#include <wx/arrimpl.cpp>
//WX_DEFINE_OBJARRAY( ArrayOfTexDescriptors );

GLuint g_raster_format = GL_RGB;
long g_tex_mem_used;

bool            b_timeGL;
wxStopWatch     g_glstopwatch;
double          g_gl_ms_per_frame;

int g_tile_size;
int g_uncompressed_tile_size;

extern wxProgressDialog *pprog;
extern bool b_skipout;
extern wxSize pprog_size;
extern int pprog_count;
extern int pprog_threads;
extern float g_ChartScaleFactorExp;
extern MyFrame *gFrame;

//#if defined(__MSVC__) && !defined(ocpnUSE_GLES) /* this compiler doesn't support vla */
//const
//#endif
//extern int g_mipmap_max_level;
int panx, pany;

bool glChartCanvas::s_b_useScissorTest;
bool glChartCanvas::s_b_useStencil;
bool glChartCanvas::s_b_useStencilAP;
bool glChartCanvas::s_b_useFBO;

#ifdef USE_ANDROID_GLES2
static int          s_tess_vertex_idx;
static int          s_tess_vertex_idx_this;
static int          s_tess_buf_len;
static GLfloat     *s_tess_work_buf;
GLenum              s_tess_mode;
static int          s_nvertex;
static vec4         s_tess_color;
ViewPort            s_tessVP;
#endif


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

GLboolean QueryExtension( const char *extName )
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

void GetglEntryPoints( OCPN_GLCaps *pcaps )
{
    
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
        if((pcaps->m_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenFramebuffers", extensions[i])))
            break;
    }

    if(i<n_ext){
        pcaps->m_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenRenderbuffers", extensions[i]);
        pcaps->m_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
            ocpnGetProcAddress( "glFramebufferTexture2D", extensions[i]);
        pcaps->m_glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindFramebuffer", extensions[i]);
        pcaps->m_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glFramebufferRenderbuffer", extensions[i]);
        pcaps->m_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)
            ocpnGetProcAddress( "glRenderbufferStorage", extensions[i]);
        pcaps->m_glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindRenderbuffer", extensions[i]);
        pcaps->m_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
            ocpnGetProcAddress( "glCheckFramebufferStatus", extensions[i]);
        pcaps->m_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteFramebuffers", extensions[i]);
        pcaps->m_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteRenderbuffers", extensions[i]);
            
        //VBO
        pcaps->m_glGenBuffers = (PFNGLGENBUFFERSPROC)
            ocpnGetProcAddress( "glGenBuffers", extensions[i]);
        pcaps->m_glBindBuffer = (PFNGLBINDBUFFERPROC)
            ocpnGetProcAddress( "glBindBuffer", extensions[i]);
        pcaps->m_glBufferData = (PFNGLBUFFERDATAPROC)
            ocpnGetProcAddress( "glBufferData", extensions[i]);
        pcaps->m_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
            ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
        pcaps->m_glGetBufferParameteriv = (PFNGLBUFFERPARAMETERSIVPROC)
            ocpnGetProcAddress( "glGetBufferParameteriv", extensions[i]);

             
    }

    //  Retry VBO entry points with all extensions
    if(0 == pcaps->m_glGenBuffers){
        for( i=0; i<n_ext; i++) {
            if((pcaps->m_glGenBuffers = (PFNGLGENBUFFERSPROC)ocpnGetProcAddress( "glGenBuffers", extensions[i])) )
                break;
        }
        
        if( i < n_ext ){
            pcaps->m_glBindBuffer = (PFNGLBINDBUFFERPROC) ocpnGetProcAddress( "glBindBuffer", extensions[i]);
            pcaps->m_glBufferData = (PFNGLBUFFERDATAPROC) ocpnGetProcAddress( "glBufferData", extensions[i]);
            pcaps->m_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
        }
    }
            

#ifndef __OCPN__ANDROID__            
    for(i=0; i<n_ext; i++) {
        if((pcaps->m_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)
            ocpnGetProcAddress( "glCompressedTexImage2D", extensions[i])))
            break;
    }

    if(i<n_ext){
        pcaps->m_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)
            ocpnGetProcAddress( "glGetCompressedTexImage", extensions[i]);
    }
#else    
    pcaps->m_glCompressedTexImage2D =          glCompressedTexImage2D;
#endif
    
}





static void GetglEntryPoints( void )
{
    b_glEntryPointsSet = true;
    
    // the following are all part of framebuffer object,
    // according to opengl spec, we cannot mix EXT and ARB extensions
    // (I don't know that it could ever happen, but if it did, bad things would happen)

#ifndef __OCPN__ANDROID__
    const char *extensions[] = {"", "ARB", "EXT", 0 };
#else
    const char *extensions[] = {"", "OES", 0 };
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
            
        //VBO
        s_glGenBuffers = (PFNGLGENBUFFERSPROC)
            ocpnGetProcAddress( "glGenBuffers", extensions[i]);
        s_glBindBuffer = (PFNGLBINDBUFFERPROC)
            ocpnGetProcAddress( "glBindBuffer", extensions[i]);
        s_glBufferData = (PFNGLBUFFERDATAPROC)
            ocpnGetProcAddress( "glBufferData", extensions[i]);
        s_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
            ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
        s_glGetBufferParameteriv = (PFNGLBUFFERPARAMETERSIVPROC)
            ocpnGetProcAddress( "glGetBufferParameteriv", extensions[i]);

            
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

int test_attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 8, 0 };

glTestCanvas::glTestCanvas( wxWindow *parent ) :
    wxGLCanvas( parent, wxID_ANY, test_attribs, wxDefaultPosition, wxSize( 2, 2 ) )
{
}


// This attribute set works OK with vesa software only OpenGL renderer
int attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 8, 0 };
BEGIN_EVENT_TABLE ( glChartCanvas, wxGLCanvas ) EVT_PAINT ( glChartCanvas::OnPaint )
    EVT_ACTIVATE ( glChartCanvas::OnActivate )
    EVT_SIZE ( glChartCanvas::OnSize )
    EVT_MOUSE_EVENTS ( glChartCanvas::MouseEvent )
END_EVENT_TABLE()

glChartCanvas::glChartCanvas( wxWindow *parent, wxGLCanvas *share ) :
    wxGLCanvas( parent, wxID_ANY, attribs, wxDefaultPosition, wxSize( 256, 256 ),
                        wxFULL_REPAINT_ON_RESIZE | wxBG_STYLE_CUSTOM, _T("") )

{ 
    m_pParentCanvas = dynamic_cast<ChartCanvas *>( parent );

    Init();
}

                        
void glChartCanvas::Init()
{
    m_bsetup = false;

//    m_pParentCanvas = dynamic_cast<ChartCanvas *>( GetParent() );
    
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
    m_binGesture = false;
    
    b_timeGL = true;
    m_last_render_time = -1;

    m_LRUtime = 0;
    
    m_tideTex = 0;
    m_currentTex = 0;
    m_inFade = false;

    m_gldc.SetGLCanvas( this );
    
    
#ifdef __OCPN__ANDROID__    
    //  Create/connect a dynamic event handler slot for gesture and some timer events
    Connect( wxEVT_QT_PANGESTURE,
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::OnEvtPanGesture, NULL, this );
    
    Connect( wxEVT_QT_PINCHGESTURE,
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::OnEvtPinchGesture, NULL, this );

    Connect( GESTURE_EVENT_TIMER, wxEVT_TIMER, 
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::onGestureTimerEvent, NULL, this );
    
    Connect( GESTURE_FINISH_TIMER, wxEVT_TIMER, 
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::onGestureFinishTimerEvent, NULL, this );
    
    Connect( ZOOM_TIMER, wxEVT_TIMER, 
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::onZoomTimerEvent, NULL, this );

    
    m_gestureEeventTimer.SetOwner( this, GESTURE_EVENT_TIMER );
    m_gestureFinishTimer.SetOwner( this, GESTURE_FINISH_TIMER );
    zoomTimer.SetOwner( this, ZOOM_TIMER );

#ifdef USE_ANDROID_GLES2    
    Connect( TEX_FADE_TIMER, wxEVT_TIMER,
             (wxObjectEventFunction) (wxEventFunction) &glChartCanvas::onFadeTimerEvent, NULL, this );
    m_fadeTimer.SetOwner( this, TEX_FADE_TIMER );
#endif
    
    m_bgestureGuard = false;
    
#endif    

    if( !g_glTextureManager)
        g_glTextureManager = new glTextureManager;
}

glChartCanvas::~glChartCanvas()
{
#ifdef __OCPN__ANDROID__    
    unloadShaders();
#endif    
}

void glChartCanvas::FlushFBO( void ) 
{
    if(m_bsetup)
        BuildFBO();
}


void glChartCanvas::OnActivate( wxActivateEvent& event )
{
    m_pParentCanvas->OnActivate( event );
}

void glChartCanvas::OnSize( wxSizeEvent& event )
{
#if 0    
#ifdef __OCPN__ANDROID__ 
     if(!g_running){
         wxLogMessage(_T("Got OnSize event while NOT running"));
         event.Skip();
         qDebug() << "OnSizeB";

         return;
     }
#endif
#endif

    if(!IsShown())
        return;

    SetCurrent(*m_pcontext);
    
    if( !g_bopengl ) {
        SetSize( GetSize().x, GetSize().y );
        event.Skip();
        return;
    }

    
    // this is also necessary to update the context on some platforms
    // OnSize can be called with a different OpenGL context (when a plugin uses a different GL context).
    if( m_bsetup && m_pcontext && IsShown()) {
        SetCurrent(*m_pcontext);
    }

    /* expand opengl widget to fill viewport */
    //if( GetSize() != m_pParentCanvas->GetSize() )
    {

         SetSize( m_pParentCanvas->GetSize() );
        if( m_bsetup ){
            wxLogMessage(_T("BuildFBO 3"));

             BuildFBO();
     }
    }

    GetClientSize( &m_pParentCanvas->m_canvas_width, &m_pParentCanvas->m_canvas_height );

#ifdef USE_ANDROID_GLES2
    qDebug() << " glChartCanvas::OnSize()" << m_pParentCanvas->m_canvasIndex << m_pParentCanvas->m_canvas_width << m_pParentCanvas->m_canvas_height;

    if(m_pParentCanvas->m_canvasIndex >0){
        int xnew = gFrame->GetClientSize().x -m_pParentCanvas->m_canvas_width;
        //qDebug() << "XNEW" << xnew;
        //SetPosition(wxPoint(xnew, 0));
        SetSize( xnew, 0, m_pParentCanvas->m_canvas_width, m_pParentCanvas->m_canvas_height);
    }
    
    //  Set the shader viewport transform matrix
    mat4x4 m;
    ViewPort *vp = m_pParentCanvas->GetpVP();
    mat4x4_identity(m);
    mat4x4_scale_aniso((float (*)[4])vp->vp_transform, m, 2.0 / (float)vp->pix_width, -2.0 / (float)vp->pix_height, 1.0);
    mat4x4_translate_in_place((float (*)[4])vp->vp_transform, -vp->pix_width/2, -vp->pix_height/2, 0);
#endif

}

void glChartCanvas::MouseEvent( wxMouseEvent& event )
{
    if(m_pParentCanvas->MouseEventOverlayWindows( event ))
        return;

#ifndef __OCPN__ANDROID__
        if(m_pParentCanvas->MouseEventSetup( event )) 
        return;                 // handled, no further action required
        
        bool obj_proc = m_pParentCanvas->MouseEventProcessObjects( event );
    
        if(!obj_proc && !m_pParentCanvas->singleClickEventIsValid ) 
            m_pParentCanvas->MouseEventProcessCanvas( event );
    
    if( !g_btouch )
        m_pParentCanvas->SetCanvasCursor( event );
 
#else

    if(m_bgestureGuard){
        m_pParentCanvas->r_rband.x = 0;             // turn off rubberband temporarily
         
         // Sometimes we get a Gesture Pan start on a simple tap operation.
         // When this happens, we usually get no Gesture Finished event.
         // So, we need to process the next LeftUp event normally, to handle things like Measure and Route Create.
         
         // Allow LeftUp() event through if the pan action is very small
         //  Otherwise, drop the LeftUp() event, since it is not wanted for a Pan Gesture.
         if(event.LeftUp()){
             //qDebug() << panx << pany;
             if((abs(panx) > 2) || (abs(pany) > 2)){
                return;
             }
             else{                      // Cancel the in=process Gesture state
                m_gestureEeventTimer.Start(10, wxTIMER_ONE_SHOT);       // Short Circuit
             }
         }
         else
        return;
    }
        
            
    if(m_pParentCanvas->MouseEventSetup( event, false )) {
        if(!event.LeftDClick()){
            return;                 // handled, no further action required
        }
    }

    if(m_binPan && event.RightDown()){
        qDebug() << "Skip right on pan";
        return;
    }
    else{
        bool obj_proc = m_pParentCanvas->MouseEventProcessObjects( event );

        if(!obj_proc && !m_pParentCanvas->singleClickEventIsValid ) {
            if(!m_bgestureGuard)
                m_pParentCanvas->MouseEventProcessCanvas( event );      // This is where a physical mouse gets processed, if detected
        }
    
    }

#endif    
        
}

#ifndef GL_MAX_RENDERBUFFER_SIZE
#define GL_MAX_RENDERBUFFER_SIZE          0x84E8
#endif

#ifndef USE_ANDROID_GLES2
bool glChartCanvas::buildFBOSize(int fboSize)
{
    bool retVal = true;
    if(IsShown())
        SetCurrent(*m_pcontext);
    
    if( m_b_BuiltFBO ) {
        glDeleteTextures( 2, m_cache_tex );
        ( s_glDeleteFramebuffers )( 1, &m_fb0 );
        ( s_glDeleteRenderbuffers )( 1, &m_renderbuffer );
        m_b_BuiltFBO = false;
    }

    if( m_b_DisableFBO)
        return false;

#ifdef __OCPN__ANDROID__
    // We use the smallest possible (POT) FBO
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

#else
    m_cache_tex_x = GetSize().x;
    m_cache_tex_y = GetSize().y;
#endif

    int err = GL_NO_ERROR;
    GLint params;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,&params);
    
    err = glGetError();
    if(err == GL_INVALID_ENUM){
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&params);
        err = glGetError();
            
    }
     
    if(err == GL_NO_ERROR){
        if( fboSize > params ){
            wxLogMessage(_T("    OpenGL-> Requested Framebuffer size exceeds GL_MAX_RENDERBUFFER_SIZE") );
            return false;
    }        
    }
        
        
    
   
        
    ( s_glGenFramebuffers )( 1, &m_fb0 );
    err = glGetError();
    if(err){
        wxString msg;
        msg.Printf( _T("    OpenGL-> Framebuffer GenFramebuffers error:  %08X"), err );
        wxLogMessage(msg);
        retVal = false;
    }

    ( s_glGenRenderbuffers )( 1, &m_renderbuffer );
    err = glGetError();
    if(err){
        wxString msg;
        msg.Printf( _T("    OpenGL-> Framebuffer GenRenderbuffers error:  %08X"), err );
        wxLogMessage(msg);
        retVal = false;
    }


    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );
    err = glGetError();
    if(err){
        wxString msg;
        msg.Printf( _T("    OpenGL-> Framebuffer BindFramebuffers error:  %08X"), err );
        wxLogMessage(msg);
        retVal = false;
    }

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
#if 1        
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT,
                                         m_cache_tex_x, m_cache_tex_y );
        
        int err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> glRenderbufferStorage error:  %08X"), err );
            wxLogMessage(msg);
        }
        
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
        err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> glFramebufferRenderbuffer depth error:  %08X"), err );
            wxLogMessage(msg);
        }
        

        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
        err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> glFramebufferRenderbuffer stencil error:  %08X"), err );
            wxLogMessage(msg);
        }
        
#else
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES,
                                     m_cache_tex_x, m_cache_tex_y );
        
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                         GL_RENDERBUFFER, m_renderbuffer );
        
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                         GL_RENDERBUFFER, m_renderbuffer );
#endif        
        
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
                retVal = false;
        }
                
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
        
        err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> Framebuffer Depth Buffer Attach error:  %08X"), err );
            wxLogMessage(msg);
                retVal = false;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D,0);
    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );
    
        // Check framebuffer completeness at the end of initialization.
    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );
        
    ( s_glFramebufferTexture2D )
        ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
          g_texture_rectangle_format, m_cache_tex[0], 0 );
    
    GLenum fb_status = ( s_glCheckFramebufferStatus )( GL_FRAMEBUFFER_EXT );
    
    
    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

    if( fb_status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
        wxString msg;
        msg.Printf( _T("    OpenGL-> buildFBOSize->Framebuffer Incomplete:  %08X"), fb_status );
        wxLogMessage( msg );
        retVal = false;
    }
    
    return retVal;
}
#endif    

#ifdef USE_ANDROID_GLES2
bool glChartCanvas::buildFBOSize(int fboSize)
{
    bool retVal = true;


    // We use the smallest possible (POT) FBO
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

    //qDebug() << "FBO Size: " << GetSize().x << GetSize().y << m_cache_tex_x;


    int err = GL_NO_ERROR;
    GLint params;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,&params);
    
    err = glGetError();
    if(err == GL_INVALID_ENUM){
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&params);
        err = glGetError();
            
    }
     
    if(err == GL_NO_ERROR){
        if( fboSize > params ){
            wxLogMessage(_T("    OpenGL-> Requested Framebuffer size exceeds GL_MAX_RENDERBUFFER_SIZE") );
            return false;
        }
    }
        
        
    
   
    
     glGenFramebuffers ( 1, &m_fb0 );
    err = glGetError();
    if(err){
        wxString msg;
        msg.Printf( _T("    OpenGL-> Framebuffer GenFramebuffers error:  %08X"), err );
        wxLogMessage(msg);
        retVal = false;
    }

    glGenRenderbuffers( 1, &m_renderbuffer );
    err = glGetError();
    if(err){
        wxString msg;
        msg.Printf( _T("    OpenGL-> Framebuffer GenRenderbuffers error:  %08X"), err );
        wxLogMessage(msg);
        retVal = false;
    }

    
    glBindFramebuffer( GL_FRAMEBUFFER, m_fb0 );
    err = glGetError();
    if(err){
        wxString msg;
        msg.Printf( _T("    OpenGL-> Framebuffer BindFramebuffers error:  %08X"), err );
        wxLogMessage(msg);
        retVal = false;
    }
    
    // initialize color textures
    glGenTextures( 2, m_cache_tex );
    for(int i=0; i<2; i++) {
        glBindTexture( g_texture_rectangle_format, m_cache_tex[i] );
        glTexParameterf( g_texture_rectangle_format, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( g_texture_rectangle_format, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( g_texture_rectangle_format, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
        
    }

    glBindRenderbuffer( GL_RENDERBUFFER, m_renderbuffer );
    
        // initialize composite depth/stencil renderbuffer
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES,  m_cache_tex_x, m_cache_tex_y );

        err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> glRenderbufferStorage error:  %08X"), err );
            wxLogMessage(msg);
        }
        
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer );
        err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> glFramebufferRenderbuffer depth error:  %08X"), err );
            wxLogMessage(msg);
        }
        
        
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer );
        err = glGetError();
        if(err){
            wxString msg;
            msg.Printf( _T("    OpenGL-> glFramebufferRenderbuffer stencil error:  %08X"), err );
            wxLogMessage(msg);
        }
        
    
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
        // Check framebuffer completeness at the end of initialization.
    glBindFramebuffer( GL_FRAMEBUFFER, m_fb0 );
        
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  g_texture_rectangle_format, m_cache_tex[0], 0 );
    
    GLenum fb_status = glCheckFramebufferStatus( GL_FRAMEBUFFER_EXT );
    
    
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        
    if( fb_status != GL_FRAMEBUFFER_COMPLETE) {
        wxString msg;
        msg.Printf( _T("    OpenGL-> buildFBOSize->Framebuffer Incomplete:  %08X %08X"), fb_status );
        wxLogMessage( msg );
        retVal = false;
    }
    
    return retVal;
}
#endif

void glChartCanvas::BuildFBO( )
{
    
    if( m_b_BuiltFBO ) {
        //return;
        glDeleteTextures( 2, m_cache_tex );
        glDeleteFramebuffers( 1, &m_fb0 );
        glDeleteRenderbuffers( 1, &m_renderbuffer );
        m_b_BuiltFBO = false;
    }

    if( m_b_DisableFBO)
        return;

    
    int initialSize = 2048;
    
#ifdef __OCPN__ANDROID__
    //  Some low mem-spec devices have trouble with 2048 FBO size.
    //  Detect here, and choose 1024 size instead
    wxString info = androidGetDeviceInfo();
    
    if(wxNOT_FOUND != info.Find(_T("GT-S6312")) )
        initialSize = 1024;
#endif    
    
    if(!buildFBOSize(initialSize)){
        
        glDeleteTextures( 2, m_cache_tex );
        glDeleteFramebuffers( 1, &m_fb0 );
        glDeleteRenderbuffers( 1, &m_renderbuffer );
        
        if(!buildFBOSize(1024)){
            wxLogMessage(_T("BuildFBO C"));
            
            m_b_DisableFBO = true;
            wxLogMessage( _T("OpenGL-> FBO Framebuffer unavailable") );
            m_b_BuiltFBO = false;
            
            return;
        }
    }

    //  All OK
    
    wxString msg;
    msg.Printf( _T("OpenGL Framebuffer OK, size = %d"), m_cache_tex_x );
    wxLogMessage(msg);
    
    /* invalidate cache */
    Invalidate();

    glClear( GL_COLOR_BUFFER_BIT );
    m_b_BuiltFBO = true;
    
    return;
    
}


void glChartCanvas::SetupOpenGL()
{
   
    SetCurrent(*m_pcontext);

    char *str = (char *) glGetString( GL_RENDERER );
    if (str == NULL) {
        // perhaps we should edit the config and turn off opengl now
        wxLogMessage(_T("Failed to initialize OpenGL"));
        exit(1);
    }
    
    char render_string[80];
    strncpy( render_string, str, 79 );
    m_renderer = wxString( render_string, wxConvUTF8 );

    wxString msg;
    if(g_bSoftwareGL)
        msg.Printf( _T("OpenGL-> Software OpenGL") );
    msg.Printf( _T("OpenGL-> Renderer String: ") );
    msg += m_renderer;
    wxLogMessage( msg );

    if( ps52plib ) ps52plib->SetGLRendererString( m_renderer );
    
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

    bool b_oldIntel = false;
    if( GetRendererString().Upper().Find( _T("INTEL") ) != wxNOT_FOUND ){
        if( GetRendererString().Upper().Find( _T("965") ) != wxNOT_FOUND ){
            wxLogMessage( _T("OpenGL-> Detected early Intel renderer, disabling some GL features") );
            b_oldIntel = true;
        }
    }
    
    //  Set the minimum line width
    GLint parms[2];
#ifndef USE_ANDROID_GLES2
    glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0] );
#else
    glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
#endif
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

    if( GetRendererString().Find( _T("GeForce") ) != wxNOT_FOUND )   //GeForce GTX 1070
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
#endif
        
    GetglEntryPoints();
    
    if( !s_glGenFramebuffers  || !s_glGenRenderbuffers        || !s_glFramebufferTexture2D ||
        !s_glBindFramebuffer  || !s_glFramebufferRenderbuffer || !s_glRenderbufferStorage  ||
        !s_glBindRenderbuffer || !s_glCheckFramebufferStatus  || !s_glDeleteFramebuffers   ||
        !s_glDeleteRenderbuffers )
        m_b_DisableFBO = true;

    // VBO??
    
    g_b_EnableVBO = true;
    if( !s_glBindBuffer || !s_glBufferData || !s_glGenBuffers || !s_glDeleteBuffers )
        g_b_EnableVBO = false;

#if defined( __WXMSW__ ) || defined(__WXOSX__)
    if(b_oldIntel)    
        g_b_EnableVBO = false;
#endif

#ifdef __OCPN__ANDROID__
    g_b_EnableVBO = false;
#endif

#if defined( __WXMSW__ )
        g_b_EnableVBO = false;
        wxLogMessage( _T("OpenGL-> DISABLING VBO for Intel test.") );
#endif

    if(g_b_EnableVBO)
        wxLogMessage( _T("OpenGL-> Using Vertexbuffer Objects") );
    else
        wxLogMessage( _T("OpenGL-> Vertexbuffer Objects unavailable") );

// #if defined(__WXOSX__)
//     wxLogMessage( _T("OpenGL-> DISABLING VBO for Mac/Intel test.") );
//     g_b_EnableVBO = false;
// #endif    
    
    //      Can we use the stencil buffer in a FBO?
#ifdef ocpnUSE_GLES 
    m_b_useFBOStencil = QueryExtension( "GL_OES_packed_depth_stencil" );
#else
    m_b_useFBOStencil = QueryExtension( "GL_EXT_packed_depth_stencil" ) == GL_TRUE;
#endif

#ifndef USE_ANDROID_GLES2
    //  On Intel Graphics platforms, don't use stencil buffer at all
    if( bad_stencil_code)    
        s_b_useStencil = false;
#endif
    
    g_GLOptions.m_bUseCanvasPanning = false;

        
    //      Maybe build FBO(s)

    BuildFBO();
    
    
    
    
#ifndef __OCPN__ANDROID__
         /* this test sometimes fails when the fbo still works */
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

    if( m_b_BuiltFBO && !m_b_useFBOStencil )
        s_b_useStencil = false;

    //  If stencil seems to be a problem, force use of depth buffer clipping for Area Patterns
    s_b_useStencilAP = s_b_useStencil & !bad_stencil_code;

#ifdef USE_ANDROID_GLES2
    s_b_useStencilAP = s_b_useStencil;                  // required for GLES2 platform
#endif    
    
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
    
#ifdef USE_ANDROID_GLES2
    g_GLOptions.m_bUseAcceleratedPanning =  true;
#endif

    if(1)     // for now upload all levels
    {
        int max_level = 0;
        int tex_dim = g_GLOptions.m_iTextureDimension;
        for(int dim=tex_dim; dim>0; dim/=2)
            max_level++;
        g_mipmap_max_level = max_level - 1;
    } 
    
    //  Android, even though using GLES, does not require all levels.
#ifdef __OCPN__ANDROID__    
    g_mipmap_max_level = 4;
#endif
    
    
    s_b_useFBO = m_b_BuiltFBO;

    // Some older Intel GL drivers need a glFinish() call after each full frame render
    if(b_oldIntel)
        g_b_needFinish = true;
    
    //  Inform the S52 PLIB of options determined
    if(ps52plib)
        ps52plib->SetGLOptions(s_b_useStencil, s_b_useStencilAP, s_b_useScissorTest,  s_b_useFBO, g_b_EnableVBO, g_texture_rectangle_format);
    
    m_bsetup = true;
    
    SendJSONConfigMessage();    
}

void glChartCanvas::SendJSONConfigMessage()
{
    if(g_pi_manager){
        wxJSONValue v;
        v[_T("setupComplete")] =  m_bsetup;
        v[_T("useStencil")] =  s_b_useStencil;
        v[_T("useStencilAP")] =  s_b_useStencilAP;
        v[_T("useScissorTest")] =  s_b_useScissorTest;
        v[_T("useFBO")] =  s_b_useFBO;
        v[_T("useVBO")] =  g_b_EnableVBO;
        v[_T("TextureRectangleFormat")] =  g_texture_rectangle_format;
        wxString msg_id( _T("OCPN_OPENGL_CONFIG") );
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
    }
}
void glChartCanvas::SetupCompression()
{
    int dim = g_GLOptions.m_iTextureDimension;

#ifdef __WXMSW__    
    if(!::IsProcessorFeaturePresent( PF_XMMI64_INSTRUCTIONS_AVAILABLE )) {
        wxLogMessage( _T("OpenGL-> SSE2 Instruction set not available") );
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
    
        wxLogMessage( _T("OpenGL-> Using oes etc1 compression") );
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
            
            wxLogMessage( _T("OpenGL-> Using s3tc dxt1 compression") );
        } else if(QueryExtension("GL_3DFX_texture_compression_FXT1") &&
                  s_glCompressedTexImage2D && s_glGetCompressedTexImage) {
            g_raster_format = GL_COMPRESSED_RGB_FXT1_3DFX;
            
            wxLogMessage( _T("OpenGL-> Using 3dfx fxt1 compression") );
        } else {
            wxLogMessage( _T("OpenGL-> No Useable compression format found") );
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

    if(!m_pcontext)
        return;
    
    Show( g_bopengl );
    if( !g_bopengl ) {
        event.Skip();
        return;
    }

    SetCurrent(*m_pcontext);
    
    if( !m_bsetup ) {
        SetupOpenGL();
        
        if( ps52plib )
            ps52plib->FlushSymbolCaches();
        
        m_bsetup = true;
//        g_bDebugOGL = true;
    }

    //  Paint updates may have been externally disabled (temporarily, to avoid Yield() recursion performance loss)
    if(!m_b_paint_enable)
        return;
    //      Recursion test, sometimes seen on GTK systems when wxBusyCursor is activated
    if( m_in_glpaint ) return;
    
    //  If necessary, reconfigure the S52 PLIB
    m_pParentCanvas->UpdateCanvasS52PLIBConfig();
    
//     if( m_pParentCanvas->VPoint.b_quilt ){          // quilted
//         if( !m_pParentCanvas->m_pQuilt || !m_pParentCanvas->m_pQuilt->IsComposed() ) 
//             return;  // not ready
//             
//             if(m_pParentCanvas->m_pQuilt->IsQuiltVector()){    
//                 if(ps52plib->GetStateHash() != m_pParentCanvas->m_s52StateHash){
//                     m_pParentCanvas->UpdateS52State();
//                     m_pParentCanvas->m_s52StateHash = ps52plib->GetStateHash();
//                 }
//             }
//     }
    
    m_in_glpaint++;
    Render();
    m_in_glpaint--;

}


//   These routines allow reusable coordinates
bool glChartCanvas::HasNormalizedViewPort(const ViewPort &vp)
{
#ifndef USE_ANDROID_GLES2    
    return vp.m_projection_type == PROJECTION_MERCATOR ||
        vp.m_projection_type == PROJECTION_POLAR ||
        vp.m_projection_type == PROJECTION_EQUIRECTANGULAR;
#else
    return false;
#endif    
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
#ifndef USE_ANDROID_GLES2

    wxPoint2DDouble point;

    switch(vp.m_projection_type) {
    case PROJECTION_MERCATOR:
    case PROJECTION_EQUIRECTANGULAR:
    case PROJECTION_WEB_MERCATOR:
        //m_pParentCanvas->GetDoubleCanvasPointPixVP(vp, lat, lon, &point);
        point = vp.GetDoublePixFromLL(lat, lon);
        glTranslated(point.m_x, point.m_y, 0);
        glScaled(vp.view_scale_ppm/NORM_FACTOR, vp.view_scale_ppm/NORM_FACTOR, 1);
        break;

    case PROJECTION_POLAR:
        //m_pParentCanvas->GetDoubleCanvasPointPixVP(vp, vp.clat > 0 ? 90 : -90, vp.clon, &point);
        point = vp.GetDoublePixFromLL(vp.clat > 0 ? 90 : -90, vp.clon);
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
#endif
}

ViewPort glChartCanvas::NormalizedViewPort(const ViewPort &vp, float lat, float lon)
{
    ViewPort cvp = vp;

    switch(vp.m_projection_type) {
    case PROJECTION_MERCATOR:
    case PROJECTION_EQUIRECTANGULAR:
    case PROJECTION_WEB_MERCATOR:
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
    return vp.m_projection_type == PROJECTION_MERCATOR || vp.m_projection_type == PROJECTION_WEB_MERCATOR ||
        vp.m_projection_type == PROJECTION_EQUIRECTANGULAR;
}

ViewPort glChartCanvas::ClippedViewport(const ViewPort &vp, const LLRegion &region)
{
    if(!CanClipViewport(vp))
        return vp;

    ViewPort cvp = vp;
    LLBBox bbox = region.GetBox();
    
    if(!bbox.GetValid())
        return vp;

    /* region.GetBox() will always try to give coordinates from -180 to 180 but in
       the case where the viewport crosses the IDL, we actually want the clipped viewport
       to use coordinates outside this range to ensure the logic in the various rendering
       routines works the same here (with accelerated panning) as it does without, so we
       can adjust the coordinates here */

    if(bbox.GetMaxLon() < cvp.GetBBox().GetMinLon()) {
        bbox.Set(bbox.GetMinLat(), bbox.GetMinLon() + 360,
                 bbox.GetMaxLat(), bbox.GetMaxLon() + 360);
        cvp.SetBBoxDirect(bbox);
    } else if(bbox.GetMinLon() > cvp.GetBBox().GetMaxLon()) {
        bbox.Set(bbox.GetMinLat(), bbox.GetMinLon() - 360,
                 bbox.GetMaxLat(), bbox.GetMaxLon() - 360);
        cvp.SetBBoxDirect(bbox);
    } else
        cvp.SetBBoxDirect(bbox);

    return cvp;
}


void glChartCanvas::DrawStaticRoutesTracksAndWaypoints( ViewPort &vp )
{
    if(!m_pParentCanvas->m_bShowNavobjects)
        return;
    ocpnDC dc(*this);

    for(wxTrackListNode *node = pTrackList->GetFirst();
        node; node = node->GetNext() ) {
        Track *pTrackDraw = node->GetData();
                /* defer rendering active tracks until later */
        ActiveTrack *pActiveTrack = dynamic_cast<ActiveTrack *>(pTrackDraw);
        if(pActiveTrack && pActiveTrack->IsRunning() )
            continue;

        pTrackDraw->Draw( m_pParentCanvas, dc, vp, vp.GetBBox() );
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
    
        pRouteDraw->DrawGL( vp, m_pParentCanvas );
    }
        
    /* Waypoints not drawn as part of routes, and not being edited */
    if( vp.GetBBox().GetValid() && pWayPointMan) {
        for(wxRoutePointListNode *pnode = pWayPointMan->GetWaypointList()->GetFirst(); pnode; pnode = pnode->GetNext() ) {
            RoutePoint *pWP = pnode->GetData();
            if( pWP && (!pWP->m_bRPIsBeingEdited) &&(!pWP->m_bIsInRoute ) )
                if(vp.GetBBox().ContainsMarge(pWP->m_lat, pWP->m_lon, .5))
                    pWP->DrawGL( vp, m_pParentCanvas );
        }
    }
}

void glChartCanvas::DrawDynamicRoutesTracksAndWaypoints( ViewPort &vp )
{
    ocpnDC dc(*this);

    for(wxTrackListNode *node = pTrackList->GetFirst();
        node; node = node->GetNext() ) {
        Track *pTrackDraw = node->GetData();
        ActiveTrack *pActiveTrack = dynamic_cast<ActiveTrack *>(pTrackDraw);
        if(pActiveTrack && pActiveTrack->IsRunning() )
            pTrackDraw->Draw( m_pParentCanvas, dc, vp, vp.GetBBox() );     // We need Track::Draw() to dynamically render last (ownship) point.
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
                pRouteDraw->DrawGL( vp, m_pParentCanvas );
        }
    }
    
    
    /* Waypoints not drawn as part of routes, which are being edited right now */
    if( vp.GetBBox().GetValid() && pWayPointMan) {
        
        for(wxRoutePointListNode *pnode = pWayPointMan->GetWaypointList()->GetFirst(); pnode; pnode = pnode->GetNext() ) {
            RoutePoint *pWP = pnode->GetData();
            if( pWP && pWP->m_bRPIsBeingEdited && !pWP->m_bIsInRoute )
                pWP->DrawGL( vp, m_pParentCanvas );
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

void glChartCanvas::RenderChartOutline( ocpnDC &dc, int dbIndex, ViewPort &vp )
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

#ifndef USE_ANDROID_GLES2    
    if( g_GLOptions.m_GLLineSmoothing )
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
        // modulo is undefined for zero (compiler can use a div operation)
        int modulo = (nPly == 0)?1:nPly;
        for( int i = 0; i < nPly+1; i++ ) {
            if(nAuxPlyEntries)
                ChartData->GetDBAuxPlyPoint( dbIndex, i % modulo, j, &plylat, &plylon );
            else
                ChartData->GetDBPlyPoint( dbIndex, i % modulo, &plylat, &plylon );

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
                m_pParentCanvas->GetDoubleCanvasPointPix( lat, lon, &s );
                if(!std::isnan(s.m_x)) {
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
    
#else
    double nominal_line_width_pix = wxMax(2.0, floor(m_pParentCanvas->GetPixPerMM() / 4));             
    
    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_CM93 )
        dc.SetPen( wxPen( GetGlobalColor( _T ( "YELO1" ) ), nominal_line_width_pix, wxPENSTYLE_SOLID ) );
    
    else if( ChartData->GetDBChartFamily( dbIndex ) == CHART_FAMILY_VECTOR )
        dc.SetPen( wxPen( GetGlobalColor( _T ( "UINFG" ) ), nominal_line_width_pix, wxPENSTYLE_SOLID ) );
    
    else
        dc.SetPen( wxPen( GetGlobalColor( _T ( "UINFR" ) ), nominal_line_width_pix, wxPENSTYLE_SOLID ) );
    
    
    
    float plylat1, plylon1;
    int pixx, pixy, pixx1, pixy1;
    
    //        Are there any aux ply entries?
    int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex );
    if( 0 == nAuxPlyEntries )                 // There are no aux Ply Point entries
    {
        wxPoint r, r1;
        std::vector<int> points_vector;

        std::vector<float> vec = ChartData->GetReducedPlyPoints(dbIndex);
        int nPly = vec.size()/2;
        
        if(nPly == 0)
            return;
        
        for( int i = 0; i < nPly; i++ ) {
            plylon1 = vec[i *2];
            plylat1 = vec[i*2 + 1];
            
            m_pParentCanvas->GetCanvasPointPix( plylat1, plylon1, &r1 );
            pixx1 = r1.x;
            pixy1 = r1.y;
            
            points_vector.push_back(pixx1);
            points_vector.push_back(pixy1);
        }
        
        ChartData->GetDBPlyPoint( dbIndex, 0, &plylat1, &plylon1 );
        plylon1 += lon_bias;
        
        m_pParentCanvas->GetCanvasPointPix( vec[1], vec[0], &r1 );
        pixx1 = r1.x;
        pixy1 = r1.y;
        
        points_vector.push_back(pixx1);
        points_vector.push_back(pixy1);
      
        if(points_vector.size()){
            std::vector <int>::iterator it = points_vector.begin();
            dc.DrawLines( points_vector.size() / 2, (wxPoint *)&(*it), 0, 0, true);
        }
    }
    
    else                              // Use Aux PlyPoints
    {
        wxPoint r, r1;
        
        for( int j = 0; j < nAuxPlyEntries; j++ ) {

            std::vector<int> points_vector;
            
            std::vector<float> vec = ChartData->GetReducedAuxPlyPoints(dbIndex, j);
            int nAuxPly = vec.size()/2;
            
            if(nAuxPly == 0)
                continue;
            
            for( int i = 0; i < nAuxPly; i++ ) {
                plylon1 = vec[i *2];
                plylat1 = vec[i*2 + 1];
            
                m_pParentCanvas->GetCanvasPointPix( plylat1, plylon1, &r1 );
                pixx1 = r1.x;
                pixy1 = r1.y;
                
                points_vector.push_back(pixx1);
                points_vector.push_back(pixy1);
            }
            
            m_pParentCanvas->GetCanvasPointPix( vec[1], vec[0], &r1 );
            pixx1 = r1.x;
            pixy1 = r1.y;

            points_vector.push_back(pixx1);
            points_vector.push_back(pixy1);
            
            if(points_vector.size()){
                std::vector <int>::iterator it = points_vector.begin();
                dc.DrawLines( points_vector.size() / 2, (wxPoint *)&(*it), 0, 0, true);
            }
        }
    }
    
#endif    
}

extern void CalcGridSpacing( float WindowDegrees, float& MajorSpacing, float&MinorSpacing );
extern wxString CalcGridText( float latlon, float spacing, bool bPostfix );
void glChartCanvas::GridDraw( )
{
    if( !m_pParentCanvas->m_bDisplayGrid ) return;

    ViewPort &vp = m_pParentCanvas->GetVP();

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
        vp.m_projection_type == PROJECTION_WEB_MERCATOR ||
        vp.m_projection_type == PROJECTION_EQUIRECTANGULAR;
    bool straight_longitudes =
        vp.m_projection_type == PROJECTION_MERCATOR ||
        vp.m_projection_type == PROJECTION_WEB_MERCATOR ||
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

    ocpnDC gldc( *this );
    wxPen *pen = wxThePenList->FindOrCreatePen( GridColor, g_GLMinSymbolLineWidth, wxPENSTYLE_SOLID );
    gldc.SetPen( *pen );
    
    // Draw Major latitude grid lines and text

    // calculate position of first major latitude grid line
    float lon_step = elon - wlon;
    if(!straight_latitudes)
        lon_step /= ceil(lon_step / curved_step);

    for(lat = startlat; lat < nlat; lat += gridlatMajor) {
        wxPoint2DDouble r, s;
        s.m_x = NAN;

        for(lon = wlon; lon < elon+lon_step/2; lon += lon_step) {
            m_pParentCanvas->GetDoubleCanvasPointPix( lat, lon, &r );
            if(!std::isnan(s.m_x) && !std::isnan(r.m_x)) {
                gldc.DrawLine( s.m_x, s.m_y, r.m_x, r.m_y, true );
            }
            s = r;
        }
    }

    if(minorgrid) {
        // draw minor latitude grid lines
        for(lat = ceil( slat / gridlatMinor ) * gridlatMinor; lat < nlat; lat += gridlatMinor) {
        
            wxPoint r;
            m_pParentCanvas->GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
            gldc.DrawLine( 0, r.y, 10, r.y, true );
            gldc.DrawLine( w - 10, r.y, w, r.y, true );
            
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
            m_pParentCanvas->GetDoubleCanvasPointPix( lat, lon, &r );

            if(!std::isnan(s.m_x) && !std::isnan(r.m_x)) {
                gldc.DrawLine( s.m_x, s.m_y, r.m_x, r.m_y, true );
            }
            s = r;
        }
    }

    if(minorgrid) {
        // draw minor longitude grid lines
        for(lon = ceil( wlon / gridlonMinor ) * gridlonMinor; lon < elon; lon += gridlonMinor) {
            wxPoint r;
            m_pParentCanvas->GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
            gldc.DrawLine( r.x, 0, r.x, 10, true );
            gldc.DrawLine( r.x, h-10, r.x, h, true );
        }
    }

    
    // draw text labels
    glEnable(GL_TEXTURE_2D);
    glEnable( GL_BLEND );
    for(lat = startlat; lat < nlat; lat += gridlatMajor) {
        if( fabs( lat - wxRound( lat ) ) < 1e-5 )
            lat = wxRound( lat );

        wxString st = CalcGridText( lat, gridlatMajor, true ); // get text for grid line
        int iy;
        m_gridfont.GetTextExtent(st, 0, &iy);

        if(straight_latitudes) {
            wxPoint r, s;
            m_pParentCanvas->GetCanvasPointPix( lat, elon, &r );
            m_pParentCanvas->GetCanvasPointPix( lat, wlon, &s );
        
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
                m_pParentCanvas->GetCanvasPixPoint(0, y1, lat1, lon1);
                m_pParentCanvas->GetCanvasPixPoint(0, y2, lat2, lon2);

                double y = y1 + (lat1 - lat) * (y2 - y1) / (lat1 - lat2);

                m_pParentCanvas->GetDoubleCanvasPointPix( lat, lon1 + (y1 - y) * (lon2 - lon1) / (y1 - y2), &r);

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
                m_pParentCanvas->GetDoubleCanvasPointPix( lat, vp.clon, &r);

            m_gridfont.RenderString(st, r.m_x, r.m_y);
        }
    }


    for(lon = startlon; lon < elon; lon += gridlonMajor) {
        if( fabs( lon - wxRound( lon ) ) < 1e-5 )
            lon = wxRound( lon );

        wxPoint r, s;
        m_pParentCanvas->GetCanvasPointPix( nlat, lon, &r );
        m_pParentCanvas->GetCanvasPointPix( slat, lon, &s );

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
                m_pParentCanvas->GetCanvasPixPoint(x1, 0, lat1, lon1);
                m_pParentCanvas->GetCanvasPixPoint(x2, 0, lat2, lon2);

                double x = x1 + (lon1 - lon) * (x2 - x1) / (lon1 - lon2);

                m_pParentCanvas->GetDoubleCanvasPointPix( lat1 + (x1 - x) * (lat2 - lat1) / (x1 - x2), lon, &r);

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
                m_pParentCanvas->GetDoubleCanvasPointPix( wxMin(wxMax(vp.clat, slat), nlat), lon, &r);

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
                        (char) (emboss->pmap[( i * w ) + j] > 0 ? 0 : 255);
                    data[2 * ( ( i * emboss->glwidth ) + j ) + 1] = 
                        (char) abs( (emboss->pmap[( i * w ) + j]) );
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
    
    int x = emboss->x, y = emboss->y;

    float wp = (float) w / emboss->glwidth;
    float hp = (float) h / emboss->glheight;
    
#ifndef USE_ANDROID_GLES2

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    const float factor = 200;
    glColor4f( 1, 1, 1, factor / 256 );

    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 ), glVertex2i( x, y );
    glTexCoord2f( wp, 0 ), glVertex2i( x + w, y );
    glTexCoord2f( wp, hp ), glVertex2i( x + w, y + h );
    glTexCoord2f( 0, hp ), glVertex2i( x, y + h );
    glEnd();
    

#else
    float coords[8];
    float uv[8];

    //normal uv
    uv[0] = 0; uv[1] = 0; uv[2] = wp; uv[3] = 0;
    uv[4] = wp; uv[5] = hp; uv[6] = 0; uv[7] = hp;

    // pixels
    coords[0] = 0; coords[1] = 0; coords[2] = w; coords[3] = 0;
    coords[4] = w; coords[5] = h; coords[6] = 0; coords[7] = h;

    RenderSingleTexture(coords, uv, m_pParentCanvas->GetpVP(), x, y, 0);

#endif

    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
}

void glChartCanvas::ShipDraw(ocpnDC& dc)
{
    if( !m_pParentCanvas->GetVP().IsValid() ) return;
    wxPoint lGPSPoint, lShipMidPoint, GPSOffsetPixels(0,0);

    //  COG/SOG may be undefined in NMEA data stream
    float pCog = std::isnan(gCog) ? 0 : gCog;
    float pSog = std::isnan(gSog) ? 0 : gSog;

    m_pParentCanvas->GetCanvasPointPix( gLat, gLon, &lGPSPoint );
    lShipMidPoint = lGPSPoint;
 
    //  Draw the icon rotated to the COG
    //  or to the Hdt if available
    float icon_hdt = pCog;
    if( !std::isnan( gHdt ) ) icon_hdt = gHdt;

    //  COG may be undefined in NMEA data stream
    if( std::isnan(icon_hdt) ) icon_hdt = 0.0;

//    Calculate the ownship drawing angle icon_rad using an assumed 10 minute predictor
    double osd_head_lat, osd_head_lon;
    wxPoint osd_head_point;

    ll_gc_ll( gLat, gLon, icon_hdt, pSog * 10. / 60., &osd_head_lat, &osd_head_lon );
    
    m_pParentCanvas->GetCanvasPointPix( osd_head_lat, osd_head_lon, &osd_head_point );

    float icon_rad = atan2f( (float) ( osd_head_point.y - lShipMidPoint.y ),
                             (float) ( osd_head_point.x - lShipMidPoint.x ) );
    icon_rad += (float)PI;

    if( pSog < 0.2 ) icon_rad = ( ( icon_hdt + 90. ) * PI / 180. ) + m_pParentCanvas->GetVP().rotation;

    //    Another draw test ,based on pixels, assuming the ship icon is a fixed nominal size
    //    and is just barely outside the viewport        ....
    wxBoundingBox bb_screen( 0, 0, m_pParentCanvas->GetVP().pix_width, m_pParentCanvas->GetVP().pix_height );

    // TODO: fix to include actual size of boat that will be rendered
    int img_height = 0;
    
    if( bb_screen.PointInBox( lShipMidPoint, 20 ) ) {
        if( g_GLOptions.m_GLLineSmoothing )
            glEnable( GL_LINE_SMOOTH );
        if( g_GLOptions.m_GLPolygonSmoothing )
            glEnable( GL_POLYGON_SMOOTH );
    
        if( m_pParentCanvas->GetVP().chart_scale > 300000 )             // According to S52, this should be 50,000
        {
            double nominal_line_width_pix = wxMax(1.0, floor(m_pParentCanvas->GetPixPerMM() / 2 ));             //0.5 mm nominal, but not less than 1 pixel
        

            float scale_factor = 1.0;
        // Scale the generic icon to ChartScaleFactor, slightly softened....
            if((g_ChartScaleFactorExp > 1.0) && ( g_OwnShipIconType == 0 ))
                scale_factor = (log(g_ChartScaleFactorExp) + 1.0) * 1.1;   

            float nominal_ownship_size_mm = m_pParentCanvas->m_display_size_mm / 44.0;
            nominal_ownship_size_mm = wxMin(nominal_ownship_size_mm, 15.0);
            nominal_ownship_size_mm = wxMax(nominal_ownship_size_mm, 7.0);

            float nominal_ownship_size_pixels = wxMax(20.0, m_pParentCanvas->GetPixPerMM() * nominal_ownship_size_mm);   // nominal length, but not less than 20 pixel
            float v = (nominal_ownship_size_pixels * scale_factor) / 3;

            wxPen ppPen1( GetGlobalColor( _T ( "YELO1" ) ), v / 10 , wxPENSTYLE_SOLID );
            dc.SetPen( ppPen1 );
            dc.SetBrush( wxBrush( GetGlobalColor( _T ( "URED" ) ), wxBRUSHSTYLE_TRANSPARENT ) );

        // start with cross
            dc.DrawLine( (-v * 1.2) + lShipMidPoint.x, lShipMidPoint.y, (v * 1.2) + lShipMidPoint.x, lShipMidPoint.y);
            dc.DrawLine( lShipMidPoint.x, (-v * 1.2) + lShipMidPoint.y, lShipMidPoint.x, (v * 1.2) + lShipMidPoint.y);

        //  Two circles
            dc.StrokeCircle( lShipMidPoint.x, lShipMidPoint.y, v );
            dc.StrokeCircle( lShipMidPoint.x, lShipMidPoint.y, 0.6 * v );
            img_height = 20;
        } else {
            int draw_color = SHIP_INVALID;
            if( SHIP_NORMAL == m_pParentCanvas->m_ownship_state )
                draw_color = SHIP_NORMAL;
            else if( SHIP_LOWACCURACY == m_pParentCanvas->m_ownship_state )
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
                if(m_pParentCanvas->m_pos_image_user) {
                    switch (draw_color) {
                        case SHIP_INVALID: image = *m_pParentCanvas->m_pos_image_user_grey; break;
                        case SHIP_NORMAL: image = *m_pParentCanvas->m_pos_image_user; break;
                        case SHIP_LOWACCURACY: image = *m_pParentCanvas->m_pos_image_user_yellow; break;
                    }
                }
                else {
                    switch (draw_color) {
                        case SHIP_INVALID: image = *m_pParentCanvas->m_pos_image_grey; break;
                        case SHIP_NORMAL: image = *m_pParentCanvas->m_pos_image_red; break;
                        case SHIP_LOWACCURACY: image = *m_pParentCanvas->m_pos_image_yellow; break;
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
#ifndef USE_ANDROID_GLES2            
            if( m_pParentCanvas->m_pos_image_user )
                glColor4ub(255, 255, 255, 255);
            else if( SHIP_NORMAL == m_pParentCanvas->m_ownship_state )
                glColor4ub(255, 0, 0, 255);
            else if( SHIP_LOWACCURACY == m_pParentCanvas->m_ownship_state )
                glColor4ub(255, 255, 0, 255);
            else
                glColor4ub(128, 128, 128, 255);
#endif
            float scale_factor_y = 1.0;
            float scale_factor_x = 1.0;
        
            int ownShipWidth = 22; // Default values from s_ownship_icon
            int ownShipLength= 84;
            lShipMidPoint = lGPSPoint;

            /* scaled ship? */
            if( g_OwnShipIconType != 0 )
                m_pParentCanvas->ComputeShipScaleFactor
                    (icon_hdt, ownShipWidth, ownShipLength, lShipMidPoint,
                     GPSOffsetPixels, lGPSPoint, scale_factor_x, scale_factor_y);

            glEnable(GL_BLEND);

//            glEnableClientState(GL_VERTEX_ARRAY);

            int x = lShipMidPoint.x, y = lShipMidPoint.y;
            float deg = 180/PI * ( icon_rad - PI/2 );

        
            // Scale the generic icon to ChartScaleFactor, slightly softened....
            if((g_ShipScaleFactorExp > 1.0) && ( g_OwnShipIconType == 0 )){
                scale_factor_x = (log(g_ShipScaleFactorExp) + 1.0) * 1.1;   
                scale_factor_y = (log(g_ShipScaleFactorExp) + 1.0) * 1.1;   
            }
        
        // Set the size of the little circle showing the GPS reference position
        // Set a default early, adjust later based on render type
            float gps_circle_radius = 3.0;


            if( g_OwnShipIconType == 0 ) { // Default Bitmap

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, ownship_tex);
  
                //We choose to render the ownship bitmap at roughly the same size( in pixels ) as the DC mode renderer.
                // For ultra-high definition displays, we clamp the actual on-screen size to be no smaller than 7.0 mm
                // Similarly, for lo-res displays, we limit the actual size to be no larger than 15 mm maximum.
                
                // Get bitmap height in pixels 
                int image_height_bitmap = m_pParentCanvas->m_pos_image_red->GetHeight();
                if(m_pParentCanvas->m_pos_image_user) 
                    image_height_bitmap = m_pParentCanvas->m_pos_image_user->GetHeight();
 
                float nominal_ownship_size_mm = image_height_bitmap / m_pParentCanvas->GetPixPerMM();
                
                nominal_ownship_size_mm = wxMin(nominal_ownship_size_mm, 15.0);
                nominal_ownship_size_mm = wxMax(nominal_ownship_size_mm, 7.0);
                
                float nominal_ownship_size_pixels = wxMax(20.0, m_pParentCanvas->GetPixPerMM() * nominal_ownship_size_mm);             // nominal length, but not less than 20 pixel
                float h = nominal_ownship_size_pixels * scale_factor_y;
                float w = nominal_ownship_size_pixels * scale_factor_x * ownship_size.x / ownship_size.y ;
                float glw = ownship_tex_size.x, glh = ownship_tex_size.y;
                float u = ownship_size.x/glw, v = ownship_size.y/glh;
                
//                printf("%g %g %g %g %g %g %g\n", nominal_ownship_size_mm, nominal_ownship_size_pixels, h, w, u, v, m_pParentCanvas->m_display_size_mm);
                // tweak GPS reference point indicator size
                gps_circle_radius = w / 5;
                
                float uv[8], coords[8];
                uv[0] = 0; uv[1] = 0; uv[2] = u; uv[3] = 0;
                uv[4] = u; uv[5] = v; uv[6] = 0; uv[7] = v;
                    
                coords[0] = -w/2; coords[1] = -h/2; coords[2] =  w/2; coords[3] = -h/2;
                coords[4] =  w/2; coords[5] =  h/2; coords[6] = -w/2; coords[7] =  h/2;

                RenderSingleTexture(coords, uv, m_pParentCanvas->GetpVP(), x, y, icon_rad - PI/2); 
                
                glDisable(GL_TEXTURE_2D);
            } else if( g_OwnShipIconType == 1 ) { // Scaled Bitmap

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, ownship_tex);
            
                float nominal_ownship_size_pixels_y = 84; 
                float nominal_ownship_size_pixels_x = 22; 
                
                float h = nominal_ownship_size_pixels_y * scale_factor_y;
                float w = nominal_ownship_size_pixels_x * scale_factor_x;
                
                float u = (float)ownship_size.x/ownship_tex_size.x, v = (float)ownship_size.y/ownship_tex_size.y;

            // tweak GPS reference point indicator size
                gps_circle_radius = w / 5;
                
                float uv[8], coords[8];
                uv[0] = 0; uv[1] = 0; uv[2] = u; uv[3] = 0;
                uv[4] = u; uv[5] = v; uv[6] = 0; uv[7] = v;
                    
                coords[0] = -w/2; coords[1] = -h/2; coords[2] =  w/2; coords[3] = -h/2;
                coords[4] =  w/2; coords[5] =  h/2; coords[6] = -w/2; coords[7] =  h/2;

                RenderSingleTexture(coords, uv, m_pParentCanvas->GetpVP(), x, y, icon_rad - PI/2); 

                glDisable(GL_TEXTURE_2D);
            } else if( g_OwnShipIconType == 2 ) { // Scaled Vector
//                 static const GLint s_ownship_icon[] = { 5, -42, 11, -28, 11, 42, -11, 42,
//                                                         -11, -28, -5, -42, -11, 0, 11, 0,
//                                                         0, 42, 0, -42       };

                wxPoint shipPoints[6];

                wxColour colour = m_pParentCanvas->ShipColor();
                wxPen ppPen ( *wxBLACK, 1 );
                wxBrush ppBrush( colour );
                dc.SetPen( ppPen );
                dc.SetBrush( ppBrush );

                shipPoints[0].x = 0; shipPoints[0].y = -28;
                shipPoints[1].x = 11; shipPoints[1].y = -28;
                shipPoints[2].x = 11; shipPoints[2].y = 42;
                shipPoints[3].x = 0; shipPoints[3].y = 42;
                dc.DrawPolygon( 4, shipPoints, lShipMidPoint.x, lShipMidPoint.y, scale_factor_x, icon_rad - PI/2 );
                
                shipPoints[0].x = 0; shipPoints[0].y = -42;
                shipPoints[1].x = 5; shipPoints[1].y = -42;
                shipPoints[2].x = 11; shipPoints[2].y = -28;
                shipPoints[3].x = 0; shipPoints[3].y = -28;
                dc.DrawPolygon( 4, shipPoints, lShipMidPoint.x, lShipMidPoint.y, scale_factor_x, icon_rad - PI/2 );
                
                shipPoints[0].x = 0; shipPoints[0].y = -28;
                shipPoints[1].x = -11; shipPoints[1].y = -28;
                shipPoints[2].x = -11; shipPoints[2].y = 42;
                shipPoints[3].x = 0; shipPoints[3].y = 42;
                dc.DrawPolygon( 4, shipPoints, lShipMidPoint.x, lShipMidPoint.y, scale_factor_x, icon_rad - PI/2 );

                shipPoints[0].x = 0; shipPoints[0].y = -42;
                shipPoints[1].x = -5; shipPoints[1].y = -42;
                shipPoints[2].x = -11; shipPoints[2].y = -28;
                shipPoints[3].x = 0; shipPoints[3].y = -28;
                dc.DrawPolygon( 4, shipPoints, lShipMidPoint.x, lShipMidPoint.y, scale_factor_x, icon_rad - PI/2 );
 
                // draw with cross
                double p1x = -11 * scale_factor_x;
                double p2x =  11 * scale_factor_x;
                double p1y = 0;
                double p2y = 0;
                double p1xr = ((p1x) * cos(icon_rad - PI/2)) - ((p1y) * sin(icon_rad - PI/2));
                double p2xr = ((p2x) * cos(icon_rad - PI/2)) - ((p2y) * sin(icon_rad - PI/2));
                double p1yr = ((p1y) * cos(icon_rad - PI/2)) + ((p1x) * sin(icon_rad - PI/2));
                double p2yr = ((p2y) * cos(icon_rad - PI/2)) + ((p2x) * sin(icon_rad - PI/2));
                dc.DrawLine(p1xr+ lShipMidPoint.x, p1yr+ lShipMidPoint.y, p2xr+ lShipMidPoint.x, p2yr+ lShipMidPoint.y);    

                p1x = 0;
                p2x = 0;
                p1y = -42 * scale_factor_x;
                p2y =  42 * scale_factor_x;
                p1xr = ((p1x) * cos(icon_rad - PI/2)) - ((p1y) * sin(icon_rad - PI/2));
                p2xr = ((p2x) * cos(icon_rad - PI/2)) - ((p2y) * sin(icon_rad - PI/2));
                p1yr = ((p1y) * cos(icon_rad - PI/2)) + ((p1x) * sin(icon_rad - PI/2));
                p2yr = ((p2y) * cos(icon_rad - PI/2)) + ((p2x) * sin(icon_rad - PI/2));
                dc.DrawLine(p1xr+ lShipMidPoint.x, p1yr+ lShipMidPoint.y, p2xr+ lShipMidPoint.x, p2yr+ lShipMidPoint.y);    

                
            }

            img_height = ownShipLength * scale_factor_y;
        
        //      Reference point, where the GPS antenna is
        if( m_pParentCanvas->m_pos_image_user ) gps_circle_radius = 1;
 
        float cx = lGPSPoint.x, cy = lGPSPoint.y;
        wxPen ppPen1( GetGlobalColor( _T ( "UBLCK" ) ), 1, wxPENSTYLE_SOLID );
        dc.SetPen( ppPen1 );
        dc.SetBrush( wxBrush( GetGlobalColor( _T ( "CHWHT" ) ) ) );

        dc.StrokeCircle(cx, cy, gps_circle_radius);

    }

//        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_POLYGON_SMOOTH );
        glDisable(GL_BLEND);
    }

    m_pParentCanvas->ShipIndicatorsDraw(dc, img_height,  GPSOffsetPixels, lGPSPoint);
}

void glChartCanvas::DrawFloatingOverlayObjects( ocpnDC &dc )
{
    ViewPort &vp = m_pParentCanvas->GetVP();

    //  Draw any active or selected routes now
    extern Routeman                  *g_pRouteMan;
    extern Track                     *g_pActiveTrack;
    Route *active_route = g_pRouteMan->GetpActiveRoute();

//    if( active_route ) active_route->DrawGL( vp, region );
//    if( g_pActiveTrack ) g_pActiveTrack->Draw( dc, vp );
    //    if( m_pParentCanvas->m_pSelectedRoute ) m_pParentCanvas->m_pSelectedRoute->DrawGL( vp, region );

    GridDraw( );

    g_overlayCanvas = m_pParentCanvas;
    if (g_pi_manager) {
         g_pi_manager->SendViewPortToRequestingPlugIns(vp);
         g_pi_manager->RenderAllGLCanvasOverlayPlugIns(m_pcontext, vp, m_pParentCanvas->m_canvasIndex);
    }
   
    // all functions called with m_pParentCanvas-> are still slow because they go through ocpndc
    AISDrawAreaNotices( dc, m_pParentCanvas->GetVP(), m_pParentCanvas );

    m_pParentCanvas->DrawAnchorWatchPoints( dc );
    AISDraw( dc, m_pParentCanvas->GetVP(), m_pParentCanvas );
    ShipDraw( dc );
    m_pParentCanvas->AlertDraw( dc );

    m_pParentCanvas->RenderRouteLegs( dc );
    m_pParentCanvas->ScaleBarDraw( dc );
    s57_DrawExtendedLightSectors( dc, m_pParentCanvas->VPoint, m_pParentCanvas->extendedSectorLegs );
}

void glChartCanvas::DrawChartBar( ocpnDC &dc )
{
    if(m_pParentCanvas->GetPiano())
        m_pParentCanvas->GetPiano()->DrawGL(m_pParentCanvas->GetClientSize().y - m_pParentCanvas->GetPiano()->GetHeight());
}

void glChartCanvas::DrawQuiting()
{
#ifndef USE_ANDROID_GLES2    
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
#endif    
}

void glChartCanvas::DrawCloseMessage(wxString msg)
{
#ifndef USE_ANDROID_GLES2    
    
    if(1){
        
        wxFont *pfont = FontMgr::Get().FindOrCreateFont(12, wxFONTFAMILY_DEFAULT,
                                                        wxFONTSTYLE_NORMAL,
                                                        wxFONTWEIGHT_NORMAL);
        
        TexFont texfont;
        
        texfont.Build(*pfont);
        int w, h;
        texfont.GetTextExtent( msg, &w, &h);
        h += 2;
        int yp = m_pParentCanvas->GetVP().pix_height/2;
        int xp = (m_pParentCanvas->GetVP().pix_width - w)/2;
        
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
#endif    
}

void glChartCanvas::RotateToViewPort(const ViewPort &vp)
{
#ifndef USE_ANDROID_GLES2

    float angle = vp.rotation;

    if( fabs( angle ) > 0.0001 )
    {
        //    Rotations occur around 0,0, so translate to rotate around screen center
        float xt = vp.pix_width / 2.0, yt = vp.pix_height / 2.0;
        
        glTranslatef( xt, yt, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -xt, -yt, 0 );
    }
#endif
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

#ifdef USE_ANDROID_GLES2
void vertexCallbackD_GLSL(GLvoid *vertex)
{
    
    // Grow the work buffer if necessary
    if(s_tess_vertex_idx > s_tess_buf_len - 8)
    {
        int new_buf_len = s_tess_buf_len + 100;
        GLfloat * tmp = s_tess_work_buf;
        
        s_tess_work_buf = (GLfloat *)realloc(s_tess_work_buf, new_buf_len * sizeof(GLfloat));
        if (NULL == s_tess_work_buf)
        {
            free(tmp);
            tmp = NULL;
        }
        else
            s_tess_buf_len = new_buf_len;
    }
    
    GLdouble *pointer = (GLdouble *) vertex;
    
    s_tess_work_buf[s_tess_vertex_idx++] = (float)pointer[0];
    s_tess_work_buf[s_tess_vertex_idx++] = (float)pointer[1];
    
    s_nvertex++;
}

void beginCallbackD_GLSL( GLenum mode)
{
    s_tess_vertex_idx_this = s_tess_vertex_idx;
    s_tess_mode = mode;
    s_nvertex = 0;
}

void endCallbackD_GLSL()
{
    glUseProgram(color_tri_shader_program);
    
    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
    float *bufPt = &s_tess_work_buf[s_tess_vertex_idx_this];
    GLint pos = glGetAttribLocation(color_tri_shader_program, "position");
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), bufPt);
    glEnableVertexAttribArray(pos);
    
    GLint matloc = glGetUniformLocation(color_tri_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)s_tessVP.vp_transform); 
    
    // Use color stored in static variable.
    float colorv[4];
    colorv[0] = s_regionColor.Red() / float(256);
    colorv[1] = s_regionColor.Green() / float(256);
    colorv[2] = s_regionColor.Blue() / float(256);
    colorv[3] = s_regionColor.Alpha() / float(256);
    
    GLint colloc = glGetUniformLocation(color_tri_shader_program,"color");
    glUniform4fv(colloc, 1, colorv);
    
    glDrawArrays(s_tess_mode, 0, s_nvertex);
    
}
#else
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
#endif


void glChartCanvas::DrawRegion(ViewPort &vp, const LLRegion &region)
{
    float lat_dist, lon_dist;
    GetLatLonCurveDist(vp, lat_dist, lon_dist);

    GLUtesselator *tobj = gluNewTess();

    #ifdef USE_ANDROID_GLES2
    gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &vertexCallbackD_GLSL  );
    gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &beginCallbackD_GLSL  );
    gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &endCallbackD_GLSL  );
    gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &combineCallbackD );
    s_tessVP = vp;
    
    #else    
    gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &vertexCallbackD  );
    gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &beginCallbackD  );
    gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &endCallbackD  );
    gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &combineCallbackD );
    #endif
    
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
                if(std::isnan(q.m_x))
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
    }
#ifndef USE_ANDROID_GLES2

    else              //  Use depth buffer for clipping
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
#endif

    DrawRegion(vp, region);

    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    }
#ifndef USE_ANDROID_GLES2
    else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }
#endif
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
            glScissor(rect.x, vp.pix_height - rect.height - rect.y, rect.width, rect.height);
        }
#ifndef USE_ANDROID_GLES2
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
#endif
        return;
    }

#ifndef USE_ANDROID_GLES2
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
#endif
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

    m_cache_vp.Invalidate();

}

void glChartCanvas::RenderRasterChartRegionGL( ChartBase *chart, ViewPort &vp, LLRegion &region )
{
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );
    if( !pBSBChart ) return;

    if(b_inCompressAllCharts) return; // don't want multiple texfactories to exist
    

    //    Look for the texture factory for this chart
    wxString key = chart->GetHashKey();
    
    glTexFactory *pTexFact;
    ChartPathHashTexfactType &hash = g_glTextureManager->m_chart_texfactory_hash;
    ChartPathHashTexfactType::iterator ittf = hash.find( key );
    
    //    Not Found ?
    if( ittf == hash.end() ){
        hash[key] = new glTexFactory(chart, g_raster_format);
        hash[key]->SetHashKey(key);
    }
    
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
        double scalefactor = pBSBChart->GetRasterScaleFactor(vp);
        base_level = log(scalefactor) / log(2.0);

        if(base_level < 0) /* for overzoom */
            base_level = 0;
        if(base_level > g_mipmap_max_level)
            base_level = g_mipmap_max_level;
    } else
        base_level = 0; // base level should be computed per tile, for now load all

    /* setup opengl parameters */
    glEnable( GL_TEXTURE_2D );
#ifndef USE_ANDROID_GLES2
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if(use_norm_vp) {
        glPushMatrix();
        double lat, lon;
        pTexFact->GetCenter(lat, lon);
        MultMatrixViewPort(vp, lat, lon);
    }
#endif

    LLBBox box = region.GetBox();
    int numtiles;
    int mem_used = 0;
    if (g_memCacheLimit > 0) {
        // GetMemoryStatus is slow on linux
        GetMemoryStatus(0, &mem_used);
    }

    glTexTile **tiles = pTexFact->GetTiles(numtiles);
    for(int i = 0; i<numtiles; i++) {
        glTexTile *tile = tiles[i];
        if(region.IntersectOut(tile->box)) {
            
            /*   user setting is in MB while we count exact bytes */
            bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;
            if( bGLMemCrunch)
                pTexFact->DeleteTexture( tile->rect );
        } else {
            bool texture = pTexFact->PrepareTexture( base_level, tile->rect, global_color_scheme, mem_used );

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

#ifdef USE_ANDROID_GLES2
            RenderTextures(coords, tile->m_texcoords, 4, m_pParentCanvas->GetpVP());
#else            
            if(!texture) { // failed to load, draw red
                glDisable(GL_TEXTURE_2D);
                glColor3f(1, 0, 0);
            }

            glTexCoordPointer(2, GL_FLOAT, 2*sizeof(GLfloat), tile->m_texcoords);
            glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), coords);
            glDrawArrays(GL_QUADS, 0, tile->m_ncoords);
#endif
            if(!texture)
                glEnable(GL_TEXTURE_2D);

            if(!use_norm_vp)
                delete [] coords;
        }
    }

    glDisable(GL_TEXTURE_2D);

#ifndef USE_ANDROID_GLES2
    if(use_norm_vp)
        glPopMatrix();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}

void glChartCanvas::RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &rect_region )
{
    if( !m_pParentCanvas->m_pQuilt->GetnCharts() || m_pParentCanvas->m_pQuilt->IsBusy() )
        return;

    //  render the quilt
        ChartBase *chart = m_pParentCanvas->m_pQuilt->GetFirstChart();
        
    //  Check the first, smallest scale chart
    if(chart) {
        //            if( ! m_pParentCanvas->IsChartLargeEnoughToRender( chart, vp ) )
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
            //                if( ! m_pParentCanvas->IsChartLargeEnoughToRender( chart, vp ) ) {
                //                    chart = m_pParentCanvas->m_pQuilt->GetNextChart();
//                    continue;
//                }
        }

        QuiltPatch *pqp = m_pParentCanvas->m_pQuilt->GetCurrentPatch();
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
                        else if(chart->GetChartType() == CHART_TYPE_MBTILES){
                            SetClipRegion(vp, pqp->ActiveRegion/*pqp->quilt_region*/);
                            chart->RenderRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                            DisableClipRegion();
                        }
                        
                    } else if(chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {

                        if(chart->GetChartType() == CHART_TYPE_CM93COMP){
                           RenderNoDTA(vp, get_region);
                           chart->RenderRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                        }
                        else{
                            s57chart *Chs57 = dynamic_cast<s57chart*>( chart );
                            if(Chs57){
                                if(Chs57->m_RAZBuilt){
                                    RenderNoDTA(vp, get_region);
                                    Chs57->RenderRegionViewOnGLNoText( *m_pcontext, vp, rect_region, get_region );
                                    DisableClipRegion();
                                }
                                else{
                                    // The SENC is quesed for building, so..
                                    // Show GSHHS with compatible color scheme in the meantime.
                                    ocpnDC gldc( *this );
                                    const LLRegion &oregion = get_region;
                                    LLBBox box = oregion.GetBox();

                                    wxPoint p1 = vp.GetPixFromLL( box.GetMaxLat(), box.GetMinLon());
                                    wxPoint p2 = vp.GetPixFromLL( box.GetMaxLat(), box.GetMaxLon());
                                    wxPoint p3 = vp.GetPixFromLL( box.GetMinLat(), box.GetMaxLon());
                                    wxPoint p4 = vp.GetPixFromLL( box.GetMinLat(), box.GetMinLon());
                                        
                                    wxRect srect(p1.x, p1.y, p3.x - p1.x, p4.y - p2.y);

                                    bool world = false;
                                    ViewPort cvp = ClippedViewport(vp, get_region);
                                    if( m_pParentCanvas->GetWorldBackgroundChart()){
                                        SetClipRegion(cvp, get_region);
                                        m_pParentCanvas->GetWorldBackgroundChart()->SetColorsDirect(GetGlobalColor( _T ( "LANDA" ) ), GetGlobalColor( _T ( "DEPMS" )));
                                        RenderWorldChart(gldc, cvp, srect, world);
                                        m_pParentCanvas->GetWorldBackgroundChart()->SetColorScheme(global_color_scheme);
                                        DisableClipRegion();
                                    }
                                }
                            }
                            else{
                                ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                                if(ChPI){
                                    RenderNoDTA(vp, get_region);
                                    ChPI->RenderRegionViewOnGLNoText( *m_pcontext, vp, rect_region, get_region );
                                }
                                else{    
                                    RenderNoDTA(vp, get_region);
                                    chart->RenderRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                                }
                            }
                        }
                     }
                }
            }

            if(b_rendered) {
//                LLRegion get_region = pqp->ActiveRegion;
//                    get_region.Intersect( Region );  not technically required?
//                rendered_region.Union(get_region);
            }
        }


        chart = m_pParentCanvas->m_pQuilt->GetNextChart();
    }

    //    Render any Overlay patches for s57 charts(cells)
    if( m_pParentCanvas->m_pQuilt->HasOverlays() ) {
        ChartBase *pch = m_pParentCanvas->m_pQuilt->GetFirstChart();
        while( pch ) {
            QuiltPatch *pqp = m_pParentCanvas->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid && pqp->b_overlay && pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                LLRegion get_region = pqp->ActiveRegion;

                get_region.Intersect( region );
                if( !get_region.Empty()  ) {
                    s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                    if( Chs57 )
                        Chs57->RenderOverlayRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                    else{
                        ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( pch );
                        if(ChPI){
                            ChPI->RenderRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                        }
                    }

                }
            }

            pch = m_pParentCanvas->m_pQuilt->GetNextChart();
        }
    }

    // Hilite rollover patch
    LLRegion hiregion = m_pParentCanvas->m_pQuilt->GetHiliteRegion();

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

#ifndef USE_ANDROID_GLES2
        glColor4f( (float) .8, (float) .4, (float) .4, (float) hitrans );
#else
        s_regionColor = wxColor(204, 102, 102, hitrans * 256);
#endif

        DrawRegion(vp, hiregion);

        glDisable( GL_BLEND );
    }
    m_pParentCanvas->m_pQuilt->SetRenderedVP( vp );

}

void glChartCanvas::RenderQuiltViewGLText( ViewPort &vp, const OCPNRegion &rect_region )
{
    if( !m_pParentCanvas->m_pQuilt->GetnCharts() || m_pParentCanvas->m_pQuilt->IsBusy() )
        return;
    
    //  render the quilt
        ChartBase *chart = m_pParentCanvas->m_pQuilt->GetLargestScaleChart();
        
        LLRegion region = vp.GetLLRegion(rect_region);
        
        LLRegion rendered_region;
        while( chart ) {
            
            QuiltPatch *pqp = m_pParentCanvas->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid ) {
                LLRegion get_region = pqp->ActiveRegion;
                bool b_rendered = false;
                
                if( !pqp->b_overlay ) {
                    if(chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                        
                        s57chart *Chs57 = dynamic_cast<s57chart*>( chart );
                        if(Chs57){
                            Chs57->RenderViewOnGLTextOnly( *m_pcontext, vp);
                        }
                        else{
                            ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                            if(ChPI){
                                ChPI->RenderRegionViewOnGLTextOnly( *m_pcontext, vp, rect_region);
                            }
                        }
                    }    
                }
            }
            
            
            chart = m_pParentCanvas->m_pQuilt->GetNextSmallerScaleChart();
        }

/*        
        //    Render any Overlay patches for s57 charts(cells)
        if( m_pParentCanvas->m_pQuilt->HasOverlays() ) {
            ChartBase *pch = m_pParentCanvas->m_pQuilt->GetFirstChart();
            while( pch ) {
                QuiltPatch *pqp = m_pParentCanvas->m_pQuilt->GetCurrentPatch();
                if( pqp->b_Valid && pqp->b_overlay && pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                    LLRegion get_region = pqp->ActiveRegion;
                    
                    get_region.Intersect( region );
                    if( !get_region.Empty()  ) {
                        s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                        if( Chs57 )
                            Chs57->RenderOverlayRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                    }
                }
                
                pch = m_pParentCanvas->m_pQuilt->GetNextChart();
            }
        }
*/        
}

void glChartCanvas::RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region)
{
    ViewPort &vp = m_pParentCanvas->VPoint;

    // Only for cm93 (not quilted), SetVPParms can change the valid region of the chart
    // we need to know this before rendering the chart so we can compute the background region
    // and nodta regions correctly.  I would prefer to just perform this here (or in SetViewPoint)
    // for all vector charts instead of in their render routine, but how to handle quilted cases?
    if(!vp.b_quilt && m_pParentCanvas->m_singleChart->GetChartType() == CHART_TYPE_CM93COMP)
        static_cast<cm93compchart*>( m_pParentCanvas->m_singleChart )->SetVPParms( vp );
        
    LLRegion chart_region;
    if( !vp.b_quilt && (m_pParentCanvas->m_singleChart->GetChartType() == CHART_TYPE_PLUGIN) ){
        if(m_pParentCanvas->m_singleChart->GetChartFamily() == CHART_FAMILY_RASTER){
            // We do this the hard way, since PlugIn Raster charts do not understand LLRegion yet...
            double ll[8];
            ChartPlugInWrapper *cpw = dynamic_cast<ChartPlugInWrapper*> ( m_pParentCanvas->m_singleChart );
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
        else{
            Extent ext;
            m_pParentCanvas->m_singleChart->GetChartExtent(&ext);
            
            double ll[8] = {ext.SLAT, ext.WLON,
            ext.SLAT, ext.ELON,
            ext.NLAT, ext.ELON,
            ext.NLAT, ext.WLON};
            chart_region = LLRegion(4, ll);
        }
    }
    else
        chart_region = vp.b_quilt ? m_pParentCanvas->m_pQuilt->GetFullQuiltRegion() : m_pParentCanvas->m_singleChart->GetValidRegion();

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
        if( m_pParentCanvas->m_singleChart->GetChartFamily() == CHART_FAMILY_RASTER ){
            if(m_pParentCanvas->m_singleChart->GetChartType() == CHART_TYPE_MBTILES)
                m_pParentCanvas->m_singleChart->RenderRegionViewOnGL( *m_pcontext, vp, rect_region, region );
            else
                RenderRasterChartRegionGL( m_pParentCanvas->m_singleChart, vp, region );
        }
        else if( m_pParentCanvas->m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR ) {
            chart_region.Intersect(region);
            RenderNoDTA(vp, chart_region);
            m_pParentCanvas->m_singleChart->RenderRegionViewOnGL( *m_pcontext, vp, rect_region, region );
        } 
    }
        
}

void glChartCanvas::RenderNoDTA(ViewPort &vp, const LLRegion &region, int transparency)
{
    wxColour color = GetGlobalColor( _T ( "NODTA" ) );
#ifndef USE_ANDROID_GLES2    
    if( color.IsOk() )
        glColor4ub( color.Red(), color.Green(), color.Blue(), transparency );
    else
        glColor4ub( 163, 180, 183, transparency );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#else
    // Store the color for tesselator callback pickup.
    s_regionColor = color;
#endif
    
    DrawRegion(vp, region);
    
    glDisable(GL_BLEND);
}

void glChartCanvas::RenderNoDTA(ViewPort &vp, ChartBase *chart)
{
}

/* render world chart, but only in this rectangle */
void glChartCanvas::RenderWorldChart(ocpnDC &dc, ViewPort &vp, wxRect &rect, bool &world_view)
{
    // set gl color to water
    wxColour water = m_pParentCanvas->pWorldBackgroundChart->water;

    // clear background
    if(!world_view) {
#ifndef USE_ANDROID_GLES2
        // set gl color to water
        glColor3ub(water.Red(), water.Green(), water.Blue());

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
#endif

        if(!world_view) {
            int x1 = rect.x, y1 = rect.y, x2 = x1 + rect.width, y2 = y1 + rect.height;
#ifdef USE_ANDROID_GLES2
            glUseProgram(color_tri_shader_program);
            
            float pf[6];
            pf[0] = x1; pf[1] = y1; pf[2] = x2; pf[3] = y1; pf[4] = x2; pf[5] = y2;
            
            GLint pos = glGetAttribLocation(color_tri_shader_program, "position");
            glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), pf);
            glEnableVertexAttribArray(pos);
            
            GLint matloc = glGetUniformLocation(color_tri_shader_program,"MVMatrix");
            glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)vp.vp_transform); 
            
            float colorv[4];
            colorv[0] = water.Red() / float(256);
            colorv[1] = water.Green() / float(256);
            colorv[2] = water.Blue() / float(256);
            colorv[3] = 1.0;
            
            GLint colloc = glGetUniformLocation(color_tri_shader_program,"color");
            glUniform4fv(colloc, 1, colorv);
            
            //qDebug() << "triuni" << matloc << colloc;
            
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            pf[0] = x2; pf[1] = y2; pf[2] = x1; pf[3] = y2; pf[4] = x1; pf[5] = y1;
            glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), pf);
            glEnableVertexAttribArray(pos);
            
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
#else            
            glColor3ub(water.Red(), water.Green(), water.Blue());
            glBegin( GL_QUADS );
            glVertex2i( x1, y1 );
            glVertex2i( x2, y1 );
            glVertex2i( x2, y2 );
            glVertex2i( x1, y2 );
            glEnd();
#endif            
        }
    }

    m_pParentCanvas->pWorldBackgroundChart->RenderViewOnDC( dc, vp );
}

/* these are the overlay objects which move with the charts and
   are not frequently updated (not ships etc..) 

   many overlay objects are fixed to a geographical location or
   grounded as opposed to the floating overlay objects. */
void glChartCanvas::DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp)
{
    m_pParentCanvas->RenderAllChartOutlines( dc, vp );

    DrawStaticRoutesTracksAndWaypoints( vp );

    DisableClipRegion();
}


void glChartCanvas::DrawGLTidesInBBox(ocpnDC& dc, LLBBox& BBox)
{
    // At small scale, we render the Tide icon as a texture for best performance
    if( m_pParentCanvas->GetVP().chart_scale > 500000 ) {
        
        // Prepare the texture if necessary
        
        if(!m_tideTex){
            wxBitmap bmp = m_pParentCanvas->GetTideBitmap();
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
            if (!a)
                image.GetOrFindMaskColour( &mr, &mg, &mb );
                
            unsigned char *e = new unsigned char[4 * w * h];
            if(e && d){
                for( int y = 0; y < h; y++ )
                    for( int x = 0; x < w; x++ ) {
                        unsigned char r, g, b;
                        int off = ( y * w + x );
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
        
#ifndef USE_ANDROID_GLES2
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
                    m_pParentCanvas->GetCanvasPointPix( lat, lon, &r );
      
                    float xp = r.x;
                    float yp = r.y;
        
                    double scale = 1.0;
                    #ifdef __OCPN__ANDROID__
                    scale *= getAndroidDisplayDensity();
                    #endif                        
                    double width2 = scale * m_tideTexWidth/2;
                    double height2 = scale * m_tideTexHeight/2;
                    
                    glBegin( GL_QUADS );
                    glTexCoord2f( 0,  0 );  glVertex2f( xp - width2,  yp - height2 );
                    glTexCoord2f( 0,  1 );  glVertex2f( xp - width2,  yp + height2 );
                    glTexCoord2f( 1,  1 );  glVertex2f( xp + width2,  yp + height2 );
                    glTexCoord2f( 1,  0 );  glVertex2f( xp + width2,  yp - height2 );
                    glEnd();
                }
            } // type 'T"
        }       //loop
#else
        for( int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++ ) {
            const IDX_entry *pIDX = ptcmgr->GetIDX_entry( i );
    
            char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
            if( ( type == 't' ) || ( type == 'T' ) )  // only Tides
            {
                double lon = pIDX->IDX_lon;
                double lat = pIDX->IDX_lat;
                
                if( BBox.Contains( lat,  lon ) ) {
                    wxPoint r;
                    m_pParentCanvas->GetCanvasPointPix( lat, lon, &r );
                    
                    float xp = r.x;
                    float yp = r.y;
                    
                    double scale = 1.0;
                    scale *= getAndroidDisplayDensity();
                    double width2 = scale * m_tideTexWidth/2;
                    double height2 = scale * m_tideTexHeight/2;
                    
                    float coords[8];
                    float uv[8];
                    
                    //normal uv
                    uv[0] = 0; uv[1] = 0; uv[2] = 0; uv[3] = 1;
                    uv[4] = 1; uv[5] = 1; uv[6] = 1; uv[7] = 0;
                    
                    coords[0] = xp - width2; coords[1] = yp - height2; coords[2] = xp - width2; coords[3] = yp + height2;
                    coords[4] = xp + width2; coords[5] = yp + height2; coords[6] = xp + width2; coords[7] = yp - height2;
                    
                    RenderTextures(coords, uv, 4, m_pParentCanvas->GetpVP());
                }
            } // type 'T"
        }       //loop

            
#endif
            
        glDisable( GL_TEXTURE_2D );
        glDisable(GL_BLEND);
        glBindTexture( GL_TEXTURE_2D, 0);
    }
    else
        m_pParentCanvas->DrawAllTidesInBBox( dc, BBox );
}

void glChartCanvas::DrawGLCurrentsInBBox(ocpnDC& dc, LLBBox& BBox)
{
    m_pParentCanvas->DrawAllCurrentsInBBox(dc, BBox);
}


void glChartCanvas::SetColorScheme(ColorScheme cs)
{
    if(!m_bsetup)
        return;

    glDeleteTextures(1, &m_tideTex);
    glDeleteTextures(1, &m_currentTex);
    m_tideTex = 0;
    m_currentTex = 0;
    ownship_color = -1;
    
}

void glChartCanvas::RenderGLAlertMessage()
{
    if(!m_pParentCanvas->GetAlertString().IsEmpty())
    {
        wxString msg = m_pParentCanvas->GetAlertString(); 
    
        wxFont *pfont = GetOCPNScaledFont(_("Dialog"));
        m_gldc.SetFont( *pfont);
    
        int w, h;
        wxScreenDC sdc;
        sdc.GetTextExtent(msg, &w, &h, NULL, NULL, pfont);
 
        h += 2;
        w += 4;
        int yp = m_pParentCanvas->VPoint.pix_height - GetChartbarHeight() - h - (h/4);
        
        wxRect sbr = m_pParentCanvas->GetScaleBarRect();
        int xp = sbr.x+sbr.width + 5;
        
        wxPen ppPen1( GetGlobalColor( _T ( "UBLCK" ) ), 1, wxPENSTYLE_SOLID );
        m_gldc.SetPen( ppPen1 );
        m_gldc.SetBrush( wxBrush( GetGlobalColor( _T ( "YELO1" ) ) ) );
        
        m_gldc.DrawRectangle( xp, yp, w, h );
        
        m_gldc.DrawText( msg, xp, yp );
        
    }
}

unsigned long quiltHash;
int refChartIndex;


int n_render;
void glChartCanvas::Render()
{
    if( !m_bsetup || !m_pParentCanvas->m_pQuilt ||
        ( m_pParentCanvas->VPoint.b_quilt && !m_pParentCanvas->m_pQuilt ) ||
        ( !m_pParentCanvas->VPoint.b_quilt && !m_pParentCanvas->m_singleChart ) ) {
#ifdef __WXGTK__  // for some reason in gtk, a swap is needed here to get an initial screen update
            SwapBuffers();
#endif
            return;
        }

#ifdef USE_ANDROID_GLES2

    OCPNStopWatch sw;
    
    if(m_inFade)
        return;

    if(m_binPinch)
        return;
    
    //qDebug() << "Render" << m_pParentCanvas->m_canvasIndex << GetPosition().x << GetSize().x << m_pParentCanvas->GetPosition().x << m_pcontext;
    
    //if(m_pParentCanvas->m_canvasIndex == 0) return;
    
       // Do any setup required...
    loadShaders(0/*m_pParentCanvas->m_canvasIndex*/);
    configureShaders( m_pParentCanvas->VPoint);
    
    bool recompose = false;
    if(m_pParentCanvas->VPoint.b_quilt && m_pParentCanvas->m_pQuilt && !m_pParentCanvas->m_pQuilt->IsComposed()){
        if(m_pParentCanvas->VPoint.IsValid()){
            m_pParentCanvas->m_pQuilt->Compose(m_pParentCanvas->VPoint);
            m_pParentCanvas->UpdateCanvasControlBar();
            recompose = true;
        }
        else
            return;
    }
    
    //  Check to see if the Compose() call forced a SENC build.
    //  If so, zoom the canvas just slightly to force a deferred redraw of the full screen.
    if(sw.GetTime() > 2000){       //  long enough to detect SENC build.
        m_pParentCanvas->ZoomCanvas( 1.0001, false );
    }
        
        //qDebug() << "RenderTime1" << sw.GetTime();
        
    s_tess_vertex_idx = 0;
    quiltHash = m_pParentCanvas->m_pQuilt->GetXStackHash();
    refChartIndex = m_pParentCanvas->m_pQuilt->GetRefChartdbIndex();
        
#endif
        
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

    ViewPort VPoint = m_pParentCanvas->VPoint;
    ocpnDC gldc( *this );

    int gl_width, gl_height;
    m_pParentCanvas->GetClientSize( &gl_width, &gl_height );

#ifdef __WXOSX__    
    gl_height = m_pParentCanvas->GetClientSize().y;
#endif    
    
    OCPNRegion screen_region(wxRect(0, 0, VPoint.pix_width, VPoint.pix_height));

    glViewport( 0, 0, (GLint) gl_width, (GLint) gl_height );
       
#ifndef USE_ANDROID_GLES2
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    glOrtho( 0, (GLint) gl_width, (GLint) gl_height, 0, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    // set opengl settings that don't normally change
    // this should be able to go in SetupOpenGL, but it's
    // safer here incase a plugin mangles these
    if( g_GLOptions.m_GLLineSmoothing )
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    if( g_GLOptions.m_GLPolygonSmoothing )
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
    
    bool bpost_hilite = !m_pParentCanvas->m_pQuilt->GetHiliteRegion( ).Empty();
    bool useFBO = false;
    int sx = gl_width;
    int sy = gl_height;

    // Try to use the framebuffer object's cache of the last frame
    // to accelerate drawing this frame (if overlapping)
    if(m_b_BuiltFBO && !m_bfogit && !scale_it && !bpost_hilite
       //&& VPoint.tilt == 0 // disabling fbo in tilt mode gives better quality but slower
        ) {
        //  Is this viewpoint the same as the previously painted one?
        bool b_newview = true;
        bool b_full = false;

        // If the view is the same we do no updates, 
        // cached texture to the framebuffer
        if(    m_cache_vp.view_scale_ppm == VPoint.view_scale_ppm
               && m_cache_vp.rotation == VPoint.rotation
               && m_cache_vp.clat == VPoint.clat
               && m_cache_vp.clon == VPoint.clon
               && m_cache_vp.IsValid()
               && m_cache_vp.pix_height == VPoint.pix_height
               && m_cache_current_ch == m_pParentCanvas->m_singleChart ) {
            b_newview = false;
        }

#ifdef USE_ANDROID_GLES2
        if(recompose)
            b_newview = true;
        
        if(m_bforcefull){
            b_newview = true;
            b_full = true;
        }
        
        
        // If no charts are to be rendered, we need to refresh the entire display
        //  This fixes a problem with routes/tracks/marks rendering on pans at very small scale.
        //  It is a workaround, so finding root cause should be considered a TODO
        
        if(VPoint.b_quilt){
            ChartBase *chart = m_pParentCanvas->m_pQuilt->GetFirstChart();
            if(!chart)
                b_full = true;
        }
            
#endif        
        
        if( b_newview ) {

            bool busy = false;
            if(VPoint.b_quilt && m_pParentCanvas->m_pQuilt->IsQuiltVector() &&
                ( m_cache_vp.view_scale_ppm != VPoint.view_scale_ppm || m_cache_vp.rotation != VPoint.rotation))
            {
                    OCPNPlatform::ShowBusySpinner();
                    busy = true;
            }
            
            float dx = 0;
            float dy = 0;

            bool accelerated_pan = false;
            if( g_GLOptions.m_bUseAcceleratedPanning && m_cache_vp.IsValid()
                && ( VPoint.m_projection_type == PROJECTION_MERCATOR
                || VPoint.m_projection_type == PROJECTION_EQUIRECTANGULAR )
                && m_cache_vp.pix_height == VPoint.pix_height )
            {
                wxPoint2DDouble c_old = VPoint.GetDoublePixFromLL( VPoint.clat, VPoint.clon );
                wxPoint2DDouble c_new = m_cache_vp.GetDoublePixFromLL( VPoint.clat, VPoint.clon );

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
                    
                accelerated_pan = b_whole_pixel && abs(dx) < m_cache_tex_x && abs(dy) < m_cache_tex_y;
            }

            // do we allow accelerated panning?  can we perform it here?
#ifndef USE_ANDROID_GLES2            
            // enable rendering to texture in framebuffer object
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

            if(accelerated_pan) {
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

                    RenderCharts(m_gldc, update_region);

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
                    
                    m_fbo_offsetx = 0;
                    m_fbo_offsety = 0;
                    m_fbo_swidth = sx;
                    m_fbo_sheight = sy;
                    wxRect rect(m_fbo_offsetx, m_fbo_offsety, (GLint) sx, (GLint) sy);
                    RenderCharts(m_gldc, screen_region);
            }
                    
            // Disable Render to FBO
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );
                    
    
#else   // GLES2
        // enable rendering to texture in framebuffer object
        glBindFramebuffer( GL_FRAMEBUFFER, m_fb0 );
                            
            if(VPoint.chart_scale < 5000)
                b_full = true;
                            
            if(VPoint.chart_scale > 5e7)
                b_full = true;
                            
            if(b_full)
                accelerated_pan = false;
                            
            if(accelerated_pan) {
                //qDebug() << "AccPan";
                if((dx != 0) || (dy != 0)){   // Anything to do?
                    m_cache_page = !m_cache_page; /* page flip */
                        
                    /* perform accelerated pan rendering to the current target texture */
                    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, g_texture_rectangle_format, m_cache_tex[m_cache_page], 0 );
                        
                    //calculate the new regions to render
                    // add extra pixels to avoid coordindate rounding issues at large scale
                    OCPNRegion update_region;

                    int fluff = 0;
                        
                    // Avoid rendering artifacts caused by Multi Sampling (MSAA)
                    if(VPoint.chart_scale < 10000)
                        fluff = 8;
                        
                    if( dy > 0 && dy < VPoint.pix_height)
                        update_region.Union(wxRect( 0, VPoint.pix_height - (dy + fluff), VPoint.pix_width, dy+fluff ));
                    else if(dy < 0)
                        update_region.Union(wxRect( 0, 0, VPoint.pix_width, -dy + fluff ));

                    if( dx > 0 && dx < VPoint.pix_width )
                        update_region.Union(wxRect( VPoint.pix_width - (dx+fluff), 0, dx+fluff, VPoint.pix_height ));
                    else if (dx < 0)
                        update_region.Union(wxRect( 0, 0, -dx + fluff, VPoint.pix_height ));

                        wxColour color = GetGlobalColor( _T ( "NODTA" ) );
                            glClearColor( color.Red() / 256., color.Green()/256., color.Blue()/256., 1.0 );
                    //glClearColor(1.0f, 0.0f, 0.f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);
                    
                    // Render the new content
                    //OCPNStopWatch swr1;
 //                   RenderCharts(gldc, update_region);
                    glViewport( 0, 0, (GLint) sx, (GLint) sy );
                    

                    // using the old framebuffer
                    glBindTexture( g_texture_rectangle_format, m_cache_tex[!m_cache_page] );
                    glEnable( g_texture_rectangle_format );
                                
                    //    Render the reuseable portion of the cached texture
                    // Render the cached texture as quad to FBO(m_blit_tex) with offsets
                    float x1, x2, y1, y2;

                    if( dx > 0 )
                        x1 = dx,  x2 = 0;
                    else
                        x1 = 0,   x2 = -dx;
                            
                    if( dy > 0 )
                        y1 = dy,  y2 = 0;
                        else
                        y1 = 0,   y2 = -dy;

                    // normalize to texture coordinates range from 0 to 1
                    float tx1, tx2, ty1, ty2;
                    

                    float xcor = 0;
                    float ycor = 0;
                    
                    tx1 = 0; tx2 = sx/(float)m_cache_tex_x; ty1 = 0 ; ty2 = sy/(float)m_cache_tex_y;    
                    
                    float coords[8];
                    float uv[8];
                        
                        //normal uv
                    uv[0] = tx1; uv[1] = ty1; uv[2] = tx2; uv[3] = ty1;
                    uv[4] = tx2; uv[5] = ty2; uv[6] = tx1; uv[7] = ty2;
                        

//<<<<<<< HEAD
                    
                    coords[0] = -dx; coords[1] = dy; coords[2] = -dx + sx; coords[3] = dy;
                    coords[4] = -dx + sx; coords[5] = dy + sy; coords[6] = -dx; coords[7] = dy+sy;
                    
                    
//                         qDebug() << coords[0] << coords[1] << coords[2] << coords[3];
//                         qDebug() << coords[4] << coords[5] << coords[6] << coords[7];
//                         qDebug() << uv[0] << uv[1] << uv[2] << uv[3];
//                         qDebug() << uv[4] << uv[5] << uv[6] << uv[7];

                        
                        //build_texture_shaders();
                        glUseProgram( FBO_texture_2D_shader_program );
                        
                        // Get pointers to the attributes in the program.
                        GLint mPosAttrib = glGetAttribLocation( FBO_texture_2D_shader_program, "aPos" );
                        GLint mUvAttrib  = glGetAttribLocation( FBO_texture_2D_shader_program, "aUV" );
                        
                        // Set up the texture sampler to texture unit 0
                        GLint texUni = glGetUniformLocation( FBO_texture_2D_shader_program, "uTex" );
                        glUniform1i( texUni, 0 );
                        
                        // Disable VBO's (vertex buffer objects) for attributes.
                        glBindBuffer( GL_ARRAY_BUFFER, 0 );
                        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
                        
                        // Set the attribute mPosAttrib with the vertices in the screen coordinates...
                        glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
                        // ... and enable it.
                        glEnableVertexAttribArray( mPosAttrib );
                        
                        // Set the attribute mUvAttrib with the vertices in the GL coordinates...
                        glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uv );
                        // ... and enable it.
                        glEnableVertexAttribArray( mUvAttrib );
                        
                        
                        GLint matloc = glGetUniformLocation(FBO_texture_2D_shader_program,"MVMatrix");
                        
                        mat4x4 m, mvp;
                        
                        mat4x4_identity(m);
                        mat4x4_scale_aniso(mvp, m, 2.0 / sx, 2.0 / sy , 1.0);
                        mat4x4_translate_in_place(mvp, -sx/2, -sy/2 , 0);
                        
                        glUniformMatrix4fv( matloc, 1, GL_FALSE, (const float *)mvp); 
                        
                        // Select the active texture unit.
                        glActiveTexture( GL_TEXTURE0 );
                        // Bind our texture to the texturing unit target.
                        glBindTexture( g_texture_rectangle_format, m_cache_tex[!m_cache_page] );
                        
                        // Perform the actual drawing.
                        
                        float co1[8];
                        co1[0] = coords[0];
                        co1[1] = coords[1];
                        co1[2] = coords[2];
                        co1[3] = coords[3];
                        co1[4] = coords[6];
                        co1[5] = coords[7];
                        co1[6] = coords[4];
                        co1[7] = coords[5];
                        
                        float tco1[8];
                        tco1[0] = uv[0];
                        tco1[1] = uv[1];
                        tco1[2] = uv[2];
                        tco1[3] = uv[3];
                        tco1[4] = uv[6];
                        tco1[5] = uv[7];
                        tco1[6] = uv[4];
                        tco1[7] = uv[5];
                        
                        glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, co1 );
                        glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, tco1 );
                        
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                        
                        //qDebug() << "RenderTime2" << sw.GetTime();
                        
                        
                     // Render the new content
                        RenderCharts(m_gldc, update_region);
                        //qDebug() << "RenderTime3" << sw.GetTime();
                     
                    
                     //qDebug() << "RenderTime4" << sw.GetTime();
                     
                     glDisable( g_texture_rectangle_format );
                }

            } // accelerated pan
                
            else { // must redraw the entire screen
                //qDebug() << "Fullpage";
                glFramebufferTexture2D( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, g_texture_rectangle_format, m_cache_tex[!m_cache_page], 0 );

                        m_fbo_offsetx = 0;
                        m_fbo_offsety = 0;
                        m_fbo_swidth = sx;
                        m_fbo_sheight = sy;
                    
                // Do not need to clear screen, especially annoying on pinch zoom
                //wxColour color = GetGlobalColor( _T ( "NODTA" ) );
                //glClearColor( color.Red() / 256., color.Green() / 256. , color.Blue()/ 256. ,1.0 );
                //glClear(GL_COLOR_BUFFER_BIT);
            
                RenderCharts(m_gldc, screen_region);

                //qDebug() << "RenderTimeFULL" << sw.GetTime();
                
                m_cache_page = !m_cache_page; /* page flip */
  
                    
            } // full page render

            // Disable Render to FBO
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            
#endif  //gles2 for accpan

                
                

            if(busy)
                OCPNPlatform::HideBusySpinner();
        
        } // newview

        useFBO = true;
    }

#ifndef __OCPN__ANDROID__    
    if(VPoint.tilt) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(2*180/PI*atan2((double)gl_height, (double)gl_width), (GLfloat) gl_width/(GLfloat) gl_height, 1, gl_width);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glScalef(1, -1, 1);
        glTranslatef(-gl_width/2, -gl_height/2, -gl_width/2);
        glRotated(VPoint.tilt*180/PI, 1, 0, 0);

        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    }
#endif

    if(useFBO) {
        // Render the cached texture as quad to screen
        glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
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
        
#ifndef USE_ANDROID_GLES2        
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
        glBegin( GL_QUADS );
        glTexCoord2f( tx0, ty );  glVertex2f( 0,  0 );
        glTexCoord2f( tx,  ty );  glVertex2f( sx, 0 );
        glTexCoord2f( tx,  ty0 ); glVertex2f( sx, sy );
        glTexCoord2f( tx0, ty0 ); glVertex2f( 0,  sy );
        glEnd();
#else
        float coords[8];
        float uv[8];
        
        //normal uv
        uv[0] = tx0; uv[1] = ty; uv[2] = tx; uv[3] = ty;
        uv[4] = tx; uv[5] = ty0; uv[6] = tx0; uv[7] = ty0;
        
        // pixels
        coords[0] = 0; coords[1] = 0; coords[2] = sx; coords[3] = 0;
        coords[4] = sx; coords[5] = sy; coords[6] = 0; coords[7] = sy;
        
        if(!m_inFade){
            RenderTextures(coords, uv, 4, m_pParentCanvas->GetpVP());
        }
        else
            qDebug() << "skip FBO update for inFade";
        
#endif
        

        glDisable( g_texture_rectangle_format );

        m_cache_vp = VPoint;
        m_cache_current_ch = m_pParentCanvas->m_singleChart;

        if(VPoint.b_quilt)
            m_pParentCanvas->m_pQuilt->SetRenderedVP( VPoint );
        
    } else          // useFBO
    {
        //qDebug() << "RenderCharts No FBO";
        RenderCharts(m_gldc, screen_region);
    }
 
    // Done with base charts.
    // Now the overlays

    RenderS57TextOverlay( VPoint );
    RenderMBTilesOverlay( VPoint );
    
     
    // Render static overlay objects
    for(OCPNRegionIterator upd ( screen_region ); upd.HaveRects(); upd.NextRect()) {
         LLRegion region = VPoint.GetLLRegion(upd.GetRect());
         ViewPort cvp = ClippedViewport(VPoint, region);
         DrawGroundedOverlayObjects(gldc, cvp);
    }


    if( m_pParentCanvas->m_bShowTide  || m_pParentCanvas->m_bShowCurrent ){
        LLRegion screenLLRegion = VPoint.GetLLRegion( screen_region );
        LLBBox screenBox = screenLLRegion.GetBox();
        // Enlarge the box a bit
        screenBox.EnLarge(screenBox.GetLonRange()* 0.05);
        
        // update the tide/current select points, if necessary
        if( m_pParentCanvas->m_bShowTide ) {
            m_pParentCanvas->RebuildTideSelectList(screenBox);          // full screen
            DrawGLTidesInBBox( gldc, VPoint.GetBBox() );
        }
        
        if( m_pParentCanvas->m_bShowCurrent ){
            m_pParentCanvas->RebuildCurrentSelectList(screenBox);
            DrawGLCurrentsInBBox( gldc, VPoint.GetBBox() );
        }
    }


    // If multi-canvas, indicate which canvas has keyboard focus
    // by drawing a simple blue bar at the top.
    if(g_canvasConfig != 0){             // multi-canvas?
        if( m_pParentCanvas == wxWindow::FindFocus()){
            g_focusCanvas = m_pParentCanvas;
            
            wxColour colour = GetGlobalColor(_T("BLUE4"));
            wxPen ppBlue ( colour, 1 );
            wxBrush ppBrush( colour );
            gldc.SetPen( ppBlue );
            gldc.SetBrush( ppBrush );
            int xw = m_pParentCanvas->GetClientSize().x;
            float rect_pix = m_pParentCanvas->m_focus_indicator_pix;
            wxPoint barPoints[4];
            barPoints[0].x = 0; barPoints[0].y = 0;
            barPoints[1].x = xw; barPoints[1].y = 0;
            barPoints[2].x = xw; barPoints[2].y = rect_pix;
            barPoints[3].x = 0; barPoints[3].y = rect_pix;

            gldc.DrawPolygon( 4, barPoints, 0, 0, 1, 0);
        }
    }
    

    DrawDynamicRoutesTracksAndWaypoints( VPoint );
        
    // Now draw all the objects which normally move around and are not
    // cached from the previous frame
    DrawFloatingOverlayObjects( m_gldc );

#ifndef USE_ANDROID_GLES2    
    // from this point on don't use perspective
    if(VPoint.tilt) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        glOrtho( 0, (GLint) gl_width, (GLint) gl_height, 0, -1, 1 );
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
#endif

    DrawEmboss(m_pParentCanvas->EmbossDepthScale() );
    DrawEmboss(m_pParentCanvas->EmbossOverzoomIndicator( gldc ) );

    if( m_pParentCanvas->m_pTrackRolloverWin )
        m_pParentCanvas->m_pTrackRolloverWin->Draw(gldc);

    if( m_pParentCanvas->m_pRouteRolloverWin )
        m_pParentCanvas->m_pRouteRolloverWin->Draw(gldc);

    if( m_pParentCanvas->m_pAISRolloverWin )
        m_pParentCanvas->m_pAISRolloverWin->Draw(gldc);

    //  On some platforms, the opengl context window is always on top of any standard DC windows,
    //  so we need to draw the Chart Info Window and the Thumbnail as overlayed bmps.

#ifdef __WXOSX__    
        if(m_pParentCanvas->m_pCIWin && m_pParentCanvas->m_pCIWin->IsShown()) {
        int x, y, width, height;
        m_pParentCanvas->m_pCIWin->GetClientSize( &width, &height );
        m_pParentCanvas->m_pCIWin->GetPosition( &x, &y );
        wxBitmap bmp(width, height, -1);
        wxMemoryDC dc(bmp);
        if(bmp.IsOk()){
            dc.SetBackground( wxBrush(GetGlobalColor( _T ( "UIBCK" ) ) ));
            dc.Clear();
 
            dc.SetTextBackground( GetGlobalColor( _T ( "UIBCK" ) ) );
            dc.SetTextForeground( GetGlobalColor( _T ( "UITX1" ) ) );
            
            int yt = 0;
            int xt = 0;
            wxString s = m_pParentCanvas->m_pCIWin->GetString();
            int h = m_pParentCanvas->m_pCIWin->GetCharHeight();
            
            wxStringTokenizer tkz( s, _T("\n") );
            wxString token;
            
            while(tkz.HasMoreTokens()) {
                token = tkz.GetNextToken();
                dc.DrawText(token, xt, yt);
                yt += h;
            }
            dc.SelectObject(wxNullBitmap);
            
            m_gldc.DrawBitmap( bmp, x, y, false);
        }
    }

    if( pthumbwin && pthumbwin->IsShown()) {
        int thumbx, thumby;
        pthumbwin->GetPosition( &thumbx, &thumby );
        if( pthumbwin->GetBitmap().IsOk())
            m_gldc.DrawBitmap( pthumbwin->GetBitmap(), thumbx, thumby, false);
    }
    
    if(g_MainToolbar && g_MainToolbar->m_pRecoverwin ){
        int recoverx, recovery;
        g_MainToolbar->m_pRecoverwin->GetPosition( &recoverx, &recovery );
        if( g_MainToolbar->m_pRecoverwin->GetBitmap().IsOk())
            m_gldc.DrawBitmap( g_MainToolbar->m_pRecoverwin->GetBitmap(), recoverx, recovery, true);
    }
    
    
#endif
    // render the chart bar
    if(g_bShowChartBar)
        DrawChartBar(m_gldc);

    if (m_pParentCanvas->m_Compass)
        m_pParentCanvas->m_Compass->Paint(gldc);
    
    RenderGLAlertMessage();

    //quiting?
    if( g_bquiting )
        DrawQuiting();
    if( g_bcompression_wait)
        DrawCloseMessage( _("Waiting for raster chart compression thread exit."));

    
    
     //  Some older MSW OpenGL drivers are generally very unstable.
     //  This helps...   

    if(g_b_needFinish)
        glFinish();
    
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
    
    m_pParentCanvas->PaintCleanup();
    //OCPNPlatform::HideBusySpinner();
    m_bforcefull = false;
    
    n_render++;
}

void glChartCanvas::RenderS57TextOverlay( ViewPort &VPoint)
{
    //  Render the decluttered Text overlay for quilted vector charts, except for CM93 Composite
    if( VPoint.b_quilt ) {
        if(m_pParentCanvas->m_pQuilt->IsQuiltVector() && ps52plib && ps52plib->GetShowS57Text()){

            ChartBase *chart = m_pParentCanvas->m_pQuilt->GetRefChart();
            if(chart && (chart->GetChartType() != CHART_TYPE_CM93COMP)){
                //        Clear the text Global declutter list
                if(chart){
                    ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                    if(ChPI)
                        ChPI->ClearPLIBTextList();
                    else
                        ps52plib->ClearTextList();
                }
                
                // Grow the ViewPort a bit laterally, to minimize "jumping" of text elements at left side of screen
                ViewPort vpx = VPoint;
                vpx.BuildExpandedVP(VPoint.pix_width * 12 / 10, VPoint.pix_height);

                OCPNRegion screen_region(wxRect(0, 0, VPoint.pix_width, VPoint.pix_height));
                RenderQuiltViewGLText( vpx, screen_region );
            }
        }
    }
}

void glChartCanvas::RenderMBTilesOverlay( ViewPort &VPoint)
{
       // Render MBTiles as overlay
    std::vector<int> stackIndexArray = m_pParentCanvas->m_pQuilt->GetExtendedStackIndexArray();
    unsigned int im = stackIndexArray.size();
    // XXX should
    // assert(!VPoint.b_quilt && im == 0)
    if( VPoint.b_quilt && im > 0 ) {
        bool regionVPBuilt = false;
        OCPNRegion screen_region;
        LLRegion screenLLRegion;
        LLBBox screenBox;
        ViewPort vp;
 
        std::vector<int> tiles_to_show;
        for( unsigned int is = 0; is < im; is++ ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( stackIndexArray[is] );
            if(std::find(g_quilt_noshow_index_array.begin(), g_quilt_noshow_index_array.end(), stackIndexArray[is]) != g_quilt_noshow_index_array.end()) {
                continue;
            }
            if(cte.GetChartType() == CHART_TYPE_MBTILES){
                tiles_to_show.push_back(stackIndexArray[is]);
                if(!regionVPBuilt){
                    screen_region = OCPNRegion(wxRect(0, 0, VPoint.pix_width, VPoint.pix_height));
                    screenLLRegion = VPoint.GetLLRegion( screen_region );
                    screenBox = screenLLRegion.GetBox();

                    vp = VPoint;
                    wxPoint p;
                    p.x = VPoint.pix_width / 2;  p.y = VPoint.pix_height / 2;
                    VPoint.GetLLFromPix( p, &vp.clat, &vp.clon);

                    regionVPBuilt = true;
                }
    
            }
        }
    
        // We need to show the tilesets in reverse order to have the largest scale on top
        for(std::vector<int>::reverse_iterator rit = tiles_to_show.rbegin();
                            rit != tiles_to_show.rend(); ++rit) {
            ChartBase *chart = ChartData->OpenChartFromDBAndLock(*rit, FULL_INIT);
            ChartMBTiles *pcmbt = dynamic_cast<ChartMBTiles*>( chart );
            if(pcmbt){
                pcmbt->RenderRegionViewOnGL(*m_pcontext, vp, screen_region, screenLLRegion);
    
                //Light up the piano key if the chart was rendered
                std::vector<int>  piano_active_array_tiles = m_pParentCanvas->m_Piano->GetActiveKeyArray();
                bool bfound = false;
    
                if(std::find(piano_active_array_tiles.begin(), piano_active_array_tiles.end(), *rit) != piano_active_array_tiles.end()) {
                    bfound = true;
                }
   
                if(!bfound){
                    piano_active_array_tiles.push_back( *rit );
                    m_pParentCanvas->m_Piano->SetActiveKeyArray( piano_active_array_tiles );
                }
            }
        }
    }
}
    
    
void glChartCanvas::RenderCanvasBackingChart( ocpnDC &dc, OCPNRegion valid_region)
    {
    //  Fill the FBO with the current gshhs world chart
    int w, h;
    GetClientSize( &w, &h );
        
    glViewport( 0, 0, (GLint) m_cache_tex_x, (GLint) m_cache_tex_y );
#ifndef USE_ANDROID_GLES2
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
        
    glOrtho( 0, m_cache_tex_x, m_cache_tex_y, 0, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
    
    wxRect rtex( 0, 0, m_cache_tex_x,  m_cache_tex_y );
    ViewPort cvp = m_pParentCanvas->GetVP().BuildExpandedVP( m_cache_tex_x,  m_cache_tex_y );

    bool world_view = false;
    RenderWorldChart(dc, cvp, rtex, world_view);

    
//    dc.SetPen(wxPen(wxColour(254,254,0), 3));
//    dc.DrawLine( 0, 0, m_cache_tex_x, m_cache_tex_y);
  
    //  Reset matrices
    glViewport( 0, 0, (GLint) w, (GLint) h );
#ifndef USE_ANDROID_GLES2
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}


void glChartCanvas::FastPan(int dx, int dy)
{
#ifndef USE_ANDROID_GLES2

    int sx = GetSize().x;
    int sy = GetSize().y;
    
    //   ViewPort VPoint = m_pParentCanvas->VPoint;
    //   ViewPort svp = VPoint;
    //   svp.pix_width = svp.rv_rect.width;
    //   svp.pix_height = svp.rv_rect.height;
    
    //   OCPNRegion chart_get_region( 0, 0, m_pParentCanvas->VPoint.rv_rect.width, m_pParentCanvas->VPoint.rv_rect.height );
    
    //    ocpnDC gldc( *this );
    
    int w, h;
    GetClientSize( &w, &h );
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
    
    float vx0 = 0;
    float vy0 = 0;
    float vy = sy;
    float vx = sx;
    
    glBindTexture( g_texture_rectangle_format, 0);
    
//     /*   glColor3ub(0, 0, 120);
//      *    glBegin( GL_QUADS );
//      *    glVertex2f( 0,  0 );
//      *    glVertex2f( w, 0 );
//      *    glVertex2f( w, h );
//      *    glVertex2f( 0,  h );
//      *    glEnd();
//      */
    
    
    
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
#endif
}

void glChartCanvas::ZoomProject(float offset_x, float offset_y, float swidth, float sheight)
{
    SetCurrent(*m_pcontext);
    
    float sx = GetSize().x;
    float sy = GetSize().y;
    
    float tx, ty, tx0, ty0;
    //if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
    //  tx = sx, ty = sy;
    //else
    tx = 1, ty = 1;
    
    tx0 = ty0 = 0.;
    
    tx0 = offset_x;
    ty0 = offset_y;
    tx =  offset_x + swidth;
    ty =  offset_y + sheight;
    
    
    int w, h;
    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );
#if 0
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    
    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
    
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
//    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    
    
    float uv[8];
    float coords[8];
    
    //normal uv Texture coordinates
    uv[0] = tx0/m_cache_tex_x; uv[1] = ty/m_cache_tex_y ; uv[2] = tx/m_cache_tex_x; uv[3] = ty/m_cache_tex_y ;
    uv[4] = tx/m_cache_tex_x; uv[5] = ty0/m_cache_tex_y ; uv[6] = tx0/m_cache_tex_x; uv[7] = ty0/m_cache_tex_y ;
    
    // pixels
    coords[0] = vx0; coords[1] = vy0; coords[2] = vx; coords[3] = vy0;
    coords[4] = vx; coords[5] = vy; coords[6] = vx0; coords[7] = vy;
    
    RenderTextures(coords, uv, 4, m_pParentCanvas->GetpVP());
    
//     glBegin( GL_QUADS );
//     glTexCoord2f( tx0/m_cache_tex_x, ty/m_cache_tex_y );  glVertex2f( vx0,  vy0 );
//     glTexCoord2f( tx/m_cache_tex_x,  ty/m_cache_tex_y );  glVertex2f( vx,   vy0 );
//     glTexCoord2f( tx/m_cache_tex_x,  ty0/m_cache_tex_y ); glVertex2f( vx,   vy );
//     glTexCoord2f( tx0/m_cache_tex_x, ty0/m_cache_tex_y ); glVertex2f( vx0,  vy );
//     glEnd();
    
    glDisable( g_texture_rectangle_format );
    glBindTexture( g_texture_rectangle_format, 0);

    // For fun, we prove the coordinates of the blank area outside the chart when zooming out.
    // Bottom stripe
    //wxColour color = GetGlobalColor(_T("YELO1"));   //GREY1
    wxColour color = GetGlobalColor(_T("GREY1"));   //
    float ht = -offset_y  * ( sy / sheight);
    wxRect r(0,sy - ht,w,ht);
    RenderColorRect(r, color);

    // top stripe
    wxRect rt(0,0,w, sy - (ht + (sy * sy / sheight)));
    RenderColorRect(rt, color);
    
    // left
    float w1 = -offset_x  * sx / swidth;
    wxRect rl(0,0,w1, sy);
    RenderColorRect(rl, color);
    
    //right
    float px = w1 + sx*sx/swidth;
    wxRect rr(px, 0, sx - px, sy);
    RenderColorRect(rr, color);

    
    
    
    //  When zooming out, if we go too far, then the frame buffer is repeated on-screen due
    //  to address wrapping in the frame buffer.
    //  Detect this case, and render some simple solid covering quads to avoid a confusing display.

    
#if 0        
    if( (m_fbo_sheight > m_cache_tex_y) || (m_fbo_swidth > m_cache_tex_x) ){
        wxColour color = GetGlobalColor(_T("YELO1"));   //GREY1
        glColor3ub(color.Red(), color.Green(), color.Blue());
        
        if( m_fbo_sheight > m_cache_tex_y ){
            float h1 = sy * (1.0 - m_cache_tex_y/m_fbo_sheight) / 2.;
            
            wxRect r(0,0,w,h1);
            RenderColorRect(r, color);

            wxRect r1(0,sy - h1,w,h1);
            RenderColorRect(r1, color);
            
#if 0            
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
#endif            
        }
#endif
 
#if 0        
         // horizontal axis
         if( m_fbo_swidth > m_cache_tex_x ){
             float w1 = sx * (1.0 - m_cache_tex_x/m_fbo_swidth) / 2.;
             
            wxRect r(0,0,w1,sy);
            RenderColorRect(r, color);
            
            wxRect r1(sx-w1,0,w1,sy);
            RenderColorRect(r1, color);
            
#if 0            
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
#endif            
        }
    }
    
    
    // horizontal
    if(m_fbo_offsetx < 0){
        wxColour color = GetGlobalColor(_T("TEAL1"));
        glColor3ub(color.Red(), color.Green(), color.Blue());
        float w1 = -offset_x  * sx / swidth;
        qDebug() << "tealA" << sx << sy << w1;
        wxRect r(0,0,w1, sy);
        RenderColorRect(r, color);
/*        
        glBegin( GL_QUADS );
        glVertex2f( 0,  0 );
        glVertex2f( w1,  0 );
        glVertex2f( w1, sy );
        glVertex2f( 0, sy );
        glEnd();
*/        
    }


    qDebug() << "horiz1" << m_fbo_offsetx << m_fbo_swidth << m_cache_tex_x << m_fbo_offsetx + m_fbo_swidth;
             
    if(m_fbo_offsetx + m_fbo_swidth > m_cache_tex_x){
        wxColour color = GetGlobalColor(_T("TEAL1"));
        glColor3ub(color.Red(), color.Green(), color.Blue());
        float w1 = ((offset_x + swidth) - m_cache_tex_x) * ( sx / swidth);
        wxRect r(sx ,0, -w1, sy);
        qDebug() << "tealB" << sx << w1 << sy;
        RenderColorRect(r, color);
/*        
        glBegin( GL_QUADS );
        glVertex2f( sx,  0 );
        glVertex2f( sx - w1,  0 );
        glVertex2f( sx - w1, sy );
        glVertex2f( sx, sy );
        glEnd();
*/        
         }

    
    
    // Vertical
    if(m_fbo_offsety < 0){
        wxColour color = GetGlobalColor(_T("RED1"));
        glColor3ub(color.Red(), color.Green(), color.Blue());
        float w1 = -offset_y  * sy / sheight;
  
        wxRect r(0,0, sx, w1);
        RenderColorRect(r, color);
/*        
        glBegin( GL_QUADS );
        glVertex2f( 0,  sy );
        glVertex2f( sx,  sy );
        glVertex2f( sx, sy - w1 );
        glVertex2f( 0,  sy - w1 );
        glEnd();
*/        
    }
    
    
    
    if(m_fbo_offsety + m_fbo_sheight > m_cache_tex_y){
        wxColour color = GetGlobalColor(_T("RED1"));
        glColor3ub(color.Red(), color.Green(), color.Blue());
        float w1 = ((offset_y + sheight) - m_cache_tex_y) * ( sy / sheight);
         
        wxRect r(0,0, sx, w1);
        RenderColorRect(r, color);
/*        
        glBegin( GL_QUADS );
        glVertex2f( 0,  0 );
        glVertex2f( 0,  w1 );
        glVertex2f( sx, w1 );
        glVertex2f( sx, 0 );
        glEnd();
*/        
    }
#endif    
    SwapBuffers();
    
}
    


void glChartCanvas::onZoomTimerEvent(wxTimerEvent &event)
{
    // If m_zoomFinal is set, shortcut the timer sequence.
    
    if( (m_nRun < m_nTotal) && !m_zoomFinal){
        m_runoffsetx += m_offsetxStep; 
        if(m_offsetxStep > 0)
            m_runoffsetx = wxMin(m_runoffsetx, m_fbo_offsetx);
        else
            m_runoffsetx = wxMax(m_runoffsetx, m_fbo_offsetx);
        
        m_runoffsety += m_offsetyStep;
        if(m_offsetyStep > 0)
            m_runoffsety = wxMin(m_runoffsety, m_fbo_offsety);
        else
            m_runoffsety = wxMax(m_runoffsety, m_fbo_offsety);
        
        m_runswidth += m_swidthStep;
        if(m_swidthStep > 0)
            m_runswidth = wxMin(m_runswidth, m_fbo_swidth);
        else
            m_runswidth = wxMax(m_runswidth, m_fbo_swidth);
        
        m_runsheight += m_sheightStep;
        if(m_sheightStep > 0)
            m_runsheight = wxMin(m_runsheight, m_fbo_sheight);
        else
            m_runsheight = wxMax(m_runsheight, m_fbo_sheight);

//        qDebug() << "onZoomTimerEvent" << m_nRun << m_nTotal << m_runoffsetx << m_offsetxStep;
        
        ZoomProject(m_runoffsetx, m_runoffsety, m_runswidth, m_runsheight);
        m_nRun += m_nStep;
    }
    else{
        //qDebug() << "onZoomTimerEvent DONE" << m_nRun << m_nTotal;
        
        zoomTimer.Stop();
        if(m_zoomFinal){
            //qDebug() << "onZoomTimerEvent FINALZOOM" << m_zoomFinalZoom;


            if(m_zoomFinalZoom > 1)
                m_inFade = true;

            m_pParentCanvas->ZoomCanvas( m_zoomFinalZoom, false );

            if(m_zoomFinaldx || m_zoomFinaldy){
                m_pParentCanvas->PanCanvas( m_zoomFinaldx, m_zoomFinaldy );
            }

            if(m_zoomFinalZoom > 1){    //  only fade on ZIN

                if( (quiltHash != m_pParentCanvas->m_pQuilt->GetXStackHash()) || (refChartIndex != m_pParentCanvas->m_pQuilt->GetRefChartdbIndex()) ){

                    // Make next full render happen on new page.
                    m_cache_page = !m_cache_page;

                    RenderScene();

                    //qDebug() << "fboFade()";
                    fboFade(m_cache_tex[0],m_cache_tex[1]);
                }
                else
                    m_inFade = false;

            }
            else
                m_inFade = false;


        }
        m_zoomFinal = false;
    }
    
}

    
void glChartCanvas::FastZoom(float factor, float cp_x, float cp_y, float post_x, float post_y)
{
    //qDebug() << "FastZoom" << factor << post_x << post_y << m_nRun;
    
    int sx = GetSize().x;
    int sy = GetSize().y;
   
    m_lastfbo_offsetx = m_fbo_offsetx;
    m_lastfbo_offsety = m_fbo_offsety;
    m_lastfbo_swidth = m_fbo_swidth;
    m_lastfbo_sheight = m_fbo_sheight;
    
    float curr_fbo_offset_x = m_fbo_offsetx;
    float curr_fbo_offset_y = m_fbo_offsety;
    float curr_fbo_swidth = m_fbo_swidth;
    float curr_fbo_sheight = m_fbo_sheight;
    

    float fx = (float)cp_x / sx;
    float fy = 1.0 - (float)cp_y / sy;
    if(factor < 1.0f){
//        fx = 0.5;               // center screen
//        fy = 0.5;
    }

    float fbo_ctr_x = curr_fbo_offset_x + (curr_fbo_swidth * fx);
    float fbo_ctr_y = curr_fbo_offset_y + (curr_fbo_sheight * fy);
    
    m_fbo_swidth  = curr_fbo_swidth / factor;
    m_fbo_sheight = curr_fbo_sheight / factor;
    
    m_fbo_offsetx = fbo_ctr_x - (m_fbo_swidth * fx);
    m_fbo_offsety = fbo_ctr_y - (m_fbo_sheight * fy);
    

    m_fbo_offsetx += post_x;
    m_fbo_offsety += post_y;
    
//     if(factor < 1.0f){        
//         ZoomProject(m_fbo_offsetx, m_fbo_offsety, m_fbo_swidth, m_fbo_sheight);
//         zoomTimer.Stop();
//         m_zoomFinal = false;
//     }
//    else
    {
    m_nStep = 20; 
    m_nTotal = 100;
    
    m_nStep = 10; 
    m_nTotal = 40;
    
    m_nRun = 0;
    
    float perStep = m_nStep / m_nTotal;
    
 
    if(zoomTimer.IsRunning()){
        m_offsetxStep = (m_fbo_offsetx - m_runoffsetx) * perStep;
        m_offsetyStep = (m_fbo_offsety - m_runoffsety) * perStep;
        m_swidthStep = (m_fbo_swidth - m_runswidth) * perStep;
        m_sheightStep = (m_fbo_sheight - m_runsheight) * perStep;
        
    }
    else{
        m_offsetxStep = (m_fbo_offsetx - m_lastfbo_offsetx) * perStep;
        m_offsetyStep = (m_fbo_offsety - m_lastfbo_offsety) * perStep;
        m_swidthStep = (m_fbo_swidth - m_lastfbo_swidth) * perStep;
        m_sheightStep = (m_fbo_sheight - m_lastfbo_sheight) * perStep;

        m_runoffsetx = m_lastfbo_offsetx;
        m_runoffsety = m_lastfbo_offsety;
        m_runswidth = m_lastfbo_swidth;
        m_runsheight = m_lastfbo_sheight;
    }
    
    if(!zoomTimer.IsRunning())
        zoomTimer.Start(m_nStep);
        m_zoomFinal = false;
    }
}

#ifdef __OCPN__ANDROID__


void glChartCanvas::OnEvtPanGesture( wxQT_PanGestureEvent &event)
{
    //qDebug() << "OnEvtPanGesture" << m_pParentCanvas->m_canvasIndex << event.cursor_pos.x;

   
    if( m_pParentCanvas->isRouteEditing() || m_pParentCanvas->isMarkEditing() )
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
            m_binGesture = true;
            //qDebug() << "pg1";
            break;
            
        case GestureUpdated:
            if(m_binPan){
                if(!g_GLOptions.m_bUseCanvasPanning || m_bfogit){
                    //qDebug() << "slowpan" << dx << dy;
                    
                    m_pParentCanvas->FreezePiano();
                    m_pParentCanvas->PanCanvas( dx, -dy );
                    m_pParentCanvas->ThawPiano();

                }
                else{
                    FastPan( dx, dy ); 
                }
                
                
                panx -= dx;
                pany -= dy;
            }
            break;
            
        case GestureFinished:
            //qDebug() << "panGestureFinished";

            m_pParentCanvas->UpdateCanvasControlBar();
            
            m_binPan = false;
            m_gestureFinishTimer.Start(500, wxTIMER_ONE_SHOT);
            
            break;
            
        case GestureCanceled:
            m_binPan = false; 
            m_gestureFinishTimer.Start(500, wxTIMER_ONE_SHOT);
            break;
            
        default:
            break;
    }
    
    m_bgestureGuard = true;
    m_gestureEeventTimer.Start(500, wxTIMER_ONE_SHOT);
    m_bforcefull = false;

    //qDebug() << "panGestureDone";
    
}

float zoom_inc = 1.0;
bool first_zout = false;

void glChartCanvas::OnEvtPinchGesture( wxQT_PinchGestureEvent &event)
{
    
    float zoom_gain = 1.0;
    float zout_gain = 1.0;

    float zoom_val;
    float total_zoom_val;

    float max_zoom_scale = 1000.;
    float min_zoom_scale = 2e8;

    if( event.GetScaleFactor() > 1)
        zoom_val = ((event.GetScaleFactor() - 1.0) * zoom_gain) + 1.0;
    else
        zoom_val = 1.0 - ((1.0 - event.GetScaleFactor()) * zout_gain);

    if( event.GetTotalScaleFactor() > 1)
        total_zoom_val = ((event.GetTotalScaleFactor() - 1.0) * zoom_gain) + 1.0;
    else
#if 0        
        total_zoom_val = 1.0 - ((1.0 - event.GetTotalScaleFactor()) * zout_gain);

    double projected_scale = cc1->GetVP().chart_scale / total_zoom_val;

    // Max zoom in is set by scale of quilt reference chart, consistent with chart render limits set elsewhere.
    float max_zoom_scale = 1000.;
    if( cc1->GetVP().b_quilt) {
        int ref_index = cc1->GetQuiltRefChartdbIndex();
//         if((ref_index >= 0) && ChartData){
//             max_zoom_scale = ChartData->GetDBChartScale(ref_index) / 8.0;
//         }
    }


    float min_zoom_scale = 2e8;

=======
#endif

        total_zoom_val = 1.0 - ((1.0 - event.GetTotalScaleFactor()) * zoom_gain);
 
    double projected_scale = m_pParentCanvas->GetVP().chart_scale / total_zoom_val;
    
    switch(event.GetState()){
        case GestureStarted:
            first_zout = false;
            m_binPinch = true;
            m_binPan = false;   // cancel any tentative pan gesture, in case the "pan cancel" event was lost
            m_binGesture = true;
            //qDebug() << "pg2";
            m_pinchStart = event.GetCenterPoint();
            m_lpinchPoint = m_pinchStart;

            m_pParentCanvas->GetCanvasPixPoint(event.GetCenterPoint().x, event.GetCenterPoint().y, m_pinchlat, m_pinchlon);
//            qDebug() << "center" << event.GetCenterPoint().x << event.GetCenterPoint().y;

            m_cc_x =  m_fbo_offsetx + (m_fbo_swidth/2);
            m_cc_y =  m_fbo_offsety + (m_fbo_sheight/2);

            // Render the full charts with overlay objects onto the frame buffer.
            SetCurrent(*m_pcontext);
            RenderScene( );

            zoom_inc = 1.0;
            break;
            
        case GestureUpdated:
            if(g_GLOptions.m_bUseCanvasPanning){
                
                if( projected_scale < min_zoom_scale){
                    wxPoint pinchPoint = event.GetCenterPoint();

                     float dx = pinchPoint.x - m_lpinchPoint.x;
                     float dy = pinchPoint.y - m_lpinchPoint.y;

                     FastZoom(zoom_val, m_pinchStart.x, m_pinchStart.y, -dx / total_zoom_val, dy / total_zoom_val);

                     m_lpinchPoint = pinchPoint;

                }
            }
            else{
                //qDebug() << "update totalzoom" << total_zoom_val << projected_scale;
                if( 1 || ((total_zoom_val > 1) && !first_zout)){           // Zoom in
                    wxPoint pinchPoint = event.GetCenterPoint();

                    float dx = pinchPoint.x - m_lpinchPoint.x;
                    float dy = pinchPoint.y - m_lpinchPoint.y;

                    if(!m_inFade){
                        if( (projected_scale > max_zoom_scale) && ( projected_scale < min_zoom_scale))
                            FastZoom(zoom_val, m_pinchStart.x, m_pinchStart.y, -dx / total_zoom_val, dy / total_zoom_val);
                    }
                    else
                        qDebug() << "Skip fastzoom on zin";

                    m_lpinchPoint = pinchPoint;

                }
                else{
                    first_zout = true;
                    zoom_inc *= zoom_val;
                    if((zoom_inc < 0.9) || (zoom_inc > 1.1)){
                        m_pParentCanvas->ZoomCanvas( zoom_inc, false );
                        zoom_inc = 1.0;
                    }

                        wxPoint pinchPoint = event.GetCenterPoint();
                        float dx = pinchPoint.x - m_lpinchPoint.x;
                        float dy = pinchPoint.y - m_lpinchPoint.y;
                        m_pParentCanvas->PanCanvas( -dx, -dy );
                        m_lpinchPoint = pinchPoint;


//                         SetCurrent(*m_pcontext);
//                         RenderScene();
//                         g_Piano->DrawGL(cc1->m_canvas_height - g_Piano->GetHeight());
//                         SwapBuffers();
                
            }
            }

            break;
            
        case GestureFinished:{
//            qDebug() << "finish totalzoom" << total_zoom_val << projected_scale;

            float cc_x =  m_fbo_offsetx + (m_fbo_swidth/2);
            float cc_y =  m_fbo_offsety + (m_fbo_sheight/2);
            float dy = 0;
            float dx = 0;

            float tzoom = total_zoom_val;

            if( projected_scale >= min_zoom_scale)
                tzoom = m_pParentCanvas->GetVP().chart_scale / min_zoom_scale;

            if( projected_scale < max_zoom_scale)
                tzoom = m_pParentCanvas->GetVP().chart_scale / max_zoom_scale;

            dx = (cc_x - m_cc_x) * tzoom;
            dy = -(cc_y - m_cc_y) * tzoom;

            if(m_inFade){
                qDebug() << "Fade interrupt";
                m_binPinch = false;
                m_gestureFinishTimer.Start(500, wxTIMER_ONE_SHOT);
                break;
            }

            if(zoomTimer.IsRunning()){
//                qDebug() << "Final zoom";
                m_zoomFinal = true;
                m_zoomFinalZoom = tzoom;
                m_zoomFinaldx = dx;
                m_zoomFinaldy = dy;
            }

            else{
                double final_projected_scale = m_pParentCanvas->GetVP().chart_scale / tzoom;
                //qDebug() << "Final pinchB" << tzoom << final_projected_scale;
                    
                if( final_projected_scale < min_zoom_scale){
                    //qDebug() << "zoomit";
                    m_pParentCanvas->ZoomCanvas( tzoom, false );
                    m_pParentCanvas->PanCanvas( dx, dy );
                    m_pParentCanvas->m_pQuilt->Invalidate();
                    m_bforcefull = true;
                    
                }
                else{
                    double new_scale = m_pParentCanvas->GetCanvasScaleFactor() / min_zoom_scale;
                    //qDebug() << "clampit";
                    m_pParentCanvas->SetVPScale( new_scale ); 
                    m_pParentCanvas->m_pQuilt->Invalidate();
                    m_bforcefull = true;
                }
                            
            }


//                if( projected_scale < 3e8)
//                    m_pParentCanvas->ZoomCanvas( total_zoom_val, false );
//                else
//                    m_pParentCanvas->ZoomCanvas(m_pParentCanvas->GetVP().chart_scale / 3e8, false);
            
             m_binPinch = false;
             m_gestureFinishTimer.Start(500, wxTIMER_ONE_SHOT);
             break;
        }
            
        case GestureCanceled:
            m_binPinch = false;
            m_gestureFinishTimer.Start(500, wxTIMER_ONE_SHOT);
            break;
            
        default:
            break;
    }

    m_bgestureGuard = true;
//    m_bpinchGuard = true;
    m_gestureEeventTimer.Start(500, wxTIMER_ONE_SHOT);
    
}

void glChartCanvas::onGestureTimerEvent(wxTimerEvent &event)
{
    //  On some devices, the pan GestureFinished event fails to show up
    //  Watch for this case, and fix it.....
    //qDebug() << "onGestureTimerEvent";
    
    if(m_binPan){
        m_binPan = false;
        Invalidate();
        Update();
    }
    m_bgestureGuard = false;
    m_bpinchGuard = false;
    m_binGesture = false;
    m_bforcefull = false;
    
}

void glChartCanvas::onGestureFinishTimerEvent(wxTimerEvent &event)
{
    //qDebug() << "onGestureFinishTimerEvent";
    
    // signal gesture is finished after a delay
    m_binGesture = false;
    m_bforcefull = false;

}



#endif

void glChartCanvas::configureShaders( ViewPort &vp)
{
#ifdef USE_ANDROID_GLES2 
    mat4x4 I;
    mat4x4_identity(I);
    
    ViewPort *pvp = (ViewPort *)&vp;
    
    glUseProgram(color_tri_shader_program);
    GLint matloc = glGetUniformLocation(color_tri_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)pvp->vp_transform); 
    GLint transloc = glGetUniformLocation(color_tri_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I); 
    
    glUseProgram(texture_2D_shader_program);
    matloc = glGetUniformLocation(texture_2D_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)pvp->vp_transform); 
    transloc = glGetUniformLocation(texture_2D_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I); 
    
    glUseProgram(circle_filled_shader_program);
    matloc = glGetUniformLocation(circle_filled_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)pvp->vp_transform); 
    transloc = glGetUniformLocation(circle_filled_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I); 

    glUseProgram(texture_2DA_shader_program);
    matloc = glGetUniformLocation(texture_2DA_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)pvp->vp_transform); 
    transloc = glGetUniformLocation(texture_2DA_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I); 
    
    
#endif
}


GLint m_fadeTexOld;
GLint m_fadeTexNew;
float m_fadeFactor;
int n_fade;
unsigned int s_cachepage;

void glChartCanvas::onFadeTimerEvent(wxTimerEvent &event)
{
#ifdef USE_ANDROID_GLES2    
//     if(m_cache_page != s_cachepage)
//         qDebug() << "CACHE PAGE lost";
    
    if(m_fadeFactor < 0.2)
        m_fadeFactor = 0;
    
    //qDebug() << "fade" << m_fadeFactor << m_cache_page << n_fade;

    SetCurrent(*m_pcontext);
    
    int sx = GetSize().x;
    int sy = GetSize().y;
    
    float coords[8];
    float uv[8];
    
    float divx, divy;
    //  Normalize, or not?
    if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format ){
        divx = divy = 1.0f;
    }
    else{
        divx = m_cache_tex_x;
        divy = m_cache_tex_y;
    }
    
///    ZoomProject(m_runoffsetx, m_runoffsety, m_runswidth, m_runsheight);
    
///
///    tx = 1, ty = 1;
    
///    tx0 = ty0 = 0.;
    
    float tx0 = m_runoffsetx;
    float ty0 = m_runoffsety;
    float tx =  m_runoffsetx + m_runswidth;
    float ty =  m_runoffsety + m_runsheight;
    
    
    float vx0 = 0;
    float vy0 = 0;
    float vy = sy;
    float vx = sx;

    // pixels
    coords[0] = vx0; coords[1] = vy0; coords[2] = vx; coords[3] = vy0;
    coords[4] = vx; coords[5] = vy; coords[6] = vx0; coords[7] = vy;
    
    
    // uv coordinates for first texture
    uv[0] = tx0/m_cache_tex_x; uv[1] = ty/m_cache_tex_y ; uv[2] = tx/m_cache_tex_x; uv[3] = ty/m_cache_tex_y ;
    uv[4] = tx/m_cache_tex_x; uv[5] = ty0/m_cache_tex_y ; uv[6] = tx0/m_cache_tex_x; uv[7] = ty0/m_cache_tex_y ;
    

    
    // uv coordinates for second texture
    float tx02 = m_fbo_offsetx/divx;
    float ty02 = m_fbo_offsety/divy;
    float tx2 =  (m_fbo_offsetx + m_fbo_swidth)/divx;
    float ty2 =  (m_fbo_offsety + m_fbo_sheight)/divy;
    
    //normal uv
    float uv2[8];
    uv2[0] = tx02; uv2[1] = ty2; uv2[2] = tx2; uv2[3] = ty2;
    uv2[4] = tx2; uv2[5] = ty02; uv2[6] = tx02; uv2[7] = ty02;
    
    
///    
    
//     float tx0 = m_fbo_offsetx/divx;
//     float ty0 = m_fbo_offsety/divy;
//     float tx =  (m_fbo_offsetx + m_fbo_swidth)/divx;
//     float ty =  (m_fbo_offsety + m_fbo_sheight)/divy;
//     
//     //normal uv
//     uv[0] = tx0; uv[1] = ty; uv[2] = tx; uv[3] = ty;
//     uv[4] = tx; uv[5] = ty0; uv[6] = tx0; uv[7] = ty0;
//     
//     // pixels
//     coords[0] = 0; coords[1] = 0; coords[2] = sx; coords[3] = 0;
//     coords[4] = sx; coords[5] = sy; coords[6] = 0; coords[7] = sy;
    
    glEnable(GL_BLEND);
    
    //glClearColor(0.f, 0.f, 0.f, 1.0f);
    wxColour color = GetGlobalColor( _T ( "NODTA" ) );
    glClearColor( color.Red() / 256., color.Green() / 256. , color.Blue()/ 256. , 1.0);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    // Render old texture using shader
    glUseProgram( fade_texture_2D_shader_program );
    
    // Get pointers to the attributes in the program.
    GLint mPosAttrib = glGetAttribLocation( fade_texture_2D_shader_program, "aPos" );
    GLint mUvAttrib  = glGetAttribLocation( fade_texture_2D_shader_program, "aUV" );
    GLint mUvAttrib2  = glGetAttribLocation( fade_texture_2D_shader_program, "aUV2" );
    
    // Set up the texture sampler to texture unit 0
    GLint texUni = glGetUniformLocation( fade_texture_2D_shader_program, "uTex" );
    glUniform1i( texUni, 0 );

    GLint texUni2 = glGetUniformLocation( fade_texture_2D_shader_program, "uTex2" );
    glUniform1i( texUni2, 1 );
    
    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
    // Set the attribute mPosAttrib with the vertices in the screen coordinates...
    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
    // ... and enable it.
    glEnableVertexAttribArray( mPosAttrib );
    
    // Set the attribute mUvAttrib with the vertices in the GL coordinates...
    glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uv );
    // ... and enable it.
    glEnableVertexAttribArray( mUvAttrib );

    // Set the attribute mUvAttrib with the vertices in the GL coordinates...
    glVertexAttribPointer( mUvAttrib2, 2, GL_FLOAT, GL_FALSE, 0, uv2 );
    // ... and enable it.
    glEnableVertexAttribArray( mUvAttrib2 );
    
    
    GLint matloc = glGetUniformLocation(fade_texture_2D_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)(m_pParentCanvas->GetpVP()->vp_transform) ); 

//     GLint transloc = glGetUniformLocation(fade_texture_2D_shader_program,"trans");
//     float colVec[4] = {1.0, 1.0, 1.0, 1.0};
//     colVec[3] = m_fadeFactor;
//     glUniform4fv( transloc, 1, colVec ); 

    GLint alphaloc = glGetUniformLocation(fade_texture_2D_shader_program,"texAlpha");
    glUniform1f(alphaloc, m_fadeFactor);
    
    // Select the active texture unit.
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( g_texture_rectangle_format, m_cache_tex[!m_cache_page]);
    
    // Select the active texture unit.
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
    
    
    // Perform the actual drawing.
    
    // For some reason, glDrawElements is busted on Android
    // So we do this a hard ugly way, drawing two triangles...
    #if 1
    GLushort indices1[] = {0,1,3,2}; 
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices1);
    #else

    float co1[8];
    co1[0] = coords[0];
    co1[1] = coords[1];
    co1[2] = coords[2];
    co1[3] = coords[3];
    co1[4] = coords[6];
    co1[5] = coords[7];
    co1[6] = coords[4];
    co1[7] = coords[5];
    
    float tco1[8];
    tco1[0] = uv[0];
    tco1[1] = uv[1];
    tco1[2] = uv[2];
    tco1[3] = uv[3];
    tco1[4] = uv[6];
    tco1[5] = uv[7];
    tco1[6] = uv[4];
    tco1[7] = uv[5];
    
    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, co1 );
    glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, tco1 );
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    #endif
 
#if 0    
    // Render new texture using shader
    if(0){
        
             float tx0 = m_fbo_offsetx/divx;
             float ty0 = m_fbo_offsety/divy;
             float tx =  (m_fbo_offsetx + m_fbo_swidth)/divx;
             float ty =  (m_fbo_offsety + m_fbo_sheight)/divy;
             
             //normal uv
             uv[0] = tx0; uv[1] = ty; uv[2] = tx; uv[3] = ty;
             uv[4] = tx; uv[5] = ty0; uv[6] = tx0; uv[7] = ty0;
             
             // pixels
             coords[0] = 0; coords[1] = 0; coords[2] = sx; coords[3] = 0;
             coords[4] = sx; coords[5] = sy; coords[6] = 0; coords[7] = sy;
        
        glUseProgram( fade_texture_2D_shader_program );
        
        // Get pointers to the attributes in the program.
        GLint mPosAttrib = glGetAttribLocation( fade_texture_2D_shader_program, "aPos" );
        GLint mUvAttrib  = glGetAttribLocation( fade_texture_2D_shader_program, "aUV" );
        
        // Set up the texture sampler to texture unit 0
        GLint texUni = glGetUniformLocation( fade_texture_2D_shader_program, "uTex" );
        glUniform1i( texUni, 0 );
        
        // Disable VBO's (vertex buffer objects) for attributes.
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        
        // Set the attribute mPosAttrib with the vertices in the screen coordinates...
        glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
        // ... and enable it.
        glEnableVertexAttribArray( mPosAttrib );
        
        // Set the attribute mUvAttrib with the vertices in the GL coordinates...
        glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uv );
        // ... and enable it.
        glEnableVertexAttribArray( mUvAttrib );
        
        
        GLint matloc = glGetUniformLocation(fade_texture_2D_shader_program,"MVMatrix");
        glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)(cc1->GetpVP()->vp_transform) ); 
        
        GLint transloc = glGetUniformLocation(fade_texture_2D_shader_program,"trans");
        float colVec[4] = {1.0, 1.0, 1.0, 1.0};
        colVec[3] = 1.0 - m_fadeFactor;
        glUniform4fv( transloc, 1, colVec ); 
        
        // Select the active texture unit.
        glActiveTexture( GL_TEXTURE0 );
        
        // Bind our texture to the texturing target.
        glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
        
        
        // Perform the actual drawing.
        
        // For some reason, glDrawElements is busted on Android
        // So we do this a hard ugly way, drawing two triangles...
        #if 1
        GLushort indices1[] = {0,1,3,2}; 
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices1);
        #else
        float co1[8];
        co1[0] = coords[0];
        co1[1] = coords[1];
        co1[2] = coords[2];
        co1[3] = coords[3];
        co1[4] = coords[6];
        co1[5] = coords[7];
        co1[6] = coords[4];
        co1[7] = coords[5];
        
        float tco1[8];
        tco1[0] = uv[0];
        tco1[1] = uv[1];
        tco1[2] = uv[2];
        tco1[3] = uv[3];
        tco1[4] = uv[6];
        tco1[5] = uv[7];
        tco1[6] = uv[4];
        tco1[7] = uv[5];
        
        glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, co1 );
        glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, tco1 );
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        #endif
    }
#endif

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( g_texture_rectangle_format, 0);
    
    // Select the active texture unit.
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( g_texture_rectangle_format, 0);
    
    m_pParentCanvas->GetPiano()->DrawGL(m_pParentCanvas->m_canvas_height - m_pParentCanvas->GetPiano()->GetHeight());

    SwapBuffers();
    
    n_fade++;
 
    m_fadeFactor *= 0.8;
    if(m_fadeFactor > 0.1){
        m_fadeTimer.Start(5, wxTIMER_ONE_SHOT);
    }
    else{
        //qDebug() << "fade DONE";
        m_inFade = false;
        m_pParentCanvas->GetPiano()->DrawGL(m_pParentCanvas->m_canvas_height - m_pParentCanvas->GetPiano()->GetHeight());
        
        glDisable(GL_BLEND);
        
        Refresh();
    }
        
#endif        
}


void glChartCanvas::fboFade(GLint tex0, GLint tex1)
{
    // we have 2 FBO textures, and we want to fade between the two
    m_fadeTexOld = tex0;
    m_fadeTexNew = tex1;
    m_fadeFactor = 1.0;
    n_fade = 0;
    
    s_cachepage = m_cache_page;
    
    m_inFade = true;
    
    // Start a timer
    m_fadeTimer.Start(10, wxTIMER_ONE_SHOT);
}

void glChartCanvas::RenderTextures(float *coords, float *uvCoords, int nVertex, ViewPort *vp)
{
#ifdef USE_ANDROID_GLES2
    int nl = nVertex / 4;
    float *lc = coords;
    float *luv = uvCoords;
    
    while(nl){
        RenderSingleTexture(lc, luv, vp, 0, 0, 0);
        
        lc += 8;
        luv += 8;
        nl--;
    }
    
#else
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glTexCoordPointer(2, GL_FLOAT, 2*sizeof(GLfloat), uvCoords);
    glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), coords);
    glDrawArrays(GL_QUADS, 0, 4);
    
#endif
    
    return;
}

void glChartCanvas::RenderSingleTexture(float *coords, float *uvCoords,ViewPort *vp, float dx, float dy, float angle_rad)
{
#ifdef USE_ANDROID_GLES2
    //build_texture_shaders();
    glUseProgram( texture_2D_shader_program );
    
    // Get pointers to the attributes in the program.
    GLint mPosAttrib = glGetAttribLocation( texture_2D_shader_program, "aPos" );
    GLint mUvAttrib  = glGetAttribLocation( texture_2D_shader_program, "aUV" );
    
    // Set up the texture sampler to texture unit 0
    GLint texUni = glGetUniformLocation( texture_2D_shader_program, "uTex" );
    glUniform1i( texUni, 0 );
    
    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
    // Set the attribute mPosAttrib with the vertices in the screen coordinates...
    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
    // ... and enable it.
    glEnableVertexAttribArray( mPosAttrib );
    
    // Set the attribute mUvAttrib with the vertices in the GL coordinates...
    glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uvCoords );
    // ... and enable it.
    glEnableVertexAttribArray( mUvAttrib );
    
    // Rotate 
    mat4x4 I, Q;
    mat4x4_identity(I);
    mat4x4_rotate_Z(Q, I, angle_rad);
    
    // Translate
    Q[3][0] = dx;
    Q[3][1] = dy;
    
    
    //mat4x4 X;
    //mat4x4_mul(X, (float (*)[4])vp->vp_transform, Q);
    
    GLint matloc = glGetUniformLocation(texture_2D_shader_program,"TransformMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q); 
    
    // Select the active texture unit.
    glActiveTexture( GL_TEXTURE0 );
    
    // Bind our texture to the texturing target.
    //glBindTexture( GL_TEXTURE_2D, tex );
    
    // Perform the actual drawing.
    
    // For some reason, glDrawElements is busted on Android
    // So we do this a hard ugly way, drawing two triangles...
    #if 0
    GLushort indices1[] = {0,1,3,2}; 
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices1);
    #else
    
    float co1[8];
    co1[0] = coords[0];
    co1[1] = coords[1];
    co1[2] = coords[2];
    co1[3] = coords[3];
    co1[4] = coords[6];
    co1[5] = coords[7];
    co1[6] = coords[4];
    co1[7] = coords[5];
    
    float tco1[8];
    tco1[0] = uvCoords[0];
    tco1[1] = uvCoords[1];
    tco1[2] = uvCoords[2];
    tco1[3] = uvCoords[3];
    tco1[4] = uvCoords[6];
    tco1[5] = uvCoords[7];
    tco1[6] = uvCoords[4];
    tco1[7] = uvCoords[5];
    
    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, co1 );
    glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, tco1 );
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    #endif
    
    
#else
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glPushMatrix();
    glTranslatef(dx, dy, 0);
    glRotatef(180/PI * angle_rad, 0, 0, 1);

    glTexCoordPointer(2, GL_FLOAT, 2*sizeof(GLfloat), uvCoords);
    glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), coords);
    glDrawArrays(GL_QUADS, 0, 4);
    glPopMatrix();
    
#endif
    
    return;
}


void glChartCanvas::RenderColorRect(wxRect r, wxColor &color)
{
    
#ifdef USE_ANDROID_GLES2
    glUseProgram(color_tri_shader_program);
    
    float pf[8];
    pf[0] = r.x + r.width; pf[1] = r.y; pf[2] = r.x; pf[3] = r.y; pf[4] = r.x + r.width; pf[5] = r.y + r.height;
    pf[6] = r.x; pf[7] = r.y + r.height;
    
    GLint pos = glGetAttribLocation(color_tri_shader_program, "position");
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), pf);
    glEnableVertexAttribArray(pos);
    
    GLint matloc = glGetUniformLocation(color_tri_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)m_pParentCanvas->GetpVP()->vp_transform); 
    
    float colorv[4];
    colorv[0] = color.Red() / float(256);
    colorv[1] = color.Green() / float(256);
    colorv[2] = color.Blue() / float(256);
    colorv[3] = 1.0;
    
    GLint colloc = glGetUniformLocation(color_tri_shader_program,"color");
    glUniform4fv(colloc, 1, colorv);
    
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    //            pf[0] = r.x; pf[1] = r.y; pf[2] = r.x + r.width; pf[3] = r.y + r.height; pf[4] = r.x; pf[5] = r.y + r.height;
    
    //            glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), pf);
    //            glEnableVertexAttribArray(pos);
    //            glDrawArrays(GL_TRIANGLES, 0, 3);
    
    
#else
    glColor3ub(color.Red(), color.Green(), color.Blue());
    
    glBegin( GL_QUADS );
    glVertex2f( r.x,           r.y );
    glVertex2f( r.x + r.width, r.y );
    glVertex2f( r.x + r.width, r.y + r.height );
    glVertex2f( r.x,           r.y + r.height );
    glEnd();
#endif
}

void glChartCanvas::RenderScene( bool bRenderCharts, bool bRenderOverlays )
{
    //qDebug() << "RenderScene";
    
#ifdef USE_ANDROID_GLES2

    ViewPort VPoint = m_pParentCanvas->VPoint;
    ocpnDC gldc( *this );

    int w, h;
    GetClientSize( &w, &h );
    int sx = GetSize().x;
    int sy = GetSize().y;
    
    OCPNRegion screen_region(wxRect(0, 0, VPoint.pix_width, VPoint.pix_height));

    glViewport( 0, 0, (GLint) w, (GLint) h );

    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    // Make sure we have a valid quilt composition
    m_pParentCanvas->m_pQuilt->Compose(m_pParentCanvas->VPoint);
    
    // set opengl settings that don't normally change
    // this should be able to go in SetupOpenGL, but it's
    // safer here incase a plugin mangles these
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );



   // enable rendering to texture in framebuffer object
   glBindFramebuffer( GL_FRAMEBUFFER_EXT, m_fb0 );

   glFramebufferTexture2D( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, g_texture_rectangle_format, m_cache_tex[m_cache_page], 0 );
                    
    m_fbo_offsetx = 0;
    m_fbo_offsety = 0;
    m_fbo_swidth = sx;
    m_fbo_sheight = sy;
    
    if(bRenderCharts)
        RenderCharts(gldc, screen_region);
    
    if(bRenderOverlays){
        RenderS57TextOverlay( m_pParentCanvas->VPoint );
        RenderMBTilesOverlay( m_pParentCanvas->VPoint );
        DrawStaticRoutesTracksAndWaypoints( m_pParentCanvas->VPoint );
        DrawDynamicRoutesTracksAndWaypoints( VPoint );
        DrawFloatingOverlayObjects( m_gldc );
}

    // All done, so disable Render to FBO
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

#endif

}
