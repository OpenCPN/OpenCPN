/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Georef utility
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

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 ***************************************************************************

 */

#ifndef _GEOREF_H__
#define _GEOREF_H__

#include <stdio.h>
#include <string.h>
#include <ctype.h>

//------------------------
struct DATUM {
  const char *name;
  short ellipsoid;
  double dx;
  double dy;
  double dz;
};

struct ELLIPSOID {
  const char *name;  // name of ellipsoid
  double a;          // semi-major axis, meters
  double invf;       // 1/f
};

/**
 * Structure containing georeferencing information for transforming between
 * geographic and projected/pixel coordinate systems.
 *
 * This structure holds all necessary data to perform polynomial-based
 * coordinate transformations between world coordinates (latitude/longitude) and
 * target coordinates (typically pixels in a chart or image). It stores
 * reference points, transformation coefficients, and boundary information used
 * in the conversion process.
 *
 * The structure supports transformations of different polynomial orders (1-3)
 * with increasing accuracy and complexity. Polynomial coefficients are
 * calculated using the Georef_Calculate_Coefficients() functions.
 */
struct GeoRef {
  int status;   //!< Status of the georeferencing (0 = valid, other values
                //!< indicate errors)
  int count;    //!< Number of reference points used
  int order;    //!< Polynomial order for the transformation (1, 2, or 3)
  double *tx;   //!< Array of x-coordinates in target (typically pixel) space
  double *ty;   //!< Array of y-coordinates in target (typically pixel) space
  double *lon;  //!< Array of longitudes corresponding to reference points
  double *lat;  //!< Array of latitudes corresponding to reference points
  double *pwx;  //!< Polynomial coefficients for pixel-to-world longitude
                //!< transformation
  double *pwy;  //!< Polynomial coefficients for pixel-to-world latitude
                //!< transformation
  double *wpx;  //!< Polynomial coefficients for world-to-pixel x transformation
  double *wpy;  //!< Polynomial coefficients for world-to-pixel y transformation
  int txmax;    //!< Maximum x value in target space
  int tymax;    //!< Maximum y value in target space
  int txmin;    //!< Minimum x value in target space
  int tymin;    //!< Minimum y value in target space
  double lonmax;  //!< Maximum longitude in reference data
  double lonmin;  //!< Minimum longitude in reference data
  double latmax;  //!< Maximum latitude in reference data
  double latmin;  //!< Minimum latitude in reference data
};

#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif
#define DEGREE (PI / 180.0)
#define RADIAN (180.0 / PI)

#define DATUM_INDEX_WGS84 101
#define DATUM_INDEX_UNKNOWN -1

static const double WGS84_semimajor_axis_meters =
    6378137.0;  // WGS84 semimajor axis
static const double mercator_k0 = 0.9996;
static const double WGSinvf = 298.257223563; /* WGS84 1/f */

void datumParams(short datum, double *a, double *es);

//      Make these conversion routines useable by C or C++

#ifdef __cplusplus
/**
 * Converts decimal degrees to degrees-minutes-seconds (DMS) format.
 *
 * This function formats a decimal angle value into the traditional DMS notation
 * (DD°MM'SS.S") and places the result in the provided buffer.
 *
 * @param a Angle in decimal degrees to be converted.
 * @param bufp Buffer to store the resulting DMS string.
 * @param bufplen Length of the provided buffer.
 */
extern "C" void toDMS(double a, char *bufp, int bufplen);
/**
 * Converts decimal degrees to degrees-decimal minutes (DMM) format.
 *
 * This function formats a decimal angle value into the degrees and decimal
 * minutes notation (DD MM.MMM') and places the result in the provided buffer.
 *
 * @param a Angle in decimal degrees to be converted
 * @param bufp Buffer to store the resulting DMM string
 * @param bufplen Length of the provided buffer
 */
extern "C" void toDMM(double a, char *bufp, int bufplen);

/**
 * Converts decimal degrees to degrees-decimal minutes (DMM) format with
 * optional hemisphere indicator.
 *
 * This function formats a decimal angle value into degrees and decimal minutes,
 * with flexible formatting options controlled by the flag parameter.
 *
 * @param flag Formatting control: 0 for standard format, 1 for latitude with
 * N/S hemisphere, 2 for longitude with E/W hemisphere
 * @param a Angle in decimal degrees to be converted
 * @param bufp Buffer to store the resulting DMM string
 * @param bufplen Length of the provided buffer
 */
extern "C" void todmm(int flag, double a, char *bufp, int bufplen);

/**
 * Converts geographic coordinates to Transverse Mercator projection
 * coordinates.
 *
 * This function implements the direct Transverse Mercator projection,
 * calculating easting (x) and northing (y) coordinates from geographic
 * coordinates. The projection is centered at the specified reference point
 * (lat0, lon0).
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param lat0 Latitude of the central point/origin in decimal degrees
 * @param lon0 Longitude of the central meridian in decimal degrees
 * @param x Pointer to store the resulting easting coordinate in meters
 * @param y Pointer to store the resulting northing coordinate in meters
 */
extern "C" void toTM(float lat, float lon, float lat0, float lon0, double *x,
                     double *y);

/**
 * Converts Transverse Mercator projection coordinates to geographic
 * coordinates.
 *
 * This function implements the inverse Transverse Mercator projection,
 * calculating latitude and longitude from projected easting (x) and northing
 * (y) coordinates. The projection is centered at the specified reference point
 * (lat0, lon0).
 *
 * @param x Easting coordinate in the TM projection (in meters)
 * @param y Northing coordinate in the TM projection (in meters)
 * @param lat0 Latitude of the central point/origin in decimal degrees
 * @param lon0 Longitude of the central meridian in decimal degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromTM(double x, double y, double lat0, double lon0,
                       double *lat, double *lon);

/**
 * Converts geographic coordinates to Simple Mercator projection coordinates.
 *
 * This function implements the direct Mercator projection relative to a
 * reference point, calculating x and y coordinates from geographic coordinates.
 * The projection is centered at the specified reference point (lat0, lon0).
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param x Pointer to store the resulting x-coordinate in meters
 * @param y Pointer to store the resulting y-coordinate in meters
 */
extern "C" void toSM(double lat, double lon, double lat0, double lon0,
                     double *x, double *y);

/**
 * Pre-calculates a value needed for optimized Simple Mercator projection.
 *
 * This function calculates the y-value at the reference latitude that will be
 * subtracted in the toSMcache function. Use this with toSMcache for better
 * performance when projecting many points with the same reference latitude.
 *
 * @param lat0 Reference latitude in decimal degrees
 * @return Pre-calculated y-value for the reference latitude
 */
extern "C" double toSMcache_y30(double lat0);

/**
 * Converts geographic coordinates to Simple Mercator projection with
 * pre-calculated parameter.
 *
 * This is an optimized version of toSM that uses a pre-calculated y30 value
 * from toSMcache_y30, which avoids redundant calculations when processing
 * multiple points with the same reference latitude.
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param y30 Pre-calculated value from toSMcache_y30 for the reference latitude
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param x Pointer to store the resulting x-coordinate in meters
 * @param y Pointer to store the resulting y-coordinate in meters
 */
extern "C" void toSMcache(double lat, double lon, double y30, double lon0,
                          double *x, double *y);
/**
 * Converts Simple Mercator projection coordinates to geographic coordinates.
 *
 * This function implements the inverse Mercator projection, calculating
 * latitude and longitude from x and y coordinates. The projection is centered
 * at the specified reference point (lat0, lon0) and uses the WGS84 semimajor
 * axis.
 *
 * @param x X-coordinate in the Mercator projection (in meters)
 * @param y Y-coordinate in the Mercator projection (in meters)
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromSM(double x, double y, double lat0, double lon0,
                       double *lat, double *lon);

/**
 * Converts Simple Mercator projection coordinates to geographic coordinates
 * with custom Earth radius.
 *
 * This is a variant of fromSM that allows specifying a custom Earth radius
 * instead of using the WGS84 semimajor axis. This can be useful for specialized
 * projections.
 *
 * @param x X-coordinate in the Mercator projection (in meters)
 * @param y Y-coordinate in the Mercator projection (in meters)
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param axis_meters Custom Earth radius in meters
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromSMR(double x, double y, double lat0, double lon0,
                        double axis_meters, double *lat, double *lon);

/**
 * Converts geographic coordinates to Simple Mercator projection with
 * eccentricity correction.
 *
 * This is an enhanced version of toSM that accounts for the Earth's
 * eccentricity, providing more accurate projections, especially at higher
 * latitudes. It uses the WGS84 ellipsoid parameters.
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param x Pointer to store the resulting x-coordinate in meters
 * @param y Pointer to store the resulting y-coordinate in meters
 */
extern "C" void toSM_ECC(double lat, double lon, double lat0, double lon0,
                         double *x, double *y);

/**
 * Converts Simple Mercator projection coordinates to geographic coordinates
 * with eccentricity correction.
 *
 * This is the inverse function of toSM_ECC, accounting for the Earth's
 * eccentricity when converting from projected coordinates back to geographic
 * coordinates. It uses the WGS84 ellipsoid parameters.
 *
 * @param x X-coordinate in the Mercator projection (in meters)
 * @param y Y-coordinate in the Mercator projection (in meters)
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromSM_ECC(double x, double y, double lat0, double lon0,
                           double *lat, double *lon);

/**
 * Converts geographic coordinates to Polyconic projection coordinates.
 *
 * This function implements the direct Polyconic projection, calculating x and y
 * coordinates from latitude and longitude. The projection is centered at the
 * specified reference point. The Polyconic projection maintains scale along
 * each parallel and the central meridian.
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param x Pointer to store the resulting x-coordinate in meters
 * @param y Pointer to store the resulting y-coordinate in meters
 */
extern "C" void toPOLY(double lat, double lon, double lat0, double lon0,
                       double *x, double *y);

/**
 * Converts Polyconic projection coordinates to geographic coordinates.
 *
 * This function implements the inverse Polyconic projection, calculating
 * latitude and longitude from x and y coordinates. The projection is centered
 * at the specified reference point. This is the inverse of toPOLY.
 *
 * @param x X-coordinate in the Polyconic projection (in meters)
 * @param y Y-coordinate in the Polyconic projection (in meters)
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromPOLY(double x, double y, double lat0, double lon0,
                         double *lat, double *lon);

/**
 * Pre-calculates trigonometric values for orthographic projection functions.
 *
 * This helper function calculates the sine and cosine of the reference
 * latitude, which are used repeatedly in orthographic projection calculations.
 * Using this function improves performance when projecting multiple points with
 * the same reference latitude.
 *
 * @param lat0 Reference latitude in decimal degrees
 * @param sin_phi0 Pointer to store the sine of the reference latitude
 * @param cos_phi0 Pointer to store the cosine of the reference latitude
 */
extern "C" void cache_phi0(double lat0, double *sin_phi0, double *cos_phi0);

/**
 * Converts geographic coordinates to Orthographic projection coordinates.
 *
 * This function implements the direct Orthographic projection, a perspective
 * projection that views the globe from an infinite distance. Points on the far
 * side of the Earth from the projection center will return NaN values.
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param sin_phi0 Sine of the reference latitude (pre-computed for efficiency)
 * @param cos_phi0 Cosine of the reference latitude (pre-computed for
 * efficiency)
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param x Pointer to store the resulting x-coordinate in meters
 * @param y Pointer to store the resulting y-coordinate in meters
 */
extern "C" void toORTHO(double lat, double lon, double sin_phi0,
                        double cos_phi0, double lon0, double *x, double *y);

/**
 * Converts Orthographic projection coordinates to geographic coordinates.
 *
 * This function implements the inverse Orthographic projection, calculating
 * latitude and longitude from x and y coordinates. This is the inverse of
 * toORTHO.
 *
 * @param x X-coordinate in the Orthographic projection (in meters)
 * @param y Y-coordinate in the Orthographic projection (in meters)
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromORTHO(double x, double y, double lat0, double lon0,
                          double *lat, double *lon);

/**
 * Pre-calculates a parameter for Polar projection.
 *
 * This helper function calculates a value that depends on the reference
 * latitude, used in Polar projection calculations. Using this function improves
 * performance when projecting multiple points with the same reference latitude.
 *
 * @param lat0 Reference latitude in decimal degrees
 * @return Pre-calculated eccentricity value for the reference latitude
 */
extern "C" double toPOLARcache_e(double lat0);

/**
 * Converts geographic coordinates to Polar projection coordinates.
 *
 * This function implements the direct Polar projection (a form of azimuthal
 * projection) that is especially useful for representing polar regions with
 * minimal distortion.
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param e Pre-calculated parameter from toPOLARcache_e
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param x Pointer to store the resulting x-coordinate in meters
 * @param y Pointer to store the resulting y-coordinate in meters
 */
extern "C" void toPOLAR(double lat, double lon, double e, double lat0,
                        double lon0, double *x, double *y);

/**
 * Converts Polar projection coordinates to geographic coordinates.
 *
 * This function implements the inverse Polar projection, calculating latitude
 * and longitude from x and y coordinates. This is the inverse of toPOLAR.
 *
 * @param x X-coordinate in the Polar projection (in meters)
 * @param y Y-coordinate in the Polar projection (in meters)
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromPOLAR(double x, double y, double lat0, double lon0,
                          double *lat, double *lon);

/**
 * Converts geographic coordinates to stereographic projection coordinates.
 *
 * This function performs the stereographic projection, mapping
 * latitude/longitude to x/y coordinates. The stereographic projection is
 * conformal (preserves angles) and is useful for mapping polar regions. The
 * projection is centered at the point specified by sin_phi0/cos_phi0 and lon0.
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param sin_phi0 Sine of the reference latitude (pre-computed for efficiency)
 * @param cos_phi0 Cosine of the reference latitude (pre-computed for
 * efficiency)
 * @param lon0 Longitude of the reference point/projection center in decimal
 * degrees
 * @param x Pointer to store the resulting X-coordinate in meters
 * @param y Pointer to store the resulting Y-coordinate in meters
 */
extern "C" void toSTEREO(double lat, double lon, double sin_phi0,
                         double cos_phi0, double lon0, double *x, double *y);
/**
 * Converts stereographic projection coordinates to geographic coordinates.
 *
 * This function performs the inverse stereographic projection, converting
 * projected x/y coordinates back to latitude/longitude. The stereographic
 * projection is centered at the specified reference point.
 *
 * @param x X-coordinate in the stereographic projection (in meters)
 * @param y Y-coordinate in the stereographic projection (in meters)
 * @param lat0 Latitude of the reference point/projection center in decimal
 * degrees
 * @param lon0 Longitude of the reference point/projection center in decimal
 * degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromSTEREO(double x, double y, double lat0, double lon0,
                           double *lat, double *lon);

/**
 * Converts geographic coordinates to gnomonic projection coordinates.
 *
 * This function performs the gnomonic projection, mapping latitude/longitude to
 * x/y coordinates. The gnomonic projection has the special property that all
 * great circles map to straight lines. The projection is centered at the point
 * specified by sin_phi0/cos_phi0 and lon0. Points on the opposite side of the
 * Earth from the projection center will result in NaN values.
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param sin_phi0 Sine of the reference latitude (pre-computed for efficiency)
 * @param cos_phi0 Cosine of the reference latitude (pre-computed for
 * efficiency)
 * @param lon0 Longitude of the reference point/projection center in decimal
 * degrees
 * @param x Pointer to store the resulting X-coordinate in meters
 * @param y Pointer to store the resulting Y-coordinate in meters
 */
extern "C" void toGNO(double lat, double lon, double sin_phi0, double cos_phi0,
                      double lon0, double *x, double *y);
/**
 * Converts gnomonic projection coordinates to geographic coordinates.
 *
 * This function performs the inverse gnomonic projection, converting projected
 * x/y coordinates back to latitude/longitude. The gnomonic projection is
 * centered at the specified reference point. Points on the opposite side of the
 * Earth from the reference point will result in NaN values.
 *
 * @param x X-coordinate in the gnomonic projection (in meters)
 * @param y Y-coordinate in the gnomonic projection (in meters)
 * @param lat0 Latitude of the reference point/projection center in decimal
 * degrees
 * @param lon0 Longitude of the reference point/projection center in decimal
 * degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromGNO(double x, double y, double lat0, double lon0,
                        double *lat, double *lon);

/**
 * Converts geographic coordinates to Equirectangular projection coordinates.
 *
 * This function implements the direct Equirectangular (Plate Carrée)
 * projection, which maps meridians and parallels to equally spaced vertical and
 * horizontal lines. It's one of the simplest map projections but has
 * significant distortion away from the reference latitude.
 *
 * @param lat Latitude in decimal degrees to be projected
 * @param lon Longitude in decimal degrees to be projected
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param x Pointer to store the resulting x-coordinate in meters
 * @param y Pointer to store the resulting y-coordinate in meters
 */
extern "C" void toEQUIRECT(double lat, double lon, double lat0, double lon0,
                           double *x, double *y);

/**
 * Converts Equirectangular projection coordinates to geographic coordinates.
 *
 * This function implements the inverse Equirectangular projection, calculating
 * latitude and longitude from x and y coordinates. This is the inverse of
 * toEQUIRECT.
 *
 * @param x X-coordinate in the Equirectangular projection (in meters)
 * @param y Y-coordinate in the Equirectangular projection (in meters)
 * @param lat0 Latitude of the reference point in decimal degrees
 * @param lon0 Longitude of the reference point in decimal degrees
 * @param lat Pointer to store the resulting latitude in decimal degrees
 * @param lon Pointer to store the resulting longitude in decimal degrees
 */
extern "C" void fromEQUIRECT(double x, double y, double lat0, double lon0,
                             double *lat, double *lon);

/**
 * Calculates the destination point given starting point, bearing and distance.
 *
 * Given an initial position, bearing, and distance, this function calculates
 * the destination point following a great circle path. This is essentially the
 * forward geodesic problem.
 *
 * @param lat Latitude of starting point in decimal degrees
 * @param lon Longitude of starting point in decimal degrees
 * @param crs Bearing (course) in decimal degrees
 * @param dist Distance to travel in nautical miles
 * @param dlat Pointer to store the latitude of destination point in decimal
 * degrees
 * @param dlon Pointer to store the longitude of destination point in decimal
 * degrees
 */
extern "C" void ll_gc_ll(double lat, double lon, double crs, double dist,
                         double *dlat, double *dlon);
/**
 * Calculates the great circle distance and initial bearing between two points.
 *
 * This function calculates both the distance and the initial bearing (azimuth)
 * along the great circle path between two specified geographic coordinates. For
 * very small distances, it falls back to Mercator calculations to prevent
 * numerical instability.
 *
 * @param lat1 Latitude of starting point in decimal degrees
 * @param lon1 Longitude of starting point in decimal degrees
 * @param lat2 Latitude of ending point in decimal degrees
 * @param lon2 Longitude of ending point in decimal degrees
 * @param bearing Pointer to store the initial bearing in decimal degrees
 * @param dist Pointer to store the distance in nautical miles
 */
extern "C" void ll_gc_ll_reverse(double lat1, double lon1, double lat2,
                                 double lon2, double *bearing, double *dist);

/**
 * Calculates the destination point using great circle navigation.
 *
 * This function is a wrapper around ll_gc_ll() that calculates the position of
 * a point given a starting position, bearing, and distance. Despite the name
 * suggesting Mercator calculations, this function actually uses great circle
 * calculations.
 *
 * @param lat Latitude of starting point in decimal degrees
 * @param lon Longitude of starting point in decimal degrees
 * @param brg Bearing (course) in decimal degrees
 * @param dist Distance to travel in nautical miles
 * @param dlat Pointer to store the latitude of destination point in decimal
 * degrees
 * @param dlon Pointer to store the longitude of destination point in decimal
 * degrees
 */
extern "C" void PositionBearingDistanceMercator(double lat, double lon,
                                                double brg, double dist,
                                                double *dlat, double *dlon);
/**
 * Calculates the great circle distance between two points.
 *
 * This function calculates the shortest distance between two points on the
 * Earth, following a great circle path. For small distances (less than 10
 * nautical miles), it uses the more efficient loxodromic calculation.
 *
 * @param slat Latitude of starting point in decimal degrees
 * @param slon Longitude of starting point in decimal degrees
 * @param dlat Latitude of destination point in decimal degrees
 * @param dlon Longitude of destination point in decimal degrees
 * @return Distance between the points in nautical miles
 */
extern "C" double DistGreatCircle(double slat, double slon, double dlat,
                                  double dlon);
/**
 * Calculates the loxodromic (rhumb line) distance between two points.
 *
 * This function calculates the distance along a rhumb line (line of constant
 * bearing) between two geographic points. It handles special cases where points
 * might be on opposite sides of the International Date Line or Greenwich
 * Meridian.
 *
 * @param slat Latitude of starting point in decimal degrees
 * @param slon Longitude of starting point in decimal degrees
 * @param dlat Latitude of destination point in decimal degrees
 * @param dlon Longitude of destination point in decimal degrees
 * @return Distance between the points in nautical miles
 */
extern "C" double DistLoxodrome(double slat, double slon, double dlat,
                                double dlon);

/**
 * Calculates the destination point using rhumb line (loxodrome) navigation.
 *
 * Given an initial position, bearing, and distance, this function calculates
 * the destination point following a rhumb line path (constant bearing). This
 * is the proper rhumb line counterpart to PositionBearingDistanceMercator,
 * which uses great circle calculations.
 *
 * A rhumb line is a line of constant bearing, which appears as a straight line
 * on a Mercator projection chart. While rhumb lines are longer than great
 * circle routes, they are easier to navigate as the bearing remains constant.
 *
 * @param lat Latitude of starting point in decimal degrees
 * @param lon Longitude of starting point in decimal degrees
 * @param brg Bearing (course) in decimal degrees (0-360, 0=North, 90=East)
 * @param dist Distance to travel in nautical miles
 * @param dlat Pointer to store the latitude of destination point in decimal
 * degrees
 * @param dlon Pointer to store the longitude of destination point in decimal
 * degrees
 */
extern "C" void PositionBearingDistanceLoxodrome(double lat, double lon,
                                                 double brg, double dist,
                                                 double *dlat, double *dlon);

/**
 * Retrieves the index of a datum based on its name.
 *
 * This function searches for a datum by name in the internal datum database and
 * returns its index. The search is case-insensitive and ignores spaces. If the
 * datum is equivalent to WGS84, it returns the WGS84 index for optimization
 * purposes.
 *
 * @param str The datum name to search for
 * @return The index of the datum in the internal database, or -1 if not found
 */
extern "C" int GetDatumIndex(const char *str);

/**
 * Transforms coordinates from one datum to another using the Molodensky
 * transformation.
 *
 * This function converts coordinates from one geodetic datum to another using
 * the Molodensky transformation method. This is important when working with
 * charts or data that use different geodetic datums.
 *
 * @param lat Latitude in source datum in decimal degrees
 * @param lon Longitude in source datum in decimal degrees
 * @param to_lat Pointer to store the transformed latitude in decimal degrees
 * @param to_lon Pointer to store the transformed longitude in decimal degrees
 * @param from_datum_index Index of the source datum
 * @param to_datum_index Index of the target datum
 */
extern "C" void MolodenskyTransform(double lat, double lon, double *to_lat,
                                    double *to_lon, int from_datum_index,
                                    int to_datum_index);

/**
 * Calculates distance and bearing between two points using Mercator projection
 * formulas.
 *
 * This function calculates both the distance in nautical miles and the bearing
 * (course) in degrees between two geographic positions using Mercator formulas.
 * It handles special cases for purely north-south and east-west courses and
 * adapts calculation methods for longer distances (> 1 degree) to improve
 * accuracy.
 *
 * @param lat1 Latitude of destination point in decimal degrees
 * @param lon1 Longitude of destination point in decimal degrees
 * @param lat0 Latitude of start point in decimal degrees
 * @param lon0 Longitude of start point in decimal degrees
 * @param brg Pointer to store the bearing from start to destination point in
 * decimal degrees (0-360, where 0=North, 90=East, 180=South, 270=West)
 * @param dist Pointer to store the distance between points in nautical miles
 */
extern "C" void DistanceBearingMercator(double lat1, double lon1, double lat0,
                                        double lon0, double *brg, double *dist);

extern "C" int Georef_Calculate_Coefficients(struct GeoRef *cp, int nlin_lon);
extern "C" int Georef_Calculate_Coefficients_Proj(struct GeoRef *cp);
/**
 * Calculates the latitude where a great circle route crosses a specified
 * meridian.
 *
 * Given two points on the Earth's surface, this function calculates the
 * latitude at which the great circle path between them intersects a specific
 * meridian of longitude.
 *
 * @param lat1 Latitude of first point in decimal degrees
 * @param lon1 Longitude of first point in decimal degrees
 * @param lat2 Latitude of second point in decimal degrees
 * @param lon2 Longitude of second point in decimal degrees
 * @param lon Longitude of the meridian to test for intersection in decimal
 * degrees
 * @return Latitude of intersection in decimal degrees, or NaN if no
 * intersection exists
 */
extern "C" double lat_gc_crosses_meridian(double lat1, double lon1, double lat2,
                                          double lon2, double lon);
/**
 * Calculates the latitude where a rhumb line route crosses a specified
 * meridian.
 *
 * Given two points on the Earth's surface, this function calculates the
 * latitude at which the rhumb line (loxodrome) path between them intersects a
 * specific meridian of longitude. A rhumb line is a path of constant bearing.
 *
 * @param lat1 Latitude of first point in decimal degrees
 * @param lon1 Longitude of first point in decimal degrees
 * @param lat2 Latitude of second point in decimal degrees
 * @param lon2 Longitude of second point in decimal degrees
 * @param lon Longitude of the meridian to test for intersection in decimal
 * degrees
 * @return Latitude of intersection in decimal degrees, or NaN if no
 * intersection exists
 */
extern "C" double lat_rl_crosses_meridian(double lat1, double lon1, double lat2,
                                          double lon2, double lon);

#else
void toDMS(double a, char *bufp, int bufplen);
void toDMM(double a, char *bufp, int bufplen);
int GetDatumIndex(const char *str);
void MolodenskyTransform(double lat, double lon, double *to_lat, double *to_lon,
                         int from_datum_index, int to_datum_index);
double lat_gc_crosses_meridian(double lat1, double lon1, double lat2,
                               double lon2, double lon);
double lat_rl_crosses_meridian(double lat1, double lon1, double lat2,
                               double lon2, double lon);

#endif

//--------------------

/*
 * lmfit
 *
 * Solves or minimizes the sum of squares of m nonlinear
 * functions of n variables.
 *
 * From public domain Fortran version
 * of Argonne National Laboratories MINPACK
 *     argonne national laboratory. minpack project. march 1980.
 *     burton s. garbow, kenneth e. hillstrom, jorge j. more
 * C translation by Steve Moshier
 * Joachim Wuttke converted the source into C++ compatible ANSI style
 * and provided a simplified interface
 */

// parameters for calling the high-level interface lmfit
//   ( lmfit.c provides lm_initialize_control which sets default values ):
typedef struct {
  double ftol;       // relative error desired in the sum of squares.
  double xtol;       // relative error between last two approximations.
  double gtol;       // orthogonality desired between fvec and its derivs.
  double epsilon;    // step used to calculate the jacobian.
  double stepbound;  // initial bound to steps in the outer loop.
  double fnorm;      // norm of the residue vector fvec.
  int maxcall;       // maximum number of iterations.
  int nfev;          // actual number of iterations.
  int info;          // status of minimization.
} lm_control_type;

// the subroutine that calculates fvec:
typedef void(lm_evaluate_ftype)(double *par, int m_dat, double *fvec,
                                void *data, int *info);
// default implementation therof, provided by lm_eval.c:
void lm_evaluate_default(double *par, int m_dat, double *fvec, void *data,
                         int *info);

// the subroutine that informs about fit progress:
typedef void(lm_print_ftype)(int n_par, double *par, int m_dat, double *fvec,
                             void *data, int iflag, int iter, int nfev);
// default implementation therof, provided by lm_eval.c:
void lm_print_default(int n_par, double *par, int m_dat, double *fvec,
                      void *data, int iflag, int iter, int nfev);

// compact high-level interface:
void lm_initialize_control(lm_control_type *control);
void lm_minimize(int m_dat, int n_par, double *par, lm_evaluate_ftype *evaluate,
                 lm_print_ftype *printout, void *data,
                 lm_control_type *control);
double lm_enorm(int, double *);

// low-level interface for full control:
void lm_lmdif(int m, int n, double *x, double *fvec, double ftol, double xtol,
              double gtol, int maxfev, double epsfcn, double *diag, int mode,
              double factor, int *info, int *nfev, double *fjac, int *ipvt,
              double *qtf, double *wa1, double *wa2, double *wa3, double *wa4,
              lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
              void *data);

#ifndef _LMDIF
extern const char *lm_infmsg[];
extern const char *lm_shortmsg[];
#endif

//      This is an opaque (to lmfit) structure set up before the call to lmfit()
typedef struct {
  double *user_tx;
  double *user_ty;
  double *user_y;
  double (*user_func)(double user_tx_point, double user_ty_point, int n_par,
                      double *par);
  int print_flag;
  int n_par;
} lm_data_type;

#endif  // guard
