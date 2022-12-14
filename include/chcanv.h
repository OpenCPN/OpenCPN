
/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Canvas
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

#ifndef _CHCANV_H__
#define _CHCANV_H__

#include "bbox.h"

#include <wx/datetime.h>
#include <wx/treectrl.h>
#include <wx/dirctrl.h>
#include <wx/sound.h>
#include <wx/grid.h>
#include <wx/wxhtml.h>

#include "ocpndc.h"
#include "undo.h"

#include "ocpCursor.h"
#include "timers.h"
#include "emboss_data.h"
#include "S57Sector.h"
#include "gshhs.h"

class wxGLContext;
class GSHHSChart;
class IDX_entry;
class ocpnCompass;
class TimedPopupWin;
class Track;
class ocpnFloatingToolbarDialog;

//    Useful static routines
void ShowAISTargetQueryDialog(wxWindow *parent, int mmsi);

//--------------------------------------------------------
//    Screen Brightness Control Support Routines
//
//--------------------------------------------------------

int InitScreenBrightness(void);
int RestoreScreenBrightness(void);
int SetScreenBrightness(int brightness);

//    Set up the preferred quilt type
#define QUILT_TYPE_2

//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------
class Route;
class TCWin;
class RoutePoint;
class SelectItem;
class BoundingBox;
class ocpnBitmap;
class WVSChart;
class MyFrame;
class ChartBaseBSB;
class ChartBase;
class AisTargetData;
class S57ObjectTree;
class S57ObjectDesc;
class RolloverWin;
class Quilt;
class PixelCache;
class ChInfoWin;
class glChartCanvas;
class CanvasMenuHandler;
class ChartStack;
class Piano;
class canvasConfig;
class MUIBar;

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

enum { ID_PIANO_DISABLE_QUILT_CHART = 32000, ID_PIANO_ENABLE_QUILT_CHART };

enum { NORTH_UP_MODE, COURSE_UP_MODE, HEAD_UP_MODE };

//----------------------------------------------------------------------------
// ChartCanvas
//----------------------------------------------------------------------------
class ChartCanvas : public wxWindow {
  friend class glChartCanvas;

public:
  ChartCanvas(wxFrame *frame, int canvasIndex);
  ~ChartCanvas();

  void SetupGlCanvas();

  //    Methods
  void OnKeyDown(wxKeyEvent &event);
  void OnKeyUp(wxKeyEvent &event);
  void OnKeyChar(wxKeyEvent &event);
  void OnPaint(wxPaintEvent &event);
  void PaintCleanup();
  void Scroll(int dx, int dy);
  void OnToolLeftClick(wxCommandEvent &event);

  bool MouseEventOverlayWindows(wxMouseEvent &event);
  bool MouseEventChartBar(wxMouseEvent &event);
  bool MouseEventSetup(wxMouseEvent &event, bool b_handle_dclick = true);
  bool MouseEventProcessObjects(wxMouseEvent &event);
  bool MouseEventProcessCanvas(wxMouseEvent &event);
  void SetCanvasCursor(wxMouseEvent &event);
  void OnKillFocus(wxFocusEvent &WXUNUSED(event));
  void OnSetFocus(wxFocusEvent &WXUNUSED(event));
#ifdef HAVE_WX_GESTURE_EVENTS
  void OnZoom(wxZoomGestureEvent& event);
  void OnLongPress(wxLongPressEvent& event);
  void OnPressAndTap(wxPressAndTapEvent& event);

  void OnLeftDown(wxMouseEvent& evt);
  void OnLeftUp(wxMouseEvent& evt);

  void OnRightUp(wxMouseEvent& event);
  void OnRightDown(wxMouseEvent& event);

  void OnDoubleLeftClick(wxMouseEvent& event);

  void OnWheel(wxMouseEvent& event);
  void OnMotion(wxMouseEvent& event);
#endif /* HAVE_WX_GESTURE_EVENTS */

  void PopupMenuHandler(wxCommandEvent &event);
  bool IsPrimaryCanvas() { return (m_canvasIndex == 0); }

  bool SetUserOwnship();

  double GetCanvasRangeMeters();
  void SetCanvasRangeMeters(double range);

  void EnablePaint(bool b_enable);
  virtual bool SetCursor(const wxCursor &c);
  virtual void Refresh(bool eraseBackground = true,
                       const wxRect *rect = (const wxRect *)NULL);
  virtual void Update();

  void LostMouseCapture(wxMouseCaptureLostEvent &event);

  void CancelMouseRoute();
  void SetDisplaySizeMM(double size);
  double GetDisplaySizeMM() { return m_display_size_mm; }

  bool SetVPScale(double sc, bool b_refresh = true);
  bool SetVPProjection(int projection);
  bool SetViewPoint(double lat, double lon);
  bool SetViewPointByCorners(double latSW, double lonSW, double latNE,
                             double lonNE);
  bool SetViewPoint(double lat, double lon, double scale_ppm, double skew,
                    double rotation, int projection = 0, bool b_adjust = true,
                    bool b_refresh = true);
  void ReloadVP(bool b_adjust = true);
  void LoadVP(ViewPort &vp, bool b_adjust = true);

  ChartStack *GetpCurrentStack() { return m_pCurrentStack; }
  void SetGroupIndex(int index, bool autoswitch = false);
  bool CheckGroup(int igroup);
  void canvasRefreshGroupIndex(void);
  void canvasChartsRefresh(int dbi_hint);

  void CheckGroupValid(bool showMessage = true, bool switchGroup0 = true);

  void UpdateCanvasS52PLIBConfig();

  void TriggerDeferredFocus();
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
  void JumpToPosition(double lat, double lon, double scale);
  void SetFirstAuto(bool b_auto) { m_bFirstAuto = b_auto; }

  void GetDoubleCanvasPointPix(double rlat, double rlon, wxPoint2DDouble *r);
  void GetDoubleCanvasPointPixVP(ViewPort &vp, double rlat, double rlon,
                                 wxPoint2DDouble *r);
  bool GetCanvasPointPix(double rlat, double rlon, wxPoint *r);
  bool GetCanvasPointPixVP(ViewPort &vp, double rlat, double rlon, wxPoint *r);

  void GetCanvasPixPoint(double x, double y, double &lat, double &lon);
  void WarpPointerDeferred(int x, int y);
  void UpdateShips();
  void UpdateAIS();
  void UpdateAlerts();  // pjotrc 2010.02.22
  void ToggleCPAWarn();

  bool IsMeasureActive() { return m_bMeasure_Active; }
  wxBitmap &GetTideBitmap() { return m_cTideBitmap; }

  void UnlockQuilt();
  void SetQuiltMode(bool b_quilt);
  bool GetQuiltMode(void);
  std::vector<int> GetQuiltIndexArray(void);
  bool IsQuiltDelta(void);
  void SetQuiltChartHiLiteIndex(int dbIndex);
  int GetQuiltReferenceChartIndex(void);
  double GetBestStartScale(int dbi_hint, const ViewPort &vp);
  void ConfigureChartBar();

  int GetNextContextMenuId();

  TCWin *getTCWin() { return pCwin; }

  bool StartTimedMovement(bool stoptimer = true);
  void DoTimedMovement();
  void DoMovement(long dt);
  void StopMovement();

  void SetColorScheme(ColorScheme cs);
  ColorScheme GetColorScheme() { return m_cs; }

  void CanvasApplyLocale();
  void RebuildCursors();

  //    Accessors
  int GetCanvasWidth() { return m_canvas_width; }
  int GetCanvasHeight() { return m_canvas_height; }
  float GetVPScale() { return GetVP().view_scale_ppm; }
  float GetVPChartScale() { return GetVP().chart_scale; }
  double GetCanvasScaleFactor() { return m_canvas_scale_factor; }
  double GetCanvasTrueScale() { return m_true_scale_ppm; }
  double GetAbsoluteMinScalePpm() { return m_absolute_min_scale_ppm; }
  ViewPort &GetVP();
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

  double GetPixPerMM() { return m_pix_per_mm; }

  void SetOwnShipState(ownship_state_t state) { m_ownship_state = state; }
  void SetCursorStatus(double cursor_lat, double cursor_lon);
  void GetCursorLatLon(double *lat, double *lon);

  bool PanCanvas(double dx, double dy);
  void StopAutoPan(void);

  void ZoomCanvas(double factor, bool can_zoom_to_cursor = true,
                  bool stoptimer = true);
  void ZoomCanvasSimple(double factor);
  void DoZoomCanvas(double factor, bool can_zoom_to_cursor = true);

  void RotateCanvas(double dir);
  void DoRotateCanvas(double rotation);
  void DoTiltCanvas(double tilt);

  void ShowAISTargetList(void);

  void ShowGoToPosition(void);
  void HideGlobalToolbar();
  void ShowGlobalToolbar();

  ChartBase *GetLargestScaleQuiltChart();
  ChartBase *GetFirstQuiltChart();
  ChartBase *GetNextQuiltChart();
  int GetQuiltChartCount();
  void InvalidateAllQuiltPatchs(void);
  void SetQuiltRefChart(int dbIndex);
  std::vector<int> GetQuiltCandidatedbIndexArray(bool flag1 = true,
                                                 bool flag2 = true);
  std::vector<int> GetQuiltExtendedStackdbIndexArray();
  std::vector<int> GetQuiltEclipsedStackdbIndexArray();
  int GetQuiltRefChartdbIndex(void);
  void InvalidateQuilt(void);
  double GetQuiltMaxErrorFactor();
  bool IsChartQuiltableRef(int db_index);
  bool IsChartLargeEnoughToRender(ChartBase *chart, ViewPort &vp);
  int GetCanvasChartNativeScale();
  int FindClosestCanvasChartdbIndex(int scale);
  void UpdateCanvasOnGroupChange(void);
  void SetUpMode(int mode);
  void ToggleLookahead();
  void SetShowGPS(bool show);

  void ShowObjectQueryWindow(int x, int y, float zlat, float zlon);
  void ShowMarkPropertiesDialog(RoutePoint *markPoint);
  void ShowRoutePropertiesDialog(wxString title, Route *selected);
  void ShowTrackPropertiesDialog(Track *selected);
  void DrawTCWindow(int x, int y, void *pIDX);

  void UpdateGPSCompassStatusBox(bool b_force_new);
  ocpnCompass *GetCompass() { return m_Compass; }

  wxColour GetFogColor() { return m_fog_color; }

  void ShowChartInfoWindow(int x, int dbIndex);
  void HideChartInfoWindow(void);

  void StartMeasureRoute();
  void CancelMeasureRoute();

  bool DoCanvasUpdate(void);
  void SelectQuiltRefdbChart(int db_index, bool b_autoscale = true);
  void SelectQuiltRefChart(int selected_index);
  double GetBestVPScale(ChartBase *pchart);
  void selectCanvasChartDisplay(int type, int family);
  void RemoveChartFromQuilt(int dbIndex);

  void HandlePianoClick(int selected_index, int selected_dbIndex);
  void HandlePianoRClick(int x, int y, int selected_index,
                         int selected_dbIndex);
  void HandlePianoRollover(int selected_index, int selected_dbIndex);
  void UpdateCanvasControlBar(void);
  void FormatPianoKeys(void);
  void PianoPopupMenu(int x, int y, int selected_index, int selected_dbIndex);
  void OnPianoMenuDisableChart(wxCommandEvent &event);
  void OnPianoMenuEnableChart(wxCommandEvent &event);
  bool IsPianoContextMenuActive() { return m_piano_ctx_menu != 0; }
  void SetCanvasToolbarItemState(int tool_id, bool state);
  bool DoCanvasCOGSet(void);
  void UpdateFollowButtonState(void);
  void ApplyGlobalSettings();
  void SetShowGPSCompassWindow(bool bshow);

  void FreezePiano() { m_pianoFrozen = true; }
  void ThawPiano() { m_pianoFrozen = false; }

  // Todo build more accessors
  bool m_bFollow;
  wxCursor *pCursorPencil;
  wxCursor *pCursorArrow;
  wxCursor *pCursorCross;
  wxCursor *pPlugIn_Cursor;
  TCWin *pCwin;
  wxBitmap *pscratch_bm;
  bool m_brepaint_piano;
  double m_cursor_lon, m_cursor_lat;
  Undo *undo;
  wxPoint r_rband;
  double m_prev_rlat;
  double m_prev_rlon;
  RoutePoint *m_prev_pMousePoint;
  Quilt *m_pQuilt;
  bool m_bShowNavobjects;
  int m_canvasIndex;
  int m_groupIndex;
  int m_routeState;
  ChartBase *m_singleChart;
  int m_upMode;
  bool m_bLookAhead;
  double m_VPRotate;

#ifdef HAVE_WX_GESTURE_EVENTS
  double m_oldVPSScale;
  bool m_popupWanted;
  bool m_leftdown;
  wxPoint m_zoomStartPoint;
#endif /* HAVE_WX_GESTURE_EVENTS */

  void DrawBlinkObjects(void);

  void StartRoute(void);
  void FinishRoute(void);

  void InvalidateGL();

#ifdef ocpnUSE_GL
  glChartCanvas *GetglCanvas() { return m_glcc; }
#endif

  void JaggyCircle(ocpnDC &dc, wxPen pen, int x, int y, int radius);

  bool CheckEdgePan(int x, int y, bool bdragging, int margin, int delta);

  Route *m_pMouseRoute;
  bool m_FinishRouteOnKillFocus;
  bool m_bMeasure_Active;
  bool m_bMeasure_DistCircle;
  wxString m_active_upload_port;
  bool m_bAppendingRoute;
  int m_nMeasureState;
  Route *m_pMeasureRoute;
  MyFrame *parent_frame;
  wxString FindValidUploadPort();
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

  ocpnFloatingToolbarDialog *RequestNewCanvasToolbar(bool bforcenew = true);
  void UpdateToolbarColorScheme(ColorScheme cs);
  void SetAISCanvasDisplayStyle(int StyleIndx);
  void TouchAISToolActive(void);
  void UpdateAISTBTool(void);
  void SetToolbarScaleFactor(double scale_factor) {
    m_toolbar_scalefactor = scale_factor;
  }
  ocpnFloatingToolbarDialog *GetToolbar() { return m_toolBar; }
  void SetToolbarConfigString(wxString &config) { m_toolbarConfig = config; }
  wxString GetToolbarConfigString() { return m_toolbarConfig; }

  void SetToolbarPosition(wxPoint position);
  wxPoint GetToolbarPosition();
  void SetToolbarOrientation(long orient);
  long GetToolbarOrientation();

  void SubmergeToolbar(void);
  void SurfaceToolbar(void);
  void ToggleToolbar(bool b_smooth = false);
  bool IsToolbarShown();
  void DestroyToolbar();

  void SelectChartFromStack(int index, bool bDir = false,
                            ChartTypeEnum New_Type = CHART_TYPE_DONTCARE,
                            ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
  void SelectdbChart(int dbindex);

  void ShowTides(bool bShow);
  void ShowCurrents(bool bShow);

  void DoCanvasStackDelta(int direction);

  void ProcessNewGUIScale();

  bool GetShowDepthUnits() { return m_bShowDepthUnits; }
  void SetShowDepthUnits(bool show) { m_bShowDepthUnits = show; }
  bool GetShowGrid() { return m_bDisplayGrid; }
  void SetShowGrid(bool show) { m_bDisplayGrid = show; }
  bool GetShowOutlines() { return m_bShowOutlines; }
  void SetShowOutlines(bool show) { m_bShowOutlines = show; }
  bool GetShowChartbar() { return true; }
  bool GetToolbarEnable() { return m_bToolbarEnable; }
  void SetToolbarEnable(bool show);
  wxRect GetMUIBarRect();
  void SetMUIBarPosition();
  void DestroyMuiBar();
  void CreateMUIBar();

  void ToggleChartOutlines(void);
  void ToggleCanvasQuiltMode(void);

  wxString GetScaleText() { return m_scaleText; }
  double GetScaleValue() { return m_scaleValue; }

  bool m_b_paint_enable;

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

  int GetENCDisplayCategory() { return m_encDisplayCategory; }
  void SetENCDisplayCategory(int category);

  bool GetShowENCAnchor() { return m_encShowAnchor; }
  void SetShowENCAnchor(bool show);

  bool GetShowENCDataQual() { return m_encShowDataQual; }
  void SetShowENCDataQual(bool show);

  int GetUpMode() { return m_upMode; }
  bool GetLookahead() { return m_bLookAhead; }

  bool GetShowAIS() { return m_bShowAIS; }
  void SetShowAIS(bool show);
  bool GetAttenAIS() { return m_bShowAISScaled; }
  void SetAttenAIS(bool show);

  MUIBar *GetMUIBar() { return m_muiBar; }

  void SetAlertString(wxString str) { m_alertString = str; }
  wxString GetAlertString() { return m_alertString; }

  wxRect GetScaleBarRect() { return m_scaleBarRect; }
  void RenderAlertMessage(wxDC &dc, const ViewPort &vp);

  std::vector<int> m_tile_noshow_index_array;
  std::vector<int> m_tile_yesshow_index_array;
  std::vector<int> m_quilt_noshow_index_array;

  bool IsTileOverlayIndexInYesShow(int index);
  bool IsTileOverlayIndexInNoShow(int index);
  void AddTileOverlayIndexToNoShow(int index);

  std::vector<int> GetQuiltNoshowIindexArray() {
    return m_quilt_noshow_index_array;
  }

private:
  int AdjustQuiltRefChart();

  bool UpdateS52State();

  void CallPopupMenu(int x, int y);

  bool IsTempMenuBarEnabled();
  bool InvokeCanvasMenu(int x, int y, int seltype);

  ViewPort VPoint;
  void PositionConsole(void);

  wxColour PredColor();
  wxColour ShipColor();

  void ComputeShipScaleFactor(float icon_hdt, int ownShipWidth,
                              int ownShipLength, wxPoint &lShipMidPoint,
                              wxPoint &GpsOffsetPixels, wxPoint lGPSPoint,
                              float &scale_factor_x, float &scale_factor_y);

  void ShipDrawLargeScale(ocpnDC &dc, wxPoint lShipMidPoint);
  void ShipIndicatorsDraw(ocpnDC &dc, int img_height, wxPoint GPSOffsetPixels,
                          wxPoint lGPSPoint);

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

  double
      m_canvas_scale_factor;  // converter....
                              // useage....
                              // true_chart_scale_on_display =
                              // m_canvas_scale_factor / pixels_per_meter of
                              // displayed chart also may be considered as the
                              // "pixels-per-meter" of the canvas on-screen
  double m_pix_per_mm;  // pixels per millimeter on the screen
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

  //    Data
  int m_canvas_width, m_canvas_height;

  int xr_margin;  // chart scroll margins, control cursor, etc.
  int xl_margin;
  int yt_margin;
  int yb_margin;

  wxPoint last_drag;

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

  //#ifdef ocpnUSE_GL
  glChartCanvas *m_glcc;
  //#endif

  // Smooth movement member variables
  wxPoint m_pan_drag;
  int m_panx, m_pany, m_modkeys;
  double m_panspeed;
  bool m_bmouse_key_mod;
  double m_zoom_factor, m_rotation_speed;
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

  ocpnFloatingToolbarDialog *m_toolBar;
  double m_toolbar_scalefactor;
  wxString m_toolbarConfig;
  wxPoint m_toolbarPosition;
  long m_toolbarOrientation;

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

  wxRect m_mainlast_tb_rect;
  int m_restore_dbindex;
  int m_restore_group;

  MUIBar *m_muiBar;
  wxSize m_muiBarHOSize;

  bool m_bShowOutlines;
  bool m_bDisplayGrid;
  bool m_bShowDepthUnits;
  bool m_bToolbarEnable;
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
  double m_displayScale;

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
