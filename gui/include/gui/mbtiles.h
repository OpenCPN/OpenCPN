/***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/

#ifndef _CHARTMBTILES_H_
#define _CHARTMBTILES_H_

#include <cstdint>
#include <thread>

#include "chartbase.h"
#include "model/georef.h"  // for GeoRef type
#include "OCPNRegion.h"
#include "ocpn_pixel.h"
#include "viewport.h"
#include "tile_descr.h"
#include "tile_thread.h"
#include "tile_cache.h"

#ifdef ocpnUSE_GL
#include "shaders.h"
#endif

/** The type of a MBTiles chart. */
enum class MbTilesType : std::int8_t {
  BASE,    ///< Base map, providing the primary chart imagery
  OVERLAY  ///< Overlay map, adding additional information on top of a base map
};

/** The tile scheme used in the MBTiles chart. */
enum class MbTilesScheme : std::int8_t {
  XYZ,  ///< "Slippy Map" tilenames. Origin at top-left, Y increases
        /// southward.
  TMS   ///< Tile Map Service scheme. Origin at bottom-left, Y increases
        /// northward.
};

/**
 * Represents an MBTiles format chart.
 *
 * Provides functionality to load, render, and interact with MBTiles format
 * charts. MBTiles stores tiled map data in SQLite databases for efficient
 * storage and retrieval.
 *
 * An MBTiles chart consists of:
 *
 * - A set of image tiles at various zoom levels.
 * - Metadata about the tileset (e.g., attribution, description).
 * - An SQLite database structure for organizing and accessing tiles.
 *
 * Handles tile management, caching, and rendering for efficient
 * display of large tiled datasets, supporting both XYZ and TMS tile schemes.
 */
class ChartMbTiles : public ChartBase {
public:
  ChartMbTiles();
  virtual ~ChartMbTiles();

  //    Accessors

  /**
   * Get the Chart thumbnail data structure,
   * creating the thumbnail bitmap as required
   */
  virtual ThumbData* GetThumbData(int tnx, int tny, float lat, float lon);
  virtual ThumbData* GetThumbData();
  virtual bool UpdateThumbData(double lat, double lon);

  virtual bool AdjustVP(ViewPort& vp_last, ViewPort& vp_proposed);

  int GetNativeScale() { return m_Chart_Scale; }

  /**
   * Report recommended minimum scale values for which use of this
   *  chart is valid
   */
  double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);

  /**
   * Report recommended maximum scale values for which use of this
   *  chart is valid
   */
  double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

  virtual InitReturn Init(const wxString& name, ChartInitFlag init_flags);

  bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
                            const OCPNRegion& Region);

  virtual bool RenderRegionViewOnGL(const wxGLContext& glc,
                                    const ViewPort& vpoint,
                                    const OCPNRegion& rect_region,
                                    const LLRegion& region);

  virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

  virtual void GetValidCanvasRegion(const ViewPort& v_point,
                                    OCPNRegion* valid_region);
  virtual LLRegion GetValidRegion();

  virtual bool GetChartExtent(Extent* pext);

  void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

  double GetPPM() { return m_ppm_avg; }
  double GetZoomFactor() { return m_zoom_scale_factor; }
  MbTilesType GetTileType() { return m_tile_type; }

protected:
  //    Methods
  bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);
  InitReturn PreInit(const wxString& name, ChartInitFlag init_flags,
                     ColorScheme cs);
  InitReturn PostInit(void);

  void PrepareTiles();
  void PrepareTilesForZoom(int zoomFactor, bool bset_geom);

  /**
   * Loads a tile into OpenGL's texture memory for rendering. If the tile
   * is not ready to be rendered (i.e. the tile has not been loaded from
   * disk or ndecompressed to memory), the function sends a request to
   * the worker thread which will do this later in the background
   * @param tile Pointer to the tile descriptor to be prepared
   * @return  true if the tile is ready to be rendered, false else.
   */
  bool GetTileTexture(SharedTilePtr tile);
  void FlushTiles(void);
  bool RenderTile(SharedTilePtr tile, int zoom_level, const ViewPort& vpoint);

  //    Protected Data

  float m_lon_max;
  float m_lon_min;
  float m_lat_max;
  float m_lat_min;

  double m_ppm_avg;  // Calculated true scale factor of the 1X chart,
                     // pixels per meter
  MbTilesType m_tile_type;

  int m_b_cdebug;

  int m_min_zoom;
  int m_max_zoom;
  std::unique_ptr<TileCache> m_tile_cache;
  LLRegion m_min_zoom_region;
  wxBitmapType m_image_type;
  int m_last_clean_zoom;

  double m_zoom_scale_factor;

  MbTilesScheme m_scheme;

  std::shared_ptr<SQLite::Database> m_db;
  int m_n_tiles;
  std::string m_format;

  uint32_t m_tile_count;
  std::unique_ptr<MbtTilesThread> m_worker_thread;
  std::thread m_thread;

#ifdef ocpnUSE_GL
  GLShaderProgram* m_tile_shader_program;
#endif

  /**
   * Create and start the worker thread. This thread is dedicated at
   * loading and decompressing chart tiles into memory, in the background. If
   * for any reason the thread would fail to load, the method return false
   */
  bool StartThread();

  /** Stop and delete the worker thread. Called when OpenCPN is quitting. */
  void StopThread();

private:
  void InitFromTiles(const wxString& name);
  wxPoint2DDouble GetDoublePixFromLL(ViewPort& vp, double lat, double lon);
};

#endif
