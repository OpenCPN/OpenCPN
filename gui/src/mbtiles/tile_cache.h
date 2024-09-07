#ifndef _TILECACHE_H_
#define _TILECACHE_H_

#include "GL/glew.h"
#include "tile_descr.h"
#include <mutex>

/** Manage the tiles of a mbtiles file. */
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

  /**
   * Return mutex to lock given tile. There is a fixed number of mutexes
   * available, the mutex returned might collide with another id causing
   * random serialization.
   */
  static std::mutex &GetMutex(uint64_t tile_id);

  /**
   * Return mutex to lock given tile. There is a fixed number of mutexes
   * available, the mutex returned might collide with another id causing
   * random serialization.
   */
  static std::mutex &GetMutex(const mbTileDescriptor *tile);

  /** Flush the tile cache, including OpenGL texture memory if needed */
  void Flush();

  /**
   * Get the north limit of the cache area for a given zoom in WMTS coordinates.
   */
  int GetNorthLimit(int zoomLevel) {
    return zoomTable[zoomLevel - minZoom].tile_y_max;
  }

  /**
   * Get the south limit of the cache area for a given zoom in WMTS coordinates.
   */
  int GetSouthLimit(int zoomLevel) {
    return zoomTable[zoomLevel - minZoom].tile_y_min;
  }

  /**
   *  Get the current cache size.
   *  @return Number of tiles in cache.
   */
  uint32_t GetCacheSize() const { return tileMap.size(); }

 /**
  *  Retreive a tile from cache. If the tile is not present an empty tile 
  *  is created, added and returned.
  *  @param z Tile zoom level.
  *  @param x Tile x coordinate.
  *  @param y Tile y coordinate.
  *  @return Pointer to tile
  */
  mbTileDescriptor *GetTile(int z, int x, int y);

 /**
  *  Reduce the size of the cache if it exceeds the given limit. Must
  *  only be called by rendering thread since it uses OpenGL calls.
  *  @param maxTiles Maximum number of tiles to be kept in the cache.
  */
  void CleanCache(uint32_t maxTiles);

  void DeepCleanCache();
};

#endif
