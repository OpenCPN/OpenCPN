/******************************************************************************
 * $Id: routeman.h,v 1.15 2010/06/21 01:55:22 bdbcat Exp $
 *
 * Project:  OpenCP
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
 * $Log: routeman.h,v $
 * Revision 1.15  2010/06/21 01:55:22  bdbcat
 * 620
 *
 * Revision 1.14  2010/06/04 22:36:37  bdbcat
 * 604
 *
 * Revision 1.13  2010/04/27 01:45:21  bdbcat
 * Build 426
 *
 * Revision 1.12  2010/04/16 23:17:23  bdbcat
 * Updates.
 *
 * Revision 1.11  2010/04/15 15:52:30  bdbcat
 * Build 415.
 *
 * Revision 1.10  2010/03/29 02:59:02  bdbcat
 * 2.1.0 Beta Initial
 *
 */


#ifndef __ROUTEMAN_H__
#define __ROUTEMAN_H__


#include "chart1.h"                 // for ColorScheme definition
#include <wx/imaglist.h>

#include "nmea0183.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif



//    Constants for SendToGps... Dialog
#define ID_STGDIALOG 10005
#define SYMBOL_STG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_STG_TITLE _("Send Route To GPS")
#define SYMBOL_STG_IDNAME ID_STGDIALOG
#define SYMBOL_STG_SIZE wxSize(500, 500)
#define SYMBOL_STG_POSITION wxDefaultPosition

enum {
      ID_STG_CANCEL =            10000,
      ID_STG_OK,
      ID_STG_CHOICE_COMM
};

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

class MarkIcon
{
      public:
            wxBitmap   *picon_bitmap;
            wxString   icon_name;
            wxString   icon_description;
};


//----------------------------------------------------------------------------
//   Routeman
//----------------------------------------------------------------------------

class Routeman
{
public:
      Routeman(MyApp *parent);
      ~Routeman();

      void AssembleAllRoutes(void);
      void DeleteRoute(Route *pRoute);
      void DeleteAllRoutes(void);
      void DeleteAllTracks(void);

      void DeleteTrack(Route *pRoute);

      bool IsRouteValid(Route *pRoute);

      Route *FindRouteContainingWaypoint(RoutePoint *pWP);
      wxArrayPtrVoid *GetRouteArrayContaining(RoutePoint *pWP);

      bool ActivateRoute(Route *pRouteToActivate, RoutePoint *pStartPoint = NULL);
      bool ActivateRoutePoint(Route *pA, RoutePoint *pRP);
      bool ActivateNextPoint(Route *pr);
      RoutePoint *FindBestActivatePoint(Route *pR, double lat, double lon, double cog, double sog);

      bool UpdateProgress();
      bool UpdateAutopilot();
      bool DeactivateRoute();
      bool IsAnyRouteActive(void){ return (pActiveRoute != NULL); }
      void SetColorScheme(ColorScheme cs);

      Route *GetpActiveRoute(){ return pActiveRoute;}
      RoutePoint *GetpActivePoint(){ return pActivePoint;}
      double GetCurrentRngToActivePoint(){ return CurrentRngToActivePoint;}
      double GetCurrentBrgToActivePoint(){ return CurrentBrgToActivePoint;}
      double GetCurrentRngToActiveNormalArrival(){ return CurrentRangeToActiveNormalCrossing;}
      double GetCurrentXTEToActivePoint(){ return CurrentXTEToActivePoint;}
      double GetCurrentSegmentCourse(){ return CurrentSegmentCourse;}
      int   GetXTEDir(){ return XTEDir;}

      wxPen   * GetRoutePen(void){return m_pRoutePen;}
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

};

//----------------------------------------------------------------------------
//   Route "Send to GPS..." Dialog Definition
//----------------------------------------------------------------------------

class SendToGpsDlg : public wxDialog
{
      DECLARE_DYNAMIC_CLASS( SendToGpsDlg )
      DECLARE_EVENT_TABLE()

 public:
       SendToGpsDlg();
       SendToGpsDlg(  wxWindow* parent, wxWindowID id, const wxString& caption, const wxString& hint, const wxPoint& pos, const wxSize& size, long style );
       ~SendToGpsDlg( );

       bool Create( wxWindow* parent, wxWindowID id = SYMBOL_STG_IDNAME, const wxString& caption = SYMBOL_STG_TITLE, const wxString& hint = SYMBOL_STG_TITLE,
                    const wxPoint& pos = SYMBOL_STG_POSITION, const wxSize& size = SYMBOL_STG_SIZE,
                    long style = SYMBOL_STG_STYLE);
       void SetRoute(Route *pRoute){m_pRoute = pRoute;}
       void SetWaypoint(RoutePoint *pRoutePoint){m_pRoutePoint = pRoutePoint;}

private:
      void CreateControls(const wxString& hint);

      void OnCancelClick( wxCommandEvent& event );
      void OnSendClick( wxCommandEvent& event );

      Route       *m_pRoute;
      RoutePoint  *m_pRoutePoint;
      wxComboBox  *m_itemCommListBox;
      wxGauge     *m_pgauge;
      wxButton    *m_CancelButton;
      wxButton    *m_SendButton;

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
      int GetIconIndex(const wxBitmap *pbm);
      int GetNumIcons(void){ return m_nIcons; }
      wxString CreateGUID(RoutePoint *pRP);
      RoutePoint *GetNearbyWaypoint(double lat, double lon, double radius_meters);
      RoutePoint *GetOtherNearbyWaypoint(double lat, double lon, double radius_meters, wxString &guid);
      void SetColorScheme(ColorScheme cs);
      void DeleteAllWaypoints(bool b_delete_used);
      RoutePoint *FindRoutePointByGUID(wxString &guid);
      void DestroyWaypoint(RoutePoint *pRp);


      bool DoesIconExist(const wxString icon_key);
      wxBitmap *GetIconBitmap(int index);
      wxString *GetIconDescription(int index);
      wxString *GetIconKey(int index);

      wxImageList *Getpmarkicon_image_list(void);

      RoutePointList    *m_pWayPointList;

private:
      void ProcessIcon(wxImage *pimage, wxString key, wxString description);

      wxBitmap *CreateDimBitmap(wxBitmap *pBitmap, double factor);

      int m_nIcons;

      wxImageList       *pmarkicon_image_list;        // Current wxImageList, updated on colorscheme change

      wxArrayPtrVoid    DayIconArray;
      wxArrayPtrVoid    DuskIconArray;
      wxArrayPtrVoid    NightIconArray;

      wxArrayPtrVoid    *m_pcurrent_icon_array;

      int         m_nGUID;
};

#endif
