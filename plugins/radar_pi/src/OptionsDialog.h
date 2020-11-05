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

#ifndef _OPTIONSDIALOG_H_
#define _OPTIONSDIALOG_H_

#include "radar_pi.h"

PLUGIN_BEGIN_NAMESPACE

class OptionsDialog : public wxDialog {
 public:
  OptionsDialog(wxWindow* parent, PersistentSettings& settings, RadarType radar_type);
  PersistentSettings GetSettings() { return m_settings; };

 private:
  void OnClose(wxCloseEvent& event);
  void OnIdOKClick(wxCommandEvent& event);
  void OnRangeUnitsClick(wxCommandEvent& event);
  void OnDisplayOptionClick(wxCommandEvent& event);
  void OnDisplayModeClick(wxCommandEvent& event);
  void OnGuardZoneStyleClick(wxCommandEvent& event);
  void OnGuardZoneOnOverlayClick(wxCommandEvent& event);
  void OnOverlayOnStandbyClick(wxCommandEvent& event);
  void OnGuardZoneTimeoutClick(wxCommandEvent& event);
  void OnShowExtremeRangeClick(wxCommandEvent& event);
  void OnTrailsOnOverlayClick(wxCommandEvent& event);
  void OnTrailStartColourClick(wxCommandEvent& event);
  void OnTrailEndColourClick(wxCommandEvent& event);
  void OnWeakColourClick(wxCommandEvent& event);
  void OnArpaColourClick(wxCommandEvent& event);
  void OnDopplerApproachingColourClick(wxCommandEvent& event);
  void OnDopplerRecedingColourClick(wxCommandEvent& event);
  void OnPPIBackgroundColourClick(wxCommandEvent& event);
  void OnAisTextColourClick(wxCommandEvent& event);
  void OnIntermediateColourClick(wxCommandEvent& event);
  void OnStrongColourClick(wxCommandEvent& event);
  void OnSelectSoundClick(wxCommandEvent& event);
  void OnTestSoundClick(wxCommandEvent& event);
  void OnIgnoreHeadingClick(wxCommandEvent& event);
  void OnPassHeadingClick(wxCommandEvent& event);
  void OnDrawingMethodClick(wxCommandEvent& event);
  void OnMenuAutoHideClick(wxCommandEvent& event);
  void OnEnableCOGHeadingClick(wxCommandEvent& event);
  void OnReverseZoomClick(wxCommandEvent& event);
  void OnResetButtonClick(wxCommandEvent& event);

  PersistentSettings m_settings;

  // DisplayOptions
  wxRadioBox* m_RangeUnits;
  wxRadioBox* m_OverlayDisplayOptions;
  wxRadioBox* m_DisplayMode;
  wxRadioBox* m_GuardZoneStyle;
  wxTextCtrl* m_GuardZoneTimeout;
  wxColourPickerCtrl* m_TrailStartColour;
  wxColourPickerCtrl* m_TrailEndColour;
  wxColourPickerCtrl* m_WeakColour;
  wxColourPickerCtrl* m_IntermediateColour;
  wxColourPickerCtrl* m_StrongColour;
  wxColourPickerCtrl* m_ArpaColour;
  wxColourPickerCtrl* m_AisTextColour;
  wxColourPickerCtrl* m_PPIBackgroundColour;
  wxColourPickerCtrl* m_DopplerApproachingColour;
  wxColourPickerCtrl* m_DopplerRecedingColour;
  wxCheckBox* m_ShowExtremeRange;
  wxCheckBox* m_GuardZoneOnOverlay;
  wxCheckBox* m_TrailsOnOverlay;
  wxCheckBox* m_OverlayStandby;
  wxCheckBox* m_IgnoreHeading;
  wxCheckBox* m_PassHeading;
  wxCheckBox* m_COGHeading;
  wxComboBox* m_DrawingMethod;
  wxComboBox* m_MenuAutoHide;
  wxCheckBox* m_EnableDualRadar;
  wxCheckBox* m_ReverseZoom;
};

PLUGIN_END_NAMESPACE

#endif /* _OPTIONSDIALOG_H_ */
