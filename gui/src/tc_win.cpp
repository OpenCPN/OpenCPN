/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement tc_win.h -- tide and currents window
 */

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/font.h>
#include <wx/panel.h>
#include <wx/dcbuffer.h>
#include <wx/listctrl.h>
#include <wx/utils.h>

#include "tc_win.h"

#include "model/cutil.h"
#include "model/config_vars.h"
#include "model/gui_vars.h"

#include "abstract_chart_canv.h"
#include "chcanv.h"
#include "dychart.h"
#include "font_mgr.h"
#include "ocpn_platform.h"
#include "rollover_win.h"
#include "navutil.h"
#include "gui_lib.h"
#include "navutil.h"
#include "ocpn_platform.h"
#include "rollover_win.h"
#include "tc_data_factory.h"
#include "tcmgr.h"
#include "tide_time.h"
#include "timers.h"
#include "user_colors.h"

extern ColorScheme global_color_scheme;  // library dependence

// Custom chart panel class definition
class TCWin::TideChartPanel : public wxPanel {
public:
  TideChartPanel(TCWin *parent) : wxPanel(parent, wxID_ANY), m_tcWin(parent) {
    SetMinSize(wxSize(400, 200));
    Bind(wxEVT_PAINT, &TideChartPanel::OnPaint, this);
    Bind(wxEVT_MOTION, &TideChartPanel::OnMouseMove, this);
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);  // Prevent flicker

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont(*qFont);
    wxScreenDC dc;
    int text_height;
    dc.SetFont(*qFont);
    dc.GetTextExtent("W", NULL, &text_height);
    m_refTCWTextHeight = text_height;
  }

private:
  void OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);

    // Clear the background
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();

    // Calculate chart rectangle within this panel
    wxSize panelSize = GetClientSize();
    if (panelSize.GetWidth() <= 0 || panelSize.GetHeight() <= 0) {
      return;
    }

    // Use larger left margin for Y-axis labels and units
    int left_margin = 50;   // Space for Y-axis numbers and units
    int other_margins = 5;  // Smaller margins for top, right, bottom
    int chart_width = panelSize.GetWidth() - left_margin - other_margins;
    int chart_height = panelSize.GetHeight() - (2 * other_margins);

    // Reserve space at bottom for date/time text
    int bottom_text_space = 4 * m_refTCWTextHeight;
    chart_height -= bottom_text_space;
    chart_width = wxMax(chart_width, 300);
    chart_height = wxMax(chart_height, 150);
    wxRect chartRect(left_margin, other_margins, chart_width, chart_height);

    // Delegate chart painting to parent TCWin
    m_tcWin->PaintChart(dc, chartRect);
  }

  void OnMouseMove(wxMouseEvent &event) {
    wxPoint panelPos = event.GetPosition();
    wxPoint mainWindowPos = panelPos + GetPosition();
    m_tcWin->HandleChartMouseMove(mainWindowPos.x, mainWindowPos.y, panelPos);
    event.Skip();
  }

  TCWin *m_tcWin;
  int m_refTCWTextHeight;
};

enum { ID_TCWIN_NX, ID_TCWIN_PR };

enum { TIDE_PLOT, CURRENT_PLOT };

BEGIN_EVENT_TABLE(TCWin, wxWindow)
EVT_PAINT(TCWin::OnPaint)
EVT_SIZE(TCWin::OnSize)
EVT_MOTION(TCWin::MouseEvent)
EVT_BUTTON(wxID_OK, TCWin::OKEvent)
EVT_BUTTON(ID_TCWIN_NX, TCWin::NXEvent)
EVT_BUTTON(ID_TCWIN_PR, TCWin::PREvent)
EVT_CLOSE(TCWin::OnCloseWindow)
EVT_TIMER(TCWININF_TIMER, TCWin::OnTCWinPopupTimerEvent)
EVT_TIMER(TCWIN_TIME_INDICATOR_TIMER, TCWin::OnTimeIndicatorTimer)
END_EVENT_TABLE()

// Define a constructor
TCWin::TCWin(ChartCanvas *parent, int x, int y, void *pvIDX) {
  m_created = false;
  xSpot = 0;
  ySpot = 0;

  m_pTCRolloverWin = NULL;

  long wstyle = wxCLIP_CHILDREN | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER |
                wxFRAME_FLOAT_ON_PARENT;

  pParent = parent;
  m_x = x;
  m_y = y;

  RecalculateSize();

  // Read the config file to get the user specified time zone.
  if (pConfig) {
    pConfig->SetPath("/Settings/Others");
    pConfig->Read("TCWindowTimeZone", &m_tzoneDisplay, 0);
  }

  wxFrame::Create(parent, wxID_ANY, wxString(""), m_position, m_tc_size,
                  wstyle);

  m_created = true;
  wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  pIDX = (IDX_entry *)pvIDX;

  //    Set up plot type
  if (strchr("Tt", pIDX->IDX_type)) {
    m_plot_type = TIDE_PLOT;
    SetTitle(wxString(_("Tide")));

  } else {
    m_plot_type = CURRENT_PLOT;
    SetTitle(wxString(_("Current")));
  }

  int sx, sy;
  GetClientSize(&sx, &sy);

  SetTimeFactors();

  btc_valid = false;

  // Establish a "reference" text hieght value, for layout assistance
  wxScreenDC dc;
  int text_height;
  dc.SetFont(*qFont);
  dc.GetTextExtent("W", NULL, &text_height);
  m_refTextHeight = text_height;

  CreateLayout();
  Layout();
  m_graph_rect = wxRect(0, 0, 400, 200);

  //  Measure the size of a generic button, with label
  wxButton *test_button =
      new wxButton(this, wxID_OK, _("OK"), wxPoint(-1, -1), wxDefaultSize);
  test_button->GetSize(&m_tsx, &m_tsy);
  delete test_button;

  m_TCWinPopupTimer.SetOwner(this, TCWININF_TIMER);

  // Timer for refreshing time indicators (red line moves with current time)
  m_TimeIndicatorTimer.SetOwner(this, TCWIN_TIME_INDICATOR_TIMER);
  m_TimeIndicatorTimer.Start(60000, false);  // Refresh every 60 seconds

  m_button_height = m_tsy;

  // Build graphics tools

  wxFont *dlg_font = FontMgr::Get().GetFont(_("Dialog"));
  int dlg_font_size = dlg_font->GetPointSize();
#if defined(__WXOSX__) || defined(__WXGTK3__)
  // Support scaled HDPI displays.
  dlg_font_size /= GetContentScaleFactor();
#endif

  pSFont = FontMgr::Get().FindOrCreateFont(
      dlg_font_size - 2, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
      wxFONTWEIGHT_NORMAL, FALSE, wxString("Arial"));
  pSMFont = FontMgr::Get().FindOrCreateFont(
      dlg_font_size - 1, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
      wxFONTWEIGHT_NORMAL, FALSE, wxString("Arial"));
  pMFont = FontMgr::Get().FindOrCreateFont(
      dlg_font_size, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,
      FALSE, wxString("Arial"));
  pLFont = FontMgr::Get().FindOrCreateFont(
      dlg_font_size + 1, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
      wxFONTWEIGHT_BOLD, FALSE, wxString("Arial"));

  // Secondary grid
  pblack_1 = wxThePenList->FindOrCreatePen(
      this->GetForegroundColour(), wxMax(1, (int)(m_tcwin_scaler + 0.5)),
      wxPENSTYLE_SOLID);
  // Primary grid
  pblack_2 = wxThePenList->FindOrCreatePen(
      this->GetForegroundColour(), wxMax(2, (int)(2 * m_tcwin_scaler + 0.5)),
      wxPENSTYLE_SOLID);
  // Tide hours outline
  pblack_3 = wxThePenList->FindOrCreatePen(
      wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW),
      wxMax(1, (int)(m_tcwin_scaler + 0.5)), wxPENSTYLE_SOLID);
  // System time vertical line - solid red line showing current system time
  // position on tide/current chart
  pred_2 = wxThePenList->FindOrCreatePen(
      wxColor(230, 54, 54), wxMax(4, (int)(4 * m_tcwin_scaler + 0.5)),
      wxPENSTYLE_SOLID);
  // Selected time vertical line - dotted blue line showing timeline widget or
  // GRIB time selection on chart
  pred_time = wxThePenList->FindOrCreatePen(
      wxColour(0, 100, 255), wxMax(4, (int)(4 * m_tcwin_scaler + 0.5)),
      wxPENSTYLE_DOT);
  // Graph background
  pltgray = wxTheBrushList->FindOrCreateBrush(this->GetBackgroundColour(),
                                              wxBRUSHSTYLE_SOLID);
  // Tide hours background
  pltgray2 = wxTheBrushList->FindOrCreateBrush(this->GetBackgroundColour(),
                                               wxBRUSHSTYLE_SOLID);
  pgraph = wxThePenList->FindOrCreatePen(
      wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),
      wxMax(1, (int)(m_tcwin_scaler + 0.5)), wxPENSTYLE_SOLID);

  DimeControl(this);

  // Initialize the station text now that fonts are available
  InitializeStationText();
}

TCWin::TCWin(AbstractChartCanvas *parent, int x, int y, void *pvIDX)
    : TCWin(dynamic_cast<ChartCanvas *>(parent), x, y, pvIDX) {}

TCWin::~TCWin() {
  m_TimeIndicatorTimer.Stop();
  pParent->Refresh(false);
}

void TCWin::CreateLayout() {
  // Create main sizer
  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

  // ROW 1: Top panel for station info and tide list (two cells)
  m_topPanel = new wxPanel(this, wxID_ANY);
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  // Left cell: Station info text control with minimum size
  m_ptextctrl =
      new wxTextCtrl(m_topPanel, -1, "", wxDefaultPosition, wxDefaultSize,
                     wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
  m_ptextctrl->SetMinSize(wxSize(
      25 * m_refTextHeight, 7 * m_refTextHeight));  // Minimum readable size

  // Right cell: Tide list (LW/HW) with minimum size
  m_tList = new wxListCtrl(m_topPanel, -1, wxDefaultPosition, wxDefaultSize,
                           wxLC_REPORT | wxLC_NO_HEADER);
  m_tList->SetMinSize(
      wxSize(18 * m_refTextHeight,
             4 * m_refTextHeight));  // Minimum to show a few entries

  // Add first column to tide list
  wxListItem col0;
  col0.SetId(0);
  col0.SetText("");
  col0.SetAlign(wxLIST_FORMAT_LEFT);
  col0.SetWidth(20 * m_refTextHeight);
  m_tList->InsertColumn(0, col0);

  // Add controls to top sizer (first row: two cells)
  topSizer->Add(m_ptextctrl, 2, wxEXPAND | wxALL,
                5);                                // Left cell: 2/3 of width
  topSizer->Add(m_tList, 1, wxEXPAND | wxALL, 5);  // Right cell: 1/3 of width

  m_topPanel->SetSizer(topSizer);

  // ROW 2: Chart panel (expandable - gets remaining space)
  m_chartPanel = new TideChartPanel(this);

  // ROW 3: Button panel (fixed height at bottom)
  m_buttonPanel = new wxPanel(this, wxID_ANY);
  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

  // Create buttons
  PR_button = new wxButton(m_buttonPanel, ID_TCWIN_PR, _("Prev"));
  NX_button = new wxButton(m_buttonPanel, ID_TCWIN_NX, _("Next"));
  OK_button = new wxButton(m_buttonPanel, wxID_OK, _("OK"));

  // Create timezone choice
  wxString choiceOptions[] = {_("LMT@Station"), _("UTC")};
  int numChoices = sizeof(choiceOptions) / sizeof(wxString);
  m_choiceTimezone = new wxChoice(m_buttonPanel, wxID_ANY, wxDefaultPosition,
                                  wxDefaultSize, numChoices, choiceOptions);
  m_choiceTimezone->SetSelection(m_tzoneDisplay);
  m_choiceTimezone->SetToolTip(
      _("Select whether tide times are shown in UTC or Local Mean Time (LMT) "
        "at the station"));

  // Layout buttons: Prev/Next on left, timezone/OK on right
  buttonSizer->Add(PR_button, 0, wxALL, 5);
  buttonSizer->Add(NX_button, 0, wxALL, 5);
  buttonSizer->AddStretchSpacer(1);  // Push timezone and OK to the right
  buttonSizer->Add(m_choiceTimezone, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  buttonSizer->AddSpacer(10);  // Small space between timezone and OK
  buttonSizer->Add(OK_button, 0, wxALL, 5);

  m_buttonPanel->SetSizer(buttonSizer);

  // Add all rows to main sizer with proper proportions
  mainSizer->Add(m_topPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP,
                 5);  // Row 1: Fixed height, no overlap
  mainSizer->Add(m_chartPanel, 1, wxEXPAND | wxLEFT | wxRIGHT,
                 5);  // Row 2: Expandable, gets remaining space
  mainSizer->Add(m_buttonPanel, 0, wxEXPAND | wxALL,
                 5);  // Row 3: Fixed height at bottom

  // Set the main sizer
  SetSizer(mainSizer);

  // Connect timezone choice event
  m_choiceTimezone->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                            wxCommandEventHandler(TCWin::TimezoneOnChoice),
                            NULL, this);
}

void TCWin::InitializeStationText() {
  //  Fill station information in text control
  m_ptextctrl->Clear();

  wxString locn(pIDX->IDX_station_name, wxConvUTF8);
  wxString locna, locnb;
  if (locn.Contains(wxString(","))) {
    locna = locn.BeforeFirst(',');
    locnb = locn.AfterFirst(',');
  } else {
    locna = locn;
    locnb.Empty();
  }

  // write the first line
  wxTextAttr style;
  style.SetFont(*pLFont);
  m_ptextctrl->SetDefaultStyle(style);

  m_ptextctrl->AppendText(locna);
  m_ptextctrl->AppendText("\n");

  style.SetFont(*pSMFont);
  m_ptextctrl->SetDefaultStyle(style);

  if (!locnb.IsEmpty()) m_ptextctrl->AppendText(locnb);
  m_ptextctrl->AppendText("\n");

  // Reference to the master station
  if (('t' == pIDX->IDX_type) || ('c' == pIDX->IDX_type)) {
    wxString mref(pIDX->IDX_reference_name, wxConvUTF8);
    mref.Prepend(" ");

    m_ptextctrl->AppendText(_("Reference Station :"));
    m_ptextctrl->AppendText("\n");

    m_ptextctrl->AppendText(mref);
    m_ptextctrl->AppendText("\n");

  } else {
    m_ptextctrl->AppendText("\n");
  }

  //      Show the data source
  wxString dsource(pIDX->source_ident, wxConvUTF8);
  dsource.Prepend(" ");

  m_ptextctrl->AppendText(_("Data Source :"));
  m_ptextctrl->AppendText("\n");

  m_ptextctrl->AppendText(dsource);

  m_ptextctrl->ShowPosition(0);
}

void TCWin::PaintChart(wxDC &dc, const wxRect &chartRect) {
  if (!IsShown()) {
    return;
  }

  // Store the original graph rectangle and use the provided chartRect
  wxRect originalGraphRect = m_graph_rect;
  m_graph_rect = chartRect;

  int i;
  char sbuf[100];
  int w;
  float tcmax, tcmin;

  if (m_graph_rect.x == 0) {
    m_graph_rect = originalGraphRect;
    return;
  }

  // Get client size for positioning date/timezone text below chart
  int x, y;
  GetClientSize(&x, &y);

  // Adjust colors with current color scheme
  pblack_1->SetColour(this->GetForegroundColour());
  pblack_2->SetColour(this->GetForegroundColour());
  pltgray->SetColour(this->GetBackgroundColour());
  pltgray2->SetColour(this->GetBackgroundColour());
  pred_2->SetColour(
      GetGlobalColor("URED"));  // System time indicator - universal red
  pred_time->SetColour(
      GetGlobalColor("UINFB"));  // Selected time indicator - information blue

  // Box the graph
  dc.SetPen(*pblack_1);
  dc.SetBrush(*pltgray);
  dc.DrawRectangle(m_graph_rect.x, m_graph_rect.y, m_graph_rect.width,
                   m_graph_rect.height);

  //  On some platforms, we cannot draw rotated text.
  //  So, reduce the complexity of horizontal axis time labels
#ifndef __WXMSW__
  const int hour_delta = 4;
#else
  const int hour_delta = 1;
#endif

  int hour_start = 0;

  //    Horizontal axis
  dc.SetFont(*pSFont);
  for (i = 0; i < 25; i++) {
    int xd = m_graph_rect.x + ((i)*m_graph_rect.width / 25);
    if (hour_delta != 1) {
      if (i % hour_delta == 0) {
        dc.SetPen(*pblack_2);
        dc.DrawLine(xd, m_graph_rect.y, xd,
                    m_graph_rect.y + m_graph_rect.height + 5);
        char sbuf[16];
        int hour_show = hour_start + i;
        if (hour_show >= 24) hour_show -= 24;
        sprintf(sbuf, "%02d", hour_show);
        int x_shim = -20;
        dc.DrawText(wxString(sbuf, wxConvUTF8),
                    xd + x_shim + (m_graph_rect.width / 25) / 2,
                    m_graph_rect.y + m_graph_rect.height + 8);
      } else {
        dc.SetPen(*pblack_1);
        dc.DrawLine(xd, m_graph_rect.y, xd,
                    m_graph_rect.y + m_graph_rect.height + 5);
      }
    } else {
      dc.SetPen(*pblack_1);
      dc.DrawLine(xd, m_graph_rect.y, xd,
                  m_graph_rect.y + m_graph_rect.height + 5);
      wxString sst;
      sst.Printf("%02d", i);
      dc.DrawRotatedText(sst, xd + (m_graph_rect.width / 25) / 2,
                         m_graph_rect.y + m_graph_rect.height + 8, 270.);
    }
  }

  // Time indicators - system time and "selected" time (e.g. GRIB time)
  wxDateTime system_now = wxDateTime::Now();
  wxDateTime this_now = gTimeSource;
  bool cur_time = !gTimeSource.IsValid();
  if (cur_time) this_now = wxDateTime::Now();

  // Always draw system time indicator (solid red line)
  time_t t_system_now = system_now.GetTicks();
  t_system_now -= m_diff_mins * 60;
  if (m_tzoneDisplay == 0)  // LMT @ Station
    t_system_now += m_stationOffset_mins * 60;

  float t_system_ratio =
      m_graph_rect.width * (t_system_now - m_t_graphday_GMT) / (25 * 3600.0f);
  int x_system = (t_system_ratio < 0 || t_system_ratio > m_graph_rect.width)
                     ? -1
                     : m_graph_rect.x + (int)t_system_ratio;

  if (x_system >= 0) {
    dc.SetPen(*pred_2);  // solid red line for system time
    dc.DrawLine(x_system, m_graph_rect.y, x_system,
                m_graph_rect.y + m_graph_rect.height);
  }

  // Draw "selected time" indicator (from timeline widget) if different from
  // system time.
  if (gTimeSource.IsValid()) {
    time_t t_selected_time = gTimeSource.GetTicks();
    if (abs(t_selected_time - t_system_now) > 300) {
      t_selected_time -= m_diff_mins * 60;
      if (m_tzoneDisplay == 0)  // LMT @ Station
        t_selected_time += m_stationOffset_mins * 60;

      float t_selected_time_ratio = m_graph_rect.width *
                                    (t_selected_time - m_t_graphday_GMT) /
                                    (25 * 3600.0f);
      int x_selected_time = (t_selected_time_ratio < 0 ||
                             t_selected_time_ratio > m_graph_rect.width)
                                ? -1
                                : m_graph_rect.x + (int)t_selected_time_ratio;

      if (x_selected_time >= 0) {
        dc.SetPen(*pred_time);
        dc.DrawLine(x_selected_time, m_graph_rect.y, x_selected_time,
                    m_graph_rect.y + m_graph_rect.height);
      }
    }
  }
  dc.SetPen(*pblack_1);

  //    Build the array of values, capturing max and min and HW/LW list
  if (!btc_valid) {
    float dir;
    tcmax = -10;
    tcmin = 10;
    float val = -100;
    m_tList->DeleteAllItems();
    int list_index = 0;
    bool wt = false;

    wxBeginBusyCursor();

    // The tide/current modules calculate values based on PC local time
    // We want UTC, so adjust accordingly
    int tt_localtz = m_t_graphday_GMT + (m_diff_mins * 60);
    // then eventually we could need LMT at station
    if (m_tzoneDisplay == 0)
      tt_localtz -= m_stationOffset_mins * 60;  // LMT at station

    // get tide flow sens ( flood or ebb ? )
    ptcmgr->GetTideFlowSens(tt_localtz, BACKWARD_TEN_MINUTES_STEP,
                            pIDX->IDX_rec_num, tcv[0], val, wt);

    for (i = 0; i < 26; i++) {
      int tt = tt_localtz + (i * FORWARD_ONE_HOUR_STEP);
      ptcmgr->GetTideOrCurrent(tt, pIDX->IDX_rec_num, tcv[i], dir);
      tt_tcv[i] = tt;  // store the corresponding time_t value

      // Convert tide values from station units to user's height units
      Station_Data *pmsd = pIDX->pref_sta_data;
      if (pmsd) {
        // Convert from station units to meters first
        int unit_c = TCDataFactory::findunit(pmsd->unit);
        if (unit_c >= 0) {
          tcv[i] = tcv[i] * TCDataFactory::known_units[unit_c].conv_factor;
        }
        // Now convert from meters to preferred height units
        tcv[i] = toUsrHeight(tcv[i]);
      }

      if (tcv[i] > tcmax) tcmax = tcv[i];
      if (tcv[i] < tcmin) tcmin = tcv[i];

      if (TIDE_PLOT == m_plot_type) {
        if (!((tcv[i] > val) == wt) && (i > 0)) {  // if tide flow sense change
          float tcvalue;                           // look backward for HW or LW
          time_t tctime;
          ptcmgr->GetHightOrLowTide(tt, BACKWARD_TEN_MINUTES_STEP,
                                    BACKWARD_ONE_MINUTES_STEP, tcv[i], wt,
                                    pIDX->IDX_rec_num, tcvalue, tctime);
          if (tctime > tt_localtz) {  // Only show events visible in graphic
                                      // presently shown
            wxDateTime tcd;           // write date
            wxString s, s1;
            tcd.Set(tctime - (m_diff_mins * 60));
            if (m_tzoneDisplay == 0)  // LMT @ Station
              tcd.Set(tctime + (m_stationOffset_mins - m_diff_mins) * 60);

            s.Printf(tcd.Format("%H:%M  "));

            // Convert tcvalue to preferred height units (it comes from
            // GetHightOrLowTide in station units)
            double tcvalue_converted = tcvalue;
            Station_Data *pmsd = pIDX->pref_sta_data;
            if (pmsd) {
              // Convert from station units to meters first
              int unit_c = TCDataFactory::findunit(pmsd->unit);
              if (unit_c >= 0) {
                tcvalue_converted =
                    tcvalue_converted *
                    TCDataFactory::known_units[unit_c].conv_factor;
              }
              // Now convert from meters to preferred height units
              tcvalue_converted = toUsrHeight(tcvalue_converted);
            }

            s1.Printf("%05.2f ", tcvalue_converted);  // write converted value
            s.Append(s1);
            s.Append(getUsrHeightUnit());
            s.Append("   ");
            (wt) ? s.Append(_("HW")) : s.Append(_("LW"));  // write HW or LT

            wxListItem li;
            li.SetId(list_index);
            li.SetAlign(wxLIST_FORMAT_LEFT);
            li.SetText(s);
            li.SetColumn(0);
            m_tList->InsertItem(li);
            list_index++;
          }
          wt = !wt;  // change tide flow sens
        }
        val = tcv[i];
      }
      if (CURRENT_PLOT == m_plot_type) {
        wxDateTime thx;  // write date
        wxString s, s1;
        thx.Set((time_t)tt - (m_diff_mins * 60));
        if (m_tzoneDisplay == 0)  // LMT @ Station
          thx.Set((time_t)tt + (m_stationOffset_mins - m_diff_mins) * 60);

        s.Printf(thx.Format("%H:%M  "));
        s1.Printf("%05.2f ",
                  fabs(tcv[i]));  // tcv[i] is already converted to height units
        s.Append(s1);
        s.Append(getUsrHeightUnit());
        s1.Printf("  %03.0f", dir);  // write direction
        s.Append(s1);

        wxListItem li;
        li.SetId(list_index);
        li.SetAlign(wxLIST_FORMAT_LEFT);
        li.SetText(s);
        li.SetColumn(0);
        m_tList->InsertItem(li);
        list_index++;
      }
    }

    wxEndBusyCursor();

    //    Set up the vertical parameters based on Tide or Current plot
    if (CURRENT_PLOT == m_plot_type) {
      it = std::max(abs((int)tcmin - 1), abs((int)tcmax + 1));
      ib = -it;
      im = 2 * it;
      m_plot_y_offset = m_graph_rect.height / 2;
      val_off = 0;
    } else {
      ib = (int)tcmin;
      if (tcmin < 0) ib -= 1;
      it = (int)tcmax + 1;
      im = it - ib;
      m_plot_y_offset = (m_graph_rect.height * (it - ib)) / im;
      val_off = ib;
    }

    // Arrange to skip some lines and legends if there are too many for the
    // vertical space we have
    int height_stext;
    dc.GetTextExtent("1", NULL, &height_stext);
    float available_lines = (float)m_graph_rect.height / height_stext;
    i_skip = (int)ceil(im / available_lines);

    if (CURRENT_PLOT == m_plot_type && i_skip != 1) {
      // Adjust steps so slack current "0" line is always drawn on graph
      ib -= it % i_skip;
      it = -ib;
      im = 2 * it;
    }

    //    Build spline list of points
    for (auto it = m_sList.begin(); it != m_sList.end(); it++) delete (*it);
    m_sList.clear();

    for (i = 0; i < 26; i++) {
      wxPoint *pp = new wxPoint;
      pp->x = m_graph_rect.x + ((i)*m_graph_rect.width / 25);
      pp->y = m_graph_rect.y + (m_plot_y_offset) -
              (int)((tcv[i] - val_off) * m_graph_rect.height / im);
      m_sList.push_back(pp);
    }

    btc_valid = true;
  }

  // Graph legend
  dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

  //    Vertical Axis
  i = ib;
  while (i < it + 1) {
    int yd = m_graph_rect.y + (m_plot_y_offset) -
             ((i - val_off) * m_graph_rect.height / im);
    if ((m_plot_y_offset + m_graph_rect.y) == yd)
      dc.SetPen(*pblack_2);
    else
      dc.SetPen(*pblack_1);

    dc.DrawLine(m_graph_rect.x, yd, m_graph_rect.x + m_graph_rect.width, yd);
    if ((yd < m_graph_rect.height * 48 / 100) ||
        (yd > m_graph_rect.height * 52 / 100)) {
      snprintf(sbuf, 99, "%d", i);
      dc.DrawText(wxString(sbuf, wxConvUTF8), m_graph_rect.x - 20, yd - 5);
    }
    i += i_skip;
  }

  //    Draw the Value curve
  wxPointList list;
  for (auto &p : m_sList) list.Append(p);

  dc.SetPen(*pgraph);
#if wxUSE_SPLINES
  dc.DrawSpline(&list);
#else
  dc.DrawLines(&list);
#endif

  //  More Info - positioned below chart panel
  if (m_tzoneDisplay == 0) {
    int station_offset = ptcmgr->GetStationTimeOffset(pIDX);
    int h = station_offset / 60;
    int m = station_offset - (h * 60);
    if (m_graphday.IsDST()) h += 1;
    m_stz.Printf("UTC %+03d:%02d", h, m);

    //    Make the "nice" (for the US) station time-zone string, brutally by
    //    hand
    double lat = ptcmgr->GetStationLat(pIDX);
    if (lat > 20.0) {
      wxString mtz;
      switch (ptcmgr->GetStationTimeOffset(pIDX)) {
        case -240:
          mtz = "AST";
          break;
        case -300:
          mtz = "EST";
          break;
        case -360:
          mtz = "CST";
          break;
      }
      if (mtz.Len()) {
        if (m_graphday.IsDST()) mtz[1] = 'D';
        m_stz = mtz;
      }
    }
  } else {
    m_stz = "UTC";
  }

  int h;
  dc.SetFont(*pSFont);
  dc.GetTextExtent(m_stz, &w, &h);
  // Position timezone text below the chart, centered horizontally
  dc.DrawText(m_stz, m_graph_rect.x + (m_graph_rect.width / 2) - (w / 2),
              m_graph_rect.y + m_graph_rect.height + (2 * h));

  wxString sdate;
  if (g_locale == "en_US")
    sdate = m_graphday.Format("%A %b %d, %Y");
  else
    sdate = m_graphday.Format("%A %d %b %Y");

  dc.SetFont(*pMFont);
  dc.GetTextExtent(sdate, &w, &h);
  // Position date text below the chart, centered horizontally
  dc.DrawText(sdate, m_graph_rect.x + (m_graph_rect.width / 2) - (w / 2),
              m_graph_rect.y + m_graph_rect.height + (5 * h / 2));

  Station_Data *pmsd = pIDX->pref_sta_data;
  if (pmsd) {
    // Use user's height unit for Y-axis label instead of station units
    wxString height_unit = getUsrHeightUnit();
    dc.GetTextExtent(height_unit, &w, &h);
    dc.DrawRotatedText(height_unit, 0,
                       m_graph_rect.y + m_graph_rect.height / 2 + w / 2, 90.);
  }

  //      Show flood and ebb directions
  if ((strchr("c", pIDX->IDX_type)) || (strchr("C", pIDX->IDX_type))) {
    dc.SetFont(*pSFont);
    wxString fdir;
    fdir.Printf("%03d", pIDX->IDX_flood_dir);
    dc.DrawText(fdir, m_graph_rect.x + m_graph_rect.width + 4,
                m_graph_rect.y + m_graph_rect.height * 1 / 4);

    wxString edir;
    edir.Printf("%03d", pIDX->IDX_ebb_dir);
    dc.DrawText(edir, m_graph_rect.x + m_graph_rect.width + 4,
                m_graph_rect.y + m_graph_rect.height * 3 / 4);
  }

  //    Today or tomorrow
  if ((m_button_height * 15) < x && cur_time) {  // large enough horizontally?
    wxString sday;
    int day = m_graphday.GetDayOfYear();
    if (m_graphday.GetYear() == this_now.GetYear()) {
      if (day == this_now.GetDayOfYear())
        sday.Append(_("Today"));
      else if (day == this_now.GetDayOfYear() + 1)
        sday.Append(_("Tomorrow"));
      else
        sday.Append(m_graphday.GetWeekDayName(m_graphday.GetWeekDay()));
    } else if (m_graphday.GetYear() == this_now.GetYear() + 1 &&
               day == this_now.Add(wxTimeSpan::Day()).GetDayOfYear())
      sday.Append(_("Tomorrow"));

    dc.SetFont(*pSFont);
    dc.GetTextExtent(sday, &w, &h);
    // Position day text at the left side of the chart, below it
    dc.DrawText(sday, m_graph_rect.x,
                m_graph_rect.y + m_graph_rect.height + (2 * h));
  }

  //  Render "Spot of interest"
  double spotDim = 4 * g_Platform->GetDisplayDPmm();
  dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(GetGlobalColor("YELO1"),
                                                 wxBRUSHSTYLE_SOLID));
  dc.SetPen(wxPen(GetGlobalColor("URED"),
                  wxMax(2, 0.5 * g_Platform->GetDisplayDPmm())));
  dc.DrawRoundedRectangle(xSpot - spotDim / 2, ySpot - spotDim / 2, spotDim,
                          spotDim, spotDim / 2);

  dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(GetGlobalColor("UBLCK"),
                                                 wxBRUSHSTYLE_SOLID));
  dc.SetPen(wxPen(GetGlobalColor("UBLCK"), 1));
  double ispotDim = spotDim / 5.;
  dc.DrawRoundedRectangle(xSpot - ispotDim / 2, ySpot - ispotDim / 2, ispotDim,
                          ispotDim, ispotDim / 2);

  // Restore original graph rectangle
  m_graph_rect = originalGraphRect;
}

void TCWin::SetTimeFactors() {
  //    Figure out this computer timezone minute offset
  wxDateTime this_now = gTimeSource;
  bool cur_time = !gTimeSource.IsValid();

  if (cur_time) {
    this_now = wxDateTime::Now();
  }
  wxDateTime this_gmt = this_now.ToGMT();

#if wxCHECK_VERSION(2, 6, 2)
  wxTimeSpan diff = this_now.Subtract(this_gmt);
#else
  wxTimeSpan diff = this_gmt.Subtract(this_now);
#endif

  m_diff_mins = diff.GetMinutes();

  //  Correct a bug in wx3.0.2
  //  If the system TZ happens to be GMT, with DST active (e.g.summer in
  //  London), then wxDateTime returns incorrect results for toGMT() method
#if wxCHECK_VERSION(3, 0, 2)
  if (m_diff_mins == 0 && this_now.IsDST()) m_diff_mins += 60;
#endif

  int station_offset = ptcmgr->GetStationTimeOffset(pIDX);

  m_stationOffset_mins = station_offset;
  if (this_now.IsDST()) {
    m_stationOffset_mins += 60;
  }

  //  Correct a bug in wx3.0.2
  //  If the system TZ happens to be GMT, with DST active (e.g.summer in
  //  London), then wxDateTime returns incorrect results for toGMT() method
#if wxCHECK_VERSION(3, 0, 2)
//    if(  this_now.IsDST() )
//        m_corr_mins +=60;
#endif

  //    Establish the inital drawing day as today, in the timezone of the
  //    station
  m_graphday = this_gmt;

  int day_gmt = this_gmt.GetDayOfYear();

  time_t ttNow = this_now.GetTicks();
  time_t tt_at_station =
      ttNow - (m_diff_mins * 60) + (m_stationOffset_mins * 60);
  wxDateTime atStation(tt_at_station);
  int day_at_station = atStation.GetDayOfYear();

  if (day_gmt > day_at_station) {
    wxTimeSpan dt(24, 0, 0, 0);
    m_graphday.Subtract(dt);
  } else if (day_gmt < day_at_station) {
    wxTimeSpan dt(24, 0, 0, 0);
    m_graphday.Add(dt);
  }

  wxDateTime graphday_00 = m_graphday;  // this_gmt;
  graphday_00.ResetTime();
  time_t t_graphday_00 = graphday_00.GetTicks();

  //    Correct a Bug in wxWidgets time support
  //    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
  //    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;

  m_t_graphday_GMT = t_graphday_00;

  btc_valid = false;  // Force re-calculation
}

void TCWin::TimezoneOnChoice(wxCommandEvent &event) {
  m_tzoneDisplay = m_choiceTimezone->GetSelection();
  SetTimeFactors();

  Refresh();
}

void TCWin::RecalculateSize() {
  wxSize parent_size(2000, 2000);
  if (pParent) parent_size = pParent->GetClientSize();

  int unscaledheight = 600;
  int unscaledwidth = 650;

  // value of m_tcwin_scaler should be about unity on a 100 dpi display,
  // when scale parameter g_tcwin_scale is 100
  // parameter g_tcwin_scale is set in config file as value of
  // TideCurrentWindowScale
  g_tcwin_scale = wxMax(g_tcwin_scale, 10);  // sanity check on g_tcwin_scale
  m_tcwin_scaler = g_Platform->GetDisplayDPmm() * 0.254 * g_tcwin_scale / 100.0;

  m_tc_size.x = (int)(unscaledwidth * m_tcwin_scaler + 0.5);
  m_tc_size.y = (int)(unscaledheight * m_tcwin_scaler + 0.5);

  m_tc_size.x = wxMin(m_tc_size.x, parent_size.x);
  m_tc_size.y = wxMin(m_tc_size.y, parent_size.y);

  int xc = m_x + 8;
  int yc = m_y;

  //  Arrange for tcWindow to be always totally visible
  //  by shifting left and/or up
  if ((m_x + 8 + m_tc_size.x) > parent_size.x) xc = xc - m_tc_size.x - 16;
  if ((m_y + m_tc_size.y) > parent_size.y) yc = yc - m_tc_size.y;

  //  Don't let the window origin move out of client area
  if (yc < 0) yc = 0;
  if (xc < 0) xc = 0;

  if (pParent) pParent->ClientToScreen(&xc, &yc);
  m_position = wxPoint(xc, yc);

  if (m_created) {
    SetSize(m_tc_size);
    Move(m_position);
  }
}

void TCWin::OKEvent(wxCommandEvent &event) {
  Hide();

  // Ensure parent pointer is cleared before any potential deletion
  if (pParent && pParent->pCwin == this) {
    pParent->pCwin = NULL;
  }

  // Clean up global tide window counter and associated resources
  --gpIDXn;
  delete m_pTCRolloverWin;
  m_pTCRolloverWin = NULL;
  delete m_tList;
  m_tList = NULL;

  if (pParent) {
    pParent->Refresh(false);
  }

  // Update the config file to set the user specified time zone.
  if (pConfig) {
    pConfig->SetPath("/Settings/Others");
    pConfig->Write("TCWindowTimeZone", m_tzoneDisplay);
  }

  Destroy();  // that hurts
}

void TCWin::OnCloseWindow(wxCloseEvent &event) {
  Hide();

  // Ensure parent pointer is cleared before any potential deletion
  if (pParent && pParent->pCwin == this) {
    pParent->pCwin = NULL;
  }

  // Clean up global tide window counter and associated resources
  --gpIDXn;
  delete m_pTCRolloverWin;
  m_pTCRolloverWin = NULL;
  delete m_tList;
  m_tList = NULL;

  // Update the config file to set the user specified time zone.
  if (pConfig) {
    pConfig->SetPath("/Settings/Others");
    pConfig->Write("TCWindowTimeZone", m_tzoneDisplay);
  }

  Destroy();  // that hurts
}

void TCWin::NXEvent(wxCommandEvent &event) {
  wxTimeSpan dt(24, 0, 0, 0);
  m_graphday.Add(dt);
  wxDateTime dm = m_graphday;

  wxDateTime graphday_00 = dm.ResetTime();
  time_t t_graphday_00 = graphday_00.GetTicks();

  if (!graphday_00.IsDST() && m_graphday.IsDST()) t_graphday_00 -= 3600;
  if (graphday_00.IsDST() && !m_graphday.IsDST()) t_graphday_00 += 3600;

  m_t_graphday_GMT = t_graphday_00;

  btc_valid = false;
  Refresh();
}

void TCWin::PREvent(wxCommandEvent &event) {
  wxTimeSpan dt(-24, 0, 0, 0);
  m_graphday.Add(dt);
  wxDateTime dm = m_graphday;

  wxDateTime graphday_00 = dm.ResetTime();
  time_t t_graphday_00 = graphday_00.GetTicks();

  if (!graphday_00.IsDST() && m_graphday.IsDST()) t_graphday_00 -= 3600;
  if (graphday_00.IsDST() && !m_graphday.IsDST()) t_graphday_00 += 3600;

  m_t_graphday_GMT = t_graphday_00;

  btc_valid = false;
  Refresh();
}

void TCWin::RePosition() {
  //    Position the window
  double lon = pIDX->IDX_lon;
  double lat = pIDX->IDX_lat;

  wxPoint r;
  pParent->GetCanvasPointPix(lat, lon, &r);
  pParent->ClientToScreen(&r.x, &r.y);
  Move(r);
}

void TCWin::OnPaint(wxPaintEvent &event) {
  if (!IsShown()) {
    return;
  }

  // With the new sizer-based layout, the main OnPaint method is simplified.
  // Chart rendering is now handled by the TideChartPanel's OnPaint method,
  // which delegates to our PaintChart() method.

  wxPaintDC dc(this);

  // Clear the background
  dc.SetBrush(wxBrush(GetBackgroundColour()));
  dc.SetPen(wxPen(GetBackgroundColour()));
  wxSize size = GetClientSize();
  dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

  // Note: Chart painting is now handled by TideChartPanel::OnPaint()
  // which calls our PaintChart() method. This eliminates the need for
}

void TCWin::OnSize(wxSizeEvent &event) {
  if (!m_created) return;

  // With sizer-based layout, we don't need manual positioning.
  // The sizers automatically handle layout when the window is resized.

  // Force chart panel to refresh with new size
  if (m_chartPanel) {
    m_chartPanel->Refresh();
  }

  // Invalidate cached chart data to force recalculation
  btc_valid = false;

  // Allow sizers to handle the layout
  event.Skip();
}

void TCWin::MouseEvent(wxMouseEvent &event) {
  // This is now mainly for compatibility.
  // Chart mouse events are handled by HandleChartMouseMove
  event.GetPosition(&curs_x, &curs_y);

  if (!m_TCWinPopupTimer.IsRunning())
    m_TCWinPopupTimer.Start(20, wxTIMER_ONE_SHOT);
}

void TCWin::HandleChartMouseMove(int mainWindowX, int mainWindowY,
                                 const wxPoint &chartPanelPos) {
  // Store the main window coordinates for compatibility with existing rollover
  // code
  curs_x = mainWindowX;
  curs_y = mainWindowY;

  // Also store the chart panel relative coordinates for calculations
  if (m_chartPanel) {
    // Calculate the chart rectangle within the chart panel
    wxSize panelSize = m_chartPanel->GetClientSize();
    int left_margin = 50;   // Space for Y-axis numbers and units
    int other_margins = 5;  // Smaller margins for top, right, bottom
    int chart_width = panelSize.GetWidth() - left_margin - other_margins;
    int chart_height = panelSize.GetHeight() - (2 * other_margins);
    int bottom_text_space = 50;  // Increased space for date display
    chart_height -= bottom_text_space;
    chart_width = wxMax(chart_width, 300);
    chart_height = wxMax(chart_height, 150);

    // Update the graph rectangle to match the current chart panel layout
    wxPoint chartPanelPos = m_chartPanel->GetPosition();
    m_graph_rect =
        wxRect(chartPanelPos.x + left_margin, chartPanelPos.y + other_margins,
               chart_width, chart_height);
  }

  if (!m_TCWinPopupTimer.IsRunning())
    m_TCWinPopupTimer.Start(20, wxTIMER_ONE_SHOT);
}

void TCWin::OnTCWinPopupTimerEvent(wxTimerEvent &event) {
  int x, y;
  bool ShowRollover;

  GetClientSize(&x, &y);
  wxRegion cursorarea(m_graph_rect);
  if (cursorarea.Contains(curs_x, curs_y)) {
    ShowRollover = true;
    SetCursor(*pParent->pCursorCross);
    if (NULL == m_pTCRolloverWin) {
      m_pTCRolloverWin = new RolloverWin(this, -1, false);
      // doesn't really work, mouse positions are relative to rollover window
      // not this window.
      // effect: hide rollover window if mouse on rollover
      m_pTCRolloverWin->SetMousePropogation(1);
      m_pTCRolloverWin->Hide();
    }
    float t, d;
    wxString p, s;

    // Calculate time based on actual chart rectangle position
    // t represents hours into the 25-hour display (0-25)
    float relativeX =
        (float)(curs_x - m_graph_rect.x) / (float)m_graph_rect.width;
    t = relativeX * 25.0f;  // 25 hours displayed across the width

    // Clamp to valid range
    t = wxMax(0.0f, wxMin(25.0f, t));

    int tt = m_t_graphday_GMT + (int)(t * 3600);
    time_t ths = tt;

    wxDateTime thd;
    thd.Set(ths);
    p.Printf(thd.Format("%Hh %Mmn"));
    p.Append("\n");

    // The tide/current modules calculate values based on PC local time
    // We want UTC, so adjust accordingly
    int tt_localtz = m_t_graphday_GMT + (m_diff_mins * 60);

    int ttv = tt_localtz + (int)(t * 3600);
    if (m_tzoneDisplay == 0) {
      ttv -= m_stationOffset_mins * 60;  // LMT at station
    }

    time_t tts = ttv;

    // set tide level or current speed at that time
    ptcmgr->GetTideOrCurrent(tts, pIDX->IDX_rec_num, t, d);

    // Convert tide/current value to preferred height units
    double t_converted = (t < 0 && CURRENT_PLOT == m_plot_type) ? -t : t;
    Station_Data *pmsd = pIDX->pref_sta_data;
    if (pmsd) {
      // Convert from station units to meters first
      int unit_c = TCDataFactory::findunit(pmsd->unit);
      if (unit_c >= 0) {
        t_converted =
            t_converted * TCDataFactory::known_units[unit_c].conv_factor;
      }
      // Now convert from meters to preferred height units
      t_converted = toUsrHeight(t_converted);
    }

    s.Printf("%3.2f ", t_converted);
    p.Append(s);

    // set unit - use preferred height unit abbreviation
    p.Append(getUsrHeightUnit());

    // set current direction
    if (CURRENT_PLOT == m_plot_type) {
      s.Printf("%3.0f%c", d, 0x00B0);
      p.Append("\n");
      p.Append(s);
    }

    // set rollover area size
    wxSize win_size;
    win_size.Set(x * 90 / 100, y * 80 / 100);

    m_pTCRolloverWin->SetString(p);
    m_pTCRolloverWin->SetBestPosition(curs_x, curs_y, 1, 1, TC_ROLLOVER,
                                      win_size);
    m_pTCRolloverWin->SetBitmap(TC_ROLLOVER);
    m_pTCRolloverWin->Refresh();
    m_pTCRolloverWin->Show();

    //  Mark the actual spot on the curve
    // x value is clear...
    //  Find the point in the window that is used for the curve rendering,
    //  rounding as necessary

    int idx = 1;  // in case m_graph_rect.width is weird ie ppx never > curs_x
    for (int i = 0; i < 26; i++) {
      float ppx = m_graph_rect.x + ((i)*m_graph_rect.width / 25.f);
      if (ppx > curs_x) {
        idx = i;
        break;
      }
    }

    if (m_sList.size() > 0 && idx > 0 && idx < (int)m_sList.size()) {
      // Use iterator to access elements in std::list
      auto it_a = m_sList.begin();
      std::advance(it_a, idx - 1);
      auto it_b = m_sList.begin();
      std::advance(it_b, idx);

      wxPoint *a = *it_a;
      wxPoint *b = *it_b;

      float pct = (curs_x - a->x) / (float)((b->x - a->x));
      float dy = pct * (b->y - a->y);

      ySpot = a->y + dy;
      xSpot = curs_x;
    } else {
      // Fallback if we can't find the curve point
      xSpot = curs_x;
      ySpot = m_graph_rect.y + m_graph_rect.height / 2;
    }

    Refresh(true);

  } else {
    SetCursor(*pParent->pCursorArrow);
    ShowRollover = false;
  }

  if (m_pTCRolloverWin && m_pTCRolloverWin->IsShown() && !ShowRollover) {
    m_pTCRolloverWin->Hide();
  }
}

void TCWin::OnTimeIndicatorTimer(wxTimerEvent &event) {
  // Refresh to update the red line (system time indicator)
  Refresh(false);
}
