/**************************************************************************
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
 **************************************************************************/

#ifndef OCPN_DISTANCE_FIELD_H
#define OCPN_DISTANCE_FIELD_H

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
 * boundary features (coastlines, depth contours, navigational hazards, etc.).
 */
enum class ResolutionLevel {
  HIGH,    //!< High resolution (10-25m), used within 1nm of boundary features.
  MEDIUM,  //<! Medium resolution (100-250m), used within 10nm of boundary
           // features.
  LOW      //!< Low resolution (1-2km), used beyond 10nm from boundary features.
};

/**
 * Grid cell structure used in the Fast Marching Method (FMM) algorithm.
 * Each cell in the distance field grid stores distance and algorithm state
 * information.
 */
struct GridCell {
  /**
   * Distance to the nearest boundary feature in nautical miles.
   *
   * For coastlines: Positive values represent distance to land (water),
   *                negative values would indicate inland distance.
   * For depth contours: Positive values represent distance to areas deeper than
   *                threshold, negative values represent distance to areas
   *                shallower than threshold.
   * For hazards: Positive values represent distance from the hazard.
   */
  double distance = std::numeric_limits<double>::infinity();

  /**
   * Flag indicating whether this cell's distance value has been finalized.
   * Used by the Fast Marching Method algorithm.
   */
  bool accepted = false;

  /**
   * Flag indicating whether this cell is on a boundary feature.
   * Boundary features include coastlines, depth contours, navigational marks,
   * or hazards. Boundary cells have distance value of 0.0.
   */
  bool boundary = false;
};

/**
 * Type of boundary feature that the distance field represents.
 * Used to correctly interpret distance values and for specialized processing.
 */
enum class BoundaryFeatureType {
  COASTLINE,      //!< Coastline boundaries (land/water interface)
  DEPTH_CONTOUR,  //!< Depth contour at specified threshold
  HAZARD,         //!< Navigational hazard (e.g., rocks, wrecks, obstructions)
  NAV_MARK,       //!< Navigational mark (e.g., buoys, beacons, cardinal marks)
  CHANNEL,        //!< Navigation channel or fairway
  CUSTOM          //!< Custom boundary feature type
};

/**
 * Distance field for efficient feature proximity detection.
 * Stores a multi-resolution grid of distances to the nearest boundary feature
 * (such as coastlines, depth contours, or navigational hazards) for a specific
 * tile. Uses Fast Marching Method to efficiently compute the distance field.
 *
 * This class can be used for different types of distance calculations:
 * - Land proximity (coastline distance fields)
 * - Depth safety (distance to specified depth contour)
 * - Hazard avoidance (distance to navigational hazards)
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
   * @param featureIndices Vector of indices of polygon features in the
   * shapefile to include as boundaries in the distance field calculation. These
   * indices correspond to positions in the shapefile being read and must remain
   * valid for the duration of the call.
   * @param reader Non-owning pointer to the shapefile reader containing
   * boundary feature data. Must not be null and must remain valid for the
   * duration of the call.
   * @return True if generation was successful, false otherwise.
   */
  bool Generate(const std::vector<size_t>& featureIndices,
                shp::ShapefileReader* reader);

  /**
   * Query the distance to the nearest boundary feature at a given
   * latitude/longitude.
   *
   * @param lat Latitude in degrees.
   * @param lon Longitude in degrees.
   * @return Distance to nearest boundary feature in nautical miles.
   * Interpretation depends on the boundary feature type. For coastlines,
   * positive values indicate distance over water. For depth contours, positive
   * values indicate distance to deeper water than the threshold.
   */
  double QueryDistance(double lat, double lon) const;

  /**
   * Check if a line segment crosses a boundary feature.
   * Samples points along the line to detect crossings.
   *
   * @param lat1 Latitude of first endpoint.
   * @param lon1 Longitude of first endpoint.
   * @param lat2 Latitude of second endpoint.
   * @param lon2 Longitude of second endpoint.
   * @return True if the line segment crosses the boundary feature, false
   * otherwise.
   */
  bool CrossesBoundary(double lat1, double lon1, double lat2,
                       double lon2) const;

  /**
   * Check if a line segment crosses land (convenience method).
   * Equivalent to CrossesBoundary() when feature type is COASTLINE.
   *
   * @param lat1 Latitude of first endpoint.
   * @param lon1 Longitude of first endpoint.
   * @param lat2 Latitude of second endpoint.
   * @param lon2 Longitude of second endpoint.
   * @return True if the line segment crosses land, false otherwise.
   */
  bool CrossesLand(double lat1, double lon1, double lat2, double lon2) const;

  /**
   * Check if a line segment crosses shallow water (convenience method).
   * Equivalent to CrossesBoundary() when feature type is DEPTH_CONTOUR.
   *
   * @param lat1 Latitude of first endpoint.
   * @param lon1 Longitude of first endpoint.
   * @param lat2 Latitude of second endpoint.
   * @param lon2 Longitude of second endpoint.
   * @return True if the line segment crosses into water shallower than the
   * threshold.
   */
  bool CrossesShallowWater(double lat1, double lon1, double lat2,
                           double lon2) const;

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
   * The grid size includes an additional boundary cell in each dimension to
   * handle points that lie exactly on the tile edges (at _maxLat and _maxLon).
   *
   * For each resolution level, we allocate a grid of size (N+1) × (N+1) where:
   * - N is the number of cells per tile side (_*ResGridSize)
   * - The +1 accommodates points exactly at the maximum boundary of the tile
   *
   * This ensures that boundary features from adjacent tiles can be properly
   * connected without gaps, as OSMSHP files may include polygon vertices
   * precisely at tile boundaries.
   */
  void InitializeGrid();

  /**
   * Extract coastline boundaries from shapefiles (legacy method).
   * Marks grid cells containing coastlines as boundary points.
   *
   * @param featureIndices Vector of indices of polygon features to extract
   * coastline boundaries from. These indices correspond to positions in the
   * shapefile reader and must be valid.
   * @param reader Non-owning pointer to the shapefile reader containing
   * coastline polygon data. Must not be null and must remain valid for the
   * duration of the call.
   */
  void ExtractCoastlineBoundaries(const std::vector<size_t>& featureIndices,
                                  shp::ShapefileReader* reader);

  /**
   * Rasterize a line segment into the grid at the specified resolution level.
   * Marks all grid cells intersected by the line segment as boundary points.
   *
   * @param lat1 Latitude of first endpoint.
   * @param lon1 Longitude of first endpoint.
   * @param lat2 Latitude of second endpoint.
   * @param lon2 Longitude of second endpoint.
   * @param level Resolution level of the grid to rasterize into.
   * @return Number of new boundary cells marked during rasterization.
   */
  int RasterizeLineSegment(double lat1, double lon1, double lat2, double lon2,
                           ResolutionLevel level);

  /**
   * Compute the distance field using Fast Marching Method.
   * Propagates distances outward from boundary feature points.
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
   * @param requestedLevel Resolution level to use for conversion.
   * @return Tuple containing row index, and column index.
   */
  std::tuple<int, int> LatLonToGrid(
      double lat, double lon,
      ResolutionLevel requestedLevel = ResolutionLevel::HIGH) const;

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
   * The valid range for indices is from 0 to _*ResGridSize inclusive.
   * The upper bound includes the value _*ResGridSize (using <= rather than <)
   * because we've allocated an additional boundary cell in each dimension to
   * handle points that lie exactly on the tile edges.
   *
   * This ensures that when LatLonToGrid() converts coordinates at exactly
   * _maxLat or _maxLon to grid indices, they are still considered valid
   * and can be represented in our grid.
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

  /** Type of boundary feature this distance field represents. */
  BoundaryFeatureType _featureType = BoundaryFeatureType::COASTLINE;

  /** Threshold value for depth contours (in meters) or other parametric
   * boundaries. */
  double _thresholdValue = 0.0;

  /**
   * Distance threshold for transition to high resolution (in nautical miles).
   * Features within this distance use the highest resolution grid.
   */
  static constexpr double HIGH_RES_THRESHOLD = 1.0;

  /**
   * Distance threshold for transition to medium resolution (in nautical
   * miles). Features within this distance but beyond HIGH_RES_THRESHOLD
   * use the medium resolution grid.
   */
  static constexpr double MEDIUM_RES_THRESHOLD = 10.0;

  /**
   * Number of high resolution cells per degree, approximately 25m at equator.
   */
  static constexpr int HIGH_RES_CELLS_PER_DEG = 240;

  /**
   * Number of medium resolution cells per degree, approximately 100m at
   * equator.
   */
  static constexpr int MEDIUM_RES_CELLS_PER_DEG = 60;

  /**
   * Number of low resolution cells per degree, approximately 1km at equator.
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
   * @param reader Pointer to the shapefile reader containing boundary feature
   * data.
   * @param featureType Type of boundary feature this field represents (default:
   * coastline).
   * @param thresholdValue Optional threshold value (e.g., depth in meters).
   * @return Shared pointer to the distance field for the requested tile.
   */
  std::shared_ptr<DistanceField> GetDistanceField(
      const LatLonKey& key, const std::vector<size_t>& featureIndices,
      shp::ShapefileReader* reader,
      BoundaryFeatureType featureType = BoundaryFeatureType::COASTLINE,
      double thresholdValue = 0.0);

  /**
   * Check if a line crosses a boundary feature.
   * Determines relevant tiles and queries each for crossings.
   *
   * @param lat1 Latitude of first endpoint.
   * @param lon1 Longitude of first endpoint.
   * @param lat2 Latitude of second endpoint.
   * @param lon2 Longitude of second endpoint.
   * @param featureType Type of boundary feature to check for crossings.
   * @param relevantTiles Vector of tiles to check, or empty to determine
   * automatically.
   * @param tiles Map of tile keys to feature indices.
   * @param reader Pointer to the shapefile reader containing boundary feature
   * data.
   * @param thresholdValue Optional threshold value (e.g., depth in meters).
   * @return True if the line segment crosses the boundary feature, false
   * otherwise.
   */
  bool CrossesBoundary(
      double lat1, double lon1, double lat2, double lon2,
      BoundaryFeatureType featureType,
      const std::vector<LatLonKey>& relevantTiles,
      const std::unordered_map<LatLonKey, std::vector<size_t>>& tiles,
      shp::ShapefileReader* reader, double thresholdValue = 0.0);

  /**
   * Check if a line crosses land (convenience method).
   * Determines relevant tiles and queries each for coastline crossings.
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
   * Query the distance to the nearest boundary feature.
   * Determines the relevant tile and queries it for distance.
   *
   * @param lat Latitude in degrees.
   * @param lon Longitude in degrees.
   * @param featureType Type of boundary feature to measure distance to.
   * @param relevantTiles Vector of tiles to check, or empty to determine
   * automatically.
   * @param tiles Map of tile keys to feature indices.
   * @param reader Pointer to the shapefile reader containing boundary feature
   * data.
   * @param thresholdValue Optional threshold value (e.g., depth in meters).
   * @return Distance to nearest boundary feature in nautical miles.
   */
  double GetDistanceToFeature(
      double lat, double lon, BoundaryFeatureType featureType,
      const std::vector<LatLonKey>& relevantTiles,
      const std::unordered_map<LatLonKey, std::vector<size_t>>& tiles,
      shp::ShapefileReader* reader, double thresholdValue = 0.0);

  /**
   * Query the distance to the nearest coastline (convenience method).
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

#endif  // OCPN_DISTANCE_FIELD_H