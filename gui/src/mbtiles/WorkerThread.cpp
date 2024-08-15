
#include <wx/thread.h>
#include "dychart.h"
#include "WorkerThread.hpp"

#ifdef __WXMSW__
void my_translate_mbtile(unsigned int code, _EXCEPTION_POINTERS *ep) {
  throw SE_Exception();
}
#endif

/// @brief Worker thread of the MbTiles chart decoder. It receives requests from
/// the MbTile front-end to load and uncompress tiles from an MbTiles file. Once
/// done, the tile list in memory is updated and a refresh of the map triggered.

/// @brief Request a tile to be loaded by the thread. This method is thread
/// safe.
/// @param tile Pointer to the tile to load
void MbtTilesThread::RequestTile(mbTileDescriptor *tile) {
  tile->m_requested = true;
  m_tileQueue.Push(tile);
}

/// @brief Request the thread to stop/delete itself
void MbtTilesThread::RequestStop() {
  // Set the exit request boolean
  m_exitThread = true;
  // Force worker thread to wake-up
  m_tileQueue.Push(nullptr);
  while (!m_finished) {
  }
}

size_t MbtTilesThread::GetQueueSize() { return m_tileQueue.GetSize(); }

/// @brief Main loop of the worker thread
/// @return Always 0
wxThread::ExitCode MbtTilesThread::Entry() {
#ifdef __MSVC__
  _set_se_translator(my_translate_mbtile);

  //  On Windows, if anything in this thread produces a SEH exception (like
  //  access violation) we handle the exception locally, and simply alow the
  //  thread to exit smoothly with no results. Upstream will notice that nothing
  //  got done, and maybe try again later.

#endif

  mbTileDescriptor *tile;

  do {
    // Wait for the next job
    tile = m_tileQueue.Pop();
    // Only process non null tiles. A null pointer can be sent to force the
    // thread to check for a deletion request
    if (tile != nullptr) {
      LoadTile(tile);
    }
    // Only request a refresh of the display when there is no more tiles in
    // the queue.
    if (m_tileQueue.GetSize() == 0) {
      wxGetApp().GetTopWindow()->GetEventHandler()->CallAfter(
          &MyFrame::RefreshAllCanvas, true);
    }
    // Check if the thread has been requested to be destroyed
  } while ((TestDestroy() == false) && (m_exitThread == false));

  // Since the worker is a detached thread, we need a special mecanism to
  // allow the main thread to wait for its deletion
  m_finished = true;

  return (wxThread::ExitCode)0;
}

/// @brief Load bitmap data of a tile from the MbTiles file to the tile cache
/// @param tile Pointer to the tile to be loaded
void MbtTilesThread::LoadTile(mbTileDescriptor *tile) {
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
        if ((blobWidth != 256) || (blobHeight != 256))
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
      unsigned char *teximage = (unsigned char *)malloc(stride * tex_w * tex_h);
      bool transparent = blobImage.HasAlpha();
      //  *(int*)0 = 0;  // test exception

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
            teximage[j * stride + 3] = blobImage.GetAlpha(j % tex_w, j / tex_w);
          } else {
            teximage[j * stride + 3] = 255;
          }
        }
      }

      // Image buffer will be freed later by the main thread
      tile->m_teximage = teximage;

      return;
    }

  }
#ifdef __MSVC__
  catch (SE_Exception e) {
    wxLogMessage("MbTiles: SE_Exception");
    tile->m_bAvailable = false;
    tile->m_teximage = 0;
  }
#else
  catch (std::exception &e) {
    const char *t = e.what();
    wxLogMessage("mbtiles std::exception: %s", e.what());
  }
#endif

  return;
}
