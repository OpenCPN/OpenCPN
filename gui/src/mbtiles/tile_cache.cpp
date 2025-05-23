#include "tile_cache.h"
#include <memory>

TileCache::TileCache(int min_zoom, int max_zoom, float Lon_min, float Lat_min,
                     float lon_max, float lat_max)
    : m_min_zoom(min_zoom),
      m_max_zoom(max_zoom),
      m_nb_zoom(max_zoom - min_zoom + 1),
      zoom_table([&] {
        using Mtd = MbTileDescriptor;
        std::vector<ZoomDescriptor> v(m_nb_zoom);
        for (int i = 0; i < m_nb_zoom; i++) {
          int zoom_factor = min_zoom + i;
          v[i].m_tile_x_min = Mtd::Long2tilex(Lon_min + kEps, zoom_factor);
          v[i].m_tile_x_max = Mtd::Long2tilex(lon_max - kEps, zoom_factor);
          v[i].m_tile_y_min = Mtd::Lat2tiley(Lat_min + kEps, zoom_factor);
          v[i].m_tile_y_max = Mtd::Lat2tiley(lat_max - kEps, zoom_factor);
        }
        return v;
      }()) {
  // Set up how to handle the ~MbTileDescriptor() message sent using
  // on_delete.Notify(). The message contains GL resources to be deallocated.
  auto action = [&](ObservedEvt& evt) {
    auto teximage = static_cast<char*>(evt.GetClientData());
    if (teximage) free(teximage);
    auto gl_texture_name = static_cast<GLuint>(evt.GetInt());
    if (gl_texture_name) glDeleteTextures(1, &gl_texture_name);
  };
  delete_listener.Init(on_delete, action);
}

std::mutex& TileCache::GetMutex(uint64_t tile_id) {
  static const int kMutexCount = 100;
  static std::array<std::mutex, kMutexCount> mutexes;
  return mutexes[tile_id % kMutexCount];
}

std::mutex& TileCache::GetMutex(const SharedTilePtr& tile) {
  uint64_t key = MbTileDescriptor::GetMapKey(tile->m_zoom_level, tile->m_tile_x,
                                             tile->m_tile_y);
  return TileCache::GetMutex(key);
}

SharedTilePtr TileCache::GetTile(int z, int x, int y) {
  uint64_t index = MbTileDescriptor::GetMapKey(z, x, y);
  auto ref = m_tile_map.find(index);
  if (ref != m_tile_map.end()) {
    // The tile is in the cache
    ref->second->SetTimestamp();
    return ref->second;
  }

  // The tile is not in the cache : create an empty one and add it to the tile
  // map and list
  auto tile = std::make_shared<MbTileDescriptor>(z, x, y, on_delete);
  m_tile_map[index] = tile;
  return tile;
}

void TileCache::CleanCache(uint32_t max_tiles) {
  if (m_tile_map.size() <= max_tiles) return;

  // Create a sorted list of keys, oldest first.
  std::vector<uint64_t> keys;
  for (auto& kv : m_tile_map) keys.push_back(kv.first);
  auto compare = [&](const uint64_t lhs, const uint64_t rhs) {
    return m_tile_map[lhs]->m_last_used < m_tile_map[rhs]->m_last_used;
  };
  std::sort(keys.begin(), keys.end(), compare);

  for (size_t i = 0; i < m_tile_map.size() - max_tiles; i += 1) {
    std::lock_guard lock(TileCache::GetMutex(m_tile_map[keys[i]]));
    auto tile = m_tile_map[keys[i]];
    m_tile_map.erase(keys[i]);
  }
}

void TileCache::DeepCleanCache() {
  using namespace std::chrono;
  auto time_now =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch());

  auto age_limit = std::chrono::duration<int>(5);  // 5 seconds

  std::vector<uint64_t> keys;
  for (auto& kv : m_tile_map) keys.push_back(kv.first);

  for (size_t i = 0; i < keys.size(); i += 1) {
    std::lock_guard lock(TileCache::GetMutex(m_tile_map[keys[i]]));
    auto tile = m_tile_map[keys[i]];
    const std::chrono::duration<double> elapsed_seconds{time_now -
                                                        tile->m_last_used};

    //  Looking for tiles that have been fetched from sql,
    //  but not yet rendered.  Such tiles contain a large bitmap allocation.
    //  After some time, it is likely they never will be needed in short term.
    //  So safe to delete, and reload as necessary.
    if (elapsed_seconds > age_limit) {
      m_tile_map.erase(keys[i]);
    }
  }
}
