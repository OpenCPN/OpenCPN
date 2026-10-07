/* GPL-2.0-or-later. Conservative, display-independent safety geometry. */
#ifndef OCPN_CHART_SAFETY_GEOMETRY_H
#define OCPN_CHART_SAFETY_GEOMETRY_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <map>
#include <vector>
#include <unordered_map>

namespace ocpn::chart_safety {
struct Point {
  double x, y;
};
struct CellBox {
  double min_x, min_y, max_x, max_y;
  bool Valid() const {
    return std::isfinite(min_x) && std::isfinite(min_y) &&
           std::isfinite(max_x) && std::isfinite(max_y) && min_x <= max_x &&
           min_y <= max_y;
  }
  bool Contains(Point p) const {
    return p.x >= min_x && p.x <= max_x && p.y >= min_y && p.y <= max_y;
  }
};
// Owns copied geometry; no chart object or renderer pointers survive
// extraction.
struct PreparedGeometry {
  enum Kind { kPoints, kLine, kArea, kConservativeBox } kind = kConservativeBox;
  bool valid = false;
  bool soundings = false;
  double ref_lat = 0, ref_lon = 0;
  CellBox geographic{};
  std::vector<Point> points;
  std::vector<double> depths;
  std::unordered_map<std::uint64_t, std::vector<size_t>> point_bins;
  std::vector<std::array<Point, 3>> triangles;
  std::vector<CellBox> triangle_boxes;
  bool boundary_valid = false;
  std::vector<std::pair<Point, Point>> boundary;
};
inline bool Finite(Point p) { return std::isfinite(p.x) && std::isfinite(p.y); }
inline std::uint64_t PointBinKey(int x, int y) {
  x = ((x + 18000) % 36000 + 36000) % 36000 - 18000;
  return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32) |
         static_cast<std::uint32_t>(y);
}
inline bool BuildPointIndex(PreparedGeometry* g) {
  g->point_bins.clear();
  for (auto p : g->points)
    if (!Finite(p) || p.y < -90 || p.y > 90) return false;
  if (g->points.size() < 32) return true;
  for (size_t i = 0; i < g->points.size(); ++i) {
    const auto p = g->points[i];
    const int x =
        static_cast<int>(std::floor(std::remainder(p.x, 360.0) * 100));
    const int y = static_cast<int>(std::floor(p.y * 100));
    g->point_bins[PointBinKey(x, y)].push_back(i);
  }
  return true;
}
template <class Visitor>
inline bool VisitPointsInCell(const PreparedGeometry& g, CellBox box,
                              Visitor visitor) {
  if (!box.Valid()) return false;
  const double centre = (box.min_x + box.max_x) / 2;
  if (!std::isfinite(centre)) return false;
  auto visit = [&](size_t i) {
    auto p = g.points[i];
    p.x = centre + std::remainder(p.x - centre, 360.0);
    return box.Contains(p) && visitor(i);
  };
  if (g.point_bins.empty() || box.max_x - box.min_x > 1 ||
      box.max_y - box.min_y > 1 || box.min_y < -90 || box.max_y > 90) {
    for (size_t i = 0; i < g.points.size(); ++i)
      if (visit(i)) return true;
    return false;
  }
  const double normalized = std::remainder(centre, 360.0);
  const int xmin =
      static_cast<int>(std::floor((normalized + box.min_x - centre) * 100)) - 1;
  const int xmax =
      static_cast<int>(std::floor((normalized + box.max_x - centre) * 100)) + 1;
  const int ymin = static_cast<int>(std::floor(box.min_y * 100)) - 1;
  const int ymax = static_cast<int>(std::floor(box.max_y * 100)) + 1;
  // One extra bin guards numerical conversion at bin boundaries. The exact
  // geographic footprint is still checked before invoking the visitor.
  for (int x = xmin; x <= xmax; ++x)
    for (int y = ymin; y <= ymax; ++y) {
      const auto bin = g.point_bins.find(PointBinKey(x, y));
      if (bin != g.point_bins.end())
        for (auto i : bin->second)
          if (visit(i)) return true;
    }
  return false;
}
inline long double Cross(Point a, Point b, Point c) {
  return (static_cast<long double>(b.x) - a.x) *
             (static_cast<long double>(c.y) - a.y) -
         (static_cast<long double>(b.y) - a.y) *
             (static_cast<long double>(c.x) - a.x);
}
inline bool OnSegment(Point a, Point b, Point p) {
  return Cross(a, b, p) == 0 && p.x >= std::min(a.x, b.x) &&
         p.x <= std::max(a.x, b.x) && p.y >= std::min(a.y, b.y) &&
         p.y <= std::max(a.y, b.y);
}
inline bool SegmentsIntersect(Point a, Point b, Point c, Point d) {
  const auto ac = Cross(a, b, c), ad = Cross(a, b, d), ca = Cross(c, d, a),
             cb = Cross(c, d, b);
  return (((ac < 0 && ad > 0) || (ac > 0 && ad < 0)) &&
          ((ca < 0 && cb > 0) || (ca > 0 && cb < 0))) ||
         OnSegment(a, b, c) || OnSegment(a, b, d) || OnSegment(c, d, a) ||
         OnSegment(c, d, b);
}
inline bool SegmentIntersectsCell(Point a, Point b, CellBox box) {
  if (!box.Valid() || !Finite(a) || !Finite(b)) return true;
  if (box.Contains(a) || box.Contains(b)) return true;
  const Point c[] = {{box.min_x, box.min_y},
                     {box.max_x, box.min_y},
                     {box.max_x, box.max_y},
                     {box.min_x, box.max_y}};
  for (int i = 0; i < 4; ++i)
    if (SegmentsIntersect(a, b, c[i], c[(i + 1) % 4])) return true;
  return false;
}
inline bool PointInTriangle(Point p, Point a, Point b, Point c) {
  if (Cross(a, b, c) == 0)
    return OnSegment(a, b, p) || OnSegment(b, c, p) || OnSegment(c, a, p);
  const auto x = Cross(a, b, p), y = Cross(b, c, p), z = Cross(c, a, p);
  return !((x < 0 || y < 0 || z < 0) && (x > 0 || y > 0 || z > 0));
}
inline bool TriangleIntersectsCell(Point a, Point b, Point c, CellBox box) {
  if (!box.Valid() || !Finite(a) || !Finite(b) || !Finite(c)) return true;
  if (std::max({a.x, b.x, c.x}) < box.min_x ||
      std::min({a.x, b.x, c.x}) > box.max_x ||
      std::max({a.y, b.y, c.y}) < box.min_y ||
      std::min({a.y, b.y, c.y}) > box.max_y)
    return false;
  if (box.Contains(a) || box.Contains(b) || box.Contains(c)) return true;
  const Point corners[] = {{box.min_x, box.min_y},
                           {box.max_x, box.min_y},
                           {box.max_x, box.max_y},
                           {box.min_x, box.max_y}};
  for (auto p : corners)
    if (PointInTriangle(p, a, b, c)) return true;
  return SegmentIntersectsCell(a, b, box) || SegmentIntersectsCell(b, c, box) ||
         SegmentIntersectsCell(c, a, box);
}
// Positive proof is deliberately stricter than intersection. One convex
// triangle must contain the complete box, with a numerical guard. Neither
// corner sampling of a polygon nor summing triangle areas proves coverage.
inline bool TriangleContainsCell(Point a, Point b, Point c, CellBox box) {
  if (!box.Valid() || !Finite(a) || !Finite(b) || !Finite(c)) return false;
  const long double scale =
      std::max({1.0L, std::abs(Cross(a, b, c)),
                static_cast<long double>(std::hypot(b.x - a.x, b.y - a.y)) *
                    std::hypot(c.x - a.x, c.y - a.y)});
  const long double guard = scale * 1e-12L;
  const Point corners[] = {{box.min_x, box.min_y},
                           {box.max_x, box.min_y},
                           {box.max_x, box.max_y},
                           {box.min_x, box.max_y}};
  const bool ccw = Cross(a, b, c) > guard;
  if (!ccw && Cross(a, b, c) >= -guard) return false;
  for (auto p : corners) {
    const auto x = Cross(a, b, p), y = Cross(b, c, p), z = Cross(c, a, p);
    if (ccw ? (x <= guard || y <= guard || z <= guard)
            : (x >= -guard || y >= -guard || z >= -guard))
      return false;
  }
  return true;
}
// Remove only exact, oppositely directed shared edges after normalising
// winding. Duplicate or non-manifold tessellations cannot certify an
// area. Every external edge and every hole edge therefore remains visible.
inline void BuildTriangleBoundary(PreparedGeometry* g) {
  g->boundary_valid = false;
  g->boundary.clear();
  std::map<std::array<double, 4>, std::pair<int, int>> edges;
  for (auto t : g->triangles) {
    if (!Finite(t[0]) || !Finite(t[1]) || !Finite(t[2])) return;
    const auto area = Cross(t[0], t[1], t[2]);
    if (area == 0) continue;  // zero-area evidence cannot establish coverage
    if (area < 0) std::swap(t[1], t[2]);
    for (int i = 0; i < 3; ++i) {
      auto a = t[i], b = t[(i + 1) % 3];
      const bool reverse = std::pair<double, double>{b.x, b.y} <
                           std::pair<double, double>{a.x, a.y};
      if (reverse) std::swap(a, b);
      auto& count = edges[{a.x, a.y, b.x, b.y}];
      if (reverse)
        ++count.second;
      else
        ++count.first;
    }
  }
  for (const auto& item : edges) {
    const auto key = item.first;
    const auto counts = item.second;
    if (counts.first == 1 && counts.second == 1) continue;
    if (counts.first + counts.second != 1) {
      g->boundary.clear();
      return;
    }
    g->boundary.push_back({{key[0], key[1]}, {key[2], key[3]}});
  }
  g->boundary_valid = !g->boundary.empty();
}
inline bool TessellationContainsCell(const PreparedGeometry& g, CellBox box) {
  if (!g.valid || !g.boundary_valid || !box.Valid()) return false;
  for (const auto& edge : g.boundary)
    if (SegmentIntersectsCell(edge.first, edge.second, box)) return false;
  const Point centre{(box.min_x + box.max_x) / 2, (box.min_y + box.max_y) / 2};
  if (!Finite(centre)) return false;
  for (const auto& t : g.triangles)
    if (Cross(t[0], t[1], t[2]) != 0 &&
        PointInTriangle(centre, t[0], t[1], t[2]))
      return true;
  return false;
}
inline bool SoundingMinDepth(const PreparedGeometry& geometry, double lat,
                             double lon, double radius, double* minimum,
                             bool* unknown) {
  *unknown = false;
  if (!geometry.soundings) return false;
  if (!geometry.valid || geometry.points.empty() ||
      geometry.depths.size() != geometry.points.size()) {
    *unknown = true;
    return false;
  }
  bool found = false;
  VisitPointsInCell(geometry,
                    {lon - radius - .00002, lat - radius - .00002,
                     lon + radius + .00002, lat + radius + .00002},
                    [&](size_t i) {
                      const double depth = geometry.depths[i];
                      if (!std::isfinite(depth)) {
                        *unknown = true;
                        return false;
                      }
                      if (!found || depth < *minimum) *minimum = depth;
                      found = true;
                      return false;
                    });
  return found;
}
inline bool IsIsolatedDanger(const char* name) {
  return name &&
         (!std::strncmp(name, "UWTROC", 6) ||
          !std::strncmp(name, "WRECKS", 6) || !std::strncmp(name, "OBSTRN", 6));
}
// Check every contour, including holes and small polygons wholly inside the
// box. Callers still need authoritative point membership for a positive proof.
inline bool BoundaryMayIntersectCell(
    const std::vector<std::vector<Point>>& rings, CellBox box) {
  if (!box.Valid() || rings.empty()) return true;
  for (const auto& ring : rings) {
    if (ring.size() < 3) return true;
    for (size_t i = 0; i < ring.size(); ++i)
      if (SegmentIntersectsCell(ring[i], ring[(i + 1) % ring.size()], box))
        return true;
  }
  return false;
}
inline bool PointInsideRings(const std::vector<std::vector<Point>>& rings,
                             Point p) {
  if (!Finite(p) || rings.empty()) return false;
  bool inside = false;
  for (const auto& ring : rings) {
    if (ring.size() < 3) return false;
    long double area = 0;
    for (size_t i = 0; i < ring.size(); ++i) {
      const Point a = ring[i], b = ring[(i + 1) % ring.size()];
      if (!Finite(a) || !Finite(b) || OnSegment(a, b, p)) return false;
      area += static_cast<long double>(a.x) * b.y -
              static_cast<long double>(b.x) * a.y;
      if ((a.y > p.y) != (b.y > p.y)) {
        const long double crossing = static_cast<long double>(a.x) +
                                     (static_cast<long double>(p.y) - a.y) *
                                         (static_cast<long double>(b.x) - a.x) /
                                         (static_cast<long double>(b.y) - a.y);
        if (crossing > p.x) inside = !inside;
      }
    }
    if (area == 0) return false;
  }
  return inside;
}
}  // namespace ocpn::chart_safety
#endif
