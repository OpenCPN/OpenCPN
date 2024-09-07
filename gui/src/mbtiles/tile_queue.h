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
  void Push(mbTileDescriptor *tile) {
    wxMutexLocker lock(m_queueMutex);
    m_tileList.push_back(tile);
    m_tileCounter.Post();
  }

  /**
  *  Retreive a tile from the queue. If there is no tile in the queue,
  *  calling thread is blocked until a tile is available.
  *
  *  @return Pointer to tile descriptor
  */
  mbTileDescriptor *Pop() {
    m_tileCounter.Wait();
    wxMutexLocker lock(m_queueMutex);
    mbTileDescriptor *tile = m_tileList.at(0);
    m_tileList.erase(m_tileList.cbegin());
    return tile;
  }

  /**  Retrieve current size of queue. */
  uint32_t GetSize() {
    uint32_t size;
    wxMutexLocker lock(m_queueMutex);

    size = m_tileList.size();

    return size;
  }

private:
  std::vector<mbTileDescriptor *> m_tileList;
  wxMutex m_queueMutex;
  wxSemaphore m_tileCounter;
};

#endif
