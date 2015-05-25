/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Object
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */

#include "wx/wx.h"

#include "folder.xpm"

#include "GrabberWin.h"

//----------------------------------------------------------------------------
// GrabberWindow Implementation
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GribGrabberWin, wxPanel) EVT_MOUSE_EVENTS ( GribGrabberWin::OnMouseEvent )
EVT_PAINT ( GribGrabberWin::OnPaint )
END_EVENT_TABLE()

GribGrabberWin::GribGrabberWin( wxWindow *parent )
{
    Create( parent );

    m_bLeftDown = false;

}

void GribGrabberWin::OnMouseEvent( wxMouseEvent& event )
{
    if( event.RightDown() ) {
        wxMouseEvent evt(event);
        ((wxEvtHandler*)GetParent())->ProcessEvent( evt );
        return;
    }

    static wxPoint s_gspt;
    int x, y;

    event.GetPosition( &x, &y );
    wxPoint spt = ClientToScreen( wxPoint( x, y ) );

#ifdef __WXOSX__
    if (!m_bLeftDown && event.LeftIsDown())
    {
        m_bLeftDown = true;
        s_gspt = spt;
        if (!HasCapture()) CaptureMouse();
    }
    else if (m_bLeftDown && !event.LeftIsDown())
    {
       // GetParent()->Move( GetParent()->GetPosition() );
        m_bLeftDown = false;
        if (HasCapture()) ReleaseMouse();
    }
#else

    if( event.LeftDown() ) {
        s_gspt = spt;
        CaptureMouse();
    }

    if( event.LeftUp() ) {
        //GetParent()->Move( GetParent()->GetPosition() );
        if( HasCapture() ) ReleaseMouse();
    }
#endif

    if( event.Dragging() ) {

        wxPoint par_pos_old = GetParent()->GetPosition();

        wxPoint par_pos = par_pos_old;
        par_pos.x += spt.x - s_gspt.x;
        par_pos.y += spt.y - s_gspt.y;

        wxPoint pos_in_parent = GetOCPNCanvasWindow()->ScreenToClient( par_pos );
        wxPoint pos_in_parent_old = GetOCPNCanvasWindow()->ScreenToClient( par_pos_old );

		// X
		if( pos_in_parent.x < pos_in_parent_old.x ) {           // moving left
			if( pos_in_parent.x < 10 ) {
				pos_in_parent.x = 0;
			}
		} else
        if( pos_in_parent.x > pos_in_parent_old.x ) {           // moving right
            int max_right = GetOCPNCanvasWindow()->GetClientSize().x - GetParent()->GetSize().x;
            if( pos_in_parent.x > ( max_right - 10 ) ) {
                pos_in_parent.x = max_right;
            }
        }

		// Y
		if( pos_in_parent.y < pos_in_parent_old.y ) {            // moving up
			if( pos_in_parent.y < 10 ) {
				pos_in_parent.y = 0;
			}
		} else
        if( pos_in_parent.y > pos_in_parent_old.y ) {            // moving dow
            int max_down = GetOCPNCanvasWindow()->GetClientSize().y - GetParent()->GetSize().y;
            if( pos_in_parent.y > ( max_down - 10 ) ) {
                pos_in_parent.y = max_down;
            }
        }

		wxPoint final_pos = GetOCPNCanvasWindow()->ClientToScreen( pos_in_parent );

		GetParent()->Move( final_pos );

        s_gspt = spt;

    }
}

void GribGrabberWin::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );
    dc.DrawBitmap( m_bitmap, 0, 0, true );

}

int GribGrabberWin::Size( int height )

{
 //   int margin = 8;     //bitmap y will be always extended at least by 8 pix

    wxBitmap bitmap = (wxBitmap( grabber ));
    int width = bitmap.GetWidth();

/*    if( height < bitmap.GetHeight() + margin )
        bitmap = bitmap.GetSubBitmap( wxRect(0, 0, width, height - margin) );*/

    wxImage scaled_image = bitmap.ConvertToImage();
    m_bitmap = wxBitmap(scaled_image.Scale(width, height, wxIMAGE_QUALITY_HIGH));

    SetSize( wxSize( width, height ));
    SetMinSize( wxSize( width, height ));

    return width;
}
