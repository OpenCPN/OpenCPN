/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Shapefile basemap
 *
 ***************************************************************************
 *   Copyright (C) 2012-2023 by David S. Register                          *
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
 ***************************************************************************
 *
 *
 */

#ifndef SHAPEFILE_BASEMAP_H
#define SHAPEFILE_BASEMAP_H

#include <functional>
#include <vector>
#include <map>
#include <thread>
#include <future>
#include "ShapefileReader.hpp"
#include "poly_math.h"
#include "ocpndc.h"

#if (defined(OCPN_GHC_FILESYSTEM) || \
     (defined(__clang_major__) && (__clang_major__ < 15)))
// MacOS 1.13
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#else
#include <filesystem>
#include <utility>
namespace fs = std::filesystem;
#endif

/**
 * A latitude/longitude key for 1x1 or 10x10 degree grid tiles.
 * Used for indexing geographical data in a grid-based system where each tile
 * represents a 1x1 degree area. The lat/lon values represent the top-left
 * corner of each grid tile. For example, a coordinate of (lat=45.7,
 * lon=-120.3) would be in the tile with LatLonKey(45, -121).
 *
 * The tiles are 1x1 degree, except the crude resolution which is 10x10 to
 * minimize the number of polygons needed to draw the planetary scale basemap.
 * This logic is inherited from the workflow used by the source dataset
 * https://osmdata.openstreetmap.de/data/land-polygons.html
 */
class LatLonKey {
public:
  /**
   * Constructor for creating a LatLonKey.
   * @param lat Integer latitude value indicating the north edge of latitude
   * band.
   * @param lon Integer longitude value indicating the west edge of a longitude
   * band.
   */
  LatLonKey(int lat, int lon) {
    this->lat = lat;
    this->lon = lon;
  }
  /**
   * Integer latitude value representing the northern (top) boundary of a
   * latitude band. For example, lat=45 represents the area between 45°N and
   * 44°N.
   */
  int lat;
  /**
   * Integer longitude value representing the western (left) boundary of a
   * longitude band. For example, lon=-120 represents the area between 120°W and
   * 119°W.
   */
  int lon;

  bool operator<(const LatLonKey &k) const {
    if (this->lat < k.lat) {
      return this->lon < k.lon;
    }
    return this->lat < k.lat;
  }

  bool operator==(const LatLonKey &other) const {
    return (lat == other.lat && lon == other.lon);
  }
};

template <>
struct std::hash<LatLonKey> {
  std::size_t operator()(const LatLonKey &k) const {
    return 360 * k.lat + k.lon;
  }
};

/// @brief Basemap quality
enum Quality {
  /// @brief Planetary scale dataset
  crude,
  /// @brief Low resolution
  low,
  /// @brief Medium resolution
  medium,
  /// @brief High resolution
  high,
  /// @brief Full resolution of the OSM dataset. Huge dataset, the performance
  /// on slow machines is low
  full
};

typedef std::vector<wxRealPoint> contour;
typedef std::vector<contour> contour_list;

/**
 * Represents a basemap chart based on shapefile data.
 *
 * This class loads, manages, and renders geographical polygon data from a
 * shapefile for a particular resolution level. It supports tiled organization
 * where data is divided into 1-degree cells for efficient access. It provides
 * methods for drawing filled polygons.
 */
class ShapeBaseChart {
public:
  ShapeBaseChart() = delete;
  ShapeBaseChart(const std::string &filename, const size_t &min_scale,
                 const wxColor &color = *wxBLACK)
      : _dmod(1),
        _loading(false),
        _is_usable(false),
        _is_tiled(false),
        _min_scale(min_scale),
        _filename(filename),
        _reader(nullptr),
        _color(color) {
    _is_usable = fs::exists(filename);
  }

  ShapeBaseChart(const ShapeBaseChart &t) {
    this->_filename = t._filename;
    this->_is_usable = t._is_usable;
    this->_is_tiled = t._is_tiled;
    this->_min_scale = t._min_scale;
    this->_reader = nullptr;
    this->_color = t._color;
    this->_dmod = t._dmod;
    this->_loading = t._loading;
  }
  ~ShapeBaseChart() {
    CancelLoading();  // Ensure async operation is done before cleanup.
    delete _reader;
  }

  void SetColor(wxColor color) { _color = color; }

  /**
   * Tile size in degrees. Most map charts use 1-degree tiles (default), but
   * the crude resolution chart uses 10-degree tiles to optimize performance
   * for global views. The tiling system divides map data into grid cells of
   * _dmod x _dmod degrees for efficient spatial indexing and rendering.
   */
  int _dmod;

  /**
   * Loads the shapefile data into memory. Validates the file for geographical
   * bounds
   * (-180 to 180 longitude, -90 to 90 latitude) and ensures it contains polygon
   * geometry. If the shapefile is tiled (contains 'x' and 'y' attributes), it
   * organizes features by their tile location using the LatLonKey indexing
   * system.
   * @return true if the shapefile was successfully loaded and is valid, false
   * otherwise.
   */
  bool LoadSHP();
  /**
   * Determines if the chart is ready to be used for rendering or spatial
   * queries.
   * @return true if the chart exists, has been loaded, and is not currently
   * loading data.
   */
  bool IsUsable() { return _is_usable && !_loading; }
  size_t MinScale() { return _min_scale; }
  void RenderViewOnDC(ocpnDC &dc, ViewPort &vp) { DrawPolygonFilled(dc, vp); }
  static const std::string ConstructPath(const std::string &dir,
                                         const std::string &quality_suffix) {
    return std::string(dir + fs::path::preferred_separator + "basemap_" +
                       quality_suffix + ".shp");
  }

  /**
   * Determines if a line segment between two geographical points intersects any
   * land mass represented in the chart.
   *
   * @param lat1 Latitude of the first point of the line segment.
   * @param lon1 Longitude of the first point of the line segment.
   * @param lat2 Latitude of the second point of the line segment.
   * @param lon2 Longitude of the second point of the line segment.
   * @return true if the line segment crosses any polygon in the shapefile,
   * false if the chart is not loaded yet, the line segment is entirely over
   * water, or the line segment is entirely over land.
   *
   * @note The longitude of the line segment coordinates are normalized to the
   * -180 to 180 range before performing the intersection test. This ensures
   * that the line segment and polygon edges use the same longitude convention,
   * which is necessary for accurate intersection detection.
   */
  bool CrossesLand(double &lat1, double &lon1, double &lat2, double &lon2);

  /** Cancel the chart loading operation. */
  void CancelLoading();

private:
  std::future<bool> _loaded;
  bool _loading;
  bool _is_usable;
  /**
   * Indicates whether the shapefile uses a tiled organization where features
   * are associated with specific 1-degree cells. When true, the _tiles map
   * contains feature indexes grouped by LatLonKey.
   */
  bool _is_tiled;
  /**
   * The minimum scale threshold at which this chart should be displayed. Lower
   * quality charts are used for smaller scales (more zoomed out), and higher
   * quality charts for larger scales (more zoomed in).
   */
  size_t _min_scale;
  void DoDrawPolygonFilled(ocpnDC &pnt, ViewPort &vp,
                           const shp::Feature &feature);
  void DoDrawPolygonFilledGL(ocpnDC &pnt, ViewPort &vp,
                             const shp::Feature &feature);
  void DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp);
  void AddPointToTessList(shp::Point &point, ViewPort &vp, GLUtesselator *tobj,
                          bool idl);

  /**
   * Path to the shapefile that contains the geographical data for this chart.
   * Set during construction and used when loading the shapefile data.
   */
  std::string _filename;
  /**
   * Pointer to the shapefile reader object that provides access to the
   * geographical data. Initialized during LoadSHP() and used for reading
   * polygon features and their attributes. Owned by this class and deleted in
   * the destructor.
   */
  shp::ShapefileReader *_reader;
  /**
   * Maps geographical grid cells to feature indices. Each LatLonKey corresponds
   * to a 1-degree cell, and the associated vector contains indices of features
   * that belong to that cell. Only populated when _is_tiled is true and used
   * for efficient spatial queries.
   */
  std::unordered_map<LatLonKey, std::vector<size_t>> _tiles;
  /**
   * The color used for rendering land areas in this specific chart instance.
   * Initially set during construction from the parent ShapeBaseChartSet's
   * land_color, and can be updated via SetColor() method. Used when drawing
   * filled polygons representing land masses.
   */
  wxColor _color;

  /**
   * Calculates whether two line segments intersect. Uses the coordinate system
   * of the input points, typically in (latitude, longitude) format.
   *
   * @note This function assumes that the line segment coordinates and the
   * polygon feature coordinates use the same longitude convention (e.g., both
   * in -180 to 180 range). No normalization is performed on the coordinates
   * before the intersection test, which may lead to false negatives if the line
   * segment and polygon edges use different longitude ranges or if either
   * crosses the international date line.
   *
   * @param A First endpoint of the first line segment as (latitude, longitude)
   * pair.
   * @param B Second endpoint of the first line segment as (latitude, longitude)
   * pair.
   * @param C First endpoint of the second line segment as (latitude, longitude)
   * pair.
   * @param D Second endpoint of the second line segment as (latitude,
   * longitude) pair.
   * @return true if the line segments intersect at a point that lies within
   * both segments.
   */
  bool LineLineIntersect(const std::pair<double, double> &A,
                         const std::pair<double, double> &B,
                         const std::pair<double, double> &C,
                         const std::pair<double, double> &D);

  /**
   * Tests if a line segment intersects with any edge of a polygon feature.
   * Iterates through all points in all rings of the polygon, testing each edge
   * against the line segment.
   *
   * @note This function assumes that the line segment coordinates and the
   * polygon feature coordinates use the same longitude convention (e.g., both
   * in -180 to 180 range). No normalization is performed on the coordinates
   * before the intersection test, which may lead to false negatives if the line
   * segment and polygon edges use different longitude ranges or if either
   * crosses the international date line.
   *
   * @param feature The shapefile polygon feature to test against.
   * @param A First endpoint of the line segment as (latitude, longitude) pair.
   * @param B Second endpoint of the line segment as (latitude, longitude) pair.
   * @return true if the line segment intersects any edge of any ring in the
   * polygon.
   */
  bool PolygonLineIntersect(const shp::Feature &feature,
                            const std::pair<double, double> &A,
                            const std::pair<double, double> &B);
};

/**
 * Manages a set of ShapeBaseChart objects at different resolutions. Maintains
 * multiple basemap charts at varying quality levels and scales. Handles
 * selection of the appropriate chart based on the current view scale.
 */
class ShapeBaseChartSet {
public:
  ShapeBaseChartSet();
  ~ShapeBaseChartSet() { Cleanup(); }
  static wxPoint2DDouble GetDoublePixFromLL(ViewPort &vp, double lat,
                                            double lon);

  void SetBasemapLandColor(wxColor color);
  wxColor GetBasemapLandColor();

  void RenderViewOnDC(ocpnDC &dc, ViewPort &vp);

  ShapeBaseChart &SelectBaseMap(const size_t &scale);
  /**
   * Checks if the chart set contains at least one usable chart.
   * @return true if the chart set contains at least one chart and the lowest
   * quality chart is usable as determined by ShapeBaseChart::IsUsable().
   */
  bool IsUsable() {
    return _basemap_map.size() > 0 && LowestQualityBaseMap().IsUsable();
  }

  /**
   * Determines if a line segment between two geographical points crosses any
   * land mass. Uses the highest quality chart available for the test to get the
   * most accurate result.
   * @param lat1 Latitude of the first point of the line segment.
   * @param lon1 Longitude of the first point of the line segment.
   * @param lat2 Latitude of the second point of the line segment.
   * @param lon2 Longitude of the second point of the line segment.
   * @return true if the line segment crosses land according to the highest
   * quality chart, false if no crossing is detected or if no charts are usable.
   */
  bool CrossesLand(double lat1, double lon1, double lat2, double lon2) {
    if (IsUsable()) {
      return HighestQualityBaseMap().CrossesLand(lat1, lon1, lat2, lon2);
    }
    return false;
  }

  void Cleanup() {
    for (auto &pair : _basemap_map) {
      pair.second.CancelLoading();
    }
    _basemap_map.clear();
    _loaded = false;
  }
  void Reset();

private:
  void LoadBasemaps(const std::string &dir);
  void DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp, wxColor const &color);
  void DrawPolygonFilledGL(ocpnDC &pnt, int *pvc, ViewPort &vp,
                           wxColor const &color, bool idl);
  /**
   * Retrieves the lowest quality chart in the set. Searches for charts in order
   * of increasing quality (crude, low, medium, high, full) and returns the
   * first one found.
   * @return Reference to the lowest quality chart in the set. If no crude chart
   * exists, returns the next available quality level.
   */
  ShapeBaseChart &LowestQualityBaseMap();
  /**
   * Retrieves the highest quality chart in the set. Searches for charts in
   * order of decreasing quality (full, high, medium, low, crude) and returns
   * the first one found.
   * @return Reference to the highest quality chart in the set. If no full
   * quality chart exists, returns the next available quality level.
   */
  ShapeBaseChart &HighestQualityBaseMap();

  bool _loaded;
  /**
   * The master color setting for all land masses across all charts in this set.
   * This color is applied to individual charts at render time via their
   * SetColor() method.
   */
  wxColor land_color;

  std::map<Quality, ShapeBaseChart> _basemap_map;
};

#endif
