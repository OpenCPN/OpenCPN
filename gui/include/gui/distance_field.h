/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Distance Field for efficient land crossing detection
 *
 ***************************************************************************
 *   Copyright (C) 2025 by OpenCPN Contributors                            *
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

#ifndef DISTANCE_FIELD_H
#define DISTANCE_FIELD_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <tuple>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <limits>
#include <atomic>
#include <future>

#include "shapefile_basemap.h"
#include "ShapefileReader.hpp"

/**
 * Resolution levels for multi-resolution distance field.
 * The distance field uses different grid resolutions based on distance to
 * coastline.
 */
enum class ResolutionLevel {
  HIGH,   /**< High resolution (10-25m), used within 1nm of coastlines. */
  MEDIUM, /**< Medium resolution (100-250m), used within 10nm of coastlines. */
  LOW     /**< Low resolution (1-2km), used beyond 10nm from coastlines. */
};

/**
 * Grid cell structure used in the Fast Marching Method algorithm.
 * Each cell in the distance field grid stores distance and algorithm state
 * information.
 */
struct GridCell {
  /**
   * Distance to the nearest coastline in nautical miles.
   * Positive values represent distance to land (water), negative values would
   * indicate inland distance.
   */
  double distance = std::numeric_limits<double>::infinity();

  /**
   * Flag indicating whether this cell's distance value has been finalized.
   * Used by the Fast Marching Method algorithm.
   */
  bool accepted = false;

  /**
   * Flag indicating whether this cell is on a coastline boundary.
   * Boundary cells have distance value of 0.0.
   */
  bool boundary = false;
};

/**
 * Distance field for efficient land crossing detection.
 * Stores a multi-resolution grid of distances to the nearest coastline for a
 * specific tile. Uses Fast Marching Method to efficiently compute the distance
 * field.
 */
class DistanceField {
public:
  /**
   * Constructor for distance field tile.
   *
   * @param key The LatLonKey identifying this tile's position.
   * @param degreeSize The size of the tile in degrees, defaults to 1 degree.
   */
  DistanceField(const LatLonKey& key, int degreeSize = 1);

  /**
   * Destructor for distance field.
   */
  ~DistanceField();

  /**
   * Generate the distance field for a tile using the provided shapefile
   * features.
   *
   * @param featureIndices Vector of indices of features to include in distance
   * field.
   * @param reader Pointer to the shapefile reader containing coastline data.
   * @return True if generation was successful, false otherwise.
   */
  bool Generate(const std::vector<size_t>& featureIndices,
                shp::ShapefileReader* reader);

  /**
   * Query the distance to the nearest coastline at a given latitude/longitude.
   *
   * @param lat Latitude in degrees.
   * @param lon Longitude in degrees.
   * @return Distance to nearest coastline in nautical miles. Positive values
   * indicate distance over water.
   */
  double QueryDistance(double lat, double lon) const;

  /**
   * Check if a line segment crosses land.
   * Samples points along the line to detect crossings.
   *
   * @param lat1 Latitude of first endpoint.
   * @param lon1 Longitude of first endpoint.
   * @param lat2 Latitude of second endpoint.
   * @param lon2 Longitude of second endpoint.
   * @return True if the line segment crosses land, false otherwise.
   */
  bool CrossesLand(double lat1, double lon1, double lat2, double lon2) const;

  /**
   * Save the distance field to disk for later retrieval.
   *
   * @param baseDir Base directory to save the file in.
   * @return True if save was successful, false otherwise.
   */
  bool SaveToDisk(const std::string& baseDir) const;

  /**
   * Load a distance field from disk.
   *
   * @param baseDir Base directory to load the file from.
   * @return True if load was successful, false otherwise.
   */
  bool LoadFromDisk(const std::string& baseDir);

  /**
   * Get the key for this tile.
   *
   * @return The LatLonKey identifying this tile.
   */
  const LatLonKey& GetKey() const { return _key; }

  /**
   * Check if this distance field is valid and ready for queries.
   *
   * @return True if the distance field is valid, false otherwise.
   */
  bool IsValid() const { return _valid; }

private:
  /**
   * Initialize the grid with appropriate resolution levels.
   * Creates the multi-resolution grids and sets default values.
   */
  void InitializeGrid();

  /**
   * Extract coastline boundaries from shapefiles.
   * Marks grid cells containing coastlines as boundary points.
   *
   * @param featureIndices Vector of indices of features to extract boundaries
   * from.
   * @param reader Pointer to the shapefile reader containing coastline data.
   */
  void ExtractCoastlineBoundaries(const std::vector<size_t>& featureIndices,
                                  shp::ShapefileReader* reader);

  /**
   * Compute the distance field using Fast Marching Method.
   * Propagates distances outward from coastline boundary points.
   */
  void ComputeDistanceField();

  /**
   * Update distance for a grid point using neighbors.
   * Used by the Fast Marching Method algorithm.
   *
   * @param i Row index of the grid point.
   * @param j Column index of the grid point.
   * @return Updated distance value.
   */
  double UpdateDistance(int i, int j);

  /**
   * Convert latitude/longitude to grid coordinates.
   *
   * @param lat Latitude in degrees.
   * @param lon Longitude in degrees.
   * @return Tuple containing row index, column index, and resolution level.
   */
  std::tuple<int, int, ResolutionLevel> LatLonToGrid(double lat,
                                                     double lon) const;

  /**
   * Convert grid coordinates to latitude/longitude.
   *
   * @param i Row index.
   * @param j Column index.
   * @param level Resolution level of the grid.
   * @return Tuple containing latitude and longitude in degrees.
   */
  std::tuple<double, double> GridToLatLon(int i, int j,
                                          ResolutionLevel level) const;

  /**
   * Check if indices are within grid bounds for a specific resolution level.
   *
   * @param i Row index.
   * @param j Column index.
   * @param level Resolution level to check bounds for.
   * @return True if indices are valid, false otherwise.
   */
  bool IsValidIndex(int i, int j, ResolutionLevel level) const;

  /** Tile key (latitude/longitude indices). */
  LatLonKey _key;

  /** Size of the tile in degrees. */
  int _degreeSize;

  /** Grid dimension for high resolution grid. */
  int _highResGridSize;

  /** Grid dimension for medium resolution grid. */
  int _mediumResGridSize;

  /** Grid dimension for low resolution grid. */
  int _lowResGridSize;

  /** High resolution distance field grid. */
  std::vector<std::vector<GridCell>> _highResGrid;

  /** Medium resolution distance field grid. */
  std::vector<std::vector<GridCell>> _mediumResGrid;

  /** Low resolution distance field grid. */
  std::vector<std::vector<GridCell>> _lowResGrid;

  /** Cell size in degrees for high resolution grid. */
  double _highResCellSize;

  /** Cell size in degrees for medium resolution grid. */
  double _mediumResCellSize;

  /** Cell size in degrees for low resolution grid. */
  double _lowResCellSize;

  /** Minimum latitude of this tile. */
  double _minLat;

  /** Maximum latitude of this tile. */
  double _maxLat;

  /** Minimum longitude of this tile. */
  double _minLon;

  /** Maximum longitude of this tile. */
  double _maxLon;

  /** Flag indicating whether this distance field is valid and ready for
   * queries. */
  bool _valid;

  /** Distance threshold for transition to high resolution (in nautical miles).
   */
  static constexpr double HIGH_RES_THRESHOLD = 1.0;

  /** Distance threshold for transition to medium resolution (in nautical
   * miles). */
  static constexpr double MEDIUM_RES_THRESHOLD = 10.0;

  /** Number of high resolution cells per degree, approximately 25m at equator.
   */
  static constexpr int HIGH_RES_CELLS_PER_DEG = 240;

  /** Number of medium resolution cells per degree, approximately 100m at
   * equator. */
  static constexpr int MEDIUM_RES_CELLS_PER_DEG = 60;

  /** Number of low resolution cells per degree, approximately 1km at equator.
   */
  static constexpr int LOW_RES_CELLS_PER_DEG = 6;

  /** Earth radius in nautical miles. */
  static constexpr double EARTH_RADIUS_NM = 3443.92;
};

/**
 * Manager for distance field tiles.
 * Handles caching, on-demand generation, and persistence of distance fields.
 * Provides a singleton interface for accessing distance fields throughout the
 * application.
 */
class DistanceFieldManager {
public:
  /**
   * Get the singleton instance of the distance field manager.
   *
   * @return Reference to the singleton instance.
   */
  static DistanceFieldManager& GetInstance();

  /**
   * Generate or retrieve a distance field for a specific tile.
   * Checks cache first, then disk, then generates on demand if needed.
   *
   * @param key The LatLonKey identifying the tile.
   * @param featureIndices Vector of indices of features to include in distance
   * field.
   * @param reader Pointer to the shapefile reader containing coastline data.
   * @return Shared pointer to the distance field for the requested tile.
   */
  std::shared_ptr<DistanceField> GetDistanceField(
      const LatLonKey& key, const std::vector<size_t>& featureIndices,
      shp::ShapefileReader* reader);

  /**
   * Check if a line crosses land.
   * Determines relevant tiles and queries each for crossings.
   *
   * @param lat1 Latitude of first endpoint.
   * @param lon1 Longitude of first endpoint.
   * @param lat2 Latitude of second endpoint.
   * @param lon2 Longitude of second endpoint.
   * @param relevantTiles Vector of tiles to check, or empty to determine
   * automatically.
   * @param tiles Map of tile keys to feature indices.
   * @param reader Pointer to the shapefile reader containing coastline data.
   * @return True if the line segment crosses land, false otherwise.
   */
  bool CrossesLand(
      double lat1, double lon1, double lat2, double lon2,
      const std::vector<LatLonKey>& relevantTiles,
      const std::unordered_map<LatLonKey, std::vector<size_t>>& tiles,
      shp::ShapefileReader* reader);

  /**
   * Query the distance to the nearest coastline.
   * Determines the relevant tile and queries it for distance.
   *
   * @param lat Latitude in degrees.
   * @param lon Longitude in degrees.
   * @param relevantTiles Vector of tiles to check, or empty to determine
   * automatically.
   * @param tiles Map of tile keys to feature indices.
   * @param reader Pointer to the shapefile reader containing coastline data.
   * @return Distance to nearest coastline in nautical miles.
   */
  double GetDistanceToLand(
      double lat, double lon, const std::vector<LatLonKey>& relevantTiles,
      const std::unordered_map<LatLonKey, std::vector<size_t>>& tiles,
      shp::ShapefileReader* reader);

  /**
   * Set the base directory for cached distance fields.
   *
   * @param dir Directory path for caching distance fields.
   */
  void SetCacheDirectory(const std::string& dir) { _cacheDir = dir; }

  /**
   * Clear the distance field cache.
   * Waits for any in-progress generation to complete.
   */
  void ClearCache();

  /**
   * Handle file updates by invalidating affected tiles.
   * Used when shapefile data is updated.
   *
   * @param tiles Vector of tiles to invalidate.
   */
  void InvalidateTiles(const std::vector<LatLonKey>& tiles);

private:
  /**
   * Private constructor for singleton pattern.
   */
  DistanceFieldManager();

  /**
   * Destructor.
   */
  ~DistanceFieldManager();

  /**
   * Deleted copy constructor to prevent copying.
   */
  DistanceFieldManager(const DistanceFieldManager&) = delete;

  /**
   * Deleted assignment operator to prevent copying.
   */
  DistanceFieldManager& operator=(const DistanceFieldManager&) = delete;

  /**
   * Get relevant tiles for a line segment.
   * Determines all tiles that a line segment intersects.
   *
   * @param lat1 Latitude of first endpoint.
   * @param lon1 Longitude of first endpoint.
   * @param lat2 Latitude of second endpoint.
   * @param lon2 Longitude of second endpoint.
   * @return Vector of tile keys that the line segment intersects.
   */
  std::vector<LatLonKey> GetRelevantTiles(double lat1, double lon1, double lat2,
                                          double lon2) const;

  /**
   * Check if a tile is already in the process of being generated.
   *
   * @param key The tile key to check.
   * @return True if the tile is being generated, false otherwise.
   */
  bool IsTileGenerating(const LatLonKey& key) const;

  /** Cache of previously generated distance fields. */
  std::unordered_map<LatLonKey, std::shared_ptr<DistanceField>> _distanceFields;

  /** Directory for caching distance fields to disk. */
  std::string _cacheDir;

  /** Mutex for thread safety. */
  mutable std::mutex _mutex;

  /** Map of tiles currently being generated. */
  std::unordered_map<LatLonKey, std::future<std::shared_ptr<DistanceField>>>
      _generatingTiles;
};

#endif  // DISTANCE_FIELD_H