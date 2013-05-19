/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 */

#include "wx/wxprec.h"

#include <wx/html/htmlwin.h>

#include "AISTargetQueryDialog.h"
#include "chcanv.h"
#include "navutil.h"
#include "ais.h"
#include "FontMgr.h"
#include "AIS_Target_Data.h"
#include "AIS_Decoder.h"
#include "Select.h"
#include "routemanagerdialog.h"

extern AISTargetQueryDialog *g_pais_query_dialog_active;
extern int g_ais_query_dialog_x;
extern int g_ais_query_dialog_y;
extern ColorScheme global_color_scheme;
extern AIS_Decoder *g_pAIS;
extern wxString g_default_wp_icon;
extern Select *pSelect;
extern MyConfig *pConfig;
extern RouteManagerDialog *pRouteManagerDialog;
extern ChartCanvas *cc1;

#define xID_OK 10009
#define xID_WPT_CREATE 10010
IMPLEMENT_CLASS ( AISTargetQueryDialog, wxDialog )
// AISTargetQueryDialog event table definition
BEGIN_EVENT_TABLE ( AISTargetQueryDialog, wxDialog )
    EVT_BUTTON( xID_OK, AISTargetQueryDialog::OnIdOKClick )
    EVT_BUTTON( xID_WPT_CREATE, AISTargetQueryDialog::OnIdWptCreateClick )
    EVT_CLOSE(AISTargetQueryDialog::OnClose)
    EVT_MOVE( AISTargetQueryDialog::OnMove )
END_EVENT_TABLE()

AISTargetQueryDialog::AISTargetQueryDialog()
{
    Init();
}

AISTargetQueryDialog::AISTargetQueryDialog( wxWindow* parent, wxWindowID id,
        const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}

AISTargetQueryDialog::~AISTargetQueryDialog()
{
    delete m_pQueryTextCtl;
}

void AISTargetQueryDialog::Init()
{
    m_MMSI = -1;
    m_pQueryTextCtl = NULL;
    m_nl = 0;
    m_colorscheme = (ColorScheme) ( -1 );
    m_okButton = NULL;

}
void AISTargetQueryDialog::OnClose( wxCloseEvent& event )
{
    Destroy();
    g_pais_query_dialog_active = NULL;
}

void AISTargetQueryDialog::OnIdOKClick( wxCommandEvent& event )
{
    Close();
}

void AISTargetQueryDialog::OnIdWptCreateClick( wxCommandEvent& event )
{
    if( m_MMSI != 0 ) { //  Faulty MMSI could be reported as 0
        AIS_Target_Data *td = g_pAIS->Get_Target_Data_From_MMSI( m_MMSI );
        if( td ) {
            RoutePoint *pWP = new RoutePoint( td->Lat, td->Lon, g_default_wp_icon, wxEmptyString, GPX_EMPTY_STRING );
            pWP->m_bIsolatedMark = true;                      // This is an isolated mark
            pSelect->AddSelectableRoutePoint( td->Lat, td->Lon, pWP );
            pConfig->AddNewWayPoint( pWP, -1 );    // use auto next num

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
                pRouteManagerDialog->UpdateWptListCtrl();
            cc1->undo->BeforeUndoableAction( Undo_CreateWaypoint, pWP, Undo_HasParent, NULL );
            cc1->undo->AfterUndoableAction( NULL );
            Refresh( false );
        }
    }
}

bool AISTargetQueryDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size, long style )
{
    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxDEFAULT_FRAME_STYLE;
    if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY )
            && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) ) wstyle |= ( wxNO_BORDER );

    if( !wxDialog::Create( parent, id, caption, pos, size, wstyle ) ) return false;

    wxFont *dFont = FontMgr::Get().GetFont( _("AISTargetQuery"), 12 );
    int font_size = wxMax(8, dFont->GetPointSize());
    wxString face = dFont->GetFaceName();
#ifdef __WXGTK__
    face = _T("Monospace");
#endif
    wxFont *fp_font = wxTheFontList->FindOrCreateFont( font_size, wxFONTFAMILY_MODERN,
                      wxFONTSTYLE_NORMAL, dFont->GetWeight(), false, face );

    SetFont( *fp_font );

    CreateControls();

    SetColorScheme( global_color_scheme );

// This ensures that the dialog cannot be sized smaller
// than the minimum size
    GetSizer()->SetSizeHints( this );

    return true;
}

void AISTargetQueryDialog::SetColorScheme( ColorScheme cs )
{
    if( cs != m_colorscheme ) {
        DimeControl( this );
        Refresh();
    }
}

void AISTargetQueryDialog::CreateControls()
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer );

    m_pQueryTextCtl = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxHW_SCROLLBAR_AUTO );
    m_pQueryTextCtl->SetBorders( 5 );

    topSizer->Add( m_pQueryTextCtl, 1, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5 );

    wxSizer* ok = CreateButtonSizer( wxOK );
    wxButton* createWptBtn = new wxButton( this, xID_WPT_CREATE, _("Create Waypoint"), wxDefaultPosition, wxDefaultSize, 0 );
    ok->Add( createWptBtn, 0, wxALL|wxEXPAND, 5 );
    topSizer->Add( ok, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5 );
}

void AISTargetQueryDialog::UpdateText()
{
    wxString html;

    if( !m_pQueryTextCtl ) return;

    DimeControl( this );
    wxColor bg = GetBackgroundColour();
    m_pQueryTextCtl->SetBackgroundColour( bg );

    if( m_MMSI != 0 ) { //  Faulty MMSI could be reported as 0
        AIS_Target_Data *td = g_pAIS->Get_Target_Data_From_MMSI( m_MMSI );
        if( td ) {
            wxFont *dFont = FontMgr::Get().GetFont( _("AISTargetQuery"), 12 );
            wxString face = dFont->GetFaceName();
            int sizes[7];
            for( int i=-2; i<5; i++ ) {
                sizes[i+2] = dFont->GetPointSize() + i + (i>0?i:0);
            }

            html.Printf( _T("<html><body bgcolor=#%02x%02x%02x><center>"), bg.Red(), bg.Blue(),
                            bg.Green() );

            html << td->BuildQueryResult();
            html << _T("</center></font></body></html>");

            m_pQueryTextCtl->SetFonts( face, face, sizes );
            m_pQueryTextCtl->SetPage( html );

            // Try to create a min size that works across font sizes.
            wxSize sz;
            if( ! IsShown() ) {
                sz = m_pQueryTextCtl->GetVirtualSize();
                sz.x = 300;
                m_pQueryTextCtl->SetSize( sz );
            }
            m_pQueryTextCtl->Layout();
            wxSize ir(m_pQueryTextCtl->GetInternalRepresentation()->GetWidth(),
                    m_pQueryTextCtl->GetInternalRepresentation()->GetHeight() );
            sz.x = wxMax( m_pQueryTextCtl->GetSize().x, ir.x );
            sz.y = wxMax( m_pQueryTextCtl->GetSize().y, ir.y );
            m_pQueryTextCtl->SetMinSize( sz );
            Fit();
            sz -= wxSize( 200, 200 );
            m_pQueryTextCtl->SetMinSize( sz );
        }
    }
}

void AISTargetQueryDialog::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_ais_query_dialog_x = p.x;
    g_ais_query_dialog_y = p.y;
    event.Skip();
}

