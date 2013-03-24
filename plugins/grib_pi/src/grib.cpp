/******************************************************************************
 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
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

#include "folder.xpm"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( ArrayOfGribRecordSets );
WX_DEFINE_OBJARRAY( ArrayOfGribRecordPtrs );

enum {
    ID_GRIBNEXTRECORD = 10001, ID_GRIBPREVRECORD, ID_GRIBNOWRECORD, ID_CHOOSEGRIBDATA, ID_CHOOSEGRIBFILE,
    ID_GRIBSETDATA, ID_GRIBREQUEST
};

const wxString resolution1[] = { _T("0.5 Deg"), _("1.0 Deg"), _("1.5 Deg"), _("2.0 Deg") };
const wxString resolution2[] = { _("0.2 Deg"), _("0.6 Deg"), _("1.2 Deg"), _("2.0 Deg") };

//    Sort compare function for File Modification Time
static int CompareFileStringTime( const wxString& first, const wxString& second )
{
    wxFileName f( first );
    wxFileName s( second );
    wxTimeSpan sp = s.GetModificationTime() - f.GetModificationTime();
    return sp.GetMinutes();
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

//speed in the desired unit
static wxString SUToString(  double val, const int unit )
{
    switch( unit ) {
    case 0:
       if( val < 1 ) return wxString::Format( _T("%3.2f m/s"), val ); else return wxString::Format( _T("%3.1f m/s"), val );
    case 1:
    default:
        val *= 1.943845;             // ( 1.943845 = ( 3.6 / 1.852 )   translated in kts
        if( val < 2 ) return wxString::Format( _T("%3.2f kts"), val ); else return wxString::Format( _T("%3.1f kts"), val );
    }
}

static wxString MToString( int DataCenterModel )
{
    switch( DataCenterModel ) {
    case NOAA_GFS: return  _T("NOAA_GFS");
    case NOAA_NCEP_WW3: return  _T("NOAA_NCEP_WW3");
    case NOAA_NCEP_SST: return  _T("NOAA_NCEP_SST");
    case NOAA_RTOFS: return  _T("NOAA_RTOFS");
    case FNMOC_WW3_GLB: return  _T("FNMOC_WW3");
    case FNMOC_WW3_MED: return  _T("FNMOC_WW3");
    case NORWAY_METNO: return  _T("NORWAY_METNO");
    default : return  _T("OTHER_DATA_CENTER");
    }
}

static wxString DToString( int Data )
{
    switch( Data ) {
    case ID_CB_NOMAP:  return _("No other graphic display");
    case ID_CB_WINDSP: return _("Wind Speed(10m)");
    case ID_CB_WINDIR: return _("Wind Direction(10m)");
    case ID_CB_WINDM:  return  _("Wind Speed and Direction(10m)");
    case ID_CB_WIGUSM:
    case ID_CB_WIGUST: return  _("Wind Gust(Surface)");
    case ID_CB_SIWAVD:
    case ID_CB_SIWAVM:
        return  _("Sign. Wave Height");
    case ID_CB_PRESS:
    case ID_CB_PRESSD:
    case ID_CB_PRESSM:
        return _("Pressure (MSL)");
    case ID_CB_PRETOD:
    case ID_CB_PRETOM: 
        return  _("Total Rainfall");
    case ID_CB_CLOCVD:
    case ID_CB_CLOCVM: 
        return  _("Clouds Cover");
    case ID_CB_ATEM2D:
    case ID_CB_ATEM2M: 
        return  _("Air Temp.(2m)");
    case ID_CB_SEATED:
    case ID_CB_SEATEM: 
        return  _("Sea Temp.(Surf)");
    case ID_CB_CURRED:
    case ID_CB_CURREM:
        return  _("Current Velocity");
    default : return  _("Unnown");
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


//---------------------------------------------------------------------------------------
//          GRIB Selector/Control Dialog Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS ( GRIBUIDialog, wxDialog )

BEGIN_EVENT_TABLE ( GRIBUIDialog, wxDialog )

EVT_CLOSE ( GRIBUIDialog::OnClose )
EVT_MOVE ( GRIBUIDialog::OnMove )
EVT_SIZE ( GRIBUIDialog::OnSize )
EVT_TIMER ( GRIB_FORECAST_TIMER, GRIBUIDialog::OnGRIBForecastTimerEvent )
EVT_TIMER ( CANVAS_REFRESH_TIMER, GRIBUIDialog::OnCanvasRefreshTimerEvent )
EVT_BUTTON( ID_CHOOSEGRIBFILE, GRIBUIDialog::OnButtonOpenFileClick )
EVT_COMBOBOX( ID_CHOOSEGRIBDATA, GRIBUIDialog::OnGRIBForecastChange )
EVT_BUTTON ( ID_GRIBNEXTRECORD, GRIBUIDialog::OnButtonNextClick )
EVT_BUTTON ( ID_GRIBPREVRECORD, GRIBUIDialog::OnButtonPrevClick )
EVT_BUTTON ( ID_GRIBNOWRECORD, GRIBUIDialog::OnButtonNowClick )
EVT_BUTTON (ID_GRIBSETDATA,GRIBUIDialog::OnButtonSettingClick)
EVT_BUTTON (ID_GRIBREQUEST,GRIBUIDialog::OnGribRequestClick)
END_EVENT_TABLE()

GRIBUIDialog::GRIBUIDialog()
{
    Init();
}

GRIBUIDialog::~GRIBUIDialog()
{
}

void GRIBUIDialog::Init()
{
    m_bGRIBActiveFile = NULL;
    m_pCurrentGribRecordSet = NULL;
    m_pRecordForecast = NULL;
}

bool GRIBUIDialog::Create( wxWindow *parent, grib_pi *ppi, wxWindowID id, const wxString& caption,
        const wxPoint& pos, const wxSize& size, long style )
{
    pParent = parent;
    pPlugIn = ppi;

    m_DataDisplayConfig = pPlugIn->GetGRIBDataConfig();
    m_NumberDataShown = GetNumberDataShown();

     m_pPrev_bitmap = new wxBitmap( prev );   // comes from XPM include
     m_pNext_bitmap = new wxBitmap( next );
     m_pNow_bitmap = new wxBitmap( now );
     m_pOpen_bitmap = new wxBitmap( openfile );
     m_pPref_bitmap = new wxBitmap( setting );
     m_pRequ_bitmap = new wxBitmap( request );

    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN;
//      if ( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY ) && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) )
//            wstyle |= ( wxNO_BORDER );

 //   wxSize size_min = size;
//      size_min.IncTo ( wxSize ( 500,600 ) );
    if( !wxDialog::Create( parent, id, caption, pos, size, wstyle ) ) return false;

    CreateControls();

    DimeWindow( this );

    Fit();
    SetMinSize( GetBestSize() );

    return true;
}

void GRIBUIDialog::CreateControls()
{
    int border_size = 0;
    int item_spacing = 0;
    int button_spacing = 3;           // used for align buttons and text

    wxFont *dFont_For = new wxFont( 11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    wxFont *dFont_Lab = new wxFont( 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    wxFont *dFont_val = new wxFont( 9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
    //SetOwnFont(*dFont_For);

// A top-level sizer
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer );

// A second box sizer to give more space around the controls
    wxBoxSizer* boxSizer = new wxBoxSizer( wxVERTICAL );
    topSizer->Add( boxSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 1 );

//    The GRIB File and Forecast selection
    wxFlexGridSizer *pFileGrid = new wxFlexGridSizer( 7 );
    topSizer->Add( pFileGrid, 0, wxALL | wxEXPAND, border_size );

    m_pButtonPrev = new wxBitmapButton( this, ID_GRIBPREVRECORD, *m_pPrev_bitmap , wxDefaultPosition, wxDefaultSize);
    pFileGrid->Add( m_pButtonPrev, 0,  wxLEFT | wxRIGHT, button_spacing  );

    m_pRecordForecast = new wxComboBox(this, ID_CHOOSEGRIBDATA, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    pFileGrid->Add( m_pRecordForecast, 2,  wxTOP | wxEXPAND, 3 );
    m_pRecordForecast->SetFont( *dFont_For );

    m_pButtonNext = new wxBitmapButton( this, ID_GRIBNEXTRECORD, *m_pNext_bitmap, wxDefaultPosition, wxDefaultSize);
    pFileGrid->Add( m_pButtonNext, 0,  wxLEFT | wxRIGHT, button_spacing  );

    m_pButtonNow = new wxBitmapButton( this, ID_GRIBNOWRECORD, *m_pNow_bitmap, wxDefaultPosition, wxDefaultSize);
    pFileGrid->Add( m_pButtonNow, 0,  wxLEFT | wxRIGHT, button_spacing  );

    m_pButtonOpen = new wxBitmapButton( this, ID_CHOOSEGRIBFILE, *m_pOpen_bitmap, wxDefaultPosition, wxDefaultSize);
    pFileGrid->Add( m_pButtonOpen, 0,  wxLEFT, 20  );

    m_pButtonPref = new wxBitmapButton( this, ID_GRIBSETDATA, *m_pPref_bitmap, wxDefaultPosition, wxDefaultSize);
    pFileGrid->Add( m_pButtonPref, 0,  wxLEFT | wxRIGHT, 20 );

    m_pButtonRequ = new wxBitmapButton( this, ID_GRIBREQUEST, *m_pRequ_bitmap, wxDefaultPosition, wxDefaultSize);
    pFileGrid->Add( m_pButtonRequ, 0,  wxRIGHT, 2 );

        //Data Box
        wxStaticBox* itemStaticBoxData = new wxStaticBox( this, wxID_ANY, _T("") );
        wxStaticBoxSizer* itemStaticBoxSizerData = new wxStaticBoxSizer( itemStaticBoxData,
            wxVERTICAL );
        topSizer->Add( itemStaticBoxSizerData, 0, wxALL | wxEXPAND, border_size );

        wxFlexGridSizer *pDataGrid = new wxFlexGridSizer( 6 );
        itemStaticBoxSizerData->Add( pDataGrid, 0, wxALL | wxEXPAND, border_size );

        m_pT0StaticText = new wxStaticText( this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize );
        pDataGrid->Add( m_pT0StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
        m_pT0StaticText->SetFont( *dFont_Lab );
         
        m_pT0TextCtrl = new wxTextCtrl( this, -1,  _T(""), wxDefaultPosition, wxSize( 75, -1 ),
            wxTE_READONLY );
        pDataGrid->Add( m_pT0TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
        m_pT0TextCtrl->SetFont( *dFont_val );

        m_pT1StaticText = new wxStaticText( this, wxID_ANY,  _T(""), wxDefaultPosition, wxDefaultSize );
        pDataGrid->Add( m_pT1StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
        m_pT1StaticText->SetFont( *dFont_Lab );

        m_pT1TextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxSize( 75, -1 ),
            wxTE_READONLY );
        pDataGrid->Add( m_pT1TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
        m_pT1TextCtrl->SetFont( *dFont_val );

        m_pT2StaticText = new wxStaticText( this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize );
        pDataGrid->Add( m_pT2StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
        m_pT2StaticText->SetFont( *dFont_Lab );

        m_pT2TextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxSize( 75, -1 ),
            wxTE_READONLY );
        pDataGrid->Add( m_pT2TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
        m_pT2TextCtrl->SetFont( *dFont_val );

        if( m_NumberDataShown > 3 ) {
            m_pT3StaticText = new wxStaticText( this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize );
            pDataGrid->Add( m_pT3StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
            m_pT3StaticText->SetFont( *dFont_Lab );

            m_pT3TextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxSize( 75, -1 ),
                wxTE_READONLY );
            pDataGrid->Add( m_pT3TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
            m_pT3TextCtrl->SetFont( *dFont_val );

            m_pT4StaticText = new wxStaticText( this, wxID_ANY,  _T(""), wxDefaultPosition, wxDefaultSize );
            pDataGrid->Add( m_pT4StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
            m_pT4StaticText->SetFont( *dFont_Lab );

            m_pT4TextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxSize( 75, -1 ),
                wxTE_READONLY );
            pDataGrid->Add( m_pT4TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
            m_pT4TextCtrl->SetFont( *dFont_val );

            m_pT5StaticText = new wxStaticText( this, wxID_ANY,  _T(""), wxDefaultPosition, wxDefaultSize );
            pDataGrid->Add( m_pT5StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
            m_pT5StaticText->SetFont( *dFont_Lab );

            m_pT5TextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxSize( 75, -1 ),
                wxTE_READONLY );
            pDataGrid->Add( m_pT5TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
            m_pT5TextCtrl->SetFont( *dFont_val );
        }
        if( m_NumberDataShown > 6 ) {
            m_pT6StaticText = new wxStaticText( this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize );
            pDataGrid->Add( m_pT6StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
            m_pT6StaticText->SetFont( *dFont_Lab );

            m_pT6TextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxSize( 75, -1 ),
                wxTE_READONLY );
            pDataGrid->Add( m_pT6TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
             m_pT6TextCtrl->SetFont( *dFont_val );

            m_pT7StaticText = new wxStaticText( this, wxID_ANY,  _T(""), wxDefaultPosition, wxDefaultSize );
            pDataGrid->Add( m_pT7StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
            m_pT7StaticText->SetFont( *dFont_Lab );

            m_pT7TextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxSize( 75, -1 ),
                wxTE_READONLY );
            pDataGrid->Add( m_pT7TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
             m_pT7TextCtrl->SetFont( *dFont_val );

            m_pT8StaticText = new wxStaticText( this, wxID_ANY,  _T(""), wxDefaultPosition, wxDefaultSize );
            pDataGrid->Add( m_pT8StaticText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT| wxALIGN_CENTER_VERTICAL | wxEXPAND, 2 );
            m_pT8StaticText->SetFont( *dFont_Lab );

            m_pT8TextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxSize( 75, -1 ),
                wxTE_READONLY );
            pDataGrid->Add( m_pT8TextCtrl, 0, wxALIGN_RIGHT, item_spacing );
            m_pT8TextCtrl->SetFont( *dFont_val );
        } 

    m_pGribForecastTimer.SetOwner( this, GRIB_FORECAST_TIMER );

    UpdateTrackingLabels();

    Fit();

    m_pCanvasRefreshTimer.SetOwner( this, CANVAS_REFRESH_TIMER );
    m_pCanvasRefreshTimer.Start( CANVAS_REFRESH_100MS ) ;
}

void GRIBUIDialog::SetCursorLatLon( double lat, double lon )
{
    m_cursor_lon = lon;
    m_cursor_lat = lat;

    UpdateTrackingControls();
}

void GRIBUIDialog::UpdateTrackingControls()
{
    if( m_pCurrentGribRecordSet ) {
        //Update the wind control
        wxArrayString val;
        int i = 1;             //index 0 & 1 are systematicaly used by wind speed and dir
        val.Add( _T(""), m_DataDisplayConfig.Len() -1 );   //open as position as panel data existing

        if( ( m_RS_Idx_WIND_VX != -1 ) && ( m_RS_Idx_WIND_VY != -1 ) ) {
            double vx =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_WIND_VX )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );
            double vy =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_WIND_VY )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );

            if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                double vkn = sqrt( vx * vx + vy * vy ); //* 3.6 / 1.852;            kept in m/s
                double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                if( ang > 360. ) ang -= 360.;
                if( ang < 0. ) ang += 360.;

                val[0] = SUToString( vkn , pPlugIn->GetSpeedUnit() );

                val[1].Printf( _T("%03d Deg"), (int) ( ang ) );
            }
        }
        //    Update the QuickScat (aka Wind) control
        if( ( m_RS_Idx_WINDSCAT_VX != -1 ) && ( m_RS_Idx_WINDSCAT_VY != -1 ) ) {
            double vx =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_WINDSCAT_VX )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );
            double vy =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_WINDSCAT_VY )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );

            if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                double vkn = sqrt( vx * vx + vy * vy ); //* 3.6 / 1.852;             //kept in m/s
                double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                if( ang > 360. ) ang -= 360.;
                if( ang < 0. ) ang += 360.;

                val[0] = SUToString( vkn , pPlugIn->GetSpeedUnit() );

                val[1].Printf( _T("%03d Deg"), (int) ( ang ) );      
            }
        }
        //    Update Wind gusts control
        if( m_DataDisplayConfig.GetChar( ID_CB_WIGUST ) == _T('X') ) {
            i++;
            if( m_RS_Idx_WIND_GUST != -1 ) {
                double vgkn =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_WIND_GUST )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );
                if( vgkn != GRIB_NOTDEF ) {
                val[i] = SUToString( vgkn , pPlugIn->GetSpeedUnit() );
                }
            }
        }
         //    Update the Pressure control
        if( m_DataDisplayConfig.GetChar( ID_CB_PRESSD ) == _T('X') ) {
            i++;
            if( m_RS_Idx_PRESS != -1 ) {
                double press =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_PRESS )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );
                if( press != GRIB_NOTDEF ) {
                    val[i].Printf( _T("%2d mBa"), (int) ( press / 100. ) );
                }
            }
        }
        //    Update the Sig Wave Height
        if( m_DataDisplayConfig.GetChar( ID_CB_SIWAVD ) == _T('X') ) {
            i++;
            if( m_RS_Idx_HTSIGW != -1 ) {
                double height =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_HTSIGW )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );
                if( height != GRIB_NOTDEF ) {
                    val[i].Printf( _T("%3.1f m"), height );
                    if( m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_HTSIGW )->isDuplicated() )
                        val[i].Prepend( _T("D ") );
                }       
            }
        }
        //    Update total rainfall control
        if( m_DataDisplayConfig.GetChar( ID_CB_PRETOD ) == _T('X') ) {
            i++;
            if( m_RS_Idx_PRECIP_TOT != -1 ) {
                double pretot =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_PRECIP_TOT )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );
                if( pretot != GRIB_NOTDEF ) {
                    pretot < 10. ? val[i].Printf( _T("%3.2f mm/h"), pretot ) : val[i].Printf( _T("%3.1f mm/h"), pretot );
                    if( m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_PRECIP_TOT )->isDuplicated() )
                        val[i].Prepend( _T("D ") );
                }
            }
        }
        //    Update Cloud Cover control
        if( m_DataDisplayConfig.GetChar( ID_CB_CLOCVD ) == _T('X') ) {
            i++;
            if( m_RS_Idx_CLOUD_TOT != -1 ) {
                double clocv =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_CLOUD_TOT )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );
                if( clocv != GRIB_NOTDEF ){
                    val[i].Printf( _T("%3.0f"), clocv );
                    val[i].Append( _T(" %") );
                    if( m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_CLOUD_TOT )->isDuplicated() )
                        val[i].Prepend( _T("D ") );
                }
            }
        }
        //update air temperature at 2m
        if( m_DataDisplayConfig.GetChar( ID_CB_ATEM2D ) == _T('X') ) {
            i++;
            if( m_RS_Idx_AIR_TEMP_2M != -1 ) {
                double atemp2 =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_AIR_TEMP_2M )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );
                if( atemp2 != GRIB_NOTDEF ){
                    atemp2 -= 273.15;
                    val[i].Printf( _T("%3.1f C"), atemp2 );
                    if( m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_AIR_TEMP_2M )->isDuplicated() )
                        val[i].Prepend( _T("D ") );
                }
            }
        }
        //update sea surface temperature
        if( m_DataDisplayConfig.GetChar( ID_CB_SEATED ) == _T('X') ) {
            i++;
            if( m_RS_Idx_SEATEMP != -1 ) {
                double temp =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_SEATEMP )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );
                if( temp != GRIB_NOTDEF ) {
                    temp -= 273.15;
//                      std::cout << "temp";
                    val[i].Printf( _T("%2.2f C"), temp );
                }
            }
        }
        //    Update the Current control
        if( m_DataDisplayConfig.GetChar( ID_CB_CURRED ) == _T('X') ) {
            i++;
            if( ( m_RS_Idx_SEACURRENT_VX != -1 ) && ( m_RS_Idx_SEACURRENT_VY != -1 ) ) {
                double vx =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_SEACURRENT_VX )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );
                double vy =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_SEACURRENT_VY )->getInterpolatedValue(
                    m_cursor_lon, m_cursor_lat, true );

                if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                    double vkn = sqrt( vx * vx + vy * vy );// * 3.6 / 1.852;
                    double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                    if( ang > 360. ) ang -= 360.;
                    if( ang < 0. ) ang += 360.;

                    val[i] = SUToString( vkn , pPlugIn->GetSpeedUnit() );

                }
            }
            //t.Printf(_T("%03d Deg"), (int)(ang));
            //val[i+1] = t;
        }

        //update controls values
        m_pT0TextCtrl->SetLabel( val[0] );
        m_pT1TextCtrl->SetLabel( val[1] );
        m_pT2TextCtrl->SetLabel( val[2] );
        if( m_NumberDataShown > 3 ) {
        m_pT3TextCtrl->SetLabel( val[3] );
        m_pT4TextCtrl->SetLabel( val[4] );
        m_pT5TextCtrl->SetLabel( val[5] );
        }
        if( m_NumberDataShown > 6) {
        m_pT6TextCtrl->SetLabel( val[6] );
        m_pT7TextCtrl->SetLabel( val[7] );
        m_pT8TextCtrl->SetLabel( val[8] );
        }
    }
}

void GRIBUIDialog::UpdateTrackingLabels( void )
{
    wxArrayString label;
    label.Add( _T(""), m_NumberDataShown );   //alloc numeric data shown number
    int j = 2;                                //label positions 0,1 are systematicaly used for wind so let's start at position 2
    for( size_t i = 3 ; i < m_DataDisplayConfig.Len() ; i++ ) { //config position 0,1,2 are used for graphic so let's start at pos 3
        if( m_DataDisplayConfig.GetChar( i ) == _T('X') ) {
            label[j].Printf( DToString( i + 96 ) );
            if( j == m_NumberDataShown -1 ) break;
            j++;
        }
    }
    m_pT0StaticText->SetLabel( DToString(ID_CB_WINDSP) );
    m_pT1StaticText->SetLabel( DToString(ID_CB_WINDIR) );
    m_pT2StaticText->SetLabel( label[2] );
    if( m_NumberDataShown > 3 ) {
    m_pT3StaticText->SetLabel( label[3] );
    m_pT4StaticText->SetLabel( label[4] );
    m_pT5StaticText->SetLabel( label[5] );
    }
    if( m_NumberDataShown > 6 ) {
    m_pT6StaticText->SetLabel( label[6] );
    m_pT7StaticText->SetLabel( label[7] ); 
    m_pT8StaticText->SetLabel( label[8] );
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

void GRIBUIDialog::OnButtonSettingClick( wxCommandEvent& event )
{
    wxDialog *dialog = new wxDialog( this, wxID_ANY, _("Data Display Preferences"), wxDefaultPosition,
        wxDefaultSize, wxDEFAULT_DIALOG_STYLE );

    int border_size = 4;

    wxBoxSizer* itemBoxSizerDataSelPanel = new wxBoxSizer(wxVERTICAL);
    dialog->SetSizer(itemBoxSizerDataSelPanel);

    //  Grib toolbox icon checkbox
    wxStaticBox* itemStaticBoxSizerGRIBStatic = new wxStaticBox(dialog, wxID_ANY, _("Numeric Data to show in Panel"));
    wxStaticBoxSizer* itemStaticBoxSizerGRIB = new wxStaticBoxSizer(itemStaticBoxSizerGRIBStatic, wxVERTICAL);
    itemBoxSizerDataSelPanel->Add(itemStaticBoxSizerGRIB, 0, wxGROW|wxALL, border_size);

    wxFlexGridSizer *paneldata = new wxFlexGridSizer( 2 );
    itemStaticBoxSizerGRIB->Add( paneldata, 0, wxALL | wxEXPAND, border_size );

    wxCheckBox* m_WindSpeed = new wxCheckBox( dialog, -1, DToString(ID_CB_WINDSP) );
    paneldata->Add(m_WindSpeed, 1, wxALIGN_LEFT|wxALL, border_size);
    m_WindSpeed->SetValue( true );          //always shown
    m_WindSpeed->Disable();

    wxCheckBox* m_WindDir = new wxCheckBox( dialog, -1, DToString(ID_CB_WINDIR) );
    paneldata->Add(m_WindDir, 1, wxALIGN_LEFT|wxALL, border_size);
    m_WindDir->SetValue( true );            //always shown
    m_WindDir->Disable();

    wxCheckBox* m_WindGust = new wxCheckBox( dialog, -1, DToString(ID_CB_WIGUST) );
    paneldata->Add(m_WindGust, 1, wxALIGN_LEFT|wxALL, border_size);

    wxCheckBox* m_Pressure = new wxCheckBox( dialog, -1, DToString(ID_CB_PRESSD) );
    paneldata->Add(m_Pressure, 1, wxALIGN_LEFT|wxALL, border_size);

    wxCheckBox* m_SigWave = new wxCheckBox( dialog, -1, DToString(ID_CB_SIWAVD) );
    paneldata->Add(m_SigWave, 1, wxALIGN_LEFT|wxALL, border_size);

    wxCheckBox* m_TRainfall = new wxCheckBox( dialog, -1, DToString(ID_CB_PRETOD) );
    paneldata->Add(m_TRainfall, 1, wxALIGN_LEFT|wxALL, border_size);

    wxCheckBox* m_CloudCover = new wxCheckBox( dialog, -1, DToString(ID_CB_CLOCVD) );
    paneldata->Add(m_CloudCover, 1, wxALIGN_LEFT|wxALL, border_size);

    wxCheckBox* m_AirTemp2 = new wxCheckBox( dialog, -1, DToString(ID_CB_ATEM2D) );
    paneldata->Add(m_AirTemp2, 1, wxALIGN_LEFT|wxALL, border_size);

    wxCheckBox* m_SeaSurfTemp = new wxCheckBox( dialog, -1, DToString(ID_CB_SEATED) );
    paneldata->Add(m_SeaSurfTemp, 1, wxALIGN_LEFT|wxALL, border_size);

    wxCheckBox* m_CurrVeloc = new wxCheckBox( dialog, -1, DToString(ID_CB_CURRED) );
    paneldata->Add(m_CurrVeloc, 1, wxALIGN_LEFT|wxALL, border_size);

    //Graphic data
    wxStaticBox* itemStaticBoxSizerGraphStatic = new wxStaticBox( dialog, wxID_ANY, _("Graphic Data to show in Map"));
    wxStaticBoxSizer* itemStaticBoxSizerGraph = new wxStaticBoxSizer(itemStaticBoxSizerGraphStatic, wxVERTICAL);
    itemBoxSizerDataSelPanel->Add(itemStaticBoxSizerGraph, 0, wxGROW|wxALL, border_size);

    wxFlexGridSizer *graphdata = new wxFlexGridSizer( 1 );
    itemStaticBoxSizerGraph->Add( graphdata, 0, wxALL | wxEXPAND, border_size );

    wxCheckBox *m_Windgraph = new wxCheckBox( dialog, -1, DToString(ID_CB_WINDM) );
    graphdata->Add(m_Windgraph, 1, wxALIGN_LEFT|wxALL, border_size);

    wxCheckBox *m_Presgraph = new wxCheckBox( dialog, -1, DToString(ID_CB_PRESSM) );
    graphdata->Add(m_Presgraph, 1, wxALIGN_LEFT|wxALL, border_size);

    wxArrayString nmap;
    for( size_t i = 0 ; i < 7 ; i++ ) {
        nmap.Add( DToString( i + ( i == 0 ? 0 : 100 ) ) );
    }
    wxRadioBox *m_GraphMap = new wxRadioBox( dialog, -1, _T(""), wxDefaultPosition, wxSize(-1, -1),
         nmap, 1, wxRA_SPECIFY_COLS );
    graphdata->Add(m_GraphMap, 1, wxALIGN_LEFT|wxALL, border_size);

    wxStdDialogButtonSizer* DialogButtonSizer = dialog->CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    itemBoxSizerDataSelPanel->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    m_WindGust->SetValue( m_DataDisplayConfig.GetChar( ID_CB_WIGUST ) == _T('X') );
    m_Pressure->SetValue( m_DataDisplayConfig.GetChar( ID_CB_PRESSD ) == _T('X') );
    m_SigWave->SetValue( m_DataDisplayConfig.GetChar( ID_CB_SIWAVD ) == _T('X') );
    m_TRainfall->SetValue( m_DataDisplayConfig.GetChar( ID_CB_PRETOD ) == _T('X') );
    m_CloudCover->SetValue( m_DataDisplayConfig.GetChar( ID_CB_CLOCVD ) == _T('X') );
    m_AirTemp2->SetValue( m_DataDisplayConfig.GetChar( ID_CB_ATEM2D ) == _T('X') );
    m_SeaSurfTemp->SetValue( m_DataDisplayConfig.GetChar( ID_CB_SEATED ) == _T('X') );
    m_CurrVeloc->SetValue( m_DataDisplayConfig.GetChar( ID_CB_CURRED ) == _T('X') );

    m_Windgraph->SetValue( m_DataDisplayConfig.GetChar( ID_CB_WINDM ) == _T('X') );
    m_Presgraph->SetValue( m_DataDisplayConfig.GetChar( ID_CB_PRESSM ) == _T('X') );

    wxString s(m_DataDisplayConfig.GetChar( 0 ) );
    long i;s.ToLong( &i );               
    m_GraphMap->SetSelection( i );

    dialog->Fit();
    
    if(dialog->ShowModal() == wxID_OK) {
        m_WindGust->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_WIGUST,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_WIGUST,_T('.') );
        m_Pressure->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_PRESSD,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_PRESSD,_T('.') );
        m_SigWave->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_SIWAVD,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_SIWAVD,_T('.') );
        m_TRainfall->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_PRETOD,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_PRETOD,_T('.') );
        m_CloudCover->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_CLOCVD,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_CLOCVD,_T('.') );
        m_AirTemp2->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_ATEM2D,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_ATEM2D,_T('.') );
        m_SeaSurfTemp->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_SEATED,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_SEATED,_T('.') );
        m_CurrVeloc->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_CURRED,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_CURRED,_T('.') );

        m_Windgraph->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_WINDM,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_WINDM,_T('.') );
        m_Presgraph->IsChecked() ? m_DataDisplayConfig.SetChar( ID_CB_PRESSM,_T('X') ) :
            m_DataDisplayConfig.SetChar( ID_CB_PRESSM,_T('.') );

        m_DataDisplayConfig.SetChar( 0, (char) ( m_GraphMap->GetSelection() +'0') );

        pPlugIn->SetGRIBDataConfig( m_DataDisplayConfig );

        if( GetNumberDataShown() != m_NumberDataShown ) {
            pPlugIn->CreateGribDialog( m_pRecordForecast->GetCurrentSelection(),                
            m_bGRIBActiveFile->GetFileName(), false );
        }
        else {
            UpdateTrackingLabels();
            UpdateTrackingControls();
        }

        SetFactoryOptions();                     // Reload the visibility options
 
    }
}

void GRIBUIDialog::OnButtonOpenFileClick( wxCommandEvent& event )
{
    m_pGribForecastTimer.Stop();
       
    wxFileDialog *dialog = new wxFileDialog(this, _("Select a GRIB file"), pPlugIn->GetGribDirectory(), 
        _T(""), wxT ( "Grib files (*.grb;*.grb.bz2| *.grb;*.grb.bz2"), wxFD_OPEN, wxDefaultPosition,
        wxDefaultSize, _T("File Dialog") ); 

    if( dialog->ShowModal() == wxID_OK ) {
        m_pRecordForecast->Clear();
        m_bGRIBActiveFile = NULL;
        pPlugIn->SetGribDirectory( dialog->GetDirectory() );

        m_bGRIBActiveFile = new GRIBFile( dialog->GetPath(),
            pPlugIn->GetCopyFirstCumRec(), pPlugIn->GetCopyMissWaveRec() );

        wxFileName fn( dialog->GetPath() );
        SetLabel( fn.GetFullName() );

        if( m_bGRIBActiveFile && m_bGRIBActiveFile->IsOK() ) { 
            PopulateComboDataList( 0 );
            ComputeBestForecastForNow();
        } else 
            pPlugIn->GetGRIBOverlayFactory()->SetMessage( m_bGRIBActiveFile->GetLastMessage() );

        DisplayDataGRS();
    }
}

void GRIBUIDialog::OnGRIBForecastChange( wxCommandEvent& event )
{
    m_pGribForecastTimer.Stop();
    DisplayDataGRS( );
}

void GRIBUIDialog::OnButtonNextClick( wxCommandEvent& event )
{
    if( m_pRecordForecast->GetCurrentSelection() == m_pRecordForecast->GetCount() - 1 ) return; //end of list

    m_pGribForecastTimer.Stop();
    m_pRecordForecast->SetSelection( m_pRecordForecast->GetCurrentSelection() + 1 );
    DisplayDataGRS( );
}

void GRIBUIDialog::OnButtonPrevClick( wxCommandEvent& event )
{
    if( m_pRecordForecast->GetCurrentSelection() == 0 ) return;                 //start of list

    m_pGribForecastTimer.Stop();
    m_pRecordForecast->SetSelection( m_pRecordForecast->GetCurrentSelection() - 1 );
    DisplayDataGRS( );
}

void GRIBUIDialog::OnGRIBForecastTimerEvent( wxTimerEvent& event )
{
    ComputeBestForecastForNow();
    DisplayDataGRS();
}

void GRIBUIDialog::OnCanvasRefreshTimerEvent( wxTimerEvent& event )
{
    if( m_height != GetChartbarHeight() ) {
        RequestRefresh( pParent );
        m_height = GetChartbarHeight();
    }
}

void GRIBUIDialog::OnButtonNowClick( wxCommandEvent& event )
{
    ComputeBestForecastForNow();
    DisplayDataGRS();
}

void GRIBUIDialog::OnGribRequestClick(  wxCommandEvent& event )
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
    ShowSendRequest( zone );
}

void GRIBUIDialog::ShowSendRequest( wxString r_zone )
{
    wxString config = pPlugIn->GetMailRequestConfig();
    wxString r_action = wxT("send");
    wxString r_model,r_resolution,r_interval,r_period,r_parameters;
    if( config.GetChar( 1 ) == '0' ) {                          //GFS
        r_model = wxT("GFS:");
        if( config.GetChar( 2 ) == '0' )
            r_resolution = wxT("0.5,0.5");
        else if( config.GetChar( 2 ) == '1' )
            r_resolution = wxT("1.0,1.0");
        else if( config.GetChar( 2 ) == '2' )
            r_resolution = wxT("1.5,1.5");
        else if( config.GetChar( 2 ) == '3' )
            r_resolution = wxT("2.0,2.0");
        r_parameters = wxT("WIND,PRESS");
        if( config.GetChar( 7 ) == 'X' )
            r_parameters.Append( wxT(",WAVES") );
        if( config.GetChar( 8 ) == 'X' )
            r_parameters.Append( wxT(",APCP") );
        if( config.GetChar( 9 ) == 'X' )
            r_parameters.Append( wxT(",TCDC") );
        if( config.GetChar( 10 ) == 'X' )
            r_parameters.Append( wxT(",AIRTMP") );
        if( config.GetChar( 11 ) == 'X' )
            r_parameters.Append( wxT(",SEATMP") );
    } else if( config.GetChar( 1 ) == '1' ) {                   //COAMPS
        r_model = wxT("COAMPS:");
        if( config.GetChar( 2 ) == '0' )
            r_resolution = wxT("0.2,0.2");
        else if( config.GetChar( 2 ) == '1' )
            r_resolution = wxT("0.6,0.6");
        else if( config.GetChar( 2 ) == '2' )
            r_resolution = wxT("1.2,1.2");
        else if( config.GetChar( 2 ) == '3' )
            r_resolution = wxT("2.0,2.0");
        r_parameters = wxT("WIND,PRMSL");
    } else if( config.GetChar( 1 ) == '2' ) {                   //RTOFS
        r_model = wxT("RTOFS:");
        r_resolution = wxT("0.5,0.5");
        r_parameters = wxT("CUR,WTMP");
    }
    if( config.GetChar( 3 ) == '0' )                            //ALL
        r_interval = wxT("0,3,6");
    else if( config.GetChar( 3 ) == '1' )
        r_interval = wxT("0,6,12");
    else if( config.GetChar( 3 ) == '2' )
        r_interval = wxT("0,12,24");
    if( config.GetChar( 4 ) == '0' ) 
        r_period = wxT("..96");
    else if( config.GetChar( 4 ) == '1' ) 
        r_period = wxT("..144");
    else if( config.GetChar( 4 ) == '2' ) 
        r_period = wxT("..192");

    wxString r_separator = wxT("|");
    wxString r_return = wxT("|=\n");
    wxString r_colon = wxT(": ");

    //display request profile
    wxString r_info,i;
    double v;
    r_info.Append( _("eMail To") + r_colon );
    r_info.Append( pPlugIn->GetSaildocAdresse() );
    r_info.Append( wxT("\n") );
    r_info.Append( _("Action") + r_colon );
    r_info.Append( _("Send") );
    r_info.Append( wxT("\n") );
    r_info.Append( _("Model") + r_colon );
    r_info.Append( r_model );
    r_info.Append( wxT("\n") );
    r_info.Append( _("Zone") + r_colon );
    r_info.Append( r_zone );
    r_info.Append( wxT("\n") );
    r_info.Append( _("Resolution") + r_colon );
    r_info.Append( r_resolution.BeforeFirst( ',' ) + _(" Deg") );
    r_info.Append( wxT("\n") );
    r_info.Append( _("Interval") + r_colon );
    i = r_interval.AfterFirst( ',' );
    r_info.Append( i.BeforeLast( ',' ) + _(" h") );
    r_info.Append( wxT("\n") );
    r_info.Append( _("Period") + r_colon );
    r_period.AfterLast( '.' ).ToDouble( &v );
    i.Printf( _T("%.0f " ), v/24. );
    r_info.Append( i );
    r_info.Append( _("Days") );
    r_info.Append( wxT("\n") );
    r_info.Append( _("Parameters") + r_colon );
    r_info.Append( r_parameters );
    
     GribPofileDisplay *r_dialog = new GribPofileDisplay(this, wxID_ANY, r_info );
     int choice = r_dialog->ShowModal();
        if( choice == wxID_OK ) {               //print and send request mail

            wxMailMessage *message = new wxMailMessage( 
            wxT("Grib-Request"),
            pPlugIn->GetSaildocAdresse(),       //to ( saildoc request adresse )
            wxT("send ") + r_model + r_zone + r_separator + r_resolution
            + r_separator + r_interval + r_period + r_return + r_parameters,                    //message
            wxT("")
            );
            wxEmail mail ;
            if(mail.Send( *message ) ) {
                wxMessageDialog *dialog = new wxMessageDialog(this,
#ifdef __WXMSW__
                    _("Your request is ready. An eMail is prepared in your eMail environment.\nYou have just to click 'send' to send it.\nOK to continue ...")
#else
                    _("Your request was sent (if your system has an MTA configured and is able to send mail).\nOK to continue ...")
#endif
                    ,_("eMail"),wxOK );
                dialog->ShowModal();
            }
        }
        else if( choice == wxID_SAVE ) ShowGribReqPrefDialog( r_zone );             //modify request

        r_dialog->Destroy();
}

void GRIBUIDialog::ShowGribReqPrefDialog( wxString zone )
{
    GribReqPrefDialog *req_Dialog = new GribReqPrefDialog( pParent, wxID_ANY, pPlugIn->GetMailRequestConfig() );
    
    if( req_Dialog->ShowModal() == wxID_OK ) {
        wxString config = pPlugIn->GetMailRequestConfig();
        config.SetChar( 1, (char) ( req_Dialog->GetModel() + '0') );
        config.SetChar( 2, (char) ( req_Dialog->GetResolution() + '0') );
        config.SetChar( 3, (char) ( req_Dialog->GetInterval() + '0') );
        config.SetChar( 4, (char) ( req_Dialog->GetHorizon() + '0') );
        config.SetChar( 5, 'X' ); 
        config.SetChar( 6, 'X' ); 
        req_Dialog->GetWaves() ? config.SetChar( 7, 'X' ) : config.SetChar( 7, '.' );
        req_Dialog->GetRain() ? config.SetChar( 8, 'X' ) : config.SetChar( 8, '.' );
        req_Dialog->GetClouds() ? config.SetChar( 9, 'X' ) : config.SetChar( 9, '.' );
        req_Dialog->GetAirTemp() ? config.SetChar( 10, 'X' ) : config.SetChar( 10, '.' );
        req_Dialog->GetSeaTemp() ? config.SetChar( 11, 'X' ) : config.SetChar( 11, '.' );

        pPlugIn->SetMailRequestConfig( config );

        ShowSendRequest( zone );

    }
}

void GRIBUIDialog::CreateActiveFileFromName( wxString filename )
{
    if( !filename.IsEmpty() ) {
        m_bGRIBActiveFile = NULL;
        m_bGRIBActiveFile = new GRIBFile( filename , pPlugIn->GetCopyFirstCumRec(), pPlugIn->GetCopyMissWaveRec() );
    }
}
void GRIBUIDialog::GetFirstrFileInDirectory()
{
    //reinitialise data containers
     m_pRecordForecast->Clear();
     m_bGRIBActiveFile = NULL;
     if( !wxDir::Exists( pPlugIn->GetGribDirectory() ) ) {
         wxStandardPaths path;
         pPlugIn->SetGribDirectory( path.GetDocumentsDir() );
    }
    //    Get an array of GRIB file names in the target directory, not descending into subdirs
    wxArrayString file_array;
    int m_n_files = 0;
    m_n_files = wxDir::GetAllFiles( pPlugIn->GetGribDirectory(), &file_array, _T ( "*.grb" ), wxDIR_FILES );
    m_n_files += wxDir::GetAllFiles( pPlugIn->GetGribDirectory(), &file_array, _T ( "*.grb.bz2" ),
        wxDIR_FILES );
    if( m_n_files ) {
        file_array.Sort( CompareFileStringTime );              //sort the files by File Modification Date  

        m_bGRIBActiveFile = new GRIBFile( file_array[0] ,      //take the younger
            pPlugIn->GetCopyFirstCumRec(), pPlugIn->GetCopyMissWaveRec() );

        wxFileName fn( file_array[0] );
        SetLabel( fn.GetFullName() );

        if( m_bGRIBActiveFile && m_bGRIBActiveFile->IsOK() ) {
            PopulateComboDataList(0);
            ComputeBestForecastForNow();
        } else 
            pPlugIn->GetGRIBOverlayFactory()->SetMessage( m_bGRIBActiveFile->GetLastMessage() );
    } else {
         pPlugIn->GetGRIBOverlayFactory()->SetMessage( _("Warning :  This directory is Empty!") );
         SetLabel( pPlugIn->GetGribDirectory() );
    }
}

void GRIBUIDialog::PopulateComboDataList( int index )
{
    m_pRecordForecast->Clear();

    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();

    for( size_t i = 0; i < rsa->GetCount(); i++ ) {
        wxDateTime t( rsa->Item( i ).m_Reference_Time );

        m_pRecordForecast->Append( TToString( t, pPlugIn->GetTimeZone() ) );
    }
    m_pRecordForecast->SetSelection( index );
}

int  GRIBUIDialog::GetNumberDataShown()
{
    int checked_item = 2;         //wind speed and dir are always shown so a minimum of 2 data displayed
    //the 0,1,2 config positions are used for graphic data so let's start at position 3
    for( size_t i = 3; i < m_DataDisplayConfig.Len(); i++ ) {
        if( m_DataDisplayConfig.GetChar( i ) == _T('X') ) checked_item++ ;
    }
    checked_item = abs( (checked_item + 2) / 3 ) * 3;     //must be rounded at 3,6,9.. ( as multiple of lines nb )
    return checked_item;                     
}

void GRIBUIDialog::ComputeBestForecastForNow()
{
    if( !HasValidData() ) return;                        //no data

    ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
    wxDateTime t1( rsa->Item( 1 ).m_Reference_Time ) ;              //calculate forecast time interval
    wxDateTime t0( rsa->Item( 0 ).m_Reference_Time ) ;
    int d = t1.Subtract(t0).GetMinutes() / 4;                       //get a quarter of the forecast time interval

    for( size_t i = 0; i < rsa->GetCount(); i++ ) {                 //get the more interesting forecast for now
        wxDateTime t( rsa->Item( i ).m_Reference_Time );
        t.MakeFromTimezone( wxDateTime::UTC );
        if( t.IsDST() ) t.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );
        if( (t.Add(wxTimeSpan( 0, d, 0, 0 )).IsEarlierThan( wxDateTime::Now() ) ) ) 
            if( i != ( rsa->GetCount() -1 ) ) continue;             //keep the desired list item 
        m_pRecordForecast->SetSelection( i );
        break;
    }
    m_pGribForecastTimer.Start( FORECAST_TIMER_10MN, wxTIMER_ONE_SHOT );
}

void GRIBUIDialog::DisplayDataGRS()
{
    if( m_bGRIBActiveFile &&  m_bGRIBActiveFile->IsOK() ) {
        ArrayOfGribRecordSets *rsa = m_bGRIBActiveFile->GetRecordSetArrayPtr();
        GribRecordSet *record = &rsa->Item( m_pRecordForecast->GetCurrentSelection() );
        SetGribRecordSet( record );
    } else 
        SetGribRecordSet( NULL );
 }

void GRIBUIDialog::SetGribRecordSet( GribRecordSet *pGribRecordSet )
{
    m_pCurrentGribRecordSet = pGribRecordSet;

    //    Clear all the flags
    m_RS_Idx_WIND_VX = -1;
    m_RS_Idx_WIND_VY = -1;
    m_RS_Idx_WIND_GUST = -1;
    m_RS_Idx_PRESS = -1;
    m_RS_Idx_HTSIGW = -1;
    m_RS_Idx_PRECIP_TOT = -1;
    m_RS_Idx_CLOUD_TOT = -1;
    m_RS_Idx_AIR_TEMP_2M = -1;
    m_RS_Idx_WINDSCAT_VX = -1;
    m_RS_Idx_WINDSCAT_VY = -1;
    m_RS_Idx_SEATEMP = -1;
    m_RS_Idx_SEACURRENT_VX = -1;
    m_RS_Idx_SEACURRENT_VY = -1;

    if( pGribRecordSet && pGribRecordSet != NULL ) {
        //    Inventory this record set
        //          Walk thru the GribRecordSet, flagging existence of various record types
        for( unsigned int i = 0; i < m_pCurrentGribRecordSet->m_GribRecordPtrArray.GetCount();
                i++ ) {
            GribRecord *pGR = m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( i );

            // Wind
            //    Actually need two records to draw the wind arrows
            if( pGR->getDataType() == GRB_WIND_VX ) m_RS_Idx_WIND_VX = i;       
            if( pGR->getDataType() == GRB_WIND_VY ) m_RS_Idx_WIND_VY = i;

            //Wind gust
            if( pGR->getDataType() == GRB_WIND_GUST ) m_RS_Idx_WIND_GUST = i;

            //Pressure
            if( pGR->getDataType() == GRB_PRESSURE ) m_RS_Idx_PRESS = i;

            // Significant Wave Height
            if( pGR->getDataType() == GRB_HTSGW ) m_RS_Idx_HTSIGW = i;

            //Total rainfall
            if( pGR->getDataType() == GRB_PRECIP_TOT ) m_RS_Idx_PRECIP_TOT = i;
             
            //Total cloud cover
            if( pGR->getDataType() == GRB_CLOUD_TOT ) m_RS_Idx_CLOUD_TOT = i;

            //air temperature(2m)
            if( pGR->getDataType() == GRB_TEMP ) m_RS_Idx_AIR_TEMP_2M = i;

            // QuickScat Winds
            if( pGR->getDataType() == GRB_USCT ) m_RS_Idx_WINDSCAT_VX = i;

            if( pGR->getDataType() == GRB_VSCT ) m_RS_Idx_WINDSCAT_VY = i;

            // RTOFS SEATMP
            if( pGR->getDataType() == GRB_WTMP ) m_RS_Idx_SEATEMP = i;

            // RTOFS Sea Current Actually need two records to draw 
            if( pGR->getDataType() == GRB_UOGRD ) m_RS_Idx_SEACURRENT_VX = i;
            if( pGR->getDataType() == GRB_VOGRD ) m_RS_Idx_SEACURRENT_VY = i;

        }
    }

    UpdateTrackingControls();

    if( pGribRecordSet || NULL == pGribRecordSet ) {
        //    Give the overlay factory the GribRecordSet
        pPlugIn->GetGRIBOverlayFactory()->SetGribRecordSet( pGribRecordSet );
    
        SetFactoryOptions();
    }

//      printf("GRIBUI: Requesting Refresh\n");
    //RequestRefresh( pParent );
}

void GRIBUIDialog::SetFactoryOptions()
{
    //    Set the visibility options
   // pPlugIn->GetGRIBOverlayFactory()->SetTimeZone( pPlugIn->GetTimeZone() );
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderWind(  m_DataDisplayConfig.GetChar( ID_CB_WINDM ) == _T('X') );
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderQuickscat(  m_DataDisplayConfig.GetChar( ID_CB_WINDM ) == _T('X') ); // Note that Quickscat display shares with Wind Speed/Dir forecast
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderPressure(  m_DataDisplayConfig.GetChar( ID_CB_PRESSM ) == _T('X') );
    wxString s(m_DataDisplayConfig.GetChar( 0 ) );
    long i;s.ToLong( &i );i += 100;                                   //Graphic data map constants start at 100
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderGMap( i ); 
    pPlugIn->GetGRIBOverlayFactory()->ClearCachedData();

    RequestRefresh( pParent );
}

//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Factory Implementation
//----------------------------------------------------------------------------------------------------------
GRIBOverlayFactory::GRIBOverlayFactory( bool hiDefGraphics )
{
    m_hiDefGraphics = hiDefGraphics;
    m_dFont_map = new wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    m_dFont_war = new wxFont( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    m_pGribRecordSet = NULL;
    m_last_vp_scale = 0.;

    m_pgob_sigwh = NULL;
    m_pgob_crain = NULL;
    m_pgob_train = NULL;
    m_pgob_clocv = NULL;
    m_pgob_atemp2 = NULL;
    m_pgob_seatemp = NULL;
    m_pgob_current = NULL;

    m_bReadyToRender = false;
}

GRIBOverlayFactory::~GRIBOverlayFactory()
{
    delete m_pgob_sigwh;
    delete m_pgob_crain;
    delete m_pgob_train;
    delete m_pgob_clocv;
    delete m_pgob_atemp2;
    delete m_pgob_seatemp;
    delete m_pgob_current;

    for( unsigned int i = 0; i < m_IsobarArray.GetCount(); i++ ) {
        IsoLine *piso = (IsoLine *) m_IsobarArray.Item( i );
        delete piso;
    }

    m_IsobarArray.Empty();

}
void GRIBOverlayFactory::Reset()
{
    m_pGribRecordSet = NULL;

    ClearCachedData();

    //    Clear out the cached isobars
    for( unsigned int i = 0; i < m_IsobarArray.GetCount(); i++ ) {
        IsoLine *piso = (IsoLine *) m_IsobarArray.Item( i );
        delete piso;
    }
    m_IsobarArray.Clear();                            // Will need to rebuild Isobar list

    m_bReadyToRender = false;

}

void GRIBOverlayFactory::SetGribRecordSet( GribRecordSet *pGribRecordSet )
{
    Reset();
    m_pGribRecordSet = pGribRecordSet;

    m_bReadyToRender = true;

}
void GRIBOverlayFactory::ClearCachedData( void )
{
    //    Clear out the cached bitmaps
    delete m_pgob_sigwh;
    m_pgob_sigwh = NULL;

    delete m_pgob_crain;
    m_pgob_crain = NULL;

    delete m_pgob_train;
    m_pgob_train = NULL;

    delete m_pgob_clocv;
    m_pgob_clocv = NULL;

    delete m_pgob_atemp2;
    m_pgob_atemp2 = NULL;

    delete m_pgob_seatemp;
    m_pgob_seatemp = NULL;

    delete m_pgob_current;
    m_pgob_current = NULL;

}

bool GRIBOverlayFactory::RenderGLGribOverlay( wxGLContext *pcontext, PlugIn_ViewPort *vp )
{
    m_pdc = NULL;                  // inform lower layers that this is OpenGL render
    return DoRenderGribOverlay( vp );
}


bool GRIBOverlayFactory::RenderGribOverlay( wxDC &dc, PlugIn_ViewPort *vp )
{
#if wxUSE_GRAPHICS_CONTEXT
    wxMemoryDC *pmdc;
    pmdc = wxDynamicCast(&dc, wxMemoryDC);
    wxGraphicsContext *pgc = wxGraphicsContext::Create( *pmdc );
    m_gdc = pgc;
    m_pdc = &dc;
#else
    m_pdc = &dc;
#endif
    return DoRenderGribOverlay( vp );

}

bool GRIBOverlayFactory::DoRenderGribOverlay( PlugIn_ViewPort *vp )
{
    if( !m_pGribRecordSet ) {//return false;
        DrawMessageWindow( ( m_Message ), vp->pix_width, vp->pix_height, m_dFont_war );
        return false;
    }

    //    If the scale has changed, clear out the cached bitmaps
    if( vp->view_scale_ppm != m_last_vp_scale ) ClearCachedData();

    m_last_vp_scale = vp->view_scale_ppm;

    GribRecord *pGRWindVX = NULL;
    GribRecord *pGRWindVY = NULL;

    GribRecord *pGRCurrentVX = NULL;
    GribRecord *pGRCurrentVY = NULL;

    GribRecord *pGRPressure = NULL;

    int ref_mod = -1;
    int map_mod = -1;
    wxString string_ref;
    wxString string_map;

    m_SecString.Empty();

    //          Walk thru the GribRecordSet, and render each type of record
    for( unsigned int i = 0; i < m_pGribRecordSet->m_GribRecordPtrArray.GetCount(); i++ ) {
        GribRecord *pGR = m_pGribRecordSet->m_GribRecordPtrArray.Item( i );

        //search for the main data center model in wind and pressure 
        if( pGR->getDataType() == GRB_WIND_VX || pGR->getDataType() == GRB_WIND_VX 
            || pGR->getDataType() == GRB_PRESSURE ) {
            ref_mod = pGR->getDataCenterModel();
            string_ref.Printf( _("Ref : ") + MToString( ref_mod ) + TToString( pGR->getRecordRefDate(), m_TimeZone ) );
        }

        // Wind
        //    Actually need two records to draw the wind arrows
        if( m_ben_Wind && ( pGR->getDataType() == GRB_WIND_VX ) ) pGRWindVX = pGR;
     
        else if( m_ben_Wind && ( pGR->getDataType() == GRB_WIND_VY ) ) pGRWindVY = pGR;
    
        //Pressure
        if( m_ben_Pressure && ( pGR->getDataType() == GRB_PRESSURE ) ) pGRPressure = pGR;
  
        // Significant Wave Height
        if( m_ben_GMap == ID_CB_SIWAVM && ( pGR->getDataType() == GRB_HTSGW ) ) {
            RenderGribSigWh( pGR, vp );
            map_mod = pGR->getDataCenterModel();
            string_map = GetRefString( pGR, m_ben_GMap );
        }
        // Wind wave direction
        if( m_ben_GMap == ID_CB_SIWAVM && ( pGR->getDataType() == GRB_WVDIR ) ) {
            RenderGribWvDir( pGR, vp );
            map_mod = pGR->getDataCenterModel();
            string_map = GetRefString( pGR, m_ben_GMap );
        }

        // total rainfall
        if( m_ben_GMap == ID_CB_PRETOM && ( pGR->getDataType() == GRB_PRECIP_TOT ) ) {
            RenderGribTRAIN( pGR, vp );
            map_mod = pGR->getDataCenterModel();
            string_map = GetRefString( pGR, m_ben_GMap );
        }
        // cloud cover
        if( m_ben_GMap == ID_CB_CLOCVM && ( pGR->getDataType() == GRB_CLOUD_TOT ) ) {
            RenderGribCloudCover( pGR, vp );
            map_mod = pGR->getDataCenterModel();
            string_map = GetRefString( pGR, m_ben_GMap );
        }
        // air temperature (2m)
        if( m_ben_GMap == ID_CB_ATEM2M && ( pGR->getDataType() == GRB_TEMP ) ) {
            RenderGribAirTemp2m( pGR, vp );
            map_mod = pGR->getDataCenterModel();
            string_map = GetRefString( pGR, m_ben_GMap );
        }
        //SEATEMP
        if( m_ben_GMap == ID_CB_SEATEM && ( pGR->getDataType() == GRB_WTMP ) ) {
            RenderGribSeaTemp( pGR, vp );
            map_mod = pGR->getDataCenterModel();
            string_map = GetRefString( pGR, m_ben_GMap );
        }
        // RTOFS Current
        //    Actually need two records to render the current speed

        if( m_ben_GMap == ID_CB_CURREM && ( pGR->getDataType() == GRB_UOGRD ) ) {
            map_mod = pGR->getDataCenterModel();
            string_map = GetRefString( pGR, m_ben_GMap );
            if( pGRCurrentVY ) RenderGribCurrent( pGR, pGRCurrentVY, vp );
            else
                pGRCurrentVX = pGR;
        }
            else if( m_ben_GMap == ID_CB_CURREM && ( pGR->getDataType() == GRB_VOGRD ) ) {
                if( pGRCurrentVX ) RenderGribCurrent( pGRCurrentVX, pGR, vp );
                else
                    pGRCurrentVY = pGR;
        }
    }
        //wind and pressure rendering has been postponed after graphic maps to ensure a better rendering
    if( m_ben_Wind && pGRWindVX && pGRWindVY )
        RenderGribWind( pGRWindVX, pGRWindVY, vp );

    if( m_ben_Pressure && pGRPressure )
        RenderGribPressure( pGRPressure , vp );

    //display references
    if( ref_mod == -1 && map_mod == -1 ) {
        DrawMessageWindow( ( _("No Reference. Try another Graphic Data Selection") ),
            vp->pix_width, vp->pix_height, m_dFont_map );
        return false;
    }

    if( !m_SecString.IsEmpty() )
        DrawMessageWindow( m_SecString, vp->pix_width, vp->pix_height, m_dFont_war );
    else {
        if( ref_mod == map_mod || string_map.IsEmpty() ) 
            DrawMessageWindow(  ( string_ref ), vp->pix_width, vp->pix_height, m_dFont_map );
        else {
            if( !string_ref.IsEmpty() ) string_map.Prepend( wxT("\n") );
            DrawMessageWindow( ( string_ref + string_map ), vp->pix_width, vp->pix_height, m_dFont_map );
        }
    }
 
    return true;
}

bool GRIBOverlayFactory::RenderGribWind( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp )
{

    //    Get the the grid
    int imax = pGRX->getNi();                  // Longitude
    int jmax = pGRX->getNj();                  // Latitude

    //    Barbs?
    bool barbs = true;

    //    Set minimum spacing between wind arrows
    int space;

    if( barbs ) space = 30;
    else
        space = 20;

    int oldx = -1000;
    int oldy = -1000;

    wxColour colour;
    GetGlobalColor( _T ( "YELO2" ), &colour );

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRX->getX( i );
        double latl = pGRX->getY( 0 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( abs( pl.x - oldx ) >= space ) {
            oldx = pl.x;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGRX->getX( i );
                double lat = pGRX->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( abs( p.y - oldy ) >= space ) {
                    oldy = p.y;

                    if( PointInLLBox( vp, lon, lat ) || PointInLLBox( vp, lon - 360., lat ) ) {
                        double vx = pGRX->getValue( i, j );
                        double vy = pGRY->getValue( i, j );

                        if( vx != GRIB_NOTDEF && vy != GRIB_NOTDEF ) drawWindArrowWithBarbs( p.x,
                                p.y, vx, vy, ( lat < 0. ), colour );
                    }
                }
            }
        }
    }
    return true;
}

bool GRIBOverlayFactory::RenderGribScatWind( GribRecord *pGRX, GribRecord *pGRY,
        PlugIn_ViewPort *vp )
{

    wxDateTime t( m_pGribRecordSet->m_Reference_Time );

    //    Get the the grid
    int imax = pGRX->getNi();                  // Longitude
    int jmax = pGRX->getNj();                  // Latitude

    //    Barbs?
    bool barbs = true;

    //    Set minimum spacing between wind arrows
    int space;

    if( barbs ) space = 10;
    else
        space = 10;

    int oldx = -1000;
    int oldy = -1000;

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRX->getX( i );
        double latl = pGRX->getY( 0 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( abs( pl.x - oldx ) >= space ) {
            oldx = pl.x;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGRX->getX( i );
                double lat = pGRX->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( abs( p.y - oldy ) >= space ) {
                    oldy = p.y;

                    if( PointInLLBox( vp, lon, lat ) || PointInLLBox( vp, lon - 360., lat ) ) {
                        double vx = pGRX->getValue( i, j );
                        double vy = pGRY->getValue( i, j );
                        double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                        wxColour c = GetQuickscatColor( vkn );

                        if( vx != GRIB_NOTDEF && vy != GRIB_NOTDEF ) drawWindArrowWithBarbs( p.x,
                                p.y, vx, vy, ( lat < 0. ), c );
                    }
                }
            }
        }
    }
    return true;
}

bool GRIBOverlayFactory::RenderGribSigWh( GribRecord *pGR, PlugIn_ViewPort *vp )
{

    bool b_drawn = RenderGribFieldOverlay( pGR, NULL, vp, 4, GENERIC_GRAPHIC_INDEX, &m_pgob_sigwh );

    if( !b_drawn )
        m_SecString =  _("Please Zoom or Scale Out to view suppressed HTSGW GRIB");

    return true;
}

bool GRIBOverlayFactory::RenderGribWvDir( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    //    Get the the grid
    int imax = pGR->getNi();                  // Longitude
    int jmax = pGR->getNj();                  // Latitude

    //    Set minimum spacing between wave arrows
    int space;
    space = 60;

    int oldx = -1000;
    int oldy = -1000;

    wxColour colour;
    GetGlobalColor( _T ( "UBLCK" ), &colour );

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGR->getX( i );
        double latl = pGR->getY( 0 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( abs( pl.x - oldx ) >= space ) {
            oldx = pl.x;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGR->getX( i );
                double lat = pGR->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( abs( p.y - oldy ) >= space ) {
                    oldy = p.y;

                    if( PointInLLBox( vp, lon, lat ) || PointInLLBox( vp, lon - 360., lat ) ) {
                        double dir = pGR->getValue( i, j );

                        if( dir != GRIB_NOTDEF ) drawWaveArrow( p.x, p.y, dir - 90., colour );
                    }
                }
            }
        }
    }

    return true;
}
/*
bool GRIBOverlayFactory::RenderGribCRAIN( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGR, NULL, vp, 4, CRAIN_GRAPHIC_INDEX, &m_pgob_crain );

    if( !b_drawn ) m_SecString = _("Please Zoom or Scale Out to view suppressed CRAIN GRIB");

    return true;
}
*/
bool GRIBOverlayFactory::RenderGribTRAIN( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGR, NULL, vp, 4, TRAIN_GRAPHIC_INDEX, &m_pgob_train );

    if( !b_drawn )
        m_SecString =  _("Please Zoom or Scale Out to view suppressed Total Rainfall GRIB");

    return true;
}

bool GRIBOverlayFactory::RenderGribCloudCover( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGR, NULL, vp, 4, CLOCV_GRAPHIC_INDEX, &m_pgob_clocv );

    if( !b_drawn )
        m_SecString =  _("Please Zoom or Scale Out to view suppressed HTSGW GRIB");
    
    return true;
}

bool GRIBOverlayFactory::RenderGribAirTemp2m( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGR, NULL, vp, 4, ATEMP2_GRAPHIC_INDEX, &m_pgob_atemp2 );

    if( !b_drawn ) 
        m_SecString = _("Please Zoom or Scale Out to view suppressed Air Temp.(2m) GRIB");
   
    return true;
}

bool GRIBOverlayFactory::RenderGribSeaTemp( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGR, NULL, vp, 4, SEATEMP_GRAPHIC_INDEX, &m_pgob_seatemp );

    if( !b_drawn ) 
        m_SecString = _("Please Zoom or Scale Out to view suppressed Sea Surf. Temp. GRIB");

    return true;
}

bool GRIBOverlayFactory::RenderGribCurrent( GribRecord *pGRX, GribRecord *pGRY,
        PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGRX, pGRY, vp, 4, CURRENT_GRAPHIC_INDEX,
            &m_pgob_current );

    if( b_drawn ){

        //    Draw little arrows for current direction
        {
            int width, height;
            if( m_pdc ) {
                width = m_pgob_current->m_pDCBitmap->GetWidth();
                height = m_pgob_current->m_pDCBitmap->GetHeight();
            } else {
                width = m_pgob_current->m_RGBA_width;
                height = m_pgob_current->m_RGBA_height;
            }

            wxPoint porg;
            GetCanvasPixLL( vp, &porg, pGRX->getLatMax(), pGRX->getLonMin() );
            int arrow_pixel_size = 60;
            for( int ipix = 0; ipix < ( width - arrow_pixel_size + 1 ); ipix += arrow_pixel_size ) {
                for( int jpix = 0; jpix < ( height - arrow_pixel_size + 1 ); jpix +=
                        arrow_pixel_size ) {
                    double lat, lon;
                    wxPoint p;
                    p.x = ipix + porg.x;
                    p.y = jpix + porg.y;
                    GetCanvasLLPix( vp, p, &lat, &lon );

                    double vx = pGRX->getInterpolatedValue( lon, lat );
                    double vy = pGRY->getInterpolatedValue( lon, lat );

                    if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                        double angle = atan2( vx, vy ) * 180. / PI;
                        drawSingleArrow( p.x, p.y, angle + 90., *wxLIGHT_GREY, 2);
                        drawSingleArrow( p.x + 1, p.y + 1, angle + 90., *wxBLACK, 2);
                    }
                }
            }
        }
    }

    if( !b_drawn )  m_SecString = _("Please Zoom or Scale Out to view suppressed CURRENT GRIB");

    return true;
}

bool GRIBOverlayFactory::RenderGribFieldOverlay( GribRecord *pGRA, GribRecord *pGRB,
        PlugIn_ViewPort *vp, int grib_pixel_size, int colormap_index, GribOverlayBitmap **ppGOB )
{
    wxPoint porg;
    GetCanvasPixLL( vp, &porg, pGRA->getLatMax(), pGRA->getLonMin() );

    //    Check two BBoxes....
    //    TODO Make a better Intersect method
    bool bdraw = false;
    if( Intersect( vp, pGRA->getLatMin(), pGRA->getLatMax(), pGRA->getLonMin(), pGRA->getLonMax(),
            30. ) != _OUT ) bdraw = true;
    if( Intersect( vp, pGRA->getLatMin(), pGRA->getLatMax(), pGRA->getLonMin() - 360.,
            pGRA->getLonMax() - 360., 30. ) != _OUT ) bdraw = true;

    if( bdraw ) {
        // If needed, create the bitmap
        if( !*ppGOB ) {
            *ppGOB = new GribOverlayBitmap;
        }

        GribOverlayBitmap *pGOB = *ppGOB;

        if( m_pdc == NULL )       //OpenGL mode
        {
            if( pGOB->m_pRGBA == NULL ) {
                wxImage bl_image = CreateGribImage( pGRA, pGRB, vp, grib_pixel_size, colormap_index,
                        porg );
                //  Create the RGBA buffer
                if( bl_image.IsOk() ) CreateRGBAfromImage( &bl_image, pGOB );
            }

            if( pGOB->m_pRGBA ) {
                DrawGLRGBA( pGOB->m_pRGBA, pGOB->m_RGBA_width, pGOB->m_RGBA_height, porg.x,
                        porg.y );
                return true;
            }
        } else        //DC mode
        {
            if( pGOB->m_pDCBitmap == NULL ) {
                wxImage bl_image = CreateGribImage( pGRA, pGRB, vp, grib_pixel_size, colormap_index,
                        porg );
                if( bl_image.IsOk() ) {
                    //    Create a Bitmap
                    pGOB->m_pDCBitmap = new wxBitmap( bl_image );
                    wxMask *gr_mask = new wxMask( *( pGOB->m_pDCBitmap ), wxColour( 0, 0, 0 ) );
                    pGOB->m_pDCBitmap->SetMask( gr_mask );

                }
            }

            if( pGOB->m_pDCBitmap ) {
                m_pdc->DrawBitmap( *( pGOB->m_pDCBitmap ), porg.x, porg.y, true );
                return true;
            }
        }

    }
    return false;
}

wxImage GRIBOverlayFactory::CreateGribImage( GribRecord *pGRA, GribRecord *pGRB,
        PlugIn_ViewPort *vp, int grib_pixel_size, int colormap_index, const wxPoint &porg )
{
    wxPoint pmin;
    GetCanvasPixLL( vp, &pmin, pGRA->getLatMin(), pGRA->getLonMin() );
    wxPoint pmax;
    GetCanvasPixLL( vp, &pmax, pGRA->getLatMax(), pGRA->getLonMax() );

    int width = abs( pmax.x - pmin.x );
    int height = abs( pmax.y - pmin.y );

    //    Dont try to create enormous GRIB bitmaps
    if( ( width < 2000 ) && ( height < 2000 ) ) {
        //    This could take a while....
        wxImage gr_image( width, height );
        gr_image.InitAlpha();

        wxPoint p;
        for( int ipix = 0; ipix < ( width - grib_pixel_size + 1 ); ipix += grib_pixel_size ) {
            for( int jpix = 0; jpix < ( height - grib_pixel_size + 1 ); jpix += grib_pixel_size ) {
                double lat, lon;
                p.x = ipix + porg.x;
                p.y = jpix + porg.y;
                GetCanvasLLPix( vp, p, &lat, &lon );

                double vkn = 0.;
                bool n_def = true;
                if( pGRB )                // two component, e.g. velocity
                {
                    double vx = pGRA->getInterpolatedValue( lon, lat );
                    double vy = pGRB->getInterpolatedValue( lon, lat );

                    if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                        vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                        n_def = false;
                    }
                } else {
                    double vx = pGRA->getInterpolatedValue( lon, lat );
                    if( vx != GRIB_NOTDEF ) {
                        vkn = vx;
                        n_def = false;
                    }
                }

                if( !n_def ) {
                    wxColour c;
                    if( colormap_index == CURRENT_GRAPHIC_INDEX ) c = GetSeaCurrentGraphicColor(
                            vkn );
                    else if( colormap_index == SEATEMP_GRAPHIC_INDEX ) c = GetTempGraphicColor(
                            vkn );                
                    else if( colormap_index == CRAIN_GRAPHIC_INDEX ) c = wxColour(
                            (unsigned char) vkn * 255, 0, 0 );
                    else if( colormap_index == TRAIN_GRAPHIC_INDEX ) c = GetTotRainGraphicColor(
                            vkn );
                    else if( colormap_index == CLOCV_GRAPHIC_INDEX ) c = GetCloudCoverGraphicColor(
                            vkn );
                    else if( colormap_index == ATEMP2_GRAPHIC_INDEX ) c = GetTempGraphicColor(
                            vkn );
                    else
                        c = GetGenericGraphicColor( vkn );

                    unsigned char r = c.Red();
                    unsigned char g = c.Green();
                    unsigned char b = c.Blue();

                    for( int xp = 0; xp < grib_pixel_size; xp++ )
                        for( int yp = 0; yp < grib_pixel_size; yp++ ) {
                            gr_image.SetRGB( ipix + xp, jpix + yp, r, g, b );
                            gr_image.SetAlpha( ipix + xp, jpix + yp, 180 );
                        }
                } else {
                    for( int xp = 0; xp < grib_pixel_size; xp++ )
                        for( int yp = 0; yp < grib_pixel_size; yp++ ) {
                            gr_image.SetAlpha( ipix + xp, jpix + yp, 0 );
                        }
                }
            }
        }

        wxImage bl_image = gr_image.Blur( 4 );
        return bl_image;
    } else
        return wxNullImage;
}

wxString GRIBOverlayFactory::GetRefString( GribRecord *rec, int map )
{
        wxString string = DToString( map );
        if( rec->isDuplicated() ) string.Append(_(" (Dup)") );
        string.Append( _T(" ") );
        string.Append( _("Ref : ") );
        string.Append( MToString( rec->getDataCenterModel() ) );
        string.Append( TToString( rec->getRecordRefDate(), m_TimeZone ) );

        return string;
}

wxColour GRIBOverlayFactory::GetGenericGraphicColor( double val )
{

    //    HTML colors taken from NOAA WW3 Web representation

    wxColour c;
    if( ( val > 0 ) && ( val < 1 ) ) c.Set( _T("#002ad9") );
    else if( ( val >= 1 ) && ( val < 2 ) ) c.Set( _T("#006ed9") );
    else if( ( val >= 2 ) && ( val < 3 ) ) c.Set( _T("#00b2d9") );
    else if( ( val >= 3 ) && ( val < 4 ) ) c.Set( _T("#00d4d4") );
    else if( ( val >= 4 ) && ( val < 5 ) ) c.Set( _T("#00d9a6") );
    else if( ( val >= 5 ) && ( val < 7 ) ) c.Set( _T("#00d900") );
    else if( ( val >= 7 ) && ( val < 9 ) ) c.Set( _T("#95d900") );
    else if( ( val >= 9 ) && ( val < 12 ) ) c.Set( _T("#d9d900") );
    else if( ( val >= 12 ) && ( val < 15 ) ) c.Set( _T("#d9ae00") );
    else if( ( val >= 15 ) && ( val < 18 ) ) c.Set( _T("#d98300") );
    else if( ( val >= 18 ) && ( val < 21 ) ) c.Set( _T("#d95700") );
    else if( ( val >= 21 ) && ( val < 24 ) ) c.Set( _T("#d90000") );
    else if( ( val >= 24 ) && ( val < 27 ) ) c.Set( _T("#ae0000") );
    else if( ( val >= 27 ) && ( val < 30 ) ) c.Set( _T("#8c0000") );
    else if( ( val >= 30 ) && ( val < 36 ) ) c.Set( _T("#870000") );
    else if( ( val >= 36 ) && ( val < 42 ) ) c.Set( _T("#690000") );
    else if( ( val >= 42 ) && ( val < 48 ) ) c.Set( _T("#550000") );
    else if( val >= 48 ) c.Set( _T("#410000") );

    return c;
}

wxColour GRIBOverlayFactory::GetQuickscatColor( double val )
{

    wxColour c;
    if( ( val > 0 ) && ( val < 5 ) ) c.Set( _T("#000000") );
//      else if((val > 1)  && (val < 2))   c.Set(_T("#006ed9"));
    else if( ( val >= 5 ) && ( val < 10 ) ) c.Set( _T("#00b2d9") );
    else if( ( val >= 10 ) && ( val < 20 ) ) c.Set( _T("#00d4d4") );
//      else if((val > 4)  && (val < 5))   c.Set(_T("#00d9a6"));
    else if( ( val >= 15 ) && ( val < 20 ) ) c.Set( _T("#00d900") );
//      else if((val > 7)  && (val < 9))   c.Set(_T("#95d900"));
    else if( ( val >= 20 ) && ( val < 25 ) ) c.Set( _T("#d9d900") );
//      else if((val > 12) && (val < 15))  c.Set(_T("#d9ae00"));
//      else if((val > 15) && (val < 18))  c.Set(_T("#d98300"));
    else if( ( val >= 25 ) && ( val < 30 ) ) c.Set( _T("#d95700") );
//      else if((val >=30) && (val < 35))  c.Set(_T("#d90000"));
    else if( ( val >= 30 ) && ( val < 35 ) ) c.Set( _T("#ae0000") );
//      else if((val > 27) && (val < 30))  c.Set(_T("#8c0000"));
    else if( ( val >= 35 ) && ( val < 40 ) ) c.Set( _T("#870000") );
//      else if((val > 36) && (val < 42))  c.Set(_T("#690000"));
//      else if((val >=40) && (val < 48))  c.Set(_T("#550000"));
    else if( val >= 40 ) c.Set( _T("#414100") );

    return c;
}
/*
wxColour GRIBOverlayFactory::GetSeaTempGraphicColor( double val_in )
{

    //    HTML colors taken from NOAA WW3 Web representation

    double val = val_in - 273.0;

    val -= 15.;

    val *= 50. / 15.;

    val = wxMax(val, 0.0);

    wxColour c;
    if( ( val >= 0 ) && ( val < 1 ) ) c.Set( _T("#002ad9") );
    else if( ( val >= 1 ) && ( val < 2 ) ) c.Set( _T("#006ed9") );
    else if( ( val >= 2 ) && ( val < 3 ) ) c.Set( _T("#00b2d9") );
    else if( ( val >= 3 ) && ( val < 4 ) ) c.Set( _T("#00d4d4") );
    else if( ( val >= 4 ) && ( val < 5 ) ) c.Set( _T("#00d9a6") );
    else if( ( val >= 5 ) && ( val < 7 ) ) c.Set( _T("#00d900") );
    else if( ( val >= 7 ) && ( val < 9 ) ) c.Set( _T("#95d900") );
    else if( ( val >= 9 ) && ( val < 12 ) ) c.Set( _T("#d9d900") );
    else if( ( val >= 12 ) && ( val < 15 ) ) c.Set( _T("#d9ae00") );
    else if( ( val >= 15 ) && ( val < 18 ) ) c.Set( _T("#d98300") );
    else if( ( val >= 18 ) && ( val < 21 ) ) c.Set( _T("#d95700") );
    else if( ( val >= 21 ) && ( val < 24 ) ) c.Set( _T("#d90000") );
    else if( ( val >= 24 ) && ( val < 27 ) ) c.Set( _T("#ae0000") );
    else if( ( val >= 27 ) && ( val < 30 ) ) c.Set( _T("#8c0000") );
    else if( ( val >= 30 ) && ( val < 36 ) ) c.Set( _T("#870000") );
    else if( ( val >= 36 ) && ( val < 42 ) ) c.Set( _T("#690000") );
    else if( ( val >= 42 ) && ( val < 48 ) ) c.Set( _T("#550000") );
    else if( val >= 48 )                     c.Set( _T("#410000") );

    return c;
}
*/
wxColour GRIBOverlayFactory::GetTempGraphicColor( double val_in )
{

    //    HTML colors taken from zygrib representation
     
    double val = val_in - 273.15;
    wxColour c;
    if(         val < -50 )                    c.Set( _T("#283282") );
    else if( ( val >= -50 ) && ( val < -45 ) ) c.Set( _T("#273c8c") );
    else if( ( val >= -45 ) && ( val < -40 ) ) c.Set( _T("#264696") );
    else if( ( val >= -40 ) && ( val < -36 ) ) c.Set( _T("#2350a0") );
    else if( ( val >= -36 ) && ( val < -32 ) ) c.Set( _T("#1f5aaa") );
    else if( ( val >= -32 ) && ( val < -28 ) ) c.Set( _T("#1a64b4") );
    else if( ( val >= -28 ) && ( val < -24 ) ) c.Set( _T("#136ec8") );
    else if( ( val >= -24 ) && ( val < -21 ) ) c.Set( _T("#0c78e1") );
    else if( ( val >= -21 ) && ( val < -18 ) ) c.Set( _T("#0382e6") );
    else if( ( val >= -18 ) && ( val < -15 ) ) c.Set( _T("#0091e6") );
    else if( ( val >= -15 ) && ( val < -12 ) ) c.Set( _T("#009ee1") );
    else if( ( val >= -12 ) && ( val < -9  ) ) c.Set( _T("#00a6dc") );
    else if( ( val >= -9  ) && ( val < -6  ) ) c.Set( _T("#00b2d7") );
    else if( ( val >= -6  ) && ( val < -3  ) ) c.Set( _T("#00bed2") );
    else if( ( val >= -3  ) && ( val < 0   ) ) c.Set( _T("#28c8c8") );
    else if( ( val >= 0   ) && ( val < 3   ) ) c.Set( _T("#78d2aa") );
    else if( ( val >= 3   ) && ( val < 6   ) ) c.Set( _T("#8cdc78") );
    else if( ( val >= 6   ) && ( val < 9   ) ) c.Set( _T("#a0eb5f") );
    else if( ( val >= 9   ) && ( val < 12  ) ) c.Set( _T("#c8f550") );
    else if( ( val >= 12  ) && ( val < 15  ) ) c.Set( _T("#f3fb02") );
    else if( ( val >= 15  ) && ( val < 18  ) ) c.Set( _T("#ffed00") );
    else if( ( val >= 18  ) && ( val < 21  ) ) c.Set( _T("#ffdd00") );
    else if( ( val >= 21  ) && ( val < 24  ) ) c.Set( _T("#ffc900") );
    else if( ( val >= 24  ) && ( val < 28  ) ) c.Set( _T("#ffab00") );
    else if( ( val >= 28  ) && ( val < 32  ) ) c.Set( _T("#ff8100") );
    else if( ( val >= 32  ) && ( val < 36  ) ) c.Set( _T("#f1780c") );
    else if( ( val >= 36  ) && ( val < 40  ) ) c.Set( _T("#e26a23") );
    else if( ( val >= 40  ) && ( val < 45  ) ) c.Set( _T("#d5453c") );
    else if( val >= 45    )                    c.Set( _T("#b53c59") );

    return c;
}

wxColour GRIBOverlayFactory::GetSeaCurrentGraphicColor( double val_in )
{
    //    HTML colors taken from NOAA WW3 Web representation

    double val = val_in;
    val *= 50. / 2.;

    val = wxMax(val, 0.0);

    wxColour c;
    if( ( val >= 0 ) && ( val < 1 ) ) c.Set( _T("#002ad9") );
    else if( ( val >= 1 ) && ( val < 2 ) ) c.Set( _T("#006ed9") );
    else if( ( val >= 2 ) && ( val < 3 ) ) c.Set( _T("#00b2d9") );
    else if( ( val >= 3 ) && ( val < 4 ) ) c.Set( _T("#00d4d4") );
    else if( ( val >= 4 ) && ( val < 5 ) ) c.Set( _T("#00d9a6") );
    else if( ( val >= 5 ) && ( val < 7 ) ) c.Set( _T("#00d900") );
    else if( ( val >= 7 ) && ( val < 9 ) ) c.Set( _T("#95d900") );
    else if( ( val >= 9 ) && ( val < 12 ) ) c.Set( _T("#d9d900") );
    else if( ( val >= 12 ) && ( val < 15 ) ) c.Set( _T("#d9ae00") );
    else if( ( val >= 15 ) && ( val < 18 ) ) c.Set( _T("#d98300") );
    else if( ( val >= 18 ) && ( val < 21 ) ) c.Set( _T("#d95700") );
    else if( ( val >= 21 ) && ( val < 24 ) ) c.Set( _T("#d90000") );
    else if( ( val >= 24 ) && ( val < 27 ) ) c.Set( _T("#ae0000") );
    else if( ( val >= 27 ) && ( val < 30 ) ) c.Set( _T("#8c0000") );
    else if( ( val >= 30 ) && ( val < 36 ) ) c.Set( _T("#870000") );
    else if( ( val >= 36 ) && ( val < 42 ) ) c.Set( _T("#690000") );
    else if( ( val >= 42 ) && ( val < 48 ) ) c.Set( _T("#550000") );
    else if( val >= 48 ) c.Set( _T("#410000") );

    return c;
}

wxColour GRIBOverlayFactory::GetTotRainGraphicColor( double val )
{

    //    HTML colors taken from ZyGrib representation

    wxColour c;
    if       ( val <  0.01 )                     c.Set( _T("#ffffff") );
    else if( ( val >= 0.01 ) && ( val < 0.02 ) ) c.Set( _T("#c8f0ff") );
    else if( ( val >= 0.02 ) && ( val < 0.05 ) ) c.Set( _T("#b4e6ff") );
    else if( ( val >= 0.05 ) && ( val < 0.07 ) ) c.Set( _T("#8cd3ff") );
    else if( ( val >= 0.07 ) && ( val < 0.1  ) ) c.Set( _T("#78caff") );
    else if( ( val >= 0.1  ) && ( val < 0.2  ) ) c.Set( _T("#6ec1ff") );
    else if( ( val >= 0.2  ) && ( val < 0.5  ) ) c.Set( _T("#64b8ff") );
    else if( ( val >= 0.5  ) && ( val < 0.7  ) ) c.Set( _T("#50a6ff") );
    else if( ( val >= 0.7  ) && ( val < 1.0  ) ) c.Set( _T("#469eff") );
    else if( ( val >= 1.0  ) && ( val < 2.0  ) ) c.Set( _T("#3c96ff") );
    else if( ( val >= 2.0  ) && ( val < 5.0  ) ) c.Set( _T("#328eff") );
    else if( ( val >= 5.0  ) && ( val < 7.0  ) ) c.Set( _T("#1e7eff") );
    else if( ( val >= 7.0  ) && ( val < 10.0 ) ) c.Set( _T("#1476f0") );
    else if( ( val >= 10.0 ) && ( val < 20.0 ) ) c.Set( _T("#0a6edc") );
    else if( ( val >= 20.0 ) && ( val < 50.0 ) ) c.Set( _T("#0064c8") );
    else if(   val >= 50   )                     c.Set( _T("#0052aa") );

    return c;
}

wxColour GRIBOverlayFactory::GetCloudCoverGraphicColor( double val )
{

    //    HTML colors taken from black ZyGrib representation

    wxColour c;
    if     (   val <= 0  )                   c.Set( _T("#ffffff") );
    else if( ( val > 0   ) && ( val < 10 ) ) c.Set( _T("#f0f0e6") );
    else if( ( val >= 10 ) && ( val < 20 ) ) c.Set( _T("#e6e6dc") );
    else if( ( val >= 20 ) && ( val < 30 ) ) c.Set( _T("#dcdcd2") );
    else if( ( val >= 30 ) && ( val < 40 ) ) c.Set( _T("#c8c8b4") );
    else if( ( val >= 40 ) && ( val < 50 ) ) c.Set( _T("#aaaa8c") );
    else if( ( val >= 50 ) && ( val < 60 ) ) c.Set( _T("#969678") );
    else if( ( val >= 60 ) && ( val < 70 ) ) c.Set( _T("#787864") );
    else if( ( val >= 70 ) && ( val < 80 ) ) c.Set( _T("#646450") );
    else if( ( val >= 80 ) && ( val < 90 ) ) c.Set( _T("#5a5a46") );
    else if(   val >= 90 )                   c.Set( _T("#505036") );

    return c;
}

bool GRIBOverlayFactory::RenderGribPressure( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    //    Initialize the array of Isobars if necessary
    if( !m_IsobarArray.GetCount() ) {
        IsoLine *piso;
        for( double press = 840; press < 1120; press += 2/*isobarsStep*/) {
            piso = new IsoLine( press * 100, pGR );
            m_IsobarArray.Add( piso );
        }
    }

    //    Draw the Isobars
    for( unsigned int i = 0; i < m_IsobarArray.GetCount(); i++ ) {
        IsoLine *piso = (IsoLine *) m_IsobarArray.Item( i );
        if( m_pdc ) piso->drawIsoLine( this, *m_pdc, vp, true, true ); //g_bGRIBUseHiDef
        else
            piso->drawGLIsoLine( this, vp, true, true ); //g_bGRIBUseHiDef

        // Draw Isobar labels
        wxColour text_color;
        GetGlobalColor( _T ( "DILG3" ), &text_color );

        wxColour back_color;
        GetGlobalColor( _T ( "DILG0" ), &back_color );

        int density = 40;
        int first = 0;

        double coef = .01;
        if( m_pdc ) piso->drawIsoLineLabels( this, *m_pdc, text_color, back_color, vp, density,
                first, coef );
        else
            piso->drawGLIsoLineLabels( this, text_color, back_color, vp, density, first, coef );

        //

    }
    return true;
}

#if 0
bool GRIBOverlayFactory::RenderGLGribPressure(GribRecord *pGR, wxGLContext *pcontext, PlugIn_ViewPort *vp)
{

    //    Initialize the array of Isobars if necessary
    if(!m_IsobarArray.GetCount())
    {
        IsoLine *piso;
        for (double press=840; press<1120; press += 2) // 2 = isobarsStep
        {
            piso = new IsoLine(press*100, pGR);
            m_IsobarArray.Add(piso);
        }
    }

    //    Draw the Isobars
    for(unsigned int i = 0; i < m_IsobarArray.GetCount(); i++)
    {
        IsoLine *piso = (IsoLine *)m_IsobarArray.Item(i);
        if(m_pdc)
        piso->drawGLIsoLine(this, pcontext, vp, true, true); //g_bGRIBUseHiDef

        // Draw Isobar labels
        int gr = 80;
        wxColour color = wxColour(gr,gr,gr);
        int density = 40;//40;
        int first = 0;

        double coef = .01;
        piso->drawGLIsoLineLabels(this, pcontext, color, vp, density, first, coef);

    }
    return true;
}

#endif

void GRIBOverlayFactory::drawWaveArrow( int i, int j, double ang, wxColour arrowColor )
{
    double si = sin( ang * PI / 180. ), co = cos( ang * PI / 180. );

    wxPen pen( arrowColor, 1 );

    if( m_pdc && m_pdc->IsOk() ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    int arrowSize = 26;
    int dec = -arrowSize / 2;

    drawTransformedLine( pen, si, co, i, j, dec, -2, dec + arrowSize, -2 );
    drawTransformedLine( pen, si, co, i, j, dec, 2, dec + arrowSize, +2 );

    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, 6 );    // flèche
    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, -6 );   // flèche

}

void GRIBOverlayFactory::drawSingleArrow( int i, int j, double ang, wxColour arrowColor, int width )
{
    double si = sin( ang * PI / 180. ), co = cos( ang * PI / 180. );

    wxPen pen( arrowColor, width );

    if( m_pdc && m_pdc->IsOk() ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    int arrowSize = 26;
    int dec = -arrowSize / 2;

    drawTransformedLine( pen, si, co, i, j, dec, 0, dec + arrowSize, 0 );

    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, 6 );    // flèche
    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, -6 );   // flèche

}

void GRIBOverlayFactory::drawWindArrowWithBarbs( int i, int j, double vx, double vy, bool south,
        wxColour arrowColor )
{
    double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
    double ang = atan2( vy, -vx );
    double si = sin( ang ), co = cos( ang );

    wxPen pen( arrowColor, 2 );

    if( m_pdc && m_pdc->IsOk() ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    if( vkn < 1 ) {
        int r = 5;     // wind is very light, draw a circle
        if( m_pdc && m_pdc->IsOk() ) m_pdc->DrawCircle( i, j, r );
    } else {
        // Arrange for arrows to be centered on origin
        int windBarbuleSize = 26;
        int dec = -windBarbuleSize / 2;
        drawTransformedLine( pen, si, co, i, j, dec, 0, dec + windBarbuleSize, 0 );   // hampe
        drawTransformedLine( pen, si, co, i, j, dec, 0, dec + 5, 2 );    // flèche
        drawTransformedLine( pen, si, co, i, j, dec, 0, dec + 5, -2 );   // flèche

        int b1 = dec + windBarbuleSize - 4;  // position de la 1ère barbule
        if( vkn >= 7.5 && vkn < 45 ) {
            b1 = dec + windBarbuleSize;  // position de la 1ère barbule si >= 10 noeuds
        }

        if( vkn < 7.5 ) {  // 5 ktn
            drawPetiteBarbule( pen, south, si, co, i, j, b1 );
        } else if( vkn < 12.5 ) { // 10 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
        } else if( vkn < 17.5 ) { // 15 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawPetiteBarbule( pen, south, si, co, i, j, b1 - 4 );
        } else if( vkn < 22.5 ) { // 20 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
        } else if( vkn < 27.5 ) { // 25 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
            drawPetiteBarbule( pen, south, si, co, i, j, b1 - 8 );
        } else if( vkn < 32.5 ) { // 30 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
        } else if( vkn < 37.5 ) { // 35 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
            drawPetiteBarbule( pen, south, si, co, i, j, b1 - 12 );
        } else if( vkn < 45 ) { // 40 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 12 );
        } else if( vkn < 55 ) { // 50 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
        } else if( vkn < 65 ) { // 60 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
        } else if( vkn < 75 ) { // 70 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 12 );
        } else if( vkn < 85 ) { // 80 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 12 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 16 );
        } else { // > 90 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
            drawTriangle( pen, south, si, co, i, j, b1 - 12 );
        }
    }
}

void GRIBOverlayFactory::drawTransformedLine( wxPen pen, double si, double co, int di, int dj,
        int i, int j, int k, int l )
{
    int ii, jj, kk, ll;
    double fi, fj, fk, fl; // For Hi Def Graphics.

    fi = ( i * co - j * si + 0.5 ) + di;
    fj = ( i * si + j * co + 0.5 ) + dj;
    fk = ( k * co - l * si + 0.5 ) + di;
    fl = ( k * si + l * co + 0.5 ) + dj;

    ii = fi; jj = fj; kk = fk; ll = fl;

    if( m_pdc && m_pdc->IsOk() ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
#if wxUSE_GRAPHICS_CONTEXT
        if( m_hiDefGraphics && m_gdc ) {
            m_gdc->SetPen( pen );
            m_gdc->StrokeLine( fi, fj, fk, fl );
        }
        else {
            m_pdc->DrawLine( ii, jj, kk, ll );
        }
#else
        m_pdc->DrawLine(ii, jj, kk, ll);
#endif
    } else {                       // OpenGL mode
        wxColour c = pen.GetColour();
        glColor4ub( c.Red(), c.Green(), c.Blue(), 255);
        double w = pen.GetWidth();
        if( m_hiDefGraphics ) w *= 0.7;
        DrawGLLine( fi, fj, fk, fl, w );
    }
}

void GRIBOverlayFactory::drawPetiteBarbule( wxPen pen, bool south, double si, double co, int di,
        int dj, int b )
{
    if( south ) drawTransformedLine( pen, si, co, di, dj, b, 0, b + 2, -5 );
    else
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 2, 5 );
}

void GRIBOverlayFactory::drawGrandeBarbule( wxPen pen, bool south, double si, double co, int di,
        int dj, int b )
{
    if( south ) drawTransformedLine( pen, si, co, di, dj, b, 0, b + 4, -10 );
    else
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 4, 10 );
}

void GRIBOverlayFactory::drawTriangle( wxPen pen, bool south, double si, double co, int di, int dj,
        int b )
{
    if( south ) {
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 4, -10 );
        drawTransformedLine( pen, si, co, di, dj, b + 8, 0, b + 4, -10 );
    } else {
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 4, 10 );
        drawTransformedLine( pen, si, co, di, dj, b + 8, 0, b + 4, 10 );
    }
}

void GRIBOverlayFactory::DrawGLLine( double x1, double y1, double x2, double y2, double width )
{
    {
        glPushAttrib(
                GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_HINT_BIT ); //Save state
        {

            glDisable( GL_MULTISAMPLE );

            //      Enable anti-aliased lines, at best quality
            glEnable( GL_LINE_SMOOTH );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
            glLineWidth( width );

            glBegin( GL_LINES );
            glVertex2d( x1, y1 );
            glVertex2d( x2, y2 );
            glEnd();
        }

        glPopAttrib();
    }
}

void GRIBOverlayFactory::DrawOLBitmap( const wxBitmap &bitmap, wxCoord x, wxCoord y, bool usemask )
{
    wxBitmap bmp;
    if( x < 0 || y < 0 ) {
        int dx = ( x < 0 ? -x : 0 );
        int dy = ( y < 0 ? -y : 0 );
        int w = bitmap.GetWidth() - dx;
        int h = bitmap.GetHeight() - dy;
        /* picture is out of viewport */
        if( w <= 0 || h <= 0 ) return;
        wxBitmap newBitmap = bitmap.GetSubBitmap( wxRect( dx, dy, w, h ) );
        x += dx;
        y += dy;
        bmp = newBitmap;
    } else {
        bmp = bitmap;
    }
    if( m_pdc && m_pdc->IsOk() ) m_pdc->DrawBitmap( bmp, x, y, usemask );
    else {
        wxImage image = bmp.ConvertToImage();
        int w = image.GetWidth(), h = image.GetHeight();

        if( usemask ) {
            unsigned char *d = image.GetData();
            unsigned char *a = image.GetAlpha();

            unsigned char mr, mg, mb;
            if( !image.GetOrFindMaskColour( &mr, &mg, &mb ) && !a ) printf(
                    "trying to use mask to draw a bitmap without alpha or mask\n" );

            unsigned char *e = new unsigned char[4 * w * h];
            {
                for( int y = 0; y < h; y++ )
                    for( int x = 0; x < w; x++ ) {
                        unsigned char r, g, b;
                        int off = ( y * image.GetWidth() + x );
                        r = d[off * 3 + 0];
                        g = d[off * 3 + 1];
                        b = d[off * 3 + 2];

                        e[off * 4 + 0] = r;
                        e[off * 4 + 1] = g;
                        e[off * 4 + 2] = b;

                        e[off * 4 + 3] =
                                a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                    }
            }

            glColor4f( 1, 1, 1, 1 );

            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 );
            glDrawPixels( w, h, GL_RGBA, GL_UNSIGNED_BYTE, e );
            glPixelZoom( 1, 1 );
            glDisable( GL_BLEND );

            delete[] ( e );
        } else {
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 ); /* draw data from top to bottom */
            glDrawPixels( w, h, GL_RGB, GL_UNSIGNED_BYTE, image.GetData() );
            glPixelZoom( 1, 1 );
        }
    }
}

void GRIBOverlayFactory::DrawGLImage( wxImage *pimage, wxCoord xd, wxCoord yd, bool usemask )
{
    int w = pimage->GetWidth(), h = pimage->GetHeight();
    int x_offset = 0;
    int y_offset = 0;

    /*
     if(xd < 0)
     {
     x_offset = -xd;
     w += xd;
     }
     if(yd < 0)
     {
     y_offset = -yd;
     h += yd;
     }
     */
    unsigned char *d = pimage->GetData();
    unsigned char *a = pimage->GetAlpha();

    unsigned char *e = new unsigned char[4 * w * h];
    {
        for( int y = 0; y < h; y++ )
            for( int x = 0; x < w; x++ ) {
                unsigned char r, g, b;
                int off = ( ( y + y_offset ) * pimage->GetWidth() + x + x_offset );
                r = d[off * 3 + 0];
                g = d[off * 3 + 1];
                b = d[off * 3 + 2];

                int doff = ( y * w + x );
                e[doff * 4 + 0] = r;
                e[doff * 4 + 1] = g;
                e[doff * 4 + 2] = b;

                e[doff * 4 + 3] = a ? a[off] : 255;
            }
    }

    DrawGLRGBA( e, w, h, xd, yd );
    /*
     glColor4f(1, 1, 1, 1);

     glEnable(GL_BLEND);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glPixelZoom(1, -1);

     glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

     glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
     if(xd < 0)
     {
     x_offset = -xd;
     w += xd;
     }
     if(yd < 0)
     {
     y_offset = -yd;
     h += yd;
     }

     glRasterPos2i(xd + x_offset, yd + y_offset);

     glPixelStorei(GL_UNPACK_SKIP_PIXELS, x_offset);
     glPixelStorei(GL_UNPACK_SKIP_ROWS, y_offset);

     glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, e);
     glPixelZoom(1, 1);
     glDisable(GL_BLEND);

     glPopClientAttrib();
     */
    delete[] e;
}

void GRIBOverlayFactory::DrawGLRGBA( unsigned char *pRGBA, int RGBA_width, int RGBA_height, int xd,
        int yd )
{
    int x_offset = 0;
    int y_offset = 0;
    int draw_width = RGBA_width;
    int draw_height = RGBA_height;

    glColor4f( 1, 1, 1, 1 );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glPixelZoom( 1, -1 );

    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, RGBA_width );
    if( xd < 0 ) {
        x_offset = -xd;
        draw_width += xd;
    }
    if( yd < 0 ) {
        y_offset = -yd;
        draw_height += yd;
    }

    glRasterPos2i( xd + x_offset, yd + y_offset );

    glPixelStorei( GL_UNPACK_SKIP_PIXELS, x_offset );
    glPixelStorei( GL_UNPACK_SKIP_ROWS, y_offset );

    glDrawPixels( draw_width, draw_height, GL_RGBA, GL_UNSIGNED_BYTE, pRGBA );
    glPixelZoom( 1, 1 );
    glDisable( GL_BLEND );

    glPopClientAttrib();

}

void GRIBOverlayFactory::DrawMessageWindow( wxString msg, int x, int y , wxFont *mfont)
{
    wxMemoryDC mdc;
    wxBitmap bm( 1000, 1000 );
    mdc.SelectObject( bm );
    mdc.Clear();

    //wxFont mfont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    mdc.SetFont( *mfont );
    mdc.SetPen( *wxTRANSPARENT_PEN);
   // mdc.SetBrush( *wxLIGHT_GREY_BRUSH );
    mdc.SetBrush( wxColour(243, 229, 47 ) );
    int w, h;
    mdc.GetMultiLineTextExtent( msg, &w, &h );
    h += 2;
    int label_offset = 10;
    int wdraw = w + ( label_offset * 2 );
    mdc.DrawRectangle( 0, 0, wdraw, h );

    mdc.DrawLabel( msg, wxRect( label_offset, 0, wdraw, h ), wxALIGN_LEFT| wxALIGN_CENTRE_VERTICAL);
    mdc.SelectObject( wxNullBitmap );

    wxBitmap sbm = bm.GetSubBitmap( wxRect( 0, 0, wdraw, h ) );

    DrawOLBitmap( sbm, 0, y - ( GetChartbarHeight() + h ), false );
    
}

void GRIBOverlayFactory::CreateRGBAfromImage( wxImage *pimage, GribOverlayBitmap *pGOB )
{
    if( !pGOB ) return;

    unsigned char *d = pimage->GetData();
    unsigned char *a = pimage->GetAlpha();
    int width = pimage->GetWidth();
    int height = pimage->GetHeight();

    pGOB->m_pRGBA = new unsigned char[4 * width * height];
    pGOB->m_RGBA_width = width;
    pGOB->m_RGBA_height = height;

    unsigned char *pdest = pGOB->m_pRGBA;
    if( !pdest ) return;

    for( int y = 0; y < height; y++ )
        for( int x = 0; x < width; x++ ) {
            unsigned char r, g, b;
            int off = ( y * width + x );
            r = d[off * 3 + 0];
            g = d[off * 3 + 1];
            b = d[off * 3 + 2];

            pdest[off * 4 + 0] = r;
            pdest[off * 4 + 1] = g;
            pdest[off * 4 + 2] = b;

            pdest[off * 4 + 3] = a ? a[off] : 255;
        }

}

//----------------------------------------------------------------------------------------------------------
//          GRIBFile Object Implementation
//----------------------------------------------------------------------------------------------------------

GRIBFile::GRIBFile( const wxString file_name, bool CumRec, bool WaveRec )
{
    m_bOK = true;           // Assume ok until proven otherwise

    if( !::wxFileExists( file_name ) ) {
        m_last_message = _T ( "Error:  File does not exist!" );
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
                    m_GribRecordSetArray.Item( j ).m_GribRecordPtrArray.Add( pRec );
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

// Calculates if two boxes intersect. If so, the function returns _ON.
// If they do not intersect, two scenario's are possible:
// other is outside this -> return _OUT
// other is inside this -> return _IN
OVERLAP Intersect( PlugIn_ViewPort *vp, double lat_min, double lat_max, double lon_min,
        double lon_max, double Marge )
{

    if( ( ( vp->lon_min - Marge ) > ( lon_max + Marge ) )
            || ( ( vp->lon_max + Marge ) < ( lon_min - Marge ) )
            || ( ( vp->lat_max + Marge ) < ( lat_min - Marge ) )
            || ( ( vp->lat_min - Marge ) > ( lat_max + Marge ) ) ) return _OUT;

    // Check if other.bbox is inside this bbox
    if( ( vp->lon_min <= lon_min ) && ( vp->lon_max >= lon_max ) && ( vp->lat_max >= lat_max )
            && ( vp->lat_min <= lat_min ) ) return _IN;

    // Boundingboxes intersect
    return _ON;
}

// Is the given point in the vp ??
bool PointInLLBox( PlugIn_ViewPort *vp, double x, double y )
{

    if( x >= ( vp->lon_min ) && x <= ( vp->lon_max ) && y >= ( vp->lat_min )
            && y <= ( vp->lat_max ) ) return TRUE;
    return FALSE;
}

//-------------------------------------------------------------------------------------------------------
//Request Profile Display implementation
//---------------------------------------------------------------------------------------------------------
GribPofileDisplay::GribPofileDisplay( wxWindow *parent, wxWindowID id, wxString profile ):
    wxDialog( parent, id, _("Request Pofile"), wxDefaultPosition,  wxSize( 200, 200 ),
        wxCAPTION )
{
    int border_size = 5;

    wxBoxSizer* itemBoxSizerDisplay = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizerDisplay);

    wxStaticBox* itemStaticBoxSizerDisplayStatic = new wxStaticBox(this, wxID_ANY, wxT("") );
    wxStaticBoxSizer* itemStaticBoxSizerDisplay = new wxStaticBoxSizer(itemStaticBoxSizerDisplayStatic, wxVERTICAL);
    itemBoxSizerDisplay->Add(itemStaticBoxSizerDisplay, 0, wxGROW|wxALL, border_size);

    wxStaticText *pProfile = new wxStaticText( this, wxID_ANY, _T("Model"), wxDefaultPosition, wxSize( -1, -1), wxALIGN_LEFT );
    itemStaticBoxSizerDisplay->Add( pProfile, 0, wxTOP, border_size );
    pProfile->SetLabel( profile );

    wxStaticBox* itemStaticBoxSizerButtonStatic = new wxStaticBox(this, wxID_ANY, wxT("") );
    wxStaticBoxSizer* itemStaticBoxSizerButton = new wxStaticBoxSizer(itemStaticBoxSizerButtonStatic, wxVERTICAL);
    itemBoxSizerDisplay->Add(itemStaticBoxSizerButton, 0, wxGROW|wxALL, border_size);

    wxButton *pModify = new wxButton( this, wxID_SAVE, _("Modify"), wxDefaultPosition, wxDefaultSize, 0);
    pModify->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GribPofileDisplay::OnModifyButtonClick), NULL, this );

    wxButton *pSend = new wxButton( this, wxID_OK, _("Send"), wxDefaultPosition, wxDefaultSize, 0);

    wxStdDialogButtonSizer* DialogButtonSizer = this->CreateStdDialogButtonSizer(wxCANCEL);
    DialogButtonSizer->SetAffirmativeButton( pSend );
    DialogButtonSizer->Realize();
    DialogButtonSizer->Add(pModify);
    itemStaticBoxSizerButton->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, border_size);

    Fit();
}

void GribPofileDisplay::OnModifyButtonClick( wxCommandEvent &event )
{
    EndModal( wxID_SAVE );
}


//-------------------------------------------------------------------------------------------------------
//Request Preferences Dialog implementation
//---------------------------------------------------------------------------------------------------------
GribReqPrefDialog::GribReqPrefDialog( wxWindow *parent, wxWindowID id, wxString config ):
    wxDialog( parent, id, _("Request Setting"), wxDefaultPosition, wxSize( -1, -1),
        wxCAPTION )
{
    int border_size = 5;
    long i;
    wxString s;
    m_DataRequestConfig = config;
    wxBoxSizer* itemBoxSizerRGRIBPanel = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizerRGRIBPanel);

     //  General request Setting
    wxStaticBox* itemStaticBoxSizerRGRIBStatic = new wxStaticBox(this, wxID_ANY, _("Profil"));
    wxStaticBoxSizer* itemStaticBoxSizerRGRIB = new wxStaticBoxSizer(itemStaticBoxSizerRGRIBStatic, wxVERTICAL);
    itemBoxSizerRGRIBPanel->Add(itemStaticBoxSizerRGRIB, 0, wxGROW|wxALL, border_size);

    m_pTopSizer = new wxFlexGridSizer(2);
    itemStaticBoxSizerRGRIB->Add( m_pTopSizer, 0, wxALL | wxEXPAND, border_size );

    wxStaticText* m_model_text = new wxStaticText( this, wxID_ANY, _("Forecast Model   "), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_model_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    const wxString model[] = { wxT("NOAA_GFS"), wxT("NOAA_COAMPS") ,wxT("NOAA_RTOFS") };
    m_pModel = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, model, wxALIGN_LEFT );
    m_pModel->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribReqPrefDialog::OnChoiceChange), NULL, this );
    m_pTopSizer->Add( m_pModel, 0, wxTOP, border_size );
    s = config.GetChar(1);
    s.ToLong( &i );
    m_pModel->SetSelection( i );

    wxStaticText* m_Resolution_text = new wxStaticText( this, wxID_ANY, _("Resolution"), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_Resolution_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    if( i == 1 )
        m_pResolution = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, resolution2, wxALIGN_LEFT );
    else
        m_pResolution = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, resolution1, wxALIGN_LEFT );
    
    if( i == 2 ) {
        m_pResolution->SetSelection( 0 );
        m_pResolution->Enable( false );
    } else {
        s = config.GetChar(2);
        s.ToLong( &i );
        m_pResolution->SetSelection( i );
    }
    m_pTopSizer->Add( m_pResolution , 0, wxTOP, border_size );

    wxStaticText* m_Interval_text = new wxStaticText( this, wxID_ANY, _("Interval"), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_Interval_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    const wxString interval[] = { _("3 h"), _("6 h"), _("12 h"), _("24 h") };
    m_pInterval = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, interval, wxALIGN_LEFT );
    m_pTopSizer->Add( m_pInterval, 0, wxTOP, border_size );
   s = config.GetChar(3);
    s.ToLong( &i );
    m_pInterval->SetSelection( i );

    wxStaticText* m_Horizon_text = new wxStaticText( this, wxID_ANY, _T("Horizon"), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_Horizon_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    const wxString horizon[] = { _("4 days"), _("6 days"), _("8 days") };
    m_pHorizon = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, horizon, wxALIGN_LEFT );
    m_pTopSizer->Add( m_pHorizon, 0, wxTOP, border_size );
    s = config.GetChar(4);
    s.ToLong( &i );
    m_pHorizon->SetSelection( i );

    //  data request Setting
    wxStaticBox* itemStaticBoxSizerRGRIB2Static = new wxStaticBox(this, wxID_ANY, _("Data"));
    wxStaticBoxSizer* itemStaticBoxSizerRGRIB2 = new wxStaticBoxSizer(itemStaticBoxSizerRGRIB2Static, wxVERTICAL);
    itemBoxSizerRGRIBPanel->Add(itemStaticBoxSizerRGRIB2, 0, wxGROW|wxALL, border_size);

    wxFlexGridSizer *pTopSizer2 = new wxFlexGridSizer( 2 );
    itemStaticBoxSizerRGRIB2->Add( pTopSizer2, 0, wxALL | wxEXPAND, border_size );

    m_pWind = new wxCheckBox( this, -1, _("Wind"));
    pTopSizer2->Add( m_pWind, 1, wxALIGN_LEFT|wxALL, border_size );
    m_pWind->SetValue( true );
    m_pWind->Enable( false );

    m_pPress = new wxCheckBox( this, -1, _("Pressure"));
    pTopSizer2->Add( m_pPress, 1, wxALIGN_LEFT|wxALL, border_size );
    m_pPress->SetValue( true );
    m_pPress->Enable( false );
  
    m_pWaves = new wxCheckBox( this, -1, _("Waves"));
    pTopSizer2->Add( m_pWaves, 1, wxALIGN_LEFT|wxALL, border_size );
    m_pWaves->SetValue( config.GetChar(7) == 'X' );
    if( m_pModel->GetCurrentSelection() == 1 || m_pModel->GetCurrentSelection() == 2 ) m_pWaves->Enable( false );

    m_pRainfall = new wxCheckBox( this, -1, _("Rainfall"));
    pTopSizer2->Add( m_pRainfall, 1, wxALIGN_LEFT|wxALL, border_size );
    m_pRainfall->SetValue( config.GetChar(8) == 'X' );
    if( m_pModel->GetCurrentSelection() == 1 || m_pModel->GetCurrentSelection() == 2 ) m_pRainfall->Enable( false );

    m_pCloudCover = new wxCheckBox( this, -1, _("Clouds Cover"));
    pTopSizer2->Add( m_pCloudCover, 1, wxALIGN_LEFT|wxALL, border_size );
    m_pCloudCover->SetValue( config.GetChar(9) == 'X' );
    if( m_pModel->GetCurrentSelection() == 1 || m_pModel->GetCurrentSelection() == 2 ) m_pCloudCover->Enable( false );

    m_pAirTemp = new wxCheckBox( this, -1, _("Air Temperature(2m)"));
    pTopSizer2->Add( m_pAirTemp, 1, wxALIGN_LEFT|wxALL, border_size );
    m_pAirTemp->SetValue( config.GetChar(10) == 'X' );
    if( m_pModel->GetCurrentSelection() == 1 || m_pModel->GetCurrentSelection() == 2 ) m_pAirTemp->Enable( false );

    m_pSeaTemp = new wxCheckBox( this, -1, _("Sea Temperature(surf)"));
    pTopSizer2->Add( m_pSeaTemp, 1, wxALIGN_LEFT|wxALL, border_size );
    m_pSeaTemp->SetValue( config.GetChar(11) == 'X' );
    if( m_pModel->GetCurrentSelection() == 1 || m_pModel->GetCurrentSelection() == 2 ) m_pSeaTemp->Enable( false );

    wxStaticBox* itemStaticBoxSizerButtonStatic = new wxStaticBox(this, wxID_ANY, wxT("") );
    wxStaticBoxSizer* itemStaticBoxSizerButton = new wxStaticBoxSizer(itemStaticBoxSizerButtonStatic, wxVERTICAL);
    itemBoxSizerRGRIBPanel->Add(itemStaticBoxSizerButton, 0, wxGROW|wxALL, border_size);

    wxStdDialogButtonSizer* DialogButtonSizer = this->CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    itemStaticBoxSizerButton->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, border_size);

    Fit();
}

void GribReqPrefDialog::OnChoiceChange(wxCommandEvent &event)
{
    int choice = m_pResolution->GetCurrentSelection();
    if( m_pModel->GetSelection() == 1 ) {
        for( int i = 0; i<4; i++ ){
            m_pResolution->SetString(i,resolution2[i]);
        }
    } else {
        for( int i = 0; i<4; i++ ){
            m_pResolution->SetString(i,resolution1[i]);
        }
    }
    if( m_pModel->GetSelection() == 2 ) {
        m_pResolution->SetSelection( 0 );
        m_pResolution->Enable( false );
    } else {
        m_pResolution->SetSelection( choice );
        m_pResolution->Enable( true );
    }
    if( m_pModel->GetSelection() == 1 || m_pModel->GetSelection() == 2 ) {
        m_pWaves->SetValue( false );
        m_pWaves->Enable( false );
        m_pRainfall->SetValue( false );
        m_pRainfall->Enable( false );
        m_pCloudCover->SetValue( false );
        m_pCloudCover->Enable( false );
        m_pAirTemp->SetValue( false );
        m_pAirTemp->Enable( false );
        m_pSeaTemp->SetValue( false );
        m_pSeaTemp->Enable( false );
    } else {
        m_pWaves->SetValue( m_DataRequestConfig.GetChar( 7 ) == 'X' );
        m_pWaves->Enable( true );
        m_pRainfall->SetValue( m_DataRequestConfig.GetChar( 8 ) == 'X' );
        m_pRainfall->Enable( true );
        m_pCloudCover->SetValue( m_DataRequestConfig.GetChar( 9 ) == 'X' );
        m_pCloudCover->Enable( true );
        m_pAirTemp->SetValue( m_DataRequestConfig.GetChar( 10 ) == 'X' );
        m_pAirTemp->Enable( true );
        m_pSeaTemp->SetValue( m_DataRequestConfig.GetChar( 11 ) == 'X' );
        m_pSeaTemp->Enable( true );
    }
}
