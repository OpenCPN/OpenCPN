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

#include <wx/dcclient.h>

#include "ChInfoWin.h"
#include "chart1.h"
#include "OCPNPlatform.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

extern bool g_btouch;
extern OCPNPlatform  *g_Platform;

BEGIN_EVENT_TABLE(ChInfoWin, wxWindow)
    EVT_PAINT ( ChInfoWin::OnPaint )
    EVT_ERASE_BACKGROUND(ChInfoWin::OnEraseBackground)
    EVT_MOUSE_EVENTS ( ChInfoWin::MouseEvent )
END_EVENT_TABLE()

// Define a constructor
ChInfoWin::ChInfoWin( wxWindow *parent ) :
    wxWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER )
{
    int ststyle = wxALIGN_LEFT | wxST_NO_AUTORESIZE;
    m_pInfoTextCtl = new wxStaticText( this, -1, _T ( "" ), wxDefaultPosition, wxDefaultSize,
                                       ststyle );

    dbIndex = -1;
    Hide();
}

ChInfoWin::~ChInfoWin()
{
    delete m_pInfoTextCtl;
}

void ChInfoWin::OnEraseBackground( wxEraseEvent& event )
{
}

void ChInfoWin::MouseEvent( wxMouseEvent& event )
{
    if(g_btouch){
        if( event.LeftDown() ) {
            Hide();
            
            #ifdef __OCPN__ANDROID__        
            androidForceFullRepaint();
            #endif
        }
    }
}

    
void ChInfoWin::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
    dc.SetPen( wxPen( GetGlobalColor( _T ( "UITX1" ) ) ) );
    dc.DrawRectangle( 0, 0, width, height );
}

void ChInfoWin::SetBitmap()
{
    SetBackgroundColour( GetGlobalColor( _T ( "UIBCK" ) ) );

    m_pInfoTextCtl->SetBackgroundColour( GetGlobalColor( _T ( "UIBCK" ) ) );
    m_pInfoTextCtl->SetForegroundColour( GetGlobalColor( _T ( "UITX1" ) ) );

    m_pInfoTextCtl->SetSize( 1, 1, m_size.x - 2, m_size.y - 2 );
    m_pInfoTextCtl->SetLabel( m_string );

    SetSize( m_position.x, m_position.y, m_size.x, m_size.y );
}

void ChInfoWin::FitToChars( int char_width, int char_height )
{
    wxSize size;

    int adjust = 1;
#ifdef __WXOSX__
    adjust = 2;
#endif
    
#ifdef __OCPN__ANDROID__
    adjust = 4;
#endif
    
    size.x = GetCharWidth() * char_width;
    size.y = GetCharHeight() * ( char_height + adjust );
    size.x = wxMin(size.x, g_Platform->getDisplaySize().x-10);
    SetWinSize( size );
}

