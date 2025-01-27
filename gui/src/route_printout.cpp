/***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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

#include <iostream>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers
#ifdef __WXMSW__
// #include "c:\\Program Files\\visual leak detector\\include\\vld.h"
#endif

#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/artprov.h>
#include <wx/stdpaths.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/aui/aui.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/brush.h>
#include <wx/colour.h>

#include <wx/dialog.h>

#include "navutil.h"
#include "dychart.h"

#ifdef __WXMSW__
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <psapi.h>
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>
#endif
#include "route_printout.h"

#include "gui_lib.h"
#include "model/navutil_base.h"
#include "model/route.h"
#include "model/track.h"
#include "model/wx28compat.h"
#include "print_dialog.h"
#include "printtable.h"

using namespace std;

RoutePrintout::RoutePrintout(Route* route, GUI::KeySet options)
    : BasePrintout(_("Route Print").ToStdString()), m_route(route) {
  // Offset text from the edge of the cell (Needed on Linux)
  textOffsetX = 5;
  textOffsetY = 8;

  table.StartFillHeader();
  // setup widths for columns

  table << _("Leg");

  if (options.hasKey(RoutePrintOptions::WaypointName)) {
    table << _("To Waypoint");
  }
  if (options.hasKey(RoutePrintOptions::WaypointPosition)) {
    table << _("Position");
  }
  if (options.hasKey(RoutePrintOptions::WaypointCourse)) {
    table << _("Course");
  }
  if (options.hasKey(RoutePrintOptions::WaypointDistance)) {
    table << _("Distance");
  }
  if (options.hasKey(RoutePrintOptions::WaypointDescription)) {
    table << _("Description");
  }

  table.StartFillWidths();

  table << 20;  // "Leg" column

  // setup widths for columns
  if (options.hasKey(RoutePrintOptions::WaypointName)) {
    table << 40;
  }
  if (options.hasKey(RoutePrintOptions::WaypointPosition)) {
    table << 40;
  }
  if (options.hasKey(RoutePrintOptions::WaypointCourse)) {
    table << 40;
  }
  if (options.hasKey(RoutePrintOptions::WaypointDistance)) {
    table << 80;
  }
  if (options.hasKey(RoutePrintOptions::WaypointDescription)) {
    table << 100;
  }

  table.StartFillData();

  for (int n = 1; n <= m_route->GetnPoints(); n++) {
    RoutePoint* point = m_route->GetPoint(n);

    RoutePoint* pointm1 = NULL;
    if (n - 1 >= 0) pointm1 = m_route->GetPoint(n - 1);

    if (NULL == point) continue;

    wxString leg = _T("---");
    if (n > 1) leg.Printf(_T("%d"), n - 1);

    string cell(leg.mb_str());

    table << cell;

    if (options.hasKey(RoutePrintOptions::WaypointName)) {
      string cell(point->GetName().mb_str());
      table << cell;
    }
    if (options.hasKey(RoutePrintOptions::WaypointPosition)) {
      wxString point_position = toSDMM(1, point->m_lat, false) + _T( "\n" ) +
                                toSDMM(2, point->m_lon, false);
      string cell(point_position.mb_str());
      table << cell;
    }
    if (options.hasKey(RoutePrintOptions::WaypointCourse)) {
      wxString point_course = "---";
      if (pointm1) {
        point_course = formatAngle(point->GetCourse());
      }
      table << point_course;
    }
    if (options.hasKey(RoutePrintOptions::WaypointDistance)) {
      wxString point_distance = _T("---");
      if (n > 1)
        point_distance.Printf(_T("%6.2f" + getUsrDistanceUnit()),
                              toUsrDistance(point->GetDistance()));
      table << point_distance;
    }
    if (options.hasKey(RoutePrintOptions::WaypointDescription)) {
      table << point->GetDescription();
    }
    table << "\n";
  }
}

void RoutePrintout::OnPreparePrinting() {
  wxDC* dc = GetDC();
  wxFont routePrintFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_NORMAL);
  dc->SetFont(routePrintFont);

  // Get the size of the DC in pixels
  int w, h;
  dc->GetSize(&w, &h);

  // We don't know before hand what size the Print DC will be, in pixels. Varies
  // by host. So, if the dc size is greater than 1000 pixels, we scale
  // accordinly.
  int maxX = wxMin(w, 1000);
  int maxY = wxMin(h, 1000);

  // Calculate a suitable scaling factor
  double scaleX = (double)(w / maxX);
  double scaleY = (double)(h / maxY);

  // Use x or y scaling factor, whichever fits on the DC
  double actualScale = wxMin(scaleX, scaleY);

  // Set the scale and origin
  dc->SetUserScale(actualScale, actualScale);
  dc->SetDeviceOrigin((long)m_margin_x, (long)m_margin_y);

  table.AdjustCells(dc, m_margin_x, m_margin_y);
  m_pages = table.GetNumberPages();
}

void RoutePrintout::DrawPage(wxDC* dc, int page) {
  wxFont routePrintFont_bold(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                             wxFONTWEIGHT_BOLD);
  dc->SetFont(routePrintFont_bold);
  wxBrush brush(wxColour(255, 255, 255), wxBRUSHSTYLE_TRANSPARENT);
  dc->SetBrush(brush);

  int header_textOffsetX = 2;
  int header_textOffsetY = 2;

  dc->DrawText(m_route->m_RouteNameString, 150, 20);

  int currentX = m_margin_x;
  int currentY = m_margin_y;
  vector<PrintCell>& header_content = table.GetHeader();
  for (size_t j = 0; j < header_content.size(); j++) {
    PrintCell& cell = header_content[j];
    dc->DrawRectangle(currentX, currentY, cell.GetWidth(), cell.GetHeight());
    dc->DrawText(cell.GetText(), currentX + header_textOffsetX,
                 currentY + header_textOffsetY);
    currentX += cell.GetWidth();
  }

  wxFont routePrintFont_normal(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                               wxFONTWEIGHT_NORMAL);
  dc->SetFont(routePrintFont_normal);

  vector<vector<PrintCell> >& cells = table.GetContent();
  currentY = m_margin_y + table.GetHeaderHeight();
  int currentHeight = 0;
  for (size_t i = 0; i < cells.size(); i++) {
    vector<PrintCell>& content_row = cells[i];
    currentX = m_margin_x;
    for (size_t j = 0; j < content_row.size(); j++) {
      PrintCell& cell = content_row[j];
      if (cell.GetPage() == page) {
        wxRect r(currentX, currentY, cell.GetWidth(), cell.GetHeight());
        dc->DrawRectangle(r);
        r.Offset(textOffsetX, textOffsetY);
        dc->DrawLabel(cell.GetText(), r);
        currentX += cell.GetWidth();
        currentHeight = cell.GetHeight();
      }
    }
    currentY += currentHeight;
  }
}
