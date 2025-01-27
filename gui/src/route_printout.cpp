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

#include "dychart.h"
#include "gui_lib.h"
#include "model/navutil_base.h"
#include "model/route.h"
#include "model/track.h"
#include "model/wx28compat.h"
#include "navutil.h"
#include "print_dialog.h"
#include "printtable.h"
#include "route_printout.h"

using namespace std;

RoutePrintout::RoutePrintout(Route* route, const std::set<int>& options)
    : BasePrintout(_("Route Print").ToStdString()), m_route(route) {
  // Offset text from the edge of the cell (Needed on Linux)
  m_text_offset_x = 5;
  m_text_offset_y = 8;

  m_table.StartFillHeader();

  m_table << _("Leg");

  if (GUI::HasKey(options, RoutePrintOptions::kWaypointName)) {
    m_table << _("To Waypoint");
  }
  if (GUI::HasKey(options, RoutePrintOptions::kWaypointPosition)) {
    m_table << _("Position");
  }
  if (GUI::HasKey(options, RoutePrintOptions::kWaypointCourse)) {
    m_table << _("Course");
  }
  if (GUI::HasKey(options, RoutePrintOptions::kWaypointDistance)) {
    m_table << _("Distance");
  }
  if (GUI::HasKey(options, RoutePrintOptions::kWaypointDescription)) {
    m_table << _("Description");
  }

  // setup widths for columns
  m_table.StartFillWidths();
  m_table << 20;

  if (GUI::HasKey(options, RoutePrintOptions::kWaypointName)) {
    m_table << 40;
  }
  if (GUI::HasKey(options, RoutePrintOptions::kWaypointPosition)) {
    m_table << 40;
  }
  if (GUI::HasKey(options, RoutePrintOptions::kWaypointCourse)) {
    m_table << 40;
  }
  if (GUI::HasKey(options, RoutePrintOptions::kWaypointDistance)) {
    m_table << 80;
  }
  if (GUI::HasKey(options, RoutePrintOptions::kWaypointDescription)) {
    m_table << 100;
  }

  m_table.StartFillData();

  for (int n = 1; n <= m_route->GetnPoints(); n++) {
    RoutePoint* point = m_route->GetPoint(n);
    if (NULL == point) continue;

    if (n > 1) {
      m_table << n - 1;
    } else {
      m_table << "---";
    }

    if (GUI::HasKey(options, RoutePrintOptions::kWaypointName)) {
      m_table << point->GetName();
    }
    if (GUI::HasKey(options, RoutePrintOptions::kWaypointPosition)) {
      std::wostringstream point_position;
      point_position << toSDMM(1, point->m_lat, false) << "\n"
                     << toSDMM(2, point->m_lon, false);
      m_table << point_position.str();
    }
    if (GUI::HasKey(options, RoutePrintOptions::kWaypointCourse)) {
      if (n > 1) {
        m_table << formatAngle(point->GetCourse());
      } else {
        m_table << "---";
      }
    }
    if (GUI::HasKey(options, RoutePrintOptions::kWaypointDistance)) {
      if (n > 1) {
        std::ostringstream point_distance;
        point_distance << std::fixed << std::setprecision(2) << std::setw(6)
                       << toUsrDistance(point->GetDistance())
                       << getUsrDistanceUnit();
        m_table << point_distance.str();
      } else {
        m_table << "---";
      }
    }
    if (GUI::HasKey(options, RoutePrintOptions::kWaypointDescription)) {
      m_table << point->GetDescription();
    }
    m_table << "\n";
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
  int max_x = wxMin(w, 1000);
  int max_y = wxMin(h, 1000);

  // Calculate a suitable scaling factor
  double scale_x = (double)(w / max_x);
  double scale_y = (double)(h / max_y);

  // Use x or y scaling factor, whichever fits on the DC
  double actual_scale = wxMin(scale_x, scale_y);

  // Set the scale and origin
  dc->SetUserScale(actual_scale, actual_scale);
  dc->SetDeviceOrigin((long)m_margin_x, (long)m_margin_y);

  m_table.AdjustCells(dc, m_margin_x, m_margin_y);
  m_pages = m_table.GetNumberPages();
}

void RoutePrintout::DrawPage(wxDC* dc, int page) {
  wxFont routePrintFont_bold(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                             wxFONTWEIGHT_BOLD);
  dc->SetFont(routePrintFont_bold);
  wxBrush brush(wxColour(255, 255, 255), wxBRUSHSTYLE_TRANSPARENT);
  dc->SetBrush(brush);

  int header_text_offset_x = 2;
  int header_text_offset_y = 2;

  dc->DrawText(m_route->m_RouteNameString, 150, 20);

  int current_x = m_margin_x;
  int current_y = m_margin_y;
  vector<PrintCell>& header_content = m_table.GetHeader();
  for (size_t j = 0; j < header_content.size(); j++) {
    PrintCell& cell = header_content[j];
    dc->DrawRectangle(current_x, current_y, cell.GetWidth(), cell.GetHeight());
    dc->DrawText(cell.GetText(), current_x + header_text_offset_x,
                 current_y + header_text_offset_y);
    current_x += cell.GetWidth();
  }

  wxFont routePrintFont_normal(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                               wxFONTWEIGHT_NORMAL);
  dc->SetFont(routePrintFont_normal);

  vector<vector<PrintCell> >& cells = m_table.GetContent();
  current_y = m_margin_y + m_table.GetHeaderHeight();
  int current_height = 0;
  for (size_t i = 0; i < cells.size(); i++) {
    vector<PrintCell>& content_row = cells[i];
    current_x = m_margin_x;
    for (size_t j = 0; j < content_row.size(); j++) {
      PrintCell& cell = content_row[j];
      if (cell.GetPage() == page) {
        wxRect r(current_x, current_y, cell.GetWidth(), cell.GetHeight());
        dc->DrawRectangle(r);
        r.Offset(m_text_offset_x, m_text_offset_y);
        dc->DrawLabel(cell.GetText(), r);
        current_x += cell.GetWidth();
        current_height = cell.GetHeight();
      }
    }
    current_y += current_height;
  }
}
