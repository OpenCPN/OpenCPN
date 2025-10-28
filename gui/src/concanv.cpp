/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 ****************************************************************************/

/**
 * \file
 *
 * Implement concanv.h  --  Console canvas
 */

#include <stdlib.h>
#include <time.h>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/datetime.h>

#include "gl_headers.h"  // Must be before anything using GL

#include "model/config_vars.h"
#include "model/navutil_base.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/routeman.h"

#include "concanv.h"
#include "font_mgr.h"
#include "gui_lib.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "ocpn_platform.h"
#include "ocpn_plugin.h"
#include "styles.h"

enum eMenuItems { ID_NAVLEG = 1, ID_NAVROUTE, ID_NAVHIGHWAY } menuItems;

APConsole* console;  ///< Global instance

//------------------------------------------------------------------------------
//    ConsoleCanvasWin Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ConsoleCanvasWin, wxWindow)
EVT_PAINT(ConsoleCanvasWin::OnPaint)
EVT_SHOW(ConsoleCanvasWin::OnShow)
EVT_CONTEXT_MENU(ConsoleCanvasWin::OnContextMenu)
EVT_MENU(ID_NAVLEG, ConsoleCanvasWin::OnContextMenuSelection)
EVT_MENU(ID_NAVROUTE, ConsoleCanvasWin::OnContextMenuSelection)
EVT_MENU(ID_NAVHIGHWAY, ConsoleCanvasWin::OnContextMenuSelection)
END_EVENT_TABLE()

// Define a constructor for my canvas
ConsoleCanvasWin::ConsoleCanvasWin(wxWindow* parent) {
  m_speedUsed = SPEED_SOG;
  pbackBrush = NULL;
  m_bNeedClear = false;

  long style = wxSIMPLE_BORDER | wxCLIP_CHILDREN | wxFRAME_FLOAT_ON_PARENT;

#ifdef __WXMSW__
  style |= wxFRAME_NO_TASKBAR;
#endif

  wxWindow::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

  m_pParent = parent;

  m_pitemBoxSizerLeg = new wxBoxSizer(wxVERTICAL);

  pThisLegText = new wxStaticText(this, -1, _("This Leg"));
  pThisLegText->Fit();
  m_pitemBoxSizerLeg->Add(pThisLegText, 0, wxALIGN_CENTER_HORIZONTAL, 2);

  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));

  wxFont* pThisLegFont = FontMgr::Get().FindOrCreateFont(
      qFont->GetPointSize(), wxFONTFAMILY_DEFAULT, qFont->GetStyle(),
      wxFONTWEIGHT_BOLD, false, qFont->GetFaceName());
  pThisLegText->SetFont(*pThisLegFont);

  pXTE = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pXTE->SetALabel("XTE");
  m_pitemBoxSizerLeg->Add(pXTE, 1, wxALIGN_LEFT | wxALL, 2);

  pBRG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pBRG->SetALabel("BRG");
  m_pitemBoxSizerLeg->Add(pBRG, 1, wxALIGN_LEFT | wxALL, 2);

  pVMG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pVMG->SetALabel("VMG");
  m_pitemBoxSizerLeg->Add(pVMG, 1, wxALIGN_LEFT | wxALL, 2);

  pRNG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pRNG->SetALabel("RNG");
  m_pitemBoxSizerLeg->Add(pRNG, 1, wxALIGN_LEFT | wxALL, 2);

  pTTG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pTTG->SetALabel("TTG  @VMG");
  m_pitemBoxSizerLeg->Add(pTTG, 1, wxALIGN_LEFT | wxALL, 2);

  //    Create CDI Display Window

  pCDI = new CDI(this, -1, wxSIMPLE_BORDER, "CDI");
  m_pitemBoxSizerLeg->AddSpacer(5);
  m_pitemBoxSizerLeg->Add(pCDI, 0, wxALL | wxEXPAND, 2);

  SetSizer(m_pitemBoxSizerLeg);  // use the sizer for layout
  m_pitemBoxSizerLeg->SetSizeHints(this);
  Layout();
  Fit();

  if (g_bShowRouteTotal)
    pThisLegText->SetLabel(_("Route"));
  else
    pThisLegText->SetLabel(_("This Leg"));

  Hide();
}

ConsoleCanvasWin::~ConsoleCanvasWin() { delete pCDI; }

void ConsoleCanvasWin::SetColorScheme(ColorScheme cs) {
  pbackBrush = wxTheBrushList->FindOrCreateBrush(
      GetGlobalColor("DILG1" /*UIBDR*/), wxBRUSHSTYLE_SOLID);
  SetBackgroundColour(GetGlobalColor("DILG1" /*"UIBDR"*/));

  if (g_bShowRouteTotal)
    pThisLegText->SetLabel(_("Route"));
  else
    pThisLegText->SetLabel(_("This Leg"));

  //  Also apply color scheme to all known children

  pThisLegText->SetBackgroundColour(GetGlobalColor("DILG1" /*"UIBDR"*/));

  pXTE->SetColorScheme(cs);
  pBRG->SetColorScheme(cs);
  pRNG->SetColorScheme(cs);
  pTTG->SetColorScheme(cs);
  pVMG->SetColorScheme(cs);

  pCDI->SetColorScheme(cs);
}

void ConsoleCanvasWin::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(this);

  if (g_pRouteMan->GetpActiveRoute()) {
    if (m_bNeedClear) {
      pThisLegText->Refresh();
      m_bNeedClear = false;
    }
  }

  if (!g_bShowActiveRouteHighway) pCDI->Hide();
}

void ConsoleCanvasWin::OnShow(wxShowEvent& event) {
  pCDI->Show(g_bShowActiveRouteHighway);
  m_pitemBoxSizerLeg->SetSizeHints(this);
}

void ConsoleCanvasWin::LegRoute() {
  if (g_bShowRouteTotal)
    pThisLegText->SetLabel(_("Route"));
  else
    pThisLegText->SetLabel(_("This Leg"));

  pThisLegText->Refresh(true);
  RefreshConsoleData();
}

void ConsoleCanvasWin::OnContextMenu(wxContextMenuEvent& event) {
  wxMenu* contextMenu = new wxMenu();
  wxMenuItem* btnLeg =
      new wxMenuItem(contextMenu, ID_NAVLEG, _("This Leg"), "", wxITEM_RADIO);
  wxMenuItem* btnRoute = new wxMenuItem(contextMenu, ID_NAVROUTE,
                                        _("Full Route"), "", wxITEM_RADIO);
  wxMenuItem* btnHighw = new wxMenuItem(contextMenu, ID_NAVHIGHWAY,
                                        _("Show Highway"), "", wxITEM_CHECK);
  contextMenu->Append(btnLeg);
  contextMenu->Append(btnRoute);
  contextMenu->AppendSeparator();
  contextMenu->Append(btnHighw);

  btnLeg->Check(!g_bShowRouteTotal);
  btnRoute->Check(g_bShowRouteTotal);
  btnHighw->Check(g_bShowActiveRouteHighway);

  PopupMenu(contextMenu);

  delete contextMenu;
}

void ConsoleCanvasWin::OnContextMenuSelection(wxCommandEvent& event) {
  switch (event.GetId()) {
    case ID_NAVLEG: {
      g_bShowRouteTotal = false;
      LegRoute();
      break;
    }
    case ID_NAVROUTE: {
      g_bShowRouteTotal = true;
      LegRoute();
      break;
    }
    case ID_NAVHIGHWAY: {
      g_bShowActiveRouteHighway = !g_bShowActiveRouteHighway;
      if (g_bShowActiveRouteHighway) {
        pCDI->Show();
      } else {
        pCDI->Hide();
      }
      m_pitemBoxSizerLeg->SetSizeHints(this);
      break;
    }
  }
}

void ConsoleCanvasWin::ToggleShowHighway() {
  g_bShowActiveRouteHighway = !g_bShowActiveRouteHighway;
  if (g_bShowActiveRouteHighway) {
    pCDI->Show();
  } else {
    pCDI->Hide();
  }
  m_pitemBoxSizerLeg->SetSizeHints(this);
  Layout();
}

void ConsoleCanvasWin::ToggleRouteTotalDisplay() {
  // Implement 3-state logic
  //  Route/SOG -> LEG/SOG -> LEG/VMG -> ....

  if (g_bShowRouteTotal) {
    g_bShowRouteTotal = false;
    m_speedUsed = SPEED_SOG;
  } else {
    if (m_speedUsed == SPEED_VMG) {
      g_bShowRouteTotal = true;
      m_speedUsed = SPEED_SOG;
    } else
      m_speedUsed = SPEED_VMG;
  }
  LegRoute();
}

void ConsoleCanvasWin::UpdateRouteData() {
  wxString str_buf;
  if (g_pRouteMan->GetpActiveRoute()) {
    if (g_pRouteMan->m_bDataValid) {
      // Range to the next waypoint is needed always
      float rng = g_pRouteMan->GetCurrentRngToActivePoint();

      // Brg to the next waypoint
      float dcog = g_pRouteMan->GetCurrentBrgToActivePoint();
      if (dcog >= 359.5) dcog = 0;

      wxString cogstr;
      if (g_bShowTrue)
        cogstr << wxString::Format(wxString("%6.0f", wxConvUTF8), dcog);
      if (g_bShowMag)
        cogstr << wxString::Format(wxString("%6.0f(M)", wxConvUTF8),
                                   toMagnetic(dcog));

      pBRG->SetAValue(cogstr);

      double speed = 0.;
      if (!std::isnan(gCog) && !std::isnan(gSog)) {
        double BRG;
        BRG = g_pRouteMan->GetCurrentBrgToActivePoint();
        double vmg = gSog * cos((BRG - gCog) * PI / 180.);
        str_buf.Printf("%6.2f", toUsrSpeed(vmg));

        if (m_speedUsed == SPEED_VMG) {
          // VMG
          // VMG is always to next waypoint, not to end of route
          // VMG is SOG x cosine (difference between COG and BRG to Waypoint)
          speed = vmg;
        } else {
          speed = gSog;
        }
      } else
        str_buf = "---";

      pVMG->SetAValue(str_buf);

      if (!g_bShowRouteTotal) {
        float nrng = g_pRouteMan->GetCurrentRngToActiveNormalArrival();
        wxString srng;
        double deltarng = fabs(rng - nrng);
        if ((deltarng > .01) && ((deltarng / rng) > .10) &&
            (rng < 10.0))  // show if there is more than 10% difference in
                           // ranges, etc...
        {
          if (nrng < 10.0)
            srng.Printf("%5.2f/%5.2f", toUsrDistance(rng), toUsrDistance(nrng));
          else
            srng.Printf("%5.1f/%5.1f", toUsrDistance(rng), toUsrDistance(nrng));
        } else {
          if (rng < 10.0)
            srng.Printf("%6.2f", toUsrDistance(rng));
          else
            srng.Printf("%6.1f", toUsrDistance(rng));
        }

        // RNG to the next WPT
        pRNG->SetAValue(srng);
        // XTE
        str_buf.Printf(
            "%6.2f", toUsrDistance(g_pRouteMan->GetCurrentXTEToActivePoint()));
        pXTE->SetAValue(str_buf);
        if (g_pRouteMan->GetXTEDir() < 0)
          pXTE->SetALabel(wxString(_("XTE         L")));
        else
          pXTE->SetALabel(wxString(_("XTE         R")));
        // TTG
        // In all cases, ttg/eta are declared invalid if VMG <= 0.
        // If showing only "this leg", use VMG for calculation of ttg
        wxString ttg_s;
        if ((speed > 0.) && !std::isnan(gCog) && !std::isnan(gSog)) {
          float ttg_sec = (rng / speed) * 3600.;
          wxTimeSpan ttg_span(0, 0, long(ttg_sec), 0);
          ttg_s = ttg_span.Format();
        } else
          ttg_s = "---";

        pTTG->SetAValue(ttg_s);
        if (m_speedUsed == SPEED_VMG) {
          pTTG->SetALabel(wxString(_("TTG  @VMG")));
        } else {
          pTTG->SetALabel(wxString(_("TTG  @SOG")));
        }
      } else {
        //    Remainder of route
        float trng = rng;

        Route* prt = g_pRouteMan->GetpActiveRoute();
        int n_addflag = 0;
        for (RoutePoint* prp : *prt->pRoutePointList) {
          if (n_addflag) trng += prp->m_seg_len;

          if (prp == prt->m_pRouteActivePoint) n_addflag++;
        }

        //                total rng
        wxString strng;
        if (trng < 10.0)
          strng.Printf("%6.2f", toUsrDistance(trng));
        else
          strng.Printf("%6.1f", toUsrDistance(trng));

        pRNG->SetAValue(strng);

        // total TTG
        // If showing total route TTG/ETA, use gSog for calculation

        wxString tttg_s;
        wxTimeSpan tttg_span;
        float tttg_sec = 0.0;
        if (speed > 0.) {
          tttg_sec = (trng / gSog) * 3600.;
          tttg_span = wxTimeSpan::Seconds((long)tttg_sec);
          // Show also #days if TTG > 24 h
          tttg_s = tttg_sec > SECONDS_PER_DAY ? tttg_span.Format(_("%Dd %H:%M"))
                                              : tttg_span.Format("%H:%M:%S");
        } else {
          tttg_span = wxTimeSpan::Seconds(0);
          tttg_s = "---";
        }

        pTTG->SetAValue(tttg_s);

        //                total ETA to be shown on XTE panel
        wxDateTime dtnow, eta;
        dtnow.SetToCurrent();
        eta = dtnow.Add(tttg_span);
        wxString seta;

        if (speed > 0.) {
          // Show date, e.g. Feb 15, if TTG > 24 h
          seta = tttg_sec > SECONDS_PER_DAY ? eta.Format("%d/%m %H:%M")
                                            : eta.Format("%H:%M");
        } else {
          seta = "---";
        }
        pXTE->SetAValue(seta);
        if (m_speedUsed == SPEED_VMG) {
          pTTG->SetALabel(wxString(_("TTG  @VMG")));
          pXTE->SetALabel(wxString(_("ETA  @VMG")));
        } else {
          pTTG->SetALabel(wxString(_("TTG  @SOG")));
          pXTE->SetALabel(wxString(_("ETA  @SOG")));
        }
      }

      pRNG->Refresh();
      pBRG->Refresh();
      pVMG->Refresh();
      pTTG->Refresh();
      pXTE->Refresh();
    }
  }
}

void ConsoleCanvasWin::RefreshConsoleData() {
  UpdateRouteData();

  pRNG->Refresh();
  pBRG->Refresh();
  pVMG->Refresh();
  pTTG->Refresh();
  pXTE->Refresh();
  pCDI->Refresh();
}

void ConsoleCanvasWin::ShowWithFreshFonts() {
  Hide();
  UpdateFonts();
  gFrame->PositionConsole();
  Show();
}

void ConsoleCanvasWin::UpdateFonts() {
  pBRG->RefreshFonts();
  pXTE->RefreshFonts();
  pTTG->RefreshFonts();
  pRNG->RefreshFonts();
  pVMG->RefreshFonts();

  m_pitemBoxSizerLeg->SetSizeHints(this);
  Layout();
  Fit();

  Refresh();
}

//------------------------------------------------------------------------------
//    ConsoleCanvasFrame Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ConsoleCanvasFrame, wxWindow)
EVT_PAINT(ConsoleCanvasFrame::OnPaint)
EVT_SHOW(ConsoleCanvasFrame::OnShow)
EVT_CONTEXT_MENU(ConsoleCanvasFrame::OnContextMenu)
EVT_MENU(ID_NAVLEG, ConsoleCanvasFrame::OnContextMenuSelection)
EVT_MENU(ID_NAVROUTE, ConsoleCanvasFrame::OnContextMenuSelection)
EVT_MENU(ID_NAVHIGHWAY, ConsoleCanvasFrame::OnContextMenuSelection)
END_EVENT_TABLE()

// Define a constructor for my canvas
ConsoleCanvasFrame::ConsoleCanvasFrame(wxWindow* parent) {
  m_speedUsed = SPEED_SOG;
  pbackBrush = NULL;
  m_bNeedClear = false;

  long style = wxSIMPLE_BORDER | wxCLIP_CHILDREN | wxFRAME_FLOAT_ON_PARENT;

#ifdef __WXMSW__
  style |= wxFRAME_NO_TASKBAR;
#endif

  wxFrame::Create(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                  style);

  m_pParent = parent;

  m_pitemBoxSizerLeg = new wxBoxSizer(wxVERTICAL);

  pThisLegText = new wxStaticText(this, -1, _("This Leg"));
  pThisLegText->Fit();
  m_pitemBoxSizerLeg->Add(pThisLegText, 0, wxALIGN_CENTER_HORIZONTAL, 2);

  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));

  wxFont* pThisLegFont = FontMgr::Get().FindOrCreateFont(
      qFont->GetPointSize(), wxFONTFAMILY_DEFAULT, qFont->GetStyle(),
      wxFONTWEIGHT_BOLD, false, qFont->GetFaceName());
  pThisLegText->SetFont(*pThisLegFont);

  pXTE = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pXTE->SetALabel("XTE");
  m_pitemBoxSizerLeg->Add(pXTE, 1, wxALIGN_LEFT | wxALL, 2);

  pBRG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pBRG->SetALabel("BRG");
  m_pitemBoxSizerLeg->Add(pBRG, 1, wxALIGN_LEFT | wxALL, 2);

  pVMG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pVMG->SetALabel("VMG");
  m_pitemBoxSizerLeg->Add(pVMG, 1, wxALIGN_LEFT | wxALL, 2);

  pRNG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pRNG->SetALabel("RNG");
  m_pitemBoxSizerLeg->Add(pRNG, 1, wxALIGN_LEFT | wxALL, 2);

  pTTG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
  pTTG->SetALabel("TTG  @VMG");
  m_pitemBoxSizerLeg->Add(pTTG, 1, wxALIGN_LEFT | wxALL, 2);

  //    Create CDI Display Window

  pCDI = new CDI(this, -1, wxSIMPLE_BORDER, "CDI");
  m_pitemBoxSizerLeg->AddSpacer(5);
  m_pitemBoxSizerLeg->Add(pCDI, 0, wxALL | wxEXPAND, 2);

  SetSizer(m_pitemBoxSizerLeg);  // use the sizer for layout
  m_pitemBoxSizerLeg->SetSizeHints(this);
  Layout();
  Fit();

  if (g_bShowRouteTotal)
    pThisLegText->SetLabel(_("Route"));
  else
    pThisLegText->SetLabel(_("This Leg"));

  Hide();
}

ConsoleCanvasFrame::~ConsoleCanvasFrame() { delete pCDI; }

void ConsoleCanvasFrame::SetColorScheme(ColorScheme cs) {
  pbackBrush = wxTheBrushList->FindOrCreateBrush(
      GetGlobalColor("DILG1" /*UIBDR*/), wxBRUSHSTYLE_SOLID);
  SetBackgroundColour(GetGlobalColor("DILG1" /*"UIBDR"*/));

  if (g_bShowRouteTotal)
    pThisLegText->SetLabel(_("Route"));
  else
    pThisLegText->SetLabel(_("This Leg"));

  //  Also apply color scheme to all known children

  pThisLegText->SetBackgroundColour(GetGlobalColor("DILG1" /*"UIBDR"*/));

  pXTE->SetColorScheme(cs);
  pBRG->SetColorScheme(cs);
  pRNG->SetColorScheme(cs);
  pTTG->SetColorScheme(cs);
  pVMG->SetColorScheme(cs);

  pCDI->SetColorScheme(cs);
}

void ConsoleCanvasFrame::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(this);

  if (g_pRouteMan->GetpActiveRoute()) {
    if (m_bNeedClear) {
      pThisLegText->Refresh();
      m_bNeedClear = false;
    }
    UpdateRouteData();
  }

  if (!g_bShowActiveRouteHighway) pCDI->Hide();
}

void ConsoleCanvasFrame::OnShow(wxShowEvent& event) {
  pCDI->Show(g_bShowActiveRouteHighway);
  m_pitemBoxSizerLeg->SetSizeHints(this);
}

void ConsoleCanvasFrame::LegRoute() {
  if (g_bShowRouteTotal)
    pThisLegText->SetLabel(_("Route"));
  else
    pThisLegText->SetLabel(_("This Leg"));

  pThisLegText->Refresh(true);
  RefreshConsoleData();
}

void ConsoleCanvasFrame::OnContextMenu(wxContextMenuEvent& event) {
  wxMenu* contextMenu = new wxMenu();
  wxMenuItem* btnLeg =
      new wxMenuItem(contextMenu, ID_NAVLEG, _("This Leg"), "", wxITEM_RADIO);
  wxMenuItem* btnRoute = new wxMenuItem(contextMenu, ID_NAVROUTE,
                                        _("Full Route"), "", wxITEM_RADIO);
  wxMenuItem* btnHighw = new wxMenuItem(contextMenu, ID_NAVHIGHWAY,
                                        _("Show Highway"), "", wxITEM_CHECK);

#ifdef __ANDROID__
  wxFont sFont = GetOCPNGUIScaledFont(_("Menu"));
  btnLeg->SetFont(sFont);
  btnRoute->SetFont(sFont);
  btnHighw->SetFont(sFont);
#endif

  contextMenu->Append(btnLeg);
  contextMenu->Append(btnRoute);
  contextMenu->AppendSeparator();
  contextMenu->Append(btnHighw);

  btnLeg->Check(!g_bShowRouteTotal);
  btnRoute->Check(g_bShowRouteTotal);
  btnHighw->Check(g_bShowActiveRouteHighway);

  PopupMenu(contextMenu);

  delete contextMenu;
}

void ConsoleCanvasFrame::OnContextMenuSelection(wxCommandEvent& event) {
  switch (event.GetId()) {
    case ID_NAVLEG: {
      g_bShowRouteTotal = false;
      LegRoute();
      break;
    }
    case ID_NAVROUTE: {
      g_bShowRouteTotal = true;
      LegRoute();
      break;
    }
    case ID_NAVHIGHWAY: {
      g_bShowActiveRouteHighway = !g_bShowActiveRouteHighway;
      if (g_bShowActiveRouteHighway) {
        pCDI->Show();
      } else {
        pCDI->Hide();
      }
      m_pitemBoxSizerLeg->SetSizeHints(this);
      break;
    }
  }
}

void ConsoleCanvasFrame::ToggleShowHighway() {
  g_bShowActiveRouteHighway = !g_bShowActiveRouteHighway;
  if (g_bShowActiveRouteHighway) {
    pCDI->Show();
  } else {
    pCDI->Hide();
  }
  m_pitemBoxSizerLeg->SetSizeHints(this);
  Layout();
}

void ConsoleCanvasFrame::ToggleRouteTotalDisplay() {
  // Implement 3-state logic
  //  Route/SOG -> LEG/SOG -> LEG/VMG -> ....

  if (g_bShowRouteTotal) {
    g_bShowRouteTotal = false;
    m_speedUsed = SPEED_SOG;
  } else {
    if (m_speedUsed == SPEED_VMG) {
      g_bShowRouteTotal = true;
      m_speedUsed = SPEED_SOG;
    } else
      m_speedUsed = SPEED_VMG;
  }
  LegRoute();
}

void ConsoleCanvasFrame::UpdateRouteData() {
  wxString str_buf;

  if (g_pRouteMan->GetpActiveRoute()) {
    if (g_pRouteMan->m_bDataValid) {
      // Range to the next waypoint is needed always
      float rng = g_pRouteMan->GetCurrentRngToActivePoint();

      // Brg to the next waypoint
      float dcog = g_pRouteMan->GetCurrentBrgToActivePoint();
      if (dcog >= 359.5) dcog = 0;

      wxString cogstr;
      if (g_bShowTrue)
        cogstr << wxString::Format(wxString("%6.0f", wxConvUTF8), dcog);
      if (g_bShowMag)
        cogstr << wxString::Format(wxString("%6.0f(M)", wxConvUTF8),
                                   toMagnetic(dcog));

      pBRG->SetAValue(cogstr);

      double speed = 0.;
      if (!std::isnan(gCog) && !std::isnan(gSog)) {
        double BRG;
        BRG = g_pRouteMan->GetCurrentBrgToActivePoint();
        double vmg = gSog * cos((BRG - gCog) * PI / 180.);
        str_buf.Printf("%6.2f", toUsrSpeed(vmg));

        if (m_speedUsed == SPEED_VMG) {
          // VMG
          // VMG is always to next waypoint, not to end of route
          // VMG is SOG x cosine (difference between COG and BRG to Waypoint)
          speed = vmg;
        } else {
          speed = gSog;
        }
      } else
        str_buf = "---";

      pVMG->SetAValue(str_buf);

      if (!g_bShowRouteTotal) {
        float nrng = g_pRouteMan->GetCurrentRngToActiveNormalArrival();
        wxString srng;
        double deltarng = fabs(rng - nrng);
        if ((deltarng > .01) && ((deltarng / rng) > .10) &&
            (rng < 10.0))  // show if there is more than 10% difference in
                           // ranges, etc...
        {
          if (nrng < 10.0)
            srng.Printf("%5.2f/%5.2f", toUsrDistance(rng), toUsrDistance(nrng));
          else
            srng.Printf("%5.1f/%5.1f", toUsrDistance(rng), toUsrDistance(nrng));
        } else {
          if (rng < 10.0)
            srng.Printf("%6.2f", toUsrDistance(rng));
          else
            srng.Printf("%6.1f", toUsrDistance(rng));
        }

        // RNG to the next WPT
        pRNG->SetAValue(srng);
        // XTE
        str_buf.Printf(
            "%6.2f", toUsrDistance(g_pRouteMan->GetCurrentXTEToActivePoint()));
        pXTE->SetAValue(str_buf);
        if (g_pRouteMan->GetXTEDir() < 0)
          pXTE->SetALabel(wxString(_("XTE         L")));
        else
          pXTE->SetALabel(wxString(_("XTE         R")));
        // TTG
        // In all cases, ttg/eta are declared invalid if VMG <= 0.
        // If showing only "this leg", use VMG for calculation of ttg
        wxString ttg_s;
        if ((speed > 0.) && !std::isnan(gCog) && !std::isnan(gSog)) {
          float ttg_sec = (rng / speed) * 3600.;
          wxTimeSpan ttg_span(0, 0, long(ttg_sec), 0);
          ttg_s = ttg_span.Format();
        } else
          ttg_s = "---";

        pTTG->SetAValue(ttg_s);
        if (m_speedUsed == SPEED_VMG) {
          pTTG->SetALabel(wxString(_("TTG  @VMG")));
        } else {
          pTTG->SetALabel(wxString(_("TTG  @SOG")));
        }
      } else {
        //    Remainder of route
        float trng = rng;

        Route* prt = g_pRouteMan->GetpActiveRoute();
        int n_addflag = 0;
        for (RoutePoint* prp : *prt->pRoutePointList) {
          if (n_addflag) trng += prp->m_seg_len;

          if (prp == prt->m_pRouteActivePoint) n_addflag++;
        }

        //                total rng
        wxString strng;
        if (trng < 10.0)
          strng.Printf("%6.2f", toUsrDistance(trng));
        else
          strng.Printf("%6.1f", toUsrDistance(trng));

        pRNG->SetAValue(strng);

        // total TTG
        // If showing total route TTG/ETA, use gSog for calculation

        wxString tttg_s;
        wxTimeSpan tttg_span;
        float tttg_sec = 0.0;
        if (speed > 0.) {
          tttg_sec = (trng / gSog) * 3600.;
          tttg_span = wxTimeSpan::Seconds((long)tttg_sec);
          // Show also #days if TTG > 24 h
          tttg_s = tttg_sec > SECONDS_PER_DAY ? tttg_span.Format(_("%Dd %H:%M"))
                                              : tttg_span.Format("%H:%M:%S");
        } else {
          tttg_span = wxTimeSpan::Seconds(0);
          tttg_s = "---";
        }

        pTTG->SetAValue(tttg_s);

        //                total ETA to be shown on XTE panel
        wxDateTime dtnow, eta;
        dtnow.SetToCurrent();
        eta = dtnow.Add(tttg_span);
        wxString seta;

        if (speed > 0.) {
          // Show date, e.g. Feb 15, if TTG > 24 h
          seta = tttg_sec > SECONDS_PER_DAY ? eta.Format("%d/%m %H:%M")
                                            : eta.Format("%H:%M");
        } else {
          seta = "---";
        }
        pXTE->SetAValue(seta);
        if (m_speedUsed == SPEED_VMG) {
          pTTG->SetALabel(wxString(_("TTG  @VMG")));
          pXTE->SetALabel(wxString(_("ETA  @VMG")));
        } else {
          pTTG->SetALabel(wxString(_("TTG  @SOG")));
          pXTE->SetALabel(wxString(_("ETA  @SOG")));
        }
      }

      pRNG->Refresh();
      pBRG->Refresh();
      pVMG->Refresh();
      pTTG->Refresh();
      pXTE->Refresh();
    }
  }
}

void ConsoleCanvasFrame::RefreshConsoleData() {
  UpdateRouteData();

  pRNG->Refresh();
  pBRG->Refresh();
  pVMG->Refresh();
  pTTG->Refresh();
  pXTE->Refresh();
  pCDI->Refresh();
}

void ConsoleCanvasFrame::ShowWithFreshFonts() {
  Hide();
  Move(0, 0);
  UpdateFonts();
  gFrame->PositionConsole();
  Show();
}

void ConsoleCanvasFrame::UpdateFonts() {
  pBRG->RefreshFonts();
  pXTE->RefreshFonts();
  pTTG->RefreshFonts();
  pRNG->RefreshFonts();
  pVMG->RefreshFonts();

  m_pitemBoxSizerLeg->SetSizeHints(this);
  Layout();
  Fit();

  Refresh();
}

//------------------------------------------------------------------------------
//    AnnunText Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(AnnunText, wxWindow)
EVT_PAINT(AnnunText::OnPaint)
EVT_MOUSE_EVENTS(AnnunText::MouseEvent)
END_EVENT_TABLE()

AnnunText::AnnunText(wxWindow* parent, wxWindowID id,
                     const wxString& LegendElement,
                     const wxString& ValueElement)
    : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER) {
  m_label = "Label";
  m_value = "-----";

  m_plabelFont = FontMgr::Get().FindOrCreateFont(
      14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE,
      wxString("Arial Bold"));
  m_pvalueFont = FontMgr::Get().FindOrCreateFont(
      24, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE,
      wxString("helvetica"), wxFONTENCODING_ISO8859_1);

  m_LegendTextElement = LegendElement;
  m_ValueTextElement = ValueElement;

  RefreshFonts();
}

AnnunText::~AnnunText() {}
void AnnunText::MouseEvent(wxMouseEvent& event) {
  ConsoleCanvasWin* ccpw = dynamic_cast<ConsoleCanvasWin*>(GetParent());
  if (ccpw) {
    if (event.RightDown()) {
      wxContextMenuEvent cevt;
      cevt.SetPosition(event.GetPosition());
      ccpw->OnContextMenu(cevt);
    } else if (event.LeftDown()) {
      ccpw->ToggleRouteTotalDisplay();
    }
  } else {
    ConsoleCanvasFrame* ccpf = dynamic_cast<ConsoleCanvasFrame*>(GetParent());
    if (ccpf) {
      if (event.RightDown()) {
        wxContextMenuEvent cevt;
        cevt.SetPosition(event.GetPosition());
        ccpf->OnContextMenu(cevt);
      } else if (event.LeftDown()) {
        ccpf->ToggleRouteTotalDisplay();
      }
    }
  }
}

void AnnunText::CalculateMinSize() {
  //    Calculate the minimum required size of the window based on text size

  int wl = 50;  // reasonable defaults?
  int hl = 20;
  int wv = 50;
  int hv = 20;

  if (m_plabelFont)
    GetTextExtent("TTG @SOG", &wl, &hl, NULL, NULL, m_plabelFont);

  if (m_pvalueFont)
    GetTextExtent("123.4567", &wv, &hv, NULL, NULL, m_pvalueFont);

  double pdifactor = g_BasePlatform->GetDisplayDIPMult(gFrame);
  wl *= pdifactor;
  hl *= pdifactor;
  wv *= pdifactor;
  hv *= pdifactor;

  wxSize min;
  min.x = wv * 1.2;
  min.x = wxMax(min.x, wl * 1.2);
  min.y = (int)((hl + hv) * 1.2);

  SetMinSize(min);

  // resize background to the necessary size
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
  if (style->consoleTextBackground.IsOk()) {
    wxImage img = style->consoleTextBackground.ConvertToImage();
    style->consoleTextBackground = wxBitmap(img.Rescale(min.x, min.y));
  }
}

void AnnunText::SetColorScheme(ColorScheme cs) {
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
  m_backBrush = *wxTheBrushList->FindOrCreateBrush(GetGlobalColor("UBLCK"),
                                                   wxBRUSHSTYLE_SOLID);

  m_default_text_color = style->consoleFontColor;
  RefreshFonts();
}

void AnnunText::RefreshFonts() {
  wxFont* pl = FontMgr::Get().GetFont(m_LegendTextElement);
  m_plabelFont = FontMgr::Get().FindOrCreateFont(
      pl->GetPointSize() / OCPN_GetWinDIPScaleFactor(), pl->GetFamily(),
      pl->GetStyle(), pl->GetWeight(), FALSE, pl->GetFaceName());

  wxFont* pv = FontMgr::Get().GetFont(m_ValueTextElement);
  m_pvalueFont = FontMgr::Get().FindOrCreateFont(
      pv->GetPointSize() / OCPN_GetWinDIPScaleFactor(), pv->GetFamily(),
      pv->GetStyle(), pv->GetWeight(), FALSE, pv->GetFaceName());

  m_legend_color = FontMgr::Get().GetFontColor(_("Console Legend"));
  m_val_color = FontMgr::Get().GetFontColor(_("Console Value"));

  CalculateMinSize();

  // Make sure that the background color and the text colors are not too close,
  // for contrast
  if (m_backBrush.IsOk()) {
    wxColour back_color = m_backBrush.GetColour();

    wxColour legend_color = m_legend_color;
    if ((abs(legend_color.Red() - back_color.Red()) < 5) &&
        (abs(legend_color.Green() - back_color.Blue()) < 5) &&
        (abs(legend_color.Blue() - back_color.Blue()) < 5))
      m_legend_color = m_default_text_color;

    wxColour value_color = m_val_color;
    if ((abs(value_color.Red() - back_color.Red()) < 5) &&
        (abs(value_color.Green() - back_color.Blue()) < 5) &&
        (abs(value_color.Blue() - back_color.Blue()) < 5))
      m_val_color = m_default_text_color;
  }
}

void AnnunText::SetLegendElement(const wxString& element) {
  m_LegendTextElement = element;
}

void AnnunText::SetValueElement(const wxString& element) {
  m_ValueTextElement = element;
}

void AnnunText::SetALabel(const wxString& l) { m_label = l; }

void AnnunText::SetAValue(const wxString& v) { m_value = v; }

void AnnunText::OnPaint(wxPaintEvent& event) {
  int sx, sy;
  GetClientSize(&sx, &sy);
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

  //    Do the drawing on an off-screen memory DC, and blit into place
  //    to avoid objectionable flashing
  wxMemoryDC mdc;

  wxBitmap m_bitmap(sx, sy, -1);
  mdc.SelectObject(m_bitmap);
  mdc.SetBackground(m_backBrush);
  mdc.Clear();

  if (style->consoleTextBackground.IsOk())
    mdc.DrawBitmap(style->consoleTextBackground, 0, 0);

  mdc.SetTextForeground(m_default_text_color);

  if (m_plabelFont) {
    mdc.SetFont(*m_plabelFont);
    mdc.SetTextForeground(m_legend_color);
    mdc.DrawText(m_label, 5, 2);
  }

  if (m_pvalueFont) {
    mdc.SetFont(*m_pvalueFont);
    mdc.SetTextForeground(m_val_color);

    int w, h;
    mdc.GetTextExtent(m_value, &w, &h);
    int cw, ch;
    mdc.GetSize(&cw, &ch);

    mdc.DrawText(m_value, cw - w - 2, ch - h - 2);
  }

  wxPaintDC dc(this);
  dc.Blit(0, 0, sx, sy, &mdc, 0, 0);
}
//------------------------------------------------------------------------------
//    CDI Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(CDI, wxWindow)
EVT_PAINT(CDI::OnPaint)
EVT_MOUSE_EVENTS(CDI::MouseEvent)
END_EVENT_TABLE()

CDI::CDI(wxWindow* parent, wxWindowID id, long style, const wxString& name)
    : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, style, name)

{
  SetMinSize(wxSize(10, 150));
}

void CDI::MouseEvent(wxMouseEvent& event) {
#ifdef __ANDROID__
  if (event.RightDown()) {
    qDebug() << "right down";

    wxContextMenuEvent cevt;
    cevt.SetPosition(event.GetPosition());

    ConsoleCanvasFrame* ccp = dynamic_cast<ConsoleCanvasFrame*>(GetParent());
    if (ccp) ccp->OnContextMenu(cevt);
  }
#endif
}

void CDI::SetColorScheme(ColorScheme cs) {
  m_pbackBrush = wxTheBrushList->FindOrCreateBrush(GetGlobalColor("DILG2"),
                                                   wxBRUSHSTYLE_SOLID);
  m_proadBrush = wxTheBrushList->FindOrCreateBrush(GetGlobalColor("DILG1"),
                                                   wxBRUSHSTYLE_SOLID);
  m_proadPen = wxThePenList->FindOrCreatePen(GetGlobalColor("CHBLK"), 1,
                                             wxPENSTYLE_SOLID);
}

void CDI::OnPaint(wxPaintEvent& event) {
  int sx, sy;
  GetClientSize(&sx, &sy);

  //    Do the drawing on an off-screen memory DC, and blit into place
  //    to avoid objectionable flashing
  wxMemoryDC mdc;

  wxBitmap m_bitmap(sx, sy, -1);
  mdc.SelectObject(m_bitmap);
  mdc.SetBackground(*m_pbackBrush);
  mdc.Clear();

  int xp = sx / 2;
  int yp = sy * 9 / 10;

  int path_length = sy * 3;
  int pix_per_xte = 120;

  if (g_pRouteMan->GetpActiveRoute()) {
    double angle = 90 - (g_pRouteMan->GetCurrentSegmentCourse() - gCog);

    double dy = path_length * sin(angle * PI / 180.);
    double dx = path_length * cos(angle * PI / 180.);

    int xtedir;
    xtedir = g_pRouteMan->GetXTEDir();
    double xte = g_pRouteMan->GetCurrentXTEToActivePoint();

    double ddy = xtedir * pix_per_xte * xte * sin((90 - angle) * PI / 180.);
    double ddx = xtedir * pix_per_xte * xte * cos((90 - angle) * PI / 180.);

    int ddxi = (int)ddx;
    int ddyi = (int)ddy;

    int xc1 = xp - (int)(dx / 2) + ddxi;
    int yc1 = yp + (int)(dy / 2) + ddyi;
    int xc2 = xp + (int)(dx / 2) + ddxi;
    int yc2 = yp - (int)(dy / 2) + ddyi;

    wxPoint road[4];

    int road_top_width = 10;
    int road_bot_width = 40;

    road[0].x = xc1 - (int)(road_bot_width * cos((90 - angle) * PI / 180.));
    road[0].y = yc1 - (int)(road_bot_width * sin((90 - angle) * PI / 180.));

    road[1].x = xc2 - (int)(road_top_width * cos((90 - angle) * PI / 180.));
    road[1].y = yc2 - (int)(road_top_width * sin((90 - angle) * PI / 180.));

    road[2].x = xc2 + (int)(road_top_width * cos((90 - angle) * PI / 180.));
    road[2].y = yc2 + (int)(road_top_width * sin((90 - angle) * PI / 180.));

    road[3].x = xc1 + (int)(road_bot_width * cos((90 - angle) * PI / 180.));
    road[3].y = yc1 + (int)(road_bot_width * sin((90 - angle) * PI / 180.));

    mdc.SetBrush(*m_proadBrush);
    mdc.SetPen(*m_proadPen);
    mdc.DrawPolygon(4, road, 0, 0, wxODDEVEN_RULE);

    ///        mdc.DrawLine( xc1, yc1, xc2, yc2 );

    mdc.DrawLine(0, yp, sx, yp);
    mdc.DrawCircle(xp, yp, 6);
    mdc.DrawLine(xp, yp + 5, xp, yp - 5);
  }

  wxPaintDC dc(this);
  dc.Blit(0, 0, sx, sy, &mdc, 0, 0);
}

#if defined(__WXMSW__) || defined(__WXMAC__) || defined(__ANDROID__)
APConsole::APConsole(wxWindow* parent) {
  m_con_frame = new ConsoleCanvasFrame(gFrame);
}
APConsole::~APConsole() {}
void APConsole::SetColorScheme(ColorScheme cs) {
  m_con_frame->SetColorScheme(cs);
}
bool APConsole::IsShown() { return m_con_frame->IsShown(); }
void APConsole::UpdateFonts() { m_con_frame->UpdateFonts(); }
void APConsole::RefreshConsoleData() { m_con_frame->RefreshConsoleData(); }
void APConsole::Raise() { m_con_frame->Raise(); }
void APConsole::ShowWithFreshFonts() { m_con_frame->ShowWithFreshFonts(); }
void APConsole::Show(bool bshow) { m_con_frame->Show(bshow); }
CDI* APConsole::GetCDI() { return m_con_frame->pCDI; }
wxSize APConsole::GetSize() { return m_con_frame->GetSize(); }
void APConsole::ToggleShowHighway() { m_con_frame->ToggleShowHighway(); }
void APConsole::Move(wxPoint p) { m_con_frame->Move(p); }

#else

APConsole::APConsole(wxWindow* parent) {
  m_con_win = new ConsoleCanvasWin(parent);
}
APConsole::~APConsole() {}
void APConsole::SetColorScheme(ColorScheme cs) {
  m_con_win->SetColorScheme(cs);
}
bool APConsole::IsShown() { return m_con_win->IsShown(); }
void APConsole::UpdateFonts() { m_con_win->UpdateFonts(); }
void APConsole::RefreshConsoleData() { m_con_win->RefreshConsoleData(); }
void APConsole::Raise() {}
void APConsole::ShowWithFreshFonts() { m_con_win->ShowWithFreshFonts(); }
void APConsole::Show(bool bshow) { m_con_win->Show(bshow); }
CDI* APConsole::GetCDI() { return m_con_win->pCDI; }
wxSize APConsole::GetSize() { return m_con_win->GetSize(); }
void APConsole::ToggleShowHighway() { m_con_win->ToggleShowHighway(); }
void APConsole::Move(wxPoint p) { m_con_win->Move(p); }

#endif
