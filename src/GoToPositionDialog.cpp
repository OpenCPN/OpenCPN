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
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/clipbrd.h>

#include "GoToPositionDialog.h"
#include "PositionParser.h"
#include "routeprop.h"
#include "navutil.h"

extern ChartCanvas *cc1;
extern MyFrame *gFrame;

/*!
 * GoToPositionDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( GoToPositionDialog, wxDialog )
/*!
 * GoToPositionDialog event table definition
 */BEGIN_EVENT_TABLE( GoToPositionDialog, wxDialog )

////@begin GoToPositionDialog event table entries

    EVT_BUTTON( ID_GOTOPOS_CANCEL, GoToPositionDialog::OnGoToPosCancelClick )
    EVT_BUTTON( ID_GOTOPOS_OK, GoToPositionDialog::OnGoToPosOkClick )
    EVT_COMMAND(ID_LATCTRL, EVT_LLCHANGE, GoToPositionDialog::OnPositionCtlUpdated)
    EVT_COMMAND(ID_LONCTRL, EVT_LLCHANGE, GoToPositionDialog::OnPositionCtlUpdated)

////@end GoToPositionDialog event table entries

END_EVENT_TABLE()

/*!
 * GoToPositionDialog constructors
 */

GoToPositionDialog::GoToPositionDialog()
{
}

GoToPositionDialog::GoToPositionDialog( wxWindow* parent, wxWindowID id, const wxString& caption,
                                        const wxPoint& pos, const wxSize& size, long style )
{

    long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
    wxDialog::Create( parent, id, caption, pos, size, wstyle );

    CreateControls();
    GetSizer()->SetSizeHints( this );
    Centre();

}

GoToPositionDialog::~GoToPositionDialog()
{
    delete m_MarkLatCtl;
    delete m_MarkLonCtl;
}

/*!
 * GoToPositionDialog creator
 */

bool GoToPositionDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                                 const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->SetSizeHints( this );
    Centre();

    return TRUE;
}

/*!
 * Control creation for GoToPositionDialog
 */

void GoToPositionDialog::CreateControls()
{
    GoToPositionDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox( itemDialog1, wxID_ANY,
            _("Position") );

    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer( itemStaticBoxSizer4Static,
            wxVERTICAL );
    itemBoxSizer2->Add( itemStaticBoxSizer4, 0, wxEXPAND | wxALL, 5 );

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Latitude"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add( itemStaticText5, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5 );

    m_MarkLatCtl = new LatLonTextCtrl( itemDialog1, ID_LATCTRL, _T(""), wxDefaultPosition,
                                       wxSize( 180, -1 ), 0 );
    itemStaticBoxSizer4->Add( m_MarkLatCtl, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Longitude"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add( itemStaticText6, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5 );

    m_MarkLonCtl = new LatLonTextCtrl( itemDialog1, ID_LONCTRL, _T(""), wxDefaultPosition,
                                       wxSize( 180, -1 ), 0 );
    itemStaticBoxSizer4->Add( m_MarkLonCtl, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5 );

    m_CancelButton = new wxButton( itemDialog1, ID_GOTOPOS_CANCEL, _("Cancel"), wxDefaultPosition,
                                   wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_OKButton = new wxButton( itemDialog1, ID_GOTOPOS_OK, _("OK"), wxDefaultPosition,
                               wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    m_OKButton->SetDefault();

    SetColorScheme( (ColorScheme) 0 );
}

void GoToPositionDialog::SetColorScheme( ColorScheme cs )
{
    DimeControl( this );
}

bool GoToPositionDialog::ShowToolTips()
{
    return TRUE;
}

void GoToPositionDialog::OnGoToPosCancelClick( wxCommandEvent& event )
{
    Hide();
    cc1->ReloadVP();

    event.Skip();
}

void GoToPositionDialog::OnGoToPosOkClick( wxCommandEvent& event )
{
    double lat, lon;

    if( m_MarkLatCtl->GetValue().Length() == 0 ) goto noGo;
    if( m_MarkLonCtl->GetValue().Length() == 0 ) goto noGo;

    lat = fromDMM( m_MarkLatCtl->GetValue() );
    lon = fromDMM( m_MarkLonCtl->GetValue() );

    if( lat == 0.0 && lon == 0.0 ) goto noGo;
    if( lat > 80.0 || lat < -80.0 ) goto noGo;
    if( lon > 180.0 || lon < -180.0 ) goto noGo;

    gFrame->JumpToPosition( lat, lon, cc1->GetVPScale() );
    Hide();
    event.Skip();
    return;

    noGo:
    wxBell();
    event.Skip();
    return;
}

void GoToPositionDialog::CheckPasteBufferForPosition() {
    if( wxTheClipboard->Open() ) {
        wxTextDataObject data;
        wxTheClipboard->GetData( data );
        wxString pasteBuf = data.GetText();

        PositionParser pparse( pasteBuf );

        if( pparse.IsOk() ) {
            m_MarkLatCtl->SetValue( pparse.GetLatitudeString() );
            m_MarkLonCtl->SetValue( pparse.GetLongitudeString() );
        }
        wxTheClipboard->Close();
    }
}

void GoToPositionDialog::OnPositionCtlUpdated( wxCommandEvent& event )
{
    // We do not want to change the position on lat/lon now
}

