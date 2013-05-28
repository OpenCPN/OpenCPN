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

wxString toSAILDOC ( int NEflag, int MMflag, double a )
{
    //saildoc needs value in degré (without decimal) so translate doube value in integer and if necessary tacking one degré more 
    //to be sure all the screen is covered
    short neg = 0;
    if ( a < 0.0 ) {
        a = -a;
        neg = 1;
    }
    char c;
    switch(NEflag) {
        case 1: {
            if ( !neg ) {
                if( MMflag == 1 ) a += 1.;
                c = 'N';
            } else {
                if( MMflag == 2 ) a += 1.;
                c = 'S';                               
            }
            break;
        }
        case 2: {
            if ( !neg ) {
                if( MMflag == 1 ) a += 1.; 
                c = 'E';
            } else {
                if( MMflag == 2 ) a += 1.;
                c = 'W';                                  
            }
        }
    }
    wxString s;
    s.Printf ( _T ( "%01d%c" ), (int) a, c );
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
//    delete m_bGRIBActiveFile;
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
    SetLabel( fn.GetFullName() );

    if( m_bGRIBActiveFile ) {
        if( m_bGRIBActiveFile->IsOK() ) { 
            //there could be valid but empty file
            if( rsa->GetCount() == 0 ) {
                m_bGRIBActiveFile = NULL;
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( _("Error:  No valid data in this file!") );
            } else
                PopulateComboDataList( 0 );
        } else {
            if( fn.IsDir() ) {
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( _("Warning:  Empty directory!") );
                SetLabel( fn.GetFullPath() );
            }
            else
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( m_bGRIBActiveFile->GetLastMessage() );
	}
        SetFactoryOptions();
        DisplayDataGRS();
        PopulateTrackingControls();
    }
}

wxString GRIBUIDialog::GetNewestFileInDirectory()
{
    if( !wxDir::Exists( m_grib_dir ) ) {
         wxStandardPaths path;
         m_grib_dir = path.GetDocumentsDir();
    }
    //    Get an array of GRIB file names in the target directory, not descending into subdirs
    wxArrayString file_array;
    int m_n_files = 0;
    m_n_files = wxDir::GetAllFiles( m_grib_dir, &file_array, _T ( "*.grb" ), wxDIR_FILES );
    m_n_files += wxDir::GetAllFiles( m_grib_dir, &file_array, _T ( "*.grb.bz2" ),
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

        pConf->Read ( _T ( "lastdatatype" ), &m_lastdatatype, 0);

        pConf->Read ( _T ( "Filename" ), &m_file_name );

        wxStandardPaths spath;
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

    GribRecord **RecordArray;
    if( m_pTimelineSet )
        RecordArray = m_pTimelineSet->m_GribRecordPtrArray;

    AddTrackingControl(m_cbWind, m_tcWindSpeed, m_tcWindDirection,
                       m_pTimelineSet && RecordArray[Idx_WIND_VX] && RecordArray[Idx_WIND_VY]);
    AddTrackingControl(m_cbWindScat, m_tcWindScatSpeed, m_tcWindScatDirection,
                       m_pTimelineSet && RecordArray[Idx_WINDSCAT_VX] && RecordArray[Idx_WINDSCAT_VY]);
    AddTrackingControl(m_cbWindGust, m_tcWindGust, 0, m_pTimelineSet && RecordArray[Idx_WIND_GUST]);
    AddTrackingControl(m_cbPressure, m_tcPressure, 0, m_pTimelineSet && RecordArray[Idx_PRESSURE]);

    /* tracking for wave is funky */
    if(m_pTimelineSet && RecordArray[Idx_HTSIGW]) {
        if(RecordArray[Idx_WVDIR])
            AddTrackingControl(m_cbWave, m_tcWaveHeight, m_tcWaveDirection, true);
        else {
            AddTrackingControl(m_cbWave, m_tcWaveHeight, 0, true);
            m_tcWaveDirection->Hide();
        }
    } else
        if(m_pTimelineSet && RecordArray[Idx_WVDIR]) {
            AddTrackingControl(m_cbWave, 0, m_tcWaveDirection, true);
            m_tcWaveHeight->Hide();
        } else
            AddTrackingControl(m_cbWave, m_tcWaveHeight, m_tcWaveDirection, false);

    AddTrackingControl(m_cbCurrent, m_tcCurrentVelocity, m_tcCurrentDirection,
                       m_pTimelineSet && RecordArray[Idx_SEACURRENT_VX] && RecordArray[Idx_SEACURRENT_VY]);
    AddTrackingControl(m_cbPrecipitation, m_tcPrecipitation, 0,
                       m_pTimelineSet && RecordArray[Idx_PRECIP_TOT]);
    AddTrackingControl(m_cbCloud, m_tcCloud, 0,
                       m_pTimelineSet && RecordArray[Idx_CLOUD_TOT]);
    AddTrackingControl(m_cbAirTemperature, m_tcAirTemperature, 0,
                       m_pTimelineSet && RecordArray[Idx_AIR_TEMP_2M]);
    AddTrackingControl(m_cbSeaTemperature, m_tcSeaTemperature, 0,
                       m_pTimelineSet && RecordArray[Idx_SEA_TEMP]);

    Fit();
    Refresh();
}

void GRIBUIDialog::UpdateTrackingControls( void )
{
    if( !m_pTimelineSet )
        return;
#if(0)
    wxDateTime t = m_pTimelineSet->m_Reference_Time;
    t.MakeFromTimezone( wxDateTime::UTC );
    if( t.IsDST() ) t.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );
    m_cRecordForecast->SetLabel(t.Format(_T("%Y-%m-%d %H:%M:%S "), wxDateTime::Local) + _T("Local - ") +
                                t.Format(_T("%H:%M:%S "), wxDateTime::UTC) + _T("GMT"));
#endif   
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
    
    //    Update the QuickScat (aka Wind) control
    if( RecordArray[Idx_WINDSCAT_VX] && RecordArray[Idx_WINDSCAT_VY] ) {
        double vx = RecordArray[Idx_WINDSCAT_VX]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
        double vy = RecordArray[Idx_WINDSCAT_VY]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
        
        if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
            vx = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND, vx);
            vy = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND, vy);
            
            double vkn = sqrt( vx * vx + vy * vy );
            m_tcWindScatSpeed->SetValue( wxString::Format( _T("%2d ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WIND), (int) vkn ) );
            
            double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
            if( ang > 360. ) ang -= 360.;
            if( ang < 0. ) ang += 360.;
            m_tcWindScatDirection->SetValue( wxString::Format( _T("%03d\u00B0"), (int) ( ang ) ) );
        } else {
            m_tcWindScatSpeed->SetValue( _("N/A") );
            m_tcWindScatDirection->SetValue( _("N/A") );
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
    wxString zone( 
        toSAILDOC( 1, 1, m_vp->lat_max ) + wxT(",") +
        toSAILDOC( 1, 2, m_vp->lat_min ) + wxT(",") +
        toSAILDOC( 2, 2, lonmin ) + wxT(",") +
        toSAILDOC( 2, 1, lonmax ) );

    GribRequestSetting *req_Dialog = new GribRequestSetting( this, pPlugIn->GetRequestConfig(), zone, pPlugIn->GetMailAdresse() );
    
    if( req_Dialog->ShowModal() == wxID_SAVE ) {
        req_Dialog->m_RequestConfigBase.SetChar( 2, (char) ( req_Dialog->m_pModel->GetCurrentSelection() + '0' ) );
        if( req_Dialog->m_pModel->GetCurrentSelection() == 0 ) {
            req_Dialog->m_RequestConfigBase.SetChar( 3, (char) ( req_Dialog->m_pResolution->GetCurrentSelection() + '0' ) );
            req_Dialog->m_pWaves->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 8, 'X' ) : req_Dialog->m_RequestConfigBase.SetChar( 8, '.' );
            req_Dialog->m_pRainfall->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 9, 'X' ) : req_Dialog->m_RequestConfigBase.SetChar( 9, '.' );
            req_Dialog->m_pCloudCover->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 10, 'X' ) : req_Dialog->m_RequestConfigBase.SetChar( 10, '.' );
            req_Dialog->m_pAirTemp->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 11, 'X' ) : req_Dialog->m_RequestConfigBase.SetChar( 11, '.' );
            req_Dialog->m_pSeaTemp->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 12, 'X' ) : req_Dialog->m_RequestConfigBase.SetChar( 12, '.' );
            req_Dialog->m_pCurrent->IsChecked() ? req_Dialog->m_RequestConfigBase.SetChar( 13, 'X' ) : req_Dialog->m_RequestConfigBase.SetChar( 13, '.' );
        } else if( req_Dialog->m_pModel->GetCurrentSelection() == 1 ) 
            req_Dialog->m_RequestConfigBase.SetChar( 3, (char) ( req_Dialog->m_pResolution->GetCurrentSelection() + '0' ) );

        req_Dialog->m_RequestConfigBase.SetChar( 6, 'X' );              //must be always selected
        req_Dialog->m_RequestConfigBase.SetChar( 7, 'X' );

        req_Dialog->m_RequestConfigBase.SetChar( 4, (char) ( req_Dialog->m_pInterval->GetCurrentSelection() + '0' ) );
        req_Dialog->m_RequestConfigBase.SetChar( 5, (char) ( req_Dialog->m_pTimeRange->GetCurrentSelection() + '0' ) );

        pPlugIn->SetRequestConfig( req_Dialog->m_RequestConfigBase );
    }

    req_Dialog->Destroy();
}

void GRIBUIDialog::OnSettings( wxCommandEvent& event )
{
    GribOverlaySettings initSettings = m_OverlaySettings;
    GribSettingsDialog *dialog = new GribSettingsDialog( *this, m_OverlaySettings,  m_lastdatatype);
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
        TimelineChanged();
    }
}

#if 0
void GRIBUIDialog::OnFileDirChange( wxFileDirPickerEvent &event )
{
    m_pRecordTree->DeleteAllItems();
    delete m_pRecordTree->m_file_id_array;

    m_RecordTree_root_id = m_pRecordTree->AddRoot( _T ( "GRIBs" ) );
    PopulateTreeControl();
    m_pRecordTree->Expand( m_RecordTree_root_id );

    pPlugIn->GetGRIBOverlayFactory()->Reset();

    Refresh();

    m_grib_dir = event.GetPath();
}
#endif

void GRIBUIDialog::TimelineChanged(bool sync)
{
    if(!m_bGRIBActiveFile)
        return;

    wxDateTime time = TimelineTime();
    SetGribTimelineRecordSet(GetTimeLineRecordSet(time));

    /* get closest index to update combo box */
    unsigned int i;
    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();

    if(rsa->GetCount() < 2) { /* this case should be handled below instead,
                                 but apparently the logic is wrong and sometimes crashes */
        m_cRecordForecast->SetSelection(0);
        return;
    }

    wxDateTime itime, ip1time;
    for(i=0; i<rsa->GetCount()-1; i++) {
        itime = rsa->Item(i).m_Reference_Time;
        ip1time = rsa->Item(i+1).m_Reference_Time;
        if(ip1time >= time)
            break;
    }

    if(time - itime < ip1time - time)
        m_cRecordForecast->SetSelection(i);
    else
        m_cRecordForecast->SetSelection(i+1);

    if(sync)
        return;

//    m_cRecordForecast->ToggleWindowStyle(wxCB_READONLY);
        m_cRecordForecast->SetValue( TToString( time, pPlugIn->GetTimeZone() ) );
//    m_cRecordForecast->ToggleWindowStyle(wxCB_READONLY);
    
    pPlugIn->SendTimelineMessage(time);
    RequestRefresh( pParent );
}

wxDateTime GRIBUIDialog::TimelineTime()
{
    int tl = (m_TimeLineHours == 0) ? 0 : m_sTimeline->GetValue();
    if(m_OverlaySettings.m_bInterpolate) {
        //compute timeline with the true step = slices per update*divider 
        int hours = tl*m_OverlaySettings.m_SlicesPerUpdate/m_OverlaySettings.m_HourDivider;
        int minutes = ((tl*m_OverlaySettings.m_SlicesPerUpdate)%m_OverlaySettings.m_HourDivider)*(60*m_OverlaySettings.m_SlicesPerUpdate/m_OverlaySettings.m_HourDivider);
        return MinTime() + wxTimeSpan(hours, minutes);
    } else {
        //compute timeline from the record selected
        ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
        return rsa->Item(tl).m_Reference_Time;
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

#if 0
wxDateTime GRIBUIDialog::MaxTime()
{
    return MinTime() + wxTimeSpan(m_sTimeline->GetMax()/m_OverlaySettings.m_HourDivider);
}
#endif

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
    
    if(!m_OverlaySettings.m_bInterpolate)
        interp_const = round(interp_const);

    GribRecordSet &GRS1 = rsa->Item(im1), &GRS2 = rsa->Item(i);
    return new GribTimelineRecordSet(GRS1, GRS2, interp_const);
}

void GRIBUIDialog::OnTimeline( wxScrollEvent& event )
{
    TimelineChanged();
}

void GRIBUIDialog::OnCBAny( wxCommandEvent& event )
{
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::OnOpenFile( wxCommandEvent& event )
{
    if( !wxDir::Exists( m_grib_dir ) ) {
         wxStandardPaths path;
         m_grib_dir = path.GetDocumentsDir();
    }
       
    wxFileDialog *dialog = new wxFileDialog(this, _("Select a GRIB file"), m_grib_dir, 
        _T(""), wxT ( "Grib files (*.grb;*.grb.bz2|*.grb;*.grb.bz2"), wxFD_OPEN, wxDefaultPosition,
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

void GRIBUIDialog::PopulateComboDataList( int index )
{
    m_cRecordForecast->Clear();

    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
    for( size_t i = 0; i < rsa->GetCount(); i++ ) {
        wxDateTime t( rsa->Item( i ).m_Reference_Time );

        m_cRecordForecast->Append( TToString( t, pPlugIn->GetTimeZone() ) );
    }
    m_cRecordForecast->SetSelection( index );
}

void GRIBUIDialog::OnPrev( wxCommandEvent& event )
{
    if(m_cRecordForecast->GetCurrentSelection() == -1) /* set to interpolated entry */
        TimelineChanged(true);

//    m_pGribForecastTimer.Stop();
    int selection = m_cRecordForecast->GetCurrentSelection() - 1;
    if(selection < 0)
        selection = 0;
    m_cRecordForecast->SetSelection( selection );
    DisplayDataGRS( );
}

void GRIBUIDialog::OnNext( wxCommandEvent& event )
{
    if(m_cRecordForecast->GetCurrentSelection() == -1) /* set to interpolated entry */
        TimelineChanged(true);

    if( m_cRecordForecast->GetCurrentSelection() == (int)m_cRecordForecast->GetCount() - 1 ) return; //end of list

//    m_pGribForecastTimer.Stop();
    m_cRecordForecast->SetSelection( m_cRecordForecast->GetCurrentSelection() + 1 );
    DisplayDataGRS( );
}

void GRIBUIDialog::ComputeBestForecastForNow()
{
    //wxDateTime::Now() is in local time and must be transslated to UTC to be compared to grib times
    wxDateTime now = wxDateTime::Now().ToUTC(wxDateTime::Now().IsDST()==0);
    if(now.IsDST()) now.Add(wxTimeSpan( 1,0,0,0));          //bug in wxWidgets ?

    if( m_OverlaySettings.m_bInterpolate ) {
        wxTimeSpan span = now - MinTime();
        int stepmin = 60*m_OverlaySettings.m_SlicesPerUpdate/m_OverlaySettings.m_HourDivider;
        if((span.GetMinutes()%stepmin) > (stepmin/2)) 
            m_sTimeline->SetValue((span.GetMinutes()+stepmin)/stepmin);
        else
            m_sTimeline->SetValue(span.GetMinutes()/stepmin);
    }
    else {
        ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
        size_t i;
        for( i=0; i<rsa->GetCount()-1; i++ ) {
            wxDateTime ti2( rsa->Item(i+1).m_Reference_Time );
            if(ti2 >= now) {
                wxDateTime ti1( rsa->Item(i).m_Reference_Time );
                if( (now-ti1) > (ti2-now) ) i++;
                break;
            }
        }
        m_sTimeline->SetValue(i);
    }
    TimelineChanged();
}

void GRIBUIDialog::DisplayDataGRS()
{
    if( m_bGRIBActiveFile &&  m_bGRIBActiveFile->IsOK() ) {
        ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
        int selection = m_cRecordForecast->GetCurrentSelection();
        if(selection >=0) {
            GribRecordSet *record = &rsa->Item( selection );
            if(m_OverlaySettings.m_bInterpolate)
                SelectGribRecordSet( record );
            else {
                m_sTimeline->SetValue(selection);
                TimelineChanged();
            }
        }
    } else 
        SelectGribRecordSet( NULL );
}

void GRIBUIDialog::SelectGribRecordSet( GribRecordSet *pGribRecordSet )
{
    if(!pGribRecordSet) {
        pPlugIn->GetGRIBOverlayFactory()->SetGribTimelineRecordSet(NULL);
        return;
    }

    wxDateTime mintime = MinTime(), curtime = pGribRecordSet->m_Reference_Time;
    double hour = (curtime - mintime).GetMinutes()/60.0;

    m_sTimeline->SetValue(hour/m_OverlaySettings.m_SlicesPerUpdate*m_OverlaySettings.m_HourDivider);
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
    int max = m_sTimeline->GetMax(), val = m_sTimeline->GetValue();             //memorize the old range and value

    if(m_OverlaySettings.m_bInterpolate)
        m_sTimeline->SetMax(m_TimeLineHours/m_OverlaySettings.m_SlicesPerUpdate*m_OverlaySettings.m_HourDivider);
    else {
        if(m_bGRIBActiveFile) {
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

    //    Get the map of GribRecord vectors
    std::map<std::string, std::vector<GribRecord *>*> *p_map = m_pGribReader->getGribMap();

    //    Iterate over the map to get vectors of related GribRecords
    std::map<std::string, std::vector<GribRecord *>*>::iterator it;
    for( it = p_map->begin(); it != p_map->end(); it++ ) {
        std::vector<GribRecord *> *ls = ( *it ).second;
        for( zuint i = 0; i < ls->size(); i++ ) {
            pRec = ls->at( i );

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
                    case GRB_USCT:     idx = Idx_WINDSCAT_VX; break;
                    case GRB_VSCT:     idx = Idx_WINDSCAT_VY; break;
                    case GRB_UOGRD:    idx = Idx_SEACURRENT_VX; break;
                    case GRB_VOGRD:    idx = Idx_SEACURRENT_VY; break;
                    case GRB_PRECIP_TOT: idx = Idx_PRECIP_TOT; break;
                    case GRB_CLOUD_TOT:  idx = Idx_CLOUD_TOT; break;
                    case GRB_TEMP:     idx = Idx_AIR_TEMP_2M; break;
                    case GRB_WTMP:     idx = Idx_SEA_TEMP; break;
                    }

                    if(idx != -1)
                        m_GribRecordSetArray.Item( j ).m_GribRecordPtrArray[idx]= pRec;
                    break;
                }
            }
        }
    }
}

GRIBFile::~GRIBFile()
{
    delete m_pGribReader;
}

void GribRequestSetting::InitRequestConfig()
{
    long i,j;
    ( (wxString) m_RequestConfigBase.GetChar(2) ).ToLong( &i );
    ( (wxString) m_RequestConfigBase.GetChar(3) ).ToLong( &j );

    ApplyRequestConfig( i, j );
   
    ( (wxString) m_RequestConfigBase.GetChar(4) ).ToLong( &j );
    m_pInterval->SetSelection( j );

    ( (wxString) m_RequestConfigBase.GetChar(5) ).ToLong( &j );
    m_pTimeRange->SetSelection( j );

    m_pWind->Enable( false );
    m_pPress->Enable( false );

    m_MailImage->SetLabel( WriteMail() );
    Fit();

}

void GribRequestSetting::ApplyRequestConfig( int sel1, int sel2 )
{
    long j;
    m_pModel->SetSelection( sel1 );
    switch( sel1 ) {
    case 0:
        for( int i = 0; i<4; i++ ){
            m_pResolution->SetString(i,resolution0[i]);
        }
        ( (wxString) m_RequestConfigBase.GetChar(3) ).ToLong( &j );
        m_pResolution->SetSelection( j );
        m_pResolution->Enable( true );

        m_pWind->SetValue( true );
        m_pPress->SetValue( true );

        m_pWaves->SetValue( m_RequestConfigBase.GetChar(8) == 'X' );
        m_pWaves->Enable( true );
        m_pRainfall->SetValue( m_RequestConfigBase.GetChar(9) == 'X' );
        m_pRainfall->Enable( true );
        m_pCloudCover->SetValue( m_RequestConfigBase.GetChar(10) == 'X' );
        m_pCloudCover->Enable( true );
        m_pAirTemp->SetValue( m_RequestConfigBase.GetChar(11) == 'X' );
        m_pAirTemp->Enable( true );
        m_pSeaTemp->SetValue( m_RequestConfigBase.GetChar(12) == 'X' );
        m_pSeaTemp->Enable( true );
        break;
    case 1:
        for( int i = 0; i<4; i++ ){
            m_pResolution->SetString(i,resolution1[i]);
        }
        
        m_pResolution->SetSelection( sel2 );
        m_pResolution->Enable( true );
        m_pWind->SetValue( true );                          //default parameters wind and pression
        m_pPress->SetValue( true );
        m_pSeaTemp->Enable(false);                       
        break;
    case 2 :
        for( int i = 0; i<4; i++ ){
            m_pResolution->SetString(i, resolution0[i]);
        }
       
        m_pResolution->SetSelection( 0 );                   //default resolution ("05,05")
        m_pResolution->Enable( false );
        m_pWind->SetValue( false );
        m_pPress->SetValue( false );
        m_pSeaTemp->SetValue( true );                       //default parameters current and sea temperature
        m_pCurrent->SetValue( true );
        m_pCurrent->Enable( false );
        break;
    }
    
    if( sel1 != 0 ) {
        m_pWaves->SetValue( false );
        m_pWaves->Enable( false );
        m_pRainfall->SetValue( false );
        m_pRainfall->Enable( false );
        m_pCloudCover->SetValue( false );
        m_pCloudCover->Enable( false );
        m_pAirTemp->SetValue( false );
        m_pAirTemp->Enable( false );
        m_pSeaTemp->Enable( false );
    }

    if( sel1 != 2 ) {
        m_pCurrent->SetValue( false );
        m_pCurrent->Enable( false );
    }

}

void GribRequestSetting::OnModelChange(wxCommandEvent &event)
{
    ApplyRequestConfig( m_pModel->GetCurrentSelection(), m_pResolution->GetCurrentSelection() );

    //permit to send a new message
    m_MailImage->SetForegroundColour(wxColor( 0, 0, 0 ));
    m_bSend->Show();

    m_MailImage->SetLabel( WriteMail() );
    Fit();
}

void GribRequestSetting::OnAnyChange(wxCommandEvent &event)
{
    //permit to send a new message
    m_MailImage->SetForegroundColour(wxColor( 0, 0, 0 ));
    m_bSend->Show();

    m_MailImage->SetLabel( WriteMail() );
    Fit();
}

wxString GribRequestSetting::WriteMail()
{
    //some useful strings
    const wxString s[] = { wxT("GFS"), wxT("COAMPS"), wxT("RTOFS"),wxT("|"), wxT("|=\n"), wxT(":") };
    const wxString r[][4] = { wxT("0.5,0.5"), wxT("1.0,1.0"), wxT("1.5,1.5"), wxT("2.0,2.0"),
        wxT("0.2,0.2"), wxT("0.6,0.6"), wxT("1.2,1.2"), wxT("2.0,2.0") };
    const wxString i[] = { wxT("0,3,6"), wxT("0,6,12"), wxT("0,12,24") };
    const wxString p[] = { wxT("..96"), wxT("..144"), wxT("..192") };

    wxString r_action = wxT("send"),r_parameters;

    int model = m_pModel->GetCurrentSelection();
    switch( m_pModel->GetCurrentSelection() ) {               
    case 0:                     //GFS
        r_parameters = wxT("WIND,PRESS");                  // the default minimum request parameters
        if( m_pWaves->GetValue() )
            r_parameters.Append( wxT(",WAVES") );
        if( m_pRainfall->GetValue() )
            r_parameters.Append( wxT(",APCP") );
        if( m_pCloudCover->GetValue() )
            r_parameters.Append( wxT(",TCDC") );
        if( m_pAirTemp->GetValue() )
            r_parameters.Append( wxT(",AIRTMP") );
        if( m_pSeaTemp->GetValue() )
            r_parameters.Append( wxT(",SEATMP") );
        break;
    case 1:                    //COAMPS
        r_parameters = wxT("WIND,PRMSL");                                 //the default parameters for this model
        break;
    case 2:                    //RTOFS
        model = 0;                                                        // same resolution range as GFS
        r_parameters = wxT("CUR,WTMP");                                   //the default parameters for this model
    }
    return wxString( 
    wxT("send ") 
    + s[m_pModel->GetCurrentSelection()] 
    + s[5] + m_RequestZoneBase + s[3] 
    + r[model][m_pResolution->GetCurrentSelection()] + s[3] 
    + i[m_pInterval->GetCurrentSelection()] 
    + p[m_pTimeRange->GetCurrentSelection()] 
    + s[4] + r_parameters
    );
}

void GribRequestSetting::OnSendMaiL( wxCommandEvent& event  )
{
    wxMailMessage *message = new wxMailMessage( 
    wxT("Grib-Request"),
    m_MailAdressBase,               //to ( saildoc request adresse )
    WriteMail()                     //message
    );
    wxEmail mail ;
    m_MailImage->SetForegroundColour(wxColor( 255, 0, 0 ));
    if(mail.Send( *message ) ) {
#ifdef __WXMSW__
        m_MailImage->SetLabel(
            _("Your request is ready. An eMail is prepared in your eMail environment. \nYou have just to click 'send' to send it...\nSave or Cancel to finish...or new parameters for a new eMail ...") );
#else
        m_MailImage->SetLabel(
            _("Your request was sent \n(if your system has an MTA configured and is able to send eMail).\nSave or Cancel to finish...or new parameters for a new eMail ..."));
#endif
    } else {
        m_MailImage->SetLabel(
            _("Request can't be sent. Please verify your eMail systeme parameters.\nYou should also have a look at your log file.\nSave or Cancel to finish..."));
    }
    m_bSend->Hide();
    Fit();
}
