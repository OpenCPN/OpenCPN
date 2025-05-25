#ifndef _TILECACHE_H_
#define _TILECACHE_H_

#include <mutex>

#include "tile_descr.h"
#include "observable_evtvar.h"

/** Manage the tiles of a mbtiles file. */
class TileCache {
  //  Per zoomlevel descriptor of tile array for that zoomlevel
  class ZoomDescriptor {
  public:
    int m_tile_x_min;
    int m_tile_x_max;
    int m_tile_y_min;
    int m_tile_y_max;
  };

private:
  const double kEps = 6e-6;  // about 1cm on earth's surface at equator
  std::unordered_map<uint64_t, SharedTilePtr> m_tile_map;
  const int m_min_zoom;
  const int m_max_zoom;
  const int m_nb_zoom;
  const std::vector<ZoomDescriptor> zoom_table;
  ObsListener delete_listener;

public:
  TileCache(int min_zoom, int max_zoom, float Lon_min, float Lat_min,
            float lon_max, float lat_max);

  /** Notified with a GLUint and const char* when a tile goes out of scope. */
  EventVar on_delete;

  /**
   * Return mutex to lock given tile. There is a fixed number of mutexes
   * available, the mutex returned might collide with another id causing
   * random serialization.
   */
  static std::mutex& GetMutex(uint64_t tile_id);

  /**
   * Return mutex to lock given tile. There is a fixed number of mutexes
   * available, the mutex returned might collide with another id causing
   * random serialization.
   */
  static std::mutex& GetMutex(const SharedTilePtr& tile);

  /** Flush the tile cache, including OpenGL texture memory if needed */
  void Flush() { m_tile_map.clear(); }

  /**
   * Get the north limit of the cache area for a given zoom in WMTS coordinates.
   */
  int GetNorthLimit(int zoom_level) {
    return zoom_table[zoom_level - m_min_zoom].m_tile_y_max;
  }

  /**
   * Get the south limit of the cache area for a given zoom in WMTS coordinates.
   */
  int GetSouthLimit(int zoom_level) {
    return zoom_table[zoom_level - m_min_zoom].m_tile_y_min;
  }

  /**
   *  Get the current cache size.
   *  @return Number of tiles in cache.
   */
  uint32_t GetCacheSize() const { return m_tile_map.size(); }

  /**
   *  Retreive a tile from cache. If the tile is not present an empty tile
   *  is created, added and returned.
   *  @param z Tile zoom level.
   *  @param x Tile x coordinate.
   *  @param y Tile y coordinate.
   *  @return Pointer to tile
   */
  SharedTilePtr GetTile(int z, int x, int y);

  /**
   *  Reduce the size of the cache if it exceeds the given limit. Must
   *  only be called by rendering thread since it uses OpenGL calls.
   *  @param max_tiles Maximum number of tiles to be kept in the cache.
   */
  void CleanCache(uint32_t max_tiles);

  void DeepCleanCache();
};

#endif
