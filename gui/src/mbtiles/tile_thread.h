
#ifndef _MBTILESTHREAD_H_
#define _MBTILESTHREAD_H_

#include <thread>

#include <wx/event.h>
#include <wx/mstream.h>

#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include "chartbase.h"
#include "ocpn_frame.h"
#include "ocpn_app.h"
#include "tile_queue.h"

#ifdef __WXMSW__
class SE_Exception {
private:
  unsigned int nSE;

public:
  SE_Exception() {}
  SE_Exception(unsigned int n) : nSE(n) {}
  ~SE_Exception() {}
  unsigned int getSeNumber() { return nSE; }
};

#endif

/**
 *  MbTiles chart decoder worker thread. Receives requests from
 *  the MbTile front-end to load and uncompress tiles from an MbTiles file. Once
 *  done, the tile list in memory is updated and a refresh of the map triggered.
 */
class MbtTilesThread {
public:
  /**
   * Create worker thread instance.
   * @param pDB Pointer to SQL database handler.
   */
  MbtTilesThread(std::shared_ptr<SQLite::Database> db)
      : m_exit_thread(false), m_finished(false), m_db(db) {}

  virtual ~MbtTilesThread() {}

  /**
   * Request a tile to be loaded by the thread. This method is thread
   * safe.
   * @param tile Pointer to tile to load
   */
  void RequestTile(SharedTilePtr tile);

  /** Request the thread to stop/delete itself. */
  void RequestStop();

  /** Return number of tiles in worker thread queue. */
  size_t GetQueueSize();

  /**  Worker thread main loop. */
  virtual void Run();

private:
  /// Set to true to tell the main loop to stop execution
  bool m_exit_thread;

  /// Set to true when the thread has finished
  bool m_finished;

  /// The queue storing all the tile requests
  TileQueue m_tile_queue;

  /// Pointer to SQL object managing the MbTiles file
  std::shared_ptr<SQLite::Database> m_db;

  /**
   * Load bitmap data of a tile from the MbTiles file to the tile cache
   * @param tile Pointer to the tile to be loaded
   */
  void LoadTile(SharedTilePtr tile);
};

#endif /* _MBTILESTHREAD_H_ */
