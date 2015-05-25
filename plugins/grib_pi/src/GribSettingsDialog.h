/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Grib Settings Dialog
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Sean D'Epagnier                                 *
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
    void SaveSettingGroups(wxFileConfig *pConf, int settings, int group);

    double CalibrationOffset(int settings);
    double CalibrationFactor(int settings, double input, bool reverse = false);
    double CalibrateValue(int settings, double input)
        { return (input+CalibrationOffset(settings))*CalibrationFactor(settings, input); }
    int GetMinFromIndex( int index );
    wxString GetAltitudeFromIndex( int index, int unit );
    double GetmstobfFactor(double input);
    double GetbftomsFactor(double input);
    wxString GetUnitSymbol(int settings);
    double GetMin(int settings);
    double GetMax(int settings);
    // playback options
    bool m_bInterpolate;
    bool m_bLoopMode;
    int m_LoopStartPoint;
    int m_SlicesPerUpdate;
    int m_UpdatesPerSecond;
	//display
    int m_iOverlayTransparency;
    //gui
    int m_iCtrlandDataStyle;
    wxString m_iCtrlBarCtrlVisible[2];

    enum SettingsType {WIND, WIND_GUST, PRESSURE, WAVE, CURRENT, PRECIPITATION, CLOUD, 
                       AIR_TEMPERATURE, SEA_TEMPERATURE, CAPE, GEO_ALTITUDE, REL_HUMIDITY, SETTINGS_COUNT};
    enum Units0 {KNOTS, M_S, MPH, KPH, BFS};
    enum Units1 {MILLIBARS, MMHG, INHG};
    enum Units2 {METERS, FEET};
    enum Units3 {CELCIUS, FAHRENHEIT};
    enum Units4 {MILLIMETERS, INCHES};
    enum Units5 {PERCENTAGE};
    enum Units6 {JPKG};

    struct OverlayDataSettings {
        int m_Units;
        bool m_bBarbedArrows;
        bool m_iBarbedVisibility;
        int m_iBarbedColour;
		bool m_bBarbArrFixSpac;
		int m_iBarbArrSpacing;
        bool m_bIsoBars;
        bool m_iIsoBarVisibility;
        double m_iIsoBarSpacing;
        bool m_bDirectionArrows;
        int m_iDirectionArrowForm;
		bool m_bDirArrFixSpac;
        int m_iDirectionArrowSize;
		int m_iDirArrSpacing;
        bool m_bOverlayMap;
        int m_iOverlayMapColors;
        bool m_bNumbers;
		bool m_bNumFixSpac;
        int m_iNumbersSpacing;
        bool m_bParticles;
        double m_dParticleDensity;

    } Settings[SETTINGS_COUNT];
};

class GRIBUICtrlBar;

class GribSettingsDialog : public GribSettingsDialogBase
{
public:
    GribSettingsDialog(GRIBUICtrlBar &parent, GribOverlaySettings &extSettings, int &lastdatatype, int fileIntervalIndex);
    void WriteSettings();

	void SetSettingsDialogSize();
	void SaveLastPage();
	int  GetPageIndex() { return m_SetBookpageIndex; }

private:
    void SetDataTypeSettings(int settings);
    void ReadDataTypeSettings(int settings);
    void PopulateUnits(int settings);
    void ShowFittingSettings (int settings);
    void ShowSettings( int params, bool show = true );
    void OnDataTypeChoice( wxCommandEvent& event );
	void OnUnitChange( wxCommandEvent& event );
    void OnTransparencyChange( wxScrollEvent& event  );
    void OnApply( wxCommandEvent& event );
    void OnIntepolateChange( wxCommandEvent& event );
	void OnSpacingModeChange( wxCommandEvent& event );
	void OnPageChange( wxNotebookEvent& event );
	void OnCtrlandDataStyleChanged( wxCommandEvent& event );

    GRIBUICtrlBar &m_parent;

    GribOverlaySettings m_Settings, &m_extSettings;
    int &m_lastdatatype;
	int m_SetBookpageIndex;
};

#endif
