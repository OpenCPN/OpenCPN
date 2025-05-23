/***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/

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
 * ViewPort - Core geographic projection and coordinate transformation engine
 *
 * Represents the fundamental viewing parameters and coordinate transformations
 * for chart display. This class serves as the mathematical foundation for
 * mapping between geographic coordinates (latitude/longitude) and screen
 * coordinates (pixels). It encapsulates all parameters that define how a chart
 * section is viewed, including position, scale, rotation, and projection.
 *
 * ViewPort is designed to be a relatively lightweight, data-focused class that
 * handles the complex mathematical operations needed for accurate chart
 * rendering without direct dependencies on UI components.
 */
class ViewPort {
public:
  ViewPort();

  /**
   * Convert latitude and longitude on the ViewPort to physical pixel
   * coordinates.
   *
   * @param lat Latitude in degrees.
   * @param lon Longitude in degrees.
   * @return wxPoint Pixel coordinates.
   *
   * @see ChartCanvas::GetCanvasPointPix() for the canvas-level implementation.
   */
  wxPoint GetPixFromLL(double lat, double lon);
  /**
   * Convert physical pixel coordinates on the ViewPort to latitude and
   * longitude.
   * @param p Physical pixel coordinates.
   * @param lat Pointer to store resulting latitude.
   * @param lon Pointer to store resulting longitude.
   *
   * @see ChartCanvas::GetCanvasPixPoint() for the canvas-level implementation.
   */
  void GetLLFromPix(const wxPoint &p, double *lat, double *lon) {
    GetLLFromPix(wxPoint2DDouble(p), lat, lon);
  }
  /**
   * Convert physical pixel coordinates on the ViewPort to latitude and
   * longitude using double precision.
   * @param p Physical pixel coordinates as wxPoint2DDouble.
   * @param lat Pointer to store resulting latitude.
   * @param lon Pointer to store resulting longitude.
   */
  void GetLLFromPix(const wxPoint2DDouble &p, double *lat, double *lon);
  /**
   * Convert latitude and longitude on the ViewPort to physical pixel
   * coordinates with double precision.
   * @param lat Latitude in degrees.
   * @param lon Longitude in degrees.
   * @return wxPoint2DDouble Physical pixel coordinates.
   */
  wxPoint2DDouble GetDoublePixFromLL(double lat, double lon);

  LLRegion GetLLRegion(const OCPNRegion &region);
  /**
   * Get the intersection of the viewport with a given region.
   * @param region OCPNRegion to intersect with
   * @param llregion LLRegion to use for the intersection
   * @param chart_native_scale Native scale of the chart
   * @return OCPNRegion Resulting intersected region
   */
  OCPNRegion GetVPRegionIntersect(const OCPNRegion &region,
                                  const LLRegion &llregion,
                                  int chart_native_scale);

  /**
   * Get the intersection of the viewport with a polygon defined by lat/lon
   * points.
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
   * Get the viewport rectangle intersecting with a set of lat/lon points.
   * @param n Number of points
   * @param llpoints Array of lat/lon points
   * @return wxRect Intersecting rectangle
   */
  wxRect GetVPRectIntersect(size_t n, float *llpoints);
  ViewPort BuildExpandedVP(int width, int height);

  /**
   * Computes the bounding box coordinates for the current viewport.
   * This function is responsible for determining the lat/lon boundaries of the
   * current viewport, which are used for various purposes including plugin
   * rendering.
   *
   * The function calculates a larger "virtual" pixel window size when rotation
   * is applied to ensure that enough chart data is fetched to fill the rotated
   * screen. It then computes the viewport lat/lon reference points based on
   * screen coordinates.
   *
   * Different algorithms are used depending on the projection type (POLAR,
   * ORTHOGRAPHIC, STEREOGRAPHIC, GNOMONIC, MERCATOR, EQUIRECTANGULAR).
   *
   * Edge cases are handled such as:
   * - IDL (International Date Line) crossings
   * - Poles being visible on screen
   * - Non-rectangular mappings between screen space and geographical
   * coordinates
   *
   * The computed bounding box is stored in vpBBox and is used by various parts
   * of the program to determine which chart features should be rendered and
   * which are outside the visible area.
   *
   * @note When rotation or skew is applied, the function creates a larger
   * rectangle that encompasses the rotated viewport, which can lead to data
   * being fetched and rendered for areas that may be just outside the visible
   * part of the screen.
   */
  void SetBoxes(void);
  /**
   * Set the physical to logical pixel ratio for the display.
   *
   * On standard displays, one logical pixel equals one physical pixel, so this
   * value is 1.0. On high-DPI/Retina displays, one logical pixel may equal
   * multiple physical pixels:
   * - MacBook Pro Retina: 2.0 (2x2 physical pixels per logical pixel)
   * - Other HiDPI displays: May be 1.5, 1.75, etc.
   *
   * @param scale The ratio of physical pixels to logical pixels.
   */
  void SetPixelScale(double scale);

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
  /** Center latitude of the viewport in degrees. */
  double clat;
  /** Center longitude of the viewport in degrees. */
  double clon;
  /**
   * Requested view scale in physical pixels per meter (ppm), before applying
   * projections.
   */
  double view_scale_ppm;
  /**
   * Angular distortion (shear transform) applied to the viewport in radians.
   *
   * The skew parameter represents a shear transformation applied to the
   * viewport, which maintains parallel lines while changing their angles
   * relative to the axes.
   */
  double skew;
  /** Rotation angle of the viewport in radians. */
  double rotation;
  /** Tilt angle for perspective view in radians. */
  double tilt;

  /** Chart scale denominator (e.g., 50000 for a 1:50000 scale). */
  double chart_scale;
  /** The nominal scale of the "reference chart" for this view. */
  double ref_scale;

  /**
   * Width of the viewport in physical pixels.
   *
   * This may be larger than the actual screen width if the viewport is expanded
   * for operations like text rendering or to accommodate rotation.
   *
   * @see BuildExpandedVP() for how this is modified for text rendering
   */
  int pix_width;
  /** Height of the viewport in physical pixels. */
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
  /** An un-skewed rectangular lat/lon bounding box which contains the entire
   * viewport. */
  LLBBox vpBBox;

  bool bValid;  // This VP is valid

  double lat0_cache, cache0, cache1;

  /** The ratio of physical pixel to logical pixels. */
  double m_displayScale;
};

#endif
