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

#include <wx/combobox.h>

#include "SendToGpsDlg.h"
#include "Route.h"
#include "RoutePoint.h"
#include "chart1.h"

extern wxString g_uploadConnection;

IMPLEMENT_DYNAMIC_CLASS(SendToGpsDlg, wxDialog)

BEGIN_EVENT_TABLE( SendToGpsDlg, wxDialog ) EVT_BUTTON( ID_STG_CANCEL, SendToGpsDlg::OnCancelClick )
	EVT_BUTTON( ID_STG_OK, SendToGpsDlg::OnSendClick )
END_EVENT_TABLE()

SendToGpsDlg::SendToGpsDlg()
{
    m_itemCommListBox = NULL;
    m_pgauge = NULL;
    m_SendButton = NULL;
    m_CancelButton = NULL;
    m_pRoute = NULL;
    m_pRoutePoint = NULL;
}

SendToGpsDlg::SendToGpsDlg( wxWindow* parent, wxWindowID id, const wxString& caption,
        const wxString& hint, const wxPoint& pos, const wxSize& size, long style )
{
    Create( parent, id, caption, hint, pos, size, style );
}

SendToGpsDlg::~SendToGpsDlg()
{
    delete m_itemCommListBox;
    delete m_pgauge;
    delete m_SendButton;
    delete m_CancelButton;
}

bool SendToGpsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
        const wxString& hint, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls( hint );
    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();

    return TRUE;
}

void SendToGpsDlg::CreateControls( const wxString& hint )
{
    SendToGpsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

//      Create the ScrollBox list of available com ports in a labeled static box
    wxStaticBox* comm_box = new wxStaticBox( this, wxID_ANY, _("GPS/Plotter Port") );

    wxStaticBoxSizer* comm_box_sizer = new wxStaticBoxSizer( comm_box, wxVERTICAL );
    itemBoxSizer2->Add( comm_box_sizer, 0, wxEXPAND | wxALL, 5 );

    wxArrayString *pSerialArray = EnumerateSerialPorts();

    m_itemCommListBox = new wxComboBox( this, ID_STG_CHOICE_COMM );

    //    Fill in the listbox with all detected serial ports
    for( unsigned int iPortIndex = 0; iPortIndex < pSerialArray->GetCount(); iPortIndex++ ) {
        wxString full_port = pSerialArray->Item( iPortIndex );
        full_port.Prepend(_T("Serial:"));
        m_itemCommListBox->Append( full_port );
    }

    delete pSerialArray;

    //    Make the proper inital selection
    if( !g_uploadConnection.IsEmpty() )
        m_itemCommListBox->SetValue( g_uploadConnection );
    else
        m_itemCommListBox->SetSelection( 0 );

    comm_box_sizer->Add( m_itemCommListBox, 0, wxEXPAND | wxALL, 5 );

    //    Add a reminder text box
    itemBoxSizer2->AddSpacer( 20 );

    wxStaticText *premtext = new wxStaticText( this, -1,
            _("Prepare GPS for Route/Waypoint upload and press Send...") );
    itemBoxSizer2->Add( premtext, 0, wxEXPAND | wxALL, 10 );

    //    Create a progress gauge
    wxStaticBox* prog_box = new wxStaticBox( this, wxID_ANY, _("Progress...") );

    wxStaticBoxSizer* prog_box_sizer = new wxStaticBoxSizer( prog_box, wxVERTICAL );
    itemBoxSizer2->Add( prog_box_sizer, 0, wxEXPAND | wxALL, 5 );

    m_pgauge = new wxGauge( this, -1, 100 );
    prog_box_sizer->Add( m_pgauge, 0, wxEXPAND | wxALL, 5 );

    //    OK/Cancel/etc.
    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5 );

    m_CancelButton = new wxButton( itemDialog1, ID_STG_CANCEL, _("Cancel"), wxDefaultPosition,
            wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_SendButton = new wxButton( itemDialog1, ID_STG_OK, _("Send"), wxDefaultPosition,
            wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_SendButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    m_SendButton->SetDefault();

}

void SendToGpsDlg::OnSendClick( wxCommandEvent& event )
{
    //    Get the selected comm port
    wxString src = m_itemCommListBox->GetValue();
    g_uploadConnection = src;                   // save for persistence

    //    And send it out
    if( m_pRoute ) m_pRoute->SendToGPS( src.BeforeFirst(' '), true, m_pgauge );
    if( m_pRoutePoint ) m_pRoutePoint->SendToGPS( src.BeforeFirst(' '), m_pgauge );

//    Show( false );
//    event.Skip();
    Close();
}

void SendToGpsDlg::OnCancelClick( wxCommandEvent& event )
{
//    Show( false );
//    event.Skip();
    Close();
}


