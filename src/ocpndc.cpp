/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Layer to perform wxDC drawing using wxDC or opengl
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Sean D'Epagnier                                 *
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
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include "dychart.h"
#include "ocpn_plugin.h"

#ifdef __MSVC__
#include <windows.h>
#endif

#ifdef ocpnUSE_GL
#include <wx/glcanvas.h>
#endif

#include <wx/graphics.h>
#include <wx/dcclient.h>

#include <vector>

#include "ocpndc.h"
#include "wx28compat.h"

extern float g_GLMinSymbolLineWidth;
wxArrayPtrVoid gTesselatorVertices;

//----------------------------------------------------------------------------
/* pass the dc to the constructor, or NULL to use opengl */
ocpnDC::ocpnDC( wxGLCanvas &canvas ) :
        glcanvas( &canvas ), dc( NULL ), m_pen( wxNullPen ), m_brush( wxNullBrush )
{
#if wxUSE_GRAPHICS_CONTEXT
    pgc = NULL;
#endif
#ifdef ocpnUSE_GL
    m_textforegroundcolour = wxColour( 0, 0, 0 );
#endif   
    m_buseTex = GetLocaleCanonicalName().IsSameAs(_T("en_US"));
}

ocpnDC::ocpnDC( wxDC &pdc ) :
        glcanvas( NULL ), dc( &pdc ), m_pen( wxNullPen ), m_brush( wxNullBrush )
{
#if wxUSE_GRAPHICS_CONTEXT
    pgc = NULL;
    wxMemoryDC *pmdc = wxDynamicCast(dc, wxMemoryDC);
    if( pmdc ) pgc = wxGraphicsContext::Create( *pmdc );
    else {
        wxClientDC *pcdc = wxDynamicCast(dc, wxClientDC);
        if( pcdc ) pgc = wxGraphicsContext::Create( *pcdc );
    }
#endif
    m_textforegroundcolour = wxColour( 0, 0, 0 );
    m_buseTex = GetLocaleCanonicalName().IsSameAs(_T("en_US"));
}

ocpnDC::ocpnDC() :
        glcanvas( NULL ), dc( NULL ), m_pen( wxNullPen ), m_brush( wxNullBrush )
{
#if wxUSE_GRAPHICS_CONTEXT
    pgc = NULL;
#endif
    m_buseTex = GetLocaleCanonicalName().IsSameAs(_T("en_US"));
}

ocpnDC::~ocpnDC()
{
#if wxUSE_GRAPHICS_CONTEXT
    if( pgc ) delete pgc;
#endif
}

void ocpnDC::Clear()
{
    if( dc ) dc->Clear();
    else {
#ifdef ocpnUSE_GL
        wxBrush tmpBrush = m_brush;
        int w, h;
        SetBrush( wxBrush( glcanvas->GetBackgroundColour() ) );
        glcanvas->GetSize( &w, &h );
        DrawRectangle( 0, 0, w, h );
        SetBrush( tmpBrush );
#endif        
    }
}

void ocpnDC::SetBackground( const wxBrush &brush )
{
    if( dc )
        dc->SetBackground( brush );
    else {
#ifdef ocpnUSE_GL
        glcanvas->SetBackgroundColour( brush.GetColour() );
#endif
    }
}

void ocpnDC::SetPen( const wxPen &pen )
{
    if( dc ) {
        if( pen == wxNullPen ) dc->SetPen( *wxTRANSPARENT_PEN );
        else
            dc->SetPen( pen );
    } else
        m_pen = pen;
}

void ocpnDC::SetBrush( const wxBrush &brush )
{
    if( dc ) dc->SetBrush( brush );
    else
        m_brush = brush;
}

void ocpnDC::SetTextForeground( const wxColour &colour )
{
    if( dc ) dc->SetTextForeground( colour );
    else
        m_textforegroundcolour = colour;
}

void ocpnDC::SetFont( const wxFont& font )
{
    if( dc ) dc->SetFont( font );
    else
        m_font = font;
}

const wxPen& ocpnDC::GetPen() const
{
    if( dc ) return dc->GetPen();
    return m_pen;
}

const wxBrush& ocpnDC::GetBrush() const
{
    if( dc ) return dc->GetBrush();
    return m_brush;
}

const wxFont& ocpnDC::GetFont() const
{
    if( dc ) return dc->GetFont();
    return m_font;
}

void ocpnDC::GetSize( wxCoord *width, wxCoord *height ) const
{
    if( dc )
        dc->GetSize( width, height );
    else {
#ifdef ocpnUSE_GL
        glcanvas->GetSize( width, height );
#endif
    }
}

void ocpnDC::SetGLAttrs( bool highQuality )
{
#ifdef ocpnUSE_GL

 // Enable anti-aliased polys, at best quality
    if( highQuality ) {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_POLYGON_SMOOTH );
        glEnable( GL_BLEND );
    } else {
        glDisable(GL_LINE_SMOOTH);
        glDisable( GL_POLYGON_SMOOTH );
        glDisable( GL_BLEND );
    }
#endif
}

void ocpnDC::SetGLStipple() const
{
#ifdef ocpnUSE_GL
    
    switch( m_pen.GetStyle() ) {
        case wxDOT: {
            glLineStipple( 1, 0x3333 );
            glEnable( GL_LINE_STIPPLE );
            break;
        }
        case wxLONG_DASH: {
            glLineStipple( 1, 0xFFF8 );
            glEnable( GL_LINE_STIPPLE );
            break;
        }
        case wxSHORT_DASH: {
            glLineStipple( 1, 0x3F3F );
            glEnable( GL_LINE_STIPPLE );
            break;
        }
        case wxDOT_DASH: {
            glLineStipple( 1, 0x8FF1 );
            glEnable( GL_LINE_STIPPLE );
            break;
        }
        default: break;
    }
#endif    
}

#ifdef ocpnUSE_GL
/* draw a half circle using triangles */
void DrawEndCap(float x1, float y1, float t1, float angle)
{
    const int steps = 16;
    float xa, ya;
    bool first = true;
    for(int i = 0; i <= steps; i++) {
        float a = angle + M_PI/2 + M_PI/steps*i;

        float xb = x1 + t1 / 2 * cos( a );
        float yb = y1 + t1 / 2 * sin( a );
        if(first)
            first = false;
        else {
            glVertex2f( x1, y1 );
            glVertex2f( xa, ya );
            glVertex2f( xb, yb );
        }
        xa = xb, ya = yb;
    }
}
#endif

// Draws a line between (x1,y1) - (x2,y2) with a start thickness of t1
void DrawGLThickLine( float x1, float y1, float x2, float y2, wxPen pen, bool b_hiqual )
{
#ifdef ocpnUSE_GL
    
    float angle = atan2f( y2 - y1, x2 - x1 );
    float t1 = pen.GetWidth();
    float t2sina1 = t1 / 2 * sinf( angle );
    float t2cosa1 = t1 / 2 * cosf( angle );

    glBegin( GL_TRIANGLES );

    //    n.b.  The dwxDash interpretation for GL only allows for 2 elements in the dash table.
    //    The first is assumed drawn, second is assumed space
    wxDash *dashes;
    int n_dashes = pen.GetDashes( &dashes );
    if( n_dashes ) {
        float lpix = sqrtf( powf( (float) (x1 - x2), 2) + powf( (float) (y1 - y2), 2) );
        float lrun = 0.;
        float xa = x1;
        float ya = y1;
        float ldraw = t1 * dashes[0];
        float lspace = t1 * dashes[1];

        while( lrun < lpix ) {
            //    Dash
            float xb = xa + ldraw * cosf( angle );
            float yb = ya + ldraw * sinf( angle );

            if( ( lrun + ldraw ) >= lpix )         // last segment is partial draw
            {
                xb = x2;
                yb = y2;
            }

            glVertex2f( xa + t2sina1, ya - t2cosa1 );
            glVertex2f( xb + t2sina1, yb - t2cosa1 );
            glVertex2f( xb - t2sina1, yb + t2cosa1 );

            glVertex2f( xb - t2sina1, yb + t2cosa1 );
            glVertex2f( xa - t2sina1, ya + t2cosa1 );
            glVertex2f( xa + t2sina1, ya - t2cosa1 );

            xa = xb;
            ya = yb;
            lrun += ldraw;

            //    Space
            xb = xa + lspace * cos( angle );
            yb = ya + lspace * sin( angle );

            xa = xb;
            ya = yb;
            lrun += lspace;
        }
    } else {
        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );
        glVertex2f( x2 + t2sina1, y2 - t2cosa1 );
        glVertex2f( x2 - t2sina1, y2 + t2cosa1 );

        glVertex2f( x2 - t2sina1, y2 + t2cosa1 );
        glVertex2f( x1 - t2sina1, y1 + t2cosa1 );
        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );

        /* wx draws a nice rounded end in dc mode, so replicate
           this for opengl mode, should this be done for the dashed mode case? */
        if(pen.GetCap() == wxCAP_ROUND) {
            DrawEndCap( x1, y1, t1, angle);
            DrawEndCap( x2, y2, t1, angle + M_PI);
        }

    }

    glEnd();
#endif    
}

void ocpnDC::DrawLine( wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, bool b_hiqual )
{
    if( dc )
        dc->DrawLine( x1, y1, x2, y2 );
#ifdef ocpnUSE_GL
    else if( ConfigurePen() ) {
        bool b_draw_thick = false;

        float pen_width = wxMax(g_GLMinSymbolLineWidth, m_pen.GetWidth());

        //      Enable anti-aliased lines, at best quality
        if( b_hiqual ) {
            SetGLStipple();

#ifndef __WXQT__
            glEnable( GL_BLEND );
            glEnable( GL_LINE_SMOOTH );
#endif            

            if( pen_width > 1.0 ) {
                GLint parms[2];
                glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0] );
                if( pen_width > parms[1] ) b_draw_thick = true;
                else
                    glLineWidth( pen_width );
            } else
                glLineWidth( pen_width );
        } else {            
            if( pen_width > 1 ) {
                GLint parms[2];
                glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
                if( pen_width > parms[1] ) b_draw_thick = true;
                    else
                        glLineWidth( pen_width );
            } else
                glLineWidth( pen_width );
        }
        
        if( b_draw_thick ) DrawGLThickLine( x1, y1, x2, y2, m_pen, b_hiqual );
        else {
            wxDash *dashes;
            int n_dashes = m_pen.GetDashes( &dashes );
            if( n_dashes ) {
                float angle = atan2f( (float) ( y2 - y1 ), (float) ( x2 - x1 ) );
                float cosa = cosf( angle );
                float sina = sinf( angle );
                float t1 = m_pen.GetWidth();
                    
                float lpix = sqrtf( powf(x1 - x2, 2) + powf(y1 - y2, 2) );
                float lrun = 0.;
                float xa = x1;
                float ya = y1;
                float ldraw = t1 * dashes[0];
                float lspace = t1 * dashes[1];

                ldraw = wxMax(ldraw, 4.0);
                lspace = wxMax(lspace, 4.0);
                lpix = wxMin(lpix, 2000.0);
                
                glBegin( GL_LINES );
                while( lrun < lpix ) {
                    //    Dash
                    float xb = xa + ldraw * cosa;
                    float yb = ya + ldraw * sina;

                    if( ( lrun + ldraw ) >= lpix )         // last segment is partial draw
                    {
                        xb = x2;
                        yb = y2;
                        }

                        glVertex2f( xa, ya );
                        glVertex2f( xb, yb );

                        xa = xa + ( lspace + ldraw ) * cosa;
                        ya = ya + ( lspace + ldraw ) * sina;
                        lrun += lspace + ldraw;

                    }
                glEnd();
            } else                    // not dashed
            {
                glBegin( GL_LINES );
                glVertex2i( x1, y1 );
                glVertex2i( x2, y2 );
                glEnd();
            }
        }

        glDisable( GL_LINE_STIPPLE );

        if( b_hiqual ) {
            glDisable( GL_LINE_SMOOTH );
            glDisable( GL_BLEND );
        }
    }
#endif    
}

// Draws thick lines from triangles
void DrawGLThickLines( int n, wxPoint points[],wxCoord xoffset,
                       wxCoord yoffset, wxPen pen, bool b_hiqual )
{
#ifdef ocpnUSE_GL
    if(n < 2)
        return;

    /* for dashed case, for now just draw thick lines */
    wxDash *dashes;
    if( pen.GetDashes( &dashes ) )
    {
        wxPoint p0 = points[0];
        for( int i = 1; i < n; i++ ) {
            DrawGLThickLine( p0.x + xoffset, p0.y + yoffset, points[i].x + xoffset,
                             points[i].y + yoffset, pen, b_hiqual );
            p0 = points[i];
        }
        return;
    }

    /* cull zero segments */
    wxPoint *cpoints = new wxPoint[n];
    cpoints[0] = points[0];
    int c = 1;
    for( int i = 1; i < n; i++ ) {
        if(points[i].x != points[i-1].x || points[i].y != points[i-1].y)
            cpoints[c++] = points[i];
    }

    /* nicer than than rendering each segment separately, this is because thick
       line segments drawn as rectangles which have different angles have
       rectangles which overlap and also leave a gap.
       This code properly calculates vertexes for adjoining segments */
    float t1 = pen.GetWidth();

    float x0 = cpoints[0].x, y0 = cpoints[0].y, x1 = cpoints[1].x, y1 = cpoints[1].y;
    float a0 = atan2f( y1 - y0, x1 - x0 );

    // It is also possible to use triangle strip, (and triangle fan for endcap)
    // to reduce vertex count.. is it worth it?
    glBegin( GL_TRIANGLES );

    float t2sina0 = t1 / 2 * sinf( a0 );
    float t2cosa0 = t1 / 2 * cosf( a0 );

    for( int i = 1; i < c; i++ ) {
        float x2, y2;
        float a1;

        if(i < c - 1) {
            x2 = cpoints[i + 1].x, y2 = cpoints[i + 1].y;
            a1 = atan2f( y2 - y1, x2 - x1 );
        } else {
            x2 = x1, y2 = y1;
            a1 = a0;
        }

        float aa = (a0 + a1) / 2;
        float diff = fabsf(a0 - a1);
        if(diff > M_PI)
            diff -= 2 * (float)M_PI;
        float rad = t1 / 2 / wxMax(cosf(diff / 2), .4);

        float t2sina1 = rad * sinf( aa );
        float t2cosa1 = rad * cosf( aa );

        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );
        glVertex2f( x1 - t2sina1, y1 + t2cosa1 );
        glVertex2f( x0 + t2sina0, y0 - t2cosa0 );

        glVertex2f( x0 - t2sina0, y0 + t2cosa0 );
        glVertex2f( x0 + t2sina0, y0 - t2cosa0 );

        float dot = t2sina0 * t2sina1 + t2cosa0 * t2cosa1;
        if(dot > 0)
            glVertex2f( x1 - t2sina1, y1 + t2cosa1 );
        else
            glVertex2f( x1 + t2sina1, y1 - t2cosa1 );

        x0 = x1, x1 = x2;
        y0 = y1, y1 = y2;
        a0 = a1;
        t2sina0 = t2sina1, t2cosa0 = t2cosa1;
    }
 
    if(pen.GetCap() == wxCAP_ROUND) {
        DrawEndCap( x0, y0, t1, a0);
        DrawEndCap( x0, y0, t1, a0 + M_PI);
     }

    glEnd();

    glPopAttrib();

    delete [] cpoints;

 #endif    
 }

void ocpnDC::DrawLines( int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset, bool b_hiqual )
{
    if( dc )
        dc->DrawLines( n, points, xoffset, yoffset );
#ifdef ocpnUSE_GL
    else if( ConfigurePen() ) {

        SetGLAttrs( b_hiqual ); 
        bool b_draw_thick = false;

        glDisable( GL_LINE_STIPPLE );
        SetGLStipple();

        //      Enable anti-aliased lines, at best quality
        if( b_hiqual ) {
            glEnable( GL_BLEND );
            if( m_pen.GetWidth() > 1 ) {
                GLint parms[2];
                glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0] );
                if( m_pen.GetWidth() > parms[1] ) b_draw_thick = true;
                else
                    glLineWidth( wxMax(g_GLMinSymbolLineWidth, m_pen.GetWidth()) );
            } else
                glLineWidth( wxMax(g_GLMinSymbolLineWidth, 1) );
        } else {
            if( m_pen.GetWidth() > 1 ) {
                GLint parms[2];
                glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
                if( m_pen.GetWidth() > parms[1] ) b_draw_thick = true;
                else
                    glLineWidth( wxMax(g_GLMinSymbolLineWidth, m_pen.GetWidth()) );
            } else
                glLineWidth( wxMax(g_GLMinSymbolLineWidth, 1) );
        }

        if( b_draw_thick) {
            DrawGLThickLines( n, points, xoffset, yoffset, m_pen, b_hiqual );
        } else {

            if( b_hiqual ) {
                glEnable( GL_LINE_SMOOTH );
                ;//                SetGLStipple(m_pen.GetStyle());
            }

            glBegin( GL_LINE_STRIP );
            for( int i = 0; i < n; i++ )
                glVertex2i( points[i].x + xoffset, points[i].y + yoffset );
            glEnd();
        }

        if( b_hiqual ) {
            glDisable( GL_LINE_STIPPLE );
            glDisable( GL_POLYGON_SMOOTH );
            glDisable( GL_BLEND );
        }
    }
#endif    
}

void ocpnDC::StrokeLine( wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2 )
{
#if wxUSE_GRAPHICS_CONTEXT
    if( pgc ) {
        pgc->SetPen( dc->GetPen() );
        pgc->StrokeLine( x1, y1, x2, y2 );

        dc->CalcBoundingBox( x1, y1 );
        dc->CalcBoundingBox( x2, y2 );
    } else
#endif
        DrawLine( x1, y1, x2, y2, true );
}

void ocpnDC::StrokeLines( int n, wxPoint *points) {
    if(n < 2) /* optimization and also to avoid assertion in pgc->StrokeLines */
        return;

#if wxUSE_GRAPHICS_CONTEXT
    if( pgc ) {
        wxPoint2DDouble* dPoints = (wxPoint2DDouble*) malloc( n * sizeof( wxPoint2DDouble ) );
        for( int i=0; i<n; i++ ) {
            dPoints[i].m_x = points[i].x;
            dPoints[i].m_y = points[i].y;
        }
        pgc->SetPen( dc->GetPen() );
        pgc->StrokeLines( n, dPoints );
        free( dPoints );
    } else
#endif
        DrawLines( n, points, 0, 0, true );
}

void ocpnDC::DrawRectangle( wxCoord x, wxCoord y, wxCoord w, wxCoord h )
{
    if( dc )
        dc->DrawRectangle( x, y, w, h );
#ifdef ocpnUSE_GL
    else {
        if( ConfigureBrush() ) {
            glBegin( GL_QUADS );
            glVertex2i( x, y );
            glVertex2i( x + w, y );
            glVertex2i( x + w, y + h );
            glVertex2i( x, y + h );
            glEnd();
        }

        if( ConfigurePen() ) {
            glBegin( GL_LINE_LOOP );
            glVertex2i( x, y );
            glVertex2i( x + w, y );
            glVertex2i( x + w, y + h );
            glVertex2i( x, y + h );
            glEnd();
        }
    }
#endif    
}

/* draw the arc along corners */
static void drawrrhelper( wxCoord x0, wxCoord y0, wxCoord r, int quadrant, int steps )
{
#ifdef ocpnUSE_GL
    float step = 1.0/steps, rs = 2.0*r*step, rss = rs*step, x, y, dx, dy, ddx, ddy;
    switch(quadrant) {
    case 0: x =  r, y =  0, dx =   0, dy = -rs, ddx = -rss, ddy =  rss; break;
    case 1: x =  0, y = -r, dx = -rs, dy =   0, ddx =  rss, ddy =  rss; break;
    case 2: x = -r, y =  0, dx =   0, dy =  rs, ddx =  rss, ddy = -rss; break;
    case 3: x =  0, y =  r, dx =  rs, dy =   0, ddx = -rss, ddy = -rss; break;
    default: return; // avoid unitialized compiler warnings
    }

    for(int i=0; i<steps; i++) {
        glVertex2i( x0 + floor(x), y0 + floor(y) );
         x += dx+ddx/2,  y += dy+ddy/2;
        dx += ddx,      dy += ddy;
    }
    glVertex2i( x0 + floor(x), y0 + floor(y) );
#endif
}

void ocpnDC::DrawRoundedRectangle( wxCoord x, wxCoord y, wxCoord w, wxCoord h, wxCoord r )
{
    if( dc )
        dc->DrawRoundedRectangle( x, y, w, h, r );
#ifdef ocpnUSE_GL
    else {
        r++;
        int steps = ceil(sqrt((float)r));

        wxCoord x1 = x + r, x2 = x + w - r;
        wxCoord y1 = y + r, y2 = y + h - r;
        if( ConfigureBrush() ) {
            glBegin( GL_TRIANGLE_FAN );
            drawrrhelper( x2, y1, r, 0, steps );
            drawrrhelper( x1, y1, r, 1, steps );
            drawrrhelper( x1, y2, r, 2, steps );
            drawrrhelper( x2, y2, r, 3, steps );
            glEnd();
        }

        if( ConfigurePen() ) {
            glBegin( GL_LINE_LOOP );
            drawrrhelper( x2, y1, r, 0, steps );
            drawrrhelper( x1, y1, r, 1, steps );
            drawrrhelper( x1, y2, r, 2, steps );
            drawrrhelper( x2, y2, r, 3, steps );
            glEnd();
        }
    }
#endif    
}

void ocpnDC::DrawCircle( wxCoord x, wxCoord y, wxCoord radius )
{
    DrawEllipse( x - radius, y - radius, 2 * radius, 2 * radius );
}

void ocpnDC::StrokeCircle( wxCoord x, wxCoord y, wxCoord radius )
{
#if wxUSE_GRAPHICS_CONTEXT
    if( pgc ) {
        wxGraphicsPath gpath = pgc->CreatePath();
        gpath.AddCircle( x, y, radius );

        pgc->SetPen( GetPen() );
        pgc->SetBrush( GetBrush() );
        pgc->DrawPath( gpath );

        // keep dc dirty box up-to-date
        dc->CalcBoundingBox( x + radius + 2, y + radius + 2 );
        dc->CalcBoundingBox( x - radius - 2, y - radius - 2 );
    } else
#endif
        DrawCircle( x, y, radius );
}

void ocpnDC::DrawEllipse( wxCoord x, wxCoord y, wxCoord width, wxCoord height )
{
    if( dc )
        dc->DrawEllipse( x, y, width, height );
#ifdef ocpnUSE_GL
    else {
        float r1 = width / 2, r2 = height / 2;
        float cx = x + r1, cy = y + r2;

        //      Enable anti-aliased lines, at best quality
        glEnable( GL_BLEND );

        /* formula for variable step count to produce smooth ellipse */
        float steps = floorf(wxMax(sqrtf(sqrtf((float)(width*width + height*height))), 1) * M_PI);

        if( ConfigureBrush() ) {
            glBegin( GL_TRIANGLE_FAN );
            glVertex2f( cx, cy );
            for( float a = 0; a <= 2 * M_PI + M_PI/steps; a += 2 * M_PI / steps )
                glVertex2f( cx + r1 * sinf( a ), cy + r2 * cosf( a ) );
            glEnd();
        }

        if( ConfigurePen() ) {
            glBegin( GL_LINE_LOOP );
            for( float a = 0; a < 2 * M_PI - M_PI/steps; a += 2 * M_PI / steps )
                glVertex2f( cx + r1 * sinf( a ), cy + r2 * cosf( a ) );
            glEnd();
        }

        glDisable( GL_BLEND );
    }
#endif    
}

void ocpnDC::DrawPolygon( int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset, float scale )
{
    if( dc )
        dc->DrawPolygon( n, points, xoffset, yoffset );
#ifdef ocpnUSE_GL
    else {
        
#ifdef __WXQT__        
        SetGLAttrs( false );            // Some QT platforms (Android) have trouble with GL_BLEND / GL_LINE_SMOOTH 
#else
        SetGLAttrs( true );
#endif        

        if( ConfigureBrush() ) {
            glEnable( GL_POLYGON_SMOOTH );
            glBegin( GL_POLYGON );
            for( int i = 0; i < n; i++ )
                glVertex2f( (points[i].x * scale) + xoffset, (points[i].y * scale) + yoffset );
            glEnd();
            glDisable( GL_POLYGON_SMOOTH );
        }

        if( ConfigurePen() ) {
            glEnable( GL_LINE_SMOOTH );
            glBegin( GL_LINE_LOOP );
            for( int i = 0; i < n; i++ )
                glVertex2f( (points[i].x * scale) + xoffset, (points[i].y * scale) + yoffset );
            glEnd();
            glDisable( GL_LINE_SMOOTH );
        }

        SetGLAttrs( false ); 
        
    }
#endif    
}

#ifdef ocpnUSE_GL

typedef union {
    GLdouble data[6];
    struct sGLvertex {
        GLdouble x;
        GLdouble y;
        GLdouble z;
        GLdouble r;
        GLdouble g;
        GLdouble b;
    } info;
} GLvertex;

void APIENTRY ocpnDCcombineCallback( GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4],
        GLdouble **dataOut )
{
    GLvertex *vertex;

    vertex = new GLvertex();
    gTesselatorVertices.Add(vertex );

    vertex->info.x = coords[0];
    vertex->info.y = coords[1];
    vertex->info.z = coords[2];

    for( int i = 3; i < 6; i++ ) {
        vertex->data[i] = weight[0] * vertex_data[0][i] + weight[1] * vertex_data[1][i];
    }

    *dataOut = &(vertex->data[0]);
}

void APIENTRY ocpnDCvertexCallback( GLvoid* arg )
{
    GLvertex* vertex;
    vertex = (GLvertex*) arg;
    glVertex2f( (float)vertex->info.x, (float)vertex->info.y );
}

void APIENTRY ocpnDCerrorCallback( GLenum errorCode )
{
   const GLubyte *estring;
   estring = gluErrorString(errorCode);
   wxLogMessage( _T("OpenGL Tessellation Error: %s"), (char *)estring );
}

void APIENTRY ocpnDCbeginCallback( GLenum type )
{
    glBegin( type );
}

void APIENTRY ocpnDCendCallback()
{
    glEnd();
}
#endif          //#ifdef ocpnUSE_GL

void ocpnDC::DrawPolygonTessellated( int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset )
{
    if( dc )
        dc->DrawPolygon( n, points, xoffset, yoffset );
#ifdef ocpnUSE_GL
    else {
# ifndef ocpnUSE_GLES  // tessalator in glues is broken
        if( n < 5 )
# endif
        {
            DrawPolygon( n, points, xoffset, yoffset );
            return;
        }

        static GLUtesselator *tobj = NULL;
        if( ! tobj ) tobj = gluNewTess();

        gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &ocpnDCvertexCallback );
        gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &ocpnDCbeginCallback );
        gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &ocpnDCendCallback );
        gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &ocpnDCcombineCallback );
        gluTessCallback( tobj, GLU_TESS_ERROR, (_GLUfuncptr) &ocpnDCerrorCallback );

        gluTessNormal( tobj, 0, 0, 1);
        gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO );

        if( ConfigureBrush() ) {
            gluTessBeginPolygon( tobj, NULL );
            gluTessBeginContour( tobj );

            for( int i = 0; i < n; i++ ) {
                GLvertex* vertex = new GLvertex();
                gTesselatorVertices.Add( vertex );
                vertex->info.x = (GLdouble) points[i].x;
                vertex->info.y = (GLdouble) points[i].y;
                vertex->info.z = (GLdouble) 0.0;
                vertex->info.r = (GLdouble) 0.0;
                vertex->info.g = (GLdouble) 0.0;
                vertex->info.b = (GLdouble) 0.0;
                gluTessVertex( tobj, (GLdouble*)vertex, (GLdouble*)vertex );
            }
            gluTessEndContour( tobj );
            gluTessEndPolygon( tobj );
        }

        for( unsigned int i=0; i<gTesselatorVertices.Count(); i++ )
            delete (GLvertex*)gTesselatorVertices.Item(i);
        gTesselatorVertices.Clear();
    }
#endif    
}

void ocpnDC::StrokePolygon( int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset, float scale )
{
#if wxUSE_GRAPHICS_CONTEXT
    if( pgc ) {
        wxGraphicsPath gpath = pgc->CreatePath();
        gpath.MoveToPoint( points[0].x + xoffset, points[0].y + yoffset );
        for( int i = 1; i < n; i++ )
            gpath.AddLineToPoint( points[i].x + xoffset, points[i].y + yoffset );
        gpath.AddLineToPoint( points[0].x + xoffset, points[0].y + yoffset );

        pgc->SetPen( GetPen() );
        pgc->SetBrush( GetBrush() );
        pgc->DrawPath( gpath );

        for( int i = 0; i < n; i++ )
            dc->CalcBoundingBox( points[i].x + xoffset, points[i].y + yoffset );
    } else
#endif
        DrawPolygon( n, points, xoffset, yoffset, scale );
}

void ocpnDC::DrawBitmap( const wxBitmap &bitmap, wxCoord x, wxCoord y, bool usemask )
{
#ifdef ocpnUSE_GLES  // Do not attempt to do anything with glDrawPixels if using opengles
        return;
#endif

    wxBitmap bmp;
    if( x < 0 || y < 0 ) {
        int dx = ( x < 0 ? -x : 0 );
        int dy = ( y < 0 ? -y : 0 );
        int w = bitmap.GetWidth() - dx;
        int h = bitmap.GetHeight() - dy;
        /* picture is out of viewport */
        if( w <= 0 || h <= 0 ) return;
        wxBitmap newBitmap = bitmap.GetSubBitmap( wxRect( dx, dy, w, h ) );
        x += dx;
        y += dy;
        bmp = newBitmap;
    } else {
        bmp = bitmap;
    }
    if( dc )
        dc->DrawBitmap( bmp, x, y, usemask );
#ifdef ocpnUSE_GL
    else {
        wxImage image = bmp.ConvertToImage();
        int w = image.GetWidth(), h = image.GetHeight();

        if( usemask ) {
            unsigned char *d = image.GetData();
            unsigned char *a = image.GetAlpha();

            unsigned char mr, mg, mb;
            if( !image.GetOrFindMaskColour( &mr, &mg, &mb ) && !a ){
                printf("trying to use mask to draw a bitmap without alpha or mask\n" );
            }

#ifdef __WXOSX__            
            if(image.HasMask())
                a=0;
#endif
            
            unsigned char *e = new unsigned char[4 * w * h];
            if(e && d){
                for( int y = 0; y < h; y++ )
                    for( int x = 0; x < w; x++ ) {
                        unsigned char r, g, b;
                        int off = ( y * image.GetWidth() + x );
                        r = d[off * 3 + 0];
                        g = d[off * 3 + 1];
                        b = d[off * 3 + 2];

                        e[off * 4 + 0] = r;
                        e[off * 4 + 1] = g;
                        e[off * 4 + 2] = b;

                        e[off * 4 + 3] =
                                a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
//                        e[off * 4 + 3] = ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                    }
            }

            glColor4f( 1, 1, 1, 1 );
            GLDrawBlendData( x, y, w, h, GL_RGBA, e );
            delete[] ( e );
        } else {
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 ); /* draw data from top to bottom */
            if(image.GetData())
                glDrawPixels( w, h, GL_RGB, GL_UNSIGNED_BYTE, image.GetData() );
            glPixelZoom( 1, 1 );
        }
    }
#endif    
}

void ocpnDC::DrawText( const wxString &text, wxCoord x, wxCoord y )
{
    if( dc )
        dc->DrawText( text, x, y );
#ifdef ocpnUSE_GL
    else {
       wxCoord w = 0;
        wxCoord h = 0;

        if(m_buseTex){
        
            m_texfont.Build( m_font );      // make sure the font is ready
            m_texfont.GetTextExtent(text, &w, &h);
            
            if( w && h ) {
                
                glEnable( GL_BLEND );
                glEnable( GL_TEXTURE_2D );
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

                glPushMatrix();
                glTranslatef(x, y, 0);
                
                glColor3ub( m_textforegroundcolour.Red(), m_textforegroundcolour.Green(),
                            m_textforegroundcolour.Blue() );
                

                m_texfont.RenderString(text);
                glPopMatrix();

                glDisable( GL_TEXTURE_2D );
                glDisable( GL_BLEND );

            }
        }
        else{           
            wxScreenDC sdc;
            sdc.GetTextExtent(text, &w, &h, NULL, NULL, &m_font);
            
            /* create bitmap of appropriate size and select it */
            wxBitmap bmp( w, h );
            wxMemoryDC temp_dc;
            temp_dc.SelectObject( bmp );

            /* fill bitmap with black */
            temp_dc.SetBackground( wxBrush( wxColour( 0, 0, 0 ) ) );
            temp_dc.Clear();

            /* draw the text white */
            temp_dc.SetFont( m_font );
            temp_dc.SetTextForeground( wxColour( 255, 255, 255 ) );
            temp_dc.DrawText( text, 0, 0 );
            temp_dc.SelectObject( wxNullBitmap );

            /* use the data in the bitmap for alpha channel,
             and set the color to text foreground */
            wxImage image = bmp.ConvertToImage();
            if( x < 0 || y < 0 ) { // Allow Drawing text which is offset to start off screen
                int dx = ( x < 0 ? -x : 0 );
                int dy = ( y < 0 ? -y : 0 );
                w = bmp.GetWidth() - dx;
                h = bmp.GetHeight() - dy;
                /* picture is out of viewport */
                if( w <= 0 || h <= 0 ) return;
                image = image.GetSubImage( wxRect( dx, dy, w, h ) );
                x += dx;
                y += dy;
            }

            unsigned char *data = new unsigned char[w * h * 4];
            unsigned char *im = image.GetData();
            
            
            if(im){
                unsigned int r = m_textforegroundcolour.Red();
                unsigned int g = m_textforegroundcolour.Green();
                unsigned int b = m_textforegroundcolour.Blue();
                for( int i = 0; i < h; i++ ){
                    for(int j=0 ; j < w ; j++){
                        unsigned int index = ((i*w) + j) * 4;
                        data[index] = r;
                        data[index+1] = g;
                        data[index+2] = b;
                        data[index+3] = im[((i*w) + j) * 3];
                    }
                }
            }

            glColor4ub( 255, 255, 255, 255 );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 );
            glDrawPixels( w, h, GL_RGBA, GL_UNSIGNED_BYTE, data );
            glPixelZoom( 1, 1 );
            glDisable( GL_BLEND );
            
            delete[] data;
        }            
    }
#endif    
}

void ocpnDC::GetTextExtent( const wxString &string, wxCoord *w, wxCoord *h, wxCoord *descent,
        wxCoord *externalLeading, wxFont *font )
{
    //  Give at least reasonable results on failure.
    if(w) *w = 100;
    if(h) *h = 100;
    
    if( dc ) dc->GetTextExtent( string, w, h, descent, externalLeading, font );
    else {
        wxFont f = m_font;
        if( font ) f = *font;

        if(m_buseTex){
  #ifdef ocpnUSE_GL       
        m_texfont.Build( f );      // make sure the font is ready
        m_texfont.GetTextExtent(string, w, h);
  #else        
        wxMemoryDC temp_dc;
        temp_dc.GetTextExtent( string, w, h, descent, externalLeading, &f );
  #endif      
        }
        else{
            wxMemoryDC temp_dc;
            temp_dc.GetTextExtent( string, w, h, descent, externalLeading, &f );
        }

     }
     
     //  Sometimes GetTextExtent returns really wrong, uninitialized results.
     //  Dunno why....
     if( w && (*w > 500) ) *w = 500;
     if( h && (*h > 500) ) *h = 500;
}

void ocpnDC::ResetBoundingBox()
{
    if( dc ) dc->ResetBoundingBox();
}

void ocpnDC::CalcBoundingBox( wxCoord x, wxCoord y )
{
    if( dc ) dc->CalcBoundingBox( x, y );
}

bool ocpnDC::ConfigurePen()
{
    if( !m_pen.IsOk() ) return false;
    if( m_pen == *wxTRANSPARENT_PEN ) return false;

    wxColour c = m_pen.GetColour();
    int width = m_pen.GetWidth();
#ifdef ocpnUSE_GL
    glColor4ub( c.Red(), c.Green(), c.Blue(), c.Alpha() );
    glLineWidth( wxMax(g_GLMinSymbolLineWidth, width) );
#endif    
    return true;
}

bool ocpnDC::ConfigureBrush()
{
    if( m_brush == wxNullBrush || m_brush.GetStyle() == wxBRUSHSTYLE_TRANSPARENT )
        return false;
#ifdef ocpnUSE_GL
    wxColour c = m_brush.GetColour();
    glColor4ub( c.Red(), c.Green(), c.Blue(), c.Alpha() );
#endif    
    return true;
}

void ocpnDC::GLDrawBlendData( wxCoord x, wxCoord y, wxCoord w, wxCoord h, int format,
        const unsigned char *data )
{
#ifdef ocpnUSE_GL
    glEnable( GL_BLEND );
    glRasterPos2i( x, y );
    glPixelZoom( 1, -1 );
    glDrawPixels( w, h, format, GL_UNSIGNED_BYTE, data );
    glPixelZoom( 1, 1 );
    glDisable( GL_BLEND );
#endif
}
