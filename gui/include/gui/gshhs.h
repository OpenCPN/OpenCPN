/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GSHHS Chart Object (Global Self-consistent, Hierarchical,
 *High-resolution Shoreline) Author:   Jesper Weissglas for the OpenCPN port.
 *
 *           Derived from http://www.zygrib.org/ and
 *http://sourceforge.net/projects/qtvlm/ which has the original copyright:
 *   zUGrib: meteorologic GRIB file data viewer
 *   Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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
 ***************************************************************************
 *
 *
 */

#ifndef GSHHS_H
#define GSHHS_H

#include <stdio.h>
#include <string>
#include <math.h>
#include <assert.h>
#include <vector>

#include <wx/geometry.h>

#include "model/ocpn_types.h"
// #include "ocpndc.h"
#include "viewport.h"
#include "model/cutil.h"
#include "poly_math.h"
#include "color_types.h"

#ifdef __MSVC__
#pragma warning(disable : 4251)  // relates to std::string fpath
#endif

//-------------------------------------------------------------------------
// Subset of original Projection, only whats needed for GSHHS.

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif

//-------------------------------------------------------------------------
class wxLineF {
public:
  wxLineF(double x1, double y1, double x2, double y2) {
    m_p1 = wxRealPoint(x1, y1);
    m_p2 = wxRealPoint(x2, y2);
  }
  wxRealPoint &p1() { return m_p1; }
  wxRealPoint &p2() { return m_p2; }
  wxRealPoint m_p1, m_p2;
};

struct PolygonFileHeader {
  int version;  // Version number of the GSHHS data format
  int pasx;     // Longitude step size for cells (1/10th degree)
  int pasy;     // Latitude step size for cells (1/10th degree)
  // xmin, ymin, xmax, ymax define the bounding box of the entire grid in 1/10th
  // of a degree. This allows for quick determination of whether a given point
  // is within the covered area.
  int xmin;  // Minimum longitude of the grid (1/10th degree)
  int ymin;  // Minimum latitude of the grid (1/10th degree)
  int xmax;  // Maximum longitude of the grid (1/10th degree)
  int ymax;  // Maximum latitude of the grid (1/10th degree)
  int p1;    // Spare (reserved for future use)
  int p2;    // Spare (reserved for future use)
  int p3;    // Spare (reserved for future use)
  int p4;    // Spare (reserved for future use)
  int p5;    // Spare (reserved for future use)
};

typedef std::vector<wxRealPoint> contour;
typedef std::vector<contour> contour_list;
#define GSSH_SUBM 16  // divide each cell to 16x16 sub cells

//==========================================================================

class GshhsPolyCell {
public:
  GshhsPolyCell(FILE *fpoly, int x0, int y0, PolygonFileHeader *header);
  ~GshhsPolyCell();

  void ClearPolyV();

  void drawMapPlain(ocpnDC &pnt, double dx, ViewPort &vp, wxColor seaColor,
                    wxColor landColor, bool idl);

  void drawSeaBorderLines(ocpnDC &pnt, double dx, ViewPort &vp);
  std::vector<wxLineF> *getCoasts() { return &coasts; }
  contour_list &getPoly1() { return poly1; }

  /* we remap the segments into a high resolution map to
     greatly reduce intersection testing time */
  std::vector<wxLineF> *high_res_map[GSSH_SUBM * GSSH_SUBM];

private:
  int nbpoints;
  int x0cell, y0cell;

  FILE *fpoly;

  std::vector<wxLineF> coasts;
  PolygonFileHeader *header;
  contour_list poly1, poly2, poly3, poly4, poly5;

  // used for opengl vertex cache
  float_2Dpt *polyv[6];
  int polyc[6];

  void DrawPolygonFilled(ocpnDC &pnt, contour_list *poly, double dx,
                         ViewPort &vp, wxColor const &color);
#ifdef ocpnUSE_GL
  void DrawPolygonFilledGL(ocpnDC &pnt, contour_list *p, float_2Dpt **pv,
                           int *pvc, ViewPort &vp, wxColor const &color,
                           bool idl);
#endif
  void DrawPolygonContour(ocpnDC &pnt, contour_list *poly, double dx,
                          ViewPort &vp);

  void ReadPoly(contour_list &poly);
  void ReadPolygonFile();
};

class GshhsPolyReader {
public:
  GshhsPolyReader(int quality);
  ~GshhsPolyReader();

  void drawGshhsPolyMapPlain(ocpnDC &pnt, ViewPort &vp, wxColor const &seaColor,
                             wxColor const &landColor);

  void drawGshhsPolyMapSeaBorders(ocpnDC &pnt, ViewPort &vp);

  void InitializeLoadQuality(int quality);  // 5 levels: 0=low ... 4=full
  bool crossing1(wxLineF trajectWorld);
  int currentQuality;
  int ReadPolyVersion();
  int GetPolyVersion() { return polyHeader.version; }

private:
  FILE *fpoly;
  GshhsPolyCell *allCells[360][180];

  PolygonFileHeader polyHeader;
  void readPolygonFileHeader(FILE *polyfile, PolygonFileHeader *header);

  wxMutex mutex1, mutex2;

  ViewPort last_rendered_vp;
};

// GSHHS file format:
//
// int id;           /* Unique polygon id number, starting at 0 */
// int n;            /* Number of points in this polygon */
// int flag;             /* level + version << 8 + greenwich << 16 + source <<
// 24 */ int west, east, south, north; /* min/max extent in micro-degrees */ int
// area;             /* Area of polygon in 1/10 km^2 */
//
// Here, level, version, greenwhich, and source are
// level:   1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake
// version: Set to 4 for GSHHS version 1.4
// greenwich:   1 if Greenwich is crossed
// source:  0 = CIA WDBII, 1 = WVS

//==========================================================
class GshhsPoint {
public:
  double lon, lat;  // longitude, latitude
  GshhsPoint(double lo, double la) {
    lon = lo;
    lat = la;
  }
};

//==========================================================

class GshhsPolygon {
public:
  GshhsPolygon(FILE *file);

  virtual ~GshhsPolygon();

  int getLevel() { return flag & 255; }
  int isGreenwich() { return greenwich; }
  int isAntarctic() { return antarctic; }
  bool isOk() { return ok; }
  int readInt4();
  int readInt2();

  int id;   /* Unique polygon id number, starting at 0 */
  int n;    /* Number of points in this polygon */
  int flag; /* level + version << 8 + greenwich << 16 + source << 24 */
  double west, east, south, north; /* min/max extent in DEGREES */
  int area;                        /* Area of polygon in 1/10 km^2 */
  int areaFull, container, ancestor;

  std::vector<GshhsPoint *> lsPoints;

protected:
  FILE *file;
  bool ok;
  bool greenwich, antarctic;
};

//==========================================================

class GshhsReader {
public:
  GshhsReader();
  ~GshhsReader();

  void drawContinents(ocpnDC &pnt, ViewPort &vp, wxColor const &seaColor,
                      wxColor const &landColor);

  void drawSeaBorders(ocpnDC &pnt, ViewPort &vp);
  void drawBoundaries(ocpnDC &pnt, ViewPort &vp);
  void drawRivers(ocpnDC &pnt, ViewPort &vp);

  int GetPolyVersion() { return gshhsPoly_reader->GetPolyVersion(); }

  static wxString getNameExtension(int quality);
  static wxString getFileName_boundaries(int quality);
  static wxString getFileName_rivers(int quality);
  static wxString getFileName_Land(int quality);
  static bool gshhsFilesExists(int quality);

  int getQuality() { return quality; }

  //    bool crossing( wxLineF traject, wxLineF trajectWorld ) const;
  bool crossing1(wxLineF trajectWorld);
  // Open the polygon file, read and return the version from the header.
  int ReadPolyVersion();
  bool qualityAvailable[6];

  void LoadQuality(int quality);
  int GetMinAvailableQuality() { return minQualityAvailable; }
  int GetMaxAvailableQuality() { return maxQualityAvailable; }

private:
  int quality;  // 5 levels: 0=low ... 4=full
  int selectBestQuality(void);
  /**
   * @brief Selects the best available quality level for GSHHS data based on the
   * scale in the ViewPort.
   *
   * This function determines the optimal quality level for GSHHS data based on
   * the viewport scale. It considers both the desired quality at the current
   * scale in the ViewPort and the actual availability of quality levels.
   *
   * @param vp The current ViewPort, containing information about the current
   * chart scale display.
   * @return int The selected quality level, ranging from 0 (lowest) to 4
   * (highest). Returns the best available quality if the ideal quality is not
   * available. Returns -1 if no quality level is available at all.
   */
  int selectBestQuality(ViewPort &vp);

  int maxQualityAvailable;
  int minQualityAvailable;

  std::string fpath;  // directory containing gshhs files

  GshhsPolyReader *gshhsPoly_reader;

  std::vector<GshhsPolygon *> *lsPoly_boundaries[5];
  std::vector<GshhsPolygon *> *lsPoly_rivers[5];

  std::vector<GshhsPolygon *> &getList_boundaries();
  std::vector<GshhsPolygon *> &getList_rivers();
  //-----------------------------------------------------

  int GSHHS_scaledPoints(GshhsPolygon *pol, wxPoint *pts, double decx,
                         ViewPort &vp);

  void GsshDrawLines(ocpnDC &pnt, std::vector<GshhsPolygon *> &lst,
                     ViewPort &vp, bool isClosed);
  void clearLists();
};

inline bool GshhsReader::crossing1(wxLineF trajectWorld) {
  return this->gshhsPoly_reader->crossing1(trajectWorld);
}
#define GSHHS_SCL 1.0e-6 /* Convert micro-degrees to degrees */

//-------------------------------------------------------------------------------

class GSHHSChart {
public:
  GSHHSChart();
  ~GSHHSChart();
  void SetColorScheme(ColorScheme scheme);
  void RenderViewOnDC(ocpnDC &dc, ViewPort &VPoint);
  void Reset();
  void SetColorsDirect(wxColour newLand, wxColour newWater);

  wxColor land;
  wxColor water;
  int GetMinAvailableQuality();
  int GetMaxAvailableQuality();

private:
  GshhsReader *reader;
};

/*
 * Create a GSHHS singleton for detecting land crossing and load the data
 * from GSHHS files on disk.
 */
void gshhsCrossesLandInit();
/*
 * Reset the GSHHS singleton and reload the data from the GSHHS files on disk.
 * For example, this should be invoked when new GSHHS files have been installed.
 */
void gshhsCrossesLandReset();
/*
 * Detects if the rectangle specified with the coordinates crosses land.
 * The best available quality is used, regardless of the chart scale.
 */
bool gshhsCrossesLand(double lat1, double lon1, double lat2, double lon2);

#endif
