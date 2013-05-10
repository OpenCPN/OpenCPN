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

#ifndef GSHHS_H
#define GSHHS_H

#include <stdio.h>
#include <string>
#include <math.h>
#include <assert.h>
#include <vector>

#include <wx/geometry.h>

#include "ocpn_types.h"
#include "ocpndc.h"

#ifdef __WXMSW__
#pragma warning(disable: 4251)   // relates to std::string fpath
#endif

//-------------------------------------------------------------------------
// Subset of original Projection, only whats needed for GSHHS.

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616
#endif

class Projection {
public:
    Projection();
    Projection(int w, int h, double cx, double cy);
    void SetScreenSize(int w, int h);
    void SetCenterInMap(double x, double y);
    void SetScale(double sc);
    bool isInBounderies( int x, int y ) const;
    double getXmin() const { return xW; }
    double getXmax() const { return xE; }
    double getYmin() const { return yS; }
    double getYmax() const { return yN; }
    void map2screen( double wx, double wy, int* x, int* y ) const;
    void map2screenDouble( double wx, double wy, double* x, double* y ) const;
    bool intersect( double w, double e, double s, double n ) const;
    int getW() { return W; }
    int getH() { return H; }
    double getCoefremp() { return coefremp; }
    double degToRad( double d ) const;
    double radToDeg( double r ) const;


private:
    void updateBoundaries();

    int W, H;
    double CX, CY;
    double xW, xE, yN, yS;  // fenetre visible (repere longitude/latitude)
    double PX,PY;       // center in mercator projection
    double scale;       // Echelle courante
    double scalemax;    // Echelle maxi
    double scaleall;    // Echelle pour afficher le monde entier
    double coefremp;       // Coefficient de remplissage (surface_visible/pixels)
    bool frozen;
    bool useTempo;
};

//-------------------------------------------------------------------------
class QLineF {
public:
    QLineF( double x1, double y1, double x2, double y2 ) {
        m_p1 = wxRealPoint( x1, y1 );
        m_p2 = wxRealPoint( x2, y2 );
    }
    wxRealPoint& p1() { return m_p1; }
    wxRealPoint& p2() { return m_p2; }
    wxRealPoint m_p1, m_p2;
};

#define GSHHS_SCL    1.0e-6    /* Convert micro-degrees to degrees */
#  define INTER_MAX_LIMIT 1.0000001
#  define INTER_MIN_LIMIT -0.0000001

struct PolygonFileHeader {
    int version;
    int pasx;
    int pasy;
    int xmin;
    int ymin;
    int xmax;
    int ymax;
    int p1;
    int p2;
    int p3;
    int p4;
    int p5;
};

typedef std::vector<wxRealPoint> contour;
typedef std::vector<contour> contour_list;

//==========================================================================

class GshhsPolyCell {
public:

    GshhsPolyCell( FILE *fpoly, int x0, int y0, Projection *proj, PolygonFileHeader *header );
    ~GshhsPolyCell();

    void drawMapPlain( ocpnDC &pnt, double dx, Projection *proj, wxColor seaColor,
            wxColor landColor );

    void drawSeaBorderLines( ocpnDC &pnt, double dx, Projection *proj );
    std::vector<QLineF> * getCoasts() { return &coasts; }
    contour_list &getPoly1() { return poly1; }

private:
    int nbpoints;
    int x0cell, y0cell;

    FILE *fpoly;

    std::vector<QLineF> coasts;
    Projection *proj;
    PolygonFileHeader *header;
    contour_list poly1, poly2, poly3, poly4, poly5;

    void DrawPolygonFilled( ocpnDC &pnt, contour_list * poly, double dx, Projection *proj,
            wxColor color );
    void DrawPolygonContour( ocpnDC &pnt, contour_list * poly, double dx, Projection *proj );

    void ReadPolygonFile( FILE *polyfile, int x, int y, int pas_x, int pas_y, contour_list *p1,
            contour_list *p2, contour_list *p3, contour_list *p4, contour_list *p5 );

};

class GshhsPolyReader {
public:
    GshhsPolyReader( int quality );
    ~GshhsPolyReader();

    void drawGshhsPolyMapPlain( ocpnDC &pnt, Projection *proj, wxColor seaColor,
            wxColor landColor );

    void drawGshhsPolyMapSeaBorders( ocpnDC &pnt, Projection *proj );

    void InitializeLoadQuality( int quality ); // 5 levels: 0=low ... 4=full
    bool crossing( QLineF traject, QLineF trajectWorld ) const;
    bool crossing1( QLineF trajectWorld );
    int currentQuality;
    void setProj( Projection * p )
    {
        this->proj = p;
    }
    int ReadPolyVersion();
    int GetPolyVersion() { return polyHeader.version; }

private:
    FILE *fpoly;
    GshhsPolyCell * allCells[360][180];

    PolygonFileHeader polyHeader;

	bool my_intersects( QLineF line1, QLineF line2 ) const;
    void readPolygonFileHeader( FILE *polyfile, PolygonFileHeader *header );
    bool abortRequested;
    Projection * proj;
};

inline bool GshhsPolyReader::crossing( QLineF traject, QLineF trajectWorld ) const
{
    if( !proj || proj == NULL ) return false;
    if( !proj->isInBounderies( traject.p1().x, traject.p1().y )
            && !proj->isInBounderies( traject.p2().x, traject.p2().y ) ) return false;
    //wxRealPoint dummy;
    int cxmin, cxmax, cymax, cymin;
    cxmin = (int) floor( wxMin( trajectWorld.p1().x, trajectWorld.p2().x ) );
    cxmax = (int) ceil( wxMax( trajectWorld.p1().x, trajectWorld.p2().x ) );
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

        for( cy = cymin; cy < cymax; cy++ ) {
            if( this->abortRequested ) return false;
            if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
                if( this->abortRequested ) return false;
                if( allCells[cxx][cy + 90] == NULL ) continue;
                cel = allCells[cxx][cy + 90];
                std::vector < QLineF > *coasts = cel->getCoasts();
                if( coasts->empty() ) continue;
                for( unsigned int cs = 0; cs < coasts->size(); cs++ ) {
                    if( this->abortRequested ) {
                        return false;
                    }
                    if( my_intersects( traject, coasts->at( cs ) ) )
                    return true;
                }
            }
        }
    }
    return false;
}

inline bool GshhsPolyReader::my_intersects( QLineF line1, QLineF line2 ) const
{
    // implementation is based on Graphics Gems III's "Faster Line Segment Intersection"
    wxRealPoint a = line1.p2() - line1.p1();
    wxRealPoint b = line2.p1() - line2.p2();
    wxRealPoint c = line1.p1() - line2.p1();

    const double denominator = a.y * b.x - a.x * b.y;
    if( denominator == 0 ) return false;

    const double reciprocal = 1 / denominator;
    const double na = ( b.y * c.x - b.x * c.y ) * reciprocal;

    if( na < INTER_MIN_LIMIT || na > INTER_MAX_LIMIT ) return false;

    const double nb = ( a.x * c.y - a.y * c.x ) * reciprocal;
    if( nb < INTER_MIN_LIMIT || nb > INTER_MAX_LIMIT ) return false;

    return true;
}

// GSHHS file format:
//
// int id;           /* Unique polygon id number, starting at 0 */
// int n;            /* Number of points in this polygon */
// int flag;             /* level + version << 8 + greenwich << 16 + source << 24 */
// int west, east, south, north; /* min/max extent in micro-degrees */
// int area;             /* Area of polygon in 1/10 km^2 */
//
// Here, level, version, greenwhich, and source are
// level:   1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake
// version: Set to 4 for GSHHS version 1.4
// greenwich:   1 if Greenwich is crossed
// source:  0 = CIA WDBII, 1 = WVS

//==========================================================
class GshhsPoint {
public:
    double lon, lat;    // longitude, latitude
    GshhsPoint( double lo, double la )
    {
        lon = lo;
        lat = la;
    }
};

//==========================================================

class GshhsPolygon {
public:
    GshhsPolygon( FILE *file );

    virtual ~GshhsPolygon();

    int getLevel() { return flag & 255; }
    int isGreenwich() { return greenwich; }
    int isAntarctic() { return antarctic; }
    bool isOk() { return ok; }
    int readInt4();
    int readInt2();

    int id; /* Unique polygon id number, starting at 0 */
    int n; /* Number of points in this polygon */
    int flag; /* level + version << 8 + greenwich << 16 + source << 24 */
    double west, east, south, north; /* min/max extent in DEGREES */
    int area; /* Area of polygon in 1/10 km^2 */
    int areaFull, container, ancestor;

    std::vector<GshhsPoint *> lsPoints;

protected:
    FILE *file;
    bool ok;
    bool greenwich, antarctic;
};

//==========================================================

class DECL_EXP GshhsReader {
public:
    GshhsReader( Projection* proj );
    ~GshhsReader();

    void drawBackground( ocpnDC &pnt, Projection *proj, wxColor seaColor, wxColor backgroundColor );
    void drawContinents( ocpnDC &pnt, Projection *proj, wxColor seaColor, wxColor landColor );

    void drawSeaBorders( ocpnDC &pnt, Projection *proj );
    void drawBoundaries( ocpnDC &pnt, Projection *proj );
    void drawRivers( ocpnDC &pnt, Projection *proj );

    int GetPolyVersion() { return gshhsPoly_reader->GetPolyVersion(); }

    static wxString getNameExtension( int quality );
    static wxString getFileName_boundaries( int quality );
    static wxString getFileName_rivers( int quality );
    static wxString getFileName_Land( int quality );
    static bool gshhsFilesExists( int quality );

    int getQuality() { return quality; }

    bool crossing( QLineF traject, QLineF trajectWorld ) const;
    bool crossing1( QLineF trajectWorld );
    void setProj( Projection * p ) { this->gshhsPoly_reader->setProj( p ); }
    int ReadPolyVersion();
    bool qualityAvailable[6];

private:
    int quality;  // 5 levels: 0=low ... 4=full
    void LoadQuality( int quality );
    int selectBestQuality( Projection *proj );

    std::string fpath;     // directory containing gshhs files

    GshhsPolyReader * gshhsPoly_reader;

    std::vector<GshhsPolygon*> * lsPoly_boundaries[5];
    std::vector<GshhsPolygon*> * lsPoly_rivers[5];

    std::vector<GshhsPolygon*> & getList_boundaries();
    std::vector<GshhsPolygon*> & getList_rivers();
    //-----------------------------------------------------

    int GSHHS_scaledPoints( GshhsPolygon *pol, wxPoint *pts, double decx, Projection *proj );

    void GsshDrawLines( ocpnDC &pnt, std::vector<GshhsPolygon*> &lst, Projection *proj,
            bool isClosed );
    void clearLists();
};

inline bool GshhsReader::crossing( QLineF traject, QLineF trajectWorld ) const
{
    return this->gshhsPoly_reader->crossing( traject, trajectWorld );
}

inline bool GshhsReader::crossing1(QLineF trajectWorld )
{
    return this->gshhsPoly_reader->crossing1(trajectWorld );
}

//-------------------------------------------------------------------------------

class GSHHSChart {
public:
    GSHHSChart();
    ~GSHHSChart();
    void SetColorScheme( ColorScheme scheme );
    void RenderViewOnDC( ocpnDC& dc, ViewPort& VPoint );

private:
    Projection* proj;
    GshhsReader* reader;
    wxColor land;
    wxColor water;
};

bool gshhsCrossesLand(double lat1, double lon1, double lat2, double lon2);

#endif
