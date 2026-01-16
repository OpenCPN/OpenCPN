
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
 *\file
 *
 * Generic %Chart canvas base.
 */

#ifndef _CHCANV_H__
#define _CHCANV_H__

#include "gl_headers.h"  // Must go before wx/glcanvas

#include <wx/datetime.h>
#include <wx/grid.h>
#include <wx/treectrl.h>

#ifdef ocpnUSE_GL
#include <wx/glcanvas.h>
#endif

#include "model/route.h"
#include "model/route_point.h"
#include "model/select_item.h"

#include "abstract_chart_canv.h"
#include "bbox.h"
#include "canvas_menu.h"
#include "chartdb.h"
#include "chartimg.h"
#include "ch_info_win.h"
#include "compass.h"
#include "emboss_data.h"
#include "gshhs.h"
#include "gl_chart_canvas.h"
#include "gui_lib.h"
#include "idx_entry.h"
#include "mui_bar.h"
#include "notification_manager_gui.h"
#include "observable_evtvar.h"
#include "observable.h"
#include "ocp_cursor.h"
#include "ocpn_pixel.h"
#include "ocpn_plugin.h"
#include "piano.h"
#include "quilt.h"
#include "top_frame.h"
#include "rollover_win.h"
#include "s57_sector.h"
#include "tc_win.h"
#include "undo.h"

WX_DEFINE_ARRAY_PTR(ChartCanvas *, arrayofCanvasPtr);

#ifdef __ANDROID__
#define STAT_FIELD_TICK -1
#define STAT_FIELD_SOGCOG 0
#define STAT_FIELD_CURSOR_LL -1
#define STAT_FIELD_CURSOR_BRGRNG -1
#define STAT_FIELD_SCALE 1
#else
#define STAT_FIELD_TICK 0
#define STAT_FIELD_SOGCOG 1
#define STAT_FIELD_CURSOR_LL 2
#define STAT_FIELD_CURSOR_BRGRNG 3
#define STAT_FIELD_SCALE 4
#endif

class canvasConfig;        // circular
class CanvasMenuHandler;   // circular
class MyFrame;             // circular
class NotificationsList;   // circular
class NotificationButton;  // circular
class Quilt;               // circular
class TCWin;               // circular
class Undo;                // circular

class ChartCanvas;                      // forward
extern ChartCanvas *g_overlayCanvas;    ///< Global instance
extern ChartCanvas *g_focusCanvas;      ///< Global instance
extern arrayofCanvasPtr g_canvasArray;  ///< Global instance

//    Useful static routines
void ShowAISTargetQueryDialog(wxWindow *parent, int mmsi);

//    Set up the preferred quilt type
#define QUILT_TYPE_2

//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------

//--------------------------------------------------------
//    Screen Brightness Control Support Routines
//
//--------------------------------------------------------

int InitScreenBrightness(void);
int RestoreScreenBrightness(void);
int SetScreenBrightness(int brig1Ghtness);

enum  //  specify the render behaviour of SetViewPoint()
{
  CURRENT_RENDER,  // use the current render type
  FORCE_SUBSAMPLE  // force sub-sampled render, with re-render timer
};

//          Cursor region enumerator
enum {
  CENTER,
  MID_RIGHT,
  MID_LEFT,
  MID_TOP,
  MID_BOT,
};

typedef enum ownship_state_t {
  SHIP_NORMAL = 0,
  SHIP_LOWACCURACY,
  SHIP_INVALID
} _ownship_state_t;

enum { ID_S57QUERYTREECTRL = 10000, ID_AISDIALOGOK };

enum {
  ID_PIANO_DISABLE_QUILT_CHART = 32000,
  ID_PIANO_ENABLE_QUILT_CHART,
  ID_PIANO_CONTRACT_PIANO,
  ID_PIANO_EXPAND_PIANO
};

enum { NORTH_UP_MODE, COURSE_UP_MODE, HEAD_UP_MODE };

extern void pupHandler_PasteRoute();  // forward

extern void pupHandler_PasteWaypoint();  // forward

extern void pupHandler_PasteTrack();  // forward

/**
 * ChartCanvas - Main chart display and interaction component
 *
 * Manages the visualization of nautical charts and all chart-related user
 * interactions. This class integrates geographic data, user interface elements,
 * and navigation tools into a comprehensive chart viewing experience. It
 * handles everything from chart loading and rendering to user gestures, route
 * planning, and navigation aids.
 *
 * ChartCanvas uses ViewPort internally to handle the mathematical
 * transformations between geographic and screen coordinates. While ViewPort
 * focuses on the projection math, ChartCanvas manages the actual rendering,
 * user interaction, and application logic.
 *
 * As a wxWindow subclass, it responds to paint, mouse, keyboard, and other
 * window events, translating them into appropriate chart operations like
 * panning, zooming, and object manipulation.
 */
class ChartCanvas : public AbstractChartCanvas {
  friend class glChartCanvas;

public:
  ChartCanvas(wxFrame *frame, int canvasIndex, wxWindow *nmea_log);
  ~ChartCanvas();

  virtual void ReloadVP(bool b_adjust = true) override;

  /** Return ViewPort scale factor, in physical pixels per meter. */
  float GetVPScale() override { return GetVP().view_scale_ppm; }

  bool Show(bool show = true) override { return wxWindow::Show(show); }
  double GetCanvasRangeMeters() override;
  int GetENCDisplayCategory() override { return m_encDisplayCategory; }
  void SetCanvasRangeMeters(double range) override;
  wxBitmap *GetScratchBitmap() const override { return pscratch_bm; }
  void ResetGridFont() override { m_pgridFont = nullptr; }
  void ResetGlGridFont() override;
  void EnablePaint(bool b_enable) override;
  bool CanAccelerateGlPanning() override;
  void SetupGlCompression() override;

  void TriggerDeferredFocus() override;
  void Refresh(bool eraseBackground = true,
               const wxRect *rect = nullptr) override;
  wxWindow *GetWindow() override { return this; }
  double GetScaleValue() override { return m_scaleValue; }

  void SetupGlCanvas();

  //    Methods
  void OnKeyDown(wxKeyEvent &event);
  void OnKeyUp(wxKeyEvent &event);
  void OnKeyChar(wxKeyEvent &event);
  void OnPaint(wxPaintEvent &event);
  void OnToolLeftClick(wxCommandEvent &event);
  void PaintCleanup();
  void Scroll(int dx, int dy);

  bool MouseEventOverlayWindows(wxMouseEvent &event);
  bool MouseEventChartBar(wxMouseEvent &event);
  bool MouseEventMUIBar(wxMouseEvent &event);
  bool MouseEventToolbar(wxMouseEvent &event);
  bool MouseEventIENCBar(wxMouseEvent &event);

  bool MouseEventSetup(wxMouseEvent &event, bool b_handle_dclick = true);
  bool MouseEventProcessObjects(wxMouseEvent &event);
  /**
   * Processes mouse events for core chart panning and zooming operations.
   *
   * This method handles the main chart navigation interactions:
   * - Mouse wheel zooming with configurable sensitivity and optional smoothing.
   * - Left-click chart centering. Centers chart on click point in non-drag
   * scenarios.
   * - Click-and-drag panning. Updates chart position continuously during drag.
   *
   * @param event The mouse event to process
   * @return true if event was processed, false to allow focus shifting between
   * canvases.
   *
   * @note Interacts with route creation and measurement tools active states
   */
  bool MouseEventProcessCanvas(wxMouseEvent &event);
  void SetCanvasCursor(wxMouseEvent &event);
  void OnKillFocus(wxFocusEvent &WXUNUSED(event));
  void OnSetFocus(wxFocusEvent &WXUNUSED(event));
#ifdef HAVE_WX_GESTURE_EVENTS
  void OnZoom(wxZoomGestureEvent &event);
  void OnLongPress(wxLongPressEvent &event);
  void OnPressAndTap(wxPressAndTapEvent &event);

  void OnLeftDown(wxMouseEvent &evt);
  void OnLeftUp(wxMouseEvent &evt);

  void OnRightUp(wxMouseEvent &event);
  void OnRightDown(wxMouseEvent &event);

  void OnDoubleLeftClick(wxMouseEvent &event);

  void OnWheel(wxMouseEvent &event);
  void OnMotion(wxMouseEvent &event);
#endif /* HAVE_WX_GESTURE_EVENTS */

  void PopupMenuHandler(wxCommandEvent &event);
  bool IsPrimaryCanvas() { return (m_canvasIndex == 0); }

  bool SetUserOwnship();

  bool SetCursor(const wxCursor &c) override;
  void Update() override;

  void LostMouseCapture(wxMouseCaptureLostEvent &event);

  void CancelMouseRoute();
  /**
   * Set the width of the screen in millimeters.
   */
  void SetDisplaySizeMM(double size);
  /**
   * Get the width of the screen in millimeters.
   */
  double GetDisplaySizeMM() { return m_display_size_mm; }

  /**
   * Sets the viewport scale while maintaining the center point.
   *
   * Changes the chart display scale while preserving the current view center,
   * orientation, skew, and projection settings. This is typically used for zoom
   * operations.
   *
   * @param scale The new viewport scale to set (pixels per meter)
   * @param refresh Whether to refresh the display after changing the scale
   * @return bool True if the scale change was successful
   */
  bool SetVPScale(double sc, bool b_refresh = true);
  bool SetVPProjection(int projection);
  /**
   * Centers the view on a specific lat/lon position.
   *
   * @param lat Latitude in degrees
   * @param lon Longitude in degrees
   * @return true if view was changed successfully
   */
  bool SetViewPoint(double lat, double lon);
  bool SetViewPointByCorners(double latSW, double lonSW, double latNE,
                             double lonNE);
  /**
   * Set the viewport center point, scale, skew, rotation and projection.
   *
   * @param lat Latitude of viewport center in degrees.
   * @param lon Longitude of viewport center in degrees.
   * @param scale_ppm Requested viewport scale in physical pixels per meter.
   * This is the desired rendering scale before projection effects.
   * @param skew Chart skew angle in radians.
   * @param rotation Viewport rotation in radians.
   * @param projection Projection type (default=0). If 0, maintains current
   * projection.
   * @param b_adjust Allow small viewport adjustments for performance.
   * @param b_refresh Request screen refresh after change.
   */
  bool SetViewPoint(double lat, double lon, double scale_ppm, double skew,
                    double rotation, int projection = 0, bool b_adjust = true,
                    bool b_refresh = true);
  void LoadVP(ViewPort &vp, bool b_adjust = true);

  ChartStack *GetpCurrentStack() { return m_pCurrentStack; }
  void SetGroupIndex(int index, bool autoswitch = false);
  bool CheckGroup(int igroup);
  void canvasRefreshGroupIndex(void);
  void canvasChartsRefresh(int dbi_hint);

  void CheckGroupValid(bool showMessage = true, bool switchGroup0 = true);

  void UpdateCanvasS52PLIBConfig();

  void OnDeferredFocusTimerEvent(wxTimerEvent &event);
  void OnRouteFinishTimerEvent(wxTimerEvent &event);

  void ClearS52PLIBStateHash() { m_s52StateHash = 0; }
  void SetupCanvasQuiltMode(void);
  void ApplyCanvasConfig(canvasConfig *pcc);

  bool SetVPRotation(double angle);
  double GetVPRotation(void) { return GetVP().rotation; }
  double GetVPSkew(void) { return GetVP().skew; }
  double GetVPTilt(void) { return GetVP().tilt; }

  void ClearbFollow(void);
  void SetbFollow(void);
  void TogglebFollow(void);
  bool GetbFollow() { return m_bFollow; }

  void JumpToPosition(double lat, double lon, double scale);
  void SetFirstAuto(bool b_auto) { m_bFirstAuto = b_auto; }
  void SetAbsoluteMinScale(double min_scale);
  std::shared_ptr<HostApi121::PiPointContext> GetCanvasContextAtPoint(int x,
                                                                      int y);

  /**
   * Convert latitude/longitude to canvas pixel coordinates (physical pixels)
   * with double precision.
   *
   * Returns unrounded floating point pixel coordinates. When used with drawing
   * functions that take integer coordinates, values will be truncated.
   *
   * @param rlat Latitude in degrees
   * @param rlon Longitude in degrees
   * @param r [out] Pointer to wxPoint2DDouble to receive the canvas pixel
   * coordinates as unrounded floating point values
   */
  void GetDoubleCanvasPointPix(double rlat, double rlon, wxPoint2DDouble *r);
  /**
   * Convert latitude/longitude to canvas pixel coordinates (physical pixels)
   * with double precision, using specified viewport.
   *
   * Returns unrounded floating point pixel coordinates. When used with drawing
   * functions that take integer coordinates, values will be truncated.
   *
   * @param vp ViewPort containing projection parameters and canvas settings
   * @param rlat Latitude in degrees
   * @param rlon Longitude in degrees
   * @param r [out] Pointer to wxPoint2DDouble to receive the canvas pixel
   * coordinates as unrounded floating point values
   */
  void GetDoubleCanvasPointPixVP(ViewPort &vp, double rlat, double rlon,
                                 wxPoint2DDouble *r);

  /**
   * Convert latitude/longitude to canvas pixel coordinates (physical pixels)
   * rounded to nearest integer.
   *
   * Uses GetDoubleCanvasPointPixVP internally and rounds results using wxRound
   * (std::lround). This means 3.7 becomes 4, -3.7 becomes -4.
   *
   * @param rlat Latitude in degrees
   * @param rlon Longitude in degrees
   * @param r [out] Pointer to wxPoint to receive the canvas pixel coordinates
   * in physical pixels, as rounded integer values.
   * @return true if conversion successful, false if coordinates are invalid or
   * out of bounds.
   *
   * @see ViewPort::GetPixFromLL() for the underlying coordinate transformation.
   */
  bool GetCanvasPointPix(double rlat, double rlon, wxPoint *r);

  /**
   * Convert latitude/longitude to canvas pixel coordinates rounded to nearest
   * integer using specified viewport.
   *
   * Uses GetDoubleCanvasPointPixVP internally and rounds results using wxRound
   * (std::lround). This means 3.7 becomes 4, -3.7 becomes -4.
   *
   * @param vp ViewPort containing projection parameters and canvas settings
   * @param rlat Latitude in degrees
   * @param rlon Longitude in degrees
   * @param r [out] Pointer to wxPoint to receive the canvas pixel coordinates
   * in physical pixels, as rounded integer values.
   * @return true if conversion successful, false if:
   *         - Coordinates would be on other side of the world (resulting in
   * NaN)
   *         - Resulting pixel values would be too large (>1e6)
   *         In these cases, r is set to INVALID_COORD
   *
   * @see ViewPort::GetPixFromLL() for the underlying coordinate transformation
   */
  bool GetCanvasPointPixVP(ViewPort &vp, double rlat, double rlon, wxPoint *r);

  /**
   * Convert canvas pixel coordinates (physical pixels) to latitude/longitude.
   *
   * Uses BSB chart geo-referencing for compatible raster charts when conditions
   * permit, otherwise falls back to viewport projection estimation.
   *
   * @param x X-coordinate in physical pixels
   * @param y Y-coordinate in physical pixels
   * @param lat [out] Reference to receive the resulting latitude at the given
   * (x, y) coordinates.
   * @param lon [out] Reference to receive the resulting longitude at the given
   * (x, y) coordinates.
   *
   * @see ViewPort::GetLLFromPix() for the underlying coordinate transformation.
   */
  void GetCanvasPixPoint(double x, double y, double &lat, double &lon);
  void WarpPointerDeferred(int x, int y);
  void UpdateShips();
  void UpdateAIS();
  void UpdateAlerts();  // pjotrc 2010.02.22
  void ToggleCPAWarn();

  bool IsMeasureActive() { return m_bMeasure_Active; }
  wxBitmap &GetTideBitmap() { return m_cTideBitmap; }
  Undo *undo;

  int GetUpMode() { return m_upMode; }
  bool GetLookahead() { return m_bLookAhead; }

  void UnlockQuilt();
  void SetQuiltMode(bool b_quilt);
  bool GetQuiltMode(void);
  std::vector<int> GetQuiltIndexArray(void);
  bool IsQuiltDelta(void);
  void SetQuiltChartHiLiteIndex(int dbIndex);
  void SetQuiltChartHiLiteIndexArray(std::vector<int> hilite_array);
  void ClearQuiltChartHiLiteIndexArray();
  int GetQuiltReferenceChartIndex(void);
  double GetBestStartScale(int dbi_hint, const ViewPort &vp);
  void ConfigureChartBar();

  int GetNextContextMenuId();

  TCWin *getTCWin() { return pCwin; }

  bool StartTimedMovement(bool stoptimer = true);
  void DoTimedMovement();
  /**
   * Performs a step of smooth movement animation on the chart canvas.
   *
   * This function is called to update the canvas position, scale, or rotation
   * during animated panning, zooming, or rotating operations. The amount of
   * movement performed is determined by the elapsed time parameter @p dt, and
   * the operation continues as long as the internal movement timer is nonzero.
   *
   * @param dt Elapsed time in milliseconds since the last movement step.
   *
   * @see StartTimedMovement(), StopMovement(), m_mustmove
   */
  void DoMovement(long dt);
  void StopMovement();

  void StartTimedMovementVP(double target_lat, double target_lon, int nstep);
  void DoTimedMovementVP();
  void StopMovementVP();

  void StartTimedMovementTarget();
  void DoTimedMovementTarget();
  void StopMovementTarget();

  void SetColorScheme(ColorScheme cs);
  ColorScheme GetColorScheme() { return m_cs; }

  void CanvasApplyLocale();
  void RebuildCursors();

  //    Accessors
  int GetCanvasWidth() { return m_canvas_width; }
  int GetCanvasHeight() { return m_canvas_height; }
  /** Return the ViewPort chart scale denominator (e.g., 50000 for a 1:50000
   * scale). */
  float GetVPChartScale() { return GetVP().chart_scale; }
  /**
   * Return the number of logical pixels per meter for the screen.
   *
   * @todo The name of this function is misleading. It should be renamed
   * tgui/include/gui/quilt.ho GetCanvasLogicalPixelsPerMeter() or similar. It
   * looks like some callers are expecting the physical pixels per meter, which
   * is incorrect.
   */
  double GetCanvasScaleFactor() { return m_canvas_scale_factor; }
  /*chcanv*
   * Return the physical pixels per meter at chart center, accounting for
   * latitude distortion.
   */
  double GetCanvasTrueScale() { return m_true_scale_ppm; }
  double GetAbsoluteMinScalePpm() { return m_absolute_min_scale_ppm; }
  ViewPort *GetpVP() { return &VPoint; }
  void SetVP(ViewPort &);
  ChartBase *GetChartAtCursor();
  ChartBase *GetOverlayChartAtCursor();
  Piano *GetPiano() { return m_Piano; }
  int GetPianoHeight();

  bool isRouteEditing(void) {
    return m_bRouteEditing && m_pRoutePointEditTarget;
  }
  bool isMarkEditing(void) { return m_bMarkEditing && m_pRoutePointEditTarget; }

  GSHHSChart *GetWorldBackgroundChart() { return pWorldBackgroundChart; }
  void ResetWorldBackgroundChart() { pWorldBackgroundChart->Reset(); }

  void SetbTCUpdate(bool f) { m_bTCupdate = f; }
  bool GetbTCUpdate() { return m_bTCupdate; }
  void SetbShowCurrent(bool f) { m_bShowCurrent = f; }
  bool GetbShowCurrent() { return m_bShowCurrent; }
  void SetbShowTide(bool f) { m_bShowTide = f; }
  bool GetbShowTide() { return m_bShowTide; }
  void SetShowVisibleSectors(bool val) { m_bShowVisibleSectors = val; }
  bool GetShowVisibleSectors() { return m_bShowVisibleSectors; }
  /** Get the number of logical pixels per millimeter on the screen. */
  double GetPixPerMM() { return m_pix_per_mm; }

  void SetOwnShipState(ownship_state_t state) { m_ownship_state = state; }
  void SetCursorStatus(double cursor_lat, double cursor_lon);
  void GetCursorLatLon(double *lat, double *lon);
  /** Pans (moves) the canvas by the specified physical pixels in x and y
   * directions. */
  bool PanCanvas(double dx, double dy);
  void StopAutoPan(void);
  bool IsOwnshipOnScreen();
  void DisableQuiltAdjustOnZoom(bool disable) {
    m_disable_adjust_on_zoom = disable;
  }

  /**
   * Perform a smooth zoom operation on the chart canvas by the specified
   * factor.
   *
   * The zoom can either be centered on the cursor position or the center of the
   * viewport.
   *
   * @param factor The zoom factor to apply:
   *              - factor > 1: Zoom in, e.g. 2.0 makes objects twice as large
   *              - factor < 1: Zoom out, e.g. 0.5 makes objects half as large
   * @param can_zoom_to_cursor If true, zoom operation will be centered on
   * current cursor position. If false, zoom operation centers on current
   * viewport center.
   * @param stoptimer If true, stops any ongoing movement/zoom operations before
   * starting this zoom. If false, allows this zoom to blend with existing
   * operations.
   */

  void ZoomCanvas(double factor, bool can_zoom_to_cursor = true,
                  bool stoptimer = true);

  /**
   * Perform an immediate zoom operation without smooth transitions.
   *
   * This is a simplified version of ZoomCanvas that applies the zoom
   * immediately without animation or cursor-centering logic. Typically used for
   * programmatic zooming.
   *
   * @param factor The zoom factor to apply:
   *              - factor > 1: Zoom in, e.g. 2.0 makes objects twice as large
   *             chcanv - factor < 1: Zoom out, e.g. 0.5 makes objects half as
   * large
   */
  void ZoomCanvasSimple(double factor);

  void RotateCanvas(double dir);
  void DoRotateCanvas(double rotation);
  void DoTiltCanvas(double tilt);

  void ShowAISTargetList(void);

  void ShowGoToPosition(void);
  void HideGlobalToolbar();
  void ShowGlobalToolbar();

  bool GetShowDepthUnits() { return m_bShowDepthUnits; }
  void SetShowDepthUnits(bool show) { m_bShowDepthUnits = show; }
  bool GetShowGrid() { return m_bDisplayGrid; }
  void SetShowGrid(bool show) { m_bDisplayGrid = show; }
  bool GetShowOutlines() { return m_bShowOutlines; }
  void SetShowOutlines(bool show) { m_bShowOutlines = show; }
  bool GetShowChartbar() { return true; }
  wxRect GetMUIBarRect();
  void SetMUIBarPosition();
  void DestroyMuiBar();
  void CreateMUIBar();

  void ToggleChartOutlines(void);
  void ToggleCanvasQuiltMode(void);

  wxString GetScaleText() { return m_scaleText; }
  bool GetShowAIS() { return m_bShowAIS; }
  void SetShowAIS(bool show);
  bool GetAttenAIS() { return m_bShowAISScaled; }
  void SetAttenAIS(bool show);
  void SetShowFocusBar(bool enable) { m_show_focus_bar = enable; }
  bool GetShowFocusBar() { return m_show_focus_bar; }
  MUIBar *GetMUIBar() { return m_muiBar; }
  void SetAlertString(wxString str) { m_alertString = str; }
  wxString GetAlertString() { return m_alertString; }
  bool GetShowENCText() { return m_encShowText; }
  void SetShowENCText(bool show);

  bool GetShowENCDepth() { return m_encShowDepth; }
  void SetShowENCDepth(bool show);

  bool GetShowENCLightDesc() { return m_encShowLightDesc; }
  void SetShowENCLightDesc(bool show);

  bool GetShowENCBuoyLabels() { return m_encShowBuoyLabels; }
  void SetShowENCBuoyLabels(bool show);

  bool GetShowENCLights() { return m_encShowLights; }
  void SetShowENCLights(bool show);

  void SetENCDisplayCategory(int category);

  bool GetShowENCAnchor() { return m_encShowAnchor; }
  void SetShowENCAnchor(bool show);

  bool GetShowENCDataQual() { return m_encShowDataQual; }
  void SetShowENCDataQual(bool show);

  void JaggyCircle(ocpnDC &dc, wxPen pen, int x, int y, int radius);
  int m_canvasIndex;
  void ShowTides(bool bShow);
  void ShowCurrents(bool bShow);
  void SetUpMode(int mode);
  void ToggleLookahead();
  void SetShowGPS(bool show);
  void UpdateFollowButtonState(void);
  void InvalidateGL();
  bool IsTileOverlayIndexInYesShow(int index);
  bool IsTileOverlayIndexInNoShow(int index);
  void AddTileOverlayIndexToNoShow(int index);
  int m_groupIndex;
  Route *m_pMouseRoute;
  bool m_bMeasure_Active;
  ViewPort &GetVP();
  ChartBase *m_singleChart;
  Quilt *m_pQuilt;
  wxString FindValidUploadPort();
  wxString m_active_upload_port;

  // protected:

  // private:
  ChartBase *GetLargestScaleQuiltChart();
  ChartBase *GetFirstQuiltChart();
  ChartBase *GetNextQuiltChart();
  int GetQuiltChartCount();
  void InvalidateAllQuiltPatchs(void);
  void SetQuiltRefChart(int dbIndex);
  std::vector<int> GetQuiltCandidatedbIndexArray(bool flag1 = true,
                                                 bool flag2 = true);
  std::vector<int> &GetQuiltExtendedStackdbIndexArray();
  std::vector<int> &GetQuiltFullScreendbIndexArray();
  std::vector<int> GetQuiltEclipsedStackdbIndexArray();
  int GetQuiltRefChartdbIndex(void);
  void InvalidateQuilt(void);
  double GetQuiltMaxErrorFactor();
  bool IsChartQuiltableRef(int db_index);
  bool IsChartLargeEnoughToRender(ChartBase *chart, ViewPort &vp);
  int GetCanvasChartNativeScale();
  int FindClosestCanvasChartdbIndex(int scale);
  void UpdateCanvasOnGroupChange(void);

  void ShowObjectQueryWindow(int x, int y, float zlat, float zlon);
  void ShowMarkPropertiesDialog(RoutePoint *markPoint);
  void ShowRoutePropertiesDialog(wxString title, Route *selected);
  void ShowTrackPropertiesDialog(Track *selected);
  /** Legacy tide dialog creation method. Redirects to ShowSingleTideDialog for
   * single-instance behavior. */
  void DrawTCWindow(int x, int y, void *pIDX);

  /**
   * Display tide/current dialog with single-instance management.
   *
   * Handles the following scenarios:
   * - If no dialog exists: Creates new dialog
   * - If same station clicked: Brings existing dialog to front with visual
   * feedback
   * - If different station clicked: Closes current dialog and opens new one
   * @param x Mouse click x-coordinate in canvas pixels
   * @param y Mouse click y-coordinate in canvas pixels
   * @param pvIDX Pointer to IDX_entry for the tide/current station
   */
  void ShowSingleTideDialog(int x, int y, void *pvIDX);

  /** @return true if a tide dialog is currently open and visible */
  bool IsTideDialogOpen() const;

  /** Close any open tide dialog */
  void CloseTideDialog();

  void UpdateGPSCompassStatusBox(bool b_force_new);
  ocpnCompass *GetCompass() { return m_Compass; }

  wxColour GetFogColor() { return m_fog_color; }

  void ShowChartInfoWindow(int x, int dbIndex);
  void HideChartInfoWindow(void);
  void ShowCompositeInfoWindow(int x, int n_charts, int scale,
                               const std::vector<int> &index_vector);

  void StartMeasureRoute();
  void CancelMeasureRoute();

  bool DoCanvasUpdate(void);
  void SelectQuiltRefdbChart(int db_index, bool b_autoscale = true);
  void SelectQuiltRefChart(int selected_index);
  double GetBestVPScale(ChartBase *pchart);
  void selectCanvasChartDisplay(int type, int family);
  void RemoveChartFromQuilt(int dbIndex);

  void HandlePianoClick(int selected_index,
                        const std::vector<int> &selected_dbIndex_array);
  void HandlePianoRClick(int x, int y, int selected_index,
                         const std::vector<int> &selected_dbIndex_array);
  void HandlePianoRollover(int selected_index,
                           const std::vector<int> &selected_dbIndex_array,
                           int n_charts, int scale);
  void ClearPianoRollover();
  void UpdateCanvasControlBar(void);
  void FormatPianoKeys(void);
  void PianoPopupMenu(int x, int y, int selected_index,
                      const std::vector<int> &selected_dbIndex_array);
  void OnPianoMenuDisableChart(wxCommandEvent &event);
  void OnPianoMenuEnableChart(wxCommandEvent &event);

  bool IsPianoContextMenuActive() { return m_piano_ctx_menu != 0; }
  bool DoCanvasCOGSet(void);
  void ApplyGlobalSettings();
  void SetShowGPSCompassWindow(bool bshow);
  bool GetShowGPSCompassWindow() { return m_bShowCompassWin; }
  void FreezePiano() { m_pianoFrozen = true; }
  void ThawPiano() { m_pianoFrozen = false; }
  void StartChartDragInertia();
  void SetupGridFont();

  // Todo build more accessors
  bool m_bFollow;
  wxCursor *pCursorPencil;
  wxCursor *pCursorArrow;
  wxCursor *pCursorCross;
  wxCursor *pPlugIn_Cursor;
  TCWin *pCwin;
  bool m_brepaint_piano;
  /**
   * The longitude in degrees corresponding to the most recently processed
   * cursor position.
   *
   * This variable does NOT continuously track the mouse cursor position. It is
   * updated only:
   * 1. When processing left mouse clicks
   * 2. During panning operations via mouse dragging
   * 3. When the OnCursorTrackTimerEvent fires after mouse movement stops
   * 4. During certain object selection and editing operations
   *
   * For code that needs the current geographic coordinates under the current
   * mouse pointer, use GetCanvasPixPoint(mouse_x, mouse_y, lat, lon) instead of
   * accessing this variable directly.
   */
  double m_cursor_lon;
  /**
   * The latitude in degrees corresponding to the most recently processed cursor
   * position.
   *
   * This variable does NOT continuously track the mouse cursor position. It is
   * updated only:
   * 1. When processing left mouse clicks
   * 2. During panning operations via mouse dragging
   * 3. When the OnCursorTrackTimerEvent fires after mouse movement stops
   * 4. During certain object selection and editing operations
   *
   * For code that needs the current geographic coordinates under the current
   * mouse pointer, use GetCanvasPixPoint(mouse_x, mouse_y, lat, lon) instead of
   * accessing this variable directly.
   */
  double m_cursor_lat;
  wxPoint r_rband;
  double m_prev_rlat;
  double m_prev_rlon;
  RoutePoint *m_prev_pMousePoint;
  bool m_bShowNavobjects;
  int m_routeState;
  int m_upMode;
  bool m_bLookAhead;

#ifdef HAVE_WX_GESTURE_EVENTS
  double m_oldVPSScale;
  bool m_popupWanted;
  bool m_leftdown;
  wxPoint m_zoomStartPoint;
#endif /* HAVE_WX_GESTURE_EVENTS */

  void DrawBlinkObjects(void);

  void StartRoute(void);
  wxString FinishRoute(void);

#ifdef ocpnUSE_GL
  glChartCanvas *GetglCanvas() { return m_glcc; }
#endif

  bool CheckEdgePan(int x, int y, bool bdragging, int margin, int delta);

  bool m_FinishRouteOnKillFocus;
  bool m_bMeasure_DistCircle;
  bool m_bAppendingRoute;
  int m_nMeasureState;
  Route *m_pMeasureRoute;
  AbstractTopFrame *parent_frame;
  CanvasMenuHandler *m_canvasMenu;
  int GetMinAvailableGshhgQuality() {
    return pWorldBackgroundChart->GetMinAvailableQuality();
  }
  int GetMaxAvailableGshhgQuality() {
    return pWorldBackgroundChart->GetMaxAvailableQuality();
  }
  Route *GetSelectedRoute() const { return m_pSelectedRoute; }
  Track *GetSelectedTrack() const { return m_pSelectedTrack; }
  RoutePoint *GetSelectedRoutePoint() const { return m_pFoundRoutePoint; }

  void SetAISCanvasDisplayStyle(int StyleIndx);
  void TouchAISToolActive(void);
  void UpdateAISTBTool(void);

  void SelectChartFromStack(int index, bool bDir = false,
                            ChartTypeEnum New_Type = CHART_TYPE_DONTCARE,
                            ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
  void SelectdbChart(int dbindex);

  void DoCanvasStackDelta(int direction);

  void ProcessNewGUIScale();

  bool m_b_paint_enable;

  wxRect GetScaleBarRect() { return m_scaleBarRect; }
  void RenderAlertMessage(wxDC &dc, const ViewPort &vp);

  std::vector<int> m_tile_noshow_index_array;
  std::vector<int> m_tile_yesshow_index_array;
  std::vector<int> m_quilt_noshow_index_array;

  std::vector<int> GetQuiltNoshowIindexArray() {
    return m_quilt_noshow_index_array;
  }
  /**
   * Get the ratio of physical to logical pixel for the display.
   *
   * On standard displays, one logical pixel equals one physical pixel, so this
   * value is 1.0. On high-DPI/Retina displays, one logical pixel may equal
   * multiple physical pixels:
   * - MacBook Pro Retina: 2.0 (2x2 physical pixels per logical pixel)
   * - Other HiDPI displays: May be 1.5, 1.75, etc.
   */
  double GetDisplayScale() { return m_displayScale; }
  void ResetOwnshipOffset() { m_OSoffsetx = m_OSoffsety = 0; }
  NotificationsList *GetNotificationsList() { return m_NotificationsList; }

  int PrepareContextSelections(double lat, double lon);

  RoutePoint *GetFoundRoutepoint() { return m_pFoundRoutePoint; }

  /**
   * Notified with message targeting all plugins. Contains a message type
   * string and a wxJSONValue shared_ptr.
   */
  EventVar json_msg;

  bool m_inPinch;

private:
  /**
   * Internal function that implements the actual zoom operation.
   *
   * This function handles the core zoom functionality including scale
   * calculations, chart selection and viewport updates.
   *
   * @param factor The zoom factor to apply:
   *              - factor > 1: Zoom in, e.g. 2.0 makes objects twice as large
   *              - factor < 1: Zoom out, e.g. 0.5 makes objects half as large
   *
   * @param can_zoom_to_cursor If true, zoom operation will be centered on
   * cursor position. If false, zoom operation centers on viewport center.
   */
  void DoZoomCanvas(double factor, bool can_zoom_to_cursor = true);

  int AdjustQuiltRefChart();
  bool UpdateS52State();
  void CallPopupMenu(int x, int y);
  bool IsTempMenuBarEnabled();
  bool InvokeCanvasMenu(int x, int y, int seltype);
  void OnMenuTimer(wxTimerEvent &event);
  wxTimer m_menuTimer;
  wxPoint m_menuPos;
  bool m_inLongPress;

  wxBitmap *pscratch_bm;
  ViewPort VPoint;
  void PositionConsole(void);
  wxWindow *m_nmea_log;

  wxColour PredColor();
  wxColour ShipColor();

  void ComputeShipScaleFactor(float icon_hdt, int ownShipWidth,
                              int ownShipLength, wxPoint2DDouble &lShipMidPoint,
                              wxPoint &GpsOffsetPixels,
                              wxPoint2DDouble lGPSPoint, float &scale_factor_x,
                              float &scale_factor_y);

  void ShipDrawLargeScale(ocpnDC &dc, wxPoint2DDouble lShipMidPoint);
  void ShipIndicatorsDraw(ocpnDC &dc, int img_height, wxPoint GPSOffsetPixels,
                          wxPoint2DDouble lGPSPoint);

  ChInfoWin *m_pCIWin;

  bool m_bShowCurrent;
  bool m_bShowTide;
  int cursor_region;
  bool m_bTCupdate;
  wxString m_scaleText;
  double m_scaleValue;
  bool m_bShowScaleInStatusBar;
  wxRect bbRect;

  wxPoint LastShipPoint;
  wxPoint LastPredPoint;
  bool m_bDrawingRoute;
  bool m_bRouteEditing;
  bool m_bMarkEditing;
  bool m_bRoutePoinDragging;
  bool m_bIsInRadius;
  bool m_bMayToggleMenuBar;

  RoutePoint *m_pRoutePointEditTarget;
  RoutePoint *m_lastRoutePointEditTarget;
  SelectItem *m_pFoundPoint;
  bool m_bChartDragging;
  Route *m_pSelectedRoute;
  Track *m_pSelectedTrack;
  wxArrayPtrVoid *m_pEditRouteArray;
  RoutePoint *m_pFoundRoutePoint;

  int m_FoundAIS_MMSI;

  wxCursor *pCursorLeft;
  wxCursor *pCursorRight;
  wxCursor *pCursorUp;
  wxCursor *pCursorDown;

  wxCursor *pCursorUpLeft;
  wxCursor *pCursorUpRight;
  wxCursor *pCursorDownLeft;
  wxCursor *pCursorDownRight;

  int popx, popy;

  wxBitmap *pThumbDIBShow;
  wxBitmap *pThumbShowing;

  bool bShowingCurrent;
  bool bShowingTide;

  /**
   * Display-specific scale factor in logical pixels per meter for the physical
   * screen.
   *
   * @note This is the same as m_pix_per_mm / 1000.0
   */
  double m_canvas_scale_factor;
  /** Number of logical pixels per millimeter on the screen. */
  double m_pix_per_mm;
  double m_display_size_mm;

  double m_absolute_min_scale_ppm;

  bool singleClickEventIsValid;
  wxMouseEvent singleClickEvent;

  std::vector<s57Sector_t> extendedSectorLegs;
  wxFont m_overzoomFont;
  int m_overzoomTextWidth;
  int m_overzoomTextHeight;

  //    Methods
  void OnActivate(wxActivateEvent &event);
  void OnSize(wxSizeEvent &event);
  void MouseTimedEvent(wxTimerEvent &event);
  void MouseEvent(wxMouseEvent &event);
  void ShipDraw(ocpnDC &dc);
  void DrawArrow(ocpnDC &dc, int x, int y, double rot_angle, double scale);
  void OnRolloverPopupTimerEvent(wxTimerEvent &event);
  void FindRoutePointsAtCursor(float selectRadius, bool setBeingEdited);

  void RotateTimerEvent(wxTimerEvent &event);
  void PanTimerEvent(wxTimerEvent &event);
  void MovementTimerEvent(wxTimerEvent &);
  void MovementStopTimerEvent(wxTimerEvent &);
  void OnCursorTrackTimerEvent(wxTimerEvent &event);

  void MovementVPTimerEvent(wxTimerEvent &event);

  void DrawAllTracksInBBox(ocpnDC &dc, LLBBox &BltBBox);
  void DrawActiveTrackInBBox(ocpnDC &dc, LLBBox &BltBBox);
  void DrawAllRoutesInBBox(ocpnDC &dc, LLBBox &BltBBox);
  void DrawActiveRouteInBBox(ocpnDC &dc, LLBBox &BltBBox);
  void DrawAllWaypointsInBBox(ocpnDC &dc, LLBBox &BltBBox);
  void DrawAnchorWatchPoints(ocpnDC &dc);
  double GetAnchorWatchRadiusPixels(RoutePoint *pAnchorWatchPoint);

  void DrawAllTidesInBBox(ocpnDC &dc, LLBBox &BBox);
  void DrawAllCurrentsInBBox(ocpnDC &dc, LLBBox &BBox);
  void RebuildTideSelectList(LLBBox &BBox);
  void RebuildCurrentSelectList(LLBBox &BBox);
  IDX_entry *FindBestCurrentObject(double lat, double lon);

  void RenderAllChartOutlines(ocpnDC &dc, ViewPort &vp);
  void RenderChartOutline(ocpnDC &dc, int dbIndex, ViewPort &vp);
  void RenderRouteLegs(ocpnDC &dc);
  void RenderVisibleSectorLights(ocpnDC &dc);

  void AlertDraw(ocpnDC &dc);  // pjotrc 2010.02.22

  void GridDraw(ocpnDC &dc);  // Display lat/lon Grid in chart display
  void ScaleBarDraw(ocpnDC &dc);

  void DrawOverlayObjects(ocpnDC &dc, const wxRegion &ru);
  void RenderShipToActive(ocpnDC &dc, bool Use_Opengl);

  emboss_data *EmbossDepthScale();
  emboss_data *CreateEmbossMapData(wxFont &font, int width, int height,
                                   const wxString &str, ColorScheme cs);
  void CreateDepthUnitEmbossMaps(ColorScheme cs);
  wxBitmap CreateDimBitmap(wxBitmap &Bitmap, double factor);

  void CreateOZEmbossMapData(ColorScheme cs);
  emboss_data *EmbossOverzoomIndicator(ocpnDC &dc);
  void SetOverzoomFont();

  //      void CreateCM93OffsetEmbossMapData(ColorScheme cs);
  //      void EmbossCM93Offset ( wxMemoryDC *pdc);

  void DrawEmboss(ocpnDC &dc, emboss_data *pemboss);

  void ShowBrightnessLevelTimedPopup(int brightness, int min, int max);
  void HandleNotificationMouseClick();

  //    Data
  /** The width of the canvas in physical pixels. */
  int m_canvas_width;
  /** The height of the canvas in physical pixels. */
  int m_canvas_height;

  int xr_margin;  // chart scroll margins, control cursor, etc.
  int xl_margin;
  int yt_margin;
  int yb_margin;

  wxPoint last_drag;
  wxPoint m_last_touch_down_pos;
  bool m_ignore_next_leftup;

  wxMemoryDC *pmemdc;

  int warp_x, warp_y;
  bool warp_flag;

  wxTimer *
      pPanTimer;  // This timer used for auto panning on route creation and edit
  wxTimer *
      pMovementTimer;  // This timer used for smooth movement in non-opengl mode
  wxTimer *pMovementStopTimer;  // This timer used to stop movement if a keyup
                                // event is lost
  wxTimer *pCurTrackTimer;  // This timer used to update the status window on
                            // mouse idle
  wxTimer *pRotDefTimer;    // This timer used to control rotaion rendering on
                            // mouse moves
  wxTimer *m_DoubleClickTimer;
  wxTimer m_routeFinishTimer;

  wxTimer m_RolloverPopupTimer;

  wxTimer m_VPMovementTimer;

  int m_wheelzoom_stop_oneshot;
  int m_last_wheel_dir;
  wxStopWatch m_wheelstopwatch;
  double m_zoom_target;

  int m_curtrack_timer_msec;
  int m_rollover_popup_timer_msec;

  GSHHSChart *pWorldBackgroundChart;

  ChartBaseBSB *pCBSB;
  wxBitmap *pss_overlay_bmp;
  wxMask *pss_overlay_mask;

  wxRect ship_draw_rect;
  wxRect ship_draw_last_rect;
  wxRect ais_draw_rect;
  wxRect alert_draw_rect;  // pjotrc 2010.02.22

  wxBitmap *proute_bm;  // a bitmap and dc used to calculate route bounding box
  wxMemoryDC m_dc_route;  // seen in mouse->edit->route

  emboss_data *m_pEM_Feet;  // maps for depth unit emboss pattern
  emboss_data *m_pEM_Meters;
  emboss_data *m_pEM_Fathoms;

  emboss_data *m_pEM_OverZoom;
  //      emboss_data *m_pEM_CM93Offset;  // Flav

  /**
   * Physical pixels per meter at chart center, accounting for latitude
   * distortion.
   *
   * This represents the true displayed scale at the chart center, calculated by
   * measuring the screen distance in pixels between two points of known
   * geographic distance. Unlike GetVPScale() which is the requested scale, this
   * accounts for projection distortions, particularly in Mercator projection
   * where scale varies with latitude.
   *
   * @note Key scale relationships:
   * - GetVPScale() is the scale the user asks for.
   * - m_true_scale_ppm is the actual scale after projection and latitude
   * effects.
   */
  double m_true_scale_ppm;

  ownship_state_t m_ownship_state;

  ColorScheme m_cs;

  wxBitmap m_bmTideDay;
  wxBitmap m_bmTideDusk;
  wxBitmap m_bmTideNight;
  wxBitmap m_bmCurrentDay;
  wxBitmap m_bmCurrentDusk;
  wxBitmap m_bmCurrentNight;
  wxBitmap m_cTideBitmap;
  wxBitmap m_cCurrentBitmap;

  RolloverWin *m_pRouteRolloverWin;
  RolloverWin *m_pTrackRolloverWin;
  RolloverWin *m_pAISRolloverWin;

  TimedPopupWin *m_pBrightPopup;

  wxImage m_os_image_red_day;
  wxImage m_os_image_red_dusk;
  wxImage m_os_image_red_night;
  wxImage m_os_image_grey_day;
  wxImage m_os_image_grey_dusk;
  wxImage m_os_image_grey_night;
  wxImage m_os_image_yellow_day;
  wxImage m_os_image_yellow_dusk;
  wxImage m_os_image_yellow_night;

  wxImage *m_pos_image_red;
  wxImage *m_pos_image_grey;
  wxImage *m_pos_image_yellow;

  wxImage *m_pos_image_user;
  wxImage *m_pos_image_user_grey;
  wxImage *m_pos_image_user_yellow;

  wxImage *m_pos_image_user_day;
  wxImage *m_pos_image_user_dusk;
  wxImage *m_pos_image_user_night;
  wxImage *m_pos_image_user_grey_day;
  wxImage *m_pos_image_user_grey_dusk;
  wxImage *m_pos_image_user_grey_night;
  wxImage *m_pos_image_user_yellow_day;
  wxImage *m_pos_image_user_yellow_dusk;
  wxImage *m_pos_image_user_yellow_night;

  wxImage m_ship_pix_image;  // cached ship draw image for high overzoom
  int m_cur_ship_pix;
  bool m_cur_ship_pix_isgrey;
  ColorScheme m_ship_cs;

  ViewPort m_cache_vp;
  wxBitmap *m_prot_bm;
  wxPoint m_roffset;

  bool m_b_rot_hidef;

  SelectItem *m_pRolloverRouteSeg;
  SelectItem *m_pRolloverTrackSeg;

  double m_wheel_lat, m_wheel_lon;
  int m_wheel_x, m_wheel_y;

  ViewPort m_bm_cache_vp;
  wxBitmap m_working_bm;       // Used to build quilt in OnPaint()
  wxBitmap m_cached_chart_bm;  // A cached copy of the fully drawn quilt

  bool m_bbrightdir;
  int m_brightmod;

  bool m_bzooming, m_bzooming_to_cursor;
  IDX_entry *m_pIDXCandidate;

  // #ifdef ocpnUSE_GL
  glChartCanvas *m_glcc;
  // #endif

  // Smooth movement member variables
  wxPoint m_pan_drag;
  int m_panx, m_pany, m_modkeys;
  double m_panspeed;
  bool m_bmouse_key_mod;
  double m_zoom_factor, m_rotation_speed;
  /**
   * Timer/counter for smooth movement operations (panning, zooming, rotating).
   * Represents the remaining duration (in milliseconds) for which movement
   * should continue. Used to control animation steps.
   */
  int m_mustmove;

  wxDateTime m_last_movement_time;

  int m_AISRollover_MMSI;
  bool m_bsectors_shown;
  bool m_bedge_pan;
  double m_displayed_scale_factor;

  wxColour m_fog_color;
  bool m_disable_edge_pan;
  wxFont *m_pgridFont;

  bool m_dragoffsetSet;

  bool m_bautofind;
  bool m_bFirstAuto;
  double m_vLat, m_vLon;
  ChartStack *m_pCurrentStack;
  Piano *m_Piano;
  bool m_bpersistent_quilt;

  wxMenu *m_piano_ctx_menu;
  int menu_selected_dbIndex, menu_selected_index;

  ocpnCompass *m_Compass;
  bool m_bShowGPS;
  /**
   * Track whether a previous wxMouseEvent was in the m_Compass area.
   * This is used to determine whether to display tooltips for the compass.
   */
  bool m_mouseWasInCompass;

  wxRect m_mainlast_tb_rect;
  int m_restore_dbindex;
  int m_restore_group;

  MUIBar *m_muiBar;
  wxSize m_muiBarHOSize;

  bool m_bShowOutlines;
  bool m_bDisplayGrid;
  bool m_bShowDepthUnits;
  bool m_bShowAIS;
  bool m_bShowAISScaled;

  // S52PLib state storage
  long m_s52StateHash;
  bool m_encShowText;
  bool m_encShowDepth;
  bool m_encShowLightDesc;
  bool m_encShowBuoyLabels;
  int m_encDisplayCategory;
  bool m_encShowLights;
  bool m_encShowAnchor;
  bool m_encShowDataQual;

  wxTimer m_deferredFocusTimer;
  float m_focus_indicator_pix;
  bool m_bENCGroup;
  bool m_last_TBviz;

  double m_OSoffsetx, m_OSoffsety;
  bool m_MouseDragging;

  wxString m_alertString;
  wxRect m_scaleBarRect;
  bool m_bShowCompassWin;
  bool m_pianoFrozen;

  double m_sector_glat, m_sector_glon;
  std::vector<s57Sector_t> m_sectorlegsVisible;
  bool m_bShowVisibleSectors;
  /** Physical to logical pixel ratio for the display. */
  double m_displayScale;
  bool m_show_focus_bar;

  double m_panx_target_final;
  double m_pany_target_final;
  double m_panx_target_now;
  double m_pany_target_now;

  double m_start_lat, m_start_lon;
  double m_target_lat, m_target_lon;
  double m_run_lat, m_run_lon;
  bool m_timed_move_vp_active;
  int m_timedVP_step;
  int m_stvpc;

  double meters_to_shift = 0;
  double dir_to_shift = 0;

  // Chart drag inertia support
  wxTimer m_chart_drag_inertia_timer;
  void OnChartDragInertiaTimer(wxTimerEvent &event);

  uint64_t m_last_drag_time;
  int m_chart_drag_total_x;
  int m_chart_drag_total_y;
  double m_chart_drag_total_time;
  double m_chart_drag_velocity_x;
  double m_chart_drag_velocity_y;
  wxLongLong m_chart_drag_inertia_time;
  wxLongLong m_chart_drag_inertia_start_time;
  bool m_chart_drag_inertia_active;
  double m_last_elapsed;
  std::vector<int> m_drag_vec_x;
  std::vector<int> m_drag_vec_y;
  std::vector<double> m_drag_vec_t;
  int m_inertia_last_drag_x;
  int m_inertia_last_drag_y;

  // For Jump animation
  wxTimer m_easeTimer;
  wxLongLong m_animationStart;
  wxLongLong m_animationDuration;  // e.g. 300 ms
  double m_startLat, m_startLon, m_startScale;
  double m_endLat, m_endLon, m_endScale;
  bool m_animationActive;
  void OnJumpEaseTimer(wxTimerEvent &event);
  bool StartSmoothJump(double lat, double lon, double scale_ppm);
  bool m_disable_adjust_on_zoom;

  NotificationButton *m_notification_button;
  NotificationsList *m_NotificationsList;
  ObservableListener evt_notificationlist_change_listener;

  wxStopWatch m_sw_left_down;
  wxStopWatch m_sw_left_up;
  long m_sw_down_time;
  long m_sw_up_time;
  wxTimer m_tap_timer;
  wxPoint m_lastTapPos;
  int m_tap_count;
  void OnTapTimer(wxTimerEvent &event);
  int m_DragTrigger;
  uint64_t m_DragTriggerStartTime;

  DECLARE_EVENT_TABLE()
};

// CUSTOMIZATION - FORMAT MINUTES

wxString minutesToHoursDays(float timeInMinutes);

// END OF CUSTOMIZATION - FORMAT MINUTES

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

#endif  //  _CHCANV_H__
