//-------------------------------------------------------------------------------------------------------
//  Cohen & Sutherland Line clipping algorithms
//-------------------------------------------------------------------------------------------------------
/*
 *
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
 * WWW Home: http://gxsm.sf.net
 *
 */

#include "line_clip.h"

#define CTRUE -1
#define CFALSE 0

typedef enum { LEFT, RIGHT, BOTTOM, TOP } edge;
typedef long outcode;

/* Local variables for cohen_sutherland_line_clip: */
struct LOC_cohen_sutherland_line_clip {
  double xmin, xmax, ymin, ymax;
};

void CompOutCode(double x, double y, outcode *code,
                 struct LOC_cohen_sutherland_line_clip *LINK) {
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

ClipResult cohen_sutherland_line_clip_d(double *x0, double *y0, double *x1,
                                        double *y1, double xmin_, double xmax_,
                                        double ymin_, double ymax_) {
  /* Cohen-Sutherland clipping algorithm for line P0=(x1,y0) to P1=(x1,y1)
and clip rectangle with diagonal from (xmin,ymin) to (xmax,ymax).*/
  struct LOC_cohen_sutherland_line_clip V;
  int done = CFALSE;
  ClipResult clip = Visible;
  outcode outcode0, outcode1, outcodeOut;
  /*Outcodes for P0,P1, and whichever point lies outside the clip rectangle*/
  double x = 0., y = 0.;

  V.xmin = xmin_;
  V.xmax = xmax_;
  V.ymin = ymin_;
  V.ymax = ymax_;
  CompOutCode(*x0, *y0, &outcode0, &V);
  CompOutCode(*x1, *y1, &outcode1, &V);
  do {
    if (outcode0 == 0 && outcode1 == 0) { /*Trivial accept and exit*/
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

ClipResult cohen_sutherland_line_clip_i(int *x0_, int *y0_, int *x1_, int *y1_,
                                        int xmin_, int xmax_, int ymin_,
                                        int ymax_) {
  ClipResult ret;
  double x0, y0, x1, y1;
  x0 = *x0_;
  y0 = *y0_;
  x1 = *x1_;
  y1 = *y1_;
  ret =
      cohen_sutherland_line_clip_d(&x0, &y0, &x1, &y1, (double)xmin_,
                                   (double)xmax_, (double)ymin_, (double)ymax_);
  *x0_ = (int)x0;
  *y0_ = (int)y0;
  *x1_ = (int)x1;
  *y1_ = (int)y1;
  return ret;
}


