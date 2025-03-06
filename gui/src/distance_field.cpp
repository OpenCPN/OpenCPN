/***************************************************************************
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

#include "distance_field.h"
#include "model/logger.h"
#include "OCPNPlatform.h"

extern OCPNPlatform* g_Platform;

// Simple structure to compare distances for the priority queue
struct DistPoint {
  int i, j;
  double distance;
  ResolutionLevel level;

  bool operator>(const DistPoint& other) const {
    return distance > other.distance;
  }
};

//----------------------------------------------------------
//    DistanceField Implementation
//----------------------------------------------------------

DistanceField::DistanceField(const LatLonKey& key, int degreeSize)
    : _key(key), _degreeSize(degreeSize), _valid(false) {
  // Set the bounds of this tile.
  // The LatLonKey key represents the top-left corner of the tile.
  _maxLat = _key.lat;
  _minLat = _key.lat - _degreeSize;
  _minLon = _key.lon;
  _maxLon = _key.lon + _degreeSize;

  // Initialize grid sizes based on resolution
  _highResGridSize = HIGH_RES_CELLS_PER_DEG * _degreeSize;
  _mediumResGridSize = MEDIUM_RES_CELLS_PER_DEG * _degreeSize;
  _lowResGridSize = LOW_RES_CELLS_PER_DEG * _degreeSize;

  // Calculate cell sizes in degrees
  _highResCellSize = static_cast<double>(_degreeSize) / _highResGridSize;
  _mediumResCellSize = static_cast<double>(_degreeSize) / _mediumResGridSize;
  _lowResCellSize = static_cast<double>(_degreeSize) / _lowResGridSize;

  // Initialize the grids
  InitializeGrid();
}

DistanceField::~DistanceField() {
  // Clean up any resources
}

void DistanceField::InitializeGrid() {
  // Initialize high-resolution grid
  // A grid of size _highResGridSize+1 can represent points from index 0 to
  // _highResGridSize inclusive
  _highResGrid.resize(_highResGridSize + 1);
  for (int i = 0; i <= _highResGridSize; ++i) {
    _highResGrid[i].resize(_highResGridSize + 1);
  }

  // Initialize medium-resolution grid
  _mediumResGrid.resize(_mediumResGridSize + 1);
  for (int i = 0; i <= _mediumResGridSize; ++i) {
    _mediumResGrid[i].resize(_mediumResGridSize + 1);
  }

  // Initialize low-resolution grid
  _lowResGrid.resize(_lowResGridSize + 1);
  for (int i = 0; i <= _lowResGridSize; ++i) {
    _lowResGrid[i].resize(_lowResGridSize + 1);
  }
}

bool DistanceField::Generate(const std::vector<size_t>& featureIndices,
                             shp::ShapefileReader* reader) {
  // Verify reader is valid
  if (!reader) {
    MESSAGE_LOG << "Cannot generate distance field: ShapefileReader is null";
    return false;
  }

  // Verify we have features to process
  if (featureIndices.empty()) {
    MESSAGE_LOG << "No feature indices provided for distance field generation";
    return false;
  }

  MESSAGE_LOG << "Generating distance field for tile (" << _key.lat << ", "
              << _key.lon << ") with " << featureIndices.size() << " features";

  // Extract coastline boundaries
  ExtractCoastlineBoundaries(featureIndices, reader);

  // Compute the distance field using Fast Marching Method
  ComputeDistanceField();

  _valid = true;
  MESSAGE_LOG << "Distance field generation completed for tile (" << _key.lat
              << ", " << _key.lon << ")";

  return true;
}

void DistanceField::ExtractCoastlineBoundaries(
    const std::vector<size_t>& featureIndices, shp::ShapefileReader* reader) {
  int highBoundaryCount = 0;
  int medBoundaryCount = 0;
  int lowBoundaryCount = 0;

  for (size_t featureIndex : featureIndices) {
    auto feature = reader->getFeature(featureIndex);
    auto geometry = feature.getGeometry();

    auto polygon = static_cast<shp::Polygon*>(geometry);

    for (auto& ring : polygon->getRings()) {
      const auto& points = ring.getPoints();

      // Process each point and the segment to the next point
      for (size_t i = 0; i < points.size(); ++i) {
        // Current point
        double lat1 = points[i].getY();
        double lon1 = points[i].getX();

        // Skip if outside tile bounds (with a small margin)
        if (lat1 < _minLat - 0.01 || lat1 > _maxLat + 0.01 ||
            lon1 < _minLon - 0.01 || lon1 > _maxLon + 0.01) {
          continue;
        }

        // Mark the current point in all resolution grids
        auto [highI1, highJ1] = LatLonToGrid(lat1, lon1, ResolutionLevel::HIGH);
        auto [medI1, medJ1] = LatLonToGrid(lat1, lon1, ResolutionLevel::MEDIUM);
        auto [lowI1, lowJ1] = LatLonToGrid(lat1, lon1, ResolutionLevel::LOW);

        // Mark as boundary in the high-resolution grid if within bounds
        if (IsValidIndex(highI1, highJ1, ResolutionLevel::HIGH)) {
          _highResGrid[highI1][highJ1].boundary = true;
          _highResGrid[highI1][highJ1].distance = 0.0;
          _highResGrid[highI1][highJ1].accepted = true;
          highBoundaryCount++;
        }

        // Also mark in medium resolution grid
        if (IsValidIndex(medI1, medJ1, ResolutionLevel::MEDIUM)) {
          _mediumResGrid[medI1][medJ1].boundary = true;
          _mediumResGrid[medI1][medJ1].distance = 0.0;
          _mediumResGrid[medI1][medJ1].accepted = true;
          medBoundaryCount++;
        }

        // Also mark in low resolution grid
        if (IsValidIndex(lowI1, lowJ1, ResolutionLevel::LOW)) {
          _lowResGrid[lowI1][lowJ1].boundary = true;
          _lowResGrid[lowI1][lowJ1].distance = 0.0;
          _lowResGrid[lowI1][lowJ1].accepted = true;
          lowBoundaryCount++;
        }

        // Get the next point to form a segment (close the loop if at the end)
        size_t nextIdx = (i + 1) % points.size();
        double lat2 = points[nextIdx].getY();
        double lon2 = points[nextIdx].getX();

        // Skip if the segment is completely outside tile bounds
        if ((lat1 < _minLat - 0.01 && lat2 < _minLat - 0.01) ||
            (lat1 > _maxLat + 0.01 && lat2 > _maxLat + 0.01) ||
            (lon1 < _minLon - 0.01 && lon2 < _minLon - 0.01) ||
            (lon1 > _maxLon + 0.01 && lon2 > _maxLon + 0.01)) {
          continue;
        }

        // Process the segment between current point and next point
        // We'll use different algorithms based on resolution

        // High resolution: Use Bresenham's line algorithm for precise
        // rasterization
        highBoundaryCount +=
            RasterizeLineSegment(lat1, lon1, lat2, lon2, ResolutionLevel::HIGH);

        // Medium resolution: Use a simpler approach with fewer samples
        medBoundaryCount += RasterizeLineSegment(lat1, lon1, lat2, lon2,
                                                 ResolutionLevel::MEDIUM);

        // Low resolution: Use even fewer samples
        lowBoundaryCount +=
            RasterizeLineSegment(lat1, lon1, lat2, lon2, ResolutionLevel::LOW);
      }
    }
  }

  MESSAGE_LOG << "Marked boundary points: " << highBoundaryCount << " high, "
              << medBoundaryCount << " medium, " << lowBoundaryCount
              << " low resolution";
}

int DistanceField::RasterizeLineSegment(double lat1, double lon1, double lat2,
                                        double lon2, ResolutionLevel level) {
  int boundaryCount = 0;

  // Convert endpoints to grid coordinates
  auto [i1, j1] = LatLonToGrid(lat1, lon1, level);
  auto [i2, j2] = LatLonToGrid(lat2, lon2, level);

  // Skip if both endpoints are outside the grid
  if (!IsValidIndex(i1, j1, level) && !IsValidIndex(i2, j2, level)) {
    return 0;
  }

  // Determine grid size and access the appropriate grid based on resolution
  // level
  int gridSize;
  std::vector<std::vector<GridCell>>* grid;

  switch (level) {
    case ResolutionLevel::HIGH:
      gridSize = _highResGridSize;
      grid = &_highResGrid;
      break;
    case ResolutionLevel::MEDIUM:
      gridSize = _mediumResGridSize;
      grid = &_mediumResGrid;
      break;
    case ResolutionLevel::LOW:
      gridSize = _lowResGridSize;
      grid = &_lowResGrid;
      break;
  }

  // Calculate the number of samples needed based on grid resolution
  // This ensures we don't miss any grid cells that the line passes through
  double cellSize;
  switch (level) {
    case ResolutionLevel::HIGH:
      cellSize = _highResCellSize;
      break;
    case ResolutionLevel::MEDIUM:
      cellSize = _mediumResCellSize;
      break;
    case ResolutionLevel::LOW:
      cellSize = _lowResCellSize;
      break;
  }

  // Calculate maximum required number of samples. This formula ensures at least
  // one sample per grid cell along the longest axis.
  double latDiff = std::abs(lat2 - lat1);
  double lonDiff = std::abs(lon2 - lon1);
  double maxDiff = std::max(latDiff, lonDiff);
  int numSamples =
      std::max(2, static_cast<int>(std::ceil(maxDiff / cellSize)) + 1);

  // Step through the line and mark grid cells
  for (int step = 0; step <= numSamples; ++step) {
    double t = static_cast<double>(step) / numSamples;
    double lat = lat1 + t * (lat2 - lat1);
    double lon = lon1 + t * (lon2 - lon1);

    auto [i, j] = LatLonToGrid(lat, lon, level);

    if (IsValidIndex(i, j, level) && !(*grid)[i][j].boundary) {
      (*grid)[i][j].boundary = true;
      (*grid)[i][j].distance = 0.0;
      (*grid)[i][j].accepted = true;
      boundaryCount++;
    }
  }

  return boundaryCount;
}

void DistanceField::ComputeDistanceField() {
  // Priority queue for Fast Marching Method
  std::priority_queue<DistPoint, std::vector<DistPoint>,
                      std::greater<DistPoint>>
      narrowBand;

  // First process the high-resolution grid
  // Initialize the narrow band with cells adjacent to boundary cells
  const std::vector<std::pair<int, int>> neighbors = {
      {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

  // Initialize narrow band for high-resolution grid
  for (int i = 0; i < _highResGridSize; ++i) {
    for (int j = 0; j < _highResGridSize; ++j) {
      if (_highResGrid[i][j].boundary) {
        // Add neighbors to the narrow band
        for (const auto& [di, dj] : neighbors) {
          int ni = i + di;
          int nj = j + dj;

          if (IsValidIndex(ni, nj, ResolutionLevel::HIGH) &&
              !_highResGrid[ni][nj].accepted) {
            // Calculate distance to this boundary point
            double dist = std::sqrt(di * di + dj * dj) * _highResCellSize *
                          60.0;  // Convert to nautical miles

            if (dist < _highResGrid[ni][nj].distance) {
              _highResGrid[ni][nj].distance = dist;
              narrowBand.push({ni, nj, dist, ResolutionLevel::HIGH});
            }
          }
        }
      }
    }
  }

  // Fast Marching Method for high-resolution grid
  while (!narrowBand.empty()) {
    DistPoint current = narrowBand.top();
    narrowBand.pop();

    // Skip if already processed
    if (current.level == ResolutionLevel::HIGH &&
        _highResGrid[current.i][current.j].accepted) {
      continue;
    }

    // Mark as accepted
    if (current.level == ResolutionLevel::HIGH) {
      _highResGrid[current.i][current.j].accepted = true;
      _highResGrid[current.i][current.j].distance = current.distance;

      // Add neighbors to the narrow band
      for (const auto& [di, dj] : neighbors) {
        int ni = current.i + di;
        int nj = current.j + dj;

        if (IsValidIndex(ni, nj, ResolutionLevel::HIGH) &&
            !_highResGrid[ni][nj].accepted) {
          double newDist = UpdateDistance(ni, nj);
          if (newDist < _highResGrid[ni][nj].distance) {
            _highResGrid[ni][nj].distance = newDist;
            narrowBand.push({ni, nj, newDist, ResolutionLevel::HIGH});
          }
        }
      }
    }
  }

  // Now propagate high-resolution values to medium-resolution grid
  for (int i = 0; i < _mediumResGridSize; ++i) {
    for (int j = 0; j < _mediumResGridSize; ++j) {
      if (!_mediumResGrid[i][j].boundary) {
        // Sample from high-resolution grid
        int highI = i * (_highResGridSize / _mediumResGridSize);
        int highJ = j * (_highResGridSize / _mediumResGridSize);

        if (IsValidIndex(highI, highJ, ResolutionLevel::HIGH)) {
          _mediumResGrid[i][j].distance = _highResGrid[highI][highJ].distance;
          _mediumResGrid[i][j].accepted = true;
        }
      }
    }
  }

  // Finally propagate medium-resolution values to low-resolution grid
  for (int i = 0; i < _lowResGridSize; ++i) {
    for (int j = 0; j < _lowResGridSize; ++j) {
      if (!_lowResGrid[i][j].boundary) {
        // Sample from medium-resolution grid
        int medI = i * (_mediumResGridSize / _lowResGridSize);
        int medJ = j * (_mediumResGridSize / _lowResGridSize);

        if (IsValidIndex(medI, medJ, ResolutionLevel::MEDIUM)) {
          _lowResGrid[i][j].distance = _mediumResGrid[medI][medJ].distance;
          _lowResGrid[i][j].accepted = true;
        }
      }
    }
  }
}

double DistanceField::UpdateDistance(int i, int j) {
  // Implementation of Eikonal equation solver for Fast Marching Method
  // We'll use a simple first-order approximation

  double minDist = std::numeric_limits<double>::infinity();

  // Check neighboring cells in all directions
  const std::vector<std::pair<int, int>> neighbors = {
      {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

  for (const auto& [di, dj] : neighbors) {
    int ni = i + di;
    int nj = j + dj;

    if (IsValidIndex(ni, nj, ResolutionLevel::HIGH) &&
        _highResGrid[ni][nj].accepted) {
      double dist = _highResGrid[ni][nj].distance +
                    std::sqrt(di * di + dj * dj) * _highResCellSize *
                        60.0;  // Convert to nautical miles
      minDist = std::min(minDist, dist);
    }
  }

  return minDist;
}

double DistanceField::QueryDistance(double lat, double lon) const {
  if (!_valid) {
    return std::numeric_limits<double>::infinity();
  }

  // Check if point is within tile bounds
  if (lat < _minLat || lat >= _maxLat || lon < _minLon || lon >= _maxLon) {
    return std::numeric_limits<double>::infinity();
  }

  // Determine which resolution level to use based on estimated distance
  ResolutionLevel level = ResolutionLevel::LOW;
  double distance = std::numeric_limits<double>::infinity();

  // Query from low-resolution grid first to get an initial estimate
  auto [lowI, lowJ] = LatLonToGrid(lat, lon);
  if (IsValidIndex(lowI, lowJ, ResolutionLevel::LOW)) {
    distance = _lowResGrid[lowI][lowJ].distance;
  }

  // If we're potentially close to land, refine using higher resolution
  if (distance <= MEDIUM_RES_THRESHOLD) {
    level = ResolutionLevel::MEDIUM;
    auto [medI, medJ] = LatLonToGrid(lat, lon);
    if (IsValidIndex(medI, medJ, ResolutionLevel::MEDIUM)) {
      distance = _mediumResGrid[medI][medJ].distance;
    }

    // If we're very close to land, use highest resolution
    if (distance <= HIGH_RES_THRESHOLD) {
      level = ResolutionLevel::HIGH;
      auto [highI, highJ] = LatLonToGrid(lat, lon);
      if (IsValidIndex(highI, highJ, ResolutionLevel::HIGH)) {
        distance = _highResGrid[highI][highJ].distance;
      }
    }
  }

  return distance;
}

bool DistanceField::CrossesLand(double lat1, double lon1, double lat2,
                                double lon2) const {
  if (!_valid) {
    return false;  // Can't determine if crosses land
  }

  // Check if line segment is completely outside the tile
  if ((lat1 < _minLat && lat2 < _minLat) ||
      (lat1 >= _maxLat && lat2 >= _maxLat) ||
      (lon1 < _minLon && lon2 < _minLon) ||
      (lon1 >= _maxLon && lon2 >= _maxLon)) {
    return false;
  }

  // Check if either endpoint is on land
  double dist1 = QueryDistance(lat1, lon1);
  double dist2 = QueryDistance(lat2, lon2);

  if (dist1 <= 0.0 || dist2 <= 0.0) {
    return true;  // One endpoint is on land
  }

  // Sample points along the line to check for land crossings
  const int numSamples = 20;  // Adjust based on precision needs

  for (int i = 1; i < numSamples; ++i) {
    double t = static_cast<double>(i) / numSamples;
    double lat = lat1 + t * (lat2 - lat1);
    double lon = lon1 + t * (lon2 - lon1);

    double dist = QueryDistance(lat, lon);
    if (dist <= 0.0) {
      return true;  // Found a point on land
    }
  }

  return false;  // No land crossings found
}

std::tuple<int, int> DistanceField::LatLonToGrid(
    double lat, double lon, ResolutionLevel requestedLevel) const {
  int i, j;

  switch (requestedLevel) {
    case ResolutionLevel::HIGH:
      i = static_cast<int>((lat - _minLat) / _highResCellSize);
      j = static_cast<int>((lon - _minLon) / _highResCellSize);
      break;
    case ResolutionLevel::MEDIUM:
      i = static_cast<int>((lat - _minLat) / _mediumResCellSize);
      j = static_cast<int>((lon - _minLon) / _mediumResCellSize);
      break;
    case ResolutionLevel::LOW:
      i = static_cast<int>((lat - _minLat) / _lowResCellSize);
      j = static_cast<int>((lon - _minLon) / _lowResCellSize);
      break;
  }

  return {i, j};
}

std::tuple<double, double> DistanceField::GridToLatLon(
    int i, int j, ResolutionLevel level) const {
  double cellSize;

  switch (level) {
    case ResolutionLevel::HIGH:
      cellSize = _highResCellSize;
      break;
    case ResolutionLevel::MEDIUM:
      cellSize = _mediumResCellSize;
      break;
    case ResolutionLevel::LOW:
      cellSize = _lowResCellSize;
      break;
  }

  double lat = _minLat + i * cellSize;
  double lon = _minLon + j * cellSize;

  return {lat, lon};
}

bool DistanceField::IsValidIndex(int i, int j, ResolutionLevel level) const {
  switch (level) {
    case ResolutionLevel::HIGH:
      // Change < to <= to include boundary points
      return i >= 0 && i <= _highResGridSize && j >= 0 && j <= _highResGridSize;
    case ResolutionLevel::MEDIUM:
      return i >= 0 && i <= _mediumResGridSize && j >= 0 &&
             j <= _mediumResGridSize;
    case ResolutionLevel::LOW:
      return i >= 0 && i <= _lowResGridSize && j >= 0 && j <= _lowResGridSize;
    default:
      return false;
  }
}

bool DistanceField::SaveToDisk(const std::string& baseDir) const {
  if (!_valid) {
    return false;
  }

  std::string fileName = baseDir + "/df_" + std::to_string(_key.lat) + "_" +
                         std::to_string(_key.lon) + ".dat";

  std::ofstream file(fileName, std::ios::binary);
  if (!file) {
    MESSAGE_LOG << "Failed to open file for writing: " << fileName;
    return false;
  }

  // Write schema version
  int schemaVersion = 1;
  file.write(reinterpret_cast<const char*>(&schemaVersion),
             sizeof(schemaVersion));

  // Write feature type and threshold value
  int featureType = static_cast<int>(_featureType);
  file.write(reinterpret_cast<const char*>(&featureType), sizeof(featureType));
  file.write(reinterpret_cast<const char*>(&_thresholdValue),
             sizeof(_thresholdValue));

  // Write header information
  file.write(reinterpret_cast<const char*>(&_key.lat), sizeof(_key.lat));
  file.write(reinterpret_cast<const char*>(&_key.lon), sizeof(_key.lon));
  file.write(reinterpret_cast<const char*>(&_degreeSize), sizeof(_degreeSize));

  // Write grid dimensions
  file.write(reinterpret_cast<const char*>(&_highResGridSize),
             sizeof(_highResGridSize));
  file.write(reinterpret_cast<const char*>(&_mediumResGridSize),
             sizeof(_mediumResGridSize));
  file.write(reinterpret_cast<const char*>(&_lowResGridSize),
             sizeof(_lowResGridSize));

  // Write low-resolution grid.
  for (int i = 0; i < _lowResGridSize; ++i) {
    for (int j = 0; j < _lowResGridSize; ++j) {
      file.write(reinterpret_cast<const char*>(&_lowResGrid[i][j].distance),
                 sizeof(double));
    }
  }

  // Write medium-resolution grid.
  for (int i = 0; i < _mediumResGridSize; ++i) {
    for (int j = 0; j < _mediumResGridSize; ++j) {
      file.write(reinterpret_cast<const char*>(&_mediumResGrid[i][j].distance),
                 sizeof(double));
    }
  }

  // Write high-resolution grid.
  for (int i = 0; i < _highResGridSize; ++i) {
    for (int j = 0; j < _highResGridSize; ++j) {
      file.write(reinterpret_cast<const char*>(&_highResGrid[i][j].distance),
                 sizeof(double));
    }
  }

  file.close();
  MESSAGE_LOG << "Distance field saved to " << fileName;
  return true;
}

bool DistanceField::LoadFromDisk(const std::string& baseDir) {
  std::string fileName = baseDir + "/df_" + std::to_string(_key.lat) + "_" +
                         std::to_string(_key.lon) + ".dat";

  std::ifstream file(fileName, std::ios::binary);
  if (!file) {
    return false;  // File doesn't exist or can't be read
  }

  // Read and verify schema version
  int schemaVersion;
  file.read(reinterpret_cast<char*>(&schemaVersion), sizeof(schemaVersion));
  if (schemaVersion != 1) {
    MESSAGE_LOG << "Distance field file has incompatible schema version: "
                << schemaVersion;
    return false;
  }

  // Read feature type and threshold value
  int featureType;
  file.read(reinterpret_cast<char*>(&featureType), sizeof(featureType));
  file.read(reinterpret_cast<char*>(&_thresholdValue), sizeof(_thresholdValue));

  // Verify feature type is valid
  if (featureType < 0 ||
      featureType > static_cast<int>(BoundaryFeatureType::CUSTOM)) {
    MESSAGE_LOG << "Distance field file has invalid feature type: "
                << featureType;
    return false;
  }
  _featureType = static_cast<BoundaryFeatureType>(featureType);

  // Read header information
  int lat, lon, degreeSize;
  file.read(reinterpret_cast<char*>(&lat), sizeof(lat));
  file.read(reinterpret_cast<char*>(&lon), sizeof(lon));
  file.read(reinterpret_cast<char*>(&degreeSize), sizeof(degreeSize));

  // Verify the header matches our expectations
  if (lat != _key.lat || lon != _key.lon || degreeSize != _degreeSize) {
    MESSAGE_LOG << "Distance field file header mismatch";
    return false;
  }

  // Read grid dimensions
  int highResSize, mediumResSize, lowResSize;
  file.read(reinterpret_cast<char*>(&highResSize), sizeof(highResSize));
  file.read(reinterpret_cast<char*>(&mediumResSize), sizeof(mediumResSize));
  file.read(reinterpret_cast<char*>(&lowResSize), sizeof(lowResSize));

  // Verify grid dimensions match
  if (highResSize != _highResGridSize || mediumResSize != _mediumResGridSize ||
      lowResSize != _lowResGridSize) {
    MESSAGE_LOG << "Distance field grid dimensions mismatch";
    return false;
  }

  // Read low-resolution grid
  for (int i = 0; i < _lowResGridSize; ++i) {
    for (int j = 0; j < _lowResGridSize; ++j) {
      file.read(reinterpret_cast<char*>(&_lowResGrid[i][j].distance),
                sizeof(double));
      _lowResGrid[i][j].accepted = true;
      _lowResGrid[i][j].boundary = (_lowResGrid[i][j].distance == 0.0);
    }
  }

  // Read medium-resolution grid
  for (int i = 0; i < _mediumResGridSize; ++i) {
    for (int j = 0; j < _mediumResGridSize; ++j) {
      file.read(reinterpret_cast<char*>(&_mediumResGrid[i][j].distance),
                sizeof(double));
      _mediumResGrid[i][j].accepted = true;
      _mediumResGrid[i][j].boundary = (_mediumResGrid[i][j].distance == 0.0);
    }
  }

  // Read high-resolution grid
  for (int i = 0; i < _highResGridSize; ++i) {
    for (int j = 0; j < _highResGridSize; ++j) {
      file.read(reinterpret_cast<char*>(&_highResGrid[i][j].distance),
                sizeof(double));
      _highResGrid[i][j].accepted = true;
      _highResGrid[i][j].boundary = (_highResGrid[i][j].distance == 0.0);
    }
  }

  file.close();
  _valid = true;
  MESSAGE_LOG << "Distance field loaded from " << fileName << " (schema v"
              << schemaVersion
              << ", feature type: " << static_cast<int>(_featureType) << ")";
  return true;
}

//----------------------------------------------------------
//    DistanceFieldManager Implementation
//----------------------------------------------------------

DistanceFieldManager& DistanceFieldManager::GetInstance() {
  static DistanceFieldManager instance;
  return instance;
}

DistanceFieldManager::DistanceFieldManager() {
  // Initialize with default cache directory
  _cacheDir = g_Platform->GetPrivateDataDir() + "/distance_fields";

  // Create cache directory if it doesn't exist
  if (!fs::exists(_cacheDir)) {
    fs::create_directories(_cacheDir);
  }
}

DistanceFieldManager::~DistanceFieldManager() {
  // Clean up resources
  ClearCache();
}

std::shared_ptr<DistanceField> DistanceFieldManager::GetDistanceField(
    const LatLonKey& key, const std::vector<size_t>& featureIndices,
    shp::ShapefileReader* reader, BoundaryFeatureType featureType,
    double thresholdValue) {
  std::lock_guard<std::mutex> lock(_mutex);

  // Check if tile is already in the cache
  auto it = _distanceFields.find(key);
  if (it != _distanceFields.end()) {
    return it->second;
  }

  // Check if tile is already being generated
  if (IsTileGenerating(key)) {
    // Wait for the tile to be generated
    auto futureField = _generatingTiles[key].get();
    return futureField;
  }

  // Create a new distance field
  auto distField = std::make_shared<DistanceField>(key);

  // Try to load from disk first
  if (distField->LoadFromDisk(_cacheDir)) {
    _distanceFields[key] = distField;
    return distField;
  }

  // Generate the distance field asynchronously
  _generatingTiles[key] =
      std::async(std::launch::async, [this, key, featureIndices, reader]() {
        auto newField = std::make_shared<DistanceField>(key);
        // Pass validated feature indices to Generate method
        if (!featureIndices.empty() && reader) {
          newField->Generate(featureIndices, reader);
        } else {
          MESSAGE_LOG << "Cannot generate distance field: Invalid inputs";
        }

        // Save to disk for future use
        newField->SaveToDisk(_cacheDir);

        // Add to cache
        std::lock_guard<std::mutex> cacheLock(_mutex);
        _distanceFields[key] = newField;

        return newField;
      });

  // Return placeholder while generating
  return std::make_shared<DistanceField>(key);
}

bool DistanceFieldManager::CrossesLand(
    double lat1, double lon1, double lat2, double lon2,
    const std::vector<LatLonKey>& relevantTiles,
    const std::unordered_map<LatLonKey, std::vector<size_t>>& tiles,
    shp::ShapefileReader* reader) {
  // Get all tiles that the line segment intersects
  std::vector<LatLonKey> tilesToCheck = relevantTiles;
  if (tilesToCheck.empty()) {
    tilesToCheck = GetRelevantTiles(lat1, lon1, lat2, lon2);
  }

  // Check each tile for land crossings
  for (const auto& key : tilesToCheck) {
    auto it = tiles.find(key);
    if (it == tiles.end()) {
      continue;  // No features in this tile
    }

    auto distField = GetDistanceField(key, it->second, reader);
    if (distField->IsValid() &&
        distField->CrossesLand(lat1, lon1, lat2, lon2)) {
      return true;  // Found a land crossing
    }
  }

  return false;  // No land crossings found
}

double DistanceFieldManager::GetDistanceToLand(
    double lat, double lon, const std::vector<LatLonKey>& relevantTiles,
    const std::unordered_map<LatLonKey, std::vector<size_t>>& tiles,
    shp::ShapefileReader* reader) {
  // Determine which tile contains the point
  int latIdx = floor(lat);
  int lonIdx = floor(lon);
  if (lonIdx < -180) lonIdx += 360;
  if (lonIdx >= 180) lonIdx -= 360;

  LatLonKey key(latIdx, lonIdx);

  // Check if this tile is in the provided list
  bool tileExists = false;
  if (!relevantTiles.empty()) {
    tileExists = std::find(relevantTiles.begin(), relevantTiles.end(), key) !=
                 relevantTiles.end();
  } else {
    tileExists = tiles.find(key) != tiles.end();
  }

  if (!tileExists) {
    return std::numeric_limits<double>::infinity();  // No land data for this
                                                     // location
  }

  // Get the distance field for this tile
  auto distField = GetDistanceField(key, tiles.at(key), reader);

  // Query the distance
  return distField->QueryDistance(lat, lon);
}

void DistanceFieldManager::ClearCache() {
  std::lock_guard<std::mutex> lock(_mutex);

  // Wait for any ongoing generation to complete
  for (auto& [key, future] : _generatingTiles) {
    if (future.valid()) {
      future.wait();
    }
  }

  _generatingTiles.clear();
  _distanceFields.clear();
}

void DistanceFieldManager::InvalidateTiles(
    const std::vector<LatLonKey>& tiles) {
  std::lock_guard<std::mutex> lock(_mutex);

  for (const auto& key : tiles) {
    // Remove from cache
    _distanceFields.erase(key);

    // Remove cached file
    std::string fileName = _cacheDir + "/df_" + std::to_string(key.lat) + "_" +
                           std::to_string(key.lon) + ".dat";

    if (fs::exists(fileName)) {
      fs::remove(fileName);
    }
  }
}

std::vector<LatLonKey> DistanceFieldManager::GetRelevantTiles(
    double lat1, double lon1, double lat2, double lon2) const {
  std::vector<LatLonKey> tiles;

  // Normalize longitudes to -180 to 180 range
  while (lon1 < -180) lon1 += 360;
  while (lon1 >= 180) lon1 -= 360;
  while (lon2 < -180) lon2 += 360;
  while (lon2 >= 180) lon2 -= 360;

  // Get the bounding box of the line segment
  double minLat = std::min(lat1, lat2);
  double maxLat = std::max(lat1, lat2);
  double minLon = std::min(lon1, lon2);
  double maxLon = std::max(lon1, lon2);

  // Handle the case where the line crosses the date line
  bool crossesDateLine = false;
  if (maxLon - minLon > 180) {
    crossesDateLine = true;
    if (minLon < 0)
      minLon += 360;
    else
      maxLon -= 360;
  }

  // Find all tiles that the line segment intersects
  int startLat = floor(minLat);
  int endLat = floor(maxLat);
  int startLon = floor(minLon);
  int endLon = floor(maxLon);

  for (int lat = startLat; lat <= endLat; ++lat) {
    for (int lon = startLon; lon <= endLon; ++lon) {
      int normLon = lon;
      if (normLon < -180) normLon += 360;
      if (normLon >= 180) normLon -= 360;

      tiles.emplace_back(lat, normLon);
    }
  }

  // If the line crosses the date line, add tiles on both sides
  if (crossesDateLine) {
    int altStartLon = floor(minLon - 360);
    int altEndLon = floor(maxLon + 360);

    for (int lat = startLat; lat <= endLat; ++lat) {
      for (int lon = altStartLon; lon < -180; ++lon) {
        tiles.emplace_back(lat, lon + 360);
      }
      for (int lon = 180; lon <= altEndLon; ++lon) {
        tiles.emplace_back(lat, lon - 360);
      }
    }
  }

  return tiles;
}

bool DistanceFieldManager::IsTileGenerating(const LatLonKey& key) const {
  auto it = _generatingTiles.find(key);
  return it != _generatingTiles.end() && it->second.valid();
}
