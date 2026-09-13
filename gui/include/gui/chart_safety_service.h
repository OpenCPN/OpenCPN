/***************************************************************************
 * Copyright (C) 2026 OpenCPN contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 ***************************************************************************/

#ifndef GUI_CHART_SAFETY_SERVICE_H
#define GUI_CHART_SAFETY_SERVICE_H

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace ocpn::chart_safety {

/**
 * Whether optional neighbour prefetch is still permitted by a service-call
 * time budget.  A non-positive budget means unlimited processing.
 *
 * The requested centre tile is authoritative work and may itself exceed the
 * budget.  This policy applies only to speculative neighbours after it.
 */
inline bool MayPrefetchNeighbour(long elapsed_ms, int max_milliseconds) {
  return max_milliseconds <= 0 || elapsed_ms < max_milliseconds;
}

struct TileBatchBlock {
  long min_lat_tile;
  long max_lat_tile;
  long min_lon_tile;
  long max_lon_tile;
  std::set<std::pair<long, long>> tiles;
};

struct GeographicBounds {
  double min_lat;
  double min_lon;
  double max_lat;
  double max_lon;
};

struct CoveragePoint {
  double lat;
  double lon;
};

using CoverageRing = std::vector<CoveragePoint>;

inline double CoverageCross(const CoveragePoint& a, const CoveragePoint& b,
                            const CoveragePoint& c) {
  return (b.lon - a.lon) * (c.lat - a.lat) -
         (b.lat - a.lat) * (c.lon - a.lon);
}

inline bool CoveragePointInBox(const CoveragePoint& point,
                               const GeographicBounds& box) {
  return point.lat >= box.min_lat && point.lat <= box.max_lat &&
         point.lon >= box.min_lon && point.lon <= box.max_lon;
}

inline bool CoveragePointInRing(const CoveragePoint& point,
                                const CoverageRing& ring) {
  if (ring.size() < 3) return false;
  bool inside = false;
  for (std::size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++) {
    const CoveragePoint& a = ring[i];
    const CoveragePoint& b = ring[j];
    if (((a.lat > point.lat) != (b.lat > point.lat)) &&
        point.lon < (b.lon - a.lon) * (point.lat - a.lat) /
                            (b.lat - a.lat) +
                        a.lon)
      inside = !inside;
  }
  return inside;
}

inline bool CoverageOnSegment(const CoveragePoint& a, const CoveragePoint& b,
                              const CoveragePoint& point) {
  constexpr double epsilon = 1e-10;
  return std::abs(CoverageCross(a, b, point)) < epsilon &&
         point.lon >= std::min(a.lon, b.lon) - epsilon &&
         point.lon <= std::max(a.lon, b.lon) + epsilon &&
         point.lat >= std::min(a.lat, b.lat) - epsilon &&
         point.lat <= std::max(a.lat, b.lat) + epsilon;
}

inline bool CoverageSegmentsIntersect(const CoveragePoint& a,
                                      const CoveragePoint& b,
                                      const CoveragePoint& c,
                                      const CoveragePoint& d) {
  const double c1 = CoverageCross(a, b, c);
  const double c2 = CoverageCross(a, b, d);
  const double c3 = CoverageCross(c, d, a);
  const double c4 = CoverageCross(c, d, b);
  if (((c1 > 0.0 && c2 < 0.0) || (c1 < 0.0 && c2 > 0.0)) &&
      ((c3 > 0.0 && c4 < 0.0) || (c3 < 0.0 && c4 > 0.0)))
    return true;
  return CoverageOnSegment(a, b, c) || CoverageOnSegment(a, b, d) ||
         CoverageOnSegment(c, d, a) || CoverageOnSegment(c, d, b);
}

inline bool CoverageRingIntersectsBox(const CoverageRing& ring,
                                      const GeographicBounds& box) {
  if (ring.size() < 3) return false;
  const CoveragePoint corners[] = {
      {box.min_lat, box.min_lon}, {box.min_lat, box.max_lon},
      {box.max_lat, box.max_lon}, {box.max_lat, box.min_lon}};
  for (const auto& point : ring)
    if (CoveragePointInBox(point, box)) return true;
  for (const auto& corner : corners)
    if (CoveragePointInRing(corner, ring)) return true;
  for (std::size_t i = 0; i < ring.size(); ++i)
    for (std::size_t edge = 0; edge < 4; ++edge)
      if (CoverageSegmentsIntersect(ring[i], ring[(i + 1) % ring.size()],
                                    corners[edge], corners[(edge + 1) % 4]))
        return true;
  return false;
}

inline bool CoverageRingContainsBox(const CoverageRing& ring,
                                    const GeographicBounds& box) {
  const CoveragePoint corners[] = {
      {box.min_lat, box.min_lon}, {box.min_lat, box.max_lon},
      {box.max_lat, box.max_lon}, {box.max_lat, box.min_lon}};
  for (const auto& corner : corners)
    if (!CoveragePointInRing(corner, ring)) return false;
  for (std::size_t i = 0; i < ring.size(); ++i) {
    if (CoveragePointInBox(ring[i], box)) return false;
    for (std::size_t edge = 0; edge < 4; ++edge)
      if (CoverageSegmentsIntersect(ring[i], ring[(i + 1) % ring.size()],
                                    corners[edge], corners[(edge + 1) % 4]))
        return false;
  }
  return true;
}

/**
 * Add every canonical tile which has non-zero-area contact with a chart's
 * true coverage polygons.  Bounding rectangles are used only to constrain
 * polygon intersection work.  A tile wholly inside a no-coverage polygon is
 * omitted; boundary and partial-hole tiles remain conservative candidates.
 */
inline bool AddCoverageTiles(
    const std::vector<CoverageRing>& coverage,
    const std::vector<CoverageRing>& no_coverage, double tile_degrees,
    std::size_t maximum_tiles, std::set<std::pair<long, long>>* tiles) {
  if (!tiles || !(tile_degrees > 0.0) || maximum_tiles == 0) return false;
  const long world_tiles =
      static_cast<long>(std::llround(360.0 / tile_degrees));
  const long half_world = world_tiles / 2;
  const auto canonical_lon_tile = [world_tiles, half_world](long tile) {
    while (tile < -half_world) tile += world_tiles;
    while (tile >= half_world) tile -= world_tiles;
    return tile;
  };

  // Build a sparse exclusion index once per chart.  Testing every coverage
  // candidate against every no-coverage polygon made metadata-only size
  // estimates needlessly expensive for large official chart packs.
  std::set<std::pair<long, long>> excluded_tiles;
  for (const auto& source_hole : no_coverage) {
    if (source_hole.size() < 3) continue;
    CoverageRing hole = source_hole;
    for (std::size_t i = 1; i < hole.size(); ++i) {
      while (hole[i].lon - hole[i - 1].lon > 180.0) hole[i].lon -= 360.0;
      while (hole[i].lon - hole[i - 1].lon < -180.0) hole[i].lon += 360.0;
    }
    double min_lat = hole.front().lat, max_lat = hole.front().lat;
    double min_lon = hole.front().lon, max_lon = hole.front().lon;
    for (const auto& point : hole) {
      min_lat = std::min(min_lat, point.lat);
      max_lat = std::max(max_lat, point.lat);
      min_lon = std::min(min_lon, point.lon);
      max_lon = std::max(max_lon, point.lon);
    }
    const long min_lat_tile =
        static_cast<long>(std::floor(min_lat / tile_degrees));
    const long max_lat_tile =
        static_cast<long>(std::ceil(max_lat / tile_degrees)) - 1;
    const long min_lon_tile =
        static_cast<long>(std::floor(min_lon / tile_degrees));
    const long max_lon_tile =
        static_cast<long>(std::ceil(max_lon / tile_degrees)) - 1;
    for (long lat_tile = min_lat_tile; lat_tile <= max_lat_tile; ++lat_tile) {
      for (long lon_tile = min_lon_tile; lon_tile <= max_lon_tile;
           ++lon_tile) {
        const GeographicBounds box{lat_tile * tile_degrees,
                                   lon_tile * tile_degrees,
                                   (lat_tile + 1) * tile_degrees,
                                   (lon_tile + 1) * tile_degrees};
        if (CoverageRingContainsBox(hole, box))
          excluded_tiles.insert(
              {lat_tile, canonical_lon_tile(lon_tile)});
      }
    }
  }

  for (const auto& source_ring : coverage) {
    if (source_ring.size() < 3) continue;
    CoverageRing ring = source_ring;
    for (std::size_t i = 1; i < ring.size(); ++i) {
      while (ring[i].lon - ring[i - 1].lon > 180.0) ring[i].lon -= 360.0;
      while (ring[i].lon - ring[i - 1].lon < -180.0) ring[i].lon += 360.0;
    }
    double min_lat = ring.front().lat, max_lat = ring.front().lat;
    double min_lon = ring.front().lon, max_lon = ring.front().lon;
    for (const auto& point : ring) {
      min_lat = std::min(min_lat, point.lat);
      max_lat = std::max(max_lat, point.lat);
      min_lon = std::min(min_lon, point.lon);
      max_lon = std::max(max_lon, point.lon);
    }
    const long min_lat_tile =
        static_cast<long>(std::floor(min_lat / tile_degrees));
    const long max_lat_tile =
        static_cast<long>(std::ceil(max_lat / tile_degrees)) - 1;
    const long min_lon_tile =
        static_cast<long>(std::floor(min_lon / tile_degrees));
    const long max_lon_tile =
        static_cast<long>(std::ceil(max_lon / tile_degrees)) - 1;
    for (long lat_tile = min_lat_tile; lat_tile <= max_lat_tile; ++lat_tile) {
      if (lat_tile * tile_degrees < -90.0 ||
          lat_tile * tile_degrees >= 90.0)
        continue;
      for (long lon_tile = min_lon_tile; lon_tile <= max_lon_tile;
           ++lon_tile) {
        const GeographicBounds box{lat_tile * tile_degrees,
                                   lon_tile * tile_degrees,
                                   (lat_tile + 1) * tile_degrees,
                                   (lon_tile + 1) * tile_degrees};
        if (!CoverageRingIntersectsBox(ring, box)) continue;
        const long canonical_lon = canonical_lon_tile(lon_tile);
        if (excluded_tiles.count({lat_tile, canonical_lon})) continue;
        if (tiles->count({lat_tile, canonical_lon}) == 0 &&
            tiles->size() >= maximum_tiles)
          return false;
        tiles->insert({lat_tile, canonical_lon});
      }
    }
  }
  return true;
}

/**
 * Return the canonical coordinate for a cell on the global safety lattice.
 *
 * Deriving the coordinate from the global integer cell index (instead of a
 * batch origin plus a local offset) makes a cell shared by two tiles or
 * provider batches bit-identical in both requests.
 */
inline double GlobalGridCoordinate(long tile_index, int cell_offset,
                                   int cells_per_tile,
                                   double resolution_degrees) {
  const long long global_cell =
      static_cast<long long>(tile_index) * cells_per_tile + cell_offset;
  return static_cast<double>(global_cell) * resolution_degrees;
}

/**
 * Expand chart candidate discovery without changing the requested raster.
 * Exact per-cell chart coverage is still tested by the caller.  The halo
 * prevents float-precision chart extents from dropping a candidate at a
 * shared provider-batch edge.
 */
inline GeographicBounds ExpandCandidateDiscoveryBounds(
    const GeographicBounds& bounds, double grid_resolution_degrees) {
  if (!(grid_resolution_degrees > 0.0)) return bounds;
  return {bounds.min_lat - grid_resolution_degrees,
          bounds.min_lon - grid_resolution_degrees,
          bounds.max_lat + grid_resolution_degrees,
          bounds.max_lon + grid_resolution_degrees};
}

/**
 * Partition sparse fine tiles into small geographic blocks suitable for one
 * rectangular provider-grid call.  The provider grid shares boundary cells,
 * so a block spanning N tiles uses N * cells_per_tile + 1 rows/columns.
 */
inline std::vector<TileBatchBlock> PlanTileBatchBlocks(
    const std::set<std::pair<long, long>>& tiles, int maximum_tile_span = 6) {
  std::vector<TileBatchBlock> result;
  if (tiles.empty() || maximum_tile_span <= 0) return result;

  std::map<std::pair<long, long>, std::set<std::pair<long, long>>> buckets;
  for (const auto& tile : tiles) {
    const long lat_bucket = static_cast<long>(
        std::floor(static_cast<double>(tile.first) / maximum_tile_span));
    const long lon_bucket = static_cast<long>(
        std::floor(static_cast<double>(tile.second) / maximum_tile_span));
    buckets[{lat_bucket, lon_bucket}].insert(tile);
  }

  result.reserve(buckets.size());
  for (const auto& bucket : buckets) {
    TileBatchBlock block{};
    block.tiles = bucket.second;
    block.min_lat_tile = block.max_lat_tile = block.tiles.begin()->first;
    block.min_lon_tile = block.max_lon_tile = block.tiles.begin()->second;
    for (const auto& tile : block.tiles) {
      block.min_lat_tile = std::min(block.min_lat_tile, tile.first);
      block.max_lat_tile = std::max(block.max_lat_tile, tile.first);
      block.min_lon_tile = std::min(block.min_lon_tile, tile.second);
      block.max_lon_tile = std::max(block.max_lon_tile, tile.second);
    }
    result.push_back(std::move(block));
  }
  return result;
}

}  // namespace ocpn::chart_safety

#endif  // GUI_CHART_SAFETY_SERVICE_H
