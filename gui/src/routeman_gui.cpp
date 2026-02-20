/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 ******************A********************************************************/

/**
 * \file
 *
 * implement routeman_gui.h: Routeman drawing stuff
 */

#include "gl_headers.h"  // Must be included before anything using GL stuff

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/gdicmn.h>
#include <wx/utils.h>

#include "model/ais_decoder.h"
#include "model/config_vars.h"
#include "model/georef.h"
#include "model/gui_vars.h"
#include "model/navobj_db.h"
#include "model/nav_object_database.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/route_point.h"
#include "model/select.h"
#include "model/track.h"

#include "chcanv.h"
#include "color_handler.h"
#include "concanv.h"
#include "navutil.h"
#include "routemanagerdialog.h"
#include "routeman_gui.h"
#include "top_frame.h"
#include "track_prop_dlg.h"
#include "vector2D.h"

static bool ConfirmDeleteAisMob() {
  int r = OCPNMessageBox(NULL,
                         _("You are trying to delete an active AIS MOB "
                           "route, are you REALLY sure?"),
                         _("OpenCPN Warning"), wxYES_NO);

  return r == wxID_YES;
}

RoutemanDlgCtx RoutemanGui::GetDlgCtx() {
  RoutemanDlgCtx ctx;
  ctx.confirm_delete_ais_mob = []() { return ConfirmDeleteAisMob(); };
  ctx.get_global_colour = [](wxString c) { return GetGlobalColor(c); };
  ctx.show_with_fresh_fonts = [] {
    if (console && !g_bhide_route_console) console->ShowWithFreshFonts();
  };
  ctx.clear_console_background = []() {
    console->GetCDI()->ClearBackground();
    console->Show(false);
  };
  ctx.route_mgr_dlg_update_list_ctrl = []() {
    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateRouteListCtrl();
  };
  return ctx;
}

bool RoutemanGui::UpdateProgress() {
  bool bret_val = false;

  if (m_routeman.pActiveRoute) {
    //      Update bearing, range, and crosstrack error

    //  Bearing is calculated as Mercator Sailing, i.e. a  cartographic
    //  "bearing"
    double north, east;
    toSM(m_routeman.pActivePoint->m_lat, m_routeman.pActivePoint->m_lon, gLat,
         gLon, &east, &north);
    double a = atan(north / east);
    if (fabs(m_routeman.pActivePoint->m_lon - gLon) < 180.) {
      if (m_routeman.pActivePoint->m_lon >= gLon)
        m_routeman.CurrentBrgToActivePoint = 90. - (a * 180 / PI);
      else
        m_routeman.CurrentBrgToActivePoint = 270. - (a * 180 / PI);
    } else {
      if (m_routeman.pActivePoint->m_lon >= gLon)
        m_routeman.CurrentBrgToActivePoint = 270. - (a * 180 / PI);
      else
        m_routeman.CurrentBrgToActivePoint = 90. - (a * 180 / PI);
    }

    //      Calculate range using Great Circle Formula

    double d5 = DistGreatCircle(gLat, gLon, m_routeman.pActivePoint->m_lat,
                                m_routeman.pActivePoint->m_lon);
    m_routeman.CurrentRngToActivePoint = d5;

    //      Get the XTE vector, normal to current segment
    vector2D va, vb, vn;

    double brg1, dist1, brg2, dist2;
    DistanceBearingMercator(
        m_routeman.pActivePoint->m_lat, m_routeman.pActivePoint->m_lon,
        m_routeman.pActiveRouteSegmentBeginPoint->m_lat,
        m_routeman.pActiveRouteSegmentBeginPoint->m_lon, &brg1, &dist1);
    vb.x = dist1 * sin(brg1 * PI / 180.);
    vb.y = dist1 * cos(brg1 * PI / 180.);

    DistanceBearingMercator(m_routeman.pActivePoint->m_lat,
                            m_routeman.pActivePoint->m_lon, gLat, gLon, &brg2,
                            &dist2);
    va.x = dist2 * sin(brg2 * PI / 180.);
    va.y = dist2 * cos(brg2 * PI / 180.);

    double sdelta = vGetLengthOfNormal(&va, &vb, &vn);  // NM
    m_routeman.CurrentXTEToActivePoint = sdelta;

    //    Calculate the distance to the arrival line, which is perpendicular to
    //    the current route segment Taking advantage of the calculated normal
    //    from current position to route segment vn
    vector2D vToArriveNormal;
    vSubtractVectors(&va, &vn, &vToArriveNormal);

    m_routeman.CurrentRangeToActiveNormalCrossing =
        vVectorMagnitude(&vToArriveNormal);

    //          Compute current segment course
    //          Using simple Mercater projection
    double x1, y1, x2, y2;
    toSM(m_routeman.pActiveRouteSegmentBeginPoint->m_lat,
         m_routeman.pActiveRouteSegmentBeginPoint->m_lon,
         m_routeman.pActiveRouteSegmentBeginPoint->m_lat,
         m_routeman.pActiveRouteSegmentBeginPoint->m_lon, &x1, &y1);

    toSM(m_routeman.pActivePoint->m_lat, m_routeman.pActivePoint->m_lon,
         m_routeman.pActiveRouteSegmentBeginPoint->m_lat,
         m_routeman.pActiveRouteSegmentBeginPoint->m_lon, &x2, &y2);

    double e1 = atan2((x2 - x1), (y2 - y1));
    m_routeman.CurrentSegmentCourse = e1 * 180 / PI;
    if (m_routeman.CurrentSegmentCourse < 0)
      m_routeman.CurrentSegmentCourse += 360;

    //      Compute XTE direction
    double h = atan(vn.y / vn.x);
    if (vn.x > 0)
      m_routeman.CourseToRouteSegment = 90. - (h * 180 / PI);
    else
      m_routeman.CourseToRouteSegment = 270. - (h * 180 / PI);

    h = m_routeman.CurrentBrgToActivePoint - m_routeman.CourseToRouteSegment;
    if (h < 0) h = h + 360;

    if (h > 180)
      m_routeman.XTEDir = 1;
    else
      m_routeman.XTEDir = -1;

    // Allow DirectShipToActivePoint line (distance XTE in mm is > 3 (arbitrary)
    // or when active point is the first
    if (g_bShowShipToActive) {
      int active_index =
          m_routeman.pActiveRoute->GetIndexOf(m_routeman.pActivePoint);
      if (active_index < 0)
        g_bAllowShipToActive = false;
      else if (active_index == 0)
        g_bAllowShipToActive = true;
      else {
        // compute XTE in pixels
        double tlat, tlon;
        wxPoint r, r1;
        ll_gc_ll(gLat, gLon, m_routeman.CourseToRouteSegment,
                 (m_routeman.CurrentXTEToActivePoint / 1.852), &tlat, &tlon);
        top_frame::Get()->GetCanvasPointPix(gLat, gLon, &r1);
        top_frame::Get()->GetCanvasPointPix(tlat, tlon, &r);
        double xtepix =
            sqrt(pow((double)(r1.x - r.x), 2) + pow((double)(r1.y - r.y), 2));
        // xte in mm
        double xtemm = xtepix / top_frame::Get()->GetPixPerMM();
        // allow display (or not)
        g_bAllowShipToActive = (xtemm > 3.0) ? true : false;
      }
    }

    //      Determine Arrival

    bool bDidArrival = false;

    // Duplicate points can result in NaN for normal crossing range.
    if (isnan(m_routeman.CurrentRangeToActiveNormalCrossing)) {
      m_routeman.CurrentRangeToActiveNormalCrossing =
          m_routeman.CurrentRngToActivePoint;
    }

    // Special signal:  if ArrivalRadius < 0, NEVER arrive...
    //  Used for MOB auto-created routes.
    if (m_routeman.pActivePoint->GetWaypointArrivalRadius() > 0) {
      if (m_routeman.CurrentRangeToActiveNormalCrossing <=
          m_routeman.pActivePoint->GetWaypointArrivalRadius()) {
        m_routeman.m_bArrival = true;
        m_routeman.UpdateAutopilot();

        bDidArrival = true;
        DoAdvance();

      } else {
        //      Test to see if we are moving away from the arrival point, and
        //      have been moving away for 2 seconds.
        //      If so, we should declare "Arrival"
        if ((m_routeman.CurrentRangeToActiveNormalCrossing -
             m_routeman.m_arrival_min) >
            m_routeman.pActivePoint->GetWaypointArrivalRadius()) {
          if (++m_routeman.m_arrival_test > 2 &&
              !g_bAdvanceRouteWaypointOnArrivalOnly) {
            m_routeman.m_bArrival = true;
            m_routeman.UpdateAutopilot();

            bDidArrival = true;
            DoAdvance();
          }
        } else
          m_routeman.m_arrival_test = 0;
      }
    }
    if (!bDidArrival)
      m_routeman.m_arrival_min =
          wxMin(m_routeman.m_arrival_min,
                m_routeman.CurrentRangeToActiveNormalCrossing);
    // Only once on arrival
    if (!bDidArrival) m_routeman.UpdateAutopilot();
    bret_val = true;  // a route is active
  }
  m_routeman.m_bDataValid = true;
  return bret_val;
}

void RoutemanGui::DeleteTrack(Track *pTrack) {
  if (pTrack) {
    if (pTrack->m_bIsInLayer) return;

    ::wxBeginBusyCursor();

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
    if (TrackPropDlg::getInstanceFlag() && pTrackPropDialog &&
        (pTrackPropDialog->IsShown()) &&
        (pTrack == pTrackPropDialog->GetTrack())) {
      pTrackPropDialog->Hide();
    }

    if ((pTrack == g_pActiveTrack) && pTrack->IsRunning()) {
      pTrack = top_frame::Get()->TrackOff();
    }
    //    Remove the track from associated lists
    pSelect->DeleteAllSelectableTrackSegments(pTrack);
    auto it = std::find(g_TrackList.begin(), g_TrackList.end(), pTrack);
    if (it != g_TrackList.end()) {
      g_TrackList.erase(it);
    }
    delete pTrack;

    ::wxEndBusyCursor();

    delete pprog;
  }
}

void RoutemanGui::DeleteAllTracks() {
  top_frame::Get()->TrackOff();

  ::wxBeginBusyCursor();

  // Iterate on the RouteList, we delete from g_TrackList in DeleteTrack,
  // bigger refactoring is viable, but for now, we simply make a copy
  // that goes out of scope soon.
  std::vector<Track *> to_del = g_TrackList;
  for (Track *ptrack : to_del) {
    if (ptrack->m_bIsInLayer) continue;

    g_pAIS->DeletePersistentTrack(ptrack);
    NavObj_dB::GetInstance().DeleteTrack(ptrack);
    DeleteTrack(ptrack);
  }
  ::wxEndBusyCursor();
}

void RoutemanGui::DoAdvance() {
  if (!m_routeman.ActivateNextPoint(m_routeman.pActiveRoute,
                                    false))  // at the end?
  {
    Route *pthis_route = m_routeman.pActiveRoute;
    m_routeman.DeactivateRoute(true);  // this is an arrival

    if (pthis_route->m_bDeleteOnArrival && !pthis_route->m_bIsBeingEdited) {
      m_routeman.DeleteRoute(pthis_route);
    }

    if (pRouteManagerDialog) pRouteManagerDialog->UpdateRouteListCtrl();
  }
}
