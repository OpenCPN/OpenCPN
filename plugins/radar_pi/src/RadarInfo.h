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

#ifndef _RADAR_INFO_H_
#define _RADAR_INFO_H_

#include "radar_pi.h"

#include "ControlsDialog.h"
#include "RadarControlItem.h"
#include "RadarReceive.h"

PLUGIN_BEGIN_NAMESPACE

class RadarDraw;
class RadarCanvas;
class RadarPanel;
class GuardZoneBogey;
class RadarInfo;
class TrailBuffer;

struct DrawInfo {
  RadarDraw *draw;
  int drawing_method;
  bool color_option;
};

#define SECONDS_TO_REVOLUTIONS(x) ((x)*2 / 5)
#define TRAIL_MAX_REVOLUTIONS SECONDS_TO_REVOLUTIONS(600) + 1
enum { TRAIL_15SEC, TRAIL_30SEC, TRAIL_1MIN, TRAIL_3MIN, TRAIL_5MIN, TRAIL_10MIN, TRAIL_CONTINUOUS, TRAIL_ARRAY_SIZE };

#define COURSE_SAMPLES (16)

class RadarInfo {
  friend class TrailBuffer;

 public:
  wxString m_name;         // Either "Radar", "Radar A", "Radar B".
  radar_pi *m_pi;          // Pointer back to the plugin
  size_t m_radar;          // Which radar this is [0..RADARS>
  RadarType m_radar_type;  // Which radar type
  size_t m_spokes;         // # of spokes per rotation
  size_t m_spoke_len_max;  // Max # of bytes per spoke

  // Digital radars cannot produce just any range. When asked for a particular value
  // they produce a slightly larger range.
  //
  // The 'official' value, which is also what we tell users, is set via m_range.Update() by
  // the receive threads.
  // The 'display' value (m_range_meters), used to compute the proper display, which is up to 40% larger
  // is set by ProcessRadarSpoke.
  // int m_range_meters;  // internal value, length of m_spoke_len_max in meters
  double m_pixels_per_meter;  // How many pixels of a line in a meter

  double m_course;  // m_course is the moving everage of m_hdt used for course_up
  double m_predictor;
  double m_course_log[COURSE_SAMPLES];
  int m_course_index;
  wxPoint m_off_center, m_drag;
  double m_radar_radius;  // radius in pixels of the outer ring in the panel
  double m_panel_zoom;    // zooming factor for the panel image

  RadarArpa *m_arpa;
  wxCriticalSection m_exclusive;

  /* User radar settings */

  RadarControlItem m_state;        // RadarState (observed)
  RadarControlItem m_boot_state;   // Can contain RADAR_TRANSMIT until radar is seen at boot
  RadarControlItem m_orientation;  // See below for allowed values.
  RadarControlItem m_view_center;

  int m_min_contour_length;  // minimum contour length of an ARPA or MARPA target

  RadarControlItem m_overlay_canvas[MAX_CHART_CANVAS];
  RadarRangeControlItem m_range;  // value in meters, shown on display
  RadarControlItem m_gain;
  RadarControlItem m_interference_rejection;
  RadarControlItem m_target_separation;
  RadarControlItem m_noise_rejection;
  RadarControlItem m_target_boost;
  RadarControlItem m_target_expansion;
  RadarControlItem m_sea;
  RadarControlItem m_rain;
  RadarControlItem m_ftc;
  RadarControlItem m_scan_speed;
  RadarControlItem m_bearing_alignment;
  RadarControlItem m_no_transmit_start;
  RadarControlItem m_no_transmit_end;
  RadarControlItem m_antenna_height;
  RadarControlItem m_antenna_forward;
  RadarControlItem m_antenna_starboard;
  RadarControlItem m_main_bang_size;
  RadarControlItem m_local_interference_rejection;
  RadarControlItem m_side_lobe_suppression;
  RadarControlItem m_target_trails;
  RadarControlItem m_trails_motion;
  RadarControlItem m_target_on_ppi;
  RadarControlItem m_next_state_change;
  RadarControlItem m_timed_idle;  // CT_TIMED_IDLE
  RadarControlItem m_timed_run;   // CT_TIMED_RUN
  RadarControlItem m_doppler;

  bool m_showManualValueInAuto;  // Does radar adjust manual value in auto mode? True for Garmin, False for others
  bool m_timed_idle_hardware;    // Does radar handle timed idle itself?

  /* Per radar objects */

  RadarControl *m_control;
  RadarReceive *m_receive;
  ControlsDialog *m_control_dialog;
  RadarPanel *m_radar_panel;
  RadarCanvas *m_radar_canvas;

  /* Abstractions of our own. Some filled by RadarReceive. */

  double m_viewpoint_rotation;

  time_t m_radar_timeout;      // When we consider the radar no longer valid
  time_t m_data_timeout;       // When we consider the data to be obsolete (radar no longer sending data)
  time_t m_stayalive_timeout;  // When we will send another stayalive ping
#define STAYALIVE_TIMEOUT (5)  // Send data every 5 seconds to ping radar
#define DATA_TIMEOUT (5)

  bool m_status_text_hide;

  int m_refresh_millis;

  GuardZone *m_guard_zone[GUARD_ZONES];
  double m_ebl[ORIENTATION_NUMBER][BEARING_LINES];
  double m_vrm[BEARING_LINES];
  receive_statistics m_statistics;

  struct line_history {
    uint8_t *line;
    wxLongLong time;
    GeoPosition pos;
  };

  line_history *m_history;

  int m_old_range;
  int m_dir_lat;
  int m_dir_lon;
  TrailBuffer *m_trails;

  // Timed Transmit
  time_t m_idle_standby;   // When we will change to standby
  time_t m_idle_transmit;  // When we will change to transmit

  /* Methods */

  RadarInfo(radar_pi *pi, int radar);
  ~RadarInfo();

  bool Init();
  void SetName(wxString name);
  wxString GetInfoStatus();

  void AdjustRange(int adjustment);
  void SetAutoRangeMeters(int meters);
  bool SetControlValue(ControlType controlType, RadarControlItem &item, RadarControlButton *button);
  void ProcessRadarSpoke(SpokeBearing angle, SpokeBearing bearing, uint8_t *data, size_t len, int range_meters, wxLongLong time);
  void RefreshDisplay();
  void RenderGuardZone();
  void ResetRadarImage();
  void ShiftImageLonToCenter();
  void ShiftImageLatToCenter();
  void RenderRadarImage1(wxPoint center, double scale, double rotation, bool overlay);
  void ShowRadarWindow(bool show);
  void ShowControlDialog(bool show, bool reparent);
  void Shutdown();
  // void DeleteReceive();
  void UpdateTransmitState();
  void RequestRadarState(RadarState state);
  int GetDrawTime() {
    wxCriticalSectionLocker lock(m_exclusive);
    return IsPaneShown() ? m_draw_time_ms : 0;
  };
  bool IsPaneShown();

  void resetTimeout(time_t now) {
    wxCriticalSectionLocker lock(m_exclusive);

    m_radar_timeout = now + WATCHDOG_TIMEOUT;
  };

  void UpdateControlState(bool all);
  void ComputeColourMap();
  void ComputeTargetTrails();
  void CheckTimedTransmit();
  void SetTimedNextStateTimer(int ms);
  wxString GetRangeText();
  wxString GetDisplayRangeStr(int meters, bool unit);
  int GetDisplayRange() { return m_range.GetValue(); };
  void DetectedRadar(NetworkAddress &interfaceAddress, NetworkAddress &radarAddress);
  void SetMousePosition(GeoPosition pos);
  void SetMouseVrmEbl(double vrm, double ebl);
  void SetBearing(int bearing);
  void SampleCourse(int angle);
  int GetOrientation();
  void ClearTrails();
  void SetRadarPosition(GeoPosition boat_pos, double heading) {
    wxCriticalSectionLocker lock(m_exclusive);

    if (m_antenna_starboard.GetValue() != 0 || m_antenna_forward.GetValue() != 0) {
      double sine = sin(deg2rad(heading));
      double cosine = cos(deg2rad(heading));
      double dist_forward = (double)m_antenna_forward.GetValue() / 1852 / 60;
      double dist_starboard = (double)m_antenna_starboard.GetValue() / 1852 / 60;
      m_radar_position.lat = dist_forward * cosine - dist_starboard * sine + boat_pos.lat;
      m_radar_position.lon = (dist_forward * sine + dist_starboard * cosine) / cos(deg2rad(boat_pos.lat)) + boat_pos.lon;
    } else {
      m_radar_position = boat_pos;
    }
  }

  bool GetRadarPosition(GeoPosition *pos);
  bool GetRadarPosition(ExtendedPosition *radar_pos);

  wxString GetCanvasTextTopLeft();
  wxString GetCanvasTextBottomLeft();
  wxString GetCanvasTextCenter();
  wxString GetTimedIdleText();
  wxString GetRadarStateText();

  GeoPosition m_mouse_pos;
  double m_mouse_ebl[ORIENTATION_NUMBER];
  double m_mouse_vrm;

  // Speedup lookup tables of color to r,g,b, set dependent on m_settings.display_option.
  PixelColour m_colour_map_rgb[BLOB_COLOURS];
  BlobColour m_colour_map[UINT8_MAX + 1];

  // Speedup PolarToCartesian lookup (angle,radius) -> (x, y)
  PolarToCartesianLookup *m_polar_lookup;

  void AdjustRange(int adjustment, int current_range_meters);
  int GetNearestRange(int range_meters, int units);

  int GetOverlayCanvasIndex() {
    for (int i = 0; i < CANVAS_COUNT; i++) {
      if (m_overlay_canvas[i].GetValue() > 0) {
        return i;
      }
    }
    return -1;
  }

 private:
  void ResetSpokes();
  void RenderRadarImage2(DrawInfo *di, double radar_scale, double panel_rotate);
  wxString FormatDistance(double distance);
  wxString FormatAngle(double angle);

  int m_previous_auto_range_meters;

  //  wxCriticalSection m_exclusive;  // protects the following two
  DrawInfo m_draw_panel;    // Draw onto our own panel
  DrawInfo m_draw_overlay;  // Abstract painting method

  int m_verbose;
  int m_draw_time_ms;  // Number of millis spent drawing

  wxString m_range_text;

  BlobColour m_trail_colour[TRAIL_MAX_REVOLUTIONS + 1];

  int m_previous_orientation;

  GeoPosition m_radar_position;
};

PLUGIN_END_NAMESPACE

#endif
