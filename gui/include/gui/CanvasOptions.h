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

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
enum {
  IDCO_SHOWDEPTHUNITSBOX1 = 31000,
  IDCO_OUTLINECHECKBOX1,
  IDCO_CHECK_DISPLAYGRID,
  IDCO_ZTCCHECKBOX,
  IDCO_SMOOTHPANZOOMBOX,
  IDCO_PRESERVECHECKBOX,
  IDCO_QUILTCHECKBOX1,
  IDCO_CHECK_LOOKAHEAD,
  IDCO_COURSEUPCHECKBOX,
  IDCO_HEADUPCHECKBOX,
  IDCO_TIDES_CHECKBOX,
  IDCO_CURRENTS_CHECKBOX,
  IDCO_ENCTEXT_CHECKBOX1,
  ID_TOOLBARCHECKBOX,
  ID_CODISPCAT,
  IDCO_ENCDEPTH_CHECKBOX1,
  IDCO_ENCBUOY_CHECKBOX1,
  IDCO_ENCBUOYLABEL_CHECKBOX1,
  IDCO_ENCANCHOR_CHECKBOX1,
  IDCO_SHOW_AIS_CHECKBOX,
  IDCO_ATTEN_AIS_CHECKBOX,
  IDCO_ENCVISIBLESECTORS_CHECKBOX1,
  IDCO_ENCDATAQUALITY_CHECKBOX,
};

class MyFrame;
class ChartCanvas;

//----------------------------------------------------------------------------
// CanvasOptions
//----------------------------------------------------------------------------

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

  DECLARE_EVENT_TABLE()
};

#endif  // guard
