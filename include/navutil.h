/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions
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

#ifndef __NAVUTIL__
#define __NAVUTIL__

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <wx/sound.h>

#ifdef __WXMSW__
#include <wx/msw/regconf.h>
#include <wx/msw/iniconf.h>
#endif

#include "bbox.h"
//#include "chcanv.h"
#include "chartdbs.h"
// nclude "RoutePoint.h"
#include "vector2D.h"
#include "select_item.h"
#include "ocpndc.h"
#include "navutil_base.h"


enum { TEMPERATURE_C = 0, TEMPERATURE_F = 1, TEMPERATURE_K = 2 };


extern bool LogMessageOnce(const wxString &msg);
extern double fromUsrDistance(double usr_distance, int unit = -1);
extern double fromUsrSpeed(double usr_speed, int unit = -1);
extern double toUsrTemp(double cel_temp, int unit = -1);
extern double fromUsrTemp(double usr_temp, int unit = -1);
extern wxString getUsrTempUnit(int unit = -1);
extern wxString formatAngle(double angle);

extern void AlphaBlending(ocpnDC &dc, int x, int y, int size_x, int size_y,
                          float radius, wxColour color,
                          unsigned char transparency);

// Central dimmer...
void DimeControl(wxWindow *ctrl);
void DimeControl(wxWindow *ctrl, wxColour col, wxColour col1,
                 wxColour back_color, wxColour text_color, wxColour uitext,
                 wxColour udkrd, wxColour gridline);


class Route;
class NavObjectCollection;
class wxGenericProgressDialog;
class ocpnDC;
class NavObjectCollection1;
class NavObjectChanges;
class TrackPoint;
class RouteList;
class canvasConfig;
class RoutePointList;
class RoutePoint;
class Track;

//----------------------------------------------------------------------------
//    Static XML Helpers
//----------------------------------------------------------------------------

// RoutePoint *LoadGPXWaypoint (GpxWptElement *wptnode, wxString
// def_symbol_name, bool b_fullviz = false ); Route *LoadGPXRoute (GpxRteElement
// *rtenode, int routenum, bool b_fullviz = false ); Route *LoadGPXTrack
// (GpxTrkElement *trknode, bool b_fullviz = false ); void GPXLoadTrack (
// GpxTrkElement *trknode, bool b_fullviz = false  ); void GPXLoadRoute (
// GpxRteElement *rtenode, int routenum, bool b_fullviz = false ); void
// InsertRoute(Route *pTentRoute, int routenum); void UpdateRoute(Route
// *pTentRoute);

// GpxWptElement *CreateGPXWpt ( RoutePoint *pr, char * waypoint_type, bool
// b_props_explicit = false, bool b_props_minimal = false ); GpxRteElement
// *CreateGPXRte ( Route *pRoute ); GpxTrkElement *CreateGPXTrk ( Route *pRoute
// );

bool WptIsInRouteList(RoutePoint *pr);
RoutePoint *WaypointExists(const wxString &name, double lat, double lon);
RoutePoint *WaypointExists(const wxString &guid);
Route *RouteExists(const wxString &guid);
Route *RouteExists(Route *pTentRoute);
Track *TrackExists(const wxString &guid);

void ExportGPX(wxWindow *parent, bool bviz_only = false, bool blayer = false);
void UI_ImportGPX(wxWindow *parent, bool islayer = false,
                  wxString dirpath = _T(""), bool isdirectory = true,
                  bool isPersistent = false);

bool ExportGPXRoutes(wxWindow *parent, RouteList *pRoutes,
                     const wxString suggestedName = _T("routes"));
bool ExportGPXTracks(wxWindow *parent, std::vector<Track*> *pRoutes,
                     const wxString suggestedName = _T("tracks"));
bool ExportGPXWaypoints(wxWindow *parent, RoutePointList *pRoutePoints,
                        const wxString suggestedName = _T("waypoints"));


class MouseZoom {
public:

  /** Convert a slider scale 1-100 value to configuration value 1.02..3.0. */
  static double ui_to_config(int slider_pos) {
    return (2.0/100) * static_cast<double>(slider_pos) + 1.02;
  }

  /** Convert configuration 1.02..3.0 value to slider scale 1..100. */
  static int config_to_ui(double value) {
    return std::round((100.0 * (static_cast<double>(value) - 1.02)) / 2.0);
  }
};


//----------------------------------------------------------------------------
//    Config
//----------------------------------------------------------------------------
class MyConfig : public wxFileConfig {
public:
  MyConfig(const wxString &LocalFileName);
  ~MyConfig();

  int LoadMyConfig();
  void LoadS57Config();
  void LoadNavObjects();
  virtual void AddNewRoute(Route *pr);
  virtual void UpdateRoute(Route *pr);
  virtual void DeleteConfigRoute(Route *pr);

  virtual void AddNewTrack(Track *pt);
  virtual void UpdateTrack(Track *pt);
  virtual void DeleteConfigTrack(Track *pt);

  virtual void AddNewWayPoint(RoutePoint *pWP, int ConfigRouteNum = -1);
  virtual void UpdateWayPoint(RoutePoint *pWP);
  virtual void DeleteWayPoint(RoutePoint *pWP);
  virtual void AddNewTrackPoint(TrackPoint *pWP, const wxString &parent_GUID);

  virtual void CreateConfigGroups(ChartGroupArray *pGroupArray);
  virtual void DestroyConfigGroups(void);
  virtual void LoadConfigGroups(ChartGroupArray *pGroupArray);

  virtual void LoadCanvasConfigs(bool bApplyAsTemplate = false);
  virtual void LoadConfigCanvas(canvasConfig *cConfig, bool bApplyAsTemplate);

  virtual void SaveCanvasConfigs();
  virtual void SaveConfigCanvas(canvasConfig *cc);

  virtual bool UpdateChartDirs(ArrayOfCDI &dirarray);
  virtual bool LoadChartDirArray(ArrayOfCDI &ChartDirArray);
  virtual void UpdateSettings();
  virtual void UpdateNavObj(bool bRecreate = false);
  virtual bool IsChangesFileDirty();

  bool LoadLayers(wxString &path);
  int LoadMyConfigRaw(bool bAsTemplate = false);

  void CreateRotatingNavObjBackup();

  wxString m_sNavObjSetFile;
  wxString m_sNavObjSetChangesFile;

  NavObjectChanges *m_pNavObjectChangesSet;
  NavObjectCollection1 *m_pNavObjectInputSet;
};

void SwitchInlandEcdisMode(bool Switch);


#endif
