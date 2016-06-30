/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Latitude and Longitude regions
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2015 by Sean D'Epagnier                                 *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "LLRegion.h"

static inline double cross(const contour_pt &v1, const contour_pt &v2)
{
    return v1.y*v2.x - v1.x*v2.y;
}

static inline double dot(const contour_pt &v1, const contour_pt &v2)
{
    return v1.x*v2.x + v1.y*v2.y;
}

static inline double dist2(const contour_pt &v)
{
    return dot(v, v);
}

static inline contour_pt vector(const contour_pt &p1, const contour_pt &p2)
{
    contour_pt r = {p2.y - p1.y, p2.x - p1.x};
    return r;
}

LLRegion::LLRegion( float minlat, float minlon, float maxlat, float maxlon)
{
    InitBox(minlat, minlon, maxlat, maxlon);
}

LLRegion::LLRegion( const LLBBox& llbbox )
{
    InitBox(llbbox.GetMinLat(), llbbox.GetMinLon(), llbbox.GetMaxLat(), llbbox.GetMaxLon());
}

LLRegion::LLRegion( size_t n, const float *points )
{
    double *pts = new double[2*n];
    for(size_t i=0; i<2*n; i++)
        pts[i] = points[i];
    InitPoints(n, pts);
    delete [] pts;
}

LLRegion::LLRegion( size_t n, const double *points )
{
    InitPoints(n, points);
}

// determine if a loop of points is counter clockwise
bool LLRegion::PointsCCW( size_t n, const double *points )
{
    double total = 0;
    for(unsigned int i=0; i<2*n; i+=2) {
        int pn = i < 2*(n-1) ? i + 2 : 0;
        total += (points[pn+0] - points[i+0]) * (points[pn+1] + points[i+1]);
    }
    return total > 0;
}

void LLRegion::Print() const
{
    for(std::list<poly_contour>::const_iterator i = contours.begin(); i != contours.end(); i++) {
        printf("[");
        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++)
            printf("(%g %g) ", j->y, j->x);
        printf("]\n");
    }
}

void LLRegion::plot(const char*fn) const
{
    char filename[100] = "/home/sean/";
    strcat(filename, fn);
    FILE *f = fopen(filename, "w");
    for(std::list<poly_contour>::const_iterator i = contours.begin(); i != contours.end(); i++) {
        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++)
            fprintf(f, "%f %f\n", j->x, j->y);
        
        fprintf(f, "%f %f\n", i->begin()->x, i->begin()->y);
        fprintf(f, "\n");
    }
    fclose(f);
}

LLBBox LLRegion::GetBox() const
{
    if(contours.empty())
        return LLBBox(); // invalid box

    if(m_box.GetValid())
        return m_box;

    // there are 3 possible longitude bounds: -180 to 180, 0 to 360, -360 to 0
    double minlat = 90, minlon[3] = {180, 360, 0};
    double maxlat = -90, maxlon[3] = {-180, 0, -360};
    for(std::list<poly_contour>::const_iterator i = contours.begin(); i != contours.end(); i++) {
        bool neg = false, pos = false;
        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++)
            if(j->x < 0)
                neg = true;
            else
                pos = true;

        double resolved[3] = {0, 0, 0};
        if(neg && !pos)
            resolved[1] = 360;
        if(pos && !neg)
            resolved[2] = -360;

        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++) {
            minlat = wxMin(minlat, j->y);
            maxlat = wxMax(maxlat, j->y);
                
            for(int k = 0; k<3; k++) {
                minlon[k] = wxMin(minlon[k], j->x + resolved[k]);
                maxlon[k] = wxMax(maxlon[k], j->x + resolved[k]);
            }
        }
    }

    double d[3];
    for(int k = 0; k<3; k++) {
        double a = maxlon[k] + minlon[k];
        // eliminate cases where the average longitude falls outside of -180 to 180
        if(a <= -360 || a >= 360)
            d[k] = 360;
        else
            d[k] = maxlon[k] - minlon[k];
    }

    // find minimum difference (best case to use)
    double epsilon = 1e-2;  // because floating point rounding favor... d1, then d2 then d3
    d[1] += epsilon, d[2] += 2*epsilon;
    int mink = 0;
    for(int k=1; k<3; k++)
        if(d[k] < d[mink])
            mink = k;

    LLBBox &box = const_cast<LLBBox&>(m_box);
    box.Set(minlat, minlon[mink], maxlat, maxlon[mink]);
    return m_box;
}

static inline int ComputeState(const LLBBox &box, const contour_pt &p)
{
    int state = 0;
    if(p.x >= box.GetMinLon()) {
        if(p.x > box.GetMaxLon())
            state = 2;
        else
            state = 1;
    }

    if(p.y >= box.GetMinLat()) {
        if(p.y > box.GetMaxLat())
            state += 6;
        else
            state += 3;
    }
    return state;
}

inline bool TestPoint(contour_pt p0, contour_pt p1, double x, double y)
{
    contour_pt p = { y, x};
    return cross(vector(p0, p1), vector(p0, p)) < 0;
}

bool LLRegion::IntersectOut(const LLBBox &box) const
{
    // First do faster test of bounding boxes
    if(GetBox().IntersectOut(box))
        return true;

    return NoIntersection(box);
}

// this function may produce false positives in degenerate cases
bool LLRegion::Contains(float lat, float lon) const
{
    if(lon == 180 && Contains(lat, -180)) return true;
    if(lon > 180)  return Contains(lat, lon-360);

    int cnt = 0;
    for(std::list<poly_contour>::const_iterator i = contours.begin(); i != contours.end(); i++) {
        contour_pt l = *i->rbegin();
        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++) {
            contour_pt p = *j, a, b;
            if(p.x < l.x)
                a = p, b = l;
            else
                a = l, b = p;
            if(lon > a.x && lon < b.x) {
                if(lat > a.y) {
                    if(lat > b.y)
                        cnt++;
                    else
                        cnt += TestPoint(a, b, lon, lat);
                } else if(lat > b.y)
                    cnt += TestPoint(a, b, lon, lat);
            }

            if(lat == a.y || lat == b.y)
                return true;

            if(lon == a.x || lon == b.x)
                return true;
            l = p;
        }
    }
    return cnt&1;
}

struct work
{
    work(LLRegion &r) : region(r) {
        tobj = gluNewTess();
    }

    ~work() {
        gluDeleteTess(tobj);
        for(std::list<double*>::iterator i = data.begin(); i!=data.end(); i++)
            delete [] *i;
        data.clear();
    }

    double *NewData() {
        double *d = new double[3];
        data.push_back(d);
        return d;
    }

    void PutVertex(const contour_pt &j) {
        double *p = NewData();
        p[0] = j.x, p[1] = j.y, p[2] = 0;
        gluTessVertex(tobj, p, p);
    }

    std::list<double*> data;
    poly_contour contour;
    GLUtesselator *tobj;
    LLRegion &region;
};


static void /*APIENTRY*/ LLvertexCallback(GLvoid *vertex, void *user_data)
{
    work *w = (work*)user_data;
    const GLdouble *pointer = (GLdouble *)vertex;
    contour_pt p;
    p.x = pointer[0], p.y = pointer[1];
    w->contour.push_back(p);
}

static void /*APIENTRY*/ LLbeginCallback(GLenum which) {
}

static void /*APIENTRY*/ LLendCallback(void *user_data)
{
    work *w = (work*)user_data;
    if(w->contour.size()) {
        w->region.contours.push_back(w->contour);
        w->contour.clear();
    }    
}

static void /*APIENTRY*/ LLcombineCallback( GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4],
                      GLdouble **dataOut, void *user_data )
{
    work *w = (work*)user_data;
    GLdouble *vertex = w->NewData(); 
    memcpy(vertex, coords, 3*(sizeof *coords)); 
    *dataOut = vertex;    
}

static void /*APIENTRY*/ LLerrorCallback(GLenum errorCode)
{
    const GLubyte *estring;
    estring = gluErrorString(errorCode);
    fprintf (stderr, "Tessellation Error: %s\n", estring);
    exit (0);
}

void LLRegion::Intersect(const LLRegion& region)
{
    if(NoIntersection(region)) {
        Clear();
        return;
    }

    Put(region, GLU_TESS_WINDING_ABS_GEQ_TWO, false);
}

void LLRegion::Union(const LLRegion& region)
{
    if(NoIntersection(region)) {
        Combine(region);
        return;
    }

    Put(region, GLU_TESS_WINDING_POSITIVE, false);
}

void LLRegion::Subtract(const LLRegion& region)
{
    if(NoIntersection(region))
        return;
    
    Put(region, GLU_TESS_WINDING_POSITIVE, true);
}

void LLRegion::Reduce(double factor)
{
    double factor2 = factor*factor;

    std::list<poly_contour>::iterator i = contours.begin();
    while(i != contours.end()) {
        if(i->size() < 3) {
            printf("invalid contour");
            continue;
        }

        // reduce segments
        contour_pt l = *i->rbegin();
        poly_contour::iterator j = i->begin(), k;
        while(j != i->end()) {
            k = j;
            j++;
            if(dist2(vector(*k, l)) < factor2)
                i->erase(k);
            else
                l = *k;
        }

        // erase zero contours
        if(i->size() < 3)
            i = contours.erase(i);
        else
            i++;
    }

    //Optimize();
}

// slightly ugly, but efficient intersection algorithm
bool LLRegion::NoIntersection(const LLBBox& box) const
{
    return false; // there are occasional false positives we must fix first

#if 0    
    double minx = box.GetMinLon(), maxx = box.GetMaxLon(), miny = box.GetMinLat(), maxy = box.GetMaxLat();
    if(Contains(miny, minx))
        return false;

    // test if any segment crosses the box
    for(std::list<poly_contour>::const_iterator i = contours.begin(); i != contours.end(); i++) {
        contour_pt l = *i->rbegin();
        int state = ComputeState(box, l), lstate = state;
        if(state == 4) return false;
        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++) {
            contour_pt p = *j;
            int quadrant = p.x > l.x ? 1 : 0;
            if(p.y > l.y) quadrant += 2;
            switch(state*4 + quadrant) {
            case 0: goto skip;
            case 1: if(p.x >= minx) state = p.x > maxx ? 2 : 1; goto skip;
            case 2: if(p.y >= miny) state = p.y > maxy ? 6 : 3; goto skip;
            case 4: if(p.x < minx) state = 0; goto skip;
            case 5: if(p.x > maxx) state = 2; goto skip;
            case 8: if(p.x <= maxx) state = p.x < minx ? 0 : 1; goto skip;
            case 9: goto skip;
            case 11: if(p.y >= miny) state = p.y > maxy ? 8 : 5; goto skip;
            case 12: if(p.y < miny) state = 0; goto skip;
            case 14: if(p.y > maxy) state = 6; goto skip;
            case 21: if(p.y < miny) state = 2; goto skip;
            case 23: if(p.y > maxy) state = 8; goto skip;
            case 24: if(p.y <= maxy) state = p.y < miny ? 0 : 3; goto skip;
            case 26: goto skip;
            case 27: if(p.x >= minx) state = p.x > maxx ? 8 : 7; goto skip;
            case 30: if(p.x < minx) state = 6; goto skip;
            case 31: if(p.x > maxx) state = 8; goto skip;
            case 33: if(p.y <= maxy) state = p.y < miny ? 2 : 5; goto skip;
            case 34: if(p.x <= maxx) state = p.x < minx ? 6 : 7; goto skip;
            case 35: goto skip;
            }

            state = ComputeState(box, *j);
            if(state == 4) return false;
            switch(lstate) {
#define TEST_CASE(NO_INT, CASEA, CASEB, CASEAB, AX, AY, BX, BY) \
                switch(state) { NO_INT break; \
                    CASEAB if(TestPoint(l, p, BX##x, BY##y)) return false; \
                    CASEA  if(TestPoint(p, l, AX##x, AY##y)) return false; break;  \
                    CASEB  if(TestPoint(l, p, BX##x, BY##y)) return false; break;  \
                default: printf("invalid state inner %d %d\n", lstate, state); } break;
            case 0: TEST_CASE(case 0: case 1: case 2: case 3: case 6:,
                case 5:, case 7:, case 8:, max, min, min, max)
            case 1: TEST_CASE(case 0: case 1: case 2:,
                case 5: case 8:, case 3: case 6:, case 7:, max, min, min, min)
            case 2: TEST_CASE(case 0: case 1: case 2: case 5: case 8:,
                case 7:, case 3:, case 6:, max, max, min, min)
            case 3: TEST_CASE(case 0: case 3: case 6:,
                case 1: case 2:, case 7: case 8:, case 5:, min, min, min, max)
//            case 4: return false; // should never hit
            case 5: TEST_CASE(case 2: case 5: case 8:,
                case 6: case 7:, case 0: case 1:, case 3:, max, max, max, min)
            case 6: TEST_CASE(case 0: case 3: case 6: case 7: case 8:,
                case 1:, case 5:, case 2:, min, min, max, max)
            case 7: TEST_CASE(case 6: case 7: case 8:,
                case 0: case 3:, case 2: case 5:, case 1:, min, max, max, max)
            case 8: TEST_CASE(case 2: case 5: case 6: case 7: case 8:,
                case 3:, case 1:, case 0:, min, max, max, min)
            default: printf("invalid state\n");
            }
        skip:
            lstate = state;
            l = p;
        }
    }

    return true;
#endif    
}

// internal test to see if regions don't intersect (optimization)
bool LLRegion::NoIntersection(const LLRegion& region) const
{
    if(Empty() || region.Empty())
        return true;
    
    LLBBox box = GetBox(), rbox = region.GetBox();
    return box.IntersectOut(rbox) || NoIntersection(rbox) || region.NoIntersection(box);
}

void LLRegion::PutContours(work &w, const LLRegion& region, bool reverse)
{
    for(std::list<poly_contour>::const_iterator i = region.contours.begin(); i != region.contours.end(); i++) {
        gluTessBeginContour(w.tobj);
        if(reverse)
            for(poly_contour::const_reverse_iterator j = i->rbegin(); j != i->rend(); j++)
                w.PutVertex(*j);
        else
            for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++)
                w.PutVertex(*j);
        gluTessEndContour(w.tobj);
    }
}

void LLRegion::Put( const LLRegion& region, int winding_rule, bool reverse)
{
    work w(*this);
   
    gluTessCallback( w.tobj, GLU_TESS_VERTEX_DATA, (_GLUfuncptr) &LLvertexCallback );
    gluTessCallback( w.tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &LLbeginCallback );
    gluTessCallback( w.tobj, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) &LLcombineCallback );
    gluTessCallback( w.tobj, GLU_TESS_END_DATA, (_GLUfuncptr) &LLendCallback );
    gluTessCallback( w.tobj, GLU_TESS_ERROR, (_GLUfuncptr) &LLerrorCallback );
    gluTessProperty(w.tobj, GLU_TESS_WINDING_RULE, winding_rule);
    gluTessProperty(w.tobj, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);
//    gluTessProperty(w.tobj, GLU_TESS_TOLERANCE, 1e-5);

    gluTessNormal( w.tobj, 0, 0, 1);

    gluTessBeginPolygon(w.tobj, &w);

    PutContours(w, *this);
    PutContours(w, region, reverse);
    contours.clear();
    gluTessEndPolygon( w.tobj ); 

    Optimize();
    m_box.Invalidate();
}

// same result as union, but only allowed if there is no intersection
void LLRegion::Combine(const LLRegion& region)
{
    for(std::list<poly_contour>::const_iterator i = region.contours.begin(); i != region.contours.end(); i++)
        contours.push_back(*i);
    m_box.Invalidate();
}

void LLRegion::InitBox( float minlat, float minlon, float maxlat, float maxlon)
{
    if(minlon < -180)
        minlon += 360, maxlon += 360;

    contour_pt p[4];
    p[0].x = maxlon, p[0].y = minlat;
    p[1].x = maxlon, p[1].y = maxlat;
    p[2].x = minlon, p[2].y = maxlat;
    p[3].x = minlon, p[3].y = minlat;
    poly_contour c;
    for(int i=0; i<4; i++)
        c.push_back(p[i]);
    contours.push_back(c);

    if(minlon < -180 || maxlon > 180)
        AdjustLongitude();
}

void LLRegion::InitPoints( size_t n, const double *points)
{
    if(n < 3) {
        printf("invalid point count\n");
        return;
    }

    std::list<contour_pt> pts;
    bool adjust = false;

    bool ccw = PointsCCW(n, points);
    for(unsigned int i=0; i<2*n; i+=2) {
        contour_pt p;
        p.y = points[i+0];
        p.x = points[i+1];
        if(p.x < -180 || p.x > 180)
            adjust = true;
        if(ccw)
            pts.push_back(p);
        else
            pts.push_front(p);
    }

    contours.push_back(pts);

    if(adjust)
        AdjustLongitude();
    Optimize();
}

void LLRegion::AdjustLongitude()
{
    // Make region bounded for longitude of +- 180
    // areas from -360 to -180 and 180 to 360 are shifted
    LLRegion clip(-90, -180, 90, 180), resolved = *this;
    resolved.Subtract(clip);
    if(!resolved.Empty()) {
        Intersect(clip);
        // apply longitude offset
        for(std::list<poly_contour>::iterator i = resolved.contours.begin(); i != resolved.contours.end(); i++)
            for(poly_contour::iterator j = i->begin(); j != i->end(); j++)
                if(j->x > 0)
                    j->x -= 360;
                else
                    j->x += 360;
        Union(resolved);
    }
    Intersect(clip);
}

void LLRegion::Optimize()
{
    // merge parallel segments
    std::list<poly_contour>::iterator i = contours.begin();
    while(i != contours.end()) {
        if(i->size() < 3) {
            printf("invalid contour");
            continue;
        }

        // round toward 180 and -180 as this is where adjusted longitudes
        // are split, and so zero contours can get eliminated by the next step
        for(poly_contour::iterator j = i->begin(); j != i->end(); j++)
            if(fabs(j->x - 180) < 2e-4) j->x = 180;
            else if(fabs(j->x + 180) < 2e-4) j->x = -180;

        // eliminiate parallel segments
        contour_pt l = *i->rbegin();
        poly_contour::iterator j = i->begin(), k = j;
        k++;
        while(k != i->end()) {
            if(fabs(cross(vector(*j, l), vector(*j, *k))) < 1e-12)
                i->erase(j);
            else
                l = *j;
            j = k;
            k++;
        }

        // erase zero contours
        if(i->size() < 3)
            i = contours.erase(i);
        else
            i++;
    }
}
