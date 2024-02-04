#ifndef _MBTILES_TILEDESCRIPTOR_H_
#define _MBTILES_TILEDESCRIPTOR_H_

#include <cstdint>
#include <unordered_map>
#include <atomic>

#include "chartbase.h"
#include "glChartCanvas.h"

/// @brief Per tile descriptor
class mbTileDescriptor {
public:
  mbTileDescriptor() {
    glTextureName = 0;
    m_bAvailable = true;
    m_bgeomSet = false;
    m_teximage = nullptr;
    m_requested = false;
  }

  mbTileDescriptor(int zoomFactor, int x, int y) {
    glTextureName = 0;
    m_bAvailable = true;
    m_bgeomSet = false;
    m_teximage = nullptr;
    m_requested = false;
    tile_x = x;
    tile_y = y;
    m_zoomLevel = zoomFactor;
  }

  virtual ~mbTileDescriptor() {
    if (m_teximage != nullptr) {
      free(m_teximage);
    }
  }

  int tile_x, tile_y;
  int m_zoomLevel;
  float latmin, lonmin, latmax, lonmax;
  LLBBox box;

  // Set to true if a load request is already pending for this tile
  bool m_requested;
  // Pointer to the decompressed tile image
  std::atomic<unsigned char *>  m_teximage;
  GLuint glTextureName;

  // Set to true if the tile has not been found into the SQL database.
  std::atomic<bool> m_bAvailable;
  bool m_bgeomSet;
};

#endif /* _MBTILES_TILEDESCRIPTOR_H_ */