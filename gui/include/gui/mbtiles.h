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
#include "chartbase.h"
#include "model/georef.h"  // for GeoRef type
#include "OCPNRegion.h"
#include "viewport.h"
#include "tile_descr.h"
#include "worker_thread.h"
#include "tile_cache.h"

enum class MBTilesType : std::int8_t { BASE, OVERLAY };
enum class MBTilesScheme : std::int8_t { XYZ, TMS };

class WXDLLEXPORT ChartMbTiles;


class ViewPort;
class PixelCache;
class ocpnBitmap;
class mbTileDescriptor;

namespace SQLite {
class Database;
}

class GLShaderProgram;
class MbtTilesThread;

class ChartMBTiles : public ChartBase {
public:

  ChartMBTiles();
  virtual ~ChartMBTiles();

  //    Accessors

  /**
   * Get the Chart thumbnail data structure,
   * creating the thumbnail bitmap as required
   */
  virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
  virtual ThumbData *GetThumbData();
  virtual bool UpdateThumbData(double lat, double lon);

  virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);

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

  virtual InitReturn Init(const wxString &name, ChartInitFlag init_flags);

  bool RenderRegionViewOnDC(wxMemoryDC &dc, const ViewPort &VPoint,
                            const OCPNRegion &Region);

  virtual bool RenderRegionViewOnGL(const wxGLContext &glc,
                                    const ViewPort &VPoint,
                                    const OCPNRegion &RectRegion,
                                    const LLRegion &Region);

  virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

  virtual void GetValidCanvasRegion(const ViewPort &VPoint,
                                    OCPNRegion *pValidRegion);
  virtual LLRegion GetValidRegion();

  virtual bool GetChartExtent(Extent *pext);

  void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

  double GetPPM() { return m_ppm_avg; }
  double GetZoomFactor() { return m_zoomScaleFactor; }
  MBTilesType GetTileType() { return m_TileType; }

protected:
  //    Methods
  bool RenderViewOnDC(wxMemoryDC &dc, const ViewPort &VPoint);
  InitReturn PreInit(const wxString &name, ChartInitFlag init_flags,
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
  bool getTileTexture(mbTileDescriptor *tile);
  void FlushTiles(void);
  bool RenderTile(mbTileDescriptor *tile, int zoomLevel,
                  const ViewPort &VPoint);

  //    Protected Data

  float m_LonMax, m_LonMin, m_LatMax, m_LatMin;

  double m_ppm_avg;  // Calculated true scale factor of the 1X chart,
                     // pixels per meter
  MBTilesType m_TileType;

  int m_b_cdebug;

  int m_minZoom, m_maxZoom;
  TileCache *m_tileCache;
  LLRegion m_minZoomRegion;
  wxBitmapType m_imageType;
  int m_last_clean_zoom;

  double m_zoomScaleFactor;

  MBTilesScheme m_Scheme;

  SQLite::Database *m_pDB;
  int m_nTiles;
  std::string m_format;

  GLShaderProgram *m_tile_shader_program;
  uint32_t m_tileCount;
  MbtTilesThread *m_workerThread;

  /**
   * Create and start the worker thread. This thread is dedicated at
   * loading and decompressing chart tiles into memory, in the background. If
   * for any reason the thread would fail to load, the method return false
   */
  bool StartThread();

  /** Stop and delete the worker thread. Called when OpenCPN is quitting. */
  void StopThread();

private:
  void InitFromTiles(const wxString &name);
  wxPoint2DDouble GetDoublePixFromLL(ViewPort &vp, double lat, double lon);
};

#endif