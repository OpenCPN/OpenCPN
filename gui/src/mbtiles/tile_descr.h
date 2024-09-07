#ifndef _MBTILES_TILEDESCRIPTOR_H_
#define _MBTILES_TILEDESCRIPTOR_H_

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>

#include "chartbase.h"
#include "glChartCanvas.h"

/** Per tile descriptor. */
class MbTileDescriptor {
public:
  int m_tile_x;
  int m_tile_y;
  int m_zoomLevel;
  float m_latmin;
  float m_lonmin;
  float m_latmax;
  float m_lonmax;
  LLBBox m_box;
  std::chrono::milliseconds m_last_used;

  /// Set to true if a load request from main thread is already pending for this
  /// tile
  bool m_requested;

  /// Pointer to the decompressed tile image
  std::atomic<unsigned char *> m_teximage;

  /// Identifier of the tile texture in OpenGL memory
  GLuint m_gl_texture_name;

  /// Set to true if the tile has not been found into the SQL database.
  std::atomic<bool> m_is_available;

  MbTileDescriptor(int zoomFactor, int x, int y) {
    m_gl_texture_name = 0;
    m_is_available = true;
    m_teximage = nullptr;
    m_requested = false;
    m_tile_x = x;
    m_tile_y = y;
    m_zoomLevel = zoomFactor;
    // Calculate tile boundaries
    m_lonmin =
        round(MbTileDescriptor::Tilex2long(m_tile_x, zoomFactor) / kEps) * kEps;
    m_lonmax =
        round(MbTileDescriptor::Tilex2long(m_tile_x + 1, zoomFactor) / kEps) *
        kEps;
    m_latmin =
        round(MbTileDescriptor::Tiley2lat(m_tile_y - 1, zoomFactor) / kEps) *
        kEps;
    m_latmax = round(MbTileDescriptor::Tiley2lat(m_tile_y, zoomFactor) / kEps) * kEps;

    m_box.Set(m_latmin, m_lonmin, m_latmax, m_lonmax);
    SetTimestamp();
  }

  virtual ~MbTileDescriptor() {
    // Delete intermediate texture buffer if still allocated
    if (m_teximage != nullptr) {
      free(m_teximage);
    }
    // Delete OpenGL texture buffer if allocated
    if (m_gl_texture_name > 0) {
      glDeleteTextures(1, &m_gl_texture_name);
    }
  }
 /**
  * Generate an unique 64 bit key/identifier for a tile. This key can
  * be used to uniquely reference tiles in an unordered_map or other similar
  * list, with not risk of key collision up to zoom level 20
  * @param z Tile Zoom level.
  * @param x Tile x coordinate.
  * @param y Tile y coordinate.
  * @return Unique 64 bit key for tile
  */
  static uint64_t GetMapKey(int z, int x, int y) {
    return ((uint64_t)z << 40) | ((uint64_t)y << 20) | x;
  }

  /**
  * Generate a unique 64 bit key/identifier for  tile. This key can
  * be used to uniquely reference tiles in a unordered_map or other similar
  * list, with not risk of key collision up to zoom level 20
  * @return Unique 64 bit key for tile
  */
  uint64_t GetMapKey() {
    return MbTileDescriptor::GetMapKey(m_zoomLevel, m_tile_x, m_tile_y);
  }

  static int Long2tilex(double lon, int z) {
    if (lon < -180) lon += 360;
    return (int)(floor((lon + 180.0) / 360.0 * (1 << z)));
  }

  static int Lat2tiley(double lat, int z) {
    int y = (int)(floor(
        (1.0 -
         log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) /
        2.0 * (1 << z)));
    int ymax = 1 << z;
    y = ymax - y - 1;
    return y;
  }

  static double Tilex2long(int x, int z) {
    return x / (double)(1 << z) * 360.0 - 180;
  }

  static double Tiley2lat(int y, int z) {
    // double n = pow(2.0, z);
    double n = 1 << z;
    int ymax = 1 << z;
    y = ymax - y - 1;
    double lat_rad = atan(sinh(M_PI * (1 - (2 * y / n))));
    return 180.0 / M_PI * lat_rad;
  }

  /** Update m_last_used to current time. */
  void SetTimestamp() {
    using namespace std::chrono;
    m_last_used =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  }

private:
  const double kEps = 6e-6;  // about 1cm on earth's surface at equator
};

#endif /* _MBTILES_TILEDESCRIPTOR_H_ */
