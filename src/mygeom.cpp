/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tesselated Polygon Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *
 */
// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "wx/tokenzr.h"
#include <wx/mstream.h>

#ifdef USE_S57
#include "mygdal/ogr_geometry.h"
#endif

#include "cutil.h"

#include "vector2D.h"

#include "s52s57.h"

#include "mygeom.h"
#include "georef.h"

#include "dychart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tesselator.h"

#ifdef __WXMSW__
#include <windows.h>
#endif



//      Module Internal Prototypes


static int            s_nvcall;
static int            s_nvmax;
static GLdouble       *s_pwork_buf;
static int            s_buf_len;
static int            s_buf_idx;
static unsigned int   s_gltri_type;
TriPrim               *s_pTPG_Head;
TriPrim               *s_pTPG_Last;
static GLUtesselator  *GLUtessobj;
static double         s_ref_lat;
static double         s_ref_lon;

static bool           s_bmerc_transform;
static double         s_transform_x_rate;
static double         s_transform_x_origin;
static double         s_transform_y_rate;
static double         s_transform_y_origin;
wxArrayPtrVoid        *s_pCombineVertexArray;
int s_tess_orient;

static const double   CM93_semimajor_axis_meters = 6378388.0;            // CM93 semimajor axis
static bool           s_bSENC_SM;

static wxMemoryOutputStream *ostream1;
static wxMemoryOutputStream *ostream2;

int g_keep;
int g_orig;


//  For __WXMSW__ builds using GLU_TESS and glu32.dll
//  establish the dll entry points

#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL

//  Formal definitions of required functions
typedef void (CALLBACK* LPFNDLLTESSPROPERTY)      ( GLUtesselator *tess,
                                                    GLenum        which,
                                                    GLdouble      value );
typedef GLUtesselator * (CALLBACK* LPFNDLLNEWTESS)( void);
typedef void (CALLBACK* LPFNDLLTESSBEGINCONTOUR)  ( GLUtesselator *);
typedef void (CALLBACK* LPFNDLLTESSENDCONTOUR)    ( GLUtesselator *);
typedef void (CALLBACK* LPFNDLLTESSBEGINPOLYGON)  ( GLUtesselator *, void*);
typedef void (CALLBACK* LPFNDLLTESSENDPOLYGON)    ( GLUtesselator *);
typedef void (CALLBACK* LPFNDLLDELETETESS)        ( GLUtesselator *);
typedef void (CALLBACK* LPFNDLLTESSVERTEX)        ( GLUtesselator *, GLdouble *, GLdouble *);
typedef void (CALLBACK* LPFNDLLTESSCALLBACK)      ( GLUtesselator *, const int, void (CALLBACK *fn)() );

//  Static pointers to the functions
static LPFNDLLTESSPROPERTY      s_lpfnTessProperty;
static LPFNDLLNEWTESS           s_lpfnNewTess;
static LPFNDLLTESSBEGINCONTOUR  s_lpfnTessBeginContour;
static LPFNDLLTESSENDCONTOUR    s_lpfnTessEndContour;
static LPFNDLLTESSBEGINPOLYGON  s_lpfnTessBeginPolygon;
static LPFNDLLTESSENDPOLYGON    s_lpfnTessEndPolygon;
static LPFNDLLDELETETESS        s_lpfnDeleteTess;
static LPFNDLLTESSVERTEX        s_lpfnTessVertex;
static LPFNDLLTESSCALLBACK      s_lpfnTessCallback;

//  Mapping of pointers to glu functions by substitute macro
#define gluTessProperty         s_lpfnTessProperty
#define gluNewTess              s_lpfnNewTess
#define gluTessBeginContour     s_lpfnTessBeginContour
#define gluTessEndContour       s_lpfnTessEndContour
#define gluTessBeginPolygon     s_lpfnTessBeginPolygon
#define gluTessEndPolygon       s_lpfnTessEndPolygon
#define gluDeleteTess           s_lpfnDeleteTess
#define gluTessVertex           s_lpfnTessVertex
#define gluTessCallback         s_lpfnTessCallback


#endif
#endif
//  Flag to tell that dll is ready
bool           s_glu_dll_ready;
HINSTANCE      s_hGLU_DLL;                   // Handle to DLL
#endif

//-----------------------------------------------------------
//
//  Static memory allocators for libtess2
//
//-----------------------------------------------------------
//#define USE_POOL 1

void* stdAlloc(void* userData, unsigned int size)
{
    int* allocated = ( int*)userData;
    TESS_NOTUSED(userData);
    *allocated += (int)size;
    return malloc(size);
}

void stdFree(void* userData, void* ptr)
{
    TESS_NOTUSED(userData);
    free(ptr);
}

struct MemPool
{
    unsigned char* buf;
    unsigned int cap;
    unsigned int size;
};

void* poolAlloc( void* userData, unsigned int size )
{
    struct MemPool* pool = (struct MemPool*)userData;
    size = (size+0x7) & ~0x7;
    if (pool->size + size < pool->cap)
    {
        unsigned char* ptr = pool->buf + pool->size;
        pool->size += size;
        return ptr;
    }
    printf("out of mem: %d < %d!\n", pool->size + size, pool->cap);
    return 0;
}

void poolFree( void* userData, void* ptr )
{
    // empty
    TESS_NOTUSED(userData);
    TESS_NOTUSED(ptr);
}








/**
 * Returns TRUE if the ring has clockwise winding.
 *
 * @return TRUE if clockwise otherwise FALSE.
 */

bool isRingClockwise(wxPoint2DDouble *pp, int nPointCount)

{
    double dfSum = 0.0;
    
    for( int iVert = 0; iVert < nPointCount-1; iVert++ )
    {
        dfSum += pp[iVert].m_x * pp[iVert+1].m_y
        - pp[iVert].m_y * pp[iVert+1].m_x;
    }
    
    dfSum += pp[nPointCount-1].m_x * pp[0].m_y
    - pp[nPointCount-1].m_y * pp[0].m_x;
    
    return dfSum < 0.0;
}

//------------------------------------------------------------------------------
//          Extended_Geometry Implementation
//------------------------------------------------------------------------------
Extended_Geometry::Extended_Geometry()
{
      vertex_array = NULL;
      contour_array = NULL;
}

Extended_Geometry::~Extended_Geometry()
{
      free(vertex_array);
      free(contour_array);
}


//------------------------------------------------------------------------------
//          PolyTessGeo Implementation
//------------------------------------------------------------------------------
PolyTessGeo::PolyTessGeo()
{
    m_pxgeom = NULL;
    
}

//      Build PolyTessGeo Object from Extended_Geometry
PolyTessGeo::PolyTessGeo(Extended_Geometry *pxGeom)
{
      m_ppg_head = NULL;
      m_bOK = false;

      m_pxgeom = pxGeom;

}

//      Build PolyTessGeo Object from OGR Polygon
PolyTessGeo::PolyTessGeo(OGRPolygon *poly, bool bSENC_SM, double ref_lat, double ref_lon,
                         double LOD_meters)
{
    ErrorCode = 0;
    m_ppg_head = NULL;
    m_pxgeom = NULL;
    m_tess_orient = TESS_HORZ;
    
    m_ref_lat = ref_lat;
    m_ref_lon = ref_lon;
    m_LOD_meters = LOD_meters;
    m_b_senc_sm = bSENC_SM;
    m_bmerc_transform = false;
    
    //  Build the array of contour point counts
      //      Get total number of contours
    m_ncnt = poly->getNumInteriorRings() + 1;

      // build the contour point countarray
    m_cntr = (int *)malloc(m_ncnt * sizeof(int));

    m_cntr[0] = poly->getExteriorRing()->getNumPoints();

    for(int i = 1; i < m_ncnt ; i++){
          m_cntr[i] = poly->getInteriorRing( i-1 )->getNumPoints();
    }

    //  Build the point array index table
    
    OGRPoint p;
    m_vertexPtrArray = (double **)malloc(m_ncnt * sizeof(double *));
    
    m_vertexPtrArray[0] = (double *)malloc(m_cntr[0] * sizeof(double) * 2);
    double *pp = m_vertexPtrArray[0];
    for(int i = 0 ; i < m_cntr[0] ; i++){
        poly->getExteriorRing()->getPoint(i, &p);
        *pp++ = p.getX();
        *pp++ = p.getY();
    }

    for(int i = 1; i < m_ncnt ; i++){
        m_vertexPtrArray[i] = (double *)malloc(m_cntr[i] * sizeof(double) * 2);
        double *pp = m_vertexPtrArray[i];
        for(int j = 0 ; j < m_cntr[i] ; j++){
            poly->getInteriorRing(i-1)->getPoint(j, &p);
            *pp++ = p.getX();
            *pp++ = p.getY();
        }
    }
       
//    PolyGeo BBox as lat/lon
    OGREnvelope Envelope;
    poly->getEnvelope(&Envelope);
    xmin = Envelope.MinX;
    ymin = Envelope.MinY;
    xmax = Envelope.MaxX;
    ymax = Envelope.MaxY;

  
    mx_rate = 1.0;
    mx_offset = 0.0;
    my_rate = 1.0;
    my_offset = 0.0;

    BuildTessGL();
    
    //ErrorCode = PolyTessGeoGL(poly, bSENC_SM, ref_lat, ref_lon);
}


void PolyTessGeo::SetExtents(double x_left, double y_bot, double x_right, double y_top)
{
    xmin = x_left;
    ymin = y_bot;
    xmax = x_right;
    ymax = y_top;
}



PolyTessGeo::~PolyTessGeo()
{

    delete  m_ppg_head;

    delete  m_pxgeom;

#ifdef USE_GLU_TESS
    if(s_pwork_buf)
        free( s_pwork_buf );
    s_pwork_buf = NULL;
#endif

}

int PolyTessGeo::BuildDeferredTess(void)
{
    if(m_pxgeom){
        
      // For cm93
      m_b_senc_sm =  false;
      m_bmerc_transform = true;
      m_tess_orient = TESS_HORZ;                    // prefer horizontal tristrips

      //      Get total number of contours
      m_ncnt = m_pxgeom->n_contours;

      // build the contour point countarray
      m_cntr = (int *)malloc(m_ncnt * sizeof(int));

      for(int i = 0; i < m_ncnt ; i++){
          m_cntr[i] = m_pxgeom->contour_array[i];
      }
      

      //  Build the point array index table
    
      m_vertexPtrArray = (double **)malloc(m_ncnt * sizeof(double *));
    
      double *vertex_pointer = (double *)m_pxgeom->vertex_array;
      vertex_pointer += 2;      // skip 0
      
      for(int k = 0 ; k < m_ncnt ; k++){
            m_vertexPtrArray[k] = vertex_pointer;
            vertex_pointer += 2 * m_cntr[k];
      }
  
      mx_rate = m_pxgeom->x_rate;
      mx_offset = m_pxgeom->x_offset;
      my_rate = m_pxgeom->y_rate;
      my_offset = m_pxgeom->y_offset;

      int rv = BuildTessGL();

      // All done with this geometry
      delete m_pxgeom;
      m_pxgeom = NULL;
      
      return rv;

    }
    else
        return 0;
}



#ifdef __WXMSW__
#define __CALL_CONVENTION /*__stdcall*/
#else
#define __CALL_CONVENTION
#endif


void __CALL_CONVENTION beginCallback(GLenum which);
void __CALL_CONVENTION errorCallback(GLenum errorCode);
void __CALL_CONVENTION endCallback(void);
void __CALL_CONVENTION vertexCallback(GLvoid *vertex);
void __CALL_CONVENTION combineCallback(GLdouble coords[3],
                     GLdouble *vertex_data[4],
                     GLfloat weight[4], GLdouble **dataOut );


//      Build PolyTessGeo Object from OGR Polygon
//      Using OpenGL/GLU tesselator
int PolyTessGeo::PolyTessGeoGL(OGRPolygon *poly, bool bSENC_SM, double ref_lat, double ref_lon)
{
#ifdef ocpnUSE_GL
    
    int iir, ip;
    int *cntr;
    GLdouble *geoPt;

    wxString    sout;
    wxString    sout1;
    wxString    stemp;

    //  Make a quick sanity check of the polygon coherence
    bool b_ok = true;
    OGRLineString *tls = poly->getExteriorRing();
    if(!tls) {
        b_ok = false;
    }
    else {
        int tnpta  = poly->getExteriorRing()->getNumPoints();
        if(tnpta < 3 )
            b_ok = false;
    }

    
    for( iir=0 ; iir < poly->getNumInteriorRings() ; iir++)
    {
        int tnptr = poly->getInteriorRing(iir)->getNumPoints();
        if( tnptr < 3 )
            b_ok = false;
    }
    
    if( !b_ok )
        return ERROR_BAD_OGRPOLY;
    

#ifdef __WXMSW__
//  If using the OpenGL dlls provided with Windows,
//  load the dll and establish addresses of the entry points needed

#ifdef USE_GLU_DLL

    if(!s_glu_dll_ready)
    {


        s_hGLU_DLL = LoadLibrary("glu32.dll");
        if (s_hGLU_DLL != NULL)
        {
            s_lpfnTessProperty = (LPFNDLLTESSPROPERTY)GetProcAddress(s_hGLU_DLL,"gluTessProperty");
            s_lpfnNewTess = (LPFNDLLNEWTESS)GetProcAddress(s_hGLU_DLL, "gluNewTess");
            s_lpfnTessBeginContour = (LPFNDLLTESSBEGINCONTOUR)GetProcAddress(s_hGLU_DLL, "gluTessBeginContour");
            s_lpfnTessEndContour = (LPFNDLLTESSENDCONTOUR)GetProcAddress(s_hGLU_DLL, "gluTessEndContour");
            s_lpfnTessBeginPolygon = (LPFNDLLTESSBEGINPOLYGON)GetProcAddress(s_hGLU_DLL, "gluTessBeginPolygon");
            s_lpfnTessEndPolygon = (LPFNDLLTESSENDPOLYGON)GetProcAddress(s_hGLU_DLL, "gluTessEndPolygon");
            s_lpfnDeleteTess = (LPFNDLLDELETETESS)GetProcAddress(s_hGLU_DLL, "gluDeleteTess");
            s_lpfnTessVertex = (LPFNDLLTESSVERTEX)GetProcAddress(s_hGLU_DLL, "gluTessVertex");
            s_lpfnTessCallback = (LPFNDLLTESSCALLBACK)GetProcAddress(s_hGLU_DLL, "gluTessCallback");

            s_glu_dll_ready = true;
        }
        else
        {
            return ERROR_NO_DLL;
        }
    }

#endif
#endif


    //  Allocate a work buffer, which will be grown as needed
#define NINIT_BUFFER_LEN 10000
    s_pwork_buf = (GLdouble *)malloc(NINIT_BUFFER_LEN * 2 * sizeof(GLdouble));
    s_buf_len = NINIT_BUFFER_LEN * 2;
    s_buf_idx = 0;

      //    Create an array to hold pointers to allocated vertices created by "combine" callback,
      //    so that they may be deleted after tesselation.
    s_pCombineVertexArray = new wxArrayPtrVoid;

    //  Create tesselator
    GLUtessobj = gluNewTess();

    //  Register the callbacks
    gluTessCallback(GLUtessobj, GLU_TESS_BEGIN,   (GLvoid (__CALL_CONVENTION *) ())&beginCallback);
    gluTessCallback(GLUtessobj, GLU_TESS_BEGIN,   (GLvoid (__CALL_CONVENTION *) ())&beginCallback);
    gluTessCallback(GLUtessobj, GLU_TESS_VERTEX,  (GLvoid (__CALL_CONVENTION *) ())&vertexCallback);
    gluTessCallback(GLUtessobj, GLU_TESS_END,     (GLvoid (__CALL_CONVENTION *) ())&endCallback);
    gluTessCallback(GLUtessobj, GLU_TESS_COMBINE, (GLvoid (__CALL_CONVENTION *) ())&combineCallback);

//    gluTessCallback(GLUtessobj, GLU_TESS_ERROR,   (GLvoid (__CALL_CONVENTION *) ())&errorCallback);

//    glShadeModel(GL_SMOOTH);
    gluTessProperty(GLUtessobj, GLU_TESS_WINDING_RULE,
                    GLU_TESS_WINDING_POSITIVE );

    //  gluTess algorithm internally selects vertically oriented triangle strips and fans.
    //  This orientation is not optimal for conventional memory-mapped raster display shape filling.
    //  We can "fool" the algorithm by interchanging the x and y vertex values passed to gluTessVertex
    //  and then reverting x and y on the resulting vertexCallbacks.
    //  In this implementation, we will explicitely set the preferred orientation.

    //Set the preferred orientation
    s_tess_orient = TESS_HORZ;                    // prefer horizontal tristrips



//    PolyGeo BBox as lat/lon
    OGREnvelope Envelope;
    poly->getEnvelope(&Envelope);
    xmin = Envelope.MinX;
    ymin = Envelope.MinY;
    xmax = Envelope.MaxX;
    ymax = Envelope.MaxY;


//      Get total number of contours
    m_ncnt = 1;                         // always exterior ring
    int nint = poly->getNumInteriorRings();  // interior rings
    m_ncnt += nint;


//      Allocate cntr array
    cntr = (int *)malloc(m_ncnt * sizeof(int));


//      Get total number of points(vertices)
    int npta  = poly->getExteriorRing()->getNumPoints();
    cntr[0] = npta;
    npta += 2;                            // fluff

    for( iir=0 ; iir < nint ; iir++)
    {
        int nptr = poly->getInteriorRing(iir)->getNumPoints();
        cntr[iir+1] = nptr;

        npta += nptr + 2;
    }

//    printf("pPoly npta: %d\n", npta);

    geoPt = (GLdouble *)malloc((npta) * 3 * sizeof(GLdouble));     // vertex array



   //  Grow the work buffer if necessary

    if((npta * 4) > s_buf_len)
    {
        s_pwork_buf = (GLdouble *)realloc(s_pwork_buf, npta * 4 * sizeof(GLdouble));
        s_buf_len = npta * 4;
    }


//  Define the polygon
    gluTessBeginPolygon(GLUtessobj, NULL);


//      Create input structures

//    Exterior Ring
    int npte  = poly->getExteriorRing()->getNumPoints();
    cntr[0] = npte;

    GLdouble *ppt = geoPt;


//  Check and account for winding direction of ring
    bool cw = !(poly->getExteriorRing()->isClockwise() == 0);

    double x0, y0, x, y;
    OGRPoint p;

    if(cw)
    {
        poly->getExteriorRing()->getPoint(0, &p);
        x0 = p.getX();
        y0 = p.getY();
    }
    else
    {
        poly->getExteriorRing()->getPoint(npte-1, &p);
        x0 = p.getX();
        y0 = p.getY();
    }

    //  Transcribe contour to an array of doubles, with duplicates eliminated
    double *DPbuffer = (double *)malloc(npte * 2 * sizeof(double));
    double *DPrun = DPbuffer;
    int nPoints = npte;
    
    for(ip = 0 ; ip < npte ; ip++)
    {
        int pidx;
        if(cw)
            pidx = npte - ip - 1;
    
        else
            pidx = ip;
    
        poly->getExteriorRing()->getPoint(pidx, &p);
        x = p.getX();
        y = p.getY();
    
        if(  ((fabs(x-x0) > EQUAL_EPS) || (fabs(y-y0) > EQUAL_EPS)))
        {
            GLdouble *ppt_temp = ppt;
            if(s_tess_orient == TESS_VERT)
            {
                *DPrun++ = x;
                *DPrun++ = y;
            }
            else
            {
                *DPrun++ = y;
                *DPrun++ = x;
            }
        
            x0 = x;
            y0 = y;
        }
        else
            nPoints--;
    
    }

    std::vector<int> index_keep;
    if(nPoints > 5 && (m_LOD_meters > .01)){
        index_keep.push_back(0);
        index_keep.push_back(nPoints-1);
        index_keep.push_back(1);
        index_keep.push_back(nPoints-2);
        
        DouglasPeucker(DPbuffer, 1, nPoints-2, m_LOD_meters/(1852 * 60), &index_keep);
//        printf("DP Reduction: %d/%d\n", index_keep.size(), nPoints);
        
        g_keep += index_keep.size();
        g_orig += nPoints;
//        printf("...................Running: %g\n", (double)g_keep/g_orig);
    }
    else {
        index_keep.resize(nPoints);
        for(int i = 0 ; i < nPoints ; i++)
            index_keep[i] = i;
    }
    
    cntr[0] = index_keep.size();
 
    
    // Mark the keepers by adding a simple constant to X
    for(unsigned int i=0 ; i < index_keep.size() ; i++){
        DPbuffer[2*index_keep[i]] += 2000.;
    }

    

    //  Declare the gluContour and copy the points
    gluTessBeginContour(GLUtessobj);
    
    DPrun = DPbuffer;
    for(ip = 0 ; ip < nPoints ; ip++)
    {
        x = *DPrun++;
        y = *DPrun++;
        
        if(x > 1000.){
            
            GLdouble *ppt_top = ppt;
            *ppt++ = x-2000;
            *ppt++ = y;
            *ppt++ = 0;
            
            gluTessVertex( GLUtessobj, ppt_top, ppt_top ) ;
        }
    }
    
    gluTessEndContour(GLUtessobj);
    
    free(DPbuffer);    
    
  
//  Now the interior contours
    for(iir=0 ; iir < nint ; iir++)
    {
        gluTessBeginContour(GLUtessobj);

        int npti = poly->getInteriorRing(iir)->getNumPoints();

      //  Check and account for winding direction of ring
        bool cw = !(poly->getInteriorRing(iir)->isClockwise() == 0);

        if(!cw)
        {
            poly->getInteriorRing(iir)->getPoint(0, &p);
            x0 = p.getX();
            y0 = p.getY();
        }
        else
        {
            poly->getInteriorRing(iir)->getPoint(npti-1, &p);
            x0 = p.getX();
            y0 = p.getY();
        }

//  Transcribe points to vertex array, in proper order with no duplicates
//   also, accounting for tess_orient
        for(int ip = 0 ; ip < npti ; ip++)
        {
            OGRPoint p;
            int pidx;
            if(!cw)                               // interior contours must be cw
                pidx = npti - ip - 1;
            else
                pidx = ip;

            poly->getInteriorRing(iir)->getPoint(pidx, &p);
            x = p.getX();
            y = p.getY();

            if((fabs(x-x0) > EQUAL_EPS) || (fabs(y-y0) > EQUAL_EPS))
            {
                GLdouble *ppt_temp = ppt;
                if(s_tess_orient == TESS_VERT)
                {
                    *ppt++ = x;
                    *ppt++ = y;
                }
                else
                {
                    *ppt++ = y;
                    *ppt++ = x;
                }
                *ppt++ = 0.0;

                gluTessVertex( GLUtessobj, ppt_temp, ppt_temp ) ;

//       printf("tess from Poly, internal vertex %d %g %g\n", ip, x, y);

            }
            else
                cntr[iir+1]--;

            x0 = x;
            y0 = y;

        }

        gluTessEndContour(GLUtessobj);
    }

    //  Store some SM conversion data in static store,
    //  for callback access
    s_ref_lat = ref_lat;
    s_ref_lon = ref_lon;
    s_bSENC_SM = bSENC_SM;

    s_bmerc_transform = false;

    //      Ready to kick off the tesselator

    s_pTPG_Last = NULL;
    s_pTPG_Head = NULL;

    s_nvmax = 0;

    gluTessEndPolygon(GLUtessobj);          // here it goes

    m_nvertex_max = s_nvmax;               // record largest vertex count, updates in callback


    //  Tesselation all done, so...

    //  Create the data structures

    m_ppg_head = new PolyTriGroup;
    m_ppg_head->m_bSMSENC = s_bSENC_SM;

    m_ppg_head->nContours = m_ncnt;

    m_ppg_head->pn_vertex = cntr;             // pointer to array of poly vertex counts
    m_ppg_head->data_type = DATA_TYPE_DOUBLE;
    

//  Transcribe the raw geometry buffer
//  Converting to float as we go, and
//  allowing for tess_orient
//  Also, convert to SM if requested

// Recalculate the size of the geometry buffer
    int nptfinal = cntr[0] + 2;
    for(int i=0 ; i < nint ; i++)
        nptfinal += cntr[i+1] + 2;
    
    //  No longer need the full geometry in the SENC,
    nptfinal = 1;
    
    m_nwkb = (nptfinal + 1) * 2 * sizeof(float);
    m_ppg_head->pgroup_geom = (float *)calloc(sizeof(float), (nptfinal + 1) * 2);
    float *vro = m_ppg_head->pgroup_geom;
    ppt = geoPt;
    float tx,ty;

    for(ip = 0 ; ip < nptfinal ; ip++)
    {
        if(TESS_HORZ == s_tess_orient)
        {
            ty = *ppt++;
            tx = *ppt++;
        }
        else
        {
            tx = *ppt++;
            ty = *ppt++;
        }

        if(bSENC_SM)
        {
            //  Calculate SM from chart common reference point
            double easting, northing;
            toSM(ty, tx, ref_lat, ref_lon, &easting, &northing);
            *vro++ = easting;              // x
            *vro++ = northing;             // y
        }
        else
        {
            *vro++ = tx;                  // lon
            *vro++ = ty;                  // lat
        }

        ppt++;                      // skip z
    }

    m_ppg_head->tri_prim_head = s_pTPG_Head;         // head of linked list of TriPrims


    //  Convert the Triangle vertex arrays into a single memory allocation of floats
    //  to reduce SENC size and enable efficient access later
    
    //  First calculate the total byte size
    int total_byte_size = 2 * sizeof(float);
    TriPrim *p_tp = m_ppg_head->tri_prim_head;
    while( p_tp ) {
        total_byte_size += p_tp->nVert * 2 * sizeof(float);
        p_tp = p_tp->p_next; // pick up the next in chain
    }
    
    float *vbuf = (float *)malloc(total_byte_size);
    p_tp = m_ppg_head->tri_prim_head;
    float *p_run = vbuf;
    while( p_tp ) {
        float *pfbuf = p_run;
        GLdouble *pdouble_buf = (GLdouble *)p_tp->p_vertex;
        
        for( int i=0 ; i < p_tp->nVert * 2 ; ++i){
            float x = (float)( *((GLdouble *)pdouble_buf) );
            pdouble_buf++;
            *p_run++ = x;
        }
        
        free(p_tp->p_vertex);
        p_tp->p_vertex = (double *)pfbuf;
        p_tp = p_tp->p_next; // pick up the next in chain
    }
    m_ppg_head->bsingle_alloc = true;
    m_ppg_head->single_buffer = (unsigned char *)vbuf;
    m_ppg_head->single_buffer_size = total_byte_size;
    m_ppg_head->data_type = DATA_TYPE_FLOAT;
    
    
    
    
    
    
    gluDeleteTess(GLUtessobj);

    free( s_pwork_buf );
    s_pwork_buf = NULL;

    free (geoPt);

    //      Free up any "Combine" vertices created
    for(unsigned int i = 0; i < s_pCombineVertexArray->GetCount() ; i++)
          free (s_pCombineVertexArray->Item(i));
    delete s_pCombineVertexArray;

    m_bOK = true;

#endif          //    #ifdef ocpnUSE_GL
    
    return 0;
}

int PolyTessGeo::BuildTessGL(void)
{
    //  Setup the tesselator
    
    TESSalloc ma;
    TESStesselator* tess = 0;
    const int nvp = 3;
//    unsigned char* vflags = 0;
#ifdef USE_POOL
    struct MemPool pool;
    unsigned char mem[4024*1024];
//    int nvflags = 0;
#else
    int allocated = 0;
    double t0 = 0, t1 = 0;
#endif

#ifdef USE_POOL

    pool.size = 0;
    pool.cap = sizeof(mem);
    pool.buf = mem;
    memset(&ma, 0, sizeof(ma));
    ma.memalloc = poolAlloc;
    ma.memfree = poolFree;
    ma.userData = (void*)&pool;
    ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.

#else
 
    memset(&ma, 0, sizeof(ma));
    ma.memalloc = stdAlloc;
    ma.memfree = stdFree;
    ma.userData = (void*)&allocated;
    ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.
#endif

    tess = tessNewTess(&ma);
    if (!tess)
        return -1;

    //tessSetOption(tess, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);

    
    //  Create the contour vertex arrays
    
      int iir, ip;

//      Get max number number of points(vertices) in any contour
      int npta  = m_cntr[0];
      npta += 2;                            // fluff

      for( iir=0 ; iir < m_ncnt-1 ; iir++){
            int nptr = m_cntr[iir+1];
            npta = wxMax(npta, nptr); 
      }


      float *geoPt = (float *)malloc((npta) * 2 * sizeof(float));     // tess input vertex array
      float *ppt = geoPt;


//      Create input structures

//    Exterior Ring
      int npte = m_cntr[0];
      int ptValid = npte;

//  Check and account for winding direction of ring


      double x0, y0, x, y;
      OGRPoint p;

      wxPoint2DDouble *pp = (wxPoint2DDouble *)m_vertexPtrArray[0];
      bool cw = isRingClockwise(pp, m_cntr[0]);
      
      //pp++;       // skip 0?

      if(cw)
      {
            x0 = pp->m_x;
            y0 = pp->m_y;
      }
      else
      {
            x0 = pp[npte-1].m_x;
            y0 = pp[npte-1].m_y;
      }



//  Transcribe points to vertex array, in proper order with no duplicates
//   also, accounting for tess_orient
      for(ip = 0 ; ip < npte ; ip++)
      {
            int pidx;
            if(cw)
                  pidx = npte - ip - 1;

            else
                  pidx = ip;

            x = pp[pidx].m_x;
            y = pp[pidx].m_y;

            if((fabs(x-x0) > EQUAL_EPS) || (fabs(y-y0) > EQUAL_EPS))
            {
                  if(m_tess_orient == TESS_VERT)
                  {
                        *ppt++ = x;
                        *ppt++ = y;
                  }
                  else
                  {
                        *ppt++ = y;
                        *ppt++ = x;
                  }
            }
            else
                  ptValid--;

            x0 = x;
            y0 = y;
      }


      for(int i = 0 ; i < ptValid ; i++){
          double x = geoPt[i *2];
          double y = geoPt[i*2 + 1];
//          printf("%g  %g\n", x, y);
      }
//      printf("\n");
      tessAddContour(tess, 2, geoPt, sizeof(float)*2, ptValid);

#if 1
//  Now the interior contours
      for(iir=0; iir < m_ncnt-1; iir++)
      {
            ppt = geoPt;
            wxPoint2DDouble *pp = (wxPoint2DDouble *)m_vertexPtrArray[iir + 1];

            int npti  = m_cntr[iir+1];
            ptValid = npti;
            
      //  Check and account for winding direction of ring
            bool cw = isRingClockwise(pp, m_cntr[iir+1]);

            if(!cw)
            {
                  x0 = pp[0].m_x;
                  y0 = pp[0].m_y;
            }
            else
            {
                  x0 = pp[npti-1].m_x;
                  y0 = pp[npti-1].m_y;
            }

//  Transcribe points to vertex array, in proper order with no duplicates
//   also, accounting for tess_orient
            for(int ip = 0 ; ip < npti ; ip++)
            {
                  OGRPoint p;
                  int pidx;
                  if(!cw)                               // interior contours must be cw
                        pidx = npti - ip - 1;
                  else
                        pidx = ip;


                  x = pp[pidx].m_x;
                  y = pp[pidx].m_y;

                  if((fabs(x-x0) > EQUAL_EPS) || (fabs(y-y0) > EQUAL_EPS))
                  {
                        if(m_tess_orient == TESS_VERT)
                        {
                              *ppt++ = x;
                              *ppt++ = y;
                        }
                        else
                        {
                              *ppt++ = y;
                              *ppt++ = x;
                        }
                  }
                  else
                        ptValid--;

                  x0 = x;
                  y0 = y;

            }
            
            tessAddContour(tess, 2, geoPt, sizeof(float)*2, ptValid);

      }
#endif


      //      Ready to kick off the tesselator

      TriPrim *pTPG_Last = NULL;
      TriPrim *pTPG_Head = NULL;

      s_nvmax = 0;

      if (!tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0))
        return -1;


    //  Tesselation all done, so...
    //  Create the data structures
    
    //  Make a linked list of TriPrims from tess output arrays
    
    const float* verts = tessGetVertices(tess);
//    const int* vinds = tessGetVertexIndices(tess);
    const int* elems = tessGetElements(tess);
    const int nverts = tessGetVertexCount(tess);
    const int nelems = tessGetElementCount(tess);

    m_nvertex_max = nverts;               // record largest vertex count

    for (int i = 0; i < nelems; ++i){
        const int* p = &elems[i*nvp];

        TriPrim *pTPG = new TriPrim;
        if(NULL == pTPG_Last){
                pTPG_Head = pTPG;
                pTPG_Last = pTPG;
        }
        else{
                pTPG_Last->p_next = pTPG;
                pTPG_Last = pTPG;
        }

        pTPG->p_next = NULL;
        pTPG->type = GL_TRIANGLES;
        pTPG->nVert = nvp;
        
        pTPG->p_vertex = (double *)malloc(nvp * 2 * sizeof(double));
        GLdouble *pdd = (GLdouble*)pTPG->p_vertex;

        //  Preset LLBox bounding box limits
        double sxmax = -1000;
        double sxmin = 1000;
        double symax = -90;
        double symin = 90;
        


        for (size_t j = 0; j < nvp && p[j] != TESS_UNDEF; ++j){
            double yd = verts[p[j]*2];
            double xd = verts[p[j]*2+1];
            
            // Calculate LLBox bounding box for each Tri-prim
            if(m_bmerc_transform){
                double valx = ( xd * mx_rate ) + mx_offset;
                double valy = ( yd * my_rate ) + my_offset;
 
                       //    quickly convert to lat/lon
                double lat = ( 2.0 * atan ( exp ( valy/CM93_semimajor_axis_meters ) ) - PI/2. ) / DEGREE;
                double lon= ( valx / ( DEGREE * CM93_semimajor_axis_meters ) );


                sxmax = wxMax(lon, sxmax);
                sxmin = wxMin(lon, sxmin);
                symax = wxMax(lat, symax);
                symin = wxMin(lat, symin);
            }
            else{
                sxmax = wxMax(xd, sxmax);
                sxmin = wxMin(xd, sxmin);
                symax = wxMax(yd, symax);
                symin = wxMin(yd, symin);
            }
            
            //  Append this point to TriPrim, converting to SM if called for

            if(m_b_senc_sm){
                double easting, northing;
                toSM(yd, xd, m_ref_lat, m_ref_lon, &easting, &northing);
                *pdd++ = easting;
                *pdd++ = northing;
            }
            else{
                *pdd++ = xd;
                *pdd++ = yd;
            }
        }
       
       pTPG->tri_box.Set(symin, sxmin, symax, sxmax);
    }
    
   
    
      m_ppg_head = new PolyTriGroup;
      m_ppg_head->m_bSMSENC = s_bSENC_SM;

      m_ppg_head->nContours = m_ncnt;
      m_ppg_head->pn_vertex = m_cntr;             // pointer to array of poly vertex counts
      m_ppg_head->data_type = DATA_TYPE_DOUBLE;
      

//  Transcribe the raw geometry buffer
//  Converting to float as we go, and
//  allowing for tess_orient
//  Also, convert to SM if requested

      int nptfinal = npta;
      
      //  No longer need the full geometry in the SENC,
      nptfinal = 1;
      
      m_nwkb = (nptfinal +1) * 2 * sizeof(float);
      m_ppg_head->pgroup_geom = (float *)malloc(m_nwkb);
      float *vro = m_ppg_head->pgroup_geom;
      ppt = geoPt;
      float tx,ty;

      for(ip = 0 ; ip < nptfinal ; ip++)
      {
            if(TESS_HORZ == m_tess_orient)
            {
                  ty = *ppt++;
                  tx = *ppt++;
            }
            else
            {
                  tx = *ppt++;
                  ty = *ppt++;
            }

            if(m_b_senc_sm)
            {
            //  Calculate SM from chart common reference point
                  double easting, northing;
                  toSM(ty, tx, 0/*ref_lat*/, 0/*ref_lon*/, &easting, &northing);
                  *vro++ = easting;              // x
                  *vro++ = northing;             // y
            }
            else
            {
                  *vro++ = tx;                  // lon
                  *vro++ = ty;                  // lat
            }

            ppt++;                      // skip z
      }

      m_ppg_head->tri_prim_head = pTPG_Head;         // head of linked list of TriPrims

      //  Convert the Triangle vertex arrays into a single memory allocation of floats
      //  to reduce SENC size and enable efficient access later
      
      //  First calculate the total byte size
      int total_byte_size = 2 * sizeof(float);
      TriPrim *p_tp = m_ppg_head->tri_prim_head;
      while( p_tp ) {
          total_byte_size += p_tp->nVert * 2 * sizeof(float);
          p_tp = p_tp->p_next; // pick up the next in chain
      }
      
      float *vbuf = (float *)malloc(total_byte_size);
      p_tp = m_ppg_head->tri_prim_head;
      float *p_run = vbuf;
      while( p_tp ) {
          float *pfbuf = p_run;
          GLdouble *pdouble_buf = (GLdouble *)p_tp->p_vertex;
          
          for( int i=0 ; i < p_tp->nVert * 2 ; ++i){
              float x = (float)( *((GLdouble *)pdouble_buf) );
              *p_run++ = x;
              pdouble_buf++;
          }
          
          free(p_tp->p_vertex);
          p_tp->p_vertex = (double *)pfbuf;
          p_tp = p_tp->p_next; // pick up the next in chain
      }
      m_ppg_head->bsingle_alloc = true;
      m_ppg_head->single_buffer = (unsigned char *)vbuf;
      m_ppg_head->single_buffer_size = total_byte_size;
      m_ppg_head->data_type = DATA_TYPE_FLOAT;
      

      free (geoPt);

      //    All allocated buffers are owned now by the m_ppg_head
      //    And will be freed on dtor of this object
 
      if (tess) tessDeleteTess(tess);

      m_bOK = true;
      
      return 0;
}


int PolyTessGeo::BuildTessGL2(void)
{
#ifdef ocpnUSE_GL
    
      int iir, ip;
      int *cntr;
      GLdouble *geoPt;

      wxString    sout;
      wxString    sout1;
      wxString    stemp;


#ifdef __WXMSW__
//  If using the OpenGL dlls provided with Windows,
//  load the dll and establish addresses of the entry points needed

#ifdef USE_GLU_DLL

      if(!s_glu_dll_ready)
      {


            s_hGLU_DLL = LoadLibrary("glu32.dll");
            if (s_hGLU_DLL != NULL)
            {
                  s_lpfnTessProperty = (LPFNDLLTESSPROPERTY)GetProcAddress(s_hGLU_DLL,"gluTessProperty");
                  s_lpfnNewTess = (LPFNDLLNEWTESS)GetProcAddress(s_hGLU_DLL, "gluNewTess");
                  s_lpfnTessBeginContour = (LPFNDLLTESSBEGINCONTOUR)GetProcAddress(s_hGLU_DLL, "gluTessBeginContour");
                  s_lpfnTessEndContour = (LPFNDLLTESSENDCONTOUR)GetProcAddress(s_hGLU_DLL, "gluTessEndContour");
                  s_lpfnTessBeginPolygon = (LPFNDLLTESSBEGINPOLYGON)GetProcAddress(s_hGLU_DLL, "gluTessBeginPolygon");
                  s_lpfnTessEndPolygon = (LPFNDLLTESSENDPOLYGON)GetProcAddress(s_hGLU_DLL, "gluTessEndPolygon");
                  s_lpfnDeleteTess = (LPFNDLLDELETETESS)GetProcAddress(s_hGLU_DLL, "gluDeleteTess");
                  s_lpfnTessVertex = (LPFNDLLTESSVERTEX)GetProcAddress(s_hGLU_DLL, "gluTessVertex");
                  s_lpfnTessCallback = (LPFNDLLTESSCALLBACK)GetProcAddress(s_hGLU_DLL, "gluTessCallback");

                  s_glu_dll_ready = true;
            }
            else
            {
                  return ERROR_NO_DLL;
            }
      }

#endif
#endif


    //  Allocate a work buffer, which will be grown as needed
#define NINIT_BUFFER_LEN 10000
      s_pwork_buf = (GLdouble *)malloc(NINIT_BUFFER_LEN * 2 * sizeof(GLdouble));
      s_buf_len = NINIT_BUFFER_LEN * 2;
      s_buf_idx = 0;

      //    Create an array to hold pointers to allocated vertices created by "combine" callback,
      //    so that they may be deleted after tesselation.
      s_pCombineVertexArray = new wxArrayPtrVoid;

    //  Create tesselator
      GLUtessobj = gluNewTess();

    //  Register the callbacks
      gluTessCallback(GLUtessobj, GLU_TESS_BEGIN,   (GLvoid (__CALL_CONVENTION *) ())&beginCallback);
      gluTessCallback(GLUtessobj, GLU_TESS_BEGIN,   (GLvoid (__CALL_CONVENTION *) ())&beginCallback);
      gluTessCallback(GLUtessobj, GLU_TESS_VERTEX,  (GLvoid (__CALL_CONVENTION *) ())&vertexCallback);
      gluTessCallback(GLUtessobj, GLU_TESS_END,     (GLvoid (__CALL_CONVENTION *) ())&endCallback);
      gluTessCallback(GLUtessobj, GLU_TESS_COMBINE, (GLvoid (__CALL_CONVENTION *) ())&combineCallback);

//    gluTessCallback(GLUtessobj, GLU_TESS_ERROR,   (GLvoid (__CALL_CONVENTION *) ())&errorCallback);

//    glShadeModel(GL_SMOOTH);
      gluTessProperty(GLUtessobj, GLU_TESS_WINDING_RULE,
                      GLU_TESS_WINDING_POSITIVE );

    //  gluTess algorithm internally selects vertically oriented triangle strips and fans.
    //  This orientation is not optimal for conventional memory-mapped raster display shape filling.
    //  We can "fool" the algorithm by interchanging the x and y vertex values passed to gluTessVertex
    //  and then reverting x and y on the resulting vertexCallbacks.
    //  In this implementation, we will explicitely set the preferred orientation.

    //Set the preferred orientation
      s_tess_orient = TESS_HORZ;                    // prefer horizontal tristrips



//      Get total number of contours
      m_ncnt  = m_pxgeom->n_contours;

//      Allocate cntr array
      cntr = (int *)malloc(m_ncnt * sizeof(int));

//      Get total number of points(vertices)
      int npta  = m_pxgeom->contour_array[0];
      cntr[0] = npta;
      npta += 2;                            // fluff

      for( iir=0 ; iir < m_ncnt-1 ; iir++)
      {
            int nptr = m_pxgeom->contour_array[iir+1];
            cntr[iir+1] = nptr;

            npta += nptr + 2;             // fluff
      }



//      printf("xgeom npta: %d\n", npta);
      geoPt = (GLdouble *)malloc((npta) * 3 * sizeof(GLdouble));     // vertex array



   //  Grow the work buffer if necessary

      if((npta * 4) > s_buf_len)
      {
            s_pwork_buf = (GLdouble *)realloc(s_pwork_buf, npta * 4 * sizeof(GLdouble));
            s_buf_len = npta * 4;
      }


//  Define the polygon
      gluTessBeginPolygon(GLUtessobj, NULL);


//      Create input structures

//    Exterior Ring
      int npte = m_pxgeom->contour_array[0];
      cntr[0] = npte;

      GLdouble *ppt = geoPt;


//  Check and account for winding direction of ring
      bool cw = true;

      double x0, y0, x, y;
      OGRPoint p;

      wxPoint2DDouble *pp = m_pxgeom->vertex_array;
      pp++;       // skip 0?

      if(cw)
      {
//            poly->getExteriorRing()->getPoint(0, &p);
            x0 = pp->m_x;
            y0 = pp->m_y;
      }
      else
      {
//            poly->getExteriorRing()->getPoint(npte-1, &p);
            x0 = pp[npte-1].m_x;
            y0 = pp[npte-1].m_y;
//            x0 = p.getX();
//            y0 = p.getY();
      }

//      pp++;

      gluTessBeginContour(GLUtessobj);

//  Transcribe points to vertex array, in proper order with no duplicates
//   also, accounting for tess_orient
      for(ip = 0 ; ip < npte ; ip++)
      {
            int pidx;
            if(cw)
                  pidx = npte - ip - 1;

            else
                  pidx = ip;

//            poly->getExteriorRing()->getPoint(pidx, &p);
            x = pp[pidx].m_x;
            y = pp[pidx].m_y;

//            pp++;

            if((fabs(x-x0) > EQUAL_EPS) || (fabs(y-y0) > EQUAL_EPS))
            {
                  GLdouble *ppt_temp = ppt;
                  if(s_tess_orient == TESS_VERT)
                  {
                        *ppt++ = x;
                        *ppt++ = y;
                  }
                  else
                  {
                        *ppt++ = y;
                        *ppt++ = x;
                  }

                  *ppt++ = 0.0;

                  gluTessVertex( GLUtessobj, ppt_temp, ppt_temp ) ;
 //printf("tess from xgeom, external vertex %g %g\n", x, y);

            }
            else
                  cntr[0]--;

            x0 = x;
            y0 = y;
      }

      gluTessEndContour(GLUtessobj);


      int index_offset = npte;
#if 1
//  Now the interior contours
      for(iir=0; iir < m_ncnt-1; iir++)
      {
            gluTessBeginContour(GLUtessobj);

//            int npti = cntr[iir];
            int npti  = m_pxgeom->contour_array[iir+1];

      //  Check and account for winding direction of ring
            bool cw = false; //!(poly->getInteriorRing(iir)->isClockwise() == 0);

            if(!cw)
            {
                  x0 = pp[index_offset].m_x;
                  y0 = pp[index_offset].m_y;
            }
            else
            {
                  x0 = pp[index_offset + npti-1].m_x;
                  y0 = pp[index_offset + npti-1].m_y;
            }

//  Transcribe points to vertex array, in proper order with no duplicates
//   also, accounting for tess_orient
            for(int ip = 0 ; ip < npti ; ip++)
            {
                  OGRPoint p;
                  int pidx;
                  if(!cw)                               // interior contours must be cw
                        pidx = npti - ip - 1;
                  else
                        pidx = ip;


                  x = pp[pidx + index_offset].m_x;
                  y = pp[pidx + index_offset].m_y;

                  if((fabs(x-x0) > EQUAL_EPS) || (fabs(y-y0) > EQUAL_EPS))
                  {
                        GLdouble *ppt_temp = ppt;
                        if(s_tess_orient == TESS_VERT)
                        {
                              *ppt++ = x;
                              *ppt++ = y;
                        }
                        else
                        {
                              *ppt++ = y;
                              *ppt++ = x;
                        }
                        *ppt++ = 0.0;

                        gluTessVertex( GLUtessobj, ppt_temp, ppt_temp ) ;
//      printf("tess from xgeom, internal vertex %d %g %g\n", ip, x, y);

                  }
                  else
                        cntr[iir+1]--;

                  x0 = x;
                  y0 = y;

            }

            gluTessEndContour(GLUtessobj);

            index_offset += npti;
      }
#endif

    //  Store some SM conversion data in static store,
    //  for callback access
      s_bSENC_SM =  false;

      s_bmerc_transform = true;
      s_transform_x_rate   =  m_pxgeom->x_rate;
      s_transform_x_origin =  m_pxgeom->x_offset;
      s_transform_y_rate   =  m_pxgeom->y_rate;
      s_transform_y_origin =  m_pxgeom->y_offset;


    //      Ready to kick off the tesselator

      s_pTPG_Last = NULL;
      s_pTPG_Head = NULL;

      s_nvmax = 0;

      gluTessEndPolygon(GLUtessobj);          // here it goes

      m_nvertex_max = s_nvmax;               // record largest vertex count, updates in callback


    //  Tesselation all done, so...

    //  Create the data structures

      m_ppg_head = new PolyTriGroup;
      m_ppg_head->m_bSMSENC = s_bSENC_SM;

      m_ppg_head->nContours = m_ncnt;
      m_ppg_head->pn_vertex = cntr;             // pointer to array of poly vertex counts
      m_ppg_head->data_type = DATA_TYPE_DOUBLE;
      

//  Transcribe the raw geometry buffer
//  Converting to float as we go, and
//  allowing for tess_orient
//  Also, convert to SM if requested

      int nptfinal = npta;
      
      //  No longer need the full geometry in the SENC,
      nptfinal = 1;
      
      m_nwkb = (nptfinal +1) * 2 * sizeof(float);
      m_ppg_head->pgroup_geom = (float *)malloc(m_nwkb);
      float *vro = m_ppg_head->pgroup_geom;
      ppt = geoPt;
      float tx,ty;

      for(ip = 0 ; ip < nptfinal ; ip++)
      {
            if(TESS_HORZ == s_tess_orient)
            {
                  ty = *ppt++;
                  tx = *ppt++;
            }
            else
            {
                  tx = *ppt++;
                  ty = *ppt++;
            }

            if(0/*bSENC_SM*/)
            {
            //  Calculate SM from chart common reference point
                  double easting, northing;
//                  toSM(ty, tx, ref_lat, ref_lon, &easting, &northing);
                  *vro++ = easting;              // x
                  *vro++ = northing;             // y
            }
            else
            {
                  *vro++ = tx;                  // lon
                  *vro++ = ty;                  // lat
            }

            ppt++;                      // skip z
      }

      m_ppg_head->tri_prim_head = s_pTPG_Head;         // head of linked list of TriPrims

      //  Convert the Triangle vertex arrays into a single memory allocation of floats
      //  to reduce SENC size and enable efficient access later
      
      //  First calculate the total byte size
      int total_byte_size = 2 * sizeof(float);
      TriPrim *p_tp = m_ppg_head->tri_prim_head;
      while( p_tp ) {
          total_byte_size += p_tp->nVert * 2 * sizeof(float);
          p_tp = p_tp->p_next; // pick up the next in chain
      }
      
      float *vbuf = (float *)malloc(total_byte_size);
      p_tp = m_ppg_head->tri_prim_head;
      float *p_run = vbuf;
      while( p_tp ) {
          float *pfbuf = p_run;
          GLdouble *pdouble_buf = (GLdouble *)p_tp->p_vertex;
          
          for( int i=0 ; i < p_tp->nVert * 2 ; ++i){
              float x = (float)( *((GLdouble *)pdouble_buf) );
              *p_run++ = x;
              pdouble_buf++;
          }
          
          free(p_tp->p_vertex);
          p_tp->p_vertex = (double *)pfbuf;
          p_tp = p_tp->p_next; // pick up the next in chain
      }
      m_ppg_head->bsingle_alloc = true;
      m_ppg_head->single_buffer = (unsigned char *)vbuf;
      m_ppg_head->single_buffer_size = total_byte_size;
      m_ppg_head->data_type = DATA_TYPE_FLOAT;
      
      gluDeleteTess(GLUtessobj);

      free( s_pwork_buf );
      s_pwork_buf = NULL;

      free (geoPt);

      //    All allocated buffers are owned now by the m_ppg_head
      //    And will be freed on dtor of this object
      delete m_pxgeom;

      //      Free up any "Combine" vertices created
      for(unsigned int i = 0; i < s_pCombineVertexArray->GetCount() ; i++)
            free (s_pCombineVertexArray->Item(i));
      delete s_pCombineVertexArray;


      m_pxgeom = NULL;

      m_bOK = true;

#endif          //#ifdef ocpnUSE_GL
      
      return 0;
}





// GLU tesselation support functions
void __CALL_CONVENTION beginCallback(GLenum which)
{
    s_buf_idx = 0;
    s_nvcall = 0;
    s_gltri_type = which;
}

/*
void __CALL_CONVENTION errorCallback(GLenum errorCode)
{
    const GLubyte *estring;

    estring = gluErrorString(errorCode);
    printf("Tessellation Error: %s\n", estring);
    exit(0);
}
*/

void __CALL_CONVENTION endCallback(void)
{
    //      Create a TriPrim

    char buf[40];

    if(s_nvcall > s_nvmax)                            // keep track of largest number of triangle vertices
          s_nvmax = s_nvcall;

    switch(s_gltri_type)
    {
        case GL_TRIANGLE_FAN:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLES:
        {
            TriPrim *pTPG = new TriPrim;
            if(NULL == s_pTPG_Last)
            {
                s_pTPG_Head = pTPG;
                s_pTPG_Last = pTPG;
            }
            else
            {
                s_pTPG_Last->p_next = pTPG;
                s_pTPG_Last = pTPG;
            }

            pTPG->p_next = NULL;
            pTPG->type = s_gltri_type;
            pTPG->nVert = s_nvcall;

        //  Calculate bounding box
            float sxmax = -1000;                   // this poly BBox
            float sxmin = 1000;
            float symax = -90;
            float symin = 90;

            GLdouble *pvr = s_pwork_buf;
            for(int iv=0 ; iv < s_nvcall ; iv++)
            {
                GLdouble xd, yd;
                xd = *pvr++;
                yd = *pvr++;

                if(s_bmerc_transform)
                {
                      double valx = ( xd * s_transform_x_rate ) + s_transform_x_origin;
                      double valy = ( yd * s_transform_y_rate ) + s_transform_y_origin;

                      //    Convert to lat/lon
                      double lat = ( 2.0 * atan ( exp ( valy/CM93_semimajor_axis_meters ) ) - PI/2. ) / DEGREE;
                      double lon = ( valx / ( DEGREE * CM93_semimajor_axis_meters ) );

                      sxmax = wxMax(lon, sxmax);
                      sxmin = wxMin(lon, sxmin);
                      symax = wxMax(lat, symax);
                      symin = wxMin(lat, symin);
                }
                else
                {
                      sxmax = wxMax(xd, sxmax);
                      sxmin = wxMin(xd, sxmin);
                      symax = wxMax(yd, symax);
                      symin = wxMin(yd, symin);
                }
            }

            pTPG->tri_box.Set(symin, sxmin, symax, sxmax);

            //  Transcribe this geometry to TriPrim, converting to SM if called for

            if(s_bSENC_SM)
            {
                GLdouble *pds = s_pwork_buf;
                pTPG->p_vertex = (double *)malloc(s_nvcall * 2 * sizeof(double));
                GLdouble *pdd = (GLdouble*)pTPG->p_vertex;

                for(int ip = 0 ; ip < s_nvcall ; ip++)
                {
                    double dlon = *pds++;
                    double dlat = *pds++;

                    double easting, northing;
                    toSM(dlat, dlon, s_ref_lat, s_ref_lon, &easting, &northing);
                    *pdd++ = easting;
                    *pdd++ = northing;
                }
            }
            else
            {
                pTPG->p_vertex = (double *)malloc(s_nvcall * 2 * sizeof(double));
                memcpy(pTPG->p_vertex, s_pwork_buf, s_nvcall * 2 * sizeof(double));
            }


            break;
        }
        default:
        {
            sprintf(buf, "....begin Callback  unknown\n");
            break;
        }
    }
}

void __CALL_CONVENTION vertexCallback(GLvoid *vertex)
{
    GLdouble *pointer;

    pointer = (GLdouble *) vertex;

    if(s_buf_idx > s_buf_len - 4)
    {
        int new_buf_len = s_buf_len + 100;
        GLdouble * tmp = s_pwork_buf;

        s_pwork_buf = (GLdouble *)realloc(s_pwork_buf, new_buf_len * sizeof(GLdouble));
        if (NULL == s_pwork_buf)
        {
            free(tmp);
            tmp = NULL;
        }
        else
            s_buf_len = new_buf_len;
    }

    if(s_tess_orient == TESS_VERT)
    {
        s_pwork_buf[s_buf_idx++] = pointer[0];
        s_pwork_buf[s_buf_idx++] = pointer[1];
    }
    else
    {
        s_pwork_buf[s_buf_idx++] = pointer[1];
        s_pwork_buf[s_buf_idx++] = pointer[0];
    }


    s_nvcall++;

}

/*  combineCallback is used to create a new vertex when edges
 *  intersect.
 */
void __CALL_CONVENTION combineCallback(GLdouble coords[3],
                     GLdouble *vertex_data[4],
                     GLfloat weight[4], GLdouble **dataOut )
{
    GLdouble *vertex = (GLdouble *)malloc(6 * sizeof(GLdouble));

    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    vertex[3] = vertex[4] = vertex[5] = 0. ; //01/13/05 bugfix

    *dataOut = vertex;

    s_pCombineVertexArray->Add(vertex);
}



wxStopWatch *s_stwt;


//      Build Trapezoidal PolyTessGeoTrap Object from Extended_Geometry
PolyTessGeoTrap::PolyTessGeoTrap(Extended_Geometry *pxGeom)
{
      m_bOK = false;

      m_ptg_head = new PolyTrapGroup(pxGeom);
      m_nvertex_max = pxGeom->n_max_vertex;           // record the maximum number of segment vertices

      //    All allocated buffers are owned now by the m_ptg_head
      //    And will be freed on dtor of this object
      delete pxGeom;
}


PolyTessGeoTrap::~PolyTessGeoTrap()
{
      delete m_ptg_head;
}




//------------------------------------------------------------------------------
//          PolyTriGroup Implementation
//------------------------------------------------------------------------------
PolyTriGroup::PolyTriGroup()
{
    pn_vertex = NULL;             // pointer to array of poly vertex counts
    pgroup_geom = NULL;           // pointer to Raw geometry, used for contour line drawing
    tri_prim_head = NULL;         // head of linked list of TriPrims
    m_bSMSENC = false;
    bsingle_alloc = false;
    single_buffer = NULL;
    single_buffer_size = 0;
    data_type = DATA_TYPE_DOUBLE;
    

}

PolyTriGroup::~PolyTriGroup()
{
    free(pn_vertex);
    free(pgroup_geom);
    //Walk the list of TriPrims, deleting as we go
    TriPrim *tp_next;
    TriPrim *tp = tri_prim_head;
    
    if(bsingle_alloc){
        free(single_buffer);
        while(tp) {
            tp_next = tp->p_next;
            delete tp;
            tp = tp_next;
        }
    }
    else {
        while(tp) {
            tp_next = tp->p_next;
            tp->FreeMem();
            delete tp;
            tp = tp_next;
        }
    }
}

//------------------------------------------------------------------------------
//          TriPrim Implementation
//------------------------------------------------------------------------------
TriPrim::TriPrim()
{
}

TriPrim::~TriPrim()
{
}

void TriPrim::FreeMem()
{
    free(p_vertex);
}


//------------------------------------------------------------------------------
//          PolyTrapGroup Implementation
//------------------------------------------------------------------------------
PolyTrapGroup::PolyTrapGroup()
{
      pn_vertex = NULL;             // pointer to array of poly vertex counts
      ptrapgroup_geom = NULL;           // pointer to Raw geometry, used for contour line drawing
      trap_array = NULL;            // pointer to trapz_t array

      ntrap_count = 0;
}

PolyTrapGroup::PolyTrapGroup(Extended_Geometry *pxGeom)
{
      m_trap_error = 0;

      nContours = pxGeom->n_contours;

      pn_vertex = pxGeom->contour_array;             // pointer to array of poly vertex counts
      pxGeom->contour_array = NULL;

      ptrapgroup_geom = pxGeom->vertex_array;
      pxGeom->vertex_array = NULL;

      ntrap_count = 0;                                // provisional
      trap_array = NULL;                              // pointer to generated trapz_t array
}

PolyTrapGroup::~PolyTrapGroup()
{
      free(pn_vertex);
      free(ptrapgroup_geom);
      free (trap_array);
}






