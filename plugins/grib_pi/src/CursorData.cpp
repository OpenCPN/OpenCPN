/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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

#include "folder.xpm"

extern int m_Altitude;
extern double m_cursor_lat, m_cursor_lon;
extern int m_DialogStyle;
#if defined (_WIN32)
#define round(x) wxRound(x) //extern int round (double x);
#endif

enum SettingsDisplay {B_ARROWS, ISO_LINE, D_ARROWS, OVERLAY, NUMBERS, PARTICLES};

//---------------------------------------------------------------------------------------
//               GRIB Cursor Data  implementation
//---------------------------------------------------------------------------------------
CursorData::CursorData( wxWindow *window, GRIBUICtrlBar &parent )
		: CursorDataBase(window),m_gparent(parent)
{

	//transform checkboxes ID to have a formal link to data type and set the initial value
    wxWindowListNode *node =  this->GetChildren().GetFirst();
    while( node ) {
        wxWindow *win = node->GetData();
        if( win->IsKindOf(CLASSINFO(wxCheckBox)) ) {
			int winId = ((wxCheckBox*) win )->GetId() - ID_CB_WIND;
			((wxCheckBox*) win )->SetId( winId );
            ((wxCheckBox*) win )->SetValue( m_gparent.m_bDataPlot[winId] );
		}
		node = node->GetNext();
	}

    m_bLeftDown = false;

    m_tCursorTrackTimer.Connect(wxEVT_TIMER, wxTimerEventHandler( CursorData::OnCursorTrackTimer ), NULL, this);

	DimeWindow( this );
}

void CursorData::OnCBAny( wxCommandEvent& event )
{
	int id = event.GetId();
	wxWindow *win = this->FindWindow( id );
	m_gparent.m_bDataPlot[id] = ((wxCheckBox*) win )->IsChecked();
    ResolveDisplayConflicts( id );
}

void CursorData::ResolveDisplayConflicts( int Id )
{
    //allow multi selection only if there is no display type superposition
	for( int i = 0; i < GribOverlaySettings::GEO_ALTITUDE; i++ ) {
		if( i != Id && m_gparent.m_bDataPlot[i] ) {
			if( (m_gparent.m_OverlaySettings.Settings[Id].m_bBarbedArrows &&
					m_gparent.m_OverlaySettings.Settings[i].m_bBarbedArrows)
					|| (m_gparent.m_OverlaySettings.Settings[Id].m_bDirectionArrows &&
					m_gparent.m_OverlaySettings.Settings[i].m_bDirectionArrows)
					|| (m_gparent.m_OverlaySettings.Settings[Id].m_bIsoBars &&
					m_gparent.m_OverlaySettings.Settings[i].m_bIsoBars)
					|| (m_gparent.m_OverlaySettings.Settings[Id].m_bNumbers &&
					m_gparent.m_OverlaySettings.Settings[i].m_bNumbers)
					|| (m_gparent.m_OverlaySettings.Settings[Id].m_bOverlayMap &&
					m_gparent.m_OverlaySettings.Settings[i].m_bOverlayMap)
					|| (m_gparent.m_OverlaySettings.Settings[Id].m_bParticles &&
					m_gparent.m_OverlaySettings.Settings[i].m_bParticles) ) {
				m_gparent.m_bDataPlot[i] = false;
				wxWindow *win = FindWindow(i);
				((wxCheckBox*) win )->SetValue( false );
			}
		}
	}
    m_gparent.SetFactoryOptions();                     // Reload the visibility options
}

void CursorData::AddTrackingControl( wxControl *ctrl1,  wxControl *ctrl2,  wxControl *ctrl3, wxControl *ctrl4, bool show,
        bool vertical, int wctrl2, int wctrl3_4)
{
    if(show) {
        m_fgTrackingControls->Add(ctrl1, 0, wxALL, 1);
        ctrl1->Show();
        if(ctrl2) {
            m_fgTrackingControls->Add(ctrl2, 0, wxALL, 0);
            ctrl2->SetMinSize(wxSize(wctrl2, -1));
            ctrl2->Show();
        } else
              m_fgTrackingControls->Add(0, 0, 1, wxALL, 1); /* spacer */

        if(ctrl3) {
            long flag1 = wxALIGN_CENTER;
            long flag  = vertical? flag1: wxALL;
            m_fgTrackingControls->Add(ctrl3, 0, flag, 0);
            ctrl3->SetMinSize(wxSize(wctrl3_4, -1));
            ctrl3->Show();
        } else
            if( !vertical ) m_fgTrackingControls->Add(0, 0, 1, wxALL, 1); /* spacer */

        if(ctrl4) {
            m_fgTrackingControls->Add(ctrl4, 0, wxALL, 0);
            ctrl4->SetMinSize(wxSize(wctrl3_4, -1));
            ctrl4->Show();
        } else
            if( !vertical ) m_fgTrackingControls->Add(0, 0, 1, wxALL, 1 ); /* spacer */

    } else {
        if(ctrl1) ctrl1->Hide();
        if(ctrl2) ctrl2->Hide();
        if(ctrl3) ctrl3->Hide();
        if(ctrl4) ctrl4->Hide();
    }
}

void CursorData::PopulateTrackingControls( bool vertical )
{
    m_fgTrackingControls->Clear();
    m_fgTrackingControls->SetCols( vertical ? 2 : 12);
    this->Fit();
    //Get text controls sizing data
    wxFont *font = OCPNGetFont(_("Dialog"), 10);
    int wn, wd, ws,wl;
    GetTextExtent( _T("abcdefghih"), &wn, NULL, 0, 0, font); // normal width text control size
    GetTextExtent( _T("abcdef"), &ws, NULL, 0, 0, font); // short width text control size for direction only
    GetTextExtent( _T("abcdefghijklmopq"), &wd, NULL, 0, 0, font); // long width text control size for double unit wind display
    GetTextExtent( _T("abcdefghijklm"), &wl, NULL, 0, 0, font); // long width text control size for double unit wave display
    //
    bool bf = m_gparent.m_OverlaySettings.Settings[GribOverlaySettings::WIND].m_Units == GribOverlaySettings::BFS;
    wd = vertical ? wn: bf? wn: wd;
    wl = vertical? wn : wl;

    AddTrackingControl(m_cbWind, m_tcWindSpeed, m_tcWindSpeedBf, m_tcWindDirection, false, vertical, 0, 0 ); //hide all wind's parameters
    AddTrackingControl(m_cbWind, m_tcWindSpeed, vertical? (bf ? m_tcWindDirection: m_tcWindSpeedBf) : m_tcWindDirection, vertical? (bf ? 0: m_tcWindDirection): 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VX) != wxNOT_FOUND
        && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VY) != wxNOT_FOUND, vertical, wd, ws);
    AddTrackingControl(m_cbWindGust, m_tcWindGust, 0, 0, m_gparent.m_pTimelineSet
        && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_GUST) != wxNOT_FOUND
		&& m_Altitude == 0, vertical, wn);
    AddTrackingControl(m_cbPressure, m_tcPressure, 0, 0, m_gparent.m_pTimelineSet
        && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRESSURE) != wxNOT_FOUND
		&& m_Altitude == 0, vertical, wn);

    /* tracking for wave is funky */
    AddTrackingControl(m_cbWave, m_tcWaveHeight, m_tcWavePeriode, m_tcWaveDirection, false, vertical, 0, 0); //hide all waves's parameters
    if(m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_HTSIGW) != wxNOT_FOUND) {
        if(m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVDIR) != wxNOT_FOUND)
            AddTrackingControl(m_cbWave, m_tcWaveHeight, vertical? m_tcWavePeriode: m_tcWaveDirection,
                    vertical? m_tcWaveDirection: 0 , m_Altitude == 0, vertical, wl, ws);
        else
            AddTrackingControl(m_cbWave, m_tcWaveHeight, 0, vertical? m_tcWavePeriode: 0, m_Altitude == 0, vertical, wn, ws);
    } else {
        if(m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVDIR) != wxNOT_FOUND)
            AddTrackingControl(m_cbWave, m_tcWaveDirection, 0,0 , m_Altitude == 0, vertical, ws, ws);
    }

    AddTrackingControl(m_cbCurrent, m_tcCurrentVelocity, m_tcCurrentDirection, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEACURRENT_VX) != wxNOT_FOUND
        && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEACURRENT_VY) != wxNOT_FOUND
		&& m_Altitude == 0, vertical, wn, ws);
    AddTrackingControl(m_cbPrecipitation, m_tcPrecipitation, 0, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRECIP_TOT) != wxNOT_FOUND
		&& m_Altitude == 0, vertical, wn);
    AddTrackingControl(m_cbCloud, m_tcCloud, 0, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CLOUD_TOT) != wxNOT_FOUND
		&& m_Altitude == 0, vertical, wn);
    AddTrackingControl(m_cbAirTemperature, m_tcAirTemperature, 0, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_AIR_TEMP) != wxNOT_FOUND
		&& m_Altitude == 0, vertical, wn);
    AddTrackingControl(m_cbSeaTemperature, m_tcSeaTemperature, 0, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEA_TEMP) != wxNOT_FOUND
		&& m_Altitude == 0, vertical, wn);
    AddTrackingControl(m_cbCAPE, m_tcCAPE, 0, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CAPE) != wxNOT_FOUND
		&& m_Altitude == 0, vertical, wn);
    //
    //init and show extra parameters for altitude tracking if necessary
	AddTrackingControl(m_cbAltitude, m_tcAltitude, 0, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_GEOP_HGT + m_Altitude) != wxNOT_FOUND
		&& m_Altitude != 0, vertical, wn);
	AddTrackingControl(m_cbTemp, m_tcTemp, 0, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_AIR_TEMP + m_Altitude) != wxNOT_FOUND
		&& m_Altitude != 0, vertical, wn);
	AddTrackingControl(m_cbRelHumid, m_tcRelHumid, 0, 0,
        m_gparent.m_pTimelineSet && m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_HUMID_RE + m_Altitude) != wxNOT_FOUND
		&& m_Altitude != 0, vertical, wn);
	//
    m_stTrackingText->SetLabel( _("Data at cursor position") );
    //add tooltips
    wxString t; double lev;

    lev = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::GEO_ALTITUDE, 10 );     //convert 10m in current altitude unit
    t.Printf(
        m_Altitude ? m_gparent.m_OverlaySettings.GetAltitudeFromIndex(
                m_Altitude, m_gparent.m_OverlaySettings.Settings[GribOverlaySettings::PRESSURE].m_Units)
                .Append( _T(" ") ).Append( m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRESSURE) )
           : wxString::Format( _T("%1.*f "), lev == (int) lev ? 0 : 1, lev )
                .Append( m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::GEO_ALTITUDE) )
                );
    m_tcWindSpeed->SetToolTip( _("Wind Speed at") + t.Prepend( _T(" ") ) );
    m_tcWindSpeedBf->SetToolTip( _("Wind Speed in at") + t.Prepend( _T(" ") ) );
    m_tcWindDirection->SetToolTip( _("Wind Direction at") + t );

    t.Printf( _T(" %1.*f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::GEO_ALTITUDE), lev == (int) lev ? 0 : 1, lev );
    m_tcWindGust->SetToolTip( _("Wind Gust at") + t );

    if( m_gparent.m_pTimelineSet ) {
        wxString s[] = { _T(" "), _("Air Temperature at"), _("Surface level"), _("Sea Surface Temperature") };

        lev = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::GEO_ALTITUDE, 2 );      //convert 2m in current altitude unit
        t.Printf( m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index(1000 + NORWAY_METNO) != wxNOT_FOUND ? s[0] + s[2]
            : _T(" %1.*f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::GEO_ALTITUDE), lev == (int) lev ? 0 : 1, lev );
        m_tcAirTemperature->SetToolTip(s[1] + t );

        m_tcSeaTemperature->SetToolTip( m_gparent.m_bGRIBActiveFile->m_GribIdxArray.Index( 1000 + NOAA_GFS) != wxNOT_FOUND ? s[1] + s[0] + s[2] : s[3] );
    }
}

void CursorData::UpdateTrackingControls( void )
{
    if( !m_gparent.m_pTimelineSet )
        return;

    GribRecord **RecordArray = m_gparent.m_pTimelineSet->m_GribRecordPtrArray;
    //    Update the wind control
    double vkn, ang;
    if(GribRecord::getInterpolatedValues(vkn, ang,
                                         RecordArray[Idx_WIND_VX + m_Altitude],
                                         RecordArray[Idx_WIND_VY + m_Altitude],
                                         m_cursor_lon, m_cursor_lat)) {
        double vk = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND, vkn);

        m_tcWindSpeed->SetValue( wxString::Format( _T("%3d ") + m_gparent.m_OverlaySettings.GetUnitSymbol
                                                   (GribOverlaySettings::WIND) , (int)round( vk )) );

        //wind is a special case: if current unit is not bf ==> double speed display (current unit + bf)
        if(m_gparent.m_OverlaySettings.Settings[GribOverlaySettings::WIND].m_Units != GribOverlaySettings::BFS) {
            vk = m_gparent.m_OverlaySettings.GetmstobfFactor(vkn)* vkn;
            if( m_DialogStyle == SEPARATED_VERTICAL )
                m_tcWindSpeedBf->SetValue( wxString::Format( _T("%2d bf"), (int)round( vk )));
            else
                m_tcWindSpeed->SetValue(m_tcWindSpeed->GetValue().Append(_T(" - "))
                        .Append(wxString::Format(_T("%2d bf"), (int)round( vk))) );
        }

        m_tcWindDirection->SetValue( wxString::Format( _T("%03d\u00B0"), (int) ( ang ) ));
    } else {
        m_tcWindSpeed->SetValue( _("N/A") );
        m_tcWindSpeedBf->SetValue( _("N/A") );
        m_tcWindDirection->SetValue(  _("N/A") );
    }

    //    Update the Wind gusts control
    if( RecordArray[Idx_WIND_GUST] ) {
        double vkn = RecordArray[Idx_WIND_GUST]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( vkn != GRIB_NOTDEF ) {
            vkn = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND_GUST, vkn);
            m_tcWindGust->SetValue( wxString::Format(_T("%2d ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WIND_GUST), (int)round( vkn )) );
        } else
            m_tcWindGust->SetValue( _("N/A") );
    }

    //    Update the Pressure control
    if( RecordArray[Idx_PRESSURE] ) {
        double press = RecordArray[Idx_PRESSURE]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( press != GRIB_NOTDEF ) {
            press = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::PRESSURE, press);
            int p = (m_gparent.m_OverlaySettings.Settings[GribOverlaySettings::PRESSURE].m_Units == 2) ? 2 : 0;  // if PRESSURE & inHG = two decimals
            m_tcPressure->SetValue( wxString::Format(_T("%2.*f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRESSURE), p, ( press )) );
        } else
            m_tcPressure->SetValue( _("N/A") );
    }

    //    Update the Sig Wave Height
    if( RecordArray[Idx_HTSIGW] ) {
        double height = RecordArray[Idx_HTSIGW]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( height != GRIB_NOTDEF ) {
            height = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::WAVE, height);
            wxString w( wxString::Format( _T("%4.1f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WAVE), height ));
            if( RecordArray[Idx_WVPER] ) {
                double period = RecordArray[Idx_WVPER]->
                    getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
                if( period != GRIB_NOTDEF ) {
                    if( m_DialogStyle == SEPARATED_VERTICAL )
                        m_tcWavePeriode->SetValue( wxString::Format(_T("%01ds") , (int) round(period)) );
                    else
                        w.Append( wxString::Format( _T(" - %01ds") , (int) round(period) ));
                } else
                    m_tcWavePeriode->SetValue( _("N/A") );
            } else
                m_tcWavePeriode->SetValue( _("N/A") );

            m_tcWaveHeight->SetValue(w);
        } else
            m_tcWaveHeight->SetValue( _("N/A") );
    }

    // Update the Wave direction
    if( RecordArray[Idx_WVDIR] ) {
        double direction = RecordArray[Idx_WVDIR]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true, true );
        if( direction != GRIB_NOTDEF )
            m_tcWaveDirection->SetValue( wxString::Format( _T("%03d\u00B0"), (int)direction ));
        else
            m_tcWaveDirection->SetValue( _("N/A") );
    }

    //    Update the Current control
    if(GribRecord::getInterpolatedValues(vkn, ang,
                                         RecordArray[Idx_SEACURRENT_VX],
                                         RecordArray[Idx_SEACURRENT_VY],
                                         m_cursor_lon, m_cursor_lat)) {
        vkn = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::CURRENT, vkn);

        m_tcCurrentVelocity->SetValue( wxString::Format( _T("%4.1f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CURRENT), vkn ) );

        m_tcCurrentDirection->SetValue( wxString::Format( _T("%03d\u00B0"), (int) ( ang ) ) );
    } else {
        m_tcCurrentVelocity->SetValue( _("N/A") );
        m_tcCurrentDirection->SetValue( _("N/A") );
    }

    //    Update total rainfall control
    if( RecordArray[Idx_PRECIP_TOT] ) {
        double precip = RecordArray[Idx_PRECIP_TOT]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( precip != GRIB_NOTDEF ) {
            precip = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::PRECIPITATION, precip);
            int p = precip < 10. ? 2 : precip < 100. ? 1 : 0;
            p += m_gparent.m_OverlaySettings.Settings[GribOverlaySettings::PRECIPITATION].m_Units == 1 ? 1 : 0 ;  // if PRESSURE & in = one decimal more
            m_tcPrecipitation->SetValue( wxString::Format( _T("%4.*f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRECIPITATION), p, precip ) );
        } else
            m_tcPrecipitation->SetValue( _("N/A") );
    }

    //    Update total cloud control
    if( RecordArray[Idx_CLOUD_TOT] ) {
        double cloud = RecordArray[Idx_CLOUD_TOT]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( cloud != GRIB_NOTDEF ) {
            cloud = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::CLOUD, cloud);
            wxString val( wxString::Format( _T("%5.0f "), cloud ) );
            m_tcCloud->SetValue( val + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CLOUD) );
        } else
            m_tcCloud->SetValue( _("N/A") );
    }

    //    Update the Air Temperature
    if( RecordArray[Idx_AIR_TEMP] ) {
        double temp = RecordArray[Idx_AIR_TEMP]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( temp != GRIB_NOTDEF ) {
            temp = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::AIR_TEMPERATURE, temp);
            m_tcAirTemperature->SetValue( wxString::Format( _T("%5.1f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::AIR_TEMPERATURE), temp ) );
        } else
            m_tcAirTemperature->SetValue( _("N/A") );
    }

    //    Update the Sea Surface Temperature
    if( RecordArray[Idx_SEA_TEMP] ) {
        double temp = RecordArray[Idx_SEA_TEMP]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( temp != GRIB_NOTDEF ) {
            temp = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::SEA_TEMPERATURE, temp);
            m_tcSeaTemperature->SetValue( wxString::Format( _T("%5.1f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::SEA_TEMPERATURE), temp ) );
        } else
            m_tcSeaTemperature->SetValue( _("N/A") );
    }

    //    Update the Convective Available Potential Energy (CAPE)
    if( RecordArray[Idx_CAPE] ) {
        double cape = RecordArray[Idx_CAPE]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( cape != GRIB_NOTDEF ) {
            cape = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::CAPE, cape);
            m_tcCAPE->SetValue( wxString::Format( _T("%5.0f ") + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CAPE), cape ) );
        } else
            m_tcCAPE->SetValue( _("N/A") );
    }
    // Update extra data for altitude
    // geopotential altitude
    if( RecordArray[Idx_GEOP_HGT + m_Altitude] ) {
        double geop = RecordArray[Idx_GEOP_HGT + m_Altitude]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( geop != GRIB_NOTDEF ) {
            geop = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::GEO_ALTITUDE, geop);
            m_tcAltitude->SetValue( wxString::Format( _T("%5.0f "), geop ) + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::GEO_ALTITUDE) );
        } else
            m_tcAltitude->SetValue( _("N/A") );
    }

    // temperature
    if( RecordArray[Idx_AIR_TEMP + m_Altitude] ) {
        double temp = RecordArray[Idx_AIR_TEMP + m_Altitude]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( temp != GRIB_NOTDEF ) {
            temp = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::AIR_TEMPERATURE, temp);
            m_tcTemp->SetValue( wxString::Format( _T("%5.1f "), temp ) + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::AIR_TEMPERATURE) );
        } else
            m_tcTemp->SetValue(  _("N/A") );
    }
    // relative humidity
    if( RecordArray[Idx_HUMID_RE + m_Altitude] ) {
        double humi = RecordArray[Idx_HUMID_RE + m_Altitude]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( humi != GRIB_NOTDEF ) {
            humi = m_gparent.m_OverlaySettings.CalibrateValue(GribOverlaySettings::REL_HUMIDITY, humi);
            m_tcRelHumid->SetValue( wxString::Format( _T("%5.0f "), humi ) + m_gparent.m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::REL_HUMIDITY) );
        } else
            m_tcRelHumid->SetValue(  _("N/A") );
    }
}

void CursorData::OnMenuCallBack( wxMouseEvent& event )
{
    //populate menu
    wxMenu* menu = new wxMenu();
    int id = event.GetId();
    switch( id ) {
        case GribOverlaySettings::WIND:
            MenuAppend( menu, B_ARROWS, _("Barbed Arrows"), id );
            MenuAppend( menu, ISO_LINE, _("Display Isotachs"), id );
            MenuAppend( menu, OVERLAY, _("OverlayMap"), id );
            MenuAppend( menu, NUMBERS, _("Numbers"), id );
            MenuAppend( menu, PARTICLES, _("Particle Map"), id );
            break;
        case GribOverlaySettings::WIND_GUST:
            MenuAppend( menu, ISO_LINE, _("Display Isotachs"), id );
            MenuAppend( menu, OVERLAY, _("OverlayMap"), id );
            MenuAppend( menu, NUMBERS, _("Numbers"), id );
            break;
        case GribOverlaySettings::PRESSURE:
            MenuAppend( menu, ISO_LINE, _("Display Isobars"), id );
            MenuAppend( menu, NUMBERS, _("Numbers"), id );
            break;
        case GribOverlaySettings::AIR_TEMPERATURE:
        case GribOverlaySettings::SEA_TEMPERATURE:
            MenuAppend( menu, ISO_LINE, _("Display Isotherms"), id );
        case GribOverlaySettings::CLOUD:
        case GribOverlaySettings::PRECIPITATION:
            MenuAppend( menu, OVERLAY, _("OverlayMap"), id );
            MenuAppend( menu, NUMBERS, _("Numbers"), id );
            break;
        case GribOverlaySettings::CAPE:
            MenuAppend( menu, ISO_LINE, _("Display Iso CAPE"), id );
            MenuAppend( menu, OVERLAY, _("OverlayMap"), id );
            MenuAppend( menu, NUMBERS, _("Numbers"), id );
            break;
        case GribOverlaySettings::WAVE:
            MenuAppend( menu, D_ARROWS, _("Direction Arrows"), id );
            MenuAppend( menu, OVERLAY, _("OverlayMap"), id );
            MenuAppend( menu, NUMBERS, _("Numbers"), id );
            break;
        case GribOverlaySettings::CURRENT:
            MenuAppend( menu, D_ARROWS, _("Direction Arrows"), id );
            MenuAppend( menu, OVERLAY, _("OverlayMap"), id );
            MenuAppend( menu, NUMBERS, _("Numbers"), id );
            MenuAppend( menu, PARTICLES, _("Particle Map"), id );
    }

    PopupMenu( menu );

    //apply new config
    wxwxMenuItemListNode *node = menu->GetMenuItems().GetFirst();
    while( node ) {
        wxMenuItem *it = node->GetData();
        switch( it->GetId() ) {
            case B_ARROWS:
                m_gparent.m_OverlaySettings.Settings[id].m_bBarbedArrows = it->IsChecked();
                break;
            case ISO_LINE:
                m_gparent.m_OverlaySettings.Settings[id].m_bIsoBars = it->IsChecked();
                break;
            case D_ARROWS:
                m_gparent.m_OverlaySettings.Settings[id].m_bDirectionArrows = it->IsChecked();
                break;
            case OVERLAY:
                m_gparent.m_OverlaySettings.Settings[id].m_bOverlayMap = it->IsChecked();
                break;
            case NUMBERS:
                m_gparent.m_OverlaySettings.Settings[id].m_bNumbers = it->IsChecked();
                break;
            case PARTICLES:
                m_gparent.m_OverlaySettings.Settings[id].m_bParticles = it->IsChecked();
        }
        node = node->GetNext();
    }

    //if the current parameter type is selected then resolve display conflicts
	if( m_gparent.m_bDataPlot[id] )
		ResolveDisplayConflicts( id );

    //save new config
    m_gparent.m_OverlaySettings.Write();

    delete menu;
}

void CursorData::MenuAppend( wxMenu *menu, int id, wxString label, int setting)
{
    wxMenuItem *item = new wxMenuItem(menu, id, label, _T(""), wxITEM_CHECK);

#ifdef __WXMSW__
    wxFont *qFont = OCPNGetFont( _("Menu"), 10 );
    item->SetFont(*qFont);
#endif

    menu->Append(item);

    bool check;
    if( id == B_ARROWS )
        check = m_gparent.m_OverlaySettings.Settings[setting].m_bBarbedArrows;
    else if( id == ISO_LINE )
        check = m_gparent.m_OverlaySettings.Settings[setting].m_bIsoBars;
    else if( id == D_ARROWS )
        check = m_gparent.m_OverlaySettings.Settings[setting].m_bDirectionArrows;
    else if( id == OVERLAY )
        check = m_gparent.m_OverlaySettings.Settings[setting].m_bOverlayMap;
    else if( id == NUMBERS )
        check = m_gparent.m_OverlaySettings.Settings[setting].m_bNumbers;
    else if( id == PARTICLES )
        check = m_gparent.m_OverlaySettings.Settings[setting].m_bParticles;
    else
        check = false;
    item->Check( check );
}

void CursorData::OnMouseEvent( wxMouseEvent &event )
{
    if( event.RightDown() ) {
        if( m_DialogStyle >> 1 == ATTACHED ) {
            wxMouseEvent evt(event);
            m_gparent.OnMouseEvent( evt );
        }
        return;
    }

    static wxPoint s_gspt;
    int x, y;

    event.GetPosition( &x, &y );
    wxPoint spt = wxPoint( x, y );
    if( event.GetId() != 1000 )
        spt = ClientToScreen( spt );
    else
        spt = GetParent()->ClientToScreen( spt );

#ifdef __WXOSX__
    if (!m_bLeftDown && event.LeftIsDown())
    {
        m_bLeftDown = true;
        s_gspt = spt;
        if (!HasCapture()) CaptureMouse();
    }
    else if (m_bLeftDown && !event.LeftIsDown())
    {
       // GetParent()->Move( GetParent()->GetPosition() );
        m_bLeftDown = false;
        if (HasCapture()) ReleaseMouse();
    }
#else

    if( event.LeftDown() ) {
        s_gspt = spt;
        CaptureMouse();
    }

    if( event.LeftUp() ) {
        //GetParent()->Move( GetParent()->GetPosition() );
        if( HasCapture() ) ReleaseMouse();
    }
#endif

    if( event.Dragging() ) {

        wxPoint par_pos_old = GetParent()->GetPosition();

        wxPoint par_pos = par_pos_old;
        par_pos.x += spt.x - s_gspt.x;
        par_pos.y += spt.y - s_gspt.y;

        wxPoint pos_in_parent = GetOCPNCanvasWindow()->ScreenToClient( par_pos );
        wxPoint pos_in_parent_old = GetOCPNCanvasWindow()->ScreenToClient( par_pos_old );

		// X
		if( pos_in_parent.x < pos_in_parent_old.x ) {           // moving left
			if( pos_in_parent.x < 10 ) {
				pos_in_parent.x = 0;
			}
		} else
        if( pos_in_parent.x > pos_in_parent_old.x ) {           // moving right
            int max_right = GetOCPNCanvasWindow()->GetClientSize().x - GetParent()->GetSize().x;
            if( pos_in_parent.x > ( max_right - 10 ) ) {
                pos_in_parent.x = max_right;
            }
        }

		// Y
		if( pos_in_parent.y < pos_in_parent_old.y ) {            // moving up
			if( pos_in_parent.y < 10 ) {
				pos_in_parent.y = 0;
			}
		} else
        if( pos_in_parent.y > pos_in_parent_old.y ) {            // moving dow
            int max_down = GetOCPNCanvasWindow()->GetClientSize().y - GetParent()->GetSize().y;
            if( pos_in_parent.y > ( max_down - 10 ) ) {
                pos_in_parent.y = max_down;
            }
        }

		wxPoint final_pos = GetOCPNCanvasWindow()->ClientToScreen( pos_in_parent );

		GetParent()->Move( final_pos );

        s_gspt = spt;

    }
}
