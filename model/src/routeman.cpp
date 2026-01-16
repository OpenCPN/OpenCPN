/***************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement routeman.h -- route manager.
 */

#include <algorithm>
#include <cmath>
#include <list>
#include <memory>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include <wx/wxprec.h>
#include <wx/image.h>
#include <wx/jsonval.h>

#include "model/ais_decoder.h"
#include "model/autopilot_output.h"
#include "model/base_platform.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_n0183_output.h"
#include "model/config_vars.h"
#include "model/georef.h"
#include "model/nav_object_database.h"
#include "model/navutil_base.h"
#include "model/navobj_db.h"
#include "model/nmea_ctx_factory.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/track.h"

#include "observable_globvar.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

bool g_bPluginHandleAutopilotRoute;

Routeman *g_pRouteMan;
Route *pAISMOBRoute;

RoutePoint *pAnchorWatchPoint1;
RoutePoint *pAnchorWatchPoint2;

RouteList *pRouteList;

float g_ChartScaleFactorExp;

// Helper conditional file name dir slash
void appendOSDirSlash(wxString *pString);

static void ActivatePersistedRoute(Routeman *routeman) {
  if (g_active_route == "") {
    wxLogWarning("\"Persist route\" but no persisted route configured");
    return;
  }
  Route *route = routeman->FindRouteByGUID(g_active_route);
  if (!route) {
    wxLogWarning("Persisted route GUID not available");
    return;
  }
  routeman->ActivateRoute(route);  // FIXME (leamas) better start point
}

//--------------------------------------------------------------------------------
//      Routeman   "Route Manager"
//--------------------------------------------------------------------------------

Routeman::Routeman(struct RoutePropDlgCtx ctx,
                   struct RoutemanDlgCtx route_dlg_ctx)
    : pActiveRoute(0),
      pActivePoint(0),
      pRouteActivatePoint(0),
      m_NMEA0183(NmeaCtxFactory()),
      m_prop_dlg_ctx(ctx),
      m_route_dlg_ctx(route_dlg_ctx) {
  GlobalVar<wxString> active_route(&g_active_route);
  auto route_action = [&](wxCommandEvent) {
    if (g_persist_active_route) ActivatePersistedRoute(this);
  };
  active_route_listener.Init(active_route, route_action);
}

Routeman::~Routeman() {
  if (pRouteActivatePoint) delete pRouteActivatePoint;
}

bool Routeman::IsRouteValid(Route *pRoute) {
  for (Route *route : *pRouteList) {
    if (pRoute == route) return true;
  }
  return false;
}

//    Make a 2-D search to find the route containing a given waypoint
Route *Routeman::FindRouteContainingWaypoint(RoutePoint *pWP) {
  for (Route *proute : *pRouteList) {
    for (RoutePoint *prp : *proute->pRoutePointList) {
      if (prp == pWP) return proute;
    }
  }
  return NULL;  // not found
}

//    Make a 2-D search to find the route containing a given waypoint, by GUID
Route *Routeman::FindRouteContainingWaypoint(const std::string &guid) {
  for (Route *proute : *pRouteList) {
    for (RoutePoint *prp : *proute->pRoutePointList) {
      if (prp->m_GUID == guid) return proute;
    }
  }

  return NULL;  // not found
}

//    Make a 2-D search to find the visual route containing a given waypoint
Route *Routeman::FindVisibleRouteContainingWaypoint(RoutePoint *pWP) {
  for (Route *proute : *pRouteList) {
    if (proute->IsVisible()) {
      for (RoutePoint *prp : *proute->pRoutePointList) {
        if (prp == pWP) return proute;
      }
    }
  }

  return NULL;  // not found
}

wxArrayPtrVoid *Routeman::GetRouteArrayContaining(RoutePoint *pWP) {
  wxArrayPtrVoid *pArray = new wxArrayPtrVoid;

  for (Route *proute : *pRouteList) {
    for (RoutePoint *prp : *proute->pRoutePointList) {
      if (prp == pWP) {  // success
        pArray->Add((void *)proute);
        break;  // only add a route to the array once, even if there are
                // duplicate points in the route...See FS#1743
      }
    }
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
    g_pRouteMan->DeleteRoute(route);
    route = NULL;
  }
  //  Add this point back into the selectables
  pSelect->AddSelectableRoutePoint(point->m_lat, point->m_lon, point);

  // if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
  //   pRoutePropDialog->SetRouteAndUpdate(route, true);
  // }
  m_prop_dlg_ctx.set_route_and_update(route);
}

RoutePoint *Routeman::FindBestActivatePoint(Route *pR, double lat, double lon,
                                            double cog, double sog) {
  if (!pR) return NULL;

  // Walk thru all the points to find the "best"
  RoutePoint *best_point = NULL;
  double min_time_found = 1e6;

  for (RoutePoint *pn : *pR->pRoutePointList) {
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
  }
  return best_point;
}

bool Routeman::ActivateRoute(Route *pRouteToActivate, RoutePoint *pStartPoint) {
  g_bAllowShipToActive = false;
  wxJSONValue v;
  v["Route_activated"] = pRouteToActivate->m_RouteNameString;
  v["GUID"] = pRouteToActivate->m_GUID;
  json_msg.Notify(std::make_shared<wxJSONValue>(v), "OCPN_RTE_ACTIVATED");
  if (g_bPluginHandleAutopilotRoute) return true;

  // Capture and maintain a list of data connections configured as "output"
  // This is performed on "Activate()" to allow dynamic re-config of drivers
  m_have_n0183_out = false;
  m_have_n2000_out = false;

  m_output_drivers.clear();
  for (const auto &handle : GetActiveDrivers()) {
    const auto &attributes = GetAttributes(handle);
    if (attributes.find("protocol") == attributes.end()) continue;
    if (attributes.at("protocol") == "nmea0183") {
      if (attributes.find("ioDirection") != attributes.end()) {
        if ((attributes.at("ioDirection") == "IN/OUT") ||
            (attributes.at("ioDirection") == "OUT")) {
          m_output_drivers.push_back(handle);
          m_have_n0183_out = true;
        }
      }
      continue;
    }
    // Check N2K dirvers for OUTPUT configuration
    if (attributes.find("protocol") == attributes.end()) continue;
    if (attributes.at("protocol") == "nmea2000") {
      if (attributes.find("ioDirection") != attributes.end()) {
        if ((attributes.at("ioDirection") == "IN/OUT") ||
            (attributes.at("ioDirection") == "OUT")) {
          m_output_drivers.push_back(handle);
          m_have_n2000_out = true;
        }
      }
      continue;
    }
  }

  pActiveRoute = pRouteToActivate;
  g_active_route = pActiveRoute->GetGUID();

  if (pStartPoint) {
    pActivePoint = pStartPoint;
  } else {
    pActivePoint = *pActiveRoute->pRoutePointList->begin();
  }

  ActivateRoutePoint(pRouteToActivate, pActivePoint);

  m_bArrival = false;
  m_arrival_min = 1e6;
  m_arrival_test = 0;

  pRouteToActivate->m_bRtIsActive = true;

  m_bDataValid = false;

  m_route_dlg_ctx.show_with_fresh_fonts();
  return true;
}

bool Routeman::ActivateRoutePoint(Route *pA, RoutePoint *pRP_target) {
  g_bAllowShipToActive = false;
  wxJSONValue v;
  v["GUID"] = pRP_target->m_GUID;
  v["WP_activated"] = pRP_target->GetName();

  json_msg.Notify(std::make_shared<wxJSONValue>(v), "OCPN_WPT_ACTIVATED");

  if (g_bPluginHandleAutopilotRoute) return true;

  pActiveRoute = pA;

  pActivePoint = pRP_target;
  pActiveRoute->m_pRouteActivePoint = pRP_target;

  for (RoutePoint *pn : *pActiveRoute->pRoutePointList) {
    pn->m_bBlink = false;  // turn off all blinking points
    pn->m_bIsActive = false;
  }

  auto node = (pActiveRoute->pRoutePointList)->begin();
  RoutePoint *prp_first = *node;

  //  If activating first point in route, create a "virtual" waypoint at present
  //  position
  if (pRP_target == prp_first) {
    if (pRouteActivatePoint) delete pRouteActivatePoint;

    pRouteActivatePoint =
        new RoutePoint(gLat, gLon, wxString(""), wxString("Begin"), "",
                       false);  // Current location
    pRouteActivatePoint->m_bShowName = false;

    pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
  }

  else {
    prp_first->m_bBlink = false;
    ++node;
    RoutePoint *np_prev = prp_first;
    while (node != pActiveRoute->pRoutePointList->end()) {
      RoutePoint *pnext = *node;
      if (pnext == pRP_target) {
        pActiveRouteSegmentBeginPoint = np_prev;
        break;
      }

      np_prev = pnext;
      ++node;
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
  m_prop_dlg_ctx.set_enroute_point(pA, pActivePoint);
  return true;
}

bool Routeman::ActivateNextPoint(Route *pr, bool skipped) {
  g_bAllowShipToActive = false;
  wxJSONValue v;
  bool result = false;
  if (pActivePoint) {
    pActivePoint->m_bBlink = false;
    pActivePoint->m_bIsActive = false;

    v["isSkipped"] = skipped;
    v["GUID"] = pActivePoint->m_GUID;
    v["GUID_WP_arrived"] = pActivePoint->m_GUID;
    v["WP_arrived"] = pActivePoint->GetName();
  }
  int n_index_active = pActiveRoute->GetIndexOf(pActivePoint);
  if (n_index_active < 0) return false;
  int step = 1;
  while (n_index_active == pActiveRoute->GetIndexOf(pActivePoint)) {
    int candidate = n_index_active + step;
    if (candidate < pActiveRoute->GetnPoints()) {
      int candidate_point = candidate + 1;  // GetPoint expects 1-based
      pActiveRouteSegmentBeginPoint = pActivePoint;
      pActiveRoute->m_pRouteActivePoint =
          pActiveRoute->GetPoint(candidate_point);
      pActivePoint = pActiveRoute->GetPoint(candidate_point);
      step++;
      result = true;
    } else {
      n_index_active = -1;  // stop the while loop
      result = false;
    }
  }
  if (result) {
    v["Next_WP"] = pActivePoint->GetName();
    v["GUID_Next_WP"] = pActivePoint->m_GUID;

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
    m_prop_dlg_ctx.set_enroute_point(pr, pActivePoint);

    json_msg.Notify(std::make_shared<wxJSONValue>(v), "OCPN_WPT_ARRIVED");
  }
  return result;
}

bool Routeman::DeactivateRoute(bool b_arrival) {
  if (pActivePoint) {
    pActivePoint->m_bBlink = false;
    pActivePoint->m_bIsActive = false;
  }

  if (pActiveRoute) {
    pActiveRoute->m_bRtIsActive = false;
    pActiveRoute->m_pRouteActivePoint = NULL;
    g_active_route.Clear();

    wxJSONValue v;
    if (!b_arrival) {
      v["Route_deactivated"] = pActiveRoute->m_RouteNameString;
      v["GUID"] = pActiveRoute->m_GUID;
      json_msg.Notify(std::make_shared<wxJSONValue>(v), "OCPN_RTE_DEACTIVATED");
    } else {
      v["GUID"] = pActiveRoute->m_GUID;
      v["Route_ended"] = pActiveRoute->m_RouteNameString;
      json_msg.Notify(std::make_shared<wxJSONValue>(v), "OCPN_RTE_ENDED");
    }
  }

  pActiveRoute = NULL;

  if (pRouteActivatePoint) delete pRouteActivatePoint;
  pRouteActivatePoint = NULL;

  pActivePoint = NULL;

  m_route_dlg_ctx.clear_console_background();
  m_bDataValid = false;

  return true;
}

bool Routeman::UpdateAutopilot() {
  if (!pActiveRoute) return false;

  if (!bGPSValid) return false;
  bool rv = false;

  // Set max WP name length
  int maxName = 6;
  if ((g_maxWPNameLength >= 3) && (g_maxWPNameLength <= 32))
    maxName = g_maxWPNameLength;

  if (m_have_n0183_out) rv |= UpdateAutopilotN0183(*this);
  if (m_have_n2000_out) rv |= UpdateAutopilotN2K(*this);

  // Route may have been deactivated or deleted during the
  // N2K port setup conversation.  The message loop runs...
  if (!pActiveRoute) return false;

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

  json_leg_info.Notify(std::make_shared<ActiveLegDat>(leg_info), "");

#if 0


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

  json_leg_info.Notify(std::make_shared<ActiveLegDat>(leg_info), "");

  // RMB
  {
    m_NMEA0183.TalkerID = "EC";
    SENTENCE snt;
    m_NMEA0183.Rmb.IsDataValid = bGPSValid ? NTrue : NFalse;
    m_NMEA0183.Rmb.CrossTrackError = CurrentXTEToActivePoint;
    m_NMEA0183.Rmb.DirectionToSteer = XTEDir < 0 ? Left : Right;
    m_NMEA0183.Rmb.RangeToDestinationNauticalMiles = CurrentRngToActivePoint;
    m_NMEA0183.Rmb.BearingToDestinationDegreesTrue = CurrentBrgToActivePoint;

    if (pActivePoint->m_lat < 0.)
      m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(-pActivePoint->m_lat,
                                                      "S");
    else
      m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(pActivePoint->m_lat, "N");

    if (pActivePoint->m_lon < 0.)
      m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(-pActivePoint->m_lon,
                                                       "W");
    else
      m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(pActivePoint->m_lon,
                                                       "E");

    m_NMEA0183.Rmb.DestinationClosingVelocityKnots =
        r_Sog * cos((r_Cog - CurrentBrgToActivePoint) * PI / 180.0);
    m_NMEA0183.Rmb.IsArrivalCircleEntered = m_bArrival ? NTrue : NFalse;
    m_NMEA0183.Rmb.FAAModeIndicator = bGPSValid ? "A" : "N";
    // RMB is close to NMEA0183 length limit
    // Restrict WP names further if necessary
    int wp_len = maxName;
    do {
      m_NMEA0183.Rmb.To = pActivePoint->GetName().Truncate(wp_len);
      m_NMEA0183.Rmb.From =
          pActiveRouteSegmentBeginPoint->GetName().Truncate(wp_len);
      m_NMEA0183.Rmb.Write(snt);
      wp_len -= 1;
    } while (snt.Sentence.size() > 82 && wp_len > 0);

    BroadcastNMEA0183Message(snt.Sentence, *m_nmea_log, on_message_sent);
  }

  // RMC
  {
    m_NMEA0183.TalkerID = "EC";

    SENTENCE snt;
    m_NMEA0183.Rmc.IsDataValid = NTrue;
    if (!bGPSValid) m_NMEA0183.Rmc.IsDataValid = NFalse;

    if (gLat < 0.)
      m_NMEA0183.Rmc.Position.Latitude.Set(-gLat, "S");
    else
      m_NMEA0183.Rmc.Position.Latitude.Set(gLat, "N");

    if (gLon < 0.)
      m_NMEA0183.Rmc.Position.Longitude.Set(-gLon, "W");
    else
      m_NMEA0183.Rmc.Position.Longitude.Set(gLon, "E");

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
      wxString time = utc.Format("%H%M%S");
      m_NMEA0183.Rmc.UTCTime = time;
      wxString date = utc.Format("%d%m%y");
      m_NMEA0183.Rmc.Date = date;
    }

    m_NMEA0183.Rmc.FAAModeIndicator = "A";
    if (!bGPSValid) m_NMEA0183.Rmc.FAAModeIndicator = "N";

    m_NMEA0183.Rmc.Write(snt);

    BroadcastNMEA0183Message(snt.Sentence, *m_nmea_log, on_message_sent);
  }

  // APB
  {
    m_NMEA0183.TalkerID = "EC";

    SENTENCE snt;

    m_NMEA0183.Apb.IsLoranBlinkOK =
        NTrue;  // considered as "generic invalid fix" flag
    if (!bGPSValid) m_NMEA0183.Apb.IsLoranBlinkOK = NFalse;

    m_NMEA0183.Apb.IsLoranCCycleLockOK = NTrue;
    if (!bGPSValid) m_NMEA0183.Apb.IsLoranCCycleLockOK = NFalse;

    m_NMEA0183.Apb.CrossTrackErrorMagnitude = CurrentXTEToActivePoint;

    if (XTEDir < 0)
      m_NMEA0183.Apb.DirectionToSteer = Left;
    else
      m_NMEA0183.Apb.DirectionToSteer = Right;

    m_NMEA0183.Apb.CrossTrackUnits = "N";

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
      m_NMEA0183.Apb.BearingOriginToDestinationUnits = "M";

      m_NMEA0183.Apb.BearingPresentPositionToDestination = bapm;
      m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = "M";

      m_NMEA0183.Apb.HeadingToSteer = bapm;
      m_NMEA0183.Apb.HeadingToSteerUnits = "M";
    } else {
      m_NMEA0183.Apb.BearingOriginToDestination = brg1;
      m_NMEA0183.Apb.BearingOriginToDestinationUnits = "T";

      m_NMEA0183.Apb.BearingPresentPositionToDestination =
          CurrentBrgToActivePoint;
      m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = "T";

      m_NMEA0183.Apb.HeadingToSteer = CurrentBrgToActivePoint;
      m_NMEA0183.Apb.HeadingToSteerUnits = "T";
    }

    m_NMEA0183.Apb.Write(snt);
    BroadcastNMEA0183Message(snt.Sentence, *m_nmea_log, on_message_sent);
  }

  // XTE
  {
    m_NMEA0183.TalkerID = "EC";

    SENTENCE snt;

    m_NMEA0183.Xte.IsLoranBlinkOK =
        NTrue;  // considered as "generic invalid fix" flag
    if (!bGPSValid) m_NMEA0183.Xte.IsLoranBlinkOK = NFalse;

    m_NMEA0183.Xte.IsLoranCCycleLockOK = NTrue;
    if (!bGPSValid) m_NMEA0183.Xte.IsLoranCCycleLockOK = NFalse;

    m_NMEA0183.Xte.CrossTrackErrorDistance = CurrentXTEToActivePoint;

    if (XTEDir < 0)
      m_NMEA0183.Xte.DirectionToSteer = Left;
    else
      m_NMEA0183.Xte.DirectionToSteer = Right;

    m_NMEA0183.Xte.CrossTrackUnits = "N";

    m_NMEA0183.Xte.Write(snt);
    BroadcastNMEA0183Message(snt.Sentence, *m_nmea_log, on_message_sent);
  }
#endif

  return true;
}

bool Routeman::DoesRouteContainSharedPoints(Route *pRoute) {
  if (pRoute) {
    // walk the route, looking at each point to see if it is used by another
    // route or is isolated
    for (RoutePoint *prp : *pRoute->pRoutePointList) {
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
        delete pRA;
      }
    }

    // Now walk the route again, looking for isolated type shared waypoints
    for (RoutePoint *prp : *pRoute->pRoutePointList) {
      if (prp->IsShared()) return true;
    }
  }

  return false;
}

bool Routeman::DeleteTrack(Track *pTrack) {
  if (pTrack && !pTrack->m_bIsInLayer) {
    ::wxBeginBusyCursor();
    /*
    wxGenericProgressDialog *pprog = nullptr;

    int count = pTrack->GetnPoints();
    if (count > 10000) {
      pprog = new wxGenericProgressDialog(
          _("OpenCPN Track Delete"), "0/0", count, NULL,
          wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_ELAPSED_TIME |
              wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
      pprog->SetSize(400, wxDefaultCoord);
      pprog->Centre();
    }
    */

    //    Remove the track from associated lists
    pSelect->DeleteAllSelectableTrackSegments(pTrack);
    auto it = std::find(g_TrackList.begin(), g_TrackList.end(), pTrack);
    if (it != g_TrackList.end()) {
      g_TrackList.erase(it);
    }
    delete pTrack;

    ::wxEndBusyCursor();

    // delete pprog;
    return true;
  }
  return false;
}

bool Routeman::DeleteRoute(Route *pRoute) {
  if (pRoute) {
    if (pRoute == pAISMOBRoute) {
      if (!m_route_dlg_ctx.confirm_delete_ais_mob()) {
        return false;
      }
      pAISMOBRoute = 0;
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
    m_prop_dlg_ctx.hide(pRoute);

    // if (nav_obj_changes) nav_obj_changes->DeleteConfigRoute(pRoute);

    //    Remove the route from associated lists
    pSelect->DeleteAllSelectableRouteSegments(pRoute);
    auto pos = std::find(pRouteList->begin(), pRouteList->end(), pRoute);
    if (pos != pRouteList->end()) pRouteList->erase(pos);

    m_route_dlg_ctx.route_mgr_dlg_update_list_ctrl();

    // walk the route, tentatively deleting/marking points used only by this
    // route
    auto &list = pRoute->pRoutePointList;
    auto pnode = list->begin();
    while (pnode != list->end()) {
      RoutePoint *prp = *pnode;

      // check all other routes to see if this point appears in any other route
      Route *pcontainer_route = FindRouteContainingWaypoint(prp);

      if (pcontainer_route == NULL && prp->m_bIsInRoute) {
        prp->m_bIsInRoute =
            false;  // Take this point out of this (and only) route
        if (!prp->IsShared()) {
          pSelect->DeleteSelectablePoint(prp, SELTYPE_ROUTEPOINT);

          // Remove all instances of this point from the list.
          auto pdnode = pnode;
          while (pdnode != list->end()) {
            pRoute->pRoutePointList->erase(pdnode);
            pdnode = std::find(list->begin(), list->end(), prp);
          }

          pnode = list->end();
          NavObj_dB::GetInstance().DeleteRoutePoint(prp);
          delete prp;
        } else {
          prp->m_bIsolatedMark = true;  // This has become an isolated mark
          prp->SetShared(false);        // and is no longer part of a route
          NavObj_dB::GetInstance().UpdateRoutePoint(prp);
        }
      }
      if (pnode != list->end())
        ++pnode;
      else
        pnode = list->begin();
    }

    NavObj_dB::GetInstance().DeleteRoute(pRoute);
    delete pRoute;

    ::wxEndBusyCursor();
  }
  return true;
}

void Routeman::DeleteAllRoutes() {
  ::wxBeginBusyCursor();

  //    Iterate on the RouteList
  for (Route *proute : *pRouteList) {
    if (proute == pAISMOBRoute) {
      if (!m_route_dlg_ctx.confirm_delete_ais_mob()) {
        return;
      }
      pAISMOBRoute = 0;
      ::wxBeginBusyCursor();
    }
    if (proute->m_bIsInLayer) continue;

    DeleteRoute(proute);
  }

  ::wxEndBusyCursor();
}

void Routeman::SetColorScheme(ColorScheme cs, double displayDPmm) {
  // Re-Create the pens and colors

  int scaled_line_width = g_route_line_width;
  int track_scaled_line_width = g_track_line_width;
  if (g_btouch) {
    // 0.4 mm nominal, but not less than 2 pixel
    double nominal_line_width_pix = wxMax(2.0, floor(displayDPmm * 0.4));

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

  m_pRoutePen =
      wxThePenList->FindOrCreatePen(m_route_dlg_ctx.get_global_colour("UINFB"),
                                    scaled_line_width, wxPENSTYLE_SOLID);
  m_pSelectedRoutePen =
      wxThePenList->FindOrCreatePen(m_route_dlg_ctx.get_global_colour("UINFO"),
                                    scaled_line_width, wxPENSTYLE_SOLID);
  m_pActiveRoutePen =
      wxThePenList->FindOrCreatePen(m_route_dlg_ctx.get_global_colour("UARTE"),
                                    scaled_line_width, wxPENSTYLE_SOLID);
  m_pTrackPen =
      wxThePenList->FindOrCreatePen(m_route_dlg_ctx.get_global_colour("CHMGD"),
                                    track_scaled_line_width, wxPENSTYLE_SOLID);
  m_pRouteBrush = wxTheBrushList->FindOrCreateBrush(
      m_route_dlg_ctx.get_global_colour("UINFB"), wxBRUSHSTYLE_SOLID);
  m_pSelectedRouteBrush = wxTheBrushList->FindOrCreateBrush(
      m_route_dlg_ctx.get_global_colour("UINFO"), wxBRUSHSTYLE_SOLID);
  m_pActiveRouteBrush = wxTheBrushList->FindOrCreateBrush(
      m_route_dlg_ctx.get_global_colour("PLRTE"), wxBRUSHSTYLE_SOLID);
}

wxString Routeman::GetRouteReverseMessage() {
  return wxString(
      _("Waypoints can be renamed to reflect the new order, the names will be "
        "'001', '002' etc.\n\nDo you want to rename the waypoints?"));
}

wxString Routeman::GetRouteResequenceMessage() {
  return wxString(
      _("Waypoints will be renamed to reflect the natural order, the names "
        "will be '001', '002' etc.\n\nDo you want to rename the waypoints?"));
}

Route *Routeman::FindRouteByGUID(const wxString &guid) {
  for (Route *pRoute : *pRouteList) {
    if (pRoute->m_GUID == guid) return pRoute;
  }
  return NULL;
}

Track *Routeman::FindTrackByGUID(const wxString &guid) {
  for (Track *pTrack : g_TrackList) {
    if (pTrack->m_GUID == guid) return pTrack;
  }
  return NULL;
}

void Routeman::ZeroCurrentXTEToActivePoint() {
  // When zeroing XTE create a "virtual" waypoint at present position
  if (pRouteActivatePoint) delete pRouteActivatePoint;
  pRouteActivatePoint = new RoutePoint(gLat, gLon, wxString(""), wxString(""),
                                       "", false);  // Current location
  pRouteActivatePoint->m_bShowName = false;

  pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
  m_arrival_min = 1e6;
}

//--------------------------------------------------------------------------------
//      WayPointman   Implementation
//--------------------------------------------------------------------------------

WayPointman::WayPointman(GlobalColourFunc color_func)
    : m_get_global_colour(color_func) {
  m_pWayPointList = new RoutePointList;

  pmarkicon_image_list = NULL;

  // ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
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

  for (RoutePoint *pr : *m_pWayPointList) {
    temp_list.push_back(pr);
  }

  for (RoutePoint *rp : temp_list) delete rp;
  temp_list.clear();

  m_pWayPointList->clear();
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
  m_pLegacyIconArray->Clear();
  delete m_pLegacyIconArray;
  m_pExtendedIconArray->Clear();
  delete m_pExtendedIconArray;
}

bool WayPointman::AddRoutePoint(RoutePoint *prp) {
  if (!prp) return false;

  m_pWayPointList->push_back(prp);
  prp->SetManagerListNode(prp);

  return true;
}

bool WayPointman::RemoveRoutePoint(RoutePoint *prp) {
  if (!prp) return false;

  auto *prpnode = (RoutePoint *)(prp->GetManagerListNode());

  if (prpnode) {
    auto pos =
        std::find(m_pWayPointList->begin(), m_pWayPointList->end(), prpnode);
    if (pos != m_pWayPointList->end()) m_pWayPointList->erase(pos);
  } else {
    auto pos = std::find(m_pWayPointList->begin(), m_pWayPointList->end(), prp);
    if (pos != m_pWayPointList->end()) m_pWayPointList->erase(pos);
  }

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

wxBitmap *WayPointman::GetIconBitmap(const wxString &icon_key) const {
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
      //            if( pmi->icon_name.IsSameAs( "circle" ) )
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

bool WayPointman::GetIconPrescaled(const wxString &icon_key) const {
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
      //            if( pmi->icon_name.IsSameAs( "circle" ) )
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

wxBitmap WayPointman::GetIconBitmapForList(int index, int height) const {
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

wxString *WayPointman::GetIconDescription(int index) const {
  wxString *pret = NULL;

  if (index >= 0) {
    MarkIcon *pmi = (MarkIcon *)m_pIconArray->Item(index);
    pret = &pmi->icon_description;
  }
  return pret;
}

wxString WayPointman::GetIconDescription(wxString icon_key) const {
  MarkIcon *pmi;
  unsigned int i;

  for (i = 0; i < m_pIconArray->GetCount(); i++) {
    pmi = (MarkIcon *)m_pIconArray->Item(i);
    if (pmi->icon_name.IsSameAs(icon_key))
      return wxString(pmi->icon_description);
  }

  return "";
}

wxString *WayPointman::GetIconKey(int index) const {
  wxString *pret = NULL;

  if ((index >= 0) && ((unsigned int)index < m_pIconArray->GetCount())) {
    MarkIcon *pmi = (MarkIcon *)m_pIconArray->Item(index);
    pret = &pmi->icon_name;
  }
  return pret;
}

int WayPointman::GetIconIndex(const wxBitmap *pbm) const {
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

int WayPointman::GetIconImageListIndex(const wxBitmap *pbm) const {
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
      // We want to maintain the aspect ratio of the original image, but need
      // the canvas to fit the fixed cell size rescale in one or two directions
      // to avoid cropping, then resize to fit to cell (Adds border/croops as
      // necessary)
      int h1 = h;
      int w1 = w;
      if (h0 > h)
        w1 = wxRound((double)w0 * ((double)h / (double)h0));

      else if (w0 > w)
        h1 = wxRound((double)h0 * ((double)w / (double)w0));

      icon_larger = pmi->iconImage.Rescale(w1, h1).Resize(
          wxSize(w, h), wxPoint(w / 2 - w1 / 2, h / 2 - h1 / 2));
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
    wxPen red(m_get_global_colour("URED"), width);
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
    wxPen fred(m_get_global_colour("UGREN"), width);
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

int WayPointman::GetXIconImageListIndex(const wxBitmap *pbm) const {
  return GetIconImageListIndex(pbm) + 1;
}

int WayPointman::GetFIconImageListIndex(const wxBitmap *pbm) const {
  return GetIconImageListIndex(pbm) + 2;
}

//  Create the unique identifier
wxString WayPointman::CreateGUID(RoutePoint *pRP) {
  return GpxDocument::GetUUID();
}

RoutePoint *WayPointman::FindRoutePointByGUID(const wxString &guid) {
  for (RoutePoint *prp : *m_pWayPointList) {
    if (prp->m_GUID == guid) return (prp);
  }
  return NULL;
}

RoutePoint *WayPointman::GetNearbyWaypoint(double lat, double lon,
                                           double radius_meters) {
  //    Iterate on the RoutePoint list, checking distance

  for (RoutePoint *pr : *m_pWayPointList) {
    double a = lat - pr->m_lat;
    double b = lon - pr->m_lon;
    double l = sqrt((a * a) + (b * b));

    if ((l * 60. * 1852.) < radius_meters) return pr;
  }
  return NULL;
}

RoutePoint *WayPointman::GetOtherNearbyWaypoint(double lat, double lon,
                                                double radius_meters,
                                                const wxString &guid) {
  //    Iterate on the RoutePoint list, checking distance

  for (RoutePoint *pr : *m_pWayPointList) {
    double a = lat - pr->m_lat;
    double b = lon - pr->m_lon;
    double l = sqrt((a * a) + (b * b));

    if ((l * 60. * 1852.) < radius_meters)
      if (pr->m_GUID != guid) return pr;
  }
  return NULL;
}

bool WayPointman::IsReallyVisible(RoutePoint *pWP) {
  if (pWP->m_bIsolatedMark) return pWP->IsVisible();  // isolated point
  for (Route *proute : *pRouteList) {
    if (proute && proute->pRoutePointList) {
      auto &list = proute->pRoutePointList;
      auto pos = std::find(list->begin(), list->end(), pWP);
      if (pos != list->end()) {
        if (proute->IsVisible()) return true;
      }
    }
  }
  if (pWP->IsShared())  // is not visible as part of route, but still exists as
                        // a waypoint
    return pWP->IsVisible();  // so treat as isolated point

  return false;
}

void WayPointman::ClearRoutePointFonts() {
  //    Iterate on the RoutePoint list, clearing Font pointers
  //    This is typically done globally after a font switch
  for (RoutePoint *pr : *m_pWayPointList) {
    pr->m_pMarkFont = NULL;
  }
}

bool WayPointman::SharedWptsExist() {
  for (RoutePoint *prp : *m_pWayPointList) {
    if (prp->IsShared() && (prp->m_bIsInRoute || prp == pAnchorWatchPoint1 ||
                            prp == pAnchorWatchPoint2))
      return true;
  }
  return false;
}

void WayPointman::DeleteAllWaypoints(bool b_delete_used) {
  //    Iterate on the RoutePoint list, deleting all
  auto it = m_pWayPointList->begin();
  while (it != m_pWayPointList->end()) {
    RoutePoint *prp = *it;
    // if argument is false, then only delete non-route waypoints
    if (!prp->m_bIsInLayer && (prp->GetIconName() != "mob") &&
        ((b_delete_used && prp->IsShared()) ||
         ((!prp->m_bIsInRoute) && !(prp == pAnchorWatchPoint1) &&
          !(prp == pAnchorWatchPoint2)))) {
      DestroyWaypoint(prp);
      delete prp;
      it = m_pWayPointList->begin();
    } else
      ++it;
  }
  return;
}

RoutePoint *WayPointman::FindWaypointByGuid(const std::string &guid) {
  for (RoutePoint *rp : *m_pWayPointList) {
    if (guid == rp->m_GUID) return rp;
  }
  return 0;
}
void WayPointman::DestroyWaypoint(RoutePoint *pRp, bool b_update_changeset) {
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
          g_pRouteMan->DeleteRoute(pr);
        }
      }

      delete proute_array;
    }

    // Now it is safe to delete the point
    NavObj_dB::GetInstance().DeleteRoutePoint(pRp);

    pSelect->DeleteSelectableRoutePoint(pRp);

    //    The RoutePoint might be currently in use as an anchor watch point
    if (pRp == pAnchorWatchPoint1) pAnchorWatchPoint1 = NULL;
    if (pRp == pAnchorWatchPoint2) pAnchorWatchPoint2 = NULL;

    RemoveRoutePoint(pRp);
  }
}
