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

WayPointman *pWayPointMan;
double g_defaultBoatSpeed;

#include <wx/listimpl.cpp>

WX_DEFINE_LIST(RouteList);

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
}

Route::~Route() {
  pRoutePointList->DeleteContents(false);  // do not delete Marks
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
  RoutePoint *point;
  wxRoutePointListNode *node = pRoutePointList->GetFirst();
  wxString substr = name.SubString(0, 6);

  while (node) {
    point = node->GetData();
    wxString exist = point->GetName().SubString(0, 6);

    if (pPoint->m_GUID == point->m_GUID) {
      node = node->GetNext();
    } else if (substr == exist) {
      return wxString("Name is not unique in route");
    } else {
      node = node->GetNext();
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
  pRoutePointList->Append(pNewPoint);

  if (!b_deferBoxCalc) FinalizeForRendering();

  if (prev) UpdateSegmentDistance(prev, pNewPoint);

  if (b_rename_in_sequence && pNewPoint->GetName().IsEmpty() &&
      !pNewPoint->IsShared()) {
    wxString name;
    name.Printf(_T("%03d"), GetnPoints());
    pNewPoint->SetName(name);
  }
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

    int insert = insert_after++;
    pNewPoint->m_bIsInRoute = true;
    pNewPoint->SetNameShown(false);
    pRoutePointList->Insert(insert, pNewPoint);
    if (bRenamePoints) RenameRoutePoints();
    m_lastMousePointIndex = GetnPoints();
    FinalizeForRendering();
    UpdateSegmentDistances();
    return;
  }
}

RoutePoint *Route::GetPoint(int nWhichPoint) {
  RoutePoint *prp;
  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  int i = 1;
  while (node) {
    prp = node->GetData();
    if (i == nWhichPoint) {
      return prp;
    }
    i++;
    node = node->GetNext();
  }

  return (NULL);
}

RoutePoint *Route::GetPoint(const wxString &guid) {
  RoutePoint *prp;
  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  while (node) {
    prp = node->GetData();
    if (guid == prp->m_GUID) return prp;

    node = node->GetNext();
  }

  return (NULL);
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
  for (wxRoutePointListNode *node = pRoutePointList->GetFirst(); node;
       node = node->GetNext()) {
    RoutePoint *prp = node->GetData();
    if (prp->IsShared()) return true;
  }
  return false;
}

// FIXME (leamas): can this be moved to GUI?
int s_arrow_icon[] = {0, 0, 5, 2, 18, 6, 12, 0, 18, -6, 5, -2, 0, 0};
void Route::ClearHighlights(void) {
  RoutePoint *prp = NULL;
  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  while (node) {
    prp = node->GetData();
    if (prp) prp->m_bPtIsSelected = false;
    node = node->GetNext();
  }
}

RoutePoint *Route::InsertPointBefore(RoutePoint *pRP, double rlat, double rlon,
                                     bool bRenamePoints) {
  RoutePoint *newpoint = new RoutePoint(rlat, rlon, g_default_routepoint_icon,
                                        GetNewMarkSequenced(), wxEmptyString);
  newpoint->m_bIsInRoute = true;
  newpoint->SetNameShown(false);

  int nRP = pRoutePointList->IndexOf(pRP);
  pRoutePointList->Insert(nRP, newpoint);

  if (bRenamePoints) RenameRoutePoints();

  FinalizeForRendering();
  UpdateSegmentDistances();

  return (newpoint);
}

RoutePoint *Route::InsertPointAfter(RoutePoint *pRP, double rlat, double rlon,
                                    bool bRenamePoints) {
  int nRP = pRoutePointList->IndexOf(pRP);
  if (nRP >= GetnPoints() - 1) return NULL;
  nRP++;

  RoutePoint *newpoint = new RoutePoint(rlat, rlon, g_default_routepoint_icon,
                                        GetNewMarkSequenced(), wxEmptyString);
  newpoint->m_bIsInRoute = true;
  newpoint->SetNameShown(false);

  pRoutePointList->Insert(nRP, newpoint);

  if (bRenamePoints) RenameRoutePoints();

  FinalizeForRendering();
  UpdateSegmentDistances();

  return (newpoint);
}

wxString Route::GetNewMarkSequenced(void) {
  wxString ret;
  ret.Printf(_T ( "NM%03d" ), m_nm_sequence);
  m_nm_sequence++;

  return ret;
}

RoutePoint *Route::GetLastPoint() {
  if (pRoutePointList->IsEmpty()) return NULL;

  return pRoutePointList->GetLast()->GetData();
}

int Route::GetIndexOf(RoutePoint *prp) {
  int ret = pRoutePointList->IndexOf(prp) + 1;
  if (ret == wxNOT_FOUND)
    return 0;
  else
    return ret;
}

void Route::DeletePoint(RoutePoint *rp, bool bRenamePoints) {
  //    n.b. must delete Selectables  and update config before deleting the
  //    point
  if (rp->m_bIsInLayer) return;

  pSelect->DeleteAllSelectableRoutePoints(this);
  pSelect->DeleteAllSelectableRouteSegments(this);
  NavObjectChanges::getInstance()->DeleteWayPoint(rp);

  pRoutePointList->DeleteObject(rp);

  delete rp;

  if (bRenamePoints) RenameRoutePoints();

  if (GetnPoints() > 1) {
    pSelect->AddAllSelectableRouteSegments(this);
    pSelect->AddAllSelectableRoutePoints(this);

    NavObjectChanges::getInstance()->UpdateRoute(this);

    FinalizeForRendering();
    UpdateSegmentDistances();
  }
}

void Route::RemovePoint(RoutePoint *rp, bool bRenamePoints) {
  if (rp->m_bIsActive && this->IsActive())  // FS#348
    g_pRouteMan->DeactivateRoute();

  pSelect->DeleteAllSelectableRoutePoints(this);
  pSelect->DeleteAllSelectableRouteSegments(this);

  pRoutePointList->DeleteObject(rp);

  // check all other routes to see if this point appears in any other route
  Route *pcontainer_route = FindRouteContainingWaypoint(rp);

  if (pcontainer_route == NULL) {
    rp->m_bIsInRoute = false;    // Take this point out of this (and only) route
    rp->m_bIsolatedMark = true;  // This has become an isolated mark
  }

  if (bRenamePoints) RenameRoutePoints();

  //      if ( m_nPoints > 1 )
  {
    pSelect->AddAllSelectableRouteSegments(this);
    pSelect->AddAllSelectableRoutePoints(this);

    NavObjectChanges::getInstance()->UpdateRoute(this);

    FinalizeForRendering();
    UpdateSegmentDistances();
  }
}

void Route::DeSelectRoute() {
  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  RoutePoint *rp;
  while (node) {
    rp = node->GetData();
    rp->m_bPtIsSelected = false;

    node = node->GetNext();
  }
}

void Route::ReloadRoutePointIcons() {
  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  RoutePoint *rp;
  while (node) {
    rp = node->GetData();
    rp->ReLoadIcon();

    node = node->GetNext();
  }
}

void Route::FinalizeForRendering() { RBBox.Invalidate(); }

LLBBox &Route::GetBBox(void) {
  if (RBBox.GetValid()) return RBBox;

  double bbox_lonmin, bbox_lonmax, bbox_latmin, bbox_latmax;

  wxRoutePointListNode *node = pRoutePointList->GetFirst();
  RoutePoint *data = node->GetData();

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

  node = node->GetNext();
  while (node) {
    data = node->GetData();

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
    node = node->GetNext();
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

  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  if (node) {
    //  Route start point
    RoutePoint *prp0 = node->GetData();
    if (!prp0->m_manual_etd) {
      prp0->m_seg_eta = m_PlannedDeparture;
      prp0->m_seg_etd = m_PlannedDeparture;
    }
    node = node->GetNext();

    while (node) {
      RoutePoint *prp = node->GetData();
      UpdateSegmentDistance(prp0, prp, planspeed);

      prp0 = prp;

      node = node->GetNext();
    }
  }
}

void Route::Reverse(bool bRenamePoints) {
  //    Reverse the GUID list
  wxArrayString RoutePointGUIDList;

  int ncount = pRoutePointList->GetCount();
  for (int i = 0; i < ncount; i++)
    RoutePointGUIDList.Add(GetPoint(ncount - i)->m_GUID);

  pRoutePointList->DeleteContents(false);
  pRoutePointList->Clear();
  m_route_length = 0.0;

  //    iterate over the RoutePointGUIDs
  for (unsigned int ip = 0; ip < RoutePointGUIDList.GetCount(); ip++) {
    wxString GUID = RoutePointGUIDList[ip];

    //    And on the RoutePoints themselves
    wxRoutePointListNode *prpnode = pWayPointMan->GetWaypointList()->GetFirst();
    while (prpnode) {
      RoutePoint *prp = prpnode->GetData();

      if (prp->m_GUID == GUID) {
        AddPoint(prp);
        break;
      }
      prpnode = prpnode->GetNext();  // RoutePoint
    }
  }

  if (bRenamePoints) RenameRoutePoints();

  // Switch start/end strings. anders, 2010-01-29
  wxString tmp = m_RouteStartString;
  m_RouteStartString = m_RouteEndString;
  m_RouteEndString = tmp;
}

void Route::SetVisible(bool visible, bool includeWpts) {
  m_bVisible = visible;

  if (!includeWpts) return;

  wxRoutePointListNode *node = pRoutePointList->GetFirst();
  RoutePoint *rp;
  while (node) {
    rp = node->GetData();

    // if this is a "shared" point, then do not turn off visibility.
    // This step keeps the point available for selection to other routes,
    // or may be manaully hidden in route-manager dialog.
    if (rp->IsShared()) {
      if (visible) rp->SetVisible(visible);
    }
    node = node->GetNext();
  }
}

void Route::SetListed(bool visible) { m_bListed = visible; }

void Route::AssembleRoute(void) {}

void Route::ShowWaypointNames(bool bshow) {
  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  while (node) {
    RoutePoint *prp = node->GetData();
    prp->SetNameShown(bshow);

    node = node->GetNext();
  }
}

bool Route::AreWaypointNamesVisible() {
  bool bvis = false;
  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  while (node) {
    RoutePoint *prp = node->GetData();
    if (prp->GetNameShown()) bvis = true;

    node = node->GetNext();
  }

  return bvis;
}

void Route::RenameRoutePoints(void) {
  //    iterate on the route points.
  //    If dynamically named, rename according to current list position

  wxRoutePointListNode *node = pRoutePointList->GetFirst();

  int i = 1;
  while (node) {
    RoutePoint *prp = node->GetData();
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

    node = node->GetNext();
    i++;
  }
}

//    Is this route equal to another, meaning,
//    Do all routepoint positions and names match?
bool Route::IsEqualTo(Route *ptargetroute) {
  wxRoutePointListNode *pthisnode = (this->pRoutePointList)->GetFirst();
  wxRoutePointListNode *pthatnode = (ptargetroute->pRoutePointList)->GetFirst();

  if (NULL == pthisnode) return false;

  if (this->m_bIsInLayer || ptargetroute->m_bIsInLayer) return false;

  if (this->GetnPoints() != ptargetroute->GetnPoints()) return false;

  while (pthisnode) {
    if (NULL == pthatnode) return false;

    RoutePoint *pthisrp = pthisnode->GetData();
    RoutePoint *pthatrp = pthatnode->GetData();

    if ((fabs(pthisrp->m_lat - pthatrp->m_lat) > 1.0e-6) ||
        (fabs(pthisrp->m_lon - pthatrp->m_lon) > 1.0e-6))
      return false;

    if (!pthisrp->GetName().IsSameAs(pthatrp->GetName())) return false;

    pthisnode = pthisnode->GetNext();
    pthatnode = pthatnode->GetNext();
  }

  return true;  // success, they are the same
}
