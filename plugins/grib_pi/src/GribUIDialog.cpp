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
#include "GribTable.h"
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

enum SettingsDisplay {B_ARROWS, ISO_LINE, D_ARROWS, OVERLAY, NUMBERS, PARTICLES};

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
        m_GribRecordPtrArray[i] = NULL;
        m_IsobarArray[i] = NULL;
    }

    for(int i=0; i<Idx_COUNT; i++) {
        if(m_GribRecordPtrArray[i])
            continue;

        GribRecord *GR1 = GRS1.m_GribRecordPtrArray[i];
        GribRecord *GR2 = GRS2.m_GribRecordPtrArray[i];

        if(!GR1 || !GR2)
            continue;

        /* if this is a vector interpolation use the 2d method */
        if(i < Idx_WIND_VY) {
            GribRecord *GR1y = GRS1.m_GribRecordPtrArray[i + Idx_WIND_VY];
            GribRecord *GR2y = GRS2.m_GribRecordPtrArray[i + Idx_WIND_VY];
            if(GR1y && GR2y) {
                m_GribRecordPtrArray[i] = GribRecord::Interpolated2DRecord
                    (m_GribRecordPtrArray[i + Idx_WIND_VY], *GR1, *GR1y, *GR2, *GR2y, interp_const);
                continue;
            }
        } else if(i <= Idx_WIND_VY300)
            continue;
        else if(i == Idx_SEACURRENT_VX) {
            GribRecord *GR1y = GRS1.m_GribRecordPtrArray[Idx_SEACURRENT_VY];
            GribRecord *GR2y = GRS2.m_GribRecordPtrArray[Idx_SEACURRENT_VY];
            if(GR1y && GR2y) {
                m_GribRecordPtrArray[i] = GribRecord::Interpolated2DRecord
                    (m_GribRecordPtrArray[Idx_SEACURRENT_VY], *GR1, *GR1y, *GR2, *GR2y, interp_const);
                continue;
            }
        } else if(i == Idx_SEACURRENT_VY)
            continue;

        m_GribRecordPtrArray[i] = GribRecord::InterpolatedRecord(*GR1, *GR2, interp_const);
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
    m_bpPlay->SetBitmap(wxBitmap( play ));
    m_bpPlay->SetToolTip(_("Play"));
    m_cRecordForecast->Clear();
    m_cbAltitude->Clear();
    pPlugIn->GetGRIBOverlayFactory()->SetAltitude( 0 );
    m_FileIntervalIndex = m_OverlaySettings.m_SlicesPerUpdate;
    delete m_bGRIBActiveFile;
    m_pTimelineSet = NULL;
    m_sTimeline->SetValue(0);
    m_TimeLineHours = 0;
    m_InterpolateMode = false;
    m_pNowMode = false;
    m_SelectionIsSaved = false;

    //get more recent file in default directory if necessary
    wxFileName f( m_file_name );
    if( newestFile || f.GetFullName().IsEmpty() ) m_file_name = GetNewestFileInDirectory();

    m_bGRIBActiveFile = new GRIBFile( m_file_name,
                                      pPlugIn->GetCopyFirstCumRec(),
                                      pPlugIn->GetCopyMissWaveRec() );

    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();

    wxFileName fn( m_file_name );
    wxString title( fn.GetFullName() );

    if( m_bGRIBActiveFile ) {
        if( m_bGRIBActiveFile->IsOK() ) {
            if( rsa->GetCount() == 0 ) {                        //valid but empty file
                m_bGRIBActiveFile = NULL;
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( _("Error:  No valid data in this file!") );

            } else {
                PopulateComboDataList();
                title.append( _T("(") + TToString( m_bGRIBActiveFile->GetRefDateTime(), pPlugIn->GetTimeZone()) + _T(")"));

                if( rsa->GetCount() > 1 ) {
                    GribRecordSet &first=rsa->Item(0), &second = rsa->Item(1), &last = rsa->Item(rsa->GetCount()-1);

                    //compute ntotal time span
                    wxTimeSpan span = wxDateTime(last.m_Reference_Time) - wxDateTime(first.m_Reference_Time);
                    m_TimeLineHours = span.GetHours();

                    //get file interval index and update intervale choice if necessary
                    int halfintermin(wxTimeSpan(wxDateTime(second.m_Reference_Time) - wxDateTime(first.m_Reference_Time)).GetMinutes() / 2);
                    for( m_FileIntervalIndex = 0;; m_FileIntervalIndex++){
                        if(m_OverlaySettings.GetMinFromIndex(m_FileIntervalIndex) > halfintermin) break;
                    }
                    m_FileIntervalIndex--;
                    if(m_OverlaySettings.m_SlicesPerUpdate > m_FileIntervalIndex) m_OverlaySettings.m_SlicesPerUpdate = m_FileIntervalIndex;
                }
            }
        } else {
            if( fn.IsDir() ) {
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( _("Warning:  Empty directory!") );
            }
            else
                pPlugIn->GetGRIBOverlayFactory()->SetMessage( m_bGRIBActiveFile->GetLastMessage() );
        }
        this->SetTitle(title);
        SetTimeLineMax(false);
        SetFactoryOptions();
        if( pPlugIn->GetStartOptions() && m_TimeLineHours != 0)                             //fix a crash for one date files
            ComputeBestForecastForNow();
        else
            TimelineChanged();

        PopulateTrackingControls();
    }
    SetCanvasContextMenuItemViz( pPlugIn->m_MenuItem, m_TimeLineHours != 0);
}

bool GRIBUIDialog::GetGribZoneLimits(GribTimelineRecordSet *timelineSet, double *latmin, double *latmax, double *lonmin, double *lonmax)
{
    //calculate the largest overlay size
    GribRecord **pGR = timelineSet->m_GribRecordPtrArray;
    double ltmi = -GRIB_NOTDEF, ltma = GRIB_NOTDEF, lnmi = -GRIB_NOTDEF, lnma = GRIB_NOTDEF;
    for( int i = 0; i<36; i++){
        GribRecord *pGRA = pGR[i];
        if(!pGRA) continue;
        if(pGRA->getLatMin() < ltmi) ltmi = pGRA->getLatMin();
        if(pGRA->getLatMax() > ltma) ltma = pGRA->getLatMax();
        if(pGRA->getLonMin() < lnmi) lnmi = pGRA->getLonMin();
        if(pGRA->getLonMax() > lnma) lnma = pGRA->getLonMax();
    }
    if( ltmi == -GRIB_NOTDEF || lnmi == -GRIB_NOTDEF ||
        ltma ==  GRIB_NOTDEF || lnma ==  GRIB_NOTDEF)
        return false;

    if(latmin) *latmin = ltmi;
    if(latmax) *latmax = ltma;
    if(lonmin) *lonmin = lnmi;
    if(lonmax) *lonmax = lnma;
    return true;
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

    pReq_Dialog = NULL;
    m_bGRIBActiveFile = NULL;
    m_pTimelineSet = NULL;

    wxFileConfig *pConf = GetOCPNConfigObject();

    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/GRIB" ) );

        bool value;
        pConf->Read( _T ( "WindPlot" ), &value, true );
        m_cbWind->SetValue(value);
        pConf->Read( _T ( "WindGustPlot" ), &value, false );
        m_cbWindGust->SetValue(value);
        pConf->Read( _T ( "PressurePlot" ), &value, false );
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
    //set checkboxes ID to have a formal link to data type
    m_cbWind->SetId( GribOverlaySettings::WIND );
    m_cbWindGust->SetId( GribOverlaySettings::WIND_GUST );
    m_cbPressure->SetId( GribOverlaySettings::PRESSURE );
    m_cbCurrent->SetId( GribOverlaySettings::CURRENT );
    m_cbWave->SetId( GribOverlaySettings::WAVE );
    m_cbPrecipitation->SetId( GribOverlaySettings::PRECIPITATION );
    m_cbCloud->SetId( GribOverlaySettings::CLOUD );
    m_cbAirTemperature->SetId( GribOverlaySettings::AIR_TEMPERATURE );
    m_cbSeaTemperature->SetId( GribOverlaySettings::SEA_TEMPERATURE );
    m_cbCAPE->SetId( GribOverlaySettings::CAPE );

    //set buttons bitmap
    m_bpPrev->SetBitmap(wxBitmap( prev ));
    m_bpNext->SetBitmap(wxBitmap( next ));
    m_bpNow->SetBitmap(wxBitmap( now ));
    m_bpZoomToCenter->SetBitmap(wxBitmap( zoomto ));
    m_bpPlay->SetBitmap(wxBitmap( play ));
    m_bpOpenFile->SetBitmap(wxBitmap( openfile ));
    m_bpSettings->SetBitmap(wxBitmap( setting ));
    m_bpRequest->SetBitmap(wxBitmap( request ));

    //connect events have not been done in dialog base
    this->Connect( wxEVT_MOVE, wxMoveEventHandler( GRIBUIDialog::OnMove ) );
    m_tPlayStop.Connect(wxEVT_TIMER, wxTimerEventHandler( GRIBUIDialog::OnPlayStopTimer ), NULL, this);
    m_tCursorTrackTimer.Connect(wxEVT_TIMER, wxTimerEventHandler( GRIBUIDialog::OnCursorTrackTimer ), NULL, this);

    m_OverlaySettings.Read();

    DimeWindow( this );

    m_pTimelineSet = NULL;
    m_fgTrackingDisplay->Show(2,false);         //Hide extra altitude data parameters
    //is there a bug in wxWigget? m_fgTrackingControls->Clear() delete m_fcAltitude so it's necessary to detach it before
    m_fgTrackingControls->Detach(m_fcAltitude);
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
    if(!m_tCursorTrackTimer.IsRunning()) m_tCursorTrackTimer.Start(50, wxTIMER_ONE_SHOT );
    m_cursor_lon = lon;
    m_cursor_lat = lat;
}

void GRIBUIDialog::OnCursorTrackTimer( wxTimerEvent & event)
{
    UpdateTrackingControls();
}

void GRIBUIDialog::OnMouseEvent( wxMouseEvent& event )
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
                m_OverlaySettings.Settings[id].m_bBarbedArrows = it->IsChecked();
                break;
            case ISO_LINE:
                m_OverlaySettings.Settings[id].m_bIsoBars = it->IsChecked();
                break;
            case D_ARROWS:
                m_OverlaySettings.Settings[id].m_bDirectionArrows = it->IsChecked();
                break;
            case OVERLAY:
                m_OverlaySettings.Settings[id].m_bOverlayMap = it->IsChecked();
                break;
            case NUMBERS:
                m_OverlaySettings.Settings[id].m_bNumbers = it->IsChecked();
                break;
            case PARTICLES:
                m_OverlaySettings.Settings[id].m_bParticles = it->IsChecked();
        }
        node = node->GetNext();
    }

    //if the current box is checked then resolve display conflicts
    wxWindowListNode *wnode =  this->GetChildren().GetFirst();
    while( wnode ) {
        wxWindow *win = wnode->GetData();
        if( id == win->GetId() ) {
            if( ((wxCheckBox*) win )->IsChecked() )
                ResolveDisplayConflicts( this, id );
            break;
        }
        wnode = wnode->GetNext();
    }

    //save new config
    m_OverlaySettings.Write();

    delete menu;
}

void GRIBUIDialog::MenuAppend( wxMenu *menu, int id, wxString label, int setting)
{
    wxMenuItem *item = new wxMenuItem(menu, id, label, _T(""), wxITEM_CHECK);

#ifdef __WXMSW__
    wxFont *qFont = OCPNGetFont( _("Menu"), 10 );
    item->SetFont(*qFont);
#endif

    menu->Append(item);

    bool check;
    if( id == B_ARROWS )
        check = m_OverlaySettings.Settings[setting].m_bBarbedArrows;
    else if( id == ISO_LINE )
        check = m_OverlaySettings.Settings[setting].m_bIsoBars;
    else if( id == D_ARROWS )
        check = m_OverlaySettings.Settings[setting].m_bDirectionArrows;
    else if( id == OVERLAY )
        check = m_OverlaySettings.Settings[setting].m_bOverlayMap;
    else if( id == NUMBERS )
        check = m_OverlaySettings.Settings[setting].m_bNumbers;
    else if( id == PARTICLES )
        check = m_OverlaySettings.Settings[setting].m_bParticles;
    else
        check = false;
    item->Check( check );
}

void GRIBUIDialog::ContextMenuItemCallback(int id)
{
     wxFileConfig *pConf = GetOCPNConfigObject();

     int x,y,w,h;

     if(pConf) {
        pConf->SetPath ( _T ( "/Settings/GRIB" ) );

        pConf->Read( _T ( "GribDataTablePosition_x" ), &x, -1 );
        pConf->Read( _T ( "GribDataTablePosition_y" ), &y, -1 );
        pConf->Read( _T ( "GribDataTableWidth" ), &w, 900 );
        pConf->Read( _T ( "GribDataTableHeight" ), &h, 350 );
     }
     //init centered position and default size if not set yet
     if(x==-1 && y == -1) { x = (m_vp->pix_width - w) / 2; y = (m_vp->pix_height - h) /2; }

     ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
     GRIBTable *table = new GRIBTable(*this);

     table->InitGribTable( pPlugIn->GetTimeZone(), rsa );
     table->m_pButtonTableOK->SetLabel(_("Close"));

     //set dialog size and position
     table->SetSize(w, h);
     table->SetPosition(wxPoint(x, y));
     table->ShowModal();
}

void GRIBUIDialog::SetViewPort( PlugIn_ViewPort *vp )
{
    if(m_vp == vp)  return;

    m_vp = new PlugIn_ViewPort(*vp);

    if(pReq_Dialog)
        if(pReq_Dialog->IsShown()) pReq_Dialog->OnVpChange(vp);
}

void GRIBUIDialog::AddTrackingControl( wxControl *ctrl1,  wxControl *ctrl2,  wxControl *ctrl3, bool show,
        int wctrl2, int wctrl3, bool altitude )
{
    if(show) {
        if( altitude ){
            m_fcAltitude->Add(ctrl1, 0, wxALL, 1);
            ctrl1->Show();
            m_cbAltitude->Show();
            m_fgTrackingControls->Add( m_fcAltitude, 1, wxRIGHT|wxBOTTOM, 3 );
        } else {
        m_fgTrackingControls->Add(ctrl1, 0, wxALL, 1);
        ctrl1->Show();
        }
        if(ctrl2) {
            m_fgTrackingControls->Add(ctrl2, 0, wxALL, 1);
            ctrl2->SetMinSize(wxSize(wctrl2, -1));
            ctrl2->Show();
        } else
              m_fgTrackingControls->Add(0, 0, 1, wxEXPAND, 1); /* spacer */

        if(ctrl3) {
            m_fgTrackingControls->Add(ctrl3, 0, wxALL, 1);
            ctrl3->SetMinSize(wxSize(wctrl3, -1));
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

void GRIBUIDialog::ResolveDisplayConflicts( wxWindow *window, int enventId )
{
    //allow multi selection only if there is no display type superposition
    int winId;
    wxWindowListNode *node =  window->GetChildren().GetFirst();
    while( node ) {
        wxWindow *win = node->GetData();
        if( win->IsKindOf(CLASSINFO(wxCheckBox)) && ((wxCheckBox*) win )->IsChecked() ) {
            winId = win->GetId();
            if( enventId != winId ) {
                if( (m_OverlaySettings.Settings[enventId].m_bBarbedArrows &&
                        m_OverlaySettings.Settings[winId].m_bBarbedArrows)
                        || (m_OverlaySettings.Settings[enventId].m_bDirectionArrows &&
                        m_OverlaySettings.Settings[winId].m_bDirectionArrows)
                        || (m_OverlaySettings.Settings[enventId].m_bIsoBars &&
                        m_OverlaySettings.Settings[winId].m_bIsoBars)
                        || (m_OverlaySettings.Settings[enventId].m_bNumbers &&
                        m_OverlaySettings.Settings[winId].m_bNumbers)
                        || (m_OverlaySettings.Settings[enventId].m_bOverlayMap &&
                        m_OverlaySettings.Settings[winId].m_bOverlayMap)
                        || (m_OverlaySettings.Settings[enventId].m_bParticles &&
                        m_OverlaySettings.Settings[winId].m_bParticles) )
                    ((wxCheckBox*) win )->SetValue(false);
            }
        }
        node = node->GetNext();
    }
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::PopulateTrackingControls( bool Populate_Altitude )
{
    //fix crash with curious files with no record
    m_bpSettings->Enable(m_pTimelineSet != NULL);
    m_bpZoomToCenter->Enable(m_pTimelineSet != NULL);

    m_sTimeline->Show(m_pTimelineSet != NULL && m_TimeLineHours);
    m_bpPlay->Show(m_pTimelineSet != NULL && m_TimeLineHours);

    m_bpPrev->Enable(m_pTimelineSet != NULL && m_TimeLineHours);
    m_bpNext->Enable(m_pTimelineSet != NULL && m_TimeLineHours);
    m_bpNow->Enable(m_pTimelineSet != NULL && m_TimeLineHours);

    //is there a bug in wxWigget? flexsizer->Clear() delete child if it's a flexsizer so it's necessary to detach it before
    if(m_fgTrackingControls->GetItem(m_fcAltitude) != NULL) m_fgTrackingControls->Detach(m_fcAltitude);
    m_fgTrackingControls->Clear();
    m_fgTrackingControls->SetCols(9);
    if(m_fcAltitude->GetItem(m_cbWind) != NULL) m_fcAltitude->Detach(m_cbWind);
    m_cbAltitude->Hide();
    this->Fit();

    //populate and set altitude choice if necessary
    if( Populate_Altitude ) {
        int selection = m_cbAltitude->GetCurrentSelection() < 1 ? 0 : m_cbAltitude->GetCurrentSelection();
        m_cbAltitude->Clear();
        for( int i = 0; i<5; i++) {
            if( (( m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VX + i) != wxNOT_FOUND
                && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VY + i) != wxNOT_FOUND )) || i == 0 )
                    m_cbAltitude->Append(m_OverlaySettings.GetAltitudeFromIndex( i , m_OverlaySettings.Settings[GribOverlaySettings::PRESSURE].m_Units));
        }
        m_cbAltitude->SetSelection( selection );
    }

    //Get text controls sizing data
    wxFont *font = OCPNGetFont(_("Dialog"), 10);
    int wn, ws, wd, wl;
    GetTextExtent( _T("abcdefghih"), &wn, NULL, 0, 0, font); // normal width text control size
    GetTextExtent( _T("abcdef"), &ws, NULL, 0, 0, font); // short width text control size for direction only
    GetTextExtent( _T("abcdefghijklmopq"), &wd, NULL, 0, 0, font); // long width text control size for double unit wind display
    GetTextExtent( _T("abcdefghijklm"), &wl, NULL, 0, 0, font); // long width text control size for double unit wave display
    //

    int w1, w2;
     m_OverlaySettings.Settings[GribOverlaySettings::WIND].m_Units == GribOverlaySettings::BFS ? w1 = wn, w2 = 0 : w1 = wd, w2 = ws;
    AddTrackingControl(m_cbWind, m_tcWindSpeed, m_tcWindDirection,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VX) != wxNOT_FOUND
        && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VY) != wxNOT_FOUND, w1, w2,
        m_cbAltitude->GetCount() > 1 );
    AddTrackingControl(m_cbWindGust, m_tcWindGust, 0, m_pTimelineSet
        && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_GUST) != wxNOT_FOUND, wn);
    AddTrackingControl(m_cbPressure, m_tcPressure, 0, m_pTimelineSet
        && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRESSURE) != wxNOT_FOUND, wn);

    /* tracking for wave is funky */
    if(m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_HTSIGW) != wxNOT_FOUND) {
        if(m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVDIR) != wxNOT_FOUND)
            AddTrackingControl(m_cbWave, m_tcWaveHeight, m_tcWaveDirection, true, wl, ws);
        else {
            AddTrackingControl(m_cbWave, m_tcWaveHeight, 0, true, wn);
            m_tcWaveDirection->Hide();
        }
    } else
        if(m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVDIR) != wxNOT_FOUND) {
            AddTrackingControl(m_cbWave, 0, m_tcWaveDirection, true, 0, ws);
            m_tcWaveHeight->Hide();
        } else
            AddTrackingControl(m_cbWave, m_tcWaveHeight, m_tcWaveDirection, false, 0);

    AddTrackingControl(m_cbCurrent, m_tcCurrentVelocity, m_tcCurrentDirection,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEACURRENT_VX) != wxNOT_FOUND
        && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEACURRENT_VY) != wxNOT_FOUND, wn, ws);
    AddTrackingControl(m_cbPrecipitation, m_tcPrecipitation, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRECIP_TOT) != wxNOT_FOUND, wn);
    AddTrackingControl(m_cbCloud, m_tcCloud, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CLOUD_TOT) != wxNOT_FOUND, wn);
    AddTrackingControl(m_cbAirTemperature, m_tcAirTemperature, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_AIR_TEMP) != wxNOT_FOUND, wn);
    AddTrackingControl(m_cbSeaTemperature, m_tcSeaTemperature, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEA_TEMP) != wxNOT_FOUND, wn);
    AddTrackingControl(m_cbCAPE, m_tcCAPE, 0,
        m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CAPE) != wxNOT_FOUND, wn);
    //
    //init and show extra parameters for altitude tracking if necessary
    if( pPlugIn->GetGRIBOverlayFactory()->m_Altitude ) {
        if( (m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_AIR_TEMP
                + pPlugIn->GetGRIBOverlayFactory()->m_Altitude) != wxNOT_FOUND)
            || (m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_HUMID_RE
                + pPlugIn->GetGRIBOverlayFactory()->m_Altitude) != wxNOT_FOUND)
            || (m_pTimelineSet && m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_GEOP_HGT
                + pPlugIn->GetGRIBOverlayFactory()->m_Altitude) != wxNOT_FOUND) ) {
                    m_fgTrackingDisplay->Show(2, true);
                    m_tcAltitude->SetMinSize( wxSize( wn, -1 ));
                    m_tcAltitude->SetValue( _("N/A") );
                    m_tcTemp->SetMinSize( wxSize( wn, -1 ));
                    m_tcTemp->SetValue( _("N/A") );
                    m_tcRelHumid->SetMinSize( wxSize( wn, -1 ));
                    m_tcRelHumid->SetValue( _("N/A") );
        }

        m_stAltitudeText->SetLabel( wxString(_("Data at")).Append(_T(" ")).Append(m_OverlaySettings.GetAltitudeFromIndex(
            pPlugIn->GetGRIBOverlayFactory()->m_Altitude, m_OverlaySettings.Settings[GribOverlaySettings::PRESSURE].m_Units))
            .Append( _T(" ") ).Append( m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRESSURE) ) );
    } else
        m_fgTrackingDisplay->Show(2,false);

    //add tooltips
    wxString t; double lev;

    lev = m_OverlaySettings.CalibrateValue(GribOverlaySettings::GEO_ALTITUDE, 10 );     //convert 10m in current altitude unit
    t.Printf(
        pPlugIn->GetGRIBOverlayFactory()->m_Altitude ? m_OverlaySettings.GetAltitudeFromIndex(
        pPlugIn->GetGRIBOverlayFactory()->m_Altitude, m_OverlaySettings.Settings[GribOverlaySettings::PRESSURE].m_Units)
        .Append( _T(" ") ).Append( m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRESSURE) )
        : wxString::Format( _T("%1.*f "), lev == (int) lev ? 0 : 1, lev ).Append( m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::GEO_ALTITUDE) )
        );
    m_tcWindSpeed->SetToolTip( _("Wind Speed at") + t.Prepend( _T(" ") ) );
    m_tcWindDirection->SetToolTip( _("Wind Direction at") + t );

    t.Printf( _T(" %1.*f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::GEO_ALTITUDE), lev == (int) lev ? 0 : 1, lev );
    m_tcWindGust->SetToolTip( _("Wind Gust at") + t );

    if( m_pTimelineSet ) {
        wxString s[] = { _T(" "), _("Air Temperature at"), _("Surface level"), _("Sea Surface Temperature") };

        lev = m_OverlaySettings.CalibrateValue(GribOverlaySettings::GEO_ALTITUDE, 2 );      //convert 2m in current altitude unit
        t.Printf( m_bGRIBActiveFile->m_GribIdxArray.Index(1000 + NORWAY_METNO) != wxNOT_FOUND ? s[0] + s[2]
            : _T(" %1.*f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::GEO_ALTITUDE), lev == (int) lev ? 0 : 1, lev );
        m_tcAirTemperature->SetToolTip(s[1] + t );

        m_tcSeaTemperature->SetToolTip( m_bGRIBActiveFile->m_GribIdxArray.Index( 1000 + NOAA_GFS) != wxNOT_FOUND ? s[1] + s[0] + s[2] : s[3] );
    }

    m_cbAltitude->SetToolTip( wxString::Format( _("Select Geopotential Height (in %s) or Standard Altitude."),
        m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRESSURE).c_str() ) );

    SetDataBackGroundColor();

    Fit();
    Refresh();
}

void GRIBUIDialog::SetDataBackGroundColor()
{
    wxColour bgc;
    GetGlobalColor( pPlugIn->GetGRIBOverlayFactory()->m_Altitude ? _T("YELO1") : _T("DILG0"), &bgc );
    m_tcWindSpeed->SetBackgroundColour(bgc);
    m_tcWindDirection->SetBackgroundColour(bgc);
    m_tcAltitude->SetBackgroundColour(bgc);
    m_tcTemp->SetBackgroundColour(bgc);
    m_tcRelHumid->SetBackgroundColour(bgc);
}

void GRIBUIDialog::UpdateTrackingControls( void )
{
    if( !m_pTimelineSet )
        return;

    GribRecord **RecordArray = m_pTimelineSet->m_GribRecordPtrArray;
    //    Update the wind control
    int altitude = pPlugIn->GetGRIBOverlayFactory()->m_Altitude;
    double vkn, ang;
    if(GribRecord::getInterpolatedValues(vkn, ang,
                                         RecordArray[Idx_WIND_VX + altitude],
                                         RecordArray[Idx_WIND_VY + altitude],
                                         m_cursor_lon, m_cursor_lat)) {
        double vk = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND, vkn);
        m_tcWindSpeed->SetValue( wxString::Format( _T("%3d ") + m_OverlaySettings.GetUnitSymbol
                                                   (GribOverlaySettings::WIND) , (int)round( vk )) );

        //wind is a special case: if current unit is not bf ==> double speed display (current unit + bf)
        if(m_OverlaySettings.Settings[GribOverlaySettings::WIND].m_Units != GribOverlaySettings::BFS) {
            vk = m_OverlaySettings.GetmstobfFactor(vkn)* vkn;
            m_tcWindSpeed->SetValue(m_tcWindSpeed->GetValue().Append(_T(" - ")).
                                    Append(wxString::Format( _T("%2d bf"), (int)round( vk ))));
        }

        m_tcWindDirection->SetValue( wxString::Format( _T("%03d\u00B0"), (int) ( ang ) ));
    } else {
        m_tcWindSpeed->SetValue( _("N/A") );
        m_tcWindDirection->SetValue(  _("N/A") );
    }

    //    Update the Wind gusts control
    if( RecordArray[Idx_WIND_GUST] ) {
        double vkn = RecordArray[Idx_WIND_GUST]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( vkn != GRIB_NOTDEF ) {
            vkn = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND_GUST, vkn);
            m_tcWindGust->SetValue( wxString::Format(_T("%2d ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WIND_GUST), (int)round( vkn )) );
        } else
            m_tcWindGust->SetValue( _("N/A") );
    }

    //    Update the Pressure control
    if( RecordArray[Idx_PRESSURE] ) {
        double press = RecordArray[Idx_PRESSURE]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( press != GRIB_NOTDEF ) {
            press = m_OverlaySettings.CalibrateValue(GribOverlaySettings::PRESSURE, press);
            int p = (m_OverlaySettings.Settings[GribOverlaySettings::PRESSURE].m_Units == 2) ? 2 : 0;  // if PRESSURE & inHG = two decimals
            m_tcPressure->SetValue( wxString::Format(_T("%2.*f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRESSURE), p, ( press )) );
        } else
            m_tcPressure->SetValue( _("N/A") );
    }

    //    Update the Sig Wave Height
    if( RecordArray[Idx_HTSIGW] ) {
        double height = RecordArray[Idx_HTSIGW]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( height != GRIB_NOTDEF ) {
            height = m_OverlaySettings.CalibrateValue(GribOverlaySettings::WAVE, height);
            wxString w( wxString::Format( _T("%4.1f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WAVE), height ));
            if( RecordArray[Idx_WVPER] ) {
                double period = RecordArray[Idx_WVPER]->
                    getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
                if( period != GRIB_NOTDEF ) w.Append( wxString::Format( _T(" - %01ds") , (int) round(period) ));
            }
            m_tcWaveHeight->SetValue(w);
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
    if(GribRecord::getInterpolatedValues(vkn, ang,
                                         RecordArray[Idx_SEACURRENT_VX],
                                         RecordArray[Idx_SEACURRENT_VY],
                                         m_cursor_lon, m_cursor_lat)) {
        vkn = m_OverlaySettings.CalibrateValue(GribOverlaySettings::CURRENT, vkn);

        m_tcCurrentVelocity->SetValue( wxString::Format( _T("%4.1f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CURRENT), vkn ) );

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
            precip = m_OverlaySettings.CalibrateValue(GribOverlaySettings::PRECIPITATION, precip);
            int p = precip < 10. ? 2 : precip < 100. ? 1 : 0;
            p += m_OverlaySettings.Settings[GribOverlaySettings::PRECIPITATION].m_Units == 1 ? 1 : 0 ;  // if PRESSURE & in = one decimal more
            m_tcPrecipitation->SetValue( wxString::Format( _T("%4.*f ") + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRECIPITATION), p, precip ) );
        } else
            m_tcPrecipitation->SetValue( _("N/A") );
    }

    //    Update total cloud control
    if( RecordArray[Idx_CLOUD_TOT] ) {
        double cloud = RecordArray[Idx_CLOUD_TOT]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( cloud != GRIB_NOTDEF ) {
            cloud = m_OverlaySettings.CalibrateValue(GribOverlaySettings::CLOUD, cloud);
            wxString val( wxString::Format( _T("%5.0f "), cloud ) );
            m_tcCloud->SetValue( val + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CLOUD) );
        } else
            m_tcCloud->SetValue( _("N/A") );
    }

    //    Update the Air Temperature
    if( RecordArray[Idx_AIR_TEMP] ) {
        double temp = RecordArray[Idx_AIR_TEMP]->
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
    // Update extra data for altitude
    // geopotential altitude
    if( RecordArray[Idx_GEOP_HGT + altitude] ) {
        double geop = RecordArray[Idx_GEOP_HGT + altitude]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( geop != GRIB_NOTDEF ) {
            geop = m_OverlaySettings.CalibrateValue(GribOverlaySettings::GEO_ALTITUDE, geop);
            m_tcAltitude->SetValue( wxString::Format( _T("%5.0f "), geop ) + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::GEO_ALTITUDE) );
        } else
            m_tcAltitude->SetValue( _("N/A") );
    }

    // temperature
    if( RecordArray[Idx_AIR_TEMP + altitude] ) {
        double temp = RecordArray[Idx_AIR_TEMP + altitude]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( temp != GRIB_NOTDEF ) {
            temp = m_OverlaySettings.CalibrateValue(GribOverlaySettings::AIR_TEMPERATURE, temp);
            m_tcTemp->SetValue( wxString::Format( _T("%5.1f "), temp ) + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::AIR_TEMPERATURE) );
        } else
            m_tcTemp->SetValue(  _("N/A") );
    }
    // relative humidity
    if( RecordArray[Idx_HUMID_RE + altitude] ) {
        double humi = RecordArray[Idx_HUMID_RE + altitude]->
            getInterpolatedValue( m_cursor_lon, m_cursor_lat, true );

        if( humi != GRIB_NOTDEF ) {
            humi = m_OverlaySettings.CalibrateValue(GribOverlaySettings::REL_HUMIDITY, humi);
            m_tcRelHumid->SetValue( wxString::Format( _T("%5.0f "), humi ) + m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::REL_HUMIDITY) );
        } else
            m_tcRelHumid->SetValue(  _("N/A") );
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
    if( m_tPlayStop.IsRunning() ) return;                            // do nothing when play back is running !

    if(pReq_Dialog){                                                 //there is one instance of the dialog
        if(pReq_Dialog->IsShown()) return;                           //already visible
    }

    delete pReq_Dialog;                                              //delete to be re-created

    pReq_Dialog = new GribRequestSetting( *this );
    pPlugIn->SetDialogFont( pReq_Dialog );
    pPlugIn->SetDialogFont( pReq_Dialog->m_sScrolledDialog );
    pReq_Dialog->OnVpChange(m_vp);
    pReq_Dialog->SetRequestDialogSize();
    //need to set a position at start
    int w;
    ::wxDisplaySize( &w, NULL);
    pReq_Dialog->Move( (w - pReq_Dialog->GetSize().GetX() ) / 3, 30 );
    //
    pReq_Dialog->Show();
}

void GRIBUIDialog::OnSettings( wxCommandEvent& event )
{
    if( m_tPlayStop.IsRunning() ) return;      // do nothing when play back is running !

    GribOverlaySettings initSettings = m_OverlaySettings;
    GribSettingsDialog *dialog = new GribSettingsDialog( *this, m_OverlaySettings,  m_lastdatatype, m_FileIntervalIndex);
    pPlugIn->SetDialogFont( dialog );
    if(dialog->ShowModal() == wxID_OK)
    {
        dialog->WriteSettings();
        m_OverlaySettings.Write();
    } else
        m_OverlaySettings = initSettings;
    if( !m_OverlaySettings.m_bInterpolate ) m_InterpolateMode = false;        //Interpolate could have been unchecked
    SetTimeLineMax(true);
    SetFactoryOptions();
    PopulateTrackingControls();
}

void GRIBUIDialog::OnPlayStop( wxCommandEvent& event )
{
    if( m_tPlayStop.IsRunning() ) {
        StopPlayBack();
    } else {
        m_bpPlay->SetBitmap(wxBitmap( stop ));
        m_bpPlay->SetToolTip( _("Stop") );
        m_tPlayStop.Start( 1000/m_OverlaySettings.m_UpdatesPerSecond, wxTIMER_CONTINUOUS );
        m_InterpolateMode = m_OverlaySettings.m_bInterpolate;
    }
}

void GRIBUIDialog::OnPlayStopTimer( wxTimerEvent & event )
{
    if(m_sTimeline->GetValue() >= m_sTimeline->GetMax()) {
        if(m_OverlaySettings.m_bLoopMode) {
            if(m_OverlaySettings.m_LoopStartPoint) {
                ComputeBestForecastForNow();
                if(m_sTimeline->GetValue() >= m_sTimeline->GetMax()) StopPlayBack();        //will stop playback
                return;
            } else
                m_sTimeline->SetValue(0);
        } else {
            StopPlayBack();                                           //will stop playback
            return;
        }
    } else {
        int value = m_pNowMode ? m_OverlaySettings.m_bInterpolate ?
            GetNearestValue(GetNow(), 1) : GetNearestIndex(GetNow(), 2) : m_sTimeline->GetValue();
        m_sTimeline->SetValue(value + 1);
    }

    m_pNowMode = false;
    if(!m_InterpolateMode) m_cRecordForecast->SetSelection( m_sTimeline->GetValue() );
    TimelineChanged();
}

void GRIBUIDialog::StopPlayBack()
{
    if( m_tPlayStop.IsRunning() ) {
        m_tPlayStop.Stop();
        m_bpPlay->SetBitmap(wxBitmap( play ));
        m_bpPlay->SetToolTip( _("Play") );
    }
}

void GRIBUIDialog::TimelineChanged()
{
    if( !m_bGRIBActiveFile || (m_bGRIBActiveFile && !m_bGRIBActiveFile->IsOK()) ) {
        pPlugIn->GetGRIBOverlayFactory()->SetGribTimelineRecordSet(NULL);
        return;
    }

     RestaureSelectionString();                      //eventually restaure the previousely saved time label

    wxDateTime time = TimelineTime();
    SetGribTimelineRecordSet(GetTimeLineRecordSet(time));

    if( !m_InterpolateMode ){
    /* get closest value to update timeline */
        ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
        GribRecordSet &sel=rsa->Item(m_cRecordForecast->GetCurrentSelection());
        wxDateTime t = sel.m_Reference_Time;
        m_sTimeline->SetValue(
            m_OverlaySettings.m_bInterpolate ?
                wxTimeSpan(t - MinTime()).GetMinutes() / m_OverlaySettings.GetMinFromIndex(m_OverlaySettings.m_SlicesPerUpdate)
                : m_cRecordForecast->GetCurrentSelection()
            );
    } else {
        m_cRecordForecast->SetSelection(GetNearestIndex(time, 2));
        SaveSelectionString();                                                                     //memorize index and label
        m_cRecordForecast->SetString( m_Selection_index, TToString(time, pPlugIn->GetTimeZone()) );//replace it by the interpolated time label
        m_cRecordForecast->SetStringSelection( TToString(time, pPlugIn->GetTimeZone()) );          //ensure it's visible in the box
    }

    UpdateTrackingControls();

    pPlugIn->SendTimelineMessage(time);
    RequestRefresh( pParent );
}

void GRIBUIDialog::RestaureSelectionString()
{
    if( !m_SelectionIsSaved ) return;

    int sel = m_cRecordForecast->GetSelection();
    m_cRecordForecast->SetString( m_Selection_index, m_Selection_label );
    m_cRecordForecast->SetSelection( sel );
    m_SelectionIsSaved = false;
}

int GRIBUIDialog::GetNearestIndex(wxDateTime time, int model)
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
    if(!model) return (time - itime > (ip1time - time)*3) ? i+1 : i;

    return model == 1 ? time == ip1time ? i : i+1 : time == ip1time ? i+1 : i;
}

int GRIBUIDialog::GetNearestValue(wxDateTime time, int model)
{
    /* get closest value to update Time line */
    if(m_TimeLineHours == 0) return 0;
    wxDateTime itime, ip1time;
    int stepmin = m_OverlaySettings.GetMinFromIndex(m_OverlaySettings.m_SlicesPerUpdate);
    wxTimeSpan span = time - MinTime();
    int t = span.GetMinutes()/stepmin;
    itime = MinTime() + wxTimeSpan( t * stepmin / 60, (t * stepmin) % 60 );     //time at t
    ip1time = itime + wxTimeSpan( stepmin / 60, stepmin % 60 );                 //time at t+1

    if(model == 1) return time == ip1time ? t+1 : t;

    return (time - itime > (ip1time - time)*3) ? t+1 : t;
}

wxDateTime GRIBUIDialog::GetNow()
{
    //wxDateTime::Now() is in local time and must be transslated to UTC to be compared to grib times
    wxDateTime now = wxDateTime::Now().ToUTC(wxDateTime::Now().IsDST()==0).SetSecond(0);
    if(now.IsDST()) now.Add(wxTimeSpan( 1,0,0,0));          //bug in wxWidgets ?

    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();

    //verifie if we are outside of the file time range
    now = (now > rsa->Item(rsa->GetCount()-1).m_Reference_Time) ? rsa->Item(rsa->GetCount()-1).m_Reference_Time :
        (now < rsa->Item(0).m_Reference_Time) ? rsa->Item(0).m_Reference_Time : now;
    return now;
}

wxDateTime GRIBUIDialog::TimelineTime()
{
    if(m_InterpolateMode) {
        int tl = (m_TimeLineHours == 0) ? 0 : m_sTimeline->GetValue();
        int stepmin = m_OverlaySettings.GetMinFromIndex(m_OverlaySettings.m_SlicesPerUpdate);
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

    if(rsa->GetCount() == 0)
        return NULL;

    wxDateTime curtime;
    for(i=0; i<rsa->GetCount(); i++) {
        GribRecordSet &cur=rsa->Item(i);
        curtime = cur.m_Reference_Time;
        if(curtime >= time)
            break;
    }
    if(i == 0)
        im1 = 0;
    else
        im1 = i-1;

    if(curtime == time) im1 = i;                            //no interpolation for record boundary

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
    StopPlayBack();
    m_InterpolateMode = m_OverlaySettings.m_bInterpolate;
    if(!m_InterpolateMode) m_cRecordForecast->SetSelection(m_sTimeline->GetValue());
    m_pNowMode = false;
    TimelineChanged();
}

void GRIBUIDialog::OnAltitudeChange( wxCommandEvent& event )
{
    double alt;
    m_cbAltitude->GetString( m_cbAltitude->GetCurrentSelection() ).ToDouble(&alt);
    switch((int) alt) {
    case 8:
    case 225:
    case 300: pPlugIn->GetGRIBOverlayFactory()->SetAltitude(4);break;
    case 14:
    case 375:
    case 500: pPlugIn->GetGRIBOverlayFactory()->SetAltitude(3);break;
    case 20:
    case 525:
    case 700: pPlugIn->GetGRIBOverlayFactory()->SetAltitude(2);break;
    case 25:
    case 637:
    case 850: pPlugIn->GetGRIBOverlayFactory()->SetAltitude(1);break;
    default:  pPlugIn->GetGRIBOverlayFactory()->SetAltitude(0);break;
    }
    PopulateTrackingControls( false );
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::OnCBAny( wxCommandEvent& event )
{
    ResolveDisplayConflicts( this, event.GetId() );
}

void GRIBUIDialog::OnOpenFile( wxCommandEvent& event )
{
    if( m_tPlayStop.IsRunning() ) return;      // do nothing when play back is running !



    if( !wxDir::Exists( m_grib_dir ) ) {
        wxStandardPathsBase& path = wxStandardPaths::Get();
        m_grib_dir = path.GetDocumentsDir();
    }

    wxFileDialog *dialog = new wxFileDialog(NULL, _("Select a GRIB file"), m_grib_dir,
        _T(""), wxT ( "Grib files (*.grb;*.bz2)|*.grb;*.bz2|All files (*)|*.*"), wxFD_OPEN, wxDefaultPosition,
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
    if( m_cRecordForecast->GetCount() ){
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

void GRIBUIDialog::OnZoomToCenterClick( wxCommandEvent& event )
{
    if(!m_pTimelineSet) return;

    double latmin,latmax,lonmin,lonmax;
    if(!GetGribZoneLimits(m_pTimelineSet, &latmin, &latmax, &lonmin, &lonmax ))
        return;

    //calculate overlay size
    double width = lonmax - lonmin;
    double height = latmax - latmin;

    // Calculate overlay center
    double clat = latmin + height / 2;
    double clon = lonmin + width / 2;

    //try to limit the ppm at a reasonable value
    if(width  > 120.){
        lonmin = clon - 60.;
        lonmax = clon + 60.;
    }
    if(height > 120.){
        latmin = clat - 60.;
        latmax = clat + 60.;
    }


    //Calculate overlay width & height in nm (around the center)
    double ow, oh;
    DistanceBearingMercator_Plugin(clat, lonmin, clat, lonmax, NULL, &ow );
    DistanceBearingMercator_Plugin( latmin, clon, latmax, clon, NULL, &oh );

    //calculate screen size
    int w = pPlugIn->GetGRIBOverlayFactory()->m_ParentSize.GetWidth();
    int h = pPlugIn->GetGRIBOverlayFactory()->m_ParentSize.GetHeight();

    //calculate final ppm scale to use
    double ppm;
    ppm = wxMin(w/(ow*1852), h/(oh*1852)) * ( 100 - fabs( clat ) ) / 90;

    ppm = wxMin(ppm, 1.0);

    JumpToPosition(clat, clon, ppm);

    RequestRefresh( pParent );

}

void GRIBUIDialog::OnPrev( wxCommandEvent& event )
{
    if( m_tPlayStop.IsRunning() ) return;      // do nothing when play back is running !

    RestaureSelectionString();

    int selection;
    if(m_pNowMode)
        selection = GetNearestIndex(GetNow(), 1);
    else if(m_InterpolateMode)
        selection = GetNearestIndex(TimelineTime(), 1);  /* set to interpolated entry */
    else
        selection = m_cRecordForecast->GetCurrentSelection();

    m_pNowMode = false;
    m_InterpolateMode = false;

    m_cRecordForecast->SetSelection( selection < 1 ? 0: selection - 1 );

    TimelineChanged();

}

void GRIBUIDialog::OnNext( wxCommandEvent& event )
{
    if( m_tPlayStop.IsRunning() ) return;      // do nothing when play back is running !

    RestaureSelectionString();

    int selection;
    if(m_pNowMode)
        selection = GetNearestIndex(GetNow(), 2);
    else if(m_InterpolateMode)
        selection = GetNearestIndex(TimelineTime(), 2);  /* set to interpolated entry */
    else
        selection = m_cRecordForecast->GetCurrentSelection();

    m_cRecordForecast->SetSelection( selection );

    m_pNowMode = false;
    m_InterpolateMode = false;

    if( selection == (int)m_cRecordForecast->GetCount() - 1 ) return; //end of list

    m_cRecordForecast->SetSelection( selection  + 1 );

    TimelineChanged();
}

void GRIBUIDialog::ComputeBestForecastForNow()
{
    if( !m_bGRIBActiveFile || (m_bGRIBActiveFile && !m_bGRIBActiveFile->IsOK()) ) {
        pPlugIn->GetGRIBOverlayFactory()->SetGribTimelineRecordSet(NULL);
        return;
    }

    wxDateTime now = GetNow();

    if( m_OverlaySettings.m_bInterpolate )
        m_sTimeline->SetValue(GetNearestValue(now, 0));
    else{
        m_cRecordForecast->SetSelection(GetNearestIndex(now, 0));
        m_sTimeline->SetValue(m_cRecordForecast->GetCurrentSelection());
    }

    if( pPlugIn->GetStartOptions() != 2 ) {         //no interpolation at start : take the nearest forecast
        m_InterpolateMode = m_OverlaySettings.m_bInterpolate;
        TimelineChanged();
        return;
    }
    //interpolation on 'now' at start
    m_InterpolateMode = true;
    m_pNowMode = true;
    SetGribTimelineRecordSet(GetTimeLineRecordSet(now));             //take current time & interpolate forecast

    RestaureSelectionString();                                       //eventually restaure the previousely saved wxChoice date time label
    m_cRecordForecast->SetSelection(GetNearestIndex(now, 2));
    SaveSelectionString();                                           //memorize the new selected wxChoice date time label
    m_cRecordForecast->SetString( m_Selection_index, TToString(now, pPlugIn->GetTimeZone()) );        //write the now date time label in the right place in wxChoice
    m_cRecordForecast->SetStringSelection( TToString(now, pPlugIn->GetTimeZone()) );                  //put it in the box

    UpdateTrackingControls();

    pPlugIn->SendTimelineMessage(now);
    RequestRefresh( pParent );

}

void GRIBUIDialog::SetGribTimelineRecordSet(GribTimelineRecordSet *pTimelineSet)
{
    delete m_pTimelineSet;
    m_pTimelineSet = pTimelineSet;

    if(!pPlugIn->GetGRIBOverlayFactory())
        return;

    pPlugIn->GetGRIBOverlayFactory()->SetGribTimelineRecordSet(m_pTimelineSet);
}

void GRIBUIDialog::SetTimeLineMax( bool SetValue )
{
    int oldmax = wxMax(m_sTimeline->GetMax(), 1), oldval = m_sTimeline->GetValue();             //memorize the old range and value

    if(m_OverlaySettings.m_bInterpolate){
        int stepmin = m_OverlaySettings.GetMinFromIndex(m_OverlaySettings.m_SlicesPerUpdate);
        m_sTimeline->SetMax(m_TimeLineHours * 60 / stepmin );
    } else {
        if(m_bGRIBActiveFile && m_bGRIBActiveFile->IsOK()) {
            ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
            m_sTimeline->SetMax(rsa->GetCount()-1);
        }
    }
    //try to retrieve a coherent timeline value with the new timeline range if it has changed
        if( SetValue && m_sTimeline->GetMax() != 0 ) {
            if( m_pNowMode )
                ComputeBestForecastForNow();
            else
                m_sTimeline->SetValue( m_sTimeline->GetMax() * oldval / oldmax );
        }
}

void GRIBUIDialog::SetFactoryOptions()
{
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
    m_pGribReader = NULL;

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
                    int idx = -1, mdx = -1;
                    switch(pRec->getDataType()) {
                    case GRB_WIND_VX:
                        if(pRec->getLevelType() == LV_ISOBARIC){
                            switch(pRec->getLevelValue()){
                            case 300: idx = Idx_WIND_VX300;break;
                            case 500: idx = Idx_WIND_VX500;break;
                            case 700: idx = Idx_WIND_VX700;break;
                            case 850: idx = Idx_WIND_VX850;break;
                            }
                        } else
                            idx = Idx_WIND_VX;
                        break;
                    case GRB_WIND_VY:
                        if(pRec->getLevelType() == LV_ISOBARIC){
                            switch(pRec->getLevelValue()){
                            case 300: idx = Idx_WIND_VY300;break;
                            case 500: idx = Idx_WIND_VY500;break;
                            case 700: idx = Idx_WIND_VY700;break;
                            case 850: idx = Idx_WIND_VY850;break;
                            }
                        } else
                            idx = Idx_WIND_VY;
                        break;
                    case GRB_WIND_GUST: idx = Idx_WIND_GUST; break;
                    case GRB_PRESSURE: idx = Idx_PRESSURE;   break;
                    case GRB_HTSGW:    idx = Idx_HTSIGW;  break;
                    case GRB_WVPER:    idx = Idx_WVPER;  break;
                    case GRB_WVDIR:    idx = Idx_WVDIR;   break;
                    case GRB_UOGRD:    idx = Idx_SEACURRENT_VX; break;
                    case GRB_VOGRD:    idx = Idx_SEACURRENT_VY; break;
                    case GRB_PRECIP_TOT: idx = Idx_PRECIP_TOT; break;
                    case GRB_CLOUD_TOT:  idx = Idx_CLOUD_TOT; break;
                    case GRB_TEMP:
                        if(pRec->getLevelType() == LV_ISOBARIC){
                            switch(pRec->getLevelValue()){
                            case 300: idx = Idx_AIR_TEMP300;break;
                            case 500: idx = Idx_AIR_TEMP500;break;
                            case 700: idx = Idx_AIR_TEMP700;break;
                            case 850: idx = Idx_AIR_TEMP850;break;
                            }
                        } else
                            idx = Idx_AIR_TEMP;
                        if(pRec->getDataCenterModel() == NORWAY_METNO ) mdx = 1000 + NORWAY_METNO;
                        break;
                    case GRB_WTMP:
                        idx = Idx_SEA_TEMP;
                        if(pRec->getDataCenterModel() == NOAA_GFS ) mdx = 1000 + NOAA_GFS;
                        break;
                    case GRB_CAPE:      idx = Idx_CAPE;break;
                    case GRB_HUMID_REL:
                        if(pRec->getLevelType() == LV_ISOBARIC){
                            switch(pRec->getLevelValue()){
                            case 300: idx = Idx_HUMID_RE300;break;
                            case 500: idx = Idx_HUMID_RE500;break;
                            case 700: idx = Idx_HUMID_RE700;break;
                            case 850: idx = Idx_HUMID_RE850;break;
                            }
                        }
                        break;
                    case GRB_GEOPOT_HGT:
                        if(pRec->getLevelType() == LV_ISOBARIC){
                            switch(pRec->getLevelValue()){
                            case 300: idx = Idx_GEOP_HGT300;break;
                            case 500: idx = Idx_GEOP_HGT500;break;
                            case 700: idx = Idx_GEOP_HGT700;break;
                            case 850: idx = Idx_GEOP_HGT850;break;
                            }
                        }
                        break;
                     }

                    if(idx != -1) {
                        m_GribRecordSetArray.Item( j ).m_GribRecordPtrArray[idx]= pRec;
                        if(m_GribIdxArray.Index(idx) == wxNOT_FOUND ) m_GribIdxArray.Add(idx, 1);
                        if(mdx != -1 && m_GribIdxArray.Index(mdx) == wxNOT_FOUND ) m_GribIdxArray.Add(mdx, 1);
                    }
                    break;
                }
            }
        }
    }
    if(isOK) m_pRefDateTime = pRec->getRecordRefDate();     //to ovoid crash with some bad files
}

GRIBFile::~GRIBFile()
{
    delete m_pGribReader;
}

