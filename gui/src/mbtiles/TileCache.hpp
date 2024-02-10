#ifndef _TILECACHE_H_
#define _TILECACHE_H_

#include "TileDescriptor.hpp"

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

public:
  TileCache(int minZoom, int maxZoom, float LonMin, float LatMin, float LonMax,
            float LatMax) {
    this->minZoom = minZoom;
    this->maxZoom = maxZoom;
    nbZoom = (maxZoom - minZoom) + 1;
    zoomTable = new ZoomDescriptor[nbZoom];

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

  int GetAreaNorth(int zoomLevel) {
    return zoomTable[zoomLevel - minZoom].tile_y_max;
  }

  int GetAreaSouth(int zoomLevel) {
    return zoomTable[zoomLevel - minZoom].tile_y_min;
  }

  /// @brief Retreive a tile from the cache
  /// @param z Zoom level of the tile
  /// @param x x coordinate of the tile
  /// @param y y coordinate of the tile
  /// @return Pointer to the tile, nullptr if the tile is not present in the
  /// cache
  mbTileDescriptor *GetTile(int z, int x, int y) {
    auto ref = tileMap.find(mbTileDescriptor::GetMapKey(z, x, y));
    if (ref != tileMap.end()) {
      return ref->second;
    }

    return nullptr;
  }

  void AddTile(mbTileDescriptor *tile) {
    uint64_t index = mbTileDescriptor::GetMapKey(tile->m_zoomLevel,
                                                 tile->tile_x, tile->tile_y);
    tileMap[index] = tile;
  }
};

#endif