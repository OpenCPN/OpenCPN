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

#include "wx/wx.h"
#include "wx/tokenzr.h"
#include "wx/datetime.h"
#include "wx/sound.h"
#include <wx/wfstream.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/debug.h>
#include <wx/graphics.h>

#include <wx/stdpaths.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "grib_pi.h"
#include "email.h"
#include "folder.xpm"

#include <wx/arrimpl.cpp>

#if defined (_WIN32)
int round (double x) {
	int i = (int) x;
	if (x >= 0.0) {
		return ((x-i) >= 0.5) ? (i + 1) : (i);
	} else {
		return (-x+i >= 0.5) ? (i - 1) : (i);
	}
}
#endif

WX_DEFINE_OBJARRAY( ArrayOfGribRecordSets );

enum { SAILDOCS,ZYGRIB };
enum { GFS,COAMPS,RTOFS };

//    Sort compare function for File Modification Time
static int CompareFileStringTime( const wxString& first, const wxString& second )
{
    wxFileName f( first );
    wxFileName s( second );
    wxTimeSpan sp = s.GetModificationTime() - f.GetModificationTime();
    return sp.GetMinutes();

//      return ::wxFileModificationTime(first) - ::wxFileModificationTime(second);
}

//date/time in the desired time zone format
static wxString TToString( const wxDateTime date_time, const int time_zone )
{
    wxDateTime t( date_time );
    t.MakeFromTimezone( wxDateTime::UTC );
    if( t.IsDST() ) t.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );
    switch( time_zone ) {
        case 0: return t.Format( _T(" %a %d-%b-%Y  %H:%M "), wxDateTime::Local ) + _T("LOC");//:%S
        case 1:
        default: return t.Format( _T(" %a %d-%b-%Y %H:%M  "), wxDateTime::UTC ) + _T("UTC");
    }
}

wxString toMailFormat ( int NEflag, int a )
{
    char c;
    switch(NEflag) {
        case 1:
            c = 'N';
            if ( a < 0.0 ) c = 'S';
            break;
        case 2:
            c = 'E';
            if ( a < 0.0 ) c = 'W';
    }
    wxString s;
    s.Printf ( _T ( "%01d%c" ), abs(a), c );
    return s;
}

#if !wxCHECK_VERSION(2,9,4) /* to work with wx 2.8 */
#define SetBitmap SetBitmapLabel
#endif

//---------------------------------------------------------------------------------------
//          GRIB Selector/Control Dialog Implementation
//---------------------------------------------------------------------------------------

/* interpolating constructor
   as a possible optimization, write this function to also
   take latitude longitude boundaries so the resulting record can be
   a subset of the input, but also would need to be recomputed when panning the screen */
GribTimelineRecordSet::GribTimelineRecordSet(GribRecordSet &GRS1, GribRecordSet &GRS2, double interp_const)
{
    for(int i=0; i<Idx_COUNT; i++) {
        GribRecord *GR1 = GRS1.m_GribRecordPtrArray[i];
        GribRecord *GR2 = GRS2.m_GribRecordPtrArray[i];

        if(GR1 && GR2 && GR1->isOk() && GR2->isOk() &&
           GR1->getDi() == GR2->getDi() && GR1->getDj() == GR2->getDj() &&
           GR1->getLatMin() == GR2->getLatMin() && GR1->getLonMin() == GR2->getLonMin() &&
           GR1->getLatMax() == GR2->getLatMax() && GR1->getLonMax() == GR2->getLonMax())
            m_GribRecordPtrArray[i] = new GribRecord(*GR1, *GR2, interp_const);
        else
            m_GribRecordPtrArray[i] = NULL;
        m_IsobarArray[i] = NULL;
    }

    m_Reference_Time = (1-interp_const)*GRS1.m_Reference_Time
        + interp_const*GRS2.m_Reference_Time;
}

GribTimelineRecordSet::~GribTimelineRecordSet()
{
    for(int i=0; i<Idx_COUNT; i++)
        delete m_GribRecordPtrArray[i]; /* delete these for timeline */
    ClearCachedData();
}

void GRIBUIDialog::OpenFile(bool newestFile)
{
    m_bpPlay->SetBitmap(*m_bPlay);
    m_bpPlay->SetToolTip(_("Play"));
    m_tPlayStop.Stop();

    m_cRecordForecast->Clear();
    /* this should be un-commented to avoid a memory leak,
       but for some reason it crbashes windows */
    delete m_bGRIBActiveFile;
    m_pTimelineSet = NULL;

    //get more recent file in default directory if necessary
    wxFileName f( m_file_name );
    if( newestFile || f.GetFullName().IsEmpty() ) m_file_name = GetNewestFileInDirectory();

    m_bGRIBActiveFile = new GRIBFile( m_file_name,
                                      pPlugIn->GetCopyFirstCumRec(),
                                      pPlugIn->GetCopyMissWaveRec() );

    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
    if(rsa->GetCount() < 2)
        m_TimeLineHours = 0;
    else {
        GribRecordSet &first=rsa->Item(0), &last = rsa->Item(rsa->GetCount()-1);

        wxTimeSpan span = wxDateTime(last.m_Reference_Time) - wxDateTime(first.m_Reference_Time);
        m_TimeLineHours = span.GetHours();
        m_sTimeline->Enable();
    }
    m_sTimeline->SetValue(0);

    wxFileName fn( m_file_name );
    wxString title( fn.GetFullName() );

    if( m_bGRIBActiveFile ) {
        if( m_bGRIBActiveFile->IsOK() ) {
            //there could be valid but empty file
            if( rsa->GetCount() == 0 ) {
                m_bGRIBActiveFile = NULL;
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( _("Error:  No valid data in this file!") );
            } else {
                PopulateComboDataList();
                title.append( _T("(") + TToString( m_bGRIBActiveFile->GetRefDateTime(), pPlugIn->GetTimeZone()) + _T(")"));
            }
        } else {
            if( fn.IsDir() ) {
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( _("Warning:  Empty directory!") );
            }
            else
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( m_bGRIBActiveFile->GetLastMessage() );
        }
        this->SetTitle(title);
        SetFactoryOptions();
        TimelineChanged();
        PopulateTrackingControls();
    }
}

wxString GRIBUIDialog::GetNewestFileInDirectory()
{
    if( !wxDir::Exists( m_grib_dir ) ) {
         wxStandardPathsBase& path = wxStandardPaths::Get();
         m_grib_dir = path.GetDocumentsDir();
    }
    //    Get an array of GRIB file names in the target directory, not descending into subdirs
    wxArrayString file_array;
    int m_n_files = 0;
    m_n_files = wxDir::GetAllFiles( m_grib_dir, &file_array, _T ( "*.grb" ), wxDIR_FILES );
    m_n_files += wxDir::GetAllFiles( m_grib_dir, &file_array, _T ( "*.bz2" ),
        wxDIR_FILES );
    if( m_n_files ) {
        file_array.Sort( CompareFileStringTime );              //sort the files by File Modification Date

        return file_array[0];                                  //return the first file (the more recent one)
    } else {
        wxFileName d(m_grib_dir);
        return wxString( m_grib_dir.Append(d.GetPathSeparator()) );                                      //this directory is empty
    }
}

void GribTimelineRecordSet::ClearCachedData()
{
    for(int i=0; i<Idx_COUNT; i++) {
        if(m_IsobarArray[i]) {
            //    Clear out the cached isobars
            for( unsigned int j = 0; j < m_IsobarArray[i]->GetCount(); j++ ) {
                IsoLine *piso = (IsoLine *) m_IsobarArray[i]->Item( j );
                delete piso;
            }
            delete m_IsobarArray[i];
            m_IsobarArray[i] = NULL;
        }
    }
}

GRIBUIDialog::GRIBUIDialog(wxWindow *parent, grib_pi *ppi)
: GRIBUIDialogBase(parent)
{
    pParent = parent;
    pPlugIn = ppi;

    m_bGRIBActiveFile = NULL;
    m_pTimelineSet = NULL;
    m_InterpolateMode = false;

    wxFileConfig *pConf = GetOCPNConfigObject();

    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/GRIB" ) );

        bool value;
        pConf->Read( _T ( "WindPlot" ), &value, true );
        m_cbWind->SetValue(value);
        pConf->Read( _T ( "WindGustPlot" ), &value, true );
        m_cbWindGust->SetValue(value);
        pConf->Read( _T ( "PressurePlot" ), &value, true );
        m_cbPressure->SetValue(value);
        pConf->Read( _T ( "WavePlot" ), &value, false );
        m_cbWave->SetValue(value);
        pConf->Read( _T ( "CurrentPlot" ), &value, false );
        m_cbCurrent->SetValue(value);
        pConf->Read( _T ( "PrecipitationPlot" ), &value, false );
        m_cbPrecipitation->SetValue(value);
        pConf->Read( _T ( "CloudPlot" ), &value, false );
        m_cbCloud->SetValue(value);
        pConf->Read( _T ( "AirTemperaturePlot" ), &value, false );
        m_cbAirTemperature->SetValue(value);
        pConf->Read( _T ( "SeaTemperaturePlot" ), &value, false );
        m_cbSeaTemperature->SetValue(value);
        pConf->Read( _T ( "CAPEPlot" ), &value, false );
        m_cbCAPE->SetValue(value);

        pConf->Read ( _T ( "lastdatatype" ), &m_lastdatatype, 0);

        pConf->Read ( _T ( "Filename" ), &m_file_name );

        wxStandardPathsBase& spath = wxStandardPaths::Get();

        pConf->SetPath ( _T ( "/Directories" ) );
        pConf->Read ( _T ( "GRIBDirectory" ), &m_grib_dir, spath.GetDocumentsDir()  );
    }

    m_bpPrev->SetBitmap(wxBitmap( prev ));
    m_bpNext->SetBitmap(wxBitmap( next ));
    m_bpNow->SetBitmap(wxBitmap( now ));
    m_bPlay = new wxBitmap( play );
    m_bpPlay->SetBitmap(*m_bPlay );
    m_bpOpenFile->SetBitmap(wxBitmap( openfile ));
    m_bpSettings->SetBitmap(wxBitmap( setting ));
    m_bpRequest->SetBitmap(wxBitmap( request ));

    //connect events have not been done in dialog base
    this->Connect( wxEVT_MOVE, wxMoveEventHandler( GRIBUIDialog::OnMove ) );
    m_tPlayStop.Connect(wxEVT_TIMER, wxTimerEventHandler( GRIBUIDialog::OnPlayStopTimer ), NULL, this);

    m_OverlaySettings.Read();

    DimeWindow( this );

    m_pTimelineSet = NULL;
    m_fgTrackingControls->Clear();

    Fit();
    SetMinSize( GetBestSize() );
}

GRIBUIDialog::~GRIBUIDialog()
{
    wxFileConfig *pConf = GetOCPNConfigObject();;

    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/GRIB" ) );

        pConf->Write( _T ( "WindPlot" ), m_cbWind->GetValue());
        pConf->Write( _T ( "WindGustPlot" ), m_cbWindGust->GetValue());
        pConf->Write( _T ( "PressurePlot" ), m_cbPressure->GetValue());
        pConf->Write( _T ( "WavePlot" ), m_cbWave->GetValue());
        pConf->Write( _T ( "CurrentPlot" ), m_cbCurrent->GetValue());
        pConf->Write( _T ( "PrecipitationPlot" ), m_cbPrecipitation->GetValue());
        pConf->Write( _T ( "CloudPlot" ), m_cbCloud->GetValue());
        pConf->Write( _T ( "AirTemperaturePlot" ), m_cbAirTemperature->GetValue());
        pConf->Write( _T ( "SeaTemperaturePlot" ), m_cbSeaTemperature->GetValue());
        pConf->Write( _T ( "CAPEPlot" ), m_cbCAPE->GetValue());
        pConf->Write( _T ( "lastdatatype" ), m_lastdatatype);

        pConf->Write ( _T ( "Filename" ), m_file_name );

        pConf->SetPath ( _T ( "/Directories" ) );
        pConf->Write ( _T ( "GRIBDirectory" ), m_grib_dir );
    }

    delete m_pTimelineSet;
}

void GRIBUIDialog::SetCursorLatLon( double lat, double lon )
{
    m_cursor_lon = lon;
    m_cursor_lat = lat;

    UpdateTrackingControls();
}

void GRIBUIDialog::AddTrackingControl( wxControl *ctrl1,  wxControl *ctrl2,  wxControl *ctrl3, bool show )
{
    if(show) {
        m_fgTrackingControls->Add(ctrl1, 0, wxALL, 1);
        ctrl1->Show();
        if(ctrl2) {
            m_fgTrackingControls->Add(ctrl2, 0, wxALL, 1);
            ctrl2->Show();
        } else
              m_fgTrackingControls->Add(0, 0, 1, wxEXPAND, 1); /* spacer */

        if(ctrl3) {
            m_fgTrackingControls->Add(ctrl3, 0, wxALL, 1);
            ctrl3->Show();
        } else
            m_fgTrackingControls->Add(0, 0, 1, wxEXPAND, 1); /* spacer */
    } else {
        ctrl1->Hide();
        if(ctrl2)
            ctrl2->Hide();
        if(ctrl3)
            ctrl3->Hide();
    }
}

void GRIBUIDialog::PopulateTrackingControls( void )
{
    //fix crash with curious files with no record
    if(m_pTimelineSet)
        m_bpSettings->Enable();
    else
        m_bpSettings->Disable();

    if(m_pTimelineSet && m_TimeLineHours) {
        m_sTimeline->Show();
        m_bpPlay->Show();

        m_bpPrev->Enable();
        m_bpNext->Enable();
        m_bpNow->Enable();
    } else {
        m_sTimeline->Hide();
        m_bpPlay->Hide();

        m_bpPrev->Disable();
        m_bpNext->Disable();
        m_bpNow->Disable();
    }

    m_fgTrackingControls->Clear();
    m_fgTrackingControls->SetCols(9);
    this->Fit();

    AddTrackingControl(m_cbWind, m_tcWindSpeed, m_tcWindDirection,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VX) != wxNOT_FOUND
        && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VY) != wxNOT_FOUND);
    AddTrackingControl(m_cbWindGust, m_tcWindGust, 0, m_pTimelineSet
        && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_GUST) != wxNOT_FOUND);
    AddTrackingControl(m_cbPressure, m_tcPressure, 0, m_pTimelineSet
        && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRESSURE) != wxNOT_FOUND);

    /* tracking for wave is funky */
    if(m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_HTSIGW) != wxNOT_FOUND) {
        if(m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVDIR) != wxNOT_FOUND)
            AddTrackingControl(m_cbWave, m_tcWaveHeight, m_tcWaveDirection, true);
        else {
            AddTrackingControl(m_cbWave, m_tcWaveHeight, 0, true);
            m_tcWaveDirection->Hide();
        }
    } else
        if(m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVDIR) != wxNOT_FOUND) {
            AddTrackingControl(m_cbWave, 0, m_tcWaveDirection, true);
            m_tcWaveHeight->Hide();
        } else
            AddTrackingControl(m_cbWave, m_tcWaveHeight, m_tcWaveDirection, false);

    AddTrackingControl(m_cbCurrent, m_tcCurrentVelocity, m_tcCurrentDirection,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEACURRENT_VX) != wxNOT_FOUND
        && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEACURRENT_VY) != wxNOT_FOUND);
    AddTrackingControl(m_cbPrecipitation, m_tcPrecipitation, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRECIP_TOT) != wxNOT_FOUND);
    AddTrackingControl(m_cbCloud, m_tcCloud, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CLOUD_TOT) != wxNOT_FOUND);
    AddTrackingControl(m_cbAirTemperature, m_tcAirTemperature, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_AIR_TEMP_2M) != wxNOT_FOUND);
    AddTrackingControl(m_cbSeaTemperature, m_tcSeaTemperature, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEA_TEMP) != wxNOT_FOUND);
    AddTrackingControl(m_cbCAPE, m_tcCAPE, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CAPE) != wxNOT_FOUND);

    Fit();
    Refresh();
}

void GRIBUIDialog::UpdateTrackingControls( void )
{
    if( !m_pTimelineSet )
        return;

    GribRecord **RecordArray = m_pTimelineSet->m_GribRecordPtrArray;
    //    Update the wind control
    if( RecordArray[Idx_WIND_VX] && RecordArray[Idx_WIND_VY] ) {
        double vx = RecordArray[Idx_WIND_VX]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
        double vy = RecordArray[Idx_WIND_VY]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
            vx = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND, vx);
            vy = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND, vy);

            double vkn = sqrt( vx * vx + vy * vy );
            m_tcWindSpeed->SetValue( wxString::Format( _T("%2d ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WIND) , (int) vkn ) );

            double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
            if( ang > 360. ) ang -= 360.;
            if( ang < 0. ) ang += 360.;
            m_tcWindDirection->SetValue( wxString::Format( _T("%03d\u00B0"), (int) ( ang ) ));
        } else {
            m_tcWindSpeed->SetValue( _("N/A") );
            m_tcWindDirection->SetValue(  _("N/A") );
        }
    }

    //    Update the Wind gusts control
    if( RecordArray[Idx_WIND_GUST] ) {
        double vkn = RecordArray[Idx_WIND_GUST]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( vkn != GRIB_NOTDEF ) {
            vkn = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND_GUST, vkn);
            m_tcWindGust->SetValue( wxString::Format(_T("%2d ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WIND_GUST), (int) ( vkn )) );
        } else
            m_tcWindGust->SetValue( _("N/A") );
    }

    //    Update the Pressure control
    if( RecordArray[Idx_PRESSURE] ) {
        double press = RecordArray[Idx_PRESSURE]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( press != GRIB_NOTDEF ) {
            press = m_OverlaySettings.CalibrateValue(GribOverlaySettings::PRESSURE, press);
            m_tcPressure->SetValue( wxString::Format(_T("%2d ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRESSURE), (int) ( press )) );
        } else
            m_tcPressure->SetValue( _("N/A") );
    }

    //    Update the Sig Wave Height
    if( RecordArray[Idx_HTSIGW] ) {
        double height = RecordArray[Idx_HTSIGW]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( height != GRIB_NOTDEF ) {
            height = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WAVE, height);
            m_tcWaveHeight->SetValue( wxString::Format( _T("%4.1f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WAVE), height ));
        } else
            m_tcWaveHeight->SetValue( _("N/A") );
    }

    // Update the Wave direction
    if( RecordArray[Idx_WVDIR] ) {
        double direction = RecordArray[Idx_WVDIR]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
        if( direction != GRIB_NOTDEF )
            m_tcWaveDirection->SetValue( wxString::Format( _T("%03d\u00B0"), (int)direction ));
        else
            m_tcWaveDirection->SetValue( _("N/A") );
    }


    //    Update the Current control
    if( RecordArray[Idx_SEACURRENT_VX] && RecordArray[Idx_SEACURRENT_VY] ) {
        double vx = RecordArray[Idx_SEACURRENT_VX]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
        double vy = RecordArray[Idx_SEACURRENT_VY]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
            vx = m_OverlaySettings.CalibrateValue(GribOverlaySettings::CURRENT, vx);
            vy = m_OverlaySettings.CalibrateValue(GribOverlaySettings::CURRENT, vy);

            double vkn = sqrt( vx * vx + vy * vy );
            m_tcCurrentVelocity->SetValue( wxString::Format( _T("%4.1f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CURRENT), vkn ) );

            double ang = 90. + ( atan2( -vy, vx ) * 180. / PI );
            if( ang > 360. ) ang -= 360.;
            if( ang < 0. ) ang += 360.;
            m_tcCurrentDirection->SetValue( wxString::Format( _T("%03d\u00B0"), (int) ( ang ) ) );
        } else {
            m_tcCurrentVelocity->SetValue( _("N/A") );
            m_tcCurrentDirection->SetValue( _("N/A") );
        }
    }

    //    Update total rainfall control
    if( RecordArray[Idx_PRECIP_TOT] ) {
        double precip = RecordArray[Idx_PRECIP_TOT]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( precip != GRIB_NOTDEF ) {
            precip = m_OverlaySettings.CalibrateValue(GribOverlaySettings::PRECIPITATION, precip);
            m_tcPrecipitation->SetValue( wxString::Format( _T("%6.2f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRECIPITATION), precip ) );
        } else
            m_tcPrecipitation->SetValue( _("N/A") );
    }

    //    Update total cloud control
    if( RecordArray[Idx_CLOUD_TOT] ) {
        double cloud = RecordArray[Idx_CLOUD_TOT]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( cloud != GRIB_NOTDEF ) {
            cloud = m_OverlaySettings.CalibrateValue(GribOverlaySettings::CLOUD, cloud);
            wxString val( wxString::Format( _T("%5.1f "), cloud ) );
            m_tcCloud->SetValue( val + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CLOUD) );
        } else
            m_tcCloud->SetValue( _("N/A") );
    }

    //    Update the Air Temperature
    if( RecordArray[Idx_AIR_TEMP_2M] ) {
        double temp = RecordArray[Idx_AIR_TEMP_2M]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( temp != GRIB_NOTDEF ) {
            temp = m_OverlaySettings.CalibrateValue(GribOverlaySettings::AIR_TEMPERATURE, temp);
            m_tcAirTemperature->SetValue( wxString::Format( _T("%5.1f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::AIR_TEMPERATURE), temp ) );
        } else
            m_tcAirTemperature->SetValue( _("N/A") );
    }

    //    Update the Sea Surface Temperature
    if( RecordArray[Idx_SEA_TEMP] ) {
        double temp = RecordArray[Idx_SEA_TEMP]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( temp != GRIB_NOTDEF ) {
            temp = m_OverlaySettings.CalibrateValue(GribOverlaySettings::SEA_TEMPERATURE, temp);
            m_tcSeaTemperature->SetValue( wxString::Format( _T("%5.1f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::SEA_TEMPERATURE), temp ) );
        } else
            m_tcSeaTemperature->SetValue( _("N/A") );
    }

    //    Update the Convective Available Potential Energy (CAPE)
    if( RecordArray[Idx_CAPE] ) {
        double cape = RecordArray[Idx_CAPE]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( cape != GRIB_NOTDEF ) {
            cape = m_OverlaySettings.CalibrateValue(GribOverlaySettings::CAPE, cape);
            m_tcCAPE->SetValue( wxString::Format( _T("%5.0f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CAPE), cape ) );
        } else
            m_tcCAPE->SetValue( _("N/A") );
    }

    Fit();
}

void GRIBUIDialog::OnClose( wxCloseEvent& event )
{
    pPlugIn->OnGribDialogClose();
}

void GRIBUIDialog::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = GetPosition();
    pPlugIn->SetGribDialogX( p.x );
    pPlugIn->SetGribDialogY( p.y );

    event.Skip();
}

void GRIBUIDialog::OnSize( wxSizeEvent& event )
{
    //    Record the dialog size
    wxSize p = event.GetSize();
    pPlugIn->SetGribDialogSizeX( p.x );
    pPlugIn->SetGribDialogSizeY( p.y );

    event.Skip();
}


void GRIBUIDialog::OnRequest(  wxCommandEvent& event )
{
    double lonmax=m_vp->lon_max;
    double lonmin=m_vp->lon_min;
    if( ( fabs( m_vp->lat_max ) < 90. ) && ( fabs( lonmax ) < 360. ) ) {
           if( lonmax < -180. ) lonmax += 360.;
           if( lonmax > 180. ) lonmax -= 360.;
    }
    if( ( fabs( m_vp->lat_min ) < 90. ) && ( fabs( lonmin ) < 360. ) ) {
            if( lonmin < -180. ) lonmin += 360.;
            if( lonmin > 180. ) lonmin -= 360.;
    }
     int latmaxi =  (int) ceil(m_vp->lat_max);
     int latmini =  (int) floor(m_vp->lat_min);
     int lonmini =  (int) floor(lonmin);
     int lonmaxi =  (int) ceil(lonmax);

    GribRequestSetting *req_Dialog = new GribRequestSetting( this, pPlugIn->GetRequestConfig(), latmaxi, latmini, lonmini, lonmaxi,
        pPlugIn->GetMailFromAddress(), pPlugIn->GetMailToAddresses(), pPlugIn->GetZyGribLogin(), pPlugIn->GetZyGribCode() );
    wxString s1[] = {_T("GFS"),_T("COAMPS"),_T("RTOFS")};
    for( unsigned int i= 0;  i<(sizeof(s1) / sizeof(wxString));i++)
        req_Dialog->m_pModel->Append( s1[i] );
    wxString s2[] = {_T("Saildocs"),_T("zyGrib")};
    for( unsigned int i= 0;  i<(sizeof(s2) / sizeof(wxString));i++)
        req_Dialog->m_pMailTo->Append( s2[i] );
    for( double i=0.5; i<3; i*=2)
        req_Dialog->m_pResolution->Append( wxString::Format(_T("%0.1f"), i));
    for( int i=3; i<25; i*=2)
        req_Dialog->m_pInterval->Append( wxString::Format(_T("%d"), i));
    for( int i=2; i<9; i++)
        req_Dialog->m_pTimeRange->Append( wxString::Format(_T("%d"), i));
    wxString s3[] = {_T("WW3-GLOBAL"),_T("WW3-MEDIT")};
    for( unsigned int i= 0;  i<(sizeof(s3) / sizeof(wxString));i++)
        req_Dialog->m_pWModel->Append( s3[i] );
    req_Dialog->m_rButtonYes->SetLabel(_("Send"));
    req_Dialog->m_rButtonApply->SetLabel(_("Save"));
    req_Dialog->m_tResUnit->SetLabel(wxString::Format( _T("\u00B0")));
    req_Dialog->m_pSenderAddress->SetToolTip(_("Address used to send request eMail. (Mandatory for LINUX)"));
    req_Dialog->m_pLogin->SetToolTip(_("This is your zyGrib's forum access Login"));
    req_Dialog->m_pCode->SetToolTip(_("Get this Code in zyGrib's forum ( This is not your password! )"));
    req_Dialog->InitRequestConfig();
    req_Dialog->Fit();
    if( req_Dialog->ShowModal() == wxID_APPLY ) {
        //save all enabled parameters
        req_Dialog->m_RequestConfigBase.SetChar( 0, (char) ( req_Dialog->m_pMailTo->GetCurrentSelection() + '0' ) );
        if(req_Dialog->m_pMailTo->GetCurrentSelection() == SAILDOCS)
            req_Dialog->m_RequestConfigBase.SetChar( 1, (char) ( req_Dialog->m_pModel->GetCurrentSelection() + '0' ) );
        if(req_Dialog->m_pModel->GetCurrentSelection() != RTOFS)
            req_Dialog->m_RequestConfigBase.SetChar( 2, (char) ( req_Dialog->m_pResolution->GetCurrentSelection() + '0' ) );
        req_Dialog->m_RequestConfigBase.SetChar( 3, (char) ( req_Dialog->m_pInterval->GetCurrentSelection() + '0' ) );
        req_Dialog->m_RequestConfigBase.SetChar( 4, (char) ( req_Dialog->m_pTimeRange->GetCurrentSelection() + '0' ) );
        if(req_Dialog->m_pMailTo->GetCurrentSelection() == ZYGRIB && req_Dialog->m_pWModel->IsShown())
            req_Dialog->m_RequestConfigBase.SetChar( 5, (char) ( req_Dialog->m_pWModel->GetCurrentSelection() + '0' ) );
        req_Dialog->m_RequestConfigBase.SetChar( 6, 'X' );              //must be always selected as a default
        req_Dialog->m_RequestConfigBase.SetChar( 7, 'X' );
        if(req_Dialog->m_pModel->GetCurrentSelection() != COAMPS) {
            req_Dialog->m_pWaves->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 8, 'X' )
                : req_Dialog->m_RequestConfigBase.SetChar( 8, '.' );
            req_Dialog->m_pRainfall->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 9, 'X' )
                : req_Dialog->m_RequestConfigBase.SetChar( 9, '.' );
            req_Dialog->m_pCloudCover->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 10, 'X' )
                : req_Dialog->m_RequestConfigBase.SetChar( 10, '.' );
            req_Dialog->m_pAirTemp->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 11, 'X' )
                : req_Dialog->m_RequestConfigBase.SetChar( 11, '.' );
            req_Dialog->m_pSeaTemp->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 12, 'X' )
                : req_Dialog->m_RequestConfigBase.SetChar( 12, '.' );
            req_Dialog->m_pCAPE->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 15, 'X' )
                : req_Dialog->m_RequestConfigBase.SetChar( 15, '.' );
        }
        if(req_Dialog->m_pModel->GetCurrentSelection() != ZYGRIB && req_Dialog->m_pModel->GetCurrentSelection() != COAMPS)
            req_Dialog->m_pCurrent->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 13, 'X' )
                : req_Dialog->m_RequestConfigBase.SetChar( 13, '.' );
        if(req_Dialog->m_pMailTo->GetCurrentSelection() == ZYGRIB)
            req_Dialog->m_pWindGust->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 14, 'X' )
                : req_Dialog->m_RequestConfigBase.SetChar( 14, '.' );

        pPlugIn->SetMailFromAddress(req_Dialog->m_pSenderAddress->GetValue());
        pPlugIn->SetZyGribLogin(req_Dialog->m_pLogin->GetValue());
        pPlugIn->SetZyGribCode(req_Dialog->m_pCode->GetValue());

        pPlugIn->SetRequestConfig(req_Dialog->m_RequestConfigBase);
    }

    req_Dialog->Destroy();
}

void GRIBUIDialog::OnSettings( wxCommandEvent& event )
{
    GribOverlaySettings initSettings = m_OverlaySettings;
    GribSettingsDialog *dialog = new GribSettingsDialog( *this, m_OverlaySettings,  m_lastdatatype);
    dialog->m_sButtonApply->SetLabel(_("Apply"));
    if(dialog->ShowModal() == wxID_OK)
    {
        dialog->WriteSettings();
        m_OverlaySettings.Write();
    } else
        m_OverlaySettings = initSettings;

    SetFactoryOptions(true);
    TimelineChanged();
}

void GRIBUIDialog::OnPlayStop( wxCommandEvent& event )
{
    if( m_bPlay->IsSameAs( m_bpPlay->GetBitmapLabel()) ) {
        m_bpPlay->SetBitmap(wxBitmap( stop ));
        m_bpPlay->SetToolTip( _("Stop") );
        m_tPlayStop.Start( 1000/m_OverlaySettings.m_UpdatesPerSecond, wxTIMER_CONTINUOUS );
    } else {
        m_bpPlay->SetBitmap(*m_bPlay );
        m_bpPlay->SetToolTip( _("Play") );
    }
}

void GRIBUIDialog::OnPlayStopTimer( wxTimerEvent & )
{
    if( m_bPlay->IsSameAs( m_bpPlay->GetBitmapLabel()) )
        m_tPlayStop.Stop();
    else if(m_sTimeline->GetValue() >= m_sTimeline->GetMax()) {
        if(m_OverlaySettings.m_bLoopMode) {
            m_sTimeline->SetValue(0);
            TimelineChanged();
        } else {
            m_bpPlay->SetBitmap(*m_bPlay );
            m_bpPlay->SetToolTip( _("Play") );
            m_tPlayStop.Stop();
        }
    } else {
        m_sTimeline->SetValue(m_sTimeline->GetValue() + 1);
        m_InterpolateMode = m_OverlaySettings.m_bInterpolate;
        if(!m_InterpolateMode) m_cRecordForecast->SetSelection( m_sTimeline->GetValue() );
        TimelineChanged();
    }
}

void GRIBUIDialog::TimelineChanged()
{
    if( !m_bGRIBActiveFile || (m_bGRIBActiveFile && !m_bGRIBActiveFile->IsOK()) ) {
        pPlugIn->GetGRIBOverlayFactory()->SetGribTimelineRecordSet(NULL);
        return;
    }

    wxDateTime time = TimelineTime();
    SetGribTimelineRecordSet(GetTimeLineRecordSet(time));

    if( !m_InterpolateMode ){
        double sel = (m_cRecordForecast->GetCurrentSelection());

    /* get closest value to update timeline */
        m_sTimeline->SetValue((int)
            m_OverlaySettings.m_bInterpolate ? sel / (m_cRecordForecast->GetCount()-1) * m_sTimeline->GetMax() : sel
            );
    } else
        m_cRecordForecast->SetValue( TToString( time, pPlugIn->GetTimeZone() ) );

    pPlugIn->SendTimelineMessage(time);
    RequestRefresh( pParent );
}

int GRIBUIDialog::GetTimePosition(wxDateTime time)
{
    /* get closest index to update combo box */
    size_t i;
    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();

    wxDateTime itime, ip1time;
    for(i=0; i<rsa->GetCount()-1; i++) {
        itime = rsa->Item(i).m_Reference_Time;
        ip1time = rsa->Item(i+1).m_Reference_Time;
        if(ip1time >= time)
            break;
    }
    return (time - itime < ip1time - time) ? i : i+1;
}

wxDateTime GRIBUIDialog::TimelineTime()
{
    if(m_InterpolateMode) {
        int tl = (m_TimeLineHours == 0) ? 0 : m_sTimeline->GetValue();
        int stepmin = round ( 60. * (double)m_OverlaySettings.m_SlicesPerUpdate/(double)m_OverlaySettings.m_HourDivider );
        return MinTime() + wxTimeSpan( tl * stepmin / 60, (tl * stepmin) % 60 );
    } else {
        ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
        int index = m_cRecordForecast->GetCurrentSelection() < 1 ? 0 :m_cRecordForecast->GetCurrentSelection();
        return rsa->Item(index).m_Reference_Time;
    }
}

wxDateTime GRIBUIDialog::MinTime()
{
    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
    if(rsa && rsa->GetCount()) {
        GribRecordSet &first = rsa->Item(0);
        return first.m_Reference_Time;
    }
    return wxDateTime(0.0);
}

GribTimelineRecordSet* GRIBUIDialog::GetTimeLineRecordSet(wxDateTime time)
{
    unsigned int i, im1;
    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();

    wxDateTime curtime;
    for(i=0; i<rsa->GetCount(); i++) {
        GribRecordSet &cur=rsa->Item(i);
        curtime = cur.m_Reference_Time;
        if(curtime >= time)
            break;
    }
    im1 = i-1;
    if(i == 0)
        im1 = 0;

    if(!m_InterpolateMode) im1 = i;

    wxDateTime mintime = MinTime();
    double minute2 = (curtime - mintime).GetMinutes();
    curtime = rsa->Item(im1).m_Reference_Time;
    double minute1 = (curtime - mintime).GetMinutes();
    double nminute = (time - mintime).GetMinutes();

    if(minute2<minute1 || nminute < minute1 || nminute > minute2)
        return NULL;

    double interp_const;
    if(minute1 == minute2)
        interp_const = 0;
    else
        interp_const = (nminute-minute1) / (minute2-minute1);

    GribRecordSet &GRS1 = rsa->Item(im1), &GRS2 = rsa->Item(i);
    return new GribTimelineRecordSet(GRS1, GRS2, interp_const);
}

void GRIBUIDialog::OnTimeline( wxScrollEvent& event )
{
    m_InterpolateMode = m_OverlaySettings.m_bInterpolate;
    if(!m_InterpolateMode) m_cRecordForecast->SetSelection(m_sTimeline->GetValue());
    TimelineChanged();
}

void GRIBUIDialog::OnCBAny( wxCommandEvent& event )
{
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::OnOpenFile( wxCommandEvent& event )
{
    if( !wxDir::Exists( m_grib_dir ) ) {
        wxStandardPathsBase& path = wxStandardPaths::Get();
        m_grib_dir = path.GetDocumentsDir();
    }

    wxFileDialog *dialog = new wxFileDialog(this, _("Select a GRIB file"), m_grib_dir,
        _T(""), wxT ( "Grib files (*.grb;*.bz2|*.grb;*.bz2|All files (*)|*.*"), wxFD_OPEN, wxDefaultPosition,
        wxDefaultSize, _T("File Dialog") );

    if( dialog->ShowModal() == wxID_OK ) {
        m_grib_dir = dialog->GetDirectory();
        m_file_name = dialog->GetPath();
        OpenFile();
    }
}

void GRIBUIDialog::CreateActiveFileFromName( wxString filename )
{
    if( !filename.IsEmpty() ) {
        m_bGRIBActiveFile = NULL;
        m_bGRIBActiveFile = new GRIBFile( filename , pPlugIn->GetCopyFirstCumRec(),
                                          pPlugIn->GetCopyMissWaveRec() );
    }
}

void GRIBUIDialog::PopulateComboDataList()
{
    int index = 0;
#if wxCHECK_VERSION(2,9,0)
    if (!m_cRecordForecast->IsListEmpty() ){
#else
    if( !m_cRecordForecast->IsEmpty() ){
#endif
        index = m_cRecordForecast->GetCurrentSelection();
        m_cRecordForecast->Clear();
    }

    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
    for( size_t i = 0; i < rsa->GetCount(); i++ ) {
        wxDateTime t( rsa->Item( i ).m_Reference_Time );

        m_cRecordForecast->Append( TToString( t, pPlugIn->GetTimeZone() ) );
    }
    m_cRecordForecast->SetSelection( index );
}

void GRIBUIDialog::OnPrev( wxCommandEvent& event )
{
    if(m_InterpolateMode) m_cRecordForecast->SetSelection(GetTimePosition(TimelineTime()));  /* set to interpolated entry */

    m_InterpolateMode = false;

    m_cRecordForecast->SetSelection( m_cRecordForecast->GetCurrentSelection() < 1 ? 0: m_cRecordForecast->GetCurrentSelection() - 1 );

    TimelineChanged();
}

void GRIBUIDialog::OnNext( wxCommandEvent& event )
{
    if(m_InterpolateMode) m_cRecordForecast->SetSelection(GetTimePosition(TimelineTime()));  /* set to interpolated entry */

    m_InterpolateMode = false;

    if( m_cRecordForecast->GetCurrentSelection() == (int)m_cRecordForecast->GetCount() - 1 ) return; //end of list

    m_cRecordForecast->SetSelection( m_cRecordForecast->GetCurrentSelection() + 1 );

    TimelineChanged();
}

void GRIBUIDialog::ComputeBestForecastForNow()
{
    //wxDateTime::Now() is in local time and must be transslated to UTC to be compared to grib times
    wxDateTime now = wxDateTime::Now().ToUTC(wxDateTime::Now().IsDST()==0);
    if(now.IsDST()) now.Add(wxTimeSpan( 1,0,0,0));          //bug in wxWidgets ?

    m_InterpolateMode = m_OverlaySettings.m_bInterpolate;
    if( m_InterpolateMode ) {
        wxTimeSpan span = now - MinTime();
        int stepmin = round ( 60. * (double)m_OverlaySettings.m_SlicesPerUpdate/(double)m_OverlaySettings.m_HourDivider );
        m_sTimeline->SetValue(span.GetMinutes()%stepmin < stepmin/2 ? span.GetMinutes()/stepmin : (span.GetMinutes()/stepmin) + 1);
    }
    else {
        ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
        for( size_t i=0; i<rsa->GetCount()-1; i++ ) {
            wxDateTime ti2( rsa->Item(i+1).m_Reference_Time );
            if(ti2 >= now) {
                wxDateTime ti1( rsa->Item(i).m_Reference_Time );
                m_cRecordForecast->SetSelection(now-ti1 < ti2-now ? i : i+1);
                break;
            }
        }
    }
    TimelineChanged();

}

void GRIBUIDialog::SetGribTimelineRecordSet(GribTimelineRecordSet *pTimelineSet)
{
    delete m_pTimelineSet;
    m_pTimelineSet = pTimelineSet;

    if(!pPlugIn->GetGRIBOverlayFactory())
        return;

    pPlugIn->GetGRIBOverlayFactory()->SetGribTimelineRecordSet(m_pTimelineSet);
}

void GRIBUIDialog::SetFactoryOptions( bool set_val )
{
    int max = wxMax(m_sTimeline->GetMax(), 1), val = m_sTimeline->GetValue();             //memorize the old range and value

    if(m_OverlaySettings.m_bInterpolate){
        int stepmin = round ( 60. * (double)m_OverlaySettings.m_SlicesPerUpdate/(double)m_OverlaySettings.m_HourDivider );
        m_sTimeline->SetMax(m_TimeLineHours * 60 / stepmin );
    }
    else {
        if(m_bGRIBActiveFile && m_bGRIBActiveFile->IsOK()) {
            ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
            m_sTimeline->SetMax(rsa->GetCount()-1);
        }
    }

    //try to retrieve a coherent timeline value with the new timeline range if it has changed
        if( set_val && m_sTimeline->GetMax() != 0 )
            m_sTimeline->SetValue( m_sTimeline->GetMax() * val / max );

    if(m_pTimelineSet)
        m_pTimelineSet->ClearCachedData();

    pPlugIn->GetGRIBOverlayFactory()->ClearCachedData();

    UpdateTrackingControls();
    RequestRefresh( pParent );
}

//----------------------------------------------------------------------------------------------------------
//          GRIBFile Object Implementation
//----------------------------------------------------------------------------------------------------------

GRIBFile::GRIBFile( const wxString file_name, bool CumRec, bool WaveRec )
{
    m_bOK = true;           // Assume ok until proven otherwise

    if( !::wxFileExists( file_name ) ) {
        m_last_message = _( "Error:  File does not exist!" );
        m_bOK = false;
        return;
    }

    //    Use the zyGrib support classes, as (slightly) modified locally....

    m_pGribReader = new GribReader();

    //    Read and ingest the entire GRIB file.......
    m_pGribReader->openFile( file_name );

    if( !m_pGribReader->isOk() ) {
        m_last_message = _( "Error:  Can't read this File!" );
        m_bOK = false;
        return;
    }

    m_FileName = file_name;

    if( CumRec ) m_pGribReader->copyFirstCumulativeRecord();            //add missing records if option selected
    if( WaveRec ) m_pGribReader->copyMissingWaveRecords ();             //  ""                   ""

    m_nGribRecords = m_pGribReader->getTotalNumberOfGribRecords();

    //    Walk the GribReader date list to populate our array of GribRecordSets

    std::set<time_t>::iterator iter;
    std::set<time_t> date_list = m_pGribReader->getListDates();
    for( iter = date_list.begin(); iter != date_list.end(); iter++ ) {
        GribRecordSet *t = new GribRecordSet();
        time_t reftime = *iter;
        t->m_Reference_Time = reftime;
        m_GribRecordSetArray.Add( t );
    }

    //    Convert from zyGrib organization by data type/level to our organization by time.

    GribRecord *pRec;
    bool isOK(false);
    //    Get the map of GribRecord vectors
    std::map<std::string, std::vector<GribRecord *>*> *p_map = m_pGribReader->getGribMap();

    //    Iterate over the map to get vectors of related GribRecords
    std::map<std::string, std::vector<GribRecord *>*>::iterator it;
    for( it = p_map->begin(); it != p_map->end(); it++ ) {
        std::vector<GribRecord *> *ls = ( *it ).second;
        for( zuint i = 0; i < ls->size(); i++ ) {
            pRec = ls->at( i );
            isOK = true;
            time_t thistime = pRec->getRecordCurrentDate();

            //   Search the GribRecordSet array for a GribRecordSet with matching time
            for( unsigned int j = 0; j < m_GribRecordSetArray.GetCount(); j++ ) {
                if( m_GribRecordSetArray.Item( j ).m_Reference_Time == thistime ) {
                    int idx = -1;
                    switch(pRec->getDataType()) {
                    case GRB_WIND_VX:  idx = Idx_WIND_VX; break;
                    case GRB_WIND_VY:  idx = Idx_WIND_VY; break;
                    case GRB_WIND_GUST: idx = Idx_WIND_GUST; break;
                    case GRB_PRESSURE: idx = Idx_PRESSURE;   break;
                    case GRB_HTSGW:    idx = Idx_HTSIGW;  break;
                    case GRB_WVDIR:    idx = Idx_WVDIR;   break;
                    case GRB_UOGRD:    idx = Idx_SEACURRENT_VX; break;
                    case GRB_VOGRD:    idx = Idx_SEACURRENT_VY; break;
                    case GRB_PRECIP_TOT: idx = Idx_PRECIP_TOT; break;
                    case GRB_CLOUD_TOT:  idx = Idx_CLOUD_TOT; break;
                    case GRB_TEMP:     idx = Idx_AIR_TEMP_2M; break;
                    case GRB_WTMP:     idx = Idx_SEA_TEMP; break;
                    case GRB_CAPE:      idx = Idx_CAPE;break;
                    }

                    if(idx != -1) {
                        m_GribRecordSetArray.Item( j ).m_GribRecordPtrArray[idx]= pRec;
                        if(m_GribIdxArray.Index(idx) == wxNOT_FOUND ) m_GribIdxArray.Add(idx, 1);
                    }
                    break;
                }
            }
        }
    }
    if(isOK) m_pRefDateTime = pRec->getRecordRefDate();     //to ovaid crash with ceratain bad files
}

GRIBFile::~GRIBFile()
{
    delete m_pGribReader;
}

//----------------------------------------------------------------------------------------------------------
//          GRIB Request Implementation
//----------------------------------------------------------------------------------------------------------
void GribRequestSetting::InitRequestConfig()
{
    long i,j;
    ( (wxString) m_RequestConfigBase.GetChar(0) ).ToLong( &i );             //MailTo
    m_pMailTo->SetSelection(i);
    ( (wxString) m_RequestConfigBase.GetChar(1) ).ToLong( &i );             //Model
    ( (wxString) m_RequestConfigBase.GetChar(2) ).ToLong( &j );             //Resolution

    ApplyRequestConfig( i, j );

    ( (wxString) m_RequestConfigBase.GetChar(3) ).ToLong( &j );             //interval
    m_pInterval->SetSelection( j );

    ( (wxString) m_RequestConfigBase.GetChar(4) ).ToLong( &j );             //Time Range
    m_pTimeRange->SetSelection( j );

    ( (wxString) m_RequestConfigBase.GetChar(5) ).ToLong( &j );             //Waves model
    m_pWModel->SetSelection( j );

    m_pWind->Enable( false );                                               //always selected if available
    m_pPress->Enable( false );

    m_MailImage->SetForegroundColour(wxColor( 0, 0, 0 ));               //permit to send a (new) message
    m_rButtonYes->Show();

    m_MailImage->SetValue( WriteMail() );

    this->Fit();
    this->Refresh();

}

void GribRequestSetting::ApplyRequestConfig( int sel1, int sel2 )
{
    //some useful  strings
    const wxString res[][3] = { {_T("0.5"), _T("1"), _T("2")},
        {_T("0.2"), _T("0.8"), _T("1.6")} };

    bool IsZYGRIB = false, IsGFS = false, IsRTOFS = false;

    //Model is always GFS if Zygrib
    if( m_pMailTo->GetCurrentSelection() == ZYGRIB ) { sel1 = GFS; IsZYGRIB = true; }

    if( sel1 == GFS ) IsGFS = true;

    //Resolution is always 0.5 if RTOFS
    if( sel1 == RTOFS) { sel2 = 0; IsRTOFS = true; }

    m_pModel->SetSelection( sel1 );

    //populate resolution choice
    int m = (sel1 == 2) ? 0 : sel1;
    for( int i = 0; i<3; i++ ) {
        m_pResolution->SetString(i,res[m][i]);
    }
     m_pResolution->SetSelection(sel2);
    //apply time & resolution limits
    if(!IsGFS) m_pInterval->SetSelection( wxMax(1, m_pInterval->GetCurrentSelection()));               //mini 6 hours for COAMPS & RTOFS
    if( sel1 == COAMPS) m_pTimeRange->SetSelection( wxMin(1, m_pTimeRange->GetCurrentSelection()));    //maxi 3 jours for this model
    if(IsRTOFS) m_pTimeRange->SetSelection( wxMin(4, m_pTimeRange->GetCurrentSelection()));            // maxi 6 jours for this model

    m_pResolution->Enable( !IsRTOFS );
    m_pModel->Enable(!IsZYGRIB);
    m_pWind->SetValue( !IsRTOFS );
    m_pPress->SetValue( !IsRTOFS );
    m_pWaves->SetValue( m_RequestConfigBase.GetChar(8) == 'X' && IsGFS );
    m_pWaves->Enable( IsGFS );
    m_pRainfall->SetValue( m_RequestConfigBase.GetChar(9) == 'X' && IsGFS );
    m_pRainfall->Enable( IsGFS );
    m_pCloudCover->SetValue( m_RequestConfigBase.GetChar(10) == 'X' && IsGFS );
    m_pCloudCover->Enable( IsGFS );
    m_pAirTemp->SetValue( m_RequestConfigBase.GetChar(11) == 'X' && IsGFS );
    m_pAirTemp->Enable( IsGFS );
    m_pSeaTemp->SetValue( (m_RequestConfigBase.GetChar(12) == 'X' && (!IsZYGRIB && IsGFS)) || IsRTOFS );
    m_pSeaTemp->Enable( !IsZYGRIB && IsGFS );
    m_pWindGust->SetValue( m_RequestConfigBase.GetChar(14) == 'X' && IsZYGRIB);
    m_pWindGust->Enable( IsZYGRIB );
    m_pCAPE->SetValue( m_RequestConfigBase.GetChar(15) == 'X' && IsGFS );
    m_pCAPE->Enable( IsGFS );
    m_pCurrent->SetValue( IsRTOFS );
    m_pCurrent->Enable( false );

    //show parameters only if necessary
    m_tLogin->Show(IsZYGRIB);
    m_pLogin->Show(IsZYGRIB);
    m_tCode->Show(IsZYGRIB);
    m_pCode->Show(IsZYGRIB);
    m_tWModel->Show(IsZYGRIB && m_pWaves->IsChecked());
    m_pWModel->Show(IsZYGRIB && m_pWaves->IsChecked());
}

void GribRequestSetting::OnTopChange(wxCommandEvent &event)
{

    ApplyRequestConfig( m_pModel->GetCurrentSelection(), m_pResolution->GetCurrentSelection() );

    m_MailImage->SetForegroundColour(wxColor( 0, 0, 0 ));                   //permit to send a (new) message
    m_rButtonYes->Show();

    m_MailImage->SetValue( WriteMail() );

    this->Fit();
    this->Refresh();
}

void GribRequestSetting::OnAnyChange(wxCommandEvent &event)
{
    m_tWModel->Show(m_pMailTo->GetCurrentSelection() == ZYGRIB && m_pWaves->IsChecked());
    m_pWModel->Show(m_pMailTo->GetCurrentSelection() == ZYGRIB && m_pWaves->IsChecked());

    //apply time & resolution limits
    if(m_pModel->GetCurrentSelection() != GFS) m_pInterval->SetSelection(
        wxMax(1, m_pInterval->GetCurrentSelection()));  //mini 6 hours for COAMPS & RTOFS
    if( m_pModel->GetCurrentSelection() == COAMPS) m_pTimeRange->SetSelection(
        wxMin(1, m_pTimeRange->GetCurrentSelection()));   //maxi 3 days for this model
    if(m_pModel->GetCurrentSelection() == RTOFS) m_pTimeRange->SetSelection(
        wxMin(4, m_pTimeRange->GetCurrentSelection()));     // maxi 6 days for this model

    m_MailImage->SetForegroundColour(wxColor( 0, 0, 0 ));                   //permit to send a (new) message
    m_rButtonYes->Show();

    m_MailImage->SetValue( WriteMail() );

    this->Fit();
    this->Refresh();
}

wxString GribRequestSetting::WriteMail()
{
    m_MailError_Nb = 0;
    //some useful strings
    const wxString s[] = { _T(","), _T(" ") };        //separators
    const wxString p[][7] = {{ _T("APCP"), _T("TCDC"), _T("AIRTMP"), _T("WAVES"), _T("SEATMP"), wxEmptyString, _T("CAPE")}, //parameters
        {_T("PRECIP"), _T("CLOUD"), _T("TEMP"), _T("WVSIG WVWIND"), wxEmptyString, _T("GUST"), _T("CAPE")} };

    wxString r_topmess,r_parameters,r_zone;
    //write the top part of the mail
    switch( m_pMailTo->GetCurrentSelection() ) {
    case SAILDOCS:                                                                         //Saildocs
        r_zone = toMailFormat(1, m_LatmaxBase) + _T(",") + toMailFormat(1, m_LatminBase) + _T(",")
            + toMailFormat(2, m_LonminBase) + _T(",") + toMailFormat(2, m_LonmaxBase);
        r_topmess = wxT("send ");
        r_topmess.Append(m_pModel->GetStringSelection() + _T(":"));
        r_topmess.Append( r_zone  + _T("|"));
        double v;
        m_pResolution->GetStringSelection().ToDouble(&v);
        r_topmess.Append(wxString::Format(_T("%1.1f,%1.1f"), v, v) + _T("|"));
        m_pInterval->GetStringSelection().ToDouble(&v);
        r_topmess.Append(wxString::Format(_T("0,%d,%d"), (int) v, (int) v*2));
        m_pTimeRange->GetStringSelection().ToDouble(&v);
        r_topmess.Append(wxString::Format(_T("..%d"), (int) v*24) + _T("|=\n"));
        break;
    case ZYGRIB:                                                                         //Zygrib
        r_zone = toMailFormat(1, m_LatminBase) + toMailFormat(2, m_LonminBase) + _T(" ")
            + toMailFormat(1, m_LatmaxBase) + toMailFormat(2, m_LonmaxBase);
        r_topmess = wxT("login : ");
        r_topmess.Append(m_pLogin->GetValue() + _T("\n"));
        r_topmess.Append(wxT("code :"));
        r_topmess.Append(m_pCode->GetValue() + _T("\n"));
        r_topmess.Append(wxT("area : "));
        r_topmess.append(r_zone + _T("\n"));
        r_topmess.Append(wxT("resol : "));
        r_topmess.append(m_pResolution->GetStringSelection() + _T("\n"));
        r_topmess.Append(wxT("days : "));
        r_topmess.append(m_pTimeRange->GetStringSelection() + _T("\n"));
        r_topmess.Append(wxT("hours : "));
        r_topmess.append(m_pInterval->GetStringSelection() + _T("\n"));
        if(m_pWaves->IsChecked()) {
            r_topmess.Append(wxT("waves : "));
            r_topmess.append(m_pWModel->GetStringSelection() + _T("\n"));
        }
        r_topmess.Append(wxT("meteo : "));
        r_topmess.append(m_pModel->GetStringSelection() + _T("\n"));
        if ( m_pLogin->GetValue().IsEmpty() || m_pCode->GetValue().IsEmpty() ) m_MailError_Nb =1;
        break;
    }
    //write the parameters part of the mail
    switch( m_pModel->GetCurrentSelection() ) {
    case GFS:                                                                           //GFS
        r_parameters = wxT("WIND") + s[m_pMailTo->GetCurrentSelection()] + wxT("PRESS");    // the default minimum request parameters
        if( m_pRainfall->GetValue() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][0] );
        if( m_pCloudCover->GetValue() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][1] );
        if( m_pAirTemp->GetValue() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][2] );
        if( m_pWaves->GetValue() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][3] );
        if( m_pSeaTemp->GetValue() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][4] );
        if( m_pWindGust->GetValue() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][5] );
        if( m_pCAPE->GetValue() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][6] );
        break;
    case COAMPS:                                                                           //COAMPS
        r_parameters = wxT("WIND,PRMSL");                                 //the default parameters for this model
        break;
    case RTOFS:                                                                           //RTOFS
        r_parameters = wxT("CUR,WTMP");                                   //the default parameters for this model
        break;
    }
    if( !EstimateFileSize() ) m_MailError_Nb += 2;
    return wxString( r_topmess + r_parameters );
}

bool GribRequestSetting::EstimateFileSize()
{
    //define size limits
    int limit;
    if(m_pMailTo->GetCurrentSelection() == ZYGRIB )
        limit = 2;
    else                                                                 //No size limit for SaiDocs
        limit = 0;

    //too small zone ? ( mini 2 * resolutions )
    double reso,time,inter;
    m_pResolution->GetStringSelection().ToDouble(&reso);
    m_pTimeRange->GetStringSelection().ToDouble(&time);
    m_pInterval->GetStringSelection().ToDouble(&inter);
    if (m_LonmaxBase-m_LonminBase < 2*reso) {
        m_LonminBase -= reso;
        m_LonmaxBase += reso;
    }
    if (m_LatmaxBase-m_LatminBase < 2*reso) {
        m_LatmaxBase += reso;
        m_LatminBase -= reso;
    }
    int npts = (int) (  ceil(((double)(m_LatmaxBase-m_LatminBase)/reso))
                       * ceil(((double)(m_LonmaxBase-m_LonminBase)/reso)) );

    if(m_pModel->GetCurrentSelection() == COAMPS )                                           //limited area for COAMPS
        npts = wxMin(npts, (int) (  ceil(40.0/reso) * ceil(40.0/reso) ) );

    // Nombre de GribRecords
    int nbrec = (int) (time*24/inter)+1;
    int nbPress = (m_pPress->IsChecked()) ?  nbrec   : 0;
    int nbWind  = (m_pWind->IsChecked()) ?  2*nbrec : 0;
    int nbRain  = (m_pRainfall->IsChecked()) ?  nbrec-1 : 0;
    int nbCloud = (m_pCloudCover->IsChecked()) ?  nbrec-1 : 0;
    int nbTemp  = (m_pAirTemp->IsChecked())    ?  nbrec   : 0;
    int nbSTemp  = (m_pSeaTemp->IsChecked())    ?  nbrec   : 0;
    int nbGUSTsfc  = (m_pWindGust->IsChecked()) ?  nbrec : 0;
    int nbCurrent  = (m_pCurrent->IsChecked()) ?  nbrec : 0;
    int nbCape  = (m_pCAPE->IsChecked()) ?  nbrec : 0;

    int head = 84;
    double estime = 0.0;
    int nbits;

    nbits = 13;
    estime += nbWind*(head+(nbits*npts)/8+2 );
    estime += nbCurrent*(head+(nbits*npts)/8+2 );

    nbits = 11;
    estime += nbTemp*(head+(nbits*npts)/8+2 );
    estime += nbSTemp*(head+(nbits*npts)/8+2 );

    nbits = 4;
    estime += nbRain*(head+(nbits*npts)/8+2 );

    nbits = 15;
    estime += nbPress*(head+(nbits*npts)/8+2 );

    nbits = 4;
    estime += nbCloud*(head+(nbits*npts)/8+2 );

    nbits = 7;
    estime += nbGUSTsfc*(head+(nbits*npts)/8+2 );

    nbits = 5;
    estime += nbCape*(head+(nbits*npts)/8+2 );

	int nbwave = 0;
	if (m_pWaves->IsChecked()) nbwave++;
	if (m_pWModel->IsShown()) nbwave ++;
	nbits = 6;
	estime += nbrec*nbwave*(head+(nbits*npts)/8+2 );

	estime /= (1024*1024);

    m_tFileSize->SetLabel(wxString::Format(_T("%1.2f Mo"), estime));

    if(limit) {
        m_tLimit->SetLabel(_("( Max ") + wxString::Format(_T("%d"), limit) + _(" Mo )"));
        if(estime > limit) return false;
    } else
        m_tLimit->SetLabel(_T(""));

    return true;
}

void GribRequestSetting::OnSendMaiL( wxCommandEvent& event  )
{
    const wxString error[] = { _T("\n\n"), _("Before sending an email to Zygrib you have to enter your Login and Code.\nPlease visit www.zygrib.org/ and follow instructions..."),
        _("The file size limit is overcome!\nYou can zoom in and/or change parameters...") };

    if( m_MailError_Nb ) {
        m_MailImage->SetForegroundColour(wxColor( 255, 0, 0 ));
        if( m_MailError_Nb == 3 )
            m_MailImage->SetValue( error[1] + error[0] + error[2] );
        else
            m_MailImage->SetValue( error[m_MailError_Nb] );
        m_rButtonYes->Hide();
        this->Fit();
        this->Refresh();
        return;
    }

    wxMailMessage *message = new wxMailMessage(
    wxT("gribauto"),                                                                            //requested subject
    (m_pMailTo->GetCurrentSelection() == SAILDOCS) ? m_MailToAddresses.BeforeFirst(_T(';'))     //to request address
        : m_MailToAddresses.AfterFirst(_T(';')),
    WriteMail(),                                                                                 //message image
    m_pSenderAddress->GetValue()
    );
    wxEmail mail ;
    m_MailImage->SetForegroundColour(wxColor( 255, 0, 0 ));
    if(mail.Send( *message ) ) {
#ifdef __WXMSW__
        m_MailImage->SetValue(
            _("Your request is ready. An email is prepared in your email environment. \nYou have just to verify and send it...\nSave or Cancel to finish...or new parameters for a new email ...") );
#else
        m_MailImage->SetValue(
            _("Your request was sent \n(if your system has an MTA configured and is able to send email).\nSave or Cancel to finish...or new parameters for a new email ..."));
#endif
    } else {
        m_MailImage->SetValue(
            _("Request can't be sent. Please verify your email systeme parameters.\nYou should also have a look at your log file.\nSave or Cancel to finish..."));
    }
    m_rButtonYes->Hide();
    this->Fit();
    this->Refresh();
}
