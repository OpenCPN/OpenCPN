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
#include "model/gui.h"
#include "model/idents.h"
#include "model/ocpn_types.h"
#include "model/track.h"
#include "model/comm_appmsg_bus.h"
#include "model/rest_server.h"

#include "bbox.h"
#include "canvas_menu.h"
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
#include "top_frame.h"
#include "undo_defs.h"

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

// FIXME (leamas) to have utility functions in top window is a really bad idea.
bool ShowNavWarning();

void ApplyLocale(void);

/**
 * Main application frame. Top-level window frame for OpenCPN that manages
 * overall application state, menus, toolbars, and child windows like chart
 * canvases.
 */
class MyFrame : public AbstractTopFrame, public S52PlibUtils {
public:
  MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size,
          RestServer& rest_server, wxAuiDefaultDockArt* pauidockart,
          OpenFileFunc open_gpx_file);

  ~MyFrame();

  void SetGPSCompassScale() override;
  void FastClose() override;
  void RefreshAllCanvas(bool bErase = true) override;
  void UpdateStatusBar() override;
  void ToggleFullScreen() override;
  Track* TrackOff(bool do_add_point = false) override;
  void TrackOn(void) override;
  void ProcessOptionsDialog(int resultFlags) override {
    ProcessOptionsDialog(resultFlags, nullptr);
  }

  void SetAlertString(wxString msg) override {
    GetPrimaryCanvas()->SetAlertString(msg);
  }

  void JumpToPosition(double lat, double lon) override {
    if (!GetFocusCanvas()) return;
    JumpToPosition(GetFocusCanvas(), lat, lon, GetFocusCanvas()->GetVPScale());
  }

  void JumpToPosition(double lat, double lon, double scale) override {
    if (!GetFocusCanvas()) return;
    JumpToPosition(GetFocusCanvas(), lat, lon, scale);
  }

  void JumpToPosition(AbstractChartCanvas* acc, double lat, double lon,
                      double scale) override {
    if (!acc) return;
    auto cc = dynamic_cast<ChartCanvas*>(acc);
    assert(cc);
    JumpToPosition(cc, lat, lon, scale);
  }
  void JumpToPosition(AbstractChartCanvas* acc, double lat,
                      double lon) override {
    if (!acc) return;
    auto cc = dynamic_cast<ChartCanvas*>(acc);
    assert(cc);
    JumpToPosition(cc, lat, lon, GetFocusCanvas()->GetVPScale());
  }

  AbstractChartCanvas* GetAbstractPrimaryCanvas() override {
    return GetPrimaryCanvas();
  }

  AbstractChartCanvas* GetAbstractFocusCanvas() override {
    return GetFocusCanvas();
  }

  void SwitchKBFocus(AbstractChartCanvas* acc) override {
    return SwitchKBFocusCanvas(dynamic_cast<ChartCanvas*>(acc));
  }

  double GetCanvasTrueScale() override {
    return GetPrimaryCanvas()->GetCanvasTrueScale();
  }

  double GetPixPerMM() override { return GetPrimaryCanvas()->GetPixPerMM(); }

  double GetContentScaleFactor() override {
    if (GetPrimaryCanvas())
      return GetPrimaryCanvas()->GetContentScaleFactor();
    else
      return 1.0;
  }

  void RequestNewToolbars(bool bforcenew = false) override;

  bool GetCanvasPointPix(double rlat, double rlon, wxPoint* r) override {
    return GetFocusCanvas()->GetCanvasPointPix(rlat, rlon, r);
  }

  virtual wxSize GetFocusCanvasSize() override {
    return GetFocusCanvas() ? GetFocusCanvas()->GetSize() : wxSize(0, 0);
  }

  void CancelAllMouseRoute() override;
  void InvalidateAllCanvasUndo() override;
  void PositionConsole() override;
  void InvalidateAllGL() override;
  double GetBestVPScale(AbstractChart* arg) override {
    auto chart = dynamic_cast<ChartBase*>(arg);
    assert(chart);
    return GetPrimaryCanvas()->GetBestVPScale(chart);
  }

  void DoStackUp(AbstractChartCanvas* cc) override;
  void DoStackDown(AbstractChartCanvas* cc) override;
  void LoadHarmonics() override;

  bool DropMarker(bool atOwnShip = true) override;
  double GetMag(double a, double lat, double lon) override;
  void SetMasterToolbarItemState(int tool_id, bool state) override;
  bool DoChartUpdate() override;
  void ProcessCanvasResize() override;
  bool SetGlobalToolbarViz(bool viz) override;
  void ToggleQuiltMode(AbstractChartCanvas* acc) override {
    auto cc = dynamic_cast<ChartCanvas*>(acc);
    ToggleQuiltMode(cc);
  }
  void UpdateGlobalMenuItems(AbstractChartCanvas* acc) override {
    UpdateGlobalMenuItems(dynamic_cast<ChartCanvas*>(acc));
  }
  void UpdateGlobalMenuItems() override;
  void RefreshCanvasOther(AbstractChartCanvas* ccThis) override {
    RefreshCanvasOther(dynamic_cast<ChartCanvas*>(ccThis));
  }
  virtual double* GetCOGTable() override { return COGTable; }
  virtual void StartCogTimer() override {
    FrameCOGTimer.Start(100, wxTIMER_CONTINUOUS);
  }

  wxWindow* GetPrimaryCanvasWindow() override { return GetPrimaryCanvas(); }
  void ApplyGlobalSettings(bool bnewtoolbar) override;
  void SetMenubarItemState(int item_id, bool state) override;
  void ToggleColorScheme() override;
  void ActivateMOB() override;
  void ToggleTestPause() override;

  void ToggleChartBar(AbstractChartCanvas* acc) override {
    ToggleChartBar(dynamic_cast<ChartCanvas*>(acc));
  }
  void DoSettings() override;

  void UpdateRotationState(double rotation) override;
  void SetChartUpdatePeriod() override;
  wxStatusBar* GetStatusBar() override { return wxFrame::GetStatusBar(); }
  wxStatusBar* GetFrameStatusBar() const override { return m_pStatusBar; }
  void SetENCDisplayCategory(AbstractChartCanvas* acc,
                             enum _DisCat nset) override {
    SetENCDisplayCategory(dynamic_cast<ChartCanvas*>(acc), nset);
  }
  int GetCanvasIndexUnderMouse() override;

  double GetCanvasRefScale() override {
    return GetPrimaryCanvas()->GetVP().ref_scale;
    ;
  }

#ifdef ocpnUSE_GL
  void SendGlJsonConfigMsg() override {
    if (GetPrimaryCanvas() && GetPrimaryCanvas()->GetglCanvas())
      GetPrimaryCanvas()->GetglCanvas()->SendJSONConfigMessage();
  }
  bool CanAccelerateGlPanning() override {
    return GetPrimaryCanvas()->GetglCanvas()->CanAcceleratePanning();
  }
  void SetupGlCompression() override {
    GetPrimaryCanvas()->GetglCanvas()->SetupCompression();
  }
  wxString GetGlVersionString() override {
    return GetPrimaryCanvas()->GetglCanvas()->GetVersionString();
  }
  wxGLCanvas* GetWxGlCanvas() override {
    return GetPrimaryCanvas()->GetglCanvas();
  }

#else
  void SendGlJsonConfigMsg() override {}
  bool CanAccelerateGlPanning() override { return false; }
  void SetupGlCompression() override {}
  wxString GetGlVersionString() override { return ""; }
  wxGLCanvas* GetWxGlCanvas() override { return nullptr; }
#endif
  void SwitchKBFocusCanvas(ChartCanvas* pCanvas);
  int GetNextToolbarToolId() override {
    return m_next_available_plugin_tool_id;
  }
  void SetToolbarItemBitmaps(int tool_id, wxBitmap* bitmap,
                             wxBitmap* bmpDisabled) override;
  void SetToolbarItemSVG(int tool_id, wxString normalSVGfile,
                         wxString rolloverSVGfile,
                         wxString toggledSVGfile) override;
  void UpdateAllFonts() override;
  void ScheduleReconfigAndSettingsReload(bool reload, bool new_dialog) override;
  void ScheduleReloadCharts() override;
  void ChartsRefresh() override;
  void FreezeCharts() override;
  void ThawCharts() override;

  void ScheduleDeleteSettingsDialog() override;

  void BeforeUndoableAction(UndoType undo_type, RoutePoint* point,
                            UndoBeforePointerType pointer_type,
                            UndoItemPointer pointer) override {
    if (!GetPrimaryCanvas()) return;
    GetPrimaryCanvas()->undo->BeforeUndoableAction(undo_type, point,
                                                   pointer_type, pointer);
  }

  virtual void AfterUndoableAction(UndoItemPointer pointer) override {
    GetPrimaryCanvas()->undo->AfterUndoableAction(pointer);
  }
  virtual void TouchAISActive() override;
  virtual void UpdateAISMOBRoute(const AisTargetData* ptarget) override;
  virtual void ActivateAISMOBRoute(const AisTargetData* ptarget) override;
  void EnableSettingsTool(bool _enable) override {
    if (g_MainToolbar) {
      g_MainToolbar->EnableTool(ID_SETTINGS, _enable);
      g_MainToolbar->GetToolbar()->SetDirty(true);
      g_MainToolbar->RefreshToolbar();
    }
  }
  void OnToolLeftClick(wxCommandEvent& event) override;

  void SetENCDisplayCategory(ChartCanvas* cc, enum _DisCat nset);
  void ToggleQuiltMode(ChartCanvas* cc);

  wxFont* GetFont(wxFont* font, double scale) override;
  wxFont* GetDefaultFont(wxString label, int Ptsize) override;
  wxFont* GetScaledFont(int pointSize, wxFontFamily family, wxFontStyle style,
                        wxFontWeight weight, const wxString faceName,
                        double scale) override;

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
  void selectChartDisplay(int type, int family);
  void applySettingsString(wxString settings);
  void setStringVP(wxString VPS);
  void InvalidateAllQuilts();

  void SetUpMode(ChartCanvas* cc, int mode);

  ChartCanvas* GetPrimaryCanvas();
  ChartCanvas* GetFocusCanvas();
  void DoStackUp(ChartCanvas* cc);
  void DoStackDown(ChartCanvas* cc);
  void NotifyChildrenResize();

  void DoStackDelta(ChartCanvas* cc, int direction);
  void DoSettingsNew(void);
  ChartCanvas* GetCanvasUnderMouse();

  void TriggerRecaptureTimer();

  void MouseEvent(wxMouseEvent& event);
  void CenterView(ChartCanvas* cc, const LLBBox& bbox);

  void JumpToPosition(ChartCanvas* cc, double lat, double lon, double scale);

  void BuildMenuBar(void);
  void RegisterGlobalMenuItems();
  void UpdateGlobalMenuItems(ChartCanvas* cc);
  void DoOptionsDialog();
  void ProcessOptionsDialog(int resultFlags, ArrayOfCDI* pNewDirArray);
  void PrepareOptionsClose(options* settings, int settings_return_value);

  void DoPrint(void);
  void ToggleDataQuality(ChartCanvas* cc);
  void TogglebFollow(ChartCanvas* cc);
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

  void ToggleNavobjects(ChartCanvas* cc);

  void TrackDailyRestart(void);
  bool ShouldRestartTrack();
  void InitializeTrackRestart();

  void UpdateControlBar(ChartCanvas* cc);

  void SubmergeAllCanvasToolbars(void);
  void SurfaceAllCanvasToolbars(void);
  void SetAllToolbarScale(void);

  void RefreshGroupIndices(void);

  double GetBestVPScale(ChartBase* pchart);

  DataMonitor* GetDataMonitor() const { return m_data_monitor; }

  void SetAndApplyColorScheme(ColorScheme cs);

  void OnFrameTCTimer(wxTimerEvent& event);
  void OnFrameCOGTimer(wxTimerEvent& event);

  void HandleBasicNavMsg(std::shared_ptr<const BasicNavDataMsg> msg);
  void HandleGPSWatchdogMsg(std::shared_ptr<const GPSWatchdogMsg> msg);

  bool CheckGroup(int igroup);
  bool SendJSON_WMM_Var_Request(double lat, double lon, wxDateTime date);

  void DestroyPersistentDialogs();
  void UpdateAISTool(void);
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
  void RequestNewToolbarArgEvent(wxCommandEvent& WXUNUSED(event)) {
    return RequestNewMasterToolbar();
  }

  void UpdateGPSCompassStatusBoxes(bool b_force_new = false);

  bool UpdateChartDatabaseInplace(ArrayOfCDI& DirArray, bool b_force,
                                  bool b_prog,
                                  const wxString& ChartListFileName);
  void FinalizeChartDBUpdate();

  bool m_bdefer_resize;
  wxSize m_defer_size;
  double COGTable[kMaxCogAverageSeconds];

  void CreateCanvasLayout(bool b_useStoredSize = false);
  void ReloadAllVP();
  void SetCanvasSizes(wxSize frameSize);

  ocpnToolBarSimple* CreateMasterToolbar();
  void RequestNewMasterToolbar(bool bforcenew = true);
  bool CheckAndAddPlugInTool();
  bool AddDefaultPositionPlugInTools();

  void UpdateCanvasConfigDescriptors();
  void ScheduleSettingsDialog();
  void ScheduleSettingsDialogNew();
  static void RebuildChartDatabase();
  void PositionIENCToolbar();

  void InitAppMsgBusListener();
  void InitApiListeners();
  void ReleaseApiListeners();
  void ConfigureStatusBar();

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
  ObsListener m_reload_charts_listener;
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
