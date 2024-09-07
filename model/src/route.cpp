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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <wx/arrstr.h>
#include <wx/datetime.h>
#include <wx/gdicmn.h>
#include <wx/log.h>
#include <wx/pen.h>
#include <wx/string.h>

#include "model/config_vars.h"
#include "model/cutil.h"
#include "model/georef.h"
#include "model/georef.h"
#include "model/config_vars.h"
#include "model/nav_object_database.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/navobj_db.h"

WayPointman *pWayPointMan;

#include <wx/listimpl.cpp>

Route::Route() {
  m_bRtIsSelected = false;
  m_bRtIsActive = false;
  m_pRouteActivePoint = NULL;
  m_bIsBeingEdited = false;
  m_bIsBeingCreated = false;
  m_nm_sequence = 1;
  m_route_length = 0.0;
  m_route_time = 0.0;
  m_bVisible = true;
  m_bListed = true;
  m_bDeleteOnArrival = false;
  m_width = WIDTH_UNDEFINED;
  m_style = wxPENSTYLE_INVALID;
  m_hiliteWidth = 0;

  pRoutePointList = new RoutePointList;
  m_GUID = pWayPointMan->CreateGUID(NULL);
  m_btemp = false;

  m_ArrivalRadius = g_n_arrival_circle_radius;  // Nautical Miles

  m_LayerID = 0;
  m_bIsInLayer = false;

  m_Colour = wxEmptyString;

  m_lastMousePointIndex = 0;
  m_NextLegGreatCircle = false;

  m_PlannedSpeed = ROUTE_DEFAULT_SPEED;
  if (g_defaultBoatSpeed != ROUTE_DEFAULT_SPEED)
    m_PlannedSpeed = g_defaultBoatSpeed;

  m_PlannedDeparture = RTE_UNDEF_DEPARTURE;
  m_TimeDisplayFormat = RTE_TIME_DISP_PC;
  m_HyperlinkList = new HyperlinkList;

  m_bsharedWPViz = false;

  // Initialize position cache
  m_cachedTimestamp = wxInvalidDateTime;
  m_cachedLat = 0.0;
  m_cachedLon = 0.0;
  m_cacheValid = false;
}

Route::~Route() {
  delete pRoutePointList;
  delete m_HyperlinkList;
}

// The following is used only for route splitting, assumes just created, empty
// route
//
void Route::CloneRoute(Route *psourceroute, int start_nPoint, int end_nPoint,
                       const wxString &suffix,
                       const bool duplicate_first_point) {
  m_RouteNameString = psourceroute->m_RouteNameString + suffix;
  m_RouteStartString = psourceroute->m_RouteStartString;
  m_RouteEndString = psourceroute->m_RouteEndString;

  int i;
  for (i = start_nPoint; i <= end_nPoint; i++) {
    if (!psourceroute->m_bIsInLayer &&
        !(i == start_nPoint && duplicate_first_point)) {
      AddPoint(psourceroute->GetPoint(i), false);
    } else {
      RoutePoint *psourcepoint = psourceroute->GetPoint(i);
      RoutePoint *ptargetpoint = new RoutePoint(
          psourcepoint->m_lat, psourcepoint->m_lon, psourcepoint->GetIconName(),
          psourcepoint->GetName(), wxEmptyString, true);
      ptargetpoint->m_bShowName =
          psourcepoint->m_bShowName;  // do not change new wpt's name visibility
      AddPoint(ptargetpoint, false);
    }
  }

  FinalizeForRendering();
}

wxString Route::IsPointNameValid(RoutePoint *pPoint,
                                 const wxString &name) const {
  wxString substr = name;
  auto it = pRoutePointList->begin();
  while (it != pRoutePointList->end()) {
    RoutePoint *point = *it;
    wxString exist = point->GetName();

    if (pPoint->m_GUID == point->m_GUID) {
      ++it;
    } else if (substr == exist) {
      return _("Name is not unique in route");
    } else {
      ++it;
    }
  }
  return wxEmptyString;
}

void Route::AddPoint(RoutePoint *pNewPoint, bool b_rename_in_sequence,
                     bool b_deferBoxCalc) {
  if (pNewPoint->m_bIsolatedMark) {
    pNewPoint->SetShared(true);
  }
  pNewPoint->m_bIsolatedMark = false;  // definitely no longer isolated
  pNewPoint->m_bIsInRoute = true;

  RoutePoint *prev = GetLastPoint();
  pRoutePointList->push_back(pNewPoint);

  if (!b_deferBoxCalc) FinalizeForRendering();

  if (prev) UpdateSegmentDistance(prev, pNewPoint);

  if (b_rename_in_sequence && pNewPoint->GetName().IsEmpty() &&
      !pNewPoint->IsShared()) {
    wxString name;
    name.Printf(_T("%03d"), GetnPoints());
    pNewPoint->SetName(name);
  }

  // Invalidate position cache since route structure changed
  InvalidatePositionCache();

  return;
}

void Route::AddPointAndSegment(RoutePoint *pNewPoint, bool b_rename_in_sequence,
                               bool b_deferBoxCalc) {
  int npoints = GetnPoints();
  RoutePoint *newpoint = pNewPoint;
  if (newpoint->m_bIsInLayer) {
    newpoint = new RoutePoint(pNewPoint->m_lat, pNewPoint->m_lon,
                              pNewPoint->GetIconName(), pNewPoint->GetName(),
                              wxEmptyString, false);
    newpoint->m_bShowName =
        pNewPoint->m_bShowName;  // do not change new wpt's name visibility
  }
  AddPoint(newpoint, false);
  if (npoints != 0) {
    double rlat = GetPoint(npoints)->m_lat;
    double rlon = GetPoint(npoints)->m_lon;
    npoints = GetnPoints();
    pSelect->AddSelectableRouteSegment(
        rlat, rlon, GetPoint(npoints)->m_lat, GetPoint(npoints)->m_lon,
        GetPoint(npoints - 1), GetPoint(npoints), this);
  }
  m_lastMousePointIndex = GetnPoints();
}

void Route::InsertPointAndSegment(RoutePoint *pNewPoint, int insert_after,
                                  bool bRenamePoints, bool b_deferBoxCalc) {
  {
    bool add = false;

    if (pNewPoint->m_bIsolatedMark) {
      pNewPoint->SetShared(true);
    }
    pNewPoint->m_bIsolatedMark = false;  // definitely no longer isolated
    pNewPoint->m_bIsInRoute = true;

    if (insert_after >= GetnPoints() - 1) {
      wxLogMessage(wxT("Error insert after last point"));
      return;
    }

    auto pos = pRoutePointList->begin() + insert_after + 1;
    pNewPoint->m_bIsInRoute = true;
    pNewPoint->SetNameShown(false);
    pRoutePointList->insert(pos, pNewPoint);
    if (bRenamePoints) RenameRoutePoints();
    m_lastMousePointIndex = GetnPoints();
    FinalizeForRendering();
    UpdateSegmentDistances();

    // Invalidate position cache since route structure changed
    InvalidatePositionCache();

    return;
  }
}

RoutePoint *Route::GetPoint(int nWhichPoint) {
  // nWhichPoint is 1-based.
  if (nWhichPoint < 1) return nullptr;
  if (nWhichPoint > static_cast<int>(pRoutePointList->size())) return nullptr;
  auto pos = pRoutePointList->begin() + nWhichPoint - 1;
  return *pos;
}

RoutePoint *Route::GetPoint(const wxString &guid) {
  for (RoutePoint *prp : *pRoutePointList) {
    if (guid == prp->m_GUID) return prp;
  }
  return (nullptr);
}

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

bool Route::ContainsSharedWP() {
  for (RoutePoint *prp : *pRoutePointList) {
    if (prp->IsShared()) return true;
  }
  return false;
}

// FIXME (leamas): can this be moved to GUI?
int s_arrow_icon[] = {0, 0, 5, 2, 18, 6, 12, 0, 18, -6, 5, -2, 0, 0};

void Route::ClearHighlights() {
  for (RoutePoint *prp : *pRoutePointList) {
    if (prp) prp->m_bPtIsSelected = false;
  }
}

RoutePoint *Route::InsertPointBefore(RoutePoint *pRP, double rlat, double rlon,
                                     bool bRenamePoints) {
  RoutePoint *newpoint = new RoutePoint(rlat, rlon, g_default_routepoint_icon,
                                        GetNewMarkSequenced(), wxEmptyString);
  newpoint->m_bIsInRoute = true;
  newpoint->SetNameShown(false);

  auto pos = std::find(pRoutePointList->begin(), pRoutePointList->end(), pRP);
  if (pos == pRoutePointList->end()) pos = pRoutePointList->begin();
  pRoutePointList->insert(pos, newpoint);

  if (bRenamePoints) RenameRoutePoints();

  FinalizeForRendering();
  UpdateSegmentDistances();

  // Invalidate position cache since route structure changed
  InvalidatePositionCache();

  return (newpoint);
}

RoutePoint *Route::InsertPointAfter(RoutePoint *pRP, double rlat, double rlon,
                                    bool bRenamePoints) {
  auto pos = std::find(pRoutePointList->begin(), pRoutePointList->end(), pRP);
  if (pos == pRoutePointList->end()) return nullptr;
  ++pos;

  RoutePoint *newpoint = new RoutePoint(rlat, rlon, g_default_routepoint_icon,
                                        GetNewMarkSequenced(), wxEmptyString);
  newpoint->m_bIsInRoute = true;
  newpoint->SetNameShown(false);

  pRoutePointList->insert(pos, newpoint);

  if (bRenamePoints) RenameRoutePoints();

  FinalizeForRendering();
  UpdateSegmentDistances();

  // Invalidate position cache since route structure changed
  InvalidatePositionCache();

  return (newpoint);
}

wxString Route::GetNewMarkSequenced(void) {
  wxString ret;
  ret.Printf(_T ( "NM%03d" ), m_nm_sequence);
  m_nm_sequence++;

  return ret;
}

RoutePoint *Route::GetLastPoint() {
  if (pRoutePointList->empty()) return nullptr;
  return *(pRoutePointList->end() - 1);
}

int Route::GetIndexOf(RoutePoint *prp) {
  auto pos = std::find(pRoutePointList->begin(), pRoutePointList->end(), prp);

  if (pos == pRoutePointList->end()) return 0;
  return static_cast<int>(pos - pRoutePointList->begin());
}

void Route::DeletePoint(RoutePoint *rp, bool bRenamePoints) {
  //    n.b. must delete Selectables  and update config before deleting the
  //    point
  if (rp->m_bIsInLayer) return;

  pSelect->DeleteAllSelectableRoutePoints(this);
  pSelect->DeleteAllSelectableRouteSegments(this);
  auto pos = std::find(pRoutePointList->begin(), pRoutePointList->end(), rp);
  if (pos != pRoutePointList->end()) pRoutePointList->erase(pos);
  delete rp;

  if (bRenamePoints) RenameRoutePoints();

  if (GetnPoints() > 1) {
    pSelect->AddAllSelectableRouteSegments(this);
    pSelect->AddAllSelectableRoutePoints(this);

    FinalizeForRendering();
    UpdateSegmentDistances();
  }

  // Invalidate position cache since route structure changed
  InvalidatePositionCache();
}

void Route::RemovePoint(RoutePoint *rp, bool bRenamePoints) {
  if (rp->m_bIsActive && this->IsActive())  // FS#348
    g_pRouteMan->DeactivateRoute();

  pSelect->DeleteAllSelectableRoutePoints(this);
  pSelect->DeleteAllSelectableRouteSegments(this);

  // Arrange to remove all references to the same routepoint
  // within the route.  This can happen with circular or "round-trip" routes.
  auto pos = std::find(pRoutePointList->begin(), pRoutePointList->end(), rp);
  while (pos != pRoutePointList->end()) {
    pRoutePointList->erase(pos);
    pos = std::find(pRoutePointList->begin(), pRoutePointList->end(), rp);
  }

  // check all other routes to see if this point appears in any other route
  Route *pcontainer_route = FindRouteContainingWaypoint(rp);

  if (pcontainer_route == NULL) {
    rp->m_bIsInRoute = false;    // Take this point out of this (and only) route
    rp->m_bIsolatedMark = true;  // This has become an isolated mark
    NavObj_dB::GetInstance().UpdateRoutePoint(rp);
  }

  if (bRenamePoints) RenameRoutePoints();

  //      if ( m_nPoints > 1 )
  {
    pSelect->AddAllSelectableRouteSegments(this);
    pSelect->AddAllSelectableRoutePoints(this);

    // NavObjectChanges::getInstance()->UpdateRoute(this);
    NavObj_dB::GetInstance().UpdateRoute(this);
    FinalizeForRendering();
    UpdateSegmentDistances();
  }

  // Invalidate position cache since route structure changed
  InvalidatePositionCache();
}

void Route::DeSelectRoute() {
  for (RoutePoint *rp : *pRoutePointList) {
    rp->m_bPtIsSelected = false;
  }
}

void Route::ReloadRoutePointIcons() {
  for (RoutePoint *rp : *pRoutePointList) {
    rp->ReLoadIcon();
  }
}

void Route::FinalizeForRendering() { RBBox.Invalidate(); }

LLBBox &Route::GetBBox(void) {
  if (RBBox.GetValid()) return RBBox;

  double bbox_lonmin, bbox_lonmax, bbox_latmin, bbox_latmax;

  // wxRoutePointListNode *node = pRoutePointList->GetFirst();
  auto it = pRoutePointList->begin();
  RoutePoint *data = *it;

  if (data->m_wpBBox.GetValid()) {
    bbox_lonmax = data->m_wpBBox.GetMaxLon();
    bbox_lonmin = data->m_wpBBox.GetMinLon();
    bbox_latmax = data->m_wpBBox.GetMaxLat();
    bbox_latmin = data->m_wpBBox.GetMinLat();
  } else {
    bbox_lonmax = bbox_lonmin = data->m_lon;
    bbox_latmax = bbox_latmin = data->m_lat;
  }

  double lastlon = data->m_lon, wrap = 0;
  for (++it; it != pRoutePointList->end(); ++it) {
    data = *it;

    if (lastlon - data->m_lon > 180)
      wrap += 360;
    else if (data->m_lon - lastlon > 180)
      wrap -= 360;

    double lon = data->m_lon + wrap;

    if (lon > bbox_lonmax) bbox_lonmax = lon;
    if (lon < bbox_lonmin) bbox_lonmin = lon;

    if (data->m_lat > bbox_latmax) bbox_latmax = data->m_lat;
    if (data->m_lat < bbox_latmin) bbox_latmin = data->m_lat;

    lastlon = data->m_lon;
  }

  if (bbox_lonmin < -360)
    bbox_lonmin += 360, bbox_lonmax += 360;
  else if (bbox_lonmax > 360)
    bbox_lonmin -= 360, bbox_lonmax -= 360;

  if (bbox_lonmax - bbox_lonmin > 360) bbox_lonmin = -180, bbox_lonmax = 180;

  RBBox.Set(bbox_latmin, bbox_lonmin, bbox_latmax, bbox_lonmax);

  return RBBox;
}

/*
 Update a single route segment lengths
 Also, compute total route length by summing segment distances.
 */
void Route::UpdateSegmentDistance(RoutePoint *prp0, RoutePoint *prp,
                                  double planspeed) {
  double slat1 = prp0->m_lat, slon1 = prp0->m_lon;
  double slat2 = prp->m_lat, slon2 = prp->m_lon;

  //    Calculate the absolute distance from 1->2

  double dd;
  double br;
  // why are we using mercator rather than great circle here?? [sean 8-11-2015]
  DistanceBearingMercator(slat2, slon2, slat1, slon1, &br, &dd);

  prp->SetCourse(br);
  prp->SetDistance(dd);

  //    And store in Point 2
  prp->m_seg_len = dd;

  m_route_length += dd;

  //    If Point1 Description contains VMG, store it for Properties Dialog in
  //    Point2 If Point1 Description contains ETD, store it in Point1

  if (planspeed > 0.) {
    wxDateTime etd;

    double legspeed = planspeed;
    if (prp->GetPlannedSpeed() > 0.1 && prp->GetPlannedSpeed() < 1000.)
      legspeed = prp->GetPlannedSpeed();
    if (legspeed > 0.1 && legspeed < 1000.) {
      m_route_time += 3600. * dd / legspeed;
      prp->m_seg_vmg = legspeed;
    }
    wxLongLong duration = wxLongLong(3600.0 * prp->m_seg_len / prp->m_seg_vmg);
    prp->SetETE(duration);
    wxTimeSpan ts(0, 0, duration);
    if (!prp0->GetManualETD().IsValid()) {
      prp0->m_manual_etd = false;
      if (prp0->GetETA().IsValid()) {
        prp0->m_seg_etd = prp0->GetETA();
      } else {
        prp0->m_seg_etd =
            m_PlannedDeparture + wxTimeSpan(0, 0, m_route_time - duration);
      }
    }

    prp->m_seg_eta = prp0->GetETD() + ts;
    if (!prp->m_manual_etd || !prp->GetETD().IsValid()) {
      prp->m_seg_etd = prp->m_seg_eta;
      prp->m_manual_etd = false;
    }
  }
}

/*
 Update the route segment lengths, storing each segment length in <destination>
 point. Also, compute total route length by summing segment distances.
 */
void Route::UpdateSegmentDistances(double planspeed) {
  wxPoint rpt, rptn;

  m_route_length = 0.0;
  m_route_time = 0.0;

  // wxRoutePointListNode *node = pRoutePointList->GetFirst();
  auto it = pRoutePointList->begin();

  if (it != pRoutePointList->end()) {
    //  Route start point
    RoutePoint *prp0 = *it;
    if (!prp0->m_manual_etd) {
      prp0->m_seg_eta = m_PlannedDeparture;
      prp0->m_seg_etd = m_PlannedDeparture;
    }
    for (++it; it != pRoutePointList->end(); ++it) {
      RoutePoint *prp = *it;
      UpdateSegmentDistance(prp0, prp, planspeed);

      prp0 = prp;
    }
  }

  // Invalidate position cache since timing changed
  InvalidatePositionCache();
}

void Route::Reverse(bool bRenamePoints) {
  //    Reverse the GUID list
  wxArrayString RoutePointGUIDList;

  int ncount = pRoutePointList->size();
  for (int i = 0; i < ncount; i++)
    RoutePointGUIDList.Add(GetPoint(ncount - i)->m_GUID);

  pRoutePointList->clear();
  m_route_length = 0.0;

  //  Iterate over the RoutePointGUIDs
  for (unsigned int ip = 0; ip < RoutePointGUIDList.GetCount(); ip++) {
    wxString GUID = RoutePointGUIDList[ip];
    for (RoutePoint *prp : *pWayPointMan->GetWaypointList()) {
      if (prp->m_GUID == GUID) {
        AddPoint(prp);
        break;
      }
    }
  }

  if (bRenamePoints) RenameRoutePoints();

  // Switch start/end strings. anders, 2010-01-29
  wxString tmp = m_RouteStartString;
  m_RouteStartString = m_RouteEndString;
  m_RouteEndString = tmp;

  // Invalidate position cache since route structure changed
  InvalidatePositionCache();
}

void Route::SetVisible(bool visible, bool includeWpts) {
  m_bVisible = visible;

  if (!includeWpts) return;

  for (RoutePoint *rp : *pWayPointMan->GetWaypointList()) {
    // if this is a "shared" point, then do not turn off visibility.
    // This step keeps the point available for selection to other routes,
    // or may be manaully hidden in route-manager dialog.
    if (rp->IsShared()) {
      if (visible) rp->SetVisible(visible);
    }
  }
}

void Route::SetListed(bool visible) { m_bListed = visible; }

void Route::AssembleRoute(void) {}

void Route::ShowWaypointNames(bool bshow) {
  for (RoutePoint *prp : *pRoutePointList) {
    prp->SetNameShown(bshow);
  }
}

bool Route::AreWaypointNamesVisible() {
  bool bvis = false;
  for (RoutePoint *prp : *pRoutePointList) {
    if (prp->GetNameShown()) bvis = true;
  }
  return bvis;
}

void Route::RenameRoutePoints(void) {
  //    iterate on the route points.
  //    If dynamically named, rename according to current list position

  int i = 1;
  for (RoutePoint *prp : *pRoutePointList) {
    if (prp->IsNameDynamic()) {
      wxString name = prp->GetName();
      if (name.Len() == 3) {
        name.Printf(_T ( "%03d" ), i);
      } else if (name.Left(2) == "NM") {
        name.Printf(_T ( "%03d" ), i);
        if (prp->GetName().Len() >= 5) {
          name.Append(prp->GetName().Mid(5));
        }
      } else {
        name.Printf(_T ( "%03d" ), i);
        name.Append(prp->GetName().Mid(3));
      }
      prp->SetName(name);
    }
    i++;
  }
}

//    Is this route equal to another, meaning,
//    Do all routepoint positions and names match?
bool Route::IsEqualTo(Route *ptargetroute) {
  auto pthisnode = (this->pRoutePointList)->begin();
  auto pthatnode = (ptargetroute->pRoutePointList)->begin();

  if (pthisnode == this->pRoutePointList->end()) return false;

  if (this->m_bIsInLayer || ptargetroute->m_bIsInLayer) return false;

  if (this->GetnPoints() != ptargetroute->GetnPoints()) return false;

  while (pthisnode != this->pRoutePointList->end()) {
    if (pthatnode == ptargetroute->pRoutePointList->end()) return false;

    RoutePoint *pthisrp = *pthisnode;
    RoutePoint *pthatrp = *pthatnode;

    if ((fabs(pthisrp->m_lat - pthatrp->m_lat) > 1.0e-6) ||
        (fabs(pthisrp->m_lon - pthatrp->m_lon) > 1.0e-6))
      return false;

    if (!pthisrp->GetName().IsSameAs(pthatrp->GetName())) return false;

    ++pthisnode;
    ++pthatnode;
  }

  return true;  // success, they are the same
}

bool Route::GetPositionAtTime(const wxDateTime &timestamp, double &lat,
                              double &lon) const {
  // Check cache first
  if (m_cacheValid && timestamp.IsValid() && m_cachedTimestamp.IsValid() &&
      timestamp == m_cachedTimestamp) {
    // This function is called during every chart canvas paint operation
    // from the rendering pipeline to draw the vessel's interpolated position
    // along the route at the current timeline timestamp.
    lat = m_cachedLat;
    lon = m_cachedLon;
    return true;
  }

  // Validate inputs
  if (!timestamp.IsValid() || !m_PlannedDeparture.IsValid()) {
    return false;
  }

  // Check if timestamp is before route start
  if (timestamp < m_PlannedDeparture) {
    return false;
  }

  // Handle empty route
  if (pRoutePointList->empty()) {
    return false;
  }

  // Single waypoint case
  if (pRoutePointList->size() == 1) {
    RoutePoint *rp = pRoutePointList->front();
    lat = rp->m_lat;
    lon = rp->m_lon;

    // Cache the result
    m_cachedTimestamp = timestamp;
    m_cachedLat = lat;
    m_cachedLon = lon;
    m_cacheValid = true;

    return true;
  }

  // Find the appropriate segment
  for (size_t i = 0; i < pRoutePointList->size() - 1; i++) {
    RoutePoint *prevPoint = (*pRoutePointList)[i];
    RoutePoint *currPoint = (*pRoutePointList)[i + 1];

    // Check if timestamp falls within this segment
    if (timestamp >= prevPoint->GetETD() && timestamp <= currPoint->GetETA()) {
      // Calculate time elapsed in this segment
      wxTimeSpan segmentDuration = currPoint->GetETA() - prevPoint->GetETD();
      wxTimeSpan elapsedTime = timestamp - prevPoint->GetETD();

      // Avoid division by zero
      if (segmentDuration.GetSeconds() == 0) {
        lat = currPoint->m_lat;
        lon = currPoint->m_lon;

        // Cache the result
        m_cachedTimestamp = timestamp;
        m_cachedLat = lat;
        m_cachedLon = lon;
        m_cacheValid = true;

        return true;
      }

      // Calculate interpolation factor (0.0 = at prevPoint, 1.0 = at currPoint)
      double factor = (double)elapsedTime.GetSeconds().ToDouble() /
                      (double)segmentDuration.GetSeconds().ToDouble();

      // Clamp factor to [0, 1]
      factor = wxMax(0.0, wxMin(1.0, factor));

      // Use rhumb line interpolation to match OpenCPN's navigation behavior
      // Calculate bearing and distance from previous to current waypoint.
      double bearing, totalDistance;
      DistanceBearingMercator(currPoint->m_lat, currPoint->m_lon,
                              prevPoint->m_lat, prevPoint->m_lon, &bearing,
                              &totalDistance);

      // Calculate distance along rhumb line based on time elapsed.
      double distanceAlongSegment = factor * totalDistance;

      // Calculate position along the rhumb line bearing.
      PositionBearingDistanceLoxodrome(prevPoint->m_lat, prevPoint->m_lon,
                                       bearing, distanceAlongSegment, &lat,
                                       &lon);

      // Cache the result
      m_cachedTimestamp = timestamp;
      m_cachedLat = lat;
      m_cachedLon = lon;
      m_cacheValid = true;

      return true;
    }

    // Check if we're past the route end time
    if (timestamp > currPoint->GetETA() && i == pRoutePointList->size() - 2) {
      // Return the last waypoint position
      lat = currPoint->m_lat;
      lon = currPoint->m_lon;

      // Cache the result
      m_cachedTimestamp = timestamp;
      m_cachedLat = lat;
      m_cachedLon = lon;
      m_cacheValid = true;

      return true;
    }
  }

  // If we get here, timestamp might be after the route ends
  RoutePoint *lastPoint = pRoutePointList->back();
  lat = lastPoint->m_lat;
  lon = lastPoint->m_lon;

  // Cache the result
  m_cachedTimestamp = timestamp;
  m_cachedLat = lat;
  m_cachedLon = lon;
  m_cacheValid = true;

  return true;
}
wxDateTime Route::GetTimeAtPosition(double targetLat, double targetLon,
                                    double *actualLat,
                                    double *actualLon) const {
  // Validate route has timing information
  if (!m_PlannedDeparture.IsValid() || pRoutePointList->empty()) {
    return wxInvalidDateTime;
  }

  // Single waypoint case
  if (pRoutePointList->size() == 1) {
    RoutePoint *rp = pRoutePointList->front();
    if (actualLat) *actualLat = rp->m_lat;
    if (actualLon) *actualLon = rp->m_lon;
    return m_PlannedDeparture;
  }

  // Find the closest point on the route to the target position
  double minDistance = 1e10;
  RoutePoint *closestPrevPoint = nullptr;
  RoutePoint *closestNextPoint = nullptr;
  double closestPointLat = 0.0;
  double closestPointLon = 0.0;

  for (size_t i = 0; i < pRoutePointList->size() - 1; i++) {
    RoutePoint *prevPoint = (*pRoutePointList)[i];
    RoutePoint *currPoint = (*pRoutePointList)[i + 1];

    // Find the closest point on this segment to the target position
    double segLat, segLon;
    double distToSegment = DistanceToSegment(
        targetLat, targetLon, prevPoint->m_lat, prevPoint->m_lon,
        currPoint->m_lat, currPoint->m_lon, &segLat, &segLon);

    if (distToSegment < minDistance) {
      minDistance = distToSegment;
      closestPrevPoint = prevPoint;
      closestNextPoint = currPoint;
      closestPointLat = segLat;
      closestPointLon = segLon;
    }
  }

  // If no valid segment found, return invalid time
  if (!closestPrevPoint || !closestNextPoint) {
    return wxInvalidDateTime;
  }

  // Check if route points have valid timing information
  wxDateTime prevETD = closestPrevPoint->GetETD();
  wxDateTime nextETA = closestNextPoint->GetETA();

  if (!prevETD.IsValid() || !nextETA.IsValid()) {
    // If timing information is not available, we can't calculate the time
    // The route should have UpdateSegmentDistances() called to set up timing
    return wxInvalidDateTime;
  }

  // Calculate the ratio of how far along the segment the closest point is
  double segmentBearing, segmentDistance;
  DistanceBearingMercator(closestNextPoint->m_lat, closestNextPoint->m_lon,
                          closestPrevPoint->m_lat, closestPrevPoint->m_lon,
                          &segmentBearing, &segmentDistance);

  double distanceFromStart, unused;
  DistanceBearingMercator(closestPointLat, closestPointLon,
                          closestPrevPoint->m_lat, closestPrevPoint->m_lon,
                          &unused, &distanceFromStart);

  // Calculate interpolation factor (0.0 = at prevPoint, 1.0 = at currPoint)
  double factor = 0.0;
  if (segmentDistance > 0.0) {
    factor = distanceFromStart / segmentDistance;
    factor = wxMax(0.0, wxMin(1.0, factor));
  }

  // Set the actual coordinates of the closest point on the route
  if (actualLat) *actualLat = closestPointLat;
  if (actualLon) *actualLon = closestPointLon;

  // Interpolate the time
  wxTimeSpan segmentDuration = nextETA - prevETD;
  wxTimeSpan elapsedTime = wxTimeSpan::Seconds(
      (long)(factor * segmentDuration.GetSeconds().ToDouble()));

  wxDateTime result = prevETD + elapsedTime;
  return result;
}

// Helper function to calculate distance from a point to a line segment
double Route::DistanceToSegment(double pointLat, double pointLon,
                                double segStartLat, double segStartLon,
                                double segEndLat, double segEndLon,
                                double *closestLat, double *closestLon) const {
  // Calculate the deltas for the segment
  double dLat = segEndLat - segStartLat;
  double dLon = segEndLon - segStartLon;

  if (dLat == 0.0 && dLon == 0.0) {
    // Segment is actually a point
    *closestLat = segStartLat;
    *closestLon = segStartLon;
    double dist;
    DistanceBearingMercator(pointLat, pointLon, segStartLat, segStartLon,
                            nullptr, &dist);
    return dist;
  }

  // Calculate parameter t for the closest point on the line
  double t =
      ((pointLat - segStartLat) * dLat + (pointLon - segStartLon) * dLon) /
      (dLat * dLat + dLon * dLon);

  // Clamp t to the segment [0,1]
  t = wxMax(0.0, wxMin(1.0, t));

  // Calculate the closest point on the segment
  *closestLat = segStartLat + t * dLat;
  *closestLon = segStartLon + t * dLon;

  // Return distance using Mercator calculation
  double distance;
  DistanceBearingMercator(pointLat, pointLon, *closestLat, *closestLon, nullptr,
                          &distance);
  return distance;
}

void Route::InvalidatePositionCache() const { m_cacheValid = false; }
