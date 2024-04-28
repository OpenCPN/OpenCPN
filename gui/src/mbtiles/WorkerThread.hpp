
#ifndef _MBTILESTHREAD_H_
#define _MBTILESTHREAD_H_

#include <wx/event.h>
#include <wx/thread.h>
#include <wx/mstream.h>

#include "chartbase.h"
#include "ocpn_frame.h"

#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include "TileQueue.hpp"

// Pointer to the ChartCanvas object owning our MbTiles ChartBase object.
// Since ChartBase class does not have a pointer to the parent widget, we have
// to use a direct link to the chart canvas object to force a refresh of the
// widget when a new tile has been loaded
extern ChartCanvas *g_focusCanvas;

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
        m_condition(m_mutex),
        m_pDB(pDB) {}

  virtual ~MbtTilesThread() {}

  /// @brief Request a tile to be loaded by the thread. This method is thread
  /// safe.
  /// @param tile Pointer to the tile to load
  void RequestTile(mbTileDescriptor *tile) {
    tile->m_requested = true;
    m_tileQueue.Push(tile);
  }

  /// @brief Request the thread to stop/delete itself
  void RequestStop() {
    // Set the exit request boolean
    m_exitThread = true;
    // Force worker thread to wake-up
    m_tileQueue.Push(nullptr);
    // Wait for the thread to be deleted
    m_mutex.Lock();
    if (!m_finished) m_condition.Wait();
    m_mutex.Unlock();
  }

private:
  // Set to true to tell the main loop to stop execution
  bool m_exitThread;
  // Set to true when the thread has finished
  bool m_finished;
  wxMutex m_mutex;
  wxCondition m_condition;
  // The queue storing all the tile requests
  TileQueue m_tileQueue;
  // Pointer the SQL object managing the MbTiles file
  SQLite::Database *m_pDB;

  /// @brief Main loop of the worker thread
  /// @return Always 0
  virtual ExitCode Entry() {
    mbTileDescriptor *tile;

    do {
      // Wait for the next job
      tile = m_tileQueue.Pop();
      // Only process non null tiles. A null pointer can be sent to force the
      // thread to check for a deletion request
      if (tile != nullptr) {
        LoadTile(tile);
        // Only request a refresh of the display if there is no more tiles in
        // the queue. This avoid annoying flickering effects on the screen when
        // the computer is fast and able to redraw very quickly
        if (m_tileQueue.GetSize() == 0) {
          g_focusCanvas->Refresh();
        }
      }
      // Check if the thread has been requested to be destroyed
    } while (TestDestroy() == false && m_exitThread == false);

    // Since the worker is a detached thread, we nee a special mecanism to allow
    // the main thread to wait for its deletion
    m_mutex.Lock();
    m_finished = true;
    m_condition.Signal();
    m_mutex.Unlock();

    return (wxThread::ExitCode)0;
  }

  /// @brief Load bitmap data of a tile from the MbTiles file to the tile cache
  /// @param tile Pointer to the tile to be loaded
  void LoadTile(mbTileDescriptor *tile) {
    // If the tile has not been found in the SQL database in a previous attempt,
    // don't search for it again
    if (!tile->m_bAvailable) return;

    // If the tile has already been uncompressed, don't uncompress it
    // again
    if (tile->m_teximage != nullptr) return;
    if (tile->glTextureName > 0) return;

    // Fetch the tile data from the mbtile database
    try {
      char qrs[2100];

      // Compile a SQL query, getting the specific blob
      sprintf(qrs,
              "select tile_data, length(tile_data) from tiles where zoom_level "
              "= %d AND tile_column=%d AND tile_row=%d",
              tile->m_zoomLevel, tile->tile_x, tile->tile_y);
      SQLite::Statement query(*m_pDB, qrs);

      int queryResult = query.tryExecuteStep();
      if (SQLITE_DONE == queryResult) {
        // The tile has not been found in databse, mark it as "not available" so
        // that we won't try to find it again later
        tile->m_bAvailable = false;
        return;
      } else {
        // Get the blob
        SQLite::Column blobColumn = query.getColumn(0);
        const void *blob = blobColumn.getBlob();
        // Get the length
        int length = query.getColumn(1);

        // Uncompress the tile
        wxMemoryInputStream blobStream(blob, length);
        wxImage blobImage;
        blobImage = wxImage(blobStream, wxBITMAP_TYPE_ANY);
        int blobWidth, blobHeight;
        unsigned char *imgdata;

        // Check that the tile is OK and rescale it to 256x256 if necessary
        if (blobImage.IsOk()) {
          blobWidth = blobImage.GetWidth();
          blobHeight = blobImage.GetHeight();
          // Support MapTiler HiDPI tiles, 512x512
          if (blobWidth != 256 || blobHeight != 256)
            blobImage.Rescale(256, 256, wxIMAGE_QUALITY_NORMAL);
          imgdata = blobImage.GetData();
        } else {
          // wxWidget can't uncompress the tile : mark it as not available and
          // exit
          tile->m_bAvailable = false;
          return;
        }

        if (!imgdata) {
          // wxWidget can't uncompress the tile : mark it as not available and
          // exit
          tile->m_bAvailable = false;
          return;
        }

        int stride = 4;
        int tex_w = 256;
        int tex_h = 256;
        // Copy and process the tile
        unsigned char *teximage =
            (unsigned char *)malloc(stride * tex_w * tex_h);
        bool transparent = blobImage.HasAlpha();

        for (int j = 0; j < tex_w * tex_h; j++) {
          for (int k = 0; k < 3; k++)
            teximage[j * stride + k] = imgdata[3 * j + k];

          // Some NOAA Tilesets do not give transparent tiles, so we detect
          // NOAA's idea of blank as RGB(1,0,0) and force  alpha = 0;
          if (imgdata[3 * j] == 1 && imgdata[3 * j + 1] == 0 &&
              imgdata[3 * j + 2] == 0) {
            teximage[j * stride + 3] = 0;
          } else {
            if (transparent) {
              teximage[j * stride + 3] =
                  blobImage.GetAlpha(j % tex_w, j / tex_w);
            } else {
              teximage[j * stride + 3] = 255;
            }
          }
        }

        // Image buffer will be freed later by the main thread
        tile->m_teximage = teximage;

        return;
      }

    } catch (std::exception &e) {
      const char *t = e.what();
      wxLogMessage("mbtiles exception: %s", e.what());
    }
  }
};

#endif /* _MBTILESTHREAD_H_ */
