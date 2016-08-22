/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Portins Copyright (C) 2010 by David S. Register                       *
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
// Name:        src/gtk/region.cpp
// Purpose:
// Author:      Robert Roebling
// Modified:    VZ at 05.10.00: use AllocExclusive(), comparison fixed
// Id:          $Id: region.cpp 42903 2006-11-01 12:56:38Z RR $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/region.h"
#include "OCPNRegion.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif


typedef enum
{
    OGDK_EVEN_ODD_RULE,
    OGDK_WINDING_RULE
} OGdkFillRule;

typedef enum
{
    OGDK_OVERLAP_RECTANGLE_IN,
    OGDK_OVERLAP_RECTANGLE_OUT,
    OGDK_OVERLAP_RECTANGLE_PART
} OGdkOverlapType;

#define EMPTY_REGION(pReg) pReg->numRects = 0
#define REGION_NOT_EMPTY(pReg) pReg->numRects


typedef struct _OGdkPoint          OGdkPoint;
struct _OGdkPoint
{
    int x;
    int y;
};



typedef struct _OGdkRectangle          OGdkRectangle;
struct _OGdkRectangle
{
    int x;
    int y;
    int width;
    int height;
};

//#define gboolean bool;

typedef struct _OGdkSegment            OGdkSegment;
struct _OGdkSegment
{
    int x1;
    int y1;
    int x2;
    int y2;
};

typedef OGdkSegment OGdkRegionBox;


typedef struct _OGdkRegion             OGdkRegion;
struct _OGdkRegion
{
    long size;
    long numRects;
    OGdkRegionBox *rects;
    OGdkRegionBox extents;
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
typedef struct _OPOINTBLOCK {
    OGdkPoint pts[NUMPTSTOBUFFER];
    struct _OPOINTBLOCK *next;
} OPOINTBLOCK;

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


/*
 * #define _OG_NEW(struct_type, n_structs, func) \
 *    ((struct_type *) malloc ((n_structs), sizeof (struct_type)))
 * #define _OG_RENEW(struct_type, mem, n_structs, func) \
 *    ((struct_type *) realloc (mem, (n_structs), sizeof (struct_type)))
 * 
 * #define og_new(struct_type, n_structs)                   _OG_NEW (struct_type, n_structs, malloc)
 * #define og_renew(struct_type, mem, n_structs)            _OG_RENEW (struct_type, mem, n_structs, realloc)
 */


#define OGROWREGION(reg, nRects) {                                          \
if ((nRects) == 0) {                                             \
    if ((reg)->rects != &(reg)->extents) {                         \
        free ((reg)->rects);                                       \
        (reg)->rects = &(reg)->extents;                              \
        }                                                              \
        }                                                                \
        else if ((reg)->rects == &(reg)->extents) {                      \
            (reg)->rects = (OGdkRegionBox *)malloc(nRects * sizeof(OGdkRegionBox));    \
            (reg)->rects[0] = (reg)->extents;                              \
            }                                                                \
            else                                                             \
                (reg)->rects = (OGdkRegionBox *)realloc((reg)->rects, sizeof(OGdkRegionBox) * nRects); \
                (reg)->size = (nRects);                                          \
                }                                 
                
                /*
                 *   Check to see if there is enough memory in the present region.
                 */
                #define OMEMCHECK(reg, rect, firstrect){                                          \
                if ((reg)->numRects >= ((reg)->size - 1)) {                              \
                    OGROWREGION(reg,2*(reg)->size);                                         \
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
                    #define OBRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
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
                    
                    #define OBRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
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
                    } OBRESINFO;
                    
                    #define OBRESINITPGONSTRUCT(dmaj, min1, min2, bres) \
                    OBRESINITPGON(dmaj, min1, min2, bres.minor_axis, bres.d, \
                    bres.m, bres.m1, bres.incr1, bres.incr2)
                    
                    #define OBRESINCRPGONSTRUCT(bres) \
                    OBRESINCRPGON(bres.d, bres.minor_axis, bres.m, bres.m1, bres.incr1, bres.incr2)
                    
                    
                    /*
                     * for the winding number rule
                     */
                    #define CLOCKWISE          1
                    #define COUNTERCLOCKWISE  -1 
                    
                    typedef struct _OEdgeTableEntry {
                        int ymax;             /* ycoord at which we exit this edge. */
                        OBRESINFO bres;        /* Bresenham info to run the edge     */
                        struct _OEdgeTableEntry *next;       /* next in the list     */
                        struct _OEdgeTableEntry *back;       /* for insertion sort   */
                        struct _OEdgeTableEntry *nextWETE;   /* for winding num rule */
                        int ClockWise;        /* flag for winding number rule       */
                    } OEdgeTableEntry;
                    
                    
                    typedef struct _OScanLineList{
                        int scanline;              /* the scanline represented */
                        OEdgeTableEntry *edgelist;  /* header node              */
                        struct _OScanLineList *next;  /* next in the list       */
                    } OScanLineList;
                    
                    
                    typedef struct {
                        int ymax;                 /* ymax for the polygon     */
                        int ymin;                 /* ymin for the polygon     */
                        OScanLineList scanlines;   /* header node              */
                    } OEdgeTable;
                    
                    /*
                     * Here is a struct to help with storage allocation
                     * so we can allocate a big chunk at a time, and then take
                     * pieces from this heap when we need to.
                     */
                    #define SLLSPERBLOCK 25
                    
                    typedef struct _OScanLineListBlock {
                        OScanLineList SLLs[SLLSPERBLOCK];
                        struct _OScanLineListBlock *next;
                    } OScanLineListBlock;
                    
                    
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
                    #define OEVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
                    if (pAET->ymax == y) {          /* leaving this edge */ \
                        pPrevAET->next = pAET->next; \
                        pAET = pPrevAET->next; \
                        fixWAET = 1; \
                        if (pAET) \
                            pAET->back = pPrevAET; \
                    } \
                    else { \
                        OBRESINCRPGONSTRUCT(pAET->bres); \
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
                    #define OEVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
                    if (pAET->ymax == y) {          /* leaving this edge */ \
                        pPrevAET->next = pAET->next; \
                        pAET = pPrevAET->next; \
                        if (pAET) \
                            pAET->back = pPrevAET; \
                    } \
                    else { \
                        OBRESINCRPGONSTRUCT(pAET->bres); \
                        pPrevAET = pAET; \
                        pAET = pAET->next; \
                    } \
                    }
                    
                    
                    

OGdkRegion    * gdk_region_copy            (const OGdkRegion    *region);
void           gdk_region_destroy         (OGdkRegion          *region);
OGdkRegion    * gdk_region_rectangle       (const OGdkRectangle *rectangle);
bool           ogdk_region_equal           (const OGdkRegion    *region1,
                                           const OGdkRegion    *region2);
bool           gdk_region_point_in        (const OGdkRegion    *region,
                                           int                 x,
                                           int                 y);
OGdkOverlapType gdk_region_rect_in         (const OGdkRegion    *region,
                                           const OGdkRectangle *rectangle);
void           gdk_region_offset          (OGdkRegion          *region,
                                           int                dx,
                                           int                dy);
void           gdk_region_union_with_rect (OGdkRegion          *region,
                                               const OGdkRectangle *rect);
void           gdk_region_union           (OGdkRegion          *source1,
                                           const OGdkRegion    *source2);
void           gdk_region_intersect       (OGdkRegion          *source1,
                                           const OGdkRegion    *source2);
OGdkRegion    * gdk_region_polygon         (const OGdkPoint     *points,
                                           int                n_points,
                                           OGdkFillRule         fill_rule);

OGdkRegion    * gdk_region_new (void);
void           gdk_region_subtract        (OGdkRegion       *source1,
                                           const OGdkRegion *source2);
bool           gdk_region_empty           (const OGdkRegion *region);

void           gdk_region_get_rectangles  (const OGdkRegion  *region,
                                           OGdkRectangle    **rectangles,
                                           int             *n_rectangles);
void           gdk_region_get_clipbox      (const OGdkRegion *region,
                                            OGdkRectangle    *rectangle);

// ----------------------------------------------------------------------------
// OCPNRegionRefData: private class containing the information about the region
// ----------------------------------------------------------------------------

class OCPNRegionRefData : public wxObjectRefData
{
public:
    OCPNRegionRefData()
    {
        m_region = NULL;
    }

    OCPNRegionRefData(const OCPNRegionRefData& refData)
        : wxObjectRefData()
    {
        m_region = gdk_region_copy(refData.m_region);
    }

    virtual ~OCPNRegionRefData()
    {
        if (m_region)
            gdk_region_destroy( m_region );
        free( m_region );
    }

    OGdkRegion  *m_region;
};

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

#define M_REGIONDATA ((OCPNRegionRefData *)m_refData)
#define M_REGIONDATA_OF(rgn) ((OCPNRegionRefData *)(rgn.m_refData))

IMPLEMENT_DYNAMIC_CLASS(OCPNRegion, wxGDIObject)

// ----------------------------------------------------------------------------
// OCPNRegion construction
// ----------------------------------------------------------------------------

#define M_REGIONDATA ((OCPNRegionRefData *)m_refData)

#ifndef USE_NEW_REGION

OCPNRegion::OCPNRegion( wxCoord x, wxCoord y, wxCoord w, wxCoord h ) : wxRegion(x,y,w,h)

{
}

OCPNRegion::OCPNRegion( const wxPoint& topLeft, const wxPoint& bottomRight ) : wxRegion(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y) 
{
}

OCPNRegion::OCPNRegion( const wxRect& rect ) : wxRegion(rect.x, rect.y, rect.width, rect.height)
{
}

OCPNRegion::OCPNRegion( const wxRegion& region ) : wxRegion(region)
{
}

OCPNRegion::OCPNRegion( size_t n, const wxPoint *points, int fillStyle )
    : wxRegion(n, points,
#if wxCHECK_VERSION(2,9,0)
               (wxPolygonFillMode)
#endif
               fillStyle)
{
}

wxRegion *OCPNRegion::GetNew_wxRegion() const
{
    return new wxRegion(this);
}

#endif    

#ifdef USE_NEW_REGION

OCPNRegion::OCPNRegion( wxCoord x, wxCoord y, wxCoord w, wxCoord h )
{
    InitRect(x, y, w, h);
}

OCPNRegion::OCPNRegion( const wxPoint& topLeft, const wxPoint& bottomRight )
{
    InitRect(topLeft.x, topLeft.y,
             bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);
}

OCPNRegion::OCPNRegion( const wxRect& rect )
{
    InitRect(rect.x, rect.y, rect.width, rect.height);
}

OCPNRegion::OCPNRegion( const wxRegion& region )
{
    wxRegionIterator ri(region);
    if(!ri.HaveRects())
        return;

    wxRect rect = ri.GetRect();
    InitRect(rect.x, rect.y, rect.width, rect.height);
    ri++;

    while(ri.HaveRects()) {
        Union(ri.GetRect());
        ri++;
    }
}

OCPNRegion::~OCPNRegion()
{
}


void OCPNRegion::InitRect(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
{
    OGdkRectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = w;
    rect.height = h;

    m_refData = new OCPNRegionRefData();

    M_REGIONDATA->m_region = gdk_region_rectangle( &rect );
}

//OCPNRegion::OCPNRegion( GdkRegion *region )
//{
//    m_refData = new OCPNRegionRefData();
//    M_REGIONDATA->m_region = gdk_region_copy( region );
//}

OCPNRegion::OCPNRegion( size_t n, const wxPoint *points, int fillStyle )
{
    OGdkPoint *gdkpoints = new OGdkPoint[n];
    for ( size_t i = 0 ; i < n ; i++ )
    {
        gdkpoints[i].x = points[i].x;
        gdkpoints[i].y = points[i].y;
    }

    m_refData = new OCPNRegionRefData();

    OGdkRegion* reg = gdk_region_polygon
                     (
                        gdkpoints,
                        n,
                        fillStyle == wxWINDING_RULE ? OGDK_WINDING_RULE
                                                    : OGDK_EVEN_ODD_RULE
                     );

    M_REGIONDATA->m_region = reg;

    delete [] gdkpoints;
}

//OCPNRegion::~OCPNRegion()
//{
    // m_refData unrefed in ~wxObject
//}

wxObjectRefData *OCPNRegion::CreateRefData() const
{
    return new OCPNRegionRefData;
}

wxObjectRefData *OCPNRegion::CloneRefData(const wxObjectRefData *data) const
{
   return new OCPNRegionRefData(*(OCPNRegionRefData *)data);
}

// ----------------------------------------------------------------------------
// OCPNRegion comparison
// ----------------------------------------------------------------------------

bool OCPNRegion::ODoIsEqual(const OCPNRegion& region) const
{
    OGdkRegion *a = ((OCPNRegionRefData *)m_refData)->m_region;
    
    if(!region.m_refData)
        return false;
    
    OGdkRegion *b = ((OCPNRegionRefData *)(region.m_refData))->m_region;
    return ogdk_region_equal(M_REGIONDATA->m_region, M_REGIONDATA_OF(region)->m_region);
}

// ----------------------------------------------------------------------------
// OCPNRegion operations
// ----------------------------------------------------------------------------

void OCPNRegion::Clear()
{
    UnRef();
}

bool OCPNRegion::ODoUnionWithRect(const wxRect& r)
{
    // workaround for a strange GTK/X11 bug: taking union with an empty
    // rectangle results in an empty region which is definitely not what we
    // want
    if ( r.IsEmpty() )
        return true;

    if ( !m_refData )
    {
        InitRect(r.x, r.y, r.width, r.height);
    }
    else
    {
        AllocExclusive();

        OGdkRectangle rect;
        rect.x = r.x;
        rect.y = r.y;
        rect.width = r.width;
        rect.height = r.height;

        gdk_region_union_with_rect( M_REGIONDATA->m_region, &rect );
    }

    return true;
}

bool OCPNRegion::ODoUnionWithRegion( const OCPNRegion& region )
{
    wxCHECK_MSG( region.Ok(), false, _T("invalid region") );

    if (!m_refData)
    {
        m_refData = new OCPNRegionRefData();
        M_REGIONDATA->m_region = gdk_region_new();
    }
    else
    {
        AllocExclusive();
    }

    gdk_region_union( M_REGIONDATA->m_region, (OGdkRegion *)region.GetRegion() );

    return true;
}

bool OCPNRegion::ODoIntersect( const OCPNRegion& region )
{
    wxCHECK_MSG( region.Ok(), false, _T("invalid region") );

    if (!m_refData)
    {
        // intersecting with invalid region doesn't make sense
        return false;
    }

    AllocExclusive();

    gdk_region_intersect( M_REGIONDATA->m_region, (OGdkRegion *)region.GetRegion() );

    return true;
}

bool OCPNRegion::ODoSubtract( const OCPNRegion& region )
{
    wxCHECK_MSG( region.Ok(), false, _T("invalid region") );
    if (!m_refData)
    {
        // subtracting from an invalid region doesn't make sense
        return false;
    }

    AllocExclusive();

    gdk_region_subtract( M_REGIONDATA->m_region, (OGdkRegion *)region.GetRegion() );

    return true;
}

#if 0
bool OCPNRegion::DoXor( const OCPNRegion& region )
{
    wxCHECK_MSG( region.Ok(), false, _T("invalid region") );

    if (!m_refData)
    {
        return false;
    }

    AllocExclusive();

    ///    gdk_region_xor( M_REGIONDATA->m_region, (OGdkRegion *)region.GetRegion() );

    return true;
}
#endif

bool OCPNRegion::ODoOffset( wxCoord x, wxCoord y )
{
    if (!m_refData)
        return false;

    AllocExclusive();

    gdk_region_offset( M_REGIONDATA->m_region, x, y );

    return true;
}

// ----------------------------------------------------------------------------
// OCPNRegion tests
// ----------------------------------------------------------------------------

bool OCPNRegion::ODoGetBox( wxCoord &x, wxCoord &y, wxCoord &w, wxCoord &h ) const
{
    if ( m_refData )
    {
        OGdkRectangle rect;
        gdk_region_get_clipbox( M_REGIONDATA->m_region, &rect );
        x = rect.x;
        y = rect.y;
        w = rect.width;
        h = rect.height;

        return true;
    }
    else
    {
        x = 0;
        y = 0;
        w = -1;
        h = -1;

        return false;
    }
}

bool OCPNRegion::IsEmpty() const
{
    if (!m_refData)
        return true;

    return gdk_region_empty( M_REGIONDATA->m_region );
}

wxRegionContain OCPNRegion::ODoContainsPoint( wxCoord x, wxCoord y ) const
{
    if (!m_refData)
        return wxOutRegion;

    if (gdk_region_point_in( M_REGIONDATA->m_region, x, y ))
        return wxInRegion;
    else
        return wxOutRegion;
}

wxRegionContain OCPNRegion::ODoContainsRect(const wxRect& r) const
{
    
    if (!m_refData)
        return wxOutRegion;

    OGdkRectangle rect;
    rect.x = r.x;
    rect.y = r.y;
    rect.width = r.width;
    rect.height = r.height;
    OGdkOverlapType res = gdk_region_rect_in( M_REGIONDATA->m_region, &rect );
    switch (res)
    {
        case OGDK_OVERLAP_RECTANGLE_IN:   return wxInRegion;
        case OGDK_OVERLAP_RECTANGLE_OUT:  return wxOutRegion;
        case OGDK_OVERLAP_RECTANGLE_PART: return wxPartRegion;
    }
    
    return wxOutRegion;
}


void *OCPNRegion::GetRegion() const
{
    if (!m_refData)
        return  NULL;

    return M_REGIONDATA->m_region;
}

wxRegion *OCPNRegion::GetNew_wxRegion() const
{
    wxRegion *r = new wxRegion;
    r->Clear();
    
    OGdkRectangle *gdkrects = NULL;
    int numRects = 0;
    gdk_region_get_rectangles( (OGdkRegion *)GetRegion(), &gdkrects, &numRects );
    
    if (numRects)
    {
        for (int i=0; i < numRects; ++i)
        {
            OGdkRectangle &gr = gdkrects[i];
            
            wxRect wr;
            wr.x = gr.x;
            wr.y = gr.y;
            wr.width = gr.width;
            wr.height = gr.height;
            
            r->Union(wr);
        }
    }
    free( gdkrects );
    
    return r;
}


#endif
// ----------------------------------------------------------------------------
// OCPNRegionIterator
// ----------------------------------------------------------------------------

#ifndef USE_NEW_REGION

OCPNRegionIterator::OCPNRegionIterator( const OCPNRegion& region )
{
    m_ri = new wxRegionIterator(region);
}

OCPNRegionIterator::~OCPNRegionIterator()
{
    delete m_ri;
}

void OCPNRegionIterator::Reset()
{
    m_ri->Reset();
}

void OCPNRegionIterator::Reset(const OCPNRegion& region)
{
    m_ri->Reset(region);
}

wxRect OCPNRegionIterator::GetRect() const
{
    return m_ri->GetRect();
}

bool OCPNRegionIterator::HaveRects() const
{
    return m_ri->HaveRects();
}

void OCPNRegionIterator::NextRect()
{
    ++(*m_ri);
}


#endif


#ifdef USE_NEW_REGION

OCPNRegionIterator::OCPNRegionIterator()
{
    Init();
    Reset();
}

OCPNRegionIterator::OCPNRegionIterator( const OCPNRegion& region )
{
    Init();
    Reset(region);
}

void OCPNRegionIterator::Init()
{
    m_rects = NULL;
    m_numRects = 0;
}

OCPNRegionIterator::~OCPNRegionIterator()
{
    wxDELETEA(m_rects);
}

void OCPNRegionIterator::Reset()
{
    m_current = 0u;
}

void OCPNRegionIterator::NextRect()
{
    if (HaveRects())
        ++m_current;
}

void OCPNRegionIterator::CreateRects( const OCPNRegion& region )
{
    wxDELETEA(m_rects);
    m_numRects = 0;

    OGdkRegion *gdkregion = (OGdkRegion *)region.GetRegion();
    if (!gdkregion)
        return;

    OGdkRectangle *gdkrects = NULL;
    int numRects = 0;
    gdk_region_get_rectangles( gdkregion, &gdkrects, &numRects );

    m_numRects = numRects;
    if (numRects)
    {
        m_rects = new wxRect[m_numRects];
        for (size_t i=0; i < m_numRects; ++i)
        {
            OGdkRectangle &gr = gdkrects[i];
            wxRect &wr = m_rects[i];
            wr.x = gr.x;
            wr.y = gr.y;
            wr.width = gr.width;
            wr.height = gr.height;
        }
    }
    free( gdkrects );
}

void OCPNRegionIterator::Reset( const OCPNRegion& region )
{
    m_region = region;
    CreateRects(region);
    Reset();
}

bool OCPNRegionIterator::HaveRects() const
{
    return m_current < m_numRects;
}


wxRect OCPNRegionIterator::GetRect() const
{
    wxRect r;
    if( HaveRects() )
        r = m_rects[m_current];

    return r;
}

#endif

#ifdef USE_NEW_REGION

/* $TOG: Region.c /main/31 1998/02/06 17:50:22 kaleb $ */
/************************************************************************
 * 
 * Copyright 1987, 1988, 1998  The Open Group
 * 
 * All Rights Reserved.
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of The Open Group shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from The Open Group.
 * 
 * 
 * Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                        All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * 
 ************************************************************************/
/* $XFree86: xc/lib/X11/Region.c,v 1.5 1999/05/09 10:50:01 dawes Exp $ */
/*
 * The functions in this file implement the Region abstraction, similar to one
 * used in the X11 sample server. A Region is simply an area, as the name
 * implies, and is implemented as a "y-x-banded" array of rectangles. To
 * explain: Each Region is made up of a certain number of rectangles sorted
 * by y coordinate first, and then by x coordinate.
 *
 * Furthermore, the rectangles are banded such that every rectangle with a
 * given upper-left y coordinate (y1) will have the same lower-right y
 * coordinate (y2) and vice versa. If a rectangle has scanlines in a band, it
 * will span the entire vertical distance of the band. This means that some
 * areas that could be merged into a taller rectangle will be represented as
 * several shorter rectangles to account for shorter rectangles to its left
 * or right but within its "vertical scope".
 *
 * An added constraint on the rectangles is that they must cover as much
 * horizontal area as possible. E.g. no two rectangles in a band are allowed
 * to touch.
 *
 * Whenever possible, bands will be merged together to cover a greater vertical
 * distance (and thus reduce the number of rectangles). Two bands can be merged
 * only if the bottom of one touches the top of the other and they have
 * rectangles in the same places (of the same width, of course). This maintains
 * the y-x-banding that's so nice to have...
 */

//#include "config.h"
//#include <stdlib.h>
//#include <string.h>
//#include <gdkregion.h>
//#include "gdkregion-generic.h"
//#include "gdkalias.h"

typedef void (* overlapFunc)    (OGdkRegion    *pReg,
                                 OGdkRegionBox *r1,
                                 OGdkRegionBox *r1End,
                                 OGdkRegionBox *r2,
                                 OGdkRegionBox *r2End,
                                 int          y1,
                                 int          y2);
typedef void (* nonOverlapFunc) (OGdkRegion    *pReg,
                                 OGdkRegionBox *r,
                                 OGdkRegionBox *rEnd,
                                 int          y1,
                                 int          y2);

static void miRegionCopy (OGdkRegion       *dstrgn,
                          const OGdkRegion *rgn);
static void miRegionOp   (OGdkRegion       *newReg,
                          OGdkRegion       *reg1,
                          const OGdkRegion *reg2,
                          overlapFunc      overlapFn,
                          nonOverlapFunc   nonOverlap1Fn,
                          nonOverlapFunc   nonOverlap2Fn);

/**
 * gdk_region_new:
 *
 * Creates a new empty #GdkRegion.
 *
 * Returns: a new empty #GdkRegion
 */
OGdkRegion *
gdk_region_new (void)
{
    OGdkRegion *temp;
    
    temp = (OGdkRegion *) malloc(sizeof(OGdkRegion));
    
    temp->numRects = 0;
    temp->rects = &temp->extents;
    temp->extents.x1 = 0;
    temp->extents.y1 = 0;
    temp->extents.x2 = 0;
    temp->extents.y2 = 0;
    temp->size = 1;
    
    return temp;
}

/**
 * gdk_region_rectangle:
 * @rectangle: a #GdkRectangle
 * 
 * Creates a new region containing the area @rectangle.
 * 
 * Return value: a new region
 **/
OGdkRegion *
gdk_region_rectangle (const OGdkRectangle *rectangle)
{
    OGdkRegion *temp;
    
///    g_return_val_if_fail (rectangle != NULL, NULL);
    
    if (rectangle->width <= 0 || rectangle->height <= 0)
        return gdk_region_new();
    
    temp = gdk_region_new(); ///    temp = g_slice_new (GdkRegion);
    
    temp->numRects = 1;
    temp->rects = &temp->extents;
    temp->extents.x1 = rectangle->x;
    temp->extents.y1 = rectangle->y;
    temp->extents.x2 = rectangle->x + rectangle->width;
    temp->extents.y2 = rectangle->y + rectangle->height;
    temp->size = 1;
    
    return temp;
}

/**
 * gdk_region_copy:
 * @region: a #GdkRegion
 * 
 * Copies @region, creating an identical new region.
 * 
 * Return value: a new region identical to @region
 **/
OGdkRegion *
gdk_region_copy (const OGdkRegion *region)
{
    OGdkRegion *temp;
    
 ///   g_return_val_if_fail (region != NULL, NULL);
    
    temp = gdk_region_new ();
    
    miRegionCopy (temp, region);
    
    return temp;
}

/**
 * gdk_region_get_clipbox:
 * @region: a #GdkRegion
 * @rectangle: return location for the clipbox
 *
 * Obtains the smallest rectangle which includes the entire #GdkRegion.
 *
 */
void
gdk_region_get_clipbox (const OGdkRegion *region,
                        OGdkRectangle    *rectangle)
{
///    g_return_if_fail (region != NULL);
///    g_return_if_fail (rectangle != NULL);
    
    rectangle->x = region->extents.x1;
    rectangle->y = region->extents.y1;
    rectangle->width = region->extents.x2 - region->extents.x1;
    rectangle->height = region->extents.y2 - region->extents.y1;
}


/**
 * gdk_region_get_rectangles:
 * @region: a #GdkRegion
 * @rectangles: return location for an array of rectangles
 * @n_rectangles: length of returned array
 *
 * Obtains the area covered by the region as a list of rectangles.
 * The array returned in @rectangles must be freed with g_free().
 **/
void
gdk_region_get_rectangles (const OGdkRegion  *region,
                           OGdkRectangle    **rectangles,
                           int             *n_rectangles)
{
    int i;
    
 ///   g_return_if_fail (region != NULL);
 ///   g_return_if_fail (rectangles != NULL);
 ///   g_return_if_fail (n_rectangles != NULL);
    
    *n_rectangles = region->numRects;
    *rectangles = (OGdkRectangle *)malloc (sizeof(OGdkRectangle) * region->numRects);
    
    for (i = 0; i < region->numRects; i++)
    {
        OGdkRegionBox rect;
        rect = region->rects[i];
        (*rectangles)[i].x = rect.x1;
        (*rectangles)[i].y = rect.y1;
        (*rectangles)[i].width = rect.x2 - rect.x1;
        (*rectangles)[i].height = rect.y2 - rect.y1;
    }
}

/**
 * gdk_region_union_with_rect:
 * @region: a #GdkRegion.
 * @rect: a #GdkRectangle.
 * 
 * Sets the area of @region to the union of the areas of @region and
 * @rect. The resulting area is the set of pixels contained in
 * either @region or @rect.
 **/
void
gdk_region_union_with_rect (OGdkRegion          *region,
                            const OGdkRectangle *rect)
{
    OGdkRegion tmp_region;
    
//    g_return_if_fail (region != NULL);
 //   g_return_if_fail (rect != NULL);
    
    if (rect->width <= 0 || rect->height <= 0)
        return;
    
    tmp_region.rects = &tmp_region.extents;
    tmp_region.numRects = 1;
    tmp_region.extents.x1 = rect->x;
    tmp_region.extents.y1 = rect->y;
    tmp_region.extents.x2 = rect->x + rect->width;
    tmp_region.extents.y2 = rect->y + rect->height;
    tmp_region.size = 1;
    
    gdk_region_union (region, &tmp_region);
}

/*-
 * -----------------------------------------------------------------------
 * miSetExtents --
 *      Reset the extents of a region to what they should be. Called by
 *      miSubtract and miIntersect b/c they can't figure it out along the
 *      way or do so easily, as miUnion can.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      The region's 'extents' structure is overwritten.
 *
 *-----------------------------------------------------------------------
 */
static void
miSetExtents (OGdkRegion *pReg)
{
    OGdkRegionBox *pBox, *pBoxEnd, *pExtents;
    
    if (pReg->numRects == 0)
    {
        pReg->extents.x1 = 0;
        pReg->extents.y1 = 0;
        pReg->extents.x2 = 0;
        pReg->extents.y2 = 0;
        return;
    }
    
    pExtents = &pReg->extents;
    pBox = pReg->rects;
    pBoxEnd = &pBox[pReg->numRects - 1];
    
    /*
     * Since pBox is the first rectangle in the region, it must have the
     * smallest y1 and since pBoxEnd is the last rectangle in the region,
     * it must have the largest y2, because of banding. Initialize x1 and
     * x2 from  pBox and pBoxEnd, resp., as good things to initialize them
     * to...
     */
    pExtents->x1 = pBox->x1;
    pExtents->y1 = pBox->y1;
    pExtents->x2 = pBoxEnd->x2;
    pExtents->y2 = pBoxEnd->y2;
    
///    g_assert(pExtents->y1 < pExtents->y2);
    while (pBox <= pBoxEnd)
    {
        if (pBox->x1 < pExtents->x1)
        {
            pExtents->x1 = pBox->x1;
        }
        if (pBox->x2 > pExtents->x2)
        {
            pExtents->x2 = pBox->x2;
        }
        pBox++;
    }
///    g_assert(pExtents->x1 < pExtents->x2);
}

/**
 * gdk_region_destroy:
 * @region: a #GdkRegion
 *
 * Destroys a #GdkRegion.
 */
void
gdk_region_destroy (OGdkRegion *region)
{
///    g_return_if_fail (region != NULL);
    
    if (region->rects != &region->extents)
        free (region->rects);
 ///   g_slice_free (GdkRegion, region);
}


/**
 * gdk_region_offset:
 * @region: a #GdkRegion
 * @dx: the distance to move the region horizontally
 * @dy: the distance to move the region vertically
 *
 * Moves a region the specified distance.
 */
void
gdk_region_offset (OGdkRegion *region,
                   int       x,
                   int       y)
{
    int nbox;
    OGdkRegionBox *pbox;
    
///    g_return_if_fail (region != NULL);
    
    pbox = region->rects;
    nbox = region->numRects;
    
    while(nbox--)
    {
        pbox->x1 += x;
        pbox->x2 += x;
        pbox->y1 += y;
        pbox->y2 += y;
        pbox++;
    }
    if (region->rects != &region->extents)
    {
        region->extents.x1 += x;
        region->extents.x2 += x;
        region->extents.y1 += y;
        region->extents.y2 += y;
    }
}

/* 
 *   Utility procedure Compress:
 *   Replace r by the region r', where 
 *     p in r' iff (Quantifer m <= dx) (p + m in r), and
 *     Quantifier is Exists if grow is TRUE, For all if grow is FALSE, and
 *     (x,y) + m = (x+m,y) if xdir is TRUE; (x,y+m) if xdir is FALSE.
 * 
 *   Thus, if xdir is TRUE and grow is FALSE, r is replaced by the region
 *   of all points p such that p and the next dx points on the same
 *   horizontal scan line are all in r.  We do this using by noting
 *   that p is the head of a run of length 2^i + k iff p is the head
 *   of a run of length 2^i and p+2^i is the head of a run of length
 *   k. Thus, the loop invariant: s contains the region corresponding
 *   to the runs of length shift.  r contains the region corresponding
 *   to the runs of length 1 + dxo & (shift-1), where dxo is the original
 *   value of dx.  dx = dxo & ~(shift-1).  As parameters, s and t are
 *   scratch regions, so that we don't have to allocate them on every
 *   call.
 */

#define ZOpRegion(a,b) if (grow) gdk_region_union (a, b); \
else gdk_region_intersect (a,b)
    #define ZShiftRegion(a,b) if (xdir) gdk_region_offset (a,b,0); \
    else gdk_region_offset (a,0,b)
        
        static void
        Compress(OGdkRegion *r,
                 OGdkRegion *s,
                 OGdkRegion *t,
                 unsigned int dx,
                 int        xdir,
                 int        grow)
        {
            unsigned int shift = 1;
            
            miRegionCopy (s, r);
            while (dx)
            {
                if (dx & shift)
                {
                    ZShiftRegion(r, -(int)shift);
                    ZOpRegion(r, s);
                    dx -= shift;
                    if (!dx) break;
                }
                miRegionCopy (t, s);
                ZShiftRegion(s, -(int)shift);
                ZOpRegion(s, t);
                shift <<= 1;
            }
        }
        
        #undef ZOpRegion
        #undef ZShiftRegion
        #undef ZCopyRegion
        
        /**
         * gdk_region_shrink:
         * @region: a #GdkRegion
         * @dx: the number of pixels to shrink the region horizontally
         * @dy: the number of pixels to shrink the region vertically
         *
         * Resizes a region by the specified amount.
         * Positive values shrink the region. Negative values expand it.
         */
        void
        gdk_region_shrink (OGdkRegion *region,
                           int        dx,
                           int        dy)
        {
            OGdkRegion *s, *t;
            int grow;
            
///            g_return_if_fail (region != NULL);
            
            if (!dx && !dy)
                return;
            
            s = gdk_region_new ();
            t = gdk_region_new ();
            
            grow = (dx < 0);
            if (grow)
                dx = -dx;
            if (dx)
                Compress(region, s, t, (unsigned) 2*dx, TRUE, grow);
            
            grow = (dy < 0);
            if (grow)
                dy = -dy;
            if (dy)
                Compress(region, s, t, (unsigned) 2*dy, FALSE, grow);
            
            gdk_region_offset (region, dx, dy);
            gdk_region_destroy (s);
            gdk_region_destroy (t);
        }
        
        
        /*======================================================================
         *          Region Intersection
         *====================================================================*/
        /*-
         * -----------------------------------------------------------------------
         * miIntersectO --
         *      Handle an overlapping band for miIntersect.
         *
         * Results:
         *      None.
         *
         * Side Effects:
         *      Rectangles may be added to the region.
         *
         *-----------------------------------------------------------------------
         */
        /* static void*/
        static void
        miIntersectO (OGdkRegion    *pReg,
                      OGdkRegionBox *r1,
                      OGdkRegionBox *r1End,
                      OGdkRegionBox *r2,
                      OGdkRegionBox *r2End,
                      int          y1,
                      int          y2)
        {
            int   x1;
            int   x2;
            OGdkRegionBox *pNextRect;
            
            pNextRect = &pReg->rects[pReg->numRects];
            
            while ((r1 != r1End) && (r2 != r2End))
            {
                x1 = MAX (r1->x1,r2->x1);
                x2 = MIN (r1->x2,r2->x2);
                
                /*
                 * If there's any overlap between the two rectangles, add that
                 * overlap to the new region.
                 * There's no need to check for subsumption because the only way
                 * such a need could arise is if some region has two rectangles
                 * right next to each other. Since that should never happen...
                 */
                if (x1 < x2)
                {
///                    g_assert (y1<y2);
                    
                    OMEMCHECK (pReg, pNextRect, pReg->rects);
                    pNextRect->x1 = x1;
                    pNextRect->y1 = y1;
                    pNextRect->x2 = x2;
                    pNextRect->y2 = y2;
                    pReg->numRects += 1;
                    pNextRect++;
 //                   g_assert (pReg->numRects <= pReg->size);
                }
                
                /*
                 * Need to advance the pointers. Shift the one that extends
                 * to the right the least, since the other still has a chance to
                 * overlap with that region's next rectangle, if you see what I mean.
                 */
                if (r1->x2 < r2->x2)
                {
                    r1++;
                }
                else if (r2->x2 < r1->x2)
                {
                    r2++;
                }
                else
                {
                    r1++;
                    r2++;
                }
            }
        }
        
        /**
         * gdk_region_intersect:
         * @source1: a #GdkRegion
         * @source2: another #GdkRegion
         *
         * Sets the area of @source1 to the intersection of the areas of @source1
         * and @source2. The resulting area is the set of pixels contained in
         * both @source1 and @source2.
         **/
        void
        gdk_region_intersect (OGdkRegion       *source1,
                              const OGdkRegion *source2)
        {
///            g_return_if_fail (source1 != NULL);
///            g_return_if_fail (source2 != NULL);
            
            /* check for trivial reject */
            if ((!(source1->numRects)) || (!(source2->numRects))  ||
                (!EXTENTCHECK(&source1->extents, &source2->extents)))
                source1->numRects = 0;
            else
                miRegionOp (source1, source1, source2,
                            miIntersectO, (nonOverlapFunc) NULL, (nonOverlapFunc) NULL);
                
            /*
             * Can't alter source1's extents before miRegionOp depends on the
             * extents of the regions being unchanged. Besides, this way there's
             * no checking against rectangles that will be nuked due to
             * coalescing, so we have to examine fewer rectangles.
             */
            miSetExtents(source1);
        }
        
        static void
        miRegionCopy (OGdkRegion       *dstrgn,
                      const OGdkRegion *rgn)
        {
            if (dstrgn != rgn) /*  don't want to copy to itself */
            {  
                if (dstrgn->size < rgn->numRects)
                {
                    if (dstrgn->rects != &dstrgn->extents)
                        free (dstrgn->rects);
                    
                    dstrgn->rects = (OGdkRegionBox *)malloc(sizeof(OGdkRegionBox) * rgn->numRects);
                    dstrgn->size = rgn->numRects;
                }
                
                dstrgn->numRects = rgn->numRects;
                dstrgn->extents = rgn->extents;
                
                memcpy (dstrgn->rects, rgn->rects, rgn->numRects * sizeof (OGdkRegionBox));
            }
        }
        
        
        /*======================================================================
         *          Generic Region Operator
         *====================================================================*/
        
        /*-
         * -----------------------------------------------------------------------
         * miCoalesce --
         *      Attempt to merge the boxes in the current band with those in the
         *      previous one. Used only by miRegionOp.
         *
         * Results:
         *      The new index for the previous band.
         *
         * Side Effects:
         *      If coalescing takes place:
         *          - rectangles in the previous band will have their y2 fields
         *            altered.
         *          - pReg->numRects will be decreased.
         *
         *-----------------------------------------------------------------------
         */
        /* static int*/
        static int
        miCoalesce (OGdkRegion *pReg,         /* Region to coalesce */
                    int       prevStart,    /* Index of start of previous band */
                    int       curStart)     /* Index of start of current band */
        {
            OGdkRegionBox *pPrevBox;       /* Current box in previous band */
            OGdkRegionBox *pCurBox;        /* Current box in current band */
            OGdkRegionBox *pRegEnd;        /* End of region */
            int           curNumRects;    /* Number of rectangles in current
            * band */
            int           prevNumRects;   /* Number of rectangles in previous
            * band */
            int           bandY1;         /* Y1 coordinate for current band */
            
            pRegEnd = &pReg->rects[pReg->numRects];
            
            pPrevBox = &pReg->rects[prevStart];
            prevNumRects = curStart - prevStart;
            
            /*
             * Figure out how many rectangles are in the current band. Have to do
             * this because multiple bands could have been added in miRegionOp
             * at the end when one region has been exhausted.
             */
            pCurBox = &pReg->rects[curStart];
            bandY1 = pCurBox->y1;
            for (curNumRects = 0;
                 (pCurBox != pRegEnd) && (pCurBox->y1 == bandY1);
            curNumRects++)
                 {
                     pCurBox++;
                 }
                 
                 if (pCurBox != pRegEnd)
                 {
                     /*
                      * If more than one band was added, we have to find the start
                      * of the last band added so the next coalescing job can start
                      * at the right place... (given when multiple bands are added,
                      * this may be pointless -- see above).
                      */
                     pRegEnd--;
                     while (pRegEnd[-1].y1 == pRegEnd->y1)
                     {
                         pRegEnd--;
                     }
                     curStart = pRegEnd - pReg->rects;
                     pRegEnd = pReg->rects + pReg->numRects;
                 }
                 
                 if ((curNumRects == prevNumRects) && (curNumRects != 0)) {
                     pCurBox -= curNumRects;
                     /*
                      * The bands may only be coalesced if the bottom of the previous
                      * matches the top scanline of the current.
                      */
                     if (pPrevBox->y2 == pCurBox->y1)
                     {
                         /*
                          * Make sure the bands have boxes in the same places. This
                          * assumes that boxes have been added in such a way that they
                          * cover the most area possible. I.e. two boxes in a band must
                          * have some horizontal space between them.
                          */
                         do
                         {
                             if ((pPrevBox->x1 != pCurBox->x1) ||
                                 (pPrevBox->x2 != pCurBox->x2))
                             {
                                 /*
                                  * The bands don't line up so they can't be coalesced.
                                  */
                                 return (curStart);
                             }
                             pPrevBox++;
                             pCurBox++;
                             prevNumRects -= 1;
                         } while (prevNumRects != 0);
                         
                         pReg->numRects -= curNumRects;
                         pCurBox -= curNumRects;
                         pPrevBox -= curNumRects;
                         
                         /*
                          * The bands may be merged, so set the bottom y of each box
                          * in the previous band to that of the corresponding box in
                          * the current band.
                          */
                         do
                         {
                             pPrevBox->y2 = pCurBox->y2;
                             pPrevBox++;
                             pCurBox++;
                             curNumRects -= 1;
                         }
                         while (curNumRects != 0);
                
                /*
                 * If only one band was added to the region, we have to backup
                 * curStart to the start of the previous band.
                 *
                 * If more than one band was added to the region, copy the
                 * other bands down. The assumption here is that the other bands
                 * came from the same region as the current one and no further
                 * coalescing can be done on them since it's all been done
                 * already... curStart is already in the right place.
                 */
                if (pCurBox == pRegEnd)
                {
                    curStart = prevStart;
                }
                else
                {
                    do
                    {
                        *pPrevBox++ = *pCurBox++;
                    }
                    while (pCurBox != pRegEnd);
                }
                
                     }
                 }
                 return curStart;
        }
        
        /*-
         * -----------------------------------------------------------------------
         * miRegionOp --
         *      Apply an operation to two regions. Called by miUnion, miInverse,
         *      miSubtract, miIntersect...
         *
         * Results:
         *      None.
         *
         * Side Effects:
         *      The new region is overwritten.
         *
         * Notes:
         *      The idea behind this function is to view the two regions as sets.
         *      Together they cover a rectangle of area that this function divides
         *      into horizontal bands where points are covered only by one region
         *      or by both. For the first case, the nonOverlapFunc is called with
         *      each the band and the band's upper and lower extents. For the
         *      second, the overlapFunc is called to process the entire band. It
         *      is responsible for clipping the rectangles in the band, though
         *      this function provides the boundaries.
         *      At the end of each band, the new region is coalesced, if possible,
         *      to reduce the number of rectangles in the region.
         *
         *-----------------------------------------------------------------------
         */
        /* static void*/
        static void
        miRegionOp(OGdkRegion       *newReg,
                   OGdkRegion       *reg1,
                   const OGdkRegion *reg2,
                   overlapFunc      overlapFn,          /* Function to call for over-
                   * lapping bands */
                   nonOverlapFunc   nonOverlap1Fn,      /* Function to call for non-
                   * overlapping bands in region
                   * 1 */
                   nonOverlapFunc   nonOverlap2Fn)      /* Function to call for non-
                   * overlapping bands in region
                   * 2 */
                   {
                       OGdkRegionBox *r1;                   /* Pointer into first region */
                       OGdkRegionBox *r2;                   /* Pointer into 2d region */
                       OGdkRegionBox *r1End;                /* End of 1st region */
                       OGdkRegionBox *r2End;                /* End of 2d region */
                       int           ybot;                 /* Bottom of intersection */
                       int           ytop;                 /* Top of intersection */
                       OGdkRegionBox *oldRects;             /* Old rects for newReg */
                       int           prevBand;             /* Index of start of
                       * previous band in newReg */
                       int           curBand;              /* Index of start of current
                       * band in newReg */
                       OGdkRegionBox *r1BandEnd;            /* End of current band in r1 */
                       OGdkRegionBox *r2BandEnd;            /* End of current band in r2 */
                       int           top;                  /* Top of non-overlapping
                       * band */
                       int           bot;                  /* Bottom of non-overlapping
                       * band */
                       
                       /*
                        * Initialization:
                        *  set r1, r2, r1End and r2End appropriately, preserve the important
                        * parts of the destination region until the end in case it's one of
                        * the two source regions, then mark the "new" region empty, allocating
                        * another array of rectangles for it to use.
                        */
                       r1 = reg1->rects;
                       r2 = reg2->rects;
                       r1End = r1 + reg1->numRects;
                       r2End = r2 + reg2->numRects;
                       
                       oldRects = newReg->rects;
                       
                       EMPTY_REGION(newReg);
                       
                       /*
                        * Allocate a reasonable number of rectangles for the new region. The idea
                        * is to allocate enough so the individual functions don't need to
                        * reallocate and copy the array, which is time consuming, yet we don't
                        * have to worry about using too much memory. I hope to be able to
                        * nuke the Xrealloc() at the end of this function eventually.
                        */
                       newReg->size = MAX (reg1->numRects, reg2->numRects) * 2;
                       newReg->rects = (OGdkRegionBox *)malloc(sizeof(OGdkRegionBox) * newReg->size);
                       
                       /*
                        * Initialize ybot and ytop.
                        * In the upcoming loop, ybot and ytop serve different functions depending
                        * on whether the band being handled is an overlapping or non-overlapping
                        * band.
                        *  In the case of a non-overlapping band (only one of the regions
                        * has points in the band), ybot is the bottom of the most recent
                        * intersection and thus clips the top of the rectangles in that band.
                        * ytop is the top of the next intersection between the two regions and
                        * serves to clip the bottom of the rectangles in the current band.
                        *  For an overlapping band (where the two regions intersect), ytop clips
                        * the top of the rectangles of both regions and ybot clips the bottoms.
                        */
                       if (reg1->extents.y1 < reg2->extents.y1)
                           ybot = reg1->extents.y1;
                       else
                           ybot = reg2->extents.y1;
                       
                       /*
                        * prevBand serves to mark the start of the previous band so rectangles
                        * can be coalesced into larger rectangles. qv. miCoalesce, above.
                        * In the beginning, there is no previous band, so prevBand == curBand
                        * (curBand is set later on, of course, but the first band will always
                        * start at index 0). prevBand and curBand must be indices because of
                        * the possible expansion, and resultant moving, of the new region's
                        * array of rectangles.
                        */
                       prevBand = 0;
                       
                       do
                       {
                           curBand = newReg->numRects;
                           
                           /*
                            * This algorithm proceeds one source-band (as opposed to a
                            * destination band, which is determined by where the two regions
                            * intersect) at a time. r1BandEnd and r2BandEnd serve to mark the
                            * rectangle after the last one in the current band for their
                            * respective regions.
                            */
                           r1BandEnd = r1;
                           while ((r1BandEnd != r1End) && (r1BandEnd->y1 == r1->y1))
                           {
                               r1BandEnd++;
                           }
                           
                           r2BandEnd = r2;
                           while ((r2BandEnd != r2End) && (r2BandEnd->y1 == r2->y1))
                           {
                               r2BandEnd++;
                           }
                           
                           /*
                            * First handle the band that doesn't intersect, if any.
                            *
                            * Note that attention is restricted to one band in the
                            * non-intersecting region at once, so if a region has n
                            * bands between the current position and the next place it overlaps
                            * the other, this entire loop will be passed through n times.
                            */
                           if (r1->y1 < r2->y1)
                           {
                               top = MAX (r1->y1,ybot);
                               bot = MIN (r1->y2,r2->y1);
                               
                               if ((top != bot) && (nonOverlap1Fn != (void (*)(OGdkRegion *, OGdkRegionBox *, OGdkRegionBox *, int,int))NULL))
                               {
                                   (* nonOverlap1Fn) (newReg, r1, r1BandEnd, top, bot);
                               }
                               
                               ytop = r2->y1;
                           }
                           else if (r2->y1 < r1->y1)
                           {
                               top = MAX (r2->y1,ybot);
                               bot = MIN (r2->y2,r1->y1);
                               
                               if ((top != bot) && (nonOverlap2Fn != (void (*)(OGdkRegion *, OGdkRegionBox *, OGdkRegionBox *, int,int))NULL))
                               {
                                   (* nonOverlap2Fn) (newReg, r2, r2BandEnd, top, bot);
                               }
                               
                               ytop = r1->y1;
                           }
                           else
                           {
                               ytop = r1->y1;
                           }
                           
                           /*
                            * If any rectangles got added to the region, try and coalesce them
                            * with rectangles from the previous band. Note we could just do
                            * this test in miCoalesce, but some machines incur a not
                            * inconsiderable cost for function calls, so...
                            */
                           if (newReg->numRects != curBand)
                           {
                               prevBand = miCoalesce (newReg, prevBand, curBand);
                           }
                           
                           /*
                            * Now see if we've hit an intersecting band. The two bands only
                            * intersect if ybot > ytop
                            */
                           ybot = MIN (r1->y2, r2->y2);
                           curBand = newReg->numRects;
                           if (ybot > ytop)
                           {
                               (* overlapFn) (newReg, r1, r1BandEnd, r2, r2BandEnd, ytop, ybot);
                               
                           }
                           
                           if (newReg->numRects != curBand)
                           {
                               prevBand = miCoalesce (newReg, prevBand, curBand);
                           }
                           
                           /*
                            * If we've finished with a band (y2 == ybot) we skip forward
                            * in the region to the next band.
                            */
                           if (r1->y2 == ybot)
                           {
                               r1 = r1BandEnd;
                           }
                           if (r2->y2 == ybot)
                           {
                               r2 = r2BandEnd;
                           }
                       } while ((r1 != r1End) && (r2 != r2End));
                       
                       /*
                        * Deal with whichever region still has rectangles left.
                        */
                       curBand = newReg->numRects;
                       if (r1 != r1End)
                       {
                           if (nonOverlap1Fn != (nonOverlapFunc )NULL)
                           {
                               do
                               {
                                   r1BandEnd = r1;
                                   while ((r1BandEnd < r1End) && (r1BandEnd->y1 == r1->y1))
                                   {
                                       r1BandEnd++;
                                   }
                                   (* nonOverlap1Fn) (newReg, r1, r1BandEnd,
                                                      MAX (r1->y1,ybot), r1->y2);
                                   r1 = r1BandEnd;
                               } while (r1 != r1End);
                           }
                       }
                       else if ((r2 != r2End) && (nonOverlap2Fn != (nonOverlapFunc) NULL))
                       {
                           do
                           {
                               r2BandEnd = r2;
                               while ((r2BandEnd < r2End) && (r2BandEnd->y1 == r2->y1))
                               {
                                   r2BandEnd++;
                               }
                               (* nonOverlap2Fn) (newReg, r2, r2BandEnd,
                                                  MAX (r2->y1,ybot), r2->y2);
                               r2 = r2BandEnd;
                           } while (r2 != r2End);
                       }
                       
                       if (newReg->numRects != curBand)
                       {
                           (void) miCoalesce (newReg, prevBand, curBand);
                       }
                       
                       /*
                        * A bit of cleanup. To keep regions from growing without bound,
                        * we shrink the array of rectangles to match the new number of
                        * rectangles in the region. This never goes to 0, however...
                        *
                        * Only do this stuff if the number of rectangles allocated is more than
                        * twice the number of rectangles in the region (a simple optimization...).
                        */
                       if (newReg->numRects < (newReg->size >> 1))
                       {
                           if (REGION_NOT_EMPTY (newReg))
                           {
                               newReg->size = newReg->numRects;
//                               newReg->rects = g_renew (GdkRegionBox, newReg->rects, newReg->size);
                               newReg->rects = (OGdkRegionBox *)realloc(newReg->rects, sizeof(OGdkRegionBox) * newReg->size);
                           }
                           else
                           {
                               /*
                                * No point in doing the extra work involved in an Xrealloc if
                                * the region is empty
                                */
                               newReg->size = 1;
                               free (newReg->rects);
                               newReg->rects = &newReg->extents;
                           }
                       }
                       
                       if (oldRects != &newReg->extents)
                           free (oldRects);
                   }
                   
                   
                   /*======================================================================
                    *          Region Union
                    *====================================================================*/
                   
                   /*-
                    * -----------------------------------------------------------------------
                    * miUnionNonO --
                    *      Handle a non-overlapping band for the union operation. Just
                    *      Adds the rectangles into the region. Doesn't have to check for
                    *      subsumption or anything.
                    *
                    * Results:
                    *      None.
                    *
                    * Side Effects:
                    *      pReg->numRects is incremented and the final rectangles overwritten
                    *      with the rectangles we're passed.
                    *
                    *-----------------------------------------------------------------------
                    */
                   static void
                   miUnionNonO (OGdkRegion    *pReg,
                                OGdkRegionBox *r,
                                OGdkRegionBox *rEnd,
                                int          y1,
                                int          y2)
                   {
                       OGdkRegionBox *pNextRect;
                       
                       pNextRect = &pReg->rects[pReg->numRects];
                       
 ///                      g_assert(y1 < y2);
                       
                       while (r != rEnd)
                       {
///                           g_assert(r->x1 < r->x2);
                           OMEMCHECK(pReg, pNextRect, pReg->rects);
                           pNextRect->x1 = r->x1;
                           pNextRect->y1 = y1;
                           pNextRect->x2 = r->x2;
                           pNextRect->y2 = y2;
                           pReg->numRects += 1;
                           pNextRect++;
                           
///                           g_assert(pReg->numRects<=pReg->size);
                           r++;
                       }
                   }
                   
                   
                   /*-
                    * -----------------------------------------------------------------------
                    * miUnionO --
                    *      Handle an overlapping band for the union operation. Picks the
                    *      left-most rectangle each time and merges it into the region.
                    *
                    * Results:
                    *      None.
                    *
                    * Side Effects:
                    *      Rectangles are overwritten in pReg->rects and pReg->numRects will
                    *      be changed.
                    *
                    *-----------------------------------------------------------------------
                    */
                   
                   /* static void*/
                   static void
                   miUnionO (OGdkRegion *pReg,
                             OGdkRegionBox *r1,
                             OGdkRegionBox *r1End,
                             OGdkRegionBox *r2,
                             OGdkRegionBox *r2End,
                             int          y1,
                             int          y2)
                   {
                       OGdkRegionBox *        pNextRect;
                       
                       pNextRect = &pReg->rects[pReg->numRects];
                       
                       #define MERGERECT(r)                                    \
                       if ((pReg->numRects != 0) &&                        \
                           (pNextRect[-1].y1 == y1) &&                     \
                           (pNextRect[-1].y2 == y2) &&                     \
                           (pNextRect[-1].x2 >= r->x1))                    \
                           {                                                 \
                           if (pNextRect[-1].x2 < r->x2)                   \
                           {                                             \
                           pNextRect[-1].x2 = r->x2;                   \
                           /*g_assert(pNextRect[-1].x1<pNextRect[-1].x2);*/        \
                   }                                             \
                   }                                                 \
                   else                                                \
                   {                                                 \
                   OMEMCHECK(pReg, pNextRect, pReg->rects);         \
                   pNextRect->y1 = y1;                             \
                   pNextRect->y2 = y2;                             \
                   pNextRect->x1 = r->x1;                          \
                   pNextRect->x2 = r->x2;                          \
                   pReg->numRects += 1;                            \
                   pNextRect += 1;                                 \
                   }                                                 \
                   /*g_assert(pReg->numRects<=pReg->size);*/                       \
                   r++;
                   
///                   g_assert (y1<y2);
                   while ((r1 != r1End) && (r2 != r2End))
                   {
                       if (r1->x1 < r2->x1)
                       {
                           MERGERECT(r1);
                       }
                       else
                       {
                           MERGERECT(r2);
                       }
                   }
                   
                   if (r1 != r1End)
                   {
                       do
                       {
                           MERGERECT(r1);
                       } while (r1 != r1End);
                   }
                   else while (r2 != r2End)
                   {
                       MERGERECT(r2);
                   }
                   }
                   
                   /**
                    * gdk_region_union:
                    * @source1:  a #GdkRegion
                    * @source2: a #GdkRegion 
                    * 
                    * Sets the area of @source1 to the union of the areas of @source1 and
                    * @source2. The resulting area is the set of pixels contained in
                    * either @source1 or @source2.
                    **/
                   void
                   gdk_region_union (OGdkRegion       *source1,
                                     const OGdkRegion *source2)
                   {
///                       g_return_if_fail (source1 != NULL);
///                       g_return_if_fail (source2 != NULL);
                       
                       /*  checks all the simple cases */
                       
                       /*
                        * source1 and source2 are the same or source2 is empty
                        */
                       if ((source1 == source2) || (!(source2->numRects)))
                           return;
                       
                       /* 
                        * source1 is empty
                        */
                       if (!(source1->numRects))
                       {
                           miRegionCopy (source1, source2);
                           return;
                       }
                       
                       /*
                        * source1 completely subsumes source2
                        */
                       if ((source1->numRects == 1) && 
                           (source1->extents.x1 <= source2->extents.x1) &&
                           (source1->extents.y1 <= source2->extents.y1) &&
                           (source1->extents.x2 >= source2->extents.x2) &&
                           (source1->extents.y2 >= source2->extents.y2))
                           return;
                       
                       /*
                        * source2 completely subsumes source1
                        */
                       if ((source2->numRects == 1) && 
                           (source2->extents.x1 <= source1->extents.x1) &&
                           (source2->extents.y1 <= source1->extents.y1) &&
                           (source2->extents.x2 >= source1->extents.x2) &&
                           (source2->extents.y2 >= source1->extents.y2))
                       {
                           miRegionCopy(source1, source2);
                           return;
                       }
                       
                       miRegionOp (source1, source1, source2, miUnionO, 
                                   miUnionNonO, miUnionNonO);
                       
                       source1->extents.x1 = MIN (source1->extents.x1, source2->extents.x1);
                       source1->extents.y1 = MIN (source1->extents.y1, source2->extents.y1);
                       source1->extents.x2 = MAX (source1->extents.x2, source2->extents.x2);
                       source1->extents.y2 = MAX (source1->extents.y2, source2->extents.y2);
                   }
                   
                   
                   /*======================================================================
                    *                Region Subtraction
                    *====================================================================*/
                   
                   /*-
                    * -----------------------------------------------------------------------
                    * miSubtractNonO --
                    *      Deal with non-overlapping band for subtraction. Any parts from
                    *      region 2 we discard. Anything from region 1 we add to the region.
                    *
                    * Results:
                    *      None.
                    *
                    * Side Effects:
                    *      pReg may be affected.
                    *
                    *-----------------------------------------------------------------------
                    */
                   /* static void*/
                   static void
                   miSubtractNonO1 (OGdkRegion    *pReg,
                                    OGdkRegionBox *r,
                                    OGdkRegionBox *rEnd,
                                    int          y1,
                                    int          y2)
                   {
                       OGdkRegionBox *        pNextRect;
                       
                       pNextRect = &pReg->rects[pReg->numRects];
                       
///                       g_assert(y1<y2);
                       
                       while (r != rEnd)
                       {
///                           g_assert (r->x1<r->x2);
                           OMEMCHECK (pReg, pNextRect, pReg->rects);
                           pNextRect->x1 = r->x1;
                           pNextRect->y1 = y1;
                           pNextRect->x2 = r->x2;
                           pNextRect->y2 = y2;
                           pReg->numRects += 1;
                           pNextRect++;
                           
///                           g_assert (pReg->numRects <= pReg->size);
                           
                           r++;
                       }
                   }
                   
                   /*-
                    * -----------------------------------------------------------------------
                    * miSubtractO --
                    *      Overlapping band subtraction. x1 is the left-most point not yet
                    *      checked.
                    *
                    * Results:
                    *      None.
                    *
                    * Side Effects:
                    *      pReg may have rectangles added to it.
                    *
                    *-----------------------------------------------------------------------
                    */
                   /* static void*/
                   static void
                   miSubtractO (OGdkRegion    *pReg,
                                OGdkRegionBox *r1,
                                OGdkRegionBox *r1End,
                                OGdkRegionBox *r2,
                                OGdkRegionBox *r2End,
                                int          y1,
                                int          y2)
                   {
                       OGdkRegionBox *        pNextRect;
                       int   x1;
                       
                       x1 = r1->x1;
                       
 ///                      g_assert(y1<y2);
                       pNextRect = &pReg->rects[pReg->numRects];
                       
                       while ((r1 != r1End) && (r2 != r2End))
                       {
                           if (r2->x2 <= x1)
                           {
                               /*
                                * Subtrahend missed the boat: go to next subtrahend.
                                */
                               r2++;
                           }
                           else if (r2->x1 <= x1)
                           {
                               /*
                                * Subtrahend preceeds minuend: nuke left edge of minuend.
                                */
                               x1 = r2->x2;
                               if (x1 >= r1->x2)
                               {
                                   /*
                                    * Minuend completely covered: advance to next minuend and
                                    * reset left fence to edge of new minuend.
                                    */
                                   r1++;
                                   if (r1 != r1End)
                                       x1 = r1->x1;
                               }
                               else
                               {
                                   /*
                                    * Subtrahend now used up since it doesn't extend beyond
                                    * minuend
                                    */
                                   r2++;
                               }
                           }
                           else if (r2->x1 < r1->x2)
                           {
                               /*
                                * Left part of subtrahend covers part of minuend: add uncovered
                                * part of minuend to region and skip to next subtrahend.
                                */
 ///                              g_assert(x1<r2->x1);
                               OMEMCHECK(pReg, pNextRect, pReg->rects);
                               pNextRect->x1 = x1;
                               pNextRect->y1 = y1;
                               pNextRect->x2 = r2->x1;
                               pNextRect->y2 = y2;
                               pReg->numRects += 1;
                               pNextRect++;
                               
 ///                              g_assert(pReg->numRects<=pReg->size);
                               
                               x1 = r2->x2;
                               if (x1 >= r1->x2)
                               {
                                   /*
                                    * Minuend used up: advance to new...
                                    */
                                   r1++;
                                   if (r1 != r1End)
                                       x1 = r1->x1;
                               }
                               else
                               {
                                   /*
                                    * Subtrahend used up
                                    */
                                   r2++;
                               }
                           }
                           else
                           {
                               /*
                                * Minuend used up: add any remaining piece before advancing.
                                */
                               if (r1->x2 > x1)
                               {
                                   OMEMCHECK(pReg, pNextRect, pReg->rects);
                                   pNextRect->x1 = x1;
                                   pNextRect->y1 = y1;
                                   pNextRect->x2 = r1->x2;
                                   pNextRect->y2 = y2;
                                   pReg->numRects += 1;
                                   pNextRect++;
 ///                                  g_assert(pReg->numRects<=pReg->size);
                               }
                               r1++;
                               if (r1 != r1End)
                                   x1 = r1->x1;
                           }
                       }
                       
                       /*
                        * Add remaining minuend rectangles to region.
                        */
                       while (r1 != r1End)
                       {
///                           g_assert(x1<r1->x2);
                           OMEMCHECK(pReg, pNextRect, pReg->rects);
                           pNextRect->x1 = x1;
                           pNextRect->y1 = y1;
                           pNextRect->x2 = r1->x2;
                           pNextRect->y2 = y2;
                           pReg->numRects += 1;
                           pNextRect++;
                           
///                           g_assert(pReg->numRects<=pReg->size);
                           
                           r1++;
                           if (r1 != r1End)
                           {
                               x1 = r1->x1;
                           }
                       }
                   }
                   
                   /**
                    * gdk_region_subtract:
                    * @source1: a #GdkRegion
                    * @source2: another #GdkRegion
                    *
                    * Subtracts the area of @source2 from the area @source1. The resulting
                    * area is the set of pixels contained in @source1 but not in @source2.
                    **/
                   void
                   gdk_region_subtract (OGdkRegion       *source1,
                                        const OGdkRegion *source2)
                   {
 //                      g_return_if_fail (source1 != NULL);
 //                      g_return_if_fail (source2 != NULL);
                       
                       /* check for trivial reject */
                       if ((!(source1->numRects)) || (!(source2->numRects)) ||
                           (!EXTENTCHECK(&source1->extents, &source2->extents)))
                           return;
                       
                       miRegionOp (source1, source1, source2, miSubtractO,
                                   miSubtractNonO1, (nonOverlapFunc) NULL);
                       
                       /*
                        * Can't alter source1's extents before we call miRegionOp because miRegionOp
                        * depends on the extents of those regions being the unaltered. Besides, this
                        * way there's no checking against rectangles that will be nuked
                        * due to coalescing, so we have to examine fewer rectangles.
                        */
                       miSetExtents (source1);
                   }
                   
                   /**
                    * gdk_region_xor:
                    * @source1: a #GdkRegion
                    * @source2: another #GdkRegion
                    *
                    * Sets the area of @source1 to the exclusive-OR of the areas of @source1
                    * and @source2. The resulting area is the set of pixels contained in one
                    * or the other of the two sources but not in both.
                    **/
                   void
                   gdk_region_xor (OGdkRegion       *source1,
                                   const OGdkRegion *source2)
                   {
                       OGdkRegion *trb;
                       
 //                      g_return_if_fail (source1 != NULL);
 //                      g_return_if_fail (source2 != NULL);
                       
                       trb = gdk_region_copy (source2);
                       
                       gdk_region_subtract (trb, source1);
                       gdk_region_subtract (source1, source2);
                       
                       gdk_region_union (source1, trb);
                       
                       gdk_region_destroy (trb);
                   }
                   
                   /**
                    * gdk_region_empty: 
                    * @region: a #GdkRegion
                    *
                    * Finds out if the #GdkRegion is empty.
                    *
                    * Returns: %TRUE if @region is empty.
                    */
                   bool
                   gdk_region_empty (const OGdkRegion *region)
                   {
//                       g_return_val_if_fail (region != NULL, FALSE);
                       
                       if (region->numRects == 0)
                           return TRUE;
                       else
                           return FALSE;
                   }
                   
                   /**
                    * gdk_region_equal:
                    * @region1: a #GdkRegion
                    * @region2: a #GdkRegion
                    *
                    * Finds out if the two regions are the same.
                    *
                    * Returns: %TRUE if @region1 and @region2 are equal.
                    */
                   bool
                   ogdk_region_equal (const OGdkRegion *region1,
                                     const OGdkRegion *region2)
                   {
                       int i;
                       
 //                      g_return_val_if_fail (region1 != NULL, FALSE);
 //                      g_return_val_if_fail (region2 != NULL, FALSE);
                       
                       if (region1->numRects != region2->numRects) return FALSE;
                       if (region1->numRects == 0) return TRUE;
                       if (region1->extents.x1 != region2->extents.x1) return FALSE;
                       if (region1->extents.x2 != region2->extents.x2) return FALSE;
                       if (region1->extents.y1 != region2->extents.y1) return FALSE;
                       if (region1->extents.y2 != region2->extents.y2) return FALSE;
                       for(i = 0; i < region1->numRects; i++ )
                       {
                           if (region1->rects[i].x1 != region2->rects[i].x1) return FALSE;
                           if (region1->rects[i].x2 != region2->rects[i].x2) return FALSE;
                           if (region1->rects[i].y1 != region2->rects[i].y1) return FALSE;
                           if (region1->rects[i].y2 != region2->rects[i].y2) return FALSE;
                       }
                       return TRUE;
                   }
                   
                   /**
                    * gdk_region_point_in:
                    * @region: a #GdkRegion
                    * @x: the x coordinate of a point
                    * @y: the y coordinate of a point
                    *
                    * Finds out if a point is in a region.
                    *
                    * Returns: %TRUE if the point is in @region.
                    */
                   bool
                   gdk_region_point_in (const OGdkRegion *region,
                                        int              x,
                                        int              y)
                   {
                       int i;
                       
//                       g_return_val_if_fail (region != NULL, FALSE);
                       
                       if (region->numRects == 0)
                           return FALSE;
                       if (!INBOX(region->extents, x, y))
                           return FALSE;
                       for (i = 0; i < region->numRects; i++)
                       {
                           if (INBOX (region->rects[i], x, y))
                               return TRUE;
                       }
                       return FALSE;
                   }
 

                   /**
                    * gdk_region_rect_in: 
                    * @region: a #GdkRegion.
                    * @rectangle: a #GdkRectangle.
                    *
                    * Tests whether a rectangle is within a region.
                    *
                    * Returns: %GDK_OVERLAP_RECTANGLE_IN, %GDK_OVERLAP_RECTANGLE_OUT, or
                    *   %GDK_OVERLAP_RECTANGLE_PART, depending on whether the rectangle is inside,
                    *   outside, or partly inside the #GdkRegion, respectively.
                    */
                   OGdkOverlapType
                   gdk_region_rect_in (const OGdkRegion    *region,
                                       const OGdkRectangle *rectangle)
                   {
                       OGdkRegionBox *pbox;
                       OGdkRegionBox *pboxEnd;
                       OGdkRegionBox  rect;
                       OGdkRegionBox *prect = &rect;
                       bool          partIn, partOut;
                       int rx, ry;
                       
//                       g_return_val_if_fail (region != NULL, GDK_OVERLAP_RECTANGLE_OUT);
//                       g_return_val_if_fail (rectangle != NULL, GDK_OVERLAP_RECTANGLE_OUT);
                       
                       rx = rectangle->x;
                       ry = rectangle->y;
                       
                       prect->x1 = rx;
                       prect->y1 = ry;
                       prect->x2 = rx + rectangle->width;
                       prect->y2 = ry + rectangle->height;
                       
                       /* this is (just) a useful optimization */
                       if ((region->numRects == 0) || !EXTENTCHECK (&region->extents, prect))
                           return OGDK_OVERLAP_RECTANGLE_OUT;
                       
                       partOut = FALSE;
                       partIn = FALSE;
                       
                       /* can stop when both partOut and partIn are TRUE, or we reach prect->y2 */
                       for (pbox = region->rects, pboxEnd = pbox + region->numRects; pbox < pboxEnd; pbox++)
                       {
                                
                           if (pbox->y2 <= ry)
                               continue;       /* getting up to speed or skipping remainder of band */

                           if (pbox->y1 > ry)
                           {
                               partOut = TRUE;       /* missed part of rectangle above */
                               if (partIn || (pbox->y1 >= prect->y2))
                                   break;
                               ry = pbox->y1;        /* x guaranteed to be == prect->x1 */
                           }
                           
                           if (pbox->x2 <= rx)
                                continue;               /* not far enough over yet */
                                    
                           if (pbox->x1 > rx)
                           {
                               partOut = TRUE;       /* missed part of rectangle to left */
                               if (partIn)
                                   break;
                           }
                           
                           if (pbox->x1 < prect->x2)
                           {
                               partIn = TRUE;        /* definitely overlap */
                               if (partOut)
                                   break;
                           }
                           
                           if (pbox->x2 >= prect->x2)
                           {
                               ry = pbox->y2;        /* finished with this band */
                               if (ry >= prect->y2)
                                   break;
                               rx = prect->x1;       /* reset x out to left again */
                           }
                           else
                           {
                               /*
                                * Because boxes in a band are maximal width, if the first box
                                * to overlap the rectangle doesn't completely cover it in that
                                * band, the rectangle must be partially out, since some of it
                                * will be uncovered in that band. partIn will have been set true
                                * by now...
                                */
                               break;
                           }
                                        
                       }
                            
                       return (partIn ?
                           ((ry < prect->y2) ?
                           OGDK_OVERLAP_RECTANGLE_PART : OGDK_OVERLAP_RECTANGLE_IN) : 
                           OGDK_OVERLAP_RECTANGLE_OUT);
                   }
                   
#if 0                   
                   static void
                   gdk_region_unsorted_spans_intersect_foreach (GdkRegion     *region,
                                                                const GdkSpan *spans,
                                                                int            n_spans,
                                                                GdkSpanFunc    function,
                                                                gpointer       data)
                   {
                       gint i, left, right, y;
                       gint clipped_left, clipped_right;
                       GdkRegionBox *pbox;
                       GdkRegionBox *pboxEnd;
                       
                       if (!region->numRects)
                           return;
                       
                       for (i=0;i<n_spans;i++)
                       {
                           y = spans[i].y;
                           left = spans[i].x;
                           right = left + spans[i].width; /* right is not in the span! */
                           
                           if (! ((region->extents.y1 <= y) &&
                               (region->extents.y2 > y) &&
                               (region->extents.x1 < right) &&
                               (region->extents.x2 > left)) ) 
                               continue;
                           
                           /* can stop when we passed y */
                           for (pbox = region->rects, pboxEnd = pbox + region->numRects;
                                pbox < pboxEnd;
                           pbox++)
                                {
                                    if (pbox->y2 <= y)
                                        continue; /* Not quite there yet */
                                        
                                        if (pbox->y1 > y)
                                            break; /* passed the spanline */
                                            
                                            if ((right > pbox->x1) && (left < pbox->x2)) 
                                            {
                                                GdkSpan out_span;
                                                
                                                clipped_left = MAX (left, pbox->x1);
                                                clipped_right = MIN (right, pbox->x2);
                                                
                                                out_span.y = y;
                                                out_span.x = clipped_left;
                                                out_span.width = clipped_right - clipped_left;
                                                (*function) (&out_span, data);
                                            }
                                }
                       }
                   }
#endif

#if 0
                   /**
                    * gdk_region_spans_intersect_foreach:
                    * @region: a #GdkRegion
                    * @spans: an array of #GdkSpans
                    * @n_spans: the length of @spans
                    * @sorted: %TRUE if @spans is sorted wrt. the y coordinate
                    * @function: function to call on each span in the intersection
                    * @data: data to pass to @function
                    *
                    * Calls a function on each span in the intersection of @region and @spans.
                    */
                   void
                   gdk_region_spans_intersect_foreach (GdkRegion     *region,
                                                       const GdkSpan *spans,
                                                       int            n_spans,
                                                       gboolean       sorted,
                                                       GdkSpanFunc    function,
                                                       gpointer       data)
                   {
                       gint left, right, y;
                       gint clipped_left, clipped_right;
                       GdkRegionBox *pbox;
                       GdkRegionBox *pboxEnd;
                       const GdkSpan *span, *tmpspan;
                       const GdkSpan *end_span;
                       
                       g_return_if_fail (region != NULL);
                       g_return_if_fail (spans != NULL);
                       
                       if (!sorted)
                       {
                           gdk_region_unsorted_spans_intersect_foreach (region,
                                                                        spans,
                                                                        n_spans,
                                                                        function,
                                                                        data);
                           return;
                       }
                       
                       if ((!region->numRects) || (n_spans == 0))
                           return;
                       
                       /* The main method here is to step along the
                        * sorted rectangles and spans in lock step, and
                        * clipping the spans that are in the current
                        * rectangle before going on to the next rectangle.
                        */
                       
                       span = spans;
                       end_span = spans + n_spans;
                       pbox = region->rects;
                       pboxEnd = pbox + region->numRects;
                       while (pbox < pboxEnd)
                       {
                           while ((pbox->y2 < span->y) || (span->y < pbox->y1))
                           {
                               /* Skip any rectangles that are above the current span */
                               if (pbox->y2 < span->y)
                               {
                                   pbox++;
                                   if (pbox == pboxEnd)
                                       return;
                               }
                               /* Skip any spans that are above the current rectangle */
                               if (span->y < pbox->y1)
                               {
                                   span++;
                                   if (span == end_span)
                                       return;
                               }
                           }
                           
                           /* Ok, we got at least one span that might intersect this rectangle. */
                           tmpspan = span;
                           while ((tmpspan < end_span) &&
                               (tmpspan->y < pbox->y2))
                           {
                               y = tmpspan->y;
                               left = tmpspan->x;
                               right = left + tmpspan->width; /* right is not in the span! */
                               
                               if ((right > pbox->x1) && (left < pbox->x2))
                               {
                                   GdkSpan out_span;
                                   
                                   clipped_left = MAX (left, pbox->x1);
                                   clipped_right = MIN (right, pbox->x2);
                                   
                                   out_span.y = y;
                                   out_span.x = clipped_left;
                                   out_span.width = clipped_right - clipped_left;
                                   (*function) (&out_span, data);
                               }
                               
                               tmpspan++;
                           }
                           
                           /* Finished this rectangle.
                            * The spans could still intersect the next one
                            */
                           pbox++;
                       }
                   }
#endif                  
 
 
 /* $TOG: PolyReg.c /main/15 1998/02/06 17:47:08 kaleb $ */
 /************************************************************************
  * 
  * Copyright 1987, 1998  The Open Group
  * 
  * All Rights Reserved.
  * 
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  * 
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  * OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  * 
  * Except as contained in this notice, the name of The Open Group shall not be
  * used in advertising or otherwise to promote the sale, use or other dealings
  * in this Software without prior written authorization from The Open Group.
  * 
  * 
  * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
  * 
  *                        All Rights Reserved
  * 
  * Permission to use, copy, modify, and distribute this software and its 
  * documentation for any purpose and without fee is hereby granted, 
  * provided that the above copyright notice appear in all copies and that
  * both that copyright notice and this permission notice appear in 
  * supporting documentation, and that the name of Digital not be
  * used in advertising or publicity pertaining to distribution of the
  * software without specific, written prior permission.  
  * 
  * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
  * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
  * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
  * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
  * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
  * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
  * SOFTWARE.
  * 
  ************************************************************************/
 /* $XFree86: xc/lib/X11/PolyReg.c,v 1.4 1998/10/03 08:41:21 dawes Exp $ */
 
 #define LARGE_COORDINATE 1000000
 #define SMALL_COORDINATE -LARGE_COORDINATE
 
// #include "config.h"
// #include <gdkregion.h>
// #include "gdkregion-generic.h"
// #include "gdkpoly-generic.h"
// #include "gdkalias.h"
 
 /*
  *     InsertEdgeInET
  *
  *     Insert the given edge into the edge table.
  *     First we must find the correct bucket in the
  *     Edge table, then find the right slot in the
  *     bucket.  Finally, we can insert it.
  *
  */
 static void
 InsertEdgeInET (OEdgeTable          *ET,
                 OEdgeTableEntry     *ETE,
                 int                 scanline,
                 OScanLineListBlock **SLLBlock,
                 int                *iSLLBlock)
 {
     OEdgeTableEntry *start, *prev;
     OScanLineList *pSLL, *pPrevSLL;
     OScanLineListBlock *tmpSLLBlock;
     
     /*
      * find the right bucket to put the edge into
      */
     pPrevSLL = &ET->scanlines;
     pSLL = pPrevSLL->next;
     while (pSLL && (pSLL->scanline < scanline)) 
     {
         pPrevSLL = pSLL;
         pSLL = pSLL->next;
     }
     
     /*
      * reassign pSLL (pointer to ScanLineList) if necessary
      */
     if ((!pSLL) || (pSLL->scanline > scanline)) 
     {
         if (*iSLLBlock > SLLSPERBLOCK-1) 
         {
             tmpSLLBlock = 
             (OScanLineListBlock *)malloc(sizeof(OScanLineListBlock));
             (*SLLBlock)->next = tmpSLLBlock;
             tmpSLLBlock->next = (OScanLineListBlock *)NULL;
             *SLLBlock = tmpSLLBlock;
             *iSLLBlock = 0;
         }
         pSLL = &((*SLLBlock)->SLLs[(*iSLLBlock)++]);
         
         pSLL->next = pPrevSLL->next;
         pSLL->edgelist = (OEdgeTableEntry *)NULL;
         pPrevSLL->next = pSLL;
     }
     pSLL->scanline = scanline;
     
     /*
      * now insert the edge in the right bucket
      */
     prev = (OEdgeTableEntry *)NULL;
     start = pSLL->edgelist;
     while (start && (start->bres.minor_axis < ETE->bres.minor_axis)) 
     {
         prev = start;
         start = start->next;
     }
     ETE->next = start;
     
     if (prev)
         prev->next = ETE;
     else
         pSLL->edgelist = ETE;
 }
 
 /*
  *     CreateEdgeTable
  *
  *     This routine creates the edge table for
  *     scan converting polygons. 
  *     The Edge Table (ET) looks like:
  *
  *    EdgeTable
  *     --------
  *    |  ymax  |        ScanLineLists
  *    |scanline|-->------------>-------------->...
  *     --------   |scanline|   |scanline|
  *                |edgelist|   |edgelist|
  *                ---------    ---------
  *                    |             |
  *                    |             |
  *                    V             V
  *              list of ETEs   list of ETEs
  *
  *     where ETE is an EdgeTableEntry data structure,
  *     and there is one ScanLineList per scanline at
  *     which an edge is initially entered.
  *
  */
 
 static void
 CreateETandAET (int                count,
                 const OGdkPoint    *pts,
                 OEdgeTable         *ET,
                 OEdgeTableEntry    *AET,
                 OEdgeTableEntry    *pETEs,
                 OScanLineListBlock *pSLLBlock)
 {
     const OGdkPoint *top, *bottom;
     const OGdkPoint *PrevPt, *CurrPt;
     int iSLLBlock = 0;
     int dy;
     
     if (count < 2)  return;
     
     /*
      *  initialize the Active Edge Table
      */
     AET->next = (OEdgeTableEntry *)NULL;
     AET->back = (OEdgeTableEntry *)NULL;
     AET->nextWETE = (OEdgeTableEntry *)NULL;
     AET->bres.minor_axis = SMALL_COORDINATE;
     
     /*
      *  initialize the Edge Table.
      */
     ET->scanlines.next = (OScanLineList *)NULL;
     ET->ymax = SMALL_COORDINATE;
     ET->ymin = LARGE_COORDINATE;
     pSLLBlock->next = (OScanLineListBlock *)NULL;
     
     PrevPt = &pts[count-1];
     
     /*
      *  for each vertex in the array of points.
      *  In this loop we are dealing with two vertices at
      *  a time -- these make up one edge of the polygon.
      */
     while (count--) 
     {
         CurrPt = pts++;
         
         /*
          *  find out which point is above and which is below.
          */
         if (PrevPt->y > CurrPt->y) 
         {
             bottom = PrevPt, top = CurrPt;
             pETEs->ClockWise = 0;
         }
         else 
         {
             bottom = CurrPt, top = PrevPt;
             pETEs->ClockWise = 1;
         }
         
         /*
          * don't add horizontal edges to the Edge table.
          */
         if (bottom->y != top->y) 
         {
             pETEs->ymax = bottom->y-1;  /* -1 so we don't get last scanline */
             
             /*
              *  initialize integer edge algorithm
              */
             dy = bottom->y - top->y;
             OBRESINITPGONSTRUCT(dy, top->x, bottom->x, pETEs->bres);
             
             InsertEdgeInET(ET, pETEs, top->y, &pSLLBlock, &iSLLBlock);
             
             if (PrevPt->y > ET->ymax)
                 ET->ymax = PrevPt->y;
             if (PrevPt->y < ET->ymin)
                 ET->ymin = PrevPt->y;
             pETEs++;
         }
         
         PrevPt = CurrPt;
     }
 }
 
 /*
  *     loadAET
  *
  *     This routine moves EdgeTableEntries from the
  *     EdgeTable into the Active Edge Table,
  *     leaving them sorted by smaller x coordinate.
  *
  */
 
 static void
 loadAET(OEdgeTableEntry *AET,
         OEdgeTableEntry *ETEs)
 {
     OEdgeTableEntry *pPrevAET;
     OEdgeTableEntry *tmp;
     
     pPrevAET = AET;
     AET = AET->next;
     while (ETEs) 
     {
         while (AET && (AET->bres.minor_axis < ETEs->bres.minor_axis)) 
         {
             pPrevAET = AET;
             AET = AET->next;
         }
         tmp = ETEs->next;
         ETEs->next = AET;
         if (AET)
             AET->back = ETEs;
         ETEs->back = pPrevAET;
         pPrevAET->next = ETEs;
         pPrevAET = ETEs;
         
         ETEs = tmp;
     }
 }
 
 /*
  *     computeWAET
  *
  *     This routine links the AET by the
  *     nextWETE (winding EdgeTableEntry) link for
  *     use by the winding number rule.  The final 
  *     Active Edge Table (AET) might look something
  *     like:
  *
  *     AET
  *     ----------  ---------   ---------
  *     |ymax    |  |ymax    |  |ymax    | 
  *     | ...    |  |...     |  |...     |
  *     |next    |->|next    |->|next    |->...
  *     |nextWETE|  |nextWETE|  |nextWETE|
  *     ---------   ---------   ^--------
  *         |                   |       |
  *         V------------------->       V---> ...
  *
  */
 static void
 computeWAET (OEdgeTableEntry *AET)
 {
     OEdgeTableEntry *pWETE;
     int inside = 1;
     int isInside = 0;
     
     AET->nextWETE = (OEdgeTableEntry *)NULL;
     pWETE = AET;
     AET = AET->next;
     while (AET) 
     {
         if (AET->ClockWise)
             isInside++;
         else
             isInside--;
         
         if ((!inside && !isInside) ||
             ( inside &&  isInside)) 
         {
             pWETE->nextWETE = AET;
             pWETE = AET;
             inside = !inside;
         }
         AET = AET->next;
     }
     pWETE->nextWETE = (OEdgeTableEntry *)NULL;
 }
 
 /*
  *     InsertionSort
  *
  *     Just a simple insertion sort using
  *     pointers and back pointers to sort the Active
  *     Edge Table.
  *
  */
 
 static int
 InsertionSort (OEdgeTableEntry *AET)
 {
     OEdgeTableEntry *pETEchase;
     OEdgeTableEntry *pETEinsert;
     OEdgeTableEntry *pETEchaseBackTMP;
     int changed = 0;
     
     AET = AET->next;
     while (AET) 
     {
         pETEinsert = AET;
         pETEchase = AET;
         while (pETEchase->back->bres.minor_axis > AET->bres.minor_axis)
             pETEchase = pETEchase->back;
         
         AET = AET->next;
         if (pETEchase != pETEinsert) 
         {
             pETEchaseBackTMP = pETEchase->back;
             pETEinsert->back->next = AET;
             if (AET)
                 AET->back = pETEinsert->back;
             pETEinsert->next = pETEchase;
             pETEchase->back->next = pETEinsert;
             pETEchase->back = pETEinsert;
             pETEinsert->back = pETEchaseBackTMP;
             changed = 1;
         }
     }
     return(changed);
 }
 
 /*
  *     Clean up our act.
  */
 static void
 FreeStorage (OScanLineListBlock *pSLLBlock)
 {
     OScanLineListBlock   *tmpSLLBlock;
     
     while (pSLLBlock) 
     {
         tmpSLLBlock = pSLLBlock->next;
         free (pSLLBlock);
         pSLLBlock = tmpSLLBlock;
     }
 }
 
 /*
  *     Create an array of rectangles from a list of points.
  *     If indeed these things (POINTS, RECTS) are the same,
  *     then this proc is still needed, because it allocates
  *     storage for the array, which was allocated on the
  *     stack by the calling procedure.
  *
  */
 static int
 PtsToRegion (int         numFullPtBlocks,
              int         iCurPtBlock,
              OPOINTBLOCK *FirstPtBlock,
              OGdkRegion  *reg)
 {
     OGdkRegionBox *rects;
     OGdkPoint *pts;
     OPOINTBLOCK *CurPtBlock;
     int i;
     OGdkRegionBox *extents;
     int numRects;
     
     extents = &reg->extents;
     
     numRects = ((numFullPtBlocks * NUMPTSTOBUFFER) + iCurPtBlock) >> 1;
     
     OGROWREGION(reg, numRects);
     
     CurPtBlock = FirstPtBlock;
     rects = reg->rects - 1;
     numRects = 0;
     extents->x1 = 1000000 /*G_MAXSHORT*/,  extents->x2 = -1000000 /*G_MINSHORT*/;
     
     for ( ; numFullPtBlocks >= 0; numFullPtBlocks--) {
         /* the loop uses 2 points per iteration */
         i = NUMPTSTOBUFFER >> 1;
         if (!numFullPtBlocks)
             i = iCurPtBlock >> 1;
         for (pts = CurPtBlock->pts; i--; pts += 2) {
             if (pts->x == pts[1].x)
                 continue;
             if (numRects && pts->x == rects->x1 && pts->y == rects->y2 &&
                 pts[1].x == rects->x2 &&
                 (numRects == 1 || rects[-1].y1 != rects->y1) &&
                 (i && pts[2].y > pts[1].y)) {
                 rects->y2 = pts[1].y + 1;
             continue;
                 }
                 numRects++;
                 rects++;
                 rects->x1 = pts->x;  rects->y1 = pts->y;
                 rects->x2 = pts[1].x;  rects->y2 = pts[1].y + 1;
                 if (rects->x1 < extents->x1)
                     extents->x1 = rects->x1;
                 if (rects->x2 > extents->x2)
                     extents->x2 = rects->x2;
         }
         CurPtBlock = CurPtBlock->next;
     }
     
     if (numRects) {
         extents->y1 = reg->rects->y1;
         extents->y2 = rects->y2;
     } else {
         extents->x1 = 0;
         extents->y1 = 0;
         extents->x2 = 0;
         extents->y2 = 0;
     }
     reg->numRects = numRects;
     
     return(TRUE);
 }
 
 /**
  * gdk_region_polygon:
  * @points: an array of #GdkPoint structs
  * @n_points: the number of elements in the @points array
  * @fill_rule: specifies which pixels are included in the region when the 
  *     polygon overlaps itself.
  * 
  * Creates a new #GdkRegion using the polygon defined by a 
  * number of points.
  *
  * Returns: a new #GdkRegion based on the given polygon
  */
 OGdkRegion *
 gdk_region_polygon (const OGdkPoint *points,
                     int            n_points,
                     OGdkFillRule     fill_rule)
 {
     OGdkRegion *region;
     OEdgeTableEntry *pAET;   /* Active Edge Table       */
     int y;                  /* current scanline        */
     int iPts = 0;           /* number of pts in buffer */
     OEdgeTableEntry *pWETE;  /* Winding Edge Table Entry*/
     OScanLineList *pSLL;     /* current scanLineList    */
     OGdkPoint *pts;          /* output buffer           */
     OEdgeTableEntry *pPrevAET;        /* ptr to previous AET     */
     OEdgeTable ET;                    /* header node for ET      */
     OEdgeTableEntry AET;              /* header node for AET     */
     OEdgeTableEntry *pETEs;           /* EdgeTableEntries pool   */
     OScanLineListBlock SLLBlock;      /* header for scanlinelist */
     int fixWAET = FALSE;
     OPOINTBLOCK FirstPtBlock, *curPtBlock; /* PtBlock buffers    */
     OPOINTBLOCK *tmpPtBlock;
     int numFullPtBlocks = 0;
     
     region = gdk_region_new ();
     
     /* special case a rectangle */
     if (((n_points == 4) ||
         ((n_points == 5) && (points[4].x == points[0].x) && (points[4].y == points[0].y))) &&
         (((points[0].y == points[1].y) &&
         (points[1].x == points[2].x) &&
         (points[2].y == points[3].y) &&
         (points[3].x == points[0].x)) ||
         ((points[0].x == points[1].x) &&
         (points[1].y == points[2].y) &&
         (points[2].x == points[3].x) &&
         (points[3].y == points[0].y)))) {
         region->extents.x1 = MIN(points[0].x, points[2].x);
     region->extents.y1 = MIN(points[0].y, points[2].y);
     region->extents.x2 = MAX(points[0].x, points[2].x);
     region->extents.y2 = MAX(points[0].y, points[2].y);
     if ((region->extents.x1 != region->extents.x2) &&
         (region->extents.y1 != region->extents.y2)) {
         region->numRects = 1;
     *(region->rects) = region->extents;
         }
         return(region);
         }
         
         pETEs = (OEdgeTableEntry *)malloc(sizeof(OEdgeTableEntry) * n_points);
         
         pts = FirstPtBlock.pts;
         CreateETandAET(n_points, points, &ET, &AET, pETEs, &SLLBlock);
         pSLL = ET.scanlines.next;
         curPtBlock = &FirstPtBlock;
         
         if (fill_rule == OGDK_EVEN_ODD_RULE) {
             /*
              *  for each scanline
              */
             for (y = ET.ymin; y < ET.ymax; y++) {
                 /*
                  *  Add a new edge to the active edge table when we
                  *  get to the next edge.
                  */
                 if (pSLL != NULL && y == pSLL->scanline) {
                     loadAET(&AET, pSLL->edgelist);
                     pSLL = pSLL->next;
                 }
                 pPrevAET = &AET;
                 pAET = AET.next;
                 
                 /*
                  *  for each active edge
                  */
                 while (pAET) {
                     pts->x = pAET->bres.minor_axis,  pts->y = y;
                     pts++, iPts++;
                     
                     /*
                      *  send out the buffer
                      */
                     if (iPts == NUMPTSTOBUFFER) {
                         tmpPtBlock = (OPOINTBLOCK *)malloc(sizeof(OPOINTBLOCK));
                         tmpPtBlock->next = NULL;
                         curPtBlock->next = tmpPtBlock;
                         curPtBlock = tmpPtBlock;
                         pts = curPtBlock->pts;
                         numFullPtBlocks++;
                         iPts = 0;
                     }
                     OEVALUATEEDGEEVENODD(pAET, pPrevAET, y);
                 }
                 (void) InsertionSort(&AET);
             }
         }
         else {
             /*
              *  for each scanline
              */
             for (y = ET.ymin; y < ET.ymax; y++) {
                 /*
                  *  Add a new edge to the active edge table when we
                  *  get to the next edge.
                  */
                 if (pSLL != NULL && y == pSLL->scanline) {
                     loadAET(&AET, pSLL->edgelist);
                     computeWAET(&AET);
                     pSLL = pSLL->next;
                 }
                 pPrevAET = &AET;
                 pAET = AET.next;
                 pWETE = pAET;
                 
                 /*
                  *  for each active edge
                  */
                 while (pAET) {
                     /*
                      *  add to the buffer only those edges that
                      *  are in the Winding active edge table.
                      */
                     if (pWETE == pAET) {
                         pts->x = pAET->bres.minor_axis,  pts->y = y;
                         pts++, iPts++;
                         
                         /*
                          *  send out the buffer
                          */
                         if (iPts == NUMPTSTOBUFFER) {
                             tmpPtBlock = (OPOINTBLOCK *)malloc(sizeof(OPOINTBLOCK));
                             tmpPtBlock->next = NULL;
                             curPtBlock->next = tmpPtBlock;
                             curPtBlock = tmpPtBlock;
                             pts = curPtBlock->pts;
                             numFullPtBlocks++;    iPts = 0;
                         }
                         pWETE = pWETE->nextWETE;
                     }
                     OEVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
                 }
                 
                 /*
                  *  recompute the winding active edge table if
                  *  we just resorted or have exited an edge.
                  */
                 if (InsertionSort(&AET) || fixWAET) {
                     computeWAET(&AET);
                     fixWAET = FALSE;
                 }
             }
         }
         FreeStorage(SLLBlock.next); 
         (void) PtsToRegion(numFullPtBlocks, iPts, &FirstPtBlock, region);
         for (curPtBlock = FirstPtBlock.next; --numFullPtBlocks >= 0;) {
             tmpPtBlock = curPtBlock->next;
             free (curPtBlock);
             curPtBlock = tmpPtBlock;
         }
         free (pETEs);
         return(region);
 }
 
// #define __GDK_POLYREG_GENERIC_C__
// #include "gdkaliasdef.c"
#endif  //USE_NEW_REGION 
