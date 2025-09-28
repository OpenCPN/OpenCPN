
/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Route drawing stuff
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
 *   Copyright (C) 2025 by NoCodeHummel                                    *
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
 **************************************************************************/

#ifndef _ROUTE_GUI_H
#define _ROUTE_GUI_H

#include <wx/gdicmn.h>
#include <wx/dc.h>

#include "bbox.h"
#include "chcanv.h"
#include "ocpndc.h"
#include "model/route.h"
#include "viewport.h"
#include "SendToGpsDlg.h"

class RouteGui {
public:
  RouteGui(Route &route) : m_route(route) {}
  virtual void Draw(ocpnDC &dc, ChartCanvas *canvas, const LLBBox &box);
  void DrawPointWhich(ocpnDC &dc, ChartCanvas *canvas, int iPoint,
                      wxPoint *rpn);
  void DrawSegment(ocpnDC &dc, ChartCanvas *canvas, wxPoint *rp1, wxPoint *rp2,
                   ViewPort &vp, bool bdraw_arrow);

  void DrawGLLines(ViewPort &vp, ocpnDC *dc, ChartCanvas *canvas);
  void DrawGL(ViewPort &vp, ChartCanvas *canvas, ocpnDC &dc);
  void DrawGLRouteLines(ViewPort &vp, ChartCanvas *canvas, ocpnDC &dc);
  void CalculateDCRect(wxDC &dc_route, ChartCanvas *canvas, wxRect *prect);
  void RenderSegment(ocpnDC &dc, int xa, int ya, int xb, int yb, ViewPort &vp,
                     bool bdraw_arrow, int hilite_width = 0);
  void RenderSegmentArrowsGL(ocpnDC &dc, int xa, int ya, int xb, int yb,
                             ViewPort &vp);
  int SendToGPS(const wxString &com_name, bool bsend_waypoints,
                SendToGpsDlg *dialog);
  static bool OnDelete(wxWindow *parent, const int count = 0);

  /**
   * Draw the expected boat position on this route at the specified time.
   *
   * This method calculates where a boat would be positioned along the route
   * at the given timestamp, based on the route's planned departure time and
   * waypoint ETAs/ETDs, then renders a position marker on the chart.
   *
   * @param dc Device context for drawing
   * @param canvas Chart canvas for coordinate transformations
   * @param timestamp Time for which to calculate and draw position
   * @param positionColor Color for the position marker (default: red)
   * @return true if position was calculated and drawn, false otherwise
   */
  bool DrawPositionAtTime(ocpnDC &dc, ChartCanvas *canvas,
                          const wxDateTime &timestamp,
                          const wxColour &positionColor = wxColour(255, 0, 0));

  /**
   * Draw a cursor position marker for route rollover tooltip.
   * Shows only crosshairs to indicate the exact position used for "arrival at
   * cursor" calculation.
   *
   * @param dc Device context for drawing
   * @param canvas Chart canvas for coordinate transformations
   * @param lat Latitude of cursor position
   * @param lon Longitude of cursor position
   * @param color Color for the crosshairs (default: yellow)
   */
  void DrawCursorPositionMarker(ocpnDC &dc, ChartCanvas *canvas, double lat,
                                double lon,
                                const wxColour &color = wxColour(255, 255, 0));

private:
  Route &m_route;

  /**
   * Draw a boat position marker at the specified lat/lon coordinates.
   *
   * @param dc Device context for drawing
   * @param canvas Chart canvas for coordinate transformations
   * @param lat Latitude of position marker
   * @param lon Longitude of position marker
   * @param color Color for the marker
   * @param showCircle Whether to draw the circle (default: true)
   * @param showCrosshairs Whether to draw crosshairs (default: true)
   * @param markerStyle Style of the marker (0=both, 1=circle only, 2=crosshairs
   * only)
   */
  void DrawBoatPositionMarker(ocpnDC &dc, ChartCanvas *canvas, double lat,
                              double lon, const wxColour &color,
                              bool showCircle = true,
                              bool showCrosshairs = true, int markerStyle = 0);
};

#endif  // _ROUTE_GUI_H
