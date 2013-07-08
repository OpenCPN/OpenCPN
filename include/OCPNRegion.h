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

#ifdef __WXOSX__
#define USE_NEW_REGION
#endif


typedef enum
{
    GDK_EVEN_ODD_RULE,
    GDK_WINDING_RULE
} GdkFillRule;

typedef enum
{
    GDK_OVERLAP_RECTANGLE_IN,
    GDK_OVERLAP_RECTANGLE_OUT,
    GDK_OVERLAP_RECTANGLE_PART
} GdkOverlapType;

#define EMPTY_REGION(pReg) pReg->numRects = 0
#define REGION_NOT_EMPTY(pReg) pReg->numRects


typedef struct _GdkPoint          GdkPoint;
struct _GdkPoint
{
    int x;
    int y;
};



typedef struct _GdkRectangle          GdkRectangle;
struct _GdkRectangle
{
    int x;
    int y;
    int width;
    int height;
};

#define gboolean bool;

typedef struct _GdkSegment            GdkSegment;
struct _GdkSegment
{
    int x1;
    int y1;
    int x2;
    int y2;
};

typedef GdkSegment GdkRegionBox;


typedef struct _GdkRegion             GdkRegion;
struct _GdkRegion
{
    long size;
    long numRects;
    GdkRegionBox *rects;
    GdkRegionBox extents;
};

/*
* number of points to buffer before sending them off
* to scanlines() :  Must be an even number
*/
#define NUMPTSTOBUFFER 200

/*
 * used to allocate buffers for points and link
 * the buffers together
 */
typedef struct _POINTBLOCK {
    GdkPoint pts[NUMPTSTOBUFFER];
    struct _POINTBLOCK *next;
} POINTBLOCK;

#define INBOX(r, x, y) \
( ( ((r).x2 >  x)) && \
( ((r).x1 <= x)) && \
( ((r).y2 >  y)) && \
( ((r).y1 <= y)) )


/*  1 if two BOXs overlap.
 *  0 if two BOXs do not overlap.
 *  Remember, x2 and y2 are not in the region 
 */
#define EXTENTCHECK(r1, r2) \
((r1)->x2 > (r2)->x1 && \
(r1)->x1 < (r2)->x2 && \
(r1)->y2 > (r2)->y1 && \
(r1)->y1 < (r2)->y2)



#define _G_NEW(struct_type, n_structs, func) \
    ((struct_type *) malloc ((n_structs), sizeof (struct_type)))
#define _G_RENEW(struct_type, mem, n_structs, func) \
    ((struct_type *) realloc (mem, (n_structs), sizeof (struct_type)))

#define g_new(struct_type, n_structs)                   _G_NEW (struct_type, n_structs, malloc)
#define g_renew(struct_type, mem, n_structs)            _G_RENEW (struct_type, mem, n_structs, realloc)



#define GROWREGION(reg, nRects) {                                          \
if ((nRects) == 0) {                                             \
    if ((reg)->rects != &(reg)->extents) {                         \
        free ((reg)->rects);                                       \
        (reg)->rects = &(reg)->extents;                              \
    }                                                              \
}                                                                \
else if ((reg)->rects == &(reg)->extents) {                      \
            (reg)->rects = (GdkRegionBox *)malloc(nRects * sizeof(GdkRegionBox));    \
            (reg)->rects[0] = (reg)->extents;                              \
}                                                                \
else                                                             \
   (reg)->rects = (GdkRegionBox *)realloc((reg)->rects, sizeof(GdkRegionBox) * nRects); \
   (reg)->size = (nRects);                                          \
}                                 
                
/*
*   Check to see if there is enough memory in the present region.
*/
#define MEMCHECK(reg, rect, firstrect){                                          \
      if ((reg)->numRects >= ((reg)->size - 1)) {                              \
        GROWREGION(reg,2*(reg)->size);                                         \
        (rect) = &(firstrect)[(reg)->numRects];                                \
      }                                                                       \
}
                    
#ifndef MIN
#define MIN(a,b) wxMin(a,b)
#endif

#ifndef MAX
#define MAX(a,b) wxMax(a,b)
#endif


/*
 *  In scan converting polygons, we want to choose those pixels
 *  which are inside the polygon.  Thus, we add .5 to the starting
 *  x coordinate for both left and right edges.  Now we choose the
 *  first pixel which is inside the pgon for the left edge and the
 *  first pixel which is outside the pgon for the right edge.
 *  Draw the left pixel, but not the right.
 *
 *  How to add .5 to the starting x coordinate:
 *      If the edge is moving to the right, then subtract dy from the
 *  error term from the general form of the algorithm.
 *      If the edge is moving to the left, then add dy to the error term.
 *
 *  The reason for the difference between edges moving to the left
 *  and edges moving to the right is simple:  If an edge is moving
 *  to the right, then we want the algorithm to flip immediately.
 *  If it is moving to the left, then we don't want it to flip until
 *  we traverse an entire pixel.
 */
#define BRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
int dx;      /* local storage */ \
\
/* \
 *  if the edge is horizontal, then it is ignored \
 *  and assumed not to be processed.  Otherwise, do this stuff. \
 */ \
 if ((dy) != 0) { \
     xStart = (x1); \
     dx = (x2) - xStart; \
     if (dx < 0) { \
         m = dx / (dy); \
         m1 = m - 1; \
         incr1 = -2 * dx + 2 * (dy) * m1; \
         incr2 = -2 * dx + 2 * (dy) * m; \
         d = 2 * m * (dy) - 2 * dx - 2 * (dy); \
         } else { \
             m = dx / (dy); \
             m1 = m + 1; \
             incr1 = 2 * dx - 2 * (dy) * m1; \
             incr2 = 2 * dx - 2 * (dy) * m; \
             d = -2 * m * (dy) + 2 * dx; \
             } \
             } \
             }
             
#define BRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
             if (m1 > 0) { \
                 if (d > 0) { \
                     minval += m1; \
                     d += incr1; \
                 } \
                 else { \
                     minval += m; \
                     d += incr2; \
                 } \
             } else {\
                 if (d >= 0) { \
                     minval += m1; \
                     d += incr1; \
                 } \
                 else { \
                     minval += m; \
                     d += incr2; \
                 } \
             } \
         }
             
             /*
              *     This structure contains all of the information needed
              *     to run the bresenham algorithm.
              *     The variables may be hardcoded into the declarations
              *     instead of using this structure to make use of
              *     register declarations.
              */
             typedef struct {
                 int minor_axis;     /* minor axis        */
                 int d;              /* decision variable */
                 int m, m1;          /* slope and slope+1 */
                 int incr1, incr2;   /* error increments */
             } BRESINFO;
             
#define BRESINITPGONSTRUCT(dmaj, min1, min2, bres) \
BRESINITPGON(dmaj, min1, min2, bres.minor_axis, bres.d, \
bres.m, bres.m1, bres.incr1, bres.incr2)

#define BRESINCRPGONSTRUCT(bres) \
BRESINCRPGON(bres.d, bres.minor_axis, bres.m, bres.m1, bres.incr1, bres.incr2)


/*
 * for the winding number rule
 */
#define CLOCKWISE          1
#define COUNTERCLOCKWISE  -1 

typedef struct _EdgeTableEntry {
    int ymax;             /* ycoord at which we exit this edge. */
    BRESINFO bres;        /* Bresenham info to run the edge     */
    struct _EdgeTableEntry *next;       /* next in the list     */
    struct _EdgeTableEntry *back;       /* for insertion sort   */
    struct _EdgeTableEntry *nextWETE;   /* for winding num rule */
    int ClockWise;        /* flag for winding number rule       */
} EdgeTableEntry;


typedef struct _ScanLineList{
    int scanline;              /* the scanline represented */
    EdgeTableEntry *edgelist;  /* header node              */
    struct _ScanLineList *next;  /* next in the list       */
} ScanLineList;


typedef struct {
    int ymax;                 /* ymax for the polygon     */
    int ymin;                 /* ymin for the polygon     */
    ScanLineList scanlines;   /* header node              */
} EdgeTable;

/*
 * Here is a struct to help with storage allocation
 * so we can allocate a big chunk at a time, and then take
 * pieces from this heap when we need to.
 */
#define SLLSPERBLOCK 25

typedef struct _ScanLineListBlock {
    ScanLineList SLLs[SLLSPERBLOCK];
    struct _ScanLineListBlock *next;
} ScanLineListBlock;


/*
*
*     a few macros for the inner loops of the fill code where
*     performance considerations don't allow a procedure call.
*
*     Evaluate the given edge at the given scanline.
*     If the edge has expired, then we leave it and fix up
*     the active edge table; otherwise, we increment the
*     x value to be ready for the next scanline.
*     The winding number rule is in effect, so we must notify
*     the caller when the edge has been removed so he
*     can reorder the Winding Active Edge Table.
*/
#define EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
if (pAET->ymax == y) {          /* leaving this edge */ \
    pPrevAET->next = pAET->next; \
    pAET = pPrevAET->next; \
    fixWAET = 1; \
    if (pAET) \
        pAET->back = pPrevAET; \
        } \
        else { \
            BRESINCRPGONSTRUCT(pAET->bres); \
            pPrevAET = pAET; \
            pAET = pAET->next; \
        } \
        }
        
        
        /*
         *     Evaluate the given edge at the given scanline.
         *     If the edge has expired, then we leave it and fix up
         *     the active edge table; otherwise, we increment the
         *     x value to be ready for the next scanline.
         *     The even-odd rule is in effect.
         */
#define EVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
        if (pAET->ymax == y) {          /* leaving this edge */ \
            pPrevAET->next = pAET->next; \
            pAET = pPrevAET->next; \
            if (pAET) \
                pAET->back = pPrevAET; \
        } \
        else { \
            BRESINCRPGONSTRUCT(pAET->bres); \
            pPrevAET = pAET; \
            pAET = pAET->next; \
        } \
        }
        


// ----------------------------------------------------------------------------
// OCPNRegion
// ----------------------------------------------------------------------------

class OCPNRegion : public wxRegion 
{
public:
    OCPNRegion() { }

    OCPNRegion( wxCoord x, wxCoord y, wxCoord w, wxCoord h );
    OCPNRegion( const wxPoint& topLeft, const wxPoint& bottomRight );
    OCPNRegion( const wxRect& rect );
    OCPNRegion( size_t n, const wxPoint *points, int fillStyle = wxODDEVEN_RULE );
    
    virtual ~OCPNRegion(){}
//    void InitRect(wxCoord x, wxCoord y, wxCoord w, wxCoord h);
    
    wxRegion &ConvertTowxRegion();
    
    
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
    OCPNRegion( GdkRegion *region );

    GdkRegion *GetRegion() const;

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
    virtual bool DoXor(const OCPNRegion& region);
    using wxRegion::DoXor;
    

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

    void Reset() { m_current = 0u; }
    void Reset(const OCPNRegion& region);

    bool HaveRects() const;
    void NextRect(void);
    wxRect GetRect() const;

private:
    void Init();
    void CreateRects( const OCPNRegion& r );

    size_t   m_current;
    OCPNRegion m_region;

    wxRect *m_rects;
    size_t  m_numRects;

    wxRegionIterator *m_ri;
    
};


#endif
// _OCPN_REGION_H_
