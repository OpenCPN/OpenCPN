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

class glTextureDescriptor;          // Defined/implemented in chcanv.cpp

WX_DECLARE_OBJARRAY(glTextureDescriptor, ArrayOfTexDescriptors);

WX_DECLARE_HASH_MAP( int, glTextureDescriptor*, wxIntegerHash, wxIntegerEqual, ChartTextureHashType );
WX_DECLARE_HASH_MAP( void*, ChartTextureHashType*, wxPointerHash, wxPointerEqual, ChartPointerHashType );


class ocpnGLOptions
{
public:
    bool m_bUseAcceleratedPanning;

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
    static void MultMatrixViewPort(const ViewPort &vp);
    static ViewPort NormalizedViewPort(const ViewPort &vp);

    static void RotateToViewPort(const ViewPort &vp);
    static void SetClipRegion(const ViewPort &vp, const OCPNRegion &region,
                              bool apply_rotation=true, bool b_clear=false);
    static void DisableClipRegion();

    static bool         s_b_useScissorTest;
    static bool         s_b_useStencil;
    static bool         s_b_UploadFullCompressedMipmaps;
    
    glChartCanvas(wxWindow *parent);
    ~glChartCanvas();

    void SetContext(wxGLContext *pcontext) { m_pcontext = pcontext; }

    void OnPaint(wxPaintEvent& event);
    void OnEraseBG(wxEraseEvent& evt);
    void Render();
    void OnActivate ( wxActivateEvent& event );
    void OnSize ( wxSizeEvent& event );
    void MouseEvent(wxMouseEvent& event);

    wxString GetRendererString(){ return m_renderer; }
    void EnablePaint(bool b_enable){ m_b_paint_enable = b_enable; }

    static void Invalidate();
    void RenderRasterChartRegionGL(ChartBase *chart, ViewPort &vp, OCPNRegion &region);
    bool PurgeChartTextures(ChartBase *pc);
    void ClearAllRasterTextures(void);
    void DrawGLOverLayObjects(void);

    void GridDraw( );

    static void FixRenderIDL(int dl);

    void DrawAllRoutesAndWaypoints( ViewPort &vp, OCPNRegion &region );
    void RenderAllChartOutlines( ocpnDC &dc, ViewPort &VP );
    void RenderChartOutline( int dbIndex, ViewPort &VP );

    void DrawEmboss( emboss_data *emboss );
    void ShipDraw(ocpnDC& dc);

    void SetupCompression();
    bool CanAcceleratePanning() { return m_b_BuiltFBO; }
    bool UsingFBO() { return m_b_BuiltFBO; }

    time_t m_last_render_time;

protected:
    void RenderQuiltViewGL(ViewPort &vp, const OCPNRegion &Region, bool b_clear = true);
    void BuildFBO();
    void SetupOpenGL();

    void ComputeRenderQuiltViewGLRegion( ViewPort &vp, OCPNRegion &Region );
    void RenderCharts(ocpnDC &dc, OCPNRegion &region);
    void RenderWorldChart(ocpnDC &dc, OCPNRegion &region);
    ViewPort BuildClippedVP(ViewPort &VP, wxRect &rect);
    void DeleteChartTextures(ChartBaseBSB *pc);

    void DrawFloatingOverlayObjects( ocpnDC &dc, OCPNRegion &region );
    void DrawGroundedOverlayObjectsRect(ocpnDC &dc, wxRect &rect);

    void DrawQuiting();

    wxGLContext       *m_pcontext;

    int max_texture_dimension;

    unsigned char *m_data;
    int m_datasize;

    bool m_bsetup;

    wxString m_renderer;

    void GrowData(int size);

    ArrayOfTexDescriptors         m_tex_array;

    //    This is a hash table
    //    key is ChartBaseBSB pointer
    //    Value is ChartTextureHashType*
    ChartPointerHashType          m_chart_hash;

    ViewPort    m_cache_vp;
    ChartBase   *m_cache_current_ch;

    bool m_b_paint_enable;
    int m_in_glpaint;

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
    OCPNRegion   m_gl_rendered_region;

    GLuint ownship_tex;
    wxSize ownship_size, ownship_tex_size;
    GLuint ownship_large_scale_display_lists[2];

    DECLARE_EVENT_TABLE()
};

extern void BuildCompressedCache();

#endif
