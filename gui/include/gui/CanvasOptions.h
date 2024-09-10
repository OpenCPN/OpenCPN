/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Canvas Options Window/Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 ***************************************************************************
 */

#ifndef __canvasoption_H__
#define __canvasoption_H__

/**
 * Enumeration of control IDs used in the CanvasOptions dialog.
 */
enum {
  /** @brief ID for checkbox to show depth units */
  IDCO_SHOWDEPTHUNITSBOX1 = 31000,
  /** @brief ID for checkbox to show chart outlines */
  IDCO_OUTLINECHECKBOX1,
  /** @brief ID for checkbox to display grid */
  IDCO_CHECK_DISPLAYGRID,
  /** @brief ID for checkbox related to zoom-to-cursor functionality */
  IDCO_ZTCCHECKBOX,
  /** @brief ID for checkbox to enable smooth pan/zoom */
  IDCO_SMOOTHPANZOOMBOX,
  /** @brief ID for checkbox to preserve scale during operations */
  IDCO_PRESERVECHECKBOX,
  /** @brief ID for checkbox to enable chart quilting
   *
   * Chart quilting combines multiple electronic charts into a seamless display.
   * When enabled, the system automatically selects and blends the most appropriate
   * charts based on the current view and zoom level. This provides a continuous
   * navigation experience without visible chart boundaries, potentially combining
   * charts of different scales or from different sources.
   */
  IDCO_QUILTCHECKBOX1,
  /** @brief ID for checkbox to enable look-ahead mode
   *
   * Look-ahead mode automatically adjusts the chart view to show more of the area
   * ahead of the vessel based on its current course and speed. This feature helps
   * navigators anticipate upcoming navigational challenges or points of interest.
   * When enabled, the chart view is typically shifted so that the vessel's position
   * is not centered but offset towards the bottom of the screen, showing more of
   * the area ahead.
   */
  IDCO_CHECK_LOOKAHEAD,
  /** @brief ID for checkbox to set course-up orientation
   *
   * In course-up mode, the chart is rotated so that the vessel's current course
   * is always pointing towards the top of the screen. This orientation changes
   * dynamically as the vessel changes course. It's particularly useful for
   * navigation as it aligns the chart view with the vessel's direction of travel,
   * making it easier to interpret upcoming features or obstacles.
   */
  IDCO_COURSEUPCHECKBOX,
  /** @brief ID for checkbox to set head-up orientation
   *
   * In head-up mode, the chart is rotated so that the vessel's current heading
   * (the direction the bow is pointing) is always at the top of the screen.
   * This mode updates in real-time based on data from a compass or heading sensor.
   * It's useful for understanding the vessel's orientation relative to charted
   * objects, especially in situations with strong currents where heading and
   * course might differ significantly.
   */
  IDCO_HEADUPCHECKBOX,
  /** @brief ID for checkbox to show tides */
  IDCO_TIDES_CHECKBOX,
  /** @brief ID for checkbox to show currents */
  IDCO_CURRENTS_CHECKBOX,
  /** @brief ID for checkbox to show ENC (Electronic Navigational Chart) text */
  IDCO_ENCTEXT_CHECKBOX1,
  /** @brief ID for checkbox to show/hide toolbar */
  ID_TOOLBARCHECKBOX,
  /** @brief ID for control to set ENC display category */
  ID_CODISPCAT,
  /** @brief ID for checkbox to show ENC depths */
  IDCO_ENCDEPTH_CHECKBOX1,
  /** @brief ID for checkbox to show ENC buoys */
  IDCO_ENCBUOY_CHECKBOX1,
  /** @brief ID for checkbox to show ENC buoy labels */
  IDCO_ENCBUOYLABEL_CHECKBOX1,
  /** @brief ID for checkbox to show ENC anchor details */
  IDCO_ENCANCHOR_CHECKBOX1,
  /** @brief ID for checkbox to show AIS (Automatic Identification System) targets */
  IDCO_SHOW_AIS_CHECKBOX,
  /** @brief ID for checkbox to attenuate AIS targets */
  IDCO_ATTEN_AIS_CHECKBOX,
  /** @brief ID for checkbox to show ENC visible light sectors */
  IDCO_ENCVISIBLESECTORS_CHECKBOX1,
  /** @brief ID for checkbox to show ENC data quality indicators */
  IDCO_ENCDATAQUALITY_CHECKBOX,
};

class MyFrame;
class ChartCanvas;

/**
 * Represents the Canvas Options dialog. Implements a dialog that allows users
 * to configure various options for the chart canvas display.
 */
class CanvasOptions : public wxDialog {
public:
  CanvasOptions(wxWindow *parent);

  void OnClose(wxCloseEvent &event);
  void OnOptionChange(wxCommandEvent &event);

  void RefreshControlValues(void);
  void UpdateCanvasOptions(void);
  void OnEraseBackground(wxEraseEvent &event);
  void SetENCAvailable(bool avail);

private:
  int m_style;
  bool m_ENCAvail;
  wxScrolledWindow *m_sWindow;

  wxCheckBox *pShowStatusBar, *pShowMenuBar, *pShowChartBar, *pShowCompassWin;
  wxCheckBox *pPrintShowIcon, *pCDOOutlines, *pSDepthUnits, *pSDisplayGrid;
  wxCheckBox *pAutoAnchorMark, *pCDOQuilting, *pCBRaster, *pCBVector;
  wxCheckBox *pCBCM93, *pCBLookAhead, *pSkewComp, *pOpenGL, *pSmoothPanZoom;
  wxCheckBox *pFullScreenQuilt, *pMobile, *pResponsive, *pOverzoomEmphasis;
  wxCheckBox *pOZScaleVector, *pToolbarAutoHideCB, *pInlandEcdis;
  wxTextCtrl *pCOGUPUpdateSecs, *m_pText_OSCOG_Predictor, *pScreenMM;
  wxTextCtrl *pToolbarHideSecs, *m_pText_OSHDT_Predictor;
  wxChoice *m_pShipIconType, *m_pcTCDatasets;
  wxSlider *m_pSlider_Zoom, *m_pSlider_GUI_Factor, *m_pSlider_Chart_Factor,
      *m_pSlider_Ship_Factor;
  wxSlider *m_pSlider_Zoom_Vector;
  wxRadioButton *pCBCourseUp, *pCBNorthUp, *pCBHeadUp, *pRBSizeAuto,
      *pRBSizeManual;
  wxCheckBox *pEnableZoomToCursor, *pPreserveScale;

  wxCheckBox *pCDOTides, *pCDOCurrents;
  wxCheckBox *pCDOENCText, *pCBToolbar;
  wxChoice *m_pDispCat;
  wxCheckBox *pCBENCDepth, *pCBENCLightDesc, *pCBENCBuoyLabels, *pCBENCLights,
      *pCBENCAnchorDetails;
  wxCheckBox *pCBShowAIS, *pCBAttenAIS, *pCBENCVisibleSectors,
      *pCBENCDataQuality;
  bool m_bmode_change_while_hidden;  // Coming from compass icon click

  DECLARE_EVENT_TABLE()
};

#endif  // guard
