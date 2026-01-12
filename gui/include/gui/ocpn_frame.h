/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * OpenCPN top window
 */

#ifndef _OFRAME_H
#define _OFRAME_H

#include <wx/print.h>
#include <wx/power.h>
#include <wx/artprov.h>
#include <wx/socket.h>
#include <wx/html/htmlwin.h>
#include <wx/aui/aui.h>
#include <wx/aui/dockart.h>

#include <memory>
#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "ocpn_plugin.h"

#include "model/ais_target_data.h"
#include "model/ocpn_types.h"
#include "model/track.h"
#include "model/comm_appmsg_bus.h"
#include "model/rest_server.h"

#include "bbox.h"
#include "chartbase.h"
#include "chartdbs.h"
#include "chcanv.h"
#include "color_handler.h"
#include "connections_dlg.h"
#include "data_monitor.h"
#include "displays.h"
#include "gui_lib.h"
#include "load_errors_dlg.h"
#include "observable_evtvar.h"
#include "options.h"
#include "pluginmanager.h"
#include "s52_plib_utils.h"
#include "s52s57.h"
#include "s57registrar_mgr.h"
#include "senc_manager.h"
#include "toolbar.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define TIMER_GFRAME_1 999

// Command identifiers for wxCommandEvents coming from the outside world.
// Removed from enum to facilitate constant definition
//
// NOLINTBEGIN
#define ID_CMD_APPLY_SETTINGS 300
#define ID_CMD_NULL_REFRESH 301
#define ID_CMD_TRIGGER_RESIZE 302
#define ID_CMD_SETVP 303
#define ID_CMD_POST_JSON_TO_PLUGINS 304
#define ID_CMD_SOUND_FINISHED 306
// NOLINTEND

using OpenFileFunc = std::function<bool(const std::string& path)>;
//----------------------------------------------------------------------------
// fwd class declarations
//----------------------------------------------------------------------------

class MyFrame;          // forward
extern MyFrame* gFrame; /**< Global instance */

class options;  // circular

// FIXME (leamas) to have utility functions in top window is a really bad idea.
bool ShowNavWarning();

void ApplyLocale(void);

/**
 * Main application frame. Top-level window frame for OpenCPN that manages
 * overall application state, menus, toolbars, and child windows like chart
 * canvases.
 */
class MyFrame : public wxFrame, public S52PlibUtils {
public:
  MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size,
          RestServer& rest_server, wxAuiDefaultDockArt* pauidockart,
          OpenFileFunc open_gpx_file);

  ~MyFrame();

  wxFont* GetFont(wxFont* font, double scale) override;
  int GetApplicationMemoryUse(void);

  void OnEraseBackground(wxEraseEvent& event);
  void OnMaximize(wxMaximizeEvent& event);
  void OnCloseWindow(wxCloseEvent& event);
  void OnExit(wxCommandEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnMove(wxMoveEvent& event);
  void OnInitTimer(wxTimerEvent& event);

  /**
   * Main application timer handler called approximately once per second.
   *
   * Key responsibilities include anchor watch monitoring, GPS position
   * updates to plugins, toolbar management, and chart database refreshes.
   *
   * @param event The wxTimerEvent that triggered this handler
   */
  void OnFrameTimer1(wxTimerEvent& event);

  bool DoChartUpdate(void);
  void OnEvtPlugInMessage(OCPN_MsgEvent& event);
  void OnMemFootTimer(wxTimerEvent& event);
  void OnRecaptureTimer(wxTimerEvent& event);
  void OnSENCEvtThread(OCPN_BUILDSENC_ThreadEvent& event);
  void OnIconize(wxIconizeEvent& event);
  void OnBellsFinished(wxCommandEvent& event);

  /**
   * High-frequency timer handler running at 10Hz for smooth navigation updates.
   *
   * This timer event handler runs at 10Hz to provide smooth, high-frequency
   * updates for navigation display and position estimation. It serves two
   * primary purposes: handling compass rose rotation in non-North-Up modes and
   * providing interpolated position/heading estimates when 10Hz updates are
   * enabled.
   *
   * @param event The wxTimerEvent that triggered this handler (100ms interval)
   */
  void OnFrameTenHzTimer(wxTimerEvent& event);

#ifdef wxHAS_POWER_EVENTS
  void OnSuspending(wxPowerEvent& event);
  void OnSuspended(wxPowerEvent& event);
  void OnSuspendCancel(wxPowerEvent& event);
  void OnResume(wxPowerEvent& event);
#endif  // wxHAS_POWER_EVENTS

  void RefreshCanvasOther(ChartCanvas* ccThis);
  void UpdateAllFonts(void);
  void PositionConsole(void);
  void OnToolLeftClick(wxCommandEvent& event);
  void DoStackUp(ChartCanvas* cc);
  void DoStackDown(ChartCanvas* cc);
  void selectChartDisplay(int type, int family);
  void applySettingsString(wxString settings);
  void setStringVP(wxString VPS);
  void InvalidateAllGL();
  void RefreshAllCanvas(bool bErase = true);
  void CancelAllMouseRoute();
  void InvalidateAllQuilts();

  void SetUpMode(ChartCanvas* cc, int mode);

  ChartCanvas* GetPrimaryCanvas();
  ChartCanvas* GetFocusCanvas();

  void DoStackDelta(ChartCanvas* cc, int direction);
  void DoSettings(void);
  void DoSettingsNew(void);
  void SwitchKBFocus(ChartCanvas* pCanvas);
  ChartCanvas* GetCanvasUnderMouse();
  int GetCanvasIndexUnderMouse();

  bool DropMarker(bool atOwnShip = true);

  void TriggerRecaptureTimer();
  bool SetGlobalToolbarViz(bool viz);

  void MouseEvent(wxMouseEvent& event);
  void CenterView(ChartCanvas* cc, const LLBBox& bbox);

  void JumpToPosition(ChartCanvas* cc, double lat, double lon, double scale);

  void ProcessCanvasResize(void);

  void BuildMenuBar(void);
  void ApplyGlobalSettings(bool bnewtoolbar);
  void RegisterGlobalMenuItems();
  void UpdateGlobalMenuItems();
  void UpdateGlobalMenuItems(ChartCanvas* cc);
  void DoOptionsDialog();
  void ProcessOptionsDialog(int resultFlags, ArrayOfCDI* pNewDirArray);
  void PrepareOptionsClose(options* settings, int settings_return_value);

  void DoPrint(void);
  void ToggleDataQuality(ChartCanvas* cc);
  void TogglebFollow(ChartCanvas* cc);
  void ToggleFullScreen();
  void ToggleChartBar(ChartCanvas* cc);
  void SetbFollow(ChartCanvas* cc);
  void ClearbFollow(ChartCanvas* cc);
  void ToggleChartOutlines(ChartCanvas* cc);
  void ToggleENCText(ChartCanvas* cc);
  void ToggleSoundings(ChartCanvas* cc);
#if 0
  void ToggleRocks(void);
#endif
  bool ToggleLights(ChartCanvas* cc);
  void ToggleAnchor(ChartCanvas* cc);
  void ToggleAISDisplay(ChartCanvas* cc);
  void ToggleAISMinimizeTargets(ChartCanvas* cc);

  void ToggleTestPause(void);
  void TrackOn(void);
  void SetENCDisplayCategory(ChartCanvas* cc, enum _DisCat nset);
  void ToggleNavobjects(ChartCanvas* cc);

  Track* TrackOff(bool do_add_point = false);
  void TrackDailyRestart(void);
  bool ShouldRestartTrack();
  void InitializeTrackRestart();

  void ToggleColorScheme();
  void SetMenubarItemState(int item_id, bool state);
  void SetMasterToolbarItemState(int tool_id, bool state);

  void SetToolbarItemBitmaps(int tool_id, wxBitmap* bitmap,
                             wxBitmap* bmpDisabled);
  void SetToolbarItemSVG(int tool_id, wxString normalSVGfile,
                         wxString rolloverSVGfile, wxString toggledSVGfile);
  void ToggleQuiltMode(ChartCanvas* cc);
  void UpdateControlBar(ChartCanvas* cc);

  void SubmergeAllCanvasToolbars(void);
  void SurfaceAllCanvasToolbars(void);
  void SetAllToolbarScale(void);
  void SetGPSCompassScale(void);
  void InvalidateAllCanvasUndo();

  void RefreshGroupIndices(void);

  double GetBestVPScale(ChartBase* pchart);

  DataMonitor* GetDataMonitor() const { return m_data_monitor; }

  void SetAndApplyColorScheme(ColorScheme cs);

  void OnFrameTCTimer(wxTimerEvent& event);
  void OnFrameCOGTimer(wxTimerEvent& event);

  void HandleBasicNavMsg(std::shared_ptr<const BasicNavDataMsg> msg);
  void HandleGPSWatchdogMsg(std::shared_ptr<const GPSWatchdogMsg> msg);

  void ChartsRefresh();

  bool CheckGroup(int igroup);
  double GetMag(double a, double lat, double lon);
  bool SendJSON_WMM_Var_Request(double lat, double lon, wxDateTime date);

  void DestroyPersistentDialogs();
  void TouchAISActive(void);
  void UpdateAISTool(void);

  void ActivateAISMOBRoute(const AisTargetData* ptarget);
  void UpdateAISMOBRoute(const AisTargetData* ptarget);
  wxStatusBar* GetFrameStatusBar() { return m_pStatusBar; }

  wxMenuBar* m_pMenuBar;
  bool m_bTimeIsSet;

  wxTimer InitTimer;
  int m_iInitCount;
  bool m_initializing;

  wxTimer FrameTCTimer;
  wxTimer FrameTimer1;
  wxTimer FrameCOGTimer;
  wxTimer MemFootTimer;
  wxTimer m_resizeTimer;
  wxTimer FrameTenHzTimer;

  int m_BellsToPlay;
  wxTimer BellsTimer;

  wxGenericProgressDialog* Updateprog = nullptr;

  //      PlugIn support
  int GetNextToolbarToolId() { return m_next_available_plugin_tool_id; }
  void RequestNewToolbarArgEvent(wxCommandEvent& WXUNUSED(event)) {
    return RequestNewMasterToolbar();
  }
  void RequestNewToolbars(bool bforcenew = false);

  void ActivateMOB(void);
  void UpdateGPSCompassStatusBoxes(bool b_force_new = false);
  void UpdateRotationState(double rotation);

  bool UpdateChartDatabaseInplace(ArrayOfCDI& DirArray, bool b_force,
                                  bool b_prog,
                                  const wxString& ChartListFileName);
  void FinalizeChartDBUpdate();

  bool m_bdefer_resize;
  wxSize m_defer_size;
  double COGTable[kMaxCogAverageSeconds];

  void FastClose();
  void SetChartUpdatePeriod();
  void CreateCanvasLayout(bool b_useStoredSize = false);
  void LoadHarmonics();
  void ReloadAllVP();
  void SetCanvasSizes(wxSize frameSize);

  ocpnToolBarSimple* CreateMasterToolbar();
  void RequestNewMasterToolbar(bool bforcenew = true);
  bool CheckAndAddPlugInTool();
  bool AddDefaultPositionPlugInTools();

  void UpdateCanvasConfigDescriptors();
  void ScheduleSettingsDialog();
  void ScheduleSettingsDialogNew();
  void ScheduleDeleteSettingsDialog();
  void ScheduleReconfigAndSettingsReload(bool reload, bool new_dialog);
  void ScheduleReloadCharts();
  static void RebuildChartDatabase();
  void PositionIENCToolbar();

  void InitAppMsgBusListener();
  void InitApiListeners();
  void ReleaseApiListeners();
  void UpdateStatusBar(void);
  void ConfigureStatusBar();
  void FreezeCharts();
  void ThawCharts();

private:
  void ProcessUnitTest();
  bool ProcessQuitFlag();
  void ProcessDeferredTrackOn();
  void SendFixToPlugins();
  void ProcessAnchorWatch();
  void ProcessLogAndBells();
  void CalculateCOGAverage();
  void CheckToolbarPosition();

  void ODoSetSize(void);
  void DoCOGSet(void);

  void UpdateAllToolbars(ColorScheme cs);

  void FilterCogSog(void);

  void ApplyGlobalColorSchemetoStatusBar(void);

  bool ScrubGroupArray();

  void OnToolbarAnimateTimer(wxTimerEvent& event);
  bool CollapseGlobalToolbar();

  int m_StatusBarFieldCount;

  wxDateTime m_MMEAeventTime;
  unsigned long m_ulLastNMEATicktime;
  int m_tick_idx;
  wxDateTime m_fix_start_time;

  wxString m_last_reported_chart_name;
  wxString m_last_reported_chart_pubdate;

  wxString m_lastAISiconName;

  //      Plugin Support
  int m_next_available_plugin_tool_id;

  double COGFilterTable[kMaxCogsogFilterSeconds];
  double SOGFilterTable[kMaxCogsogFilterSeconds];

  int m_ChartUpdatePeriod;
  bool m_last_bGPSValid;
  bool m_last_bVelocityValid;
  double m_last_hdt;

  wxString prev_locale;

  /**
   * The last time basic navigational data was received, or 0 if no data
   * has been received.
   *
   * @todo Change time_t to wxLongLong, as time_t is susceptible to the
   * year 2038 problem on 32-bit builds.
   */
  time_t m_fixtime;
  bool b_autofind;

  wxDateTime m_target_rotate_time;

  wxTimer ToolbarAnimateTimer;
  int m_nMasterToolCountShown;
  wxTimer m_recaptureTimer;

  std::unique_ptr<LoadErrorsDlgCtrl> m_load_errors_dlg_ctrl;
  wxString m_gshhg_chart_loc;

private:
  ObservableListener listener_basic_navdata;
  ObservableListener listener_gps_watchdog;
  ObsListener m_on_raise_listener;
  ObsListener m_on_quit_listener;
  ObsListener m_routes_update_listener;
  ObsListener m_evt_drv_msg_listener;
  ObsListener m_update_statusbar_listener;
  ObsListener m_center_aistarget_listener;

  ToolbarDlgCallbacks m_toolbar_callbacks;

  wxStatusBar* m_pStatusBar;
  ConnectionsDlg* m_connections_dlg;
  bool m_need_new_options;
  wxArrayString pathArray;
  double restoreScale[4];
  unsigned int last_canvasConfig;
  DataMonitor* m_data_monitor;
  wxAuiDefaultDockArt* m_pauidockart;
  RestServer& m_rest_server;
  OpenFileFunc m_open_gpx_file;

  void CenterAisTarget(const std::shared_ptr<const AisTargetData>& ais_target);

  DECLARE_EVENT_TABLE()
};

#endif  // _OFRAME_H
