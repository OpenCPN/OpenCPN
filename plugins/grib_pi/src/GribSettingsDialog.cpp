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

static const wxString units0_names[] = {_("Knots"), _("m/s"), _("mph"), _("km/h"), wxEmptyString};
static const wxString units1_names[] = {_("MilliBars"), _("mmHG"), wxEmptyString};
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

enum SettingsDisplay {B_ARROWS, ISO_LINES, D_ARROWS, OVERLAY};

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
    pConf->Read ( _T ( "Interpolate" ), &m_bInterpolate, 1 );
    pConf->Read ( _T ( "LoopMode" ), &m_bLoopMode, 0 );
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
        int defrange[SETTINGS_COUNT] = {100, 100, 0, 0, 0, 0, 0, 0, 0, 0};
        pConf->Read ( Name + _T ( "BarbedRange" ), &Settings[i].m_iBarbedRange, defrange[i]);

        pConf->Read ( Name + _T ( "IsoBars" ), &Settings[i].m_bIsoBars, i==PRESSURE);
        double defspacing[SETTINGS_COUNT] = {4, 0, 4, 0, 0, 0, 0, 2, 2, 100};
        pConf->Read ( Name + _T ( "IsoBarSpacing" ), &Settings[i].m_iIsoBarSpacing, defspacing[i]);

        pConf->Read ( Name + _T ( "DirectionArrows" ), &Settings[i].m_bDirectionArrows, i==CURRENT);
        pConf->Read ( Name + _T ( "DirectionArrowSize" ), &Settings[i].m_iDirectionArrowSize, 10);

        pConf->Read ( Name + _T ( "OverlayMap" ), &Settings[i].m_bOverlayMap, i!=WIND && i!=PRESSURE);
        int defcolor[SETTINGS_COUNT] = {1, 1, 0, 0, 6, 4, 5, 2, 3, 0};
        pConf->Read ( Name + _T ( "OverlayMapColors" ), &Settings[i].m_iOverlayMapColors, defcolor[i]);

        pConf->Read ( Name + _T ( "Numbers" ), &Settings[i].m_bNumbers, 0);
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
    pConf->Write ( _T ( "SlicesPerUpdate" ), m_SlicesPerUpdate);
    pConf->Write ( _T ( "UpdatesPerSecond" ), m_UpdatesPerSecond);
    pConf->Write ( _T ( "HourDivider" ), m_HourDivider);
    pConf->Write ( _T ( "OverlayTransparency" ), m_iOverlayTransparency);

    for(int i=0; i<SETTINGS_COUNT; i++) {
        wxString Name=name_from_index[i];

        pConf->Write ( Name + _T ( "Units" ), (int)Settings[i].m_Units);
        pConf->Write ( Name + _T ( "BarbedArrows" ), Settings[i].m_bBarbedArrows);
        pConf->Write ( Name + _T ( "BarbedRange" ), Settings[i].m_iBarbedRange);
        pConf->Write ( Name + _T ( "Display Isobars" ), Settings[i].m_bIsoBars);
        pConf->Write ( Name + _T ( "IsoBarSpacing" ), Settings[i].m_iIsoBarSpacing);
        pConf->Write ( Name + _T ( "DirectionArrows" ), Settings[i].m_bDirectionArrows);
        pConf->Write ( Name + _T ( "DirectionArrowSize" ), Settings[i].m_iDirectionArrowSize);
        pConf->Write ( Name + _T ( "OverlayMap" ), Settings[i].m_bOverlayMap);
        pConf->Write ( Name + _T ( "OverlayMapColors" ), Settings[i].m_iOverlayMapColors);
        pConf->Write ( Name + _T ( "Numbers" ), Settings[i].m_bNumbers);
        pConf->Write ( Name + _T ( "NumbersSpacing" ), Settings[i].m_iNumbersSpacing);
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

double GribOverlaySettings::CalibrationFactor(int settings)
{
    switch(unittype[settings]) {
    case 0: switch(Settings[settings].m_Units) {
        case KNOTS:  return 3.6 / 1.852;
        case M_S:    return 1;
        case MPH:    return 3.6 / 1.60934;
        case KPH:    return 3.6;
        } break;
    case 1: switch(Settings[settings].m_Units) {
        case MILLIBARS: return 1 / 100.;
        case MMHG: return 1 / (100. * 1.33);
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
        case INCHES:      return 25.4;
        } break;
    case 5:
    case 6: return 1;
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
        } break;
        case 1: switch(Settings[settings].m_Units) {
            case MILLIBARS: return _T("hPa");
            case MMHG: return _T("mmHg");
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
    odc.m_bIsoBars = m_cbIsoBars->GetValue();
    odc.m_iIsoBarSpacing = m_sIsoBarSpacing->GetValue();
    odc.m_bDirectionArrows = m_cbDirectionArrows->GetValue();
    odc.m_iDirectionArrowSize = m_sDirectionArrowSize->GetValue();
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
    m_sBarbedRange->SetValue(odc.m_iBarbedRange);
    m_cbIsoBars->SetValue(odc.m_bIsoBars);
    m_sIsoBarSpacing->SetValue(odc.m_iIsoBarSpacing);
    m_cbDirectionArrows->SetValue(odc.m_bDirectionArrows);
    m_sDirectionArrowSize->SetValue(odc.m_iDirectionArrowSize);
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
    m_tBarbedRange->Show(false);
    m_sBarbedRange->Show(false);
    m_cbIsoBars->Show(false);
    m_tIsoBarSpacing->Show(false);
    m_sIsoBarSpacing->Show(false);
    m_cbDirectionArrows->Show(false);
    m_tDirectionArrowSize->Show(false);
    m_sDirectionArrowSize->Show(false);
    m_cbOverlayMap->Show(false);
    m_tOverlayColors->Show(false);
    m_cOverlayColors->Show(false);
    this->Fit();
    //Show only fitting parameters
    switch(settings){
    case 0:
        ShowSettings( ISO_LINES );
        m_cbIsoBars->SetLabel(_("Display Isotachs"));
        ShowSettings( B_ARROWS );
        ShowSettings( OVERLAY );
        break;
    case 1:
        ShowSettings( ISO_LINES );
        m_cbIsoBars->SetLabel(_("Display Isotachs"));
        ShowSettings( OVERLAY );
        break;
    case 2:
        ShowSettings( ISO_LINES );
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
        ShowSettings( ISO_LINES );
        m_cbIsoBars->SetLabel(_("Display Isotherms"));
        ShowSettings( OVERLAY );
        break;
    case 9:
        ShowSettings( ISO_LINES );
        m_cbIsoBars->SetLabel(_("Display Iso CAPE"));
        ShowSettings( OVERLAY );

    }
}

void GribSettingsDialog::ShowSettings( int params )
{
    switch(params){
    case B_ARROWS:
        m_cbBarbedArrows->Show();
        m_tBarbedRange->Show();
        m_sBarbedRange->Show();
        break;
    case ISO_LINES:
        m_cbIsoBars->Show();
        m_tIsoBarSpacing->Show();
        m_sIsoBarSpacing->Show();
        break;
    case D_ARROWS:
        m_cbDirectionArrows->Show();
        m_tDirectionArrowSize->Show();
        m_sDirectionArrowSize->Show();
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
