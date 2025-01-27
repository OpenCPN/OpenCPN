/***************************************************************************
 *   Copyright (C) 2017 by David S. Register                               *
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
using namespace std;

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

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
#include "dychart.h"
#include "ocpn_frame.h"

#ifdef __WXMSW__
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <psapi.h>
#endif

#include "gui_lib.h"
#include "model/track.h"
#include "print_dialog.h"
#include "printtable.h"
#include "track_printout.h"

enum { PRINT_POSITION, PRINT_DISTANCE, PRINT_BEARING, PRINT_TIME, PRINT_SPEED };

TrackPrintout::TrackPrintout(Track* track, OCPNTrackListCtrl* lcPoints,
                             GUI::KeySet options)
    : BasePrintout(_("Track Print").ToStdString()), m_track(track) {
  // Offset text from the edge of the cell (Needed on Linux)
  textOffsetX = 5;
  textOffsetY = 8;

  table.StartFillHeader();
  // setup widths for columns

  table << (const char*)wxString(_("Leg")).mb_str();

  if (options.hasKey(TrackPrintOptions::TrackPosition)) {
    table << (const char*)wxString(_("Position")).mb_str();
  }
  if (options.hasKey(TrackPrintOptions::TrackCourse)) {
    table << (const char*)wxString(_("Course")).mb_str();
  }
  if (options.hasKey(TrackPrintOptions::TrackDistance)) {
    table << (const char*)wxString(_("Distance")).mb_str();
  }
  if (options.hasKey(TrackPrintOptions::TrackTime)) {
    table << (const char*)wxString(_("Time")).mb_str();
  }
  if (options.hasKey(TrackPrintOptions::TrackSpeed)) {
    table << (const char*)wxString(_("Speed")).mb_str();
  }

  table.StartFillWidths();

  table << 20;  // "Leg" column
  // setup widths for columns
  if (options.hasKey(TrackPrintOptions::TrackPosition)) table << 80;
  if (options.hasKey(TrackPrintOptions::TrackCourse)) table << 40;
  if (options.hasKey(TrackPrintOptions::TrackDistance)) table << 40;
  if (options.hasKey(TrackPrintOptions::TrackTime)) table << 60;
  if (options.hasKey(TrackPrintOptions::TrackSpeed)) table << 40;

  table.StartFillData();
  for (int n = 0; n <= m_track->GetnPoints(); n++) {
    table << lcPoints->OnGetItemText(n, 0);  // leg

    if (options.hasKey(TrackPrintOptions::TrackPosition)) {
      // lat + lon
      wxString pos = lcPoints->OnGetItemText(n, 3) + _T(" ") +
                     lcPoints->OnGetItemText(n, 4);
      table << pos;
    }
    if (options.hasKey(TrackPrintOptions::TrackCourse))
      table << lcPoints->OnGetItemText(n, 2);  // bearing
    if (options.hasKey(TrackPrintOptions::TrackDistance))
      table << lcPoints->OnGetItemText(n, 1);  // distance
    if (options.hasKey(TrackPrintOptions::TrackTime))
      table << lcPoints->OnGetItemText(n, 5);  // time
    if (options.hasKey(TrackPrintOptions::TrackSpeed))
      table << lcPoints->OnGetItemText(n, 6);  // speed
    table << "\n";
  }
}

void TrackPrintout::OnPreparePrinting() {
  wxDC* dc = GetDC();
  wxFont trackPrintFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_NORMAL);
  dc->SetFont(trackPrintFont);

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

void TrackPrintout::DrawPage(wxDC* dc, int page) {
  wxFont trackPrintFont_bold(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                             wxFONTWEIGHT_BOLD);
  dc->SetFont(trackPrintFont_bold);
  wxBrush brush(wxColour(255, 255, 255), wxBRUSHSTYLE_TRANSPARENT);
  dc->SetBrush(brush);

  int header_textOffsetX = 2;
  int header_textOffsetY = 2;

  dc->DrawText(m_track->GetName(), 150, 20);

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

  wxFont trackPrintFont_normal(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                               wxFONTWEIGHT_NORMAL);
  dc->SetFont(trackPrintFont_normal);

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
