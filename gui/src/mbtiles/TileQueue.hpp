#ifndef _MBTILESTILEQUEUE_H_
#define _MBTILESTILEQUEUE_H_

#include <vector>
#include <wx/event.h>
#include <wx/thread.h>

#include "TileDescriptor.hpp"

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
    m_tileCounter.Wait();
    wxMutexLocker lock(m_queueMutex);
    mbTileDescriptor *tile = m_tileList.at(0);
    m_tileList.erase(m_tileList.cbegin());
    return tile;
  }

  /// @brief Get current size of the queue.
  /// @return Queue size in tiles
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