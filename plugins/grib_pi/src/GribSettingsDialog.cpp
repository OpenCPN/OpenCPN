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

static const wxString units0_names[] = {_("Knots"), _("M/S"), _("MPH"), _("KPH"), wxEmptyString};
static const wxString units1_names[] = {_("MilliBars"), _("mmHG"), wxEmptyString};
static const wxString units2_names[] = {_("Meters"), _("Feet"), wxEmptyString};
static const wxString units3_names[] = {_("Celcius"), _("Fahrenheit"), wxEmptyString};
static const wxString units4_names[] = {_("Millimeters"), _("Inches"), wxEmptyString};
static const wxString units5_names[] = {_("Percentage"), wxEmptyString};
static const wxString *unit_names[] = {units0_names, units1_names, units2_names,
                                       units3_names, units4_names, units5_names};

static const wxString name_from_index[] = {_("Wind"), _("WindGust"), _("Pressure"),
                                           _("Wave"), _("Current"),
                                           _("Precipitation"), _("CloudCover"),
                                           _("AirTemperature"), _("SeaTemperature")};
static const wxString tname_from_index[] = {_("Wind"), _("Wind Gust"),  _("Pressure"),
                                            _("Wave"), _("Current"),
                                            _("Precipitation"), _("Cloud Cover"),
                                            _("Air Temperature"), _("Sea Temperature")};

static const int unittype[GribOverlaySettings::SETTINGS_COUNT] = {0, 0, 1, 2, 0, 4, 5, 3, 3};

wxString GribOverlaySettings::NameFromIndex(int index)
{
    return tname_from_index[index];
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

    for(int i=0; i<SETTINGS_COUNT; i++) {
        wxString Name=name_from_index[i];

        int units;
        pConf->Read ( Name + _T ( "Units" ), &units);
        Settings[i].m_Units = (SettingsType)units;

        pConf->Read ( Name + _T ( "BarbedArrows" ), &Settings[i].m_bBarbedArrows, i==WIND);
        int defrange[SETTINGS_COUNT] = {100, 100, 100, 10, 10, 1, 10, 1, 1};
        pConf->Read ( Name + _T ( "BarbedRange" ), &Settings[i].m_iBarbedRange, defrange[i]);

        pConf->Read ( Name + _T ( "IsoBars" ), &Settings[i].m_bIsoBars, i==PRESSURE);
        double defspacing[SETTINGS_COUNT] = {10, 10, 10, 1, 2, .1, 10, 1, 1};
        pConf->Read ( Name + _T ( "IsoBarSpacing" ), &Settings[i].m_iIsoBarSpacing, defspacing[i]);

        pConf->Read ( Name + _T ( "DirectionArrows" ), &Settings[i].m_bDirectionArrows, i==CURRENT);
        pConf->Read ( Name + _T ( "DirectionArrowSize" ), &Settings[i].m_iDirectionArrowSize, 10);

        pConf->Read ( Name + _T ( "OverlayMap" ), &Settings[i].m_bOverlayMap, i!=WIND && i!=PRESSURE);
        int defcolor[SETTINGS_COUNT] = {1, 1, 1, 1, 0, 4, 5, 3, 3};
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

    for(int i=0; i<SETTINGS_COUNT; i++) {
        wxString Name=name_from_index[i];

        pConf->Write ( Name + _T ( "Units" ), (int)Settings[i].m_Units);
        pConf->Write ( Name + _T ( "BarbedArrows" ), Settings[i].m_bBarbedArrows);
        pConf->Write ( Name + _T ( "BarbedRange" ), Settings[i].m_iBarbedRange);
        pConf->Write ( Name + _T ( "IsoBars" ), Settings[i].m_bIsoBars);
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
        case MPH:    return 3.6 / 1.852 * 1.15;
        case KPH:    return 3.6 / 1.852 * 1.85;
        } break;
    case 1: switch(Settings[settings].m_Units) {
        case MILLIBARS: return 1 / 100.;
        case MMHG: return 1 / 100. * 1.33;
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
    case 5: return 1;
    }
        
    return 1;
}

double GribOverlaySettings::GetMin(int settings)
{
    double min = 0;
    switch(settings) {
    case PRESSURE:        min = 84000;   break; /* 100's of millibars */
    case AIR_TEMPERATURE: min = 273.15-100; break; /* kelvin */
    case SEA_TEMPERATURE: min = 273.15-10;  break; /* kelvin */
    }
    return CalibrateValue(settings, min);
}

double GribOverlaySettings::GetMax(int settings)
{
    double max = 0;
    switch(settings) {
    case WIND:            max = 200;     break; /* m/s */
    case WIND_GUST:       max = 200;     break; /* m/s */
    case PRESSURE:        max = 112000;  break; /* 100s of millibars */
    case WAVE:            max = 12;      break; /* meters */
    case CURRENT:         max = 20;      break; /* m/s */
    case PRECIPITATION:   max = 80;      break; /* mm */
    case CLOUD:           max = 100;     break; /* percent */
    case AIR_TEMPERATURE: max = 273.15+100;  break; /* kelvin */
    case SEA_TEMPERATURE: max = 273.15+50;  break; /* kelvin */
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

    for(int i=0; i<GribOverlaySettings::SETTINGS_COUNT; i++)
        m_cDataType->Append(tname_from_index[i]);

    m_cDataType->SetSelection(m_lastdatatype);
    PopulateUnits(m_lastdatatype);
    ReadDataTypeSettings(m_lastdatatype);
}

/* set settings to the dialog controls */
void GribSettingsDialog::WriteSettings()
{
    m_Settings.m_bInterpolate = m_cInterpolate->GetValue();
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
}

void GribSettingsDialog::PopulateUnits(int settings)
{
    m_cDataUnits->Clear();
    for(int i=0; !unit_names[unittype[m_lastdatatype]][i].empty(); i++)
        m_cDataUnits->Append(unit_names[unittype[m_lastdatatype]][i]);
}

void GribSettingsDialog::OnDataTypeChoice( wxCommandEvent& event )
{
    SetDataTypeSettings(m_lastdatatype);
    m_lastdatatype = m_cDataType->GetSelection();
    PopulateUnits(m_lastdatatype);
    ReadDataTypeSettings(m_lastdatatype);
}

void GribSettingsDialog::OnApply( wxCommandEvent& event )
{
    WriteSettings();
    m_parent.SetFactoryOptions();
}
