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

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#include "gshhs.h"
#include "chartbase.h" // for projections
#include "wx28compat.h"

#include "dychart.h"


#ifdef __WXMSW__
#define __CALL_CONVENTION  //__stdcall
#else
#define __CALL_CONVENTION
#endif

//typedef void (APIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);

extern wxString *pWorldMapLocation;

//-------------------------------------------------------------------------

GSHHSChart::GSHHSChart() {
    reader = NULL;
}

GSHHSChart::~GSHHSChart() {
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


void GSHHSChart::RenderViewOnDC( ocpnDC& dc, ViewPort& vp )
{
    if( ! reader ) {
        reader = new GshhsReader( );
        if( reader->GetPolyVersion() < 210 || reader->GetPolyVersion() > 220 ) {
            wxLogMessage( _T("GSHHS World chart files have wrong version. Found %ld, expected 210-220."),
                    reader->GetPolyVersion() );
        } else {
            wxLogMessage(
                    _T("Background world map loaded from GSHHS datafiles found in: ") +
                    *pWorldMapLocation );
        }
    }

    reader->drawContinents( dc, vp, water, land );

    /* this is very inefficient since it draws the entire world*/
//    reader->drawBoundaries( dc, vp );
}

GshhsPolyCell::GshhsPolyCell( FILE *fpoly_, int x0_, int y0_, PolygonFileHeader *header_ )
{
    header = header_;
    fpoly = fpoly_;
    x0cell = x0_;
    y0cell = y0_;

    for(int i=0; i<6; i++)
        polyv[i] = NULL;

    ReadPolygonFile( );

    for(int i=0; i<GSSH_SUBM*GSSH_SUBM; i++)
        high_res_map[i] = NULL;
}

GshhsPolyCell::~GshhsPolyCell()
{
    ClearPolyV();

    for(int i=0; i<GSSH_SUBM*GSSH_SUBM; i++)
        delete high_res_map[i];
    for(int i=0; i<6; i++)
        delete [] polyv[i];
}

void GshhsPolyCell::ClearPolyV()
{
    for(int i=0; i<6; i++) {
        delete [] polyv[i];
        polyv[i] = NULL;
    }
}

void GshhsPolyCell::ReadPoly(contour_list &poly)
{
    double X,Y;
    contour tmp_contour;
    int32_t num_vertices, num_contours;
    poly.clear();
    if(fread(&num_contours, sizeof num_contours, 1, fpoly) != 1)
        goto fail;

    for (int c= 0; c < num_contours; c++)
    {
        int32_t value;
        if(fread(&value, sizeof value, 1, fpoly) != 1 ||  /* discarding hole value */
           fread(&value, sizeof value, 1, fpoly) != 1)
            goto fail;

        num_vertices=value;

        tmp_contour.clear();
        for (int v= 0; v < num_vertices; v++)
        {
            if(fread(&X, sizeof X, 1, fpoly) != 1 ||
               fread(&Y, sizeof Y, 1, fpoly) != 1)
                goto fail;

            tmp_contour.push_back(wxRealPoint(X*GSHHS_SCL,Y*GSHHS_SCL));
        }
        poly.push_back(tmp_contour);
    }
    return;

fail:
    wxLogMessage( _T("gshhs ReadPoly failed") );
}

void GshhsPolyCell::ReadPolygonFile()
{
    if(!fpoly)
        return;

    int pos_data;
    int tab_data;

    tab_data = ( x0cell / header->pasx ) * ( 180 / header->pasy )
        + ( y0cell + 90 ) / header->pasy;
    fseek( fpoly, sizeof(PolygonFileHeader) + tab_data * sizeof(int), SEEK_SET );
    if(fread( &pos_data, sizeof(int), 1, fpoly ) != 1)
        goto fail;

    fseek( fpoly, pos_data, SEEK_SET );

    ReadPoly( poly1 );
    ReadPoly( poly2 );
    ReadPoly( poly3 );
    ReadPoly( poly4 );
    ReadPoly( poly5 );
    return;

fail:
    wxLogMessage( _T("gshhs ReadPolygon failed") );
}

wxPoint2DDouble GetDoublePixFromLL(ViewPort &vp, double lat, double lon)
{
    wxPoint2DDouble p = vp.GetDoublePixFromLL(lat, lon);
    p.m_x -= vp.rv_rect.x, p.m_y -= vp.rv_rect.y;
    return p;
}

void GshhsPolyCell::DrawPolygonFilled( ocpnDC &pnt, contour_list * p, double dx, ViewPort &vp,  wxColor const &color )
{
    if( !p->size() ) /* size of 0 is very common, and setting the brush is
                        actually quite slow, so exit early */
        return;

    int x, y;
    unsigned int c, v;
    int pointCount;

    int x_old = 0;
    int y_old = 0;

    pnt.SetBrush( color );

    for( c = 0; c < p->size(); c++ ) {
        if( !p->at( c ).size() ) continue;

        wxPoint* poly_pt = new wxPoint[ p->at(c).size() ];

        contour &cp = p->at( c );
        pointCount = 0;

        for( v = 0; v < p->at( c ).size(); v++ ) {
            wxRealPoint &ccp = cp.at( v );
            wxPoint2DDouble q = GetDoublePixFromLL(vp, ccp.y, ccp.x + dx );
            if(wxIsNaN(q.m_x)) {
                pointCount = 0;
                break;
            }

            x = q.m_x, y = q.m_y;

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

#include <list>

static std::list<float_2Dpt> g_pv;
static std::list<GLvertex*> g_vertexes;
static int g_type, g_pos;
static float_2Dpt g_p1, g_p2;

void __CALL_CONVENTION gshhscombineCallback( GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4],
        GLdouble **dataOut )
{
    GLvertex *vertex;

    vertex = new GLvertex();
    g_vertexes.push_back(vertex);

    vertex->info.x = coords[0];
    vertex->info.y = coords[1];

    *dataOut = vertex->data;
}

void __CALL_CONVENTION gshhsvertexCallback( GLvoid* arg )
{
    GLvertex* vertex;
    vertex = (GLvertex*) arg;
    float_2Dpt p;
    p.y = vertex->info.x;
    p.x = vertex->info.y;

    // convert strips and fans into triangles
    if(g_type != GL_TRIANGLES) {
        if(g_pos > 2) {
            g_pv.push_back(g_p1);
            g_pv.push_back(g_p2);
        }

        if(g_type == GL_TRIANGLE_STRIP)
            g_p1 = g_p2;
        else if(g_pos == 0)
            g_p1 = p;
        g_p2 = p;
    }

    g_pv.push_back(p);
    g_pos++;
}

void __CALL_CONVENTION gshhserrorCallback( GLenum errorCode )
{
   const GLubyte *estring;
   estring = gluErrorString(errorCode);
   wxLogMessage( _T("OpenGL Tessellation Error: %s"), estring );
}

void __CALL_CONVENTION gshhsbeginCallback( GLenum type )
{
    switch(type) {
    case GL_TRIANGLES:
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
        g_type = type;
        break;
    default:
    printf("tess unhandled begin type: %d\n", type);
    }

    g_pos = 0;
}

void __CALL_CONVENTION gshhsendCallback()
{
}

void GshhsPolyCell::DrawPolygonFilledGL( contour_list * p, float_2Dpt **pv, int *pvc, ViewPort &vp,  wxColor const &color, bool idl )
{
    if( !p->size() ) // size of 0 is very common, exit early
        return;

    // build the contour vertex array converted to normalized coordinates (if needed)
    if(!*pv) {
        for(unsigned int c = 0; c < p->size(); c++ ) {
            if( !p->at( c ).size() ) continue;

            contour &cp = p->at( c );

            GLUtesselator *tobj = gluNewTess();
            
            gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &gshhsvertexCallback );
            gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &gshhsbeginCallback );
            gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &gshhsendCallback );
            gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &gshhscombineCallback );
            gluTessCallback( tobj, GLU_TESS_ERROR, (_GLUfuncptr) &gshhserrorCallback );
            
            gluTessNormal( tobj, 0, 0, 1);
            gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO );
            
            gluTessBeginPolygon( tobj, NULL );
            gluTessBeginContour( tobj );

            for(unsigned int v = 0; v < p->at( c ).size(); v++ ) {
                wxRealPoint &ccp = cp.at( v );

                if( v == 0 || ccp != cp.at(v-1) ) {
                    GLvertex* vertex = new GLvertex();
                    g_vertexes.push_back(vertex);

                    wxPoint2DDouble q;
                    if(glChartCanvas::HasNormalizedViewPort(vp))
                        q = GetDoublePixFromLL(vp, ccp.y, ccp.x );
                    else // tesselation directly from lat/lon
                        q.m_x = ccp.y, q.m_y = ccp.x;

                    if(vp.m_projection_type != PROJECTION_POLAR) {
                        // need to correctly pick +180 or -180 longitude for projections
                        // that have a discontiguous date line
                            
                        if(idl && ccp.x == 180) {
                            if(vp.m_projection_type == PROJECTION_MERCATOR ||
                               vp.m_projection_type == PROJECTION_EQUIRECTANGULAR)
                                q.m_x -= 40058986*4096.0; // 360 degrees in normalized viewport
                            else
                                q.m_x -= 360; // lat/lon coordinates
                        }
                    }

                    vertex->info.x = q.m_x;
                    vertex->info.y = q.m_y;

                    gluTessVertex( tobj, (GLdouble*)vertex, (GLdouble*)vertex);
                }
            }

            gluTessEndContour( tobj );
            gluTessEndPolygon( tobj );
            gluDeleteTess( tobj );

            for(std::list<GLvertex*>::iterator it = g_vertexes.begin(); it != g_vertexes.end(); it++)
                delete *it;
            g_vertexes.clear();
        }

        *pv = new float_2Dpt[g_pv.size()];
        int i=0;
        for(std::list<float_2Dpt>::iterator it = g_pv.begin(); it != g_pv.end(); it++)
            (*pv)[i++] = *it;
            
        *pvc = g_pv.size();
        g_pv.clear();
    }

    glColor3ub(color.Red(), color.Green(), color.Blue());

    if(glChartCanvas::HasNormalizedViewPort(vp)) {
        glVertexPointer(2, GL_FLOAT, 2*sizeof(float), *pv);
        glDrawArrays(GL_TRIANGLES, 0, *pvc);
    } else {
        float_2Dpt *pvt = new float_2Dpt[*pvc];
        for(int i=0; i<*pvc; i++) {
            float_2Dpt *pc = *pv + i;
            wxPoint2DDouble q = vp.GetDoublePixFromLL(pc->y, pc->x);
            pvt[i].x = q.m_y;
            pvt[i].y = q.m_x;
        }

        glVertexPointer(2, GL_FLOAT, 2*sizeof(float), pvt);
        glDrawArrays(GL_TRIANGLES, 0, *pvc);

        delete [] pvt;
    }
}
#endif          //#ifdef ocpnUSE_GL

#define DRAW_POLY_FILLED(POLY,COL) if(POLY) DrawPolygonFilled(pnt,POLY,dx,vp,COL);
#define DRAW_POLY_FILLED_GL(NUM,COL) DrawPolygonFilledGL(&poly##NUM,&polyv[NUM],&polyc[NUM],vp,COL, idl);

void GshhsPolyCell::drawMapPlain( ocpnDC &pnt, double dx, ViewPort &vp, wxColor seaColor,
                                  wxColor landColor, bool idl )
{
#ifdef ocpnUSE_GL        
    if(!pnt.GetDC()) { // opengl
#define NORM_FACTOR 4096.0
        if(dx && (vp.m_projection_type == PROJECTION_MERCATOR ||
                  vp.m_projection_type == PROJECTION_EQUIRECTANGULAR)) {
            double ts = 40058986*NORM_FACTOR; /* 360 degrees in normalized viewport */
            glPushMatrix();
            glTranslated(dx > 0 ? ts : -ts, 0, 0);
        }

        DRAW_POLY_FILLED_GL( 1, landColor );
        DRAW_POLY_FILLED_GL( 2, seaColor );
        DRAW_POLY_FILLED_GL( 3, landColor );
        DRAW_POLY_FILLED_GL( 4, seaColor );
        DRAW_POLY_FILLED_GL( 5, landColor );

        if(dx)
            glPopMatrix();
    } else
#endif
    {
        DRAW_POLY_FILLED( &poly1, landColor );
        DRAW_POLY_FILLED( &poly2, seaColor );
        DRAW_POLY_FILLED( &poly3, landColor );
        DRAW_POLY_FILLED( &poly4, seaColor );
        DRAW_POLY_FILLED( &poly5, landColor );
    }
}

void GshhsPolyCell::DrawPolygonContour( ocpnDC &pnt, contour_list * p, double dx, ViewPort &vp )
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
                wxPoint2DDouble AB = GetDoublePixFromLL(vp,  x1 + dx, y1);
                wxPoint2DDouble CD = GetDoublePixFromLL(vp,  x2 + dx, y1);
                pnt.DrawLine(AB.m_x, AB.m_y, CD.m_x, CD.m_y);
            }
        }

        x1 = p->at( i ).at( v ).x;
        y1 = p->at( i ).at( v ).y;
        x2 = p->at( i ).at( 0 ).x;
        y2 = p->at( i ).at( 0 ).y;

        if( ( ( ( x1 == x2 ) && ( ( x1 == long_min ) || ( x1 == long_max ) ) )
                || ( ( y1 == y2 ) && ( ( y1 == lat_min ) || ( y1 == lat_max ) ) ) ) == 0 ) {
                wxPoint2DDouble AB = GetDoublePixFromLL(vp,  x1 + dx, y1);
                wxPoint2DDouble CD = GetDoublePixFromLL(vp,  x2 + dx, y1);
                pnt.DrawLine(AB.m_x, AB.m_y, CD.m_x, CD.m_y);
        }
    }
}

#define DRAW_POLY_CONTOUR(POLY) if(POLY) DrawPolygonContour(pnt,POLY,dx,vp);

void GshhsPolyCell::drawSeaBorderLines( ocpnDC &pnt, double dx, ViewPort &vp )
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

inline bool my_intersects( const wxLineF &line1, const wxLineF &line2 )
{
    double x1 = line1.m_p1.x, y1 = line1.m_p1.y, x2 = line1.m_p2.x, y2 = line1.m_p2.y;
    double x3 = line2.m_p1.x, y3 = line2.m_p1.y, x4 = line2.m_p2.x, y4 = line2.m_p2.y;

    // implementation is based on Graphics Gems III's "Faster Line Segment Intersection"
    double ax = x2 - x1, ay = y2 - y1;
    double bx = x3 - x4, by = y3 - y4;
    double cx = x1 - x3, cy = y1 - y3;

    double denominator = ay * bx - ax * by;
    if( denominator < 1e-10 ) {
        if(fabs((y1*ax - ay*x1)*bx - (y3*bx - by*x3)*ax) > 1e-5)
            return false; /* different intercepts, no intersection */

        return true;
    }

#  define INTER_LIMIT 1e-7

    const double reciprocal = 1 / denominator;
    const double na = ( by * cx - bx * cy ) * reciprocal;

    if( na < -INTER_LIMIT || na > 1 + INTER_LIMIT ) return false;

    const double nb = ( ax * cy - ay * cx ) * reciprocal;
    if( nb < -INTER_LIMIT || nb > 1 + INTER_LIMIT ) return false;

    return true;
}

bool GshhsPolyReader::crossing1( wxLineF trajectWorld )
{
    double x1 = trajectWorld.p1().x, y1 = trajectWorld.p1().y;
    double x2 = trajectWorld.p2().x, y2 = trajectWorld.p2().y;

    int cxmin, cxmax, cymax, cymin;
    cxmin = (int) floor( GSSH_SUBM*wxMin( x1, x2 ) );
    cxmax = (int) ceil( GSSH_SUBM*wxMax( x1, x2 ) );

    if(cxmin < 0) {
        cxmin += GSSH_SUBM*360;
        cxmax += GSSH_SUBM*360;
    }

    if(cxmax - cxmin > GSSH_SUBM*180) { /* dont go long way around world */
        cxmin = (int) floor( GSSH_SUBM*wxMax( x1, x2 ) ) - GSSH_SUBM*360;
        cxmax = (int) ceil( GSSH_SUBM*wxMin( x1, x2 ) );
    }

    cymin = (int) floor( GSSH_SUBM*wxMin( y1, y2 ));
    cymax = (int) ceil( GSSH_SUBM*wxMax( y1, y2 ));
    wxASSERT(cymin >= -GSSH_SUBM*90 && cymax <= GSSH_SUBM*89);

    // TODO: optimize by traversing only the cells the segment passes through,
    //       rather than all of the cells which fit in the bounding box,
    //       this may make a worthwhile difference for longer segments in some cases.
    int cx, cxx, cy;
    for( cx = cxmin; cx < cxmax; cx++ ) {
        cxx = cx;
        while( cxx < 0 )
            cxx += GSSH_SUBM*360;
        while( cxx >= GSSH_SUBM*360 )
            cxx -= GSSH_SUBM*360;

        wxASSERT( cxx >= 0 && cxx < GSSH_SUBM*360 );

        if(cxx < GSSH_SUBM*180) {
            if(x1 > 180) x1 -= 360;
            if(x2 > 180) x2 -= 360;
        } else {
            if(x1 < 180) x1 += 360;
            if(x2 < 180) x2 += 360;
        }

        wxLineF rtrajectWorld(x1, y1, x2, y2);

        for( cy = cymin; cy < cymax; cy++ ) {
            int cxi = cxx/GSSH_SUBM, cyi = (GSSH_SUBM*90+cy)/GSSH_SUBM;
            GshhsPolyCell *&cel = allCells[cxi][cyi];
            if(!cel) {
                mutex1.Lock();
                if(!cel) {
                    /* load the needed cell from disk */
                    cel = new GshhsPolyCell(fpoly, cxi, cyi-90, &polyHeader);
                    wxASSERT( cel );
                }
                mutex1.Unlock();
            }

            int hash = GSSH_SUBM*(GSSH_SUBM*(90-cyi) + cy - cxi) + cxx;
            std::vector<wxLineF> *&high_res_map = cel->high_res_map[hash];
            wxASSERT(hash >= 0 && hash < GSSH_SUBM*GSSH_SUBM);
            if(!high_res_map) {
                mutex2.Lock();
                if(!high_res_map) {
                    /* Build the needed sub cell of line segments from the cell */
                    contour_list &poly1 = cel->getPoly1();

                    double minlat = (double)cy/GSSH_SUBM, maxlat = (double)(cy+1)/GSSH_SUBM;
                    double minlon = (double)cxx/GSSH_SUBM, maxlon = (double)(cxx+1)/GSSH_SUBM;
                    high_res_map = new std::vector<wxLineF>;
                    for( unsigned int pi = 0; pi < poly1.size(); pi++ ) {
                        contour &c = poly1[pi];
                        double lx = c[c.size()-1].x, ly = c[c.size()-1].y;
                        /* must compute states because sometimes a
                           segment starts and ends outside our cell, but passes
                           through it so must be included */
                        int lstatex = lx < minlon ? -1 : lx > maxlon ? 1 : 0;
                        int lstatey = ly < minlat ? -1 : ly > maxlat ? 1 : 0;
                    
                        for( unsigned int pj = 0; pj < c.size(); pj++ ) {
                            double cx = c[pj].x, cy = c[pj].y;
                            // gshhs data shouldn't, but sometimes contains zero segments
                            // which enlarges our table, but
                            // more importantly, the fast segment intersection test
                            // and doesn't correctly account for it
                            if(lx == cx && ly == cy)
                                continue;

                            int statex = cx < minlon ? -1 : cx > maxlon ? 1 : 0;
                            int statey = cy < minlat ? -1 : cy > maxlat ? 1 : 0;

                            if((!statex || lstatex != statex) &&
                               (!statey || lstatey != statey))
                                high_res_map->push_back(wxLineF(lx, ly, cx, cy));

                            lx = cx, ly = cy;
                            lstatex = statex, lstatey = statey;
                        }
                    }
                }
                mutex2.Unlock();
            }

            for(std::vector<wxLineF>::iterator it2 = high_res_map->begin();
                it2 != high_res_map->end(); it2++)
                if( my_intersects( rtrajectWorld, *it2 ) )
                    return true;
        }
    }

    return false;
}

void GshhsPolyReader::readPolygonFileHeader( FILE *polyfile, PolygonFileHeader *header )
{
    fseek( polyfile, 0, SEEK_SET );
    if(fread( header, sizeof(PolygonFileHeader), 1, polyfile ) != 1)
        wxLogMessage( _T("gshhs ReadPolygonFileHeader failed") );
}

//-------------------------------------------------------------------------
void GshhsPolyReader::drawGshhsPolyMapPlain( ocpnDC &pnt, ViewPort &vp, wxColor const &seaColor,
                                             wxColor const &landColor )
{
    if( !fpoly ) return;

    pnt.SetPen( wxNullPen );

    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    wxBoundingBox bbox = vp.GetBBox();
    cxmin = bbox.GetMinX(), cxmax = bbox.GetMaxX(), cymin = bbox.GetMinY(), cymax = bbox.GetMaxY();
    if(cymin <= 0) cymin--;
    if(cymax >= 0) cymax++;
    if(cxmin <= 0) cxmin--;
    if(cxmax >= 0) cxmax++;
    int dx, cx, cxx, cy;
    GshhsPolyCell *cel;

    ViewPort nvp = vp;
#ifdef ocpnUSE_GL
    if(!pnt.GetDC()) { // opengl
        // clear cached data when the projection changes
        if(vp.m_projection_type != last_rendered_vp.m_projection_type ||
           (last_rendered_vp.m_projection_type == PROJECTION_POLAR &&
            last_rendered_vp.clat*vp.clat <= 0)) {
            last_rendered_vp = vp;
            for(int cx = 0; cx<360; cx++)
                for(int cy = 0; cy<180; cy++)
                    if(allCells[cx][cy])
                        allCells[cx][cy]->ClearPolyV();
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        
        // use a viewport that allows the vertexes to be reused over many frames
        if(glChartCanvas::HasNormalizedViewPort(vp)) {
            glPushMatrix();
            glChartCanvas::MultMatrixViewPort(vp);
            nvp = glChartCanvas::NormalizedViewPort(vp);
        }
    }
#endif
    for( cx = cxmin; cx < cxmax; cx++ ) {
        cxx = cx;
        while( cxx < 0 )
            cxx += 360;
        while( cxx >= 360 )
            cxx -= 360;

        for( cy = cymin; cy < cymax; cy++ ) {
            if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
                if( allCells[cxx][cy + 90] == NULL ) {
                    cel = new GshhsPolyCell( fpoly, cxx, cy, &polyHeader );
                    wxASSERT( cel );
                    allCells[cxx][cy + 90] = cel;
                } else {
                    cel = allCells[cxx][cy + 90];
                }
                bool idl = false;

                // only mercator needs the special idl fixes
                if(vp.m_projection_type != PROJECTION_MERCATOR &&
                   vp.m_projection_type != PROJECTION_EQUIRECTANGULAR)
                    dx = 0;
                else if(pnt.GetDC())// dc
                    dx = cx - cxx;
                else { // opengl
                    int cxn = cxx;
                    if(cxn >= 180) {
                        cxn -= 360;
                        idl = true;
                    }
                    if(vp.clon - cxn > 180)
                        dx = 1;
                    else if(vp.clon - cxn < -180)
                        dx = -1;
                    else
                        dx = 0;
                }

                cel->drawMapPlain( pnt, dx, nvp, seaColor, landColor, idl );
            }
        }
    }

#ifdef ocpnUSE_GL
    if(!pnt.GetDC()) { // opengl
        if(glChartCanvas::HasNormalizedViewPort(vp))
            glPopMatrix();
        glDisableClientState(GL_VERTEX_ARRAY);
    }
#endif
}

//-------------------------------------------------------------------------
void GshhsPolyReader::drawGshhsPolyMapSeaBorders( ocpnDC &pnt, ViewPort &vp )
{
    if( !fpoly ) return;
    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    wxBoundingBox bbox = vp.GetBBox();
    cxmin = bbox.GetMinX(), cxmax = bbox.GetMaxX(), cymin = bbox.GetMinY(), cymax = bbox.GetMaxY();

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
                    cel = new GshhsPolyCell( fpoly, cxx, cy, &polyHeader );
                    wxASSERT( cel );
                    allCells[cxx][cy + 90] = cel;
                } else {
                    cel = allCells[cxx][cy + 90];
                }
                dx = cx - cxx;
                cel->drawSeaBorderLines( pnt, dx, vp );
            }
        }
    }
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

GshhsReader::GshhsReader( )
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

//    int q = selectBestQuality( vp );
//    if( ! qualityAvailable[q] ) {
//    int q = maxQualityAvailable;
//    }

    int q = 0;

    gshhsPoly_reader = new GshhsPolyReader( q );

    for( int qual = 0; qual < 5; qual++ ) {
        lsPoly_boundaries[qual] = new std::vector<GshhsPolygon*>;
        lsPoly_rivers[qual] = new std::vector<GshhsPolygon*>;
    }

    quality = -1;
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

    quality = newQuality;
    if( quality < 0 ) quality = 0;
    else if( quality > 4 ) quality = 4;

    gshhsPoly_reader->InitializeLoadQuality( quality );
#if 0 /* too slow to load the whole world at once */
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
#endif
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

int GshhsReader::GSHHS_scaledPoints( GshhsPolygon *pol, wxPoint *pts, double decx, ViewPort &vp )
{
    wxBoundingBox box(pol->west + decx, pol->south, pol->east + decx, pol->north);
    if(vp.GetBBox().IntersectOut(box) )
        return 0;

    // Remove small polygons.

    wxPoint2DDouble p1 = GetDoublePixFromLL(vp,  pol->west + decx, pol->north );
    wxPoint2DDouble p2 = GetDoublePixFromLL(vp,  pol->east + decx, pol->south );

    if( p1.m_x == p2.m_x && p1.m_y == p2.m_y )
        return 0;

    double x, y;
    std::vector<GshhsPoint *>::iterator itp;
    int xx, yy, oxx = 0, oyy = 0;
    int j = 0;

    for( itp = ( pol->lsPoints ).begin(); itp != ( pol->lsPoints ).end(); itp++ ) {
        x = ( *itp )->lon + decx;
        y = ( *itp )->lat;
                                    {
        wxPoint2DDouble p = GetDoublePixFromLL(vp,  y, x );
        xx = p.m_x, yy = p.m_y;
        if( j == 0 || ( oxx != xx || oyy != yy ) )  // Remove close points
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
void GshhsReader::GsshDrawLines( ocpnDC &pnt, std::vector<GshhsPolygon*> &lst, ViewPort &vp,
        bool isClosed )
{
    std::vector<GshhsPolygon*>::iterator iter;
    GshhsPolygon *pol;
    wxPoint *pts = NULL;
    int i;
    int nbp;

    int nbmax = 10000;
    pts = new wxPoint[nbmax];
    wxASSERT( pts );

    for( i = 0, iter = lst.begin(); iter != lst.end(); iter++, i++ ) {
        pol = *iter;

        if( nbmax < pol->n + 2 ) {
            nbmax = pol->n + 2;
            delete[] pts;
            pts = new wxPoint[nbmax];
            wxASSERT( pts );
        }

        nbp = GSHHS_scaledPoints( pol, pts, 0, vp );
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

        nbp = GSHHS_scaledPoints( pol, pts, -360, vp );
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
void GshhsReader::drawContinents( ocpnDC &pnt, ViewPort &vp, wxColor const &seaColor,
        wxColor const &landColor )
{
    LoadQuality( selectBestQuality( vp ) );
    gshhsPoly_reader->drawGshhsPolyMapPlain( pnt, vp, seaColor, landColor );
}

//-----------------------------------------------------------------------
void GshhsReader::drawSeaBorders( ocpnDC &pnt, ViewPort &vp )
{
    pnt.SetBrush( *wxTRANSPARENT_BRUSH );
    gshhsPoly_reader->drawGshhsPolyMapSeaBorders( pnt, vp );
}

//-----------------------------------------------------------------------
void GshhsReader::drawBoundaries( ocpnDC &pnt, ViewPort &vp )
{
    pnt.SetBrush( *wxTRANSPARENT_BRUSH );

    if( pnt.GetDC() ) {
        wxPen* pen = wxThePenList->FindOrCreatePen( *wxBLACK, 1, wxPENSTYLE_DOT );
        pnt.SetPen( *pen );
    } else {
        wxPen* pen = wxThePenList->FindOrCreatePen( wxColor( 0, 0, 0, 80 ), 2, wxPENSTYLE_LONG_DASH );
        pnt.SetPen( *pen );
    }
    GsshDrawLines( pnt, getList_boundaries(), vp, false );
}

//-----------------------------------------------------------------------
void GshhsReader::drawRivers( ocpnDC &pnt, ViewPort &vp )
{
    GsshDrawLines( pnt, getList_rivers(), vp, false );
}

//-----------------------------------------------------------------------
int GshhsReader::selectBestQuality( ViewPort &vp )
{
    int bestQuality = 0;

         if(vp.chart_scale <   500000) bestQuality = 4;
    else if(vp.chart_scale <  2000000) bestQuality = 3;
    else if(vp.chart_scale <  8000000) bestQuality = 2;
    else if(vp.chart_scale < 20000000) bestQuality = 1;
    else bestQuality = 0;

    while( !qualityAvailable[bestQuality] ) {
        bestQuality--;
        if( bestQuality < 0 ) break;
    }

    if( bestQuality < 0 )
        for( int i=0; i<5; i++ )
            if( qualityAvailable[i] ) bestQuality = i;

    return bestQuality;
}

/* so plugins can determine if a line segment crosses land, must call from main
   thread once at startup to initialize array */
static GshhsReader *reader = NULL;
void gshhsCrossesLandInit()
{
    reader = new GshhsReader();

    /* load best possible quality for crossing tests */
    int bestQuality = 4;
    while( !reader->qualityAvailable[bestQuality] && bestQuality > 0)
        bestQuality--;
    reader->LoadQuality(bestQuality);
}

bool gshhsCrossesLand(double lat1, double lon1, double lat2, double lon2)
{
    if(lon1 < 0)
        lon1 += 360;
    if(lon2 < 0)
        lon2 += 360;

    wxLineF trajectWorld(lon1, lat1, lon2, lat2);
    return reader->crossing1(trajectWorld);
}
