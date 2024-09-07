#include "tile_cache.h"

TileCache::TileCache(int minZoom, int maxZoom, float LonMin, float LatMin,
                     float LonMax, float LatMax) {
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

std::mutex &TileCache::GetMutex(uint64_t tile_id) {
  static const int kMutexCount = 100;
  static std::array<std::mutex, kMutexCount> mutexes;
  return mutexes[tile_id % kMutexCount];
}

std::mutex &TileCache::GetMutex(const mbTileDescriptor *tile) {
  uint64_t key = mbTileDescriptor::GetMapKey(tile->m_zoomLevel, tile->tile_x,
                                             tile->tile_y);
  return TileCache::GetMutex(key);
}

void TileCache::Flush() {
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

mbTileDescriptor *TileCache::GetTile(int z, int x, int y) {
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

void TileCache::CleanCache(uint32_t maxTiles) {
  if (tileMap.size() <= maxTiles) return;

  // Create a sorted list of keys, oldest first.
  std::vector<uint64_t> keys;
  for (auto &kv : tileMap) keys.push_back(kv.first);
  auto compare = [&](const uint64_t lhs, const uint64_t rhs) {
    return tileMap[lhs]->last_used < tileMap[rhs]->last_used;
  };
  std::sort(keys.begin(), keys.end(), compare);

  for (size_t i = 0; i < tileMap.size() - maxTiles; i += 1) {
    std::lock_guard lock(TileCache::GetMutex(tileMap[keys[i]]));
    auto tile = tileMap[keys[i]];

    // Do not remove tiles that may be pending in the worker thread queue
    if (tile->m_bAvailable && !tile->glTextureName && !tile->m_teximage)
      continue;

    tileMap.erase(keys[i]);
    delete tile;
  }
}

void TileCache::DeepCleanCache() {
  using namespace std::chrono;
  auto time_now =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch());

  auto age_limit = std::chrono::duration<int>(5);  // 5 seconds

  std::vector<uint64_t> keys;
  for (auto &kv : tileMap) keys.push_back(kv.first);

  for (size_t i = 0; i < keys.size(); i += 1) {
    std::lock_guard lock(TileCache::GetMutex(tileMap[keys[i]]));
    auto tile = tileMap[keys[i]];
    const std::chrono::duration<double> elapsed_seconds{time_now -
                                                        tile->last_used};

    //  Looking for tiles that have been fetched from sql,
    //  but not yet rendered.  Such tiles contain a large bitmap allocation.
    //  After some time, it is likely they never will be needed in short term.
    //  So safe to delete, and reload as necessary.
    if (((!tile->glTextureName) && tile->m_teximage) &&
        (elapsed_seconds > age_limit)) {
      tileMap.erase(keys[i]);
      delete tile;
    }
  }
}
