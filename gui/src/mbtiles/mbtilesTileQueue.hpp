#ifndef _MBTILESTILEQUEUE_H_
#define _MBTILESTILEQUEUE_H_

#include <vector>
#include <wx/event.h>
#include <wx/thread.h>

#include "mbtilesTileDescriptor.hpp"

/// @brief Class implementing a thread safe tile queue between two thread
class TileQueue {
public:
  TileQueue() {}

  /// @brief Push a tile to the queue. This method is thread safe
  /// @param tile Pointer to the tile descriptor to be pushed.
  void Push(mbTileDescriptor *tile) {
    wxMutexLocker lock(m_queueMutex);
    m_tileList.push_back(tile);
    m_tileCounter.Post();
  }

  /// @brief Retreives a tile from the queue. If there is not tile in the queue,
  /// the calling thread is blocked until a tile is pushed to the queue. This
  /// method is thread safe.
  /// @return A pointer to a tile descriptor
  mbTileDescriptor *Pop() {
    mbTileDescriptor *tile;
    m_tileCounter.Wait();
    m_queueMutex.Lock();
    tile = m_tileList.at(0);
    m_tileList.erase(m_tileList.cbegin());
    m_queueMutex.Unlock();
    return tile;
  }

private:
  std::vector<mbTileDescriptor *> m_tileList;
  wxMutex m_queueMutex;
  wxSemaphore m_tileCounter;
};

#endif