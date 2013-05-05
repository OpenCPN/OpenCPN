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

extern ColorScheme global_color_scheme;
extern bool g_bopengl;
extern AISTargetAlertDialog *g_pais_alert_dialog_active;
extern MyFrame *gFrame;
extern ChartCanvas *cc1;
extern int g_ais_alert_dialog_x;
extern int g_ais_alert_dialog_y;
extern int g_ais_alert_dialog_sx;
extern int g_ais_alert_dialog_sy;

IMPLEMENT_CLASS ( AISTargetAlertDialog, wxDialog )

BEGIN_EVENT_TABLE ( AISTargetAlertDialog, wxDialog )
    EVT_CLOSE(AISTargetAlertDialog::OnClose)
    EVT_BUTTON( ID_ACKNOWLEDGE, AISTargetAlertDialog::OnIdAckClick )
    EVT_BUTTON( ID_SILENCE, AISTargetAlertDialog::OnIdSilenceClick )
    EVT_BUTTON( ID_JUMPTO, AISTargetAlertDialog::OnIdJumptoClick )
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
    m_pparent = NULL;
}

bool AISTargetAlertDialog::Create( int target_mmsi, wxWindow *parent, AIS_Decoder *pdecoder,
        bool b_jumpto, wxWindowID id, const wxString& caption, const wxPoint& pos,
        const wxSize& size, long style )
{
    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxDEFAULT_FRAME_STYLE;
    if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY )
            && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) ) wstyle |= ( wxNO_BORDER );

    m_bjumpto = b_jumpto;

    wxSize size_min = size;
    size_min.IncTo( wxSize( 500, 600 ) );
    if( !wxDialog::Create( parent, id, caption, pos, size_min, wstyle ) ) return false;

    m_target_mmsi = target_mmsi;
    m_pparent = parent;
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

    return true;
}

void AISTargetAlertDialog::CreateControls()
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer );

    m_pAlertTextCtl = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxHW_SCROLLBAR_AUTO );
    m_pAlertTextCtl->SetBorders( 5 );

    topSizer->Add( m_pAlertTextCtl, 1, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5 );

    // A horizontal box sizer to contain Ack
    wxBoxSizer* AckBox = new wxBoxSizer( wxHORIZONTAL );
    topSizer->Add( AckBox, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    // The Silence button
    wxButton* silence = new wxButton( this, ID_SILENCE, _( "&Silence Alert" ), wxDefaultPosition,
            wxDefaultSize, 0 );
    AckBox->Add( silence, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    // The Ack button
    wxButton* ack = new wxButton( this, ID_ACKNOWLEDGE, _( "&Acknowledge" ), wxDefaultPosition,
            wxDefaultSize, 0 );
    AckBox->Add( ack, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    if( m_bjumpto ) {
        wxButton* jumpto = new wxButton( this, ID_JUMPTO, _( "&Jump To" ), wxDefaultPosition,
                wxDefaultSize, 0 );
        AckBox->Add( jumpto, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    }

    UpdateText();
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
        wxColor bg = GetBackgroundColour();
        m_pAlertTextCtl->SetBackgroundColour( bg );

        wxFont *dFont = FontMgr::Get().GetFont( _("AISTargetQuery"), 12 );
        wxString face = dFont->GetFaceName();
        int sizes[7];
        for( int i = -2; i < 5; i++ ) {
            sizes[i + 2] = dFont->GetPointSize() + i + ( i > 0 ? i : 0 );
        }

        wxString html;
        html.Printf( _T("<html><body bgcolor=#%02x%02x%02x><center>"), bg.Red(), bg.Blue(),
                bg.Green() );

        html << m_alert_text;
        html << _T("</center></font></body></html>");

        m_pAlertTextCtl->SetFonts( face, face, sizes );
        m_pAlertTextCtl->SetPage( html );

        // Try to create a min size that works across font sizes.
        wxSize sz;
        if( !IsShown() ) {
            sz = m_pAlertTextCtl->GetVirtualSize();
            sz.x = 300;
            m_pAlertTextCtl->SetSize( sz );
        }
        m_pAlertTextCtl->Layout();
        wxSize ir( m_pAlertTextCtl->GetInternalRepresentation()->GetWidth(),
                m_pAlertTextCtl->GetInternalRepresentation()->GetHeight() );
        sz.x = wxMax( m_pAlertTextCtl->GetSize().x, ir.x );
        sz.y = wxMax( m_pAlertTextCtl->GetSize().y, ir.y );
        m_pAlertTextCtl->SetMinSize( sz );
        Fit();
        sz -= wxSize( 200, 200 );
        m_pAlertTextCtl->SetMinSize( sz );
    }

    DimeControl( this );
    if( !g_bopengl && CanSetTransparent() ) SetTransparent( 192 );
}

void AISTargetAlertDialog::OnClose( wxCloseEvent& event )
{
    //    Acknowledge any existing Alert, and dismiss the dialog
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) {
            if( AIS_ALARM_SET == td->n_alarm_state ) {
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
            if( AIS_ALARM_SET == td->n_alarm_state ) {
                td->m_ack_time = wxDateTime::Now();
                td->b_in_ack_timeout = true;
            }
        }
    }
    Destroy();
    g_pais_alert_dialog_active = NULL;
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
    wxPoint p = event.GetPosition();
    g_ais_alert_dialog_x = p.x;
    g_ais_alert_dialog_y = p.y;

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


