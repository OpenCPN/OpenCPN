/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  MBTiles chart type support
 * Author:   David Register
 *
 ***************************************************************************
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
 ***************************************************************************
 *
 */

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

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
#include <sstream>
#include <map>
#include <unordered_map>

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
#define NAN (*(double *)&lNaN)
#endif

#ifdef OCPN_USE_CONFIG
class MyConfig;
extern MyConfig *pConfig;
#endif

// Raster Zoom Modifier value from advanced display preference pane
// Physically located in ocpn_app.cpp
extern int g_chart_zoom_modifier_raster;

#define LON_UNDEF NAN
#define LAT_UNDEF NAN

// The OpenStreetMaps zommlevel translation tables
//  https://wiki.openstreetmap.org/wiki/Zoom_levels

/*Level   Degree  Area          m / pixel       ~Scale          # Tiles
0       360     whole world     156,412         1:500 million   1
1       180                     78,206          1:250 million   4
2       90                      39,103          1:150 million   16
3       45                      19,551          1:70 million    64
4       22.5                    9,776           1:35 million    256
5       11.25                   4,888           1:15 million    1,024
6       5.625                   2,444           1:10 million    4,096
7       2.813                   1,222           1:4 million     16,384
8       1.406                   610.984         1:2 million     65,536
9       0.703   wide area       305.492         1:1 million     262,144
10      0.352                   152.746         1:500,000       1,048,576
11      0.176   area            76.373          1:250,000       4,194,304
12      0.088                   38.187          1:150,000       16,777,216
13      0.044  village or town  19.093          1:70,000        67,108,864
14      0.022                   9.547           1:35,000        268,435,456
15      0.011                   4.773           1:15,000        1,073,741,824
16      0.005   small road      2.387           1:8,000         4,294,967,296
17      0.003                   1.193           1:4,000         17,179,869,184
18      0.001                   0.596           1:2,000         68,719,476,736
19      0.0005                  0.298           1:1,000         274,877,906,944
*/

// A "nominal" scale value, by zoom factor.  Estimated at equator, with monitor
// pixel size of 0.3mm
static const double OSM_zoomScale[] = {5e8,   2.5e8, 1.5e8, 7.0e7, 3.5e7, 1.5e7,
                                       1.0e7, 4.0e6, 2.0e6, 1.0e6, 5.0e5, 2.5e5,
                                       1.5e5, 7.0e4, 3.5e4, 1.5e4, 8.0e3, 4.0e3,
                                       2.0e3, 1.0e3, 5.0e2, 2.5e2};

//  Meters per pixel, by zoom factor
static const double OSM_zoomMPP[] = {
    156412, 78206,   39103,   19551,  9776,   4888,   2444,  1222,
    610,    305.492, 152.746, 76.373, 38.187, 19.093, 9.547, 4.773,
    2.387,  1.193,   0.596,   0.298,  0.149,  0.075};

extern MyFrame *gFrame;

static const double eps = 6e-6;  // about 1cm on earth's surface at equator

// Private tile shader source
static const GLchar *tile_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar *tile_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "uniform float brightness;\n"
    "void main() {\n"
    "   vec4 textureColor = texture2D(uTex, varCoord);\n"
    "   gl_FragColor = vec4(textureColor.rgb * brightness, textureColor.w);\n"
    "}\n";

GLShaderProgram *g_tile_shader_program;

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

// ============================================================================
// ChartMBTiles implementation
// ============================================================================

ChartMBTiles::ChartMBTiles() {
  //    Init some private data
  m_workerThread = nullptr;
  m_ChartFamily = CHART_FAMILY_RASTER;
  m_ChartType = CHART_TYPE_MBTILES;

  m_Chart_Skew = 0.0;

  m_datum_str = _T("WGS84");  // assume until proven otherwise
  m_projection = PROJECTION_WEB_MERCATOR;
  m_imageType = wxBITMAP_TYPE_ANY;

  m_b_cdebug = 0;

  m_minZoom = 0;
  m_maxZoom = 21;

  m_nNoCOVREntries = 0;
  m_nCOVREntries = 0;
  m_pCOVRTablePoints = NULL;
  m_pCOVRTable = NULL;
  m_pNoCOVRTablePoints = NULL;
  m_pNoCOVRTable = NULL;
  m_tileCache = NULL;

  m_LonMin = LON_UNDEF;
  m_LonMax = LON_UNDEF;
  m_LatMin = LAT_UNDEF;
  m_LatMax = LAT_UNDEF;

#ifdef OCPN_USE_CONFIG
  wxFileConfig *pfc = (wxFileConfig *)pConfig;
  pfc->SetPath(_T ( "/Settings" ));
  pfc->Read(_T ( "DebugMBTiles" ), &m_b_cdebug, 0);
#endif
  m_pDB = NULL;
}

ChartMBTiles::~ChartMBTiles() {
  // Stop the worker thread before destroying this instance
  StopThread();
  FlushTiles();

  if (m_tileCache) {
    delete m_tileCache;
  }

  if (m_pDB) {
    delete m_pDB;
  }
}

//-------------------------------------------------------------------------------------------------
//          Get the Chart thumbnail data structure
//          Creating the thumbnail bitmap as required
//-------------------------------------------------------------------------------------------------

ThumbData *ChartMBTiles::GetThumbData() { return NULL; }

ThumbData *ChartMBTiles::GetThumbData(int tnx, int tny, float lat, float lon) {
  return NULL;
}

bool ChartMBTiles::UpdateThumbData(double lat, double lon) { return true; }

bool ChartMBTiles::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed) {
  return true;
}

//    Report recommended minimum and maximum scale values for which use of this
//    chart is valid

double ChartMBTiles::GetNormalScaleMin(double canvas_scale_factor,
                                       bool b_allow_overzoom) {
  return (1);  // allow essentially unlimited overzoom
}

double ChartMBTiles::GetNormalScaleMax(double canvas_scale_factor,
                                       int canvas_width) {
  return (canvas_scale_factor / m_ppm_avg) *
         40.0;  // excessive underscale is slow, and unreadable
}

double ChartMBTiles::GetNearestPreferredScalePPM(double target_scale_ppm) {
  return target_scale_ppm;
}

// Checks/corrects/completes the initialization based on real data from the
// tiles table
void ChartMBTiles::InitFromTiles(const wxString &name) {
  try {
    // Open the MBTiles database file
    const char *name_UTF8 = "";
    wxCharBuffer utf8CB = name.ToUTF8();  // the UTF-8 buffer
    if (utf8CB.data()) name_UTF8 = utf8CB.data();

    SQLite::Database db(name_UTF8);

    // Check if tiles with advertised min and max zoom level really exist, or
    // correct the defaults We can't blindly use what we find though - the DB
    // often contains empty cells at very low zoom levels, so if we have some
    // info from metadata, we will use that if more conservative...
    SQLite::Statement query(db,
                            "SELECT min(zoom_level) AS min_zoom, "
                            "max(zoom_level) AS max_zoom FROM tiles");
    while (query.executeStep()) {
      const char *colMinZoom = query.getColumn(0);
      const char *colMaxZoom = query.getColumn(1);

      int min_zoom = 0, max_zoom = 0;
      sscanf(colMinZoom, "%i", &min_zoom);
      m_minZoom = wxMax(m_minZoom, min_zoom);
      sscanf(colMaxZoom, "%i", &max_zoom);
      m_maxZoom = wxMin(m_maxZoom, max_zoom);
      if (m_minZoom > m_maxZoom) {
        // We are looking at total nonsense with wrong metatadata and actual
        // tile coverage out of it, better use what's really in the data to be
        // able to show at least something
        m_minZoom = min_zoom;
        m_maxZoom = max_zoom;
      }
    }

    //        std::cout << name.c_str() << " zoom_min: " << m_minZoom << "
    //        zoom_max: " << m_maxZoom << std::endl;

    // Traversing the entire tile table can be expensive....
    //  Use declared bounds if present.

    if (!std::isnan(m_LatMin) && !std::isnan(m_LatMax) &&
        !std::isnan(m_LonMin) && !std::isnan(m_LonMax))
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
            m_minZoom, m_maxZoom)
            .c_str());
    float minLat = 999., maxLat = -999.0, minLon = 999., maxLon = -999.0;
    while (query1.executeStep()) {
      const char *colMinRow = query1.getColumn(0);
      const char *colMaxRow = query1.getColumn(1);
      const char *colMinCol = query1.getColumn(2);
      const char *colMaxCol = query1.getColumn(3);
      const char *colCnt = query1.getColumn(4);
      const char *colZoom = query1.getColumn(5);

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
      minLat = wxMin(minLat, mbTileDescriptor::tiley2lat(minRow, zoom));
      maxLat = wxMax(maxLat, mbTileDescriptor::tiley2lat(maxRow - 1, zoom));
      minLon = wxMin(minLon, mbTileDescriptor::tilex2long(minCol, zoom));
      maxLon = wxMax(maxLon, mbTileDescriptor::tilex2long(maxCol + 1, zoom));
      // std::cout << "Zoom: " << zoom << " minlat: " << tiley2lat(minRow, zoom)
      // << " maxlat: " << tiley2lat(maxRow - 1, zoom) << " minlon: " <<
      // tilex2long(minCol, zoom) << " maxlon: " << tilex2long(maxCol + 1, zoom)
      // << std::endl;
    }

    // ... and use what we found only in case we miss some of the values from
    // metadata...
    if (std::isnan(m_LatMin)) m_LatMin = minLat;
    if (std::isnan(m_LatMax)) m_LatMax = maxLat;
    if (std::isnan(m_LonMin)) m_LonMin = minLon;
    if (std::isnan(m_LonMax)) m_LonMax = maxLon;
  } catch (std::exception &e) {
    const char *t = e.what();
    wxLogMessage("mbtiles exception: %s", e.what());
  }
}

InitReturn ChartMBTiles::Init(const wxString &name, ChartInitFlag init_flags) {
  m_global_color_scheme = GLOBAL_COLOR_SCHEME_RGB;

  m_FullPath = name;
  m_Description = m_FullPath;

  try {
    // Open the MBTiles database file
    const char *name_UTF8 = "";
    wxCharBuffer utf8CB = name.ToUTF8();  // the UTF-8 buffer
    if (utf8CB.data()) name_UTF8 = utf8CB.data();

    SQLite::Database db(name_UTF8);

    // Compile a SQL query, getting everything from the "metadata" table
    SQLite::Statement query(db, "SELECT * FROM metadata ");

    // Loop to execute the query step by step, to get rows of result
    while (query.executeStep()) {
      const char *colName = query.getColumn(0);
      const char *colValue = query.getColumn(1);

      // Get the geometric extent of the data
      if (!strncmp(colName, "bounds", 6)) {
        float lon1, lat1, lon2, lat2;
        sscanf(colValue, "%g,%g,%g,%g", &lon1, &lat1, &lon2, &lat2);

        // There is some confusion over the layout of this field...
        m_LatMax = wxMax(lat1, lat2);
        m_LatMin = wxMin(lat1, lat2);
        m_LonMax = wxMax(lon1, lon2);
        m_LonMin = wxMin(lon1, lon2);

      }

      else if (!strncmp(colName, "format", 6)) {
        m_format = std::string(colValue);
      }

      // Get the min and max zoom values present in the db
      else if (!strncmp(colName, "minzoom", 7)) {
        sscanf(colValue, "%i", &m_minZoom);
      } else if (!strncmp(colName, "maxzoom", 7)) {
        sscanf(colValue, "%i", &m_maxZoom);
      }

      else if (!strncmp(colName, "description", 11)) {
        m_Description = wxString(colValue, wxConvUTF8);
      } else if (!strncmp(colName, "name", 11)) {
        m_Name = wxString(colValue, wxConvUTF8);
      } else if (!strncmp(colName, "type", 11)) {
        m_Type = wxString(colValue, wxConvUTF8).Upper().IsSameAs("OVERLAY")
                     ? MBTilesType::OVERLAY
                     : MBTilesType::BASE;
      } else if (!strncmp(colName, "scheme", 11)) {
        m_Scheme = wxString(colValue, wxConvUTF8).Upper().IsSameAs("XYZ")
                       ? MBTilesScheme::XYZ
                       : MBTilesScheme::TMS;
      }
    }
  } catch (std::exception &e) {
    const char *t = e.what();
    wxLogMessage("mbtiles exception: %s", e.what());
    return INIT_FAIL_REMOVE;
  }

  // Fix the missing/wrong metadata values
  InitFromTiles(name);

  // set the chart scale parameters based on the max zoom factor
  m_ppm_avg = 1.0 / OSM_zoomMPP[m_minZoom];
  m_Chart_Scale = OSM_zoomScale[m_maxZoom];

  // Initialize the tile data structures
  m_tileCache = new TileCache(m_minZoom, m_maxZoom, m_LonMin, m_LatMin,
                              m_LonMax, m_LatMax);

  LLRegion covrRegion;

  LLBBox extentBox;
  extentBox.Set(m_LatMin, m_LonMin, m_LatMax, m_LonMax);

  const char *name_UTF8 = "";
  wxCharBuffer utf8CB = name.ToUTF8();  // the UTF-8 buffer
  if (utf8CB.data()) name_UTF8 = utf8CB.data();

  SQLite::Database db(name_UTF8);

  int zoomFactor = m_minZoom;
  int minRegionZoom = -1;
  bool covr_populated = false;

  m_nTiles = 0;
  while ((zoomFactor <= m_maxZoom) && (minRegionZoom < 0)) {
    LLRegion covrRegionZoom;
    wxRegion regionZoom;
    char qrs[100];

    // Protect against trying to create the exact coverage for the brutal large
    // scale layers contianing tens of thousand tiles.
    sprintf(qrs, "select count(*) from tiles where zoom_level = %d ",
            zoomFactor);
    SQLite::Statement query_size(db, qrs);

    if (query_size.executeStep()) {
      const char *colValue = query_size.getColumn(0);
      int tile_at_zoom = atoi(colValue);
      m_nTiles += tile_at_zoom;

      if (tile_at_zoom > 1000) {
        zoomFactor++;
        if (!covr_populated) {
          covr_populated = true;
          covrRegion = extentBox;
        }
        continue;
      }
    }

    // query the database
    sprintf(qrs,
            "select tile_column, tile_row from tiles where zoom_level = %d ",
            zoomFactor);

    // Compile a SQL query, getting the specific  data
    SQLite::Statement query(db, qrs);
    covr_populated = true;

    while (query.executeStep()) {
      const char *colValue = query.getColumn(0);
      const char *c2 = query.getColumn(1);
      int tile_x_found = atoi(colValue);  // tile_x
      int tile_y_found = atoi(c2);        // tile_y

      regionZoom.Union(tile_x_found, tile_y_found - 1, 1, 1);

    }  // inner while

    wxRegionIterator upd(regionZoom);  // get the  rect list
    double eps_factor = eps * 100;     // roughly 1 m

    while (upd) {
      wxRect rect = upd.GetRect();

      double lonmin =
          round(mbTileDescriptor::tilex2long(rect.x, zoomFactor) / eps_factor) *
          eps_factor;
      double lonmax =
          round(mbTileDescriptor::tilex2long(rect.x + rect.width, zoomFactor) /
                eps_factor) *
          eps_factor;
      double latmin =
          round(mbTileDescriptor::tiley2lat(rect.y, zoomFactor) / eps_factor) *
          eps_factor;
      double latmax =
          round(mbTileDescriptor::tiley2lat(rect.y + rect.height, zoomFactor) /
                eps_factor) *
          eps_factor;

      LLBBox box;
      box.Set(latmin, lonmin, latmax, lonmax);

      LLRegion tileRegion(box);
      // if(i <= 1)
      covrRegionZoom.Union(tileRegion);

      upd++;
      minRegionZoom = zoomFactor;  // We take the first populated (lowest) zoom
                                   // level region as the final chart region
    }

    covrRegion.Union(covrRegionZoom);

    zoomFactor++;

  }  // while

  //  The coverage region must be reduced if necessary to include only the db
  //  specified bounds.
  covrRegion.Intersect(extentBox);

  m_minZoomRegion = covrRegion;

  //  Populate M_COVR entries for the OCPN chart database
  if (covrRegion.contours.size()) {  // Check for no intersection caused by ??
    m_nCOVREntries = covrRegion.contours.size();
    m_pCOVRTablePoints = (int *)malloc(m_nCOVREntries * sizeof(int));
    m_pCOVRTable = (float **)malloc(m_nCOVREntries * sizeof(float *));
    std::list<poly_contour>::iterator it = covrRegion.contours.begin();
    for (int i = 0; i < m_nCOVREntries; i++) {
      m_pCOVRTablePoints[i] = it->size();
      m_pCOVRTable[i] =
          (float *)malloc(m_pCOVRTablePoints[i] * 2 * sizeof(float));
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
  StartThread();

  if (pi_ret != INIT_OK)
    return pi_ret;
  else
    return INIT_OK;
}

InitReturn ChartMBTiles::PreInit(const wxString &name, ChartInitFlag init_flags,
                                 ColorScheme cs) {
  m_global_color_scheme = cs;
  return INIT_OK;
}

InitReturn ChartMBTiles::PostInit(void) {
  // Create the persistent MBTiles database file
  const char *name_UTF8 = "";
  wxCharBuffer utf8CB = m_FullPath.ToUTF8();  // the UTF-8 buffer
  if (utf8CB.data()) name_UTF8 = utf8CB.data();

  m_pDB = new SQLite::Database(name_UTF8);
  m_pDB->exec("PRAGMA locking_mode=EXCLUSIVE");
  m_pDB->exec("PRAGMA cache_size=-50000");

  bReadyToRender = true;
  return INIT_OK;
}

// FIXME : useless now
void ChartMBTiles::FlushTiles() {
  // Delete all the tiles in the tile cache
  // Note that this function also deletes OpenGL texture memory associated to
  // the tiles
  if (m_tileCache) {
    m_tileCache->Flush();
  }
}

bool ChartMBTiles::GetChartExtent(Extent *pext) {
  pext->NLAT = m_LatMax;
  pext->SLAT = m_LatMin;
  pext->ELON = m_LonMax;
  pext->WLON = m_LonMin;

  return true;
}

void ChartMBTiles::SetColorScheme(ColorScheme cs, bool bApplyImmediate) {
  m_global_color_scheme = cs;
}

void ChartMBTiles::GetValidCanvasRegion(const ViewPort &VPoint,
                                        OCPNRegion *pValidRegion) {
  pValidRegion->Clear();
  pValidRegion->Union(0, 0, VPoint.pix_width, VPoint.pix_height);
  return;
}

LLRegion ChartMBTiles::GetValidRegion() { return m_minZoomRegion; }

bool ChartMBTiles::RenderViewOnDC(wxMemoryDC &dc, const ViewPort &VPoint) {
  return true;
}

/// @brief Loads a tile into OpenGL's texture memory for rendering. If the tile
/// is not ready to be rendered (i.e. the tile has not been loaded from disk or
/// decompressed to memory), the function sends a request to the worker thread
/// which will do this later in the background.
/// @param tile Pointer to the tile descriptor to be prepared
/// @return true if the tile is ready to be rendered, false else.
bool ChartMBTiles::getTileTexture(mbTileDescriptor *tile) {
  if (!m_pDB) return false;
  m_tileCount++;
  // Is the texture ready to be rendered ?
  if (tile->glTextureName > 0) {
    // Yes : bind the texture and return to the caller
    glBindTexture(GL_TEXTURE_2D, tile->glTextureName);
    return true;
  } else if (!tile->m_bAvailable) {
    // Tile is not in MbTiles file : no texture to render
    m_tileCount--;
    return false;
  } else if (tile->m_teximage == 0) {
    if (tile->m_requested == false) {
      // The tile has not loaded and decompressed previously : request it
      // to the worker thread
      m_workerThread->RequestTile(tile);
    }
    return false;
  } else {
    // The tile has been decompressed to memory : load it into OpenGL texture
    // memory
    if (tile->glTextureName > 0) {
      glDeleteTextures(1, &tile->glTextureName);
    }
#ifndef __OCPN__ANDROID__
    glEnable(GL_COLOR_MATERIAL);
#endif
    glGenTextures(1, &tile->glTextureName);
    glBindTexture(GL_TEXTURE_2D, tile->glTextureName);

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

wxPoint2DDouble ChartMBTiles::GetDoublePixFromLL(ViewPort &vp, double lat,
                                                 double lon) {
  double easting = 0;
  double northing = 0;
  double xlon = lon - eps;

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

bool ChartMBTiles::RenderTile(mbTileDescriptor *tile, int zoomLevel,
                              const ViewPort &VPoint) {
#ifdef ocpnUSE_GL
  ViewPort vp = VPoint;

  bool btexture = getTileTexture(tile);
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

  p = GetDoublePixFromLL(mvp, tile->latmin, tile->lonmin);
  coords[0] = p.m_x;
  coords[1] = p.m_y;
  p = GetDoublePixFromLL(mvp, tile->latmax, tile->lonmin);
  coords[2] = p.m_x;
  coords[3] = p.m_y;
  p = GetDoublePixFromLL(mvp, tile->latmax, tile->lonmax);
  coords[4] = p.m_x;
  coords[5] = p.m_y;
  p = GetDoublePixFromLL(mvp, tile->latmin, tile->lonmax);
  coords[6] = p.m_x;
  coords[7] = p.m_y;

  if (!g_tile_shader_program) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(tile_vertex_shader_source,
                                       GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(tile_fragment_shader_source,
                                       GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();
    g_tile_shader_program = shaderProgram;
  }

  GLShaderProgram *shader = g_tile_shader_program;
  shader->Bind();

  // Set up the texture sampler to texture unit 0
  shader->SetUniform1i("uTex", 0);

  shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)vp.vp_matrix_transform);

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


bool ChartMBTiles::RenderRegionViewOnGL(const wxGLContext &glc,
                                        const ViewPort &VPoint,
                                        const OCPNRegion &RectRegion,
                                        const LLRegion &Region) {
#ifdef ocpnUSE_GL

  // Reset the tile counter. This counter is used to know how many tile are
  // currently used to draw the chart and then to dimension the tile cache size
  // properly w.r.t the size of the screen and the level of details
  m_tileCount = 0;

  // Do not render if significantly underzoomed
  if (VPoint.chart_scale > (20 * OSM_zoomScale[m_minZoom])) {
    if (m_nTiles > 500) {
      return true;
    }
  }

  ViewPort vp = VPoint;

  OCPNRegion screen_region(wxRect(0, 0, vp.pix_width, vp.pix_height));
  LLRegion screenLLRegion = vp.GetLLRegion(screen_region);
  LLBBox screenBox = screenLLRegion.GetBox();

  if ((m_LonMax - m_LonMin) > 180) {  // big chart
    LLRegion validRegion = m_minZoomRegion;
    validRegion.Intersect(screenLLRegion);
    glChartCanvas::SetClipRegion(vp, validRegion);
  } else
    glChartCanvas::SetClipRegion(vp, m_minZoomRegion);

  /* setup opengl parameters */
  glEnable(GL_TEXTURE_2D);

  int viewZoom = m_maxZoom;
  // Set zoom modifier according to Raster Zoom Modifier settings from display
  // preference pane
  double zoomMod = 2 * pow(2, -g_chart_zoom_modifier_raster / 3.0);

  for (int kz = m_minZoom; kz <= 19; kz++) {
    double db_mpp = OSM_zoomMPP[kz];
    double vp_mpp = 1. / vp.view_scale_ppm;

    if (db_mpp < vp_mpp * zoomMod) {
      viewZoom = kz;
      break;
    }
  }

  viewZoom = wxMin(viewZoom, m_maxZoom);
  // printf("viewZoomCalc: %d  %g   %g\n",  viewZoom,
  // VPoint.view_scale_ppm,  1. / VPoint.view_scale_ppm);

  int zoomFactor = m_minZoom;
  zoomFactor = wxMax(zoomFactor, viewZoom - g_mbtilesMaxLayers);

  // DEBUG TODO   Show single zoom
  // zoomFactor = 5; //m_minZoom;
  // viewZoom = zoomFactor;

  int maxrenZoom = m_minZoom;

  LLBBox box = Region.GetBox();

  // if the full screen box spans IDL,
  // we need to render the entire screen in two passes.
  bool btwoPass = false;
  if (((screenBox.GetMinLon() < -180) && (screenBox.GetMaxLon() > -180)) ||
      ((screenBox.GetMinLon() < 180) && (screenBox.GetMaxLon() > 180))) {
    btwoPass = true;
    box = screenBox;
  }

  while (zoomFactor <= viewZoom) {
    // Get the tile numbers of the box corners of this render region, at this
    // zoom level
    int topTile =
        wxMin(m_tileCache->GetNorthLimit(zoomFactor),
              mbTileDescriptor::lat2tiley(box.GetMaxLat(), zoomFactor));
    int botTile =
        wxMax(m_tileCache->GetSouthLimit(zoomFactor),
              mbTileDescriptor::lat2tiley(box.GetMinLat(), zoomFactor));
    int leftTile = mbTileDescriptor::long2tilex(box.GetMinLon(), zoomFactor);
    int rightTile = mbTileDescriptor::long2tilex(box.GetMaxLon(), zoomFactor);

    if (btwoPass) {
      leftTile = mbTileDescriptor::long2tilex(-180 + eps, zoomFactor);
      rightTile = mbTileDescriptor::long2tilex(box.GetMaxLon(), zoomFactor);
      vp = VPoint;
      if (vp.clon > 0) vp.clon -= 360;

    } else
      vp = VPoint;

    for (int iy = botTile; iy <= topTile; iy++) {
      for (int ix = leftTile; ix <= rightTile; ix++) {
        mbTileDescriptor *tile = m_tileCache->GetTile(zoomFactor, ix, iy);

        if (!Region.IntersectOut(tile->box)) {
          if (RenderTile(tile, zoomFactor, vp)) maxrenZoom = zoomFactor;
        }
      }
    }

    // second pass
    if (btwoPass) {
      vp = VPoint;
      if (vp.clon < 0) vp.clon += 360;

      // Get the tile numbers of the box corners of this render region, at
      // this zoom level
      int topTile =
          wxMin(m_tileCache->GetNorthLimit(zoomFactor),
                mbTileDescriptor::lat2tiley(box.GetMaxLat(), zoomFactor));
      int botTile =
          wxMax(m_tileCache->GetSouthLimit(zoomFactor),
                mbTileDescriptor::lat2tiley(box.GetMinLat(), zoomFactor));
      int leftTile = mbTileDescriptor::long2tilex(box.GetMinLon(), zoomFactor);
      int rightTile = mbTileDescriptor::long2tilex(-180 - eps, zoomFactor);

      if (rightTile < leftTile) rightTile = leftTile;

      for (int iy = botTile; iy <= topTile; iy++) {
        for (int ix = leftTile; ix <= rightTile; ix++) {
          mbTileDescriptor *tile = m_tileCache->GetTile(zoomFactor, ix, iy);

          if (!Region.IntersectOut(tile->box)) RenderTile(tile, zoomFactor, vp);
        }
      }
    }

    zoomFactor++;
  }

  glDisable(GL_TEXTURE_2D);

  m_zoomScaleFactor = 2 * OSM_zoomMPP[maxrenZoom] * VPoint.view_scale_ppm / zoomMod;

  glChartCanvas::DisableClipRegion();

  // Limit the cache size to 3 times the number of tiles to draw on a rendering
  m_tileCache->CleanCache(m_tileCount * 3);
#endif
  return true;
}

bool ChartMBTiles::RenderRegionViewOnDC(wxMemoryDC &dc, const ViewPort &VPoint,
                                        const OCPNRegion &Region) {
  gFrame->GetPrimaryCanvas()->SetAlertString(
      _("MBTile requires OpenGL to be enabled"));

  return true;
}

/// @brief Create and start the wortker thread. This thread is dedicated at
/// loading and decompressing chart tiles into memory, in the background. If
/// for any reason the thread would fail to load, a fatal error id generated
/// and a message displayed to the user.
void ChartMBTiles::StartThread() {
  // Create the worker thread
  m_workerThread = new MbtTilesThread(m_pDB);
  if (m_workerThread->Run() != wxTHREAD_NO_ERROR) {
    delete m_workerThread;
    m_workerThread = nullptr;
    // Not beeing able to create the worker thread is really a bad situation,
    // never supposed to happen. So we trigger a fatal error.
    wxLogFatalError("MbTiles: Can't create the worker thread");
  }
}

/// @brief  Stop and delete the worker thread. This function is called when
/// OpenCPN is quitting.
void ChartMBTiles::StopThread() {
  // Stop the worker thread
  if (m_workerThread != nullptr) {
    m_workerThread->RequestStop();
    m_workerThread = nullptr;
  }
}
