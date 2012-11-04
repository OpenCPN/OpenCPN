/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *
 */


#ifndef __NAVUTIL__
#define __NAVUTIL__


#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>

#ifdef __WXMSW__
#include <wx/msw/regconf.h>
#include <wx/msw/iniconf.h>
#endif

#include "bbox.h"
#include "s52s57.h"
#include "chcanv.h"
#include "tinyxml.h"
#include "gpxdocument.h"
#include "chartdbs.h"


#define STYLE_UNDEFINED -1
#define COORD_UNDEFINED -999.0

extern bool LogMessageOnce(wxString &msg);
extern wxString toSDMM(int NEflag, double a, bool hi_precision = true);
extern void AlphaBlending ( ocpnDC& dc, int x, int y, int size_x, int size_y, float radius,
                                      wxColour color, unsigned char transparency );

extern double fromDMM(wxString sdms);
extern double parseLatLon(wxString latlon);

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//   Fwd Declarations
//----------------------------------------------------------------------------

class Route;
class NavObjectCollection;
class wxProgressDialog;
class ocpnDC;

//    class declarations

class Hyperlink { // toh, 2009.02.14
      public:
            wxString DescrText;
            wxString Link;
            wxString Type;
};

WX_DECLARE_LIST(Hyperlink, HyperlinkList);// establish class as list member

class RoutePoint
{
public:
      RoutePoint(double lat, double lon, const wxString& icon_ident, const wxString& name, const wxString &pGUID = GPX_EMPTY_STRING, bool bAddToList = true);
      RoutePoint( RoutePoint* orig );
      RoutePoint();
      ~RoutePoint(void);
      void Draw(ocpnDC& dc, wxPoint *rpn = NULL);
      void ReLoadIcon(void);

      wxString CreatePropString(void);
      void SetPropFromString(const wxString &prop_string);

      void SetPosition(double lat, double lon);
      void CalculateDCRect(wxDC& dc, wxRect *prect);

      bool IsSame(RoutePoint *pOtherRP);        // toh, 2009.02.11
      bool IsVisible() { return m_bIsVisible; }
      bool IsListed() { return m_bIsListed; }
      bool IsNameShown() { return m_bShowName; }
      void SetVisible(bool viz = true){ m_bIsVisible = viz; }
      void SetListed(bool viz = true){ m_bIsListed = viz; }
      void SetNameShown(bool viz = true) { m_bShowName = viz; }
      wxString GetName(void){ return m_MarkName; }

      void SetName(wxString name);
      void CalculateNameExtents(void);

      bool SendToGPS ( wxString& com_name, wxGauge *pProgress );


      double             m_lat;
      double             m_lon;
      double             m_seg_len;              // length in NMI to this point
                                                // undefined for starting point
      double            m_seg_vmg;
      wxDateTime        m_seg_etd;

      bool              m_bPtIsSelected;
      bool              m_bIsBeingEdited;

      bool              m_bIsInRoute;
      bool              m_bIsInTrack;

      bool              m_bIsolatedMark;        // This is an isolated mark

      bool              m_bKeepXRoute;          // This is a mark which is part of a route/track
                                                //  and is also an isolated mark, so should not be deleted with
                                                //  route

      bool              m_bIsVisible;           // true if should be drawn, false if invisible
      bool              m_bIsListed;
      bool              m_bIsActive;
      int               m_ConfigWPNum;
      wxString          m_MarkDescription;
      wxString          m_GUID;
      wxString          m_IconName;
      wxString          m_prop_string_format;         // Alpha character, like "A", giving version of property string

      wxFont            *m_pMarkFont;
      wxColour          m_FontColor;

      wxSize            m_NameExtents;

      wxBitmap          *m_pbmIcon;
      bool              m_bBlink;
      bool              m_bDynamicName;
      bool              m_bShowName;
      wxRect            CurrentRect_in_DC;
      int               m_NameLocationOffsetX;
      int               m_NameLocationOffsetY;
      wxDateTime        m_CreateTime;
      int               m_GPXTrkSegNo;
      bool              m_bIsInLayer;
      int               m_LayerID;

      HyperlinkList     *m_HyperlinkList;

private:
      wxString          m_MarkName;
};

WX_DECLARE_LIST(RoutePoint, RoutePointList);// establish class as list member

class vector2D  {
public:
    vector2D() { x = 0.0; y = 0.0; }
    vector2D( double a, double b ) { x = a; y = b; }
    void Set( RoutePoint* p ) { lat = p->m_lat; lon = p->m_lon; }
    friend bool operator==( vector2D &a, vector2D &b ) { return a.x == b.x && a.y == b.y; }
    friend bool operator!=( vector2D &a, vector2D &b ) { return a.x != b.x || a.y != b.y; }
    friend vector2D operator-( vector2D a, vector2D b ) { return vector2D( a.x - b.x, a.y - b.y ); }
    friend vector2D operator+( vector2D a, vector2D b ) { return vector2D( a.x + b.x, a.y + b.y ); }
    friend vector2D operator*( double t, vector2D a ) { return vector2D( a.x * t, a.y * t ); }

    union{ double x; double lon; };
    union{ double y; double lat; };
};

typedef vector2D* pVector2D;

//----------------------------------------------------------------------------
//    Route
//----------------------------------------------------------------------------
class Route : public wxObject
{
public:
      Route(void);
      ~Route(void);

      void AddPoint(RoutePoint *pNewPoint,
                    bool b_rename_in_sequence = true,
                    bool b_deferBoxCalc = false);

      void AddTentativePoint(const wxString& GUID);
      RoutePoint *GetPoint(int nPoint);
      RoutePoint *GetPoint ( const wxString &guid );
      int GetIndexOf(RoutePoint *prp);
      RoutePoint *InsertPointBefore(RoutePoint *pRP, double rlat, double rlon, bool bRenamePoints = false);
      void DrawPointWhich(ocpnDC& dc, int iPoint, wxPoint *rpn);
      void DrawSegment(ocpnDC& dc, wxPoint *rp1, wxPoint *rp2, ViewPort &VP, bool bdraw_arrow);
      virtual void Draw(ocpnDC& dc, ViewPort &pVP);
      RoutePoint *GetLastPoint();
      void DeletePoint(RoutePoint *rp, bool bRenamePoints = false);
      void RemovePoint(RoutePoint *rp, bool bRenamePoints = false);
      void DeSelectRoute();
      void CalculateBBox();
      void UpdateSegmentDistances(double planspeed = -1.0);
      void CalculateDCRect(wxDC& dc_route, wxRect *prect, ViewPort &VP);
      int GetnPoints(void){ return m_nPoints; }
      void SetnPoints(void){ m_nPoints = pRoutePointList->GetCount(); }
      void Reverse(bool bRenamePoints = false);
      void RebuildGUIDList(void);
      void RenameRoutePoints();
      void ReloadRoutePointIcons();
      wxString GetNewMarkSequenced(void);
      void AssembleRoute();
      bool IsEqualTo(Route *ptargetroute);
      void CloneRoute(Route *psourceroute, int start_nPoint, int end_nPoint, wxString suffix);
      void CloneTrack(Route *psourceroute, int start_nPoint, int end_nPoint, wxString suffix);
      void CloneAddedTrackPoint(RoutePoint *ptargetpoint, RoutePoint *psourcepoint);
      void CloneAddedRoutePoint(RoutePoint *ptargetpoint, RoutePoint *psourcepoint);
      void ClearHighlights(void);
      void RenderSegment(ocpnDC& dc, int xa, int ya, int xb, int yb, ViewPort &VP, bool bdraw_arrow, int hilite_width = 0);

      bool CrossesIDL(){ return m_bcrosses_idl; }
      void SetVisible(bool visible = true, bool includeWpts = true);
      void SetListed(bool visible = true);
      bool IsVisible() { return m_bVisible; }
      bool IsListed() { return m_bListed; }
      bool IsActive() { return m_bRtIsActive; }
      bool IsSelected() { return m_bRtIsSelected; }
      bool IsTrack(){ return m_bIsTrack; }

      bool SendToGPS(wxString& com_name, bool bsend_waypoints, wxGauge *pProgress);

      double GetRouteArrivalRadius(void){ return m_ArrivalRadius;}
      void SetRouteArrivalRadius(double radius){m_ArrivalRadius = radius;}

      int         m_ConfigRouteNum;
      bool        m_bRtIsSelected;
      bool        m_bRtIsActive;
      RoutePoint  *m_pRouteActivePoint;
      bool        m_bIsBeingCreated;
      bool        m_bIsBeingEdited;
      double      m_route_length;
      double      m_route_time;
      wxString    m_RouteNameString;
      wxString    m_RouteStartString;
      wxString    m_RouteEndString;
      bool        m_bIsTrack;             //TODO should use class type instead
      RoutePoint  *m_pLastAddedPoint;
      bool        m_bDeleteOnArrival;
      wxString    m_GUID;
      bool        m_bIsInLayer;
      int         m_LayerID;
      int         m_width;
      int         m_style;
      int         m_lastMousePointIndex;
      bool        m_NextLegGreatCircle;

      wxArrayString      RoutePointGUIDList;
      RoutePointList     *pRoutePointList;

      wxBoundingBox     RBBox;
      wxRect      active_pt_rect;
      wxString    m_Colour;

private:
      bool        CalculateCrossesIDL();
      int         m_nPoints;
      int         m_nm_sequence;
      bool        m_bVisible; // should this route be drawn?
      bool        m_bListed;
      double      m_ArrivalRadius;
      bool        m_bcrosses_idl;
};

WX_DECLARE_LIST(Route, RouteList);                    // establish class Route as list member


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

      void ExportGPX(wxWindow* parent);
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



#define            SELTYPE_UNKNOWN              0x0001
#define            SELTYPE_ROUTEPOINT           0x0002
#define            SELTYPE_ROUTESEGMENT         0x0004
#define            SELTYPE_TIDEPOINT            0x0008
#define            SELTYPE_CURRENTPOINT         0x0010
#define            SELTYPE_ROUTECREATE          0x0020
#define            SELTYPE_AISTARGET            0x0040
#define            SELTYPE_MARKPOINT            0x0080
#define            SELTYPE_TRACKSEGMENT         0x0100

//-----------------------------------------------------------------------------
//          Selectable Item
//-----------------------------------------------------------------------------

class SelectItem
{
public:
      SelectItem();
      ~SelectItem();

      int   GetUserData(void);
      void  SetUserData(int data);

      float m_slat;
      float m_slon;
      float m_slat2;
      float m_slon2;
      int   m_seltype;
      bool  m_bIsSelected;
      void  *m_pData1;
      void  *m_pData2;
      void  *m_pData3;
      int   m_Data4;
};



WX_DECLARE_LIST(SelectItem, SelectableItemList);// establish class as list member



class Select {
public:
    Select();
    ~Select();

    bool AddSelectableRoutePoint( float slat, float slon, RoutePoint *pRoutePointAdd );
    bool AddSelectableRouteSegment( float slat1, float slon1, float slat2, float slon2,
            RoutePoint *pRoutePointAdd1, RoutePoint *pRoutePointAdd2, Route *pRoute );

    bool AddSelectableTrackSegment( float slat1, float slon1, float slat2, float slon2,
            RoutePoint *pRoutePointAdd1, RoutePoint *pRoutePointAdd2, Route *pRoute );

    SelectItem *FindSelection( float slat, float slon, int fseltype );
    SelectableItemList FindSelectionList( float slat, float slon, int fseltype );

    bool DeleteAllSelectableRouteSegments( Route * );
    bool DeleteAllSelectableTrackSegments( Route * );
    bool DeleteAllSelectableRoutePoints( Route * );
    bool AddAllSelectableRouteSegments( Route *pr );
    bool AddAllSelectableTrackSegments( Route *pr );
    bool AddAllSelectableRoutePoints( Route *pr );
    bool UpdateSelectableRouteSegments( RoutePoint *prp );
    bool DeletePointSelectableTrackSegments( RoutePoint *pr );
    bool IsSegmentSelected( float a, float b, float c, float d, float slat, float slon );
    bool IsSelectableSegmentSelected( float slat, float slon, SelectItem *pFindSel );

    //    Generic Point Support
    //      e.g. Tides/Currents and AIS Targets
    SelectItem *AddSelectablePoint( float slat, float slon, void *data, int fseltype );
    bool DeleteAllPoints( void );
    bool DeleteSelectablePoint( void *data, int SeltypeToDelete );
    bool ModifySelectablePoint( float slat, float slon, void *data, int fseltype );

    //    Delete all selectable points in list by type
    bool DeleteAllSelectableTypePoints( int SeltypeToDelete );

    //  Accessors

    SelectableItemList *GetSelectList()
    {
        return pSelectList;
    }

private:
    void CalcSelectRadius();

    SelectableItemList *pSelectList;
    int pixelRadius;
    float selectRadius;
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









//---------------------------------------------------------------------------------
//          Private Font Manager
//---------------------------------------------------------------------------------
class MyFontDesc
{
public:

      MyFontDesc(wxString DialogString, wxString ConfigString, wxFont *pFont, wxColour color);
      ~MyFontDesc();

      wxString    m_dialogstring;
      wxString    m_configstring;
      wxString    m_nativeInfo;
      wxFont      *m_font;
      wxColour    m_color;
};


WX_DECLARE_LIST(MyFontDesc, FontList);

class FontMgr
{
public:
      FontMgr();
      ~FontMgr();

      wxFont *GetFont(const wxString &TextElement, int default_size = 0);
      wxColour GetFontColor ( const wxString &TextElement );

      int GetNumFonts(void);
      wxString *GetConfigString(int i);
      wxString *GetDialogString(int i);
      wxString *GetNativeDesc(int i);
      wxString GetFullConfigDesc ( int i );
      static wxString GetFontConfigKey( const wxString &description );

      void LoadFontNative(wxString *pConfigString, wxString *pNativeDesc);
      bool SetFont(wxString &TextElement, wxFont *pFont, wxColour color);

private:
      wxString GetSimpleNativeFont(int size);

      FontList          *m_fontlist;
      wxFont            *pDefFont;

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



#endif
