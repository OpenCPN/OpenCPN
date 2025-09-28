/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Route drawing stuff
 */

#ifndef _ROUTE_GUI_H
#define _ROUTE_GUI_H

#include <wx/gdicmn.h>
#include <wx/dc.h>
#include <wx/string.h>

#include "model/route.h"

#include "bbox.h"
#include "chcanv.h"
#include "ocpndc.h"
#include "SendToGpsDlg.h"
#include "viewport.h"

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

private:
  Route &m_route;
};

#endif  // _ROUTE_GUI_H
