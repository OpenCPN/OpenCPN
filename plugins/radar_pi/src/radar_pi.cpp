/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 */

#include "radar_pi.h"
#include "GuardZone.h"
#include "GuardZoneBogey.h"
#include "Kalman.h"
#include "MessageBox.h"
#include "OptionsDialog.h"
#include "RadarMarpa.h"
#include "SelectDialog.h"
#include "icons.h"
#include "navico/NavicoLocate.h"
#include "nmea0183/nmea0183.h"

PLUGIN_BEGIN_NAMESPACE

#undef M_SETTINGS
#define M_SETTINGS m_settings

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin *create_pi(void *ppimgr) { return new radar_pi(ppimgr); }

extern "C" DECL_EXP void destroy_pi(opencpn_plugin *p) { delete p; }

/********************************************************************************************************/
//   Distance measurement for simple sphere
/********************************************************************************************************/

double local_distance(GeoPosition pos1, GeoPosition pos2) {
  double s1 = deg2rad(pos1.lat);
  double l1 = deg2rad(pos1.lon);
  double s2 = deg2rad(pos2.lat);
  double l2 = deg2rad(pos2.lon);
  double theta = l2 - l1;

  // Spherical Law of Cosines
  double dist = acos(sin(s1) * sin(s2) + cos(s1) * cos(s2) * cos(theta));

  dist = fabs(rad2deg(dist)) * 60;  // nautical miles/degree
  return dist;
}

double local_bearing(GeoPosition pos1, GeoPosition pos2) {
  double s1 = deg2rad(pos1.lat);
  double l1 = deg2rad(pos1.lon);
  double s2 = deg2rad(pos2.lat);
  double l2 = deg2rad(pos2.lon);
  double theta = l2 - l1;

  double y = sin(theta) * cos(s2);
  double x = cos(s1) * sin(s2) - sin(s1) * cos(s2) * cos(theta);

  double brg = fmod(rad2deg(atan2(y, x)) + 360.0, 360.0);
  return brg;
}

static double radar_distance(GeoPosition pos1, GeoPosition pos2, char unit) {
  double dist = local_distance(pos1, pos2);

  switch (unit) {
    case 'M':  // statute miles
      dist = dist * 1.1515;
      break;
    case 'K':  // kilometers
      dist = dist * 1.852;
      break;
    case 'm':  // meters
      dist = dist * 1852.0;
      break;
    case 'N':  // nautical miles
      break;
  }
  return dist;
}

//---------------------------------------------------------------------------------------------------------
//
//    Radar PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

enum { TIMER_ID = 51 };

BEGIN_EVENT_TABLE(radar_pi, wxEvtHandler)
EVT_TIMER(TIMER_ID, radar_pi::OnTimerNotify)
END_EVENT_TABLE()

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

radar_pi::radar_pi(void *ppimgr) : opencpn_plugin_116(ppimgr) {
  m_boot_time = wxGetUTCTimeMillis();
  m_initialized = false;
  m_predicted_position_initialised = false;

  // Create the PlugIn icons
  initialize_images();
  m_pdeficon = new wxBitmap(*_img_radar_blank);

  m_opengl_mode = OPENGL_UNKOWN;
  m_opengl_mode_changed = false;
  m_opencpn_gl_context = 0;
  m_opencpn_gl_context_broken = false;

  m_timer = 0;
  for (int r = 0; r < RADARS; r++) {
    m_context_menu_control_id[r] = -1;
  }

  m_first_init = true;
}

radar_pi::~radar_pi() {}

/*
 * Init() is called -every- time that the plugin is enabled. If a user is being nasty
 * they can enable/disable multiple times in the overview. Grrr!
 *
 */

int radar_pi::Init(void) {
  if (m_initialized) {
    // Whoops, shouldn't happen
    return PLUGIN_OPTIONS;
  }

  if (m_first_init) {
#ifdef __WXMSW__
    WSADATA wsaData;

    // Initialize Winsock
    DWORD r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (r != 0) {
      wxLogError(wxT("radar_pi: Unable to initialise Windows Sockets, error %d"), r);
      // Might as well give up now
      return 0;
    }
    wxLogMessage(wxT("radar_pi: Windows sockets initialized"));
#endif

    AddLocaleCatalog(_T("opencpn-radar_pi"));

    m_pconfig = GetOCPNConfigObject();
    m_first_init = false;
  }

  time_t now = time(0);

  // Font can change so initialize every time
  m_font = GetOCPNGUIScaledFont_PlugIn(_T("Dialog"));
  m_fat_font = m_font;
  m_fat_font.SetWeight(wxFONTWEIGHT_BOLD);
  m_fat_font.SetPointSize(m_font.GetPointSize() + 1);
  m_small_font.SetPointSize(m_font.GetPointSize() - 1);
  m_max_canvas = 0;
  for (int i = 0; i < MAX_CHART_CANVAS; i++) {
    m_chart_overlay[i] = -1;
  }
  m_context_menu_canvas_index = -1;

  m_var = 0.0;
  m_var_source = VARIATION_SOURCE_NONE;
  m_bpos_set = false;
  m_ownship.lat = nan("");
  m_ownship.lon = nan("");
  m_cursor_pos.lat = nan("");
  m_cursor_pos.lon = nan("");
  m_right_click_pos.lat = nan("");
  m_right_click_pos.lon = nan("");

  m_guard_bogey_seen = false;
  m_guard_bogey_confirmed = false;
  m_sent_toolbar_button = TB_NONE;
  m_toolbar_button = TB_NONE;
  m_opengl_mode_changed = false;
  m_notify_radar_window_viz = false;
  m_notify_control_dialog = false;

  m_render_busy = false;

  m_bogey_dialog = 0;
  m_alarm_sound_timeout = 0;
  m_guard_bogey_timeout = 0;
  m_bpos_timestamp = now;
  m_hdt = 0.0;
  m_hdt_timeout = now + WATCHDOG_TIMEOUT;
  m_hdm_timeout = now + WATCHDOG_TIMEOUT;
  m_var_timeout = now + WATCHDOG_TIMEOUT;
  m_cog_timeout = now;
  m_cog = 0.;
  m_COGAvg = 0.;
  m_heading_source = HEADING_NONE;
  m_radar_heading = nanl("");
  m_vp_rotation = 0.;
  m_arpa_max_range = BASE_ARPA_DIST;

  // Set default settings before we load config. Prevents random behavior on uninitalized behavior.
  // For instance, LOG_XXX messages before config is loaded.
  m_settings.verbose = 0;
  m_settings.overlay_transparency = DEFAULT_OVERLAY_TRANSPARENCY;
  m_settings.refreshrate = 1;
  m_settings.threshold_blue = 255;
  m_settings.threshold_red = 255;
  m_settings.threshold_green = 255;
  CLEAR_STRUCT(m_settings.radar_interface_address);
  m_settings.radar_count = 0;

  // Get a pointer to the opencpn display canvas, to use as a parent for the UI
  // dialog
  m_parent_window = GetOCPNCanvasWindow();
  m_shareLocn = *GetpSharedDataLocation() + _T("plugins") + wxFileName::GetPathSeparator() + _T("radar_pi") +
                wxFileName::GetPathSeparator() + _T("data") + wxFileName::GetPathSeparator();

  m_pMessageBox = new MessageBox;
  m_pMessageBox->Create(m_parent_window, this);
  LOG_INFO(wxT(PLUGIN_VERSION_WITH_DATE));

  m_locator = 0;

  // Create objects before config, so config can set data in it
  // This does not start any threads or generate any UI.
  for (size_t r = 0; r < RADARS; r++) {
    m_radar[r] = new RadarInfo(this, r);
    m_settings.show_radar[r] = true;
    m_settings.dock_radar[r] = false;
    m_settings.window_pos[r] = wxPoint(30 + 540 * r, 120);
  }

  m_GPS_filter = new GPSKalmanFilter();

  //    And load the configuration items
  if (LoadConfig()) {
    LOG_INFO(wxT("radar_pi: Configuration file values initialised"));
    LOG_INFO(wxT("radar_pi: Log verbosity = %d. To modify, set VerboseLog to sum of:"), m_settings.verbose);
    LOG_INFO(wxT("radar_pi: VERBOSE  = %d"), LOGLEVEL_VERBOSE);
    LOG_INFO(wxT("radar_pi: DIALOG   = %d"), LOGLEVEL_DIALOG);
    LOG_INFO(wxT("radar_pi: TRANSMIT = %d"), LOGLEVEL_TRANSMIT);
    LOG_INFO(wxT("radar_pi: RECEIVE  = %d"), LOGLEVEL_RECEIVE);
    LOG_INFO(wxT("radar_pi: GUARD    = %d"), LOGLEVEL_GUARD);
    LOG_INFO(wxT("radar_pi: ARPA     = %d"), LOGLEVEL_ARPA);
    LOG_VERBOSE(wxT("radar_pi: VERBOSE  log is enabled"));
    LOG_DIALOG(wxT("radar_pi: DIALOG   log is enabled"));
    LOG_TRANSMIT(wxT("radar_pi: TRANSMIT log is enabled"));
    LOG_RECEIVE(wxT("radar_pi: RECEIVE  log is enabled"));
    LOG_GUARD(wxT("radar_pi: GUARD    log is enabled"));
    LOG_ARPA(wxT("radar_pi: ARPA     log is enabled"));
  } else {
    wxLogError(wxT("radar_pi: configuration file values initialisation failed"));
    return 0;  // give up
  }
  //    This PlugIn needs a toolbar icon

  wxString svg_normal = m_shareLocn + wxT("radar_standby.svg");
  wxString svg_rollover = m_shareLocn + wxT("radar_searching.svg");
  wxString svg_toggled = m_shareLocn + wxT("radar_active.svg");
  m_tool_id = InsertPlugInToolSVG(wxT("Radar"), svg_normal, svg_rollover, svg_toggled, wxITEM_NORMAL, wxT("Radar"),
                                  _("Radar plugin with support for multiple radars"), NULL, RADAR_TOOL_POSITION, 0, this);

  // CacheSetToolbarToolBitmaps(BM_ID_RED, BM_ID_BLANK);

  // Now that the settings are made we can initialize the RadarInfos
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    m_radar[r]->Init();
    if ((m_radar[r]->m_radar_type == RT_3G || m_radar[r]->m_radar_type == RT_4GA || m_radar[r]->m_radar_type == RT_HaloA) && m_locator == NULL) {
      m_locator = new NavicoLocate(this);
      if (m_locator->Run() != wxTHREAD_NO_ERROR) {
        wxLogError(wxT("radar_pi: unable to start Navico Radar Locator thread"));
        return 0;
      }
    }
  }
  // and get rid of any radars we're not using
  for (size_t r = M_SETTINGS.radar_count; r < RADARS; r++) {
    delete m_radar[r];
    m_radar[r] = 0;
  }

  for (size_t r = 0; r < MAX_CHART_CANVAS; r++) {
    m_draw_time_overlay_ms[r] = 0;
  }

  m_initialized = true;
  SetRadarWindowViz();
  TimedControlUpdate();

  //    In order to avoid an ASSERT on msw debug builds,
  //    we need to create a dummy menu to act as a surrogate parent of the created MenuItems
  //    The Items will be re-parented when added to the real context meenu

  wxMenu dummy_menu;

  wxMenuItem *mi1 = new wxMenuItem(&dummy_menu, -1, _("Show radar"));
  wxMenuItem *mi2 = new wxMenuItem(&dummy_menu, -1, _("Hide radar"));

  wxMenuItem *mi4 = new wxMenuItem(&dummy_menu, -1, _("Acquire radar target"));
  wxMenuItem *mi5 = new wxMenuItem(&dummy_menu, -1, _("Delete radar target"));
  wxMenuItem *mi6 = new wxMenuItem(&dummy_menu, -1, _("Delete all radar targets"));

#ifdef __WXMSW__
  wxFont *qFont = OCPNGetFont(_("Menu"), 10);
  mi1->SetFont(*qFont);
  mi2->SetFont(*qFont);
  mi4->SetFont(*qFont);
  mi5->SetFont(*qFont);
  mi6->SetFont(*qFont);
#endif

  m_context_menu_show_id = AddCanvasContextMenuItem(mi1, this);
  m_context_menu_hide_id = AddCanvasContextMenuItem(mi2, this);
  m_context_menu_acquire_radar_target = AddCanvasContextMenuItem(mi4, this);
  m_context_menu_delete_radar_target = AddCanvasContextMenuItem(mi5, this);
  m_context_menu_delete_all_radar_targets = AddCanvasContextMenuItem(mi6, this);
  m_context_menu_show = true;
  m_context_menu_arpa = false;
  SetCanvasContextMenuItemViz(m_context_menu_show_id, false);

  LOG_VERBOSE(wxT("radar_pi: Initialized plugin transmit=%d/%d "), m_settings.show_radar[0], m_settings.show_radar[1]);

  m_notify_time_ms = 0;
  m_timer = new wxTimer(this, TIMER_ID);

  return PLUGIN_OPTIONS;
}

/**
 * DeInit() is called when OpenCPN is quitting or when the user disables the plugin.
 *
 * This should get rid of all on-screen objects and deallocate memory.
 */

bool radar_pi::DeInit(void) {
  if (!m_initialized) {
    return false;
  }

  LOG_VERBOSE(wxT("radar_pi: DeInit of plugin"));

  m_initialized = false;

  if (m_timer) {
    m_timer->Stop();
    delete m_timer;
    m_timer = 0;
  }

  if (m_locator) {
    m_locator->Shutdown();
    m_locator->Wait();
  }

  // Stop processing in all radars.
  // This waits for the receive threads to stop and removes the dialog, so that its settings
  // can be saved.
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    m_radar[r]->Shutdown();
  }

  if (m_bogey_dialog) {
    delete m_bogey_dialog;  // This will also save its current pos in m_settings
    m_bogey_dialog = 0;
  }

  SaveConfig();

  RemoveCanvasContextMenuItem(m_context_menu_show_id);
  RemoveCanvasContextMenuItem(m_context_menu_hide_id);
  RemoveCanvasContextMenuItem(m_context_menu_acquire_radar_target);
  RemoveCanvasContextMenuItem(m_context_menu_delete_radar_target);
  RemoveCanvasContextMenuItem(m_context_menu_delete_all_radar_targets);
  LOG_INFO(wxT("radar_pi Context menus removed"));

  // Delete the RadarInfo objects. This will call their destructor and delete all data.
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    delete m_radar[r];
    m_radar[r] = 0;
  }

  if (m_locator != NULL) {
    delete m_locator;
    m_locator = 0;
  }

  delete m_pMessageBox;

  // No need to delete wxWindow stuff, wxWidgets does this for us.
  LOG_VERBOSE(wxT("radar_pi: DeInit of plugin done"));
  return true;
}

int radar_pi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int radar_pi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

int radar_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int radar_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap *radar_pi::GetPlugInBitmap() { return m_pdeficon; }

wxString radar_pi::GetCommonName() { return wxT("Radar"); }

wxString radar_pi::GetShortDescription() { return _("Radar PlugIn"); }

wxString radar_pi::GetLongDescription() {
  return _("Radar PlugIn with support for Garmin and Navico radars") + wxT("\n") + wxT(PLUGIN_VERSION_WITH_DATE);
}

void radar_pi::SetDefaults(void) {
  // This will be called upon enabling a PlugIn via the user Dialog.
  // We don't need to do anything special here.
}

bool radar_pi::EnsureRadarSelectionComplete(bool force) {
  bool any = false;
  size_t r;

  if (!force) {
    for (r = 0; r < M_SETTINGS.radar_count; r++) {
      if (m_radar[r]->m_radar_type != RT_MAX) {
        any = true;
      }
    }
  }
  if (any && !force) {
    return true;
  }

  LOG_DIALOG(wxT("radar_pi: EnsureRadarSelectionComplete not yet so show selection dialog"));
  return MakeRadarSelection();
}

bool radar_pi::MakeRadarSelection() {
  bool ret = false;

  RadarType oldRadarType[RADARS];
  size_t r;

  for (r = 0; r < RADARS; r++) {
    if (m_radar[r]) {
      oldRadarType[r] = m_radar[r]->m_radar_type;
      // LOG_INFO(wxT("OLD radarnr= %i, type = %d"), r, static_cast<int>(m_radar[r]->m_radar_type));
    } else {
      oldRadarType[r] = RT_MAX;
    }
  }

  NetworkAddress null = NetworkAddress(wxT(""));
#define CLEAR_RADAR_INFO                         \
  CLEAR_STRUCT(m_settings.navico_radar_info[r]); \
  m_settings.navico_radar_info[r].serialNr = wxT(" ");

  m_initialized = false;
  SelectDialog dlg(m_parent_window, this);
  if (dlg.ShowModal() == wxID_OK) {
    m_settings.radar_count = 0;
    r = 0;
    for (size_t i = 0; i < RT_MAX; i++) {
      if (dlg.m_selected[i]->GetValue()) {
        if (!m_radar[r]) {
          m_settings.window_pos[r] = wxPoint(100 + 512 * r, 100);
          m_settings.control_pos[r] = wxDefaultPosition;
          CLEAR_RADAR_INFO;
          m_radar[r] = new RadarInfo(this, r);        
        }
        m_radar[r]->m_radar_type = (RadarType)i;
        r++;
        m_settings.radar_count = r;
        ret = true;
      }
    }
   
    for (r = 0; r < M_SETTINGS.radar_count; r++) {
      if (m_radar[r] && m_radar[r]->m_radar_type != oldRadarType[r]) {
        m_radar[r]->Shutdown();
        RemoveCanvasContextMenuItem(m_context_menu_control_id[r]);
        delete m_radar[r];
        CLEAR_RADAR_INFO;
        m_radar[r] = new RadarInfo(this, r);
      }
    }

    // And now set the radar types for the selected radars
    r = 0;
    for (size_t i = 0; i < RT_MAX; i++) {
      if (dlg.m_selected[i]->GetValue()) {
        m_radar[r]->m_radar_type = (RadarType)i;
        r++;
        m_settings.radar_count = r;
      }
    }

    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      m_radar[r]->Init();
    }
    for (size_t r = M_SETTINGS.radar_count; r < RADARS; r++) {
      if (m_radar[r]) {
        m_radar[r]->Shutdown();
        CLEAR_RADAR_INFO;
        delete m_radar[r];
        m_radar[r] = 0;
      }
    }

    SetRadarWindowViz();
    TimedControlUpdate();
  }
  m_initialized = true;
  SaveConfig();
  return ret;
}

void radar_pi::ShowPreferencesDialog(wxWindow *parent) {
  LOG_DIALOG(wxT("radar_pi: ShowPreferencesDialog"));

  bool oldShow = M_SETTINGS.show;
  M_SETTINGS.show = 0;
  M_SETTINGS.reset_radars = false;
  NotifyRadarWindowViz();

  if (EnsureRadarSelectionComplete(false)) {
    OptionsDialog dlg(parent, m_settings, m_radar[0]->m_radar_type);
    if (dlg.ShowModal() == wxID_OK) {
      m_settings = dlg.GetSettings();
      if (EnsureRadarSelectionComplete(m_settings.reset_radars)) {
        M_SETTINGS.reset_radars = false;
      }
      if (!m_guard_bogey_confirmed && m_alarm_sound_timeout && m_settings.guard_zone_timeout) {
        m_alarm_sound_timeout = time(0) + m_settings.guard_zone_timeout;
      }
    }
  }

  M_SETTINGS.show = oldShow;
  NotifyRadarWindowViz();
}

// A different thread (or even the control dialog itself) has changed state and now
// the radar window and control visibility needs to be reset. It can't call SetRadarWindowViz()
// directly so we redirect via flag and main thread.
void radar_pi::NotifyRadarWindowViz() { m_notify_radar_window_viz = true; }

// A different thread (or even the control dialog itself) has changed state and now
// the content of the control dialog needs to be reset (but not its visibility, that is what
// NotifyRadarWindowViz() does.)
//
void radar_pi::NotifyControlDialog() { m_notify_control_dialog = true; }

void radar_pi::SetRadarWindowViz(bool reparent) {
  for (size_t r = 0; r < m_settings.radar_count; r++) {
    bool showThisRadar = m_settings.show && m_settings.show_radar[r];
    bool showThisControl = m_settings.show && m_settings.show_radar_control[r];
    LOG_DIALOG(wxT("radar_pi: RadarWindow[%d] show=%d showcontrol=%d"), r, showThisRadar, showThisControl);
    m_radar[r]->ShowRadarWindow(showThisRadar);

    m_radar[r]->ShowControlDialog(showThisControl, reparent);
    m_radar[r]->UpdateTransmitState();
  }
}

/**
 * OpenCPN is about to show a context menu.
 *
 * Adjust our context menu items so that they are correct for the given canvas.
 *
 * @param canvasIndex      Canvas #
 */
void radar_pi::PrepareContextMenu(int canvasIndex) {
  int arpa_targets = GetArpaTargetCount();
  bool targets_tracked = arpa_targets > 0;
  bool show = m_settings.show;
  bool enableShowRadarControl = false;
  bool arpa = arpa_targets == 0;
  bool overlay = m_settings.show                                                                 // radar shown
                 && m_chart_overlay[canvasIndex] >= 0                                            // overlay desired
                 && m_radar[m_chart_overlay[canvasIndex]]->m_state.GetValue() == RADAR_TRANSMIT  // Radar  transmitting
                 && !isnan(m_cursor_pos.lat) && !isnan(m_cursor_pos.lon);                        // position available

  bool show_acq_delete = overlay && targets_tracked;

  LOG_DIALOG(wxT("radar_pi: PrepareContextMenu for canvas %d radar %d"), canvasIndex, m_chart_overlay[canvasIndex]);
  LOG_DIALOG(wxT("radar_pi: arpa=%d show=%d enableShowRadarControl=%d"), arpa, show, enableShowRadarControl);

  // SetCanvasContextMenuItemGrey(m_context_menu_delete_radar_target, arpa);
  // SetCanvasContextMenuItemGrey(m_context_menu_delete_all_radar_targets, arpa);
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (m_settings.show_radar_control[r] == 0) {
      // SetCanvasContextMenuItemGrey(m_context_menu_control_id[r], enableShowRadarControl);
      SetCanvasContextMenuItemViz(m_context_menu_control_id[r], show);
    } else {
      SetCanvasContextMenuItemViz(m_context_menu_control_id[r], false);
    }
  }
  SetCanvasContextMenuItemViz(m_context_menu_show_id, !show);
  SetCanvasContextMenuItemViz(m_context_menu_hide_id, show);
  SetCanvasContextMenuItemViz(m_context_menu_acquire_radar_target, overlay);
  SetCanvasContextMenuItemViz(m_context_menu_delete_radar_target, show_acq_delete);
  SetCanvasContextMenuItemViz(m_context_menu_delete_all_radar_targets, targets_tracked);
}

int radar_pi::GetArpaTargetCount(void) {
  int arpa_targets = 0;

  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (m_radar[r]->m_arpa) arpa_targets += m_radar[r]->m_arpa->GetTargetCount();
  }
  return arpa_targets;
}

//********************************************************************************
// Operation Dialogs - Control, Manual, and Options

void radar_pi::ShowRadarControl(int radar, bool show, bool reparent) {
  LOG_DIALOG(wxT("radar_pi: ShowRadarControl(%d, %d)"), radar, (int)show);
  m_settings.show_radar_control[radar] = show;
  m_radar[radar]->ShowControlDialog(show, reparent);
}

void radar_pi::OnControlDialogClose(RadarInfo *ri) {
  if (ri->m_control_dialog) {
    m_settings.control_pos[ri->m_radar] = ri->m_control_dialog->GetPosition();
  }
  m_settings.show_radar_control[ri->m_radar] = false;
  if (ri->m_control_dialog) {
    ri->m_control_dialog->HideDialog();
  }
}

void radar_pi::ConfirmGuardZoneBogeys() {
  m_guard_bogey_confirmed = true;  // This will stop the sound being repeated
}

//*******************************************************************************
// ToolBar Actions

int radar_pi::GetToolbarToolCount(void) { return 1; }

/*
 * The radar icon is clicked. In previous versions all sorts of behavior was linked to clicking on the button, which wasn't very
 * 'discoverable' -- hard to find out what your options are.
 * In this version there are two behaviors.
 * - If the radar windows are not shown:
 *    - If the radar overlay is active and the radar control dialog is not shown, we show that control dialog only.
 *    - Else show the radar windows.
 * - Else close all radar windows and control dialogs.
 *
 * that way all state decisions are visual, without extra timers.
 *
 */
void radar_pi::OnToolbarToolCallback(int id) {
  if (!m_initialized) {
    return;
  }
  if (!EnsureRadarSelectionComplete(false)) {
    return;
  }

  LOG_DIALOG(wxT("radar_pi: OnToolbarToolCallback"));

  if (m_pMessageBox->UpdateMessage(false)) {
    // Conditions for radar not satisfied, hide radar windows
    m_settings.show = 0;
    LOG_DIALOG(wxT("radar_pi: OnToolbarToolCallback set show 0"));
    SetRadarWindowViz();
    return;
  }

  if (m_settings.show) {
    LOG_DIALOG(wxT("radar_pi: OnToolbarToolCallback show"));
    // Show the control dialogs of all overlay radars
    for (int i = 0; i < CANVAS_COUNT; i++) {
      if (m_chart_overlay[i] > -1 &&
          (!m_radar[m_chart_overlay[i]]->m_control_dialog || !m_radar[m_chart_overlay[i]]->m_control_dialog->IsShown())) {
        LOG_DIALOG(wxT("radar_pi: OnToolbarToolCallback: Show control canvas %d"), i);
        ShowRadarControl(m_chart_overlay[i], true);
      }
    }
  }

  if (m_settings.show) {
    LOG_DIALOG(wxT("radar_pi: OnToolbarToolCallback: Hide radar windows"));
    m_settings.show = 0;
    SetRadarWindowViz();
  } else {
    LOG_DIALOG(wxT("radar_pi: OnToolbarToolCallback: Show radar windows"));
    m_settings.show = 1;
    SetRadarWindowViz();
  }
  UpdateState();
}

void radar_pi::OnContextMenuItemCallback(int id) {
  if (!EnsureRadarSelectionComplete(false)) {
    return;
  }
  int current_canvas_index = -1;
  int current_radar = -1;
  // find out which canvas the click is on
  m_context_menu_canvas_index = GetCanvasIndexUnderMouse();
  if (m_context_menu_canvas_index > -1 && m_context_menu_canvas_index < CANVAS_COUNT) {
    current_canvas_index = m_context_menu_canvas_index;
    current_radar = m_chart_overlay[m_context_menu_canvas_index];
  }

  if (id == m_context_menu_hide_id) {
    m_settings.show = false;
    SetRadarWindowViz();
  } else if (id == m_context_menu_show_id) {
    m_settings.show = true;
    SetRadarWindowViz();
  } else if (id == m_context_menu_acquire_radar_target) {
    if (m_settings.show                                                  // radar shown
        && HaveOverlay()                                                 // overlay desired
        && m_radar[current_radar]->m_state.GetValue() == RADAR_TRANSMIT  // Radar  transmitting
        && !isnan(m_right_click_pos.lat) && !isnan(m_right_click_pos.lon)) {
      if (m_right_click_pos.lat < 90. && m_right_click_pos.lat > -90. && m_right_click_pos.lon < 180. &&
          m_right_click_pos.lon > -180.) {
        ExtendedPosition target_pos;
        target_pos.pos = m_right_click_pos;
        m_radar[current_radar]->m_arpa->AcquireNewMARPATarget(target_pos);
      } else {
        LOG_INFO(wxT(" **error right click pos lat=%f, lon=%f"), m_right_click_pos.lat, m_right_click_pos.lon);
      }
    }
  } else if (id == m_context_menu_delete_radar_target) {
    // Targets can also be deleted when the overlay is not shown
    // In this case targets can be made by a guard zone in a radarwindow
    if (m_settings.show && current_radar >= 0) {
      ExtendedPosition target_pos;
      target_pos.pos = m_right_click_pos;
      if (m_radar[current_radar]->m_arpa) {
        m_radar[current_radar]->m_arpa->DeleteTarget(target_pos);
      }
    }
  } else if (id == m_context_menu_delete_all_radar_targets) {
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      if (m_radar[r]->m_arpa) {
        m_radar[r]->m_arpa->DeleteAllTargets();
      }
    }
  } else {
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      if (id == m_context_menu_control_id[r]) {
        LOG_DIALOG(wxT("radar_pi: OnToolbarToolCallback: show controls for radar %i"), r);
        if (m_settings.show_radar_control[r] == 0) {
          ShowRadarControl(r, true);
        }
      }
    }
  }
}

void radar_pi::PassHeadingToOpenCPN() {
  wxString nmea;
  char sentence[40];
  char checksum = 0;
  char *p;

  snprintf(sentence, sizeof(sentence), "RAHDT,%.1f,T", m_hdt);

  for (p = sentence; *p; p++) {
    checksum ^= *p;
  }

  nmea.Printf(wxT("$%s*%02X\r\n"), sentence, (unsigned)checksum);
  LOG_TRANSMIT(wxT("radar_pi: Passing heading '%s'"), nmea.c_str());
  PushNMEABuffer(nmea);
}

/**
 * Check any guard zones
 *
 */
void radar_pi::CheckGuardZoneBogeys(void) {
  bool bogeys_found = false;
  time_t now = time(0);
  wxString text;

  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (M_SETTINGS.radar_count > 1) {
      text << m_radar[r]->m_name;
      text << wxT(":\n");
    }
    if (m_radar[r]->m_state.GetValue() == RADAR_TRANSMIT) {
      bool bogeys_found_this_radar = false;

      for (size_t z = 0; z < GUARD_ZONES; z++) {
        int bogeys = m_radar[r]->m_guard_zone[z]->GetBogeyCount();
        if (bogeys > m_settings.guard_zone_threshold) {
          bogeys_found = true;
          bogeys_found_this_radar = true;
        }
        text << _(" Zone") << wxT(" ") << z + 1 << wxT(": ");
        if (bogeys > m_settings.guard_zone_threshold) {
          text << bogeys;
        } else if (bogeys >= 0) {
          text << wxT("(");
          text << bogeys;
          text << wxT(")");
        } else {
          text << wxT("-");
        }
        text << wxT("\n");
      }
      LOG_GUARD(wxT("radar_pi: Radar %c: CheckGuardZoneBogeys found=%d confirmed=%d"), r + 'A', bogeys_found_this_radar,
                m_guard_bogey_confirmed);
    }
  }

  if (bogeys_found) {
    if (!m_guard_bogey_confirmed && TIMED_OUT(now, m_alarm_sound_timeout) && m_settings.guard_zone_timeout) {
      // If the last time is 10 seconds ago we ping a sound, unless the user
      // confirmed
      m_alarm_sound_timeout = now + m_settings.guard_zone_timeout;

      if (!m_settings.alert_audio_file.IsEmpty()) {
        PlugInPlaySound(m_settings.alert_audio_file);
      } else {
        wxBell();
      }
    }
    m_guard_bogey_seen = true;
    m_guard_bogey_timeout = 0;
  } else if (m_guard_bogey_seen) {  // First time here after bogey disappeared. Start timer.
    m_guard_bogey_timeout = now + CONFIRM_RESET_TIMEOUT;
    m_guard_bogey_seen = false;
    m_alarm_sound_timeout = 0;
  } else if (TIMED_OUT(now, m_guard_bogey_timeout)) {  // No bogeys and timer elapsed, now reset confirmed
    m_guard_bogey_confirmed = false;                   // Reset for next time we see bogeys
    m_alarm_sound_timeout = 0;
  }

  if (!m_guard_bogey_confirmed && m_alarm_sound_timeout > 0) {
    if (text.length() > 0) {
      text << wxT("\n");
    }
    text << _("Next alarm in");
    text << wxString::Format(wxT(" %d s"), m_alarm_sound_timeout - now);
  }

  if (bogeys_found && !m_bogey_dialog) {
    m_bogey_dialog = new GuardZoneBogey;
    m_bogey_dialog->Create(m_parent_window, this);
  }
  if (m_bogey_dialog) {
    m_bogey_dialog->ShowBogeys(text, bogeys_found, m_guard_bogey_confirmed);
  }
}

void radar_pi::SetRadarHeading(double heading, bool isTrue) {
  wxCriticalSectionLocker lock(m_exclusive);
  m_radar_heading = heading;
  m_radar_heading_true = isTrue;
  time_t now = time(0);
  if (!wxIsNaN(m_radar_heading)) {
    if (m_radar_heading_true) {
      if (m_heading_source != HEADING_RADAR_HDT) {
        m_heading_source = HEADING_RADAR_HDT;
      }
      if (m_heading_source == HEADING_RADAR_HDT) {
        m_hdt = m_radar_heading;
        m_hdt_timeout = now + HEADING_TIMEOUT;
      }
    } else {
      if (m_heading_source != HEADING_RADAR_HDM) {
        m_heading_source = HEADING_RADAR_HDM;
      }
      if (m_heading_source == HEADING_RADAR_HDM) {
        m_hdm = m_radar_heading;
        m_hdt = m_radar_heading + m_var;
        m_hdm_timeout = now + HEADING_TIMEOUT;
      }
    }
  } else if (m_heading_source == HEADING_RADAR_HDM || m_heading_source == HEADING_RADAR_HDT) {
    // no heading on radar and heading source is still radar
    m_heading_source = HEADING_NONE;
  }
}

void radar_pi::UpdateHeadingPositionState() {
  {
    wxCriticalSectionLocker lock(m_exclusive);
    time_t now = time(0);

    if (m_bpos_set && TIMED_OUT(now, m_bpos_timestamp + WATCHDOG_TIMEOUT)) {
      // If the position data is 10s old reset our position.
      // Note that the watchdog is reset every time we receive a position.
      m_bpos_set = false;
      m_predicted_position_initialised = false;
      LOG_VERBOSE(wxT("radar_pi: Lost Boat Position data"));
    }

    switch (m_heading_source) {
      case HEADING_NONE:
        break;
      case HEADING_FIX_COG:
      case HEADING_FIX_HDT:
      case HEADING_NMEA_HDT:
      case HEADING_RADAR_HDT:
        if (TIMED_OUT(now, m_hdt_timeout)) {
          // If the position data is 10s old reset our heading.
          // Note that the watchdog is reset every time we receive a heading.
          m_heading_source = HEADING_NONE;
          LOG_VERBOSE(wxT("radar_pi: Lost Heading data"));
        }
        break;
      case HEADING_FIX_HDM:
      case HEADING_NMEA_HDM:
      case HEADING_RADAR_HDM:
        if (TIMED_OUT(now, m_hdm_timeout)) {
          // If the position data is 10s old reset our heading.
          // Note that the watchdog is continuously reset every time we receive a
          // heading
          m_heading_source = HEADING_NONE;
          LOG_VERBOSE(wxT("radar_pi: Lost Heading data"));
        }
        break;
    }

    if (m_var_source != VARIATION_SOURCE_NONE && TIMED_OUT(now, m_var_timeout)) {
      m_var_source = VARIATION_SOURCE_NONE;
      LOG_VERBOSE(wxT("radar_pi: Lost Variation source"));
    }
  }
}

/**
 * This is called whenever OpenCPN is drawing the chart, about halfway through its
 * process, e.g. as the last part of RenderGLOverlay(), and by the timer.
 *
 * This happens on the main (GUI) thread.
 */
void radar_pi::ScheduleWindowRefresh() {
  int drawTime = 0;
  int millis;
  int renderPPI[RADARS];
  int render_overlay[MAX_CHART_CANVAS];
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    m_radar[r]->RefreshDisplay();
    drawTime += m_radar[r]->GetDrawTime();
    renderPPI[r] = m_radar[r]->GetDrawTime();
  }
  int max_canvas = GetCanvasCount();
  for (int r = 0; r < max_canvas; r++) {
    drawTime += m_draw_time_overlay_ms[r];
    render_overlay[r] = m_draw_time_overlay_ms[r];
  }
  int refreshrate = m_settings.refreshrate.GetValue();

  if (refreshrate > 1 && drawTime < 500) {
    // 1 = 1 per s, 1000ms between draws, no additional refreshes
    // 2 = 2 per s,  500ms
    // 3 = 4 per s,  250ms
    // 4 = 8 per s,  125ms
    // 5 = 16 per s,  64ms
    millis = (1000 - drawTime) / (1 << (refreshrate - 1)) + drawTime;

    LOG_VERBOSE(wxT("radar_pi: rendering took %i ms, PPI0= %i ms, PPI1= %i, Overlay0= %i, Overlay1= %i next render in %i ms"),
                drawTime, renderPPI[0], renderPPI[1], render_overlay[0], render_overlay[1], millis);

    m_timer->StartOnce(millis);

  } else {
    LOG_VERBOSE(wxT("radar_pi: rendering took %dms, refreshrate=%d, no next extra render"), drawTime, refreshrate);
  }
}

void radar_pi::OnTimerNotify(wxTimerEvent &event) {
  if (!EnsureRadarSelectionComplete(false)) {
    return;
  }

  if (m_settings.show) {  // Is radar enabled?
    bool ppi_visible = false;

    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      if (m_settings.show_radar[r]) {
        ppi_visible = true;
      }
    }
    // always refresh canvas0 if radar window is visible
    // and refresh canvas with overlay
    for (int r = 0; r < CANVAS_COUNT; r++) {
      wxWindow *canvas = GetCanvasByIndex(r);
      if (m_chart_overlay[r] >= 0 || (r == 0 && ppi_visible)) {
        if (canvas) {
          canvas->Refresh(false);
        } else {
          LOG_INFO(wxT("**error canvas NOT OK, r=%i"), r);
        }
      }
    }
  }
}

// Called between 1 and 10 times per second by RenderGLOverlay call
void radar_pi::TimedControlUpdate() {
  wxLongLong now = wxGetUTCTimeMillis();
  if (!m_notify_control_dialog && !TIMED_OUT(now, m_notify_time_ms + 500)) {
    return;  // Don't run this more often than 2 times per second
  }
  // following is to prevent crash in RadarPanel::ShowFrame on m_aui_mgr->Update() line 222,
  if (m_max_canvas <= 0 || (m_max_canvas > 1 && m_current_canvas_index == 0)) {
    return;
  }

  //// for overlay testing only, simple trick to get position and heading
  // wxString nmea;
  // nmea = wxT("$APHDM,000.0,M*33<0x0D><0x0A>");
  // PushNMEABuffer(nmea);
  // nmea = wxT("$GPRMC,123519,A,5326.038,N,00611.000,E,022.4,,230394,,W,*41<0x0D><0x0A>");
  // PushNMEABuffer(nmea);

  m_notify_time_ms = now;

  bool updateAllControls = m_notify_control_dialog;
  m_notify_control_dialog = false;
  if (m_opengl_mode_changed || m_notify_radar_window_viz) {
    m_opengl_mode_changed = false;
    m_notify_radar_window_viz = false;
    SetRadarWindowViz(true);
    updateAllControls = true;
  }
  UpdateHeadingPositionState();

  // Check the age of "radar_seen", if too old radar_seen = false
  bool any_data_seen = false;
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    int state = m_radar[r]->m_state.GetValue();  // Safe, protected by lock
    if (state == RADAR_TRANSMIT) {
      any_data_seen = true;
    }
    if (!m_settings.show            // No radar shown
        || state != RADAR_TRANSMIT  // Radar not transmitting
        || !m_bpos_set) {           // No overlay possible (yet)
                                    // Conditions for ARPA not fulfilled, delete all targets
      m_radar[r]->m_arpa->RadarLost();
    }
    m_radar[r]->UpdateTransmitState();
  }

  if (any_data_seen && m_settings.show) {
    CheckGuardZoneBogeys();
  }

  if (m_settings.pass_heading_to_opencpn && m_heading_source >= HEADING_RADAR_HDM) {
    PassHeadingToOpenCPN();
  }

  if (m_pMessageBox->IsShown() || (m_settings.verbose != 0)) {
    wxString t;
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      if (m_radar[r]->m_state.GetValue() != RADAR_OFF) {
        wxCriticalSectionLocker lock(m_radar[r]->m_exclusive);

        t << wxString::Format(wxT("%s\npackets %d/%d\nspokes %d/%d/%d\n"), m_radar[r]->m_name.c_str(),
                              m_radar[r]->m_statistics.packets, m_radar[r]->m_statistics.broken_packets,
                              m_radar[r]->m_statistics.spokes, m_radar[r]->m_statistics.broken_spokes,
                              m_radar[r]->m_statistics.missing_spokes);
      }
    }
    m_pMessageBox->SetStatisticsInfo(t);

    IF_LOG_AT_LEVEL(LOGLEVEL_RECEIVE) {
      if (t.length() > 0) {
        t.Replace(wxT("\n"), wxT(" "));
        LOG_RECEIVE(wxT("radar_pi: %s"), t.c_str());
      }
    }
  }

  // Always reset the counters, so they don't show huge numbers after IsShown changes
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    wxCriticalSectionLocker lock(m_radar[r]->m_exclusive);

    m_radar[r]->m_statistics.broken_packets = 0;
    m_radar[r]->m_statistics.broken_spokes = 0;
    m_radar[r]->m_statistics.missing_spokes = 0;
    m_radar[r]->m_statistics.packets = 0;
    m_radar[r]->m_statistics.spokes = 0;
  }

  wxString info;
  switch (m_heading_source) {
    case HEADING_NONE:
    case HEADING_FIX_HDM:
    case HEADING_NMEA_HDM:
    case HEADING_RADAR_HDM:
      info = wxT("");
      break;
    case HEADING_FIX_COG:
      info = _("COG");
      break;
    case HEADING_FIX_HDT:
    case HEADING_NMEA_HDT:
      info = _("HDT");
      break;
    case HEADING_RADAR_HDT:
      info = _("RADAR");
      break;
  }
  if (info.Len() > 0 && !wxIsNaN(m_hdt)) {
    info << wxString::Format(wxT(" %3.1f"), m_hdt);
  }
  m_pMessageBox->SetTrueHeadingInfo(info);
  switch (m_heading_source) {
    case HEADING_NONE:
    case HEADING_FIX_COG:
    case HEADING_FIX_HDT:
    case HEADING_NMEA_HDT:
    case HEADING_RADAR_HDT:
      info = wxT("");
      break;
    case HEADING_FIX_HDM:
    case HEADING_NMEA_HDM:
      info = _("HDM");
      break;
    case HEADING_RADAR_HDM:
      info = _("RADAR");
      break;
  }
  if (info.Len() > 0 && !wxIsNaN(m_hdm)) {
    info << wxString::Format(wxT(" %3.1f"), m_hdm);
  }
  m_pMessageBox->SetMagHeadingInfo(info);
  m_pMessageBox->UpdateMessage(false);

  UpdateAllControlStates(updateAllControls);
  UpdateState();
}

void radar_pi::UpdateAllControlStates(bool all) {
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    m_radar[r]->UpdateControlState(all);
  }
}

void radar_pi::UpdateState(void) {
  if (m_settings.show == false) {
    m_toolbar_button = TB_HIDDEN;
  } else {
    RadarState state = RADAR_OFF;
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      state = wxMax(state, (RadarState)m_radar[r]->m_state.GetValue());
    }
    m_toolbar_button = g_toolbarIconColor[state];
  }
  CacheSetToolbarToolBitmaps();

  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    m_radar[r]->CheckTimedTransmit();
  }
}

void radar_pi::SetOpenGLMode(OpenGLMode mode) {
  if (m_opengl_mode != mode) {
    m_opengl_mode = mode;
    // Can't hide/show the windows from here, this becomes recursive because the Chart display
    // is managed by wxAuiManager as well.
    m_opengl_mode_changed = true;
  }
}

wxGLContext *radar_pi::GetChartOpenGLContext() { return m_opencpn_gl_context; }

//**************************************************************************************************
// Radar Image Graphic Display Processes
//**************************************************************************************************

bool radar_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp) {
  if (!m_initialized) {
    return true;
  }

  LOG_DIALOG(wxT("radar_pi: RenderOverlay"));

  SetOpenGLMode(OPENGL_OFF);
  return true;
}

// Called by Plugin Manager on main system process cycle

bool radar_pi::RenderGLOverlayMultiCanvas(wxGLContext *pcontext, PlugIn_ViewPort *vp, int canvasIndex) {
  GeoPosition radar_pos;
  // prevent this being called recursively
  // no critical section locker (will wait), better to return immediately
  if (m_render_busy) {
    LOG_INFO(wxT("error render busy"));
    return true;
  }
  m_render_busy = true;

  // update own ship position to best estimate
  ExtendedPosition intermediate_pos;
  if (m_predicted_position_initialised) {
    m_GPS_filter->Predict(&m_last_fixed, &m_expected_position);
  }
  m_ownship = m_expected_position.pos;
  // Update radar position offset from GPS
  if (m_heading_source != HEADING_NONE && !wxIsNaN(m_hdt)) {
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      m_radar[r]->SetRadarPosition(m_ownship, m_hdt);
    }
  }

  wxLongLong now = wxGetUTCTimeMillis();
  // Update m_overlay[canvasIndex] by checking all radars, value may be modified by the buttons
  m_chart_overlay[canvasIndex] = -1;
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (m_radar[r]->m_overlay_canvas[canvasIndex].GetValue() != 0) {
      m_chart_overlay[canvasIndex] = r;
    }
  }
  m_current_canvas_index = canvasIndex;
  int current_overlay_radar = m_chart_overlay[canvasIndex];
  m_max_canvas = GetCanvasCount();
  if (m_max_canvas <= 0 || m_current_canvas_index >= m_max_canvas) {
    m_render_busy = false;
    return true;
  }

  if (!m_initialized) {
    m_render_busy = false;
    return true;
  }

  // refresh ARPA targets only with canvas 0
  if (canvasIndex == 0) {
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      bool arpa_on = false;
      if (m_radar[r]->m_arpa) {
        for (int i = 0; i < GUARD_ZONES; i++) {
          if (m_radar[r]->m_guard_zone[i]->m_arpa_on) {
            arpa_on = true;
          }
        }
        if (m_radar[r]->m_arpa->GetTargetCount() > 0) {
          arpa_on = true;
        }
      }
      if (arpa_on) {
        m_radar[r]->m_arpa->RefreshArpaTargets();
      }
    }
  }

  m_vp = vp;

  LOG_DIALOG(wxT("radar_pi: RenderGLOverlayMultiCanvas context=%p canvas=%d"), pcontext, canvasIndex);
  m_opencpn_gl_context = pcontext;
  if (!m_opencpn_gl_context && !m_opencpn_gl_context_broken) {
    LOG_INFO(wxT("radar_pi: OpenCPN does not pass OpenGL context. Resize of OpenCPN window may be broken!"));
  }
  m_opencpn_gl_context_broken = m_opencpn_gl_context == 0;

  SetOpenGLMode(OPENGL_ON);

  if (vp->rotation != m_vp_rotation) {
    wxCriticalSectionLocker lock(m_exclusive);

    m_cog_timeout = time(0) + m_COGAvgSec;
    m_cog = m_COGAvg;
    m_vp_rotation = vp->rotation;
  }

  if (M_SETTINGS.show                                                     // Radar shown
      && current_overlay_radar > -1                                       // Overlay desired
      && current_overlay_radar < (int)M_SETTINGS.radar_count              // and still valid
      && m_radar[current_overlay_radar]->GetRadarPosition(&radar_pos)) {  // Boat position known

    GeoPosition pos_min = {vp->lat_min, vp->lon_min};
    GeoPosition pos_max = {vp->lat_max, vp->lon_max};
    double max_distance = radar_distance(pos_min, pos_max, 'm');
    // max_distance is the length of the diagonal of the viewport. If the boat
    // were centered, the max length to the edge of the screen is exactly half that.
    double edge_distance = max_distance / 2.0;
    int auto_range_meters = (int)edge_distance;
    if (auto_range_meters < 50) {
      auto_range_meters = 50;
    }

    wxPoint boat_center;
    GetCanvasPixLL(vp, &boat_center, radar_pos.lat, radar_pos.lon);

    // if this radar is overlayed on multiple canvases only adjust auto range on one of them.
    // we choose the highest canvas, which is just an arbitrary choice by us.
    int highest = -1;
    for (int i = 0; i < CANVAS_COUNT; i++) {
      if (m_chart_overlay[i] == current_overlay_radar) {
        highest = i;
      }
    }

    if (canvasIndex == highest) {
      m_radar[current_overlay_radar]->SetAutoRangeMeters(auto_range_meters);
    }

    //    Calculate image scale factor
    double dist_y, v_scale_ppm;
    GetCanvasLLPix(vp, wxPoint(0, vp->pix_height - 1), &pos_max.lat, &pos_max.lon);  // is pix_height a mapable coordinate?
    GetCanvasLLPix(vp, wxPoint(0, 0), &pos_min.lat, &pos_min.lon);
    dist_y = radar_distance(pos_min, pos_max, 'm');  // Distance of height of display - meters
    v_scale_ppm = 1.0;
    if (dist_y > 0.) {
      // v_scale_ppm = vertical pixels per meter
      v_scale_ppm = vp->pix_height / dist_y;  // pixel height of screen div by equivalent meters
    }
    double rotation = fmod(rad2deg(vp->rotation + vp->skew * m_settings.skew_factor) + 720.0, 360);
    LOG_DIALOG(wxT("radar_pi: RenderRadarOverlay lat=%g lon=%g v_scale_ppm=%g vp_rotation=%g skew=%g scale=%f rot=%g"), vp->clat,
               vp->clon, vp->view_scale_ppm, vp->rotation, vp->skew, v_scale_ppm, rotation);
    m_radar[current_overlay_radar]->RenderRadarImage1(boat_center, v_scale_ppm, rotation, true);
  }

  m_draw_time_overlay_ms[canvasIndex] = (wxGetUTCTimeMillis() - now).GetLo();

  if (canvasIndex == 0) {
    ScheduleWindowRefresh();
  }
  TimedControlUpdate();
  m_render_busy = false;
  return true;
}

//****************************************************************************

bool radar_pi::LoadConfig(void) {
  wxFileConfig *pConf = m_pconfig;
  int v, x, y, state;
  wxString s;

  if (pConf) {
    pConf->SetPath(wxT("Settings"));
    pConf->Read(wxT("COGUPAvgSeconds"), &m_COGAvgSec, 15);
    m_COGAvgSec = wxMin(m_COGAvgSec, MAX_COG_AVERAGE_SECONDS);  // Bound the array size
    for (int i = 0; i < m_COGAvgSec; i++) {
      m_COGTable[i] = NAN;
    }

    pConf->SetPath(wxT("/Plugins/Radar"));

    // Valgrind: This needs to be set before we set range, since that uses this
    pConf->Read(wxT("RangeUnits"), &v, RANGE_NAUTIC);
    m_settings.range_units = (RangeUnits)wxMax(wxMin(v, 2), 0);

    pConf->Read(wxT("VerboseLog"), &m_settings.verbose, 0);

    pConf->Read(wxT("RadarCount"), &v, 0);
    M_SETTINGS.radar_count = v;

    pConf->Read(wxT("DockSize"), &v, 0);
    m_settings.dock_size = v;

    size_t n = 0;
    for (int r = 0; r < (int)M_SETTINGS.radar_count; r++) {
      RadarInfo *ri = m_radar[n];
      pConf->Read(wxString::Format(wxT("Radar%dType"), r), &s, "unknown");
      ri->m_radar_type = RT_MAX;  // = not used
      for (int i = 0; i < RT_MAX; i++) {
        if (s.IsSameAs(RadarTypeName[i])) {
          ri->m_radar_type = (RadarType)i;
          break;
        }
      }
      if (ri->m_radar_type == RT_MAX) {
        continue;  // This happens if someone changed the name in the config file or
                   // we drop support for a type or rename it.
      }

      pConf->Read(wxString::Format(wxT("Radar%dInterface"), r), &s, "0.0.0.0");
      radar_inet_aton(s.c_str(), &m_settings.radar_interface_address[n].addr);
      m_settings.radar_interface_address[n].port = 0;
      pConf->Read(wxString::Format(wxT("Radar%dAddress"), r), &s, "0.0.0.0");
      radar_inet_aton(s.c_str(), &m_settings.radar_address[n].addr);
      m_settings.radar_address[n].port = htons(RadarOrder[ri->m_radar_type]);
      pConf->Read(wxString::Format(wxT("Radar%dNavicoInfo"), r), &s, "");
      m_settings.navico_radar_info[r] = NavicoRadarInfo(s);

      pConf->Read(wxString::Format(wxT("Radar%dRange"), r), &v, 2000);
      ri->m_range.Update(v);
      pConf->Read(wxString::Format(wxT("Radar%dRotation"), r), &v, 0);
      if (v == ORIENTATION_HEAD_UP) {
        v = ORIENTATION_STABILIZED_UP;
      }
      ri->m_orientation.Update(v);

      pConf->Read(wxString::Format(wxT("Radar%dTransmit"), r), &v, 0);
      ri->m_boot_state.Update(v);
      pConf->Read(wxString::Format(wxT("Radar%dMinContourLength"), r), &ri->m_min_contour_length, 6);
      if (ri->m_min_contour_length > 10) ri->m_min_contour_length = 6;  // Prevent user and system error

      RadarControlItem item;
      pConf->Read(wxString::Format(wxT("Radar%dTrailsState"), r), &state, RCS_OFF);
      pConf->Read(wxString::Format(wxT("Radar%dTrails"), r), &v, 0);
      m_radar[r]->m_target_trails.Update(v, (RadarControlState)state);
      pConf->Read(wxString::Format(wxT("Radar%dTrueTrailsMotion"), r), &v, 1);
      m_radar[r]->m_trails_motion.Update(v);
      pConf->Read(wxString::Format(wxT("Radar%dMainBangSize"), r), &v, 0);
      m_radar[r]->m_main_bang_size.Update(v);
      pConf->Read(wxString::Format(wxT("Radar%dAntennaForward"), r), &v, 0);
      m_radar[r]->m_antenna_forward.Update(v);
      pConf->Read(wxString::Format(wxT("Radar%dAntennaStarboard"), r), &v, 0);
      m_radar[r]->m_antenna_starboard.Update(v);
      pConf->Read(wxString::Format(wxT("Radar%dRunTimeOnIdle"), r), &v, 1);
      m_radar[r]->m_timed_run.Update(v);

      for (int i = 0; i < MAX_CHART_CANVAS; i++) {
        pConf->Read(wxString::Format(wxT("Radar%dOverlayCanvas%d"), r, i), &v, 0);
        m_radar[r]->m_overlay_canvas[i].Update(v);
      }

      pConf->Read(wxString::Format(wxT("Radar%dWindowShow"), r), &m_settings.show_radar[n], true);
      pConf->Read(wxString::Format(wxT("Radar%dWindowDock"), r), &m_settings.dock_radar[n], false);
      pConf->Read(wxString::Format(wxT("Radar%dWindowPosX"), r), &x, 30 + 540 * n);
      pConf->Read(wxString::Format(wxT("Radar%dWindowPosY"), r), &y, 120);
      m_settings.window_pos[n] = wxPoint(x, y);
      pConf->Read(wxString::Format(wxT("Radar%dControlShow"), r), &m_settings.show_radar_control[n], false);
      pConf->Read(wxString::Format(wxT("Radar%dTargetShow"), r), &v, true);
      m_radar[r]->m_target_on_ppi.Update(v);

      pConf->Read(wxString::Format(wxT("Radar%dControlPosX"), r), &x, wxDefaultPosition.x);
      pConf->Read(wxString::Format(wxT("Radar%dControlPosY"), r), &y, wxDefaultPosition.y);
      m_settings.control_pos[n] = wxPoint(x, y);
      LOG_DIALOG(wxT("radar_pi: LoadConfig: show_radar[%d]=%d control=%d,%d"), n, v, x, y);
      for (int i = 0; i < GUARD_ZONES; i++) {
        pConf->Read(wxString::Format(wxT("Radar%dZone%dStartBearing"), r, i), &ri->m_guard_zone[i]->m_start_bearing, 0);
        pConf->Read(wxString::Format(wxT("Radar%dZone%dEndBearing"), r, i), &ri->m_guard_zone[i]->m_end_bearing, 0);
        pConf->Read(wxString::Format(wxT("Radar%dZone%dOuterRange"), r, i), &ri->m_guard_zone[i]->m_outer_range, 0);
        pConf->Read(wxString::Format(wxT("Radar%dZone%dInnerRange"), r, i), &ri->m_guard_zone[i]->m_inner_range, 0);
        pConf->Read(wxString::Format(wxT("Radar%dZone%dType"), r, i), &v, 0);
        pConf->Read(wxString::Format(wxT("Radar%dZone%dAlarmOn"), r, i), &ri->m_guard_zone[i]->m_alarm_on, 0);
        pConf->Read(wxString::Format(wxT("Radar%dZone%dArpaOn"), r, i), &ri->m_guard_zone[i]->m_arpa_on, 0);
        ri->m_guard_zone[i]->SetType((GuardZoneType)v);
      }
      pConf->Read(wxT("AlarmPosX"), &x, 25);
      pConf->Read(wxT("AlarmPosY"), &y, 175);
      m_settings.alarm_pos = wxPoint(x, y);
      pConf->Read(wxT("EnableCOGHeading"), &m_settings.enable_cog_heading, false);
      pConf->Read(wxT("AISatARPAoffset"), &m_settings.AISatARPAoffset, 50);
      if (m_settings.AISatARPAoffset < 10 || m_settings.AISatARPAoffset > 300) m_settings.AISatARPAoffset = 50;

      n++;
    }
    m_settings.radar_count = n;
    pConf->Read(wxT("AlertAudioFile"), &m_settings.alert_audio_file, m_shareLocn + wxT("alarm.wav"));
    pConf->Read(wxT("ColourStrong"), &s, "red");
    m_settings.strong_colour = wxColour(s);
    pConf->Read(wxT("ColourIntermediate"), &s, "green");
    m_settings.intermediate_colour = wxColour(s);
    pConf->Read(wxT("ColourWeak"), &s, "blue");
    m_settings.weak_colour = wxColour(s);
    pConf->Read(wxT("ColourArpaEdge"), &s, "white");
    m_settings.arpa_colour = wxColour(s);
    pConf->Read(wxT("ColourAISText"), &s, "rgb(100,100,100)");
    m_settings.ais_text_colour = wxColour(s);
    pConf->Read(wxT("ColourPPIBackground"), &s, "rgb(0,0,50)");
    m_settings.ppi_background_colour = wxColour(s);
    pConf->Read(wxT("ColourDopplerApproaching"), &s, "yellow");
    m_settings.doppler_approaching_colour = wxColour(s);
    pConf->Read(wxT("ColourDopplerReceding"), &s, "cyan");
    m_settings.doppler_receding_colour = wxColour(s);
    pConf->Read(wxT("DeveloperMode"), &m_settings.developer_mode, false);
    pConf->Read(wxT("DrawingMethod"), &m_settings.drawing_method, 0);
    pConf->Read(wxT("GuardZoneDebugInc"), &m_settings.guard_zone_debug_inc, 0);
    pConf->Read(wxT("GuardZoneOnOverlay"), &m_settings.guard_zone_on_overlay, true);
    pConf->Read(wxT("OverlayStandby"), &m_settings.overlay_on_standby, true);
    pConf->Read(wxT("GuardZoneTimeout"), &m_settings.guard_zone_timeout, 30);
    pConf->Read(wxT("GuardZonesRenderStyle"), &m_settings.guard_zone_render_style, 0);
    pConf->Read(wxT("GuardZonesThreshold"), &m_settings.guard_zone_threshold, 5L);
    pConf->Read(wxT("IgnoreRadarHeading"), &m_settings.ignore_radar_heading, 0);
    pConf->Read(wxT("ShowExtremeRange"), &m_settings.show_extreme_range, false);
    pConf->Read(wxT("MenuAutoHide"), &m_settings.menu_auto_hide, 0);
    pConf->Read(wxT("PassHeadingToOCPN"), &m_settings.pass_heading_to_opencpn, false);
    pConf->Read(wxT("Refreshrate"), &v, 3);
    m_settings.refreshrate.Update(v);
    pConf->Read(wxT("ReverseZoom"), &m_settings.reverse_zoom, false);
    pConf->Read(wxT("ScanMaxAge"), &m_settings.max_age, 6);
    pConf->Read(wxT("Show"), &m_settings.show, true);
    pConf->Read(wxT("SkewFactor"), &m_settings.skew_factor, 1);
    pConf->Read(wxT("ThresholdBlue"), &m_settings.threshold_blue, 50);
    // Make room for BLOB_HISTORY_MAX history values
    m_settings.threshold_blue = MAX(m_settings.threshold_blue, BLOB_HISTORY_MAX + 1);
    pConf->Read(wxT("ThresholdGreen"), &m_settings.threshold_green, 100);
    pConf->Read(wxT("ThresholdRed"), &m_settings.threshold_red, 200);
    pConf->Read(wxT("TrailColourStart"), &s, "rgb(255,255,255)");
    m_settings.trail_start_colour = wxColour(s);
    pConf->Read(wxT("TrailColourEnd"), &s, "rgb(63,63,63)");
    m_settings.trail_end_colour = wxColour(s);
    pConf->Read(wxT("TrailsOnOverlay"), &m_settings.trails_on_overlay, false);
    pConf->Read(wxT("Transparency"), &v, DEFAULT_OVERLAY_TRANSPARENCY);
    m_settings.overlay_transparency.Update(v);

    m_settings.max_age = wxMax(wxMin(m_settings.max_age, MAX_AGE), MIN_AGE);

    SaveConfig();
    return true;
  }
  return false;
}

bool radar_pi::SaveConfig(void) {
  wxFileConfig *pConf = m_pconfig;

  if (pConf) {
    pConf->DeleteGroup(wxT("/Plugins/Radar"));
    pConf->SetPath(wxT("/Plugins/Radar"));

    pConf->Write(wxT("AlarmPosX"), m_settings.alarm_pos.x);
    pConf->Write(wxT("AlarmPosY"), m_settings.alarm_pos.y);
    pConf->Write(wxT("AlertAudioFile"), m_settings.alert_audio_file);
    pConf->Write(wxT("DeveloperMode"), m_settings.developer_mode);
    pConf->Write(wxT("DrawingMethod"), m_settings.drawing_method);
    pConf->Write(wxT("EnableCOGHeading"), m_settings.enable_cog_heading);
    pConf->Write(wxT("GuardZoneDebugInc"), m_settings.guard_zone_debug_inc);
    pConf->Write(wxT("GuardZoneOnOverlay"), m_settings.guard_zone_on_overlay);
    pConf->Write(wxT("OverlayStandby"), m_settings.overlay_on_standby);
    pConf->Write(wxT("GuardZoneTimeout"), m_settings.guard_zone_timeout);
    pConf->Write(wxT("GuardZonesRenderStyle"), m_settings.guard_zone_render_style);
    pConf->Write(wxT("GuardZonesThreshold"), m_settings.guard_zone_threshold);
    pConf->Write(wxT("IgnoreRadarHeading"), m_settings.ignore_radar_heading);
    pConf->Write(wxT("ShowExtremeRange"), m_settings.show_extreme_range);
    pConf->Write(wxT("MenuAutoHide"), m_settings.menu_auto_hide);
    pConf->Write(wxT("PassHeadingToOCPN"), m_settings.pass_heading_to_opencpn);
    pConf->Write(wxT("RangeUnits"), (int)m_settings.range_units);
    pConf->Write(wxT("Refreshrate"), m_settings.refreshrate.GetValue());
    pConf->Write(wxT("ReverseZoom"), m_settings.reverse_zoom);
    pConf->Write(wxT("ScanMaxAge"), m_settings.max_age);
    pConf->Write(wxT("Show"), m_settings.show);
    pConf->Write(wxT("SkewFactor"), m_settings.skew_factor);
    pConf->Write(wxT("ThresholdBlue"), m_settings.threshold_blue);
    pConf->Write(wxT("ThresholdGreen"), m_settings.threshold_green);
    pConf->Write(wxT("ThresholdRed"), m_settings.threshold_red);
    pConf->Write(wxT("TrailColourStart"), m_settings.trail_start_colour.GetAsString());
    pConf->Write(wxT("TrailColourEnd"), m_settings.trail_end_colour.GetAsString());
    pConf->Write(wxT("TrailsOnOverlay"), m_settings.trails_on_overlay);
    pConf->Write(wxT("Transparency"), m_settings.overlay_transparency.GetValue());
    pConf->Write(wxT("VerboseLog"), m_settings.verbose);
    pConf->Write(wxT("AISatARPAoffset"), m_settings.AISatARPAoffset);
    pConf->Write(wxT("ColourStrong"), m_settings.strong_colour.GetAsString());
    pConf->Write(wxT("ColourIntermediate"), m_settings.intermediate_colour.GetAsString());
    pConf->Write(wxT("ColourWeak"), m_settings.weak_colour.GetAsString());
    pConf->Write(wxT("ColourDopplerApproaching"), m_settings.doppler_approaching_colour.GetAsString());
    pConf->Write(wxT("ColourDopplerReceding"), m_settings.doppler_receding_colour.GetAsString());
    pConf->Write(wxT("ColourArpaEdge"), m_settings.arpa_colour.GetAsString());
    pConf->Write(wxT("ColourAISText"), m_settings.ais_text_colour.GetAsString());
    pConf->Write(wxT("ColourPPIBackground"), m_settings.ppi_background_colour.GetAsString());
    pConf->Write(wxT("RadarCount"), m_settings.radar_count);
    pConf->Write(wxT("DockSize"), m_settings.dock_size);

    for (int r = 0; r < (int)m_settings.radar_count; r++) {
      pConf->Write(wxString::Format(wxT("Radar%dType"), r), RadarTypeName[m_radar[r]->m_radar_type]);
      pConf->Write(wxString::Format(wxT("Radar%dNavicoInfo"), r), m_settings.navico_radar_info[r].to_string());
      pConf->Write(wxString::Format(wxT("Radar%dAddress"), r), m_settings.radar_address[r].FormatNetworkAddress());
      pConf->Write(wxString::Format(wxT("Radar%dInterface"), r), m_settings.radar_interface_address[r].FormatNetworkAddress());
      pConf->Write(wxString::Format(wxT("Radar%dRange"), r), m_radar[r]->m_range.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dRotation"), r), m_radar[r]->m_orientation.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dTransmit"), r), m_radar[r]->m_state.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dWindowShow"), r), m_settings.show_radar[r]);
      pConf->Write(wxString::Format(wxT("Radar%dWindowDock"), r), m_settings.dock_radar[r]);
      pConf->Write(wxString::Format(wxT("Radar%dControlShow"), r), m_settings.show_radar_control[r]);
      pConf->Write(wxString::Format(wxT("Radar%dTargetShow"), r), m_radar[r]->m_target_on_ppi.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dTrailsState"), r), (int)m_radar[r]->m_target_trails.GetState());
      pConf->Write(wxString::Format(wxT("Radar%dTrails"), r), m_radar[r]->m_target_trails.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dTrueTrailsMotion"), r), m_radar[r]->m_trails_motion.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dWindowPosX"), r), m_settings.window_pos[r].x);
      pConf->Write(wxString::Format(wxT("Radar%dWindowPosY"), r), m_settings.window_pos[r].y);
      pConf->Write(wxString::Format(wxT("Radar%dControlPosX"), r), m_settings.control_pos[r].x);
      pConf->Write(wxString::Format(wxT("Radar%dControlPosY"), r), m_settings.control_pos[r].y);
      pConf->Write(wxString::Format(wxT("Radar%dMainBangSize"), r), m_radar[r]->m_main_bang_size.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dAntennaForward"), r), m_radar[r]->m_antenna_forward.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dAntennaStarboard"), r), m_radar[r]->m_antenna_starboard.GetValue());
      pConf->Write(wxString::Format(wxT("Radar%dRunTimeOnIdle"), r), m_radar[r]->m_timed_run.GetValue());
      for (int i = 0; i < MAX_CHART_CANVAS; i++) {
        pConf->Write(wxString::Format(wxT("Radar%dOverlayCanvas%d"), r, i), m_radar[r]->m_overlay_canvas[i].GetValue());
      }

      // LOG_DIALOG(wxT("radar_pi: SaveConfig: show_radar[%d]=%d"), r, m_settings.show_radar[r]);
      for (int i = 0; i < GUARD_ZONES; i++) {
        pConf->Write(wxString::Format(wxT("Radar%dZone%dStartBearing"), r, i), m_radar[r]->m_guard_zone[i]->m_start_bearing);
        pConf->Write(wxString::Format(wxT("Radar%dZone%dEndBearing"), r, i), m_radar[r]->m_guard_zone[i]->m_end_bearing);
        pConf->Write(wxString::Format(wxT("Radar%dZone%dOuterRange"), r, i), m_radar[r]->m_guard_zone[i]->m_outer_range);
        pConf->Write(wxString::Format(wxT("Radar%dZone%dInnerRange"), r, i), m_radar[r]->m_guard_zone[i]->m_inner_range);
        pConf->Write(wxString::Format(wxT("Radar%dZone%dType"), r, i), (int)m_radar[r]->m_guard_zone[i]->m_type);
        pConf->Write(wxString::Format(wxT("Radar%dZone%dAlarmOn"), r, i), m_radar[r]->m_guard_zone[i]->m_alarm_on);
        pConf->Write(wxString::Format(wxT("Radar%dZone%dArpaOn"), r, i), m_radar[r]->m_guard_zone[i]->m_arpa_on);
      }
    }

    pConf->Flush();
    // LOG_VERBOSE(wxT("radar_pi: Saved settings"));
    return true;
  }

  return false;
}

void radar_pi::SetNavicoRadarInfo(size_t r, const NavicoRadarInfo &info) {
  wxCriticalSectionLocker lock(m_exclusive);

  M_SETTINGS.navico_radar_info[r] = info;
}

void radar_pi::FoundNavicoRadarInfo(const NetworkAddress &addr, const NetworkAddress &interface_addr, const NavicoRadarInfo &info) {
  wxCriticalSectionLocker lock(m_exclusive);

  bool halo_type = false;
  int radar_order[RT_MAX];
  for (int i = 0; i < RT_MAX; i++) {
    radar_order[i] = RadarOrder[i];
  }

  // When NavicoLocate finds a Halo type we should only put it in a info field of an Halo radar
   /*As far as we know:
   13 and 14 = 4G
   15 = old Halo
   16
   17
   18 & serialNr[4] == '4' = new 3G (or 4G?)
   19 = Halo24
   */

  // Find the number of physical Navico radars
  size_t navicos = 0;  // number of hard Navico radars
  bool navico[RT_MAX];
  CLEAR_STRUCT(navico);
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (m_radar[r]->m_radar_type == RT_3G)    navico[RT_3G] = true;
    if (m_radar[r]->m_radar_type == RT_4GA)   navico[RT_4GA] = true;
    if (m_radar[r]->m_radar_type == RT_4GB)   navico[RT_4GB] = true;
    if (m_radar[r]->m_radar_type == RT_HaloA) navico[RT_HaloA] = true;
    if (m_radar[r]->m_radar_type == RT_HaloB) navico[RT_HaloB] = true;
   }
  
  navicos = (size_t) navico[RT_3G] + (size_t) (navico[RT_4GA] || navico[RT_4GB]) + (size_t) (navico[RT_HaloA] || navico[RT_HaloB]);

  // more then 2 Navico radars: associate the info found with the right type of radar 
  if (navicos > 1) {
    if (info.serialNr[0] == '1' && (info.serialNr[1] == '9' || info.serialNr[1] == '7' ||
      info.serialNr[1] == '6' || info.serialNr[1] == '5')) {  // It seems that serial # starting with 15 - 19  refers to Halo type radars
      halo_type = true;
    }

    if (halo_type) {
      radar_order[RT_4GA] = 0;
      radar_order[RT_4GB] = 0;
    }
    else {
      radar_order[RT_HaloA] = 0;
      radar_order[RT_HaloB] = 0;
    }
  }

  if (info.serialNr[0] == '1' && info.serialNr[1] == '8' && info.serialNr[4] == '4') {
    // this is a new 3G or (may be) a 4G which will handle NavicoLocate
    radar_order[RT_3G] = 1;
  }

  NetworkAddress int_face_addr = interface_addr;
  NetworkAddress radar_addr = addr;

  // First, check if we already know this serial#
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (ntohs(addr.port) == radar_order[m_radar[r]->m_radar_type] &&  // Only put primary in primary slots, etc.
      M_SETTINGS.navico_radar_info[r].serialNr == info.serialNr) {
      SetNavicoRadarInfo(r, info);
      SetRadarInterfaceAddress(r, int_face_addr, radar_addr);
      return;
    }
  }

  // Second loop, put it in radar with same report address but no serial#
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (ntohs(addr.port) == radar_order[m_radar[r]->m_radar_type] &&  // Only put primary in primary slots, etc.
      !info.report_addr.IsNull() &&                               // If the report address fits, override the serial
      M_SETTINGS.navico_radar_info[r].report_addr == info.report_addr) {
      SetNavicoRadarInfo(r, info);
      SetRadarInterfaceAddress(r, int_face_addr, radar_addr);
      return;
    }
  }

  // Third loop, put it in radar with same IP address but no serial# nor report address
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (ntohs(addr.port) == radar_order[m_radar[r]->m_radar_type] &&  // Only put primary in primary slots, etc.
      M_SETTINGS.radar_address[r] == addr && M_SETTINGS.navico_radar_info[r].serialNr.IsNull() &&
      M_SETTINGS.navico_radar_info[r].report_addr.IsNull()) {
      SetNavicoRadarInfo(r, info);
      SetRadarInterfaceAddress(r, int_face_addr, radar_addr);
      return;
    }
  }

  // In case of desperation, put it in a free slot without serial# or address
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (ntohs(addr.port) == radar_order[m_radar[r]->m_radar_type] &&  // Only put primary in primary slots, etc.
      M_SETTINGS.radar_address[r].IsNull() && M_SETTINGS.navico_radar_info[r].serialNr.IsNull() &&
      M_SETTINGS.navico_radar_info[r].report_addr.IsNull()) {
      SetNavicoRadarInfo(r, info);
      SetRadarInterfaceAddress(r, int_face_addr, radar_addr);
      return;
    }
  }

  // No free slot, override the first radar A with A, B with B but only Halo with Halo
  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    if (ntohs(addr.port) == radar_order[m_radar[r]->m_radar_type]) {  // Only put primary in primary slots, etc.
      SetNavicoRadarInfo(r, info);
      SetRadarInterfaceAddress(r, int_face_addr, radar_addr);
      return;
    }
  }
  LOG_INFO(wxT("radar_pi: Failed to allocate info from NavicoLocate to a radar"));
}

bool radar_pi::HaveRadarSerialNo(size_t r) {
  wxCriticalSectionLocker lock(m_exclusive);

  return !M_SETTINGS.navico_radar_info[r].serialNr.IsNull();
}

NavicoRadarInfo &radar_pi::GetNavicoRadarInfo(size_t r) {
  wxCriticalSectionLocker lock(m_exclusive);

  return M_SETTINGS.navico_radar_info[r];
}

// Positional Data passed from NMEA to plugin
void radar_pi::SetPositionFix(PlugIn_Position_Fix &pfix) {}

void radar_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix) {
  wxCriticalSectionLocker lock(m_exclusive);

  time_t now = time(0);
  wxString info;
  if (m_var_source <= VARIATION_SOURCE_FIX && !wxIsNaN(pfix.Var) && (fabs(pfix.Var) > 0.0 || m_var == 0.0)) {
    if (m_var_source < VARIATION_SOURCE_FIX || fabs(pfix.Var - m_var) > 0.05) {
      LOG_VERBOSE(wxT("radar_pi: Position fix provides new magnetic variation %f"), pfix.Var);
      if (m_pMessageBox->IsShown()) {
        info = _("GPS");
        info << wxT(" ") << wxString::Format(wxT("%2.1f"), m_var);
        m_pMessageBox->SetVariationInfo(info);
      }
    }
    m_var = pfix.Var;
    m_var_source = VARIATION_SOURCE_FIX;
    m_var_timeout = now + WATCHDOG_TIMEOUT;
  }

  LOG_VERBOSE(wxT("radar_pi: SetPositionFixEx var=%f var_wd=%d"), pfix.Var, NOT_TIMED_OUT(now, m_var_timeout));

  if (!wxIsNaN(pfix.Hdt)) {
    if (m_heading_source < HEADING_FIX_HDT) {
      LOG_VERBOSE(wxT("radar_pi: Heading source is now HDT from OpenCPN (%d->%d)"), m_heading_source, HEADING_FIX_HDT);
      m_heading_source = HEADING_FIX_HDT;
    }
    if (m_heading_source == HEADING_FIX_HDT) {
      m_hdt = pfix.Hdt;
      m_hdt_timeout = now + HEADING_TIMEOUT;
    }
  } else if (!wxIsNaN(pfix.Hdm) && NOT_TIMED_OUT(now, m_var_timeout)) {
    if (m_heading_source < HEADING_FIX_HDM) {
      LOG_VERBOSE(wxT("radar_pi: Heading source is now HDM from OpenCPN + VAR (%d->%d)"), m_heading_source, HEADING_FIX_HDM);
      m_heading_source = HEADING_FIX_HDM;
    }
    if (m_heading_source == HEADING_FIX_HDM) {
      m_hdm = pfix.Hdm;
      m_hdt = pfix.Hdm + m_var;
      m_hdm_timeout = now + HEADING_TIMEOUT;
    }
  } else if (!wxIsNaN(pfix.Cog) && m_settings.enable_cog_heading) {
    if (m_heading_source < HEADING_FIX_COG) {
      LOG_VERBOSE(wxT("radar_pi: Heading source is now COG from OpenCPN (%d->%d)"), m_heading_source, HEADING_FIX_COG);
      m_heading_source = HEADING_FIX_COG;
    }
    if (m_heading_source == HEADING_FIX_COG) {
      m_hdt = pfix.Cog;
      m_hdt_timeout = now + HEADING_TIMEOUT;
    }
  }
  if (!wxIsNaN(pfix.Cog)) {
    UpdateCOGAvg(pfix.Cog);
  }
  if (TIMED_OUT(now, m_cog_timeout)) {
    m_cog_timeout = now + m_COGAvgSec;
    m_cog = m_COGAvg;
  }
  if (pfix.FixTime <= 0 || TIMED_OUT(now, pfix.FixTime + WATCHDOG_TIMEOUT) || pfix.FixTime > now) {
    return;
  }
  if (pfix.Lat > 90. || pfix.Lat < -90. || pfix.Lon < -180. || pfix.Lon > 180. || isnan(pfix.Lon) || isnan(pfix.Lat)) {
    LOG_INFO(wxT(" **error wrong position from opencpn pfix.Lat=%f, pfix.Lon=%f"), pfix.Lat, pfix.Lon);
    return;
  }
  ExtendedPosition GPS_position;

  GPS_position.pos.lat = pfix.Lat;
  GPS_position.pos.lon = pfix.Lon;
  GPS_position.time = wxGetUTCTimeMillis();
  GPS_position.dlat_dt = 0.;
  GPS_position.dlon_dt = 0.;
  GPS_position.sd_speed_kn = 0.;

  if (!m_bpos_set) {
    LOG_VERBOSE(wxT("radar_pi: GPS position is now known m_ownship.lat= %f, m_ownship.lon = %f"), GPS_position.pos.lat,
                GPS_position.pos.lon);
  }
  m_bpos_set = true;
  m_bpos_timestamp = now;

  if (IsBoatPositionValid()) {
    if (!m_predicted_position_initialised) {
      m_expected_position = GPS_position;
      m_last_fixed = GPS_position;
      m_expected_position.dlat_dt = 0.;
      m_expected_position.dlon_dt = 0.;
      m_expected_position.speed_kn = 0.;
      m_predicted_position_initialised = true;
    }

    m_GPS_filter->Predict(&m_last_fixed, &m_expected_position);         // update expected position based on previous positions
    m_GPS_filter->Update_P();                                           // update error covariance matrix
    m_GPS_filter->SetMeasurement(&GPS_position, &m_expected_position);  // improve expected postition with GPS

    // check validity of this position
    if (m_expected_position.pos.lat > 90. || m_expected_position.pos.lat < -90. || m_expected_position.pos.lon < -180. ||
        m_expected_position.pos.lat > 180. || isnan(m_expected_position.pos.lat) || isnan(m_expected_position.pos.lon)) {
      // if not valid, reset the Kalman filter
      LOG_INFO(wxT("** error in position, GPSfilter reset lat=%f, lon=%f"), m_expected_position.pos.lat,
               m_expected_position.pos.lon);
      m_expected_position = GPS_position;
      m_last_fixed = GPS_position;
      m_expected_position.dlat_dt = 0.;
      m_expected_position.dlon_dt = 0.;
      m_expected_position.speed_kn = 0.;
      m_predicted_position_initialised = true;
    }
    // Now set the expected position from the Kalmanfilter as the boat position
    m_ownship = m_expected_position.pos;
    m_last_fixed = m_expected_position;
  }
}

void radar_pi::UpdateCOGAvg(double cog) {
  // This is a straight copy (except for formatting) of the code in
  // OpenCPN/src/chart1.cpp MyFrame::PostProcessNNEA
  if (m_COGAvgSec > 0) {
    //    Make a hole
    for (int i = m_COGAvgSec - 1; i > 0; i--) {
      m_COGTable[i] = m_COGTable[i - 1];
    }
    m_COGTable[0] = cog;

    double sum = 0., count = 0;
    for (int i = 0; i < m_COGAvgSec; i++) {
      double adder = m_COGTable[i];
      if (wxIsNaN(adder)) {
        continue;
      }
      if (fabs(adder - m_COGAvg) > 180.) {
        if ((adder - m_COGAvg) > 0.) {
          adder -= 360.;
        } else {
          adder += 360.;
        }
      }

      sum += adder;
      count++;
    }
    sum /= count;

    if (sum < 0.) {
      sum += 360.;
    } else if (sum >= 360.) {
      sum -= 360.;
    }
    m_COGAvg = sum;
  } else {
    m_COGAvg = cog;
  }
}

void radar_pi::SetPluginMessage(wxString &message_id, wxString &message_body) {
  static const wxString WMM_VARIATION_BOAT = wxString(_T("WMM_VARIATION_BOAT"));
  wxString info;
  if (message_id.Cmp(WMM_VARIATION_BOAT) == 0) {
    wxJSONReader reader;
    wxJSONValue message;
    if (!reader.Parse(message_body, &message)) {
      wxCriticalSectionLocker lock(m_exclusive);
      wxJSONValue defaultValue(360);
      double variation = message.Get(_T("Decl"), defaultValue).AsDouble();

      if (variation != 360.0) {
        if (m_var_source != VARIATION_SOURCE_WMM) {
          LOG_VERBOSE(wxT("radar_pi: WMM plugin provides new magnetic variation %f"), variation);
        }
        m_var = variation;
        m_var_source = VARIATION_SOURCE_WMM;
        m_var_timeout = time(0) + WATCHDOG_TIMEOUT;
        if (m_pMessageBox->IsShown()) {
          info = _("WMM");
          info << wxT(" ") << wxString::Format(wxT("%2.1f"), m_var);
          m_pMessageBox->SetVariationInfo(info);
        }
      }
    }
  } else if (message_id == wxS("AIS") || m_ais_in_arpa_zone.size() > 0) {
    // Check for ARPA targets
    bool arpa_is_present = false;
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      if (m_radar[r]->m_arpa->GetTargetCount() > 0) {
        arpa_is_present = true;
        break;
      }
    }
    if (arpa_is_present) {
      wxJSONReader reader;
      wxJSONValue message;
      if (!reader.Parse(message_body, &message)) {
        wxJSONValue defaultValue(999);
        long json_ais_mmsi = message.Get(_T("mmsi"), defaultValue).AsLong();
        if (json_ais_mmsi > 200000000) {  // Neither ARPA targets nor SAR_aircraft
          wxJSONValue defaultValue("90.0");
          double f_AISLat = wxAtof(message.Get(_T("lat"), defaultValue).AsString());
          double f_AISLon = wxAtof(message.Get(_T("lon"), defaultValue).AsString());

          // Rectangle around own ship to look for AIS targets.
          double d_side = m_arpa_max_range / 1852.0 / 60.0;
          if (f_AISLat < (m_ownship.lat + d_side) && f_AISLat > (m_ownship.lat - d_side) &&
              f_AISLon < (m_ownship.lon + d_side * 2) && f_AISLon > (m_ownship.lon - d_side * 2)) {
            bool updated = false;
            for (size_t i = 0; i < m_ais_in_arpa_zone.size(); i++) {  // Check for existing mmsi
              if (m_ais_in_arpa_zone[i].ais_mmsi == json_ais_mmsi) {
                m_ais_in_arpa_zone[i].ais_time_upd = time(0);
                m_ais_in_arpa_zone[i].ais_lat = f_AISLat;
                m_ais_in_arpa_zone[i].ais_lon = f_AISLon;
                updated = true;
                break;
              }
            }
            if (!updated) {  // Add a new target to the list
              AisArpa m_new_ais_target;
              m_new_ais_target.ais_mmsi = json_ais_mmsi;
              m_new_ais_target.ais_time_upd = time(0);
              m_new_ais_target.ais_lat = f_AISLat;
              m_new_ais_target.ais_lon = f_AISLon;
              m_ais_in_arpa_zone.push_back(m_new_ais_target);
            }
          }
        }
      }
    }
    // Delete > 3 min old AIS items or at once if no active ARPA
    if (m_ais_in_arpa_zone.size() > 0) {
      for (size_t i = 0; i < m_ais_in_arpa_zone.size(); i++) {
        if (m_ais_in_arpa_zone[i].ais_mmsi > 0 && (time(0) - m_ais_in_arpa_zone[i].ais_time_upd > 3 * 60 || !arpa_is_present)) {
          m_ais_in_arpa_zone.erase(m_ais_in_arpa_zone.begin() + i);
          m_arpa_max_range = BASE_ARPA_DIST;  // Renew AIS search area
        }
      }
    }
  }
}

bool radar_pi::FindAIS_at_arpaPos(const GeoPosition &pos, const double &arpa_dist) {
  m_arpa_max_range = MAX(arpa_dist + 200, m_arpa_max_range);  // For AIS search area
  if (m_ais_in_arpa_zone.size() < 1) return false;
  bool hit = false;
  // Default 50 >> look 100 meters around + 4% of distance to target
  double offset = (double)m_settings.AISatARPAoffset;
  double dist2target = (4.0 / 100) * arpa_dist;
  offset += dist2target;
  offset = offset / 1852. / 60.;
  for (size_t i = 0; i < m_ais_in_arpa_zone.size(); i++) {
    if (m_ais_in_arpa_zone[i].ais_mmsi != 0) {  // Avtive post
      if (pos.lat + offset > m_ais_in_arpa_zone[i].ais_lat && pos.lat - offset < m_ais_in_arpa_zone[i].ais_lat &&
          pos.lon + (offset * 1.75) > m_ais_in_arpa_zone[i].ais_lon && pos.lon - (offset * 1.75) < m_ais_in_arpa_zone[i].ais_lon) {
        hit = true;
        break;
      }
    }
  }
  return hit;
}

//*****************************************************************************************************
void radar_pi::CacheSetToolbarToolBitmaps() {
  if (m_toolbar_button == m_sent_toolbar_button) {
    return;  // no change needed
  }

  wxString icon;

  switch (m_toolbar_button) {
    case TB_NONE:
    case TB_HIDDEN:
      icon = m_shareLocn + wxT("radar_hidden.svg");
      break;

    case TB_SEEN:
      icon = m_shareLocn + wxT("radar_seen.svg");
      break;

    case TB_STANDBY:
      icon = m_shareLocn + wxT("radar_standby.svg");
      break;

    case TB_SEARCHING:
      icon = m_shareLocn + wxT("radar_searching.svg");
      break;

    case TB_ACTIVE:
      icon = m_shareLocn + wxT("radar_active.svg");
      break;
  }
  SetToolbarToolBitmapsSVG(m_tool_id, icon, icon, icon);
  m_sent_toolbar_button = m_toolbar_button;
}

/*
   SetNMEASentence is used to speed up rotation and variation
   detection if SetPositionEx() is not called very often. This will
   be the case if you have a high speed heading sensor (for instance, 2 to 20
   Hz)
   but only a 1 Hz GPS update.
*/

void radar_pi::SetNMEASentence(wxString &sentence) {
  m_NMEA0183 << sentence;
  time_t now = time(0);
  double hdm = nan("");
  double hdt = nan("");
  double var;

  LOG_RECEIVE(wxT("radar_pi: SetNMEASentence %s"), sentence.c_str());

  if (m_NMEA0183.PreParse()) {
    if (m_NMEA0183.LastSentenceIDReceived == _T("HDG") && m_NMEA0183.Parse()) {
      if (!wxIsNaN(m_NMEA0183.Hdg.MagneticVariationDegrees)) {
        if (m_NMEA0183.Hdg.MagneticVariationDirection == East) {
          var = +m_NMEA0183.Hdg.MagneticVariationDegrees;
        } else {
          var = -m_NMEA0183.Hdg.MagneticVariationDegrees;
        }
        if (fabs(var - m_var) >= 0.05 && m_var_source <= VARIATION_SOURCE_NMEA) {
          //        LOG_INFO(wxT("radar_pi: NMEA provides new magnetic variation %f from %s"), var, sentence.c_str());
          m_var = var;
          m_var_source = VARIATION_SOURCE_NMEA;
          m_var_timeout = now + WATCHDOG_TIMEOUT;
          wxString info = _("NMEA");
          info << wxT(" ") << wxString::Format(wxT("%2.1f"), m_var);
          m_pMessageBox->SetVariationInfo(info);
        }
      }

      if (!wxIsNaN(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees)) {
        hdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;
      }
    } else if (m_NMEA0183.LastSentenceIDReceived == _T("HDM") && m_NMEA0183.Parse() && !wxIsNaN(m_NMEA0183.Hdm.DegreesMagnetic)) {
      hdm = m_NMEA0183.Hdm.DegreesMagnetic;
    } else if (m_NMEA0183.LastSentenceIDReceived == _T("HDT") && m_NMEA0183.Parse() && !wxIsNaN(m_NMEA0183.Hdt.DegreesTrue)) {
      hdt = m_NMEA0183.Hdt.DegreesTrue;
    }
  }

  if (!wxIsNaN(hdt)) {
    if (m_heading_source < HEADING_NMEA_HDT) {
      //   LOG_INFO(wxT("radar_pi: Heading source is now HDT %d from NMEA %s (%d->%d)"), m_hdt, sentence.c_str(),
      //   m_heading_source,
      //           HEADING_NMEA_HDT);    Crashes!!!
      m_heading_source = HEADING_NMEA_HDT;
    }
    if (m_heading_source == HEADING_NMEA_HDT) {
      m_hdt = hdt;
      m_hdt_timeout = now + HEADING_TIMEOUT;
    }
  } else if (!wxIsNaN(hdm) && NOT_TIMED_OUT(now, m_var_timeout)) {
    if (m_heading_source < HEADING_NMEA_HDM) {
      //   LOG_INFO(wxT("radar_pi: Heading source is now HDM %f + VAR %f from NMEA %s (%d->%d)"), hdm, m_var, sentence.c_str(),
      //            m_heading_source, HEADING_NMEA_HDT);
      m_heading_source = HEADING_NMEA_HDM;
    }
    if (m_heading_source == HEADING_NMEA_HDM) {
      m_hdm = hdm;
      m_hdt = hdm + m_var;
      m_hdm_timeout = now + HEADING_TIMEOUT;
    }
  }
}

// is not called anywhere
void radar_pi::SetCursorPosition(GeoPosition pos) { m_cursor_pos = pos; }

void radar_pi::SetCursorLatLon(double lat, double lon) {
  m_cursor_pos.lat = lat;
  m_cursor_pos.lon = lon;
}

bool radar_pi::MouseEventHook(wxMouseEvent &event) {
  if (event.LeftDown()) {
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      m_radar[r]->SetMousePosition(m_cursor_pos);
    }
  }
  if (event.RightDown()) {
    m_right_click_pos = m_cursor_pos;
  }
  return false;
}

void radar_pi::logBinaryData(const wxString &what, const uint8_t *data, int size) {
  wxString explain;
  int i = 0;

  explain.Alloc(size * 3 + 50);
  explain += wxT("radar_pi: ");
  explain += what;
  explain += wxString::Format(wxT(" %d bytes: "), size);
  for (i = 0; i < size; i++) {
    explain += wxString::Format(wxT(" %02X"), data[i]);
  }
  wxLogMessage(explain);
}

bool radar_pi::IsRadarOnScreen(int radar) {
  return m_settings.show && (m_settings.show_radar[radar] || m_radar[radar]->GetOverlayCanvasIndex() > -1);
}

PLUGIN_END_NAMESPACE
