/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Track and Trackpoint drawing stuff
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#ifndef _ROUTE_POINT_GUI_H
#define _ROUTE_POINT_GUI_H

#include <wx/gdicmn.h>

#include "chcanv.h"
#include "ocpndc.h"
#include "model/route_point.h"
#include "SendToGpsDlg.h"
#include "viewport.h"

class RoutePointGui {
public:
  RoutePointGui(RoutePoint& point) : m_point(point) { /*ReLoadIcon();*/ }

  void Draw(ocpnDC& dc, ChartCanvas* canvas, wxPoint* rpn = 0,
            bool boverride_viz = false);
  void CalculateDCRect(wxDC &dc, ChartCanvas *canvas, wxRect *prect);
  bool IsVisibleSelectable(ChartCanvas *canvas, bool boverrideViz = false);
  wxPoint2DDouble GetDragHandlePoint(ChartCanvas *canvas);
  void SetPointFromDraghandlePoint(ChartCanvas *canvas, double lat, double lon);
  void SetPointFromDraghandlePoint(ChartCanvas *canvas, int x, int y);
  void PresetDragOffset(ChartCanvas *canvas, int x, int y);
  void ShowScaleWarningMessage(ChartCanvas *canvas);

  bool SendToGPS(const wxString &com_name, SendToGpsDlg *dialog);
  void ReLoadIcon(void);
  void EnableDragHandle(bool bEnable);
  int GetIconImageIndex();
  wxBitmap* GetIconBitmap() {
    if (m_point.m_IconIsDirty) ReLoadIcon();
    return m_point.m_pbmIcon;
  }

#ifdef ocpnUSE_GL
  void DrawGL(ViewPort &vp, ChartCanvas* canvas, ocpnDC &dc,
              bool use_cached_screen_coords = false, bool vizOverride = false);
#endif

private:
  wxPoint2DDouble computeDragHandlePoint(ChartCanvas *canvas);
  RoutePoint& m_point;
};


#endif   // _ROUTE_POINT_GUI_H
