#ifndef _MBTILESTILEQUEUE_H_
#define _MBTILESTILEQUEUE_H_

#include <condition_variable>
#include <mutex>
#include <vector>

#include "tile_descr.h"

/** A thread safe tile queue between two threads. */
class TileQueue {
public:
  TileQueue() {}

  /**
   *  Push a tile to the queue.
   *  @param tile Pointer to tile descriptor to be pushed.
   */
  void Push(SharedTilePtr tile) {
    {
      std::lock_guard lock(m_mutex);
      m_tile_list.push_back(tile);
    }
    m_cv.notify_all();
  }

  /**
   *  Retrieve a tile from the queue. If there is no tile in the queue,
   *  calling thread is blocked until a tile is available.
   *
   *  @return Pointer to tile descriptor
   */
  SharedTilePtr Pop() {
    std::unique_lock lock(m_mutex);
    m_cv.wait(lock, [&] { return m_tile_list.size() > 0; });
    auto tile = m_tile_list.at(0);
    m_tile_list.erase(m_tile_list.cbegin());
    return tile;
  }

  /**  Retrieve current size of queue. */
  uint32_t GetSize() {
    std::lock_guard lock(m_mutex);
    return m_tile_list.size();
  }

private:
  std::vector<SharedTilePtr> m_tile_list;
  std::mutex m_mutex;
  std::condition_variable m_cv;
};

#endif
