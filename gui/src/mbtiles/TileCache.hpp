#ifndef _TILECACHE_H_
#define _TILECACHE_H_

#include "TileDescriptor.hpp"
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
            float LatMax) {
    this->minZoom = minZoom;
    this->maxZoom = maxZoom;
    nbZoom = (maxZoom - minZoom) + 1;
    zoomTable = new ZoomDescriptor[nbZoom];

    // Compute cache coverage for every zoom level in WMTS coordinates
    for (int i = 0; i < nbZoom; i++) {
      int zoomFactor = minZoom + i;
      zoomTable[i].tile_x_min =
          mbTileDescriptor::long2tilex(LonMin + eps, zoomFactor);
      zoomTable[i].tile_x_max =
          mbTileDescriptor::long2tilex(LonMax - eps, zoomFactor);
      zoomTable[i].tile_y_min =
          mbTileDescriptor::lat2tiley(LatMin + eps, zoomFactor);
      zoomTable[i].tile_y_max =
          mbTileDescriptor::lat2tiley(LatMax - eps, zoomFactor);
    }
  }

  virtual ~TileCache() { delete[] zoomTable; }

  /// Return mutex to lock given tile. There is a fixed number of mutexes
  /// available, the mutex returned might collide with another id causing
  /// random serialization.
  static std::mutex &GetMutex(uint64_t tile_id) {
    static const int kMutexCount = 100;
    static std::array<std::mutex, kMutexCount> mutexes;
    return mutexes[tile_id % kMutexCount];
  }

  static std::mutex &GetMutex(const mbTileDescriptor *tile) {
    uint64_t key = mbTileDescriptor::GetMapKey(tile->m_zoomLevel, tile->tile_x,
                                               tile->tile_y);
    return TileCache::GetMutex(key);
  }

  /// @brief Flush the tile cache, including OpenGL texture memory if needed
  void Flush() {
    for (auto const &it : tileMap) {
      mbTileDescriptor *tile = it.second;
      if (tile) {
        // Note that all buffers are properly freed by the destructor, including
        // OpenGL textures. It means that this function must only be called from
        // the main rendering thread since OpenGL is not thread safe.
        delete tile;
      }
    }
  }

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
  mbTileDescriptor *GetTile(int z, int x, int y) {
    uint64_t index = mbTileDescriptor::GetMapKey(z, x, y);
    auto ref = tileMap.find(index);
    if (ref != tileMap.end()) {
      // The tile is in the cache
      ref->second->SetTimestamp();
      return ref->second;
    }

    // The tile is not in the cache : create an empty one and add it to the tile
    // map and list
    mbTileDescriptor *tile = new mbTileDescriptor(z, x, y);
    tileMap[index] = tile;
    return tile;
  }

  /// @brief Reduce the size of the cache if it exceeds the given limit. To
  /// reduce the size of the cache, the tiles at the end of the tile list are
  /// deleted first (i.e. the least frequently used ones). This function must
  /// only be called by rendering thread since it uses OpenGL calls.
  /// @param maxTiles Maximum number of tiles to be kept in the list
  void CleanCache(uint32_t maxTiles) {
    if (tileMap.size() <= maxTiles) return;

    /** Create a sorted list of keys, oldest first. */
    std::vector<uint64_t> keys;
    for (auto &kv : tileMap) keys.push_back(kv.first);
    auto compare = [&](const uint64_t lhs, const uint64_t rhs) {
      return tileMap[lhs]->last_used < tileMap[rhs]->last_used;
    };
    std::sort(keys.begin(), keys.end(), compare);

    for (size_t i = 0; i < tileMap.size() - maxTiles; i += 1) {
      std::lock_guard lock(TileCache::GetMutex(tileMap[keys[i]]));

      auto tile = tileMap[keys[i]];
      tileMap.erase(keys[i]);
      delete tile;
    }
  }
};
#endif
