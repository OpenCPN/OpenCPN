/******************************************************************************
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
 ***************************************************************************
 *
 *
 */


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
#include "s52s57.h"
#include "chcanv.h"
#include "tinyxml.h"
#include "gpxdocument.h"
#include "chartdbs.h"
#include "RoutePoint.h"
#include "vector2D.h"
#include "Route.h"
#include "SelectItem.h"

extern bool LogMessageOnce(wxString &msg);
extern wxString toSDMM(int NEflag, double a, bool hi_precision = true);
extern void AlphaBlending( ocpnDC& dc, int x, int y, int size_x, int size_y, float radius,
                                      wxColour color, unsigned char transparency );

extern double fromDMM(wxString sdms);
extern double parseLatLon(wxString latlon);

class Route;
class NavObjectCollection;
class wxProgressDialog;
class ocpnDC;

#define TIMER_TRACK1           778

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
            void FixMidnight(Track *pPreviousTrack);
            bool DoExtendDaily(void);
            bool IsRunning(){ return m_bRunning; }
            void Draw(ocpnDC& dc, ViewPort &VP);

            RoutePoint* AddNewPoint( vector2D point, wxDateTime time );
            Route *RouteFromTrack(wxProgressDialog *pprog);

            void DouglasPeuckerReducer( std::vector<RoutePoint*>& list, int from, int to, double delta );
            int Simplify( double maxDelta );
            double GetXTE(RoutePoint *fm1, RoutePoint *fm2, RoutePoint *to);
            double GetXTE( double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon, double toLat, double toLon  );

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
//    Layer
//----------------------------------------------------------------------------

class Layer
{
public:
      Layer(void);
      ~Layer(void);
      wxString CreatePropString(void) { return m_LayerFileName; }
      bool IsVisibleOnChart() { return m_bIsVisibleOnChart; }
      void SetVisibleOnChart(bool viz = true){ m_bIsVisibleOnChart = viz; }
      bool IsVisibleOnListing() { return m_bIsVisibleOnListing; }
      void SetVisibleOnListing(bool viz = true){ m_bIsVisibleOnListing = viz; }
      bool HasVisibleNames() { return m_bHasVisibleNames; }
      void SetVisibleNames(bool viz = true){ m_bHasVisibleNames = viz; }

      bool m_bIsVisibleOnChart;
      bool m_bIsVisibleOnListing;
      bool m_bHasVisibleNames;
      long m_NoOfItems;
      int m_LayerID;

      wxString          m_LayerName;
      wxString          m_LayerFileName;
      wxString          m_LayerDescription;
      wxDateTime        m_CreateTime;

};

WX_DECLARE_LIST(Layer, LayerList);// establish class as list member

//----------------------------------------------------------------------------
//    Static XML Helpers
//----------------------------------------------------------------------------

RoutePoint *LoadGPXWaypoint (GpxWptElement *wptnode, wxString def_symbol_name, bool b_fullviz = false );
Route *LoadGPXRoute (GpxRteElement *rtenode, int routenum, bool b_fullviz = false );
Route *LoadGPXTrack (GpxTrkElement *trknode, bool b_fullviz = false );
void GPXLoadTrack ( GpxTrkElement *trknode, bool b_fullviz = false  );
void GPXLoadRoute ( GpxRteElement *rtenode, int routenum, bool b_fullviz = false );
void InsertRoute(Route *pTentRoute, int routenum);
void UpdateRoute(Route *pTentRoute);

GpxWptElement *CreateGPXWpt ( RoutePoint *pr, char * waypoint_type, bool b_props_explicit = false, bool b_props_minimal = false );
GpxRteElement *CreateGPXRte ( Route *pRoute );
GpxTrkElement *CreateGPXTrk ( Route *pRoute );

bool WptIsInRouteList(RoutePoint *pr);
RoutePoint *WaypointExists( const wxString& name, double lat, double lon);
RoutePoint *WaypointExists( const wxString& guid);
Route *RouteExists( const wxString& guid);
Route *RouteExists( Route * pTentRoute );

//----------------------------------------------------------------------------
//    Config
//----------------------------------------------------------------------------
class MyConfig:public wxFileConfig
{
public:

      MyConfig(wxString & appName);
      MyConfig(const wxString &appName, const wxString &vendorName,
                              const wxString &LocalFileName);

      int LoadMyConfig(int iteration);
      virtual bool AddNewRoute(Route *pr, int ConfigRouteNum = -1);
      virtual bool UpdateRoute(Route *pr);
      virtual bool DeleteConfigRoute(Route *pr);

      virtual bool AddNewWayPoint(RoutePoint *pWP, int ConfigRouteNum = -1);
      virtual bool UpdateWayPoint(RoutePoint *pWP);
      virtual bool DeleteWayPoint(RoutePoint *pWP);

      virtual void CreateConfigGroups ( ChartGroupArray *pGroupArray );
      virtual void DestroyConfigGroups ( void );
      virtual void LoadConfigGroups ( ChartGroupArray *pGroupArray );


      virtual bool UpdateChartDirs(ArrayOfCDI &dirarray);
      virtual bool LoadChartDirArray(ArrayOfCDI &ChartDirArray);
      virtual void UpdateSettings();
      virtual void UpdateNavObj();
      virtual void StoreNavObjChanges();

      void ExportGPX(wxWindow* parent, bool bviz_only = false, bool blayer = false);
      void ImportGPX(wxWindow* parent, bool islayer = false, wxString dirpath = _T(""), bool isdirectory = true);

      bool ExportGPXRoute(wxWindow* parent, Route *pRoute);
      bool ExportGPXWaypoint(wxWindow* parent, RoutePoint *pRoutePoint);

      void CreateRotatingNavObjBackup();

      int m_NextRouteNum;
      int m_NextWPNum;

      double st_lat, st_lon, st_view_scale;            // startup values
      bool  st_bFollow;

      wxString    m_gpx_path;

      wxString                m_sNavObjSetFile;
      wxString                m_sNavObjSetChangesFile;

      NavObjectCollection     *m_pNavObjectInputSet;
      NavObjectCollection     *m_pNavObjectChangesSet;

//    These members are set/reset in Options dialog
      bool  m_bShowDebugWindows;

      bool  m_bIsImporting;


};



//---------------------------------------------------------------------------------
//          XML Based NavObjectSet
//---------------------------------------------------------------------------------

class NavObjectCollection : public GpxDocument
{
      public:
            NavObjectCollection();
            ~NavObjectCollection();

            bool CreateNavObjGPXPoints(void);
            bool CreateNavObjGPXRoutes(void);
            bool CreateNavObjGPXTracks(void);

            bool LoadAllGPXObjects(void);

      private:
            GpxRootElement   *m_pXMLrootnode;
            TiXmlNode   *m_proot_next;
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
            void SetChoiceOptionsFromFacename(wxString &facename);
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


//    TTYScroll and TTYWindow Definition
//    Scrolled TTY-like window for logging, etc....
class TTYScroll : public wxScrolledWindow
{
      public:
            TTYScroll(wxWindow *parent, int n_lines)
      : wxScrolledWindow(parent), m_nLines( n_lines )
            {
                  bpause = false;
                  wxClientDC dc(this);
                  dc.GetTextExtent(_T("Line Height"), NULL, &m_hLine);

                  SetScrollRate( 0, m_hLine );
                  SetVirtualSize( -1, ( m_nLines + 1 ) * m_hLine );
                  m_plineArray = new wxArrayString;
                  for(unsigned int i=0 ; i < m_nLines ; i++)
                        m_plineArray->Add(_T(""));
            }

            virtual ~TTYScroll()
            {
                  delete m_plineArray;
            }

            virtual void OnDraw(wxDC& dc);
            virtual void Add(wxString &line);
            void OnSize(wxSizeEvent& event);
            void Pause(bool pause) { bpause = pause; }

      protected:

            wxCoord m_hLine;        // the height of one line on screen
            size_t m_nLines;        // the number of lines we draw

            wxArrayString     *m_plineArray;
            bool               bpause;

};



class TTYWindow : public wxDialog
{
      DECLARE_DYNAMIC_CLASS( TTYWindow )
      DECLARE_EVENT_TABLE()

      public:
            TTYWindow();
            TTYWindow(wxWindow *parent, int n_lines);
            ~TTYWindow();

             void Add(wxString &line);
             void OnCloseWindow(wxCloseEvent& event);
             void Close();
             void OnSize( wxSizeEvent& event );
             void OnMove( wxMoveEvent& event );
             void OnPauseClick( wxCommandEvent& event );

      protected:
            void CreateLegendBitmap();
            TTYScroll   *m_pScroll;
            wxButton    *m_buttonPause;
            bool        bpause;
            wxBitmap    m_bm_legend;
};







//---------------------------------------------------------------------------------
//      Vector Stuff for Hit Test Algorithm
//---------------------------------------------------------------------------------

extern "C" double vGetLengthOfNormal(pVector2D a, pVector2D b, pVector2D n);
extern "C" double vDotProduct(pVector2D v0, pVector2D v1);
extern "C" pVector2D vAddVectors(pVector2D v0, pVector2D v1, pVector2D v);
extern "C" pVector2D vSubtractVectors(pVector2D v0, pVector2D v1, pVector2D v);
extern "C" double vVectorMagnitude(pVector2D v0);
extern "C" double vVectorSquared(pVector2D v0);



//---------------------------------------------------------------------------------
//      OpenCPN internal Sound support class
//---------------------------------------------------------------------------------

/// Sound data, as loaded from .wav file:
class OCPNSoundData
{
public:
    OCPNSoundData() : m_dataWithHeader(NULL) {}
    ~OCPNSoundData() {};

    // .wav header information:
    unsigned m_channels;       // num of channels (mono:1, stereo:2)
    unsigned m_samplingRate;
    unsigned m_bitsPerSample;  // if 8, then m_data contains unsigned 8bit
    // samples (wxUint8), if 16 then signed 16bit
    // (wxInt16)
    unsigned m_samples;        // length in samples:

    // wave data:
    size_t   m_dataBytes;
    wxUint8 *m_data;           // m_dataBytes bytes of data

    wxUint8 *m_dataWithHeader; // ditto, but prefixed with .wav header
};




class OCPN_Sound: public wxSound
{
public:
    OCPN_Sound();
    ~OCPN_Sound();

    bool IsOk() const;
    bool Create(const wxString& fileName, bool isResource = false);
    bool Play(unsigned flags = wxSOUND_ASYNC) const;
    bool IsPlaying() const;
    void Stop();

private:
    bool m_OK;

#ifdef OCPN_USE_PORTAUDIO
    bool LoadWAV(const wxUint8 *data, size_t length, bool copyData);
    void FreeMem(void);

    OCPNSoundData *m_osdata;
    PaStream *m_stream;
#endif

};


#endif
