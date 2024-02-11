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
  // Chained list parameters
  mbTileDescriptor *listStart = nullptr;
  mbTileDescriptor *listEnd = nullptr;
  uint32_t listSize = 0;

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
    listStart = nullptr;
    listEnd = nullptr;
    listSize = 0;
  }

  int GetAreaNorth(int zoomLevel) {
    return zoomTable[zoomLevel - minZoom].tile_y_max;
  }

  int GetAreaSouth(int zoomLevel) {
    return zoomTable[zoomLevel - minZoom].tile_y_min;
  }

  uint32_t GetCacheSize() { return listSize; }

  uint32_t GetRealCacheSize() {
    uint32_t cacheSize = 0;

    mbTileDescriptor *tile = listStart;
    while (tile != nullptr) {
      cacheSize++;
      tile = tile->next;
    }

    return cacheSize;
  }

  /// @brief Retreive a tile from the cache. If the tile is not present in the
  /// cache, it is created and added.
  /// @param z Zoom level of the tile
  /// @param x x coordinate of the tile
  /// @param y y coordinate of the tile
  /// @return Pointer to the tile, nullptr if the tile is not present in the
  /// cache
  mbTileDescriptor *GetTile(int z, int x, int y) {
    uint64_t index = mbTileDescriptor::GetMapKey(z, x, y);
    auto ref = tileMap.find(index);
    if (ref != tileMap.end()) {
      MoveTileToListStart(ref->second);
      return ref->second;
    }

    mbTileDescriptor *tile = new mbTileDescriptor(z, x, y);
    tileMap[index] = tile;
    AddTileToList(tile);

    return tile;
  }

  void CleanCache(uint32_t minTiles) {
    uint64_t index;

    while (listSize > minTiles) {
      index = mbTileDescriptor::GetMapKey(listEnd->m_zoomLevel, listEnd->tile_x,
                                          listEnd->tile_y);
      auto ref = tileMap.find(index);
      if (ref == tileMap.end()) {
        break;
      }
      if ((ref->second->m_bAvailable) && (ref->second->m_teximage == 0) && (ref->second->glTextureName == 0)) {
        break;
      }
      tileMap.erase(ref);
      DeleteTileFromList(listEnd);
    }
  }

private:
  void AddTileToList(mbTileDescriptor *tile) {
    if (listStart == nullptr) {
      // List is empty : add the first element
      tile->prev = nullptr;
      tile->next = nullptr;
      listStart = tile;
      listEnd = tile;
    } else {
      // Insert tile at the start of the list
      tile->prev = nullptr;
      tile->next = listStart;
      listStart->prev = tile;
      listStart = tile;
    }
    listSize++;
  }

  void DeleteTileFromList(mbTileDescriptor *tile) {
    if (tile) {
      if (tile->prev == nullptr) {
        listStart = tile->next;
      }

      if (tile->next == nullptr) {
        listEnd = tile->prev;
      } else {
        tile->next->prev = tile->prev;
      }

      tile->prev->next = tile->next;
      listSize--;

      delete tile;
    }
  }

  void MoveTileToListStart(mbTileDescriptor *tile) {
    if (tile) {
      if (tile->prev == nullptr) {
        // Tile is already at beginning of list : exit function
        return;
      }

      if (tile->next == nullptr) {
        // Tile is at the end of list : update list end pointer
        listEnd = tile->prev;
      } else {
        // We have a successor : update its previous pointer
        tile->next->prev = tile->prev;
      }

      // Tile's predecessor must have its next pointer updated
      tile->prev->next = tile->next;

      // Insert tile at beginning of list
      listStart->prev = tile;
      tile->next = listStart;
      tile->prev = nullptr;
      listStart = tile;
    }
  }
};

#endif