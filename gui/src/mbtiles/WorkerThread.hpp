
#ifndef _MBTILESTHREAD_H_
#define _MBTILESTHREAD_H_

#include <wx/event.h>
#include <wx/thread.h>
#include <wx/mstream.h>

#include "mbtiles.h"
#include "chartbase.h"
#include "ocpn_frame.h"
#include "ocpn_app.h"

#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include "TileQueue.hpp"

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


/// @brief Worker thread of the MbTiles chart decoder. It receives requests from
/// the MbTile front-end to load and uncompress tiles from an MbTiles file. Once
/// done, the tile list in memory is updated and a refresh of the map triggered.
class MbtTilesThread : public wxThread {
public:
  /// @brief Create the instance of the worker thread
  /// @param pDB Pointer to the SQL database handler
  MbtTilesThread(SQLite::Database *pDB)
      : wxThread(wxTHREAD_DETACHED),
        m_exitThread(false),
        m_finished(false),
        m_pDB(pDB) {}

  virtual ~MbtTilesThread() {}

  /// @brief Request a tile to be loaded by the thread. This method is thread
  /// safe.
  /// @param tile Pointer to the tile to load
  void RequestTile(mbTileDescriptor *tile);

  /// @brief Request the thread to stop/delete itself
  void RequestStop();
  size_t GetQueueSize();

private:
  // Set to true to tell the main loop to stop execution
  bool m_exitThread;
  // Set to true when the thread has finished
  bool m_finished;
  // The queue storing all the tile requests
  TileQueue m_tileQueue;
  // Pointer the SQL object managing the MbTiles file
  SQLite::Database *m_pDB;

  /// @brief Main loop of the worker thread
  /// @return Always 0
  virtual ExitCode Entry();

  /// @brief Load bitmap data of a tile from the MbTiles file to the tile cache
  /// @param tile Pointer to the tile to be loaded
  void LoadTile(mbTileDescriptor *tile);

};

#endif /* _MBTILESTHREAD_H_ */
