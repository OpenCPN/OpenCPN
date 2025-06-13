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

// Include OCPNPlatform.h before shapefile_basemap.h to prevent obscure syntax
// error when compiling with VS2022
#include "OCPNPlatform.h"
#include "shapefile_basemap.h"
#include "chartbase.h"
#include "glChartCanvas.h"

#include "model/logger.h"

#ifdef ocpnUSE_GL
#include "shaders.h"
#endif

#ifdef __WXMSW__
#define __CALL_CONVENTION  //__stdcall
#else
#define __CALL_CONVENTION
#endif

extern OCPNPlatform *g_Platform;
extern wxString gWorldShapefileLocation;

#ifdef ocpnUSE_GL

typedef union {
  GLdouble data[6];
  struct sGLvertex {
    GLdouble x;
    GLdouble y;
    GLdouble z;
    GLdouble r;
    GLdouble g;
    GLdouble b;
  } info;
} GLvertexshp;
#include <list>

static std::list<float_2Dpt> g_pvshp;
static std::list<GLvertexshp *> g_vertexesshp;
static int g_typeshp, g_posshp;
static float_2Dpt g_p1shp, g_p2shp;

void __CALL_CONVENTION shpscombineCallback(GLdouble coords[3],
                                           GLdouble *vertex_data[4],
                                           GLfloat weight[4],
                                           GLdouble **dataOut) {
  GLvertexshp *vertex;

  vertex = new GLvertexshp();
  g_vertexesshp.push_back(vertex);

  vertex->info.x = coords[0];
  vertex->info.y = coords[1];

  *dataOut = vertex->data;
}

void __CALL_CONVENTION shpserrorCallback(GLenum errorCode) {
  const GLubyte *estring;
  estring = gluErrorString(errorCode);
  // wxLogMessage( _T("OpenGL Tessellation Error: %s"), estring );
}

void __CALL_CONVENTION shpsbeginCallback(GLenum type) {
  switch (type) {
    case GL_TRIANGLES:
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
      g_typeshp = type;
      break;
    default:
      printf("tess unhandled begin type: %d\n", type);
  }

  g_posshp = 0;
}

void __CALL_CONVENTION shpsendCallback() {}

void __CALL_CONVENTION shpsvertexCallback(GLvoid *arg) {
  GLvertexshp *vertex;
  vertex = (GLvertexshp *)arg;
  float_2Dpt p;
  p.y = vertex->info.x;
  p.x = vertex->info.y;

  // convert strips and fans into triangles
  if (g_typeshp != GL_TRIANGLES) {
    if (g_posshp > 2) {
      g_pvshp.push_back(g_p1shp);
      g_pvshp.push_back(g_p2shp);
    }

    if (g_typeshp == GL_TRIANGLE_STRIP)
      g_p1shp = g_p2shp;
    else if (g_posshp == 0)
      g_p1shp = p;
    g_p2shp = p;
  }

  g_pvshp.push_back(p);
  g_posshp++;
}
#endif

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
      // Create a LatLonKey using the 'y' (latitude) and 'x' (longitude)
      // attributes These values represent the top-left corner of the tiles
      _tiles[LatLonKey(std::any_cast<int>(feature.getAttributes()["y"]),
                       std::any_cast<int>(feature.getAttributes()["x"]))]
          .push_back(feat);
      feat++;
    }
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

void ShapeBaseChart::AddPointToTessList(shp::Point &point, ViewPort &vp,
                                        GLUtesselator *tobj, bool idl) {
#ifdef ocpnUSE_GL

  wxPoint2DDouble q;
  if (glChartCanvas::HasNormalizedViewPort(vp)) {
    q = ShapeBaseChartSet::GetDoublePixFromLL(vp, point.getY(), point.getX());
  } else {  // tesselation directly from lat/lon
    q.m_x = point.getY(), q.m_y = point.getX();
  }
  GLvertexshp *vertex = new GLvertexshp();
  g_vertexesshp.push_back(vertex);
  if (vp.m_projection_type != PROJECTION_POLAR) {
    // need to correctly pick +180 or -180 longitude for projections
    // that have a discontiguous date line

    if (idl && (point.getX() == 180)) {
      if (vp.m_projection_type == PROJECTION_MERCATOR ||
          vp.m_projection_type == PROJECTION_EQUIRECTANGULAR) {
        // q.m_x -= 40058986 * 4096.0;  // 360 degrees in normalized
        // viewport
      } else {
        q.m_x -= 360;  // lat/lon coordinates
      }
    }
  }
  vertex->info.x = q.m_x;
  vertex->info.y = q.m_y;

  gluTessVertex(tobj, (GLdouble *)vertex, (GLdouble *)vertex);
#endif
}

void ShapeBaseChart::DoDrawPolygonFilledGL(ocpnDC &pnt, ViewPort &vp,
                                           const shp::Feature &feature) {
#ifdef ocpnUSE_GL

  bool idl =
      vp.GetBBox().GetMinLon() <= -180 || vp.GetBBox().GetMaxLon() >= 180;
  auto polygon = static_cast<shp::Polygon *>(feature.getGeometry());
  for (auto &ring : polygon->getRings()) {
    size_t cnt{0};
    GLUtesselator *tobj = gluNewTess();

    gluTessCallback(tobj, GLU_TESS_VERTEX, (_GLUfuncptr)&shpsvertexCallback);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (_GLUfuncptr)&shpsbeginCallback);
    gluTessCallback(tobj, GLU_TESS_END, (_GLUfuncptr)&shpsendCallback);
    gluTessCallback(tobj, GLU_TESS_COMBINE, (_GLUfuncptr)&shpscombineCallback);
    gluTessCallback(tobj, GLU_TESS_ERROR, (_GLUfuncptr)&shpserrorCallback);

    gluTessNormal(tobj, 0, 0, 1);
    gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

    gluTessBeginPolygon(tobj, NULL);
    gluTessBeginContour(tobj);
    for (auto &point : ring.getPoints()) {
      AddPointToTessList(point, vp, tobj, idl);
      cnt++;
    }
    gluTessEndContour(tobj);
    gluTessEndPolygon(tobj);
    gluDeleteTess(tobj);

    for (auto ver : g_vertexesshp) delete ver;
    g_vertexesshp.clear();
  }
  float_2Dpt *polyv = new float_2Dpt[g_pvshp.size()];
  int cnt = 0;
  for (auto pt : g_pvshp) {
    polyv[cnt++] = pt;
  }
  size_t polycnt = g_pvshp.size();
  g_pvshp.clear();

  GLuint vbo = 0;

  //  Build the shader viewport transform matrix
  mat4x4 m, mvp;
  mat4x4_identity(m);
  mat4x4_scale_aniso(mvp, m, 2.0 / (float)vp.pix_width,
                     2.0 / (float)vp.pix_height, 1.0);
  mat4x4_translate_in_place(mvp, -vp.pix_width / 2, vp.pix_height / 2, 0);

  float *pvt = new float[2 * (polycnt)];
  for (size_t i = 0; i < polycnt; i++) {
    float_2Dpt *pc = polyv + i;
    // wxPoint2DDouble q(pc->y, pc->x);// = vp.GetDoublePixFromLL(pc->y, pc->x);
    wxPoint2DDouble q = vp.GetDoublePixFromLL(pc->y, pc->x);

    pvt[i * 2] = q.m_x;
    pvt[(i * 2) + 1] = q.m_y;
  }

  GLShaderProgram *shader = pcolor_tri_shader_program[pnt.m_canvasIndex];
  shader->Bind();

  float colorv[4];
  colorv[0] = _color.Red() / float(256);
  colorv[1] = _color.Green() / float(256);
  colorv[2] = _color.Blue() / float(256);
  colorv[3] = 1.0;
  shader->SetUniform4fv("color", colorv);

  shader->SetAttributePointerf("position", pvt);

  glDrawArrays(GL_TRIANGLES, 0, polycnt);

  delete[] pvt;
  delete[] polyv;

  glDeleteBuffers(1, &vbo);
  shader->UnBind();
#endif
}

void ShapeBaseChart::DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp) {
  if (!_is_usable) {
    return;
  }
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
      return;  // not yet loaded
    }
  }

  int pmod = _dmod;
  LLBBox bbox = vp.GetBBox();

  // Calculate the starting latitude for the tiles grid
  // Using floor() aligns with the top-left corner tile model
  int lat_start = floor(bbox.GetMinLat());
  if (lat_start < 0)
    lat_start = lat_start - (pmod + (lat_start % pmod));
  else
    lat_start = lat_start - (lat_start % pmod);

  // Calculate the starting longitude for the tiles grid
  // Using floor() aligns with the top-left corner tile model
  int lon_start = floor(bbox.GetMinLon());
  if (lon_start < 0)
    lon_start = lon_start - (pmod + (lon_start % pmod));
  else
    lon_start = lon_start - (lon_start % pmod);

  if (_is_tiled) {
    for (int i = lat_start; i < ceil(bbox.GetMaxLat()) + pmod; i += pmod) {
      for (int j = lon_start; j < ceil(bbox.GetMaxLon()) + pmod; j += pmod) {
        int lon{j};
        if (j < -180) {
          lon = j + 360;
        } else if (j >= 180) {
          lon = j - 360;
        }
        for (auto fid : _tiles[LatLonKey(i, lon)]) {
          auto const &feature = _reader->getFeature(fid);
          if (pnt.GetDC()) {
            DoDrawPolygonFilled(pnt, vp,
                                feature);  // Parallelize using std::async?
          } else {
            DoDrawPolygonFilledGL(pnt, vp,
                                  feature);  // Parallelize using std::async?
          }
        }
      }
    }
  } else {
    for (auto const &feature : *_reader) {
      if (pnt.GetDC()) {
        DoDrawPolygonFilled(pnt, vp,
                            feature);  // Parallelize using std::async?
      } else {
        DoDrawPolygonFilledGL(pnt, vp,
                              feature);  // Parallelize using std::async?
      }
    }
  }
}

bool ShapeBaseChart::CrossesLand(double &lat1, double &lon1, double &lat2,
                                 double &lon2) {
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
      // Chart not yet loaded. Assume no land crossing.
      return false;
    }
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
    chart.RenderViewOnDC(dc, vp);
  }
}
