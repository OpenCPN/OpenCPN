/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN ViewPort
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register   *
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
 *
 */

#ifndef __OCPNVIEWPORT_H__
#define __OCPNVIEWPORT_H__


#include "bbox.h"
class OCPNRegion;
class LLRegion;

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

#if 0
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
#endif

#define INVALID_COORD (-2147483647 - 1)

//----------------------------------------------------------------------------
// ViewPort Definition
//----------------------------------------------------------------------------
class ViewPort
{
      public:
            ViewPort();

            wxPoint GetPixFromLL(double lat, double lon);
            void GetLLFromPix(const wxPoint &p, double *lat, double *lon) { GetLLFromPix(wxPoint2DDouble(p), lat, lon); }
            void GetLLFromPix(const wxPoint2DDouble &p, double *lat, double *lon);
            wxPoint2DDouble GetDoublePixFromLL(double lat, double lon);

            LLRegion GetLLRegion( const OCPNRegion &region );
            OCPNRegion GetVPRegionIntersect( const OCPNRegion &region, const LLRegion &llregion, int chart_native_scale );
            OCPNRegion GetVPRegionIntersect( const OCPNRegion &Region, size_t nPoints, float *llpoints,
                                             int chart_native_scale, wxPoint *ppoints );
            wxRect GetVPRectIntersect( size_t n, float *llpoints );
            ViewPort BuildExpandedVP(int width, int height);
            
            void SetBoxes(void);

//  Accessors
            void Invalidate() { bValid = false; }
            void Validate() { bValid = true; }
            bool IsValid() const { return bValid; }

            void SetRotationAngle(double angle_rad) { rotation = angle_rad;}
            void SetProjectionType(int type){ m_projection_type = type; }

            LLBBox &GetBBox() { return vpBBox; }

            void SetBBoxDirect( const LLBBox &bbox ) { vpBBox = bbox; }
            void SetBBoxDirect( double latmin, double lonmin, double latmax, double lonmax);

            void InvalidateTransformCache() { lat0_cache = NAN; }
            
//  Generic
            double   clat;                   // center point
            double   clon;
            double   view_scale_ppm;
            double   skew;
            double   rotation;
            double   tilt;  // For perspective view

            double    chart_scale;            // conventional chart displayed scale
            double    ref_scale;              //  the nominal scale of the "reference chart" for this view

            int      pix_width;
            int      pix_height;

            bool     b_quilt;
            bool     b_FullScreenQuilt;

            int      m_projection_type;
            bool     b_MercatorProjectionOverride;
            wxRect   rv_rect;

      private:
            LLBBox   vpBBox;                // An un-skewed rectangular lat/lon bounding box
                                            // which contains the entire vieport

            bool     bValid;                 // This VP is valid

            double lat0_cache, cache0, cache1;
};


#endif
