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
 *
 */

#ifndef __POLYMATH_H__
#define __POLYMATH_H__

typedef struct {
  double x;
  double y;
} MyPoint;

typedef struct {
  float y;
  float x;
} float_2Dpt;

#ifdef __cplusplus
extern "C" int G_PtInPolygon(MyPoint *, int, float, float);
extern "C" int G_PtInPolygon_FL(float_2Dpt *, int, float, float);
extern "C" int Intersect_FL(float_2Dpt, float_2Dpt, float_2Dpt, float_2Dpt);
#else /* __cplusplus */
extern int G_PtInPolygon(MyPoint *, int, float, float);
#endif

#endif  //guard
