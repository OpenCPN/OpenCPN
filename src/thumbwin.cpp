/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Thumbnail Object
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
 *
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "dychart.h"

#include "thumbwin.h"
#include "chart1.h"
#include "chartdb.h"
#include "chcanv.h"
#include "wx28compat.h"

//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------
extern ChartDB          *ChartData;
extern ChartStack       *pCurrentStack;

extern MyFrame          *gFrame;
extern ChartBase        *Current_Ch;
extern ChartCanvas      *cc1;


//------------------------------------------------------------------------------
//    Thumbwin Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ThumbWin, wxWindow)
  EVT_PAINT(ThumbWin::OnPaint)
END_EVENT_TABLE()

// Define a constructor

ThumbWin::ThumbWin( wxWindow *parent ) :
        wxWindow( parent, wxID_ANY, wxPoint( 20, 20 ), wxSize( 5, 5 ), wxSIMPLE_BORDER )
{
    pThumbChart = NULL;
    m_max_size.x = m_max_size.y = 100;
    Show( false );
}

ThumbWin::~ThumbWin()
{
}

void ThumbWin::Resize( void )
{
    if( pThumbChart ) {
        if( pThumbChart->GetThumbData()->pDIBThumb ) {
            int newheight = __min(m_max_size.y, pThumbChart->GetThumbData()->pDIBThumb->GetHeight());
            int newwidth = __min(m_max_size.x, pThumbChart->GetThumbData()->pDIBThumb->GetWidth());
            SetSize( 0, 0, newwidth, newheight );
        }
    }
}

void ThumbWin::SetMaxSize( wxSize const &max_size )
{
    m_max_size = max_size;
}


void ThumbWin::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );

    if( pThumbChart ) {
        if( pThumbChart->GetThumbData() ) {
            if( pThumbChart->GetThumbData()->pDIBThumb ) dc.DrawBitmap(
                    *( pThumbChart->GetThumbData()->pDIBThumb ), 0, 0, false );

            wxPen ppPen( GetGlobalColor( _T("CHBLK") ), 1, wxPENSTYLE_SOLID );
            dc.SetPen( ppPen );
            wxBrush yBrush( GetGlobalColor( _T("CHYLW") ), wxBRUSHSTYLE_SOLID );
            dc.SetBrush( yBrush );
            dc.DrawCircle( pThumbChart->GetThumbData()->ShipX, pThumbChart->GetThumbData()->ShipY, 6 );
        }
    }
}

const wxBitmap &ThumbWin::GetBitmap(void)
{
    if( pThumbChart ) {
        if( pThumbChart->GetThumbData() ) {
            if( pThumbChart->GetThumbData()->pDIBThumb )
                m_bitmap =  *( pThumbChart->GetThumbData()->pDIBThumb );
        }
    }
    
    return m_bitmap;
}
     
