/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#include <wx/html/htmlwin.h>

#include "AISTargetAlertDialog.h"
#include "AIS_Decoder.h"
#include "AIS_Target_Data.h"
#include "FontMgr.h"
#include "ocpn_types.h"
#include "Select.h"
#include "routemanagerdialog.h"
#include "OCPNPlatform.h"

extern ColorScheme global_color_scheme;
extern bool g_bopengl;
extern AISTargetAlertDialog *g_pais_alert_dialog_active;
extern MyFrame *gFrame;
extern ChartCanvas *cc1;
extern int g_ais_alert_dialog_x;
extern int g_ais_alert_dialog_y;
extern int g_ais_alert_dialog_sx;
extern int g_ais_alert_dialog_sy;
extern bool g_bAIS_CPA_Alert_Audio;
extern wxString g_default_wp_icon;
extern Select *pSelect;
extern MyConfig *pConfig;
extern RouteManagerDialog *pRouteManagerDialog;
extern ChartCanvas *cc1;
extern OCPNPlatform  *g_Platform;


//---------------------------------------------------------------------------------------------------------------------
//
//      OCPN Alert Dialog Base Class implementation
//
//---------------------------------------------------------------------------------------------------------------------


IMPLEMENT_CLASS ( OCPN_AlertDialog, wxDialog )

BEGIN_EVENT_TABLE ( OCPN_AlertDialog, wxDialog )
END_EVENT_TABLE()

OCPN_AlertDialog::OCPN_AlertDialog()
{
    Init();
}

OCPN_AlertDialog::~OCPN_AlertDialog()
{
}

void OCPN_AlertDialog::Init(void)
{
    m_pparent = NULL;
}

bool OCPN_AlertDialog::Create( wxWindow *parent, wxWindowID id,
                                   const wxString& caption, const wxPoint& pos, const wxSize& size, long style )

{
    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision
    
    long wstyle = wxDEFAULT_FRAME_STYLE;
    if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY )
        && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) ) wstyle |= ( wxNO_BORDER );
    
    wxSize size_min = size;
    size_min.IncTo( wxSize( 500, 600 ) );
    if( !wxDialog::Create( parent, id, caption, pos, size_min, wstyle ) ) return false;
    
    m_pparent = parent;
    
    if( !g_bopengl && CanSetTransparent() ) SetTransparent( 192 );
    
    return true;
}


//---------------------------------------------------------------------------------------------------------------------
//
//      AIS Target Alert Dialog implementation
//
//---------------------------------------------------------------------------------------------------------------------

IMPLEMENT_CLASS ( AISTargetAlertDialog, wxDialog )

BEGIN_EVENT_TABLE ( AISTargetAlertDialog, wxDialog )
    EVT_CLOSE(AISTargetAlertDialog::OnClose)
    EVT_BUTTON( ID_ACKNOWLEDGE, AISTargetAlertDialog::OnIdAckClick )
    EVT_BUTTON( ID_SILENCE, AISTargetAlertDialog::OnIdSilenceClick )
    EVT_BUTTON( ID_JUMPTO, AISTargetAlertDialog::OnIdJumptoClick )
    EVT_BUTTON( ID_WPT_CREATE, AISTargetAlertDialog::OnIdCreateWPClick )
    EVT_MOVE( AISTargetAlertDialog::OnMove )
    EVT_SIZE( AISTargetAlertDialog::OnSize )
END_EVENT_TABLE()

AISTargetAlertDialog::AISTargetAlertDialog()
{
    Init();
}

AISTargetAlertDialog::~AISTargetAlertDialog()
{
}

void AISTargetAlertDialog::Init()
{
    m_target_mmsi = 0;
    m_max_nline = 20;
    m_adj_height = 0;
    
}


bool AISTargetAlertDialog::Create( int target_mmsi, wxWindow *parent, AIS_Decoder *pdecoder,
                                   bool b_jumpto, bool b_createWP, bool b_ack,
                                   wxWindowID id,  const wxString& caption,
                                   const wxPoint& pos,const wxSize& size, long style )
                     
{
    
    OCPN_AlertDialog::Create(parent, id, caption, pos, size, style);
    m_bjumpto = b_jumpto;
    m_back = b_ack;
    m_bcreateWP = b_createWP;
    
    m_target_mmsi = target_mmsi;
    m_pdecoder = pdecoder;

    wxFont *dFont = FontMgr::Get().GetFont( _("AISTargetAlert"), 12 );
    int font_size = wxMax(8, dFont->GetPointSize());
    wxString face = dFont->GetFaceName();
#ifdef __WXGTK__
    face = _T("Monospace");
#endif
    wxFont *fp_font = wxTheFontList->FindOrCreateFont( font_size, wxFONTFAMILY_MODERN,
            wxFONTSTYLE_NORMAL, dFont->GetWeight(), false, face );

    SetFont( *fp_font );

    CreateControls();
    if( !g_bopengl && CanSetTransparent() ) SetTransparent( 192 );
    DimeControl( this );
    wxColor bg = GetBackgroundColour();
    m_pAlertTextCtl->SetBackgroundColour( bg );
    SetBackgroundColour( bg );
    
    return true;
}

void AISTargetAlertDialog::CreateControls()
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer );

    m_pAlertTextCtl = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION);
    #ifdef __OCPN__ANDROID__
    m_pAlertTextCtl->GetHandle()->setStyleSheet( getQtStyleSheet());
    #endif
    
    m_pAlertTextCtl->SetBorders( 5 );

    topSizer->Add( m_pAlertTextCtl, 1, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5 );

    // A horizontal box sizer to contain Ack
    wxBoxSizer* AckBox = new wxBoxSizer( wxHORIZONTAL );
    topSizer->Add( AckBox, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    // The Silence button
    if( g_bAIS_CPA_Alert_Audio ){
        wxButton* silence = new wxButton( this, ID_SILENCE, _( "&Silence Alert" ), wxDefaultPosition,
            wxDefaultSize, 0 );
        AckBox->Add( silence, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    }

    // The Ack button
    if( m_back ) {
        wxButton* ack = new wxButton( this, ID_ACKNOWLEDGE, _( "&Acknowledge" ), wxDefaultPosition,
            wxDefaultSize, 0 );
        AckBox->Add( ack, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    }

    if( m_bjumpto ) {
        wxButton* jumpto = new wxButton( this, ID_JUMPTO, _( "&Jump To" ), wxDefaultPosition,
                wxDefaultSize, 0 );
        AckBox->Add( jumpto, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    }
    
    if( m_bcreateWP ) {
        wxButton *createWptBtn = new wxButton( this, ID_WPT_CREATE, _("Create Waypoint"), wxDefaultPosition, wxDefaultSize, 0 );
        AckBox->Add( createWptBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    }
    

    UpdateText();
    
    RecalculateSize();
    
}

bool AISTargetAlertDialog::GetAlertText()
{
    //    Search the parent AIS_Decoder's target list for specified mmsi
    if( m_pdecoder ) {
        AIS_Target_Data *td_found = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );

        if( td_found ) {
            m_alert_text = td_found->BuildQueryResult();
            return true;
        } else
            return false;
    } else
        return false;
}

void AISTargetAlertDialog::UpdateText()
{
    if( GetAlertText() ) {
        
        wxFont *dFont = FontMgr::Get().GetFont( _("AISTargetQuery"), 12 );
        wxString face = dFont->GetFaceName();
        int sizes[7];
        for( int i = -2; i < 5; i++ ) {
            sizes[i + 2] = dFont->GetPointSize() + i + ( i > 0 ? i : 0 );
        }

        wxString html;
        wxColor bg = GetBackgroundColour();
        
        html.Printf( _T("<html><body bgcolor=#%02x%02x%02x><center>"), bg.Red(), bg.Green(), bg.Blue() );
        
        html << m_alert_text;
        html << _T("</center></font></body></html>");

        m_pAlertTextCtl->SetFonts( face, face, sizes );
        m_pAlertTextCtl->SetPage( html );

        RecalculateSize();
    }
    
    SetColorScheme();
    if( !g_bopengl && CanSetTransparent() )
        SetTransparent( 192 );
}

void AISTargetAlertDialog::RecalculateSize( void )
{
    //  Count the lines in the currently displayed text string
    unsigned int i=0;
    int nline = 0;
    while(i < m_alert_text.Length() ){
        if(m_alert_text[i] == '\n')
            nline++;
        i++;
    }

    if(nline > m_max_nline)
        m_max_nline = nline;
    
    wxSize esize;
    esize.x = GetCharWidth() * 45;
    esize.y = GetCharHeight() * (m_max_nline + 4);
//    SetSize(esize);

    int height = m_pAlertTextCtl->GetInternalRepresentation()->GetHeight();
    int adj_height = height + (GetCharHeight() * 4);
    m_adj_height = wxMax(m_adj_height, adj_height);
    
    esize.y = wxMin(esize.y, m_adj_height);
    SetClientSize(esize);
    
    wxSize dsize = GetParent()->GetClientSize();
    
    wxSize fsize = GetSize();
    fsize.y = wxMin(fsize.y, dsize.y - (1 * GetCharHeight()));
    fsize.x = wxMin(fsize.x, dsize.x - (1 * GetCharHeight()));
    SetSize(fsize);
    
    g_Platform->PositionAISAlert( this );
    
}

void AISTargetAlertDialog::SetColorScheme( void )
{
    DimeControl( this );
    wxColor bg = GetBackgroundColour();
    m_pAlertTextCtl->SetBackgroundColour( bg );
    SetBackgroundColour( bg );                  // This looks like non-sense, but is needed for __WXGTK__
    // to get colours to propagate down the control's family tree.
    
#ifdef __WXQT__    
    //  wxQT has some trouble clearing the background of HTML window...
    wxBitmap tbm( GetSize().x, GetSize().y, -1 );
    wxMemoryDC tdc( tbm );
    //    wxColour cback = GetGlobalColor( _T("YELO1") );
    tdc.SetBackground( bg );
    tdc.Clear();
    m_pAlertTextCtl->SetBackgroundImage(tbm);
#endif
    
}

void AISTargetAlertDialog::OnClose( wxCloseEvent& event )
{
    //    Acknowledge any existing Alert, and dismiss the dialog
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) {
            if( AIS_ALERT_SET == td->n_alert_state ) {
                td->m_ack_time = wxDateTime::Now();
                td->b_in_ack_timeout = true;
            }
        }
    }

    Destroy();
    g_pais_alert_dialog_active = NULL;
}

void AISTargetAlertDialog::OnIdAckClick( wxCommandEvent& event )
{
    //    Acknowledge the Alert, and dismiss the dialog
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) {
            if( AIS_ALERT_SET == td->n_alert_state ) {
                td->m_ack_time = wxDateTime::Now();
                td->b_in_ack_timeout = true;
            }
        }
    }
    Destroy();
    g_pais_alert_dialog_active = NULL;
}
void AISTargetAlertDialog::OnIdCreateWPClick( wxCommandEvent& event )
{
    if( m_pdecoder ) { 
        AIS_Target_Data *td =  m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) {
            RoutePoint *pWP = new RoutePoint( td->Lat, td->Lon, g_default_wp_icon, wxEmptyString, GPX_EMPTY_STRING );
            pWP->m_bIsolatedMark = true;                      // This is an isolated mark
            pSelect->AddSelectableRoutePoint( td->Lat, td->Lon, pWP );
            pConfig->AddNewWayPoint( pWP, -1 );    // use auto next num
            
            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
                pRouteManagerDialog->UpdateWptListCtrl();
            if(cc1){
                cc1->undo->BeforeUndoableAction( Undo_CreateWaypoint, pWP, Undo_HasParent, NULL );
                cc1->undo->AfterUndoableAction( NULL );
                cc1->InvalidateGL();
            }
            Refresh( false );
        }
    }
    
}


void AISTargetAlertDialog::OnIdSilenceClick( wxCommandEvent& event )
{
    //    Set the suppress audio flag
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) td->b_suppress_audio = true;
    }
}

void AISTargetAlertDialog::OnIdJumptoClick( wxCommandEvent& event )
{
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) gFrame->JumpToPosition( td->Lat, td->Lon, cc1->GetVPScale() );
    }
}

void AISTargetAlertDialog::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    g_ais_alert_dialog_x = GetPosition().x;
    g_ais_alert_dialog_y = GetPosition().y;

    event.Skip();
}

void AISTargetAlertDialog::OnSize( wxSizeEvent& event )
{
    //    Record the dialog size
    wxSize p = event.GetSize();
    g_ais_alert_dialog_sx = p.x;
    g_ais_alert_dialog_sy = p.y;

    event.Skip();
}

