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
