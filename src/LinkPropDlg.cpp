/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Hyperlink properties
 * Author:   David Register
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
 **************************************************************************/

#include "LinkPropDlg.h"

LinkPropDlgDef::LinkPropDlgDef( wxWindow* parent, wxWindowID id, const wxString& title,
        const wxPoint& pos, const wxSize& size, long style )
{
    long wstyle = style; // wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif
    
    wxDialog::Create( parent, id, title, pos, size, wstyle );

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );
    
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* bSizerMain;
    bSizerMain = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* sbSizerLnkProp;
    sbSizerLnkProp = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Link") ),
            wxVERTICAL );

    m_staticTextLinkDesc = new wxStaticText( this, wxID_ANY, _("Link description"),
            wxDefaultPosition, wxDefaultSize, 0 );
    m_staticTextLinkDesc->Wrap( -1 );
    sbSizerLnkProp->Add( m_staticTextLinkDesc, 0, wxALL, 5 );

    m_textCtrlLinkDescription = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition,
            wxDefaultSize, 0 );
    sbSizerLnkProp->Add( m_textCtrlLinkDescription, 0, wxALL | wxEXPAND, 5 );

    m_staticTextLinkUrl = new wxStaticText( this, wxID_ANY, _("URL"), wxDefaultPosition,
            wxDefaultSize, 0 );
    m_staticTextLinkUrl->Wrap( -1 );
    sbSizerLnkProp->Add( m_staticTextLinkUrl, 0, wxALL, 5 );

    m_textCtrlLinkUrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition,
            wxDefaultSize, 0 );
    sbSizerLnkProp->Add( m_textCtrlLinkUrl, 0, wxALL | wxEXPAND, 5 );

    m_buttonBrowseLocal = new wxButton( this, wxID_ANY, _("Local file..."), wxDefaultPosition,
            wxDefaultSize, 0 );
    sbSizerLnkProp->Add( m_buttonBrowseLocal, 0, wxALL, 5 );

    bSizerMain->Add( sbSizerLnkProp, 1, wxALL | wxEXPAND, 5 );

    m_sdbSizerButtons = new wxStdDialogButtonSizer();
    m_sdbSizerButtonsOK = new wxButton( this, wxID_OK );
    m_sdbSizerButtons->AddButton( m_sdbSizerButtonsOK );
    m_sdbSizerButtonsCancel = new wxButton( this, wxID_CANCEL );
    m_sdbSizerButtons->AddButton( m_sdbSizerButtonsCancel );
    m_sdbSizerButtons->Realize();

    bSizerMain->Add( m_sdbSizerButtons, 0, wxALL | wxEXPAND, 5 );

    this->SetSizer( bSizerMain );
    this->Layout();
    Fit();
    
    this->Centre( wxBOTH );

    // Connect Events
    m_buttonBrowseLocal->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( LinkPropDlgDef::OnLocalFileClick ), NULL, this );
    m_sdbSizerButtonsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( LinkPropDlgDef::OnCancelClick ), NULL, this );
    m_sdbSizerButtonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( LinkPropDlgDef::OnOkClick ), NULL, this );
}

LinkPropDlgDef::~LinkPropDlgDef()
{
    // Disconnect Events
    m_buttonBrowseLocal->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( LinkPropDlgDef::OnLocalFileClick ), NULL, this );
    m_sdbSizerButtonsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( LinkPropDlgDef::OnCancelClick ), NULL, this );
    m_sdbSizerButtonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( LinkPropDlgDef::OnOkClick ), NULL, this );
}

LinkPropImpl::LinkPropImpl( wxWindow* parent, wxWindowID id, const wxString& title,
        const wxPoint& pos, const wxSize& size, long style ) :
        LinkPropDlgDef( parent, id, title, pos, size, style )
{
    m_parent = parent;
    DimeControl( this );
}

void LinkPropImpl::OnLocalFileClick( wxCommandEvent& event )
{
    wxString filename = wxFileSelector( _("Choose a file") );
    if( !filename.empty() ) {
        wxString url = wxFileSystem::FileNameToURL( filename );
        url.Replace( _T("%3A"), _T(":") ); //The replace hack is a way to make it work on Windows... I hate it.
        m_textCtrlLinkUrl->SetValue( url );
    }
}

void LinkPropImpl::OnOkClick( wxCommandEvent& event )
{
    if( m_textCtrlLinkUrl->GetValue() == wxEmptyString ) 
        OCPNMessageBox( NULL, _("Link not complete, can't be saved."), _("OpenCPN Info"), wxICON_HAND );
    
    else
        event.Skip();
}
