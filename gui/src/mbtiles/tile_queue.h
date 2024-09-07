#ifndef _MBTILESTILEQUEUE_H_
#define _MBTILESTILEQUEUE_H_

#include <vector>

#include <wx/event.h>
#include <wx/thread.h>

#include "tile_descr.h"

/** A thread safe tile queue between two threads. */
class TileQueue {
public:
  TileQueue() {}

  /**
   *  Push a tile to the queue.
   *  @param tile Pointer to tile descriptor to be pushed.
   */
  void Push(MbTileDescriptor *tile) {
    wxMutexLocker lock(m_mutex);
    m_tile_list.push_back(tile);
    m_tile_counter.Post();
  }

  /**
  *  Retreive a tile from the queue. If there is no tile in the queue,
  *  calling thread is blocked until a tile is available.
  *
  *  @return Pointer to tile descriptor
  */
  MbTileDescriptor *Pop() {
    m_tile_counter.Wait();
    wxMutexLocker lock(m_mutex);
    MbTileDescriptor *tile = m_tile_list.at(0);
    m_tile_list.erase(m_tile_list.cbegin());
    return tile;
  }

  /**  Retrieve current size of queue. */
  uint32_t GetSize() {
    uint32_t size;
    wxMutexLocker lock(m_mutex);

    size = m_tile_list.size();

    return size;
  }

private:
  std::vector<MbTileDescriptor *> m_tile_list;
  wxMutex m_mutex;
  wxSemaphore m_tile_counter;
};

#endif
