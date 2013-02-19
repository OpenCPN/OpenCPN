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

#ifndef __GRIBCONFIGDIALOG_H__
#define __GRIBCONFIGDIALOG_H__

#include "GribUIDialogBase.h"

//----------------------------------------------------------------------------------------------------------
//    Grib OverlayConfig Specification
//----------------------------------------------------------------------------------------------------------
struct GribOverlayConfig
{
    static wxString NameFromIndex(int index);

    void Read();
    void Write();

    double CalibrationOffset(int config);
    double CalibrationFactor(int config);
    double CalibrateValue(int config, double input)
        { return (input+CalibrationOffset(config))*CalibrationFactor(config); }
    double GetMin(int config);
    double GetMax(int config);
    // config options
    bool m_bInterpolate;
    bool m_bLoopMode;
    int m_SlicesPerUpdate;
    int m_UpdatesPerSecond;
    double m_HourDivider;

    enum ConfigType {WIND, WAVE, CURRENT, PRESSURE,
                     SEA_TEMPERATURE, CONFIG_COUNT};
    enum Units0 {KNOTS, M_S, MPH, KPH};
    enum Units1 {MILLIBARS, MMHG};
    enum Units2 {METERS, FEET};
    enum Units3 {CELCIUS, FARENHEIGHT};

    struct OverlayDataConfig {
        int m_Units;
        bool m_bBarbedArrows;
        int m_iBarbedRange;
        bool m_bIsoBars;
        int m_iIsoBarSpacing;
        bool m_bDirectionArrows;
        int m_iDirectionArrowSize;
        bool m_bOverlayMap;
        int m_iOverlayMapColors;
        bool m_bNumbers;
        int m_iNumbersSpacing;

    } Configs[CONFIG_COUNT];
};

class GRIBConfigDialog : public GRIBConfigDialogBase
{
public:
    GRIBConfigDialog(GRIBUIDialog &parent, GribOverlayConfig &extConfig, int &lastdatatype);
    void WriteConfig();

private:
    void SetDataTypeConfig(int config);
    void ReadDataTypeConfig(int config);
    void PopulateUnits(int config);
    void OnDataTypeChoice( wxCommandEvent& event );
    void OnApply( wxCommandEvent& event );

    GRIBUIDialog &m_parent;

    GribOverlayConfig m_Config, &m_extConfig;
    int &m_lastdatatype;
};

#endif
