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

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/bmpbuttn.h>
#include <wx/dcmemory.h>

#include "TTYWindow.h"
#include "TTYScroll.h"
#include "WindowDestroyListener.h"
#include "chart1.h"

IMPLEMENT_DYNAMIC_CLASS( TTYWindow, wxDialog )

BEGIN_EVENT_TABLE( TTYWindow, wxDialog )
    EVT_CLOSE(TTYWindow::OnCloseWindow)
END_EVENT_TABLE()

TTYWindow::TTYWindow()
    : m_window_destroy_listener(NULL)
    , m_pScroll(NULL)
{
}

TTYWindow::TTYWindow(wxWindow *parent, int n_lines, WindowDestroyListener * listener)
    : m_window_destroy_listener(listener)
    , m_pScroll(NULL)
{
    wxDialog::Create( parent, -1, _T("Title"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );

    wxBoxSizer* bSizerOuterContainer = new wxBoxSizer( wxVERTICAL );
    SetSizer( bSizerOuterContainer );

    m_pScroll = new TTYScroll(this, n_lines);
    m_pScroll->Scroll(-1, 1000);        // start with full scroll down

    bSizerOuterContainer->Add( m_pScroll, 1, wxEXPAND, 5 );

    wxBoxSizer* bSizerBottomContainer = new wxBoxSizer( wxHORIZONTAL );
    bSizerOuterContainer->Add( bSizerBottomContainer, 0, wxEXPAND, 5 );


    wxStaticBox *psb = new wxStaticBox( this,  wxID_ANY, _("Legend")) ;
    wxStaticBoxSizer* sbSizer1 = new wxStaticBoxSizer( psb , wxVERTICAL );

    CreateLegendBitmap();
    wxBitmapButton *bb = new wxBitmapButton(this, wxID_ANY, m_bm_legend);
    sbSizer1->Add( bb, 1, wxALL|wxEXPAND, 5 );
    bSizerBottomContainer->Add( sbSizer1, 0, wxALIGN_LEFT | wxALL, 5 );

    m_buttonPause = new wxButton( this, wxID_ANY, _("Pause"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizerBottomContainer->Add( m_buttonPause, 0, wxALIGN_RIGHT | wxALL, 5 );

    m_buttonPause->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TTYWindow::OnPauseClick ), NULL, this );

    bpause = false;
}

TTYWindow::~TTYWindow()
{
    if (m_pScroll) {
        delete m_pScroll;
        m_pScroll = NULL;
    }
}

void TTYWindow::CreateLegendBitmap()
{
    m_bm_legend.Create(400, 130);
    wxMemoryDC dc;
    dc.SelectObject( m_bm_legend );
    if( m_bm_legend.IsOk()) {

        dc.SetBackground( wxBrush(GetGlobalColor(_T("DILG1"))) );
        dc.Clear();

        wxFont f(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(f);

        int yp = 25;
        int y = 5;

        wxBrush b1(wxColour( _T("DARK GREEN")) );
        dc.SetBrush(b1);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("DARK GREEN")) );
        dc.DrawText(  _("Message accepted"), 30, y );

        y += yp;
        wxBrush b2(wxColour( _T("CORAL")) );
        dc.SetBrush(b2);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("CORAL")) );
        dc.DrawText(  _("Input message filtered, output message filtered and dropped"), 30, y );

        y += yp;
        wxBrush b3(wxColour( _T("MAROON")) );
        dc.SetBrush(b3);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("MAROON")) );
        dc.DrawText(  _("Input Message filtered and dropped"), 30, y );

        y += yp;
        wxBrush b4(wxColour( _T("BLUE")) );
        dc.SetBrush(b4);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("BLUE")) );
        dc.DrawText(  _("Output Message"), 30, y );

        y += yp;
        wxBrush b5(wxColour( _T("RED")) );
        dc.SetBrush(b5);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("RED")) );
        dc.DrawText(  _("Information Message or Message with errors"), 30, y );
    }
    dc.SelectObject( wxNullBitmap );
}

void TTYWindow::OnPauseClick( wxCommandEvent& event )
{
    if(!bpause) {
        bpause = true;
        m_pScroll->Pause( true );
        m_buttonPause->SetLabel( _("Resume") );
    } else {
        bpause = false;
        m_pScroll->Pause( false );

        m_buttonPause->SetLabel( _("Pause") );
    }
}

void TTYWindow::OnCloseWindow( wxCloseEvent& event )
{
    if (m_window_destroy_listener) {
        m_window_destroy_listener->DestroyWindow();
    } else {
        Destroy();
    }
}

void TTYWindow::Add(const wxString &line)
{
    if( m_pScroll ) m_pScroll->Add( line );
}

