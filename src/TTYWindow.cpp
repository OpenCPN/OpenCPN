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

TTYWindow::TTYWindow() : m_window_destroy_listener(NULL), m_pScroll(NULL) {}

TTYWindow::TTYWindow(wxWindow* parent, int n_lines,
                     WindowDestroyListener* listener)
    : m_window_destroy_listener(listener), m_pScroll(NULL) {
  wxFrame::Create(
      parent, -1, _T("Title"), wxDefaultPosition, wxDefaultSize,
      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxFRAME_FLOAT_ON_PARENT);

  wxBoxSizer* bSizerOuterContainer = new wxBoxSizer(wxVERTICAL);
  SetSizer(bSizerOuterContainer);

  m_tFilter = new wxTextCtrl(this, wxID_ANY);

  m_pScroll = new TTYScroll(this, n_lines, *m_tFilter);
  m_pScroll->Scroll(-1, 1000);  // start with full scroll down

  bSizerOuterContainer->Add(m_pScroll, 1, wxEXPAND, 5);

  wxStaticBox* psbf = new wxStaticBox(this, wxID_ANY, _("Filter"));
  wxStaticBoxSizer* sbSizer2 = new wxStaticBoxSizer(psbf, wxVERTICAL);
  sbSizer2->Add(m_tFilter, 1, wxALL | wxEXPAND, 5);
  bSizerOuterContainer->Add(sbSizer2, 0, wxEXPAND, 5);

  wxBoxSizer* bSizerBottomContainer = new wxBoxSizer(wxHORIZONTAL);
  bSizerOuterContainer->Add(bSizerBottomContainer, 0, wxEXPAND, 5);

  wxStaticBox* psb = new wxStaticBox(this, wxID_ANY, _("Legend"));
  wxStaticBoxSizer* sbSizer1 = new wxStaticBoxSizer(psb, wxVERTICAL);

  CreateLegendBitmap();
  wxBitmapButton* bb = new wxBitmapButton(this, wxID_ANY, m_bm_legend);
  sbSizer1->Add(bb, 1, wxALL | wxEXPAND, 5);
  bSizerBottomContainer->Add(sbSizer1, 0, wxALIGN_LEFT | wxALL, 5);

  wxStaticBox* buttonBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
  wxStaticBoxSizer* bbSizer1 = new wxStaticBoxSizer(buttonBox, wxVERTICAL);

  m_buttonPause = new wxButton(this, wxID_ANY, _("Pause"), wxDefaultPosition,
                               wxDefaultSize, 0);
  m_buttonCopy = new wxButton(this, wxID_ANY, _("Copy"), wxDefaultPosition,
                              wxDefaultSize, 0);
  m_buttonCopy->SetToolTip(_("Copy NMEA Debug window to clipboard."));

  bbSizer1->Add(m_buttonPause, 0, wxALL, 5);
  bbSizer1->Add(m_buttonCopy, 0, wxALL, 5);
  bSizerBottomContainer->Add(bbSizer1, 1, wxALL | wxEXPAND, 5);

  m_buttonCopy->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(TTYWindow::OnCopyClick), NULL,
                        this);
  m_buttonPause->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(TTYWindow::OnPauseClick), NULL,
                         this);

  bpause = false;
}

TTYWindow::~TTYWindow() {
  if (m_pScroll) {
    delete m_pScroll;
    m_pScroll = NULL;
  }
}

void TTYWindow::CreateLegendBitmap() {
  double dip_factor = OCPN_GetWinDIPScaleFactor();
  wxScreenDC dcs;
  wxFont *pmetricFont = FontMgr::Get().GetFont(_("Dialog"));

  int width, height;
  dcs.GetTextExtent("M", &width, &height, NULL, NULL, pmetricFont);
  double ref_dim = height * dip_factor;

  m_bm_legend.Create(25 * ref_dim, 6.5 * ref_dim);
  wxMemoryDC dc;
  dc.SelectObject(m_bm_legend);
  if (m_bm_legend.IsOk()) {
    dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
    dc.Clear();

    wxFont f(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(f);

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
  if (!bpause) {
    bpause = true;
    m_pScroll->Pause(true);
    m_buttonPause->SetLabel(_("Resume"));
  } else {
    bpause = false;
    m_pScroll->Pause(false);

    m_buttonPause->SetLabel(_("Pause"));
  }
}

void TTYWindow::OnCopyClick(wxCommandEvent& event) { m_pScroll->Copy(); }

void TTYWindow::OnCloseWindow(wxCloseEvent& event) {
  if (m_window_destroy_listener) {
    m_window_destroy_listener->DestroyWindow();
  } else {
    Destroy();
  }
}

void TTYWindow::Add(const wxString& line) {
  if (m_pScroll) m_pScroll->Add(line);
}
