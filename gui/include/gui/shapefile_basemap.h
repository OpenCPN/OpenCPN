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

/// @brief latitude/longitude key for 1 degree cells
class LatLonKey {
public:
  LatLonKey(int lat, int lon) {
    this->lat = lat;
    this->lon = lon;
  }
  int lat;
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

/// @brief Basemap
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
  ~ShapeBaseChart() { delete _reader; }

  int _dmod;

  bool LoadSHP();
  bool IsUsable() { return _is_usable && !_loading; }
  size_t MinScale() { return _min_scale; }
  void RenderViewOnDC(ocpnDC &dc, ViewPort &vp) { DrawPolygonFilled(dc, vp); }
  static const std::string ConstructPath(const std::string &dir,
                                         const std::string &quality_suffix) {
    return std::string(dir + fs::path::preferred_separator + "basemap_" +
                       quality_suffix + ".shp");
  }

  bool CrossesLand(double &lat1, double &lon1, double &lat2, double &lon2);

private:
  std::future<bool> _loaded;
  bool _loading;
  bool _is_usable;
  bool _is_tiled;
  size_t _min_scale;
  void DoDrawPolygonFilled(ocpnDC &pnt, ViewPort &vp,
                           const shp::Feature &feature);
  void DoDrawPolygonFilledGL(ocpnDC &pnt, ViewPort &vp,
                             const shp::Feature &feature);
  void DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp);
  void AddPointToTessList(shp::Point &point, ViewPort &vp, GLUtesselator *tobj,
                          bool idl);

  std::string _filename;
  shp::ShapefileReader *_reader;
  std::unordered_map<LatLonKey, std::vector<size_t>> _tiles;
  wxColor _color;

  bool LineLineIntersect(const std::pair<double, double> &A,
                         const std::pair<double, double> &B,
                         const std::pair<double, double> &C,
                         const std::pair<double, double> &D);

  bool PolygonLineIntersect(const shp::Feature &feature,
                            const std::pair<double, double> &A,
                            const std::pair<double, double> &B);
};

/// @brief Set of basemaps at different resolutions
class ShapeBaseChartSet {
public:
  ShapeBaseChartSet();
  ~ShapeBaseChartSet() {}
  static wxPoint2DDouble GetDoublePixFromLL(ViewPort &vp, double lat,
                                            double lon);

  void RenderViewOnDC(ocpnDC &dc, ViewPort &vp);

  ShapeBaseChart &SelectBaseMap(const size_t &scale);
  bool IsUsable() {
    return _basemap_map.size() > 0 && LowestQualityBaseMap().IsUsable();
  }

  bool CrossesLand(double lat1, double lon1, double lat2, double lon2) {
    if (IsUsable()) {
      return HighestQualityBaseMap().CrossesLand(lat1, lon1, lat2, lon2);
    }
    return false;
  }

  void Reset();

private:
  void LoadBasemaps(const std::string &dir);
  void DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp, wxColor const &color);
  void DrawPolygonFilledGL(ocpnDC &pnt, int *pvc, ViewPort &vp,
                           wxColor const &color, bool idl);
  ShapeBaseChart &LowestQualityBaseMap();
  ShapeBaseChart &HighestQualityBaseMap();

  bool _loaded;

  std::map<Quality, ShapeBaseChart> _basemap_map;
};

#endif
