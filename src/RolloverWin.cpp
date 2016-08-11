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

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>

#include "ocpndc.h"
#include "RolloverWin.h"
#include "timers.h"
#include "chart1.h"
#include "navutil.h"
#include "FontMgr.h"

extern bool             g_bopengl;
extern GLenum       g_texture_rectangle_format;

BEGIN_EVENT_TABLE(RolloverWin, wxWindow) EVT_PAINT(RolloverWin::OnPaint)
    EVT_TIMER(ROLLOVER_TIMER, RolloverWin::OnTimer)
    EVT_MOUSE_EVENTS ( RolloverWin::OnMouseEvent )

END_EVENT_TABLE()

// Define a constructor
RolloverWin::RolloverWin( wxWindow *parent, int timeout, bool maincanvas ) :
wxWindow( parent, wxID_ANY, wxPoint( 0, 0 ), wxSize( 1, 1 ), wxNO_BORDER ),
    m_bmaincanvas(maincanvas)
{
    m_pbm = NULL;

    m_timer_timeout.SetOwner( this, ROLLOVER_TIMER );
    m_timeout_sec = timeout;
    m_mmouse_propogate = 0;
    isActive = false;
    m_plabelFont = NULL;
    m_texture = 0;
    Hide();
}

RolloverWin::~RolloverWin()
{
    delete m_pbm;
}
void RolloverWin::OnTimer( wxTimerEvent& event )
{
    if( IsShown() ) Hide();
}

void RolloverWin::OnMouseEvent( wxMouseEvent& event )
{
    //    If directed, send mouse events up the window family tree,
    //    until some parent window does NOT call event.Skip()
    if( m_mmouse_propogate ) {
        event.ResumePropagation( m_mmouse_propogate );
        event.Skip();
    }
}

void RolloverWin::SetBitmap( int rollover )
{
    wxMemoryDC mdc;
    delete m_pbm;
    m_pbm = new wxBitmap( m_size.x, m_size.y );
    mdc.SelectObject( *m_pbm );

    int usegl = g_bopengl && g_texture_rectangle_format && m_bmaincanvas;
    if(!usegl) {
        wxDC* cdc = new wxScreenDC();
        int cpx = 0, cpy = 0;
        GetParent()->ClientToScreen(&cpx, &cpy);
        mdc.Blit( 0, 0, m_size.x, m_size.y, cdc,
                  m_position.x + cpx, m_position.y + cpy);
        delete cdc;
    } else
        mdc.Clear();

    ocpnDC dc( mdc );

    wxString text;
    double radius = 6.0;
    switch( rollover ) {
    case AIS_ROLLOVER: text = _("AISRollover");   break;
    case TC_ROLLOVER:  text = _("TideCurrentGraphRollover"), radius = 0; break;
    default:
    case LEG_ROLLOVER: text = _("RouteLegInfoRollover");  break;
    }

    AlphaBlending( dc, 0, 0, m_size.x, m_size.y, radius,
                   GetGlobalColor( _T ( "YELO1" ) ), usegl ? 255 : 172 );
    mdc.SetTextForeground( FontMgr::Get().GetFontColor( text ) );

    if(m_plabelFont && m_plabelFont->IsOk()) {

    //    Draw the text
        mdc.SetFont( *m_plabelFont );

        mdc.DrawLabel( m_string, wxRect( 0, 0, m_size.x, m_size.y ), wxALIGN_CENTRE_HORIZONTAL | wxALIGN_CENTRE_VERTICAL);
    }

    SetSize( m_position.x, m_position.y, m_size.x, m_size.y );   // Assumes a nominal 32 x 32 cursor

#ifdef ocpnUSE_GL
    if(usegl) {
        if(!m_texture) {
            glGenTextures( 1, &m_texture );
            glBindTexture( g_texture_rectangle_format, m_texture );
            glTexParameterf( g_texture_rectangle_format, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( g_texture_rectangle_format, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        } else
            glBindTexture( g_texture_rectangle_format, m_texture );
        mdc.SelectObject( wxNullBitmap );

        // make texture data
        wxImage image = m_pbm->ConvertToImage();

        unsigned char *d = image.GetData();
        unsigned char *e = new unsigned char[4*m_size.x*m_size.y];
        for(int y = 0; y<m_size.y; y++)
            for(int x = 0; x<m_size.x; x++) {
                int i = y * m_size.x + x;
                memcpy(e+4*i, d+3*i, 3);
                e[4*i+3] = 255 - d[3*i+2];
            }
        glTexImage2D( g_texture_rectangle_format, 0, GL_RGBA,
                      m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, e );
        delete [] e;
    }
#endif

    // Retrigger the auto timeout
    if( m_timeout_sec > 0 ) m_timer_timeout.Start( m_timeout_sec * 1000, wxTIMER_ONE_SHOT );
}

void RolloverWin::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    if( m_string.Len() ) {
        wxMemoryDC mdc;
        mdc.SelectObject( *m_pbm );
        dc.Blit( 0, 0, width, height, &mdc, 0, 0 );
    }
}

void RolloverWin::Draw(ocpnDC &dc)
{
    if(!IsActive())
        return;
#ifdef ocpnUSE_GL
    if(g_bopengl && m_texture) {
        glBindTexture( g_texture_rectangle_format, m_texture );
        glEnable(g_texture_rectangle_format);
        glEnable(GL_BLEND);
        int x0 = m_position.x, x1 = x0 + m_size.x;
        int y0 = m_position.y, y1 = y0 + m_size.y;
        float tx, ty;
        if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
            tx = m_size.x, ty = m_size.y;
        else
            tx = ty = 1;

        glColor3f(1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(0,  0);  glVertex2i(x0, y0);
        glTexCoord2f(tx, 0);  glVertex2i(x1, y0);
        glTexCoord2f(tx, ty); glVertex2i(x1, y1);
        glTexCoord2f(0,  ty); glVertex2i(x0, y1);
        glEnd();

        glDisable(g_texture_rectangle_format);
        glDisable(GL_BLEND);
    } else
#endif    
    dc.DrawBitmap( *m_pbm, m_position.x, m_position.y, false );
}

void RolloverWin::SetBestPosition( int x, int y, int off_x, int off_y, int rollover,
                                   wxSize parent_size )
{
    int h, w;

    wxFont *dFont;
    switch( rollover ) {

    case AIS_ROLLOVER:
        dFont = FontMgr::Get().GetFont( _("AISRollover") );
        break;

    case TC_ROLLOVER:
        dFont = FontMgr::Get().GetFont( _("TideCurrentGraphRollover") );
        break;

    default:
    case LEG_ROLLOVER:
        dFont = FontMgr::Get().GetFont( _("RouteLegInfoRollover") );
        break;

    }

    int font_size = wxMax(8, dFont->GetPointSize());
    m_plabelFont = FontMgr::Get().FindOrCreateFont( font_size, dFont->GetFamily(),
                         dFont->GetStyle(), dFont->GetWeight(), false, dFont->GetFaceName() );

    if(m_plabelFont && m_plabelFont->IsOk()) {
#ifdef __WXMAC__
        wxScreenDC sdc;
        sdc.GetMultiLineTextExtent(m_string, &w, &h, NULL, m_plabelFont);
#else
        wxClientDC cdc( GetParent() );
        cdc.GetMultiLineTextExtent( m_string, &w, &h, NULL, m_plabelFont );
#endif
    }
    else {
        w = 10;
        h = 10;
    }

    m_size.x = w + 8;
    m_size.y = h + 8;

    int xp, yp;
    if( ( x + off_x + m_size.x ) > parent_size.x ) {
        xp = x - ( off_x / 2 ) - m_size.x;
        xp = wxMax(0, xp);
    } else
        xp = x + off_x;

    if( ( y + off_y + m_size.y ) > parent_size.y ) {
        yp = y - ( off_y / 2 ) - m_size.y;
    } else
        yp = y + off_y;

    SetPosition( wxPoint( xp, yp ) );

}

