/**************************************************************************
 *   Copyright (C) 2026 Alec Leamas                                        *
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
 * Abstract gFrame/MyFrame interface
 */

#ifndef ToP_FraMe_h
#define ToP_FraMe_h

#include "gl_headers.h"  // Must come before anything using GL stuff

#include "wx/frame.h"
#include "wx/gdicmn.h"
#include "wx/glcanvas.h"
#include "wx/statusbr.h"
#include "wx/string.h"

#include "s52s57.h"  // enum _Discat, should be moved.

#include "model/ais_target_data.h"
#include "model/gui.h"
#include "model/track.h"

#include "abstract_chart.h"
#include "abstract_chart_canv.h"
#include "undo_defs.h"

#include "abstract_chart.h"
#include "abstract_chart_canv.h"
#include "s52s57.h"  // enum _Discat, should be moved
#include "undo_defs.h"

class AbstractTopFrame : public wxFrame {
public:
  AbstractTopFrame(wxFrame *frame, const wxString &title, const wxPoint &pos,
                   const wxSize &size, long style)
      : wxFrame(frame, -1, title, pos, size, style, kTopLevelWindowName) {}

  virtual void FastClose() = 0;

  virtual void InvalidateAllGL() = 0;

  virtual void SetGPSCompassScale() = 0;

  virtual void RefreshAllCanvas(bool erase = true) = 0;

  virtual void UpdateStatusBar() = 0;

  virtual void ToggleFullScreen() = 0;

  virtual bool DoChartUpdate(void) = 0;

  virtual Track *TrackOff(bool do_add_point = false) = 0;
  virtual void TrackOn(void) = 0;

  virtual void ProcessOptionsDialog(int resultFlags) = 0;
  virtual void SetAlertString(wxString msg) = 0;

  virtual void JumpToPosition(double lat, double lon) = 0;
  virtual void JumpToPosition(double lat, double lon, double scale) = 0;
  virtual void JumpToPosition(AbstractChartCanvas *acc, double lat, double lon,
                              double scale) = 0;
  virtual void JumpToPosition(AbstractChartCanvas *acc, double lat,
                              double lon) = 0;

  virtual double GetPixPerMM() = 0;
  virtual double GetContentScaleFactor() = 0;

  virtual void RequestNewToolbars(bool bforcenew = false) = 0;

  virtual AbstractChartCanvas *GetAbstractPrimaryCanvas() = 0;
  virtual double GetCanvasTrueScale() = 0;
  virtual bool GetCanvasPointPix(double rlat, double rlon, wxPoint *r) = 0;
  virtual wxSize GetFocusCanvasSize() = 0;
  virtual void CancelAllMouseRoute() = 0;
  virtual void InvalidateAllCanvasUndo() = 0;
  virtual void PositionConsole() = 0;

  virtual void DoStackUp(AbstractChartCanvas *cc) = 0;
  virtual void DoStackDown(AbstractChartCanvas *cc) = 0;
  virtual double GetBestVPScale(AbstractChart *arg) = 0;
  virtual void LoadHarmonics() = 0;
  virtual bool DropMarker(bool atOwnShip = true) = 0;

  virtual double GetMag(double a, double lat, double lon) = 0;

  virtual void SetMasterToolbarItemState(int tool_id, bool state) = 0;
  virtual void ProcessCanvasResize() = 0;
  virtual bool SetGlobalToolbarViz(bool viz) = 0;
  virtual void ToggleQuiltMode(AbstractChartCanvas *cc) = 0;
  virtual void UpdateGlobalMenuItems(AbstractChartCanvas *acc) = 0;
  virtual void UpdateGlobalMenuItems() = 0;
  virtual void RefreshCanvasOther(AbstractChartCanvas *ccThis) = 0;
  virtual double *GetCOGTable() = 0;
  virtual void StartCogTimer() = 0;
  virtual wxGLCanvas *GetWxGlCanvas() = 0;
  virtual wxWindow *GetPrimaryCanvasWindow() = 0;
  virtual void ApplyGlobalSettings(bool bnewtoolbar) = 0;
  virtual void SetMenubarItemState(int item_id, bool state) = 0;
  virtual void ToggleColorScheme() = 0;
  virtual void ActivateMOB() = 0;
  virtual void ToggleTestPause() = 0;
  virtual void ToggleChartBar(AbstractChartCanvas *acc) = 0;
  virtual void DoSettings() = 0;
  virtual void SwitchKBFocus(AbstractChartCanvas *pCanvas) = 0;
  virtual void UpdateRotationState(double rotation) = 0;
  virtual void SetChartUpdatePeriod() = 0;
  virtual wxStatusBar *GetStatusBar() = 0;
  virtual wxStatusBar *GetFrameStatusBar() const = 0;
  virtual void SetENCDisplayCategory(AbstractChartCanvas *cc,
                                     enum _DisCat nset) = 0;
  virtual int GetCanvasIndexUnderMouse() = 0;
  virtual double GetCanvasRefScale() = 0;
  virtual void SendGlJsonConfigMsg() = 0;
  virtual int GetNextToolbarToolId() = 0;
  virtual void SetToolbarItemBitmaps(int tool_id, wxBitmap *bitmap,
                                     wxBitmap *bmpDisabled) = 0;
  virtual void SetToolbarItemSVG(int tool_id, wxString normalSVGfile,
                                 wxString rolloverSVGfile,
                                 wxString toggledSVGfile) = 0;
  virtual void UpdateAllFonts() = 0;
  virtual bool CanAccelerateGlPanning() = 0;
  virtual void SetupGlCompression() = 0;
  virtual void ScheduleReconfigAndSettingsReload(bool reload,
                                                 bool new_dialog) = 0;
  virtual void ScheduleReloadCharts() = 0;
  virtual void FreezeCharts() = 0;
  virtual void ThawCharts() = 0;
  virtual wxString GetGlVersionString() = 0;
  virtual void ScheduleDeleteSettingsDialog() = 0;
  virtual void ChartsRefresh() = 0;
  virtual void OnToolLeftClick(wxCommandEvent &event) = 0;
  virtual AbstractChartCanvas *GetAbstractFocusCanvas() = 0;
  virtual void BeforeUndoableAction(UndoType undo_type, RoutePoint *point,
                                    UndoBeforePointerType pointer_type,
                                    UndoItemPointer pointer) = 0;
  virtual void AfterUndoableAction(UndoItemPointer pointer) = 0;
  virtual void TouchAISActive() = 0;
  virtual void UpdateAISMOBRoute(const AisTargetData *ptarget) = 0;
  virtual void ActivateAISMOBRoute(const AisTargetData *ptarget) = 0;
  virtual void EnableSettingsTool(bool _enable) = 0;
};

namespace top_frame {

AbstractTopFrame *Get();

}
#endif  // ToP_FraMe_h
