/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  ChartBaseBSB and Friends
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *
 */

#ifndef _GEOTIFF_H_
#define _GEOTIFF_H_

#include "chartimg.h"
#include "georef.h"  // for GeoRef type
#include "OCPNRegion.h"
#include "viewport.h"
#include <xtiffio.h>
#include <geotiff.h>
#include <geo_normalize.h>

// ----------------------------------------------------------------------------
// ChartGeoTIFF
// ----------------------------------------------------------------------------
class ChartGeoTIFF : public ChartBaseBSB {
public:
  //    Methods
  ChartGeoTIFF();
  ~ChartGeoTIFF();

  InitReturn Init(const wxString &name, ChartInitFlag init_flags) override;
  InitReturn PostInit();

  bool GetChartBits(wxRect &source, unsigned char *pPix, int sub_samp) override;

private:
  void AppendPoint(const int &x, const int &y, const int &dir);
  bool TraceOutline(uint8_t *image, int x0, int y0);
  bool IsPixelTransparent(uint8_t *image, const int &x, const int &y,
                          bool fix_borders = false);
  bool FindEdge(uint8_t *image, int &x, int &y);
  bool CornerLatLon(GTIF *gtif, GTIFDefn *defn, double &x, double &y,
                    float &lat, float &lon);
  TIFF *tif;
  GTIF *gtif;
  uint8_t *im;
  std::vector<std::pair<int, int>> ply_points;
};

#endif
