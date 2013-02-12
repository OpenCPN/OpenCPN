/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Config Dialog
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
static const wxString units1_names[] = {_("MilliBars"), _("hPa"), wxEmptyString};
static const wxString units2_names[] = {_("Meters"), _("Feet"), wxEmptyString};
static const wxString units3_names[] = {_("Celcius"), _("Farenheight"), wxEmptyString};
static const wxString *unit_names[] = {units0_names, units1_names, units2_names, units3_names};

static const wxString name_from_index[] = {_T("Wind"), _T("Pressure"), _T("Wave"),
                                           _T("SeaTemperature"), _T("Current")};
static const int unittype[GribOverlayConfig::CONFIG_COUNT] = {0, 1, 2, 3, 0};

void GribOverlayConfig::Read()
{
    /* read config here */
    wxFileConfig *pConf = GetOCPNConfigObject();;

    if(!pConf)
        return;

    pConf->SetPath ( _T( "/PlugIns/GRIB" ) );
    pConf->Read ( _T ( "Interpolate" ), &m_bInterpolate, 1 );
    pConf->Read ( _T ( "LoopMode" ), &m_bLoopMode, 0 );
    pConf->Read ( _T ( "SlicesPerUpdate" ), &m_SlicesPerUpdate, 2);
    pConf->Read ( _T ( "UpdatesPerSecond" ), &m_UpdatesPerSecond, 2);
    pConf->Read ( _T ( "HourDivider" ), &m_HourDivider, 2);

    for(int i=0; i<CONFIG_COUNT; i++) {
        wxString Name=name_from_index[i];

        int units;
        pConf->Read ( Name + _T ( "Units" ), &units);
        Configs[i].m_Units = (ConfigType)units;

        pConf->Read ( Name + _T ( "BarbedArrows" ), &Configs[i].m_bBarbedArrows, i==WIND);
        pConf->Read ( Name + _T ( "IsoBars" ), &Configs[i].m_bIsoBars, i==PRESSURE);
        int defspacing[CONFIG_COUNT] = {10, 2, 1, 10, 1};
        pConf->Read ( Name + _T ( "IsoBarSpacing" ), &Configs[i].m_iIsoBarSpacing, defspacing[i]);
        pConf->Read ( Name + _T ( "DirectionArrows" ), &Configs[i].m_bDirectionArrows, i==CURRENT);
        pConf->Read ( Name + _T ( "DirectionArrowSize" ), &Configs[i].m_iDirectionArrowSize, 10);
        pConf->Read ( Name + _T ( "OverlayMap" ), &Configs[i].m_bOverlayMap, i!=WIND && i!=PRESSURE);
        int defcolor[CONFIG_COUNT] = {1, 1, 1, 3, 0};
        pConf->Read ( Name + _T ( "OverlayMapColors" ), &Configs[i].m_iOverlayMapColors, defcolor[i]);
        pConf->Read ( Name + _T ( "Numbers" ), &Configs[i].m_bNumbers, i==SEA_TEMPERATURE);
        pConf->Read ( Name + _T ( "NumbersSpacing" ), &Configs[i].m_iNumbersSpacing, 50);
    }
}

void GribOverlayConfig::Write()
{
    /* save config here */
    wxFileConfig *pConf = GetOCPNConfigObject();

    if(!pConf)
        return;

    pConf->SetPath ( _T( "/PlugIns/GRIB" ) );
    pConf->Write ( _T ( "Interpolate" ), m_bInterpolate);
    pConf->Write ( _T ( "LoopMode" ), m_bLoopMode );
    pConf->Write ( _T ( "SlicesPerUpdate" ), m_SlicesPerUpdate);
    pConf->Write ( _T ( "UpdatesPerSecond" ), m_UpdatesPerSecond);
    pConf->Write ( _T ( "HourDivider" ), m_HourDivider);

    for(int i=0; i<CONFIG_COUNT; i++) {
        wxString Name=name_from_index[i];

        pConf->Write ( Name + _T ( "Units" ), (int)Configs[i].m_Units);
        pConf->Write ( Name + _T ( "BarbedArrows" ), Configs[i].m_bBarbedArrows);
        pConf->Write ( Name + _T ( "IsoBars" ), Configs[i].m_bIsoBars);
        pConf->Write ( Name + _T ( "IsoBarSpacing" ), Configs[i].m_iIsoBarSpacing);
        pConf->Write ( Name + _T ( "DirectionArrows" ), Configs[i].m_bDirectionArrows);
        pConf->Write ( Name + _T ( "DirectionArrowSize" ), Configs[i].m_iDirectionArrowSize);
        pConf->Write ( Name + _T ( "OverlayMap" ), Configs[i].m_bOverlayMap);
        pConf->Write ( Name + _T ( "OverlayMapColors" ), Configs[i].m_iOverlayMapColors);
        pConf->Write ( Name + _T ( "Numbers" ), Configs[i].m_bNumbers);
        pConf->Write ( Name + _T ( "NumbersSpacing" ), Configs[i].m_iNumbersSpacing);
    }
}

double GribOverlayConfig::CalibrationFactor(int config)
{
    switch(unittype[config]) {
    case 0: switch(Configs[config].m_Units) {
        case KNOTS:  return 3.6 / 1.852;
        case M_S:    return 1;
        case MPH:    return 3.6 / 1.852 * 1.15;
        case KPH:    return 3.6 / 1.852 * 1.95;
        } break;
    case 1: switch(Configs[config].m_Units) {
        case MILLIBARS: return 1 / 100.;
        case HPA: return 0;
        } break;
    case 2: switch(Configs[config].m_Units) {
        case METERS: return 1;
        case FEET:   return 3.3;
        } break;
    case 3: switch(Configs[config].m_Units) {
        case CELCIUS:     return 1;
        case FARENHEIGHT: return 5/9. + 32;
        } break;
    }
        
    return 1;
}

double GribOverlayConfig::GetMin(int config)
{
    double min = 0;
    switch(config) {
    case WIND:            min = 0;       break; /* m/s */
    case PRESSURE:        min = 84000;   break; /* 100's of millibars */
    case WAVE:            min = 0;       break; /* meters */
    case SEA_TEMPERATURE: min = 273.15;  break; /* kelvin */
    case CURRENT:         min = 0;       break; /* m/s */
    }
    return CalibrateValue(config, min);
}

double GribOverlayConfig::GetMax(int config)
{
    double max = 0;
    switch(config) {
    case WIND:            max = 200;     break; /* m/s */
    case PRESSURE:        max = 112000;  break; /* 100s of millibars */
    case WAVE:            max = 12;      break; /* meters */
    case SEA_TEMPERATURE: max = 323.15;  break; /* kelvin */
    case CURRENT:         max = 20;      break; /* m/s */
    }
    return CalibrateValue(config, max);
}

GRIBConfigDialog::GRIBConfigDialog(wxWindow *parent)
  : GRIBConfigDialogBase(parent)
{
    m_lastdatatype = 0;
    PopulateUnits(m_lastdatatype);
}

/* set the dialog controls to the values of config */
void GRIBConfigDialog::ReadConfig(GribOverlayConfig &Config)
{
    m_Config = Config;

    m_cInterpolate->SetValue(m_Config.m_bInterpolate);
    m_cLoopMode->SetValue(m_Config.m_bLoopMode);
    m_sSlicesPerUpdate->SetValue(m_Config.m_SlicesPerUpdate);
    m_sUpdatesPerSecond->SetValue(m_Config.m_UpdatesPerSecond);
    m_sHourDivider->SetValue(m_Config.m_HourDivider);

    ReadDataTypeConfig(0);
}

/* set config to the dialog controls */
void GRIBConfigDialog::WriteConfig(GribOverlayConfig &Config)
{
    m_Config.m_bInterpolate = m_cInterpolate->GetValue();
    m_Config.m_bLoopMode = m_cLoopMode->GetValue();
    m_Config.m_SlicesPerUpdate = m_sSlicesPerUpdate->GetValue();
    m_Config.m_UpdatesPerSecond = m_sUpdatesPerSecond->GetValue();
    m_Config.m_HourDivider = m_sHourDivider->GetValue();

    SetDataTypeConfig(m_lastdatatype);

    Config = m_Config;
}

void GRIBConfigDialog::SetDataTypeConfig(int config)
{
    GribOverlayConfig::OverlayDataConfig &odc = m_Config.Configs[config];
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

void GRIBConfigDialog::ReadDataTypeConfig(int config)
{
    GribOverlayConfig::OverlayDataConfig &odc = m_Config.Configs[config];
    m_cDataUnits->SetSelection(odc.m_Units);
    m_cbBarbedArrows->SetValue(odc.m_bBarbedArrows);
    m_cbIsoBars->SetValue(odc.m_bIsoBars);
    m_sIsoBarSpacing->SetValue(odc.m_iIsoBarSpacing);
    m_cbDirectionArrows->SetValue(odc.m_bDirectionArrows);
    m_sDirectionArrowSize->SetValue(odc.m_iDirectionArrowSize);
    m_cbOverlayMap->SetValue(odc.m_bOverlayMap);
    m_cOverlayColors->SetSelection(odc.m_iOverlayMapColors);
    m_cbNumbers->SetValue(odc.m_bNumbers);
    m_sNumbersSpacing->SetValue(odc.m_iNumbersSpacing);
}

void GRIBConfigDialog::PopulateUnits(int config)
{
    m_cDataUnits->Clear();
    for(int i=0; !unit_names[unittype[m_lastdatatype]][i].empty(); i++)
        m_cDataUnits->Append(unit_names[unittype[m_lastdatatype]][i]);
}

void GRIBConfigDialog::OnDataTypeChoice( wxCommandEvent& event )
{
    SetDataTypeConfig(m_lastdatatype);
    m_lastdatatype = m_cDataType->GetSelection();
    PopulateUnits(m_lastdatatype);
    ReadDataTypeConfig(m_lastdatatype);
}
