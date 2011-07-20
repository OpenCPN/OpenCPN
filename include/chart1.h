/******************************************************************************
 *
 * Project:  OpenCP
 * Purpose:  OpenCP Main wxWidgets Program
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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
 *
 */

#ifndef __CHART1_H__
#define __CHART1_H__

#include "wx/print.h"
#include "wx/datetime.h"
#include <wx/cmdline.h>

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

#include "ocpn_types.h"

#include "nmea0183.h"

WX_DEFINE_ARRAY_INT(int, ArrayOfInts);
WX_DECLARE_STRING_HASH_MAP( wxColour, ColourHash );

#ifdef USE_S57
#include "cpl_error.h"

//    Global Static error reporting function
extern "C" void MyCPLErrorHandler( CPLErr eErrClass, int nError,
                             const char * pszErrorMsg );
#endif

wxArrayString *EnumerateSerialPorts(void);
wxColour GetGlobalColor(wxString colorName);

int GetApplicationMemoryUse(void);

// The point for anchor watch should really be a class...
double AnchorDistFix( double const d, double const AnchorPointMinDist, double const AnchorPointMaxDist);   //  pjotrc 2010.02.22

//    Fwd definitions
class OCPN_NMEAEvent;
class ChartCanvas;
class ocpnFloatingToolbarDialog;

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

#define MAX_COG_AVERAGE_SECONDS        60
#define MAX_COGSOG_FILTER_SECONDS      60
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
WX_DECLARE_OBJARRAY(wxRect, ArrayOfRect);


class MyApp: public wxApp
{
  public:
    bool OnInit();
    int OnExit();
    void OnInitCmdLine(wxCmdLineParser& parser);
    bool OnCmdLineParsed(wxCmdLineParser& parser);
    void OnActivateApp(wxActivateEvent& event);

    void TrackOff(void);

    DECLARE_EVENT_TABLE()

};

class MyFrame: public wxFrame
{
  public:
    MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size, long style);

    ~MyFrame();

    int GetApplicationMemoryUse(void);

    void OnEraseBackground(wxEraseEvent& event);
    void OnActivate(wxActivateEvent& event);
    void OnMaximize(wxMaximizeEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMove(wxMoveEvent& event);
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
    void SelectdbChart(int dbindex);
    void SelectQuiltRefChart(int selected_index);
    void SelectQuiltRefdbChart(int db_index);

    void JumpToPosition(double lat, double lon, double scale);

    void ProcessCanvasResize(void);

    void ApplyGlobalSettings(bool bFlyingUpdate, bool bnewtoolbar);
    void SetChartThumbnail(int index);
    int  DoOptionsDialog();
    void DoPrint(void);
    void StopSockets(void);
    void ResumeSockets(void);
    void DoExportGPX(void);
    void DoImportGPX(void);
    void TogglebFollow(void);
    void ToggleFullScreen();
    void SetbFollow(void);
    void ClearbFollow(void);
    void ToggleChartOutlines(void);
    void ToggleENCText(void);
    void TrackOn(void);
    void TrackOff(bool do_add_point = false);
    void TrackMidnightRestart(void);
    void ToggleColorScheme();
    int GetnChartStack(void);
    void SetToolbarItemState ( int tool_id, bool state );
    void ToggleQuiltMode(void);
    void ToggleCourseUp(void);
    void SetQuiltMode(bool bquilt);
    bool GetQuiltMode(void);
    void UpdateControlBar(void);
    void RemoveChartFromQuilt(int dbIndex);

    void SubmergeToolbar(void);
    void SubmergeToolbarIfOverlap(int x, int y, int margin = 0);
    void SurfaceToolbar(void);

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

    void OnFrameTCTimer(wxTimerEvent& event);
    void OnFrameCOGTimer(wxTimerEvent& event);
    void SetupQuiltMode(void);

    void ChartsRefresh(int dbi_hint);
    string_to_pbitmap_hash *GetBitmapHash(){ return m_phash; }

    ArrayOfRect GetCanvasReserveRects();

    wxStatusBar         *m_pStatusBar;
    int                 nRoute_State;
    int                 nBlinkerTick;
    bool                m_bTimeIsSet;

    wxTimer             FrameTCTimer;
    wxTimer             FrameTimer1;
    wxTimer             FrameCOGTimer;
    wxTimer             MemFootTimer;


    string_to_pbitmap_hash *m_phash;

    //      PlugIn support
    int GetNextToolbarToolId(){return m_next_available_plugin_tool_id;}
    void RequestNewToolbar();

    void ActivateMOB(void);
    void UpdateGPSCompassStatusBox(bool b_force_new = false);
    bool UpdateChartDatabaseInplace(ArrayOfCDI &DirArray,
                                    bool b_force, bool b_prog,
                                    wxString &ChartListFileName);

  private:
    void DoSetSize(void);
    void DoCOGSet(void);

        //      Toolbar support
    ocpnToolBarSimple *CreateAToolbar();
    void DestroyMyToolbar();
    void UpdateToolbar(ColorScheme cs);
    void PrepareToolbarBitmaps(void);
    void BuildToolBitmap(wxImage *pimg, unsigned char back_color, wxString &index,
                         string_to_pbitmap_hash &hash, bool grey = false);
    void BuildGreyScaleTool(wxImage *pimg, unsigned char grey_val, wxString &index,
                         string_to_pbitmap_hash &hash);

    void DeleteToolbarBitmaps();
    void EnableToolbar(bool newstate);
    void UpdateToolbarDynamics(void);

    bool CheckAndAddPlugInTool(ocpnToolBarSimple *tb);
    bool AddDefaultPositionPlugInTools(ocpnToolBarSimple *tb);
    void FilterCogSog(void);
    void SetChartUpdatePeriod(ViewPort &vp);

    void ApplyGlobalColorSchemetoStatusBar(void);
    void PostProcessNNEA(bool brx_rmc, wxString &sfixtime);

    string_to_pchar_hash tool_xpm_hash;         // hash map of [static] toolbar xpm bitmaps


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

    wxString         m_AIS_bmp_hash_index_last;

    bool             m_toolbar_scale_tools_shown;

    //      Plugin Support
    int                 m_next_available_plugin_tool_id;

    double              m_COGFilterLast;
    double              COGFilterTable[MAX_COGSOG_FILTER_SECONDS];
    double              SOGFilterTable[MAX_COGSOG_FILTER_SECONDS];

    bool                m_bpersistent_quilt;
    int                 m_ChartUpdatePeriod;
    bool                m_last_bGPSValid;

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

class ToolTipWin;
class ocpnToolBarTool;

// ----------------------------------------------------------------------------
// ocpnToolBarSimple is a generic toolbar implementation in pure wxWidgets
//    Adapted from wxToolBarSimple( deprecated )
// ----------------------------------------------------------------------------

class ocpnToolBarSimple : public wxControl
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

            wxToolBarToolBase *AddTool(int toolid,
                        const wxString& label,
                        const wxBitmap& bitmap,
                        const wxBitmap& bmpDisabled,
                        wxItemKind kind = wxITEM_NORMAL,
                        const wxString& shortHelp = wxEmptyString,
                        const wxString& longHelp = wxEmptyString,
                        wxObject *data = NULL);

            wxToolBarToolBase *AddTool(int toolid,
                                       const wxString& label,
                                       const wxBitmap& bitmap,
                                       const wxString& shortHelp = wxEmptyString,
                                       wxItemKind kind = wxITEM_NORMAL)
            {
                  return AddTool(toolid, label, bitmap, wxNullBitmap, kind, shortHelp);
            }


            wxToolBarToolBase *InsertTool(size_t pos,
                        int id,
                        const wxString& label,
                        const wxBitmap& bitmap,
                        const wxBitmap& bmpDisabled,
                        wxItemKind kind,
                        const wxString& shortHelp,
                        const wxString& longHelp,
                        wxObject *clientData);

            wxToolBarToolBase *InsertTool(size_t pos, wxToolBarToolBase *tool);

    // Only allow toggle if returns true. Call when left button up.
            virtual bool OnLeftClick(int toolid, bool toggleDown);

    // Call when right button down.
            virtual void OnRightClick(int toolid, long x, long y);

    // Called when the mouse cursor enters a tool bitmap.
    // Argument is wxID_ANY if mouse is exiting the toolbar.
            virtual void OnMouseEnter(int toolid);

           wxSize GetMargins() const { return GetToolMargins(); }
           size_t GetToolsCount() const { return m_tools.GetCount(); }

    // add an arbitrary control to the toolbar (notice that
    // the control will be deleted by the toolbar and that it will also adjust
    // its position/size)
            //
    // NB: the control should have toolbar as its parent
            virtual wxToolBarToolBase *AddControl(wxControl *control);
            virtual wxToolBarToolBase *InsertControl(size_t pos, wxControl *control);

    // get the control with the given id or return NULL
            virtual wxControl *FindControl( int toolid );

    // add a separator to the toolbar
            virtual wxToolBarToolBase *AddSeparator();
            virtual wxToolBarToolBase *InsertSeparator(size_t pos);

    // remove the tool from the toolbar: the caller is responsible for actually
    // deleting the pointer
            virtual wxToolBarToolBase *RemoveTool(int toolid);

    // delete tool either by index or by position
            virtual bool DeleteToolByPos(size_t pos);
            virtual bool DeleteTool(int toolid);

    // delete all tools
            virtual void ClearTools();

    // must be called after all buttons have been created to finish toolbar
    // initialisation
            virtual bool Realize();

    // tools state
    // -----------

            virtual void EnableTool(int toolid, bool enable);
            virtual void ToggleTool(int toolid, bool toggle);

    // Set this to be togglable (or not)
            virtual void SetToggle(int toolid, bool toggle);

    // set/get tools client data (not for controls)
            virtual wxObject *GetToolClientData(int toolid) const;
            virtual void SetToolClientData(int toolid, wxObject *clientData);

    // returns tool pos, or wxNOT_FOUND if tool isn't found
            virtual int GetToolPos(int id) const;

    // return true if the tool is toggled
            virtual bool GetToolState(int toolid) const;

            virtual bool GetToolEnabled(int toolid) const;

            virtual void SetToolShortHelp(int toolid, const wxString& helpString);
            virtual wxString GetToolShortHelp(int toolid) const;
            virtual void SetToolLongHelp(int toolid, const wxString& helpString);
            virtual wxString GetToolLongHelp(int toolid) const;

    // margins/packing/separation
    // --------------------------

            virtual void SetMargins(int x, int y);
            void SetMargins(const wxSize& size) { SetMargins((int) size.x, (int) size.y); }
            virtual void SetToolPacking(int packing){ m_toolPacking = packing; }
            virtual void SetToolSeparation(int separation){ m_toolSeparation = separation; }

            virtual wxSize GetToolMargins() const { return wxSize(m_xMargin, m_yMargin); }
            virtual int GetToolPacking() const { return m_toolPacking; }
            virtual int GetToolSeparation() const { return m_toolSeparation; }

    // toolbar geometry
    // ----------------

    // set the number of toolbar rows
            virtual void SetRows(int nRows);

    // the toolbar can wrap - limit the number of columns or rows it may take
            void SetMaxRowsCols(int rows, int cols){ m_maxRows = rows; m_maxCols = cols; }
            int GetMaxRows() const { return m_maxRows; }
            int GetMaxCols() const { return m_maxCols; }

    // get/set the size of the bitmaps used by the toolbar: should be called
    // before adding any tools to the toolbar
            virtual void SetToolBitmapSize(const wxSize& size) { m_defaultWidth = size.x; m_defaultHeight = size.y; }
            virtual wxSize GetToolBitmapSize() const  { return wxSize(m_defaultWidth, m_defaultHeight); }

    // the button size in some implementations is bigger than the bitmap size:
    // get the total button size (by default the same as bitmap size)
            virtual wxSize GetToolSize() const { return GetToolBitmapSize(); }

    // returns a (non separator) tool containing the point (x, y) or NULL if
    // there is no tool at this point (corrdinates are client)
            wxToolBarToolBase *FindToolForPosition(wxCoord x, wxCoord y);

    // find the tool by id
            wxToolBarToolBase *FindById(int toolid) const;

    // return true if this is a vertical toolbar, otherwise false
            bool IsVertical() const { return HasFlag(wxTB_LEFT | wxTB_RIGHT); }

    // the list of all our tools
            wxToolBarToolsList m_tools;

    // the offset of the first tool
            int m_xMargin;
            int m_yMargin;

    // the maximum number of toolbar rows/columns
            int m_maxRows;
            int m_maxCols;

    // the tool packing and separation
            int m_toolPacking,
            m_toolSeparation;

    // the size of the toolbar bitmaps
            wxCoord m_defaultWidth, m_defaultHeight;

            void        HideTooltip();
            void        KillTooltip();
            void        ShowTooltip(){ m_btooltip_show = true; }

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
            virtual wxToolBarToolBase *CreateTool(wxControl *control, const wxString& label);
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
            bool                    m_btooltip_show;

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


extern int OCPNMessageBox(const wxString& message, const wxString& caption = _T("Message"), int style = wxOK,wxWindow *parent = NULL, int x = -1, int y = -1);

class OCPNMessageDialog
{
      public:
            OCPNMessageDialog(wxWindow* parent, const wxString& message, const wxString& caption = _T("Message box"), long style = wxOK | wxCANCEL, const wxPoint& pos = wxDefaultPosition);

            ~OCPNMessageDialog();

            int ShowModal();
      private:
            wxMessageDialog *m_pdialog;
};


#endif
