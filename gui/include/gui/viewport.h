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
#define NAN (*(double *)&lNaN)
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

/**
 * Represents the view port for chart display in OpenCPN. Encapsulates all
 * parameters that define the current view of the chart, including position,
 * scale, rotation, and projection type. Provides methods for coordinate
 * conversions and viewport manipulations.
 */
class ViewPort {
public:
  ViewPort();

  /**
   * Convert latitude and longitude to pixel coordinates.
   *
   * @param lat Latitude in degrees
   * @param lon Longitude in degrees
   * @return wxPoint Pixel coordinates
   */
  wxPoint GetPixFromLL(double lat, double lon);
  /**
   * @brief Convert pixel coordinates to latitude and longitude
   * @param p Pixel coordinates
   * @param lat Pointer to store resulting latitude
   * @param lon Pointer to store resulting longitude
   */
  void GetLLFromPix(const wxPoint &p, double *lat, double *lon) {
    GetLLFromPix(wxPoint2DDouble(p), lat, lon);
  }
  /**
   * @brief Convert pixel coordinates to latitude and longitude using double
   * precision
   * @param p Pixel coordinates as wxPoint2DDouble
   * @param lat Pointer to store resulting latitude
   * @param lon Pointer to store resulting longitude
   */
  void GetLLFromPix(const wxPoint2DDouble &p, double *lat, double *lon);
  /**
   * @brief Convert latitude and longitude to pixel coordinates with double
   * precision
   * @param lat Latitude in degrees
   * @param lon Longitude in degrees
   * @return wxPoint2DDouble Pixel coordinates
   */
  wxPoint2DDouble GetDoublePixFromLL(double lat, double lon);

  LLRegion GetLLRegion(const OCPNRegion &region);
  /**
   * @brief Get the intersection of the viewport with a given region
   * @param region OCPNRegion to intersect with
   * @param llregion LLRegion to use for the intersection
   * @param chart_native_scale Native scale of the chart
   * @return OCPNRegion Resulting intersected region
   */
  OCPNRegion GetVPRegionIntersect(const OCPNRegion &region,
                                  const LLRegion &llregion,
                                  int chart_native_scale);

  /**
   * @brief Get the intersection of the viewport with a polygon defined by
   * lat/lon points
   * @param Region OCPNRegion to intersect with
   * @param nPoints Number of points in the polygon
   * @param llpoints Array of lat/lon points defining the polygon
   * @param chart_native_scale Native scale of the chart
   * @param ppoints Array to store resulting pixel coordinates
   * @return OCPNRegion Resulting intersected region
   */
  OCPNRegion GetVPRegionIntersect(const OCPNRegion &Region, int nPoints,
                                  float *llpoints, int chart_native_scale,
                                  wxPoint *ppoints);
  /**
   * @brief Get the viewport rectangle intersecting with a set of lat/lon points
   * @param n Number of points
   * @param llpoints Array of lat/lon points
   * @return wxRect Intersecting rectangle
   */
  wxRect GetVPRectIntersect(size_t n, float *llpoints);
  ViewPort BuildExpandedVP(int width, int height);

  void SetBoxes(void);
  void PixelScale(float factor);

  //  Accessors
  void Invalidate() { bValid = false; }
  void Validate() { bValid = true; }
  bool IsValid() const { return bValid; }

  void SetRotationAngle(double angle_rad) { rotation = angle_rad; }
  void SetProjectionType(int type) { m_projection_type = type; }

  LLBBox &GetBBox() { return vpBBox; }

  void SetBBoxDirect(const LLBBox &bbox) { vpBBox = bbox; }
  void SetBBoxDirect(double latmin, double lonmin, double latmax,
                     double lonmax);

  bool ContainsIDL();
  void InvalidateTransformCache() { lat0_cache = NAN; }
  void SetVPTransformMatrix();

  //  Generic
  double clat;  // center point
  double clon;
  double view_scale_ppm;
  double skew;
  double rotation;
  double tilt;  // For perspective view

  double chart_scale;  // conventional chart displayed scale
  double
      ref_scale;  //  the nominal scale of the "reference chart" for this view

  int pix_width;
  int pix_height;

  bool b_quilt;
  bool b_FullScreenQuilt;

  int m_projection_type;
  bool b_MercatorProjectionOverride;
  wxRect rv_rect;

  // #ifdef USE_ANDROID_GLES2
  float vp_matrix_transform[16];
  float norm_transform[16];
  // #endif

  bool operator==(const ViewPort &rhs) const {
    return (clat == rhs.clat) && (clon == rhs.clon) &&
           (view_scale_ppm == rhs.view_scale_ppm) && (skew == rhs.skew) &&
           (rotation == rhs.rotation) && (tilt == rhs.tilt) &&
           (chart_scale == rhs.chart_scale) && (ref_scale == rhs.ref_scale) &&
           (pix_width == rhs.pix_width) && (pix_height == rhs.pix_height) &&
           (b_quilt == rhs.b_quilt) &&
           (b_FullScreenQuilt == rhs.b_FullScreenQuilt) &&
           (m_projection_type == rhs.m_projection_type) &&
           (b_MercatorProjectionOverride == rhs.b_MercatorProjectionOverride);
  }

private:
  LLBBox vpBBox;  // An un-skewed rectangular lat/lon bounding box
                  // which contains the entire vieport

  bool bValid;  // This VP is valid

  double lat0_cache, cache0, cache1;
};

#endif
