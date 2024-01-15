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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#if defined(__OCPN__ANDROID__)
 #include <qopengl.h>
 #include <GL/gl_private.h>  // this is a cut-down version of gl.h
 #include <GLES2/gl2.h>
#elif defined(__MSVC__)
 #include "glew.h"
#elif defined(__WXOSX__)
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 typedef void (*  _GLUfuncptr)();
 #define GL_COMPRESSED_RGB_FXT1_3DFX       0x86B0
#elif defined(__WXQT__) || defined(__WXGTK__)
 #include <GL/glew.h>
 #include <GL/glu.h>
#endif

#include "wx/tokenzr.h"
#include <wx/mstream.h>

#include "gdal/ogr_geometry.h"

#include "model/cutil.h"

#include "vector2D.h"

#include "s52s57.h"

#include "mygeom.h"
#include "model/georef.h"
#include "LOD_reduce.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tesselator.h"
#include "Striper.h"

#ifdef __WXMSW__
#include <windows.h>
#endif

static const double CM93_semimajor_axis_meters =
    6378388.0;  // CM93 semimajor axis

//      Module Internal Prototypes

#if defined(__UNIX__) && \
    !defined(__WXOSX__)  // high resolution stopwatch for profiling
class OCPNStopWatchTess {
public:
  OCPNStopWatchTess() { Reset(); }
  void Reset() { clock_gettime(CLOCK_REALTIME, &tp); }

  double GetTime() {
    timespec tp_end;
    clock_gettime(CLOCK_REALTIME, &tp_end);
    return (tp_end.tv_sec - tp.tv_sec) * 1.e3 +
           (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
  }

private:
  timespec tp;
};
#endif

//-----------------------------------------------------------
//
//  Static memory allocators for libtess2
//
//-----------------------------------------------------------
//#define USE_POOL 1

void *stdAlloc(void *userData, unsigned int size) {
  int *allocated = (int *)userData;
  TESS_NOTUSED(userData);
  *allocated += (int)size;
  return malloc(size);
}

void stdFree(void *userData, void *ptr) {
  TESS_NOTUSED(userData);
  free(ptr);
}

struct MemPool {
  unsigned char *buf;
  unsigned int cap;
  unsigned int size;
};

void *poolAlloc(void *userData, unsigned int size) {
  struct MemPool *pool = (struct MemPool *)userData;
  size = (size + 0x7) & ~0x7;
  if (pool->size + size < pool->cap) {
    unsigned char *ptr = pool->buf + pool->size;
    pool->size += size;
    return ptr;
  }
  printf("out of mem: %d < %d!\n", pool->size + size, pool->cap);
  return 0;
}

void poolFree(void *userData, void *ptr) {
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

  for (int iVert = 0; iVert < nPointCount - 1; iVert++) {
    dfSum +=
        pp[iVert].m_x * pp[iVert + 1].m_y - pp[iVert].m_y * pp[iVert + 1].m_x;
  }

  dfSum +=
      pp[nPointCount - 1].m_x * pp[0].m_y - pp[nPointCount - 1].m_y * pp[0].m_x;

  return dfSum < 0.0;
}

//------------------------------------------------------------------------------
//          Extended_Geometry Implementation
//------------------------------------------------------------------------------
Extended_Geometry::Extended_Geometry() {
  vertex_array = NULL;
  contour_array = NULL;
}

Extended_Geometry::~Extended_Geometry() {
  free(vertex_array);
  free(contour_array);
}

//------------------------------------------------------------------------------
//          PolyTessGeo Implementation
//------------------------------------------------------------------------------
PolyTessGeo::PolyTessGeo() {
  m_pxgeom = NULL;
  m_printStats = false;
  m_bstripify = false;
  m_LOD_meters = 0;
}

//      Build PolyTessGeo Object from Extended_Geometry
PolyTessGeo::PolyTessGeo(Extended_Geometry *pxGeom) {
  m_ppg_head = NULL;
  m_bOK = false;

  m_pxgeom = pxGeom;
  m_printStats = false;
  m_bstripify = false;
  m_LOD_meters = 0;
}

//      Build PolyTessGeo Object from OGR Polygon
PolyTessGeo::PolyTessGeo(OGRPolygon *poly, bool bSENC_SM, double ref_lat,
                         double ref_lon, double LOD_meters) {
  m_printStats = false;

  ErrorCode = 0;
  m_ppg_head = NULL;
  m_pxgeom = NULL;
  m_tess_orient = TESS_HORZ;

  m_ref_lat = ref_lat;
  m_ref_lon = ref_lon;
  m_LOD_meters = LOD_meters;
  m_b_senc_sm = bSENC_SM;
  m_bmerc_transform = false;

  //    PolyGeo BBox as lat/lon
  OGREnvelope Envelope;
  poly->getEnvelope(&Envelope);
  xmin = Envelope.MinX;
  ymin = Envelope.MinY;
  xmax = Envelope.MaxX;
  ymax = Envelope.MaxY;

  m_feature_ref_lat = ymin + (ymax - ymin) / 2;
  m_feature_ref_lon = xmin + (xmax - xmin) / 2;

  toSM(m_feature_ref_lat, m_feature_ref_lon, ref_lat, ref_lon,
       &m_feature_easting, &m_feature_northing);

  //  Build the array of contour point counts
  //      Get total number of contours
  m_ncnt = poly->getNumInteriorRings() + 1;

  // build the contour point countarray
  m_cntr = (int *)malloc(m_ncnt * sizeof(int));

  m_cntr[0] = poly->getExteriorRing()->getNumPoints();

  for (int i = 1; i < m_ncnt; i++) {
    m_cntr[i] = poly->getInteriorRing(i - 1)->getNumPoints();
  }

  //  Build the point array index table

  OGRPoint p;
  m_vertexPtrArray = (double **)malloc(m_ncnt * sizeof(double *));

  m_vertexPtrArray[0] = (double *)malloc(m_cntr[0] * sizeof(double) * 2);
  double *pp = m_vertexPtrArray[0];
  for (int i = 0; i < m_cntr[0]; i++) {
    poly->getExteriorRing()->getPoint(i, &p);

    //  Calculate SM from feature reference point
    double easting, northing;
    toSM(p.getY(), p.getX(), m_feature_ref_lat, m_feature_ref_lon, &easting,
         &northing);
    *pp++ = easting;   // x
    *pp++ = northing;  // y
  }

  for (int i = 1; i < m_ncnt; i++) {
    m_vertexPtrArray[i] = (double *)malloc(m_cntr[i] * sizeof(double) * 2);
    double *pp = m_vertexPtrArray[i];
    for (int j = 0; j < m_cntr[i]; j++) {
      poly->getInteriorRing(i - 1)->getPoint(j, &p);

      double easting, northing;
      toSM(p.getY(), p.getX(), m_feature_ref_lat, m_feature_ref_lon, &easting,
           &northing);
      *pp++ = easting;   // x
      *pp++ = northing;  // y
    }
  }

  mx_rate = 1.0;
  mx_offset = 0.0;
  my_rate = 1.0;
  my_offset = 0.0;

  m_bstripify = true;

  earthAxis = CM93_semimajor_axis_meters * mercator_k0;

  m_bcm93 = false;

  BuildTessGLU();

  // Free the working memory
  for (int i = 0; i < m_ncnt; i++) free(m_vertexPtrArray[i]);

  free(m_vertexPtrArray);
  m_vertexPtrArray = nullptr;
}

void PolyTessGeo::SetExtents(double x_left, double y_bot, double x_right,
                             double y_top) {
  xmin = x_left;
  ymin = y_bot;
  xmax = x_right;
  ymax = y_top;
}

PolyTessGeo::~PolyTessGeo() {
  delete m_ppg_head;
  delete m_pxgeom;
}

int PolyTessGeo::BuildDeferredTess(void) {
  if (m_pxgeom) {
    // For cm93
    m_b_senc_sm = false;
    m_bmerc_transform = true;
    m_tess_orient = TESS_HORZ;  // prefer horizontal tristrips

    //      Get total number of contours
    m_ncnt = m_pxgeom->n_contours;

    // build the contour point countarray
    m_cntr = (int *)malloc(m_ncnt * sizeof(int));

    for (int i = 0; i < m_ncnt; i++) {
      m_cntr[i] = m_pxgeom->contour_array[i];
    }

    //  Build the point array index table

    m_vertexPtrArray = (double **)malloc(m_ncnt * sizeof(double *));

    double *vertex_pointer = (double *)m_pxgeom->vertex_array;
    vertex_pointer += 2;  // skip 0

    for (int k = 0; k < m_ncnt; k++) {
      m_vertexPtrArray[k] = vertex_pointer;
      vertex_pointer += 2 * m_cntr[k];
    }

    mx_rate = m_pxgeom->x_rate;
    mx_offset = m_pxgeom->x_offset;
    my_rate = m_pxgeom->y_rate;
    my_offset = m_pxgeom->y_offset;

    m_feature_easting = 0;
    m_feature_northing = 0;

    m_ref_lat = m_pxgeom->ref_lat;
    m_ref_lon = m_pxgeom->ref_lon;

    //  Note Bene...  cm93 assumes no flattening constant
    earthAxis = CM93_semimajor_axis_meters;

    m_bcm93 = true;

    // For cm93, we prefer TessGLU, since it produces more TriangleStrips, so
    // renders faster with less storage.
    int rv = BuildTessGLU();

    // All done with this geometry
    delete m_pxgeom;
    m_pxgeom = NULL;

    // Free the working memory
    free(m_vertexPtrArray);
    m_vertexPtrArray = nullptr;

    return rv;

  } else
    return 0;
}

void beginCallback(GLenum which, void *polyData);
void errorCallback(GLenum errorCode, void *polyData);
void endCallback(void *polyData);
void vertexCallback(GLvoid *vertex, void *polyData);
void combineCallback(GLdouble coords[3], GLdouble *vertex_data[4],
                     GLfloat weight[4], GLdouble **dataOut, void *polyData);

//      Build PolyTessGeo Object from OGR Polygon
//      Using OpenGL/GLU tesselator
int PolyTessGeo::BuildTessGLU() {
  unsigned int iir, ip;
  GLdouble *geoPt;

#if 0
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

#endif

  //  Allocate a work buffer, which will be grown as needed
#define NINIT_BUFFER_LEN 10000
  m_pwork_buf = (GLdouble *)malloc(NINIT_BUFFER_LEN * 2 * sizeof(GLdouble));
  m_buf_len = NINIT_BUFFER_LEN * 2;
  m_buf_idx = 0;

  //    Create an array to hold pointers to allocated vertices created by
  //    "combine" callback, so that they may be deleted after tesselation.
  m_pCombineVertexArray = new wxArrayPtrVoid;

  //  Create tesselator
  GLUtessobj = gluNewTess();

  //  Register the callbacks
  gluTessCallback(GLUtessobj, GLU_TESS_BEGIN_DATA,
                  (GLvoid(*)()) & beginCallback);
  gluTessCallback(GLUtessobj, GLU_TESS_BEGIN_DATA,
                  (GLvoid(*)()) & beginCallback);
  gluTessCallback(GLUtessobj, GLU_TESS_VERTEX_DATA,
                  (GLvoid(*)()) & vertexCallback);
  gluTessCallback(GLUtessobj, GLU_TESS_END_DATA, (GLvoid(*)()) & endCallback);
  gluTessCallback(GLUtessobj, GLU_TESS_COMBINE_DATA,
                  (GLvoid(*)()) & combineCallback);
  gluTessCallback(GLUtessobj, GLU_TESS_ERROR_DATA,
                  (GLvoid(*)()) & errorCallback);

  gluTessProperty(GLUtessobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
  gluTessNormal(GLUtessobj, 0, 0, 1);

  //      Get total number of points(vertices)  to build a buffer
  int npta = 0;

  for (int i = 0; i < m_ncnt; i++) npta += m_cntr[i];

  geoPt =
      (GLdouble *)malloc((npta + 6) * 3 * sizeof(GLdouble));  // vertex array

  //  Grow the work buffer if necessary

  if ((npta * 4) > m_buf_len) {
    m_pwork_buf = (GLdouble *)realloc(m_pwork_buf, npta * 4 * sizeof(GLdouble));
    m_buf_len = npta * 4;
  }

  //  Define the polygon
  gluTessBeginPolygon(GLUtessobj, this);

  //  Check and account for winding direction of ring

  double x0, y0, x, y;
  //      OGRPoint p;

  wxPoint2DDouble *pp = (wxPoint2DDouble *)m_vertexPtrArray[0];
  bool cw = !isRingClockwise(pp, m_cntr[0]);

  // pp++;       // skip 0?

  if (cw) {
    x0 = pp->m_x;
    y0 = pp->m_y;
  } else {
    x0 = pp[m_cntr[0] - 1].m_x;
    y0 = pp[m_cntr[0] - 1].m_y;
  }

  unsigned int ptValid = m_cntr[0];

  //  Transcribe points to vertex array, in proper order with no duplicates
  //   also, accounting for tess_orient
  GLdouble *ppt = geoPt;

  for (ip = 0; ip < (unsigned int)m_cntr[0]; ip++) {
    int pidx;
    if (cw)
      pidx = m_cntr[0] - ip - 1;

    else
      pidx = ip;

    x = pp[pidx].m_x;
    y = pp[pidx].m_y;

    if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
      if (m_tess_orient == TESS_VERT) {
        *ppt++ = x;
        *ppt++ = y;
        *ppt++ = 0;
      } else {
        *ppt++ = y;
        *ppt++ = x;
        *ppt++ = 0;
      }
    } else
      ptValid--;

    x0 = x;
    y0 = y;
  }

  //  Apply LOD reduction

  if (ptValid > 20 && (m_LOD_meters > .01)) {
    std::vector<bool> bool_keep(ptValid, false);

    // Keep a few key points
    bool_keep[0] = true;
    bool_keep[1] = true;
    bool_keep[ptValid - 1] = true;
    bool_keep[ptValid - 2] = true;

    DouglasPeuckerDI(geoPt, 1, ptValid - 2, m_LOD_meters, bool_keep);

    // Create a new buffer
    double *LOD_result = (double *)malloc((m_cntr[0]) * 3 * sizeof(double));
    double *plod = LOD_result;
    int kept_LOD = 0;

    for (unsigned int i = 0; i < ptValid; i++) {
      if (bool_keep[i]) {
        double x = geoPt[i * 3];
        double y = geoPt[(i * 3) + 1];
        *plod++ = x;
        *plod++ = y;
        *plod++ = 0;
        kept_LOD++;
      }
    }

    // Copy the lod points back into the vertex buffer
    memcpy(geoPt, LOD_result, kept_LOD * 3 * sizeof(double));

    free(LOD_result);
    ptValid = kept_LOD;
  }

  //  Declare the gluContour and copy the points
  gluTessBeginContour(GLUtessobj);

  double *DPrun = geoPt;
  for (ip = 0; ip < ptValid; ip++) {
    gluTessVertex(GLUtessobj, DPrun, DPrun);
    DPrun += 3;
  }

  gluTessEndContour(GLUtessobj);

  //  Now the interior contours
#if 1
  int gpIndex = m_cntr[0];

  for (iir = 0; iir < (unsigned int)m_ncnt - 1; iir++) {
    wxPoint2DDouble *pp = (wxPoint2DDouble *)m_vertexPtrArray[iir + 1];

    int npti = m_cntr[iir + 1];
    ptValid = npti;

    double *ppt = &geoPt[gpIndex * 3];  // next available location in geoPT
    double *DPStart = ppt;

    //  Check and account for winding direction of ring
    bool cw = isRingClockwise(pp, m_cntr[iir + 1]);

    if (cw) {
      x0 = pp[0].m_x;
      y0 = pp[0].m_y;
    } else {
      x0 = pp[npti - 1].m_x;
      y0 = pp[npti - 1].m_y;
    }

    //  Transcribe points to vertex array, in proper order with no duplicates
    //   also, accounting for tess_orient
    for (int ip = 0; ip < npti; ip++) {
      int pidx;
      if (cw)  // interior contours must be cw
        pidx = npti - ip - 1;
      else
        pidx = ip;

      x = pp[pidx].m_x;
      y = pp[pidx].m_y;

      if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
        if (m_tess_orient == TESS_VERT) {
          *ppt++ = x;
          *ppt++ = y;
          *ppt++ = 0;
        } else {
          *ppt++ = y;
          *ppt++ = x;
          *ppt++ = 0;
        }
      } else
        ptValid--;

      x0 = x;
      y0 = y;
    }

    //  Declare the gluContour and reference the points
    gluTessBeginContour(GLUtessobj);

    double *DPruni = DPStart;
    for (ip = 0; ip < ptValid; ip++) {
      gluTessVertex(GLUtessobj, DPruni, DPruni);
      DPruni += 3;
    }

    gluTessEndContour(GLUtessobj);

    gpIndex += m_cntr[iir + 1];
  }
#endif

  m_pTPG_Last = NULL;
  m_pTPG_Head = NULL;
  m_nvmax = 0;

  //      Ready to kick off the tesselator
  gluTessEndPolygon(GLUtessobj);  // here it goes

  m_nvertex_max = m_nvmax;  // record largest vertex count, updates in callback

  //  Tesselation all done, so...

  //  Create the data structures

  m_ppg_head = new PolyTriGroup;
  m_ppg_head->m_bSMSENC = m_b_senc_sm;

  m_ppg_head->nContours = m_ncnt;

  m_ppg_head->pn_vertex = m_cntr;  // pointer to array of poly vertex counts
  m_ppg_head->data_type = DATA_TYPE_DOUBLE;

  //  Transcribe the raw geometry buffer
  //  Converting to float as we go, and
  //  allowing for tess_orient
  //  Also, convert to SM if requested

  // Recalculate the size of the geometry buffer
  unsigned int nptfinal = m_cntr[0] + 2;
  //     for(int i=0 ; i < nint ; i++)
  //         nptfinal += cntr[i+1] + 2;

  //  No longer need the full geometry in the SENC,
  nptfinal = 1;

  m_nwkb = (nptfinal + 1) * 2 * sizeof(float);
  m_ppg_head->pgroup_geom = (float *)calloc(sizeof(float), (nptfinal + 1) * 2);
  float *vro = m_ppg_head->pgroup_geom;
  ppt = geoPt;
  float tx, ty;

  for (ip = 0; ip < nptfinal; ip++) {
    if (TESS_HORZ == m_tess_orient) {
      ty = *ppt++;
      tx = *ppt++;
    } else {
      tx = *ppt++;
      ty = *ppt++;
    }

    if (m_b_senc_sm) {
      //  Calculate SM from chart common reference point
      double easting, northing;
      toSM(ty, tx, m_ref_lat, m_ref_lon, &easting, &northing);
      *vro++ = easting;   // x
      *vro++ = northing;  // y
    } else {
      *vro++ = tx;  // lon
      *vro++ = ty;  // lat
    }

    ppt++;  // skip z
  }

  m_ppg_head->tri_prim_head = m_pTPG_Head;  // head of linked list of TriPrims

  //  Convert the Triangle vertex arrays into a single memory allocation of
  //  floats to reduce SENC size and enable efficient access later

  //  First calculate the total byte size
  int total_byte_size = 2 * sizeof(float);
  TriPrim *p_tp = m_ppg_head->tri_prim_head;
  while (p_tp) {
    total_byte_size += p_tp->nVert * 2 * sizeof(float);
    p_tp = p_tp->p_next;  // pick up the next in chain
  }

  float *vbuf = (float *)malloc(total_byte_size);
  p_tp = m_ppg_head->tri_prim_head;
  float *p_run = vbuf;
  while (p_tp) {
    float *pfbuf = p_run;
    GLdouble *pdouble_buf = (GLdouble *)p_tp->p_vertex;

    for (int i = 0; i < p_tp->nVert * 2; ++i) {
      float x = (float)(*((GLdouble *)pdouble_buf));
      pdouble_buf++;
      *p_run++ = x;
    }

    free(p_tp->p_vertex);
    p_tp->p_vertex = (double *)pfbuf;
    p_tp = p_tp->p_next;  // pick up the next in chain
  }
  m_ppg_head->bsingle_alloc = true;
  m_ppg_head->single_buffer = (unsigned char *)vbuf;
  m_ppg_head->single_buffer_size = total_byte_size;
  m_ppg_head->data_type = DATA_TYPE_FLOAT;

  gluDeleteTess(GLUtessobj);

  free(m_pwork_buf);
  m_pwork_buf = NULL;

  free(geoPt);

  //      Free up any "Combine" vertices created
  for (unsigned int i = 0; i < m_pCombineVertexArray->GetCount(); i++)
    free(m_pCombineVertexArray->Item(i));
  delete m_pCombineVertexArray;

  m_bOK = true;

  return 0;
}

int PolyTessGeo::BuildTess(void) {
  //  Setup the tesselator

  TESSalloc ma;
  TESStesselator *tess = 0;
  const int nvp = 3;
//    unsigned char* vflags = 0;
#ifdef USE_POOL
  struct MemPool pool;
  unsigned char mem[4024 * 1024];
//    int nvflags = 0;
#else
  int allocated = 0;
#endif

#ifdef USE_POOL

  pool.size = 0;
  pool.cap = sizeof(mem);
  pool.buf = mem;
  memset(&ma, 0, sizeof(ma));
  ma.memalloc = poolAlloc;
  ma.memfree = poolFree;
  ma.userData = (void *)&pool;
  ma.extraVertices = 256;  // realloc not provided, allow 256 extra vertices.

#else

  memset(&ma, 0, sizeof(ma));
  ma.memalloc = stdAlloc;
  ma.memfree = stdFree;
  ma.userData = (void *)&allocated;
  ma.extraVertices = 256;  // realloc not provided, allow 256 extra vertices.
#endif

  tess = tessNewTess(&ma);
  if (!tess) return -1;

  // tessSetOption(tess, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);

  //  Create the contour vertex arrays

  int iir, ip;

  //      Get max number number of points(vertices) in any contour
  int npta = m_cntr[0];
  npta += 2;  // fluff

  for (iir = 0; iir < m_ncnt - 1; iir++) {
    int nptr = m_cntr[iir + 1];
    npta = wxMax(npta, nptr);
  }

  TESSreal *geoPt = (TESSreal *)malloc(
      (npta)*2 * sizeof(TESSreal));  // tess input vertex array
  TESSreal *ppt = geoPt;

  //      Create input structures

  //    Exterior Ring
  int npte = m_cntr[0];
  unsigned int ptValid = npte;

  //  Check and account for winding direction of ring

  double x0, y0, x, y;
  OGRPoint p;

  wxPoint2DDouble *pp = (wxPoint2DDouble *)m_vertexPtrArray[0];
  bool cw = isRingClockwise(pp, m_cntr[0]);

  // pp++;       // skip 0?

  if (cw) {
    x0 = pp->m_x;
    y0 = pp->m_y;
  } else {
    x0 = pp[npte - 1].m_x;
    y0 = pp[npte - 1].m_y;
  }

  //  Transcribe points to vertex array, in proper order with no duplicates
  //   also, accounting for tess_orient
  for (ip = 0; ip < npte; ip++) {
    int pidx;
    if (cw)
      pidx = npte - ip - 1;

    else
      pidx = ip;

    x = pp[pidx].m_x;
    y = pp[pidx].m_y;

    if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
      if (m_tess_orient == TESS_VERT) {
        *ppt++ = x;
        *ppt++ = y;
      } else {
        *ppt++ = y;
        *ppt++ = x;
      }
    } else
      ptValid--;

    x0 = x;
    y0 = y;
  }

  //  Apply LOD reduction
  int beforeLOD = ptValid;
  int afterLOD = beforeLOD;

  std::vector<bool> bool_keep;
  if (ptValid > 5 && (m_LOD_meters > .01)) {
    for (unsigned int i = 0; i < ptValid; i++) bool_keep.push_back(false);

    // Keep a few key points
    bool_keep[0] = true;
    bool_keep[1] = true;
    bool_keep[ptValid - 1] = true;
    bool_keep[ptValid - 2] = true;

    DouglasPeuckerFI(geoPt, 1, ptValid - 2, m_LOD_meters, bool_keep);

    // Create a new buffer
    float *LOD_result = (float *)malloc((npte)*2 * sizeof(float));
    float *plod = LOD_result;
    int kept_LOD = 0;

    for (unsigned int i = 0; i < ptValid; i++) {
      if (bool_keep[i]) {
        float x = geoPt[i * 2];
        float y = geoPt[(i * 2) + 1];
        *plod++ = x;
        *plod++ = y;
        kept_LOD++;
      }
    }

    beforeLOD = ptValid;
    afterLOD = kept_LOD;

    tessAddContour(tess, 2, LOD_result, sizeof(float) * 2, kept_LOD);

    free(LOD_result);
  } else {
    tessAddContour(tess, 2, geoPt, sizeof(float) * 2, ptValid);
  }

#if 1
  //  Now the interior contours
  for (iir = 0; iir < m_ncnt - 1; iir++) {
    ppt = geoPt;
    wxPoint2DDouble *pp = (wxPoint2DDouble *)m_vertexPtrArray[iir + 1];

    int npti = m_cntr[iir + 1];
    ptValid = npti;

    //  Check and account for winding direction of ring
    bool cw = isRingClockwise(pp, m_cntr[iir + 1]);

    if (!cw) {
      x0 = pp[0].m_x;
      y0 = pp[0].m_y;
    } else {
      x0 = pp[npti - 1].m_x;
      y0 = pp[npti - 1].m_y;
    }

    //  Transcribe points to vertex array, in proper order with no duplicates
    //   also, accounting for tess_orient
    for (int ip = 0; ip < npti; ip++) {
      OGRPoint p;
      int pidx;
      if (!cw)  // interior contours must be cw
        pidx = npti - ip - 1;
      else
        pidx = ip;

      x = pp[pidx].m_x;
      y = pp[pidx].m_y;

      if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
        if (m_tess_orient == TESS_VERT) {
          *ppt++ = x;
          *ppt++ = y;
        } else {
          *ppt++ = y;
          *ppt++ = x;
        }
      } else
        ptValid--;

      x0 = x;
      y0 = y;
    }

    tessAddContour(tess, 2, geoPt, sizeof(float) * 2, ptValid);
  }
#endif

  //      Ready to kick off the tesselator

  TriPrim *pTPG_Last = NULL;
  TriPrim *pTPG_Head = NULL;

  // s_nvmax = 0;

  // OCPNStopWatchTess tt0;

  if (!tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0))
    return -1;
  double tessTime = 0;  // tt0.GetTime();

  //  Tesselation all done, so...
  //  Create the data structures

  //  Make a linked list of TriPrims from tess output arrays

  const TESSreal *verts = tessGetVertices(tess);
  const int *vinds = tessGetVertexIndices(tess);
  const int *elems = tessGetElements(tess);
  const int nverts = tessGetVertexCount(tess);
  int nelems = tessGetElementCount(tess);

  bool skip = false;
  // skip = true;

  double stripTime = 0;
  // tt0.Reset();

  int bytes_needed_vbo = 0;
  float *vbo = 0;

  if (m_bstripify && nelems && !skip) {
    STRIPERCREATE sc;
    sc.DFaces = (udword *)elems;
    sc.NbFaces = nelems;
    sc.AskForWords = false;
    sc.ConnectAllStrips = false;
    sc.OneSided = false;
    sc.SGIAlgorithm = false;

    Striper Strip;
    Strip.Init(sc);

    STRIPERRESULT sr;
    Strip.Compute(sr);

    stripTime = 0;  // tt0.GetTime();

    /*
            fprintf(stdout, "Number of strips: %d\n", sr.NbStrips);
            fprintf(stdout, "Number of points: %d\n", nelems);
            uword* Refs = (uword*)sr.StripRuns;
            for(udword i=0;i<sr.NbStrips;i++)
            {
                    fprintf(stdout, "Strip %d:   ", i);
                    udword NbRefs = sr.StripLengths[i];
                    for(udword j=0;j<NbRefs;j++)
                    {
                            fprintf(stdout, "%d ", *Refs++);
                    }
                    fprintf(stdout, "\n");
            }
    */
    // calculate and allocate the final (float) VBO-like buffer for this entire
    // feature

    int *Refs = (int *)sr.StripRuns;
    for (unsigned int i = 0; i < sr.NbStrips; i++) {
      unsigned int NbRefs = sr.StripLengths[i];  //  vertices per strip
      bytes_needed_vbo += NbRefs * 2 * sizeof(float);
    }

    vbo = (float *)malloc(bytes_needed_vbo);
    float *vbo_run = vbo;

    for (unsigned int i = 0; i < sr.NbStrips; i++) {
      unsigned int NbRefs = sr.StripLengths[i];  //  vertices per strip

      if (NbRefs >= 3) {  // this is a valid primitive

        TriPrim *pTPG = new TriPrim;
        if (NULL == pTPG_Last) {
          pTPG_Head = pTPG;
          pTPG_Last = pTPG;
        } else {
          pTPG_Last->p_next = pTPG;
          pTPG_Last = pTPG;
        }

        pTPG->p_next = NULL;
        pTPG->nVert = NbRefs;

        //                pTPG->p_vertex = (double *)malloc(NbRefs * 2 *
        //                sizeof(double)); GLdouble *pdd =
        //                (GLdouble*)pTPG->p_vertex;

        pTPG->p_vertex = (double *)vbo_run;

        //  Preset LLBox bounding box limits
        double sxmax = -1e8;
        double sxmin = 1e8;
        double symax = -1e8;
        double symin = 1e8;

        if (NbRefs > 3)
          pTPG->type = GL_TRIANGLE_STRIP;
        else
          pTPG->type = GL_TRIANGLES;

        // Add the first two points
        int vindex[3];
        vindex[0] = *Refs++;
        vindex[1] = *Refs++;
        unsigned int np = 2;  // for the first triangle

        for (unsigned int i = 2; i < NbRefs; i++) {
          vindex[np] = *Refs++;
          np++;

          for (unsigned int j = 0; j < np; ++j) {
            double yd = verts[vindex[j] * 2];
            double xd = verts[vindex[j] * 2 + 1];

            // Calculate LLBox bounding box for each Tri-prim
            if (m_bmerc_transform) {
              double valx = (xd * mx_rate) + mx_offset;
              double valy = (yd * my_rate) + my_offset;

              //    quickly convert to lat/lon
              double lat = (2.0 * atan(exp(valy / CM93_semimajor_axis_meters)) -
                            PI / 2.) /
                           DEGREE;
              double lon = (valx / (DEGREE * CM93_semimajor_axis_meters));

              sxmax = wxMax(lon, sxmax);
              sxmin = wxMin(lon, sxmin);
              symax = wxMax(lat, symax);
              symin = wxMin(lat, symin);
            } else {
              sxmax = wxMax(xd, sxmax);
              sxmin = wxMin(xd, sxmin);
              symax = wxMax(yd, symax);
              symin = wxMin(yd, symin);
            }

            //  Append this point to TriPrim vbo

            *vbo_run++ =
                (xd) + m_feature_easting;  // adjust to chart ref coordinates
            *vbo_run++ = (yd) + m_feature_northing;
          }  // For

          // Compute the final LLbbox for this TriPrim chain
          double minlat, minlon, maxlat, maxlon;
          fromSM(sxmin, symin, m_feature_ref_lat, m_feature_ref_lon, &minlat,
                 &minlon);
          fromSM(sxmax, symax, m_feature_ref_lat, m_feature_ref_lon, &maxlat,
                 &maxlon);

          pTPG->tri_box.Set(minlat, minlon, maxlat, maxlon);

          // set for next single point
          np = 0;
        }
      } else
        Refs += sr.StripLengths[i];

    }       // for strips
  } else {  // not stripified

    m_nvertex_max = nverts;  // record largest vertex count

    bytes_needed_vbo = nelems * nvp * 2 * sizeof(float);
    vbo = (float *)malloc(bytes_needed_vbo);
    float *vbo_run = vbo;

    for (int i = 0; i < nelems; ++i) {
      const int *p = &elems[i * nvp];

      TriPrim *pTPG = new TriPrim;
      if (NULL == pTPG_Last) {
        pTPG_Head = pTPG;
        pTPG_Last = pTPG;
      } else {
        pTPG_Last->p_next = pTPG;
        pTPG_Last = pTPG;
      }

      pTPG->p_next = NULL;
      pTPG->type = GL_TRIANGLES;
      pTPG->nVert = nvp;

      //            pTPG->p_vertex = (double *)malloc(nvp * 2 * sizeof(double));
      //            GLdouble *pdd = (GLdouble*)pTPG->p_vertex;

      pTPG->p_vertex = (double *)vbo_run;

      //  Preset LLBox bounding box limits
      double sxmax = -1e8;
      double sxmin = 1e8;
      double symax = -1e8;
      double symin = 1e8;

      for (size_t j = 0; j < nvp && p[j] != TESS_UNDEF; ++j) {
        double yd = verts[p[j] * 2];
        double xd = verts[p[j] * 2 + 1];

        // Calculate LLBox bounding box for each Tri-prim
        if (m_bmerc_transform) {
          double valx = (xd * mx_rate) + mx_offset;
          double valy = (yd * my_rate) + my_offset;

          //    quickly convert to lat/lon
          double lat =
              (2.0 * atan(exp(valy / CM93_semimajor_axis_meters)) - PI / 2.) /
              DEGREE;
          double lon = (valx / (DEGREE * CM93_semimajor_axis_meters));

          sxmax = wxMax(lon, sxmax);
          sxmin = wxMin(lon, sxmin);
          symax = wxMax(lat, symax);
          symin = wxMin(lat, symin);
        } else {
          sxmax = wxMax(xd, sxmax);
          sxmin = wxMin(xd, sxmin);
          symax = wxMax(yd, symax);
          symin = wxMin(yd, symin);
        }

        //  Append this point to TriPrim, converting to SM if called for

        if (m_b_senc_sm) {
          double easting, northing;
          toSM(yd, xd, m_ref_lat, m_ref_lon, &easting, &northing);
          *vbo_run++ = easting;
          *vbo_run++ = northing;
        } else {
          *vbo_run++ = xd;
          *vbo_run++ = yd;
        }
      }

      pTPG->tri_box.Set(symin, sxmin, symax, sxmax);
    }
  }  // stripify

  if (m_printStats) {
    int nTri = 0;
    int nStrip = 0;

    TriPrim *p_tp = pTPG_Head;
    while (p_tp) {
      if (p_tp->type == GL_TRIANGLES) nTri++;
      if (p_tp->type == GL_TRIANGLE_STRIP) nStrip++;

      p_tp = p_tp->p_next;  // pick up the next in chain
    }

    if ((nTri + nStrip) > 10000) {
      printf("LOD:  %d/%d\n", afterLOD, beforeLOD);

      printf("Tess time(ms): %f\n", tessTime);
      printf("Strip time(ms): %f\n", stripTime);

      printf("Primitives:   Tri: %5d  Strip: %5d  Total: %5d\n", nTri, nStrip,
             nTri + nStrip);
      printf("\n");
    }
  }

  m_ppg_head = new PolyTriGroup;
  m_ppg_head->m_bSMSENC = m_b_senc_sm;

  m_ppg_head->nContours = m_ncnt;
  m_ppg_head->pn_vertex = m_cntr;  // pointer to array of poly vertex counts
  m_ppg_head->tri_prim_head = pTPG_Head;  // head of linked list of TriPrims
  // m_ppg_head->data_type = DATA_TYPE_DOUBLE;

  //  Transcribe the raw geometry buffer
  //  Converting to float as we go, and
  //  allowing for tess_orient
  //  Also, convert to SM if requested

  int nptfinal = npta;

  //  No longer need the full geometry in the SENC,
  nptfinal = 1;

  m_nwkb = (nptfinal + 1) * 2 * sizeof(float);
  m_ppg_head->pgroup_geom = (float *)malloc(m_nwkb);
  float *vro = m_ppg_head->pgroup_geom;
  ppt = geoPt;
  float tx, ty;

  for (ip = 0; ip < nptfinal; ip++) {
    if (TESS_HORZ == m_tess_orient) {
      ty = *ppt++;
      tx = *ppt++;
    } else {
      tx = *ppt++;
      ty = *ppt++;
    }

    if (m_b_senc_sm) {
      //  Calculate SM from chart common reference point
      double easting, northing;
      toSM(ty, tx, 0 /*ref_lat*/, 0 /*ref_lon*/, &easting, &northing);
      *vro++ = easting;   // x
      *vro++ = northing;  // y
    } else {
      *vro++ = tx;  // lon
      *vro++ = ty;  // lat
    }

    ppt++;  // skip z
  }

  m_ppg_head->bsingle_alloc = true;
  m_ppg_head->single_buffer = (unsigned char *)vbo;
  m_ppg_head->single_buffer_size = bytes_needed_vbo;
  m_ppg_head->data_type = DATA_TYPE_FLOAT;

  free(geoPt);

  //    All allocated buffers are owned now by the m_ppg_head
  //    And will be freed on dtor of this object

  if (tess) tessDeleteTess(tess);

  m_bOK = true;

  return 0;
}

// GLU tesselation support functions
void beginCallback(GLenum which, void *polyData) {
  PolyTessGeo *pThis = (PolyTessGeo *)polyData;

  pThis->m_buf_idx = 0;
  pThis->m_nvcall = 0;
  pThis->m_gltri_type = which;
}

void errorCallback(GLenum errorCode, void *polyData) {
  const GLubyte *estring;

  estring = gluErrorString(errorCode);
  printf("Tessellation Error: %s\n", estring);
  exit(0);
}

void endCallback(void *polyData) {
  PolyTessGeo *pThis = (PolyTessGeo *)polyData;

  //      Create a TriPrim
  if (pThis->m_nvcall >
      pThis->m_nvmax)  // keep track of largest number of triangle vertices
    pThis->m_nvmax = pThis->m_nvcall;

  switch (pThis->m_gltri_type) {
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLES: {
      TriPrim *pTPG = new TriPrim;
      if (NULL == pThis->m_pTPG_Last) {
        pThis->m_pTPG_Head = pTPG;
        pThis->m_pTPG_Last = pTPG;
      } else {
        pThis->m_pTPG_Last->p_next = pTPG;
        pThis->m_pTPG_Last = pTPG;
      }

      pTPG->p_next = NULL;
      pTPG->type = pThis->m_gltri_type;
      pTPG->nVert = pThis->m_nvcall;

      //  Calculate bounding box
      double sxmax = -1e8;  // this poly BBox
      double sxmin = 1e8;
      double symax = -1e8;
      double symin = 1e8;

      GLdouble *pvr = pThis->m_pwork_buf;
      for (int iv = 0; iv < pThis->m_nvcall; iv++) {
        GLdouble xd, yd;
        xd = *pvr++;
        yd = *pvr++;

        if (pThis->m_bcm93) {
          // cm93 hits here
          double valx = (xd * pThis->mx_rate) + pThis->mx_offset +
                        pThis->m_feature_easting;
          double valy = (yd * pThis->my_rate) + pThis->my_offset +
                        pThis->m_feature_northing;

          //    Convert to lat/lon
          double lat =
              (2.0 * atan(exp(valy / CM93_semimajor_axis_meters)) - PI / 2.) /
              DEGREE;
          double lon = (valx / (DEGREE * CM93_semimajor_axis_meters));

          sxmax = wxMax(lon, sxmax);
          sxmin = wxMin(lon, sxmin);
          symax = wxMax(lat, symax);
          symin = wxMin(lat, symin);
        } else {
          // ENC hits here, values are SM measures from feature reference point
          double valx = (xd * pThis->mx_rate) + pThis->mx_offset +
                        pThis->m_feature_easting;
          double valy = (yd * pThis->my_rate) + pThis->my_offset +
                        pThis->m_feature_northing;

          sxmax = wxMax(valx, sxmax);
          sxmin = wxMin(valx, sxmin);
          symax = wxMax(valy, symax);
          symin = wxMin(valy, symin);
        }
      }

      // Compute the final LLbbox for this TriPrim chain
      if (pThis->m_bcm93)
        pTPG->tri_box.Set(symin, sxmin, symax, sxmax);
      else {
        double minlat, minlon, maxlat, maxlon;
        fromSM(sxmin, symin, pThis->m_ref_lat, pThis->m_ref_lon, &minlat,
               &minlon);
        fromSM(sxmax, symax, pThis->m_ref_lat, pThis->m_ref_lon, &maxlat,
               &maxlon);
        pTPG->tri_box.Set(minlat, minlon, maxlat, maxlon);
      }

      //  Transcribe this geometry to TriPrim,
      {
        GLdouble *pds = pThis->m_pwork_buf;
        pTPG->p_vertex = (double *)malloc(pThis->m_nvcall * 2 * sizeof(double));
        GLdouble *pdd = (GLdouble *)pTPG->p_vertex;

        for (int ip = 0; ip < pThis->m_nvcall; ip++) {
          double dlon = *pds++;
          double dlat = *pds++;
          *pdd++ =
              dlon +
              pThis->m_feature_easting;  // adjust to feature ref coordinates
          *pdd++ = dlat + pThis->m_feature_northing;
        }
      }

      break;
    }
    default: {
      //            sprintf(buf, "....begin Callback  unknown\n");
      break;
    }
  }
}

void vertexCallback(GLvoid *vertex, void *polyData) {
  PolyTessGeo *pThis = (PolyTessGeo *)polyData;

  GLdouble *pointer;

  pointer = (GLdouble *)vertex;

  if (pThis->m_buf_idx > pThis->m_buf_len - 4) {
    int new_buf_len = pThis->m_buf_len + 100;
    GLdouble *tmp = pThis->m_pwork_buf;

    pThis->m_pwork_buf =
        (GLdouble *)realloc(pThis->m_pwork_buf, new_buf_len * sizeof(GLdouble));
    if (NULL == pThis->m_pwork_buf) {
      free(tmp);
      tmp = NULL;
    } else
      pThis->m_buf_len = new_buf_len;
  }

  if (pThis->m_tess_orient == TESS_VERT) {
    pThis->m_pwork_buf[pThis->m_buf_idx++] = pointer[0];
    pThis->m_pwork_buf[pThis->m_buf_idx++] = pointer[1];
  } else {
    pThis->m_pwork_buf[pThis->m_buf_idx++] = pointer[1];
    pThis->m_pwork_buf[pThis->m_buf_idx++] = pointer[0];
  }

  pThis->m_nvcall++;
}

/*  combineCallback is used to create a new vertex when edges
 *  intersect.
 */
void combineCallback(GLdouble coords[3], GLdouble *vertex_data[4],
                     GLfloat weight[4], GLdouble **dataOut, void *polyData) {
  PolyTessGeo *pThis = (PolyTessGeo *)polyData;

  GLdouble *vertex = (GLdouble *)malloc(6 * sizeof(GLdouble));

  vertex[0] = coords[0];
  vertex[1] = coords[1];
  vertex[2] = coords[2];
  vertex[3] = vertex[4] = vertex[5] = 0.;  // 01/13/05 bugfix

  *dataOut = vertex;

  pThis->m_pCombineVertexArray->Add(vertex);
}

wxStopWatch *s_stwt;

//------------------------------------------------------------------------------
//          PolyTriGroup Implementation
//------------------------------------------------------------------------------
PolyTriGroup::PolyTriGroup() {
  pn_vertex = NULL;    // pointer to array of poly vertex counts
  pgroup_geom = NULL;  // pointer to Raw geometry, used for contour line drawing
  tri_prim_head = NULL;  // head of linked list of TriPrims
  m_bSMSENC = false;
  bsingle_alloc = false;
  single_buffer = NULL;
  single_buffer_size = 0;
  data_type = DATA_TYPE_DOUBLE;
  sfactor = 1.0;
}

PolyTriGroup::~PolyTriGroup() {
  free(pn_vertex);
  free(pgroup_geom);
  // Walk the list of TriPrims, deleting as we go
  TriPrim *tp_next;
  TriPrim *tp = tri_prim_head;

  if (bsingle_alloc) {
    free(single_buffer);
    while (tp) {
      tp_next = tp->p_next;
      delete tp;
      tp = tp_next;
    }
  } else {
    while (tp) {
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
TriPrim::TriPrim() {}

TriPrim::~TriPrim() {}

void TriPrim::FreeMem() { free(p_vertex); }
