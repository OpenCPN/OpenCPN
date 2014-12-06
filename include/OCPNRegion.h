/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Portions Copyright (C) 2010 by David S. Register                      *
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

/////////////////////////////////////////////////////////////////////////////
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _OCPN_REGION_H_
#define _OCPN_REGION_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

//#if defined(__WXOSX__)
#define USE_NEW_REGION
//#endif


// ----------------------------------------------------------------------------
// OCPNRegion
// ----------------------------------------------------------------------------

class OCPNRegion : public
#ifdef USE_NEW_REGION
 wxObject
#else
 wxRegion 
#endif
{
public:
    OCPNRegion() { }

    OCPNRegion( wxCoord x, wxCoord y, wxCoord w, wxCoord h );
    OCPNRegion( const wxPoint& topLeft, const wxPoint& bottomRight );
    OCPNRegion( const wxRect& rect );
    OCPNRegion( const wxRegion& region );
    OCPNRegion( size_t n, const wxPoint *points, int fillStyle = wxODDEVEN_RULE );
    
    virtual ~OCPNRegion();
    
    wxRegion &ConvertTowxRegion();
    wxRegion *GetNew_wxRegion();
    
    
#ifdef USE_NEW_REGION    

    // common part of ctors for a rectangle region
    void InitRect(wxCoord x, wxCoord y, wxCoord w, wxCoord h);
 
     // operators
    // ---------
    bool operator==(const OCPNRegion& region) const { return ODoIsEqual(region); }
    bool operator!=(const OCPNRegion& region) const { return !(*this == region); }
    
    bool IsOk() const { return m_refData != NULL; }
    bool Ok() const { return IsOk(); }
    
    // Get the bounding box
    bool GetBox(wxCoord& x, wxCoord& y, wxCoord& w, wxCoord& h) const
    { return ODoGetBox(x, y, w, h); }
    wxRect GetBox() const
    {
        wxCoord x, y, w, h;
        return ODoGetBox(x, y, w, h) ? wxRect(x, y, w, h) : wxRect();
    }

    // Test if the given point or rectangle is inside this region
    wxRegionContain Contains(wxCoord x, wxCoord y) const
    { return ODoContainsPoint(x, y); }
    wxRegionContain Contains(const wxPoint& pt) const
    { return ODoContainsPoint(pt.x, pt.y); }
    wxRegionContain Contains(wxCoord x, wxCoord y, wxCoord w, wxCoord h) const
    { return ODoContainsRect(wxRect(x, y, w, h)); }
    wxRegionContain Contains(const wxRect& rect) const
    { return ODoContainsRect(rect); }
    
 // Is region equal (i.e. covers the same area as another one)?
 bool IsEqual(const OCPNRegion& region) const;
 
    // OCPNRegionBase methods
    virtual void Clear();
    virtual bool IsEmpty() const;
    bool Empty() const { return IsEmpty(); }
    
public:
//    OCPNRegion( OGdkRegion *region );

    void *GetRegion() const;

    bool Offset(wxCoord x, wxCoord y)   { return ODoOffset(x, y); }
    bool Offset(const wxPoint& pt)      { return ODoOffset(pt.x, pt.y); }
    bool Intersect(const OCPNRegion& region) { return ODoIntersect(region); }
    bool Union(const OCPNRegion& region) { return ODoUnionWithRegion(region); }
            bool Union(wxCoord x, wxCoord y, wxCoord w, wxCoord h) { return ODoUnionWithRect(wxRect(x, y, w, h)); }
    bool Union(const wxRect& rect) { return ODoUnionWithRect(rect); }
    bool Subtract(const OCPNRegion& region) { return ODoSubtract(region); }
    
protected:
    // ref counting code
    virtual wxObjectRefData *CreateRefData() const;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    // wxRegionBase pure virtuals
    virtual bool ODoIsEqual(const OCPNRegion& region) const;
    virtual bool ODoGetBox(wxCoord& x, wxCoord& y, wxCoord& w, wxCoord& h) const;
    virtual wxRegionContain ODoContainsPoint(wxCoord x, wxCoord y) const;
    virtual wxRegionContain ODoContainsRect(const wxRect& rect) const;

    virtual bool ODoOffset(wxCoord x, wxCoord y);
    virtual bool ODoUnionWithRect(const wxRect& rect);
    virtual bool ODoUnionWithRegion(const OCPNRegion& region);
    virtual bool ODoIntersect(const OCPNRegion& region);
    virtual bool ODoSubtract(const OCPNRegion& region);
//    virtual bool DoXor(const OCPNRegion& region);
    

#endif
    
private:
    DECLARE_DYNAMIC_CLASS(OCPNRegion)
};

// ----------------------------------------------------------------------------
// OCPNRegionIterator: decomposes a region into rectangles
// ----------------------------------------------------------------------------

class  OCPNRegionIterator 
{
public:
    OCPNRegionIterator();
    OCPNRegionIterator(const OCPNRegion& region);
    virtual ~OCPNRegionIterator();

    void Reset();
    void Reset(const OCPNRegion& region);

    bool HaveRects() const;
    void NextRect(void);
    wxRect GetRect() const;

private:
#ifdef USE_NEW_REGION
    void Init();
    void CreateRects( const OCPNRegion& r );

    size_t   m_current;
    OCPNRegion m_region;

    wxRect *m_rects;
    size_t  m_numRects;
#else
    wxRegionIterator *m_ri;
#endif
};


#endif
// _OCPN_REGION_H_
