/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 ************************************************************************* */

/**
 * \file
 *
 * OpenGL chart rendering canvas
 */

#ifndef __GLCHARTCANVAS_H__
#define __GLCHARTCANVAS_H__

#include <array>
#include <string>
#include <unordered_map>

// We need to set up our openGL environment before including
// glcanvas.h which includes GL/gl.h
#include "gl_headers.h"

#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/glcanvas.h>
#include <wx/pen.h>
#include <wx/string.h>
#include <wx/timer.h>

#include "model/ocpn_types.h"
#include "model/route.h"

#include "chartimg.h"
#include "chcanv.h"
#include "dychart.h"
#include "emboss_data.h"
#include "gl_tex_cache.h"
#include "gl_texture_mgr.h"
#include "LLRegion.h"
#include "ocpndc.h"
#include "ocpn_gl_options.h"
#include "ocpndc.h"
#include "ocpn_region.h"
#include "TexFont.h"
#include "viewport.h"

#ifdef __ANDROID__
#include <wx/qt/private/wxQtGesture.h>
#endif

#define FORMAT_BITS GL_RGB

#define GESTURE_EVENT_TIMER 78334
#define ZOOM_TIMER 78335
#define GESTURE_FINISH_TIMER 78336

extern GLuint g_raster_format;  ///< Global instance

extern void BuildCompressedCache();  // FIXME (leamas) find a home

extern glTextureManager *g_glTextureManager;  // FIXME (leamas) find a home

class OCPN_GLCaps {
public:
  std::string Renderer;
  std::string Version;
  std::string GLSL_Version;

  double dGLSL_Version;
  GLenum TextureRectangleFormat;

  bool bOldIntel;
  bool bCanDoVBO;
  bool bCanDoFBO;
  bool bCanDoGLSL;
};

GLboolean QueryExtension(const char *extName);

class glTestCanvas : public wxGLCanvas {
public:
  glTestCanvas(wxWindow *parent);
  ~glTestCanvas() {};
};

/**
 * OpenGL chart rendering canvas. Implements OpenGL-based rendering of charts
 * and overlays. Handles initialization of OpenGL context, rendering loop,
 * and interface with chart objects for drawing.
 */
class glChartCanvas : public wxGLCanvas {
public:
  static bool CanClipViewport(const ViewPort &vp);
  static ViewPort ClippedViewport(const ViewPort &vp, const LLRegion &region);

  static bool HasNormalizedViewPort(const ViewPort &vp);
  static void MultMatrixViewPort(ViewPort &vp, float lat = 0, float lon = 0);
  static ViewPort NormalizedViewPort(const ViewPort &vp, float lat = 0,
                                     float lon = 0);

  static void DrawRegion(ViewPort &vp, const LLRegion &region);
  static void SetClipRegion(ViewPort &vp, const LLRegion &region);
  static void SetClipRect(const ViewPort &vp, const wxRect &rect,
                          bool g_clear = false);
  static void DisableClipRegion();
  void SetColorScheme(ColorScheme cs);
  void RenderTextures(ocpnDC &dc, float *coords, float *uvCoords, int nVertex,
                      ViewPort *vp);
  static void RenderSingleTexture(ocpnDC &dc, float *coords, float *uvCoords,
                                  ViewPort *vp, float dx, float dy,
                                  float angle);
  void RenderColorRect(wxRect r, wxColor &color);

  static bool s_b_useScissorTest;
  static bool s_b_useStencil;
  static bool s_b_useStencilAP;
  static bool s_b_useFBO;
  static std::unordered_map<wxPenStyle, std::array<wxDash, 2>> dash_map;

  void SendJSONConfigMessage();

  glChartCanvas(wxWindow *parent, wxGLCanvas *share = NULL);

  ~glChartCanvas();

  void Init();
  void SetContext(wxGLContext *pcontext) { m_pcontext = pcontext; }
  int GetCanvasIndex() { return m_pParentCanvas->m_canvasIndex; }

  int GetGLCanvasWidth() { return m_glcanvas_width; }
  int GetGLCanvasHeight() { return m_glcanvas_height; }

  void OnPaint(wxPaintEvent &event);
  void OnEraseBG(wxEraseEvent &evt);
  void Render();
  void OnActivate(wxActivateEvent &event);
  void OnSize(wxSizeEvent &event);
  void MouseEvent(wxMouseEvent &event);
  void FastPan(int dx, int dy);
  void FastZoom(float factor, float cp_x, float cp_y, float post_x,
                float post_y);
  //    void RenderCanvasBackingChart( ocpnDC dc, OCPNRegion chart_get_region);
  //    void FastZoom(float factor);
  void RenderCanvasBackingChart(ocpnDC &dc, OCPNRegion chart_get_region);

#ifdef __OCPN__ANDROID__
  void OnEvtPanGesture(wxQT_PanGestureEvent &event);
  void OnEvtPinchGesture(wxQT_PinchGestureEvent &event);
  void onGestureTimerEvent(wxTimerEvent &event);
  void onGestureFinishTimerEvent(wxTimerEvent &event);
#else
#ifdef HAVE_WX_GESTURE_EVENTS
  void OnEvtPanGesture(wxPanGestureEvent &event);
  void OnEvtZoomGesture(wxZoomGestureEvent &event);
  void onGestureTimerEvent(wxTimerEvent &event);
  void onGestureFinishTimerEvent(wxTimerEvent &event);
#endif
#endif

  void onZoomTimerEvent(wxTimerEvent &event);

  wxString GetRendererString() { return m_renderer; }
  wxString GetVersionString() { return m_version; }
  wxString GetGLSLVersionString() { return m_GLSLversion; }
  void EnablePaint(bool b_enable) { m_b_paint_enable = b_enable; }

  void Invalidate();
  void RenderRasterChartRegionGL(ChartBase *chart, ViewPort &vp,
                                 LLRegion &region);

  void DrawGLOverLayObjects(void);
  void GridDraw();
  void FlushFBO(void);

  void DrawDynamicRoutesTracksAndWaypoints(ViewPort &vp);
  void DrawStaticRoutesTracksAndWaypoints(ViewPort &vp);

  void RenderAllChartOutlines(ocpnDC &dc, ViewPort &VP);
  void RenderChartOutline(ocpnDC &dc, int dbIndex, ViewPort &VP);

  void DrawEmboss(ocpnDC &dc, emboss_data *emboss);
  void ShipDraw(ocpnDC &dc);

  void SetupCompression();
  bool CanAcceleratePanning() { return m_b_BuiltFBO; }
  bool UsingFBO() { return m_b_BuiltFBO; }

  bool isInGesture() { return m_binGesture; }
  void ResetGridFont() { m_gridfont.Delete(); }
  time_t m_last_render_time;

  int viewport[4];
  double mvmatrix[16], projmatrix[16];

  void SetupOpenGL();
  ChartCanvas *m_pParentCanvas;
  ocpnDC m_gldc;

  wxStopWatch m_glstopwatch;

protected:
  void RenderS57TextOverlay(ViewPort &VPoint);
  void RenderMBTilesOverlay(ViewPort &VPoint);
  void RenderSingleMBTileOverlay(int dbIndex, bool bOVerlay, ViewPort &VPoint,
                                 OCPNRegion &screen_region,
                                 LLRegion &screenLLRegion);
  void RenderScene(bool bRenderCharts = true, bool bRenderOverlays = true);

  void RenderGLAlertMessage();

  void RenderQuiltViewGL(ViewPort &vp, const OCPNRegion &rect_region);
  void RenderQuiltViewGLText(ViewPort &vp, const OCPNRegion &rect_region);

  void BuildFBO();
  bool buildFBOSize(int fboSize);

  void configureShaders(ViewPort &vp);

  //    void ComputeRenderQuiltViewGLRegion( ViewPort &vp, OCPNRegion &Region );
  void RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region);
  void RenderNoDTA(ViewPort &vp, const LLRegion &region,
                   int transparency = 255);
  // void RenderNoDTA(ViewPort &vp, ChartBase *chart);
  void RenderWorldChart(ocpnDC &dc, ViewPort &vp, wxRect &rect,
                        bool &world_view);

  void DrawFloatingOverlayObjects(ocpnDC &dc);
  void DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp);

  void DrawChartBar(ocpnDC &dc);
  void DrawQuiting();
  void DrawCloseMessage(wxString msg);

  void DrawGLTidesInBBox(ocpnDC &dc, LLBBox &BBox);
  void DrawGLCurrentsInBBox(ocpnDC &dc, LLBBox &BBox);

  void ZoomProject(float offset_x, float offset_y, float swidth, float sheight);
  wxBitmap &GetTouchBackingBitmap(ViewPort &vp);
  void CreateBackingTexture();

  void RendertoTexture(GLint tex);

  wxGLContext *m_pcontext;

  int max_texture_dimension;

  bool m_bsetup;

  wxString m_renderer;
  wxString m_version, m_GLSLversion;
  wxString m_extensions;

  ViewPort m_cache_vp;
  ChartBase *m_cache_current_ch;

  bool m_b_paint_enable;
  int m_in_glpaint;

  //    For FBO(s)
  bool m_b_DisableFBO;
  bool m_b_BuiltFBO;
  bool m_b_useFBOStencil;
  GLuint m_fb0;
  GLuint m_renderbuffer;

  GLuint m_cache_tex[2];
  GLuint m_cache_page;
  int m_cache_tex_x;
  int m_cache_tex_y;

  GLuint ownship_tex;
  int ownship_color;
  wxSize ownship_size, ownship_tex_size;

  GLuint m_piano_tex;

  float m_fbo_offsetx;
  float m_fbo_offsety;
  float m_fbo_swidth;
  float m_fbo_sheight;

  float m_lastfbo_offsetx;
  float m_lastfbo_offsety;
  float m_lastfbo_swidth;
  float m_lastfbo_sheight;

  float m_offsetxStep, m_offsetyStep, m_swidthStep, m_sheightStep;
  float m_runoffsetx, m_runoffsety, m_runswidth, m_runsheight;
  float m_nStep, m_nTotal, m_nRun;
  bool m_zoomFinal;
  double m_zoomFinalZoom;
  int m_zoomFinaldx, m_zoomFinaldy;
  bool m_bforcefull;
  bool m_first_zout;

  wxTimer zoomTimer;

  double m_fbo_lat, m_fbo_lon;
  int m_cc_x, m_cc_y;
  wxPoint m_lpinchPoint;

  bool m_binPinch;
  bool m_binPan;
  bool m_binGesture;

  wxTimer m_gestureEeventTimer;
  wxTimer m_gestureFinishTimer;
  bool m_bgestureGuard;
  bool m_bpinchGuard;
  wxPoint m_pinchStart;
  double m_pinchlat, m_pinchlon;

  OCPNRegion m_canvasregion;
  TexFont m_gridfont;

  int m_LRUtime;

  GLuint m_tideTex;
  GLuint m_currentTex;
  int m_tideTexWidth;
  int m_tideTexHeight;
  int m_currentTexWidth;
  int m_currentTexHeight;
  int m_displayScale;

  int m_glcanvas_width;
  int m_glcanvas_height;

  float m_total_zoom_val;
  float m_final_zoom_val;
  float m_step_zoom_val;

  bool m_bUseGLSL;

  wxBitmap m_touch_backing_bitmap;
  unsigned int m_TouchBackingTexture;
  int m_tex_w, m_tex_h, m_image_width, m_image_height;
  ViewPort m_texVP;
  float m_zoom_inc;

  DECLARE_EVENT_TABLE()
};

#endif
