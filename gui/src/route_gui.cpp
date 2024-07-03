#include <string>

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/pen.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "color_handler.h"
#include "chartbase.h"
#include "model/comm_n0183_output.h"
#include "model/georef.h"
#include "gui_lib.h"
#include "model/multiplexer.h"
#include "n0183_ctx_factory.h"
#include "navutil.h"
#include "model/own_ship.h"
#include "model/routeman.h"
#include "route_gui.h"
#include "route_point_gui.h"
#include "glChartCanvas.h"
#include "line_clip.h"
#include "model/route.h"

extern Routeman* g_pRouteMan;
extern wxColour g_colourTrackLineColour;
extern Multiplexer *g_pMUX;

extern wxColor GetDimColor(wxColor c);
extern bool g_bHighliteTracks;

extern ocpnGLOptions g_GLOptions;

extern int s_arrow_icon[];

static void TestLongitude(double lon, double min, double max, bool &lonl,
                          bool &lonr) {
  double clon = (min + max) / 2;
  if (min - lon > 180) lon += 360;

  lonl = lonr = false;
  if (lon < min) {
    if (lon < clon - 180)
      lonr = true;
    else
      lonl = true;
  } else if (lon > max) {
    if (lon > clon + 180)
      lonl = true;
    else
      lonr = true;
  }
}

void RouteGui::Draw(ocpnDC &dc, ChartCanvas *canvas, const LLBBox &box) {
  if (m_route.pRoutePointList->empty()) return;

  ViewPort vp = canvas->GetVP();

  LLBBox test_box = m_route.GetBBox();
  if (box.IntersectOut(test_box))  // Route is wholly outside window
    return;

  int width = g_route_line_width;
  if (m_route.m_width != WIDTH_UNDEFINED) width = m_route.m_width;

  if (m_route.m_bVisible && m_route.m_bRtIsSelected) {
    wxPen spen = *g_pRouteMan->GetSelectedRoutePen();
    spen.SetWidth(width);
    dc.SetPen(spen);
    dc.SetBrush(*g_pRouteMan->GetSelectedRouteBrush());
  } else if (m_route.m_bVisible) {
    wxPenStyle style = wxPENSTYLE_SOLID;
    wxColour col;
    if (m_route.m_style != wxPENSTYLE_INVALID) style = m_route.m_style;
    if (m_route.m_Colour == wxEmptyString) {
      col = g_pRouteMan->GetRoutePen()->GetColour();
    } else {
      for (unsigned int i = 0; i < sizeof(::GpxxColorNames) / sizeof(wxString);
           i++) {
        if (m_route.m_Colour == ::GpxxColorNames[i]) {
          col = ::GpxxColors[i];
          break;
        }
      }
    }
    dc.SetPen(*wxThePenList->FindOrCreatePen(col, width, style));
    dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(col, wxBRUSHSTYLE_SOLID));
  }

  if (m_route.m_bVisible && m_route.m_bRtIsActive) {
    wxPen spen = *g_pRouteMan->GetActiveRoutePen();
    spen.SetWidth(width);
    dc.SetPen(spen);
    dc.SetBrush(*g_pRouteMan->GetActiveRouteBrush());
  }

  wxPoint rpt1, rpt2;
  if (m_route.m_bVisible) DrawPointWhich(dc, canvas, 1, &rpt1);

  wxRoutePointListNode *node = m_route.pRoutePointList->GetFirst();
  RoutePoint *prp1 = node->GetData();
  node = node->GetNext();

  if (m_route.m_bVisible || prp1->IsShared()) RoutePointGui(*prp1).Draw(dc, canvas, NULL); //prp1->Draw(dc, canvas, NULL);

  while (node) {
    RoutePoint *prp2 = node->GetData();

    bool draw_arrow = !(prp2->m_bIsActive && g_bAllowShipToActive);

    if (m_route.m_bVisible || prp2->IsShared()) RoutePointGui(*prp2).Draw(dc, canvas, &rpt2); //prp2->Draw(dc, canvas, &rpt2);

    if (m_route.m_bVisible) {
      //    Handle offscreen points
      bool b_2_on = vp.GetBBox().Contains(prp2->m_lat, prp2->m_lon);
      bool b_1_on = vp.GetBBox().Contains(prp1->m_lat, prp1->m_lon);

      // Simple case
      if (b_1_on && b_2_on)
        RenderSegment(dc, rpt1.x, rpt1.y, rpt2.x, rpt2.y, vp, draw_arrow,
                      m_route.m_hiliteWidth);  // with arrows

      //    In the cases where one point is on, and one off
      //    we must decide which way to go in longitude
      //     Arbitrarily, we will go the shortest way

      double pix_full_circle = WGS84_semimajor_axis_meters * mercator_k0 * 2 *
                               PI * vp.view_scale_ppm;
      double dp =
          pow((double)(rpt1.x - rpt2.x), 2) + pow((double)(rpt1.y - rpt2.y), 2);
      double dtest;
      int adder;
      if (b_1_on && !b_2_on) {
        if (rpt2.x < rpt1.x)
          adder = (int)pix_full_circle;
        else
          adder = -(int)pix_full_circle;

        dtest = pow((double)(rpt1.x - (rpt2.x + adder)), 2) +
                pow((double)(rpt1.y - rpt2.y), 2);

        if (dp < dtest) adder = 0;

        RenderSegment(dc, rpt1.x, rpt1.y, rpt2.x + adder, rpt2.y, vp,
                      draw_arrow, m_route.m_hiliteWidth);
      } else if (!b_1_on) {
        if (rpt1.x < rpt2.x)
          adder = (int)pix_full_circle;
        else
          adder = -(int)pix_full_circle;

        float rxd = rpt2.x - (rpt1.x + adder);
        float ryd = rpt1.y - rpt2.y;
        dtest = rxd * rxd + ryd * ryd;

        if (dp < dtest) adder = 0;

        RenderSegment(dc, rpt1.x + adder, rpt1.y, rpt2.x, rpt2.y, vp,
                      draw_arrow, m_route.m_hiliteWidth);
      }
    }

    rpt1 = rpt2;
    prp1 = prp2;

    node = node->GetNext();
  }
}

void RouteGui::RenderSegment(ocpnDC &dc, int xa, int ya, int xb, int yb,
                          ViewPort &vp, bool bdraw_arrow, int hilite_width) {
  //    Get the dc boundary
  int sx, sy;
  dc.GetSize(&sx, &sy);

  //    Try to exit early if the segment is nowhere near the screen
  wxRect r(0, 0, sx, sy);
  wxRect s(xa, ya, 1, 1);
  wxRect t(xb, yb, 1, 1);
  s.Union(t);
  if (!r.Intersects(s)) return;

  //    Clip the line segment to the dc boundary
  int x0 = xa;
  int y0 = ya;
  int x1 = xb;
  int y1 = yb;

  //    If hilite is desired, use a Native Graphics context to render alpha
  //    colours That is, if wxGraphicsContext is available.....

  if (hilite_width) {
    if (Visible ==
        cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, 0, sx, 0, sy)) {
      wxPen psave = dc.GetPen();

      wxColour y = GetGlobalColor(_T ( "YELO1" ));
      wxColour hilt(y.Red(), y.Green(), y.Blue(), 128);

      wxPen HiPen(hilt, hilite_width, wxPENSTYLE_SOLID);

      dc.SetPen(HiPen);
      dc.StrokeLine(x0, y0, x1, y1);

      dc.SetPen(psave);
      dc.StrokeLine(x0, y0, x1, y1);
    }
  } else {
    if (Visible ==
        cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, 0, sx, 0, sy))
      dc.StrokeLine(x0, y0, x1, y1);
  }

  if (bdraw_arrow) {
    //    Draw a direction arrow

    double theta = atan2((double)(yb - ya), (double)(xb - xa));
    theta -= PI / 2.;

    wxPoint icon[10];
    double icon_scale_factor = 100 * vp.view_scale_ppm;
    icon_scale_factor = fmin(icon_scale_factor, 1.5);  // Sets the max size
    icon_scale_factor = fmax(icon_scale_factor, .10);

    //    Get the absolute line length
    //    and constrain the arrow to be no more than xx% of the line length
    double nom_arrow_size = 20.;
    double max_arrow_to_leg = .20;
    double lpp = sqrt(pow((double)(xa - xb), 2) + pow((double)(ya - yb), 2));

    double icon_size = icon_scale_factor * nom_arrow_size;
    if (icon_size > (lpp * max_arrow_to_leg))
      icon_scale_factor = (lpp * max_arrow_to_leg) / nom_arrow_size;

    for (int i = 0; i < 7; i++) {
      int j = i * 2;
      double pxa = (double)(s_arrow_icon[j]);
      double pya = (double)(s_arrow_icon[j + 1]);

      pya *= icon_scale_factor;
      pxa *= icon_scale_factor;

      double px = (pxa * sin(theta)) + (pya * cos(theta));
      double py = (pya * sin(theta)) - (pxa * cos(theta));

      icon[i].x = (int)(px) + xb;
      icon[i].y = (int)(py) + yb;
    }
    wxPen savePen = dc.GetPen();
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.StrokePolygon(6, &icon[0], 0, 0);
    dc.SetPen(savePen);
  }
}

void RouteGui::RenderSegmentArrowsGL(ocpnDC &dc, int xa, int ya, int xb, int yb,
                                  ViewPort &vp) {
#ifdef ocpnUSE_GL
  //    Draw a direction arrow
  float icon_scale_factor = 100 * vp.view_scale_ppm;
  icon_scale_factor = fmin(icon_scale_factor, 1.5);  // Sets the max size
  icon_scale_factor = fmax(icon_scale_factor, .10);

  //    Get the absolute line length
  //    and constrain the arrow to be no more than xx% of the line length
  float nom_arrow_size = 20.;
  float max_arrow_to_leg = (float).20;
  float lpp = sqrtf(powf((float)(xa - xb), 2) + powf((float)(ya - yb), 2));

  float icon_size = icon_scale_factor * nom_arrow_size;
  if (icon_size > (lpp * max_arrow_to_leg))
    icon_scale_factor = (lpp * max_arrow_to_leg) / nom_arrow_size;

  float theta = atan2f((float)yb - ya, (float)xb - xa);
  theta -= (float)PI;

  // icon_scale_factor = 5;
  wxPoint pts[3];
  // 0
  pts[0].x = s_arrow_icon[0];
  pts[0].y = s_arrow_icon[1];
  pts[1].x = s_arrow_icon[2];
  pts[1].y = s_arrow_icon[3];
  pts[2].x = s_arrow_icon[6];
  pts[2].y = s_arrow_icon[7];

  dc.DrawPolygon(3, pts, xb, yb, icon_scale_factor, theta);

  // 1
  pts[0].x = s_arrow_icon[2];
  pts[0].y = s_arrow_icon[3];
  pts[1].x = s_arrow_icon[4];
  pts[1].y = s_arrow_icon[5];
  pts[2].x = s_arrow_icon[6];
  pts[2].y = s_arrow_icon[7];
  dc.DrawPolygon(3, pts, xb, yb, icon_scale_factor, theta);

  // 2
  pts[0].x = s_arrow_icon[0];
  pts[0].y = -s_arrow_icon[1];
  pts[1].x = s_arrow_icon[2];
  pts[1].y = -s_arrow_icon[3];
  pts[2].x = s_arrow_icon[6];
  pts[2].y = -s_arrow_icon[7];
  dc.DrawPolygon(3, pts, xb, yb, icon_scale_factor, theta);

  // 3
  pts[0].x = s_arrow_icon[2];
  pts[0].y = -s_arrow_icon[3];
  pts[1].x = s_arrow_icon[4];
  pts[1].y = -s_arrow_icon[5];
  pts[2].x = s_arrow_icon[6];
  pts[2].y = -s_arrow_icon[7];
  dc.DrawPolygon(3, pts, xb, yb, icon_scale_factor, theta);


#endif
}

void RouteGui::DrawPointWhich(ocpnDC &dc, ChartCanvas *canvas, int iPoint,
                           wxPoint *rpn) {
  if (iPoint <= m_route.GetnPoints())
    RoutePointGui(*m_route.GetPoint(iPoint)).Draw(dc, canvas, rpn);
}

void RouteGui::DrawSegment(ocpnDC &dc, ChartCanvas *canvas, wxPoint *rp1,
                        wxPoint *rp2, ViewPort &vp, bool bdraw_arrow) {
  if (m_route.m_bRtIsSelected)
    dc.SetPen(*g_pRouteMan->GetSelectedRoutePen());
  else if (m_route.m_bRtIsActive)
    dc.SetPen(*g_pRouteMan->GetActiveRoutePen());
  else
    dc.SetPen(*g_pRouteMan->GetRoutePen());

  RenderSegment(dc, rp1->x, rp1->y, rp2->x, rp2->y, vp, bdraw_arrow);
}

void RouteGui::DrawGL(ViewPort &vp, ChartCanvas *canvas, ocpnDC &dc) {
#ifdef ocpnUSE_GL
  if (m_route.pRoutePointList->empty()) return;

  if (!vp.GetBBox().IntersectOut(m_route.GetBBox()) && m_route.m_bVisible)
    DrawGLRouteLines(vp, canvas, dc);

  /*  Route points  */
  for (wxRoutePointListNode *node = m_route.pRoutePointList->GetFirst(); node;
       node = node->GetNext()) {
    RoutePoint *prp = node->GetData();
    // Inflate the bounding box a bit to ensure full drawing in accelerated pan
    // mode.
    // TODO this is a little extravagant, assumming a mark is always a large
    // fixed lat/lon extent.
    //  Maybe better to use the mark's drawn box, once it is known.
    if (vp.GetBBox().ContainsMarge(prp->m_lat, prp->m_lon, .5)) {
      if (m_route.m_bVisible || prp->IsShared()) RoutePointGui(*prp).DrawGL(vp, canvas, dc);
    }
  }
#endif
}

void RouteGui::DrawGLRouteLines(ViewPort &vp, ChartCanvas *canvas, ocpnDC &dc) {
#ifdef ocpnUSE_GL
  //  Hiliting first
  //  Being special case to draw something for a 1 point route....
  if (m_route.m_hiliteWidth) {
    wxColour y = GetGlobalColor(_T ( "YELO1" ));
    wxColour hilt(y.Red(), y.Green(), y.Blue(), 128);

    wxPen HiPen(hilt, m_route.m_hiliteWidth, wxPENSTYLE_SOLID);

    dc.SetPen(HiPen);

    DrawGLLines(vp, &dc, canvas);
  }

  /* determine color and width */
  wxColour col;

  int width = g_pRouteMan->GetRoutePen()->GetWidth();  // g_route_line_width;
  if (m_route.m_width != wxPENSTYLE_INVALID) width = m_route.m_width;

  if (m_route.m_bRtIsActive) {
    col = g_pRouteMan->GetActiveRoutePen()->GetColour();
  } else if (m_route.m_bRtIsSelected) {
    col = g_pRouteMan->GetSelectedRoutePen()->GetColour();
  } else {
    if (m_route.m_Colour == wxEmptyString) {
      col = g_pRouteMan->GetRoutePen()->GetColour();
    } else {
      for (unsigned int i = 0; i < sizeof(::GpxxColorNames) / sizeof(wxString);
           i++) {
        if (m_route.m_Colour == ::GpxxColorNames[i]) {
          col = ::GpxxColors[i];
          break;
        }
      }
    }
  }

  wxPenStyle style = wxPENSTYLE_SOLID;
  if (m_route.m_style != wxPENSTYLE_INVALID) style = m_route.m_style;
  wxPen p = *wxThePenList->FindOrCreatePen(col, width, style);
  if(glChartCanvas::dash_map.find(style) != glChartCanvas::dash_map.end()) {
    p.SetDashes(2, &glChartCanvas::dash_map[style][0]);
  }
  dc.SetPen(p);
  dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(col, wxBRUSHSTYLE_SOLID));

  glLineWidth(wxMax(g_GLMinSymbolLineWidth, width));

  dc.SetGLStipple();

  DrawGLLines(vp, &dc, canvas);

  glDisable(GL_LINE_STIPPLE);

  /* direction arrows.. could probably be further optimized for opengl */
  dc.SetPen(*wxThePenList->FindOrCreatePen(col, 1, wxPENSTYLE_SOLID));

  wxRoutePointListNode *node = m_route.pRoutePointList->GetFirst();
  wxPoint rpt1, rpt2;
  while (node) {
    RoutePoint *prp = node->GetData();
    canvas->GetCanvasPointPix(prp->m_lat, prp->m_lon, &rpt2);
    if (node != m_route.pRoutePointList->GetFirst()) {
      if (!prp->m_bIsActive || !g_bAllowShipToActive)
        RenderSegmentArrowsGL(dc, rpt1.x, rpt1.y, rpt2.x, rpt2.y, vp);
    }
    rpt1 = rpt2;
    node = node->GetNext();
  }
#endif
}

void RouteGui::DrawGLLines(ViewPort &vp, ocpnDC *dc, ChartCanvas *canvas) {
#ifdef ocpnUSE_GL
  float pix_full_circle =
      WGS84_semimajor_axis_meters * mercator_k0 * 2 * PI * vp.view_scale_ppm;

  bool r1valid = false;
  wxPoint2DDouble r1;
  wxPoint2DDouble lastpoint;

  wxRoutePointListNode *node = m_route.pRoutePointList->GetFirst();
  RoutePoint *prp2 = node->GetData();
  canvas->GetDoubleCanvasPointPix(prp2->m_lat, prp2->m_lon, &lastpoint);

  // single point.. make sure it shows up for highlighting
  if (m_route.GetnPoints() == 1 && dc) {
    canvas->GetDoubleCanvasPointPix(prp2->m_lat, prp2->m_lon, &r1);
    dc->DrawLine(r1.m_x, r1.m_y, r1.m_x + 2, r1.m_y + 2);
    return;
  }

  //    Handle offscreen points
  LLBBox bbox = vp.GetBBox();

  // dc is passed for thicker highlighted lines (performance not very important)

  for (node = node->GetNext(); node; node = node->GetNext()) {
    RoutePoint *prp1 = prp2;
    prp2 = node->GetData();

    // Provisional, to properly set status of last point in route
    prp2->m_pos_on_screen = false;
    {
      wxPoint2DDouble r2;
      canvas->GetDoubleCanvasPointPix(prp2->m_lat, prp2->m_lon, &r2);
      if (std::isnan(r2.m_x)) {
        r1valid = false;
        continue;
      }

      lastpoint = r2;  // For active track segment to ownship

      // don't need to perform calculations or render segment
      // if both points are past any edge of the vp
      // TODO: use these optimizations for dc mode
      bool lat1l = prp1->m_lat < bbox.GetMinLat(),
           lat2l = prp2->m_lat < bbox.GetMinLat();
      bool lat1r = prp1->m_lat > bbox.GetMaxLat(),
           lat2r = prp2->m_lat > bbox.GetMaxLat();
      if ((lat1l && lat2l) || (lat1r && lat2r)) {
        r1valid = false;
        prp1->m_pos_on_screen = false;
        continue;
      }

      // Possible optimization, not usable if vp crosses IDL (180 E)
      if (!vp.ContainsIDL()) {
        bool lon1l, lon1r, lon2l, lon2r;
        TestLongitude(prp1->m_lon, bbox.GetMinLon(), bbox.GetMaxLon(), lon1l,
                      lon1r);
        TestLongitude(prp2->m_lon, bbox.GetMinLon(), bbox.GetMaxLon(), lon2l,
                      lon2r);
        if ((lon1l && lon2l) || (lon1r && lon2r)) {
          r1valid = false;
          prp1->m_pos_on_screen = false;
          continue;
        }
      }


      if (!r1valid) {
        canvas->GetDoubleCanvasPointPix(prp1->m_lat, prp1->m_lon, &r1);
        if (std::isnan(r1.m_x)) continue;
      }

      //  we must decide which way to go in longitude
      //  for projections which wrap, in this case, we will render two lines
      //  (one may often be off screen which would be nice to fix but complicate
      //  things here anyway, in some cases both points are on screen, but the
      //  route wraps to either side so two lines are needed to draw this
      //  properly

      double adder = 0;
      if ((vp.m_projection_type == PROJECTION_MERCATOR ||
           vp.m_projection_type == PROJECTION_EQUIRECTANGULAR)) {
        float olon = vp.clon > 0 ? vp.clon - 180 : vp.clon + 180;

        if (prp1->m_lon < prp2->m_lon) {
          if (prp2->m_lon - prp1->m_lon < 180) {
            if (olon > prp1->m_lon && olon < prp2->m_lon)
              adder = pix_full_circle;
          } else if (olon < prp1->m_lon || olon > prp2->m_lon)
            adder = -pix_full_circle;
        } else if (prp1->m_lon - prp2->m_lon < 180) {
          if (olon < prp1->m_lon && olon > prp2->m_lon)
            adder = -pix_full_circle;
        } else if (olon > prp1->m_lon || olon < prp2->m_lon)
          adder = pix_full_circle;
      }

      if (dc)
        if (adder) {
          float adderc = cos(vp.rotation) * adder,
                adders = sin(vp.rotation) * adder;
          dc->DrawLine(r1.m_x, r1.m_y, r2.m_x + adderc, r2.m_y + adders);
          dc->DrawLine(r1.m_x - adderc, r1.m_y - adders, r2.m_x, r2.m_y);
        } else
          dc->DrawLine(r1.m_x, r1.m_y, r2.m_x, r2.m_y);
      else {
      }

      r1 = r2;
      r1valid = true;
    }
  }


#endif
}

void RouteGui::CalculateDCRect(wxDC &dc_route, ChartCanvas *canvas,
                               wxRect *prect) {
  dc_route.ResetBoundingBox();
  dc_route.DestroyClippingRegion();

  wxRect update_rect;

  // Draw the route in skeleton form on the dc
  // That is, draw only the route points, assuming that the segements will
  // always be fully contained within the resulting rectangle.
  // Can we prove this?
  if (m_route.m_bVisible) {
    wxRoutePointListNode *node = m_route.pRoutePointList->GetFirst();
    while (node) {
      RoutePoint *prp2 = node->GetData();
      bool blink_save = prp2->m_bBlink;
      prp2->m_bBlink = false;
      ocpnDC odc_route(dc_route);
      odc_route.SetVP(canvas->GetVP());
      RoutePointGui(*prp2).Draw(odc_route, canvas, NULL);
      prp2->m_bBlink = blink_save;

      wxRect r = prp2->CurrentRect_in_DC;
      // allow for large hilite circles at segment ends
      r.Inflate(m_route.m_hiliteWidth, m_route.m_hiliteWidth);

      update_rect.Union(r);
      node = node->GetNext();
    }
  }

  *prect = update_rect;
}

int RouteGui::SendToGPS(const wxString& com_name, bool bsend_waypoints,
                        SendToGpsDlg* dialog) {
  int result = 0;

  N0183DlgCtx dlg_ctx = GetDialogCtx(dialog);
  ::wxBeginBusyCursor();
  result = SendRouteToGPS_N0183(&m_route, com_name, bsend_waypoints, *g_pMUX,
                                dlg_ctx);
  ::wxEndBusyCursor();

  wxString msg;
  if (0 == result)
    msg = _("Route Transmitted.");
  else {
    if (result == ERR_GARMIN_INITIALIZE)
      msg = _("Error on Route Upload.  Garmin GPS not connected");
    else
      msg = _("Error on Route Upload.  Please check logfiles...");
  }
  OCPNMessageBox(NULL, msg, _("OpenCPN Info"), wxOK | wxICON_INFORMATION);

  return (result == 0);
}
