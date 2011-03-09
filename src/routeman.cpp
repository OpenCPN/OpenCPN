/******************************************************************************
 * $Id: routeman.cpp,v 1.38 2010/06/23 23:45:01 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Route Manager
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "wx/image.h"

#include "dychart.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <wx/listimpl.cpp>

#include "routeman.h"
#include "concanv.h"
#include "nmea.h"                   // for Autopilot
#include "navutil.h"
#include "georef.h"
#include "routeprop.h"
#include "routemanagerdialog.h"


//    Include a (large) set of XPM images for mark/waypoint icons
#include "bitmaps/empty.xpm"
#include "bitmaps/airplane.xpm"
#include "bitmaps/anchorage.xpm"
#include "bitmaps/anchor.xpm"
#include "bitmaps/boarding.xpm"
#include "bitmaps/boundary.xpm"
#include "bitmaps/bouy1.xpm"
#include "bitmaps/bouy2.xpm"
#include "bitmaps/campfire.xpm"
#include "bitmaps/camping.xpm"
#include "bitmaps/circle.xpm"
#include "bitmaps/coral.xpm"
#include "bitmaps/fishhaven.xpm"
#include "bitmaps/fishing.xpm"
#include "bitmaps/fish.xpm"
#include "bitmaps/float.xpm"
#include "bitmaps/food.xpm"
#include "bitmaps/fuel.xpm"
#include "bitmaps/greenlite.xpm"
#include "bitmaps/kelp.xpm"
#include "bitmaps/light1.xpm"
#include "bitmaps/light.xpm"
#include "bitmaps/litevessel.xpm"
#include "bitmaps/mob.xpm"
#include "bitmaps/mooring.xpm"
#include "bitmaps/oilbouy.xpm"
#include "bitmaps/platform.xpm"
#include "bitmaps/redgreenlite.xpm"
#include "bitmaps/redlite.xpm"
#include "bitmaps/rock1.xpm"
#include "bitmaps/rock2.xpm"
#include "bitmaps/sand.xpm"
#include "bitmaps/scuba.xpm"
#include "bitmaps/shoal.xpm"
#include "bitmaps/snag.xpm"
#include "bitmaps/square.xpm"
#include "bitmaps/triangle.xpm"
#include "bitmaps/wreck1.xpm"
#include "bitmaps/wreck2.xpm"
#include "bitmaps/xmblue.xpm"
#include "bitmaps/xmgreen.xpm"
#include "bitmaps/xmred.xpm"
#include "bitmaps/diamond.xpm"
#include "bitmaps/activepoint.xpm"
#include <wx/dir.h>                             // 09.10.07; toh
#include <wx/filename.h>                        // 09.10.07; toh
#include "wx/stdpaths.h"                        // 09.10.07; toh
#include "wx/apptrait.h"                        // 09.10.07; toh


extern ConsoleCanvas    *console;

extern RouteList        *pRouteList;
extern Select           *pSelect;
extern MyConfig         *pConfig;
extern NMEA0183         *pNMEA0183;
extern AutoPilotWindow  *pAPilot;
extern WayPointman      *pWayPointMan;
extern Routeman         *g_pRouteMan;

extern wxRect           g_blink_rect;
extern wxString         g_SData_Locn;     // 09.10.07; toh

extern double           gLat, gLon, gSog, gCog;

extern RoutePoint       *pAnchorWatchPoint1;   // pjotrc 2010.02.15
extern RoutePoint       *pAnchorWatchPoint2;   // pjotrc 2010.02.15

extern Track            *g_pActiveTrack;
extern RouteProp        *pRoutePropDialog;
extern RouteManagerDialog *pRouteManagerDialog;
extern RoutePoint      *pAnchorWatchPoint1;
extern RoutePoint      *pAnchorWatchPoint2;



//    List definitions for Waypoint Manager Icons
WX_DECLARE_LIST(wxBitmap, markicon_bitmap_list_type);
WX_DECLARE_LIST(wxString, markicon_key_list_type);
WX_DECLARE_LIST(wxString, markicon_description_list_type);


//    List implementation for Waypoint Manager Icons
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(markicon_bitmap_list_type);
WX_DEFINE_LIST(markicon_key_list_type);
WX_DEFINE_LIST(markicon_description_list_type);


CPL_CVSID("$Id: routeman.cpp,v 1.38 2010/06/23 23:45:01 bdbcat Exp $");

//--------------------------------------------------------------------------------
//      Routeman   "Route Manager"
//--------------------------------------------------------------------------------

Routeman::Routeman(MyApp *parent)
{
        m_pparent_app = parent;
        pActiveRoute = NULL;
        pActivePoint = NULL;
        pRouteActivatePoint = NULL;
}

Routeman::~Routeman()
{
        if( pRouteActivatePoint)
                delete  pRouteActivatePoint;
}


bool Routeman::IsRouteValid(Route *pRoute)
{
      wxRouteListNode *node = pRouteList->GetFirst();
      while(node)
      {
            if(pRoute == node->GetData())
                  return true;
            node = node->GetNext();
      }
      return false;
}



//    Make a 2-D search to find the route containing a given waypoint
Route *Routeman::FindRouteContainingWaypoint(RoutePoint *pWP)
{
      wxRouteListNode *node = pRouteList->GetFirst();
      while(node)
      {
            Route *proute = node->GetData();

            wxRoutePointListNode *pnode = (proute->pRoutePointList)->GetFirst();
            while(pnode)
            {
                  RoutePoint *prp = pnode->GetData();
                  if(prp == pWP)                // success
                        return proute;

                  pnode = pnode->GetNext();
            }

            node = node->GetNext();
      }

      return NULL;                              // not found
}

wxArrayPtrVoid *Routeman::GetRouteArrayContaining(RoutePoint *pWP)
{
      wxArrayPtrVoid *pArray = new wxArrayPtrVoid;

      wxRouteListNode *route_node = pRouteList->GetFirst();
      while(route_node)
      {
            Route *proute = route_node->GetData();

            wxRoutePointListNode *waypoint_node = (proute->pRoutePointList)->GetFirst();
            while(waypoint_node)
            {
                  RoutePoint *prp = waypoint_node->GetData();
                  if(prp == pWP)                // success
                        pArray->Add((void *)proute);

                  waypoint_node = waypoint_node->GetNext();           // next waypoint
            }

            route_node = route_node->GetNext();                         // next route
      }

      if(pArray->GetCount())
            return pArray;

      else
      {
            delete pArray;
            return NULL;
      }
}

RoutePoint *Routeman::FindBestActivatePoint(Route *pR, double lat, double lon, double cog, double sog)
{
      if(!pR)
            return NULL;

      // Walk thru all the points to find the "best"
      RoutePoint *best_point = NULL;
      double min_time_found = 1e6;

      wxRoutePointListNode *node = (pR->pRoutePointList)->GetFirst();
      while(node)
      {
            RoutePoint *pn = node->GetData();

            double brg, dist;
            DistanceBearingMercator(pn->m_lat, pn->m_lon, lat, lon, &brg, &dist);

            double angle = brg - cog;
            double soa = cos(angle * PI / 180.);


            double time_to_wp = dist / soa;

            if(time_to_wp > 0)
            {
                  if(time_to_wp < min_time_found)
                  {
                        min_time_found = time_to_wp;
                        best_point = pn;
                  }
            }
            node = node->GetNext();
      }
      return best_point;
}

bool Routeman::ActivateRoute(Route *pRouteToActivate, RoutePoint *pStartPoint)
{
        pActiveRoute = pRouteToActivate;

        if(pStartPoint)
        {
              pActivePoint = pStartPoint;
        }
        else
        {
            wxRoutePointListNode *node = (pActiveRoute->pRoutePointList)->GetFirst();
            pActivePoint = node->GetData();               // start at beginning
        }

        ActivateRoutePoint(pRouteToActivate, pActivePoint);

        m_bArrival = false;

        pRouteToActivate->m_bRtIsActive = true;

        m_bDataValid = false;

        console->ShowWithFreshFonts();
        return true;
}

bool Routeman::ActivateRoutePoint(Route *pA, RoutePoint *pRP_target)
{
        pActiveRoute = pA;
        pActivePoint = pRP_target;
        pActiveRoute->m_pRouteActivePoint = pRP_target;

        wxRoutePointListNode *node = (pActiveRoute->pRoutePointList)->GetFirst();
        while(node)
        {
              RoutePoint *pn = node->GetData();
              pn->m_bBlink = false;                     // turn off all blinking points
              pn->m_bIsActive = false;

              node = node->GetNext();
        }

        node = (pActiveRoute->pRoutePointList)->GetFirst();
        RoutePoint *prp_first = node->GetData();

        //  If activating first point in route, create a "virtual" waypoint at present position
        if(pRP_target == prp_first)
        {
                if(pRouteActivatePoint)
                        delete pRouteActivatePoint;

                pRouteActivatePoint = new RoutePoint(gLat, gLon, wxString(_T("")), wxString(_T("")), GPX_EMPTY_STRING, false); // Current location
                pRouteActivatePoint->m_bShowName = false;

                pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
        }

        else
        {
              prp_first->m_bBlink = false;
              node = node->GetNext();
              RoutePoint *np_prev = prp_first;
              while(node)
              {
                  RoutePoint *pnext = node->GetData();
                  if(pnext == pRP_target)
                  {
                          pActiveRouteSegmentBeginPoint = np_prev;
                          break;
                  }

                  np_prev = pnext;
                  node = node->GetNext();
            }
        }

        pRP_target->m_bBlink = true;                               // blink the active point
        pRP_target->m_bIsActive = true;                            // and active

        g_blink_rect = pRP_target->CurrentRect_in_DC;               // set up global blinker

        m_bArrival = false;
        return true;
}

bool Routeman::ActivateNextPoint(Route *pr)
{
      if(pActivePoint)
      {
            pActivePoint->m_bBlink = false;
            pActivePoint->m_bIsActive = false;
      }

      int n_index_active = pActiveRoute->GetIndexOf(pActivePoint);
      if((n_index_active + 1) <= pActiveRoute->GetnPoints())
      {
          pActiveRouteSegmentBeginPoint = pActivePoint;

          pActiveRoute->m_pRouteActivePoint = pActiveRoute->GetPoint(n_index_active + 1);

          pActivePoint = pActiveRoute->GetPoint(n_index_active + 1);

          pActivePoint->m_bBlink = true;
          pActivePoint->m_bIsActive = true;
          g_blink_rect = pActivePoint->CurrentRect_in_DC;               // set up global blinker

          m_bArrival = false;

          return true;
      }

      return false;
}




bool Routeman::UpdateProgress()
{
    bool bret_val = false;

        if(pActiveRoute)
        {
//      Update bearing, range, and crosstrack error
                double north, east;
                toSM(pActivePoint->m_lat, pActivePoint->m_lon, gLat, gLon, &east, &north);
                double a = atan(north / east);
                if(pActivePoint->m_lon > gLon)
                    CurrentBrgToActivePoint = 90. - (a * 180/PI);
                else
                    CurrentBrgToActivePoint = 270. - (a * 180/PI);


//      Calculate range using Great Circle Formula

                double d5 = DistGreatCircle(gLat, gLon, pActivePoint->m_lat, pActivePoint->m_lon );
                CurrentRngToActivePoint = d5;

//      Get the XTE vector, normal to current segment
                VECTOR2D va, vb, vn;

                double brg1, dist1, brg2, dist2;
                DistanceBearingMercator(pActivePoint->m_lat, pActivePoint->m_lon,
                                        pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon, &brg1, &dist1);
                vb.x = dist1 * sin(brg1 * PI / 180.);
                vb.y = dist1 * cos(brg1 * PI / 180.);

                DistanceBearingMercator(pActivePoint->m_lat, pActivePoint->m_lon, gLat, gLon, &brg2, &dist2);
                va.x = dist2 * sin(brg2 * PI / 180.);
                va.y = dist2 * cos(brg2 * PI / 180.);

                double sdelta = vGetLengthOfNormal(&va, &vb, &vn);             // NM
                CurrentXTEToActivePoint = sdelta;


//    Calculate the distance to the arrival line, which is perpendicular to the current route segment
//    Taking advantage of the calculated normal from current position to route segment vn
                VECTOR2D vToArriveNormal;
                vSubtractVectors(&va, &vn, &vToArriveNormal);

                CurrentRangeToActiveNormalCrossing = vVectorMagnitude(&vToArriveNormal);


//          Compute current segment course
//          Using simple Mercater projection
                double x1, y1, x2, y2;
                toSM(pActiveRouteSegmentBeginPoint->m_lat,  pActiveRouteSegmentBeginPoint->m_lon,
                     pActiveRouteSegmentBeginPoint->m_lat,  pActiveRouteSegmentBeginPoint->m_lon, &x1, &y1);

                toSM(pActivePoint->m_lat,  pActivePoint->m_lon,
                     pActiveRouteSegmentBeginPoint->m_lat,  pActiveRouteSegmentBeginPoint->m_lon, &x2, &y2);

                double e1 = atan2((x2 - x1), (y2-y1));
                CurrentSegmentCourse = e1 * 180/PI;
                if(CurrentSegmentCourse < 0)
                      CurrentSegmentCourse += 360;


 //      Compute XTE direction
                double h = atan(vn.y / vn.x);
                if(vn.x > 0)
                        CourseToRouteSegment = 90. - (h * 180/PI);
                else
                        CourseToRouteSegment = 270. - (h * 180/PI);

                h= CurrentBrgToActivePoint - CourseToRouteSegment;
                if(h < 0 )
                        h = h + 360;

                if(h > 180)
                        XTEDir = 1;
                else
                        XTEDir = -1;



//      Determine Arrival

                bool bDidArrival = false;


                if(CurrentRangeToActiveNormalCrossing <= pActiveRoute->GetRouteArrivalRadius())
                {
                  m_bArrival = true;
                  UpdateAutopilot();

                  bDidArrival = true;

                  if(!ActivateNextPoint(pActiveRoute))            // at the end?
                  {
                          Route *pthis_route = pActiveRoute;
                          DeactivateRoute();
                          if(pthis_route->m_bDeleteOnArrival)
                          {
                                pConfig->DeleteConfigRoute ( pthis_route );
                                DeleteRoute ( pthis_route );
                                if ( pRoutePropDialog )
                                {
                                      pRoutePropDialog->SetRouteAndUpdate ( NULL );
                                      pRoutePropDialog->UpdateProperties();
                                }
                                if(pRouteManagerDialog)
                                    pRouteManagerDialog->UpdateRouteListCtrl();

                          }
                  }

                }

                if(!bDidArrival)                                        // Only once on arrival
                        UpdateAutopilot();

                bret_val = true;                                        // a route is active
        }

        m_bDataValid = true;

        return bret_val;
}

bool Routeman::DeactivateRoute()
{
      if(pActivePoint)
      {
            pActivePoint->m_bBlink = false;
            pActivePoint->m_bIsActive = false;
      }


      if(pActiveRoute)
      {
          pActiveRoute->m_bRtIsActive = false;
          pActiveRoute->m_pRouteActivePoint = NULL;
      }
      pActiveRoute = NULL;

      if(pRouteActivatePoint)
          delete  pRouteActivatePoint;
      pRouteActivatePoint = NULL;

      console->pCDI->ClearBackground();

      console->Show(false);

      m_bDataValid = false;

      return true;
}

bool Routeman::UpdateAutopilot()
{
        wxString str_buf;

        if(pAPilot->IsOK())
        {
                m_NMEA0183.TalkerID = _T("EC");

                SENTENCE snt;
                m_NMEA0183.Rmb.IsDataValid = NTrue;
                m_NMEA0183.Rmb.CrossTrackError = CurrentXTEToActivePoint;

                if(XTEDir < 0)
                      m_NMEA0183.Rmb.DirectionToSteer = Left;
                else
                      m_NMEA0183.Rmb.DirectionToSteer = Right;


                m_NMEA0183.Rmb.To = pActivePoint->m_MarkName.Truncate(6);
                m_NMEA0183.Rmb.From = pActiveRouteSegmentBeginPoint->m_MarkName.Truncate(6);

//                str_buf.Printf(_T("%03d"), pActiveRoute->GetIndexOf(pActiveRouteSegmentBeginPoint));
//                wxString from = str_buf;
//                m_NMEA0183.Rmb.From = from;

//                str_buf.Printf(_T("%03d"), pActiveRoute->GetIndexOf(pActivePoint));
//                wxString to = str_buf;
//                m_NMEA0183.Rmb.To = to;

                if(pActivePoint->m_lat < 0.)
                      m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(-pActivePoint->m_lat, _T("S"));
                else
                      m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(pActivePoint->m_lat, _T("N"));

                if(pActivePoint->m_lon < 0.)
                      m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(-pActivePoint->m_lon, _T("W"));
                else
                      m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(pActivePoint->m_lon, _T("E"));

 //               m_NMEA0183.Rmb.DestinationPosition.Latitude.Latitude = pActivePoint->m_lat;
 //               m_NMEA0183.Rmb.DestinationPosition.Latitude.Northing = North;

 //               m_NMEA0183.Rmb.DestinationPosition.Longitude.Longitude = fabs(pActivePoint->m_lon);
 //               m_NMEA0183.Rmb.DestinationPosition.Longitude.Easting = West;


                m_NMEA0183.Rmb.RangeToDestinationNauticalMiles = CurrentRngToActivePoint;
                m_NMEA0183.Rmb.BearingToDestinationDegreesTrue = CurrentBrgToActivePoint;
                m_NMEA0183.Rmb.DestinationClosingVelocityKnots = gSog;

                if(m_bArrival)
                      m_NMEA0183.Rmb.IsArrivalCircleEntered = NTrue;
                else
                      m_NMEA0183.Rmb.IsArrivalCircleEntered = NFalse;

                m_NMEA0183.Rmb.Write(snt);

        //      stats->pTStat2->TextDraw(( const char *)snt.Sentence);

                pAPilot->AutopilotOut(snt.Sentence);
                }

        return true;
}

void Routeman::DeleteRoute(Route *pRoute)
{
      if(pRoute)
      {
            ::wxBeginBusyCursor();

            if ( GetpActiveRoute() == pRoute )
                  DeactivateRoute();

            //    Remove the route from associated lists
            pSelect->DeleteAllSelectableRouteSegments(pRoute);
            pRouteList->DeleteObject(pRoute);

            // walk the route, tentatively deleting/marking points used only by this route
            wxRoutePointListNode *pnode = (pRoute->pRoutePointList)->GetFirst();
            while(pnode)
            {
                  RoutePoint *prp = pnode->GetData();

                  // check all other routes to see if this point appears in any other route
                  Route *pcontainer_route = FindRouteContainingWaypoint(prp);

                  if(pcontainer_route == NULL && prp->m_bIsInRoute)
                  {
                        prp->m_bIsInRoute = false;          // Take this point out of this (and only) route
                        if(!prp->m_bKeepXRoute)
                        {
//    This does not need to be done with navobj.xml storage, since the waypoints are stored with the route
//                              pConfig->DeleteWayPoint(prp);

                              pSelect->DeleteSelectablePoint(prp, SELTYPE_ROUTEPOINT);

                              // Remove all instances of this point from the list.
                              wxRoutePointListNode *pdnode = pnode;
                              while(pdnode)
                              {
                                    pRoute->pRoutePointList->DeleteNode(pdnode);
                                    pdnode = pRoute->pRoutePointList->Find(prp);
                              }

                              pnode = NULL;
                              delete prp;
                        }
                        else
                        {
                              prp->m_bDynamicName = false;
                              prp->m_bIsolatedMark = true;        // This has become an isolated mark
                              prp->m_bKeepXRoute = false;         // and is no longer part of a route
                        }


                  }
                  if(pnode)
                        pnode = pnode->GetNext();
                  else
                        pnode = pRoute->pRoutePointList->GetFirst();                // restart the list
            }

            delete pRoute;

            ::wxEndBusyCursor();

      }
}

void Routeman::DeleteAllRoutes(void)
{
      ::wxBeginBusyCursor();

      //    Iterate on the RouteList
      wxRouteListNode *node = pRouteList->GetFirst();
      while(node)
      {
            Route *proute = node->GetData();

            if(!proute->m_bIsTrack)
            {
                  pConfig->m_bIsImporting = true;
                  pConfig->DeleteConfigRoute ( proute );
                  DeleteRoute(proute);
                  node = pRouteList->GetFirst();                   // Route
                  pConfig->m_bIsImporting = false;
            }
            else
                  node = node->GetNext();
      }

      ::wxEndBusyCursor();

}

void Routeman::DeleteAllTracks(void)
{
      ::wxBeginBusyCursor();

      //    Iterate on the RouteList
      wxRouteListNode *node = pRouteList->GetFirst();
      while(node)
      {
            Route *proute = node->GetData();

            if(proute->m_bIsTrack)
            {
                  pConfig->m_bIsImporting = true;
                  pConfig->DeleteConfigRoute ( proute );
                  DeleteTrack(proute);
                  node = pRouteList->GetFirst();                   // Route
                  pConfig->m_bIsImporting = false;
            }
            else
                  node = node->GetNext();
      }

      ::wxEndBusyCursor();

}


void Routeman::AssembleAllRoutes(void)
{
      //    Iterate on the RouteList
      wxRouteListNode *node = pRouteList->GetFirst();
      while(node)
      {
            Route *proute = node->GetData();

            proute->AssembleRoute();
            if(proute->GetnPoints())
            {
                  pSelect->AddAllSelectableRouteSegments(proute);
            }
            else                                // this route has no points
            {
                  pConfig->DeleteConfigRoute(proute);
                  DeleteRoute(proute);
            }

            node = node->GetNext();                   // Route
      }
}

void Routeman::DeleteTrack(Route *pRoute)
{
      if(pRoute)
      {
           ::wxBeginBusyCursor();

            //    Remove the route from associated lists
            pSelect->DeleteAllSelectableTrackSegments(pRoute);
            pRouteList->DeleteObject(pRoute);

            // walk the route, tentatively deleting/marking points used only by this route
            wxRoutePointListNode *pnode = (pRoute->pRoutePointList)->GetFirst();
            while(pnode)
            {
                  RoutePoint *prp = pnode->GetData();

                  // check all other routes to see if this point appears in any other route
                  Route *pcontainer_route = FindRouteContainingWaypoint(prp);

                  if(pcontainer_route == NULL)
                  {
                        prp->m_bIsInRoute = false;          // Take this point out of this (and only) route
                        if(!prp->m_bKeepXRoute)
                        {
                              pConfig->m_bIsImporting = true;
                              pConfig->DeleteWayPoint(prp);
                              pSelect->DeleteSelectablePoint(prp, SELTYPE_ROUTEPOINT);
                              pConfig->m_bIsImporting = false;

                              // Remove all instances of this point from the list.
                              wxRoutePointListNode *pdnode = pnode;
                              while(pdnode)
                              {
                                    pRoute->pRoutePointList->DeleteNode(pdnode);
                                    pdnode = pRoute->pRoutePointList->Find(prp);
                              }

                              pnode = NULL;
                              delete prp;
                        }

                  }
                  if(pnode)
                        pnode = pnode->GetNext();
                  else
                        pnode = pRoute->pRoutePointList->GetFirst();                // restart the list
            }


            if((Track *)pRoute == g_pActiveTrack)
            {
                  g_pActiveTrack = NULL;
                  m_pparent_app->TrackOff();
            }

            delete pRoute;

            ::wxEndBusyCursor();

      }
}





void Routeman::SetColorScheme(ColorScheme cs)
{
      // Re-Create the pens and colors

//      m_pRoutePen =             wxThePenList->FindOrCreatePen(wxColour(0,0,255), 2, wxSOLID);
//      m_pSelectedRoutePen =     wxThePenList->FindOrCreatePen(wxColour(255,0,0), 2, wxSOLID);
//      m_pActiveRoutePen =       wxThePenList->FindOrCreatePen(wxColour(255,0,255), 2, wxSOLID);
      m_pActiveRoutePointPen =  wxThePenList->FindOrCreatePen(wxColour(0,0,255), 2, wxSOLID);
      m_pRoutePointPen =        wxThePenList->FindOrCreatePen(wxColour(0,0,255), 2, wxSOLID);


//    Or in something like S-52 compliance

      m_pRoutePen =             wxThePenList->FindOrCreatePen(GetGlobalColor(_T("UINFB")), 2, wxSOLID);
      m_pSelectedRoutePen =     wxThePenList->FindOrCreatePen(GetGlobalColor(_T("UINFO")), 2, wxSOLID);
      m_pActiveRoutePen =       wxThePenList->FindOrCreatePen(GetGlobalColor(_T("PLRTE")), 2, wxSOLID);
//      m_pActiveRoutePointPen =  wxThePenList->FindOrCreatePen(GetGlobalColor(_T("PLRTE")), 2, wxSOLID);
//      m_pRoutePointPen =        wxThePenList->FindOrCreatePen(GetGlobalColor(_T("CHBLK")), 2, wxSOLID);



      m_pRouteBrush =             wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T("UINFB")), wxSOLID);
      m_pSelectedRouteBrush =     wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T("UINFO")), wxSOLID);
      m_pActiveRouteBrush =       wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T("PLRTE")), wxSOLID);
//      m_pActiveRoutePointBrush =  wxTheBrushList->FindOrCreatePen(GetGlobalColor(_T("PLRTE")), wxSOLID);
//      m_pRoutePointBrush =        wxTheBrushList->FindOrCreatePen(GetGlobalColor(_T("CHBLK")), wxSOLID);

 }


 wxString Routeman::GetRouteReverseMessage(void)
 {
       return wxString(_("Waypoints can be renamed to reflect the new order, the names will be '001', '002' etc.\n\nDo you want to rename the waypoints?"));
 }


//-------------------------------------------------------------------------------
//
//   Route "Send to GPS..." Dialog Implementation
//
//-------------------------------------------------------------------------------


IMPLEMENT_DYNAMIC_CLASS( SendToGpsDlg, wxDialog )

BEGIN_EVENT_TABLE( SendToGpsDlg, wxDialog )
      EVT_BUTTON( ID_STG_CANCEL, SendToGpsDlg::OnCancelClick )
      EVT_BUTTON( ID_STG_OK, SendToGpsDlg::OnSendClick )
END_EVENT_TABLE()



SendToGpsDlg::SendToGpsDlg( )
 {
       m_itemCommListBox = NULL;
       m_pgauge = NULL;
       m_SendButton = NULL;
       m_CancelButton = NULL;
       m_pRoute = NULL;
       m_pRoutePoint = NULL;
 }

 SendToGpsDlg::SendToGpsDlg(  wxWindow* parent, wxWindowID id,
                      const wxString& caption, const wxString& hint, const wxPoint& pos, const wxSize& size, long style )
{
      Create(parent, id, caption, hint, pos, size, style);
}

SendToGpsDlg::~SendToGpsDlg( )
{
      delete m_itemCommListBox;
      delete m_pgauge;
      delete m_SendButton;
      delete m_CancelButton;
}




bool SendToGpsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxString& hint, const wxPoint& pos, const wxSize& size, long style )
{
      SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
      wxDialog::Create( parent, id, caption, pos, size, style );

      CreateControls(hint);
      GetSizer()->Fit(this);
      GetSizer()->SetSizeHints(this);
      Centre();

      return TRUE;
}


void SendToGpsDlg::CreateControls(const wxString& hint)
{
      SendToGpsDlg* itemDialog1 = this;

      wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
      itemDialog1->SetSizer(itemBoxSizer2);

//      Create the ScrollBox list of available com ports in a labeled static box

      wxStaticBox* comm_box = new wxStaticBox(this, wxID_ANY, _("GPS/Plotter Port"));

      wxStaticBoxSizer* comm_box_sizer = new wxStaticBoxSizer(comm_box, wxVERTICAL);
      itemBoxSizer2->Add(comm_box_sizer, 0, wxEXPAND|wxALL, 5);


      wxArrayString *pSerialArray = EnumerateSerialPorts();

      m_itemCommListBox = new wxComboBox(this, ID_STG_CHOICE_COMM);

      //    Fill in the listbox with all detected serial ports
      for (unsigned int iPortIndex=0 ; iPortIndex < pSerialArray->GetCount() ; iPortIndex++)
            m_itemCommListBox->Append( pSerialArray->Item(iPortIndex) );

      delete pSerialArray;

      //    Make the proper inital selection
      int sidx = 0;
      if(hint.Upper().Contains(_T("SERIAL")))
      {
            wxString sourcex = hint.Mid(7);
            sidx = m_itemCommListBox->FindString(sourcex);
      }
      else
            sidx = m_itemCommListBox->FindString(hint);

      m_itemCommListBox->SetSelection(sidx);

      comm_box_sizer->Add(m_itemCommListBox, 0, wxEXPAND|wxALL, 5);

      //    Add a reminder text box
      itemBoxSizer2->AddSpacer(20);

      wxStaticText *premtext = new wxStaticText(this, -1, _("Prepare GPS for Route/Waypoint upload and press Send..."));
      itemBoxSizer2->Add(premtext, 0, wxEXPAND|wxALL, 10);

      //    Create a progress gauge
      wxStaticBox* prog_box = new wxStaticBox(this, wxID_ANY, _("Progress..."));

      wxStaticBoxSizer* prog_box_sizer = new wxStaticBoxSizer(prog_box, wxVERTICAL);
      itemBoxSizer2->Add(prog_box_sizer, 0, wxEXPAND|wxALL, 5);

      m_pgauge = new wxGauge(this, -1, 100);
      prog_box_sizer->Add(m_pgauge, 0, wxEXPAND|wxALL, 5);


      //    OK/Cancel/etc.
      wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
      itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT|wxALL, 5);

      m_CancelButton = new wxButton( itemDialog1, ID_STG_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

      m_SendButton = new wxButton( itemDialog1, ID_STG_OK, _("Send"), wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizer16->Add(m_SendButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
      m_SendButton->SetDefault();

}


void SendToGpsDlg::OnSendClick( wxCommandEvent& event )
{
      //    Get the selected comm port
      int i = m_itemCommListBox->GetSelection();
      wxString src(m_itemCommListBox->GetString(i));

      src = m_itemCommListBox->GetValue();

      //    And send it out
      if(m_pRoute)
            m_pRoute->SendToGPS(src, true, m_pgauge);
      if(m_pRoutePoint)
            m_pRoutePoint->SendToGPS(src, m_pgauge);

      Show(false);
      event.Skip();
}

void SendToGpsDlg::OnCancelClick( wxCommandEvent& event )
{
      Show(false);
      event.Skip();
}

















//--------------------------------------------------------------------------------
//      WayPointman   Implementation
//--------------------------------------------------------------------------------

/*
#define MAKEICONARRAYS(key, xpm_ptr, description)\
       pmarkiconImage = new wxImage((char **)xpm_ptr);\
       pmarkiconBitmap = new wxBitmap(*pmarkiconImage, (wxDC)dwxdc);\
       delete pmarkiconImage;\
       pmi = new MarkIcon;\
       pmi->picon_bitmap = pmarkiconBitmap;\
       pmi->icon_name = _T(key);\
       pmi->icon_description = _T(description);\
       DayIconArray.Add((void *)pmi);\
       pmarkiconImage = new wxImage((char **)xpm_ptr);\
       pmarkiconBitmap = new wxBitmap(*pmarkiconImage, dwxdc);\
       delete pmarkiconImage;\
       pmarkiconBitmapDim = CreateDimBitmap(pmarkiconBitmap, .50, dwxdc);\
       delete pmarkiconBitmap;\
       pmi = new MarkIcon;\
       pmi->picon_bitmap = pmarkiconBitmapDim;\
       pmi->icon_name = _T(key);\
       pmi->icon_description = _T(description);\
       DuskIconArray.Add((void *)pmi);\
       pmarkiconImage = new wxImage((char **)xpm_ptr);\
       pmarkiconBitmap = new wxBitmap(*pmarkiconImage, dwxdc);\
       delete pmarkiconImage;\
       pmarkiconBitmapDim = CreateDimBitmap(pmarkiconBitmap, .25, dwxdc);\
       delete pmarkiconBitmap;\
       pmi = new MarkIcon;\
       pmi->picon_bitmap = pmarkiconBitmapDim;\
       pmi->icon_name = _T(key);\
       pmi->icon_description = _T(description);\
       NightIconArray.Add((void *)pmi);\
*/
#define MAKEICONARRAYS(key, xpm_ptr, description)\
 pmarkiconImage = new wxImage(( const char **)xpm_ptr);\
 ProcessIcon(pmarkiconImage, _T(key), _T(description));\
 delete pmarkiconImage;


WayPointman::WayPointman()
{

      m_pWayPointList = new RoutePointList;

      wxImage *pmarkiconImage;

      pmarkicon_image_list = NULL;

      MAKEICONARRAYS("empty", empty, "Empty")
      MAKEICONARRAYS("airplane", airplane, "Airplane")
      MAKEICONARRAYS("anchorage", anchorage, "Anchorage")
      MAKEICONARRAYS("anchor", anchor, "Anchor")
      MAKEICONARRAYS("boarding", boarding, "Boarding Location")
      MAKEICONARRAYS("boundary", boundary, "Boundary Mark")
      MAKEICONARRAYS("bouy1", bouy1, "Bouy Type A")
      MAKEICONARRAYS("bouy2", bouy2, "Bouy Type B")
      MAKEICONARRAYS("campfire", campfire, "Campfire")
      MAKEICONARRAYS("camping", camping, "Camping Spot")
      MAKEICONARRAYS("coral", coral, "Coral")
      MAKEICONARRAYS("fishhaven", fishhaven, "Fish Haven")
      MAKEICONARRAYS("fishing", fishing, "Fishing Spot")
      MAKEICONARRAYS("fish", fish, "Fish")
      MAKEICONARRAYS("floating", floating, "Float")
      MAKEICONARRAYS("food", food, "Food")
      MAKEICONARRAYS("fuel", fuel, "Fuel")
      MAKEICONARRAYS("greenlite", greenlite, "Green Light")
      MAKEICONARRAYS("kelp", kelp, "Kelp")
      MAKEICONARRAYS("light", light1, "Light Type A")
      MAKEICONARRAYS("light1", light, "Light Type B")
      MAKEICONARRAYS("litevessel", litevessel, "Light Vessel")
      MAKEICONARRAYS("mob", mob, "MOB")
      MAKEICONARRAYS("mooring", mooring, "Mooring Bouy")
      MAKEICONARRAYS("oilbouy", oilbouy, "Oil Bouy")
      MAKEICONARRAYS("platform", platform, "Platform")
      MAKEICONARRAYS("redgreenlite", redgreenlite, "Red/Green Light")
      MAKEICONARRAYS("redlite", redlite, "Red Light")
      MAKEICONARRAYS("rock1", rock1, "Rock (exposed)")
      MAKEICONARRAYS("rock2", rock2, "Rock, (awash)")
      MAKEICONARRAYS("sand", sand, "Sand")
      MAKEICONARRAYS("scuba", scuba, "Scuba")
      MAKEICONARRAYS("shoal", shoal, "Shoal")
      MAKEICONARRAYS("snag", snag, "Snag")
      MAKEICONARRAYS("square", square, "Square")
      MAKEICONARRAYS("triangle", triangle, "Triangle")
      MAKEICONARRAYS("diamond", diamond, "Diamond")
      MAKEICONARRAYS("circle", circle, "Circle")
      MAKEICONARRAYS("wreck1", wreck1, "Wreck A")
      MAKEICONARRAYS("wreck2", wreck2, "Wreck B")
      MAKEICONARRAYS("xmblue", xmblue, "Blue X")
      MAKEICONARRAYS("xmgreen", xmgreen, "Green X")
      MAKEICONARRAYS("xmred", xmred, "Red X")
      MAKEICONARRAYS("activepoint", activepoint, "Active WP")

// Load user defined icons; toh, 09.10.07
      wxString UserIconPath = g_SData_Locn;
      UserIconPath.Append(_T("UserIcons"));
      if(wxDir::Exists(UserIconPath))
      {
            wxArrayString FileList;

            wxDir dir(UserIconPath);
            int n_files = dir.GetAllFiles(UserIconPath, &FileList);

            for(int ifile=0 ; ifile < n_files ; ifile++)
            {
                  wxString name = FileList.Item(ifile);

                  wxString iconname = name.BeforeFirst('.');

      #ifdef __WXMSW__
                  iconname = iconname.AfterLast('\\');
      #else
                  iconname = iconname.AfterLast('/');
      #endif

                  wxBitmap icon1;

                  if (icon1.LoadFile(name,wxBITMAP_TYPE_XPM))
                  {
                        wxImage *iconImage = new wxImage;
                        *iconImage = icon1.ConvertToImage();
                        ProcessIcon(iconImage,iconname,iconname);
                        delete iconImage;
                  }
            }
      }
// End load user defined icons; toh, 09.10.07

      m_nIcons = DayIconArray.GetCount();
      m_pcurrent_icon_array = &DayIconArray;

      m_nGUID = 0;
}

WayPointman::~WayPointman()
{

      //    Two step here, since the RoutePoint dtor also touches the
      //    RoutePoint list.
      //    Copy the master RoutePoint list to a temporary list,
      //    then clear and delete objects from the temp list

      RoutePointList    temp_list;

      wxRoutePointListNode *node = m_pWayPointList->GetFirst();
      while(node)
      {
            RoutePoint *pr = node->GetData();

            temp_list.Append(pr);
            node = node->GetNext();
      }

      temp_list.DeleteContents(true);
      temp_list.Clear();


      m_pWayPointList->Clear();
      delete m_pWayPointList;

      for( unsigned int i = 0 ; i< DayIconArray.GetCount() ; i++)
      {
            MarkIcon *pmi = (MarkIcon *)NightIconArray.Item(i);
            delete pmi->picon_bitmap;
            delete pmi;

            pmi = (MarkIcon *)DuskIconArray.Item(i);
            delete pmi->picon_bitmap;
            delete pmi;

            pmi = (MarkIcon *)DayIconArray.Item(i);
            delete pmi->picon_bitmap;
            delete pmi;
      }


      NightIconArray.Empty();
      DuskIconArray.Empty();
      DayIconArray.Empty();

      if(pmarkicon_image_list)
            pmarkicon_image_list->RemoveAll();
      delete pmarkicon_image_list;
}

void WayPointman::ProcessIcon(wxImage *pimage, wxString key, wxString description)
{

      wxBitmap *pmarkiconBitmap;
      wxBitmap *pmarkiconBitmapDim;
      MarkIcon *pmi;

//    Day Icon
#ifdef __WXMSW__
//      wxMSW Bug???
//      On Windows XP, conversion from wxImage to wxBitmap fails at the ::CreateDIBitmap() call
//      unless a "compatible" dc is provided.  Why??
//      As a workaround, just make a simple wxDC for temporary use
      wxBitmap tbmp ( 1, 1,-1 );
      wxMemoryDC dwxdc;
      dwxdc.SelectObject ( tbmp );

      pmarkiconBitmap = new wxBitmap(*pimage, dwxdc);
#else
      pmarkiconBitmap = new wxBitmap(*pimage);
#endif

      pmi = new MarkIcon;
      pmi->picon_bitmap = pmarkiconBitmap;
      pmi->icon_name = key;
      pmi->icon_description = description;
      DayIconArray.Add((void *)pmi);

//    Dusk
      pmarkiconBitmapDim = CreateDimBitmap(pmarkiconBitmap, .50);
      pmi = new MarkIcon;
      pmi->picon_bitmap = pmarkiconBitmapDim;
      pmi->icon_name = key;
      pmi->icon_description = description;
      DuskIconArray.Add((void *)pmi);

//    Night
      pmarkiconBitmapDim = CreateDimBitmap(pmarkiconBitmap, .50);
      pmi = new MarkIcon;
      pmi->picon_bitmap = pmarkiconBitmapDim;
      pmi->icon_name = key;
      pmi->icon_description = description;
      NightIconArray.Add((void *)pmi);
}



wxImageList *WayPointman::Getpmarkicon_image_list(void)
{
      // First find the largest bitmap size
      int w=0;
      int h=0;

      MarkIcon *pmi;

      for( unsigned int i = 0 ; i< m_pcurrent_icon_array->GetCount() ; i++)
      {
            pmi = (MarkIcon *)m_pcurrent_icon_array->Item(i);
            w = wxMax(w, pmi->picon_bitmap->GetWidth());
            h = wxMax(h, pmi->picon_bitmap->GetHeight());

            // toh, 10.09.29
            // User defined icons won't be displayed in the list if they are larger than 32x32 pixels (why???)
            // Work-around: limit size
            if (w > 32)
                  w = 32;
            if (h > 32)
                  h = 32;

      }

      //Build an image list large enough

      if(NULL != pmarkicon_image_list)
      {
            pmarkicon_image_list->RemoveAll();
            delete pmarkicon_image_list;
      }
      pmarkicon_image_list = new wxImageList(w, h);

      //  Add the icons
      for( unsigned int ii = 0 ; ii< m_pcurrent_icon_array->GetCount() ; ii++)
      {
            pmi = (MarkIcon *)m_pcurrent_icon_array->Item(ii);
            wxImage icon_image = pmi->picon_bitmap->ConvertToImage();

            // toh, 10.09.29
            // After limiting size user defined icons will be cut off
            // Work-around: rescale in one or both directions
            int h0 = icon_image.GetHeight();
            int w0 = icon_image.GetWidth();

            wxImage icon_larger;
            if (h0 <= h && w0 <= w)
            {
                        // Just resize
                  icon_larger = icon_image.Resize(wxSize(h,w), wxPoint(0,0));
            }
            else
            {
                        // rescale in one or two directions to avoid cropping, then resize to fit to cell
                  int h1 = h;
                  int w1 = w;
                  if (h0 > h)
                        w1 = wxRound((double)w0 * ((double)h/(double)h0));

                  else if (w0 > w)
                        h1 = wxRound((double)h0 * ((double)w/(double)w0));

                  icon_larger = icon_image.Rescale(w1, h1);
                  icon_larger = icon_larger.Resize(wxSize(h,w), wxPoint(0,0));
            }

            pmarkicon_image_list->Add(icon_larger);
       }

      return pmarkicon_image_list;
}

wxBitmap *WayPointman::CreateDimBitmap(wxBitmap *pBitmap, double factor)
{
      wxImage img = pBitmap->ConvertToImage();
      int sx = img.GetWidth();
      int sy = img.GetHeight();

      wxImage new_img(img);

      for(int i = 0 ; i < sx ; i++)
      {
            for(int j = 0 ; j < sy ; j++)
            {
                  if(!img.IsTransparent(i,j))
                  {
                        new_img.SetRGB(i, j, (unsigned char)(img.GetRed(i, j) * factor),
                                             (unsigned char)(img.GetGreen(i, j) * factor),
                                             (unsigned char)(img.GetBlue(i, j) * factor));
                  }
            }
      }

      wxBitmap *pret = new wxBitmap(new_img);

      return pret;

}

void WayPointman::SetColorScheme(ColorScheme cs)
{
      switch(cs)
      {
            case GLOBAL_COLOR_SCHEME_DAY:
                  m_pcurrent_icon_array = &DayIconArray;
                  break;
            case GLOBAL_COLOR_SCHEME_DUSK:
                  m_pcurrent_icon_array = &DuskIconArray;
                  break;
            case GLOBAL_COLOR_SCHEME_NIGHT:
                  m_pcurrent_icon_array = &NightIconArray;
                  break;
            default:
                  m_pcurrent_icon_array = &DayIconArray;
                  break;
      }

      //    Iterate on the RoutePoint list, requiring each to reload icon

      wxRoutePointListNode *node = m_pWayPointList->GetFirst();
      while(node)
      {
            RoutePoint *pr = node->GetData();
            pr->ReLoadIcon();
            node = node->GetNext();
      }
}


bool WayPointman::DoesIconExist(const wxString icon_key)
{
      MarkIcon *pmi;
      unsigned int i;

      for( i = 0 ; i< m_pcurrent_icon_array->GetCount() ; i++)
      {
            pmi = (MarkIcon *)m_pcurrent_icon_array->Item(i);
            if(pmi->icon_name.IsSameAs(icon_key))
                  return true;
      }

      return false;
}



wxBitmap *WayPointman::GetIconBitmap(const wxString& icon_key)
{
      wxBitmap *pret = NULL;
      MarkIcon *pmi = NULL;
      unsigned int i;

      for( i = 0 ; i< m_pcurrent_icon_array->GetCount() ; i++)
      {
            pmi = (MarkIcon *)m_pcurrent_icon_array->Item(i);
            if(pmi->icon_name.IsSameAs(icon_key))
                  break;
      }

      if(i == m_pcurrent_icon_array->GetCount())              // key not found
      {
            for( i = 0 ; i< m_pcurrent_icon_array->GetCount() ; i++)
            {
                  pmi = (MarkIcon *)m_pcurrent_icon_array->Item(i);
                  if(pmi->icon_name.IsSameAs(_T("circle")))
                        break;
            }
      }

      if(i == m_pcurrent_icon_array->GetCount())              // not found again
            pmi = (MarkIcon *)m_pcurrent_icon_array->Item(0);       // use item 0

      pret = pmi->picon_bitmap;


      return pret;
}

wxBitmap *WayPointman::GetIconBitmap(int index)
{
      wxBitmap *pret = NULL;

      if(index >= 0)
      {
            MarkIcon *pmi = (MarkIcon *)m_pcurrent_icon_array->Item(index);
            pret = pmi->picon_bitmap;
      }
      return pret;
}


wxString *WayPointman::GetIconDescription(int index)
{
      wxString *pret = NULL;

      if(index >= 0)
      {
            MarkIcon *pmi = (MarkIcon *)m_pcurrent_icon_array->Item(index);
            pret = &pmi->icon_description;
      }
      return pret;
}

wxString *WayPointman::GetIconKey(int index)
{
      wxString *pret = NULL;

      if(index >= 0)
      {
            MarkIcon *pmi = (MarkIcon *)m_pcurrent_icon_array->Item(index);
            pret = &pmi->icon_name;
      }
      return pret;
}

int WayPointman::GetIconIndex(const wxBitmap *pbm)
{
      unsigned int i;

      for( i = 0 ; i< m_pcurrent_icon_array->GetCount() ; i++)
      {
            MarkIcon *pmi = (MarkIcon *)m_pcurrent_icon_array->Item(i);
            if(pmi->picon_bitmap == pbm)
                  break;
      }

      return i;

}

     //  Create the unique identifier

wxString WayPointman::CreateGUID(RoutePoint *pRP)
{
      //FIXME: this method is not needed at all (if GetUUID works...)
      /*wxDateTime now = wxDateTime::Now();
      time_t ticks = now.GetTicks();
      wxString GUID;
      GUID.Printf(_T("%d-%d-%d-%d"), ((int)fabs(pRP->m_lat * 1e4)), ((int)fabs(pRP->m_lon * 1e4)), (int)ticks, m_nGUID);

      m_nGUID++;

      return GUID;*/
      return GpxDocument::GetUUID();
}

RoutePoint *WayPointman::FindRoutePointByGUID(wxString &guid)
{
            wxRoutePointListNode *prpnode = pWayPointMan->m_pWayPointList->GetFirst();
            while ( prpnode )
            {
                  RoutePoint *prp = prpnode->GetData();

                  if ( prp->m_GUID == guid )
                      return ( prp );

                  prpnode = prpnode->GetNext(); //RoutePoint
            }

            return NULL;
}

RoutePoint *WayPointman::GetNearbyWaypoint(double lat, double lon, double radius_meters)
{
      //    Iterate on the RoutePoint list, checking distance

      wxRoutePointListNode *node = m_pWayPointList->GetFirst();
      while(node)
      {
            RoutePoint *pr = node->GetData();

            double a = lat - pr->m_lat;
            double b = lon - pr->m_lon;
            double l = sqrt((a*a) + (b*b));

            if((l * 60. * 1852.) < radius_meters)
                  return pr;

            node = node->GetNext();
      }
      return NULL;

}

void WayPointman::DeleteAllWaypoints(bool b_delete_used)
{
      //    Iterate on the RoutePoint list, deleting all

      wxRoutePointListNode *node = m_pWayPointList->GetFirst();
      while(node)
      {
            RoutePoint *prp = node->GetData();

            if ( b_delete_used || ((!prp->m_bIsInRoute) && (!prp->m_bIsInTrack)     // if argument is false, then only delete non-route waypoints
                 && !(prp == pAnchorWatchPoint1) && !(prp == pAnchorWatchPoint2) )  )
            {
                  if (prp == pAnchorWatchPoint1)
                        pAnchorWatchPoint1 = NULL;
                  else if (prp == pAnchorWatchPoint2)
                        pAnchorWatchPoint2 = NULL;
                  pConfig->m_bIsImporting = true;
                  pConfig->DeleteWayPoint ( prp );
                  pSelect->DeleteSelectablePoint ( prp, SELTYPE_ROUTEPOINT );
                  delete prp;
                  node = m_pWayPointList->GetFirst();
                  pConfig->m_bIsImporting = false;
            }
            else
                  node = node->GetNext();
      }
      return;

}


void WayPointman::DestroyWaypoint(RoutePoint *pRp)
{
      if(pRp)
      {
            // Get a list of all routes containing this point
            // and remove the point from them all
            wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining(pRp);
            if(proute_array)
            {
                  for(unsigned int ir=0 ; ir < proute_array->GetCount() ; ir++)
                  {
                        Route *pr = (Route *)proute_array->Item(ir);

                        if ( g_pRouteMan->GetpActiveRoute() == pr )            // Deactivate any route containing this point
                              g_pRouteMan->DeactivateRoute();

                        pr->RemovePoint ( pRp );

                  }

                  //    Scrub the routes, looking for one-point routes
                  for(unsigned int ir=0 ; ir < proute_array->GetCount() ; ir++)
                  {
                        Route *pr = (Route *)proute_array->Item(ir);
                        if(pr->GetnPoints() < 2)
                        {
                              pConfig->m_bIsImporting = true;
                              pConfig->DeleteConfigRoute ( pr );
                              g_pRouteMan->DeleteRoute ( pr );
                              pConfig->m_bIsImporting = false;
                        }
                  }

                  delete proute_array;
            }

            // Now it is safe to delete the point
            pConfig->DeleteWayPoint ( pRp );
            pSelect->DeleteSelectablePoint ( pRp, SELTYPE_ROUTEPOINT );

            //TODO  FIXME
            // Some memory corruption occurs if the wp is deleted here.
            // To continue running OK, it is sufficient to simply remove the wp from the global list
            // This will leak, although called infrequently....
            //  12/15/10...Seems to occur only on MOB delete....

            if ( NULL != pWayPointMan )
                  pWayPointMan->m_pWayPointList->DeleteObject ( pRp );
//            delete pRp;

            //    The RoutePoint might be currently in use as an anchor watch point
            if(pRp == pAnchorWatchPoint1)
                  pAnchorWatchPoint1 = NULL;
            if(pRp == pAnchorWatchPoint2)
                  pAnchorWatchPoint2 = NULL;

      }
}
