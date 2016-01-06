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

#ifndef _LLREGION_H_
#define _LLREGION_H_

#include <list>

#ifdef __WXMSW__
    #include "GL/gl.h"            // local copy for Windows
    #include "GL/glu.h"
#else
    #ifndef __OCPN__ANDROID__
        #include <GL/gl.h>
        #include <GL/glu.h>
        #include <GL/glext.h>
    #else
        #include <qopengl.h>
        #include <GL/gl_private.h>              // this is a cut-down version of gl.h
    #endif
#endif

#include "bbox.h"
#include "cutil.h"

struct contour_pt
{
    double y, x;
};

// ----------------------------------------------------------------------------
// LLRegion
// ----------------------------------------------------------------------------

typedef std::list<contour_pt> poly_contour;
class LLBBox;

struct work;
class LLRegion
{
public:
    LLRegion() {}
    LLRegion( float minlat, float minlon, float maxlat, float maxlon);
    LLRegion( const LLBBox& llbbox );
    LLRegion( size_t n, const float *points );
    LLRegion( size_t n, const double *points );

    static bool PointsCCW( size_t n, const double *points );
    
    void Print() const;
    void plot(const char*fn) const;
    
    LLBBox GetBox() const;
    bool IntersectOut(const LLBBox &box) const;
    
    bool Contains(float lat, float lon) const;
   
    void Clear() { contours.clear(); }
    bool Empty() const { return contours.empty(); }
    
    void Intersect(const LLRegion& region);
    void Union(const LLRegion& region);
    void Subtract(const LLRegion& region);

    void Reduce(double factor);

    std::list<poly_contour> contours;

private:
    bool NoIntersection(const LLBBox& box) const;
    bool NoIntersection(const LLRegion& region) const;
    void PutContours(work &w, const LLRegion& region, bool reverse=false);
    void Put(const LLRegion& region, int winding_rule, bool reverse=false);
    void Combine(const LLRegion& region);
    void InitBox( float minlat, float minlon, float maxlat, float maxlon);
    void InitPoints( size_t n, const double *points );
    void AdjustLongitude();
    void Optimize();

    LLBBox m_box;
};

#endif
// _LLREGION_H_
