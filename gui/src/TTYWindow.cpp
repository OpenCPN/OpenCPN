/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 */

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/bmpbuttn.h>
#include <wx/dcmemory.h>
#include <wx/dialog.h>
#include <wx/settings.h>
#include <wx/dcscreen.h>

#include "TTYWindow.h"
#include "TTYScroll.h"
#include "WindowDestroyListener.h"
#include "color_handler.h"
#include "ocpn_plugin.h"
#include "FontMgr.h"

IMPLEMENT_DYNAMIC_CLASS(TTYWindow, wxFrame)

BEGIN_EVENT_TABLE(TTYWindow, wxFrame)
EVT_CLOSE(TTYWindow::OnCloseWindow)
END_EVENT_TABLE()

TTYWindow::TTYWindow() : m_window_destroy_listener(NULL), m_tty_scroll(NULL) {}

TTYWindow::TTYWindow(wxWindow* parent, int n_lines,
                     WindowDestroyListener* listener)
    : m_window_destroy_listener(listener), m_tty_scroll(NULL) {
  wxFrame::Create(
      parent, -1, _T("Title"), wxDefaultPosition, wxDefaultSize,
      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxFRAME_FLOAT_ON_PARENT);

  wxBoxSizer* bSizerOuterContainer = new wxBoxSizer(wxVERTICAL);
  SetSizer(bSizerOuterContainer);

  m_filter = new wxTextCtrl(this, wxID_ANY);

  m_tty_scroll = new TTYScroll(this, n_lines, *m_filter);
  m_tty_scroll->Scroll(-1, 1000);  // start with full scroll down

  bSizerOuterContainer->Add(m_tty_scroll, 1, wxEXPAND, 5);

  wxStaticBox* psbf = new wxStaticBox(this, wxID_ANY, _("Filter"));
  wxStaticBoxSizer* sbSizer2 = new wxStaticBoxSizer(psbf, wxVERTICAL);
  sbSizer2->Add(m_filter, 1, wxALL | wxEXPAND, 5);
  bSizerOuterContainer->Add(sbSizer2, 0, wxEXPAND, 5);

  wxBoxSizer* bSizerBottomContainer = new wxBoxSizer(wxHORIZONTAL);
  bSizerOuterContainer->Add(bSizerBottomContainer, 0, wxEXPAND, 5);

  wxStaticBox* psb = new wxStaticBox(this, wxID_ANY, _("Legend"));
  wxStaticBoxSizer* sbSizer1 = new wxStaticBoxSizer(psb, wxVERTICAL);
  bSizerBottomContainer->Add(sbSizer1, 0, wxALIGN_LEFT | wxALL, 5);

  CreateLegendBitmap();
  wxBitmapButton* bb = new wxBitmapButton(this, wxID_ANY, m_bm_legend);
  sbSizer1->Add(bb, 1, wxALL | wxEXPAND, 5);

  wxStaticBox* buttonBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
  wxStaticBoxSizer* bbSizer1 = new wxStaticBoxSizer(buttonBox, wxVERTICAL);

  m_btn_pause = new wxButton(this, wxID_ANY, _("Pause"), wxDefaultPosition,
                               wxDefaultSize, 0);
  m_btn_copy = new wxButton(this, wxID_ANY, _("Copy"), wxDefaultPosition,
                              wxDefaultSize, 0);
  m_btn_copy->SetToolTip(_("Copy NMEA Debug window to clipboard."));

  bbSizer1->Add(m_btn_pause, 0, wxALL, 5);
  bbSizer1->Add(m_btn_copy, 0, wxALL, 5);
  bSizerBottomContainer->Add(bbSizer1, 1, wxALL | wxEXPAND, 5);

  m_btn_copy->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(TTYWindow::OnCopyClick), NULL,
                        this);
  m_btn_pause->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(TTYWindow::OnPauseClick), NULL,
                         this);

  m_is_paused = false;
}

TTYWindow::~TTYWindow() {
  if (m_tty_scroll) {
    delete m_tty_scroll;
    m_tty_scroll = NULL;
  }
}

void TTYWindow::CreateLegendBitmap() {
  double dip_factor = OCPN_GetWinDIPScaleFactor();
  wxScreenDC dcs;
  wxFont font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  int width, height;
  dcs.GetTextExtent("M", &width, &height, NULL, NULL, &font);
  double ref_dim = height * dip_factor;

  m_bm_legend.Create(36 * width * dip_factor, 6.5 * ref_dim);
  wxMemoryDC dc;
  dc.SelectObject(m_bm_legend);
  if (m_bm_legend.IsOk()) {
    dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
    dc.Clear();
    dc.SetFont(font);

    int yp = ref_dim * 1.25;
    int y = ref_dim * .25;
    int bsize = ref_dim;
    int text_x = ref_dim * 1.5;
    int boff = ref_dim *.25;

    wxBrush b1(wxColour(_T("DARK GREEN")));
    dc.SetBrush(b1);
    dc.DrawRectangle(boff, y, bsize, bsize);
    dc.SetTextForeground(wxColour(_T("DARK GREEN")));
    dc.DrawText(_("Message accepted"), text_x, y);

    y += yp;
    wxBrush b2(wxColour(_T("CORAL")));
    dc.SetBrush(b2);
    dc.DrawRectangle(boff, y, bsize, bsize);
    dc.SetTextForeground(wxColour(_T("CORAL")));
    dc.DrawText(
        _("Input message filtered, output message filtered and dropped"), text_x,
        y);

    y += yp;
    wxBrush b3(wxColour(_T("MAROON")));
    dc.SetBrush(b3);
    dc.DrawRectangle(boff, y, bsize, bsize);
    dc.SetTextForeground(wxColour(_T("MAROON")));
    dc.DrawText(_("Input Message filtered and dropped"), text_x, y);

    y += yp;
    wxBrush b4(wxColour(_T("BLUE")));
    dc.SetBrush(b4);
    dc.DrawRectangle(boff, y, bsize, bsize);
    dc.SetTextForeground(wxColour(_T("BLUE")));
    dc.DrawText(_("Output Message"), text_x, y);

    y += yp;
    wxBrush b5(wxColour(_T("RED")));
    dc.SetBrush(b5);
    dc.DrawRectangle(boff, y, bsize, bsize);
    dc.SetTextForeground(wxColour(_T("RED")));
    dc.DrawText(_("Information Message or Message with errors"), text_x, y);
  }
  dc.SelectObject(wxNullBitmap);
}

void TTYWindow::OnPauseClick(wxCommandEvent& event) {
  if (!m_is_paused) {
    m_is_paused = true;
    m_tty_scroll->Pause(true);
    m_btn_pause->SetLabel(_("Resume"));
  } else {
    m_is_paused = false;
    m_tty_scroll->Pause(false);

    m_btn_pause->SetLabel(_("Pause"));
  }
}

void TTYWindow::OnCopyClick(wxCommandEvent& event) { m_tty_scroll->Copy(); }

void TTYWindow::OnCloseWindow(wxCloseEvent& event) {
  if (m_window_destroy_listener) {
    m_window_destroy_listener->DestroyWindow();
  } else {
    Destroy();
  }
}

void TTYWindow::Add(const wxString& line) {
  if (m_tty_scroll) m_tty_scroll->Add(line);
}
