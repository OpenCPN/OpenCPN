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

#ifndef __GLCHARTCANVAS_H__
#define __GLCHARTCANVAS_H__

#include <wx/glcanvas.h>

#include "dychart.h"

#include "ocpn_types.h"
#include "OCPNRegion.h"
#include "LLRegion.h"
#include "viewport.h"
#include "TexFont.h"
#include "ocpndc.h"

 #define FORMAT_BITS           GL_RGB

#ifdef __OCPN__ANDROID__
#include "wx/qt/private/wxQtGesture.h"
#endif

// Correct some deficincies in MacOS OpenGL include files
#ifdef __WXOSX__
typedef void (*PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (*PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (*PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (*PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (*PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (*PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (*PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (*PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, GLvoid *img);
typedef GLenum (*PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (*PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (*PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void (*PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef void (*PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (*PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (*PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (*PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (*PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (*PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
#endif


class glTexFactory;
class ChartCanvas;

#define GESTURE_EVENT_TIMER 78334
#define ZOOM_TIMER 78335
#define GESTURE_FINISH_TIMER 78336
#define TEX_FADE_TIMER 78337

typedef class{
  public:
    wxString Renderer;
    GLenum TextureRectangleFormat;
    
    bool bOldIntel;
    bool bCanDoVBO;
    bool bCanDoFBO;
    
    //      Vertex Buffer Object (VBO) support
    PFNGLGENBUFFERSPROC                 m_glGenBuffers;
    PFNGLBINDBUFFERPROC                 m_glBindBuffer;
    PFNGLBUFFERDATAPROC                 m_glBufferData;
    PFNGLDELETEBUFFERSPROC              m_glDeleteBuffers;

    //      Frame Buffer Object (FBO) support
    PFNGLGENFRAMEBUFFERSEXTPROC         m_glGenFramebuffers;
    PFNGLGENRENDERBUFFERSEXTPROC        m_glGenRenderbuffers;
    PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    m_glFramebufferTexture2D;
    PFNGLBINDFRAMEBUFFEREXTPROC         m_glBindFramebuffer;
    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC m_glFramebufferRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEEXTPROC     m_glRenderbufferStorage;
    PFNGLBINDRENDERBUFFEREXTPROC        m_glBindRenderbuffer;
    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  m_glCheckFramebufferStatus;
    PFNGLDELETEFRAMEBUFFERSEXTPROC      m_glDeleteFramebuffers;
    PFNGLDELETERENDERBUFFERSEXTPROC     m_glDeleteRenderbuffers;
    
    PFNGLCOMPRESSEDTEXIMAGE2DPROC       m_glCompressedTexImage2D;
    PFNGLGETCOMPRESSEDTEXIMAGEPROC      m_glGetCompressedTexImage;
    PFNGLBUFFERPARAMETERSIVPROC         m_glGetBufferParameteriv;
    
}OCPN_GLCaps;

void GetglEntryPoints( OCPN_GLCaps *pcaps );
GLboolean QueryExtension( const char *extName );


class ocpnGLOptions
{
public:
    bool m_bUseAcceleratedPanning;
    bool m_bUseCanvasPanning;
    
    bool m_bTextureCompression;
    bool m_bTextureCompressionCaching;

    int m_iTextureDimension;
    int m_iTextureMemorySize;
    
    bool m_GLPolygonSmoothing;
    bool m_GLLineSmoothing;
};


class glTestCanvas : public wxGLCanvas
{
public:
    glTestCanvas(wxWindow *parent);
    ~glTestCanvas() {};

};


class ocpnDC;
class emboss_data;
class Route;
class ChartBaseBSB;

class glChartCanvas : public wxGLCanvas
{
public:
    static bool CanClipViewport(const ViewPort &vp);
    static ViewPort ClippedViewport(const ViewPort &vp, const LLRegion &region);

    static bool HasNormalizedViewPort(const ViewPort &vp);
    static void MultMatrixViewPort(ViewPort &vp, float lat=0, float lon=0);
    static ViewPort NormalizedViewPort(const ViewPort &vp, float lat=0, float lon=0);

    static void RotateToViewPort(const ViewPort &vp);
    static void DrawRegion( ViewPort &vp, const LLRegion &region);
    static void SetClipRegion( ViewPort &vp, const LLRegion &region);
    static void SetClipRect(const ViewPort &vp, const wxRect &rect, bool g_clear=false);
    static void DisableClipRegion();
    void SetColorScheme(ColorScheme cs);
    void RenderTextures(float *coords, float *uvCoords, int nVertex, ViewPort *vp);
    static void RenderSingleTexture(float *coords, float *uvCoords,ViewPort *vp, float dx, float dy, float angle);
    void RenderColorRect(wxRect r, wxColor &color);
    
    static bool         s_b_useScissorTest;
    static bool         s_b_useStencil;
    static bool         s_b_useStencilAP;
    static bool         s_b_useFBO;
    
    void SendJSONConfigMessage();
    
    glChartCanvas(wxWindow *parent, wxGLCanvas *share = NULL);
    
    ~glChartCanvas();

    void Init();    
    void SetContext(wxGLContext *pcontext) { m_pcontext = pcontext; }

    void OnPaint(wxPaintEvent& event);
    void OnEraseBG(wxEraseEvent& evt);
    void Render();
    void OnActivate ( wxActivateEvent& event );
    void OnSize ( wxSizeEvent& event );
    void MouseEvent(wxMouseEvent& event);
    void FastPan(int dx, int dy);
    void FastZoom(float factor, float cp_x, float cp_y, float post_x, float post_y);
//    void RenderCanvasBackingChart( ocpnDC dc, OCPNRegion chart_get_region);
//    void FastZoom(float factor);
    void RenderCanvasBackingChart( ocpnDC &dc, OCPNRegion chart_get_region);
    
#ifdef __OCPN__ANDROID__    
    void OnEvtPanGesture( wxQT_PanGestureEvent &event);
    void OnEvtPinchGesture( wxQT_PinchGestureEvent &event);
    void onGestureTimerEvent(wxTimerEvent &event);
    void onGestureFinishTimerEvent(wxTimerEvent &event);
#endif
    
    void onZoomTimerEvent(wxTimerEvent &event);
    
    wxString GetRendererString(){ return m_renderer; }
    wxString GetVersionString(){ return m_version; }
    void EnablePaint(bool b_enable){ m_b_paint_enable = b_enable; }

    void Invalidate();
    void RenderRasterChartRegionGL(ChartBase *chart, ViewPort &vp, LLRegion &region);
    
    void DrawGLOverLayObjects(void);
    void GridDraw( );
    void FlushFBO( void );
    
    void DrawDynamicRoutesTracksAndWaypoints( ViewPort &vp );
    void DrawStaticRoutesTracksAndWaypoints( ViewPort &vp );
    
    void RenderAllChartOutlines( ocpnDC &dc, ViewPort &VP );
    void RenderChartOutline( ocpnDC &dc, int dbIndex, ViewPort &VP );

    void DrawEmboss( emboss_data *emboss );
    void ShipDraw(ocpnDC& dc);

    void SetupCompression();
    bool CanAcceleratePanning() { return m_b_BuiltFBO; }
    bool UsingFBO() { return m_b_BuiltFBO; }

    bool isInGesture(){ return m_binGesture; }
    
    time_t m_last_render_time;

    int viewport[4];
    double mvmatrix[16], projmatrix[16];

    void SetupOpenGL();
    ChartCanvas *m_pParentCanvas;

protected:
    void RenderS57TextOverlay( ViewPort &VPoint);
    void RenderMBTilesOverlay( ViewPort &VPoint);
    void RenderScene( bool bRenderCharts = true, bool bRenderOverlays = true);
    

    void RenderGLAlertMessage();

    void RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &rect_region );
    void RenderQuiltViewGLText( ViewPort &vp, const OCPNRegion &rect_region );
    
    void BuildFBO();
    bool buildFBOSize(int fboSize);
    
    void configureShaders( ViewPort &vp);
    
//    void ComputeRenderQuiltViewGLRegion( ViewPort &vp, OCPNRegion &Region );
    void RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region);
    void RenderNoDTA(ViewPort &vp, const LLRegion &region, int transparency = 255);
    void RenderNoDTA(ViewPort &vp, ChartBase *chart);
    void RenderWorldChart(ocpnDC &dc, ViewPort &vp, wxRect &rect, bool &world_view);

    void DrawFloatingOverlayObjects( ocpnDC &dc );
    void DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp);

    void DrawChartBar( ocpnDC &dc );
    void DrawQuiting();
    void DrawCloseMessage(wxString msg);

    void DrawGLTidesInBBox(ocpnDC& dc, LLBBox& BBox);
    void DrawGLCurrentsInBBox(ocpnDC& dc, LLBBox& BBox);
    
    void ZoomProject(float offset_x, float offset_y, float swidth, float sheight);
    
    void RendertoTexture(GLint tex);
    
    void fboFade(GLint tex0, GLint tex1);
    void onFadeTimerEvent(wxTimerEvent &event);
    bool m_inFade;
    
    wxGLContext       *m_pcontext;

    ocpnDC            m_gldc;
    
    int max_texture_dimension;

    bool m_bsetup;

    wxString m_renderer;
    wxString m_version;
    wxString m_extensions;    
    
    ViewPort    m_cache_vp;
    ChartBase   *m_cache_current_ch;
    
    bool        m_b_paint_enable;
    int         m_in_glpaint;

    //    For FBO(s)
    bool         m_b_DisableFBO;
    bool         m_b_BuiltFBO;
    bool         m_b_useFBOStencil;
    GLuint       m_fb0;
    GLuint       m_renderbuffer;

    GLuint       m_cache_tex[2];
    GLuint       m_cache_page;
    int          m_cache_tex_x;
    int          m_cache_tex_y;

    GLuint      ownship_tex;
    int         ownship_color;
    wxSize      ownship_size, ownship_tex_size;

    GLuint      m_piano_tex;
    
    float       m_fbo_offsetx;
    float       m_fbo_offsety;
    float       m_fbo_swidth;
    float       m_fbo_sheight;

    float       m_lastfbo_offsetx;
    float       m_lastfbo_offsety;
    float       m_lastfbo_swidth;
    float       m_lastfbo_sheight;
    
    float       m_offsetxStep, m_offsetyStep, m_swidthStep, m_sheightStep;
    float       m_runoffsetx, m_runoffsety, m_runswidth, m_runsheight;
    float       m_nStep, m_nTotal, m_nRun;
    bool        m_zoomFinal;
    double      m_zoomFinalZoom;
    int         m_zoomFinaldx, m_zoomFinaldy;
    bool        m_bforcefull;
    
    
    wxTimer     zoomTimer;
    
    double      m_fbo_lat, m_fbo_lon;
    int         m_cc_x,m_cc_y;
    wxPoint     m_lpinchPoint;
    
    bool        m_binPinch;
    bool        m_binPan;
    bool        m_binGesture;
    bool        m_bfogit;
    bool        m_benableFog;
    bool        m_benableVScale;
    
    wxTimer     m_gestureEeventTimer;
    wxTimer     m_gestureFinishTimer;
    bool        m_bgestureGuard;
    bool        m_bpinchGuard;
    wxPoint     m_pinchStart;
    double      m_pinchlat, m_pinchlon;
    
    wxTimer     m_fadeTimer;
    
    OCPNRegion  m_canvasregion;
    TexFont     m_gridfont;

    int		m_LRUtime;

    GLuint       m_tideTex;
    GLuint       m_currentTex;
    int          m_tideTexWidth;
    int          m_tideTexHeight;
    int          m_currentTexWidth;
    int          m_currentTexHeight;
    
    DECLARE_EVENT_TABLE()
};

extern void BuildCompressedCache();

#include "glTextureManager.h"
extern glTextureManager   *g_glTextureManager;

#endif
