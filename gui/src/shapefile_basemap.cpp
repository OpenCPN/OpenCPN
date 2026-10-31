/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement shapefile_basemap.h -- Shapefile basemap
 */

#include <algorithm>
#include <any>
#include <chrono>
#include <cstdlib>
#include <future>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <utility>

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <wx/string.h>

// Include ocpn_platform.h before shapefile_basemap.h to prevent obscure syntax
// error when compiling with VS2022. gl_headers must be first.
#include "gl_headers.h"

#include "model/config_vars.h"
#include "model/logger.h"

#include "chartbase.h"
#include "gl_chart_canvas.h"
#include "ocpn_platform.h"
#include "shapefile_basemap.h"
#include "top_frame.h"

#ifdef ocpnUSE_GL
#include "shaders.h"
#endif

#ifdef __WXMSW__
#define __CALL_CONVENTION  //__stdcall
#else
#define __CALL_CONVENTION
#endif

ShapeBaseChartSet gShapeBasemap;

#ifdef ocpnUSE_GL

/**
 * Tessellator state for one tile build, passed to the GLU *_DATA callbacks.
 * Output triangles go to `out` as (lat, lon) offsets from the tile corner, so
 * floats keep sub-meter precision.
 */
struct ShpTessState {
  std::vector<float> *out;
  double lat0, lon0;
  GLenum type;
  int pos;
  float p1[2], p2[2];
  /// Vertices created by the combine callback, freed after the tile is built
  std::vector<std::unique_ptr<GLdouble[]>> combined;
};

void __CALL_CONVENTION shpscombineCallback(GLdouble coords[3],
                                           GLdouble *vertex_data[4],
                                           GLfloat weight[4],
                                           GLdouble **dataOut, void *data) {
  auto st = static_cast<ShpTessState *>(data);
  st->combined.emplace_back(new GLdouble[3]{coords[0], coords[1], 0});
  *dataOut = st->combined.back().get();
}

void __CALL_CONVENTION shpserrorCallback(GLenum errorCode) {}

void __CALL_CONVENTION shpsbeginCallback(GLenum type, void *data) {
  auto st = static_cast<ShpTessState *>(data);
  st->type = type;
  st->pos = 0;
}

void __CALL_CONVENTION shpsvertexCallback(GLvoid *arg, void *data) {
  auto st = static_cast<ShpTessState *>(data);
  auto v = static_cast<GLdouble *>(arg);
  float p[2] = {float(v[0] - st->lat0), float(v[1] - st->lon0)};

  // convert strips and fans into triangles
  if (st->type != GL_TRIANGLES) {
    if (st->pos > 2) {
      st->out->insert(st->out->end(), st->p1, st->p1 + 2);
      st->out->insert(st->out->end(), st->p2, st->p2 + 2);
    }

    if (st->type == GL_TRIANGLE_STRIP)
      std::copy(st->p2, st->p2 + 2, st->p1);
    else if (st->pos == 0)
      std::copy(p, p + 2, st->p1);
    std::copy(p, p + 2, st->p2);
  }

  st->out->insert(st->out->end(), p, p + 2);
  st->pos++;
}
#endif  // ocpnUSE_GL

ShapeBaseChartSet::ShapeBaseChartSet() : _loaded(false) {
  land_color = wxColor(170, 175, 80);
}
void ShapeBaseChartSet::SetBasemapLandColor(wxColor color) {
  land_color = color;
}
wxColor ShapeBaseChartSet::GetBasemapLandColor() { return land_color; }

wxPoint2DDouble ShapeBaseChartSet::GetDoublePixFromLL(ViewPort &vp, double lat,
                                                      double lon) {
  wxPoint2DDouble p = vp.GetDoublePixFromLL(lat, lon);
  p.m_x -= vp.rv_rect.x, p.m_y -= vp.rv_rect.y;
  return p;
}

ShapeBaseChart &ShapeBaseChartSet::LowestQualityBaseMap() {
  if (_basemap_map.find(Quality::crude) != _basemap_map.end()) {
    return _basemap_map.at(Quality::crude);
  }
  if (_basemap_map.find(Quality::low) != _basemap_map.end()) {
    return _basemap_map.at(Quality::low);
  }
  if (_basemap_map.find(Quality::medium) != _basemap_map.end()) {
    return _basemap_map.at(Quality::medium);
  }
  if (_basemap_map.find(Quality::high) != _basemap_map.end()) {
    return _basemap_map.at(Quality::high);
  }
  return _basemap_map.at(Quality::full);
}

ShapeBaseChart &ShapeBaseChartSet::HighestQualityBaseMap() {
  if (_basemap_map.find(Quality::full) != _basemap_map.end()) {
    return _basemap_map.at(Quality::full);
  }
  if (_basemap_map.find(Quality::high) != _basemap_map.end()) {
    return _basemap_map.at(Quality::high);
  }
  if (_basemap_map.find(Quality::medium) != _basemap_map.end()) {
    return _basemap_map.at(Quality::medium);
  }
  if (_basemap_map.find(Quality::low) != _basemap_map.end()) {
    return _basemap_map.at(Quality::low);
  }
  return _basemap_map.at(Quality::crude);
}

ShapeBaseChart &ShapeBaseChartSet::SelectBaseMap(const size_t &scale) {
  if (_basemap_map.find(Quality::full) != _basemap_map.end() &&
      _basemap_map.at(Quality::full).IsUsable() &&
      scale <= _basemap_map.at(Quality::full).MinScale()) {
    return _basemap_map.at(Quality::full);
  } else if (_basemap_map.find(Quality::high) != _basemap_map.end() &&
             _basemap_map.at(Quality::high).IsUsable() &&
             scale <= _basemap_map.at(Quality::high).MinScale()) {
    return _basemap_map.at(Quality::high);
  } else if (_basemap_map.find(Quality::medium) != _basemap_map.end() &&
             _basemap_map.at(Quality::medium).IsUsable() &&
             scale <= _basemap_map.at(Quality::medium).MinScale()) {
    return _basemap_map.at(Quality::medium);
  } else if (_basemap_map.find(Quality::low) != _basemap_map.end() &&
             _basemap_map.at(Quality::low).IsUsable() &&
             scale <= _basemap_map.at(Quality::low).MinScale()) {
    return _basemap_map.at(Quality::low);
  }
  return LowestQualityBaseMap();
}

void ShapeBaseChartSet::Reset() {
  //      Establish the Shapefile basemap location
  wxString basemap_dir;
  if (gWorldShapefileLocation.empty()) {
    basemap_dir = g_Platform->GetSharedDataDir();
    basemap_dir.Append("basemap_shp");
  } else {
    basemap_dir = gWorldShapefileLocation;
  }

  LoadBasemaps(basemap_dir.ToStdString());
}
void ShapeBaseChartSet::LoadBasemaps(const std::string &dir) {
  _loaded = false;
  _basemap_map.clear();

  if (fs::exists(ShapeBaseChart::ConstructPath(dir, "crude_10x10"))) {
    auto c = ShapeBaseChart(ShapeBaseChart::ConstructPath(dir, "crude_10x10"),
                            300000000, land_color);
    c._dmod = 10;
    _basemap_map.insert(std::make_pair(Quality::crude, c));
  }

  if (fs::exists(ShapeBaseChart::ConstructPath(dir, "low"))) {
    _basemap_map.insert(std::make_pair(
        Quality::low, ShapeBaseChart(ShapeBaseChart::ConstructPath(dir, "low"),
                                     15000000, land_color)));
  }
  if (fs::exists(ShapeBaseChart::ConstructPath(dir, "medium"))) {
    _basemap_map.insert(std::make_pair(
        Quality::medium,
        ShapeBaseChart(ShapeBaseChart::ConstructPath(dir, "medium"), 1000000,
                       land_color)));
  }
  if (fs::exists(ShapeBaseChart::ConstructPath(dir, "high"))) {
    _basemap_map.insert(std::make_pair(
        Quality::high,
        ShapeBaseChart(ShapeBaseChart::ConstructPath(dir, "high"), 300000,
                       land_color)));
  }
  if (fs::exists(ShapeBaseChart::ConstructPath(dir, "full"))) {
    _basemap_map.insert(std::make_pair(
        Quality::full,
        ShapeBaseChart(ShapeBaseChart::ConstructPath(dir, "full"), 10000,
                       land_color)));
  }
  _loaded = true;
  // if(_basemap_map.size())
  // LowestQualityBaseMap().LoadSHP();
}

bool ShapeBaseChart::LoadSHP() {
  if (!fs::exists(_filename)) {
    _is_usable = false;
    return false;
  }
  std::unique_ptr<shp::ShapefileReader> temp_reader(
      new shp::ShapefileReader(_filename));
  if (!temp_reader->isOpen()) {
    MESSAGE_LOG << "Shapefile " << _filename << " is not opened";
    _is_usable = false;
    return false;
  }
  if (!_loading) {
    // Check if loading was cancelled
    return false;
  }
  auto bounds = temp_reader->getBounds();
  _is_usable = temp_reader->getCount() > 1 && bounds.getMaxX() <= 180 &&
               bounds.getMinX() >= -180 && bounds.getMinY() >= -90 &&
               bounds.getMaxY() <=
                   90;  // TODO - Do we care whether the planet is covered?
  if (!_loading) {
    // Check if loading was cancelled
    _is_usable = false;
    return false;
  }
  _is_usable &= temp_reader->getGeometryType() == shp::GeometryType::Polygon;
  bool has_x = false;
  bool has_y = false;
  for (auto field : temp_reader->getFields()) {
    if (field.getName() == "x") {
      has_x = true;
    } else if (field.getName() == "y") {
      has_y = true;
    }
  }
  _is_tiled = (has_x && has_y);
  if (_is_usable && _is_tiled) {
    size_t feat{0};
    for (auto const &feature : *temp_reader) {
      if (!_loading) {
        // Check if loading was cancelled
        _is_usable = false;
        return false;
      }
      auto f1 = feature.getAttributes();
      int y = std::any_cast<int>(f1["y"]);
      if (feat == 0) {
        // 'x' is the west edge of the tile, but 'y' is the north edge in some
        // datasets (crude, low, high) and the south edge in others (full).
        auto pts = static_cast<shp::Polygon *>(feature.getGeometry())
                       ->getRings()[0]
                       .getPoints();
        auto [lo, hi] = std::minmax_element(
            pts.begin(), pts.end(),
            [](auto &a, auto &b) { return a.getY() < b.getY(); });
        _key_is_south_edge = (lo->getY() + hi->getY()) / 2 > y;
      }
      _tiles[LatLonKey(y, std::any_cast<int>(f1["x"]))].push_back(feat);
      feat++;
    }
  } else if (_is_usable) {
    // Untiled file: draw it as one tile holding every feature.
    // ponytail: float offsets from 0,0 give ~1 m precision, tile the file if
    // that ever matters.
    auto &all = _tiles[LatLonKey(0, 0)];
    all.resize(temp_reader->getCount());
    std::iota(all.begin(), all.end(), 0);
  }
  if (_loading) {  // Only set reader if loading wasn't cancelled
    _reader = temp_reader.release();
  }
  return _is_usable;
}

void ShapeBaseChart::DoDrawPolygonFilled(ocpnDC &pnt, ViewPort &vp,
                                         const shp::Feature &feature) {
  double old_x = -9999999.0, old_y = -9999999.0;
  auto polygon = static_cast<shp::Polygon *>(feature.getGeometry());
  pnt.SetBrush(_color);
  for (auto &ring : polygon->getRings()) {
    wxPoint *poly_pt = new wxPoint[ring.getPoints().size()];
    size_t cnt{0};
    auto bbox = vp.GetBBox();
    for (auto &point : ring.getPoints()) {
      // if (bbox.ContainsMarge(point.getY(), point.getX(), 0.05)) {
      wxPoint2DDouble q =
          ShapeBaseChartSet::GetDoublePixFromLL(vp, point.getY(), point.getX());
      if (round(q.m_x) != round(old_x) || round(q.m_y) != round(old_y)) {
        poly_pt[cnt].x = round(q.m_x);
        poly_pt[cnt].y = round(q.m_y);
        cnt++;
      }
      old_x = q.m_x;
      old_y = q.m_y;
      //}
    }
    if (cnt > 1) {
      pnt.DrawPolygonTessellated(cnt, poly_pt, 0, 0);
    }
    delete[] poly_pt;
  }
}

#ifdef ocpnUSE_GL
/**
 * Tessellate all features of a tile into triangles stored as (lat, lon) offsets
 * from the tile corner. Thread safe as long as each thread has its own reader.
 */
static std::vector<float> BuildTileTriangles(shp::ShapefileReader &reader,
                                             const LatLonKey &key,
                                             const std::vector<size_t> &fids) {
  std::vector<float> tris;
  ShpTessState st{&tris, double(key.lat), double(key.lon)};

  GLUtesselator *tobj = gluNewTess();
  gluTessCallback(tobj, GLU_TESS_VERTEX_DATA, (_GLUfuncptr)&shpsvertexCallback);
  gluTessCallback(tobj, GLU_TESS_BEGIN_DATA, (_GLUfuncptr)&shpsbeginCallback);
  gluTessCallback(tobj, GLU_TESS_COMBINE_DATA,
                  (_GLUfuncptr)&shpscombineCallback);
  gluTessCallback(tobj, GLU_TESS_ERROR, (_GLUfuncptr)&shpserrorCallback);
  gluTessNormal(tobj, 0, 0, 1);
  gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

  std::vector<GLdouble> coords;
  for (auto fid : fids) {
    auto const &feature = reader.getFeature(fid);
    auto polygon = static_cast<shp::Polygon *>(feature.getGeometry());
    // Rings are contours of one polygon, so the inner rings cut holes.
    std::vector<std::vector<shp::Point>> rings;
    size_t npts{0};
    for (auto &ring : polygon->getRings()) {
      rings.push_back(ring.getPoints());
      npts += rings.back().size();
    }
    coords.resize(3 * npts);  // must not move until gluTessEndPolygon
    GLdouble *v = coords.data();
    gluTessBeginPolygon(tobj, &st);
    for (auto &ring : rings) {
      gluTessBeginContour(tobj);
      for (auto &point : ring) {
        v[0] = point.getY(), v[1] = point.getX(), v[2] = 0;
        gluTessVertex(tobj, v, v);
        v += 3;
      }
      gluTessEndContour(tobj);
    }
    gluTessEndPolygon(tobj);
    st.combined.clear();
  }
  gluDeleteTess(tobj);

  tris.shrink_to_fit();
  return tris;
}

void ShapeBaseChart::HarvestTileJobs() {
  for (auto job = _tri_jobs.begin(); job != _tri_jobs.end();) {
    if (job->second.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      ++job;
      continue;
    }
    // ponytail: whole cache dropped when full; per-tile LRU if panning around
    // the limit ever shows up as re-tessellation stutter.
    if (_tri_cache_floats > kTriCacheMaxFloats) {
      _tri_cache.clear();
      _tri_cache_floats = 0;
    }
    auto &tris = _tri_cache[job->first] = job->second.get();
    _tri_cache_floats += tris.size();
    job = _tri_jobs.erase(job);
  }
}

const std::vector<float> *ShapeBaseChart::TileTriangles(
    const LatLonKey &key, const std::vector<size_t> &fids) {
  auto it = _tri_cache.find(key);
  if (it != _tri_cache.end()) {
    return &it->second;
  }
  static const size_t max_jobs =
      std::max(1u, std::thread::hardware_concurrency());
  if (_tri_jobs.size() < max_jobs && _tri_jobs.find(key) == _tri_jobs.end()) {
    // Each job opens its own reader, shapelib handles are not thread safe.
    _tri_jobs.emplace(
        key, std::async(std::launch::async, [file = _filename, key, fids]() {
          shp::ShapefileReader reader(file);
          auto tris = BuildTileTriangles(reader, key, fids);
          if (wxTheApp) {
            wxTheApp->CallAfter([]() {
              if (auto frame = top_frame::Get()) {
                frame->RefreshAllCanvas(false);
                frame->InvalidateAllGL();
              }
            });
          }
          return tris;
        }));
  }
  return nullptr;  // not built yet, the tile is drawn after the job finishes
}

void ShapeBaseChart::DrawTrianglesGL(ocpnDC &pnt) {
  if (_frame_pts.empty()) {
    return;
  }
  GLShaderProgram *shader = pcolor_tri_shader_program[pnt.m_canvasIndex];
  shader->Bind();

  float colorv[4];
  colorv[0] = _color.Red() / float(256);
  colorv[1] = _color.Green() / float(256);
  colorv[2] = _color.Blue() / float(256);
  colorv[3] = 1.0;
  shader->SetUniform4fv("color", colorv);

  shader->SetAttributePointerf("position", _frame_pts.data());
  glDrawArrays(GL_TRIANGLES, 0, _frame_pts.size() / 2);
  shader->UnBind();
}
#endif

bool ShapeBaseChart::EnsureLoaded() {
  if (!_reader && !_loading) {
    _loading = true;
    _loaded = std::async(std::launch::async, [&]() {
      bool ret = LoadSHP();
      _loading = false;
      return ret;
    });
  }
  if (_loading) {
    if (_loaded.wait_for(std::chrono::milliseconds(0)) ==
        std::future_status::ready) {
      _is_usable = _loaded.get();
    } else {
      return false;  // not yet loaded
    }
  }
  return _is_usable && _reader;
}

LatLonKey ShapeBaseChart::TileKey(const LatLonKey &cell) const {
  return LatLonKey(_key_is_south_edge ? cell.lat - _dmod : cell.lat, cell.lon);
}

#ifdef ocpnUSE_GL
bool ShapeBaseChart::AppendTileGL(ViewPort &vp, const LatLonKey &key,
                                  const std::vector<size_t> &fids) {
  auto tris_p = TileTriangles(key, fids);
  if (!tris_p) {
    return false;
  }
  auto &tris = *tris_p;
  size_t n = _frame_pts.size();
  _frame_pts.resize(n + tris.size());
  for (size_t i = 0; i < tris.size(); i += 2) {
    wxPoint2DDouble q =
        vp.GetDoublePixFromLL(key.lat + tris[i], key.lon + tris[i + 1]);
    _frame_pts[n + i] = q.m_x;
    _frame_pts[n + i + 1] = q.m_y;
  }
  return true;
}

std::vector<LatLonKey> ShapeBaseChart::DrawCellsGL(
    ocpnDC &pnt, ViewPort &vp, const std::vector<LatLonKey> &cells) {
  if (!_is_usable || !_is_tiled || !EnsureLoaded()) {
    return cells;
  }
  HarvestTileJobs();
  _frame_pts.clear();
  std::vector<LatLonKey> pending;
  for (auto &cell : cells) {
    auto tile = _tiles.find(TileKey(cell));
    if (tile != _tiles.end() && !AppendTileGL(vp, tile->first, tile->second)) {
      pending.push_back(cell);
    }
  }
  DrawTrianglesGL(pnt);
  return pending;
}
#endif

std::vector<LatLonKey> ShapeBaseChart::DrawPolygonFilled(ocpnDC &pnt,
                                                         ViewPort &vp) {
  if (!_is_usable || !EnsureLoaded()) {
    return {};
  }
  if (!_is_tiled) {
    auto &all = _tiles[LatLonKey(0, 0)];
    if (pnt.GetDC()) {
      for (auto fid : all) {
        DoDrawPolygonFilled(pnt, vp, _reader->getFeature(fid));
      }
    }
#ifdef ocpnUSE_GL
    else {
      HarvestTileJobs();
      _frame_pts.clear();
      AppendTileGL(vp, LatLonKey(0, 0), all);
      DrawTrianglesGL(pnt);
    }
#endif
    return {};
  }

  // Cells in view, keyed by their north and west edges
  std::vector<LatLonKey> cells;
  int pmod = _dmod;
  LLBBox bbox = vp.GetBBox();
  int lat_first = floor(bbox.GetMinLat() / pmod) * pmod + pmod;
  int lat_last = ceil(bbox.GetMaxLat() / pmod) * pmod;
  int lon_first = floor(bbox.GetMinLon() / pmod) * pmod;
  for (int i = lat_first; i <= lat_last; i += pmod) {
    for (int j = lon_first; j < bbox.GetMaxLon(); j += pmod) {
      int lon{j};
      while (lon < -180) lon += 360;
      while (lon >= 180) lon -= 360;
      cells.emplace_back(i, lon);
    }
  }

  if (pnt.GetDC()) {
    for (auto &cell : cells) {
      auto tile = _tiles.find(TileKey(cell));
      if (tile != _tiles.end()) {
        for (auto fid : tile->second) {
          DoDrawPolygonFilled(pnt, vp, _reader->getFeature(fid));
        }
      }
    }
    return {};
  }
#ifdef ocpnUSE_GL
  return DrawCellsGL(pnt, vp, cells);
#else
  return {};
#endif
}

bool ShapeBaseChart::CrossesLand(double &lat1, double &lon1, double &lat2,
                                 double &lon2) {
  if (!EnsureLoaded()) {
    // Chart not yet loaded. Assume no land crossing.
    return false;
  }

  double norm_lon1 = lon1;
  double norm_lon2 = lon2;

  while (norm_lon1 < -180) norm_lon1 += 360;
  while (norm_lon1 > 180) norm_lon1 -= 360;
  while (norm_lon2 < -180) norm_lon2 += 360;
  while (norm_lon2 > 180) norm_lon2 -= 360;

  // Create normalized coordinate pairs
  auto A = std::make_pair(lat1, norm_lon1);
  auto B = std::make_pair(lat2, norm_lon2);

  if (_is_tiled) {
    // Calculate grid-aligned min/max coordinates based on _dmod
    double minLat = std::min(lat1, lat2);
    double maxLat = std::max(lat1, lat2);
    double minLon = std::min(norm_lon1, norm_lon2);
    double maxLon = std::max(norm_lon1, norm_lon2);

    // Calculate grid-aligned indices based on the top-left corner model
    // For latitudes: ceil(lat) gives the top edge of the tile containing that
    // latitude For longitudes: floor(lon) gives the left edge of the tile
    // containing that longitude
    int latmax = ceil(maxLat / _dmod) * _dmod;
    int latmin = floor(minLat / _dmod) * _dmod;
    int lonmin = floor(minLon / _dmod) * _dmod;
    int lonmax = ceil(maxLon / _dmod) * _dmod;

    // Adjust the loop to iterate through all relevant tiles
    for (int i = latmin; i <= latmax; i += _dmod) {
      for (int j = lonmin; j < lonmax; j += _dmod) {
        int lon = j;
        // Normalize tile longitude to valid range
        while (lon < -180) lon += 360;
        while (lon >= 180) lon -= 360;

        // Check if the tile exists in our map
        auto tileIter = _tiles.find(LatLonKey(i, lon));
        if (tileIter != _tiles.end()) {
          for (auto fid : tileIter->second) {
            auto const &feature = _reader->getFeature(fid);
            if (PolygonLineIntersect(feature, A, B)) {
              return true;
            }
          }
        }
      }
    }
  } else {
    // Non-tiled case: check all features
    for (auto const &feature : *_reader) {
      if (PolygonLineIntersect(feature, A, B)) {
        return true;
      }
    }
  }

  return false;
}

void ShapeBaseChart::CancelLoading() {
  if (_loading) {
    _loading = false;
    if (_loaded.valid()) {
      _loaded.wait();  // Wait for async operation to complete
    }
  }
}

bool ShapeBaseChart::LineLineIntersect(const std::pair<double, double> &A,
                                       const std::pair<double, double> &B,
                                       const std::pair<double, double> &C,
                                       const std::pair<double, double> &D) {
  constexpr double EPSILON = 1e-9;  // Tolerance for floating-point comparisons

  // Fast bounding box check - early rejection for improved performance
  double minABx = std::min(A.first, B.first);
  double maxABx = std::max(A.first, B.first);
  double minCDx = std::min(C.first, D.first);
  double maxCDx = std::max(C.first, D.first);

  if (maxABx < minCDx || maxCDx < minABx) return false;

  double minABy = std::min(A.second, B.second);
  double maxABy = std::max(A.second, B.second);
  double minCDy = std::min(C.second, D.second);
  double maxCDy = std::max(C.second, D.second);

  if (maxABy < minCDy || maxCDy < minABy) return false;

  // Line AB represented as a1x + b1y = c1
  double a1 = B.second - A.second;
  double b1 = A.first - B.first;
  double c1 = a1 * A.first + b1 * A.second;

  // Line CD represented as a2x + b2y = c2
  double a2 = D.second - C.second;
  double b2 = C.first - D.first;
  double c2 = a2 * C.first + b2 * C.second;

  double determinant = a1 * b2 - a2 * b1;

  if (std::abs(determinant) < EPSILON) {
    // Lines are parallel or collinear.
    // Check if collinear by testing if points from one segment lie on the other
    // line.
    if (std::abs(a1 * C.first + b1 * C.second - c1) < EPSILON &&
        std::abs(a1 * D.first + b1 * D.second - c1) < EPSILON) {
      // Segments are collinear, check for overlap
      return !(maxABx < minCDx || maxCDx < minABx || maxABy < minCDy ||
               maxCDy < minABy);
    }
    return false;  // Parallel but not collinear
  }

  // Calculate intersection point
  double x = (b2 * c1 - b1 * c2) / determinant;
  double y = (a1 * c2 - a2 * c1) / determinant;

  // Check if intersection point lies on both segments
  return (minABx <= x && x <= maxABx && minABy <= y && y <= maxABy &&
          minCDx <= x && x <= maxCDx && minCDy <= y && y <= maxCDy);
}

bool ShapeBaseChart::PolygonLineIntersect(const shp::Feature &feature,
                                          const std::pair<double, double> &A,
                                          const std::pair<double, double> &B) {
  auto geometry = feature.getGeometry();
  if (!geometry) {
    return false;
  }

  // Try to cast to polygon - if this fails, the geometry isn't a polygon
  auto polygon = dynamic_cast<shp::Polygon *>(geometry);
  if (!polygon) {
    return false;
  }

  // Calculate line segment bounding box (lat, lon)
  double minLat = std::min(A.first, B.first);
  double maxLat = std::max(A.first, B.first);
  double minLon = std::min(A.second, B.second);
  double maxLon = std::max(A.second, B.second);

  for (auto &ring : polygon->getRings()) {
    const auto &points = ring.getPoints();
    if (points.size() < 3) continue;

    // Quick bounding box check for ring
    double minRingLat = std::numeric_limits<double>::max();
    double maxRingLat = std::numeric_limits<double>::lowest();
    double minRingLon = std::numeric_limits<double>::max();
    double maxRingLon = std::numeric_limits<double>::lowest();

    // Pre-compute all point pairs to avoid repeated getX/getY calls
    std::vector<std::pair<double, double>> ringPoints;
    ringPoints.reserve(points.size());

    for (const auto &point : points) {
      double lat = point.getY();
      double lon = point.getX();

      minRingLat = std::min(minRingLat, lat);
      maxRingLat = std::max(maxRingLat, lat);
      minRingLon = std::min(minRingLon, lon);
      maxRingLon = std::max(maxRingLon, lon);

      ringPoints.emplace_back(lat, lon);
    }

    // Early reject if bounding boxes don't overlap
    if (maxLat < minRingLat || minLat > maxRingLat || maxLon < minRingLon ||
        minLon > maxRingLon) {
      continue;
    }

    // Check each edge for intersection
    size_t numPoints = ringPoints.size();
    std::pair<double, double> previous = ringPoints.back();

    for (size_t i = 0; i < numPoints; i++) {
      const auto &current = ringPoints[i];

      if (LineLineIntersect(A, B, previous, current)) {
        return true;
      }

      previous = current;
    }
  }

  return false;
}

void ShapeBaseChartSet::RenderViewOnDC(ocpnDC &dc, ViewPort &vp) {
  if (IsUsable()) {
    ShapeBaseChart &chart = SelectBaseMap(vp.chart_scale);
    chart.SetColor(land_color);
    auto pending = chart.RenderViewOnDC(dc, vp);
#ifdef ocpnUSE_GL
    // Fill the tiles still being built from lower qualities on the same tile
    // grid, best first, so the view has no holes while they are tessellated.
    bool below = false;
    for (auto it = _basemap_map.rbegin();
         it != _basemap_map.rend() && !pending.empty(); ++it) {
      auto &lower = it->second;
      if (below && lower._dmod == chart._dmod && lower.IsUsable()) {
        lower.SetColor(land_color);
        pending = lower.DrawCellsGL(dc, vp, pending);
      }
      below |= &lower == &chart;
    }
#endif
  }
}
