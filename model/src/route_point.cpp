/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Route Point Object
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

#include <wx/colour.h>
#include <wx/datetime.h>
#include <wx/dynarray.h>
#include <wx/string.h>
#include <wx/tokenzr.h>

#include "model/base_platform.h"
#include "model/cutil.h"
#include "model/georef.h"
#include "model/navutil_base.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/route_point.h"
#include "model/select.h"

#include <wx/listimpl.cpp>

WX_DEFINE_LIST(RoutePointList);

wxColour g_colourWaypointRangeRingsColour;

int g_LayerIdx;

wxRect g_blink_rect;

std::function<void(unsigned, const unsigned *)> RoutePoint::delete_gl_textures =
    [](unsigned, const unsigned *) { assert(true); };

RoutePoint::RoutePoint() {
  m_pbmIcon = NULL;

  //  Nice defaults
  m_seg_len = 0.0;
  m_seg_vmg = 0.0;

  m_seg_etd = wxInvalidDateTime;
  m_manual_etd = false;

  m_seg_eta = wxInvalidDateTime;
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
  m_bShowName = g_bShowWptName;
  m_ScaMax = 0;
  b_UseScamin = g_bUseWptScaMin;

  m_pos_on_screen = false;
  m_bDrawDragHandle = false;
  m_dragIconTexture = 0;
  m_draggingOffsetx = m_draggingOffsety = 0;

  m_PlannedSpeed = 0.;
  m_IconIsDirty = true;
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
  m_IconIsDirty = orig->m_IconIsDirty;

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
  m_IconIsDirty = true;

  m_iTextTexture = 0;

  if (!pGUID.IsEmpty())
    m_GUID = pGUID;
  else
    m_GUID = pWayPointMan->CreateGUID(this);

  //      Get Icon bitmap
  m_IconName = icon_ident;

  SetName(name);

  //  Possibly add the waypoint to the global list maintained by the waypoint
  //  manager

  if (bAddToList && NULL != pWayPointMan) pWayPointMan->AddRoutePoint(this);

  m_bIsInLayer = false;
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
  m_bShowName = g_bShowWptName;

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
  RoutePoint::delete_gl_textures(1, &m_dragIconTexture);
}

wxDateTime RoutePoint::GetCreateTime() {
  if (!m_CreateTimeX.IsValid()) {
    if (m_timestring.Len()) ParseGPXDateTime(m_CreateTimeX, m_timestring);
  }
  return m_CreateTimeX;
}

void RoutePoint::SetCreateTime(wxDateTime dt) { m_CreateTimeX = dt; }

void RoutePoint::SetName(const wxString &name) {
  if (m_iTextTexture) {
    RoutePoint::delete_gl_textures(1, &m_iTextTexture);
    m_iTextTexture = 0;
  }
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

bool RoutePoint::IsVisibleSelectable(double scale_val, bool boverrideViz) {
  if (m_bIsActive)  //  An active route point must always be visible
    return true;

  if (!boverrideViz) {
    if (!m_bIsVisible)  // if not visible nevermind the rest.
      return false;
  }

  if (b_UseScamin) {
    if (g_bOverruleScaMin)
      return true;
    else if (scale_val >= (double)(m_ScaMin + 1))
      return false;
  }
  return true;
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

void RoutePoint::SetPosition(double lat, double lon) {
  m_lat = lat;
  m_lon = lon;
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

/*!
 * Check if the name is dynamic for resequencing purposes.
 * If the name is part of a route, and has 3 numeric characters,
 * then it is dynamic and can be resequenced.
 */
bool RoutePoint::IsNameDynamic() {
  bool b_numeric = false;
  if (m_bIsInRoute) {
    if (GetName().Len() >= 2) {
      wxString substring = GetName().Left(2);
      if (substring == "NM") {
        substring = GetName().substr(2, 3);
      } else {
        substring = GetName().Left(3);
      }
      b_numeric = true;  // assume it is numeric
      for (unsigned int i = 0; i < substring.Len(); i++) {
        if (b_numeric == true) {
          b_numeric = wxIsdigit(substring[i]);
        }  // don't change the value if it is already false
      }
    }
  }
  return b_numeric;
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
