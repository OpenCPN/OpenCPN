/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Authors:  David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "RadarPanel.h"
#include "RadarCanvas.h"
#include "RadarInfo.h"

PLUGIN_BEGIN_NAMESPACE

RadarPanel::RadarPanel(radar_pi* pi, RadarInfo* ri, wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _("RADAR")) {
  m_parent = parent;
  m_pi = pi;
  m_ri = ri;
}

bool RadarPanel::Create() {
  m_aui_mgr = GetFrameAuiManager();

  m_aui_name = wxString::Format(wxT("radar_pi-%d"), (int)m_ri->m_radar);
  wxAuiPaneInfo pane = wxAuiPaneInfo()
                           .Name(m_aui_name)
                           .Caption(m_ri->m_name)
                           .CaptionVisible(true)  // Show caption even when docked
                           .Movable(true)
                           .TopDockable(false)
                           .BottomDockable(false)
                           .RightDockable(false)
                           .LeftDockable(false)
#ifndef __WXMAC__
                           .Hide()
#endif
                           .CloseButton(true);

  m_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_text = new wxStaticText(this, 0, wxT(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
  m_sizer->Add(m_text, 0, wxEXPAND | wxALL, 0);
  SetSizer(m_sizer);

  DimeWindow(this);
  Fit();
  Layout();
  m_best_size = wxGetDisplaySize();
  wxSize opencpn_window = GetOCPNCanvasWindow()->GetSize();
  m_best_size.x = wxMin(wxMin(m_best_size.x / 2, 512), opencpn_window.x / 3);
  m_best_size.y = wxMin(wxMin(m_best_size.y / 2, 512), opencpn_window.y / 2);
  LOG_DIALOG(wxT("radar_pi: best size %i, %i"), m_best_size.x, m_best_size.y);

  pane.MinSize(256, 256);
  pane.BestSize(m_best_size);
  pane.FloatingSize(m_best_size);
  pane.FloatingPosition(M_SETTINGS.window_pos[m_ri->m_radar]);
  pane.Right().Float();
  pane.dock_proportion = 100000;  // Secret sauce to get panels to use entire bar
  pane.dock_layer = 1;

  m_aui_mgr->AddPane(this, pane);
  m_aui_mgr->Connect(wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(RadarPanel::close), NULL, this);

  if (m_pi->m_perspective[m_ri->m_radar].length()) {
    // Do this first and it doesn't work if the pane starts docked.
    LOG_DIALOG(wxT("radar_pi: Restoring panel %s to AUI control manager: %s"), m_aui_name.c_str(),
               m_pi->m_perspective[m_ri->m_radar].c_str());
    m_aui_mgr->LoadPaneInfo(m_pi->m_perspective[m_ri->m_radar], pane);
    m_aui_mgr->Update();
  } else {
    LOG_DIALOG(wxT("radar_pi: Added panel %s to AUI control manager"), m_aui_name.c_str());
  }

  // dock or undock
  if (m_pi->m_settings.dock_radar[m_ri->m_radar]) {  // dock PPI
    pane.dock_layer = 1;
    pane.Dockable(true).CaptionVisible().Right().Dock();
    m_aui_mgr->Update();
  } else {  // float PPI
    pane.Dockable(false).Movable(true).CloseButton().CaptionVisible().Float();
    pane.dock_layer = 1;
    m_aui_mgr->Update();
  }
  return true;
}

RadarPanel::~RadarPanel() {
  wxAuiPaneInfo& pane = m_aui_mgr->GetPane(this);

  bool wasFloating = pane.IsFloating();
  if (!wasFloating) {
    pane.Float();
    m_aui_mgr->Update();
    pane = m_aui_mgr->GetPane(this);
  }

  wxPoint pos = pane.floating_pos;
  LOG_DIALOG(wxT("%s saved position %d,%d"), m_aui_name.c_str(), pos.x, pos.y);
  m_pi->m_settings.window_pos[m_ri->m_radar] = pos;

  if (!wasFloating) {
    pane.Dock();
    m_aui_mgr->Update();
    pane = m_aui_mgr->GetPane(this);
  }

  m_pi->m_perspective[m_ri->m_radar] = m_aui_mgr->SavePaneInfo(pane);
  if (m_ri->m_radar_canvas) {
    m_sizer->Detach(m_ri->m_radar_canvas);
    delete m_ri->m_radar_canvas;
    m_ri->m_radar_canvas = 0;
  }
  m_aui_mgr->DetachPane(this);
  // m_aui_mgr->Update();
  LOG_DIALOG(wxT("radar_pi: %s panel removed"), m_ri->m_name.c_str());
}

void RadarPanel::SetCaption(wxString name) { m_aui_mgr->GetPane(this).Caption(name); }

void RadarPanel::close(wxAuiManagerEvent& event) {
  event.Skip();
  // Save position of radar control before it is too late
  if (m_ri->m_control_dialog) {
    wxPoint pos = m_ri->m_control_dialog->GetPosition();
    LOG_DIALOG(wxT("X saved position ,%i, %i"), pos.x, pos.y);
    m_pi->m_settings.control_pos[m_ri->m_radar] = pos;
  }

  wxAuiPaneInfo* pane = event.GetPane();

  if (pane->window == this) {
    m_pi->m_settings.show_radar[m_ri->m_radar] = 0;
    LOG_DIALOG(wxT("radar_pi: RadarPanel::close: show_radar[%d]=%d"), m_ri->m_radar, 0);
    m_pi->NotifyRadarWindowViz();
  } else {
    LOG_DIALOG(wxT("radar_pi: RadarPanel::close: ignore close of window %s in window %s"), pane->name.c_str(), m_aui_name.c_str());
  }
}

void RadarPanel::ShowFrame(bool visible) {
  LOG_DIALOG(wxT("radar_pi %s: set visible %d"), m_ri->m_name.c_str(), visible);

  wxAuiPaneInfo& pane = m_aui_mgr->GetPane(this);

  if (!m_pi->IsOpenGLEnabled() && m_ri->m_radar_canvas) {
    m_sizer->Detach(m_ri->m_radar_canvas);
    delete m_ri->m_radar_canvas;
    m_ri->m_radar_canvas = 0;
    m_text->SetLabel(_("OpenGL mode required"));
    m_sizer->Show(m_text);
    DimeWindow(this);
    Fit();
    Layout();
  }
  if (visible) {
    if (m_pi->IsOpenGLEnabled() && !m_ri->m_radar_canvas) {
      LOG_DIALOG(wxT("radar_pi %s: creating OpenGL canvas"), m_ri->m_name.c_str());
      m_ri->m_radar_canvas = new RadarCanvas(m_pi, m_ri, this, GetSize());
      if (!m_ri->m_radar_canvas) {
        m_text->SetLabel(_("Unable to create OpenGL canvas"));
        m_sizer->Show(m_text);
      } else {
        m_sizer->Hide(m_text);
        m_sizer->Add(m_ri->m_radar_canvas, 0, wxEXPAND | wxALL, 0);
        Fit();
        Layout();
      }
    }
  }

  if (visible) {
    m_pi->m_settings.show_radar[m_ri->m_radar] = 1;
    LOG_DIALOG(wxT("radar_pi: RadarPanel::ShowFrame: show_radar[%d]=%d"), m_ri->m_radar, 1);
  }

  // What should have been a simple 'pane.Show(visible)' has devolved into a terrible hack.
  // When the entire dock row disappears because we're removing the last pane from it then the
  // next time we restore the dock gets its original size again. This is not want customers want.
  // So we store the size of the dock just before hiding the pane. This is done via parsing of the
  // perspective string, as there is no other way to access the dock information through wxAUI.

  if (!visible) {
    if (pane.IsDocked()) {
      m_dock = wxString::Format(wxT("|dock_size(%d,%d,%d)="), pane.dock_direction, pane.dock_layer, pane.dock_row);
      wxString perspective = m_aui_mgr->SavePerspective();
      int p = perspective.Find(m_dock);
      if (p != wxNOT_FOUND) {
        perspective = perspective.Mid(p + m_dock.length());
        perspective = perspective.BeforeFirst(wxT('|'));
        m_pi->m_settings.dock_size = wxAtoi(perspective);
        LOG_DIALOG(wxT("radar_pi: %s: replaced=%s, saved dock_size = %d"), m_ri->m_name.c_str(), perspective.c_str(),
                   m_pi->m_settings.dock_size);
      }
    }
  } else {
    pane.Position(m_ri->m_radar);
  }

  if (IsPaneShown() == visible) {
    return;
  }

  pane.Show(visible);
  m_aui_mgr->Update();

  if (visible && (m_pi->m_settings.dock_size > 0)) {
    // Now the reverse: take the new perspective string and replace the dock size of the dock that our pane is in and
    // reset it to the width it was before the hide.
    wxString perspective = m_aui_mgr->SavePerspective();
    int p = perspective.Find(m_dock);
    if (p != wxNOT_FOUND) {
      wxString newPerspective = perspective.Left(p);
      newPerspective << m_dock;
      newPerspective << m_pi->m_settings.dock_size;
      perspective = perspective.Mid(p + m_dock.length());
      newPerspective << wxT("|");
      newPerspective << perspective.AfterFirst(wxT('|'));
      m_aui_mgr->LoadPerspective(newPerspective);
      LOG_DIALOG(wxT("radar_pi: %s: new perspective %s"), m_ri->m_name.c_str(), newPerspective.c_str());
    }
  }
  m_aui_mgr->Update();
}

bool RadarPanel::IsPaneShown() {
  if (m_aui_mgr) {
    return m_aui_mgr->GetPane(this).IsShown();
  }
  return false;
}

wxPoint RadarPanel::GetPos() {
  if (m_aui_mgr && m_aui_mgr->GetPane(this).IsFloating()) {
    return GetParent()->GetScreenPosition();
  }
  return GetScreenPosition();
}

PLUGIN_END_NAMESPACE
