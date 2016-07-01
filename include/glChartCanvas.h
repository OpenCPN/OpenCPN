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
#include "ocpn_types.h"
#include "OCPNRegion.h"
#include "LLRegion.h"
#include "viewport.h"
#include "TexFont.h"

 #define FORMAT_BITS           GL_RGB

#ifdef __OCPN__ANDROID__
#include "wx/qt/private/wxQtGesture.h"
#endif

class glTexFactory;

#define GESTURE_EVENT_TIMER 78334

//      This is a hashmap with Chart full path as key, and glTexFactory as value
WX_DECLARE_STRING_HASH_MAP( glTexFactory*, ChartPathHashTexfactType );

class ocpnGLOptions
{
public:
    bool m_bUseAcceleratedPanning;
    bool m_bUseCanvasPanning;
    
    bool m_bTextureCompression;
    bool m_bTextureCompressionCaching;

    int m_iTextureDimension;
    int m_iTextureMemorySize;
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
    
    static bool         s_b_useScissorTest;
    static bool         s_b_useStencil;
    static bool         s_b_useStencilAP;
    static bool         s_b_UploadFullMipmaps;
    
    glChartCanvas(wxWindow *parent);
    ~glChartCanvas();

    void SetContext(wxGLContext *pcontext) { m_pcontext = pcontext; }

    void OnPaint(wxPaintEvent& event);
    void OnEraseBG(wxEraseEvent& evt);
    void Render();
    void OnActivate ( wxActivateEvent& event );
    void OnSize ( wxSizeEvent& event );
    void MouseEvent(wxMouseEvent& event);
    void FastPan(int dx, int dy);
    void FastZoom(float factor);
    void RenderCanvasBackingChart( ocpnDC dc, OCPNRegion chart_get_region);
    
#ifdef __OCPN__ANDROID__    
    void OnEvtPanGesture( wxQT_PanGestureEvent &event);
    void OnEvtPinchGesture( wxQT_PinchGestureEvent &event);
    void onGestureTimerEvent(wxTimerEvent &event);
#endif
    
    wxString GetRendererString(){ return m_renderer; }
    wxString GetVersionString(){ return m_version; }
    void EnablePaint(bool b_enable){ m_b_paint_enable = b_enable; }

    static void Invalidate();
    void RenderRasterChartRegionGL(ChartBase *chart, ViewPort &vp, LLRegion &region);
    bool PurgeChartTextures(ChartBase *pc, bool b_purge_factory = false);
    void ClearAllRasterTextures(void);
    void DrawGLOverLayObjects(void);
    void GridDraw( );
    void FlushFBO( void );
    
    void DrawDynamicRoutesTracksAndWaypoints( ViewPort &vp );
    void DrawStaticRoutesTracksAndWaypoints( ViewPort &vp );
    
    void RenderAllChartOutlines( ocpnDC &dc, ViewPort &VP );
    void RenderChartOutline( int dbIndex, ViewPort &VP );

    void DrawEmboss( emboss_data *emboss );
    void ShipDraw(ocpnDC& dc);

    void SetupCompression();
    bool CanAcceleratePanning() { return m_b_BuiltFBO; }
    bool UsingFBO() { return m_b_BuiltFBO; }

    time_t m_last_render_time;

    int viewport[4];
    double mvmatrix[16], projmatrix[16];

protected:
    void RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &rect_region );
    void BuildFBO();
    void SetupOpenGL();
    bool TextureCrunch(double factor);
    bool FactoryCrunch(double factor);
    
//    void ComputeRenderQuiltViewGLRegion( ViewPort &vp, OCPNRegion &Region );
    void RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region);
    void RenderNoDTA(ViewPort &vp, const LLRegion &region);
    void RenderNoDTA(ViewPort &vp, ChartBase *chart);
    void RenderWorldChart(ocpnDC &dc, ViewPort &vp, wxRect &rect, bool &world_view);

    void DrawFloatingOverlayObjects( ocpnDC &dc );
    void DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp);

    void DrawChartBar( ocpnDC &dc );
    void DrawQuiting();
    void DrawCloseMessage(wxString msg);

    void DrawGLTidesInBBox(ocpnDC& dc, LLBBox& BBox);
    void DrawGLCurrentsInBBox(ocpnDC& dc, LLBBox& BBox);
    
    wxGLContext       *m_pcontext;

    int max_texture_dimension;

    bool m_bsetup;

    wxString m_renderer;
    wxString m_version;
    wxString m_extensions;

    //    This is a hash table
    //    key is Chart full path
    //    Value is glTexFactory*
    ChartPathHashTexfactType   m_chart_texfactory_hash;
    
    
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

    int		m_prevMemUsed;

    GLuint      ownship_tex;
    int         ownship_color;
    wxSize      ownship_size, ownship_tex_size;

    GLuint      m_piano_tex;
    
    float       m_fbo_offsetx;
    float       m_fbo_offsety;
    float       m_fbo_swidth;
    float       m_fbo_sheight;
    bool        m_binPinch;
    bool        m_binPan;
    bool        m_bfogit;
    bool        m_benableFog;
    bool        m_benableVScale;
    
    wxTimer     m_gestureEeventTimer;
    bool        m_bgestureGuard;
    bool        m_bpinchGuard;
    
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


#endif
