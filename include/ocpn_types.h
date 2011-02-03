/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN private types and ENUMs
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 *
 *
 */




#ifndef __OCPNTYPES_H__
#define __OCPNTYPES_H__

#include "bbox.h"

//    ChartType constants
typedef enum ChartTypeEnum
{
      CHART_TYPE_UNKNOWN = 0,
      CHART_TYPE_DUMMY,
      CHART_TYPE_DONTCARE,
      CHART_TYPE_KAP,
      CHART_TYPE_GEO,
      CHART_TYPE_S57,
      CHART_TYPE_CM93,
      CHART_TYPE_CM93COMP,
      CHART_TYPE_PLUGIN
}_ChartTypeEnum;

//    ChartFamily constants
typedef enum ChartFamilyEnum
{
      CHART_FAMILY_UNKNOWN = 0,
      CHART_FAMILY_RASTER,
      CHART_FAMILY_VECTOR,
      CHART_FAMILY_DONTCARE
}_ChartFamilyEnum;

typedef enum ColorScheme
{
      GLOBAL_COLOR_SCHEME_RGB,
      GLOBAL_COLOR_SCHEME_DAY,
      GLOBAL_COLOR_SCHEME_DUSK,
      GLOBAL_COLOR_SCHEME_NIGHT,
      N_COLOR_SCHEMES
}_ColorScheme;

typedef enum ScaleTypeEnum
{
      RENDER_LODEF = 0,
      RENDER_HIDEF,
}_ScaleTypeEnum;

//----------------------------------------------------------------------------
// ViewPort
//    Implementation is in chcanv.cpp
//----------------------------------------------------------------------------
class ViewPort
{
      public:
//  ctor
            ViewPort()  { bValid = false; skew = 0.; view_scale_ppm = 1; rotation = 0.; b_quilt = false;}

            wxPoint GetMercatorPixFromLL(double lat, double lon) const;
            void GetMercatorLLFromPix(const wxPoint &p, double *lat, double *lon);

            wxRegion GetVPRegion( size_t n, float *llpoints, int chart_native_scale, wxPoint *ppoints = NULL );

//  Accessors
            void Invalidate() { bValid = false; }
            bool IsValid() { return bValid; }
            void SetRotationAngle(double angle_rad) { rotation = angle_rad;}
            void SetProjectionType(int type){ m_projection_type = type; }
//  Generic
            double   clat;                   // center point
            double   clon;
            double   view_scale_ppm;
            double   skew;
            double   rotation;


            LLBBox   vpBBox;                // An un-skewed rectangular lat/lon bounding box
                                    // which contains the entire vieport

            float    chart_scale;            // conventional chart displayed scale

            int      pix_width;
            int      pix_height;
            wxRect   rv_rect;
            bool     b_quilt;
            int      m_projection_type;

            bool     bValid;                 // This VP is valid

};





#endif
