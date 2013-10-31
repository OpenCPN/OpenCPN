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

#ifndef __GRIBSETTINGSDIALOG_H__
#define __GRIBSETTINGSDIALOG_H__

#include "GribUIDialogBase.h"

//----------------------------------------------------------------------------------------------------------
//    Grib OverlaySettings Specification
//----------------------------------------------------------------------------------------------------------
struct GribOverlaySettings
{
    static wxString NameFromIndex(int index);

    void Read();
    void Write();

    double CalibrationOffset(int settings);
    double CalibrationFactor(int settings);
    double CalibrateValue(int settings, double input)
        { return (input+CalibrationOffset(settings))*CalibrationFactor(settings); }
    wxString GetUnitSymbol(int settings);
    double GetMin(int settings);
    double GetMax(int settings);
    // settings options
    bool m_bInterpolate;
    bool m_bLoopMode;
    int m_SlicesPerUpdate;
    int m_UpdatesPerSecond;
    int m_HourDivider;
    int m_iOverlayTransparency;

    enum SettingsType {WIND, WIND_GUST, PRESSURE, WAVE, CURRENT, PRECIPITATION, CLOUD, 
                       AIR_TEMPERATURE, SEA_TEMPERATURE, CAPE, SETTINGS_COUNT};
    enum Units0 {KNOTS, M_S, MPH, KPH};
    enum Units1 {MILLIBARS, MMHG};
    enum Units2 {METERS, FEET};
    enum Units3 {CELCIUS, FAHRENHEIT};
    enum Units4 {MILLIMETERS, INCHES};
    enum Units5 {PERCENTAGE};
    enum Units6 {JPKG};

    struct OverlayDataSettings {
        int m_Units;
        bool m_bBarbedArrows;
        int m_iBarbedRange;
        bool m_bIsoBars;
        double m_iIsoBarSpacing;
        bool m_bDirectionArrows;
        int m_iDirectionArrowSize;
        bool m_bOverlayMap;
        int m_iOverlayMapColors;
        bool m_bNumbers;
        int m_iNumbersSpacing;

    } Settings[SETTINGS_COUNT];
};

class GRIBUIDialog;

class GribSettingsDialog : public GribSettingsDialogBase
{
public:
    GribSettingsDialog(GRIBUIDialog &parent, GribOverlaySettings &extSettings, int &lastdatatype);
    void WriteSettings();

private:
    void SetDataTypeSettings(int settings);
    void ReadDataTypeSettings(int settings);
    void PopulateUnits(int settings);
    void ShowFittingSettings (int settings);
    void ShowSettings( int params );
    void OnDataTypeChoice( wxCommandEvent& event );
    void OnTransparencyChange( wxScrollEvent& event  );
    void OnApply( wxCommandEvent& event );
    void OnIntepolateChange( wxCommandEvent& event );

    GRIBUIDialog &m_parent;

    GribOverlaySettings m_Settings, &m_extSettings;
    int &m_lastdatatype;
};

#endif
