#ifndef _TILECACHE_H_
#define _TILECACHE_H_

#include "GL/glew.h"
#include "tile_descr.h"
#include <mutex>

/// @brief Class managing the tiles of a mbtiles file
class TileCache {
  //  Per zoomlevel descriptor of tile array for that zoomlevel
  class ZoomDescriptor {
  public:
    int tile_x_min, tile_x_max;
    int tile_y_min, tile_y_max;
  };

private:
  const double eps = 6e-6;  // about 1cm on earth's surface at equator
  std::unordered_map<uint64_t, mbTileDescriptor *> tileMap;
  ZoomDescriptor *zoomTable;
  int minZoom, maxZoom, nbZoom;
  // Chained list parameters

public:
  TileCache(int minZoom, int maxZoom, float LonMin, float LatMin, float LonMax,
            float LatMax);

  virtual ~TileCache() { delete[] zoomTable; }

  /// Return mutex to lock given tile. There is a fixed number of mutexes
  /// available, the mutex returned might collide with another id causing
  /// random serialization.
  static std::mutex &GetMutex(uint64_t tile_id);

  static std::mutex &GetMutex(const mbTileDescriptor *tile);

  /// @brief Flush the tile cache, including OpenGL texture memory if needed
  void Flush();

  // Get the north limit of the cache area for a given zoom in WMTS coordinates
  int GetNorthLimit(int zoomLevel) {
    return zoomTable[zoomLevel - minZoom].tile_y_max;
  }

  // Get the south limit of the cache area for a given zoom in WMTS coordinates
  int GetSouthLimit(int zoomLevel) {
    return zoomTable[zoomLevel - minZoom].tile_y_min;
  }

  /// @brief Get the current size of the cache in number of tiles
  /// @return Number of tiles in the cache
  uint32_t GetCacheSize() const { return tileMap.size(); }

  /// @brief Retreive a tile from the cache. If the tile is not present in the
  /// cache, an empty tile is created and added.
  /// @param z Zoom level of the tile
  /// @param x x coordinate of the tile
  /// @param y y coordinate of the tile
  /// @return Pointer to the tile
  mbTileDescriptor *GetTile(int z, int x, int y);

  /// @brief Reduce the size of the cache if it exceeds the given limit. To
  /// reduce the size of the cache, the tiles at the end of the tile list are
  /// deleted first (i.e. the least frequently used ones). This function must
  /// only be called by rendering thread since it uses OpenGL calls.
  /// @param maxTiles Maximum number of tiles to be kept in the list
  void CleanCache(uint32_t maxTiles);

  void DeepCleanCache();
};

#endif
