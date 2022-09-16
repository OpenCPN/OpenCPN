/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Route Manager
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <wx/apptrait.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/jsonval.h>
#include <wx/listimpl.cpp>
#include <wx/progdlg.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>

#include "ais_decoder.h"
#include "base_platform.h"
#include "chcanv.h"
#include "comm_n0183_output.h"
#include "concanv.h"
#include "cutil.h"
#include "dychart.h"
#include "georef.h"
#include "MarkIcon.h"
#include "NavObjectCollection.h"
#include "navutil_base.h"
#include "navutil.h"
#include "ocpn_app.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "pluginmanager.h"
#include "Route.h"
#include "routemanagerdialog.h"
#include "routeman.h"
#include "RoutePropDlgImpl.h"
#include "styles.h"
#include "svg_utils.h"
#include "Track.h"

#ifndef CLIAPP
#include "color_handler.h"
#include "concanv.h"
#include "gui_lib.h"
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#ifndef CLIAPP
extern ConsoleCanvas *console;
#endif

extern BasePlatform* g_BasePlatform;
extern AisDecoder *g_pAIS;
extern RouteList *pRouteList;
extern std::vector<Track*> g_TrackList;
extern Select *pSelect;
extern Routeman *g_pRouteMan;

extern wxRect g_blink_rect;

extern double gLat, gLon, gSog, gCog;
extern double gVar;
extern wxString gRmcDate, gRmcTime;
extern bool g_bMagneticAPB;

extern RoutePoint *pAnchorWatchPoint1;
extern RoutePoint *pAnchorWatchPoint2;

extern ActiveTrack *g_pActiveTrack;
extern int g_track_line_width;

extern int g_route_line_width;

extern bool g_bAdvanceRouteWaypointOnArrivalOnly;
extern Route *pAISMOBRoute;
extern bool g_btouch;
extern float g_ChartScaleFactorExp;

extern bool g_bShowShipToActive;
extern bool g_bAllowShipToActive;
extern int g_maxWPNameLength;

bool g_bPluginHandleAutopilotRoute;

//    List definitions for Waypoint Manager Icons
WX_DECLARE_LIST(wxBitmap, markicon_bitmap_list_type);
WX_DECLARE_LIST(wxString, markicon_key_list_type);
WX_DECLARE_LIST(wxString, markicon_description_list_type);

//    List implementation for Waypoint Manager Icons
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(markicon_bitmap_list_type);
WX_DEFINE_LIST(markicon_key_list_type);
WX_DEFINE_LIST(markicon_description_list_type);

// Helper conditional file name dir slash
void appendOSDirSlash(wxString *pString);


//--------------------------------------------------------------------------------
//      Routeman   "Route Manager"
//--------------------------------------------------------------------------------

Routeman::Routeman(struct RoutePropDlgCtx ctx,
                   std::function<void()> dlg_update_list_ctrl) {
  m_prop_dlg_ctx = ctx;
  m_route_mgr_dlg_update_list_ctrl = dlg_update_list_ctrl;
  pActiveRoute = NULL;
  pActivePoint = NULL;
  pRouteActivatePoint = NULL;
}

Routeman::~Routeman() {
  if (pRouteActivatePoint) delete pRouteActivatePoint;
}

bool Routeman::IsRouteValid(Route *pRoute) {
  wxRouteListNode *node = pRouteList->GetFirst();
  while (node) {
    if (pRoute == node->GetData()) return true;
    node = node->GetNext();
  }
  return false;
}

//    Make a 2-D search to find the route containing a given waypoint
Route *Routeman::FindRouteContainingWaypoint(RoutePoint *pWP) {
  wxRouteListNode *node = pRouteList->GetFirst();
  while (node) {
    Route *proute = node->GetData();

    wxRoutePointListNode *pnode = (proute->pRoutePointList)->GetFirst();
    while (pnode) {
      RoutePoint *prp = pnode->GetData();
      if (prp == pWP) return proute;
      pnode = pnode->GetNext();
    }

    node = node->GetNext();
  }

  return NULL;  // not found
}

//    Make a 2-D search to find the visual route containing a given waypoint
Route *Routeman::FindVisibleRouteContainingWaypoint(RoutePoint *pWP) {
  wxRouteListNode *node = pRouteList->GetFirst();
  while (node) {
    Route *proute = node->GetData();
    if (proute->IsVisible()) {
      wxRoutePointListNode *pnode = (proute->pRoutePointList)->GetFirst();
      while (pnode) {
        RoutePoint *prp = pnode->GetData();
        if (prp == pWP) return proute;
        pnode = pnode->GetNext();
      }
    }

    node = node->GetNext();
  }

  return NULL;  // not found
}

wxArrayPtrVoid *Routeman::GetRouteArrayContaining(RoutePoint *pWP) {
  wxArrayPtrVoid *pArray = new wxArrayPtrVoid;

  wxRouteListNode *route_node = pRouteList->GetFirst();
  while (route_node) {
    Route *proute = route_node->GetData();

    wxRoutePointListNode *waypoint_node = (proute->pRoutePointList)->GetFirst();
    while (waypoint_node) {
      RoutePoint *prp = waypoint_node->GetData();
      if (prp == pWP) {  // success
        pArray->Add((void *)proute);
        break;  // only add a route to the array once, even if there are
                // duplicate points in the route...See FS#1743
      }

      waypoint_node = waypoint_node->GetNext();  // next waypoint
    }

    route_node = route_node->GetNext();  // next route
  }

  if (pArray->GetCount())
    return pArray;

  else {
    delete pArray;
    return NULL;
  }
}

void Routeman::RemovePointFromRoute(RoutePoint *point, Route *route,
                                    int route_state) {
  //  Rebuild the route selectables
  pSelect->DeleteAllSelectableRoutePoints(route);
  pSelect->DeleteAllSelectableRouteSegments(route);

  route->RemovePoint(point);

  //  Check for 1 point routes. If we are creating a route, this is an undo, so
  //  keep the 1 point.
  if (route->GetnPoints() <= 1 && route_state == 0) {
    NavObjectChanges::getInstance()->DeleteConfigRoute(route);
    g_pRouteMan->DeleteRoute(route);
    route = NULL;
  }
  //  Add this point back into the selectables
  pSelect->AddSelectableRoutePoint(point->m_lat, point->m_lon, point);

  //if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
  //  pRoutePropDialog->SetRouteAndUpdate(route, true);
  //}
  m_prop_dlg_ctx.SetRouteAndUpdate(route);

}

RoutePoint *Routeman::FindBestActivatePoint(Route *pR, double lat, double lon,
                                            double cog, double sog) {
  if (!pR) return NULL;

  // Walk thru all the points to find the "best"
  RoutePoint *best_point = NULL;
  double min_time_found = 1e6;

  wxRoutePointListNode *node = (pR->pRoutePointList)->GetFirst();
  while (node) {
    RoutePoint *pn = node->GetData();

    double brg, dist;
    DistanceBearingMercator(pn->m_lat, pn->m_lon, lat, lon, &brg, &dist);

    double angle = brg - cog;
    double soa = cos(angle * PI / 180.);

    double time_to_wp = dist / soa;

    if (time_to_wp > 0) {
      if (time_to_wp < min_time_found) {
        min_time_found = time_to_wp;
        best_point = pn;
      }
    }
    node = node->GetNext();
  }
  return best_point;
}

bool Routeman::ActivateRoute(Route *pRouteToActivate, RoutePoint *pStartPoint) {
  g_bAllowShipToActive = false;
  wxJSONValue v;
  v[_T("Route_activated")] = pRouteToActivate->m_RouteNameString;
  v[_T("GUID")] = pRouteToActivate->m_GUID;
  json_msg.notify(std::make_shared<wxJSONValue>(v), "OCPN_RTE_ACTIVATED");
  if (g_bPluginHandleAutopilotRoute) return true;

  pActiveRoute = pRouteToActivate;

  if (pStartPoint) {
    pActivePoint = pStartPoint;
  } else {
    wxRoutePointListNode *node = (pActiveRoute->pRoutePointList)->GetFirst();
    pActivePoint = node->GetData();  // start at beginning
  }

  ActivateRoutePoint(pRouteToActivate, pActivePoint);

  m_bArrival = false;
  m_arrival_min = 1e6;
  m_arrival_test = 0;

  pRouteToActivate->m_bRtIsActive = true;

  m_bDataValid = false;

#ifndef CLIAPP
  console->ShowWithFreshFonts();
#endif

  return true;
}

bool Routeman::ActivateRoutePoint(Route *pA, RoutePoint *pRP_target) {
  g_bAllowShipToActive = false;
  wxJSONValue v;
  v[_T("GUID")] = pRP_target->m_GUID;
  v[_T("WP_activated")] = pRP_target->GetName();

  json_msg.notify(std::make_shared<wxJSONValue>(v), "OCPN_WPT_ACTIVATED");

  if (g_bPluginHandleAutopilotRoute) return true;

  pActiveRoute = pA;

  pActivePoint = pRP_target;
  pActiveRoute->m_pRouteActivePoint = pRP_target;

  wxRoutePointListNode *node = (pActiveRoute->pRoutePointList)->GetFirst();
  while (node) {
    RoutePoint *pn = node->GetData();
    pn->m_bBlink = false;  // turn off all blinking points
    pn->m_bIsActive = false;

    node = node->GetNext();
  }

  node = (pActiveRoute->pRoutePointList)->GetFirst();
  RoutePoint *prp_first = node->GetData();

  //  If activating first point in route, create a "virtual" waypoint at present
  //  position
  if (pRP_target == prp_first) {
    if (pRouteActivatePoint) delete pRouteActivatePoint;

    pRouteActivatePoint =
        new RoutePoint(gLat, gLon, wxString(_T("")), wxString(_T("")),
                       wxEmptyString, false);  // Current location
    pRouteActivatePoint->m_bShowName = false;

    pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
  }

  else {
    prp_first->m_bBlink = false;
    node = node->GetNext();
    RoutePoint *np_prev = prp_first;
    while (node) {
      RoutePoint *pnext = node->GetData();
      if (pnext == pRP_target) {
        pActiveRouteSegmentBeginPoint = np_prev;
        break;
      }

      np_prev = pnext;
      node = node->GetNext();
    }
  }

  pRP_target->m_bBlink = true;     // blink the active point
  pRP_target->m_bIsActive = true;  // and active

  g_blink_rect = pRP_target->CurrentRect_in_DC;  // set up global blinker

  m_bArrival = false;
  m_arrival_min = 1e6;
  m_arrival_test = 0;

  //    Update the RouteProperties Dialog, if currently shown
  ///  if (pRoutePropDialog && pRoutePropDialog->IsShown()) {
  ///    if (pRoutePropDialog->GetRoute() == pA) {
  ///      pRoutePropDialog->SetEnroutePoint(pActivePoint);
  ///    }
  ///  }
  m_prop_dlg_ctx.SetEnroutePoint(pA, pActivePoint);
  return true;
}

bool Routeman::ActivateNextPoint(Route *pr, bool skipped) {
  g_bAllowShipToActive = false;
  wxJSONValue v;
  if (pActivePoint) {
    pActivePoint->m_bBlink = false;
    pActivePoint->m_bIsActive = false;

    v[_T("isSkipped")] = skipped;
    v[_T("GUID")] = pActivePoint->m_GUID;
    v[_T("GUID_WP_arrived")] = pActivePoint->m_GUID;
    v[_T("WP_arrived")] = pActivePoint->GetName();
  }
  int n_index_active = pActiveRoute->GetIndexOf(pActivePoint);
  if ((n_index_active + 1) <= pActiveRoute->GetnPoints()) {
    pActiveRouteSegmentBeginPoint = pActivePoint;

    pActiveRoute->m_pRouteActivePoint =
        pActiveRoute->GetPoint(n_index_active + 1);

    pActivePoint = pActiveRoute->GetPoint(n_index_active + 1);
    v[_T("Next_WP")] = pActivePoint->GetName();
    v[_T("GUID_Next_WP")] = pActivePoint->m_GUID;

    pActivePoint->m_bBlink = true;
    pActivePoint->m_bIsActive = true;
    g_blink_rect = pActivePoint->CurrentRect_in_DC;  // set up global blinker

    m_bArrival = false;
    m_arrival_min = 1e6;
    m_arrival_test = 0;

    //    Update the RouteProperties Dialog, if currently shown
    /// if (pRoutePropDialog && pRoutePropDialog->IsShown()) {
    ///   if (pRoutePropDialog->GetRoute() == pr) {
    ///     pRoutePropDialog->SetEnroutePoint(pActivePoint);
    ///   }
    /// }
    m_prop_dlg_ctx.SetEnroutePoint(pr, pActivePoint);

    json_msg.notify(std::make_shared<wxJSONValue>(v), "OCPN_WPT_ARRIVED");
    return true;
  }

  return false;
}

bool Routeman::DeactivateRoute(bool b_arrival) {
  if (pActivePoint) {
    pActivePoint->m_bBlink = false;
    pActivePoint->m_bIsActive = false;
  }

  if (pActiveRoute) {
    pActiveRoute->m_bRtIsActive = false;
    pActiveRoute->m_pRouteActivePoint = NULL;

    wxJSONValue v;
    if (!b_arrival) {
      v[_T("Route_deactivated")] = pActiveRoute->m_RouteNameString;
      v[_T("GUID")] = pActiveRoute->m_GUID;
      json_msg.notify(std::make_shared<wxJSONValue>(v), "OCPN_RTE_DEACTIVATED");
    } else {
      v[_T("GUID")] = pActiveRoute->m_GUID;
      v[_T("Route_ended")] = pActiveRoute->m_RouteNameString;
      json_msg.notify(std::make_shared<wxJSONValue>(v), "OCPN_RTE_ENDED");
    }
  }

  pActiveRoute = NULL;

  if (pRouteActivatePoint) delete pRouteActivatePoint;
  pRouteActivatePoint = NULL;

  pActivePoint = NULL;

#ifndef CLIAPP
  console->pCDI->ClearBackground();
  console->Show(false);
#endif

  m_bDataValid = false;

  return true;
}

bool Routeman::UpdateAutopilot() {
  // Send all known Autopilot messages upstream

  // Set max WP name length
  int maxName = 6;
  if ((g_maxWPNameLength >= 3) && (g_maxWPNameLength <= 32))
    maxName = g_maxWPNameLength;

  // Avoid a possible not initiated SOG/COG. APs can be confused if in NAV mode
  // wo valid GPS
  double r_Sog(0.0), r_Cog(0.0);
  if (!std::isnan(gSog)) r_Sog = gSog;
  if (!std::isnan(gCog)) r_Cog = gCog;

  // Send active leg info directly to plugins

  ActiveLegDat leg_info;
  leg_info.Btw = CurrentBrgToActivePoint;
  leg_info.Dtw = CurrentRngToActivePoint;
  leg_info.Xte = CurrentXTEToActivePoint;
  if (XTEDir < 0) {
    leg_info.Xte = -leg_info.Xte;  // Left side of the track -> negative XTE
  }
  leg_info.wp_name = pActivePoint->GetName().Truncate(maxName);
  leg_info.arrival = m_bArrival;

  json_leg_info.notify(std::make_shared<ActiveLegDat>(leg_info), "");

  // RMB
  {
    m_NMEA0183.TalkerID = _T("EC");

    SENTENCE snt;
    m_NMEA0183.Rmb.IsDataValid = NTrue;
    m_NMEA0183.Rmb.CrossTrackError = CurrentXTEToActivePoint;

    if (XTEDir < 0)
      m_NMEA0183.Rmb.DirectionToSteer = Left;
    else
      m_NMEA0183.Rmb.DirectionToSteer = Right;

    m_NMEA0183.Rmb.To = pActivePoint->GetName().Truncate(maxName);
    m_NMEA0183.Rmb.From =
        pActiveRouteSegmentBeginPoint->GetName().Truncate(maxName);

    if (pActivePoint->m_lat < 0.)
      m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(-pActivePoint->m_lat,
                                                      _T("S"));
    else
      m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(pActivePoint->m_lat,
                                                      _T("N"));

    if (pActivePoint->m_lon < 0.)
      m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(-pActivePoint->m_lon,
                                                       _T("W"));
    else
      m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(pActivePoint->m_lon,
                                                       _T("E"));

    m_NMEA0183.Rmb.RangeToDestinationNauticalMiles = CurrentRngToActivePoint;
    m_NMEA0183.Rmb.BearingToDestinationDegreesTrue = CurrentBrgToActivePoint;
    m_NMEA0183.Rmb.DestinationClosingVelocityKnots =
        r_Sog * cos((r_Cog - CurrentBrgToActivePoint) * PI / 180.0);

    if (m_bArrival)
      m_NMEA0183.Rmb.IsArrivalCircleEntered = NTrue;
    else
      m_NMEA0183.Rmb.IsArrivalCircleEntered = NFalse;

    m_NMEA0183.Rmb.FAAModeIndicator = "A";
    m_NMEA0183.Rmb.Write(snt);

    BroadcastNMEA0183Message(snt.Sentence);
  }

  // RMC
  {
    m_NMEA0183.TalkerID = _T("EC");

    SENTENCE snt;
    m_NMEA0183.Rmc.IsDataValid = NTrue;

    if (gLat < 0.)
      m_NMEA0183.Rmc.Position.Latitude.Set(-gLat, _T("S"));
    else
      m_NMEA0183.Rmc.Position.Latitude.Set(gLat, _T("N"));

    if (gLon < 0.)
      m_NMEA0183.Rmc.Position.Longitude.Set(-gLon, _T("W"));
    else
      m_NMEA0183.Rmc.Position.Longitude.Set(gLon, _T("E"));

    m_NMEA0183.Rmc.SpeedOverGroundKnots = r_Sog;
    m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue = r_Cog;

    if (!std::isnan(gVar)) {
      if (gVar < 0.) {
        m_NMEA0183.Rmc.MagneticVariation = -gVar;
        m_NMEA0183.Rmc.MagneticVariationDirection = West;
      } else {
        m_NMEA0183.Rmc.MagneticVariation = gVar;
        m_NMEA0183.Rmc.MagneticVariationDirection = East;
      }
    } else
      m_NMEA0183.Rmc.MagneticVariation =
          361.;  // A signal to NMEA converter, gVAR is unknown

    // Send GPS time to autopilot if available else send local system time
    if (!gRmcTime.IsEmpty() && !gRmcDate.IsEmpty()) {
      m_NMEA0183.Rmc.UTCTime = gRmcTime;
      m_NMEA0183.Rmc.Date = gRmcDate;
    } else {
      wxDateTime now = wxDateTime::Now();
      wxDateTime utc = now.ToUTC();
      wxString time = utc.Format(_T("%H%M%S"));
      m_NMEA0183.Rmc.UTCTime = time;
      wxString date = utc.Format(_T("%d%m%y"));
      m_NMEA0183.Rmc.Date = date;
    }

    m_NMEA0183.Rmc.FAAModeIndicator = "A";
    m_NMEA0183.Rmc.Write(snt);

    BroadcastNMEA0183Message(snt.Sentence);
  }

  // APB
  {
    m_NMEA0183.TalkerID = _T("EC");

    SENTENCE snt;

    m_NMEA0183.Apb.IsLoranBlinkOK = NTrue;
    m_NMEA0183.Apb.IsLoranCCycleLockOK = NTrue;

    m_NMEA0183.Apb.CrossTrackErrorMagnitude = CurrentXTEToActivePoint;

    if (XTEDir < 0)
      m_NMEA0183.Apb.DirectionToSteer = Left;
    else
      m_NMEA0183.Apb.DirectionToSteer = Right;

    m_NMEA0183.Apb.CrossTrackUnits = _T("N");

    if (m_bArrival)
      m_NMEA0183.Apb.IsArrivalCircleEntered = NTrue;
    else
      m_NMEA0183.Apb.IsArrivalCircleEntered = NFalse;

    //  We never pass the perpendicular, since we declare arrival before
    //  reaching this point
    m_NMEA0183.Apb.IsPerpendicular = NFalse;

    m_NMEA0183.Apb.To = pActivePoint->GetName().Truncate(maxName);

    double brg1, dist1;
    DistanceBearingMercator(pActivePoint->m_lat, pActivePoint->m_lon,
                            pActiveRouteSegmentBeginPoint->m_lat,
                            pActiveRouteSegmentBeginPoint->m_lon, &brg1,
                            &dist1);

    if (g_bMagneticAPB && !std::isnan(gVar)) {
      double brg1m =
          ((brg1 - gVar) >= 0.) ? (brg1 - gVar) : (brg1 - gVar + 360.);
      double bapm = ((CurrentBrgToActivePoint - gVar) >= 0.)
                        ? (CurrentBrgToActivePoint - gVar)
                        : (CurrentBrgToActivePoint - gVar + 360.);

      m_NMEA0183.Apb.BearingOriginToDestination = brg1m;
      m_NMEA0183.Apb.BearingOriginToDestinationUnits = _T("M");

      m_NMEA0183.Apb.BearingPresentPositionToDestination = bapm;
      m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("M");

      m_NMEA0183.Apb.HeadingToSteer = bapm;
      m_NMEA0183.Apb.HeadingToSteerUnits = _T("M");
    } else {
      m_NMEA0183.Apb.BearingOriginToDestination = brg1;
      m_NMEA0183.Apb.BearingOriginToDestinationUnits = _T("T");

      m_NMEA0183.Apb.BearingPresentPositionToDestination =
          CurrentBrgToActivePoint;
      m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("T");

      m_NMEA0183.Apb.HeadingToSteer = CurrentBrgToActivePoint;
      m_NMEA0183.Apb.HeadingToSteerUnits = _T("T");
    }

    m_NMEA0183.Apb.Write(snt);
    BroadcastNMEA0183Message(snt.Sentence);
  }

  // XTE
  {
    m_NMEA0183.TalkerID = _T("EC");

    SENTENCE snt;

    m_NMEA0183.Xte.IsLoranBlinkOK = NTrue;
    m_NMEA0183.Xte.IsLoranCCycleLockOK = NTrue;

    m_NMEA0183.Xte.CrossTrackErrorDistance = CurrentXTEToActivePoint;

    if (XTEDir < 0)
      m_NMEA0183.Xte.DirectionToSteer = Left;
    else
      m_NMEA0183.Xte.DirectionToSteer = Right;

    m_NMEA0183.Xte.CrossTrackUnits = _T("N");

    m_NMEA0183.Xte.Write(snt);
    BroadcastNMEA0183Message(snt.Sentence);
  }

  return true;
}

bool Routeman::DoesRouteContainSharedPoints(Route *pRoute) {
  if (pRoute) {
    // walk the route, looking at each point to see if it is used by another
    // route or is isolated
    wxRoutePointListNode *pnode = (pRoute->pRoutePointList)->GetFirst();
    while (pnode) {
      RoutePoint *prp = pnode->GetData();

      // check all other routes to see if this point appears in any other route
      wxArrayPtrVoid *pRA = GetRouteArrayContaining(prp);

      if (pRA) {
        for (unsigned int ir = 0; ir < pRA->GetCount(); ir++) {
          Route *pr = (Route *)pRA->Item(ir);
          if (pr == pRoute)
            continue;  // self
          else
            return true;
        }
      }

      if (pnode) pnode = pnode->GetNext();
    }

    //      Now walk the route again, looking for isolated type shared waypoints
    pnode = (pRoute->pRoutePointList)->GetFirst();
    while (pnode) {
      RoutePoint *prp = pnode->GetData();
      if (prp->IsShared()) return true;

      if (pnode) pnode = pnode->GetNext();
    }
  }

  return false;
}

bool Routeman::DeleteRoute(Route *pRoute) {
  if (pRoute) {
    if (pRoute == pAISMOBRoute) {
#ifdef CLIAPP
      pAISMOBRoute = NULL;
#else
      int ret = OCPNMessageBox(NULL,
                               _("You are trying to delete an active AIS MOB "
                                 "route, are you REALLY sure?"),
                               _("OpenCPN Warning"), wxYES_NO);

      if (ret == wxID_NO)
        return false;
      else
        pAISMOBRoute = NULL;
#endif
    }
    ::wxBeginBusyCursor();

    if (GetpActiveRoute() == pRoute) DeactivateRoute();

    if (pRoute->m_bIsInLayer) {
      ::wxEndBusyCursor();
      return false;
    }
    /// if (pRoutePropDialog && (pRoutePropDialog->IsShown()) &&
    ///     (pRoute == pRoutePropDialog->GetRoute())) {
    ///   pRoutePropDialog->Hide();
    /// }
    m_prop_dlg_ctx.Hide(pRoute);

    NavObjectChanges::getInstance()->DeleteConfigRoute(pRoute);

    //    Remove the route from associated lists
    pSelect->DeleteAllSelectableRouteSegments(pRoute);
    pRouteList->DeleteObject(pRoute);

    m_route_mgr_dlg_update_list_ctrl();   // Update the RouteManagerDialog
    ///if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    ///  pRouteManagerDialog->UpdateRouteListCtrl();

    // walk the route, tentatively deleting/marking points used only by this
    // route
    wxRoutePointListNode *pnode = (pRoute->pRoutePointList)->GetFirst();
    while (pnode) {
      RoutePoint *prp = pnode->GetData();

      // check all other routes to see if this point appears in any other route
      Route *pcontainer_route = FindRouteContainingWaypoint(prp);

      if (pcontainer_route == NULL && prp->m_bIsInRoute) {
        prp->m_bIsInRoute =
            false;  // Take this point out of this (and only) route
        if (!prp->IsShared()) {
          //    This does not need to be done with navobj.xml storage, since the
          //    waypoints are stored with the route
          //                              pConfig->DeleteWayPoint(prp);

          pSelect->DeleteSelectablePoint(prp, SELTYPE_ROUTEPOINT);

          // Remove all instances of this point from the list.
          wxRoutePointListNode *pdnode = pnode;
          while (pdnode) {
            pRoute->pRoutePointList->DeleteNode(pdnode);
            pdnode = pRoute->pRoutePointList->Find(prp);
          }

          pnode = NULL;
          delete prp;
        } else {
          prp->m_bDynamicName = false;
          prp->m_bIsolatedMark = true;  // This has become an isolated mark
          prp->SetShared(false);        // and is no longer part of a route
        }
      }
      if (pnode)
        pnode = pnode->GetNext();
      else
        pnode = pRoute->pRoutePointList->GetFirst();  // restart the list
    }

    delete pRoute;

    ::wxEndBusyCursor();
  }
  return true;
}

void Routeman::DeleteAllRoutes(void) {
  ::wxBeginBusyCursor();

  //    Iterate on the RouteList
  wxRouteListNode *node = pRouteList->GetFirst();
  while (node) {
    Route *proute = node->GetData();
    if (proute == pAISMOBRoute) {
#ifdef CLIAPP
      pAISMOBRoute = NULL;
#else
      ::wxEndBusyCursor();
      int ret = OCPNMessageBox(NULL,
                               _("You are trying to delete an active AIS MOB "
                                 "route, are you REALLY sure?"),
                               _("OpenCPN Warning"), wxYES_NO);
      if (ret == wxID_NO)
        return;
      else
        pAISMOBRoute = NULL;
      ::wxBeginBusyCursor();
#endif
    }

    node = node->GetNext();
    if (proute->m_bIsInLayer) continue;

    NavObjectChanges::getInstance()->m_bSkipChangeSetUpdate = true;
    NavObjectChanges::getInstance()->DeleteConfigRoute(proute);
    DeleteRoute(proute);
    NavObjectChanges::getInstance()->m_bSkipChangeSetUpdate = false;
  }

  ::wxEndBusyCursor();
}

#ifdef  CLIAPP
wxColour GetGlobalColor(wxString name) { return *wxBLACK; }
#endif

void Routeman::SetColorScheme(ColorScheme cs, double displayDPmm) {
  // Re-Create the pens and colors

  int scaled_line_width = g_route_line_width;
  int track_scaled_line_width = g_track_line_width;
  if (g_btouch) {
    // 0.2 mm nominal, but not less than 1 pixel
    double nominal_line_width_pix = wxMax(1.5, floor(displayDPmm / 5.0));

    double sline_width = wxMax(nominal_line_width_pix, g_route_line_width);
    sline_width *= g_ChartScaleFactorExp;
    scaled_line_width = wxMax(sline_width, 2);

    double tsline_width = wxMax(nominal_line_width_pix, g_track_line_width);
    tsline_width *= g_ChartScaleFactorExp;
    track_scaled_line_width = wxMax(tsline_width, 2);
  }

  m_pActiveRoutePointPen = wxThePenList->FindOrCreatePen(
      wxColour(0, 0, 255), scaled_line_width, wxPENSTYLE_SOLID);
  m_pRoutePointPen = wxThePenList->FindOrCreatePen(
      wxColour(0, 0, 255), scaled_line_width, wxPENSTYLE_SOLID);

  //    Or in something like S-52 compliance

  m_pRoutePen = wxThePenList->FindOrCreatePen(
      GetGlobalColor(_T("UINFB")), scaled_line_width, wxPENSTYLE_SOLID);
  m_pSelectedRoutePen = wxThePenList->FindOrCreatePen(
      GetGlobalColor(_T("UINFO")), scaled_line_width, wxPENSTYLE_SOLID);
  m_pActiveRoutePen = wxThePenList->FindOrCreatePen(
      GetGlobalColor(_T("UARTE")), scaled_line_width, wxPENSTYLE_SOLID);
  m_pTrackPen = wxThePenList->FindOrCreatePen(
      GetGlobalColor(_T("CHMGD")), track_scaled_line_width, wxPENSTYLE_SOLID);

  m_pRouteBrush = wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T("UINFB")),
                                                    wxBRUSHSTYLE_SOLID);
  m_pSelectedRouteBrush = wxTheBrushList->FindOrCreateBrush(
      GetGlobalColor(_T("UINFO")), wxBRUSHSTYLE_SOLID);
  m_pActiveRouteBrush = wxTheBrushList->FindOrCreateBrush(
      GetGlobalColor(_T("PLRTE")), wxBRUSHSTYLE_SOLID);
}

wxString Routeman::GetRouteReverseMessage(void) {
  return wxString(
      _("Waypoints can be renamed to reflect the new order, the names will be "
        "'001', '002' etc.\n\nDo you want to rename the waypoints?"));
}

wxString Routeman::GetRouteResequenceMessage(void) {
  return wxString(
      _("Waypoints will be renamed to reflect the natural order, the names "
        "will be '001', '002' etc.\n\nDo you want to rename the waypoints?"));
}

Route *Routeman::FindRouteByGUID(const wxString &guid) {
  wxRouteListNode *node1 = pRouteList->GetFirst();
  while (node1) {
    Route *pRoute = node1->GetData();

    if (pRoute->m_GUID == guid) return pRoute;
    node1 = node1->GetNext();
  }

  return NULL;
}

Track *Routeman::FindTrackByGUID(const wxString &guid) {
  for (Track* pTrack : g_TrackList) {
    if (pTrack->m_GUID == guid) return pTrack;
  }

  return NULL;
}

void Routeman::ZeroCurrentXTEToActivePoint() {
  // When zeroing XTE create a "virtual" waypoint at present position
  if (pRouteActivatePoint) delete pRouteActivatePoint;
  pRouteActivatePoint =
      new RoutePoint(gLat, gLon, wxString(_T("")), wxString(_T("")),
                     wxEmptyString, false);  // Current location
  pRouteActivatePoint->m_bShowName = false;

  pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
  m_arrival_min = 1e6;
}

//--------------------------------------------------------------------------------
//      WayPointman   Implementation
//--------------------------------------------------------------------------------

WayPointman::WayPointman() {
  m_pWayPointList = new RoutePointList;

  pmarkicon_image_list = NULL;

  //ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
  m_pIconArray = new ArrayOfMarkIcon;
  m_pLegacyIconArray = NULL;
  m_pExtendedIconArray = NULL;

  m_cs = (ColorScheme)-1;

  m_nGUID = 0;
  m_iconListScale = -999.0;
  m_iconListHeight = -1;
}

WayPointman::~WayPointman() {
  //    Two step here, since the RoutePoint dtor also touches the
  //    RoutePoint list.
  //    Copy the master RoutePoint list to a temporary list,
  //    then clear and delete objects from the temp list

  RoutePointList temp_list;

  wxRoutePointListNode *node = m_pWayPointList->GetFirst();
  while (node) {
    RoutePoint *pr = node->GetData();

    temp_list.Append(pr);
    node = node->GetNext();
  }

  temp_list.DeleteContents(true);
  temp_list.Clear();

  m_pWayPointList->Clear();
  delete m_pWayPointList;

  for (unsigned int i = 0; i < m_pIconArray->GetCount(); i++) {
    MarkIcon *pmi = (MarkIcon *)m_pIconArray->Item(i);
    delete pmi->piconBitmap;
    delete pmi;
  }

  m_pIconArray->Clear();
  delete m_pIconArray;

  if (pmarkicon_image_list) pmarkicon_image_list->RemoveAll();
  delete pmarkicon_image_list;
}

bool WayPointman::AddRoutePoint(RoutePoint *prp) {
  if (!prp) return false;

  wxRoutePointListNode *prpnode = m_pWayPointList->Append(prp);
  prp->SetManagerListNode(prpnode);

  return true;
}

bool WayPointman::RemoveRoutePoint(RoutePoint *prp) {
  if (!prp) return false;

  wxRoutePointListNode *prpnode =
      (wxRoutePointListNode *)prp->GetManagerListNode();

  if (prpnode)
    delete prpnode;
  else
    m_pWayPointList->DeleteObject(prp);

  prp->SetManagerListNode(NULL);

  return true;
}

wxImageList *WayPointman::Getpmarkicon_image_list(int nominal_height) {
  // Cached version available?
  if (pmarkicon_image_list && (nominal_height == m_iconListHeight)) {
    return pmarkicon_image_list;
  }

  // Build an image list large enough
  if (NULL != pmarkicon_image_list) {
    pmarkicon_image_list->RemoveAll();
    delete pmarkicon_image_list;
  }
  pmarkicon_image_list = new wxImageList(nominal_height, nominal_height);

  m_iconListHeight = nominal_height;
  m_bitmapSizeForList = nominal_height;

  return pmarkicon_image_list;
}

wxBitmap *WayPointman::CreateDimBitmap(wxBitmap *pBitmap, double factor) {
  wxImage img = pBitmap->ConvertToImage();
  int sx = img.GetWidth();
  int sy = img.GetHeight();

  wxImage new_img(img);

  for (int i = 0; i < sx; i++) {
    for (int j = 0; j < sy; j++) {
      if (!img.IsTransparent(i, j)) {
        new_img.SetRGB(i, j, (unsigned char)(img.GetRed(i, j) * factor),
                       (unsigned char)(img.GetGreen(i, j) * factor),
                       (unsigned char)(img.GetBlue(i, j) * factor));
      }
    }
  }

  wxBitmap *pret = new wxBitmap(new_img);

  return pret;
}

wxImage WayPointman::CreateDimImage(wxImage &image, double factor) {
  int sx = image.GetWidth();
  int sy = image.GetHeight();

  wxImage new_img(image);

  for (int i = 0; i < sx; i++) {
    for (int j = 0; j < sy; j++) {
      if (!image.IsTransparent(i, j)) {
        new_img.SetRGB(i, j, (unsigned char)(image.GetRed(i, j) * factor),
                       (unsigned char)(image.GetGreen(i, j) * factor),
                       (unsigned char)(image.GetBlue(i, j) * factor));
      }
    }
  }

  return wxImage(new_img);
}

bool WayPointman::DoesIconExist(const wxString &icon_key) const {
  MarkIcon *pmi;
  unsigned int i;

  for (i = 0; i < m_pIconArray->GetCount(); i++) {
    pmi = (MarkIcon *)m_pIconArray->Item(i);
    if (pmi->icon_name.IsSameAs(icon_key)) return true;
  }

  return false;
}

wxBitmap *WayPointman::GetIconBitmap(const wxString &icon_key) {
  wxBitmap *pret = NULL;
  MarkIcon *pmi = NULL;
  unsigned int i;

  for (i = 0; i < m_pIconArray->GetCount(); i++) {
    pmi = (MarkIcon *)m_pIconArray->Item(i);
    if (pmi->icon_name.IsSameAs(icon_key)) break;
  }

  if (i == m_pIconArray->GetCount())  // key not found
  {
    // find and return bitmap for "circle"
    for (i = 0; i < m_pIconArray->GetCount(); i++) {
      pmi = (MarkIcon *)m_pIconArray->Item(i);
      //            if( pmi->icon_name.IsSameAs( _T("circle") ) )
      //                break;
    }
  }

  if (i == m_pIconArray->GetCount())          // "circle" not found
    pmi = (MarkIcon *)m_pIconArray->Item(0);  // use item 0

  if (pmi) {
    if (pmi->piconBitmap)
      pret = pmi->piconBitmap;
    else {
      if (pmi->iconImage.IsOk()) {
        pmi->piconBitmap = new wxBitmap(pmi->iconImage);
        pret = pmi->piconBitmap;
      }
    }
  }
  return pret;
}

bool WayPointman::GetIconPrescaled(const wxString &icon_key) {
  MarkIcon *pmi = NULL;
  unsigned int i;

  for (i = 0; i < m_pIconArray->GetCount(); i++) {
    pmi = (MarkIcon *)m_pIconArray->Item(i);
    if (pmi->icon_name.IsSameAs(icon_key)) break;
  }

  if (i == m_pIconArray->GetCount())  // key not found
  {
    // find and return bitmap for "circle"
    for (i = 0; i < m_pIconArray->GetCount(); i++) {
      pmi = (MarkIcon *)m_pIconArray->Item(i);
      //            if( pmi->icon_name.IsSameAs( _T("circle") ) )
      //                break;
    }
  }

  if (i == m_pIconArray->GetCount())          // "circle" not found
    pmi = (MarkIcon *)m_pIconArray->Item(0);  // use item 0

  if (pmi)
    return pmi->preScaled;
  else
    return false;
}

unsigned int WayPointman::GetIconTexture(const wxBitmap *pbm, int &glw,
                                         int &glh) {
#ifdef ocpnUSE_GL
  int index = GetIconIndex(pbm);
  MarkIcon *pmi = (MarkIcon *)m_pIconArray->Item(index);

  if (!pmi->icon_texture) {
    /* make rgba texture */
    wxImage image = pbm->ConvertToImage();
    unsigned char *d = image.GetData();
    if (d == 0) {
      // don't create a texture with junk
      return 0;
    }

    glGenTextures(1, &pmi->icon_texture);
    glBindTexture(GL_TEXTURE_2D, pmi->icon_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    int w = image.GetWidth(), h = image.GetHeight();

    pmi->tex_w = NextPow2(w);
    pmi->tex_h = NextPow2(h);

    unsigned char *a = image.GetAlpha();

    unsigned char mr, mg, mb;
    if (!a) image.GetOrFindMaskColour(&mr, &mg, &mb);

    unsigned char *e = new unsigned char[4 * w * h];
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        unsigned char r, g, b;
        int off = (y * w + x);
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pmi->tex_w, pmi->tex_h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, e);

    delete[] e;
  }

  glw = pmi->tex_w;
  glh = pmi->tex_h;

  return pmi->icon_texture;
#else
  return 0;
#endif
}

wxBitmap WayPointman::GetIconBitmapForList(int index, int height) {
  wxBitmap pret;
  MarkIcon *pmi;

  if (index >= 0) {
    pmi = (MarkIcon *)m_pIconArray->Item(index);
    // Scale the icon to "list size" if necessary
    if (pmi->iconImage.GetHeight() != height) {
      int w = height;
      int h = height;
      int w0 = pmi->iconImage.GetWidth();
      int h0 = pmi->iconImage.GetHeight();

      wxImage icon_resized = pmi->iconImage;  // make a copy
      if (h0 <= h && w0 <= w) {
        icon_resized = pmi->iconImage.Resize(
            wxSize(w, h), wxPoint(w / 2 - w0 / 2, h / 2 - h0 / 2));
      } else {
        // rescale in one or two directions to avoid cropping, then resize to
        // fit to cell
        int h1 = h;
        int w1 = w;
        if (h0 > h)
          w1 = wxRound((double)w0 * ((double)h / (double)h0));

        else if (w0 > w)
          h1 = wxRound((double)h0 * ((double)w / (double)w0));

        icon_resized = pmi->iconImage.Rescale(w1, h1);
        icon_resized = pmi->iconImage.Resize(
            wxSize(w, h), wxPoint(w / 2 - w1 / 2, h / 2 - h1 / 2));
      }

      pret = wxBitmap(icon_resized);

    } else
      pret = wxBitmap(pmi->iconImage);
  }

  return pret;
}

wxString *WayPointman::GetIconDescription(int index) {
  wxString *pret = NULL;

  if (index >= 0) {
    MarkIcon *pmi = (MarkIcon *)m_pIconArray->Item(index);
    pret = &pmi->icon_description;
  }
  return pret;
}

wxString WayPointman::GetIconDescription(wxString icon_key) {
  MarkIcon *pmi;
  unsigned int i;

  for (i = 0; i < m_pIconArray->GetCount(); i++) {
    pmi = (MarkIcon *)m_pIconArray->Item(i);
    if (pmi->icon_name.IsSameAs(icon_key))
      return wxString(pmi->icon_description);
  }

  return wxEmptyString;
}

wxString *WayPointman::GetIconKey(int index) {
  wxString *pret = NULL;

  if ((index >= 0) && ((unsigned int)index < m_pIconArray->GetCount())) {
    MarkIcon *pmi = (MarkIcon *)m_pIconArray->Item(index);
    pret = &pmi->icon_name;
  }
  return pret;
}

int WayPointman::GetIconIndex(const wxBitmap *pbm) {
  unsigned int ret = 0;
  MarkIcon *pmi;

  wxASSERT(m_pIconArray->GetCount() >= 1);
  for (unsigned int i = 0; i < m_pIconArray->GetCount(); i++) {
    pmi = (MarkIcon *)m_pIconArray->Item(i);
    if (pmi->piconBitmap == pbm) {
      ret = i;
      break;
    }
  }

  return ret;
}

int WayPointman::GetIconImageListIndex(const wxBitmap *pbm) {
  MarkIcon *pmi = (MarkIcon *)m_pIconArray->Item(GetIconIndex(pbm));

  // Build a "list - sized" image
  if (pmarkicon_image_list && !pmi->m_blistImageOK) {
    int h0 = pmi->iconImage.GetHeight();
    int w0 = pmi->iconImage.GetWidth();
    int h = m_bitmapSizeForList;
    int w = m_bitmapSizeForList;

    wxImage icon_larger = pmi->iconImage;  // make a copy
    if (h0 <= h && w0 <= w) {
      icon_larger = pmi->iconImage.Resize(
          wxSize(w, h), wxPoint(w / 2 - w0 / 2, h / 2 - h0 / 2));
    } else {
      // rescale in one or two directions to avoid cropping, then resize to fit
      // to cell
      int h1 = h;
      int w1 = w;
      if (h0 > h)
        w1 = wxRound((double)w0 * ((double)h / (double)h0));

      else if (w0 > w)
        h1 = wxRound((double)h0 * ((double)w / (double)w0));

      icon_larger = pmi->iconImage.Rescale(w1, h1);
      icon_larger = icon_larger.Resize(wxSize(w, h),
                                       wxPoint(w / 2 - w1 / 2, h / 2 - h1 / 2));
    }

    int index = pmarkicon_image_list->Add(wxBitmap(icon_larger));

    // Create and replace "x-ed out" and "fixed visibility" icon,
    // Being careful to preserve (some) transparency

    icon_larger.ConvertAlphaToMask(128);

    unsigned char r, g, b;
    icon_larger.GetOrFindMaskColour(&r, &g, &b);
    wxColour unused_color(r, g, b);

    // X-out
    wxBitmap xIcon(icon_larger);

    wxBitmap xbmp(w, h, -1);
    wxMemoryDC mdc(xbmp);
    mdc.SetBackground(wxBrush(unused_color));
    mdc.Clear();
    mdc.DrawBitmap(xIcon, 0, 0);
    int xm = xbmp.GetWidth() / 2;
    int ym = xbmp.GetHeight() / 2;
    int dp = xm / 2;
    int width = wxMax(xm / 10, 2);
    wxPen red(GetGlobalColor(_T( "URED" )), width);
    mdc.SetPen(red);
    mdc.DrawLine(xm - dp, ym - dp, xm + dp, ym + dp);
    mdc.DrawLine(xm - dp, ym + dp, xm + dp, ym - dp);
    mdc.SelectObject(wxNullBitmap);

    wxMask *pmask = new wxMask(xbmp, unused_color);
    xbmp.SetMask(pmask);

    pmarkicon_image_list->Add(xbmp);

    // fixed Viz
    wxBitmap fIcon(icon_larger);

    wxBitmap fbmp(w, h, -1);
    wxMemoryDC fmdc(fbmp);
    fmdc.SetBackground(wxBrush(unused_color));
    fmdc.Clear();
    fmdc.DrawBitmap(xIcon, 0, 0);
    xm = fbmp.GetWidth() / 2;
    ym = fbmp.GetHeight() / 2;
    dp = xm / 2;
    width = wxMax(xm / 10, 2);
    wxPen fred(GetGlobalColor(_T( "UGREN" )), width);
    fmdc.SetPen(fred);
    fmdc.DrawLine(xm - dp, ym + dp, xm + dp, ym + dp);
    fmdc.SelectObject(wxNullBitmap);

    wxMask *pfmask = new wxMask(fbmp, unused_color);
    fbmp.SetMask(pfmask);

    pmarkicon_image_list->Add(fbmp);

    pmi->m_blistImageOK = true;
    pmi->listIndex = index;
  }

  return pmi->listIndex;
}

int WayPointman::GetXIconImageListIndex(const wxBitmap *pbm) {
  return GetIconImageListIndex(pbm) +
         1;  // index of "X-ed out" icon in the image list
}

int WayPointman::GetFIconImageListIndex(const wxBitmap *pbm) {
  return GetIconImageListIndex(pbm) +
         2;  // index of "fixed viz" icon in the image list
}

//  Create the unique identifier
wxString WayPointman::CreateGUID(RoutePoint *pRP) {
  // FIXME: this method is not needed at all (if GetUUID works...)
  /*wxDateTime now = wxDateTime::Now();
   time_t ticks = now.GetTicks();
   wxString GUID;
   GUID.Printf(_T("%d-%d-%d-%d"), ((int)fabs(pRP->m_lat * 1e4)),
   ((int)fabs(pRP->m_lon * 1e4)), (int)ticks, m_nGUID);

   m_nGUID++;

   return GUID;*/
  return GpxDocument::GetUUID();
}

RoutePoint *WayPointman::FindRoutePointByGUID(const wxString &guid) {
  wxRoutePointListNode *prpnode = m_pWayPointList->GetFirst();
  while (prpnode) {
    RoutePoint *prp = prpnode->GetData();

    if (prp->m_GUID == guid) return (prp);

    prpnode = prpnode->GetNext();  // RoutePoint
  }

  return NULL;
}

RoutePoint *WayPointman::GetNearbyWaypoint(double lat, double lon,
                                           double radius_meters) {
  //    Iterate on the RoutePoint list, checking distance

  wxRoutePointListNode *node = m_pWayPointList->GetFirst();
  while (node) {
    RoutePoint *pr = node->GetData();

    double a = lat - pr->m_lat;
    double b = lon - pr->m_lon;
    double l = sqrt((a * a) + (b * b));

    if ((l * 60. * 1852.) < radius_meters) return pr;

    node = node->GetNext();
  }
  return NULL;
}

RoutePoint *WayPointman::GetOtherNearbyWaypoint(double lat, double lon,
                                                double radius_meters,
                                                const wxString &guid) {
  //    Iterate on the RoutePoint list, checking distance

  wxRoutePointListNode *node = m_pWayPointList->GetFirst();
  while (node) {
    RoutePoint *pr = node->GetData();

    double a = lat - pr->m_lat;
    double b = lon - pr->m_lon;
    double l = sqrt((a * a) + (b * b));

    if ((l * 60. * 1852.) < radius_meters)
      if (pr->m_GUID != guid) return pr;

    node = node->GetNext();
  }
  return NULL;
}

bool WayPointman::IsReallyVisible(RoutePoint *pWP) {
  if (pWP->m_bIsolatedMark)
    return pWP->IsVisible();  // isolated point
  else {
    wxRouteListNode *node = pRouteList->GetFirst();
    while (node) {
      Route *proute = node->GetData();
      if (proute && proute->pRoutePointList) {
        if (proute->pRoutePointList->IndexOf(pWP) != wxNOT_FOUND) {
          if (proute->IsVisible()) return true;
        }
      }
      node = node->GetNext();
    }
  }
  if (pWP->IsShared())  // is not visible as part of route, but still exists as
                        // a waypoint
    return pWP->IsVisible();  // so treat as isolated point

  return false;
}

void WayPointman::ClearRoutePointFonts(void) {
  //    Iterate on the RoutePoint list, clearing Font pointers
  //    This is typically done globally after a font switch

  wxRoutePointListNode *node = m_pWayPointList->GetFirst();
  while (node) {
    RoutePoint *pr = node->GetData();

    pr->m_pMarkFont = NULL;
    node = node->GetNext();
  }
}

bool WayPointman::SharedWptsExist() {
  wxRoutePointListNode *node = m_pWayPointList->GetFirst();
  while (node) {
    RoutePoint *prp = node->GetData();
    if (prp->IsShared() && (prp->m_bIsInRoute || prp == pAnchorWatchPoint1 ||
                            prp == pAnchorWatchPoint2))
      return true;
    node = node->GetNext();
  }
  return false;
}

void WayPointman::DeleteAllWaypoints(bool b_delete_used) {
  //    Iterate on the RoutePoint list, deleting all
  wxRoutePointListNode *node = m_pWayPointList->GetFirst();
  while (node) {
    RoutePoint *prp = node->GetData();
    // if argument is false, then only delete non-route waypoints
    if (!prp->m_bIsInLayer && (prp->GetIconName() != _T("mob")) &&
        ((b_delete_used && prp->IsShared()) ||
         ((!prp->m_bIsInRoute) && !(prp == pAnchorWatchPoint1) &&
          !(prp == pAnchorWatchPoint2)))) {
      DestroyWaypoint(prp);
      delete prp;
      node = m_pWayPointList->GetFirst();
    } else
      node = node->GetNext();
  }
  return;
}

void WayPointman::DestroyWaypoint(RoutePoint *pRp, bool b_update_changeset) {
  if (!b_update_changeset)
    NavObjectChanges::getInstance()->m_bSkipChangeSetUpdate = true;
    // turn OFF change-set updating if requested

  if (pRp) {
    // Get a list of all routes containing this point
    // and remove the point from them all
    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining(pRp);
    if (proute_array) {
      for (unsigned int ir = 0; ir < proute_array->GetCount(); ir++) {
        Route *pr = (Route *)proute_array->Item(ir);

        /*  FS#348
         if ( g_pRouteMan->GetpActiveRoute() == pr )            // Deactivate
         any route containing this point g_pRouteMan->DeactivateRoute();
         */
        pr->RemovePoint(pRp);
      }

      //    Scrub the routes, looking for one-point routes
      for (unsigned int ir = 0; ir < proute_array->GetCount(); ir++) {
        Route *pr = (Route *)proute_array->Item(ir);
        if (pr->GetnPoints() < 2) {
          bool prev_bskip =
              NavObjectChanges::getInstance()->m_bSkipChangeSetUpdate;
          NavObjectChanges::getInstance()->m_bSkipChangeSetUpdate = true;
          NavObjectChanges::getInstance()->DeleteConfigRoute(pr);
          g_pRouteMan->DeleteRoute(pr);
          NavObjectChanges::getInstance()->m_bSkipChangeSetUpdate = prev_bskip;
        }
      }

      delete proute_array;
    }

    // Now it is safe to delete the point
    NavObjectChanges::getInstance()->DeleteWayPoint(pRp);
    NavObjectChanges::getInstance()->m_bSkipChangeSetUpdate = false;

    pSelect->DeleteSelectableRoutePoint(pRp);

    //    The RoutePoint might be currently in use as an anchor watch point
    if (pRp == pAnchorWatchPoint1) pAnchorWatchPoint1 = NULL;
    if (pRp == pAnchorWatchPoint2) pAnchorWatchPoint2 = NULL;

    RemoveRoutePoint(pRp);
  }
}
