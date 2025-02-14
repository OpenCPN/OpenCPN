/***************************************************************************
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
 ***************************************************************************/
/**
 * \file
 * GRIB Weather Data Control Interface.
 *
 * This module provides the primary user interface controls for the GRIB plugin,
 * including:
 * - Timeline controls for navigating forecast times
 * - Layer controls for selecting visible weather parameters
 * - Display settings for customizing visualizations
 * - File management for loading and organizing GRIB data
 *
 * The interface is designed to give users complete control over how weather
 * data is displayed while maintaining efficiency for real-time navigation and
 * animation playback. Key features include:
 * - Temporal interpolation between forecast times
 * - Multi-file data management
 * - Customizable overlay settings
 * - Cursor tracking for data inspection
 * - Animation controls
 */
#ifndef __GRIBUICTRLBAR_H__
#define __GRIBUICTRLBAR_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers
#include <wx/fileconf.h>
#include <wx/glcanvas.h>

#include "GribUIDialogBase.h"
#include "CursorData.h"
#include "GribSettingsDialog.h"
#include "GribRequestDialog.h"
#include "GribReader.h"
#include "GribRecordSet.h"
#include "IsoLine.h"
#include "GrabberWin.h"

#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif

class GRIBUICtrlBar;
class GRIBUICData;
class GRIBFile;
class GRIBRecord;
class GribRecordTree;
class GRIBOverlayFactory;
class GribRecordSet;
class GribRequestSetting;
class GribGrabberWin;
class GribSpacerWin;

class wxFileConfig;
class grib_pi;
class wxGraphicsContext;

WX_DECLARE_OBJARRAY(GribRecordSet, ArrayOfGribRecordSets);

/**
 * Defines the possible states for GRIB area selection.
 * Used to control how the GRIB download area bounds are determined.
 */
enum ZoneSelection {
  AUTO_SELECTION,   //!< Area automatically set from current viewport bounds.
  SAVED_SELECTION,  //!< Area loaded from previously saved coordinates.
  /**
   * User has clicked Shift + Left click and is drawing the bounding box
   * by dragging the mouse.
   */
  START_SELECTION,
  /**
   * Manual mode has been selected.
   * This state is set immediately after the user has clicked the "Manual
   * Selection" checkbox in the GRIB request dialog. The user can now draw a
   * selection box on the chart to specify the download area, or manually enter
   * coordinates in the dialog.
   */
  DRAW_SELECTION,
  /**
   * Selection box completed in manual mode, coordinates have been captured
   * after the user has released the mouse button.
   */
  COMPLETE_SELECTION
};

/// @brief Structure used to store XyGrib configuration. It is used to
/// recover/store model and parameter choices from/to OpenCPN configuration
/// file.
typedef struct {
  int atmModelIndex;
  int waveModelIndex;
  int resolutionIndex;
  int durationIndex;
  int runIndex;
  int intervalIndex;
  bool wind;
  bool gust;
  bool pressure;
  bool temperature;
  bool cape;
  bool reflectivity;
  bool cloudCover;
  bool precipitation;
  bool waveHeight;
  bool windWaves;
} XyGribConfig_t;

/**
 * A specialized GribRecordSet that represents temporally interpolated weather
 * data with isobar rendering optimizations.
 *
 * While GribRecordSet simply holds meteorological parameters at a point in
 * time, GribTimelineRecordSet provides:
 * 1. Temporally interpolated records between two time points.
 * 2. Cached calculations for visualization (e.g., isobars).
 *
 * This class is used when:
 * - Displaying weather conditions between available GRIB timestamps.
 * - Animating weather evolution over time.
 * - Rendering isobars, isotherms, and other derived visualizations.
 *
 * @see GribRecordSet for basic parameter storage.
 * @see GetTimeLineRecordSet() for how interpolation is performed.
 */

class GribTimelineRecordSet : public GribRecordSet {
public:
  /**
   * Creates a timeline record set containing temporally interpolated GRIB
   * records.
   *
   * Timeline record sets store cached data like isobar calculations to optimize
   * rendering performance during animation playback.
   *
   * @param cnt Source GRIB file identifier used to trace record origins
   */
  GribTimelineRecordSet(unsigned int cnt);
  //    GribTimelineRecordSet(GribRecordSet &GRS1, GribRecordSet &GRS2, double
  //    interp_const);
  ~GribTimelineRecordSet();

  void ClearCachedData();

  /**
   * Array of cached isobar calculations for each data type (wind, pressure,
   * etc).
   *
   * Each element is a pointer to a wxArrayPtrVoid containing IsoLine objects.
   * Used to speed up rendering by avoiding recalculation of isobars.
   */
  wxArrayPtrVoid *m_IsobarArray[Idx_COUNT];
};

//----------------------------------------------------------------------------------------------------------
//    GRIB CtrlBar Specification
//----------------------------------------------------------------------------------------------------------
class GRIBUICtrlBar : public GRIBUICtrlBarBase {
  friend class GribRequestSetting;

public:
  GRIBUICtrlBar(wxWindow *parent, wxWindowID id, const wxString &title,
                const wxPoint &pos, const wxSize &size, long style,
                grib_pi *ppi, double scale_factor);
  ~GRIBUICtrlBar();

  void OpenFile(bool newestFile = false);

  void ContextMenuItemCallback(int id);
  void SetFactoryOptions();

  wxDateTime TimelineTime();
  /**
   * Retrieves or creates a temporally interpolated GRIB record set for a
   * specific timestamp.
   *
   * This function performs temporal interpolation between two known timestamps
   * to estimate values at the requested time. The actual data values at each
   * lat/lon point are either:
   * 1. Linearly interpolated in time for scalar values
   * 2. Vector interpolated (2D) for wind and current vectors to maintain
   * physical consistency
   *
   * @note This function does NOT perform spatial (lat/lon) interpolation.
   *
   * @note Timestamp handling:
   *       - For timestamps between forecast times (e.g. 1AM between 12AM and
   * 3AM forecasts), data is interpolated between the bracketing forecasts.
   *       - For timestamps exactly matching forecast times (e.g. 12AM, 3AM),
   *         the original GRIB record is used directly without interpolation to
   * avoid unnecessary computation and maintain precision.
   *
   * @param time The target datetime for which to interpolate GRIB records.
   * @return Pointer to GribTimelineRecordSet containing temporally interpolated
   * data, or NULL if no valid data.
   */
  GribTimelineRecordSet *GetTimeLineRecordSet(wxDateTime time);
  void StopPlayBack();
  void TimelineChanged();
  void CreateActiveFileFromNames(const wxArrayString &filenames);
  void PopulateComboDataList();
  void ComputeBestForecastForNow();
  /** Set the ViewPort under the mouse. */
  void SetViewPortUnderMouse(PlugIn_ViewPort *vp);
  /** Set the ViewPort that has the focus */
  void SetViewPortWithFocus(PlugIn_ViewPort *vp);
  void SetDataBackGroundColor();
  void SetTimeLineMax(bool SetValue);
  void SetCursorLatLon(double lat, double lon);
  /**
   * Schedules an update of the GRIB data values display at current cursor
   * position.
   *
   * When the cursor moves over the chart or timeline changes occur, this method
   * schedules a refresh of the data values panel that shows weather parameters
   * (wind, pressure, temperature etc.) at the cursor location.
   *
   * The update is throttled using a 50ms timer to prevent excessive updates
   * during rapid cursor movement or timeline playback.
   */
  void UpdateTrackingControl();
  void SetDialogsStyleSizePosition(bool force_recompute = false);
  /** Set the icon and tooltip for the download request button. */
  void SetRequestButtonBitmap(int type);
  void OnMouseEvent(wxMouseEvent &event);
  GRIBUICData *GetCDataDialog() { return m_gGRIBUICData; }
  bool InDataPlot(int id) {
    return id > wxID_ANY && id < (int)GribOverlaySettings::GEO_ALTITUDE;
  }
  void SetScaledBitmap(double factor);
  void OpenFileFromJSON(wxString json);

  //
  double getTimeInterpolatedValue(int idx, double lon, double lat,
                                  wxDateTime t);
  bool getTimeInterpolatedValues(double &M, double &A, int idx1, int idx2,
                                 double lon, double lat, wxDateTime t);

  wxWindow *pParent;
  /** Settings that control how GRIB data is displayed and overlaid. */
  GribOverlaySettings m_OverlaySettings;
  /** Current set of GRIB records for timeline playback. */
  GribTimelineRecordSet *m_pTimelineSet;

  /** Timer for controlling GRIB animation playback. */
  wxTimer m_tPlayStop;
  /** Plugin instance that owns this control bar. */
  grib_pi *pPlugIn;
  GribRequestSetting *pReq_Dialog;
  /** Currently active GRIB file being displayed. */
  GRIBFile *m_bGRIBActiveFile;
  bool m_bDataPlot[GribOverlaySettings::GEO_ALTITUDE];  // only for no altitude
                                                        // parameters
  bool m_CDataIsShown;
  int m_ZoneSelAllowed;
  int m_old_DialogStyle;
  void DoZoomToCenter();
  const wxString GetGribDir() {
    if (m_grib_dir.IsEmpty() || !wxDirExists(m_grib_dir)) {
      m_grib_dir = GetpPrivateApplicationDataLocation()
                       ->Append(wxFileName::GetPathSeparator())
                       .Append("grib");

      if (!wxDirExists(m_grib_dir)) wxMkdir(m_grib_dir);

      wxString dir_spec;
      int response = PlatformDirSelectorDialog(
          this, &dir_spec, _("Choose GRIB File Directory"), m_grib_dir);

      if (response == wxID_OK) {
        m_grib_dir = (dir_spec);
      }
    }
    return m_grib_dir;
  }

  /**
   * Gets the projected position of vessel based on current course, speed and
   * forecast time.
   *
   * Calculates the projected latitude/longitude position by advancing the
   * vessel's current position along its course at the specified speed for the
   * time difference between the current forecast time and the vessel's position
   * time.
   *
   * The projected position is then converted to canvas (x,y) coordinates for
   * use in UI overlays and cursor tracking.
   *
   * @param[out] x The projected x-coordinate on canvas in pixels
   * @param[out] y The projected y-coordinate on canvas in pixels
   * @param[in] vp Viewport for coordinate transformation. If NULL, returns
   * (0,0)
   */
  void GetProjectedLatLon(int &x, int &y, PlugIn_ViewPort *vp);
  bool ProjectionEnabled() {
    if (m_ProjectBoatPanel)
      return m_ProjectBoatPanel->ProjectionEnabled();
    else
      return false;
  }
  double m_highlight_latmax;
  double m_highlight_lonmax;
  double m_highlight_latmin;
  double m_highlight_lonmin;
  /** Directory containing GRIB files. */
  wxString m_grib_dir;
  /** List of GRIB filenames being displayed. */
  wxArrayString m_file_names;

private:
  void OnClose(wxCloseEvent &event);
  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);
  void OnSettings(wxCommandEvent &event);
  void OnPlayStop(wxCommandEvent &event);
  void OnPlayStopTimer(wxTimerEvent &event);
  void OnMove(wxMoveEvent &event);
  void OnMenuEvent(wxMenuEvent &event);
  void MenuAppend(wxMenu *menu, int id, wxString label, wxItemKind kind,
                  wxBitmap bitmap = wxNullBitmap, wxMenu *submenu = nullptr);
  void OnZoomToCenterClick(wxCommandEvent &event);
  void OnPrev(wxCommandEvent &event);
  void OnRecordForecast(wxCommandEvent &event) {
    StopPlayBack();
    m_InterpolateMode = false;
    m_pNowMode = false;
    TimelineChanged();
  }
  void OnNext(wxCommandEvent &event);
  void OnNow(wxCommandEvent &event) {
    StopPlayBack();
    ComputeBestForecastForNow();
  }
  void OnAltitude(wxCommandEvent &event);
  void OnOpenFile(wxCommandEvent &event);
  /** Callback invoked when user clicks download/request forecast data. */
  void OnRequestForecastData(wxCommandEvent &event);
  void createRequestDialog();
  void OnCompositeDialog(wxCommandEvent &event);

  void OnTimeline(wxScrollEvent &event);
  void OnShowCursorData(wxCommandEvent &event);

  wxDateTime MinTime();
  wxArrayString GetFilesInDirectory();
  void SetGribTimelineRecordSet(GribTimelineRecordSet *pTimelineSet);
  int GetNearestIndex(wxDateTime time, int model);
  int GetNearestValue(wxDateTime time, int model);
  bool GetGribZoneLimits(GribTimelineRecordSet *timelineSet, double *latmin,
                         double *latmax, double *lonmin, double *lonmax);
  wxDateTime GetNow();
  void RestaureSelectionString();
  void SaveSelectionString() {
    m_SelectionIsSaved = true;
    m_Selection_index = m_cRecordForecast->GetSelection();
    m_Selection_label = m_cRecordForecast->GetString(m_Selection_index);
  }

  //    Data
  CursorData *m_gCursorData;
  GribGrabberWin *m_gGrabber;
  GRIBUICData *m_gGRIBUICData;
  /** ViewPort under the mouse. */
  PlugIn_ViewPort *m_vpMouse;
  int m_lastdatatype;

  int m_TimeLineHours;
  int m_FileIntervalIndex;
  bool m_InterpolateMode;
  bool m_pNowMode;
  bool m_HasAltitude;

  bool m_SelectionIsSaved;
  int m_Selection_index;
  wxString m_Selection_label;
  wxSize m_DialogsOffset;
  double m_projected_lat;
  double m_projected_lon;
  // XyGrib panel configuration
  XyGribConfig_t xyGribConfig;
  bool m_gtk_started;
};

/**
 * Manages multiple GRIB record sets from one or more GRIB files.
 *
 * GRIBFile is responsible for parsing and organizing weather data from GRIB
 * files. It can handle:
 * - Multiple files with different data types (e.g., wind in one file, waves in
 * another).
 * - Multiple files with overlapping data.
 * - Data from different meteorological models and levels.
 *
 * The class provides logic for resolving overlapping or conflicting records by:
 * - Favoring UV vector components over polar (direction/speed) representations.
 * - Preferring mean/average records over instantaneous values.
 * - Using Mean-Sea-Level (MSL) pressure over other pressure types.
 * - Prioritizing significant wave data over wind wave data.
 */
class GRIBFile {
public:
  /**
   * Creates a new GRIBFile by parsing one or more GRIB files.
   *
   * @param file_names Array of GRIB file paths to load. Can contain multiple
   * files with different or overlapping data types.
   * @param CumRec Whether to copy first cumulative record to fill gaps in
   * precipitation and cloud cover data, preventing artificial zero periods.
   * @param WaveRec Whether to copy missing wave records to fill gaps, ensuring
   *                continuous marine condition visualization.
   * @param newestFile When true, only load the newest file from the array.
   *                  When false (default), combine all records from all files.
   */
  GRIBFile(const wxArrayString &file_names, bool CumRec, bool WaveRec,
           bool newestFile = false);
  ~GRIBFile();

  /**
   * Checks if file loading and parsing was successful.
   * @return true if at least one valid GRIB record was loaded.
   */
  bool IsOK(void) { return m_bOK; }
  /**
   * Gets the list of source filenames being used.
   * When newestFile=true, will contain only the newest file.
   * Otherwise contains all input files.
   */
  wxArrayString &GetFileNames(void) { return m_FileNames; }
  /**
   * Gets the last error message if file loading failed.
   */
  wxString GetLastMessage(void) { return m_last_message; }
  /**
   * Gets pointer to array of record sets organized by timestamp.
   * Contains combined data from all source files (or just newest file
   * if newestFile=true).
   */
  ArrayOfGribRecordSets *GetRecordSetArrayPtr(void) {
    return &m_GribRecordSetArray;
  }
  /**
   * Gets reference datetime of the GRIB data.
   *
   * The reference time is when the model run started (analysis time).
   * This differs from the forecast time of individual records which is
   * reference_time + forecast_hour. For example, a 24h forecast from a
   * 00Z model run would have reference_time=00Z and forecast_time=00Z+24h.
   */
  time_t GetRefDateTime(void) { return m_pRefDateTime; }

  const unsigned int GetCounter() { return m_counter; }

  WX_DEFINE_ARRAY_INT(int, GribIdxArray);
  GribIdxArray m_GribIdxArray;

private:
  static unsigned int ID;  //!< Unique identifier counter for GRIBFile instances

  const unsigned int m_counter;  //!< This instance's unique ID
  bool m_bOK;                    //!< Whether file loading succeeded
  wxString m_last_message;       //!< Error message if loading failed
  wxArrayString m_FileNames;     //!< Source GRIB filenames
  GribReader *m_pGribReader;     //!< Parser for GRIB file format
  time_t m_pRefDateTime;         //!< Reference time of the model run

  /** An array of GribRecordSets found in this GRIB file. */
  ArrayOfGribRecordSets m_GribRecordSetArray;

  int m_nGribRecords;
};

//----------------------------------------------------------------------------------------------------------
//    GRIB CursorData Dialog Specification
//----------------------------------------------------------------------------------------------------------
class GRIBUICData : public GRIBUICDataBase {
public:
  GRIBUICData(GRIBUICtrlBar &parent);
  ~GRIBUICData() {}

  // GribGrabberWin      *m_gGrabber;
  GRIBUICtrlBar &m_gpparent;
  CursorData *m_gCursorData;

private:
  void OnMove(wxMoveEvent &event);
};

#endif
