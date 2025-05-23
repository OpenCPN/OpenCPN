
/**************************************************************************
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************/

#include <map>
#include <sstream>
#include <unordered_map>

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

//  Why are these not in wx/prec.h?
#include <wx/dir.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/fileconf.h>
#include <wx/mstream.h>
#include <sys/stat.h>

#include <sqlite3.h>  //We need some defines
#include <SQLiteCpp/SQLiteCpp.h>

#include "chcanv.h"
#include "glChartCanvas.h"
#include "ocpn_frame.h"
#ifdef ocpnUSE_GL
#include "shaders.h"
#endif
#include "mbtiles.h"
#include "model/config_vars.h"

//  Missing from MSW include files
#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

// ----------------------------------------------------------------------------
// Random Prototypes
// ----------------------------------------------------------------------------

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

#ifdef OCPN_USE_CONFIG
#include "navutil.h"
extern MyConfig* pConfig;
#endif

/// Raster Zoom Modifier value from advanced display preference pane
/// Physically located in ocpn_app.cpp
extern int g_chart_zoom_modifier_raster;

#define LON_UNDEF NAN
#define LAT_UNDEF NAN

// A "nominal" scale value, by zoom factor.  Estimated at equator, with monitor
// pixel size of 0.3mm
static double osm_zoom_scale[22];

//  Meters per pixel, by zoom factor
static double osm_zoom_mpp[22];

extern MyFrame* gFrame;

// Private tile shader source
static const GLchar* tile_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar* tile_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "uniform float brightness;\n"
    "void main() {\n"
    "   vec4 textureColor = texture2D(uTex, varCoord);\n"
    "   gl_FragColor = vec4(textureColor.rgb * brightness, textureColor.w);\n"
    "}\n";

#ifdef ocpnUSE_GL
GLShaderProgram* g_tile_shader_program;
#endif

#if defined(__UNIX__) && \
    !defined(__WXOSX__)  // high resolution stopwatch for profiling
class OCPNStopWatch {
public:
  OCPNStopWatch() { Reset(); }
  void Reset() { clock_gettime(CLOCK_REALTIME, &tp); }

  double GetTime() {
    timespec tp_end;
    clock_gettime(CLOCK_REALTIME, &tp_end);
    return (tp_end.tv_sec - tp.tv_sec) * 1.e3 +
           (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
  }

private:
  timespec tp;
};
#endif

ChartMbTiles::ChartMbTiles() {
  // Compute scale & MPP for each zoom level
  // Initial constants taken from
  // https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
  osm_zoom_scale[0] = 554680041;
  osm_zoom_mpp[0] = 156543.03;
  for (int i = 1; i < (int)(sizeof(osm_zoom_scale) / sizeof(double)); i++) {
    osm_zoom_scale[i] = osm_zoom_scale[i - 1] / 2;
    osm_zoom_mpp[i] = osm_zoom_mpp[i - 1] / 2;
  }

  //    Init some private data
  m_worker_thread = nullptr;
  m_ChartFamily = CHART_FAMILY_RASTER;
  m_ChartType = CHART_TYPE_MBTILES;

  m_Chart_Skew = 0.0;

  m_datum_str = _T("WGS84");  // assume until proven otherwise
  m_projection = PROJECTION_WEB_MERCATOR;
  m_image_type = wxBITMAP_TYPE_ANY;

  m_b_cdebug = 0;

  m_min_zoom = 0;
  m_max_zoom = 21;

  m_nNoCOVREntries = 0;
  m_nCOVREntries = 0;
  m_pCOVRTablePoints = NULL;
  m_pCOVRTable = NULL;
  m_pNoCOVRTablePoints = NULL;
  m_pNoCOVRTable = NULL;
  m_tile_cache = NULL;

  m_lon_min = LON_UNDEF;
  m_lon_max = LON_UNDEF;
  m_lat_min = LAT_UNDEF;
  m_lat_max = LAT_UNDEF;

#ifdef OCPN_USE_CONFIG
  wxFileConfig* pfc = (wxFileConfig*)pConfig;
  pfc->SetPath(_T ( "/Settings" ));
  pfc->Read(_T ( "DebugMBTiles" ), &m_b_cdebug, 0);
#endif
  m_db = NULL;
}

ChartMbTiles::~ChartMbTiles() {
  // Stop the worker thread before destroying this instance
  StopThread();
  FlushTiles();
}

ThumbData* ChartMbTiles::GetThumbData() { return NULL; }

ThumbData* ChartMbTiles::GetThumbData(int tnx, int tny, float lat, float lon) {
  return NULL;
}

bool ChartMbTiles::UpdateThumbData(double lat, double lon) { return true; }

bool ChartMbTiles::AdjustVP(ViewPort& vp_last, ViewPort& vp_proposed) {
  return true;
}

double ChartMbTiles::GetNormalScaleMin(double canvas_scale_factor,
                                       bool b_allow_overzoom) {
  return (1);  // allow essentially unlimited overzoom
}

double ChartMbTiles::GetNormalScaleMax(double canvas_scale_factor,
                                       int canvas_width) {
  return (canvas_scale_factor / m_ppm_avg) *
         40.0;  // excessive underscale is slow, and unreadable
}

double ChartMbTiles::GetNearestPreferredScalePPM(double target_scale_ppm) {
  return target_scale_ppm;
}

// Checks/corrects/completes the initialization based on real data from the
// tiles table
void ChartMbTiles::InitFromTiles(const wxString& name) {
  try {
    // Open the MBTiles database file
    const char* name_utf8 = "";
    wxCharBuffer utf8CB = name.ToUTF8();  // the UTF-8 buffer
    if (utf8CB.data()) name_utf8 = utf8CB.data();

    SQLite::Database db(name_utf8);

    // Check if tiles with advertised min and max zoom level really exist, or
    // correct the defaults We can't blindly use what we find though - the DB
    // often contains empty cells at very low zoom levels, so if we have some
    // info from metadata, we will use that if more conservative...
    SQLite::Statement query(db,
                            "SELECT min(zoom_level) AS m_min_zoom, "
                            "max(zoom_level) AS m_max_zoom FROM tiles");
    while (query.executeStep()) {
      const char* col_min_zoom = query.getColumn(0);
      const char* col_max_zoom = query.getColumn(1);

      int min_zoom = 0, max_zoom = 0;
      sscanf(col_min_zoom, "%i", &min_zoom);
      m_min_zoom = wxMax(m_min_zoom, min_zoom);
      sscanf(col_max_zoom, "%i", &max_zoom);
      m_max_zoom = wxMin(m_max_zoom, max_zoom);
      if (m_min_zoom > m_max_zoom) {
        // We are looking at total nonsense with wrong metatadata and actual
        // tile coverage out of it, better use what's really in the data to be
        // able to show at least something
        m_min_zoom = min_zoom;
        m_max_zoom = max_zoom;
      }
    }

    //        std::cout << name.c_str() << " zoom_min: " << m_min_zoom << "
    //        zoom_max: " << m_max_zoom << std::endl;

    // Traversing the entire tile table can be expensive....
    //  Use declared bounds if present.

    if (!std::isnan(m_lat_min) && !std::isnan(m_lat_max) &&
        !std::isnan(m_lon_min) && !std::isnan(m_lon_max))
      return;

    // Try to guess the coverage extents from the tiles. This will be hard to
    // get right -
    // the finest resolution likely does not cover the whole area, while the
    // lowest resolution tiles probably contain a lot of theoretical space which
    // actually is not covered. And some resolutions may be actually missing...
    // What do we use?
    // If we have the metadata and it is not completely off, we should probably
    // prefer it.
    SQLite::Statement query1(
        db,
        wxString::Format(
            "SELECT min(tile_row) AS min_row, max(tile_row) as max_row, "
            "min(tile_column) as min_column, max(tile_column) as max_column, "
            "count(*) as cnt, zoom_level FROM tiles  WHERE zoom_level >= %d "
            "AND zoom_level <= %d GROUP BY zoom_level ORDER BY zoom_level ASC",
            m_min_zoom, m_max_zoom)
            .c_str());
    float min_lat = 999., max_lat = -999.0, min_lon = 999., max_lon = -999.0;
    while (query1.executeStep()) {
      const char* colMinRow = query1.getColumn(0);
      const char* colMaxRow = query1.getColumn(1);
      const char* colMinCol = query1.getColumn(2);
      const char* colMaxCol = query1.getColumn(3);
      const char* colCnt = query1.getColumn(4);
      const char* colZoom = query1.getColumn(5);

      int minRow, maxRow, minCol, maxCol, cnt, zoom;
      sscanf(colMinRow, "%i", &minRow);
      sscanf(colMaxRow, "%i", &maxRow);
      sscanf(colMinCol, "%i", &minCol);
      sscanf(colMaxCol, "%i", &maxCol);
      sscanf(colMinRow, "%i", &minRow);
      sscanf(colMaxRow, "%i", &maxRow);
      sscanf(colCnt, "%i", &cnt);
      sscanf(colZoom, "%i", &zoom);

      // Let's try to use the simplest possible algo and just look for the zoom
      // level with largest extent (Which probably be the one with lowest
      // resolution?)...
      min_lat = wxMin(min_lat, MbTileDescriptor::Tiley2lat(minRow, zoom));
      max_lat = wxMax(max_lat, MbTileDescriptor::Tiley2lat(maxRow - 1, zoom));
      min_lon = wxMin(min_lon, MbTileDescriptor::Tilex2long(minCol, zoom));
      max_lon = wxMax(max_lon, MbTileDescriptor::Tilex2long(maxCol + 1, zoom));
      // std::cout << "Zoom: " << zoom << " minlat: " << Tiley2lat(minRow, zoom)
      // << " maxlat: " << Tiley2lat(maxRow - 1, zoom) << " minlon: " <<
      // tilex2long(minCol, zoom) << " maxlon: " << Tilex2long(maxCol + 1, zoom)
      // << std::endl;
    }

    // ... and use what we found only in case we miss some of the values from
    // metadata...
    if (std::isnan(m_lat_min)) m_lat_min = min_lat;
    if (std::isnan(m_lat_max)) m_lat_max = max_lat;
    if (std::isnan(m_lon_min)) m_lon_min = min_lon;
    if (std::isnan(m_lon_max)) m_lon_max = max_lon;
  } catch (std::exception& e) {
    const char* t = e.what();
    wxLogMessage("mbtiles exception: %s", e.what());
  }
}

InitReturn ChartMbTiles::Init(const wxString& name, ChartInitFlag init_flags) {
  m_global_color_scheme = GLOBAL_COLOR_SCHEME_RGB;

  m_FullPath = name;
  m_Description = m_FullPath;

  try {
    // Open the MBTiles database file
    const char* name_utf8 = "";
    wxCharBuffer utf8CB = name.ToUTF8();  // the UTF-8 buffer
    if (utf8CB.data()) name_utf8 = utf8CB.data();

    SQLite::Database db(name_utf8);

    // Compile a SQL query, getting everything from the "metadata" table
    SQLite::Statement query(db, "SELECT * FROM metadata ");

    // Loop to execute the query step by step, to get rows of result
    while (query.executeStep()) {
      const char* col_name = query.getColumn(0);
      const char* col_value = query.getColumn(1);

      // Get the geometric extent of the data
      if (!strncmp(col_name, "bounds", 6)) {
        float lon1, lat1, lon2, lat2;
        sscanf(col_value, "%g,%g,%g,%g", &lon1, &lat1, &lon2, &lat2);

        // There is some confusion over the layout of this field...
        m_lat_max = wxMax(lat1, lat2);
        m_lat_min = wxMin(lat1, lat2);
        m_lon_max = wxMax(lon1, lon2);
        m_lon_min = wxMin(lon1, lon2);

      }

      else if (!strncmp(col_name, "format", 6)) {
        m_format = std::string(col_value);
      }

      // Get the min and max zoom values present in the db
      else if (!strncmp(col_name, "minzoom", 7)) {
        sscanf(col_value, "%i", &m_min_zoom);
      } else if (!strncmp(col_name, "maxzoom", 7)) {
        sscanf(col_value, "%i", &m_max_zoom);
      }

      else if (!strncmp(col_name, "description", 11)) {
        m_Description = wxString(col_value, wxConvUTF8);
      } else if (!strncmp(col_name, "name", 11)) {
        m_Name = wxString(col_value, wxConvUTF8);
      } else if (!strncmp(col_name, "type", 11)) {
        m_tile_type =
            wxString(col_value, wxConvUTF8).Upper().IsSameAs("OVERLAY")
                ? MbTilesType::OVERLAY
                : MbTilesType::BASE;
      } else if (!strncmp(col_name, "scheme", 11)) {
        m_scheme = wxString(col_value, wxConvUTF8).Upper().IsSameAs("XYZ")
                       ? MbTilesScheme::XYZ
                       : MbTilesScheme::TMS;
      }
    }
  } catch (std::exception& e) {
    const char* t = e.what();
    wxLogMessage("mbtiles exception: %s", e.what());
    return INIT_FAIL_REMOVE;
  }

  // Fix the missing/wrong metadata values
  InitFromTiles(name);

  // set the chart scale parameters based on the max zoom factor
  m_ppm_avg = 1.0 / osm_zoom_mpp[m_min_zoom];
  m_Chart_Scale = osm_zoom_scale[m_max_zoom];

  // Initialize the tile data structures
  m_tile_cache = std::make_unique<TileCache>(m_min_zoom, m_max_zoom, m_lon_min,
                                             m_lat_min, m_lon_max, m_lat_max);

  LLRegion covr_region;

  LLBBox extent_box;
  extent_box.Set(m_lat_min, m_lon_min, m_lat_max, m_lon_max);

  const char* name_utf8 = "";
  wxCharBuffer utf8CB = name.ToUTF8();  // the UTF-8 buffer
  if (utf8CB.data()) name_utf8 = utf8CB.data();

  try {
    SQLite::Database db(name_utf8);

    int zoom_factor = m_min_zoom;
    int min_region_zoom = -1;
    bool covr_populated = false;

    m_n_tiles = 0;
    while ((zoom_factor <= m_max_zoom) && (min_region_zoom < 0)) {
      LLRegion covr_region_zoom;
      wxRegion region_zoom;
      char qrs[100];

      // Protect against trying to create the exact coverage for the brutal
      // large scale layers contianing tens of thousand tiles.
      sprintf(qrs, "select count(*) from tiles where zoom_level = %d ",
              zoom_factor);
      SQLite::Statement query_size(db, qrs);

      if (query_size.executeStep()) {
        const char* col_value = query_size.getColumn(0);
        int tile_at_zoom = atoi(col_value);
        m_n_tiles += tile_at_zoom;

        if (tile_at_zoom > 1000) {
          zoom_factor++;
          if (!covr_populated) {
            covr_populated = true;
            covr_region = extent_box;
          }
          continue;
        }
      }

      // query the database
      sprintf(qrs,
              "select tile_column, tile_row from tiles where zoom_level = %d ",
              zoom_factor);

      // Compile a SQL query, getting the specific  data
      SQLite::Statement query(db, qrs);
      covr_populated = true;

      while (query.executeStep()) {
        const char* col_value = query.getColumn(0);
        const char* c2 = query.getColumn(1);
        int tile_x_found = atoi(col_value);  // m_tile_x
        int tile_y_found = atoi(c2);         // m_tile_y

        region_zoom.Union(tile_x_found, tile_y_found - 1, 1, 1);

      }  // inner while

      wxRegionIterator upd(region_zoom);  // get the  rect list
      double eps_factor = kEps * 100;     // roughly 1 m

      while (upd) {
        wxRect rect = upd.GetRect();

        double lonmin =
            round(MbTileDescriptor::Tilex2long(rect.x, zoom_factor) /
                  eps_factor) *
            eps_factor;
        double lonmax = round(MbTileDescriptor::Tilex2long(rect.x + rect.width,
                                                           zoom_factor) /
                              eps_factor) *
                        eps_factor;
        double latmin = round(MbTileDescriptor::Tiley2lat(rect.y, zoom_factor) /
                              eps_factor) *
                        eps_factor;
        double latmax = round(MbTileDescriptor::Tiley2lat(rect.y + rect.height,
                                                          zoom_factor) /
                              eps_factor) *
                        eps_factor;

        LLBBox box;
        box.Set(latmin, lonmin, latmax, lonmax);

        LLRegion tile_region(box);
        // if(i <= 1)
        covr_region_zoom.Union(tile_region);

        upd++;
        min_region_zoom =
            zoom_factor;  // We take the first populated (lowest) zoom
                          // level region as the final chart region
      }

      covr_region.Union(covr_region_zoom);

      zoom_factor++;

    }  // while
  } catch (std::exception& e) {
    const char* t = e.what();
    wxLogMessage("mbtiles exception: %s", e.what());
    return INIT_FAIL_REMOVE;
  }

  //  The coverage region must be reduced if necessary to include only the db
  //  specified bounds.
  covr_region.Intersect(extent_box);

  m_min_zoom_region = covr_region;

  //  Populate M_COVR entries for the OCPN chart database
  if (covr_region.contours.size()) {  // Check for no intersection caused by ??
    m_nCOVREntries = covr_region.contours.size();
    m_pCOVRTablePoints = (int*)malloc(m_nCOVREntries * sizeof(int));
    m_pCOVRTable = (float**)malloc(m_nCOVREntries * sizeof(float*));
    std::list<poly_contour>::iterator it = covr_region.contours.begin();
    for (int i = 0; i < m_nCOVREntries; i++) {
      m_pCOVRTablePoints[i] = it->size();
      m_pCOVRTable[i] =
          (float*)malloc(m_pCOVRTablePoints[i] * 2 * sizeof(float));
      std::list<contour_pt>::iterator jt = it->begin();
      for (int j = 0; j < m_pCOVRTablePoints[i]; j++) {
        m_pCOVRTable[i][2 * j + 0] = jt->y;
        m_pCOVRTable[i][2 * j + 1] = jt->x;
        jt++;
      }
      it++;
    }
  }

  if (init_flags == HEADER_ONLY) return INIT_OK;

  InitReturn pi_ret = PostInit();

  // Start the worker thread. Be careful that the thread must be started after
  // PostInit() to ensure that SQL database has been opened.
  bool thread_return = StartThread();
  if (!thread_return) return INIT_FAIL_RETRY;

  if (pi_ret != INIT_OK)
    return pi_ret;
  else
    return INIT_OK;
}

InitReturn ChartMbTiles::PreInit(const wxString& name, ChartInitFlag init_flags,
                                 ColorScheme cs) {
  m_global_color_scheme = cs;
  return INIT_OK;
}

InitReturn ChartMbTiles::PostInit(void) {
  // Create the persistent MBTiles database file
  const char* name_UTF8 = "";
  wxCharBuffer utf8CB = m_FullPath.ToUTF8();  // the UTF-8 buffer
  if (utf8CB.data()) name_UTF8 = utf8CB.data();

  try {
    m_db = std::make_unique<SQLite::Database>(name_UTF8);
    // m_db->exec("PRAGMA locking_mode=EXCLUSIVE");
    m_db->exec("PRAGMA cache_size=-10000");
  } catch (std::exception& e) {
    const char* t = e.what();
    wxLogMessage("mbtiles exception: %s", e.what());
    return INIT_FAIL_REMOVE;
  }

  bReadyToRender = true;
  return INIT_OK;
}

// FIXME : useless now
void ChartMbTiles::FlushTiles() {
  // Delete all the tiles in the tile cache
  // Note that this function also deletes OpenGL texture memory associated to
  // the tiles
  if (m_tile_cache) {
    m_tile_cache->Flush();
  }
}

bool ChartMbTiles::GetChartExtent(Extent* pext) {
  pext->NLAT = m_lat_max;
  pext->SLAT = m_lat_min;
  pext->ELON = m_lon_max;
  pext->WLON = m_lon_min;
  return true;
}

void ChartMbTiles::SetColorScheme(ColorScheme cs, bool bApplyImmediate) {
  m_global_color_scheme = cs;
}

void ChartMbTiles::GetValidCanvasRegion(const ViewPort& v_point,
                                        OCPNRegion* valid_region) {
  valid_region->Clear();
  valid_region->Union(0, 0, v_point.pix_width, v_point.pix_height);
  return;
}

LLRegion ChartMbTiles::GetValidRegion() { return m_min_zoom_region; }

bool ChartMbTiles::RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint) {
  return true;
}

bool ChartMbTiles::GetTileTexture(SharedTilePtr tile) {
  if (!m_db) return false;
  m_tile_count++;
  // Is the texture ready to be rendered ?
  if (tile->m_gl_texture_name > 0) {
    // Yes : bind the texture and return to the caller
    glBindTexture(GL_TEXTURE_2D, tile->m_gl_texture_name);
    return true;
  } else if (!tile->m_is_available) {
    // Tile is not in MbTiles file : no texture to render
    return false;
  } else if (tile->m_teximage == 0) {
    // Throttle the worker thread
    // Avoids unmanaged spikes in memory usage that arise from
    // processing very large tiles at small display scale (issue #4043)
    if (m_worker_thread->GetQueueSize() < 500) {
      if (tile->m_requested == false) {
        // The tile has not been loaded and decompressed yet : request it
        // to the worker thread
        m_worker_thread->RequestTile(tile);
      }
    }
    return false;
  } else {
    // The tile has been decompressed to memory : load it into OpenGL texture
    // memory
#ifndef __OCPN__ANDROID__
    glEnable(GL_COLOR_MATERIAL);
#endif
    glGenTextures(1, &tile->m_gl_texture_name);
    glBindTexture(GL_TEXTURE_2D, tile->m_gl_texture_name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, tile->m_teximage);
    // The tile is loaded into OpenGL memory : we can free the memory of the
    // decompressed tile
    free(tile->m_teximage);
    tile->m_teximage = nullptr;

    return true;
  }

  return false;
}

class wxPoint2DDouble;

wxPoint2DDouble ChartMbTiles::GetDoublePixFromLL(ViewPort& vp, double lat,
                                                 double lon) {
  double easting = 0;
  double northing = 0;
  double xlon = lon - kEps;

  switch (vp.m_projection_type) {
    case PROJECTION_MERCATOR:
    case PROJECTION_WEB_MERCATOR:
    default:
      const double z = WGS84_semimajor_axis_meters * mercator_k0;

      easting = (xlon - vp.clon) * DEGREE * z;

      // y =.5 ln( (1 + sin t) / (1 - sin t) )
      const double s = sin(lat * DEGREE);
      const double y3 = (.5 * log((1 + s) / (1 - s))) * z;

      const double s0 = sin(vp.clat * DEGREE);
      const double y30 = (.5 * log((1 + s0) / (1 - s0))) * z;
      northing = y3 - y30;

      break;
  }

  double epix = easting * vp.view_scale_ppm;
  double npix = northing * vp.view_scale_ppm;
  double dxr = epix;
  double dyr = npix;

  //    Apply VP Rotation
  double angle = vp.rotation;

  if (angle) {
    dxr = epix * cos(angle) + npix * sin(angle);
    dyr = npix * cos(angle) - epix * sin(angle);
  }

  // printf("  gdpll:  %g  %g  %g\n", vp.clon, (vp.pix_width / 2.0 ) + dxr, (
  // vp.pix_height / 2.0 ) - dyr);

  return wxPoint2DDouble((vp.pix_width / 2.0) + dxr,
                         (vp.pix_height / 2.0) - dyr);
}

bool ChartMbTiles::RenderTile(SharedTilePtr tile, int zoom_level,
                              const ViewPort& vpoint) {
#ifdef ocpnUSE_GL
  ViewPort vp = vpoint;

  bool btexture = GetTileTexture(tile);
  if (!btexture) {
    // Tile is not available yet, don't render it and wait for the worker thread
    // to load and decompress it later.
    glDisable(GL_TEXTURE_2D);
    return false;
  } else {
    // Tile is available, render it on screen
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
    glColor4f(1, 1, 1, 1);
#endif
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  float coords[8];
  float texcoords[] = {0., 1., 0., 0., 1., 0., 1., 1.};

  ViewPort mvp = vp;

  wxPoint2DDouble p;

  p = GetDoublePixFromLL(mvp, tile->m_latmin, tile->m_lonmin);
  coords[0] = p.m_x;
  coords[1] = p.m_y;
  p = GetDoublePixFromLL(mvp, tile->m_latmax, tile->m_lonmin);
  coords[2] = p.m_x;
  coords[3] = p.m_y;
  p = GetDoublePixFromLL(mvp, tile->m_latmax, tile->m_lonmax);
  coords[4] = p.m_x;
  coords[5] = p.m_y;
  p = GetDoublePixFromLL(mvp, tile->m_latmin, tile->m_lonmax);
  coords[6] = p.m_x;
  coords[7] = p.m_y;

  if (!g_tile_shader_program) {
    GLShaderProgram* shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(tile_vertex_shader_source,
                                       GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(tile_fragment_shader_source,
                                       GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();
    g_tile_shader_program = shaderProgram;
  }

  GLShaderProgram* shader = g_tile_shader_program;
  shader->Bind();

  // Set up the texture sampler to texture unit 0
  shader->SetUniform1i("uTex", 0);

  shader->SetUniformMatrix4fv("MVMatrix", (GLfloat*)vp.vp_matrix_transform);

  float co1[8];
  float tco1[8];

  shader->SetAttributePointerf("aPos", co1);
  shader->SetAttributePointerf("aUV", tco1);

  // Select brightness factor depending on global color scheme
  float dimLevel = 1.0f;
  switch (m_global_color_scheme) {
    case GLOBAL_COLOR_SCHEME_DUSK:
      dimLevel = 0.8;
      break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
      dimLevel = 0.3;
      break;
    default:
      dimLevel = 1.0f;
      break;
  }
  // Give the brightness level to the shader program
  shader->SetUniform1f("brightness", dimLevel);

  // Perform the actual drawing.

// For some reason, glDrawElements is busted on Android
// So we do this a hard ugly way, drawing two triangles...
#if 0
    GLushort indices1[] = {0,1,3,2};
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices1);
#else

  co1[0] = coords[0];
  co1[1] = coords[1];
  co1[2] = coords[2];
  co1[3] = coords[3];
  co1[4] = coords[6];
  co1[5] = coords[7];
  co1[6] = coords[4];
  co1[7] = coords[5];

  tco1[0] = texcoords[0];
  tco1[1] = texcoords[1];
  tco1[2] = texcoords[2];
  tco1[3] = texcoords[3];
  tco1[4] = texcoords[6];
  tco1[5] = texcoords[7];
  tco1[6] = texcoords[4];
  tco1[7] = texcoords[5];

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

  shader->UnBind();

  glDisable(GL_BLEND);
#endif
  return true;
}

bool ChartMbTiles::RenderRegionViewOnGL(const wxGLContext& glc,
                                        const ViewPort& vpoint,
                                        const OCPNRegion& rect_region,
                                        const LLRegion& region) {
#ifdef ocpnUSE_GL

  // Reset the tile counter. This counter is used to know how many tile are
  // currently used to draw the chart and then to dimension the tile cache size
  // properly w.r.t the size of the screen and the level of details
  m_tile_count = 0;

  // Do not render if significantly underzoomed
  if (vpoint.chart_scale > (20 * osm_zoom_scale[m_min_zoom])) {
    if (m_n_tiles > 500) {
      return true;
    }
  }

  ViewPort vp = vpoint;

  OCPNRegion screen_region(wxRect(0, 0, vp.pix_width, vp.pix_height));
  LLRegion screenLLRegion = vp.GetLLRegion(screen_region);
  LLBBox screenBox = screenLLRegion.GetBox();

  if ((m_lon_max - m_lon_min) > 180) {  // big chart
    LLRegion valid_region = m_min_zoom_region;
    valid_region.Intersect(screenLLRegion);
    glChartCanvas::SetClipRegion(vp, valid_region);
  } else
    glChartCanvas::SetClipRegion(vp, m_min_zoom_region);

  /* setup opengl parameters */
  glEnable(GL_TEXTURE_2D);

  int viewZoom = m_max_zoom;
  // Set zoom modifier according to Raster Zoom Modifier settings from display
  // preference pane
  double zoom_mod = 2 * pow(2, -g_chart_zoom_modifier_raster / 3.0);

  for (int kz = m_min_zoom; kz <= 19; kz++) {
    double db_mpp = osm_zoom_mpp[kz];
    double vp_mpp = 1. / vp.view_scale_ppm;

    if (db_mpp < vp_mpp * zoom_mod) {
      viewZoom = kz;
      break;
    }
  }

  viewZoom = wxMin(viewZoom, m_max_zoom);
  // printf("viewZoomCalc: %d  %g   %g\n",  viewZoom,
  // vpoint.view_scale_ppm,  1. / vpoint.view_scale_ppm);

  int zoomFactor = m_min_zoom;
  zoomFactor = wxMax(zoomFactor, viewZoom - g_mbtilesMaxLayers);

  // DEBUG TODO   Show single zoom
  // zoomFactor = 5; //m_min_zoom;
  // viewZoom = zoomFactor;

  int maxren_zoom = m_min_zoom;

  LLBBox box = region.GetBox();
  LLBBox region_box = region.GetBox();
  bool render_pass = true;

  // if the full screen box spans IDL,
  // we need to render the entire screen in two passes.
  bool is_two_pass = false;
  if (((screenBox.GetMinLon() < -180) && (screenBox.GetMaxLon() > -180)) ||
      ((screenBox.GetMinLon() < 180) && (screenBox.GetMaxLon() > 180))) {
    is_two_pass = true;
    box = screenBox;
  }

  // For tiles declared as "OVERLAY", render only the zoom level that
  // corresponds to the currently viewed zoom level
  if (m_tile_type == MbTilesType::OVERLAY) zoomFactor = viewZoom;

  while (zoomFactor <= viewZoom) {
    // Get the tile numbers of the box corners of this render region, at this
    // zoom level
    vp = vpoint;

    // First pass, right hand side in twopass rendering
    int top_tile =
        wxMin(m_tile_cache->GetNorthLimit(zoomFactor),
              MbTileDescriptor::Lat2tiley(box.GetMaxLat(), zoomFactor));
    int bot_tile =
        wxMax(m_tile_cache->GetSouthLimit(zoomFactor),
              MbTileDescriptor::Lat2tiley(box.GetMinLat(), zoomFactor));
    int left_tile = MbTileDescriptor::Long2tilex(box.GetMinLon(), zoomFactor);
    int right_tile = MbTileDescriptor::Long2tilex(box.GetMaxLon(), zoomFactor);

    if (is_two_pass) {
      vp = vpoint;
      if (vp.clon > 0) {
        vp.clon -= 360;
        left_tile = MbTileDescriptor::Long2tilex(-180 + kEps, zoomFactor);
        right_tile =
            MbTileDescriptor::Long2tilex(box.GetMaxLon() - 360., zoomFactor);
      } else {
        left_tile = MbTileDescriptor::Long2tilex(-180 + kEps, zoomFactor);
        right_tile = MbTileDescriptor::Long2tilex(box.GetMaxLon(), zoomFactor);
      }
    }

    for (int iy = bot_tile; iy <= top_tile; iy++) {
      for (int ix = left_tile; ix <= right_tile; ix++) {
        SharedTilePtr tile = m_tile_cache->GetTile(zoomFactor, ix, iy);

        if (!region.IntersectOut(tile->m_box)) {
          if (RenderTile(tile, zoomFactor, vp)) maxren_zoom = zoomFactor;
        }
      }
    }

    // second pass
    if (is_two_pass) {
      vp = vpoint;
      if (vp.clon < 0) vp.clon += 360;

      // Get the tile numbers of the box corners of this render region, at
      // this zoom level
      int top_tile =
          wxMin(m_tile_cache->GetNorthLimit(zoomFactor),
                MbTileDescriptor::Lat2tiley(box.GetMaxLat(), zoomFactor));
      int bot_tile =
          wxMax(m_tile_cache->GetSouthLimit(zoomFactor),
                MbTileDescriptor::Lat2tiley(box.GetMinLat(), zoomFactor));
      int left_tile = MbTileDescriptor::Long2tilex(box.GetMinLon(), zoomFactor);
      int right_tile = MbTileDescriptor::Long2tilex(-180 - kEps, zoomFactor);

      if (right_tile < left_tile) right_tile = left_tile;

      for (int iy = bot_tile; iy <= top_tile; iy++) {
        for (int ix = left_tile; ix <= right_tile; ix++) {
          SharedTilePtr tile = m_tile_cache->GetTile(zoomFactor, ix, iy);

          if (!region.IntersectOut(tile->m_box))
            RenderTile(tile, zoomFactor, vp);
        }
      }
    }

    zoomFactor++;
  }

  glDisable(GL_TEXTURE_2D);

  m_zoom_scale_factor =
      2 * osm_zoom_mpp[maxren_zoom] * vpoint.view_scale_ppm / zoom_mod;

  glChartCanvas::DisableClipRegion();

  // Limit the cache size to 3 times the number of tiles to draw on the current
  // viewport. This dynamic limit allows to automatically adapt to the actual
  // resolution of the screen and to handle tricky configuration with multiple
  // screens or hdpi displays
  m_tile_cache->CleanCache(m_tile_count * 3);

  if (m_last_clean_zoom != viewZoom) {
    m_tile_cache->DeepCleanCache();
    m_last_clean_zoom = viewZoom;
  }

#endif
  return true;
}

bool ChartMbTiles::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
                                        const OCPNRegion& Region) {
  gFrame->GetPrimaryCanvas()->SetAlertString(
      _("MBTile requires OpenGL to be enabled"));

  return true;
}

bool ChartMbTiles::StartThread() {
  // Create the worker thread
  m_worker_thread = std::make_unique<MbtTilesThread>(m_db);
  m_thread = std::thread([&] { m_worker_thread->Run(); });
  m_thread.detach();
  return true;
}

void ChartMbTiles::StopThread() {
  // Stop the worker thread
  if (m_worker_thread != nullptr) {
    m_worker_thread->RequestStop();
    m_worker_thread = nullptr;
  }
}
