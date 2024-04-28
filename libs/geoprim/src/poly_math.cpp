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

#include "poly_math.h"
#include "vector2D.h"

int Intersect(MyPoint, MyPoint, MyPoint, MyPoint);
int CCW(MyPoint, MyPoint, MyPoint);

int Intersect_FL(float_2Dpt, float_2Dpt, float_2Dpt, float_2Dpt);
int CCW_FL(float_2Dpt, float_2Dpt, float_2Dpt);

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

int G_PtInPolygon(MyPoint *rgpts, int wnumpts, float x, float y) {
  MyPoint *ppt, *ppt1;
  int i;
  MyPoint pt1, pt2, pt0;
  int wnumintsct = 0;

  pt0.x = x;
  pt0.y = y;

  pt1 = pt2 = pt0;
  pt2.x = 1.e8;

  // Now go through each of the lines in the polygon and see if it
  // intersects
  for (i = 0, ppt = rgpts; i < wnumpts - 1; i++, ppt++) {
    ppt1 = ppt;
    ppt1++;
    if (Intersect(pt0, pt2, *ppt, *ppt1)) wnumintsct++;
  }

  // And the last line
  if (Intersect(pt0, pt2, *ppt, *rgpts)) wnumintsct++;

  return wnumintsct & 1;
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
  return CCW(p1, p2, p3) * CCW(p1, p2, p4) <= 0 &&
         CCW(p3, p4, p1) * CCW(p3, p4, p2) <= 0;
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
  double dx1, dx2;
  double dy1, dy2;

  dx1 = p1.x - p0.x;
  dx2 = p2.x - p0.x;
  dy1 = p1.y - p0.y;
  dy2 = p2.y - p0.y;

  /* This is a slope comparison: we don't do divisions because
   * of divide by zero possibilities with pure horizontal and pure
   * vertical lines.
   */
  return dx1 * dy2 > dy1 * dx2 ? 1 : -1;
}

int G_PtInPolygon_FL(float_2Dpt *rgpts, int wnumpts, float x, float y) {
  float_2Dpt *ppt, *ppt1;
  int i;
  float_2Dpt pt1, pt2, pt0;
  int wnumintsct = 0;

  pt0.x = x;
  pt0.y = y;

  pt1 = pt2 = pt0;
  pt2.x = 1.e8;

  // Now go through each of the lines in the polygon and see if it
  // intersects
  for (i = 0, ppt = rgpts; i < wnumpts - 1; i++, ppt++) {
    ppt1 = ppt;
    ppt1++;
    if (Intersect_FL(pt0, pt2, *ppt, *ppt1)) wnumintsct++;
  }

  // And the last line
  if (Intersect_FL(pt0, pt2, *ppt, *rgpts)) wnumintsct++;

  return wnumintsct & 1;
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
  return CCW_FL(p1, p2, p3) * CCW_FL(p1, p2, p4) <= 0 &&
         CCW_FL(p3, p4, p1) * CCW_FL(p3, p4, p2) <= 0;
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
  double dx1, dx2;
  double dy1, dy2;

  dx1 = p1.x - p0.x;
  dx2 = p2.x - p0.x;
  dy1 = p1.y - p0.y;
  dy2 = p2.y - p0.y;

  /* This is a slope comparison: we don't do divisions because
   * of divide by zero possibilities with pure horizontal and pure
   * vertical lines.
   */
  return dx1 * dy2 > dy1 * dx2 ? 1 : -1;
}

