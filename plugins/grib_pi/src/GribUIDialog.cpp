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

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "grib_pi.h"

#include <wx/arrimpl.cpp>

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
//---------------------------------------------------------------------------------------
//          GRIB Selector/Control Dialog Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS ( GRIBUIDialog, GRIBUIDialogBase )

BEGIN_EVENT_TABLE ( GRIBUIDialog, GRIBUIDialogBase )

EVT_CLOSE ( GRIBUIDialog::OnClose )
EVT_MOVE ( GRIBUIDialog::OnMove )
EVT_SIZE ( GRIBUIDialog::OnSize )
EVT_BUTTON(ID_CONFIG, GRIBUIDialog::OnConfig )
EVT_TOGGLEBUTTON(ID_PLAYSTOP, GRIBUIDialog::OnPlayStop )
EVT_TIMER(ID_PLAYSTOP, GRIBUIDialog::OnPlayStopTimer )
EVT_DIRPICKER_CHANGED ( ID_GRIBDIR, GRIBUIDialog::OnFileDirChange )
EVT_SLIDER(ID_TIMELINE, GRIBUIDialog::OnTimeline)
EVT_CHECKBOX(ID_CB_WIND_SPEED, GRIBUIDialog::OnCBAny)
EVT_CHECKBOX(ID_CB_PRESSURE, GRIBUIDialog::OnCBAny)
EVT_CHECKBOX(ID_CB_SIG_WAVE_HEIGHT, GRIBUIDialog::OnCBAny)
EVT_CHECKBOX(ID_CB_SEA_TEMPERATURE, GRIBUIDialog::OnCBAny)
EVT_CHECKBOX(ID_CB_CURRENT_VELOCITY, GRIBUIDialog::OnCBAny)

END_EVENT_TABLE()


GribRecordSet::GribRecordSet()
{
    for(int i=0; i<Idx_COUNT; i++)
        m_GribRecordPtrArray[i] = NULL;
}

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

    m_pTimelineBase = NULL;
    m_pTimelineSet = NULL;

    wxFileConfig *pConf = GetOCPNConfigObject();;

    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/GRIB" ) );

        bool value;
        pConf->Read( _T ( "WindPlot" ), &value, true );
        m_cbWind->SetValue(value);
        pConf->Read( _T ( "WavePlot" ), &value, false );
        m_cbWave->SetValue(value);
        pConf->Read( _T ( "CurrentPlot" ), &value, false );
        m_cbCurrent->SetValue(value);
        pConf->Read( _T ( "PressurePlot" ), &value, true );
        m_cbPressure->SetValue(value);
        pConf->Read( _T ( "SeaTemperaturePlot" ), &value, false );
        m_cbSeaTemperature->SetValue(value);

        pConf->SetPath ( _T ( "/Directories" ) );
        pConf->Read ( _T ( "GRIBDirectory" ), &m_grib_dir );
    }

    m_dirPicker->SetPath(m_grib_dir);

    m_RecordTree_root_id = m_pRecordTree->AddRoot( _T ( "GRIBs" ) );

    PopulateTreeControl();
    m_pRecordTree->Expand( m_RecordTree_root_id );
    m_pRecordTree->SelectItem( m_RecordTree_root_id );

    m_OverlayConfig.Read();

    DimeWindow( this );

    Fit();
    SetMinSize( GetBestSize() );
}


GRIBUIDialog::~GRIBUIDialog()
{
    wxFileConfig *pConf = GetOCPNConfigObject();;

    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/GRIB" ) );

        pConf->Write( _T ( "WindPlot" ), m_cbWind->GetValue());
        pConf->Write( _T ( "WavePlot" ), m_cbWave->GetValue());
        pConf->Write( _T ( "CurrentPlot" ), m_cbCurrent->GetValue());
        pConf->Write( _T ( "PressurePlot" ), m_cbPressure->GetValue());
        pConf->Write( _T ( "SeaTemperaturePlot" ), m_cbSeaTemperature->GetValue());

        pConf->SetPath ( _T ( "/Directories" ) );
        pConf->Write ( _T ( "GRIBDirectory" ), m_grib_dir );
    }

    delete m_pTimelineSet;
}

void GRIBUIDialog::Init()
{
    m_sequence_active = -1;
    m_pTimelineSet = NULL;
    m_pRecordTree = NULL;
}

void GRIBUIDialog::SetCursorLatLon( double lat, double lon )
{
    m_cursor_lon = lon;
    m_cursor_lat = lat;

    UpdateTrackingControls();
}

void GRIBUIDialog::UpdateTrackingControls( void )
{
    m_tcWindSpeed->Clear();
    m_tcWindDirection->Clear();
    m_tcPressure->Clear();
    m_tcWaveHeight->Clear();
    m_tcWaveDirection->Clear();
    m_tcSeaTemperature->Clear();
    m_tcCurrentVelocity->Clear();
    m_tcCurrentDirection->Clear();

    if( m_pTimelineSet ) {

        wxDateTime t = m_pTimelineSet->m_Reference_Time;
        t.MakeFromTimezone( wxDateTime::UTC );
        if( t.IsDST() ) t.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );
        m_stDateTime->SetLabel(t.Format(_T("%Y-%m-%d %H:%M:%S "), wxDateTime::Local) + _T("Local - ") +
                               t.Format(_T("%H:%M:%S "), wxDateTime::UTC) + _T("GMT"));

        GribRecord **RecordArray = m_pTimelineSet->m_GribRecordPtrArray;
        //    Update the wind control
        if( RecordArray[Idx_WIND_VX] && RecordArray[Idx_WIND_VY] ) {
            double vx = RecordArray[Idx_WIND_VX]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
            double vy = RecordArray[Idx_WIND_VY]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
            
            if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                vx = m_OverlayConfig.CalibrateValue(GribOverlayConfig::WIND, vx);
                vy = m_OverlayConfig.CalibrateValue(GribOverlayConfig::WIND, vy);

                double vkn = sqrt( vx * vx + vy * vy );
                double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                if( ang > 360. ) ang -= 360.;
                if( ang < 0. ) ang += 360.;

                wxString t;
                t.Printf( _T("%2d"), (int) vkn );
                m_tcWindSpeed->AppendText( t );

                t.Printf( _T("%03d"), (int) ( ang ) );
                m_tcWindDirection->AppendText( t );
            }
        }

        //    Update the Pressure control
        if( RecordArray[Idx_PRESS] ) {
            double press = RecordArray[Idx_PRESS]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

            if( press != GRIB_NOTDEF ) {
                press = m_OverlayConfig.CalibrateValue(GribOverlayConfig::PRESSURE, press);

                wxString t;
                t.Printf( _T("%2d"), (int) ( press ) );
                m_tcPressure->AppendText( t );
            }
        }

        //    Update the Sig Wave Height
        if( RecordArray[Idx_HTSIGW] ) {
            double height = RecordArray[Idx_HTSIGW]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

            if( height != GRIB_NOTDEF ) {
                height = m_OverlayConfig.CalibrateValue(GribOverlayConfig::WAVE, height);

                wxString t;
                t.Printf( _T("%4.1f"), height );
                m_tcWaveHeight->AppendText( t );
            }
        }

        if( RecordArray[Idx_WVDIR] ) {
            double direction = RecordArray[Idx_WVDIR]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
            if( direction != GRIB_NOTDEF ) {
                wxString t;
                t.Printf( _T("%03d"), (int)direction );
                m_tcWaveDirection->AppendText( t );
            }
        }

#if 0
        //    Update the QuickScat (aka Wind) control
        if( RecordArray[Idx_WINDSCAT_VX] && RecordArray[Idx_WINDSCAT_VY] ) {
            double vx = RecordArray[Idx_WINDSCAT_VX]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
            double vy = RecordArray[Idx_WINDSCAT_VY]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

            if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                vx = m_OverlayConfig.CalibrateValue(GribOverlayConfig::WIND, vx);
                vy = m_OverlayConfig.CalibrateValue(GribOverlayConfig::WIND, vy);

                double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                if( ang > 360. ) ang -= 360.;
                if( ang < 0. ) ang += 360.;

                wxString t;
                t.Printf( _T("%2d"), (int) vkn );
                m_tcWindSpeed->AppendText( t );

                t.Printf( _T("%03d"), (int) ( ang ) );
                m_tcWindDirection->AppendText( t );
            }
        }
#endif

        //    Update the SEATEMP
        if( RecordArray[Idx_SEATEMP] ) {
            double temp = RecordArray[Idx_SEATEMP]->
                getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

            if( temp != GRIB_NOTDEF ) {
                temp = m_OverlayConfig.CalibrateValue(GribOverlayConfig::SEA_TEMPERATURE, temp);
                wxString t;
                t.Printf( _T("%6.2f"), temp );
                m_tcSeaTemperature->AppendText( t );
            }
        }

        //    Update the Current control
        if( RecordArray[Idx_SEACURRENT_VX] && RecordArray[Idx_SEACURRENT_VY] ) {
            double vx = RecordArray[Idx_SEACURRENT_VX]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
            double vy = RecordArray[Idx_SEACURRENT_VY]->
                getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

            if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                vx = m_OverlayConfig.CalibrateValue(GribOverlayConfig::CURRENT, vx);
                vy = m_OverlayConfig.CalibrateValue(GribOverlayConfig::CURRENT, vy);

                double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                if( ang > 360. ) ang -= 360.;
                if( ang < 0. ) ang += 360.;

                wxString t;
                t.Printf( _T("%5.2f"), vkn );
                m_tcCurrentVelocity->AppendText( t );

                t.Printf( _T("%03d"), (int) ( ang ) );
                m_tcCurrentDirection->AppendText( t );
            }
        }
    }
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

void GRIBUIDialog::OnConfig( wxCommandEvent& event )
{
    GRIBConfigDialog *dialog = new GRIBConfigDialog( this );

    dialog->ReadConfig(m_OverlayConfig);

    if(dialog->ShowModal() == wxID_OK)
    {
        dialog->WriteConfig(m_OverlayConfig);
        m_OverlayConfig.Write();

        m_sTimeline->SetMax(m_TimeLineHours*m_OverlayConfig.m_HourDivider);
        if(m_pTimelineSet)
            m_pTimelineSet->ClearCachedData();
        SetFactoryOptions();
    }
}

void GRIBUIDialog::OnPlayStop( wxCommandEvent& event )
{
    if(m_tbPlayStop->GetValue()) {
        m_tbPlayStop->SetLabel(_("Stop"));
        m_tPlayStop.SetOwner( this, ID_PLAYSTOP );
        m_tPlayStop.Start( 1000/m_OverlayConfig.m_UpdatesPerSecond, wxTIMER_CONTINUOUS );
    } else
        m_tbPlayStop->SetLabel(_("Play"));
}

void GRIBUIDialog::OnPlayStopTimer( wxTimerEvent & )
{
    if(!m_tbPlayStop->GetValue())
        m_tPlayStop.Stop();
    else if(m_sTimeline->GetValue() >= m_sTimeline->GetMax()) {
        if(m_OverlayConfig.m_bLoopMode) {
            m_sTimeline->SetValue(0);
            TimelineChanged();
        } else
            m_tPlayStop.Stop();
    } else {
        m_sTimeline->SetValue(m_sTimeline->GetValue() + m_OverlayConfig.m_SlicesPerUpdate);
        TimelineChanged();
    }
}

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

void GRIBUIDialog::TimelineChanged()
{
    if(!m_pTimelineBase)
        return;

    double nhour = m_sTimeline->GetValue()/m_OverlayConfig.m_HourDivider;
    
    GribRecordSet &first=m_pTimelineBase->Item(0);
    wxDateTime firsttime = first.m_Reference_Time, curtime;
    unsigned int i;
    for(i=0; i<m_pTimelineBase->GetCount()-1; i++) {
        GribRecordSet &cur=m_pTimelineBase->Item(i+1);
        curtime = cur.m_Reference_Time;
        if((curtime - firsttime).GetHours() >= nhour)
            break;
    }
    
    double hour2 = (curtime - firsttime).GetHours();
    curtime = m_pTimelineBase->Item(i).m_Reference_Time;
    double hour1 = (curtime - firsttime).GetHours();
    
    if(hour2<=hour1 || nhour < hour1)
        return;

    double interp_const = (nhour-hour1) / (hour2-hour1);
    
    if(!m_OverlayConfig.m_bInterpolate)
        interp_const = round(interp_const);

    GribRecordSet &GRS1 = m_pTimelineBase->Item(i), &GRS2 = m_pTimelineBase->Item(i+1);
    SetGribTimelineRecordSet(new GribTimelineRecordSet(GRS1, GRS2, interp_const));
}

void GRIBUIDialog::OnTimeline( wxCommandEvent& event )
{
    TimelineChanged();
}

void GRIBUIDialog::OnCBAny( wxCommandEvent& event )
{
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::PopulateTreeControl()
{
    wxString currentGribDir = m_dirPicker->GetPath();
    if( !wxDir::Exists( currentGribDir ) ) return;

    //    Get an array of GRIB file names in the target directory, not descending into subdirs
    wxArrayString file_array;

    m_n_files = wxDir::GetAllFiles( currentGribDir, &file_array, _T ( "*.grb" ), wxDIR_FILES );
    m_n_files += wxDir::GetAllFiles( currentGribDir, &file_array, _T ( "*.grb.bz2" ), wxDIR_FILES );

    //    Sort the files by File Modification Date
    file_array.Sort( CompareFileStringTime );

    //    Add the files to the tree at the root
    m_pRecordTree->m_file_id_array = new wxTreeItemId[m_n_files];

    for( int i = 0; i < m_n_files; i++ ) {
        GribTreeItemData *pmtid = new GribTreeItemData( GRIB_FILE_TYPE );
        pmtid->m_file_name = file_array[i];
        pmtid->m_file_index = i;

        wxFileName fn( file_array[i] );
        m_pRecordTree->m_file_id_array[i] = m_pRecordTree->AppendItem( m_RecordTree_root_id,
                fn.GetFullName(), -1, -1, pmtid );
//      m_pRecordTree->SetItemTextColour(m_pRecordTree->m_file_id_array[i], GetGlobalColor ( _T ( "UBLCK")));
    }

    //    Will this be too slow?
    //    Parse and show at most "n" files, maybe move to config?
    int n_parse = wxMin(5, m_n_files);

    for( int i = 0; i < n_parse; i++ ) {
        GribTreeItemData *pdata = (GribTreeItemData *) m_pRecordTree->GetItemData(
                m_pRecordTree->m_file_id_array[i] );

        //    Create and ingest the GRIB file object if needed
        if( NULL == pdata->m_pGribFile ) {
            GRIBFile *pgribfile = new GRIBFile( pdata->m_file_name );
            if( pgribfile ) {
                if( pgribfile->IsOK() ) {
                    pdata->m_pGribFile = pgribfile;
                    PopulateTreeControlGRS( pgribfile, pdata->m_file_index );
                }
            }
        }
    }

    //    No GRS is selected on first building the tree
    SetGribTimelineRecordSet( NULL );
}

void GRIBUIDialog::SelectTreeControlGRS( GRIBFile *pgribfile )
{
    ArrayOfGribRecordSets *rsa = pgribfile->GetRecordSetArrayPtr();
    if(rsa->GetCount() < 2)
        return;

    GribRecordSet &first=rsa->Item(0), &last = rsa->Item(rsa->GetCount()-1);

    wxTimeSpan span = wxDateTime(last.m_Reference_Time) - wxDateTime(first.m_Reference_Time);
    m_TimeLineHours = span.GetHours();

    m_pTimelineBase = rsa;
    m_sTimeline->SetMax(m_TimeLineHours*m_OverlayConfig.m_HourDivider);
    m_sTimeline->Enable();
}

void GRIBUIDialog::PopulateTreeControlGRS( GRIBFile *pgribfile, int file_index )
{
    //    Get the array of GribRecordSets, and add one-by-one to the tree,
    //    each under the proper file item.
    ArrayOfGribRecordSets *rsa = pgribfile->GetRecordSetArrayPtr();

    for( unsigned int i = 0; i < rsa->GetCount(); i++ ) {
        GribTreeItemData *pmtid = new GribTreeItemData( GRIB_RECORD_SET_TYPE );
        pmtid->m_pGribFile = pgribfile;
        pmtid->m_pGribRecordSet = &rsa->Item( i );

        wxDateTime t( rsa->Item( i ).m_Reference_Time );
        t.MakeFromTimezone( wxDateTime::UTC );
        if( t.IsDST() ) t.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );

        // This is a hack because Windows is broke....
        wxString time_string = t.Format( _T("%a %d-%b-%Y %H:%M:%S "), wxDateTime::Local );
        time_string.Append( _T("Local - ") );
        time_string.Append( t.Format( _T("%a %d-%b-%Y %H:%M:%S "), wxDateTime::UTC ) );
        time_string.Append( _T("GMT") );

        m_pRecordTree->AppendItem( m_pRecordTree->m_file_id_array[file_index],
                                   time_string, -1, -1, pmtid );
    }
}

void GRIBUIDialog::SelectGribRecordSet( GribRecordSet *pGribRecordSet )
{
    ArrayOfGribRecordSets *rsa = m_pTimelineBase;

    if(!rsa)
        return;

    GribRecordSet &first=rsa->Item(0);
    wxDateTime firsttime = first.m_Reference_Time, curtime = pGribRecordSet->m_Reference_Time;
    double hour = (curtime - firsttime).GetHours();

    m_sTimeline->SetValue(hour*m_OverlayConfig.m_HourDivider);
    TimelineChanged();
}

void GRIBUIDialog::SetGribTimelineRecordSet(GribTimelineRecordSet *pTimelineSet)
{
    delete m_pTimelineSet;
    m_pTimelineSet = pTimelineSet;

    if(!pPlugIn->GetGRIBOverlayFactory())
        return;

    //    Give the overlay factory the GribRecordSet
    pPlugIn->GetGRIBOverlayFactory()->SetGribTimelineRecordSet(m_pTimelineSet);

    SetFactoryOptions();
    RequestRefresh( pParent );
    UpdateTrackingControls();
}

void GRIBUIDialog::SetFactoryOptions()
{
    pPlugIn->GetGRIBOverlayFactory()->ClearCachedData();

    RequestRefresh( pParent );
}

//----------------------------------------------------------------------------------------------------------
//          GRIBFile Object Implementation
//----------------------------------------------------------------------------------------------------------

GRIBFile::GRIBFile( const wxString file_name )
{
    m_bOK = true;           // Assume ok until proven otherwise

    if( !::wxFileExists( file_name ) ) {
        m_last_error_message = _T ( "   GRIBFile Error:  File does not exist." );
        m_bOK = false;
        return;
    }

    //    Use the zyGrib support classes, as (slightly) modified locally....

    m_pGribReader = new GribReader();

    //    Read and ingest the entire GRIB file.......
    m_pGribReader->openFile( file_name );

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
                    case GRB_PRESSURE: idx = Idx_PRESS;   break;
                    case GRB_HTSGW:    idx = Idx_HTSIGW;  break;
                    case GRB_WVDIR:    idx = Idx_WVDIR;   break;
                    case GRB_USCT:     idx = Idx_WINDSCAT_VX; break;
                    case GRB_VSCT:     idx = Idx_WINDSCAT_VY; break;
//                    case GRB_TEMP:     idx = Idx_SEATEMP break;  // GFS SEATMP
                    case GRB_WTMP:     idx = Idx_SEATEMP; break;
                    case GRB_UOGRD:    idx = Idx_SEACURRENT_VX; break;
                    case GRB_VOGRD:    idx = Idx_SEACURRENT_VY; break;
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
