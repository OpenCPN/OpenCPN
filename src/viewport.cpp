/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  ViewPort
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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
 **************************************************************************/

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include "wx/image.h"
#include <wx/graphics.h>
#include <wx/listbook.h>
#include <wx/clipbrd.h>
#include <wx/aui/aui.h>

#include "dychart.h"

#include <wx/listimpl.cpp>

#include "chcanv.h"
#include "TCWin.h"
#include "geodesic.h"
#include "styles.h"
#include "routeman.h"
#include "navutil.h"
#include "kml.h"
#include "concanv.h"
#include "thumbwin.h"
#include "chartdb.h"
#include "chartimg.h"
#include "chart1.h"
#include "cutil.h"
#include "routeprop.h"
#include "TrackPropDlg.h"
#include "tcmgr.h"
#include "routemanagerdialog.h"
#include "pluginmanager.h"
#include "ocpn_pixel.h"
#include "ocpndc.h"
#include "undo.h"
#include "multiplexer.h"
#include "timers.h"
#include "tide_time.h"
#include "glTextureDescriptor.h"
#include "ChInfoWin.h"
#include "Quilt.h"
#include "SelectItem.h"
#include "Select.h"
#include "FontMgr.h"
#include "AIS_Decoder.h"
#include "AIS_Target_Data.h"
#include "AISTargetAlertDialog.h"
#include "SendToGpsDlg.h"
#include "OCPNRegion.h"
#include "gshhs.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#ifdef USE_S57
#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#endif

#include "ais.h"

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>


extern ChartCanvas *cc1;
extern struct sigaction sa_all;
extern struct sigaction sa_all_old;

extern sigjmp_buf           env;                    // the context saved by sigsetjmp();
#endif

#include <vector>


// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------

extern void catch_signals(int signo);

//------------------------------------------------------------------------------
//    ViewPort Implementation
//------------------------------------------------------------------------------
ViewPort::ViewPort()
{
    bValid = false;
    skew = 0.;
    view_scale_ppm = 1;
    rotation = 0.;
    tilt = 0.;
    b_quilt = false;
    pix_height = pix_width = 0;
    b_MercatorProjectionOverride = false;
    lat0_cache = NAN;
    m_projection_type = PROJECTION_MERCATOR;

}

// TODO: eliminate the use of this function
wxPoint ViewPort::GetPixFromLL( double lat, double lon )
{
    wxPoint2DDouble p = GetDoublePixFromLL(lat, lon);
    if(wxIsNaN(p.m_x) || wxIsNaN(p.m_y))
        return wxPoint(INVALID_COORD, INVALID_COORD);
    return wxPoint(wxRound(p.m_x), wxRound(p.m_y));
}

wxPoint2DDouble ViewPort::GetDoublePixFromLL( double lat, double lon )
{
    double easting = 0;
    double northing = 0;
    double xlon = lon;

    /*  Make sure lon and lon0 are same phase */
    if( xlon * clon < 0. ) {
        if( xlon < 0. ) xlon += 360.;
        else
            xlon -= 360.;
    }

    if( fabs( xlon - clon ) > 180. ) {
        if( xlon > clon ) xlon -= 360.;
        else
            xlon += 360.;
    }

    // update cache of trig functions used for projections
    if(clat != lat0_cache) {
        lat0_cache = clat;
        switch( m_projection_type ) {
        case PROJECTION_MERCATOR:
            cache0 = toSMcache_y30(clat);
            break;
        case PROJECTION_POLAR:
            cache0 = toPOLARcache_e(clat);
            break;
        case PROJECTION_ORTHOGRAPHIC:
        case PROJECTION_STEREOGRAPHIC:
        case PROJECTION_GNOMONIC:
            cache_phi0(clat, &cache0, &cache1);
            break;
        }
    }

    switch( m_projection_type ) {
    case PROJECTION_MERCATOR:
#if 0
        toSM( lat, xlon, clat, clon, &easting, &northing );
#else
        toSMcache( lat, xlon, cache0, clon, &easting, &northing );
#endif
        break;

    case PROJECTION_TRANSVERSE_MERCATOR:
        //    We calculate northings as referenced to the equator
        //    And eastings as though the projection point is midscreen.

        double tmeasting, tmnorthing;
        double tmceasting, tmcnorthing;
        toTM( clat, clon, 0., clon, &tmceasting, &tmcnorthing );
        toTM( lat, xlon, 0., clon, &tmeasting, &tmnorthing );

        northing = tmnorthing - tmcnorthing;
        easting = tmeasting - tmceasting;
        break;

    case PROJECTION_POLYCONIC:

        //    We calculate northings as referenced to the equator
        //    And eastings as though the projection point is midscreen.
        double pceasting, pcnorthing;
        toPOLY( clat, clon, 0., clon, &pceasting, &pcnorthing );

        double peasting, pnorthing;
        toPOLY( lat, xlon, 0., clon, &peasting, &pnorthing );

        easting = peasting;
        northing = pnorthing - pcnorthing;
        break;

    case PROJECTION_ORTHOGRAPHIC:
        toORTHO( lat, xlon, cache0, cache1, clon, &easting, &northing );
        break;

    case PROJECTION_POLAR:
        toPOLAR( lat, xlon, cache0, clat, clon, &easting, &northing );
        break;

    case PROJECTION_STEREOGRAPHIC:
        toSTEREO( lat, xlon, cache0, cache1, clon, &easting, &northing );
        break;

    case PROJECTION_GNOMONIC:
        toGNO( lat, xlon, cache0, cache1, clon, &easting, &northing );
        break;

    case PROJECTION_EQUIRECTANGULAR:
        toEQUIRECT( lat, xlon, clat, clon, &easting, &northing );
        break;

    default:
        printf("unhandled projection\n");
    }

    if( !wxFinite(easting) || !wxFinite(northing) )
        return wxPoint2DDouble( easting, northing );

    double epix = easting * view_scale_ppm;
    double npix = northing * view_scale_ppm;
    double dxr = epix;
    double dyr = npix;

    //    Apply VP Rotation
    double angle = rotation;

    if( angle ) {
        dxr = epix * cos( angle ) + npix * sin( angle );
        dyr = npix * cos( angle ) - epix * sin( angle );
    }

    return wxPoint2DDouble(( pix_width / 2.0 ) + dxr, ( pix_height / 2.0 ) - dyr);
}

void ViewPort::GetLLFromPix( const wxPoint2DDouble &p, double *lat, double *lon )
{
    double dx = p.m_x - ( pix_width / 2.0 );
    double dy = ( pix_height / 2.0 ) - p.m_y;

    double xpr = dx;
    double ypr = dy;

    //    Apply VP Rotation
    double angle = rotation;

    if( angle ) {
        xpr = ( dx * cos( angle ) ) - ( dy * sin( angle ) );
        ypr = ( dy * cos( angle ) ) + ( dx * sin( angle ) );
    }
    double d_east = xpr / view_scale_ppm;
    double d_north = ypr / view_scale_ppm;

    double slat = 0.0, slon = 0.0;
    switch( m_projection_type ) {
    case PROJECTION_MERCATOR:
        //TODO  This could be fromSM_ECC to better match some Raster charts
        //      However, it seems that cm93 (and S57) prefer no eccentricity correction
        //      Think about it....
        fromSM( d_east, d_north, clat, clon, &slat, &slon );
        break;
        
    case PROJECTION_TRANSVERSE_MERCATOR:
    {
        double tmceasting, tmcnorthing;
        toTM( clat, clon, 0., clon, &tmceasting, &tmcnorthing );

        fromTM( d_east, d_north + tmcnorthing, 0., clon, &slat, &slon );
    } break;

    case PROJECTION_POLYCONIC:
    {
        double polyeasting, polynorthing;
        toPOLY( clat, clon, 0., clon, &polyeasting, &polynorthing );

        fromPOLY( d_east, d_north + polynorthing, 0., clon, &slat, &slon );
    } break;

    case PROJECTION_ORTHOGRAPHIC:
        fromORTHO( d_east, d_north, clat, clon, &slat, &slon );
        break;

    case PROJECTION_POLAR:
        fromPOLAR( d_east, d_north, clat, clon, &slat, &slon );
        break;

    case PROJECTION_STEREOGRAPHIC:
        fromSTEREO( d_east, d_north, clat, clon, &slat, &slon );
        break;

    case PROJECTION_GNOMONIC:
        fromGNO( d_east, d_north, clat, clon, &slat, &slon );
        break;

    case PROJECTION_EQUIRECTANGULAR:
        fromEQUIRECT( d_east, d_north, clat, clon, &slat, &slon );
        break;

    default:
        printf("unhandled projection\n");
    }

    *lat = slat;

    if( slon < -180. ) slon += 360.;
    else if( slon > 180. ) slon -= 360.;
    *lon = slon;
}

LLRegion ViewPort::GetLLRegion( const OCPNRegion &region )
{
    // todo: for these projecetions, improve this calculation by using the
    //       method in SetBoxes here
#ifndef ocpnUSE_GL
    return LLRegion(GetBBox());
#else    

    if(!glChartCanvas::CanClipViewport(*this))
        return LLRegion(GetBBox());

    OCPNRegionIterator it( region );
    LLRegion r;
    while( it.HaveRects() ) {
        wxRect rect = it.GetRect();

        int x1 = rect.x, y1 = rect.y, x2 = x1 + rect.width, y2 = y1 + rect.height;
        int p[8] = {x1, y1, x2, y1, x2, y2, x1, y2};
        double pll[540];
        int j;

        /* if the viewport is rotated, we must split the segments as straight lines in lat/lon
           coordinates map to curves in projected coordinate space */
        if(fabs( rotation ) >= 0.0001) {
            j=0;
            double lastlat, lastlon;
            int li = 6;
            GetLLFromPix(wxPoint(p[li], p[li+1]), &lastlat, &lastlon);
            for(int i=0; i<8; i+=2) {
                double lat, lon;
                GetLLFromPix(wxPoint(p[i], p[i+1]), &lat, &lon);

                // use 2 degree grid
                double grid = 2;
                int lat_splits = floor(fabs(lat-lastlat) / grid);
                double lond = fabs(lon-lastlon);
                int lon_splits = floor((lond > 180 ? 360-lond : lond) / grid);
                int splits = wxMax(lat_splits, lon_splits) + 1;

                for(int k = 1; k<splits; k++) {
                    float d = (float)k / splits;
                    GetLLFromPix(wxPoint((1-d)*p[li] + d*p[i], (1-d)*p[li+1] + d*p[i+1]), pll+j, pll+j+1);
                    j += 2;
                }
                pll[j++] = lat;
                pll[j++] = lon;
                li = i;
                lastlat = lat, lastlon = lon;
            }
        } else {
            j=8;
            for(int i=0; i<j; i+=2)
                GetLLFromPix(wxPoint(p[i], p[i+1]), pll+i, pll+i+1);
        }

        // resolve (this works even if rectangle crosses both 0 and 180)
        for(int i=0; i<j; i+=2) {
            if(pll[i+1] <= clon - 180)
                pll[i+1] += 360;
            else if(pll[i+1] >= clon + 180)
                pll[i+1] -= 360;
        }

        r.Union(LLRegion(j/2, pll));
        it.NextRect();
    }
    return r;
#endif    
}

struct ContourRegion
{
    double maxlat;
    bool subtract;
    OCPNRegion r;
};

OCPNRegion ViewPort::GetVPRegionIntersect( const OCPNRegion &region, const LLRegion &llregion, int chart_native_scale )
{
    double rotation_save = rotation;
    rotation = 0;

    std::list<ContourRegion> cregions;
    for(std::list<poly_contour>::const_iterator i = llregion.contours.begin(); i != llregion.contours.end(); i++) {
        float *contour_points = new float[2*i->size()];
        int idx = 0;
        std::list<contour_pt>::const_iterator j;
        for(j = i->begin(); j != i->end(); j++) {
            contour_points[idx++] = j->y;
            contour_points[idx++] = j->x;
        }

        double total = 0, maxlat = -90;
        int pl = idx - 2;
        double x0 = contour_points[0] - contour_points[pl+0];
        double y0 = contour_points[1] - contour_points[pl+1];
        // determine winding direction of this contour
        for(int p=0; p<idx; p+=2) {
            maxlat = wxMax(maxlat, contour_points[p]);
            int pn = p < idx - 2 ? p + 2 : 0;
            double x1 = contour_points[pn+0] - contour_points[p+0];
            double y1 = contour_points[pn+1] - contour_points[p+1];
            total += x1*y0 - x0*y1;
            x0 = x1, y0 = y1;
        }

        ContourRegion s;
        s.maxlat = maxlat;
        s.subtract = total < 0;
        s.r = GetVPRegionIntersect(region, i->size(), contour_points, chart_native_scale, NULL);
        delete [] contour_points;

        std::list<ContourRegion>::iterator k = cregions.begin();
        while(k!=cregions.end()) {
            if(k->maxlat < s.maxlat)
                break;
            k++;
        }
        cregions.insert(k, s);
    }

    OCPNRegion r;
    for(std::list<ContourRegion>::iterator k = cregions.begin(); k!=cregions.end(); k++){
        
        if(k->r.Ok()){
            if(k->subtract)
                r.Subtract(k->r);
            else
                r.Union(k->r);
        }
    }

    rotation = rotation_save;
    return r;
}

OCPNRegion ViewPort::GetVPRegionIntersect( const OCPNRegion &Region, size_t nPoints, float *llpoints,
                                           int chart_native_scale, wxPoint *ppoints )
{
    //  Calculate the intersection between a given OCPNRegion (Region) and a polygon specified by lat/lon points.

    //    If the viewpoint is highly overzoomed wrt to chart native scale, the polygon region may be huge.
    //    This can be very expensive, and lead to crashes on some platforms (gtk in particular)
    //    So, look for this case and handle appropriately with respect to the given Region

    if( chart_scale < chart_native_scale / 10 ) {

        //    Scan the points one-by-one, so that we can get min/max to make a bbox
        float *pfp = llpoints;
        float lon_max = -10000.;
        float lon_min = 10000.;
        float lat_max = -10000.;
        float lat_min = 10000.;

        for( unsigned int ip = 0; ip < nPoints; ip++ ) {
            lon_max = wxMax(lon_max, pfp[1]);
            lon_min = wxMin(lon_min, pfp[1]);
            lat_max = wxMax(lat_max, pfp[0]);
            lat_min = wxMin(lat_min, pfp[0]);

            pfp += 2;
        }

        LLBBox chart_box;
        chart_box.Set( lat_min, lon_min, lat_max, lon_max );

        //    Case:  vpBBox is completely outside the chart box, or vice versa
        //    Return an empty region
        if( chart_box.IntersectOut( vpBBox ) )
            return OCPNRegion();

        //    Case:  vpBBox is completely inside the chart box
        //      Note that this test is not perfect, and will fail for some charts.
        //      The chart coverage may be  essentially triangular, and the viewport box
        //      may be in the "cut off" segment of the chart_box, and not actually
        //      exhibit any true overlap.  Results will be reported incorrectly.
        //      How to fix: maybe scrub the chart points and see if it is likely that
        //      a region may be safely built and intersection tested.

        if( chart_box.IntersectIn( vpBBox ) )
            return Region;

        wxPoint p1 = GetPixFromLL( lat_max, lon_min );  // upper left
        wxPoint p2 = GetPixFromLL( lat_min, lon_max );   // lower right

        OCPNRegion r( p1, p2 );
        r.Intersect( Region );
        return r;
    }

    //    More "normal" case

    wxPoint *pp;

    //    Use the passed point buffer if available
    if( ppoints == NULL ) pp = new wxPoint[nPoints];
    else
        pp = ppoints;

    float *pfp = llpoints;

    
    wxPoint p = GetPixFromLL( pfp[0], pfp[1] );
    int poly_x_max = INVALID_COORD, poly_y_max = INVALID_COORD,
        poly_x_min = INVALID_COORD, poly_y_min = INVALID_COORD;
    
    bool valid = false;
    for( unsigned int ip = 0; ip < nPoints; ip++ ) {
        wxPoint p = GetPixFromLL( pfp[0], pfp[1] );
        pp[ip] = p;
        if(p.x == INVALID_COORD)
            continue;

        if(valid) {
            poly_x_max = wxMax(poly_x_max, p.x);
            poly_y_max = wxMax(poly_y_max, p.y);
            poly_x_min = wxMin(poly_x_min, p.x);
            poly_y_min = wxMin(poly_y_min, p.y);
        } else {
            poly_x_max = p.x;
            poly_y_max = p.y;
            poly_x_min = p.x;
            poly_y_min = p.y;
            valid = true;
        }
        pfp += 2;
    }

    if(!valid)
    {
        delete[] pp;
        return OCPNRegion(); //empty;
    }
 
    //  We want to avoid processing regions with very large rectangle counts,
    //  so make some tests for special cases

    float_2Dpt p0, p1, p2, p3;
    
    //  First, calculate whether any segment of the input polygon intersects the specified Region
    int nrect = 0;
    bool b_intersect = false;
    OCPNRegionIterator screen_region_it1( Region );
    while( screen_region_it1.HaveRects() ) {
        wxRect rect = screen_region_it1.GetRect();

        double lat, lon;
        
        //  The screen region corners
        GetLLFromPix( wxPoint(rect.x, rect.y), &lat, &lon );
        p0.y = lat; p0.x = lon;
        
        GetLLFromPix( wxPoint(rect.x + rect.width, rect.y), &lat, &lon );
        p1.y = lat; p1.x = lon;
        
        GetLLFromPix( wxPoint(rect.x + rect.width, rect.y + rect.height), &lat, &lon );
        p2.y = lat; p2.x = lon;
        
        GetLLFromPix( wxPoint(rect.x, rect.y + rect.height), &lat, &lon );
        p3.y = lat; p3.x = lon;
        
        
        for(size_t i=0 ; i < nPoints-1 ; i++){            
            //  Quick check on y dimension
            int y0 = pp[i].y; int y1 = pp[i+1].y;

            if(y0 == INVALID_COORD || y1 == INVALID_COORD)
                continue;
            
            if( ((y0 < rect.y) && (y1 < rect.y)) ||
                ((y0 > rect.y+rect.height) && (y1 > rect.y+rect.height)) )
                continue;               // both ends of line outside of box, top or bottom
            
            //  Look harder
            float_2Dpt f0; f0.y = llpoints[i * 2];     f0.x = llpoints[(i * 2) + 1];
            float_2Dpt f1; f1.y = llpoints[(i+1) * 2]; f1.x = llpoints[((i+1) * 2) + 1];
            b_intersect |= Intersect_FL( p0, p1, f0, f1) != 0; if(b_intersect) break;
            b_intersect |= Intersect_FL( p1, p2, f0, f1) != 0; if(b_intersect) break;
            b_intersect |= Intersect_FL( p2, p3, f0, f1) != 0; if(b_intersect) break;
            b_intersect |= Intersect_FL( p3, p0, f0, f1) != 0; if(b_intersect) break;
            
            //  Must check the case where the input polygon has been pre-normalized, eg (0 < lon < 360), as cm93
            f0.x -= 360.;
            f1.x -= 360.;
            b_intersect |= Intersect_FL( p0, p1, f0, f1) != 0; if(b_intersect) break;
            b_intersect |= Intersect_FL( p1, p2, f0, f1) != 0; if(b_intersect) break;
            b_intersect |= Intersect_FL( p2, p3, f0, f1) != 0; if(b_intersect) break;
            b_intersect |= Intersect_FL( p3, p0, f0, f1) != 0; if(b_intersect) break;
            
        }
        
        // Check segment, last point back to first point
        if(!b_intersect){
            
            float_2Dpt f0; f0.y = llpoints[(nPoints-1) * 2];     f0.x = llpoints[((nPoints-1) * 2) + 1];
            float_2Dpt f1; f1.y = llpoints[0]; f1.x = llpoints[1];
            b_intersect |= Intersect_FL( p0, p1, f0, f1) != 0;
            b_intersect |= Intersect_FL( p1, p2, f0, f1) != 0;
            b_intersect |= Intersect_FL( p2, p3, f0, f1) != 0;
            b_intersect |= Intersect_FL( p3, p0, f0, f1) != 0;
 
            f0.x -= 360.;
            f1.x -= 360.;
            b_intersect |= Intersect_FL( p0, p1, f0, f1) != 0;
            b_intersect |= Intersect_FL( p1, p2, f0, f1) != 0;
            b_intersect |= Intersect_FL( p2, p3, f0, f1) != 0;
            b_intersect |= Intersect_FL( p3, p0, f0, f1) != 0;
            
        }
                
        screen_region_it1.NextRect();
        nrect++;
    }

    //  If there is no itersection, we need to consider the case where
    //  the subject polygon is entirely within the Region
    bool b_contained = false;
    if(!b_intersect){
        OCPNRegionIterator screen_region_it2( Region );
        while( screen_region_it2.HaveRects() ) {
            wxRect rect = screen_region_it2.GetRect();
 
            for(size_t i=0 ; i < nPoints-1 ; i++){
                int x0 = pp[i].x;  int y0 = pp[i].y;
                if(x0 == INVALID_COORD)
                    continue;

                if((x0 < rect.x) || (x0 > rect.x+rect.width))
                    continue;
                
                if((y0 < rect.y) || (y0 > rect.y+rect.height))
                    continue;
                
                b_contained = true;
                break;
            }
            screen_region_it2.NextRect();
        }
    }
    
#if 1    
    // and here is the payoff
    if(!b_contained && !b_intersect){
        //  Two cases to consider
        wxRect rpoly( poly_x_min, poly_y_min, poly_x_max - poly_x_min , poly_y_max - poly_y_min);
        wxRect rRegion = Region.GetBox();
        if(rpoly.Contains(rRegion)){
        //  subject poygon may be large enough to fully encompass the target Region,
        //  but it might not, especially for irregular or concave charts.
        //  So we cannot directly shortcut here
        //  Better check....
        
#if 1
            if(nrect == 1){                 // most common case
            // If the subject polygon contains the center of the target rectangle, then
            // the intersection must be the target rectangle
                float rlat = (p0.y + p2.y)/2.;
                float rlon = (p0.x + p1.x)/2.;
                
                if(G_PtInPolygon_FL((float_2Dpt *)llpoints, nPoints, rlon, rlat)){
                    if( NULL == ppoints ) delete[] pp;
                    return Region;
                }
                rlon += 360.;
                if(G_PtInPolygon_FL((float_2Dpt *)llpoints, nPoints, rlon, rlat)){
                    if( NULL == ppoints ) delete[] pp;
                    return Region;
                }
                
                //  otherwise, there is no intersection
                else{
                    if( NULL == ppoints ) delete[] pp;
                    wxRegion r;
                    return r;
                }
            }
        
#endif        
            
        }
        else{
        //  Subject polygon is entirely outside of target Region
        //  so the intersection must be empty.
            if( NULL == ppoints ) delete[] pp;
            wxRegion r;
            return r;
        }
    }
    else if(b_contained && !b_intersect){
        //  subject polygon is entirely withing the target Region,
        //  so the intersection is the subject polygon
        OCPNRegion r = OCPNRegion( nPoints, pp );
        if( NULL == ppoints ) delete[] pp;
        return r;
    }
        
#endif    
        
    
#ifdef __WXGTK__
    sigaction(SIGSEGV, NULL, &sa_all_old);             // save existing action for this signal

    struct sigaction temp;
    sigaction(SIGSEGV, NULL, &temp);// inspect existing action for this signal

    temp.sa_handler = catch_signals;// point to my handler
    sigemptyset(&temp.sa_mask);// make the blocking set
    // empty, so that all
    // other signals will be
    // unblocked during my handler
    temp.sa_flags = 0;
    sigaction(SIGSEGV, &temp, NULL);

    if(sigsetjmp(env, 1))//  Something in the below code block faulted....
    {
        sigaction(SIGSEGV, &sa_all_old, NULL);        // reset signal handler

        return Region;

    }

    else
    {

        OCPNRegion r = OCPNRegion(nPoints, pp);
        if(NULL == ppoints)
            delete[] pp;

        sigaction(SIGSEGV, &sa_all_old, NULL);        // reset signal handler
        r.Intersect(Region);
        return r;
    }

#else
    OCPNRegion r = OCPNRegion( nPoints, pp );

    if( NULL == ppoints ) delete[] pp;

    r.Intersect( Region );
    return r;

#endif
}

wxRect ViewPort::GetVPRectIntersect( size_t n, float *llpoints )
{
    //  Calculate the intersection between the currect VP screen
    //  and the bounding box of a polygon specified by lat/lon points.

    float *pfp = llpoints;

    wxBoundingBox point_box;
    for( unsigned int ip = 0; ip < n; ip++ ) {
        point_box.Expand(pfp[1], pfp[0]);
        pfp += 2;
    }

    wxPoint pul = GetPixFromLL( point_box.GetMaxY(), point_box.GetMinX() );
    wxPoint plr = GetPixFromLL( point_box.GetMinY(), point_box.GetMaxX() );

    OCPNRegion r( pul, plr );
    OCPNRegion rs(rv_rect);

    r.Intersect(rs);

    return r.GetBox();


}

void ViewPort::SetBoxes( void )
{

    //  In the case where canvas rotation is applied, we need to define a larger "virtual" pixel window size to ensure that
    //  enough chart data is fatched and available to fill the rotated screen.
    rv_rect = wxRect( 0, 0, pix_width, pix_height );

    //  Specify the minimum required rectangle in unrotated screen space which will supply full screen data after specified rotation
    if (( fabs( skew ) > .0001 ) || (fabs(rotation )>.0001 )) {

        double rotator = rotation;
        double lpixh = pix_height;
        double lpixw = pix_width;

        lpixh = wxMax(lpixh, (fabs(pix_height * cos(skew)) + fabs(pix_width * sin(skew))));
        lpixw = wxMax(lpixw, (fabs(pix_width * cos(skew)) + fabs(pix_height * sin(skew))));

        int dy = wxRound(
                     fabs( lpixh * cos( rotator ) ) + fabs( lpixw * sin( rotator ) ) );
        int dx = wxRound(
                     fabs( lpixw * cos( rotator ) ) + fabs( lpixh * sin( rotator ) ) );

        //  It is important for MSW build that viewport pixel dimensions be multiples of 4.....
        if( dy % 4 ) dy += 4 - ( dy % 4 );
        if( dx % 4 ) dx += 4 - ( dx % 4 );

        int inflate_x = wxMax(( dx - pix_width ) / 2, 0);
        int inflate_y = wxMax(( dy - pix_height ) / 2, 0);
        
        //  Grow the source rectangle appropriately
        rv_rect.Inflate( inflate_x, inflate_y );
    }

    //  Compute Viewport lat/lon reference points for co-ordinate hit testing

    //  This must be done in unrotated space with respect to full unrotated screen space calculated above
    double rotation_save = rotation;
    SetRotationAngle(0.0);

    wxPoint ul( rv_rect.x, rv_rect.y ), lr( rv_rect.x + rv_rect.width, rv_rect.y + rv_rect.height );
    double dlat_min, dlat_max, dlon_min, dlon_max;

    bool hourglass = false;
    switch(m_projection_type) {
    case PROJECTION_TRANSVERSE_MERCATOR:
    case PROJECTION_STEREOGRAPHIC:
    case PROJECTION_GNOMONIC:
        hourglass = true;
    case PROJECTION_POLYCONIC:
    case PROJECTION_POLAR:
    case PROJECTION_ORTHOGRAPHIC:
    {
        double d;

        if( clat > 0 ) { // north polar
            wxPoint u( rv_rect.x + rv_rect.width/2, rv_rect.y );
            wxPoint ur( rv_rect.x + rv_rect.width, rv_rect.y );
            GetLLFromPix( ul, &d, &dlon_min );
            GetLLFromPix( ur, &d, &dlon_max );
            GetLLFromPix( lr, &dlat_min, &d );
            GetLLFromPix( u, &dlat_max, &d );

            if(fabs(fabs(d - clon) - 180) < 1) { // the pole is onscreen
                dlat_max = 90;
                dlon_min = -180;
                dlon_max = 180;
            } else if(wxIsNaN(dlat_max))
                dlat_max = 90;

            if(hourglass) {
                // near equator, center may be less
                wxPoint l( rv_rect.x + rv_rect.width/2, rv_rect.y + rv_rect.height );
                double dlat_min2;
                GetLLFromPix( l, &dlat_min2, &d );
                dlat_min = wxMin(dlat_min, dlat_min2);
            }

            if(wxIsNaN(dlat_min)) //  world is off-screen
                dlat_min = clat - 90;
        } else { // south polar
            wxPoint l( rv_rect.x + rv_rect.width/2, rv_rect.y + rv_rect.height );
            wxPoint ll( rv_rect.x, rv_rect.y + rv_rect.height );
            GetLLFromPix( ul, &dlat_max, &d );
            GetLLFromPix( lr, &d, &dlon_max );
            GetLLFromPix( ll, &d, &dlon_min );
            GetLLFromPix( l, &dlat_min, &d );            

            if(fabs(fabs(d - clon) - 180) < 1) { // the pole is onscreen
                dlat_min = -90;
                dlon_min = -180;
                dlon_max = 180;
            } else if(wxIsNaN(dlat_min))
                dlat_min = -90;

            if(hourglass) {
                // near equator, center may be less
                wxPoint u( rv_rect.x + rv_rect.width/2, rv_rect.y );
                double dlat_max2;
                GetLLFromPix( u, &dlat_max2, &d );
                dlat_max = wxMax(dlat_max, dlat_max2);
            }

            if(wxIsNaN(dlat_max)) //  world is off-screen
                dlat_max = clat + 90;
        }

        if(wxIsNaN(dlon_min)) {
            // if neither pole is visible, but left and right of the screen are in space
            // we can avoid drawing the far side of the earth
            if(dlat_max < 90 && dlat_min > -90) {
                dlon_min = clon - 90 - fabs(clat); // this logic is not optimal, is it always correct?
                dlon_max = clon + 90 + fabs(clat);
            } else {
                dlon_min = -180;
                dlon_max = 180;
            }
        }
    } break;

    default: // works for mercator and equirectangular
    {
        GetLLFromPix( ul, &dlat_max, &dlon_min );
        GetLLFromPix( lr, &dlat_min, &dlon_max );
    }
    }

    if( clon < dlon_min )
        dlon_min -= 360;
    else if(clon > dlon_max)
        dlon_max += 360;

    //  Set the viewport lat/lon bounding box appropriately
    vpBBox.Set( dlat_min, dlon_min, dlat_max, dlon_max );

    // Restore the rotation angle
    SetRotationAngle( rotation_save );
}

void ViewPort::SetBBoxDirect( double latmin, double lonmin, double latmax, double lonmax)
{
    vpBBox.Set( latmin, lonmin, latmax, lonmax );
}

ViewPort ViewPort::BuildExpandedVP(int width, int height)
{
    ViewPort new_vp = *this;
    
    new_vp.pix_width = width;
    new_vp.pix_height = height;
    new_vp.SetBoxes();
    
    return new_vp;
}


