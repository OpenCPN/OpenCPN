/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  ChartBase, ChartBaseBSB and Friends
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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

#include <cassert>

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/filename.h>
#include <sys/stat.h>
#include <geovalues.h>
#include <sstream>

#include "config.h"
#include "chartimg.h"
#include "chartgeotiff.h"
#include "ocpn_pixel.h"
#include "chartdata_input_stream.h"
#include "OCPNPlatform.h"

extern OCPNPlatform *g_Platform;

// ============================================================================
// ChartGeoTIFF implementation
// ============================================================================
ChartGeoTIFF::ChartGeoTIFF() : tif(nullptr), gtif(nullptr), im(nullptr) {
  m_ChartType = CHART_TYPE_GEOTIFF;
}

ChartGeoTIFF::~ChartGeoTIFF() {
  if (im) {
    _TIFFfree(im);
  }
  if (tif) {
    XTIFFClose(tif);
  }
  if (gtif) {
    GTIFFree(gtif);
  }
}

bool ChartGeoTIFF::CornerLatLon(GTIF *gtif, GTIFDefn *defn, double &x,
                                double &y, float &lat, float &lon) {
  double x_ = x, y_ = y;
  if (!GTIFImageToPCS(gtif, &x_, &y_)) {
    return false;
  }
  if (defn->Model == ModelTypeGeographic) {
    lat = y_;
    lon = x_;
  } else {
    if (GTIFProj4ToLatLong(defn, 1, &x_, &y_)) {
      lat = y_;
      lon = x_;
    } else {
      return false;
    }
  }
  return true;
}

InitReturn ChartGeoTIFF::Init(const wxString &name, ChartInitFlag init_flags) {
  PreInit(name, init_flags, GLOBAL_COLOR_SCHEME_DAY);
  tif = XTIFFOpen(name.c_str(), "r");
  if (!tif) {
    return INIT_FAIL_REMOVE;
  }

  gtif = GTIFNew(tif);
  if (!gtif) {
    return INIT_FAIL_REMOVE;
  }

  m_filesize = wxFileName::GetSize(name);

  m_FullPath = name;
  m_Description = m_FullPath;

  m_DepthUnits = "";
  m_PubYear = "1972";
  m_datum_str = "WGS 1984";
  m_Name = "";

  m_datum_index = GetDatumIndex(m_datum_str);
  if (m_datum_index < 0) {
    wxString msg(_T("   Chart datum {"));
    msg += m_datum_str;
    msg += _T("} invalid on chart ");
    msg.Append(m_FullPath);
    wxLogMessage(msg);
    wxLogMessage(_T("   Default datum (WGS84) substituted."));

    m_datum_index = DATUM_INDEX_WGS84;
  }

// Read the corners and fill the coverage table
// TODO: Do we really always have just the 4 corners as PLYs? We don't, but the
// shape is defined by the alpha channel and we would have to trace it
// ourselves
// We do have always the 4 points as REFs as it seems though
#define REF_POINTS 4
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &Size_X);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &Size_Y);

  unsigned short raster_type = RasterPixelIsArea;
  GTIFKeyGetSHORT(gtif, GTRasterTypeGeoKey, &raster_type, 0, 1);

  double xmin = (raster_type == RasterPixelIsArea) ? 0.0 : -0.5;
  double ymin = xmin;
  double ymax = ymin + Size_Y;
  double xmax = xmin + Size_X;

  GTIFDefn defn;

  if (!GTIFGetDefn(gtif, &defn)) {
    return INIT_FAIL_REMOVE;
  }

  Plypoint PlyTable[REF_POINTS];  // By default we define the polygon around the
                                  // whole image
  if (!CornerLatLon(gtif, &defn, xmin, ymin, PlyTable[0].ltp,
                    PlyTable[0].lnp)) {  // Upper Left
    return INIT_FAIL_REMOVE;
  }
  if (!CornerLatLon(gtif, &defn, xmin, ymax, PlyTable[1].ltp,
                    PlyTable[1].lnp)) {  // Lower left
    return INIT_FAIL_REMOVE;
  }
  if (!CornerLatLon(gtif, &defn, xmax, ymax, PlyTable[2].ltp,
                    PlyTable[2].lnp)) {  // Lower right
    return INIT_FAIL_REMOVE;
  }
  if (!CornerLatLon(gtif, &defn, xmax, ymin, PlyTable[3].ltp,
                    PlyTable[3].lnp)) {  // Upper right
    return INIT_FAIL_REMOVE;
  }

  const double pixelMM = (double)g_Platform->GetDisplaySizeMM() /
                   wxMax(wxGetDisplaySize().x, wxGetDisplaySize().y);
  m_Chart_Scale = (PlyTable[0].ltp - PlyTable[1].ltp) * 1852 * 60 * 1000 /
                  (pixelMM * Size_Y);

  m_nCOVREntries = 1;
  m_pCOVRTablePoints = static_cast<int *>(malloc(sizeof(int)));
  *m_pCOVRTablePoints = REF_POINTS;
  m_pCOVRTable = static_cast<float **>(malloc(sizeof(float *)));
  *m_pCOVRTable = static_cast<float *>(malloc(REF_POINTS * 2 * sizeof(float)));
  memcpy(*m_pCOVRTable, &PlyTable, REF_POINTS * 2 * sizeof(float));

  nRefpoint = REF_POINTS;
  pRefTable = static_cast<Refpoint *>(
      realloc(pRefTable, sizeof(Refpoint) * (REF_POINTS)));
  for (size_t nRefpoint = 0; nRefpoint < REF_POINTS; nRefpoint++) {
    pRefTable[nRefpoint].xr = nRefpoint <= 1 ? xmin : xmax;
    pRefTable[nRefpoint].yr = nRefpoint == 0 || nRefpoint == 3 ? ymin : ymax;
    pRefTable[nRefpoint].latr = PlyTable[nRefpoint].ltp;
    pRefTable[nRefpoint].lonr = PlyTable[nRefpoint].lnp;
    pRefTable[nRefpoint].bXValid = 1;
    pRefTable[nRefpoint].bYValid = 1;
  }

  if (!SetMinMax()) {
    return INIT_FAIL_REMOVE;  // have to bail here
  }

  return PostInit();
}

bool ChartGeoTIFF::IsPixelTransparent(const uint8_t *image, const int &x,
                                      const int &y, bool fix_borders) const {
  if (fix_borders) {
    if (x < 0 || y < 0) {
      return false;
    }
    if (x >= Size_X || y >= Size_Y) {
      return false;
    }
    if (x == 0 || y == 0 || x == Size_X - 1 || y == Size_Y - 1) {
      return true;
    }
  }
  const size_t pos = 4 * y * Size_X + 4 * x;
  return image[pos + 3] == 0;
}

bool ChartGeoTIFF::FindEdge(const uint8_t *image, int &x, int &y) const {
  y = Size_Y;
#define STEPS 4
  // Bottom to top samples
  for (int xi = 0; xi < Size_X; xi += Size_X / STEPS) {
    x = xi;
    for (int yi = 0; yi < Size_Y; yi++) {
      if (IsPixelTransparent(image, xi, yi)) {
        y = yi;
      } else {
        if (y > 0 && y < Size_Y - 1) {
          return true;
        }
      }
    }
  }
  // Left to right samples
  for (int yi = 0; yi < Size_Y; yi += Size_Y / STEPS) {
    y = yi;
    for (int xi = 0; xi < Size_X; xi++) {
      if (IsPixelTransparent(image, xi, yi)) {
        x = xi;
      } else {
        if (x > 0 && x < Size_X - 1) {
          return true;
        }
      }
    }
  }
  // TODO: Top->bottom & Right->Left (Can be done in the same loop in case we do
  // not find first pixel to be transparent, but it will probably be faster to
  // start independently in the other direction most of the time?)
  // TODO: Another possible optimization is to remember which sample was
  // completely transparent not to have to scan it in the other direction
  return false;
}

InitReturn ChartGeoTIFF::PostInit() {
  const int analyze_ret_val = AnalyzeRefpoints();
  if (0 != analyze_ret_val) {
    return INIT_FAIL_REMOVE;
  }

  pline_table = nullptr;

  im = static_cast<uint8_t *>(_TIFFmalloc(
      Size_X * Size_Y *
      4));  // FIXME: This is very simple, but takes A LOT of memory, needs to be
           // reimplemented so that GetChartBits loads only the needed
           // lines/tiles/strips instead
  const int res = TIFFReadRGBAImage(tif, Size_X, Size_Y, reinterpret_cast<uint32_t *>(im));
  if (res != 1) {
    return INIT_FAIL_REMOVE;
  }

  int x, y;
  if (FindEdge(im, x, y)) {
    // Found edge, let's trace the outline
    if (TraceOutline(im, x, y) && !ply_points.empty()) {
      *m_pCOVRTablePoints = static_cast<int>(ply_points.size());
      Plypoint PlyTable[*m_pCOVRTablePoints];  // By default we define the
                                               // polygon around the whole image
      GTIFDefn defn;

      if (!GTIFGetDefn(gtif, &defn)) {
        return INIT_FAIL_REMOVE;
      }
      size_t cnt{0};
      for (const auto &p : ply_points) {
        double dx = p.first;
        double dy = p.second;
        if (!CornerLatLon(gtif, &defn, dx, dy, PlyTable[cnt].ltp,
                          PlyTable[cnt].lnp)) {
          return INIT_FAIL_NOERROR;  // TODO: Is it OK to return this here or
                                     // should it be INIT_OK as we simply keep
                                     // the simple default table?
        }
        cnt++;
      }
      m_pCOVRTable = static_cast<float **>(malloc(sizeof(float *)));
      *m_pCOVRTable = static_cast<float *>(malloc(*m_pCOVRTablePoints * 2 * sizeof(float)));
      memcpy(*m_pCOVRTable, &PlyTable, *m_pCOVRTablePoints * 2 * sizeof(float));
    }
  }

  bReadyToRender = true;
  return INIT_OK;
}

bool ChartGeoTIFF::GetChartBits(wxRect &source, unsigned char *pPix,
                                int sub_samp) {
  wxCriticalSectionLocker locker(m_critSect);

#define FILL_BYTE 128
  memset(pPix, FILL_BYTE, source.height * source.width * BPP / 8);
  uint8_t r, g, b;
  for (int i = 0; i < source.height; i++) {
    for (int j = 0; j < source.width; j++) {
      const size_t pos = 4 * (Size_Y - i - source.y) * Size_X + 4 * (source.x + j);
      if (pos <= 4 * Size_X * Size_Y) {
        r = im[pos];
        g = im[pos + 1];
        b = im[pos + 2];
      } else {
        r = g = b = 0;
      }
      pPix[i * source.width * 3 + j * 3] = r;
      pPix[i * source.width * 3 + j * 3 + 1] = g;
      pPix[i * source.width * 3 + j * 3 + 2] = b;
    }
  }
  return true;
}

void ChartGeoTIFF::AppendPoint(const int &x, const int &y, const int &dir) {
  if (ply_points.empty()) {
    ply_points.emplace_back(x, Size_Y - y);
  } else {
    int ex_x = ply_points.back().first;
    const int ex_y = ply_points.back().second;
    if (abs(ex_x - x) == 1 && abs(ex_y - y) == 1) {
      switch (dir) {
        case 1:
          case 3:
          ply_points.back() = {ex_x, Size_Y - y};
          break;

        case 5:
          case 7:

          ply_points.back() = {x, Size_Y - ex_y};
          break;
        default:
          break;
      }
    } else {
      ply_points.emplace_back(x, Size_Y - y);
    }
  }
}

bool ChartGeoTIFF::TraceOutline(const uint8_t *image, const int x0, const int y0) {
  /*
 image: an RGBA image buffer
 x0, y0, coordinates of a point on the edge of the transparent area
          N
    ┏━━━━━━━━━━━┓
    ┃ 7   0   1 ┃
  W ┃ 6   o > 2 ┃ E   o = [2, 3, 4, 5, 6, 7, 0, 1]
    ┃ 5   4   3 ┃
    ┗━━━━━━━━━━━┛
          S
*/
  ply_points.clear();
  int x{x0};
  int y{y0};
  bool hitstart{false};
  size_t count{0};
  const size_t countlimit =
      (Size_X + Size_Y) /
      5;  // Artificial limit for the number of points in the outline
  const int indices[8] = {2, 3, 4, 5, 6, 7, 0, 1};
  bool neighbors[8];
  int newdir{-1};
  int olddir{-1};
  while (!hitstart) {
    neighbors[2] = IsPixelTransparent(image, x + 1, y, true);
    neighbors[3] = IsPixelTransparent(image, x + 1, y - 1, true);
    neighbors[4] = IsPixelTransparent(image, x, y - 1, true);
    neighbors[5] = IsPixelTransparent(image, x - 1, y - 1, true);
    neighbors[6] = IsPixelTransparent(image, x - 1, y, true);
    neighbors[7] = IsPixelTransparent(image, x - 1, y + 1, true);
    neighbors[0] = IsPixelTransparent(image, x, y + 1, true);
    neighbors[1] = IsPixelTransparent(image, x + 1, y + 1, true);
    for (const int idx : indices) {
      if (neighbors[idx]) {
        if (std::abs(idx - newdir) !=
            4) {  // We do not ever want to go back the same way we got here
                  // (And the difference between opposite directions is always
                  // 4)
          if ((newdir == 7 || newdir == 0 || newdir == 1)  // If we are going up
              && (neighbors[7] || neighbors[0] ||
                  neighbors[1])  // and can continue in that general direction
              && (idx == 3 || idx == 4 || idx == 5)  // avoid going down
          ) {
            continue;
          } else if ((newdir == 7 || newdir == 6 ||
                      newdir == 5)  // If we are going left
                     && (neighbors[7] || neighbors[6] ||
                         neighbors[5])  // and can continue in that general
                                        // direction
                     && (idx == 3 || idx == 2 || idx == 1)  // avoid going right
          ) {
            continue;
          }
          newdir = idx;
          break;
        }
      }
    }
    if (newdir != olddir) {
      // We are changing the direction of movement, need to record the corner
      AppendPoint(x, y, newdir);
      olddir = newdir;
      count++;
    }
    switch (newdir) {
      case 2:
        x++;
        break;
      case 3:
        x++;
        y--;
        break;
      case 4:
        y--;
        break;
      case 5:
        x--;
        y--;
        break;
      case 6:
        x--;
        break;
      case 7:
        x--;
        y++;
        break;
      case 0:
        y++;
        break;
      case 1:
        x++;
        y++;
        break;
      default:
        std::cerr << "Ouch, we got stuck!" << std::endl;
        return false;
    }

    if (count >= 4 && abs(x - x0) < 2 && abs(y - y0) < 2) {
      hitstart = true;
    }
    if (count == countlimit) {
      std::cerr << "Ouch, the path seems to be too long!" << std::endl;
      return false;
    }
  }
  return true;
}