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

#ifndef __CUTIL_H__
#define __CUTIL_H__


#ifdef __MSVC__
#include <windows.h>
#endif

#include <wx/dynarray.h>

typedef struct  {
      double x;
      double y;
} MyPoint;

typedef struct {
      float y;
      float x;
} float_2Dpt;

#ifdef __cplusplus
      extern "C" int G_PtInPolygon(MyPoint *, int, float, float) ;
      extern "C" int G_PtInPolygon_FL(float_2Dpt *, int, float, float) ;
      extern "C" int Intersect_FL(float_2Dpt, float_2Dpt, float_2Dpt, float_2Dpt);
          
      extern "C" int mysnprintf( char *buffer, int count, const char *format, ... );
      extern "C" int NextPow2(int size);
      extern "C" void DouglasPeucker(double *PointList, int fp, int lp, double epsilon, wxArrayInt *keep);
      
#else /* __cplusplus */
      extern int G_PtInPolygon(MyPoint *, int, float, float) ;
      extern int mysnprintf( char *buffer, int count, const char *format, ... );
#endif


#ifdef __MSVC__
#ifdef __cplusplus
     extern  "C" long  __stdcall MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo );
#else
     extern  long  __stdcall MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo );
#endif
#endif
     

     //      Replacement for round(x)???
#ifdef __cplusplus
     extern "C"  double     round_msvc (double flt);
#else
     extern double round_msvc (double flt);
#endif /* __cplusplus */
     
     
inline int roundint (double x)
{
#ifdef __WXOSX__
    return (int)round(x);     //FS#1278
#else
    int tmp = static_cast<int> (x);
    tmp += (x-tmp>=.5) - (x-tmp<=-.5);
    return tmp;
#endif    
}
     
     

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

typedef enum { Visible, Invisible } ClipResult;

#ifdef __cplusplus
extern "C"  ClipResult cohen_sutherland_line_clip_d (double *x0, double *y0, double *x1, double *y1,
                                         double xmin_, double xmax_, double ymin_, double ymax_);

extern "C"  ClipResult cohen_sutherland_line_clip_i (int *x0, int *y0, int *x1, int *y1,
                                             int xmin_, int xmax_, int ymin_, int ymax_);

#endif


//      Simple and fast CRC32 calculator

extern "C" unsigned int crc32buf(unsigned char *buf, size_t len);


#endif
