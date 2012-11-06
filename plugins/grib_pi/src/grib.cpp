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

/*
 extern FontMgr          *pFontMgr;
 extern ColorScheme      global_color_scheme;

 extern GRIBUIDialog     *g_pGribDialog;
 extern int              g_grib_dialog_x, g_grib_dialog_y;
 extern int              g_grib_dialog_sx, g_grib_dialog_sy;
 extern wxString         g_grib_dir;

 extern ChartCanvas     *cc1;
 */
//extern bool             g_bGRIBUseHiDef;

#include "folder.xpm"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( ArrayOfGribRecordSets );
WX_DEFINE_OBJARRAY( ArrayOfGribRecordPtrs );

//static GRIBOverlayFactory   *s_pGRIBOverlayFactory;
/*
 static bool GRIBOverlayFactory_RenderGribOverlay_Static_Wrapper ( wxDC *dc, PlugIn_ViewPort *vp )
 {
 return s_pGRIBOverlayFactory->RenderGribOverlay ( dc, vp );

 }
 */

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
IMPLEMENT_CLASS ( GRIBUIDialog, wxDialog )

BEGIN_EVENT_TABLE ( GRIBUIDialog, wxDialog )

EVT_CLOSE ( GRIBUIDialog::OnClose )
//            EVT_BUTTON ( ID_OK, GRIBUIDialog::OnIdOKClick )
EVT_MOVE ( GRIBUIDialog::OnMove )
EVT_SIZE ( GRIBUIDialog::OnSize )
EVT_BUTTON ( ID_CHOOSEGRIBDIR, GRIBUIDialog::OnChooseDirClick )
EVT_CHECKBOX(ID_CB_WINDSPEED, GRIBUIDialog::OnCBWindspeedClick)
EVT_CHECKBOX(ID_CB_WINDDIR, GRIBUIDialog::OnCBWinddirClick)
EVT_CHECKBOX(ID_CB_PRESS, GRIBUIDialog::OnCBPressureClick)
EVT_CHECKBOX(ID_CB_SIGHW, GRIBUIDialog::OnCBSigHwClick)
EVT_CHECKBOX(ID_CB_SEATMP, GRIBUIDialog::OnCBSeatempClick)
EVT_CHECKBOX(ID_CB_SEACURRENT, GRIBUIDialog::OnCBSeaCurrentClick)

END_EVENT_TABLE()

GRIBUIDialog::GRIBUIDialog()
{
//      printf("GRIBUIDialog ctor\n");
    Init();
}

GRIBUIDialog::~GRIBUIDialog()
{
}

void GRIBUIDialog::Init()
{
    m_sequence_active = -1;
    m_pCurrentGribRecordSet = NULL;
    m_pRecordTree = NULL;

    m_pWindSpeedTextCtrl = NULL;
    m_pWindDirTextCtrl = NULL;
    m_pPressureTextCtrl = NULL;
    m_pSigWHTextCtrl = NULL;
    m_pSeaTmpTextCtrl = NULL;
    m_pSeaCurrentTextCtrl = NULL;
}

bool GRIBUIDialog::Create( wxWindow *parent, grib_pi *ppi, wxWindowID id, const wxString& caption,
        const wxString initial_grib_dir, const wxPoint& pos, const wxSize& size, long style )
{
//      printf("GRIBUIDialog::Create\n");

    pParent = parent;
    pPlugIn = ppi;

    m_currentGribDir = initial_grib_dir;

    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxDEFAULT_FRAME_STYLE;
//      if ( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY ) && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) )
//            wstyle |= ( wxNO_BORDER );

    wxSize size_min = size;
//      size_min.IncTo ( wxSize ( 500,600 ) );
    if( !wxDialog::Create( parent, id, caption, pos, size_min, wstyle ) ) return false;

    m_pfolder_bitmap = new wxBitmap( folder );   // comes from XPM include

    CreateControls();

    DimeWindow( this );

    Fit();
    SetMinSize( GetBestSize() );

    return true;
}

void GRIBUIDialog::CreateControls()
{
    int border_size = 4;
//      int check_spacing = 4;
    int group_item_spacing = 1;           // use for items within one group, with Add(...wxALL)

// A top-level sizer
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer );

// A second box sizer to give more space around the controls
    wxBoxSizer* boxSizer = new wxBoxSizer( wxVERTICAL );
    topSizer->Add( boxSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 2 );

//    The GRIB directory
    wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox( this, wxID_ANY,
            _ ( "GRIB File Directory" ) );
    wxStaticBoxSizer *itemStaticBoxSizer11 = new wxStaticBoxSizer( itemStaticBoxSizer11Static,
            wxHORIZONTAL );
    boxSizer->Add( itemStaticBoxSizer11, 0, wxEXPAND );

    m_pitemCurrentGribDirectoryCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition,
            wxDefaultSize, wxTE_READONLY );
    itemStaticBoxSizer11->Add( m_pitemCurrentGribDirectoryCtrl, 1, wxALIGN_LEFT | wxALL, 5 );

    m_pitemCurrentGribDirectoryCtrl->AppendText( m_currentGribDir );

    wxButton* bChooseDir = new wxBitmapButton( this, ID_CHOOSEGRIBDIR, *m_pfolder_bitmap );
    itemStaticBoxSizer11->Add( bChooseDir, 0, wxALIGN_RIGHT | wxALL, 5 );

//  The Tree control
    m_pRecordTree = new GribRecordTree( this, ID_GRIBRECORDREECTRL, wxDefaultPosition,
            wxSize( -1, 200 ), wxTR_HAS_BUTTONS );
    boxSizer->Add( m_pRecordTree, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 2 );

    m_RecordTree_root_id = m_pRecordTree->AddRoot( _T ( "GRIBs" ) );
    PopulateTreeControl();
    m_pRecordTree->Expand( m_RecordTree_root_id );
    m_pRecordTree->SelectItem( m_RecordTree_root_id );

//      Data Box
    wxStaticBox* itemStaticBoxData = new wxStaticBox( this, wxID_ANY, _("GRIB Data") );
    wxStaticBoxSizer* itemStaticBoxSizerData = new wxStaticBoxSizer( itemStaticBoxData,
            wxVERTICAL );
    boxSizer->Add( itemStaticBoxSizerData, 0, wxALL | wxEXPAND, border_size );

    wxFlexGridSizer *pDataGrid = new wxFlexGridSizer( 3 );
    pDataGrid->AddGrowableCol( 1 );
    itemStaticBoxSizerData->Add( pDataGrid, 0, wxALL | wxEXPAND, border_size );

    m_cbWindSpeed.Create( this, ID_CB_WINDSPEED, _T("") );
    m_cbWindSpeed.SetValue( true );
    pDataGrid->Add( &m_cbWindSpeed, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    wxString wind_speed_str = _("Wind Speed, Kts.");
    if( pPlugIn->GetUseMS() ) wind_speed_str = _("Wind Speed, m/sec.");

    wxStaticText *ps1 = new wxStaticText( this, wxID_ANY, wind_speed_str );
    pDataGrid->Add( ps1, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    m_pWindSpeedTextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxDefaultSize,
            wxTE_READONLY );
    pDataGrid->Add( m_pWindSpeedTextCtrl, 0, wxALIGN_RIGHT, group_item_spacing );

    m_cbWindDir.Create( this, ID_CB_WINDDIR, _T("") );
    m_cbWindDir.SetValue( true );
    pDataGrid->Add( &m_cbWindDir, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    wxStaticText *ps2 = new wxStaticText( this, wxID_ANY, _("Wind Direction") );
    pDataGrid->Add( ps2, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    m_pWindDirTextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxDefaultSize,
            wxTE_READONLY );
    pDataGrid->Add( m_pWindDirTextCtrl, 0, wxALIGN_RIGHT, group_item_spacing );

    m_cbPress.Create( this, ID_CB_PRESS, _T("") );
    m_cbPress.SetValue( true );
    pDataGrid->Add( &m_cbPress, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    wxStaticText *ps3 = new wxStaticText( this, wxID_ANY, _("Pressure, mBar") );
    pDataGrid->Add( ps3, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    m_pPressureTextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxDefaultSize,
            wxTE_READONLY );
    pDataGrid->Add( m_pPressureTextCtrl, 0, wxALIGN_RIGHT, group_item_spacing );

    m_cbSigHw.Create( this, ID_CB_SIGHW, _T("") );
    m_cbSigHw.SetValue( true );
    pDataGrid->Add( &m_cbSigHw, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    wxStaticText *ps4 = new wxStaticText( this, wxID_ANY, _("Significant Wave Height, m") );
    pDataGrid->Add( ps4, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    m_pSigWHTextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxDefaultSize,
            wxTE_READONLY );
    pDataGrid->Add( m_pSigWHTextCtrl, 0, wxALIGN_RIGHT, group_item_spacing );

    m_cbSeaTmp.Create( this, ID_CB_SEATMP, _T("") );
    m_cbSeaTmp.SetValue( true );
    pDataGrid->Add( &m_cbSeaTmp, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    wxStaticText *ps5 = new wxStaticText( this, wxID_ANY, _("Sea Surface Temp, C") );
    pDataGrid->Add( ps5, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    m_pSeaTmpTextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxDefaultSize,
            wxTE_READONLY );
    pDataGrid->Add( m_pSeaTmpTextCtrl, 0, wxALIGN_RIGHT, group_item_spacing );

    m_cbSeaCurrent.Create( this, ID_CB_SEACURRENT, _T("") );
    m_cbSeaCurrent.SetValue( true );
    pDataGrid->Add( &m_cbSeaCurrent, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    wxStaticText *ps6 = new wxStaticText( this, wxID_ANY, _("Current Velocity, Kts.") );
    pDataGrid->Add( ps6, 0, wxALIGN_LEFT | wxALL, group_item_spacing );

    m_pSeaCurrentTextCtrl = new wxTextCtrl( this, -1, _T(""), wxDefaultPosition, wxDefaultSize,
            wxTE_READONLY );
    pDataGrid->Add( m_pSeaCurrentTextCtrl, 0, wxALIGN_RIGHT, group_item_spacing );

// A horizontal box sizer to contain OK
    wxBoxSizer* AckBox = new wxBoxSizer( wxHORIZONTAL );
    boxSizer->Add( AckBox, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

//    Button color
//      wxColour button_color = GetGlobalColor ( _T ( "UIBCK" ) );;

// The OK button
    /*
     wxButton* bOK = new wxButton ( this, ID_OK, _( "&Close" ),
     wxDefaultPosition, wxDefaultSize, 0 );
     AckBox->Add ( bOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
     */
}

void GRIBUIDialog::SetCursorLatLon( double lat, double lon )
{
    m_cursor_lon = lon;
    m_cursor_lat = lat;

    UpdateTrackingControls();
}

void GRIBUIDialog::UpdateTrackingControls( void )
{
    if( m_pWindSpeedTextCtrl ) m_pWindSpeedTextCtrl->Clear();
    if( m_pWindDirTextCtrl ) m_pWindDirTextCtrl->Clear();
    if( m_pPressureTextCtrl ) m_pPressureTextCtrl->Clear();
    if( m_pSigWHTextCtrl ) m_pSigWHTextCtrl->Clear();
    if( m_pSeaTmpTextCtrl ) m_pSeaTmpTextCtrl->Clear();
    if( m_pSeaCurrentTextCtrl ) m_pSeaCurrentTextCtrl->Clear();

    if( m_pCurrentGribRecordSet ) {
        //    Update the wind control
        if( ( m_RS_Idx_WIND_VX != -1 ) && ( m_RS_Idx_WIND_VY != -1 ) ) {
            double vx =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_WIND_VX )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );
            double vy =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_WIND_VY )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );

            if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                if( ang > 360. ) ang -= 360.;
                if( ang < 0. ) ang += 360.;
                if( pPlugIn->GetUseMS() ) vkn *= .5144;

                wxString t;
                t.Printf( _T("%2d"), (int) vkn );
                if( m_pWindSpeedTextCtrl ) m_pWindSpeedTextCtrl->AppendText( t );

                t.Printf( _T("%03d"), (int) ( ang ) );
                if( m_pWindDirTextCtrl ) m_pWindDirTextCtrl->AppendText( t );

            }
        }

        //    Update the Pressure control
        if( m_RS_Idx_PRESS != -1 ) {
            double press =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_PRESS )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );
            if( press != GRIB_NOTDEF ) {
                wxString t;
                t.Printf( _T("%2d"), (int) ( press / 100. ) );
                if( m_pPressureTextCtrl ) m_pPressureTextCtrl->AppendText( t );
            }
        }

        //    Update the Sig Wave Height
        if( m_RS_Idx_HTSIGW != -1 ) {
            double height =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_HTSIGW )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );
            if( height != GRIB_NOTDEF ) {
                wxString t;
                t.Printf( _T("%4.1f"), height );
                if( m_pSigWHTextCtrl ) m_pSigWHTextCtrl->AppendText( t );
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
                double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                if( ang > 360. ) ang -= 360.;
                if( ang < 0. ) ang += 360.;

                wxString t;
                t.Printf( _T("%2d"), (int) vkn );
                if( m_pWindSpeedTextCtrl ) m_pWindSpeedTextCtrl->AppendText( t );

                t.Printf( _T("%03d"), (int) ( ang ) );
                if( m_pWindDirTextCtrl ) m_pWindDirTextCtrl->AppendText( t );

            }
        }

        //    Update the SEATEMP
        if( m_RS_Idx_SEATEMP != -1 ) {
            double temp =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_SEATEMP )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );

            if( temp != GRIB_NOTDEF ) {
                temp -= 273.15;
//                        std::cout << "temp";
                wxString t;
                t.Printf( _T("%6.2f"), temp );
                if( m_pSeaTmpTextCtrl ) m_pSeaTmpTextCtrl->AppendText( t );
            }
        }

        //    Update the Current control
        if( ( m_RS_Idx_SEACURRENT_VX != -1 ) && ( m_RS_Idx_SEACURRENT_VY != -1 ) ) {
            double vx =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_SEACURRENT_VX )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );
            double vy =
                    m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( m_RS_Idx_SEACURRENT_VY )->getInterpolatedValue(
                            m_cursor_lon, m_cursor_lat, true );

            if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                double ang = 90. + ( atan2( vy, -vx ) * 180. / PI );
                if( ang > 360. ) ang -= 360.;
                if( ang < 0. ) ang += 360.;

                wxString t;
                t.Printf( _T("%5.2f"), vkn );
                if( m_pWindSpeedTextCtrl ) m_pSeaCurrentTextCtrl->AppendText( t );

                //                       t.Printf(_T("%03d"), (int)(ang));
                //                       if(m_pWindDirTextCtrl)
                //                             m_pWindDirTextCtrl->AppendText(t);

            }
        }

    }
}

void GRIBUIDialog::OnClose( wxCloseEvent& event )
{
    /*
     pPlugIn->SetGribDir(m_currentGribDir);


     RequestRefresh(pParent);

     delete m_pRecordTree;

     delete m_pfolder_bitmap;
     delete m_pitemCurrentGribDirectoryCtrl;

     Destroy();
     */
    pPlugIn->OnGribDialogClose();
}

void GRIBUIDialog::OnIdOKClick( wxCommandEvent& event )
{
    Close();
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

void GRIBUIDialog::OnChooseDirClick( wxCommandEvent& event )
{
    wxString new_dir = ::wxDirSelector( _( "Select GRIB Directory" ), m_currentGribDir );
    if( !new_dir.empty() ) {
        m_currentGribDir = new_dir;
        m_pitemCurrentGribDirectoryCtrl->SetValue( m_currentGribDir );
        m_pitemCurrentGribDirectoryCtrl->SetInsertionPoint( 0 );

        if( m_pRecordTree ) {
            m_pRecordTree->DeleteAllItems();
            delete m_pRecordTree->m_file_id_array;

            m_RecordTree_root_id = m_pRecordTree->AddRoot( _T ( "GRIBs" ) );
            PopulateTreeControl();
            m_pRecordTree->Expand( m_RecordTree_root_id );
        }

        pPlugIn->GetGRIBOverlayFactory()->Reset();

        Refresh();

        pPlugIn->SetGribDir( m_currentGribDir );

    }
}

void GRIBUIDialog::OnCBWindspeedClick( wxCommandEvent& event )
{
    m_cbWindDir.SetValue( event.IsChecked() );
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::OnCBWinddirClick( wxCommandEvent& event )
{
    m_cbWindSpeed.SetValue( event.IsChecked() );
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::OnCBPressureClick( wxCommandEvent& event )
{
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::OnCBSigHwClick( wxCommandEvent& event )
{
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::OnCBSeatempClick( wxCommandEvent& event )
{
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::OnCBSeaCurrentClick( wxCommandEvent& event )
{
    SetFactoryOptions();                     // Reload the visibility options
}

void GRIBUIDialog::PopulateTreeControl()
{
    if( !wxDir::Exists( m_currentGribDir ) ) return;

    //    Get an array of GRIB file names in the target directory, not descending into subdirs
    wxArrayString file_array;

    m_n_files = wxDir::GetAllFiles( m_currentGribDir, &file_array, _T ( "*.grb" ), wxDIR_FILES );
    m_n_files += wxDir::GetAllFiles( m_currentGribDir, &file_array, _T ( "*.grb.bz2" ),
            wxDIR_FILES );
//      m_n_files = wxDir::GetAllFiles ( m_currentGribDir, &file_array, _T ( "*" ), wxDIR_FILES );

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

//            m_pRecordTree->SetItemTextColour(m_pRecordTree->m_file_id_array[i], GetGlobalColor ( _T ( "UBLCK")));
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
    SetGribRecordSet( NULL );

}

void GRIBUIDialog::PopulateTreeControlGRS( GRIBFile *pgribfile, int file_index )
{
    //    Get the array of GribRecordSets, and add one-by-one to the tree,
    //    each under the proper file item.
    ArrayOfGribRecordSets *rsa = pgribfile->GetRecordSetArrayPtr();

//      if(rsa->GetCount() == 0)
//            m_pRecordTree->SetItemTextColour(m_pRecordTree->m_file_id_array[file_index], GetGlobalColor ( _T ( "DILG1")));
//      else
//            m_pRecordTree->SetItemTextColour(m_pRecordTree->m_file_id_array[file_index], GetGlobalColor ( _T ( "BLUE2")));

    for( unsigned int i = 0; i < rsa->GetCount(); i++ ) {
        GribTreeItemData *pmtid = new GribTreeItemData( GRIB_RECORD_SET_TYPE );
        pmtid->m_pGribRecordSet = &rsa->Item( i );

        wxDateTime t( rsa->Item( i ).m_Reference_Time );
        t.MakeFromTimezone( wxDateTime::UTC );
        if( t.IsDST() ) t.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );
//            wxString time_string = t.Format ( "%a %d-%b-%Y %H:%M:%S %Z", wxDateTime::UTC );

        // This is a hack because Windows is broke....
        wxString time_string = t.Format( _T("%a %d-%b-%Y %H:%M:%S "), wxDateTime::Local );
        time_string.Append( _T("Local - ") );
        time_string.Append( t.Format( _T("%a %d-%b-%Y %H:%M:%S "), wxDateTime::UTC ) );
        time_string.Append( _T("GMT") );

        m_pRecordTree->AppendItem( m_pRecordTree->m_file_id_array[file_index], time_string, -1, -1,
                pmtid );
    }

}

void GRIBUIDialog::SetGribRecordSet( GribRecordSet *pGribRecordSet )
{
    m_pCurrentGribRecordSet = pGribRecordSet;

    //    Clear all the flags
    m_RS_Idx_WIND_VX = -1;
    m_RS_Idx_WIND_VY = -1;
    m_RS_Idx_PRESS = -1;
    m_RS_Idx_HTSIGW = -1;
    m_RS_Idx_WINDSCAT_VX = -1;
    m_RS_Idx_WINDSCAT_VY = -1;
    m_RS_Idx_SEATEMP = -1;
    m_RS_Idx_SEACURRENT_VX = -1;
    m_RS_Idx_SEACURRENT_VY = -1;

    if( pGribRecordSet ) {
        //    Inventory this record set
        //          Walk thru the GribRecordSet, flagging existence of various record types
        for( unsigned int i = 0; i < m_pCurrentGribRecordSet->m_GribRecordPtrArray.GetCount();
                i++ ) {
            GribRecord *pGR = m_pCurrentGribRecordSet->m_GribRecordPtrArray.Item( i );

            // Wind
            //    Actually need two records to draw the wind arrows

            if( pGR->getDataType() == GRB_WIND_VX ) m_RS_Idx_WIND_VX = i;

            if( pGR->getDataType() == GRB_WIND_VY ) m_RS_Idx_WIND_VY = i;

            //Pressure
            if( pGR->getDataType() == GRB_PRESSURE ) m_RS_Idx_PRESS = i;

            // Significant Wave Height
            if( pGR->getDataType() == GRB_HTSGW ) m_RS_Idx_HTSIGW = i;

            // QuickScat Winds
            if( pGR->getDataType() == GRB_USCT ) m_RS_Idx_WINDSCAT_VX = i;

            if( pGR->getDataType() == GRB_VSCT ) m_RS_Idx_WINDSCAT_VY = i;

            // GFS SEATMP
//                  if (pGR->getDataType()==GRB_TEMP )
//                        m_RS_Idx_SEATEMP = i;

            // RTOFS SEATMP
            if( pGR->getDataType() == GRB_WTMP ) m_RS_Idx_SEATEMP = i;

            // RTOFS Sea Current
            if( pGR->getDataType() == GRB_UOGRD ) m_RS_Idx_SEACURRENT_VX = i;

            if( pGR->getDataType() == GRB_VOGRD ) m_RS_Idx_SEACURRENT_VY = i;

        }
    }

    if( pGribRecordSet ) {
        //    Give the overlay factory the GribRecordSet
        pPlugIn->GetGRIBOverlayFactory()->SetGribRecordSet( pGribRecordSet );

        SetFactoryOptions();
    }

//      printf("GRIBUI: Requesting Refresh\n");
    RequestRefresh( pParent );

    UpdateTrackingControls();
}

void GRIBUIDialog::SetFactoryOptions()
{
    //    Set the visibility options
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderWind( m_cbWindSpeed.GetValue() );
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderPressure( m_cbPress.GetValue() );
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderSigHw( m_cbSigHw.GetValue() );
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderQuickscat( m_cbWindSpeed.GetValue() ); // Note that Quickscat display shares with Wind Speed/Dir forecast
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderSeatmp( m_cbSeaTmp.GetValue() );
    pPlugIn->GetGRIBOverlayFactory()->EnableRenderSeaCurrent( m_cbSeaCurrent.GetValue() );

    pPlugIn->GetGRIBOverlayFactory()->ClearCachedData();

    RequestRefresh( pParent );

}

//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Factory Implementation
//----------------------------------------------------------------------------------------------------------
GRIBOverlayFactory::GRIBOverlayFactory( bool hiDefGraphics )
{
    m_hiDefGraphics = hiDefGraphics;
    m_pGribRecordSet = NULL;
    m_last_vp_scale = 0.;

    m_pgob_sigwh = NULL;
    m_pgob_crain = NULL;
    m_pgob_seatemp = NULL;
    m_pgob_current = NULL;

    m_bReadyToRender = false;
}

GRIBOverlayFactory::~GRIBOverlayFactory()
{
    delete m_pgob_sigwh;
    delete m_pgob_crain;
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
    if( !m_pGribRecordSet ) return false;

    //    If the scale has changed, clear out the cached bitmaps
    if( vp->view_scale_ppm != m_last_vp_scale ) ClearCachedData();

    m_last_vp_scale = vp->view_scale_ppm;

    GribRecord *pGRWindVX = NULL;
    GribRecord *pGRWindVY = NULL;

    GribRecord *pGRCurrentVX = NULL;
    GribRecord *pGRCurrentVY = NULL;

    //          Walk thru the GribRecordSet, and render each type of record
    for( unsigned int i = 0; i < m_pGribRecordSet->m_GribRecordPtrArray.GetCount(); i++ ) {
        GribRecord *pGR = m_pGribRecordSet->m_GribRecordPtrArray.Item( i );

        // Wind
        //    Actually need two records to draw the wind arrows

        if( m_ben_Wind && ( pGR->getDataType() == GRB_WIND_VX ) ) {
            if( pGRWindVY ) RenderGribWind( pGR, pGRWindVY, vp );
            else
                pGRWindVX = pGR;
        }

        else if( m_ben_Wind && ( pGR->getDataType() == GRB_WIND_VY ) ) {
            if( pGRWindVX ) RenderGribWind( pGRWindVX, pGR, vp );
            else
                pGRWindVY = pGR;
        }

        //Pressure
        if( m_ben_Pressure && ( pGR->getDataType() == GRB_PRESSURE ) ) {
            RenderGribPressure( pGR, vp );
        }

        // Significant Wave Height
        if( m_ben_SigHw && ( pGR->getDataType() == GRB_HTSGW ) ) RenderGribSigWh( pGR, vp );

        // Wind wave direction
        if( m_ben_SigHw && ( pGR->getDataType() == GRB_WVDIR ) ) RenderGribWvDir( pGR, vp );

        // GFS SEATEMP
//            if ( m_ben_Seatmp && (pGR->getDataType()==GRB_TEMP ))
//                  RenderGribSeaTemp(pGR, dc, vp);

        // RTOFS SEATEMP
        if( m_ben_Seatmp && ( pGR->getDataType() == GRB_WTMP ) ) RenderGribSeaTemp( pGR, vp );

        // RTOFS Current
        //    Actually need two records to render the current speed

        if( m_ben_SeaCurrent && ( pGR->getDataType() == GRB_UOGRD ) ) {
            if( pGRCurrentVY ) RenderGribCurrent( pGR, pGRCurrentVY, vp );
            else
                pGRCurrentVX = pGR;
        }

        else if( m_ben_SeaCurrent && ( pGR->getDataType() == GRB_VOGRD ) ) {
            if( pGRCurrentVX ) RenderGribCurrent( pGRCurrentVX, pGR, vp );
            else
                pGRCurrentVY = pGR;
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
    if( !b_drawn ) DrawMessageWindow(
            wxString( _("Please Zoom or Scale Out to view suppressed HTSGW GRIB") ),
            vp->pix_width / 2, vp->pix_height / 2 );
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

bool GRIBOverlayFactory::RenderGribCRAIN( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGR, NULL, vp, 4, CRAIN_GRAPHIC_INDEX, &m_pgob_crain );

    if( !b_drawn ) {
        DrawMessageWindow( wxString( _("Please Zoom or Scale Out to view suppressed CRAIN GRIB") ),
                vp->pix_width / 2, vp->pix_height / 2 );
    }
    return true;
}

bool GRIBOverlayFactory::RenderGribSeaTemp( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGR, NULL, vp, 4, SEATEMP_GRAPHIC_INDEX,
            &m_pgob_seatemp );

    if( !b_drawn ) {
        DrawMessageWindow(
                wxString( _("Please Zoom or Scale Out to view suppressed SEATEMP GRIB") ),
                vp->pix_width / 2, vp->pix_height / 2 );
    }

    return true;
}

bool GRIBOverlayFactory::RenderGribCurrent( GribRecord *pGRX, GribRecord *pGRY,
        PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGRX, pGRY, vp, 4, CURRENT_GRAPHIC_INDEX,
            &m_pgob_current );

    if( b_drawn ) {
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

    if( !b_drawn ) {
        DrawMessageWindow(
                wxString( _("Please Zoom or Scale Out to view suppressed CURRENT GRIB") ),
                vp->pix_width / 2, vp->pix_height / 2 );
    }
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
            0. ) != _OUT ) bdraw = true;
    if( Intersect( vp, pGRA->getLatMin(), pGRA->getLatMax(), pGRA->getLonMin() - 360.,
            pGRA->getLonMax() - 360., 0. ) != _OUT ) bdraw = true;

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
                    else if( colormap_index == SEATEMP_GRAPHIC_INDEX ) c = GetSeaTempGraphicColor(
                            vkn );
                    else if( colormap_index == CRAIN_GRAPHIC_INDEX ) c = wxColour(
                            (unsigned char) vkn * 255, 0, 0 );
                    else
                        c = GetGenericGraphicColor( vkn );

                    unsigned char r = c.Red();
                    unsigned char g = c.Green();
                    unsigned char b = c.Blue();

                    for( int xp = 0; xp < grib_pixel_size; xp++ )
                        for( int yp = 0; yp < grib_pixel_size; yp++ ) {
                            gr_image.SetRGB( ipix + xp, jpix + yp, r, g, b );
                            gr_image.SetAlpha( ipix + xp, jpix + yp, 220 );
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
    else if( val >= 48 ) c.Set( _T("#410000") );

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

void GRIBOverlayFactory::DrawMessageWindow( wxString msg, int x, int y )
{
    wxMemoryDC mdc;
    wxBitmap bm( 1000, 1000 );
    mdc.SelectObject( bm );
    mdc.Clear();

    wxFont mfont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    mdc.SetFont( mfont );
    mdc.SetPen( *wxBLACK_PEN);
    mdc.SetBrush( *wxWHITE_BRUSH);

    int w, h;
    mdc.GetTextExtent( msg, &w, &h );

    int label_offset = 10;
    int wdraw = w + ( label_offset * 2 );
    mdc.DrawRectangle( 0, 0, wdraw, h + 2 );
    mdc.DrawText( msg, label_offset / 2, -1 );

    mdc.SelectObject( wxNullBitmap );

    wxBitmap sbm = bm.GetSubBitmap( wxRect( 0, 0, wdraw, h + 2 ) );
    DrawOLBitmap( sbm, x - wdraw / 2, y, false );
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
//---------------------------------------------------------------------------------------
//          GRIB File/Record selector Tree Control Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS ( GribRecordTree, wxTreeCtrl )

// GribRecordTree event table definition

BEGIN_EVENT_TABLE ( GribRecordTree, wxTreeCtrl ) EVT_TREE_ITEM_EXPANDING ( ID_GRIBRECORDREECTRL, GribRecordTree::OnItemExpanding )
EVT_TREE_SEL_CHANGED ( ID_GRIBRECORDREECTRL, GribRecordTree::OnItemSelectChange )
END_EVENT_TABLE()

GribRecordTree::GribRecordTree()
{
    Init();
}

GribRecordTree::GribRecordTree( GRIBUIDialog* parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, long style )
{
    Init();
    m_parent = parent;
    Create( parent, id, pos, size, style );
}

GribRecordTree::~GribRecordTree()
{
    delete m_file_id_array;
}

void GribRecordTree::Init()
{
    m_file_id_array = NULL;
}

void GribRecordTree::OnItemExpanding( wxTreeEvent& event )
{
}

void GribRecordTree::OnItemSelectChange( wxTreeEvent& event )
{
    GribTreeItemData *pdata = (GribTreeItemData *) GetItemData( event.GetItem() );

    if( !pdata ) return;

    switch( pdata->m_type ){
        case GRIB_FILE_TYPE: {
            m_parent->SetGribRecordSet( NULL );                  // turn off any current display

            //    Create and ingest the GRIB file object if needed
            if( NULL == pdata->m_pGribFile ) {
                GRIBFile *pgribfile = new GRIBFile( pdata->m_file_name );
                if( pgribfile ) {
                    if( pgribfile->IsOK() ) {
                        pdata->m_pGribFile = pgribfile;
                        m_parent->PopulateTreeControlGRS( pgribfile, pdata->m_file_index );

                    } else {
                        wxLogMessage( pgribfile->GetLastErrorMessage() );
                    }
                }
            }
            break;
        }

        case GRIB_RECORD_SET_TYPE: {
            m_parent->SetGribRecordSet( pdata->m_pGribRecordSet );
            break;
        }
    }
}

//---------------------------------------------------------------------------------------
//          GRIB Tree Item Data Implementation
//---------------------------------------------------------------------------------------

GribTreeItemData::GribTreeItemData( const GribTreeItemType type )
{
    m_pGribFile = NULL;
    m_pGribRecordSet = NULL;

    m_type = type;
}

GribTreeItemData::~GribTreeItemData()
{
    delete m_pGribFile;
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
