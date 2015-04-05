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

#ifdef OCPN_USE_PORTAUDIO
#include "portaudio.h"
#endif

#include "bbox.h"
#include "chcanv.h"
#include "tinyxml.h"
#include "chartdbs.h"
#include "RoutePoint.h"
#include "vector2D.h"
#include "Route.h"
#include "SelectItem.h"

enum
{
    DISTANCE_NMI = 0,
    DISTANCE_MI,
    DISTANCE_KM,
    DISTANCE_M,
    DISTANCE_FT,
    DISTANCE_FA,
    DISTANCE_IN,
    DISTANCE_CM
};

enum
{
    SPEED_KTS = 0,
    SPEED_MPH,
    SPEED_KMH,
    SPEED_MS
};

extern bool LogMessageOnce(const wxString &msg);
extern double toUsrDistance( double nm_distance, int unit = -1 );
extern double fromUsrDistance( double usr_distance, int unit = -1 );
extern double toUsrSpeed( double kts_speed, int unit = -1 );
extern double fromUsrSpeed( double usr_speed, int unit = -1 );
extern wxString getUsrDistanceUnit( int unit = -1 );
extern wxString getUsrSpeedUnit( int unit = -1 );
extern wxString toSDMM(int NEflag, double a, bool hi_precision = true);
extern void AlphaBlending( ocpnDC& dc, int x, int y, int size_x, int size_y, float radius,
                                      wxColour color, unsigned char transparency );

extern double fromDMM(wxString sdms);
extern double parseLatLon(wxString latlon);

class Route;
class NavObjectCollection;
class wxProgressDialog;
class ocpnDC;
class NavObjectCollection1;
class NavObjectChanges;

//----------------------------------------------------------------------------
//    Track
//----------------------------------------------------------------------------
class Track : public wxEvtHandler, public Route
{
      public:
            Track(void);
            ~Track(void);

            void SetPrecision(int precision);

            void Start(void);
            void Stop(bool do_add_point = false);
            Track *DoExtendDaily(void);
            bool IsRunning(){ return m_bRunning; }
            void Draw(ocpnDC& dc, ViewPort &VP);

            RoutePoint* AddNewPoint( vector2D point, wxDateTime time );
            Route *RouteFromTrack(wxProgressDialog *pprog);

            void DouglasPeuckerReducer( std::vector<RoutePoint*>& list, int from, int to, double delta );
            int Simplify( double maxDelta );
            double GetXTE(RoutePoint *fm1, RoutePoint *fm2, RoutePoint *to);
            double GetXTE( double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon, double toLat, double toLon  );
            int GetCurrentTrackSeg(){ return m_CurrentTrackSeg; }
            void SetCurrentTrackSeg(int seg){ m_CurrentTrackSeg = seg; }
            
            void AdjustCurrentTrackPoint( RoutePoint *prototype );
            
      private:
            void OnTimerTrack(wxTimerEvent& event);
            void AddPointNow(bool do_add_point = false);

            bool              m_bRunning;
            wxTimer           m_TimerTrack;

            int               m_nPrecision;
            double            m_TrackTimerSec;
            double            m_allowedMaxXTE;
            double            m_allowedMaxAngle;

            vector2D          m_lastAddedPoint;
            double            m_prev_dist;
            wxDateTime        m_prev_time;

            RoutePoint        *m_lastStoredTP;
            RoutePoint        *m_removeTP;
            RoutePoint        *m_prevFixedTP;
            RoutePoint        *m_fixedTP;
            int               m_track_run;
            double            m_minTrackpoint_delta;
            int               m_CurrentTrackSeg;
            
            enum eTrackPointState {
                firstPoint,
                secondPoint,
                potentialPoint
            } trackPointState;

            std::deque<vector2D> skipPoints;
            std::deque<wxDateTime> skipTimes;

DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
//    Static XML Helpers
//----------------------------------------------------------------------------

//RoutePoint *LoadGPXWaypoint (GpxWptElement *wptnode, wxString def_symbol_name, bool b_fullviz = false );
//Route *LoadGPXRoute (GpxRteElement *rtenode, int routenum, bool b_fullviz = false );
//Route *LoadGPXTrack (GpxTrkElement *trknode, bool b_fullviz = false );
//void GPXLoadTrack ( GpxTrkElement *trknode, bool b_fullviz = false  );
//void GPXLoadRoute ( GpxRteElement *rtenode, int routenum, bool b_fullviz = false );
//void InsertRoute(Route *pTentRoute, int routenum);
//void UpdateRoute(Route *pTentRoute);

GpxWptElement *CreateGPXWpt ( RoutePoint *pr, char * waypoint_type, bool b_props_explicit = false, bool b_props_minimal = false );
GpxRteElement *CreateGPXRte ( Route *pRoute );
GpxTrkElement *CreateGPXTrk ( Route *pRoute );

bool WptIsInRouteList(RoutePoint *pr);
RoutePoint *WaypointExists( const wxString& name, double lat, double lon);
RoutePoint *WaypointExists( const wxString& guid);
Route *RouteExists( const wxString& guid);
Route *RouteExists( Route * pTentRoute );
const wxChar *ParseGPXDateTime( wxDateTime &dt, const wxChar *datetime );

//----------------------------------------------------------------------------
//    Config
//----------------------------------------------------------------------------
class MyConfig:public wxFileConfig
{
public:

      MyConfig(const wxString &appName, const wxString &vendorName,
                              const wxString &LocalFileName);

      int LoadMyConfig();
      void LoadS57Config();
      void LoadNavObjects();
      virtual bool AddNewRoute(Route *pr, int ConfigRouteNum = -1);
      virtual bool UpdateRoute(Route *pr);
      virtual bool DeleteConfigRoute(Route *pr);

      virtual bool AddNewWayPoint(RoutePoint *pWP, int ConfigRouteNum = -1);
      virtual bool UpdateWayPoint(RoutePoint *pWP);
      virtual bool DeleteWayPoint(RoutePoint *pWP);
      virtual bool AddNewTrackPoint( RoutePoint *pWP, const wxString& parent_GUID );
      
      virtual void CreateConfigGroups ( ChartGroupArray *pGroupArray );
      virtual void DestroyConfigGroups ( void );
      virtual void LoadConfigGroups ( ChartGroupArray *pGroupArray );


      virtual bool UpdateChartDirs(ArrayOfCDI &dirarray);
      virtual bool LoadChartDirArray(ArrayOfCDI &ChartDirArray);
      virtual void UpdateSettings();
      virtual void UpdateNavObj();

      bool LoadLayers(wxString &path);

      void ExportGPX(wxWindow* parent, bool bviz_only = false, bool blayer = false);
      void UI_ImportGPX(wxWindow* parent, bool islayer = false, wxString dirpath = _T(""), bool isdirectory = true);

      bool ExportGPXRoutes(wxWindow* parent, RouteList *pRoutes, const wxString suggestedName = _T("routes"));
      bool ExportGPXWaypoints(wxWindow* parent, RoutePointList *pRoutePoints, const wxString suggestedName = _T("waypoints"));

      void CreateRotatingNavObjBackup();

      double st_lat, st_lon, st_view_scale, st_rotation;      // startup values
      bool  st_bFollow;

      wxString                m_gpx_path;

      wxString                m_sNavObjSetFile;
      wxString                m_sNavObjSetChangesFile;

      NavObjectChanges        *m_pNavObjectChangesSet;
      NavObjectCollection1    *m_pNavObjectInputSet;
      bool                    m_bSkipChangeSetUpdate;
      
//    These members are set/reset in Options dialog
      bool  m_bShowMenuBar, m_bShowCompassWin;



};


/*
#include <wx/fontdlg.h>

class WXDLLEXPORT X11FontPicker : public wxGenericFontDialog
{
public:
      X11FontPicker(wxFrame *parent);
      ~X11FontPicker();

      virtual void CreateWidgets();


};
*/

/*
 * X11FontPicker DIALOG
 */
#include <wx/fontdlg.h>

class wxChoice;
class WXDLLEXPORT wxText;
class wxCheckBox;
class WXDLLEXPORT MyFontPreviewer;

/*
enum
{
      wxID_FONT_UNDERLINE = 3000,
      wxID_FONT_STYLE,
      wxID_FONT_WEIGHT,
      wxID_FONT_FAMILY,
      wxID_FONT_COLOUR,
      wxID_FONT_SIZE
};
*/

class WXDLLEXPORT X11FontPicker : public wxFontDialogBase
{
      public:
            X11FontPicker() { Init(); }
            X11FontPicker(wxWindow *parent, const wxFontData& data)  : wxFontDialogBase(parent, data) { Init(); }
            virtual ~X11FontPicker();

            virtual int ShowModal();


    // deprecated, for backwards compatibility only
//            X11FontPicker(wxWindow *parent, const wxFontData *data)
//      : wxFontDialogBase(parent, data) { Init(); }

    // Internal functions
            void OnCloseWindow(wxCloseEvent& event);

            virtual void CreateWidgets();
            virtual void InitializeFont();

            void OnChangeFont(wxCommandEvent& event);
            void OnChangeFace(wxCommandEvent& event);

      protected:
    // common part of all ctors
            void Init();

            virtual bool DoCreate(wxWindow *parent);
            void InitializeAllAvailableFonts();
            void SetChoiceOptionsFromFacename(const wxString &facename);
            void DoFontChange(void);

            wxFont dialogFont;

            wxChoice    *familyChoice;
            wxChoice    *styleChoice;
            wxChoice    *weightChoice;
            wxChoice    *colourChoice;
            wxCheckBox  *underLineCheckBox;
            wxChoice    *pointSizeChoice;

            MyFontPreviewer *m_previewer;
            bool        m_useEvents;

            wxArrayString     *pFaceNameArray;

            wxFont            *pPreviewFont;

    //  static bool fontDialogCancelled;
            DECLARE_EVENT_TABLE()
                        DECLARE_DYNAMIC_CLASS(X11FontPicker)
};


//      Simple and fast CRC32 calculator

extern "C" unsigned int crc32buf(unsigned char *buf, size_t len);


#endif
