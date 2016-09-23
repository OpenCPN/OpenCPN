/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Extern C Linked Utilities
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
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "cutil.h"
#include "vector2D.h"


int Intersect(MyPoint, MyPoint, MyPoint, MyPoint) ;
int CCW(MyPoint, MyPoint, MyPoint) ;

int Intersect_FL(float_2Dpt, float_2Dpt, float_2Dpt, float_2Dpt) ;
int CCW_FL(float_2Dpt, float_2Dpt, float_2Dpt) ;


/*************************************************************************


 * FUNCTION:   G_PtInPolygon
 *
 * PURPOSE
 * This routine determines if the point passed is in the polygon. It uses
 * the classical polygon hit-testing algorithm: a horizontal ray starting
 * at the point is extended infinitely rightwards and the number of
 * polygon edges that intersect the ray are counted. If the number is odd,
 * the point is inside the polygon.
 *
 * Polygon is assumed OPEN, not CLOSED.
 * RETURN VALUE
 * (bool) TRUE if the point is inside the polygon, FALSE if not.
 *************************************************************************/


int  G_PtInPolygon(MyPoint *rgpts, int wnumpts, float x, float y)
{

   MyPoint  *ppt, *ppt1 ;
   int   i ;
   MyPoint  pt1, pt2, pt0 ;
   int   wnumintsct = 0 ;


   pt0.x = x;
   pt0.y = y;

   pt1 = pt2 = pt0 ;
   pt2.x = 1.e8;

   // Now go through each of the lines in the polygon and see if it
   // intersects
   for (i = 0, ppt = rgpts ; i < wnumpts-1 ; i++, ppt++)
   {
      ppt1 = ppt;
      ppt1++;
      if (Intersect(pt0, pt2, *ppt, *(ppt1)))
         wnumintsct++ ;
   }

   // And the last line
   if (Intersect(pt0, pt2, *ppt, *rgpts))
      wnumintsct++ ;

   return (wnumintsct&1) ;

}


/*************************************************************************

              0
 * FUNCTION:   Intersect
 *
 * PURPOSE
 * Given two line segments, determine if they intersect.
 *
 * RETURN VALUE
 * TRUE if they intersect, FALSE if not.
 *************************************************************************/


int Intersect(MyPoint p1, MyPoint p2, MyPoint p3, MyPoint p4) {
      int i;
      i = CCW(p1, p2, p3);
      i = CCW(p1, p2, p4);
      i = CCW(p3, p4, p1);
      i = CCW(p3, p4, p2);
   return ((( CCW(p1, p2, p3) * CCW(p1, p2, p4)) <= 0)
        && (( CCW(p3, p4, p1) * CCW(p3, p4, p2)  <= 0) )) ;

}
/*************************************************************************


 * FUNCTION:   CCW (CounterClockWise)
 *
 * PURPOSE
 * Determines, given three points, if when travelling from the first to
 * the second to the third, we travel in a counterclockwise direction.
 *
 * RETURN VALUE
 * (int) 1 if the movement is in a counterclockwise direction, -1 if
 * not.
 *************************************************************************/


int CCW(MyPoint p0, MyPoint p1, MyPoint p2) {
   double dx1, dx2 ;
   double dy1, dy2 ;

   dx1 = p1.x - p0.x ; dx2 = p2.x - p0.x ;
   dy1 = p1.y - p0.y ; dy2 = p2.y - p0.y ;

   /* This is a slope comparison: we don't do divisions because
    * of divide by zero possibilities with pure horizontal and pure
    * vertical lines.
    */
   return ((dx1 * dy2 > dy1 * dx2) ? 1 : -1) ;

}


int  G_PtInPolygon_FL(float_2Dpt *rgpts, int wnumpts, float x, float y)
{

      float_2Dpt  *ppt, *ppt1 ;
      int   i ;
      float_2Dpt  pt1, pt2, pt0 ;
      int   wnumintsct = 0 ;


      pt0.x = x;
      pt0.y = y;

      pt1 = pt2 = pt0 ;
      pt2.x = 1.e8;

   // Now go through each of the lines in the polygon and see if it
   // intersects
      for (i = 0, ppt = rgpts ; i < wnumpts-1 ; i++, ppt++)
      {
            ppt1 = ppt;
            ppt1++;
            if (Intersect_FL(pt0, pt2, *ppt, *(ppt1)))
                  wnumintsct++ ;
      }

   // And the last line
      if (Intersect_FL(pt0, pt2, *ppt, *rgpts))
            wnumintsct++ ;

      return (wnumintsct&1) ;

}


/*************************************************************************

 * FUNCTION:   Intersect_FL
 *
 * PURPOSE
 * Given two line segments, determine if they intersect.
 *
 * RETURN VALUE
 * TRUE if they intersect, FALSE if not.
 *************************************************************************/


int Intersect_FL(float_2Dpt p1, float_2Dpt p2, float_2Dpt p3, float_2Dpt p4) {
      int i;
      i = CCW_FL(p1, p2, p3);
      i = CCW_FL(p1, p2, p4);
      i = CCW_FL(p3, p4, p1);
      i = CCW_FL(p3, p4, p2);
      return ((( CCW_FL(p1, p2, p3) * CCW_FL(p1, p2, p4)) <= 0)
                  && (( CCW_FL(p3, p4, p1) * CCW_FL(p3, p4, p2)  <= 0) )) ;

}
/*************************************************************************


 * FUNCTION:   CCW_FL (CounterClockWise)
 *
 * PURPOSE
 * Determines, given three points, if when travelling from the first to
 * the second to the third, we travel in a counterclockwise direction.
 *
 * RETURN VALUE
 * (int) 1 if the movement is in a counterclockwise direction, -1 if
 * not.
 *************************************************************************/


int CCW_FL(float_2Dpt p0, float_2Dpt p1, float_2Dpt p2) {
      double dx1, dx2 ;
      double dy1, dy2 ;

      dx1 = p1.x - p0.x ; dx2 = p2.x - p0.x ;
      dy1 = p1.y - p0.y ; dy2 = p2.y - p0.y ;

   /* This is a slope comparison: we don't do divisions because
      * of divide by zero possibilities with pure horizontal and pure
      * vertical lines.
   */
      return ((dx1 * dy2 > dy1 * dx2) ? 1 : -1) ;

}




#define CTRUE -1
#define CFALSE 0

typedef enum {
    LEFT, RIGHT, BOTTOM, TOP
} edge;
typedef long outcode;


/* Local variables for cohen_sutherland_line_clip: */
struct LOC_cohen_sutherland_line_clip {
    double xmin, xmax, ymin, ymax;
} ;

void CompOutCode (double x, double y, outcode *code, struct LOC_cohen_sutherland_line_clip *LINK)
{
    /*Compute outcode for the point (x,y) */
    *code = 0;
    if (y > LINK->ymax)
        *code = 1L << ((long)TOP);
    else if (y < LINK->ymin)
        *code = 1L << ((long)BOTTOM);
    if (x > LINK->xmax)
        *code |= 1L << ((long)RIGHT);
    else if (x < LINK->xmin)
        *code |= 1L << ((long)LEFT);
}


ClipResult cohen_sutherland_line_clip_d (double *x0, double *y0, double *x1, double *y1,
                                         double xmin_, double xmax_, double ymin_, double ymax_)
{
      /* Cohen-Sutherland clipping algorithm for line P0=(x1,y0) to P1=(x1,y1)
    and clip rectangle with diagonal from (xmin,ymin) to (xmax,ymax).*/
    struct LOC_cohen_sutherland_line_clip V;
    int accept = CFALSE, done = CFALSE;
    ClipResult clip = Visible;
    outcode outcode0, outcode1, outcodeOut;
    /*Outcodes for P0,P1, and whichever point lies outside the clip rectangle*/
    double x=0., y=0.;

    V.xmin = xmin_;
    V.xmax = xmax_;
    V.ymin = ymin_;
    V.ymax = ymax_;
    CompOutCode(*x0, *y0, &outcode0, &V);
    CompOutCode(*x1, *y1, &outcode1, &V);
    do {
        if (outcode0 == 0 && outcode1 == 0) {   /*Trivial accept and exit*/
            accept = CTRUE;
            done = CTRUE;
        } else if ((outcode0 & outcode1) != 0) {
            clip = Invisible;
            done = CTRUE;
        }
        /*Logical intersection is true, so trivial reject and exit.*/
        else {
            clip = Visible;
                        /*Failed both tests, so calculate the line segment to clip;
            from an outside point to an intersection with clip edge.*/
            /*At least one endpoint is outside the clip rectangle; pick it.*/
            if (outcode0 != 0)
                outcodeOut = outcode0;
            else
                outcodeOut = outcode1;
                        /*Now find intersection point;
            use formulas y=y0+slope*(x-x0),x=x0+(1/slope)*(y-y0).*/

            if (((1L << ((long)TOP)) & outcodeOut) != 0) {
                /*Divide line at top of clip rectangle*/
                x = *x0 + (*x1 - *x0) * (V.ymax - *y0) / (*y1 - *y0);
                y = V.ymax;
            } else if (((1L << ((long)BOTTOM)) & outcodeOut) != 0) {
                /*Divide line at bottom of clip rectangle*/
                x = *x0 + (*x1 - *x0) * (V.ymin - *y0) / (*y1 - *y0);
                y = V.ymin;
            } else if (((1L << ((long)RIGHT)) & outcodeOut) != 0) {
                /*Divide line at right edge of clip rectangle*/
                y = *y0 + (*y1 - *y0) * (V.xmax - *x0) / (*x1 - *x0);
                x = V.xmax;
            } else if (((1L << ((long)LEFT)) & outcodeOut) != 0) {
                /*Divide line at left edge of clip rectangle*/
                y = *y0 + (*y1 - *y0) * (V.xmin - *x0) / (*x1 - *x0);
                x = V.xmin;
            }
                        /*Now we move outside point to intersection point to clip,
            and get ready for next pass.*/
            if (outcodeOut == outcode0) {
                *x0 = x;
                *y0 = y;
                CompOutCode(*x0, *y0, &outcode0, &V);
            } else {
                *x1 = x;
                *y1 = y;
                CompOutCode(*x1, *y1, &outcode1, &V);
            }
        }
    } while (!done);
    return clip;
}

ClipResult cohen_sutherland_line_clip_i (int *x0_, int *y0_, int *x1_, int *y1_,
                                         int xmin_, int xmax_, int ymin_, int ymax_)
{
    ClipResult ret;
    double x0,y0,x1,y1;
    x0 = *x0_;
    y0 = *y0_;
    x1 = *x1_;
    y1 = *y1_;
    ret = cohen_sutherland_line_clip_d (&x0, &y0, &x1, &y1,
                                         (double)xmin_, (double)xmax_,
                                         (double)ymin_, (double)ymax_);
    *x0_ = (int)x0;
    *y0_ = (int)y0;
    *x1_ = (int)x1;
    *y1_ = (int)y1;
    return ret;
}


double      round_msvc (double x)
{
    return(floor(x + 0.5));

}

#ifdef __MSVC__
#include <windows.h>
#include <float.h>            // for _clear87()

extern long __stdcall MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
//    return EXCEPTION_EXECUTE_HANDLER ;        // terminates the app

    switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
        case EXCEPTION_FLT_DENORMAL_OPERAND:
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_FLT_INEXACT_RESULT:
        case EXCEPTION_FLT_INVALID_OPERATION:
        case EXCEPTION_FLT_OVERFLOW:
        case EXCEPTION_FLT_STACK_CHECK:
        case EXCEPTION_FLT_UNDERFLOW:
           _clear87();
            return EXCEPTION_CONTINUE_EXECUTION ;     // retry

        default:
           return EXCEPTION_CONTINUE_SEARCH ;         // standard fatal dialog box
    }
}

#endif

/*          Replacement for __MSVC__ in absence of snprintf or _snprintf  */
#ifdef __MSVC__
extern int mysnprintf( char *buffer, int count, const char *format, ... )
{
      int ret;

      va_list arg;
      va_start(arg, format);
      ret = _vsnprintf(buffer, count, format, arg);

      va_end(arg);
      return ret;
}
#endif

int oldNextPow2(int size)
{
    /* compute dimensions needed as next larger power of 2 */
    int a = size;
    int p = 0;
    while( a ) {
        a = a >> 1;
        p++;
    }
    return 1 << p;
}

int NextPow2(int size)
{
    int n = size-1;          // compute dimensions needed as next larger power of 2
    int shift = 1;
    while ((n+1) & n){
        n |= n >> shift;
        shift <<= 1;
    }
    
    return n + 1;
}

void DouglasPeucker(double *PointList, int fp, int lp, double epsilon, wxArrayInt *keep)
{
    // Find the point with the maximum distance
    double dmax = 0;
    int index = 0;
    
    vector2D va(PointList[2*fp] - PointList[2*lp],
                PointList[2*fp+1] - PointList[2*lp+1]);
    
    double da = va.x*va.x + va.y*va.y;
    for(int i = fp+1 ; i < lp ; ++i) {
        vector2D vb(PointList[2*i] - PointList[2*fp],
                    PointList[2*i + 1] - PointList[2*fp+1]);
        
        double dab = va.x*vb.x + va.y*vb.y;
        double db = vb.x*vb.x + vb.y*vb.y;
        double d = da - dab*dab/db;
        if ( d > dmax ) {
            index = i;
            dmax = d;
        }
    }
    // If max distance is greater than epsilon, recursively simplify
    if ( dmax > epsilon*epsilon ) {
        keep->Add(index);
        
        // Recursive call
        DouglasPeucker(PointList, fp, index, epsilon, keep);
        DouglasPeucker(PointList, index, lp, epsilon, keep);
        
    }
}

//      CRC calculation for a byte buffer

static unsigned int crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

#define UPDC32(octet,crc) (crc_32_tab[((crc)\
^ ((unsigned char)octet)) & 0xff] ^ ((crc) >> 8))


unsigned int crc32buf(unsigned char *buf, size_t len)
{
    unsigned int oldcrc32;
    
    oldcrc32 = 0xFFFFFFFF;
    
    for ( ; len; --len, ++buf)
    {
        oldcrc32 = UPDC32(*buf, oldcrc32);
    }
    
    return ~oldcrc32;
    
}
