#ifndef _MBTILES_TILEDESCRIPTOR_H_
#define _MBTILES_TILEDESCRIPTOR_H_

#include <cstdint>
#include <atomic>
#include <cmath>

#include "chartbase.h"
#include "glChartCanvas.h"

/// @brief Per tile descriptor
class mbTileDescriptor {
public:
  int tile_x, tile_y;
  int m_zoomLevel;
  float latmin, lonmin, latmax, lonmax;
  LLBBox box;

  // Set to true if a load request from main thread is already pending for this
  // tile
  bool m_requested;
  // Pointer to the decompressed tile image
  std::atomic<unsigned char *> m_teximage;
  // Identifier of the tile texture in OpenGL memory
  GLuint glTextureName;
  // Set to true if the tile has not been found into the SQL database.
  std::atomic<bool> m_bAvailable;
  // Pointer to the previous element of the tile chained list
  mbTileDescriptor *prev;
  // Pointer to the next element of the tile chained list
  mbTileDescriptor *next;

  mbTileDescriptor(int zoomFactor, int x, int y) {
    glTextureName = 0;
    m_bAvailable = true;
    m_teximage = nullptr;
    m_requested = false;
    prev = nullptr;
    next = nullptr;
    tile_x = x;
    tile_y = y;
    m_zoomLevel = zoomFactor;
    // Calculate tile boundaries
    lonmin =
        round(mbTileDescriptor::tilex2long(tile_x, zoomFactor) / eps) * eps;
    lonmax =
        round(mbTileDescriptor::tilex2long(tile_x + 1, zoomFactor) / eps) * eps;
    latmin =
        round(mbTileDescriptor::tiley2lat(tile_y - 1, zoomFactor) / eps) * eps;
    latmax = round(mbTileDescriptor::tiley2lat(tile_y, zoomFactor) / eps) * eps;

    box.Set(latmin, lonmin, latmax, lonmax);
  }

  virtual ~mbTileDescriptor() {
    if (m_teximage != nullptr) {
      free(m_teximage);
    }
    if (glTextureName > 0) {
      glDeleteTextures(1, &glTextureName);
    }
  }

  /// @brief Generates a unique 64 bit key/identifier of a tile. This key can
  /// be used to uniquely reference tiles in a unordered_map or other similar
  /// list, with not risk of key collision up to zoom level 20
  /// @param z Zoom level of the tile
  /// @param x x coordinate of the file
  /// @param y y coordinate of the tile
  /// @return Unique 64 bit key of the tile
  static uint64_t GetMapKey(int z, int x, int y) {
    return ((uint64_t)z << 40) | ((uint64_t)y << 20) | x;
  }

  /// @brief Generates a unique 64 bit key/identifier of the tile. This key can
  /// be used to uniquely reference tiles in a unordered_map or other similar
  /// list, with not risk of key collision up to zoom level 20
  /// @return Unique 64 bit key of the tile
  uint64_t GetMapKey() {
    return mbTileDescriptor::GetMapKey(m_zoomLevel, tile_x, tile_y);
  }

  static int long2tilex(double lon, int z) {
    if (lon < -180) lon += 360;

    return (int)(floor((lon + 180.0) / 360.0 * (1 << z)));
  }

  static int lat2tiley(double lat, int z) {
    int y = (int)(floor(
        (1.0 -
         log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) /
        2.0 * (1 << z)));
    int ymax = 1 << z;
    y = ymax - y - 1;
    return y;
  }

  static double tilex2long(int x, int z) {
    return x / (double)(1 << z) * 360.0 - 180;
  }

  static double tiley2lat(int y, int z) {
    // double n = pow(2.0, z);
    double n = 1 << z;
    int ymax = 1 << z;
    y = ymax - y - 1;
    double latRad = atan(sinh(M_PI * (1 - (2 * y / n))));
    return 180.0 / M_PI * latRad;
  }

private:
  const double eps = 6e-6;  // about 1cm on earth's surface at equator
};

#endif /* _MBTILES_TILEDESCRIPTOR_H_ */