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

#include <wx/slider.h>

#include "CM93DSlide.h"
#include "chart1.h"
#include "chcanv.h"
#include "OCPNPlatform.h"

extern bool g_bShowCM93DetailSlider;
extern CM93DSlide *pCM93DetailSlider;
extern int g_cm93_zoom_factor;
extern int g_cm93detail_dialog_x;
extern int g_cm93detail_dialog_y;
extern ChartCanvas *cc1;

BEGIN_EVENT_TABLE(CM93DSlide, wxDialog)
    EVT_KEY_DOWN(CM93DSlide::OnKeyDown )
    EVT_MOVE( CM93DSlide::OnMove )
    EVT_COMMAND_SCROLL_THUMBRELEASE(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_LINEUP(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_LINEDOWN(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_PAGEUP(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_PAGEDOWN(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_BOTTOM(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_TOP(-1, CM93DSlide::OnChangeValue)
    EVT_CLOSE( CM93DSlide::OnClose )
END_EVENT_TABLE()

CM93DSlide::CM93DSlide( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
                        const wxPoint& pos, const wxSize& size, long style, const wxString& title )
{
    Init();
    Create( parent, ID_CM93ZOOMG, value, minValue, maxValue, pos, size, style, title );

    m_pCM93DetailSlider->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler(CM93DSlide::OnKeyDown), NULL, this);
}

CM93DSlide::~CM93DSlide()
{
    delete m_pCM93DetailSlider;
}

void CM93DSlide::Init( void )
{
    m_pCM93DetailSlider = NULL;
}

bool CM93DSlide::Create( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
                         const wxPoint& pos, const wxSize& size, long style, const wxString& title )
{
    long wstyle = wxDEFAULT_DIALOG_STYLE;
#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif
    
    if( !wxDialog::Create( parent, id, title, pos, size, wstyle ) ) return false;

    m_pparent = parent;

    m_pCM93DetailSlider = new wxSlider( this, id, value, minValue, maxValue, wxPoint( 0, 0 ),
                                        wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS, wxDefaultValidator,
                                        title );

    m_pCM93DetailSlider->SetSize( wxSize( 200, -1 ) );

    m_pCM93DetailSlider->InvalidateBestSize();
    wxSize bs = m_pCM93DetailSlider->GetBestSize();

    m_pCM93DetailSlider->SetSize( wxSize( 200, bs.y ) );
    Fit();

    m_pCM93DetailSlider->SetValue( g_cm93_zoom_factor );

    Hide();

    return true;
}

void CM93DSlide::OnCancelClick( wxCommandEvent& event )
{
    g_bShowCM93DetailSlider = false;
    Close();
}

void CM93DSlide::OnClose( wxCloseEvent& event )
{
    g_bShowCM93DetailSlider = false;

    Destroy();
    pCM93DetailSlider = NULL;
}

void CM93DSlide::OnKeyDown( wxKeyEvent &event )
{
    int key_char = event.GetKeyCode();
    if(key_char == WXK_ESCAPE || key_char == 'D') {
        g_bShowCM93DetailSlider = false;
        Close();
    }
}

void CM93DSlide::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_cm93detail_dialog_x = p.x;
    g_cm93detail_dialog_y = p.y;

    event.Skip();
}

void CM93DSlide::OnChangeValue( wxScrollEvent& event )
{
    g_cm93_zoom_factor = m_pCM93DetailSlider->GetValue();

    cc1->ReloadVP();
    cc1->Refresh( false );

}

