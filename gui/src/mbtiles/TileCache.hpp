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
    // Reset the chained list
    listStart = nullptr;
    listEnd = nullptr;
    listSize = 0;
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
  uint32_t GetCacheSize() { return listSize; }

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
      // Move it to the beginning of the tile list so that the list will have
      // the most frequently needed tiles at the beginning
      MoveTileToListStart(ref->second);
      return ref->second;
    }

    // The tile is not in the cache : create an empty one and add it to the tile
    // map and list
    mbTileDescriptor *tile = new mbTileDescriptor(z, x, y);
    tileMap[index] = tile;
    AddTileToList(tile);

    return tile;
  }

  /// @brief Reduce the size of the cache if it exceeds the given limit. To
  /// reduce the size of the cache, the tiles at the end of the tile list are
  /// deleted first (i.e. the least frequently used ones). This function must
  /// only be called by rendering thread since it uses OpenGL calls.
  /// @param maxTiles Maximum number of tiles to be kept in the list
  void CleanCache(uint32_t maxTiles) {
    uint64_t index;

    while (listSize > maxTiles) {
      // List size exceeds the maximum value : delete the last tile of the list
      index = mbTileDescriptor::GetMapKey(listEnd->m_zoomLevel, listEnd->tile_x,
                                          listEnd->tile_y);
      auto ref = tileMap.find(index);
      if ((ref->second->m_bAvailable) && (ref->second->m_teximage == 0) &&
          (ref->second->glTextureName == 0)) {
        // If the tile is currently used by worker thread, we must not delete
        // it. Practically, this case is not supposed to happen, unless the
        // system is really, really slow. In that case we exit the function and
        // wait the next rendering to try again.
        break;
      }
      // Remove the tile from map and delete it. Tile destructor takes care to
      // properly
      tileMap.erase(ref);
      DeleteTileFromList(listEnd);
    }
  }

private:
  /// @brief Add a new tile to the tile list.
  /// @param tile Pointer to the tile
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
    // Update list size
    listSize++;
  }

  /// @brief Remove a tile from the tile list and delete it.
  /// @param tile Pointer to the tile to be deleted
  void DeleteTileFromList(mbTileDescriptor *tile) {
    if (tile) {
      if (tile->prev == nullptr) {
        // Tile is at beginning of the list
        listStart = tile->next;
      } else {
        tile->prev->next = tile->next;
      }

      if (tile->next == nullptr) {
        // Tile is at the end of the list
        listEnd = tile->prev;
      } else {
        tile->next->prev = tile->prev;
      }

      // Delete the tile
      delete tile;
      listSize--;
    }
  }

  /// @brief Move a tile at the beginning of the list.
  /// @param tile Tile to be moved
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