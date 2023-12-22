/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
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

#ifndef _OFRAME_H
#define _OFRAME_H

#include <wx/print.h>
#include <wx/power.h>

#include <memory>
#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "ocpn_types.h"
#include "ocpn_print.h"
#include "color_handler.h"
#include "gui_lib.h"
#include "s52s57.h"
#include "SencManager.h"
#include "comm_appmsg_bus.h"
#include "bbox.h"
#include "load_errors_dlg.h"

wxColour GetGlobalColor(wxString colorName);
wxColour GetDialogColor(DialogColor color);

int GetApplicationMemoryUse(void);

// Helper to create menu label + hotkey string when registering menus
wxString _menuText(wxString name, wxString shortcut);

// The point for anchor watch should really be a class...
double AnchorDistFix(double const d, double const AnchorPointMinDist,
                     double const AnchorPointMaxDist);  //  pjotrc 2010.02.22

bool TestGLCanvas(wxString prog_dir);
bool ReloadLocale();
void ApplyLocale(void);

void LoadS57();


//    Fwd definitions
class ChartCanvas;
class ocpnFloatingToolbarDialog;
class OCPN_MsgEvent;
class options;
class Track;
//class OCPN_ThreadMessageEvent;
class wxHtmlWindow;
class ArrayOfCDI;

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define TIMER_GFRAME_1 999

#define ID_QUIT 101
#define ID_CM93ZOOMG 102



//      Command identifiers for wxCommandEvents coming from the outside world.
//      Removed from enum to facilitate constant definition
#define ID_CMD_APPLY_SETTINGS 300
#define ID_CMD_NULL_REFRESH 301
#define ID_CMD_TRIGGER_RESIZE 302
#define ID_CMD_SETVP 303
#define ID_CMD_POST_JSON_TO_PLUGINS 304
#define ID_CMD_SET_LOCALE 305
#define ID_CMD_SOUND_FINISHED 306

#define N_STATUS_BAR_FIELDS_MAX 20

#ifdef __OCPN__ANDROID__
#define STAT_FIELD_COUNT 2
#define STAT_FIELD_TICK -1
#define STAT_FIELD_SOGCOG 0
#define STAT_FIELD_CURSOR_LL -1
#define STAT_FIELD_CURSOR_BRGRNG -1
#define STAT_FIELD_SCALE 1
#else
#define STAT_FIELD_COUNT 5
#define STAT_FIELD_TICK 0
#define STAT_FIELD_SOGCOG 1
#define STAT_FIELD_CURSOR_LL 2
#define STAT_FIELD_CURSOR_BRGRNG 3
#define STAT_FIELD_SCALE 4
#endif

//      Define a constant GPS signal watchdog timeout value
#define GPS_TIMEOUT_SECONDS 10

//    Define a timer value for Tide/Current updates
//    Note that the underlying data algorithms produce fresh data only every 15
//    minutes So maybe 5 minute updates should provide sufficient oversampling
#define TIMER_TC_VALUE_SECONDS 300

#define MAX_COG_AVERAGE_SECONDS 60
#define MAX_COGSOG_FILTER_SECONDS 60
//----------------------------------------------------------------------------
// fwd class declarations
//----------------------------------------------------------------------------
class ChartBase;
class wxSocketEvent;
class ocpnToolBarSimple;
class OCPN_DataStreamEvent;
class OCPN_SignalKEvent;
class DataStream;
class AisTargetData;

bool isSingleChart(ChartBase *chart);

class OCPN_ThreadMessageEvent : public wxEvent {
public:
  OCPN_ThreadMessageEvent(wxEventType commandType = wxEVT_NULL, int id = 0);
  ~OCPN_ThreadMessageEvent();

  // accessors
  void SetSString(std::string string) { m_string = string; }
  std::string GetSString() { return m_string; }

  // required for sending with wxPostEvent()
  wxEvent *Clone() const;

private:
  std::string m_string;
};

class MyFrame : public wxFrame {
  friend class SignalKEventHandler;

public:
  MyFrame(wxFrame *frame, const wxString &title, const wxPoint &pos,
          const wxSize &size, long style);

  ~MyFrame();

  int GetApplicationMemoryUse(void);

  void OnEraseBackground(wxEraseEvent &event);
  void OnMaximize(wxMaximizeEvent &event);
  void OnCloseWindow(wxCloseEvent &event);
  void OnExit(wxCommandEvent &event);
  void OnSize(wxSizeEvent &event);
  void OnMove(wxMoveEvent &event);
  void OnInitTimer(wxTimerEvent &event);
  void OnFrameTimer1(wxTimerEvent &event);
  bool DoChartUpdate(void);
  void OnEvtTHREADMSG(OCPN_ThreadMessageEvent &event);
  void OnEvtOCPN_NMEA(OCPN_DataStreamEvent &event);
  void OnEvtOCPN_SignalK(OCPN_SignalKEvent &event);
  void OnEvtOCPN_SIGNALK_Test(OCPN_SignalKEvent &event);
  void OnEvtPlugInMessage(OCPN_MsgEvent &event);
  void OnMemFootTimer(wxTimerEvent &event);
  void OnRecaptureTimer(wxTimerEvent &event);
  void OnSENCEvtThread(OCPN_BUILDSENC_ThreadEvent &event);
  void OnIconize(wxIconizeEvent &event);
  void OnBellsFinished(wxCommandEvent &event);

#ifdef wxHAS_POWER_EVENTS
  void OnSuspending(wxPowerEvent &event);
  void OnSuspended(wxPowerEvent &event);
  void OnSuspendCancel(wxPowerEvent &event);
  void OnResume(wxPowerEvent &event);
#endif  // wxHAS_POWER_EVENTS

  void RefreshCanvasOther(ChartCanvas *ccThis);
  void UpdateAllFonts(void);
  void PositionConsole(void);
  void OnToolLeftClick(wxCommandEvent &event);
  void ClearRouteTool();
  void DoStackUp(ChartCanvas *cc);
  void DoStackDown(ChartCanvas *cc);
  void selectChartDisplay(int type, int family);
  void applySettingsString(wxString settings);
  void setStringVP(wxString VPS);
  void InvalidateAllGL();
  void RefreshAllCanvas(bool bErase = true);
  void CancelAllMouseRoute();
  void InvalidateAllQuilts();

  void SetUpMode(ChartCanvas *cc, int mode);

  wxMenuBar *GetMainMenuBar() { return m_pMenuBar; }

  ChartCanvas *GetPrimaryCanvas();
  ChartCanvas *GetFocusCanvas();

  void DoStackDelta(ChartCanvas *cc, int direction);
  void DoSettings(void);
  void SwitchKBFocus(ChartCanvas *pCanvas);
  ChartCanvas *GetCanvasUnderMouse();
  int GetCanvasIndexUnderMouse();

  bool DropMarker(bool atOwnShip = true);

  void TriggerResize(wxSize sz);
  void OnResizeTimer(wxTimerEvent &event);

  void TriggerRecaptureTimer();
  bool SetGlobalToolbarViz(bool viz);

  void MouseEvent(wxMouseEvent &event);
  void CenterView(ChartCanvas *cc, const LLBBox &bbox);

  void JumpToPosition(ChartCanvas *cc, double lat, double lon, double scale);

  void ProcessCanvasResize(void);

  void BuildMenuBar(void);
  void ApplyGlobalSettings(bool bnewtoolbar);
  void RegisterGlobalMenuItems();
  void UpdateGlobalMenuItems();
  void UpdateGlobalMenuItems(ChartCanvas *cc);
  int DoOptionsDialog();
  bool ProcessOptionsDialog(int resultFlags, ArrayOfCDI *pNewDirArray);
  void DoPrint(void);
  void StopSockets(void);
  void ResumeSockets(void);
  void ToggleDataQuality(ChartCanvas *cc);
  void TogglebFollow(ChartCanvas *cc);
  void ToggleFullScreen();
  void ToggleChartBar(ChartCanvas *cc);
  void SetbFollow(ChartCanvas *cc);
  void ClearbFollow(ChartCanvas *cc);
  void ToggleChartOutlines(ChartCanvas *cc);
  void ToggleENCText(ChartCanvas *cc);
  void ToggleSoundings(ChartCanvas *cc);
  void ToggleRocks(void);
  bool ToggleLights(ChartCanvas *cc);
  void ToggleAnchor(ChartCanvas *cc);
  void ToggleAISDisplay(ChartCanvas *cc);
  void ToggleAISMinimizeTargets(ChartCanvas *cc);

  void ToggleTestPause(void);
  void TrackOn(void);
  void SetENCDisplayCategory(ChartCanvas *cc, enum _DisCat nset);
  void ToggleNavobjects(ChartCanvas *cc);

  Track *TrackOff(bool do_add_point = false);
  void TrackDailyRestart(void);
  bool ShouldRestartTrack();
  void ToggleColorScheme();
  void SetMenubarItemState(int item_id, bool state);
  void SetMasterToolbarItemState(int tool_id, bool state);

  void SetToolbarItemBitmaps(int tool_id, wxBitmap *bitmap,
                             wxBitmap *bmpDisabled);
  void SetToolbarItemSVG(int tool_id, wxString normalSVGfile,
                         wxString rolloverSVGfile, wxString toggledSVGfile);
  void ToggleQuiltMode(ChartCanvas *cc);
  void UpdateControlBar(ChartCanvas *cc);

  void ShowTides(bool bShow);
  void ShowCurrents(bool bShow);

  void SubmergeAllCanvasToolbars(void);
  void SurfaceAllCanvasToolbars(void);
  void ToggleAllToolbars(bool b_smooth = false);
  void SetAllToolbarScale(void);
  void SetGPSCompassScale(void);
  void InvalidateAllCanvasUndo();

  void RefreshGroupIndices(void);

  double GetBestVPScale(ChartBase *pchart);

  ColorScheme GetColorScheme();
  void SetAndApplyColorScheme(ColorScheme cs);

  void OnFrameTCTimer(wxTimerEvent &event);
  void OnFrameCOGTimer(wxTimerEvent &event);

  void HandleBasicNavMsg(std::shared_ptr<const BasicNavDataMsg> msg);
  void HandleGPSWatchdogMsg(std::shared_ptr<const GPSWatchdogMsg> msg);

  void ChartsRefresh();

  bool CheckGroup(int igroup);
  double GetMag(double a);
  double GetMag(double a, double lat, double lon);
  bool SendJSON_WMM_Var_Request(double lat, double lon, wxDateTime date);

  void DestroyPersistentDialogs();
  void TouchAISActive(void);
  void UpdateAISTool(void);

  void ActivateAISMOBRoute(const AisTargetData *ptarget);
  void UpdateAISMOBRoute(const AisTargetData *ptarget);

  wxStatusBar *m_pStatusBar;
  wxMenuBar *m_pMenuBar;
  int nBlinkerTick;
  bool m_bTimeIsSet;
  bool m_bDateIsSet;

  wxTimer InitTimer;
  int m_iInitCount;
  bool m_initializing;

  wxTimer FrameTCTimer;
  wxTimer FrameTimer1;
  wxTimer FrameCOGTimer;
  wxTimer MemFootTimer;
  wxTimer m_resizeTimer;

  int m_BellsToPlay;
  wxTimer BellsTimer;

  //      PlugIn support
  int GetNextToolbarToolId() { return m_next_available_plugin_tool_id; }
  void RequestNewToolbarArgEvent(wxCommandEvent &WXUNUSED(event)) {
    return RequestNewMasterToolbar();
  }
  void RequestNewToolbars(bool bforcenew = false);

  void ActivateMOB(void);
  void UpdateGPSCompassStatusBoxes(bool b_force_new = false);
  void UpdateRotationState(double rotation);

  bool UpdateChartDatabaseInplace(ArrayOfCDI &DirArray, bool b_force,
                                  bool b_prog,
                                  const wxString &ChartListFileName);

  bool m_bdefer_resize;
  wxSize m_defer_size;
  wxSize m_newsize;
  double COGTable[MAX_COG_AVERAGE_SECONDS];

  void FastClose();
  void SetChartUpdatePeriod();
  void CreateCanvasLayout(bool b_useStoredSize = false);
  void LoadHarmonics();
  void ReloadAllVP();
  void SetCanvasSizes(wxSize frameSize);

  ocpnToolBarSimple *CreateMasterToolbar();
  void RequestNewMasterToolbar(bool bforcenew = true);
  bool CheckAndAddPlugInTool();
  bool AddDefaultPositionPlugInTools();

  void NotifyChildrenResize(void);
  void UpdateCanvasConfigDescriptors();
  void ScheduleSettingsDialog();
  static void RebuildChartDatabase();
  void PositionIENCToolbar();

  void InitAppMsgBusListener();
  void InitApiListeners();
  void UpdateStatusBar(void);

private:
  void CheckToolbarPosition();
  void ODoSetSize(void);
  void DoCOGSet(void);

  void UpdateAllToolbars(ColorScheme cs);

  void FilterCogSog(void);

  void ApplyGlobalColorSchemetoStatusBar(void);

  bool ScrubGroupArray();
  wxString GetGroupName(int igroup);

  bool EvalPriority(const wxString &message, DataStream *pDS);
  void SetAISDisplayStyle(ChartCanvas *cc, int StyleIndx);

  bool GetMasterToolItemShow(int toolid);
  void OnToolbarAnimateTimer(wxTimerEvent &event);
  bool CollapseGlobalToolbar();

  int m_StatusBarFieldCount;

  wxDateTime m_MMEAeventTime;
  unsigned long m_ulLastNMEATicktime;
  int m_tick_idx;

  wxString m_last_reported_chart_name;
  wxString m_last_reported_chart_pubdate;

  wxString m_lastAISiconName;

  //      Plugin Support
  int m_next_available_plugin_tool_id;

  double COGFilterTable[MAX_COGSOG_FILTER_SECONDS];
  double SOGFilterTable[MAX_COGSOG_FILTER_SECONDS];

  bool m_bpersistent_quilt;
  int m_ChartUpdatePeriod;
  bool m_last_bGPSValid;
  bool m_last_bVelocityValid;

  wxString prev_locale;
  bool bPrevQuilt;
  bool bPrevFullScreenQuilt;
  bool bPrevOGL;

  time_t m_fixtime;
  wxMenu *piano_ctx_menu;
  bool b_autofind;

  time_t m_last_track_rotation_ts;
  wxRect m_mainlast_tb_rect;
  wxTimer ToolbarAnimateTimer;
  int m_nMasterToolCountShown;
  wxTimer m_recaptureTimer;
  bool m_b_new_data;

  std::unique_ptr<LoadErrorsDlgCtrl> m_load_errors_dlg_ctrl;

  ObservableListener listener_basic_navdata;
  ObservableListener listener_gps_watchdog;
  ObsListener m_on_raise_listener;
  ObsListener m_on_quit_listener;

  DECLARE_EVENT_TABLE()
};


#endif  // _OFRAME_H
