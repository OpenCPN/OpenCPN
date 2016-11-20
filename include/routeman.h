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

#ifndef __ROUTEMAN_H__
#define __ROUTEMAN_H__


#include "chart1.h"                 // for ColorScheme definition
#include <wx/imaglist.h>
#include "styles.h"
#include "Select.h"
#include "nmea0183.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif



//----------------------------------------------------------------------------
//    forward class declarations
//----------------------------------------------------------------------------

class Route;
class RoutePoint;
class RoutePointList;

//    List definitions for Waypoint Manager Icons

class markicon_bitmap_list_type;
class markicon_key_list_type;
class markicon_description_list_type;

//----------------------------------------------------------------------------
//   Routeman
//----------------------------------------------------------------------------

class Routeman
{
public:
      Routeman(MyApp *parent);
      ~Routeman();

      bool DeleteRoute(Route *pRoute);
      void DeleteAllRoutes(void);
      void DeleteAllTracks(void);

      void DeleteTrack(Track *pTrack);

      bool IsRouteValid(Route *pRoute);

      Route *FindRouteByGUID(const wxString &guid);
      Track *FindTrackByGUID(const wxString &guid);
      Route *FindRouteContainingWaypoint(RoutePoint *pWP);
      wxArrayPtrVoid *GetRouteArrayContaining(RoutePoint *pWP);
      bool DoesRouteContainSharedPoints( Route *pRoute );

      bool ActivateRoute(Route *pRouteToActivate, RoutePoint *pStartPoint = NULL);
      bool ActivateRoutePoint(Route *pA, RoutePoint *pRP);
      bool ActivateNextPoint(Route *pr, bool skipped);
      RoutePoint *FindBestActivatePoint(Route *pR, double lat, double lon, double cog, double sog);

      bool UpdateProgress();
      bool UpdateAutopilot();
      bool DeactivateRoute( bool b_arrival = false );
      bool IsAnyRouteActive(void){ return (pActiveRoute != NULL); }
      void SetColorScheme(ColorScheme cs);

      Route *GetpActiveRoute(){ return pActiveRoute;}
      RoutePoint *GetpActivePoint(){ return pActivePoint;}
      double GetCurrentRngToActivePoint(){ return CurrentRngToActivePoint;}
      double GetCurrentBrgToActivePoint(){ return CurrentBrgToActivePoint;}
      double GetCurrentRngToActiveNormalArrival(){ return CurrentRangeToActiveNormalCrossing;}
      double GetCurrentXTEToActivePoint(){ return CurrentXTEToActivePoint;}
      void   ZeroCurrentXTEToActivePoint();
      double GetCurrentSegmentCourse(){ return CurrentSegmentCourse;}
      int   GetXTEDir(){ return XTEDir;}

      wxPen   * GetRoutePen(void){return m_pRoutePen;}
      wxPen   * GetTrackPen(void){return m_pTrackPen;}
      wxPen   * GetSelectedRoutePen(void){return m_pSelectedRoutePen;}
      wxPen   * GetActiveRoutePen(void){return m_pActiveRoutePen;}
      wxPen   * GetActiveRoutePointPen(void){return m_pActiveRoutePointPen;}
      wxPen   * GetRoutePointPen(void){return m_pRoutePointPen;}
      wxBrush * GetRouteBrush(void){return m_pRouteBrush;}
      wxBrush * GetSelectedRouteBrush(void){return m_pSelectedRouteBrush;}
      wxBrush * GetActiveRouteBrush(void){return m_pActiveRouteBrush;}
      wxBrush * GetActiveRoutePointBrush(void){return m_pActiveRoutePointBrush;}
      wxBrush * GetRoutePointBrush(void){return m_pRoutePointBrush;}

      wxString GetRouteReverseMessage(void);

      bool        m_bDataValid;

private:
      void DoAdvance(void);
    
      MyApp       *m_pparent_app;
      Route       *pActiveRoute;
      RoutePoint  *pActivePoint;
      double       RouteBrgToActivePoint;        //TODO all these need to be doubles
      double       CurrentSegmentBeginLat;
      double       CurrentSegmentBeginLon;
      double       CurrentRngToActivePoint;
      double       CurrentBrgToActivePoint;
      double       CurrentXTEToActivePoint;
      double       CourseToRouteSegment;
      double       CurrentRangeToActiveNormalCrossing;
      RoutePoint  *pActiveRouteSegmentBeginPoint;
      RoutePoint  *pRouteActivatePoint;
      double       CurrentSegmentCourse;
      int         XTEDir;
      bool        m_bArrival;
      wxPen       *m_pRoutePen;
      wxPen       *m_pTrackPen;
      wxPen       *m_pSelectedRoutePen;
      wxPen       *m_pActiveRoutePen;
      wxPen       *m_pActiveRoutePointPen;
      wxPen       *m_pRoutePointPen;
      wxBrush     *m_pRouteBrush;
      wxBrush     *m_pSelectedRouteBrush;
      wxBrush     *m_pActiveRouteBrush;
      wxBrush     *m_pActiveRoutePointBrush;
      wxBrush     *m_pRoutePointBrush;

      NMEA0183    m_NMEA0183;                         // For autopilot output
      
      double      m_arrival_min;
      int         m_arrival_test;
      

};


//----------------------------------------------------------------------------
//   WayPointman
//----------------------------------------------------------------------------

class WayPointman
{
public:
      WayPointman();
      ~WayPointman();
      wxBitmap *GetIconBitmap(const wxString& icon_key);
      unsigned int GetIconTexture( const wxBitmap *pmb, int &glw, int &glh );
      int GetIconIndex(const wxBitmap *pbm);
      int GetXIconIndex(const wxBitmap *pbm);
      int GetNumIcons(void){ return m_pIconArray->Count(); }
      wxString CreateGUID(RoutePoint *pRP);
      RoutePoint *GetNearbyWaypoint(double lat, double lon, double radius_meters);
      RoutePoint *GetOtherNearbyWaypoint(double lat, double lon, double radius_meters, const wxString &guid);
      void SetColorScheme(ColorScheme cs);
      bool SharedWptsExist();
      void DeleteAllWaypoints(bool b_delete_used);
      RoutePoint *FindRoutePointByGUID(const wxString &guid);
      void DestroyWaypoint(RoutePoint *pRp, bool b_update_changeset = true);
      void ClearRoutePointFonts(void);
      void ProcessIcons( ocpnStyle::Style* style );
      
      bool DoesIconExist(const wxString & icon_key) const;
      wxBitmap *GetIconBitmap(int index);
      wxString *GetIconDescription(int index);
      wxString *GetIconKey(int index);

      wxImageList *Getpmarkicon_image_list(void);

      bool AddRoutePoint(RoutePoint *prp);
      bool RemoveRoutePoint(RoutePoint *prp);
      RoutePointList *GetWaypointList(void) { return m_pWayPointList; }

      void ProcessIcon(wxBitmap pimage, const wxString & key, const wxString & description);
private:
      void ProcessUserIcons( ocpnStyle::Style* style );
      RoutePointList    *m_pWayPointList;
      wxBitmap *CreateDimBitmap(wxBitmap *pBitmap, double factor);

      wxImageList       *pmarkicon_image_list;        // Current wxImageList, updated on colorscheme change
      int               m_markicon_image_list_base_count;
      wxArrayPtrVoid    *m_pIconArray;

      int         m_nGUID;
};

#endif
