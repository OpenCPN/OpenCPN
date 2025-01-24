/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Route printout
 * Author:   Pavel Saviankou, Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2017 by David S. Register                               *
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

#include "trackprintout.h"
#include "printtable.h"
#include "model/track.h"
#include "gui_lib.h"

enum { PRINT_POSITION, PRINT_DISTANCE, PRINT_BEARING, PRINT_TIME, PRINT_SPEED };

// Global print data, to remember settings during the session
extern wxPrintData* g_printData;
// Global page setup data
extern wxPageSetupData* g_pageSetupData;

MyTrackPrintout::MyTrackPrintout(std::vector<bool> _toPrintOut, Track* track,
                                 OCPNTrackListCtrl* lcPoints,
                                 const wxString& title)
    : MyPrintout(title), myTrack(track), toPrintOut(_toPrintOut) {
  // Let's have at least some device units margin
  marginX = 100;
  marginY = 100;

  // Offset text from the edge of the cell (Needed on Linux)
  textOffsetX = 5;
  textOffsetY = 8;

  table.StartFillHeader();
  // setup widths for columns

  table << (const char*)wxString(_("Leg")).mb_str();

  if (toPrintOut[PRINT_POSITION]) {
    table << (const char*)wxString(_("Position")).mb_str();
  }
  if (toPrintOut[PRINT_BEARING]) {
    table << (const char*)wxString(_("Course")).mb_str();
  }
  if (toPrintOut[PRINT_DISTANCE]) {
    table << (const char*)wxString(_("Distance")).mb_str();
  }
  if (toPrintOut[PRINT_TIME]) {
    table << (const char*)wxString(_("Time")).mb_str();
  }
  if (toPrintOut[PRINT_SPEED]) {
    table << (const char*)wxString(_("Speed")).mb_str();
  }

  table.StartFillWidths();

  table << 20;  // "Leg" column
  // setup widths for columns
  if (toPrintOut[PRINT_POSITION]) table << 80;
  if (toPrintOut[PRINT_BEARING]) table << 40;
  if (toPrintOut[PRINT_DISTANCE]) table << 40;
  if (toPrintOut[PRINT_TIME]) table << 60;
  if (toPrintOut[PRINT_SPEED]) table << 40;

  table.StartFillData();
  for (int n = 0; n <= myTrack->GetnPoints(); n++) {
    table << lcPoints->OnGetItemText(n, 0);  // leg

    if (toPrintOut[PRINT_POSITION]) {
      // lat + lon
      wxString pos = lcPoints->OnGetItemText(n, 3) + _T(" ") +
                     lcPoints->OnGetItemText(n, 4);
      table << pos;
    }
    if (toPrintOut[PRINT_BEARING])
      table << lcPoints->OnGetItemText(n, 2);  // bearing
    if (toPrintOut[PRINT_DISTANCE])
      table << lcPoints->OnGetItemText(n, 1);  // distance
    if (toPrintOut[PRINT_TIME]) table << lcPoints->OnGetItemText(n, 5);  // time
    if (toPrintOut[PRINT_SPEED])
      table << lcPoints->OnGetItemText(n, 6);  // speed
    table << "\n";
  }
}

void MyTrackPrintout::GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                                  int* selPageTo) {
  *minPage = 1;
  *maxPage = numberOfPages;
  *selPageFrom = 1;
  *selPageTo = numberOfPages;
}

void MyTrackPrintout::OnPreparePrinting() {
  pageToPrint = 1;
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
  dc->SetDeviceOrigin((long)marginX, (long)marginY);

  table.AdjustCells(dc, marginX, marginY);
  numberOfPages = table.GetNumberPages();
}

bool MyTrackPrintout::OnPrintPage(int page) {
  wxDC* dc = GetDC();
  if (dc) {
    if (page <= numberOfPages) {
      pageToPrint = page;
      DrawPage(dc);
      return true;
    } else
      return false;
  } else
    return false;
}

void MyTrackPrintout::DrawPage(wxDC* dc) {
  wxFont trackPrintFont_bold(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                             wxFONTWEIGHT_BOLD);
  dc->SetFont(trackPrintFont_bold);
  wxBrush brush(wxColour(255, 255, 255), wxBRUSHSTYLE_TRANSPARENT);
  dc->SetBrush(brush);

  int header_textOffsetX = 2;
  int header_textOffsetY = 2;

  dc->DrawText(myTrack->GetName(), 150, 20);

  int currentX = marginX;
  int currentY = marginY;
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
  currentY = marginY + table.GetHeaderHeight();
  int currentHeight = 0;
  for (size_t i = 0; i < cells.size(); i++) {
    vector<PrintCell>& content_row = cells[i];
    currentX = marginX;
    for (size_t j = 0; j < content_row.size(); j++) {
      PrintCell& cell = content_row[j];
      if (cell.GetPage() == pageToPrint) {
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

// ---------- TrackPrintSelection dialof implementation

BEGIN_EVENT_TABLE(TrackPrintSelection, wxDialog)
EVT_BUTTON(ID_TRACKPRINT_SELECTION_CANCEL,
           TrackPrintSelection::OnTrackpropCancelClick)
EVT_BUTTON(ID_TRACKPRINT_SELECTION_OK, TrackPrintSelection::OnTrackpropOkClick)
END_EVENT_TABLE()
TrackPrintSelection::TrackPrintSelection() {}

TrackPrintSelection::TrackPrintSelection(wxWindow* parent, Track* _track,
                                         OCPNTrackListCtrl* lcPoints,
                                         wxWindowID id, const wxString& caption,
                                         const wxPoint& pos, const wxSize& size,
                                         long style) {
  track = _track;
  m_lcPoints = lcPoints;

  long wstyle = style;

  Create(parent, id, caption, pos, size, wstyle);
  Centre();
}

TrackPrintSelection::~TrackPrintSelection() {}

/*!
 * TrackProp creator
 */

bool TrackPrintSelection::Create(wxWindow* parent, wxWindowID id,
                                 const wxString& caption, const wxPoint& pos,
                                 const wxSize& size, long style) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

#ifdef __WXOSX__
  style |= wxSTAY_ON_TOP;
#endif

  wxDialog::Create(parent, id, _("Print Track Selection"), pos, size, style);

  CreateControls();

  return TRUE;
}

/*!
 * Control creation for TrackProp
 */

void TrackPrintSelection::CreateControls() {
  TrackPrintSelection* itemDialog1 = this;
  wxStaticBox* itemStaticBoxSizer3Static =
      new wxStaticBox(itemDialog1, wxID_ANY, _("Elements to print..."));
  wxStaticBoxSizer* itemBoxSizer1 =
      new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer1);

  wxFlexGridSizer* fgSizer2;
  fgSizer2 = new wxFlexGridSizer(5, 2, 0, 0);

  m_checkBoxPosition =
      new wxCheckBox(itemDialog1, wxID_ANY, _("Position"), wxDefaultPosition,
                     wxDefaultSize, wxALIGN_LEFT);
  m_checkBoxPosition->SetValue(true);
  fgSizer2->Add(m_checkBoxPosition, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxStaticText* label2 =
      new wxStaticText(itemDialog1, wxID_ANY, _("Show Waypoint position."),
                       wxDefaultPosition, wxDefaultSize);
  fgSizer2->Add(label2, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_checkBoxCourse =
      new wxCheckBox(itemDialog1, wxID_ANY, _("Course"), wxDefaultPosition,
                     wxDefaultSize, wxALIGN_LEFT);
  m_checkBoxCourse->SetValue(true);
  fgSizer2->Add(m_checkBoxCourse, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxStaticText* label3 =
      new wxStaticText(itemDialog1, wxID_ANY,
                       _("Show course from each Waypoint to the next one."),
                       wxDefaultPosition, wxDefaultSize);
  fgSizer2->Add(label3, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_checkBoxDistance =
      new wxCheckBox(itemDialog1, wxID_ANY, _("Distance"), wxDefaultPosition,
                     wxDefaultSize, wxALIGN_LEFT);
  m_checkBoxDistance->SetValue(true);
  fgSizer2->Add(m_checkBoxDistance, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxStaticText* label4 =
      new wxStaticText(itemDialog1, wxID_ANY,
                       _("Show Distance from each Waypoint to the next one."),
                       wxDefaultPosition, wxDefaultSize);
  fgSizer2->Add(label4, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_checkBoxTime =
      new wxCheckBox(itemDialog1, wxID_ANY, _("Time"), wxDefaultPosition,
                     wxDefaultSize, wxALIGN_LEFT);
  m_checkBoxTime->SetValue(true);
  fgSizer2->Add(m_checkBoxTime, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxStaticText* label5 = new wxStaticText(
      itemDialog1, wxID_ANY, _("Show Time."), wxDefaultPosition, wxDefaultSize);
  fgSizer2->Add(label5, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_checkBoxSpeed =
      new wxCheckBox(itemDialog1, wxID_ANY, _("Speed"), wxDefaultPosition,
                     wxDefaultSize, wxALIGN_LEFT);
  m_checkBoxSpeed->SetValue(true);
  fgSizer2->Add(m_checkBoxSpeed, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxStaticText* label6 =
      new wxStaticText(itemDialog1, wxID_ANY, _("Show Speed."),
                       wxDefaultPosition, wxDefaultSize);
  fgSizer2->Add(label6, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  itemBoxSizer1->Add(fgSizer2, 5, wxEXPAND, 5);

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer1->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton =
      new wxButton(itemDialog1, ID_TRACKPRINT_SELECTION_CANCEL, _("Cancel"),
                   wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0,
                      wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_TRACKPRINT_SELECTION_OK, _("OK"),
                            wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0,
                      wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();

  SetColorScheme((ColorScheme)0);
}

void TrackPrintSelection::SetColorScheme(ColorScheme cs) { DimeControl(this); }

/*
 * Should we show tooltips?
 */

bool TrackPrintSelection::ShowToolTips() { return TRUE; }

void TrackPrintSelection::SetDialogTitle(const wxString& title) {
  SetTitle(title);
}

void TrackPrintSelection::OnTrackpropCancelClick(wxCommandEvent& event) {
  Close();  // Hide();
  event.Skip();
}

void TrackPrintSelection::OnTrackpropOkClick(wxCommandEvent& event) {
  std::vector<bool> toPrintOut;
  toPrintOut.push_back(m_checkBoxPosition->GetValue());
  toPrintOut.push_back(m_checkBoxCourse->GetValue());
  toPrintOut.push_back(m_checkBoxDistance->GetValue());
  toPrintOut.push_back(m_checkBoxTime->GetValue());
  toPrintOut.push_back(m_checkBoxSpeed->GetValue());

  if (NULL == g_printData) {
    g_printData = new wxPrintData;
    g_printData->SetOrientation(wxPORTRAIT);
    g_pageSetupData = new wxPageSetupDialogData;
  }

  MyTrackPrintout* mytrackprintout1 =
      new MyTrackPrintout(toPrintOut, track, m_lcPoints, _("Track Print"));

  wxPrintDialogData printDialogData(*g_printData);
  printDialogData.EnablePageNumbers(true);

  wxPrinter printer(&printDialogData);
  if (!printer.Print(this, mytrackprintout1, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      OCPNMessageBox(NULL,
                     _("There was a problem printing.\nPerhaps your current "
                       "printer is not set correctly?"),
                     _T( "OpenCPN" ), wxOK);
    }
  }

  Close();  // Hide();
  event.Skip();
}
