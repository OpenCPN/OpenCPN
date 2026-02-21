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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement track_printout.h -- track print dialog
 */

#include <iostream>

#ifdef __WXMSW__
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <psapi.h>
#endif

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "gl_headers.h"  // Must be included before anything using GL stuff

#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/brush.h>
#include <wx/colour.h>
#include <wx/dialog.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/printdlg.h>
#include <wx/print.h>
#include <wx/progdlg.h>
#include <wx/stdpaths.h>

#include "track_printout.h"

#include "model/track.h"

#include "dychart.h"
#include "gui_lib.h"
#include "print_dialog.h"
#include "printtable.h"

using namespace std;

TrackPrintout::TrackPrintout(Track* track, OCPNTrackListCtrl* lcPoints,
                             std::set<int> options)
    : BasePrintout(_("Track Print").ToStdString()), m_track(track) {
  // Offset text from the edge of the cell (Needed on Linux)
  m_text_offset_x = 5;
  m_text_offset_y = 8;

  m_table.StartFillHeader();
  // setup widths for columns

  m_table << (const char*)wxString(_("Leg")).mb_str();

  if (GUI::HasKey(options, TrackPrintOptions::kTrackPosition)) {
    m_table << (const char*)wxString(_("Position")).mb_str();
  }
  if (GUI::HasKey(options, TrackPrintOptions::kTrackCourse)) {
    m_table << (const char*)wxString(_("Course")).mb_str();
  }
  if (GUI::HasKey(options, TrackPrintOptions::kTrackDistance)) {
    m_table << (const char*)wxString(_("Distance")).mb_str();
  }
  if (GUI::HasKey(options, TrackPrintOptions::kTrackTime)) {
    m_table << (const char*)wxString(_("Time")).mb_str();
  }
  if (GUI::HasKey(options, TrackPrintOptions::kTrackSpeed)) {
    m_table << (const char*)wxString(_("Speed")).mb_str();
  }

  m_table.StartFillWidths();

  m_table << 20;  // "Leg" column
  // setup widths for columns
  if (GUI::HasKey(options, TrackPrintOptions::kTrackPosition)) m_table << 80;
  if (GUI::HasKey(options, TrackPrintOptions::kTrackCourse)) m_table << 40;
  if (GUI::HasKey(options, TrackPrintOptions::kTrackDistance)) m_table << 40;
  if (GUI::HasKey(options, TrackPrintOptions::kTrackTime)) m_table << 60;
  if (GUI::HasKey(options, TrackPrintOptions::kTrackSpeed)) m_table << 40;

  m_table.StartFillData();
  for (int n = 0; n < m_track->GetnPoints(); n++) {
    m_table << lcPoints->OnGetItemText(n, 0);  // leg

    if (GUI::HasKey(options, TrackPrintOptions::kTrackPosition)) {
      m_table << lcPoints->OnGetItemText(n, 3) + _(" ") +
                     lcPoints->OnGetItemText(n, 4);  // position
    }
    if (GUI::HasKey(options, TrackPrintOptions::kTrackCourse))
      m_table << lcPoints->OnGetItemText(n, 2);  // bearing
    if (GUI::HasKey(options, TrackPrintOptions::kTrackDistance))
      m_table << lcPoints->OnGetItemText(n, 1);  // distance
    if (GUI::HasKey(options, TrackPrintOptions::kTrackTime))
      m_table << lcPoints->OnGetItemText(n, 5);  // time
    if (GUI::HasKey(options, TrackPrintOptions::kTrackSpeed))
      m_table << lcPoints->OnGetItemText(n, 6);  // speed
    m_table << "\n";
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

void TrackPrintout::DrawPage(wxDC* dc, int page) {
  wxFont trackPrintFont_bold(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                             wxFONTWEIGHT_BOLD);
  dc->SetFont(trackPrintFont_bold);
  wxBrush brush(wxColour(255, 255, 255), wxBRUSHSTYLE_TRANSPARENT);
  dc->SetBrush(brush);

  int header_text_offset_x = 2;
  int header_text_offset_y = 2;

  dc->DrawText(m_track->GetName(), 150, 20);

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

  wxFont trackPrintFont_normal(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                               wxFONTWEIGHT_NORMAL);
  dc->SetFont(trackPrintFont_normal);

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
