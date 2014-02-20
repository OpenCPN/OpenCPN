/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Grib Settings Dialog
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Sean D'Epagnier                                 *
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
 *
 */

#include "grib_pi.h"

static const wxString units0_names[] = {_("Knots"), _("m/s"), _("mph"), _("km/h"), _("Beaufort"), wxEmptyString};
static const wxString units1_names[] = {_("MilliBars"), _("mmHG"), _("inHG"), wxEmptyString};
static const wxString units2_names[] = {_("Meters"), _("Feet"), wxEmptyString};
static const wxString units3_names[] = {_("Celsius"), _("Fahrenheit"), wxEmptyString};
static const wxString units4_names[] = {_("Millimeters"), _("Inches"), wxEmptyString};
static const wxString units5_names[] = {_("Percentage"), wxEmptyString};
static const wxString units6_names[] = {_("j/kg"), wxEmptyString};
static const wxString *unit_names[] = {units0_names, units1_names, units2_names,
                                       units3_names, units4_names, units5_names, units6_names};

static const wxString name_from_index[] = {_T("Wind"), _T("WindGust"), _T("Pressure"),
                                           _T("Waves"), _T("Current"),
                                           _T("Rainfall"), _T("CloudCover"),
                                           _T("AirTemperature"), _T("SeaTemperature"), _T("CAPE")};
static const wxString tname_from_index[] = {_("Wind"), _("Wind Gust"),  _("Pressure"),
                                            _("Waves"), _("Current"),
                                            _("Rainfall"), _("Cloud Cover"),
                                            _("Air Temperature(2m)"), _("Sea Temperature(surf.)"), _("CAPE")};

static const int unittype[GribOverlaySettings::SETTINGS_COUNT] = {0, 0, 1, 2, 0, 4, 5, 3, 3, 6};

enum SettingsDisplay {B_ARROWS, ISO_LINE_VISI, ISO_LINE_SHORT, D_ARROWS, OVERLAY, NUMBERS};

wxString GribOverlaySettings::NameFromIndex(int index)
{
    return wxGetTranslation(tname_from_index[index]);
}

void GribOverlaySettings::Read()
{
    /* read settings here */
    wxFileConfig *pConf = GetOCPNConfigObject();

    if(!pConf)
        return;

    pConf->SetPath ( _T( "/PlugIns/GRIB" ) );
    pConf->Read ( _T ( "Interpolate" ), &m_bInterpolate, false );
    pConf->Read ( _T ( "LoopMode" ), &m_bLoopMode, false );
    pConf->Read ( _T ( "LoopStartPoint" ), &m_LoopStartPoint, 0 );
    pConf->Read ( _T ( "SlicesPerUpdate" ), &m_SlicesPerUpdate, 2);
    pConf->Read ( _T ( "UpdatesPerSecond" ), &m_UpdatesPerSecond, 2);
    pConf->Read ( _T ( "HourDivider" ), &m_HourDivider, 2);
    pConf->Read ( _T ( "OverlayTransparency" ), &m_iOverlayTransparency, 220);

    for(int i=0; i<SETTINGS_COUNT; i++) {
        wxString Name=name_from_index[i];

        int units;
        pConf->Read ( Name + _T ( "Units" ), &units,0);
        Settings[i].m_Units = (SettingsType)units;

        pConf->Read ( Name + _T ( "BarbedArrows" ), &Settings[i].m_bBarbedArrows, i==WIND);
        pConf->Read ( Name + _T ( "BarbedVisibility" ), &Settings[i].m_iBarbedVisibility, i==WIND);
        pConf->Read ( Name + _T ( "BarbedColors" ), &Settings[i].m_iBarbedColour, 0);

        pConf->Read ( Name + _T ( "Display Isobars" ), &Settings[i].m_bIsoBars, i==PRESSURE);
        double defspacing[SETTINGS_COUNT] = {4, 4, 4, 0, 0, 0, 0, 2, 2, 100};
        pConf->Read ( Name + _T ( "IsoBarSpacing" ), &Settings[i].m_iIsoBarSpacing, defspacing[i]);
        pConf->Read ( Name + _T ( "IsoBarVisibility" ), &Settings[i].m_iIsoBarVisibility, i==PRESSURE);

        pConf->Read ( Name + _T ( "DirectionArrows" ), &Settings[i].m_bDirectionArrows, i==CURRENT || i==WAVE);
        double defform[SETTINGS_COUNT] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
        pConf->Read ( Name + _T ( "DirectionArrowForm" ), &Settings[i].m_iDirectionArrowForm, defform[i]);
        pConf->Read ( Name + _T ( "DirectionArrowSize" ), &Settings[i].m_iDirectionArrowSize, 0);

        pConf->Read ( Name + _T ( "OverlayMap" ), &Settings[i].m_bOverlayMap, i!=WIND && i!=PRESSURE);
        int defcolor[SETTINGS_COUNT] = {1, 1, 0, 0, 6, 4, 5, 2, 3, 0};
        pConf->Read ( Name + _T ( "OverlayMapColors" ), &Settings[i].m_iOverlayMapColors, defcolor[i]);

        pConf->Read ( Name + _T ( "Numbers" ), &Settings[i].m_bNumbers, false);
        pConf->Read ( Name + _T ( "NumbersSpacing" ), &Settings[i].m_iNumbersSpacing, 50);
    }
}

void GribOverlaySettings::Write()
{
    /* save settings here */
    wxFileConfig *pConf = GetOCPNConfigObject();

    if(!pConf)
        return;

    pConf->SetPath ( _T( "/PlugIns/GRIB" ) );
    pConf->Write ( _T ( "Interpolate" ), m_bInterpolate);
    pConf->Write ( _T ( "LoopMode" ), m_bLoopMode );
    pConf->Write ( _T ( "LoopStartPoint" ), m_LoopStartPoint);
    pConf->Write ( _T ( "SlicesPerUpdate" ), m_SlicesPerUpdate);
    pConf->Write ( _T ( "UpdatesPerSecond" ), m_UpdatesPerSecond);
    pConf->Write ( _T ( "HourDivider" ), m_HourDivider);
    pConf->Write ( _T ( "OverlayTransparency" ), m_iOverlayTransparency);

    for(int i=0; i<SETTINGS_COUNT; i++) {

        pConf->Write ( name_from_index[i] + _T ( "Units" ), (int)Settings[i].m_Units);
    
        if(i == WIND){
            SaveSettingGroups(pConf, i, B_ARROWS);
            SaveSettingGroups(pConf, i, ISO_LINE_SHORT);
            SaveSettingGroups(pConf, i, OVERLAY);
            SaveSettingGroups(pConf, i, NUMBERS);
        }
        else if(i == WIND_GUST || i == AIR_TEMPERATURE || i == SEA_TEMPERATURE || i == CAPE) {
            SaveSettingGroups(pConf, i, ISO_LINE_SHORT);
            SaveSettingGroups(pConf, i, OVERLAY);
            SaveSettingGroups(pConf, i, NUMBERS);
        }
        else if(i == PRESSURE) {
            SaveSettingGroups(pConf, i, ISO_LINE_SHORT);
            SaveSettingGroups(pConf, i, ISO_LINE_VISI);
            SaveSettingGroups(pConf, i, NUMBERS);
        }
        else if(i == WAVE || i == CURRENT) {
            SaveSettingGroups(pConf, i, D_ARROWS);
            SaveSettingGroups(pConf, i, OVERLAY);
            SaveSettingGroups(pConf, i, NUMBERS);
        }
        else if( i == PRECIPITATION || i == CLOUD) {
            SaveSettingGroups(pConf, i, OVERLAY);
            SaveSettingGroups(pConf, i, NUMBERS);
        }
    }
     
}

void GribOverlaySettings::SaveSettingGroups(wxFileConfig *pConf, int settings, int group)
{
    wxString Name=name_from_index[settings];

    switch(group) {
    case B_ARROWS:
        pConf->Write ( Name + _T ( "BarbedArrows" ), Settings[settings].m_bBarbedArrows);
        pConf->Write ( Name + _T ( "BarbedVisibility" ), Settings[settings].m_iBarbedVisibility);
        pConf->Write ( Name + _T ( "BarbedColors" ), Settings[settings].m_iBarbedColour);
        break;
    case ISO_LINE_SHORT:
        pConf->Write ( Name + _T ( "Display Isobars" ), Settings[settings].m_bIsoBars);
        pConf->Write ( Name + _T ( "IsoBarSpacing" ), Settings[settings].m_iIsoBarSpacing);
        break;
    case ISO_LINE_VISI:
        pConf->Write ( Name + _T ( "IsoBarVisibility" ), Settings[settings].m_iIsoBarVisibility);
        break;
    case D_ARROWS:
        pConf->Write ( Name + _T ( "DirectionArrows" ), Settings[settings].m_bDirectionArrows);
        pConf->Write ( Name + _T ( "DirectionArrowForm" ), Settings[settings].m_iDirectionArrowForm);
        pConf->Write ( Name + _T ( "DirectionArrowSize" ), Settings[settings].m_iDirectionArrowSize);
        break;
    case OVERLAY:
        pConf->Write ( Name + _T ( "OverlayMap" ), Settings[settings].m_bOverlayMap);
        pConf->Write ( Name + _T ( "OverlayMapColors" ), Settings[settings].m_iOverlayMapColors);
        break;
    case NUMBERS:
        pConf->Write ( Name + _T ( "Numbers" ), Settings[settings].m_bNumbers);
        pConf->Write ( Name + _T ( "NumbersSpacing" ), Settings[settings].m_iNumbersSpacing);
        break;
    }
}

double GribOverlaySettings::CalibrationOffset(int settings)
{
    switch(unittype[settings]) {
    case 3: switch(Settings[settings].m_Units) { /* only have offset for temperature */
        case CELCIUS:    return -273.15;
        case FAHRENHEIT: return -273.15 + 32*5/9.0;
        } break;
    }

    return 0;
}

double GribOverlaySettings::CalibrationFactor(int settings, double input, bool reverse)
{
    switch(unittype[settings]) {
    case 0: switch(Settings[settings].m_Units) {
        case KNOTS:  return 3.6 / 1.852;
        case M_S:    return 1;
        case MPH:    return 3.6 / 1.60934;
        case KPH:    return 3.6;
        case BFS:    return (reverse) ? GetbftomsFactor(input) : GetmstobfFactor(input);
        } break;
    case 1: switch(Settings[settings].m_Units) {
        case MILLIBARS: return 1 / 100.;
        case MMHG: return 1 / (100. * 1.333);
        case INHG: return 1 / (100. * 33.864);
        } break;
    case 2: switch(Settings[settings].m_Units) {
        case METERS: return 1;
        case FEET:   return 3.28;
        } break;
    case 3: switch(Settings[settings].m_Units) {
        case CELCIUS:     return 1;
        case FAHRENHEIT: return 9./5;
        } break;
    case 4: switch(Settings[settings].m_Units) {
        case MILLIMETERS: return 1;
        case INCHES:      return 1./25.4;
        } break;
    case 5:
    case 6: return 1;
    }

    return 1;
}

/*
Beaufort scale
    force             in knots   in m/s         in knots
    0                   < 1        <0.5             < 1
    1         from      1         0.5       to      3
    2                   4         2.1               6
    3                   7         3.6               10
    4                   11        5.7               16
    5                   17        8.7               21
    6                   22        11.3              27
    7                   28        14.4              33
    8                   34        17.5              40
    9                   41        21.1              47
    10                  48        24.7              55
    11                  56        28.8              63
    12                   >= 64     >= 32.9           >= 64
*/

double GribOverlaySettings::GetmstobfFactor(double input)
{
    double val = fabs(input);
    //find bf value ( m/s ) and return the corresponding factor
    if(val < 0.5 ) return 0;
    if(val < 2.1 ) return 1/input;
    if(val < 3.6 ) return 2/input;
    if(val < 5.7 ) return 3/input;
    if(val < 8.7 ) return 4/input;
    if(val < 11.3 ) return 5/input;
    if(val < 14.4 ) return 6/input;
    if(val < 17.5 ) return 7/input;
    if(val < 21.1 ) return 8/input;
    if(val < 24.7 ) return 9/input;
    if(val < 28.8 ) return 10/input;
    if(val < 32.9 ) return 11/input;
    return 12/input;
}

double GribOverlaySettings::GetbftomsFactor(double input)
{
    //find the limit value in m/s in Beaufort scale and return the corresponding factor
    switch((int) input) {
    case 1:  return input/0.5;
    case 2:  return input/2.1;
    case 3:  return input/3.6;
    case 4:  return input/5.7;
    case 5:  return input/8.7;
    case 6:  return input/11.3;
    case 7:  return input/14.4;
    case 8:  return input/17.5;
    case 9:  return input/21.1;
    case 10: return input/24.7;
    case 11: return input/28.7;
    case 12: return input/32.9;
    }
    return 1;
}

wxString GribOverlaySettings::GetUnitSymbol(int settings)
{
    switch(unittype[settings]) {
        case 0: switch(Settings[settings].m_Units) {
            case KNOTS:  return _T("kts");
            case M_S:    return _T("m/s");
            case MPH:    return _T("mph");
            case KPH:    return _T("km/h");
            case BFS:    return _T("bf");
        } break;
        case 1: switch(Settings[settings].m_Units) {
            case MILLIBARS: return _T("hPa");
            case MMHG: return _T("mmHg");
            case INHG: return _T("inHg");
        } break;
        case 2: switch(Settings[settings].m_Units) {
            case METERS: return _T("m");
            case FEET:   return _T("ft");
        } break;
        case 3: switch(Settings[settings].m_Units) {
            case CELCIUS:     return _T("\u00B0C");
            case FAHRENHEIT: return _T("\u00B0F");
        } break;
        case 4: switch(Settings[settings].m_Units) {
            case MILLIMETERS: return _T("mm");
            case INCHES:      return _T("in");
        } break;
        case 5: switch(Settings[settings].m_Units) {
            case PERCENTAGE:  return _T("%");
        } break;
        case 6: switch(Settings[settings].m_Units) {
            case JPKG:  return _T("j/kg");
        } break;
    }
    return _T("");
}

double GribOverlaySettings::GetMin(int settings)
{
    double min = 0;
    switch(settings) {
    case PRESSURE:        min = 84000;   break; /* 100's of millibars */
    case AIR_TEMPERATURE: min = 273.15-40; break; /* kelvin */
    case SEA_TEMPERATURE: min = 273.15-40;  break; /* kelvin */
    }
    return CalibrateValue(settings, min);
}

double GribOverlaySettings::GetMax(int settings)
{
    double max = 0;
    switch(settings) {
    case WIND:            max = 40;     break; /* m/s */
    case WIND_GUST:       max = 40;     break; /* m/s */
    case PRESSURE:        max = 112000;  break; /* 100s of millibars */
    case WAVE:            max = 30;      break; /* meters */
    case CURRENT:         max = 12;      break; /* m/s */
    case PRECIPITATION:   max = 80;      break; /* mm */
    case CLOUD:           max = 100;     break; /* percent */
    case AIR_TEMPERATURE: max = 273.15+50;  break; /* kelvin */
    case SEA_TEMPERATURE: max = 273.15+50;  break; /* kelvin */
    case CAPE:            max = 4000;    break; /* j/kg */
    }
    return CalibrateValue(settings, max);
}

GribSettingsDialog::GribSettingsDialog(GRIBUIDialog &parent, GribOverlaySettings &Settings, int &lastdatatype)
    : GribSettingsDialogBase(&parent),
      m_parent(parent), m_extSettings(Settings), m_lastdatatype(lastdatatype)
{
    m_Settings = m_extSettings;
    m_cInterpolate->SetValue(m_Settings.m_bInterpolate);
    m_cLoopMode->SetValue(m_Settings.m_bLoopMode);
    m_cLoopStartPoint->SetSelection(m_Settings.m_LoopStartPoint);
    m_sSlicesPerUpdate->SetValue(m_Settings.m_SlicesPerUpdate);
    m_sUpdatesPerSecond->SetValue(m_Settings.m_UpdatesPerSecond);
    m_sHourDivider->SetValue(m_Settings.m_HourDivider);
    m_sTransparency->SetValue(m_Settings.m_iOverlayTransparency);
    if(!m_cInterpolate->IsChecked() ) {              //hide no suiting parameters
        m_tSlicesPerUpdate->Hide();
        m_sSlicesPerUpdate->Hide();
        m_tHourDivider->Hide();
        m_sHourDivider->Hide();
    }

    for(int i=0; i<GribOverlaySettings::SETTINGS_COUNT; i++)
        m_cDataType->Append( wxGetTranslation(tname_from_index[i]) );

    m_cDataType->SetSelection(m_lastdatatype);
    PopulateUnits(m_lastdatatype);
    ReadDataTypeSettings(m_lastdatatype);
    Fit();
}

/* set settings to the dialog controls */
void GribSettingsDialog::WriteSettings()
{
    if(m_Settings.m_bInterpolate != m_cInterpolate->GetValue()) {
        m_Settings.m_bInterpolate = m_cInterpolate->GetValue();
        if(m_cInterpolate->IsChecked()) {
            wxMessageDialog mes(this, _("This file contains data for particular time intervals but you have chosen to display different intervals.\nPlease consider that the values will be interpolated."),
                _("Warning!"), wxOK);
            mes.ShowModal();
        }
    }

    m_Settings.m_bLoopMode = m_cLoopMode->GetValue();
    m_Settings.m_LoopStartPoint = m_cLoopStartPoint->GetSelection();
    m_Settings.m_SlicesPerUpdate = m_sSlicesPerUpdate->GetValue();
    m_Settings.m_UpdatesPerSecond = m_sUpdatesPerSecond->GetValue();
    m_Settings.m_HourDivider = m_sHourDivider->GetValue();

    SetDataTypeSettings(m_lastdatatype);

    m_extSettings = m_Settings;
}

void GribSettingsDialog::SetDataTypeSettings(int settings)
{
    GribOverlaySettings::OverlayDataSettings &odc = m_Settings.Settings[settings];
    odc.m_Units = m_cDataUnits->GetSelection();
    odc.m_bBarbedArrows = m_cbBarbedArrows->GetValue();
    odc.m_iBarbedVisibility = m_cBarbedVisibility->GetValue();
    odc.m_iBarbedColour = m_cBarbedColours->GetSelection();
    odc.m_bIsoBars = m_cbIsoBars->GetValue();
    odc.m_iIsoBarVisibility = m_sIsoBarVisibility->GetValue();
    odc.m_iIsoBarSpacing = m_sIsoBarSpacing->GetValue();
    odc.m_bDirectionArrows = m_cbDirectionArrows->GetValue();
    odc.m_iDirectionArrowForm = m_cDirectionArrowForm->GetSelection();
    odc.m_iDirectionArrowSize = m_cDirectionArrowSize->GetSelection();
    odc.m_bOverlayMap = m_cbOverlayMap->GetValue();
    odc.m_iOverlayMapColors = m_cOverlayColors->GetSelection();
    odc.m_bNumbers = m_cbNumbers->GetValue();
    odc.m_iNumbersSpacing = m_sNumbersSpacing->GetValue();

}

void GribSettingsDialog::ReadDataTypeSettings(int settings)
{
    GribOverlaySettings::OverlayDataSettings &odc = m_Settings.Settings[settings];

    m_cDataUnits->SetSelection(odc.m_Units);
    m_cbBarbedArrows->SetValue(odc.m_bBarbedArrows);
    m_cBarbedVisibility->SetValue(odc.m_iBarbedVisibility);
    m_cBarbedColours->SetSelection(odc.m_iBarbedColour);
    m_cbIsoBars->SetValue(odc.m_bIsoBars);
    m_sIsoBarVisibility->SetValue(odc.m_iIsoBarVisibility);
    m_sIsoBarSpacing->SetValue(odc.m_iIsoBarSpacing);
    m_cbDirectionArrows->SetValue(odc.m_bDirectionArrows);
    m_cDirectionArrowForm->SetSelection(odc.m_iDirectionArrowForm);
    m_cDirectionArrowSize->SetSelection(odc.m_iDirectionArrowSize);
    m_cbOverlayMap->SetValue(odc.m_bOverlayMap);
    m_cOverlayColors->SetSelection(odc.m_iOverlayMapColors);
    m_cbNumbers->SetValue(odc.m_bNumbers);
    m_sNumbersSpacing->SetValue(odc.m_iNumbersSpacing);

    ShowFittingSettings(settings);
}

void GribSettingsDialog::ShowFittingSettings( int settings )
{
    //Hide all Parameters
    m_cbBarbedArrows->Show(false);
    m_cBarbedVisibility->Show(false);
    m_cBarbedColours->Show(false);
    m_cbIsoBars->Show(false);
    m_fIsoBarSpacing->ShowItems(false);
    if(m_fIsoBarSpacing->GetCols() == 2) m_fIsoBarSpacing->Detach(1);
    m_fIsoBarSpacing->SetCols(1);
    m_fIsoBarVisibility->ShowItems(false);
    if(m_fIsoBarVisibility->GetCols() == 1 ) m_fIsoBarVisibility->Detach(0);
    m_fIsoBarVisibility->SetCols(0);
    m_cbDirectionArrows->Show(false);
    m_cDirectionArrowForm->Show(false);
    m_cDirectionArrowSize->Show(false);
    m_cbOverlayMap->Show(false);
    m_tOverlayColors->Show(false);
    m_cOverlayColors->Show(false);
    this->Fit();
    //Show only fitting parameters
    switch(settings){
    case 0:
        ShowSettings( ISO_LINE_SHORT );
        m_cbIsoBars->SetLabel(_("Display Isotachs"));
        ShowSettings( B_ARROWS );
        ShowSettings( OVERLAY );
        break;
    case 1:
        ShowSettings( ISO_LINE_SHORT );
        m_cbIsoBars->SetLabel(_("Display Isotachs"));
        ShowSettings( OVERLAY );
        break;
    case 2:
        ShowSettings( ISO_LINE_VISI );
        m_cbIsoBars->SetLabel(_("Display Isobars"));
        break;
    case 3:
    case 4:
        ShowSettings( D_ARROWS );
        ShowSettings( OVERLAY );
        break;
    case 5:
    case 6:
        ShowSettings( OVERLAY );
        break;
    case 7:
    case 8:
        ShowSettings( ISO_LINE_SHORT );
        m_cbIsoBars->SetLabel(_("Display Isotherms"));
        ShowSettings( OVERLAY );
        break;
    case 9:
        ShowSettings( ISO_LINE_SHORT );
        m_cbIsoBars->SetLabel(_("Display Iso CAPE"));
        ShowSettings( OVERLAY );

    }
}

void GribSettingsDialog::ShowSettings( int params )
{
    switch(params){
    case B_ARROWS:
        m_cbBarbedArrows->Show();
        m_cBarbedVisibility->Show();
        m_cBarbedColours->Show();
        break;
    case ISO_LINE_VISI:
        m_cbIsoBars->Show();
        m_fIsoBarSpacing->SetCols(2);
        m_fIsoBarSpacing->Add(m_sIsoBarSpacing, 0, 5,wxALL|wxEXPAND);
        m_fIsoBarSpacing->ShowItems(true);
        m_fIsoBarVisibility->SetCols(1);
        m_fIsoBarVisibility->Add(m_sIsoBarVisibility, 0, 5,wxTOP|wxLEFT|wxEXPAND);
        m_fIsoBarVisibility->ShowItems(true);
        break;
    case ISO_LINE_SHORT:
        m_cbIsoBars->Show();
        m_fIsoBarSpacing->ShowItems(true);
        m_fIsoBarVisibility->SetCols(1);
        m_fIsoBarVisibility->Add(m_sIsoBarSpacing, 0, 5,wxTOP|wxLEFT|wxEXPAND);
        m_fIsoBarVisibility->ShowItems(true);
        break;
    case D_ARROWS:
        m_cbDirectionArrows->Show();
        m_cDirectionArrowForm->Show();
        m_cDirectionArrowSize->Show();
        break;
    case OVERLAY:
        m_cbOverlayMap->Show();
        m_tOverlayColors->Show();
        m_cOverlayColors->Show();
    }
}

void GribSettingsDialog::PopulateUnits(int settings)
{
    m_cDataUnits->Clear();
    for(int i=0; !unit_names[unittype[m_lastdatatype]][i].empty(); i++)
        m_cDataUnits->Append( wxGetTranslation((unit_names[unittype[m_lastdatatype]][i])));
}

void GribSettingsDialog::OnDataTypeChoice( wxCommandEvent& event )
{
    SetDataTypeSettings(m_lastdatatype);
    m_lastdatatype = m_cDataType->GetSelection();
    PopulateUnits(m_lastdatatype);
    ReadDataTypeSettings(m_lastdatatype);
    this->Fit();
    this->Refresh();
}

void GribSettingsDialog::OnTransparencyChange( wxScrollEvent& event  )
{
    m_extSettings = m_Settings;
    m_Settings.m_iOverlayTransparency = m_sTransparency->GetValue();
    m_parent.SetFactoryOptions();
}

void GribSettingsDialog::OnApply( wxCommandEvent& event )
{
    WriteSettings();
    m_parent.SetFactoryOptions(true);
    m_parent.TimelineChanged();
    m_parent.PopulateTrackingControls();
}

void GribSettingsDialog::OnIntepolateChange( wxCommandEvent& event )
{
    if( m_cInterpolate->IsChecked() ) {
        m_tSlicesPerUpdate->Show();
        m_sSlicesPerUpdate->Show();
        m_tHourDivider->Show();
        m_sHourDivider->Show();
    } else {                                        //hide no suiting parameters
        m_tSlicesPerUpdate->Hide();
        m_sSlicesPerUpdate->Hide();
        m_tHourDivider->Hide();
        m_sHourDivider->Hide();
    }
    this->Fit();
    this->Refresh();
}
