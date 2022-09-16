/***************************************************************************
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
 **************************************************************************/

#include "wx/wxprec.h"

#include <wx/dcscreen.h>
#include <wx/tokenzr.h>

#include "routeman.h"
#include "chcanv.h"
#include "RoutePoint.h"
#include "comm_n0183_output.h"
#include "navutil.h"
#include "FontMgr.h"
#include "cutil.h"
#include "georef.h"
#include "wx28compat.h"
#include "OCPNPlatform.h"
#include "glChartCanvas.h"
#include "Select.h"
#include "gui_lib.h"
#include "Route.h"
#include "svg_utils.h"
#include "ocpn_frame.h"

extern WayPointman *pWayPointMan;
extern bool g_bIsNewLayer;
extern int g_LayerIdx;
extern Routeman *g_pRouteMan;
extern wxRect g_blink_rect;
extern MyFrame *gFrame;
extern bool g_btouch;
extern ocpnStyle::StyleManager *g_StyleManager;
extern double g_n_arrival_circle_radius;
extern int g_iWaypointRangeRingsNumber;
extern float g_fWaypointRangeRingsStep;
extern int g_iWaypointRangeRingsStepUnits;
extern wxColour g_colourWaypointRangeRingsColour;
extern OCPNPlatform *g_Platform;
extern float g_ChartScaleFactorExp;
extern int g_iWpt_ScaMin;
extern bool g_bUseWptScaMin;
extern bool g_bOverruleScaMin;

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(RoutePointList);

RoutePoint::RoutePoint() {
  m_pbmIcon = NULL;

  //  Nice defaults
  m_seg_len = 0.0;
  m_seg_vmg = 0.0;

  m_seg_etd = wxInvalidDateTime;
  m_manual_etd = false;

  m_seg_eta = wxInvalidDateTime;
  m_bDynamicName = false;
  m_bPtIsSelected = false;
  m_bRPIsBeingEdited = false;
  m_bIsActive = false;
  m_bBlink = false;
  m_bIsInRoute = false;
  m_CreateTimeX = wxDateTime::Now();
  m_bIsolatedMark = false;
  m_bShowName = true;
  SetShared(false);
  m_bIsVisible = true;
  m_bIsListed = true;
  CurrentRect_in_DC = wxRect(0, 0, 0, 0);
  m_NameLocationOffsetX = -10;
  m_NameLocationOffsetY = 8;
  m_pMarkFont = NULL;
  m_btemp = false;
  m_SelectNode = NULL;
  m_ManagerNode = NULL;

  m_iTextTexture = 0;

  m_HyperlinkList = new HyperlinkList;

  m_GUID = pWayPointMan->CreateGUID(this);

  m_IconName = wxEmptyString;
  ReLoadIcon();

  m_MarkName = wxEmptyString;

  m_bIsInLayer = false;
  m_LayerID = 0;

  m_WaypointArrivalRadius = g_n_arrival_circle_radius;

  m_bShowWaypointRangeRings = (bool)g_iWaypointRangeRingsNumber;

  m_iWaypointRangeRingsNumber = g_iWaypointRangeRingsNumber;
  m_fWaypointRangeRingsStep = g_fWaypointRangeRingsStep;
  m_iWaypointRangeRingsStepUnits = g_iWaypointRangeRingsStepUnits;
  m_wxcWaypointRangeRingsColour = g_colourWaypointRangeRingsColour;
  m_ScaMin = g_iWpt_ScaMin;
  m_ScaMax = 0;
  b_UseScamin = g_bUseWptScaMin;

#ifdef ocpnUSE_GL
  m_pos_on_screen = false;
#endif
  m_bDrawDragHandle = false;
  m_dragIconTexture = 0;
  m_draggingOffsetx = m_draggingOffsety = 0;

  m_PlannedSpeed = 0.;
}

// Copy Constructor
RoutePoint::RoutePoint(RoutePoint *orig) {
  m_MarkName = orig->GetName();
  m_lat = orig->m_lat;
  m_lon = orig->m_lon;
  m_seg_len = orig->m_seg_len;
  m_seg_vmg = orig->m_seg_vmg;

  m_seg_etd = orig->m_seg_etd;
  m_manual_etd = false;

  m_bDynamicName = orig->m_bDynamicName;
  m_bPtIsSelected = orig->m_bPtIsSelected;
  m_bRPIsBeingEdited = orig->m_bRPIsBeingEdited;
  m_bIsActive = orig->m_bIsActive;
  m_bBlink = orig->m_bBlink;
  m_bIsInRoute = orig->m_bIsInRoute;
  m_CreateTimeX = orig->m_CreateTimeX;
  m_bIsolatedMark = orig->m_bIsolatedMark;
  m_bShowName = orig->m_bShowName;
  SetShared(orig->IsShared());
  m_bIsVisible = orig->m_bIsVisible;
  m_bIsListed = orig->m_bIsListed;
  CurrentRect_in_DC = orig->CurrentRect_in_DC;
  m_NameLocationOffsetX = orig->m_NameLocationOffsetX;
  m_NameLocationOffsetY = orig->m_NameLocationOffsetY;
  m_pMarkFont = orig->m_pMarkFont;
  m_MarkDescription = orig->m_MarkDescription;
  m_btemp = orig->m_btemp;
  m_ScaMin = orig->m_ScaMin;
  m_ScaMax = orig->m_ScaMax;
  m_HyperlinkList = new HyperlinkList;
  m_IconName = orig->m_IconName;
  m_TideStation = orig->m_TideStation;
  SetPlannedSpeed(orig->GetPlannedSpeed());
  ReLoadIcon();

  m_bIsInLayer = orig->m_bIsInLayer;
  m_GUID = pWayPointMan->CreateGUID(this);

  m_SelectNode = NULL;
  m_ManagerNode = NULL;

  m_WaypointArrivalRadius = orig->GetWaypointArrivalRadius();
  m_bShowWaypointRangeRings = orig->m_bShowWaypointRangeRings;
  m_iWaypointRangeRingsNumber = orig->m_iWaypointRangeRingsNumber;
  m_fWaypointRangeRingsStep = orig->m_fWaypointRangeRingsStep;
  m_iWaypointRangeRingsStepUnits = orig->m_iWaypointRangeRingsStepUnits;
  m_wxcWaypointRangeRingsColour = orig->m_wxcWaypointRangeRingsColour;
  m_ScaMin = orig->m_ScaMin;
  m_ScaMax = orig->m_ScaMax;
  b_UseScamin = orig->b_UseScamin;

  m_bDrawDragHandle = false;
  m_dragIconTexture = 0;
  m_draggingOffsetx = m_draggingOffsety = 0;
}

RoutePoint::RoutePoint(double lat, double lon, const wxString &icon_ident,
                       const wxString &name, const wxString &pGUID,
                       bool bAddToList) {
  //  Establish points
  m_lat = lat;
  m_lon = lon;

  //      Normalize the longitude, to fix any old poorly formed points
  if (m_lon < -180.)
    m_lon += 360.;
  else if (m_lon > 180.)
    m_lon -= 360.;

  //  Nice defaults
  m_seg_len = 0.0;
  m_seg_vmg = 0.0;

  m_seg_etd = wxInvalidDateTime;
  m_manual_etd = false;

  m_bDynamicName = false;
  m_bPtIsSelected = false;
  m_bRPIsBeingEdited = false;
  m_bIsActive = false;
  m_bBlink = false;
  m_bIsInRoute = false;
  m_CreateTimeX = wxDateTime::Now();
  m_bIsolatedMark = false;
  m_bShowName = true;
  SetShared(false);
  m_bIsVisible = true;
  m_bIsListed = true;
  CurrentRect_in_DC = wxRect(0, 0, 0, 0);
  m_NameLocationOffsetX = -10;
  m_NameLocationOffsetY = 8;
  m_pMarkFont = NULL;
  m_btemp = false;
  m_bPreScaled = false;

  m_SelectNode = NULL;
  m_ManagerNode = NULL;
  m_IconScaleFactor = 1.0;
  m_ScaMin = MAX_INT_VAL;
  m_ScaMax = 0;
  m_HyperlinkList = new HyperlinkList;

  if (!pGUID.IsEmpty())
    m_GUID = pGUID;
  else
    m_GUID = pWayPointMan->CreateGUID(this);

  //      Get Icon bitmap
  m_IconName = icon_ident;
  ReLoadIcon();

  SetName(name);

  //  Possibly add the waypoint to the global list maintained by the waypoint
  //  manager

  if (bAddToList && NULL != pWayPointMan) pWayPointMan->AddRoutePoint(this);

  m_bIsInLayer = g_bIsNewLayer;
  if (m_bIsInLayer) {
    m_LayerID = g_LayerIdx;
    m_bIsListed = false;
  } else
    m_LayerID = 0;

  SetWaypointArrivalRadius(g_n_arrival_circle_radius);

  m_bShowWaypointRangeRings = (bool)g_iWaypointRangeRingsNumber;

  m_iWaypointRangeRingsNumber = g_iWaypointRangeRingsNumber;
  m_fWaypointRangeRingsStep = g_fWaypointRangeRingsStep;
  m_iWaypointRangeRingsStepUnits = g_iWaypointRangeRingsStepUnits;
  m_wxcWaypointRangeRingsColour = g_colourWaypointRangeRingsColour;
  m_ScaMin = g_iWpt_ScaMin;
  m_ScaMax = 0;
  b_UseScamin = g_bUseWptScaMin;

  m_bDrawDragHandle = false;
  m_dragIconTexture = 0;
  m_draggingOffsetx = m_draggingOffsety = 0;

  m_PlannedSpeed = 0.;
}

RoutePoint::~RoutePoint() {
  //  Remove this point from the global waypoint list
  if (NULL != pWayPointMan) pWayPointMan->RemoveRoutePoint(this);

  if (m_HyperlinkList) {
    m_HyperlinkList->DeleteContents(true);
    delete m_HyperlinkList;
  }
#ifdef ocpnUSE_GL
  if (m_dragIconTexture > 0) glDeleteTextures(1, &m_dragIconTexture);
#endif
}

wxPoint2DDouble RoutePoint::GetDragHandlePoint(ChartCanvas *canvas) {
  if (!m_bDrawDragHandle)
    return wxPoint2DDouble(m_lon, m_lat);
  else {
    return computeDragHandlePoint(canvas);
  }
}

wxPoint2DDouble RoutePoint::computeDragHandlePoint(ChartCanvas *canvas) {
  wxPoint r;
  canvas->GetCanvasPointPix(m_lat, m_lon, &r);
  double lat, lon;
  canvas->GetCanvasPixPoint(r.x + m_drag_icon_offset, r.y + m_drag_icon_offset,
                            lat, lon);

  // Keep the members updated
  m_dragHandleLat = lat;
  m_dragHandleLon = lon;

  return wxPoint2DDouble(lon, lat);
}

void RoutePoint::SetPointFromDraghandlePoint(ChartCanvas *canvas, double lat,
                                             double lon) {
  wxPoint r;
  canvas->GetCanvasPointPix(lat, lon, &r);
  double tlat, tlon;
  canvas->GetCanvasPixPoint(r.x - m_drag_icon_offset, r.y - m_drag_icon_offset,
                            tlat, tlon);
  m_lat = tlat;
  m_lon = tlon;
}

void RoutePoint::SetPointFromDraghandlePoint(ChartCanvas *canvas, int x,
                                             int y) {
  double tlat, tlon;
  canvas->GetCanvasPixPoint(x - m_drag_icon_offset - m_draggingOffsetx,
                            y - m_drag_icon_offset - m_draggingOffsety, tlat,
                            tlon);
  m_lat = tlat;
  m_lon = tlon;
}

void RoutePoint::PresetDragOffset(ChartCanvas *canvas, int x, int y) {
  wxPoint r;
  canvas->GetCanvasPointPix(m_lat, m_lon, &r);

  m_draggingOffsetx = x - (r.x + m_drag_icon_offset);
  m_draggingOffsety = y - (r.y + m_drag_icon_offset);
}

void RoutePoint::EnableDragHandle(bool bEnable) {
  m_bDrawDragHandle = bEnable;
  if (bEnable) {
    if (!m_dragIcon.IsOk()) {
      // Get the icon
      // What size?
      int bm_size = g_Platform->GetDisplayDPmm() * 9;  // 9 mm nominal

      // What icon?
      wxString UserIconPath = g_Platform->GetSharedDataDir() + _T("uidata") +
                              wxFileName::GetPathSeparator();

      m_dragIcon = LoadSVG(UserIconPath + _T("DragHandle.svg"), bm_size,
                           bm_size, m_pbmIcon);

      // build a texture
#ifdef ocpnUSE_GL
      /* make rgba texture */
      if (m_dragIconTexture == 0) {
        glGenTextures(1, &m_dragIconTexture);
        glBindTexture(GL_TEXTURE_2D, m_dragIconTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        wxImage image = m_dragIcon.ConvertToImage();
        int w = image.GetWidth(), h = image.GetHeight();

        m_dragIconTextureWidth = NextPow2(w);
        m_dragIconTextureHeight = NextPow2(h);

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

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_dragIconTextureWidth,
                     m_dragIconTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     NULL);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                        e);

        delete[] e;
      }
#endif

      // set the drawing metrics
      if (m_dragIcon.IsOk()) {
        m_drag_line_length_man = bm_size;
        m_drag_icon_offset = bm_size;
      } else {
        m_drag_line_length_man = 64;
        m_drag_icon_offset = 64;
      }
    }
  }
}

wxDateTime RoutePoint::GetCreateTime() {
  if (!m_CreateTimeX.IsValid()) {
    if (m_timestring.Len()) ParseGPXDateTime(m_CreateTimeX, m_timestring);
  }
  return m_CreateTimeX;
}

void RoutePoint::SetCreateTime(wxDateTime dt) { m_CreateTimeX = dt; }

void RoutePoint::SetName(const wxString &name) {
#ifdef ocpnUSE_GL
  if (m_iTextTexture) {
    glDeleteTextures(1, &m_iTextTexture);
    m_iTextTexture = 0;
  }
#endif
  m_MarkName = name;
  CalculateNameExtents();
}

void RoutePoint::CalculateNameExtents(void) {
  if (m_pMarkFont) {
    wxScreenDC dc;

#ifdef __WXQT__  // avoiding "painter not active" warning
    int w, h;
    dc.GetTextExtent(m_MarkName, &w, &h, NULL, NULL, m_pMarkFont);
    m_NameExtents = wxSize(w, h);
#else
    dc.SetFont(*m_pMarkFont);
    m_NameExtents = dc.GetMultiLineTextExtent(m_MarkName);
#endif
  } else
    m_NameExtents = wxSize(0, 0);
}

void RoutePoint::ReLoadIcon(void) {
  if (!pWayPointMan) return;
  bool icon_exists = pWayPointMan->DoesIconExist(m_IconName);

  wxString iconUse = m_IconName;
  if (!icon_exists) {
    //  Try all lower case as a favor in the case where imported waypoints use
    //  mixed case names
    wxString tentative_icon = m_IconName.Lower();
    if (pWayPointMan->DoesIconExist(tentative_icon)) {
      // if found, convert point's icon name permanently.
      m_IconName = tentative_icon;
      iconUse = m_IconName;
    }
    //      Icon name is not in the standard or user lists, so add to the list a
    //      generic placeholder
    else {
      if (!pWayPointMan->DoesIconExist(_T("tempsub"))) {
        ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
        if (style) {
          wxBitmap bmp = style->GetIcon(_T("circle"));
          if (bmp.IsOk())
            pWayPointMan->ProcessIcon(bmp, _T("tempsub"), _T("tempsub"));
        }
      }
      iconUse = _T("tempsub");
    }
  }

  m_pbmIcon = pWayPointMan->GetIconBitmap(iconUse);
  m_bPreScaled = pWayPointMan->GetIconPrescaled(iconUse);

#ifdef ocpnUSE_GL
  m_wpBBox_view_scale_ppm = -1;

  m_iTextTexture = 0;
#endif

  m_IconScaleFactor = -1;  // Force scaled icon reload
  m_pMarkFont = 0;         // Force Font color reload
}

bool RoutePoint::IsVisibleSelectable(ChartCanvas *canvas, bool boverrideViz) {
  if (m_bIsActive)  //  An active route point must always be visible
    return true;

  if (!boverrideViz) {
    if (!m_bIsVisible)  // if not visible nevermind the rest.
      return false;
  }

  if (b_UseScamin) {
    if (g_bOverruleScaMin)
      return true;
    else if (canvas->GetScaleValue() >= (double)(m_ScaMin + 1))
      return false;
  }
  return true;
}

int RoutePoint::GetIconImageIndex() {
  if (IsShared()) {
    //    Get an array of all routes using this point
    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining(this);

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
      if (IsVisible())
        return (pWayPointMan->GetIconImageListIndex(GetIconBitmap()));
      else
        return (pWayPointMan->GetXIconImageListIndex(GetIconBitmap()));
    }
  }

  else {  // point is not shared
    if (IsVisible())
      return (pWayPointMan->GetIconImageListIndex(GetIconBitmap()));
    else
      return (pWayPointMan->GetXIconImageListIndex(GetIconBitmap()));
  }
}

bool RoutePoint::IsSharedInVisibleRoute() {
  if (IsShared()) {
    //    Get an array of all routes using this point
    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining(this);

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

    return brp_viz;
  } else  // point is not shared
    return false;
}

void RoutePoint::Draw(ocpnDC &dc, ChartCanvas *canvas, wxPoint *rpn,
                      bool boverride_viz) {
  wxPoint r;
  wxRect hilitebox;

  canvas->GetCanvasPointPix(m_lat, m_lon, &r);

  //  return the home point in this dc to allow "connect the dots"
  if (NULL != rpn) *rpn = r;

  /*if( !m_bIsVisible )     // pjotrc 2010.02.13, 2011.02.24
      return;
  if( !m_bIsActive)  //  An active route point must always be visible
      if( !IsScaVisible( canvas) )
          return;   */
  if (!IsVisibleSelectable(canvas, boverride_viz)) return;

  //    Optimization, especially apparent on tracks in normal cases
  if (m_IconName == _T("empty") && !m_bShowName && !m_bPtIsSelected) return;

  wxPen *pen;
  if (m_bBlink)
    pen = g_pRouteMan->GetActiveRoutePointPen();
  else
    pen = g_pRouteMan->GetRoutePointPen();

  //    Substitue icon?
  wxBitmap *pbm;
  if ((m_bIsActive) && (m_IconName != _T("mob")))
    pbm = pWayPointMan->GetIconBitmap(_T ( "activepoint" ));
  else
    pbm = m_pbmIcon;

  wxBitmap *pbms = NULL;
  if ((g_ChartScaleFactorExp > 1.0) && !m_bPreScaled) {
    if (m_IconScaleFactor != g_ChartScaleFactorExp) {
      wxImage scaled_image = pbm->ConvertToImage();
      int new_width = pbm->GetWidth() * g_ChartScaleFactorExp;
      int new_height = pbm->GetHeight() * g_ChartScaleFactorExp;
      m_ScaledBMP = wxBitmap(
          scaled_image.Scale(new_width, new_height, wxIMAGE_QUALITY_HIGH));

      m_IconScaleFactor = g_ChartScaleFactorExp;
    }
    if (m_ScaledBMP.IsOk()) pbm = &m_ScaledBMP;
  }

  int sx2 = pbm->GetWidth() / 2;
  int sy2 = pbm->GetHeight() / 2;

  //    Calculate the mark drawing extents
  wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);  // the bitmap extents

  if (m_bShowName) {
    if (0 == m_pMarkFont) {
      m_pMarkFont = FontMgr::Get().GetFont(_("Marks"));
      m_FontColor = FontMgr::Get().GetFontColor(_("Marks"));
      CalculateNameExtents();
    }

    if (m_pMarkFont) {
      wxRect r2(r.x + m_NameLocationOffsetX, r.y + m_NameLocationOffsetY,
                m_NameExtents.x, m_NameExtents.y);
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
  if (m_bRPIsBeingEdited) {
    hi_colour = GetGlobalColor(_T ( "YELO1" ));
    transparency = 150;
  }

  //  Highlite any selected point
  if (m_bPtIsSelected || m_bRPIsBeingEdited) {
    AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width,
                  hilitebox.height, radius, hi_colour, transparency);
  }

  bool bDrawHL = false;

  if (m_bBlink && (gFrame->nBlinkerTick & 1)) bDrawHL = true;

  if ((!bDrawHL) && (NULL != m_pbmIcon)) {
    dc.DrawBitmap(*pbm, r.x - sx2, r.y - sy2, true);
    // on MSW, the dc Bounding box is not updated on DrawBitmap() method.
    // Do it explicitely here for all platforms.
    dc.CalcBoundingBox(r.x - sx2, r.y - sy2);
    dc.CalcBoundingBox(r.x + sx2, r.y + sy2);
  }

  if (m_bShowName) {
    if (m_pMarkFont) {
      dc.SetFont(*m_pMarkFont);
      dc.SetTextForeground(m_FontColor);

      dc.DrawText(m_MarkName, r.x + m_NameLocationOffsetX,
                  r.y + m_NameLocationOffsetY);
    }
  }

  // Draw waypoint radar rings if activated
  if (m_iWaypointRangeRingsNumber && m_bShowWaypointRangeRings) {
    double factor = 1.00;
    if (m_iWaypointRangeRingsStepUnits == 1)  // nautical miles
      factor = 1 / 1.852;

    factor *= m_fWaypointRangeRingsStep;

    double tlat, tlon;
    wxPoint r1;
    ll_gc_ll(m_lat, m_lon, 0, factor, &tlat, &tlon);
    canvas->GetCanvasPointPix(tlat, tlon, &r1);

    double lpp =
        sqrt(pow((double)(r.x - r1.x), 2) + pow((double)(r.y - r1.y), 2));
    int pix_radius = (int)lpp;

    wxPen ppPen1(m_wxcWaypointRangeRingsColour, 2);
    wxBrush saveBrush = dc.GetBrush();
    wxPen savePen = dc.GetPen();
    dc.SetPen(ppPen1);
    dc.SetBrush(
        wxBrush(m_wxcWaypointRangeRingsColour, wxBRUSHSTYLE_TRANSPARENT));

    for (int i = 1; i <= m_iWaypointRangeRingsNumber; i++)
      dc.StrokeCircle(r.x, r.y, i * pix_radius);
    dc.SetPen(savePen);
    dc.SetBrush(saveBrush);
  }

  //  Save the current draw rectangle in the current DC
  //    This will be useful for fast icon redraws
  CurrentRect_in_DC.x = r.x + hilitebox.x;
  CurrentRect_in_DC.y = r.y + hilitebox.y;
  CurrentRect_in_DC.width = hilitebox.width;
  CurrentRect_in_DC.height = hilitebox.height;

  if (m_bBlink)
    g_blink_rect = CurrentRect_in_DC;  // also save for global blinker

  delete pbms;  // the potentially scaled bitmap
}

#ifdef ocpnUSE_GL
void RoutePoint::DrawGL(ViewPort &vp, ChartCanvas *canvas,
                        bool use_cached_screen_coords, bool bVizOverride) {
  if (!IsVisibleSelectable(canvas, bVizOverride)) return;

  //    Optimization, especially apparent on tracks in normal cases
  if (m_IconName == _T("empty") && !m_bShowName && !m_bPtIsSelected) return;

  if (m_wpBBox.GetValid() && vp.view_scale_ppm == m_wpBBox_view_scale_ppm &&
      vp.rotation == m_wpBBox_rotation) {
    /* see if this waypoint can intersect with bounding box */
    LLBBox vpBBox = vp.GetBBox();
    if (vpBBox.IntersectOut(m_wpBBox)) {
      // Are Range Rings enabled?
      if (m_bShowWaypointRangeRings && (m_iWaypointRangeRingsNumber > 0)) {
        double factor = 1.00;
        if (m_iWaypointRangeRingsStepUnits == 1)  // convert kilometers to NMi
          factor = 1 / 1.852;

        double radius = factor * m_iWaypointRangeRingsNumber *
                        m_fWaypointRangeRingsStep / 60.;

        LLBBox radar_box = m_wpBBox;
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

  if (use_cached_screen_coords && m_pos_on_screen)
    r.x = m_screen_pos.m_x, r.y = m_screen_pos.m_y;
  else
    canvas->GetCanvasPointPix(m_lat, m_lon, &r);

  if (r.x == INVALID_COORD) return;

  //    Substitute icon?
  wxBitmap *pbm;
  if ((m_bIsActive) && (m_IconName != _T("mob")))
    pbm = pWayPointMan->GetIconBitmap(_T ( "activepoint" ));
  else
    pbm = m_pbmIcon;

  //  If icon is corrupt, there is really nothing else to do...
  if (!pbm || !pbm->IsOk()) return;

  int sx2 = pbm->GetWidth() / 2;
  int sy2 = pbm->GetHeight() / 2;

  //    Calculate the mark drawing extents
  wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);  // the bitmap extents

  wxRect r3 = r1;
  if (m_bShowName) {
    if (!m_pMarkFont) {
      m_pMarkFont = FontMgr::Get().GetFont(_("Marks"));
      m_FontColor = FontMgr::Get().GetFontColor(_("Marks"));
      if (m_iTextTexture) {
        glDeleteTextures(1, &m_iTextTexture);
        m_iTextTexture = 0;
      }

      CalculateNameExtents();
    }

    if (m_pMarkFont) {
      wxRect r2(r.x + m_NameLocationOffsetX, r.y + m_NameLocationOffsetY,
                m_NameExtents.x, m_NameExtents.y);
      r3.Union(r2);
    }
  }

  hilitebox = r3;
  hilitebox.x -= r.x;
  hilitebox.y -= r.y;

  if (!m_bPreScaled) {
    hilitebox.x *= g_ChartScaleFactorExp;
    hilitebox.y *= g_ChartScaleFactorExp;
    hilitebox.width *= g_ChartScaleFactorExp;
    hilitebox.height *= g_ChartScaleFactorExp;
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
  if (!m_wpBBox.GetValid() || vp.view_scale_ppm != m_wpBBox_view_scale_ppm ||
      vp.rotation != m_wpBBox_rotation) {
    double lat1, lon1, lat2, lon2;
    canvas->GetCanvasPixPoint(r.x + hilitebox.x,
                              r.y + hilitebox.y + hilitebox.height, lat1, lon1);
    canvas->GetCanvasPixPoint(r.x + hilitebox.x + hilitebox.width,
                              r.y + hilitebox.y, lat2, lon2);

    if (lon1 > lon2)
      m_wpBBox.Set(lat1, lon1, lat2, lon2 + 360);
    else
      m_wpBBox.Set(lat1, lon1, lat2, lon2);

    m_wpBBox_view_scale_ppm = vp.view_scale_ppm;
    m_wpBBox_rotation = vp.rotation;
  }

  //    if(region.Contains(r3) == wxOutRegion)
  //        return;

  ocpnDC dc;

  //  Highlite any selected point
  if (m_bPtIsSelected) {
    wxColour hi_colour;
    if (m_bBlink) {
      wxPen *pen = g_pRouteMan->GetActiveRoutePointPen();
      hi_colour = pen->GetColour();
    } else {
      hi_colour = GetGlobalColor(_T ( "YELO1" ));
    }

    AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width,
                  hilitebox.height, radius, hi_colour, transparency);
  }

  bool bDrawHL = false;

  if (m_bBlink && (gFrame->nBlinkerTick & 1)) bDrawHL = true;

  if ((!bDrawHL) && (NULL != m_pbmIcon)) {
    int glw, glh;
    unsigned int IconTexture = pWayPointMan->GetIconTexture(pbm, glw, glh);

    glBindTexture(GL_TEXTURE_2D, IconTexture);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w = r1.width, h = r1.height;

    float scale = 1.0;
    if (!m_bPreScaled) {
      scale = g_ChartScaleFactorExp;
    }

    float ws = r1.width * scale;
    float hs = r1.height * scale;
    float xs = r.x - ws / 2.;
    float ys = r.y - hs / 2.;
    float u = (float)w / glw, v = (float)h / glh;

#ifdef USE_ANDROID_GLES2
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

    glChartCanvas::RenderSingleTexture(coords, uv, &vp, 0, 0, 0);

#else
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(xs, ys);
    glTexCoord2f(u, 0);
    glVertex2f(xs + ws, ys);
    glTexCoord2f(u, v);
    glVertex2f(xs + ws, ys + hs);
    glTexCoord2f(0, v);
    glVertex2f(xs, ys + hs);
    glEnd();

#endif

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
  }

  if (m_bShowName && m_pMarkFont) {
    int w = m_NameExtents.x, h = m_NameExtents.y;
    if (!m_iTextTexture && w && h) {
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
      sdc.SetFont(*m_pMarkFont);
      sdc.GetTextExtent(m_MarkName, &w, &h, NULL, NULL, m_pMarkFont);

      /* create bitmap of appropriate size and select it */
      wxBitmap bmp(w, h);
      wxMemoryDC temp_dc;
      temp_dc.SelectObject(bmp);

      /* fill bitmap with black */
      temp_dc.SetBackground(wxBrush(wxColour(0, 0, 0)));
      temp_dc.Clear();

      /* draw the text white */
      temp_dc.SetFont(*m_pMarkFont);
      temp_dc.SetTextForeground(wxColour(255, 255, 255));
      temp_dc.DrawText(m_MarkName, 0, 0);
      temp_dc.SelectObject(wxNullBitmap);

      /* use the data in the bitmap for alpha channel,
       and set the color to text foreground */
      wxImage image = bmp.ConvertToImage();

      unsigned char *data = new unsigned char[w * h * 4];
      unsigned char *im = image.GetData();

      if (im) {
        unsigned int r = m_FontColor.Red();
        unsigned int g = m_FontColor.Green();
        unsigned int b = m_FontColor.Blue();
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

      glGenTextures(1, &m_iTextTexture);

      glBindTexture(GL_TEXTURE_2D, m_iTextTexture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      m_iTextTextureWidth = NextPow2(w);
      m_iTextTextureHeight = NextPow2(h);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iTextTextureWidth,
                   m_iTextTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                      data);

      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#endif
    }

    if (m_iTextTexture) {
      /* draw texture with text */
      glBindTexture(GL_TEXTURE_2D, m_iTextTexture);

      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);

      int x = r.x + m_NameLocationOffsetX, y = r.y + m_NameLocationOffsetY;
      float u = (float)w / m_iTextTextureWidth,
            v = (float)h / m_iTextTextureHeight;
#ifndef USE_ANDROID_GLES2
      glColor3ub(255, 255, 255);

      glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex2f(x, y);
      glTexCoord2f(u, 0);
      glVertex2f(x + w, y);
      glTexCoord2f(u, v);
      glVertex2f(x + w, y + h);
      glTexCoord2f(0, v);
      glVertex2f(x, y + h);
      glEnd();

#else
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

      glChartCanvas::RenderSingleTexture(coords, uv, &vp, 0, 0, 0);

#endif
      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
    }
  }

  // Draw waypoint radar rings if activated
  if (m_iWaypointRangeRingsNumber && m_bShowWaypointRangeRings) {
    double factor = 1.00;
    if (m_iWaypointRangeRingsStepUnits == 1)  // nautical miles
      factor = 1 / 1.852;

    factor *= m_fWaypointRangeRingsStep;

    double tlat, tlon;
    wxPoint r1;
    ll_gc_ll(m_lat, m_lon, 0, factor, &tlat, &tlon);
    canvas->GetCanvasPointPix(tlat, tlon, &r1);

    double lpp =
        sqrt(pow((double)(r.x - r1.x), 2) + pow((double)(r.y - r1.y), 2));
    int pix_radius = (int)lpp;

    extern wxColor GetDimColor(wxColor c);
    wxColor ring_dim_color = GetDimColor(m_wxcWaypointRangeRingsColour);

    double platform_pen_width = wxRound(
        wxMax(1.0, g_Platform->GetDisplayDPmm() /
                       2));  // 0.5 mm nominal, but not less than 1 pixel
    wxPen ppPen1(ring_dim_color, platform_pen_width);
    wxBrush saveBrush = dc.GetBrush();
    wxPen savePen = dc.GetPen();
    dc.SetPen(ppPen1);
    dc.SetBrush(wxBrush(ring_dim_color, wxBRUSHSTYLE_TRANSPARENT));

    for (int i = 1; i <= m_iWaypointRangeRingsNumber; i++)
      dc.StrokeCircle(r.x, r.y, i * pix_radius);
    dc.SetPen(savePen);
    dc.SetBrush(saveBrush);
  }

  // Render Drag handle if enabled
  if (m_bDrawDragHandle) {
    //  A line, southeast, scaled to the size of the icon
    double platform_pen_width = wxRound(
        wxMax(1.0, g_Platform->GetDisplayDPmm() /
                       2));  // 0.5 mm nominal, but not less than 1 pixel

    wxColor dh_color = GetGlobalColor(_T ( "YELO1" ));
    wxPen ppPen1(dh_color, 3 * platform_pen_width);
    dc.SetPen(ppPen1);
    dc.DrawLine(r.x + hilitebox.width / 4, r.y + hilitebox.height / 4,
                r.x + m_drag_line_length_man, r.y + m_drag_line_length_man);

    dh_color = wxColor(0, 0, 0);
    wxPen ppPen2(dh_color, platform_pen_width);
    dc.SetPen(ppPen2);
    dc.DrawLine(r.x + hilitebox.width / 4, r.y + hilitebox.height / 4,
                r.x + m_drag_line_length_man, r.y + m_drag_line_length_man);

    // The drag handle
    glBindTexture(GL_TEXTURE_2D, m_dragIconTexture);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    int x = r.x + m_drag_icon_offset, y = r.y + m_drag_icon_offset,
        w = m_dragIcon.GetWidth(), h = m_dragIcon.GetHeight();

    float scale = 1.0;

    float ws = w * scale;
    float hs = h * scale;
    float xs = x - ws / 2.;
    float ys = y - hs / 2.;
    float u = (float)w / m_dragIconTextureWidth,
          v = (float)h / m_dragIconTextureWidth;

#ifndef USE_ANDROID_GLES2
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(xs, ys);
    glTexCoord2f(u, 0);
    glVertex2f(xs + ws, ys);
    glTexCoord2f(u, v);
    glVertex2f(xs + ws, ys + hs);
    glTexCoord2f(0, v);
    glVertex2f(xs, ys + hs);
    glEnd();

#else
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

    glChartCanvas::RenderSingleTexture(coords, uv, &vp, 0, 0, 0);

#endif
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
  }

  if (m_bBlink)
    g_blink_rect = CurrentRect_in_DC;  // also save for global blinker

  //    This will be useful for fast icon redraws
  CurrentRect_in_DC.x = r.x + hilitebox.x;
  CurrentRect_in_DC.y = r.y + hilitebox.y;
  CurrentRect_in_DC.width = hilitebox.width;
  CurrentRect_in_DC.height = hilitebox.height;

  if (m_bBlink)
    g_blink_rect = CurrentRect_in_DC;  // also save for global blinker
}
#endif

void RoutePoint::SetPosition(double lat, double lon) {
  m_lat = lat;
  m_lon = lon;
}

void RoutePoint::CalculateDCRect(wxDC &dc, ChartCanvas *canvas, wxRect *prect) {
  dc.ResetBoundingBox();
  dc.DestroyClippingRegion();

  // Draw the mark on the dc
  ocpnDC odc(dc);
  Draw(odc, canvas, NULL);

  //  Retrieve the drawing extents
  prect->x = dc.MinX() - 1;
  prect->y = dc.MinY() - 1;
  prect->width = dc.MaxX() - dc.MinX() + 2;  // Mouse Poop?
  prect->height = dc.MaxY() - dc.MinY() + 2;
}

bool RoutePoint::IsSame(RoutePoint *pOtherRP) {
  bool IsSame = false;

  if (this->m_MarkName == pOtherRP->m_MarkName) {
    if (fabs(this->m_lat - pOtherRP->m_lat) < 1.e-6 &&
        fabs(this->m_lon - pOtherRP->m_lon) < 1.e-6)
      IsSame = true;
  }
  return IsSame;
}

bool RoutePoint::SendToGPS(const wxString &com_name, SendToGpsDlg *dialog) {
  int result = 0;
  result = SendWaypointToGPS_N0183(this, com_name/*, dialog*/);

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

double RoutePoint::GetWaypointArrivalRadius() {
  if ((m_WaypointArrivalRadius >= 0) && (m_WaypointArrivalRadius < 0.001)) {
    SetWaypointArrivalRadius(g_n_arrival_circle_radius);
    return m_WaypointArrivalRadius;
  } else
    return m_WaypointArrivalRadius;
}

int RoutePoint::GetWaypointRangeRingsNumber() {
  if (m_iWaypointRangeRingsNumber == -1)
    return g_iWaypointRangeRingsNumber;
  else
    return m_iWaypointRangeRingsNumber;
}

float RoutePoint::GetWaypointRangeRingsStep() {
  if (m_fWaypointRangeRingsStep == -1)
    return g_fWaypointRangeRingsStep;
  else
    return m_fWaypointRangeRingsStep;
}

int RoutePoint::GetWaypointRangeRingsStepUnits() {
  if (m_iWaypointRangeRingsStepUnits == -1)
    return g_iWaypointRangeRingsStepUnits;
  else
    return m_iWaypointRangeRingsStepUnits;
}

wxColour RoutePoint::GetWaypointRangeRingsColour(void) {
  if (m_wxcWaypointRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX) ==
      _T("#FFFFFF"))
    return g_colourWaypointRangeRingsColour;
  else
    return m_wxcWaypointRangeRingsColour;
}

void RoutePoint::SetScaMin(long val) {
  if (val < SCAMIN_MIN)
    val = SCAMIN_MIN;  // prevent from waypoints hiding always with a nonlogic
                       // value
  if (val < (long)m_ScaMax * 5) val = (long)m_ScaMax * 5;
  m_ScaMin = val;
}
void RoutePoint::SetScaMin(wxString str) {
  long val;
  if (!str.ToLong(&val)) val = MAX_INT_VAL;
  SetScaMin(val);
}

void RoutePoint::SetScaMax(long val) {
  if (val > (int)m_ScaMin / 5)
    m_ScaMax = (int)m_ScaMin /
               5;  // prevent from waypoints hiding always with a nonlogic value
}
void RoutePoint::SetScaMax(wxString str) {
  long val;
  if (!str.ToLong(&val)) val = 0;
  SetScaMax(val);
}

void RoutePoint::ShowScaleWarningMessage(ChartCanvas *canvas) {
  wxString strA = _("The ScaMin value for new waypoints is set to");
  wxString strB = _("but current chartscale is");
  wxString strC =
      _("Therefore the new waypoint will not be visible at this zoom level.");
  wxString MessStr =
      wxString::Format(_T("%s %li,\n %s %.0f.\n%s"), strA, GetScaMin(), strB,
                       canvas->GetScaleValue(), strC);
  OCPNMessageBox(canvas, MessStr);
}

void RoutePoint::SetPlannedSpeed(double spd) {
  if (spd >= 0.0 && spd <= 1000.0) m_PlannedSpeed = spd;
}

double RoutePoint::GetPlannedSpeed() {
  if (m_PlannedSpeed < 0.0001 &&
      m_MarkDescription.Find(_T("VMG=")) != wxNOT_FOUND) {
    // In case there was speed encoded in the name of the waypoint, do the
    // conversion here.
    wxString s_vmg =
        (m_MarkDescription.Mid(m_MarkDescription.Find(_T("VMG=")) + 4))
            .BeforeFirst(';');
    double vmg;
    if (!s_vmg.ToDouble(&vmg)) {
      m_MarkDescription.Replace(_T("VMG=") + s_vmg + ";", wxEmptyString);
      SetPlannedSpeed(vmg);
    }
  }
  return m_PlannedSpeed;
}

wxDateTime RoutePoint::GetETD() {
  if (m_seg_etd.IsValid()) {
    if (!GetETA().IsValid() || m_seg_etd > GetETA()) {
      return m_seg_etd;
    } else {
      return GetETA();
    }
  } else {
    if (m_MarkDescription.Find(_T("ETD=")) != wxNOT_FOUND) {
      wxDateTime etd = wxInvalidDateTime;
      wxString s_etd =
          (m_MarkDescription.Mid(m_MarkDescription.Find(_T("ETD=")) + 4))
              .BeforeFirst(';');
      const wxChar *parse_return = etd.ParseDateTime(s_etd);
      if (parse_return) {
        wxString tz(parse_return);

        if (tz.Find(_T("UT")) != wxNOT_FOUND) {
          m_seg_etd = etd;
        } else {
          if (tz.Find(_T("LMT")) != wxNOT_FOUND) {
            m_seg_etd = etd;
            long lmt_offset = (long)((m_lon * 3600.) / 15.);
            wxTimeSpan lmt(0, 0, (int)lmt_offset, 0);
            m_seg_etd -= lmt;
          } else {
            m_seg_etd = etd.ToUTC();
          }
        }
        if (etd.IsValid() && (!GetETA().IsValid() || etd > GetETA())) {
          m_MarkDescription.Replace(s_etd, wxEmptyString);
          m_seg_etd = etd;
          return m_seg_etd;
        } else {
          return GetETA();
        }
      }
    }
  }
  return wxInvalidDateTime;
}

wxDateTime RoutePoint::GetManualETD() {
  if (m_manual_etd && m_seg_etd.IsValid()) {
    return m_seg_etd;
  }
  return wxInvalidDateTime;
}

wxDateTime RoutePoint::GetETA() {
  if (m_seg_eta.IsValid()) {
    return m_seg_eta;
  }
  return wxInvalidDateTime;
}

wxString RoutePoint::GetETE() {
  if (m_seg_ete != 0) {
    return formatTimeDelta(m_seg_ete);
  }
  return wxEmptyString;
}

void RoutePoint::SetETE(wxLongLong secs) { m_seg_ete = secs; }

void RoutePoint::SetETD(const wxDateTime &etd) {
  m_seg_etd = etd;
  m_manual_etd = TRUE;
}

bool RoutePoint::SetETD(const wxString &ts) {
  if (ts.IsEmpty()) {
    m_seg_etd = wxInvalidDateTime;
    m_manual_etd = false;
    return true;
  }
  wxDateTime tmp;
  wxString::const_iterator end;
  if (tmp.ParseISOCombined(ts)) {
    SetETD(tmp);
    return TRUE;
  } else if (tmp.ParseDateTime(ts, &end)) {
    SetETD(tmp);
    return TRUE;
  }
  return FALSE;
}
