/******************************************************************************
 * $Id: chart1.h,v 1.43 2010/06/11 16:31:22 bdbcat Exp $
 *
 * Project:  OpenCP
 * Purpose:  OpenCP Main wxWidgets Program
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
 * $Log: chart1.h,v $
 * Revision 1.43  2010/06/11 16:31:22  bdbcat
 * 611a
 *
 * Revision 1.42  2010/06/04 22:36:10  bdbcat
 * 604
 *
 * Revision 1.41  2010/05/27 19:00:18  bdbcat
 * 527a
 *
 * Revision 1.40  2010/05/19 01:05:26  bdbcat
 * Build 518
 *
 *
 * Revision 1.39  2010/05/15 03:55:04  bdbcat
 * Build 514
 *
 * Revision 1.38  2010/05/04 01:34:04  bdbcat
 * Build 503
 *
 * Revision 1.37  2010/05/02 03:04:05  bdbcat
 * Build 501
 *
 * Revision 1.36  2010/04/27 01:44:36  bdbcat
 * Build 426
 *
 * Revision 1.35  2010/04/15 15:52:30  bdbcat
 * Build 415.
 *
 * Revision 1.34  2010/03/29 02:59:02  bdbcat
 * 2.1.0 Beta Initial
 *
 *
 */

#ifndef __CHART1_H__
#define __CHART1_H__

#include "wx/print.h"
#include "wx/datetime.h"

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

#include "ocpn_types.h"

#include "cpl_error.h"

#include "nmea0183.h"

WX_DEFINE_ARRAY_INT(int, ArrayOfInts);

//    Global Static utility functions
extern "C" void MyCPLErrorHandler( CPLErr eErrClass, int nError,
                             const char * pszErrorMsg );

wxArrayString *EnumerateSerialPorts(void);
wxColour GetGlobalColor(wxString colorName);

int GetApplicationMemoryUse(void);

// The point for anchor watch should really be a class...
double AnchorDistFix( double const d, double const AnchorPointMinDist, double const AnchorPointMaxDist);   //  pjotrc 2010.02.22

//    Fwd definitions
class OCPN_NMEAEvent;
class ChartCanvas;

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#define OCPN_USE_PNGICONS
#define USE_PNG_TIDESML
#define USE_PNG_OWNSHIP
#define USE_PNG_CURSOR




#define TIMER_GFRAME_1 999

#define ID_QUIT         101
#define ID_CM93ZOOMG    102

//    ToolBar Constants
const int ID_TOOLBAR = 500;

enum
{
      ID_ZOOMIN = 1550,
      ID_ZOOMOUT,
      ID_STKUP,
      ID_STKDN,
      ID_ROUTE,
      ID_FOLLOW,
      ID_SETTINGS,
      ID_AIS,           // pjotrc 2010.02.09
      ID_TEXT,
      ID_CURRENT,
      ID_TIDE,
      ID_HELP,
      ID_TBEXIT,
      ID_TBSTAT,
      ID_PRINT,
      ID_COLSCHEME,
      ID_ROUTEMANAGER,
      ID_TRACK,
      ID_TBSTATBOX,
      ID_MOB,
      ID_PLUGIN_BASE

};


static const long TOOLBAR_STYLE = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT ;

enum
{
    IDM_TOOLBAR_TOGGLETOOLBARSIZE = 200,
    IDM_TOOLBAR_TOGGLETOOLBARORIENT,
    IDM_TOOLBAR_TOGGLETOOLBARROWS,
    IDM_TOOLBAR_ENABLEPRINT,
    IDM_TOOLBAR_DELETEPRINT,
    IDM_TOOLBAR_INSERTPRINT,
    IDM_TOOLBAR_TOGGLEHELP,
    IDM_TOOLBAR_TOGGLE_TOOLBAR,
    IDM_TOOLBAR_TOGGLE_ANOTHER_TOOLBAR,
    IDM_TOOLBAR_CHANGE_TOOLTIP,
    IDM_TOOLBAR_SHOW_TEXT,
    IDM_TOOLBAR_SHOW_ICONS,
    IDM_TOOLBAR_SHOW_BOTH,

    ID_COMBO = 1000
};



#define N_STATUS_BAR_FIELDS_MAX     20

#define STAT_FIELD_TICK             0
#define STAT_FIELD_SOGCOG           1
#define STAT_FIELD_CURSOR_LL        2
#define STAT_FIELD_CURSOR_BRGRNG    3
#define STAT_FIELD_SCALE            4

//      Define a constant GPS signal watchdog timeout value
#define GPS_TIMEOUT_SECONDS  6

//    Define a timer value for Tide/Current updates
//    Note that the underlying data algorithms produce fresh data only every 15 minutes
//    So maybe 5 minute updates should provide sufficient oversampling
#define TIMER_TC_VALUE_SECONDS      300

#define MAX_COG_AVERAGE_SECONDS      60
//----------------------------------------------------------------------------
// fwd class declarations
//----------------------------------------------------------------------------
class ChartBase;
class wxSocketEvent;
class ocpnToolBarSimple;

//----------------------------------------------------------------------------
//   Classes
//----------------------------------------------------------------------------
WX_DECLARE_STRING_HASH_MAP( char*, string_to_pchar_hash);
WX_DECLARE_STRING_HASH_MAP(wxBitmap*, string_to_pbitmap_hash);
WX_DECLARE_STRING_HASH_MAP(wxString*, string_to_string_hash);



//    A small class used in an array to describe chart directories
class ChartDirInfo
{
      public:
      wxString    fullpath;
      wxString    magic_number;
};

WX_DECLARE_OBJARRAY(ChartDirInfo, ArrayOfCDI);


class MyApp: public wxApp
{
  public:
    bool OnInit();
    int OnExit();

    void TestSockets(void);
    void OnSocketEvent(wxSocketEvent& event);
    void TrackOff(void);

};

class MyFrame: public wxFrame
{
  public:
    MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size, long style);

    ~MyFrame();

    void OnEraseBackground(wxEraseEvent& event);
    void OnActivate(wxActivateEvent& event);
    void OnMaximize(wxMaximizeEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnFrameTimer1(wxTimerEvent& event);
    bool DoChartUpdate(void);
    void OnEvtNMEA(wxCommandEvent& event);
    void OnEvtTHREADMSG(wxCommandEvent& event);
    void OnEvtOCPN_NMEA(OCPN_NMEAEvent & event);
    void OnMemFootTimer(wxTimerEvent& event);

    void UpdateAllFonts(void);
    void PositionConsole(void);
    void OnToolLeftClick(wxCommandEvent& event);
    void ClearRouteTool();
    void DoStackUp(void);
    void DoStackDown(void);

    void MouseEvent(wxMouseEvent& event);
    void SelectChartFromStack(int index,  bool bDir = false,  ChartTypeEnum New_Type = CHART_TYPE_DONTCARE, ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
    void SelectQuiltRefChart(int selected_index);

    void ApplyGlobalSettings(bool bFlyingUpdate, bool bnewtoolbar);
    void SetChartThumbnail(int index);
    int  DoOptionsDialog();
    void DoPrint(void);
    void StopSockets(void);
    void ResumeSockets(void);
    void DoExportGPX(void);
    void DoImportGPX(void);
    void TogglebFollow(void);
    void SetbFollow(void);
    void ClearbFollow(void);
    void ToggleChartOutlines(void);
    void ToggleENCText(void);
    void TrackOn(void);
    void TrackOff(void);
    void ToggleColorScheme();
    int GetnChartStack(void);
    void SetToolbarItemState ( int tool_id, bool state );
    void ToggleQuiltMode(void);
    void ToggleCourseUp(void);
    void SetQuiltMode(bool bquilt);
    bool GetQuiltMode(void);
    void UpdateControlBar(void);
    void RemoveChartFromQuilt(int dbIndex);

    void HandlePianoClick(int selected_index, int selected_dbIndex);
    void HandlePianoRClick(int x, int y,int selected_index, int selected_dbIndex);
    void HandlePianoRollover(int selected_index, int selected_dbIndex);
    void HandlePianoRolloverIcon(int selected_index, int selected_dbIndex);

    void PianoPopupMenu ( int x, int y, int selected_index, int selected_dbIndex );
    void OnPianoMenuDisableChart(wxCommandEvent& event);
    void OnPianoMenuEnableChart(wxCommandEvent& event);

    double GetBestVPScale(ChartBase *pchart);

    ChartCanvas *GetCanvasWindow(){ return m_pchart_canvas; }
    void SetCanvasWindow(ChartCanvas *pcanv){ m_pchart_canvas = pcanv; }

    ColorScheme GetColorScheme();
    void SetAndApplyColorScheme(ColorScheme cs);

    bool GetMemoryStatus(int& mem_total, int& mem_used);

    void OnFrameTCTimer(wxTimerEvent& event);
    void OnFrameCOGTimer(wxTimerEvent& event);
    void SetupQuiltMode(void);

    wxStatusBar         *m_pStatusBar;
    int                 nRoute_State;
    int                 nBlinkerTick;
    bool                m_bTimeIsSet;

    wxTimer             FrameTCTimer;
    wxTimer             FrameTimer1;
    wxTimer             FrameCOGTimer;
    wxTimer             MemFootTimer;

    wxTextCtrl          *m_textWindow;

    int                 m_tool_dummy_size_x, m_tool_dummy_size_y;
    int                 m_statTool_pos;
    string_to_pbitmap_hash *m_phash;

    //      PlugIn support
    int GetNextToolbarToolId(){return m_next_available_plugin_tool_id;}
    void RequestNewToolbar();

  private:
    void DoSetSize(void);
    void DoCOGSet(void);

        //      Toolbar support
    ocpnToolBarSimple *CreateAToolbar();
    void DestroyMyToolbar();
    void UpdateToolbar(ColorScheme cs);
    void ReSizeToolbar(void);
    void PrepareToolbarBitmaps(void);
    void BuildToolBitmap(wxImage *pimg, unsigned char back_color, wxString &index,
                         string_to_pbitmap_hash &hash);
    void DeleteToolbarBitmaps();
    void EnableToolbar(bool newstate);
    void UpdateToolbarStatusWindow(ChartBase *pchart, bool bSendSize = true);
    void UpdateToolbarDynamics(void);
    void UpdateToolbarStatusBox(bool bupdate_toolbar = true);

    bool CheckAndAddPlugInTool(ocpnToolBarSimple *tb);




    void ApplyGlobalColorSchemetoStatusBar(void);
    void PostProcessNNEA(bool brx_rmc, wxString &sfixtime);

    int  toolbar_width_without_static;

    string_to_pchar_hash tool_xpm_hash;         // hash map of [static] toolbar xpm bitmaps

    int                 tool_dummy_size_x_last;

    string_to_pbitmap_hash tool_bitmap_hash_day;
    string_to_pbitmap_hash tool_bitmap_hash_dusk;
    string_to_pbitmap_hash tool_bitmap_hash_night;

    int                 m_StatusBarFieldCount;

    ChartCanvas         *m_pchart_canvas;

    NMEA0183        m_NMEA0183;                 // Used to parse messages from NMEA threads

    wxDateTime       m_MMEAeventTime;
    unsigned long    m_ulLastNEMATicktime;

    wxMutex          m_mutexNMEAEvent;         // Mutex to handle static data from NMEA threads

    wxString         m_last_reported_chart_name;
    wxString         m_last_reported_chart_pubdate;

    ocpnToolBarSimple  *m_toolBar;

    double           COGTable[MAX_COG_AVERAGE_SECONDS];

    wxString         m_last_gps_bmp_hash_index;
    wxString         m_AIS_bmp_hash_index_last;

    double           m_rose_angle;
    bool             m_bneedtoolbar;

    wxToolBarToolBase *m_pStatBoxTool;
    wxStaticBitmap   *m_pStatBoxToolStaticBmp;
    wxBitmap         m_StatBmp;

    wxToolBarToolBase *m_pStatDummyTool;
    wxStaticBitmap    *m_ptool_ct_dummyStaticBmp;
//    wxBitmapButton      *m_ptool_ct_dummy_bb;


    //      Plugin Support
    int                 m_next_available_plugin_tool_id;


    DECLARE_EVENT_TABLE()
};


//--------------------------------------------------------------------
//          Private Memory Management
//--------------------------------------------------------------------

//    Private malloc replacement
void *x_malloc(size_t t);

//--------------------------------------------------------------------
//          Printing Support
//--------------------------------------------------------------------

class MyPrintout: public wxPrintout
{
 public:
  MyPrintout(const wxChar *title = _T("My printout")):wxPrintout(title) {}
  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);

  void DrawPageOne(wxDC *dc);
};


//      A global definition for window, timer and other ID's as needed.
enum {
    ID_NMEA_WINDOW      = wxID_HIGHEST,
    ID_AIS_WINDOW,
    FRAME_TIMER_1,
    FRAME_TIMER_2,
    TIMER_AIS1,
    TIMER_AISAUDIO,
    AIS_SOCKET_ID,
    WIFI_SOCKET_ID,
    TIMER_WIFI1,
    FRAME_TIMER_DOG,
    FRAME_TC_TIMER,
    FRAME_COG_TIMER,
    MEMORY_FOOTPRINT_TIMER,
    ID_NMEA_THREADMSG

};

//-----------------------------------------------------------------------
//          Dummy Text Control for global key events
//-----------------------------------------------------------------------
class DummyTextCtrl: public wxTextCtrl
{
public:
      DummyTextCtrl(wxWindow *parent, wxWindowID id);
      void OnChar(wxKeyEvent &event);
      void OnMouseEvent(wxMouseEvent& event);

      wxTimer     m_MouseWheelTimer;
      int         m_mouse_wheel_oneshot;
      int         m_last_wheel_dir;

      DECLARE_EVENT_TABLE()
};



#include "wx/tbarbase.h"

#define TOOLTIPON_TIMER       10000

class WXDLLEXPORT wxMemoryDC;
class ToolTipWin;
class ocpnToolBarTool;

// ----------------------------------------------------------------------------
// ocpnToolBarSimple is a generic toolbar implementation in pure wxWidgets
//    Adapted from wxToolBarSimple( deprecated )
// ----------------------------------------------------------------------------

class ocpnToolBarSimple : public wxToolBarBase
{
      public:
    // ctors and dtor
            ocpnToolBarSimple() { Init(); }

            ocpnToolBarSimple(wxWindow *parent,
                            wxWindowID winid,
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            long style = wxNO_BORDER | wxTB_HORIZONTAL,
                            const wxString& name = wxToolBarNameStr)
            {
                  Init();

                  Create(parent, winid, pos, size, style, name);
            }

            bool Create(wxWindow *parent,
                        wxWindowID winid,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = wxNO_BORDER | wxTB_HORIZONTAL,
                        const wxString& name = wxToolBarNameStr);

            virtual ~ocpnToolBarSimple();

    // override/implement base class virtuals
            virtual wxToolBarToolBase *FindToolForPosition(wxCoord x, wxCoord y) const;

            virtual bool Realize();

            virtual void SetRows(int nRows);
            virtual void SetToggledBackgroundColour(wxColour c){ m_toggle_bg_color = c;};
            virtual void SetColorScheme(ColorScheme cs);


    // implementation from now on
    // --------------------------

    // SCROLLING: this has to be copied from wxScrolledWindow since
    // wxToolBarBase inherits from wxControl. This could have been put into
    // wxToolBarSimple, but we might want any derived toolbar class to be
    // scrollable.

    // Number of pixels per user unit (0 or -1 for no scrollbar)
    // Length of virtual canvas in user units
            virtual void SetScrollbars(int horizontal, int vertical,
                                       int x_length, int y_length,
                                       int x_pos = 0, int y_pos = 0);

    // Physically scroll the window
            virtual void Scroll(int x_pos, int y_pos);
            virtual void GetScrollPixelsPerUnit(int *x_unit, int *y_unit) const;
            virtual void EnableScrolling(bool x_scrolling, bool y_scrolling);
            virtual void AdjustScrollbars();

    // Prepare the DC by translating it according to the current scroll position
            virtual void PrepareDC(wxDC& dc);

            int GetScrollPageSize(int orient) const ;
            void SetScrollPageSize(int orient, int pageSize);

    // Get the view start
            virtual void ViewStart(int *x, int *y) const;

    // Actual size in pixels when scrolling is taken into account
            virtual void GetVirtualSize(int *x, int *y) const;

            int CalcScrollInc(wxScrollEvent& event);

    // event handlers
            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);
            void OnMouseEvent(wxMouseEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnScroll(wxScrollEvent& event);
            void OnToolTipTimerEvent(wxTimerEvent& event);


      protected:
    // common part of all ctors
            void Init();

    // implement base class pure virtuals
            virtual wxToolBarToolBase *DoAddTool
                        (
                         int toolid,
                         const wxString& label,
                         const wxBitmap& bitmap,
                         const wxBitmap& bmpDisabled,
                         wxItemKind kind,
                         const wxString& shortHelp = wxEmptyString,
                         const wxString& longHelp = wxEmptyString,
                         wxObject *clientData = NULL,
                         wxCoord xPos = wxDefaultCoord,
                         wxCoord yPos = wxDefaultCoord
                        );

            virtual bool DoInsertTool(size_t pos, wxToolBarToolBase *tool);
            virtual bool DoDeleteTool(size_t pos, wxToolBarToolBase *tool);

            virtual void DoEnableTool(wxToolBarToolBase *tool, bool enable);
            virtual void DoToggleTool(wxToolBarToolBase *tool, bool toggle);
            virtual void DoSetToggle(wxToolBarToolBase *tool, bool toggle);

            virtual wxToolBarToolBase *CreateTool(int winid,
                        const wxString& label,
                        const wxBitmap& bmpNormal,
                        const wxBitmap& bmpDisabled,
                        wxItemKind kind,
                        wxObject *clientData,
                        const wxString& shortHelp,
                        const wxString& longHelp);
            virtual wxToolBarToolBase *CreateTool(wxControl *control);

    // helpers
            void DrawTool(wxToolBarToolBase *tool);
            virtual void DrawTool(wxDC& dc, wxToolBarToolBase *tool);
            virtual void SpringUpButton(int index);

            int  m_currentRowsOrColumns;

            int  m_pressedTool, m_currentTool;

            wxCoord m_lastX, m_lastY;
            wxCoord m_maxWidth, m_maxHeight;
            wxCoord m_xPos, m_yPos;

            wxColour                m_toggle_bg_color;
            wxColour                m_toolOutlineColour;
            ToolTipWin              *m_pToolTipWin;
            ocpnToolBarTool         *m_last_ro_tool;

            ColorScheme             m_currentColorScheme;

            wxTimer                 m_tooltip_timer;
            int                     m_one_shot;

    // scrolling data
            int                   m_xScrollPixelsPerLine;
            int                   m_yScrollPixelsPerLine;
            bool                  m_xScrollingEnabled;
            bool                  m_yScrollingEnabled;
            int                   m_xScrollPosition;
            int                   m_yScrollPosition;
            int                   m_xScrollLines;
            int                   m_yScrollLines;
            int                   m_xScrollLinesPerPage;
            int                   m_yScrollLinesPerPage;

      private:
            DECLARE_EVENT_TABLE()
                        DECLARE_DYNAMIC_CLASS_NO_COPY(ocpnToolBarSimple)
};

//----------------------------------------------------------------------------
// Toolbar Tooltip Popup Window
//----------------------------------------------------------------------------
class ToolTipWin: public wxWindow
{
      public:
            ToolTipWin(wxWindow *parent);
            ~ToolTipWin();

            void OnPaint(wxPaintEvent& event);

            void SetColorScheme(ColorScheme cs);
            void SetString(wxString &s){ m_string = s; }
            void SetPosition(wxPoint pt){ m_position = pt; }
            void SetBitmap(void);


      private:

            wxString          m_string;
            wxSize            m_size;
            wxPoint           m_position;
            wxBitmap          *m_pbm;
            wxColour          m_back_color;
            wxColour          m_text_color;



            DECLARE_EVENT_TABLE()
};




#endif
