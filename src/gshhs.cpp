/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GSHHS Chart Object (Global Self-consistent, Hierarchical, High-resolution Shoreline)
 * Author:   Jesper Weissglas for the OpenCPN port.
 *
 *           Derived from http://www.zygrib.org/ and http://sourceforge.net/projects/qtvlm/
 *           which has the original copyright:
 *   zUGrib: meteorologic GRIB file data viewer
 *   Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/file.h>

#include "gshhs.h"

extern wxString *pWorldMapLocation;

//-------------------------------------------------------------------------

GSHHSChart::GSHHSChart() {
    proj = NULL;
    reader = NULL;
}

GSHHSChart::~GSHHSChart() {
    if( proj ) delete proj;
    if( reader ) delete reader;
}

void GSHHSChart::SetColorScheme( ColorScheme scheme ) {

    land = wxColor( 170, 175, 80 );
    water = wxColor( 170, 195, 240 );

    float dim = 1.0;

    switch( scheme ){
        case GLOBAL_COLOR_SCHEME_DUSK:
            dim = 0.5;
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            dim = 0.25;
            break;
        default:
            return;
    }

    land.Set( land.Red()*dim, land.Green()*dim, land.Blue()*dim );
    water.Set( water.Red()*dim, water.Green()*dim, water.Blue()*dim );
}

void GSHHSChart::RenderViewOnDC( ocpnDC& dc, ViewPort& vp ) {

    if( ! proj ) proj = new Projection();
    proj->SetCenterInMap( vp.clon, vp.clat );
    proj->SetScreenSize( vp.rv_rect.width, vp.rv_rect.height );

    // Calculate the horizontal extents in degrees for the enlarged rotated ViewPort
    ViewPort nvp = vp;
    nvp.SetRotationAngle( 0. );
    nvp.pix_width = vp.rv_rect.width;
    nvp.pix_height = vp.rv_rect.height;
    
    double lat_ul, lat_ur;
    double lon_ul, lon_ur;
    
    nvp.GetLLFromPix( wxPoint( 0, 0 ), &lat_ul, &lon_ul );
    nvp.GetLLFromPix( wxPoint( nvp.pix_width, nvp.pix_height ), &lat_ur, &lon_ur );
    
    if( nvp.clon < 0. ) {
        if( ( lon_ul > 0. ) && ( lon_ur < 0. ) ) {
            lon_ul -= 360.;
        }
    } else {
        if( ( lon_ul > 0. ) && ( lon_ur < 0. ) ) {
            lon_ur += 360.;
        }
    }
    
    if( lon_ur < lon_ul ) {
        lon_ur += 360.;
    }
    
    if( lon_ur > 360. ) {
        lon_ur -= 360.;
        lon_ul -= 360.;
    }

    //  And set the scale for the gshhs renderer
    proj->SetScale( (float)vp.rv_rect.width / ( fabs(lon_ul - lon_ur ) ));
        

    if( ! reader ) {
        reader = new GshhsReader( proj );
        if( reader->GetPolyVersion() < 210 || reader->GetPolyVersion() > 220 ) {
            wxLogMessage( _T("GSHHS World chart files have wrong version. Found %ld, expected 210-220."),
                    reader->GetPolyVersion() );
        } else {
            wxLogMessage(
                    _T("Background world map loaded from GSHHS datafiles found in: ") +
                    *pWorldMapLocation );
        }
    }

//    dc.SetBackground( wxBrush( water ) );
//    dc.Clear();
    dc.SetBrush( wxBrush( water ) );
    dc.DrawRectangle( 0, 0, vp.rv_rect.width, vp.rv_rect.height );
    
    reader->drawContinents( dc, proj, water, land );
    reader->drawBoundaries( dc, proj );
}

//-------------------------------------------------------------------------

Projection::Projection() :
        W( 0 ), H( 0 ), CX( 0 ), CY( 0 ), xW( -90 ), xE( 90 ), yN( 90 ), yS( -90 ), PX( 0 ), PY( 0 )
{
    frozen = false;
    scalemax = 10e20;
    scale = -1;
    useTempo = true;
}

Projection::Projection( int w, int h, double cx, double cy ) :
        W( 0 ), H( 0 ), CX( 0 ), CY( 0 ), xW( -90 ), xE( 90 ), yN( 90 ), yS( -90 ), PX( 0 ), PY( 0 )
{
    frozen = false;
    scalemax = 10e20;
    scale = -1;
    SetScreenSize( w, h );
    SetCenterInMap( cx, cy );
    useTempo = true;
}

double Projection::degToRad( double d ) const { return d * M_PI / 180; }
double Projection::radToDeg( double r ) const { return r * 180 / M_PI; }

void Projection::SetScale( double sc )
{
    scale = sc;
    if( scale < scaleall ) scale = scaleall;
    if( scale > scalemax ) scale = scalemax;
    updateBoundaries();
}

void Projection::SetCenterInMap( double x, double y )
{
    while( x > 180.0 ) {
        x -= 360.0;
    }
    while( x < -180.0 ) {
        x += 360.0;
    }
    CX = x;
    CY = y;

    /* compute projection */
    PX = CX;
    PY = radToDeg( log( tan( degToRad( CY ) / 2 + M_PI_4 ) ) );

    updateBoundaries();
}

void Projection::SetScreenSize( int w, int h )
{
    W = w;
    H = h;

    /* compute scaleall */

    double sx, sy, sy1, sy2;
    sx = W / 360.0;
    sy1 = log( tan( degToRad( 84 ) / 2 + M_PI_4 ) );
    sy2 = log( tan( degToRad( -80 ) / 2 + M_PI_4 ) );
    sy = H / fabs( radToDeg( sy1 - sy2 ) );
    scaleall = ( sy < sx ) ? sy : sx;

    double sX, sY, sYN, sYS;
    sX = W / fabs( xE - xW );
    sYN = log( tan( degToRad( yN ) / 2 + M_PI_4 ) );
    sYS = log( tan( degToRad( yS ) / 2 + M_PI_4 ) );
    sY = H / fabs( radToDeg( sYN - sYS ) );

    if( scale < scaleall ) {
        scale = scaleall;
    }

    updateBoundaries();
}

void Projection::updateBoundaries()
{
    /* lat */
    yS = radToDeg( 2 * atan( exp( (double) ( degToRad( PY - H / ( 2 * scale ) ) ) ) ) - M_PI_2 );
    yN = radToDeg( 2 * atan( exp( (double) ( degToRad( PY + H / ( 2 * scale ) ) ) ) ) - M_PI_2 );

    /* lon */
    xW = PX - W / ( 2 * scale );
    xE = PX + W / ( 2 * scale );

    /* xW and yN => upper corner */

    if( ( getW() * getH() ) != 0 )
        coefremp = 10000.0 * fabs( ( ( xE - xW ) * ( yN - yS ) ) / ( getW() * getH() ) );
    else
        coefremp = 10000.0;
}

inline bool Projection::isInBounderies (int x,int y) const
{
    return (x>=0 && y>=0 && x<=W && y<=H);
}

inline void Projection::map2screen( double x, double y, int *i, int *j ) const
{
    if( y <= -90 ) y = -89.9;
    if( y >= 90 ) y = 89.9;

    *i = wxRound( scale * ( x - xW ) );
    *j = H / 2 + wxRound( scale * ( PY - radToDeg( log( tan( degToRad( y ) / 2 + M_PI_4 ) ) ) ) );
}
inline void Projection::map2screenDouble( double x, double y, double *i, double *j ) const
{
    if( y <= -90 ) y = -89.9999999999999999999999999999999999999999999999999999999;
    if( y >= 90 ) y = 89.999999999999999999999999999999999999999999999999999999999;
    double diff = x - xW;
    *i = scale * diff;
    double trick = PY - radToDeg( log( tan( degToRad( y ) / (double) 2.0 + M_PI_4 ) ) );
    *j = ( (double) H / (double) 2.0 + ( scale * trick ) );
}

inline bool Projection::intersect(double w,double e,double s,double n) const
{
    return ! (w>xE || e<xW || s>yN || n<yS);
}

//-------------------------------------------------------------------------

GshhsPolyCell::GshhsPolyCell( FILE *fpoly_, int x0_, int y0_, Projection *proj_,
        PolygonFileHeader *header_ )
{
    proj = proj_;
    header = header_;
    fpoly = fpoly_;
    x0cell = x0_;
    y0cell = y0_;

    ReadPolygonFile( fpoly, x0cell, y0cell, header->pasx, header->pasy, &poly1, &poly2, &poly3,
            &poly4, &poly5 );

    int cnt = 0;
    for( unsigned int i = 0; i < poly1.size(); i++ )
        cnt += poly1.at( i ).size();

    //qWarning() << "Cell " << x0cell << "," << y0cell << ": " << poly1.count() << " poly in p1 - total points:"<<cnt;

}

GshhsPolyCell::~GshhsPolyCell()
{

}

#define READ_POLY(POLY) { \
double X,Y; \
contour tmp_contour; \
int num_vertices,num_contours; \
int value; \
POLY->clear(); \
fread(&(num_contours), sizeof(int), 1, polyfile); \
for (int c= 0; c < num_contours; c++) \
{ \
    fread(&(value), sizeof(int), 1, polyfile); /* discarding hole value */ \
    fread(&(value), sizeof(int), 1, polyfile); \
    num_vertices=value; \
    tmp_contour.clear(); \
    for (int v= 0; v < num_vertices; v++) \
    { \
        fread(&(X), sizeof(double), 1, polyfile); \
        fread(&(Y), sizeof(double), 1, polyfile); \
        tmp_contour.push_back(wxRealPoint(X*GSHHS_SCL,Y*GSHHS_SCL)); \
    } \
    POLY->push_back(tmp_contour); \
} \
}

void GshhsPolyCell::ReadPolygonFile( FILE *polyfile, int x, int y, int pas_x, int pas_y,
        contour_list *p1, contour_list *p2, contour_list *p3, contour_list *p4, contour_list *p5 )
{
    int pos_data;
    int tab_data;

    tab_data = ( x / pas_x ) * ( 180 / pas_y ) + ( y + 90 ) / pas_y;
    fseek( polyfile, sizeof(PolygonFileHeader) + tab_data * sizeof(int), SEEK_SET );
    fread( &pos_data, sizeof(int), 1, polyfile );

    fseek( polyfile, pos_data, SEEK_SET );

    READ_POLY( p1 )
    READ_POLY( p2 )
    READ_POLY( p3 )
    READ_POLY( p4 )
    READ_POLY( p5 )
}

void GshhsPolyCell::DrawPolygonFilled( ocpnDC &pnt, contour_list * p, double dx, Projection *proj,
        wxColor color )
{
    int x, y;
    unsigned int c, v;
    int pointCount;
    wxPoint* poly_pt;

    wxPen myPen = wxNullPen;
    pnt.SetPen( myPen );
    pnt.SetBrush( color );

    int x_old = 0;
    int y_old = 0;

    for( c = 0; c < p->size(); c++ ) {
        if( !p->at( c ).size() ) continue;

        poly_pt = new wxPoint[ p->at(c).size() ];
        pointCount = 0;

        for( v = 0; v < p->at( c ).size(); v++ ) {
            proj->map2screen( p->at( c ).at( v ).x + dx, p->at( c ).at( v ).y, &x, &y );

            if( v == 0 || x != x_old || y != y_old ) {
                poly_pt[pointCount].x = x;
                poly_pt[pointCount].y = y;
                pointCount++;
                x_old = x;
                y_old = y;
            }
        }

        if(pointCount>1)
            pnt.DrawPolygonTessellated( pointCount, poly_pt, 0, 0 );
        delete[] poly_pt;
    }
}

#define DRAW_POLY_FILLED(POLY,COL) if(POLY) DrawPolygonFilled(pnt,POLY,dx,proj,COL);

void GshhsPolyCell::drawMapPlain( ocpnDC &pnt, double dx, Projection *proj, wxColor seaColor,
        wxColor landColor )
{
    DRAW_POLY_FILLED( &poly1, landColor )
    DRAW_POLY_FILLED( &poly2, seaColor )
    DRAW_POLY_FILLED( &poly3, landColor )
    DRAW_POLY_FILLED( &poly4, seaColor )
    DRAW_POLY_FILLED( &poly5, landColor )
}

void GshhsPolyCell::DrawPolygonContour( ocpnDC &pnt, contour_list * p, double dx, Projection *proj )
{
    double x1, y1, x2, y2;
    double long_max, lat_max, long_min, lat_min;

    long_min = (double) x0cell;
    lat_min = (double) y0cell;
    long_max = ( (double) x0cell + (double) header->pasx );
    lat_max = ( (double) y0cell + (double) header->pasy );

    //qWarning()  << long_min << "," << lat_min << long_max << "," << lat_max;

    for( unsigned int i = 0; i < p->size(); i++ ) {
        if( !p->at( i ).size() ) continue;
		unsigned int v;
		for( v = 0; v < ( p->at( i ).size() - 1 ); v++ ) {
            x1 = p->at( i ).at( v ).x;
            y1 = p->at( i ).at( v ).y;
            x2 = p->at( i ).at( v + 1 ).x;
            y2 = p->at( i ).at( v + 1 ).y;

            // Elimination des traits verticaux et horizontaux
            if( ( ( ( x1 == x2 ) && ( ( x1 == long_min ) || ( x1 == long_max ) ) )
                    || ( ( y1 == y2 ) && ( ( y1 == lat_min ) || ( y1 == lat_max ) ) ) ) == 0 ) {
                int a, b, c, d;
                double A, B, C, D;
                proj->map2screen( x1 + dx, y1, &a, &b );
                proj->map2screen( x2 + dx, y2, &c, &d );
                proj->map2screenDouble( x1 + dx, y1, &A, &B );
                proj->map2screenDouble( x2 + dx, y2, &C, &D );
                if( a != c || b != d ) pnt.DrawLine( a, b, c, d );
                if( A != C || B != D ) {
                    if( proj->isInBounderies( a, b ) || proj->isInBounderies( c, d ) )
                        coasts.push_back( QLineF( A, B, C, D ) );
                }
            }
        }

        x1 = p->at( i ).at( v ).x;
        y1 = p->at( i ).at( v ).y;
        x2 = p->at( i ).at( 0 ).x;
        y2 = p->at( i ).at( 0 ).y;

        if( ( ( ( x1 == x2 ) && ( ( x1 == long_min ) || ( x1 == long_max ) ) )
                || ( ( y1 == y2 ) && ( ( y1 == lat_min ) || ( y1 == lat_max ) ) ) ) == 0 ) {
            int a, b, c, d;
            double A, B, C, D;
            proj->map2screen( x1 + dx, y1, &a, &b );
            proj->map2screen( x2 + dx, y2, &c, &d );
            proj->map2screenDouble( x1 + dx, y1, &A, &B );
            proj->map2screenDouble( x2 + dx, y2, &C, &D );
            if( a != c || b != d ) pnt.DrawLine( a, b, c, d );
            if( A != C || B != D ) {
                if( proj->isInBounderies( a, b ) || proj->isInBounderies( c, d ) )
                    coasts.push_back( QLineF( A, B, C, D ) );
            }
        }
    }
}

#define DRAW_POLY_CONTOUR(POLY) if(POLY) DrawPolygonContour(pnt,POLY,dx,proj);

void GshhsPolyCell::drawSeaBorderLines( ocpnDC &pnt, double dx, Projection *proj )
{
    coasts.clear();
    DRAW_POLY_CONTOUR( &poly1 )
    DRAW_POLY_CONTOUR( &poly2 )
    DRAW_POLY_CONTOUR( &poly3 )
    DRAW_POLY_CONTOUR( &poly4 )
    DRAW_POLY_CONTOUR( &poly5 )
}

//========================================================================

GshhsPolyReader::GshhsPolyReader( int quality )
{
    fpoly = NULL;

    for( int i = 0; i < 360; i++ ) {
        for( int j = 0; j < 180; j++ ) {
            allCells[i][j] = NULL;
        }
    }
    currentQuality = -1;
    InitializeLoadQuality( quality );
    this->abortRequested = false;
}

//-------------------------------------------------------------------------
GshhsPolyReader::~GshhsPolyReader()
{
    for( int i = 0; i < 360; i++ ) {
        for( int j = 0; j < 180; j++ ) {
            if( allCells[i][j] != NULL ) {
                delete allCells[i][j];
                allCells[i][j] = NULL;
            }
        }
    }
}

//-------------------------------------------------------------------------
int GshhsPolyReader::ReadPolyVersion()
{
    char txtn = 'c';
    wxString fname = GshhsReader::getFileName_Land( 0 );
    if( fpoly ) fclose( fpoly );
    fpoly = fopen( fname.mb_str(), "rb" );

    /* init header */
    if( !fpoly ) return 0;

    readPolygonFileHeader( fpoly, &polyHeader );

    return polyHeader.version;
}

void GshhsPolyReader::InitializeLoadQuality( int quality )  // 5 levels: 0=low ... 4=full
{
    if( currentQuality != quality ) {
        currentQuality = quality;

        wxString fname = GshhsReader::getFileName_Land( quality );

        if( fpoly ) fclose( fpoly );

        fpoly = fopen( fname.mb_str(), "rb" );
        if( fpoly ) readPolygonFileHeader( fpoly, &polyHeader );

        for( int i = 0; i < 360; i++ ) {
            for( int j = 0; j < 180; j++ ) {
                if( allCells[i][j] != NULL ) {
                    delete allCells[i][j];
                    allCells[i][j] = NULL;
                }
            }
        }
    }
}

bool GshhsPolyReader::crossing1( QLineF trajectWorld )
{
    if( !proj || proj == NULL ) return false;

    int cxmin, cxmax, cymax, cymin;
    cxmin = (int) floor( wxMin( trajectWorld.p1().x, trajectWorld.p2().x ) );
    cxmax = (int) ceil( wxMax( trajectWorld.p1().x, trajectWorld.p2().x ) );

    if(cxmin < 0) {
        cxmin += 360;
        cxmax += 360;
    }

    if(cxmax - cxmin > 180) { /* dont go long way around world */
        cxmin = (int) floor( wxMax( trajectWorld.p1().x, trajectWorld.p2().x ) ) - 360;
        cxmax = (int) ceil( wxMin( trajectWorld.p1().x, trajectWorld.p2().x ) );
    }

    cymin = (int) floor( wxMin( trajectWorld.p1().y, trajectWorld.p2().y ) );
    cymax = (int) ceil( wxMax( trajectWorld.p1().y, trajectWorld.p2().y ) );
    int cx, cxx, cy;
    GshhsPolyCell *cel;

    for( cx = cxmin; cx < cxmax; cx++ ) {
        cxx = cx;
        while( cxx < 0 )
            cxx += 360;
        while( cxx >= 360 )
            cxx -= 360;

        double p1x=trajectWorld.p1().x, p2x = trajectWorld.p2().x;
        if(cxx < 180) {
            if(p1x > 180) p1x -= 360;
            if(p2x > 180) p2x -= 360;
        } else {
            if(p1x < 180) p1x += 360;
            if(p2x < 180) p2x += 360;
        }

        QLineF rtrajectWorld(p1x, trajectWorld.p1().y, p2x, trajectWorld.p2().y);                

        for( cy = cymin; cy < cymax; cy++ ) {
            if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
                if( allCells[cxx][cy + 90] == NULL ) {
                    cel = new GshhsPolyCell( fpoly, cxx, cy, proj, &polyHeader );
                    assert( cel );
                    allCells[cxx][cy + 90] = cel;
                } else
                    cel = allCells[cxx][cy + 90];

                contour_list &poly1 = cel->getPoly1();
                for( unsigned int pi = 0; pi < poly1.size(); pi++ ) {
                    contour c = poly1[pi];
                    double lx = c[c.size()-1].x, ly = c[c.size()-1].y;
                    for( unsigned int pj = 0; pj < c.size(); pj++ ) {
                        QLineF l(lx, ly, c[pj].x, c[pj].y);
                        if( my_intersects( rtrajectWorld, l ) )
                            return true;
                        lx = c[pj].x, ly = c[pj].y;
                    }
                }
            }
        }
    }
    return false;
}

void GshhsPolyReader::readPolygonFileHeader( FILE *polyfile, PolygonFileHeader *header )
{
//    int FReadResult = 0;

    fseek( polyfile, 0, SEEK_SET );
    fread( header, sizeof(PolygonFileHeader), 1, polyfile );
}

//-------------------------------------------------------------------------
void GshhsPolyReader::drawGshhsPolyMapPlain( ocpnDC &pnt, Projection *proj, wxColor seaColor,
        wxColor landColor )
{
    if( !fpoly ) return;

    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    cxmin = (int) floor( proj->getXmin() );
    cxmax = (int) ceil( proj->getXmax() );
    cymin = (int) floor( proj->getYmin() );
    cymax = (int) ceil( proj->getYmax() );
    int dx, cx, cxx, cy;
    GshhsPolyCell *cel;

    for( cx = cxmin; cx < cxmax; cx++ ) {
        cxx = cx;
        while( cxx < 0 )
            cxx += 360;
        while( cxx >= 360 )
            cxx -= 360;

        for( cy = cymin; cy < cymax; cy++ ) {
            if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
                if( allCells[cxx][cy + 90] == NULL ) {
                    cel = new GshhsPolyCell( fpoly, cxx, cy, proj, &polyHeader );
                    assert( cel );
                    allCells[cxx][cy + 90] = cel;
                } else {
                    cel = allCells[cxx][cy + 90];
                }
                dx = cx - cxx;
                cel->drawMapPlain( pnt, dx, proj, seaColor, landColor );
            }
        }
    }
}

//-------------------------------------------------------------------------
void GshhsPolyReader::drawGshhsPolyMapSeaBorders( ocpnDC &pnt, Projection *proj )
{
    if( !fpoly ) return;
    this->abortRequested = true;
    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    cxmin = (int) floor( proj->getXmin() );
    cxmax = (int) ceil( proj->getXmax() );
    cymin = (int) floor( proj->getYmin() );
    cymax = (int) ceil( proj->getYmax() );
    int dx, cx, cxx, cy;
    GshhsPolyCell *cel;

    for( cx = cxmin; cx < cxmax; cx++ ) {
        cxx = cx;
        while( cxx < 0 )
            cxx += 360;
        while( cxx >= 360 )
            cxx -= 360;

        for( cy = cymin; cy < cymax; cy++ ) {
            if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
                if( allCells[cxx][cy + 90] == NULL ) {
                    cel = new GshhsPolyCell( fpoly, cxx, cy, proj, &polyHeader );
                    assert( cel );
                    allCells[cxx][cy + 90] = cel;
                } else {
                    cel = allCells[cxx][cy + 90];
                }
                dx = cx - cxx;
                cel->drawSeaBorderLines( pnt, dx, proj );
            }
        }
    }
    this->abortRequested = false;
}

int GshhsPolygon::readInt4()
{
    union {
		unsigned int n;
		unsigned char buf[4];
	} res;

	unsigned char in[4];

    int nb = 0;
    nb += fread( &in, 1, 4, file );
	res.buf[3] = in[0];
	res.buf[2] = in[1];
	res.buf[1] = in[2];
	res.buf[0] = in[3];

    if( nb != 4 ) {
        ok = false;
		res.n = 0;
    }

    return res.n;
}


int GshhsPolygon::readInt2()
{
    union {
		unsigned int n;
		unsigned char buf[4];
	} v;

    int nb = 0;
    nb += fread( &v.buf[0], 2, 1, file );
    if( nb != 2 ) {
        ok = false;
        v.n = 0;
    }
    return v.buf[1]<<8 | v.buf[0];
}


GshhsPolygon::GshhsPolygon( FILE *file_ )
{
    file = file_;
    ok = true;
    id = readInt4();
    n = readInt4();
    flag = readInt4();
    west = readInt4() * 1e-6;
    east = readInt4() * 1e-6;
    south = readInt4() * 1e-6;
    north = readInt4() * 1e-6;
    area = readInt4();

    if( ( ( flag >> 8 ) & 255 ) >= 7 ) { //GSHHS Release 2.0
        areaFull = readInt4();
        container = readInt4();
        ancestor = readInt4();

        greenwich = ( flag >> 16 ) & 1;
        antarctic = ( west == 0 && east == 360 );
        if( ok ) {
            double x = 0, y = 0;
            for( int i = 0; i < n; i++ ) {
                x = readInt4() * 1e-6;
                if( greenwich && x > 270 ) x -= 360;
                y = readInt4() * 1e-6;
                lsPoints.push_back( new GshhsPoint( x, y ) );
            }
            if( antarctic ) {
                lsPoints.insert( lsPoints.begin(), new GshhsPoint( 360, y ) );
                lsPoints.insert( lsPoints.begin(), new GshhsPoint( 360, -90 ) );
                lsPoints.push_back( new GshhsPoint( 0, -90 ) );
            }
        }
    } else {
        greenwich = ( flag >> 16 ) & 1;
        antarctic = ( west == 0 && east == 360 );
        if( ok ) {
            for( int i = 0; i < n; i++ ) {
                double x = 0, y = 0;
                x = readInt4() * 1e-6;
                if( greenwich && x > 270 ) x -= 360;
                y = readInt4() * 1e-6;
                lsPoints.push_back( new GshhsPoint( x, y ) );
            }
        }
    }
}

//--------------------------------------------------------

GshhsPolygon::~GshhsPolygon()
{
    std::vector<GshhsPoint *>::iterator itp;
    for( itp = lsPoints.begin(); itp != lsPoints.end(); itp++ ) {
        delete *itp;
        *itp = NULL;
    }
    lsPoints.clear();
}

//==========================================================

GshhsReader::GshhsReader( Projection* proj )
{
    int maxQualityAvailable = -1;
    int minQualityAvailable = -1;

    for( int i=0; i<5; i++ ) {
        qualityAvailable[i] = false;
        if( GshhsReader::gshhsFilesExists( i ) ) {
            qualityAvailable[i] = true;
            if( minQualityAvailable < 0 ) minQualityAvailable = i;
            maxQualityAvailable = i;
        }
    }

    if( maxQualityAvailable < 0 ) {
        wxString msg( _T("Unable to initialize background world map. No GSHHS datafiles found in ") );
        msg += *pWorldMapLocation;
        wxLogMessage( msg );
    }

    int q = selectBestQuality( proj );
    if( ! qualityAvailable[q] ) {
        q = minQualityAvailable;
    }

    gshhsPoly_reader = new GshhsPolyReader( q );
    setProj( proj );

    for( int qual = 0; qual < 5; qual++ ) {
        lsPoly_boundaries[qual] = new std::vector<GshhsPolygon*>;
        lsPoly_rivers[qual] = new std::vector<GshhsPolygon*>;
    }
    LoadQuality( q );
}

int GshhsReader::ReadPolyVersion()
{
    return gshhsPoly_reader->ReadPolyVersion();
}

//-------------------------------------------------------

GshhsReader::~GshhsReader()
{
    clearLists();
    delete gshhsPoly_reader;
}

//-----------------------------------------------------------------------
void GshhsReader::clearLists()
{
    std::vector<GshhsPolygon*>::iterator itp;
    for( int qual = 0; qual < 5; qual++ ) {
        for( itp = lsPoly_boundaries[qual]->begin(); itp != lsPoly_boundaries[qual]->end();
                itp++ ) {
            delete *itp;
            *itp = NULL;
        }
        for( itp = lsPoly_rivers[qual]->begin(); itp != lsPoly_rivers[qual]->end(); itp++ ) {
            delete *itp;
            *itp = NULL;
        }

        lsPoly_boundaries[qual]->clear();
        lsPoly_rivers[qual]->clear();
        delete lsPoly_boundaries[qual];
        delete lsPoly_rivers[qual];
    }
}
//-----------------------------------------------------------------------

wxString GshhsReader::getNameExtension( int quality )
{
    wxString ext;
    switch( quality ){
        case 0:
            ext = _T("c");
            break;
        case 1:
            ext = _T("l");
            break;
        case 2:
            ext = _T("i");
            break;
        case 3:
            ext = _T("h");
            break;
        case 4:
            ext = _T("f");
            break;
        default:
            ext = _T("l");
            break;
    }
    return ext;
}

wxString GshhsReader::getFileName_Land( int quality )
{
    wxString ext = GshhsReader::getNameExtension( quality );
    wxString fname = *pWorldMapLocation + wxString::Format( _T("poly-%c-1.dat"), ext.GetChar(0) );
    return fname;
}

wxString GshhsReader::getFileName_boundaries( int quality )
{
    wxString ext = GshhsReader::getNameExtension( quality );
    wxString fname = *pWorldMapLocation + wxString::Format( _T("wdb_borders_%c.b"), ext.GetChar(0) );
    return fname;
}

wxString GshhsReader::getFileName_rivers( int quality )
{
    wxString ext = GshhsReader::getNameExtension( quality );
    wxString fname = *pWorldMapLocation + wxString::Format( _T("wdb_rivers_%c.b"), ext.GetChar(0) );
    return fname;
}

//-----------------------------------------------------------------------
bool GshhsReader::gshhsFilesExists( int quality )
{
    if( ! wxFile::Access( GshhsReader::getFileName_Land( quality ), wxFile::read ) ) return false;
    if( ! wxFile::Access( GshhsReader::getFileName_boundaries( quality ), wxFile::read ) ) return false;
    if( ! wxFile::Access( GshhsReader::getFileName_rivers( quality ), wxFile::read ) ) return false;

    return true;
}

//-----------------------------------------------------------------------
void GshhsReader::LoadQuality( int newQuality ) // 5 levels: 0=low ... 4=full
{
    if( quality == newQuality ) return;

    wxStopWatch perftimer;

    wxString fname;
    FILE *file;
    bool ok;

    quality = newQuality;
    if( quality < 0 ) quality = 0;
    else if( quality > 4 ) quality = 4;

    gshhsPoly_reader->InitializeLoadQuality( quality );

    if( lsPoly_boundaries[quality]->size() == 0 ) {
        fname = getFileName_boundaries( quality );
        file = fopen( fname.mb_str(), "rb" );

        if( file != NULL ) {
            ok = true;
            while( ok ) {
                GshhsPolygon *poly = new GshhsPolygon( file );

                ok = poly->isOk();
                if( ok )
                    if( poly->getLevel() < 2 )
                        lsPoly_boundaries[quality]->push_back( poly );
                    else delete poly;
                else delete poly;
            }
            fclose( file );
        }
    }

    if( lsPoly_rivers[quality]->size() == 0 ) {
        fname = getFileName_rivers( quality );
        file = fopen( fname.mb_str(), "rb" );
        if( file != NULL ) {
            ok = true;
            while( ok ) {
                GshhsPolygon *poly = new GshhsPolygon( file );
                ok = poly->isOk();
                if( ok ) {
                    lsPoly_rivers[quality]->push_back( poly );
                }
                else delete poly;
            }
            fclose( file );
        }
    }

    wxLogMessage( _T("Loading World Chart Q=%d in %ld ms."), quality, perftimer.Time());

}

//-----------------------------------------------------------------------
std::vector<GshhsPolygon*> & GshhsReader::getList_boundaries()
{
    return *lsPoly_boundaries[quality];
}
//-----------------------------------------------------------------------
std::vector<GshhsPolygon*> & GshhsReader::getList_rivers()
{
    return *lsPoly_rivers[quality];
}

//=====================================================================

int GshhsReader::GSHHS_scaledPoints( GshhsPolygon *pol, wxPoint *pts, double decx, Projection *proj )
{

    if( !proj->intersect( pol->west + decx, pol->east + decx, pol->south, pol->north ) ) {
        return 0;
    }

    // Remove small polygons.
    int a1, b1;
    int a2, b2;
    proj->map2screen( pol->west + decx, pol->north, &a1, &b1 );
    proj->map2screen( pol->east + decx, pol->south, &a2, &b2 );
    if( a1 == a2 && b1 == b2 ) {
        return 0;
    }

    double x, y;
    std::vector<GshhsPoint *>::iterator itp;
    int xx, yy, oxx = 0, oyy = 0;
    int j = 0;

    for( itp = ( pol->lsPoints ).begin(); itp != ( pol->lsPoints ).end(); itp++ ) {
        x = ( *itp )->lon + decx;
        y = ( *itp )->lat;
        // Adjust scale
        proj->map2screen( x, y, &xx, &yy );
        if( j == 0 || ( oxx != xx || oyy != yy ) )  // Remove close points
                {
            oxx = xx;
            oyy = yy;
            pts[j].x = xx;
            pts[j].y = yy;
            j++;
        }
    }

    return j;
}

//-----------------------------------------------------------------------
void GshhsReader::GsshDrawLines( ocpnDC &pnt, std::vector<GshhsPolygon*> &lst, Projection *proj,
        bool isClosed )
{
    std::vector<GshhsPolygon*>::iterator iter;
    GshhsPolygon *pol;
    wxPoint *pts = NULL;
    int i;
    int nbp;

    int nbmax = 10000;
    pts = new wxPoint[nbmax];
    assert( pts );

    for( i = 0, iter = lst.begin(); iter != lst.end(); iter++, i++ ) {
        pol = *iter;

        if( nbmax < pol->n + 2 ) {
            nbmax = pol->n + 2;
            delete[] pts;
            pts = new wxPoint[nbmax];
            assert( pts );
        }

        nbp = GSHHS_scaledPoints( pol, pts, 0, proj );
        if( nbp > 1 ) {
            if( pol->isAntarctic() ) {
                pts++;
                nbp -= 2;
                pnt.DrawLines( nbp, pts );
                pts--;
            } else {
                pnt.DrawLines( nbp, pts );
                if( isClosed ) pnt.DrawLine( pts[0].x, pts[0].y, pts[nbp - 1].x, pts[nbp - 1].y );
            }
        }

        nbp = GSHHS_scaledPoints( pol, pts, -360, proj );
        if( nbp > 1 ) {
            if( pol->isAntarctic() ) {
                pts++;
                nbp -= 2;
                pnt.DrawLines( nbp, pts );
                pts--;
            } else {
                pnt.DrawLines( nbp, pts );
                if( isClosed ) pnt.DrawLine( pts[0].x, pts[0].y, pts[nbp - 1].x, pts[nbp - 1].y );
            }
        }
    }
    delete[] pts;
}

//-----------------------------------------------------------------------
void GshhsReader::drawBackground( ocpnDC &pnt, Projection *proj, wxColor seaColor,
        wxColor backgroundColor )
{
    pnt.SetBrush( backgroundColor );
    pnt.SetPen( backgroundColor );
    pnt.DrawRectangle( 0, 0, proj->getW(), proj->getH() );

    pnt.SetBrush( seaColor );
    pnt.SetPen( seaColor );
    int x0, y0, x1, y1;
    proj->map2screen( 0, 90, &x0, &y0 );
    proj->map2screen( 0, -90, &x1, &y1 );

    pnt.DrawRectangle( 0, y0, proj->getW(), y1 - y0 );

}
//-----------------------------------------------------------------------
void GshhsReader::drawContinents( ocpnDC &pnt, Projection *proj, wxColor seaColor,
        wxColor landColor )
{
    LoadQuality( selectBestQuality( proj ) );
    gshhsPoly_reader->drawGshhsPolyMapPlain( pnt, proj, seaColor, landColor );
}

//-----------------------------------------------------------------------
void GshhsReader::drawSeaBorders( ocpnDC &pnt, Projection *proj )
{
    pnt.SetBrush( *wxTRANSPARENT_BRUSH );
    gshhsPoly_reader->drawGshhsPolyMapSeaBorders( pnt, proj );
}

//-----------------------------------------------------------------------
void GshhsReader::drawBoundaries( ocpnDC &pnt, Projection *proj )
{
    pnt.SetBrush( *wxTRANSPARENT_BRUSH );

	if( pnt.GetDC() ) {
        wxPen* pen = wxThePenList->FindOrCreatePen( *wxBLACK, 1, wxDOT );
		pnt.SetPen( *pen );
    } else {
        wxPen* pen = wxThePenList->FindOrCreatePen( wxColor( 0, 0, 0, 80 ), 1, wxDOT );
        pnt.SetPen( *pen );
    }
    GsshDrawLines( pnt, getList_boundaries(), proj, false );
}

//-----------------------------------------------------------------------
void GshhsReader::drawRivers( ocpnDC &pnt, Projection *proj )
{
    GsshDrawLines( pnt, getList_rivers(), proj, false );
}

//-----------------------------------------------------------------------
int GshhsReader::selectBestQuality( Projection *proj )
{
    int bestQuality = 0;
    if( proj->getCoefremp() > 60 ) bestQuality = 0;
    else if( proj->getCoefremp() > 1 ) bestQuality = 1;
    else if( proj->getCoefremp() > 0.05 ) bestQuality = 2;
    else if( proj->getCoefremp() > 0.005 ) bestQuality = 3;
    else bestQuality = 4;

    while( !qualityAvailable[bestQuality] ) {
        bestQuality--;
        if( bestQuality < 0 ) break;
    }

    if( bestQuality < 0 )
        for( int i=0; i<5; i++ )
            if( qualityAvailable[i] ) bestQuality = i;

    return bestQuality;
}

/* so plugins can determine if a line segment crosses land */
bool gshhsCrossesLand(double lat1, double lon1, double lat2, double lon2)
{
    static GshhsReader *reader;
    static Projection proj;
    if(reader == NULL)
        reader = new GshhsReader(&proj);

    if(lon1 < 0)
        lon1 += 360;
    if(lon2 < 0)
        lon2 += 360;

    QLineF trajectWorld(lon1, lat1, lon2, lat2);
    return reader->crossing1(trajectWorld);
}
