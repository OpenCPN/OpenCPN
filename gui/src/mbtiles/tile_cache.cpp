#include "tile_cache.h"

TileCache::TileCache(int min_zoom, int max_zoom, float Lon_min, float Lat_min,
                     float lon_max, float lat_max) {
  this->m_min_zoom = min_zoom;
  this->m_max_zoom = max_zoom;
  m_nb_zoom = (max_zoom - min_zoom) + 1;
  zoom_table = new ZoomDescriptor[m_nb_zoom];

  // Compute cache coverage for every zoom level in WMTS coordinates
  for (int i = 0; i < m_nb_zoom; i++) {
    int zoomFactor = min_zoom + i;
    zoom_table[i].m_tile_x_min =
        MbTileDescriptor::Long2tilex(Lon_min + kEps, zoomFactor);
    zoom_table[i].m_tile_x_max =
        MbTileDescriptor::Long2tilex(lon_max - kEps, zoomFactor);
    zoom_table[i].m_tile_y_min =
        MbTileDescriptor::Lat2tiley(Lat_min + kEps, zoomFactor);
    zoom_table[i].m_tile_y_max =
        MbTileDescriptor::Lat2tiley(lat_max - kEps, zoomFactor);
  }
}

std::mutex &TileCache::GetMutex(uint64_t tile_id) {
  static const int kMutexCount = 100;
  static std::array<std::mutex, kMutexCount> mutexes;
  return mutexes[tile_id % kMutexCount];
}

std::mutex &TileCache::GetMutex(const MbTileDescriptor *tile) {
  uint64_t key = MbTileDescriptor::GetMapKey(tile->m_zoomLevel, tile->m_tile_x,
                                             tile->m_tile_y);
  return TileCache::GetMutex(key);
}

void TileCache::Flush() {
  for (auto const &it : tile_map) {
    MbTileDescriptor *tile = it.second;
    if (tile) {
      // Note that all buffers are properly freed by the destructor, including
      // OpenGL textures. It means that this function must only be called from
      // the main rendering thread since OpenGL is not thread safe.
      delete tile;
    }
  }
}

MbTileDescriptor *TileCache::GetTile(int z, int x, int y) {
  uint64_t index = MbTileDescriptor::GetMapKey(z, x, y);
  auto ref = tile_map.find(index);
  if (ref != tile_map.end()) {
    // The tile is in the cache
    ref->second->SetTimestamp();
    return ref->second;
  }

  // The tile is not in the cache : create an empty one and add it to the tile
  // map and list
  MbTileDescriptor *tile = new MbTileDescriptor(z, x, y);
  tile_map[index] = tile;
  return tile;
}

void TileCache::CleanCache(uint32_t max_tiles) {
  if (tile_map.size() <= max_tiles) return;

  // Create a sorted list of keys, oldest first.
  std::vector<uint64_t> keys;
  for (auto &kv : tile_map) keys.push_back(kv.first);
  auto compare = [&](const uint64_t lhs, const uint64_t rhs) {
    return tile_map[lhs]->m_last_used < tile_map[rhs]->m_last_used;
  };
  std::sort(keys.begin(), keys.end(), compare);

  for (size_t i = 0; i < tile_map.size() - max_tiles; i += 1) {
    std::lock_guard lock(TileCache::GetMutex(tile_map[keys[i]]));
    auto tile = tile_map[keys[i]];

    // Do not remove tiles that may be pending in the worker thread queue
    if (tile->m_is_available && !tile->m_gl_texture_name && !tile->m_teximage)
      continue;

    tile_map.erase(keys[i]);
    delete tile;
  }
}

void TileCache::DeepCleanCache() {
  using namespace std::chrono;
  auto time_now =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch());

  auto age_limit = std::chrono::duration<int>(5);  // 5 seconds

  std::vector<uint64_t> keys;
  for (auto &kv : tile_map) keys.push_back(kv.first);

  for (size_t i = 0; i < keys.size(); i += 1) {
    std::lock_guard lock(TileCache::GetMutex(tile_map[keys[i]]));
    auto tile = tile_map[keys[i]];
    const std::chrono::duration<double> elapsed_seconds{time_now -
                                                        tile->m_last_used};

    //  Looking for tiles that have been fetched from sql,
    //  but not yet rendered.  Such tiles contain a large bitmap allocation.
    //  After some time, it is likely they never will be needed in short term.
    //  So safe to delete, and reload as necessary.
    if (((!tile->m_gl_texture_name) && tile->m_teximage) &&
        (elapsed_seconds > age_limit)) {
      tile_map.erase(keys[i]);
      delete tile;
    }
  }
}