#if defined(__ANDROID__)
#include <qopengl.h>
#include <GL/gl_private.h>  // this is a cut-down version of gl.h
#include <GLES2/gl2.h>

#elif defined(ocpnUSE_GL)

#if defined(__MSVC__)
#include "glew.h"
#include <GL/glu.h>

#elif defined(__WXOSX__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
typedef void (*  _GLUfuncptr)();
#define GL_COMPRESSED_RGB_FXT1_3DFX       0x86B0

#elif defined(__WXQT__) || defined(__WXGTK__)
#include <GL/glew.h>
#include <GL/glu.h>
#endif  // ocpnUSE_GL
#endif


#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/pen.h>
#include <wx/brush.h>

#include "model/comm_n0183_output.h"
#include "model/georef.h"
#include "model/multiplexer.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/routeman.h"

#include "color_handler.h"
#include "FontMgr.h"
#include "glChartCanvas.h"
#include "n0183_ctx_factory.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "ocpn_plugin.h"
#include "route_point_gui.h"
#include "styles.h"
#include "svg_utils.h"
#include "viewport.h"
#include "waypointman_gui.h"


extern Multiplexer* g_pMUX;
extern ocpnGLOptions g_GLOptions;
extern float g_MarkScaleFactorExp;
extern MyFrame* gFrame;
extern OCPNPlatform* g_Platform;
extern ocpnStyle::StyleManager* g_StyleManager;

extern Routeman* g_pRouteMan;

void RoutePointGui::Draw(ocpnDC& dc, ChartCanvas* canvas, wxPoint* rpn,
                         bool boverride_viz)  {
  wxPoint r;
  wxRect hilitebox;

  canvas->GetCanvasPointPix(m_point.m_lat, m_point.m_lon, &r);

  //  return the home point in this dc to allow "connect the dots"
  if (NULL != rpn) *rpn = r;

  if (!RoutePointGui(m_point).IsVisibleSelectable(canvas, boverride_viz)) return;

  // If waypoint is well off screen, skip the drawing
  if( (abs(r.x) > canvas->GetCanvasWidth() * 4 ) ||
      (abs(r.y) > canvas->GetCanvasHeight() * 4))
    return;

  // If waypoint pixel location is invalid, skip the drawing
  if ((abs(r.x) == INVALID_COORD ) ||
      (abs(r.y) == INVALID_COORD) )
  return;



  //    Optimization, especially apparent on tracks in normal cases
  if (m_point.m_IconName == _T("empty") && !m_point.m_bShowName
       && !m_point.m_bPtIsSelected){
      return;
  }

  wxPen *pen;
  if (m_point.m_bBlink)
    pen = g_pRouteMan->GetActiveRoutePointPen();
  else
    pen = g_pRouteMan->GetRoutePointPen();

  //  Substitue icon?
  if (m_point.m_IconIsDirty) ReLoadIcon();
  wxBitmap *pbm;
  if ((m_point.m_bIsActive) && (m_point.m_IconName != _T("mob")))
    pbm = pWayPointMan->GetIconBitmap(_T ( "activepoint" ));
  else
    pbm = m_point.m_pbmIcon;

  wxBitmap *pbms = NULL;
  if ((g_MarkScaleFactorExp > 1.0) && !m_point.m_bPreScaled) {
    if (m_point.m_IconScaleFactor != g_MarkScaleFactorExp) {
      wxImage scaled_image = pbm->ConvertToImage();
      int new_width = pbm->GetWidth() * g_MarkScaleFactorExp;
      int new_height = pbm->GetHeight() * g_MarkScaleFactorExp;
      m_point.m_ScaledBMP = wxBitmap(
          scaled_image.Scale(new_width, new_height, wxIMAGE_QUALITY_HIGH));

      m_point.m_IconScaleFactor = g_MarkScaleFactorExp;
    }
    if (m_point.m_ScaledBMP.IsOk()) pbm = &m_point.m_ScaledBMP;
  }

  int sx2 = pbm->GetWidth() / 2;
  int sy2 = pbm->GetHeight() / 2;

  //    Calculate the mark drawing extents
  wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);  // the bitmap extents

  if (m_point.m_bShowName) {
    if (0 == m_point.m_pMarkFont) {
      wxFont *dFont = FontMgr::Get().GetFont(_("Marks"));
      int font_size = wxMax(8, dFont->GetPointSize());
      font_size /= OCPN_GetWinDIPScaleFactor();

      m_point.m_pMarkFont = FontMgr::Get().FindOrCreateFont(
        font_size, dFont->GetFamily(), dFont->GetStyle(), dFont->GetWeight(),
        false, dFont->GetFaceName());

      m_point.m_FontColor = FontMgr::Get().GetFontColor(_("Marks"));
      m_point.CalculateNameExtents();
    }

    if (m_point.m_pMarkFont) {
      wxRect r2(r.x + m_point.m_NameLocationOffsetX, r.y + m_point.m_NameLocationOffsetY,
                m_point.m_NameExtents.x, m_point.m_NameExtents.y);
      r1.Union(r2);
    }
  }

  hilitebox = r1;
  hilitebox.x -= r.x;
  hilitebox.y -= r.y;
  float radius;
  if (g_btouch) {
    hilitebox.Inflate(20);
    radius = 20.0f;
  } else {
    hilitebox.Inflate(4);
    radius = 4.0f;
  }

  wxColour hi_colour = pen->GetColour();
  unsigned char transparency = 100;
  if (m_point.m_bRPIsBeingEdited) {
    hi_colour = GetGlobalColor(_T ( "YELO1" ));
    transparency = 150;
  }

  //  Highlite any selected point
  if (m_point.m_bPtIsSelected || m_point.m_bRPIsBeingEdited) {
    AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width,
                  hilitebox.height, radius, hi_colour, transparency);
  }

  bool bDrawHL = false;

  if (m_point.m_bBlink && (gFrame->nBlinkerTick & 1)) bDrawHL = true;

  if ((!bDrawHL) && (NULL != m_point.m_pbmIcon)) {
    dc.DrawBitmap(*pbm, r.x - sx2, r.y - sy2, true);
    // on MSW, the dc Bounding box is not updated on DrawBitmap() method.
    // Do it explicitely here for all platforms.
    dc.CalcBoundingBox(r.x - sx2, r.y - sy2);
    dc.CalcBoundingBox(r.x + sx2, r.y + sy2);
  }

  if (m_point.m_bShowName && m_point.m_MarkName.Length()) {
    if (m_point.m_pMarkFont) {
      dc.SetFont(*m_point.m_pMarkFont);
      dc.SetTextForeground(m_point.m_FontColor);

      dc.DrawText(m_point.m_MarkName, r.x + m_point.m_NameLocationOffsetX,
                  r.y + m_point.m_NameLocationOffsetY);
    }
  }

  // Draw waypoint radar rings if activated
  if (m_point.m_iWaypointRangeRingsNumber && m_point.m_bShowWaypointRangeRings) {
    double factor = 1.00;
    if (m_point.m_iWaypointRangeRingsStepUnits == 1)  // nautical miles
      factor = 1 / 1.852;

    factor *= m_point.m_fWaypointRangeRingsStep;

    double tlat, tlon;
    wxPoint r1;
    ll_gc_ll(m_point.m_lat, m_point.m_lon, 0, factor, &tlat, &tlon);
    canvas->GetCanvasPointPix(tlat, tlon, &r1);

    double lpp =
        sqrt(pow((double)(r.x - r1.x), 2) + pow((double)(r.y - r1.y), 2));
    int pix_radius = (int)lpp;

    wxPen ppPen1(m_point.m_wxcWaypointRangeRingsColour, 2);
    wxBrush saveBrush = dc.GetBrush();
    wxPen savePen = dc.GetPen();
    dc.SetPen(ppPen1);
    dc.SetBrush(
        wxBrush(m_point.m_wxcWaypointRangeRingsColour, wxBRUSHSTYLE_TRANSPARENT));

    for (int i = 1; i <= m_point.m_iWaypointRangeRingsNumber; i++)
      dc.StrokeCircle(r.x, r.y, i * pix_radius);
    dc.SetPen(savePen);
    dc.SetBrush(saveBrush);
  }

  //  Save the current draw rectangle in the current DC
  //    This will be useful for fast icon redraws
  m_point.CurrentRect_in_DC.x = r.x + hilitebox.x;
  m_point.CurrentRect_in_DC.y = r.y + hilitebox.y;
  m_point.CurrentRect_in_DC.width = hilitebox.width;
  m_point.CurrentRect_in_DC.height = hilitebox.height;

  if (m_point.m_bBlink)
    g_blink_rect = m_point.CurrentRect_in_DC;  // also save for global blinker

  delete pbms;  // the potentially scaled bitmap
}

#ifdef ocpnUSE_GL
void RoutePointGui::DrawGL(ViewPort &vp, ChartCanvas *canvas, ocpnDC &dc,
                           bool use_cached_screen_coords, bool bVizOverride) {
  if (!RoutePointGui(m_point).IsVisibleSelectable(canvas, bVizOverride)) return;

  //    Optimization, especially apparent on tracks in normal cases
  if (m_point.m_IconName == _T("empty") && !m_point.m_bShowName && !m_point.m_bPtIsSelected) return;

  if (m_point.m_wpBBox.GetValid() && vp.view_scale_ppm == m_point.m_wpBBox_view_scale_ppm &&
      vp.rotation == m_point.m_wpBBox_rotation) {
    /* see if this waypoint can intersect with bounding box */
    LLBBox vpBBox = vp.GetBBox();
    if (vpBBox.IntersectOut(m_point.m_wpBBox)) {
      // Are Range Rings enabled?
      if (m_point.m_bShowWaypointRangeRings && (m_point.m_iWaypointRangeRingsNumber > 0)) {
        double factor = 1.00;
        if (m_point.m_iWaypointRangeRingsStepUnits == 1)  // convert kilometers to NMi
          factor = 1 / 1.852;

        double radius = factor * m_point.m_iWaypointRangeRingsNumber *
                        m_point.m_fWaypointRangeRingsStep / 60.;

        LLBBox radar_box = m_point.m_wpBBox;
        radar_box.EnLarge(radius * 2);
        if (vpBBox.IntersectOut(radar_box)) {
          return;
        }
      } else
        return;
    }
  }

  wxPoint r;
  wxRect hilitebox;
  unsigned char transparency = 150;

  if (use_cached_screen_coords && m_point.m_pos_on_screen)
    r.x = m_point.m_screen_pos.m_x, r.y = m_point.m_screen_pos.m_y;
  else
    canvas->GetCanvasPointPix(m_point.m_lat, m_point.m_lon, &r);

  if (r.x == INVALID_COORD) return;

  //    Substitute icon?
  if (m_point.m_IconIsDirty) ReLoadIcon();
  wxBitmap *pbm;
  if ((m_point.m_bIsActive) && (m_point.m_IconName != _T("mob")))
    pbm = pWayPointMan->GetIconBitmap(_T ( "activepoint" ));
  else
    pbm = m_point.m_pbmIcon;

  //  If icon is corrupt, there is really nothing else to do...
  if (!pbm || !pbm->IsOk()) return;

  int sx2 = pbm->GetWidth() / 2;
  int sy2 = pbm->GetHeight() / 2;

  //    Calculate the mark drawing extents
  wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);  // the bitmap extents

  wxRect r3 = r1;
  if (m_point.m_bShowName) {
    if (!m_point.m_pMarkFont) {
      wxFont *dFont = FontMgr::Get().GetFont(_("Marks"));
      int font_size = wxMax(8, dFont->GetPointSize());
      font_size /= OCPN_GetWinDIPScaleFactor();

      m_point.m_pMarkFont = FontMgr::Get().FindOrCreateFont(
        font_size, dFont->GetFamily(), dFont->GetStyle(), dFont->GetWeight(),
        false, dFont->GetFaceName());

      m_point.m_FontColor = FontMgr::Get().GetFontColor(_("Marks"));
      if (m_point.m_iTextTexture) {
        glDeleteTextures(1, &m_point.m_iTextTexture);
        m_point.m_iTextTexture = 0;
      }

      m_point.CalculateNameExtents();
    }

    if (m_point.m_pMarkFont) {
      wxRect r2(r.x + m_point.m_NameLocationOffsetX, r.y + m_point.m_NameLocationOffsetY,
                m_point.m_NameExtents.x, m_point.m_NameExtents.y);
      r3.Union(r2);
    }
  }

  hilitebox = r3;
  hilitebox.x -= r.x;
  hilitebox.y -= r.y;

  if (!m_point.m_bPreScaled) {
    hilitebox.x *= g_MarkScaleFactorExp;
    hilitebox.y *= g_MarkScaleFactorExp;
    hilitebox.width *= g_MarkScaleFactorExp;
    hilitebox.height *= g_MarkScaleFactorExp;
  }

  float radius;
  if (g_btouch) {
    hilitebox.Inflate(20);
    radius = 20.0f;
  } else {
    hilitebox.Inflate(4);
    radius = 4.0f;
  }

  /* update bounding box */
  if (!m_point.m_wpBBox.GetValid() || vp.view_scale_ppm != m_point.m_wpBBox_view_scale_ppm ||
      vp.rotation != m_point.m_wpBBox_rotation) {
    double lat1, lon1, lat2, lon2;
    canvas->GetCanvasPixPoint(r.x + hilitebox.x,
                              r.y + hilitebox.y + hilitebox.height, lat1, lon1);
    canvas->GetCanvasPixPoint(r.x + hilitebox.x + hilitebox.width,
                              r.y + hilitebox.y, lat2, lon2);

    if (lon1 > lon2)
      m_point.m_wpBBox.Set(lat1, lon1, lat2, lon2 + 360);
    else
      m_point.m_wpBBox.Set(lat1, lon1, lat2, lon2);

    m_point.m_wpBBox_view_scale_ppm = vp.view_scale_ppm;
    m_point.m_wpBBox_rotation = vp.rotation;
  }

  //    if(region.Contains(r3) == wxOutRegion)
  //        return;

  //ocpnDC dc;

  //  Highlite any selected point
  if (m_point.m_bPtIsSelected) {
    wxColour hi_colour;
    if (m_point.m_bBlink) {
      wxPen *pen = g_pRouteMan->GetActiveRoutePointPen();
      hi_colour = pen->GetColour();
    } else {
      hi_colour = GetGlobalColor(_T ( "YELO1" ));
    }

    AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width,
                  hilitebox.height, radius, hi_colour, transparency);
  }

  bool bDrawHL = false;

  if (m_point.m_bBlink && (gFrame->nBlinkerTick & 1)) bDrawHL = true;

  if ((!bDrawHL) && (NULL != m_point.m_pbmIcon)) {
    int glw, glh;
    unsigned int IconTexture =
       WayPointmanGui(*pWayPointMan).GetIconTexture(pbm, glw, glh);

    glBindTexture(GL_TEXTURE_2D, IconTexture);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int w = r1.width, h = r1.height;

    float scale = 1.0;
    if (!m_point.m_bPreScaled) {
      scale = g_MarkScaleFactorExp;
    }

    // Scale for MacOS Retina and GTK screen scaling
    scale *= GetOCPNCanvasWindow()->GetContentScaleFactor();

    float ws = r1.width * scale;
    float hs = r1.height * scale;
    float xs = r.x - ws / 2.;
    float ys = r.y - hs / 2.;
    float u = (float)w / glw, v = (float)h / glh;

    float coords[8];
    float uv[8];
    // normal uv
    uv[0] = 0;
    uv[1] = 0;
    uv[2] = u;
    uv[3] = 0;
    uv[4] = u;
    uv[5] = v;
    uv[6] = 0;
    uv[7] = v;

    // pixels
    coords[0] = xs;
    coords[1] = ys;
    coords[2] = xs + ws;
    coords[3] = ys;
    coords[4] = xs + ws;
    coords[5] = ys + hs;
    coords[6] = xs, coords[7] = ys + hs;

    glChartCanvas::RenderSingleTexture(dc, coords, uv, &vp, 0, 0, 0);

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
  }

  if (m_point.m_bShowName && m_point.m_pMarkFont) {
    int w = m_point.m_NameExtents.x, h = m_point.m_NameExtents.y;
    if (!m_point.m_iTextTexture && w && h) {
#if 0
      wxBitmap tbm(w, h); /* render text on dc */
      wxMemoryDC dc;
      dc.SelectObject(tbm);
      dc.SetBackground(wxBrush(*wxBLACK));
      dc.Clear();
      dc.SetFont(*m_pMarkFont);
      dc.SetTextForeground(*wxWHITE);
      dc.DrawText(m_MarkName, 0, 0);
      dc.SelectObject(wxNullBitmap);

      /* make alpha texture for text */
      wxImage image = tbm.ConvertToImage();
      unsigned char *d = image.GetData();
      unsigned char *e = new unsigned char[w * h];
      if (d && e) {
        for (int p = 0; p < w * h; p++) e[p] = d[3 * p + 0];
      }

      /* create texture for rendered text */
      glGenTextures(1, &m_iTextTexture);
      glBindTexture(GL_TEXTURE_2D, m_iTextTexture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      m_iTextTextureWidth = NextPow2(w);
      m_iTextTextureHeight = NextPow2(h);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, m_iTextTextureWidth,
                   m_iTextTextureHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_ALPHA, GL_UNSIGNED_BYTE,
                      e);
      delete[] e;
#else
      wxScreenDC sdc;
      sdc.SetFont(*m_point.m_pMarkFont);

      /* create bitmap of appropriate size and select it */
      wxBitmap bmp(w, h);
      wxMemoryDC temp_dc;
      temp_dc.SelectObject(bmp);

      /* fill bitmap with black */
      temp_dc.SetBackground(wxBrush(wxColour(0, 0, 0)));
      temp_dc.Clear();

      /* draw the text white */
      temp_dc.SetFont(*m_point.m_pMarkFont);
      temp_dc.SetTextForeground(wxColour(255, 255, 255));
      temp_dc.DrawText(m_point.m_MarkName, 0, 0);
      temp_dc.SelectObject(wxNullBitmap);

      /* use the data in the bitmap for alpha channel,
       and set the color to text foreground */
      wxImage image = bmp.ConvertToImage();

      unsigned char *data = new unsigned char[w * h * 4];
      unsigned char *im = image.GetData();

      if (im) {
        unsigned int r = m_point.m_FontColor.Red();
        unsigned int g = m_point.m_FontColor.Green();
        unsigned int b = m_point.m_FontColor.Blue();
        for (int i = 0; i < h; i++) {
          for (int j = 0; j < w; j++) {
            unsigned int index = ((i * w) + j) * 4;
            data[index] = r;
            data[index + 1] = g;
            data[index + 2] = b;
            data[index + 3] = im[((i * w) + j) * 3];
          }
        }
      }

      glGenTextures(1, &m_point.m_iTextTexture);

      glBindTexture(GL_TEXTURE_2D, m_point.m_iTextTexture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      m_point.m_iTextTextureWidth = NextPow2(w);
      m_point.m_iTextTextureHeight = NextPow2(h);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_point.m_iTextTextureWidth,
                   m_point.m_iTextTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                      data);

      delete[] data;

      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#endif
    }

    if (m_point.m_iTextTexture) {
      /* draw texture with text */
      glBindTexture(GL_TEXTURE_2D, m_point.m_iTextTexture);

      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);

      int x = r.x + m_point.m_NameLocationOffsetX, y = r.y + m_point.m_NameLocationOffsetY;
      float u = (float)w / m_point.m_iTextTextureWidth,
            v = (float)h / m_point.m_iTextTextureHeight;
      float coords[8];
      float uv[8];
      // normal uv
      uv[0] = 0;
      uv[1] = 0;
      uv[2] = u;
      uv[3] = 0;
      uv[4] = u;
      uv[5] = v;
      uv[6] = 0;
      uv[7] = v;

      // pixels
      coords[0] = x;
      coords[1] = y;
      coords[2] = x + w;
      coords[3] = y;
      coords[4] = x + w;
      coords[5] = y + h;
      coords[6] = x, coords[7] = y + h;

      glChartCanvas::RenderSingleTexture(dc, coords, uv, &vp, 0, 0, 0);

      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
    }
  }

  // Draw waypoint radar rings if activated
  if (m_point.m_iWaypointRangeRingsNumber && m_point.m_bShowWaypointRangeRings) {
    double factor = 1.00;
    if (m_point.m_iWaypointRangeRingsStepUnits == 1)  // nautical miles
      factor = 1 / 1.852;

    factor *= m_point.m_fWaypointRangeRingsStep;

    double tlat, tlon;
    wxPoint r1;
    ll_gc_ll(m_point.m_lat, m_point.m_lon, 0, factor, &tlat, &tlon);
    canvas->GetCanvasPointPix(tlat, tlon, &r1);

    double lpp =
        sqrt(pow((double)(r.x - r1.x), 2) + pow((double)(r.y - r1.y), 2));
    int pix_radius = (int)lpp;

    extern wxColor GetDimColor(wxColor c);
    wxColor ring_dim_color = GetDimColor(m_point.m_wxcWaypointRangeRingsColour);

    // 0.5 mm nominal, but not less than 1 pixel
    double platform_pen_width = wxRound(
        wxMax(1.0, g_Platform->GetDisplayDPmm() / 2));
    wxPen ppPen1(ring_dim_color, platform_pen_width);
    wxBrush saveBrush = dc.GetBrush();
    wxPen savePen = dc.GetPen();
    dc.SetPen(ppPen1);
    dc.SetBrush(wxBrush(ring_dim_color, wxBRUSHSTYLE_TRANSPARENT));

    for (int i = 1; i <= m_point.m_iWaypointRangeRingsNumber; i++)
      dc.StrokeCircle(r.x, r.y, i * pix_radius);
    dc.SetPen(savePen);
    dc.SetBrush(saveBrush);
  }

  // Render Drag handle if enabled
  if (m_point.m_bDrawDragHandle) {
    //  A line, southeast, scaled to the size of the icon
    double platform_pen_width = wxRound(
        wxMax(1.0, g_Platform->GetDisplayDPmm() /
                       2));  // 0.5 mm nominal, but not less than 1 pixel

    wxColor dh_color = GetGlobalColor(_T ( "YELO1" ));
    wxPen ppPen1(dh_color, 3 * platform_pen_width);
    dc.SetPen(ppPen1);
    dc.DrawLine(r.x + hilitebox.width / 4, r.y + hilitebox.height / 4,
                r.x + m_point.m_drag_line_length_man, r.y + m_point.m_drag_line_length_man);

    dh_color = wxColor(0, 0, 0);
    wxPen ppPen2(dh_color, platform_pen_width);
    dc.SetPen(ppPen2);
    dc.DrawLine(r.x + hilitebox.width / 4, r.y + hilitebox.height / 4,
                r.x + m_point.m_drag_line_length_man, r.y + m_point.m_drag_line_length_man);

    // The drag handle
    glBindTexture(GL_TEXTURE_2D, m_point.m_dragIconTexture);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    int x = r.x + m_point.m_drag_icon_offset, y = r.y + m_point.m_drag_icon_offset,
        w = m_point.m_dragIcon.GetWidth(), h = m_point.m_dragIcon.GetHeight();

    float scale = 1.0;

    float ws = w * scale;
    float hs = h * scale;
    float xs = x - ws / 2.;
    float ys = y - hs / 2.;
    float u = (float)w / m_point.m_dragIconTextureWidth,
          v = (float)h / m_point.m_dragIconTextureWidth;

    float coords[8];
    float uv[8];
    // normal uv
    uv[0] = 0;
    uv[1] = 0;
    uv[2] = u;
    uv[3] = 0;
    uv[4] = u;
    uv[5] = v;
    uv[6] = 0;
    uv[7] = v;

    // pixels
    coords[0] = xs;
    coords[1] = ys;
    coords[2] = xs + ws;
    coords[3] = ys;
    coords[4] = xs + ws;
    coords[5] = ys + hs;
    coords[6] = xs, coords[7] = ys + hs;

    glChartCanvas::RenderSingleTexture(dc, coords, uv, &vp, 0, 0, 0);

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
  }

  if (m_point.m_bBlink)
    g_blink_rect =m_point. CurrentRect_in_DC;  // also save for global blinker

  //    This will be useful for fast icon redraws
  m_point.CurrentRect_in_DC.x = r.x + hilitebox.x;
  m_point.CurrentRect_in_DC.y = r.y + hilitebox.y;
  m_point.CurrentRect_in_DC.width = hilitebox.width;
  m_point.CurrentRect_in_DC.height = hilitebox.height;

  if (m_point.m_bBlink)
    g_blink_rect = m_point.CurrentRect_in_DC;  // also save for global blinker
}
#endif

void RoutePointGui::CalculateDCRect(wxDC &dc, ChartCanvas *canvas, wxRect *prect) {
  if (canvas) {
    dc.ResetBoundingBox();
    dc.DestroyClippingRegion();

    // Draw the mark on the dc
    ocpnDC odc(dc);
    odc.SetVP(canvas->GetVP());

    Draw(odc, canvas, NULL);

    //  Retrieve the drawing extents
    prect->x = dc.MinX() - 1;
    prect->y = dc.MinY() - 1;
    prect->width = dc.MaxX() - dc.MinX() + 2;  // Mouse Poop?
    prect->height = dc.MaxY() - dc.MinY() + 2;
  }
}

bool RoutePointGui::IsVisibleSelectable(ChartCanvas* cc, bool boverrideViz) {
    return m_point.IsVisibleSelectable(cc->GetScaleValue(), boverrideViz);
}

wxPoint2DDouble RoutePointGui::GetDragHandlePoint(ChartCanvas *canvas) {
  if (!m_point.m_bDrawDragHandle)
    return wxPoint2DDouble(m_point.m_lon, m_point.m_lat);
  else {
    return computeDragHandlePoint(canvas);
  }
}
void RoutePointGui::SetPointFromDraghandlePoint(ChartCanvas *canvas, double lat,
                                                double lon) {
  wxPoint r;
  canvas->GetCanvasPointPix(lat, lon, &r);
  double tlat, tlon;
  canvas->GetCanvasPixPoint(r.x - m_point.m_drag_icon_offset, r.y - m_point.m_drag_icon_offset,
                            tlat, tlon);
  m_point.m_lat = tlat;
  m_point.m_lon = tlon;
}

void RoutePointGui::SetPointFromDraghandlePoint(ChartCanvas *canvas, int x,
                                                int y) {
  double tlat, tlon;
  canvas->GetCanvasPixPoint(x - m_point.m_drag_icon_offset - m_point.m_draggingOffsetx,
                            y - m_point.m_drag_icon_offset - m_point.m_draggingOffsety, tlat,
                            tlon);
  m_point.m_lat = tlat;
  m_point.m_lon = tlon;
}

void RoutePointGui::PresetDragOffset(ChartCanvas *canvas, int x, int y) {
  wxPoint r;
  canvas->GetCanvasPointPix(m_point.m_lat, m_point.m_lon, &r);

  m_point.m_draggingOffsetx = x - (r.x + m_point.m_drag_icon_offset);
  m_point.m_draggingOffsety = y - (r.y + m_point.m_drag_icon_offset);
}

wxPoint2DDouble RoutePointGui::computeDragHandlePoint(ChartCanvas *canvas) {
  wxPoint r;
  canvas->GetCanvasPointPix(m_point.m_lat, m_point.m_lon, &r);
  double lat, lon;
  canvas->GetCanvasPixPoint(r.x + m_point.m_drag_icon_offset, r.y + m_point.m_drag_icon_offset,
                            lat, lon);

  // Keep the members updated
  m_point.m_dragHandleLat = lat;
  m_point.m_dragHandleLon = lon;

  return wxPoint2DDouble(lon, lat);
}

void RoutePointGui::ShowScaleWarningMessage(ChartCanvas *canvas) {
  wxString strA = _("The ScaMin value for new waypoints is set to");
  wxString strB = _("but current chartscale is");
  wxString strC =
      _("Therefore the new waypoint will not be visible at this zoom level.");
  wxString MessStr =
      wxString::Format(_T("%s %li,\n %s %.0f.\n%s"), strA, m_point.GetScaMin(), strB,
                       canvas->GetScaleValue(), strC);
  OCPNMessageBox(canvas, MessStr);
}

void RoutePointGui::EnableDragHandle(bool bEnable) {
  m_point.m_bDrawDragHandle = bEnable;
  if (bEnable) {
    if (!m_point.m_dragIcon.IsOk()) {
      // Get the icon
      // What size?
      int bm_size = g_Platform->GetDisplayDPmm() * 9;  // 9 mm nominal

      // What icon?
      wxString UserIconPath = g_Platform->GetSharedDataDir()
                              + _T("uidata") + wxFileName::GetPathSeparator();

      m_point.m_dragIcon = LoadSVG(UserIconPath + _T("DragHandle.svg"), bm_size,
                           bm_size, m_point.m_pbmIcon);

      // build a texture
#ifdef ocpnUSE_GL
      /* make rgba texture */
      if (m_point.m_dragIconTexture == 0) {
        glGenTextures(1, &m_point.m_dragIconTexture);
        glBindTexture(GL_TEXTURE_2D, m_point.m_dragIconTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        wxImage image = m_point.m_dragIcon.ConvertToImage();
        int w = image.GetWidth(), h = image.GetHeight();

        m_point.m_dragIconTextureWidth = NextPow2(w);
        m_point.m_dragIconTextureHeight = NextPow2(h);

        unsigned char *d = image.GetData();
        unsigned char *a = image.GetAlpha();

        unsigned char mr, mg, mb;
        image.GetOrFindMaskColour(&mr, &mg, &mb);

        unsigned char *e = new unsigned char[4 * w * h];
        if (d && e) {
          for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
              unsigned char r, g, b;
              int off = (y * image.GetWidth() + x);
              r = d[off * 3 + 0];
              g = d[off * 3 + 1];
              b = d[off * 3 + 2];
              e[off * 4 + 0] = r;
              e[off * 4 + 1] = g;
              e[off * 4 + 2] = b;

              e[off * 4 + 3] =
                  a ? a[off] : ((r == mr) && (g == mg) && (b == mb) ? 0 : 255);
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_point.m_dragIconTextureWidth,
                     m_point.m_dragIconTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     NULL);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                        e);

        delete[] e;
      }
#endif

      // set the drawing metrics
      if (m_point.m_dragIcon.IsOk()) {
        m_point.m_drag_line_length_man = bm_size;
        m_point.m_drag_icon_offset = bm_size;
      } else {
        m_point.m_drag_line_length_man = 64;
        m_point.m_drag_icon_offset = 64;
      }
    }
  }
}

void RoutePointGui::ReLoadIcon(void) {
  if (!pWayPointMan) return;
  bool icon_exists = pWayPointMan->DoesIconExist(m_point.m_IconName);

  wxString iconUse = m_point.m_IconName;
  if (!icon_exists) {
    //  Try all lower case as a favor in the case where imported waypoints use
    //  mixed case names
    wxString tentative_icon = m_point.m_IconName.Lower();
    if (pWayPointMan->DoesIconExist(tentative_icon)) {
      // if found, convert point's icon name permanently.
      m_point.m_IconName = tentative_icon;
      iconUse = m_point.m_IconName;
    }
    //      Icon name is not in the standard or user lists, so add to the list a
    //      generic placeholder
    else {
      if (!pWayPointMan->DoesIconExist(_T("tempsub"))) {
        ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
        if (style) {
          wxBitmap bmp = style->GetIcon(_T("circle"));
          if (bmp.IsOk())
            WayPointmanGui(*pWayPointMan).ProcessIcon(bmp, "tempsub",
                                                      "tempsub");
        }
      }
      iconUse = _T("tempsub");
    }
  }

  m_point.m_pbmIcon = pWayPointMan->GetIconBitmap(iconUse);
  m_point.m_bPreScaled = pWayPointMan->GetIconPrescaled(iconUse);

#ifdef ocpnUSE_GL
  m_point.m_wpBBox_view_scale_ppm = -1;

  m_point.m_iTextTexture = 0;
#endif

  m_point.m_IconScaleFactor = -1;  // Force scaled icon reload
  m_point.m_pMarkFont = 0;         // Force Font color reload
  m_point.m_IconIsDirty = false;
}

bool RoutePointGui::SendToGPS(const wxString &com_name, SendToGpsDlg *dialog) {

  N0183DlgCtx dlg_ctx = GetDialogCtx(dialog);
  int result = SendWaypointToGPS_N0183(&m_point, com_name, *g_pMUX, dlg_ctx);

  wxString msg;
  if (0 == result)
    msg = _("Waypoint(s) Transmitted.");
  else {
    if (result == ERR_GARMIN_INITIALIZE)
      msg = _("Error on Waypoint Upload.  Garmin GPS not connected");
    else
      msg = _("Error on Waypoint Upload.  Please check logfiles...");
  }

  OCPNMessageBox(NULL, msg, _("OpenCPN Info"), wxOK | wxICON_INFORMATION);

  return (result == 0);
}

int RoutePointGui::GetIconImageIndex() {
  if (m_point.IsShared()) {
    //    Get an array of all routes using this point
    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining(&m_point);

    // Use route array (if any) to determine actual visibility for this point
    bool brp_viz = false;
    if (proute_array) {
      for (unsigned int ir = 0; ir < proute_array->GetCount(); ir++) {
        Route *pr = (Route *)proute_array->Item(ir);
        if (pr->IsVisible()) {
          brp_viz = true;
          break;
        }
      }
    }

    if (brp_viz)
      return (pWayPointMan->GetFIconImageListIndex(GetIconBitmap()));
    else {
      if (m_point.IsVisible())
        return (pWayPointMan->GetIconImageListIndex(GetIconBitmap()));
      else
        return (pWayPointMan->GetXIconImageListIndex(GetIconBitmap()));
    }
  }

  else {  // point is not shared
    if (m_point.IsVisible())
      return (pWayPointMan->GetIconImageListIndex(GetIconBitmap()));
    else
      return (pWayPointMan->GetXIconImageListIndex(GetIconBitmap()));
  }
}
