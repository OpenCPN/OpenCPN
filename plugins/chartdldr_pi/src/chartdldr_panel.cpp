/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_pi.h"
#include "chartdldr_panel.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_transfer.h"

#include <wx/log.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/window.h>

namespace {

void DetachFromParentSizer(wxWindow* panel) {
  if (!panel) {
    return;
  }
  if (wxWindow* const parent = panel->GetParent()) {
    if (wxSizer* const sizer = parent->GetSizer()) {
      sizer->Detach(panel);
    }
  }
}

wxSizer* EnsureVerticalSizer(wxWindow* window) {
  wxSizer* sizer = window->GetSizer();
  if (!sizer) {
    sizer = new wxBoxSizer(wxVERTICAL);
    window->SetSizer(sizer);
  }
  return sizer;
}

}  // namespace

bool ChartDldrPanelOnOptionsPage(const chartdldr_pi* pi) {
  return pi && pi->m_dldrpanel && pi->m_pOptionsPage &&
         pi->m_dldrpanel->GetParent() == pi->m_pOptionsPage;
}

void ChartDldrParkDownloaderPanelOnHost(chartdldr_pi* pi) {
  if (!pi || !pi->m_dldrpanel || !pi->m_panel_host) {
    return;
  }

  ChartDldrPanelImpl* const panel = pi->m_dldrpanel;
  wxPanel* const host = pi->m_panel_host;
  if (panel->GetParent() == host) {
    return;
  }

  DetachFromParentSizer(panel);
  panel->Reparent(host);
  panel->Hide();

  wxSizer* const host_sizer = EnsureVerticalSizer(host);
  host_sizer->Detach(panel);
  host_sizer->Add(panel, 1, wxEXPAND);
}

void ChartDldrShowDownloaderPanelOnOptions(chartdldr_pi* pi,
                                           wxScrolledWindow* page) {
  if (!pi || !pi->m_dldrpanel || !page) {
    return;
  }

  ChartDldrPanelImpl* const panel = pi->m_dldrpanel;
  DetachFromParentSizer(panel);

  if (wxSizer* const old_sizer = page->GetSizer()) {
    old_sizer->Detach(panel);
    old_sizer->Clear(false);
    delete old_sizer;
  }

  wxBoxSizer* const sizer = new wxBoxSizer(wxVERTICAL);
  page->SetSizer(sizer);
  panel->Reparent(page);
  page->InvalidateBestSize();
  sizer->Add(panel, 1, wxALL | wxEXPAND);
}

void ChartDldrDestroyDownloaderUI(chartdldr_pi* pi) {
  if (!pi) {
    return;
  }

  if (pi->m_dldrpanel) {
    // Park first so Destroy does not race an options-page teardown path.
    ChartDldrParkDownloaderPanelOnHost(pi);
    ChartDldrPanelImpl* const panel = pi->m_dldrpanel;

    panel->CancelBulkActivity(ChartDldrBulkCancelScope::PluginShutdown);
    panel->InvalidatePendingBulkPumps();
    // The pump has no modal/nested-loop paths, so DeInit cannot re-enter this
    // code while the pump stack is active. Keeping the assertion makes any
    // future violation fail in development instead of leaking executable
    // plugin callbacks across library unload.
    if (ChartDldrIsBulkPumpActive()) {
      wxLogFatalError(
          wxT("Chart Downloader unloaded from inside its bulk pump"));
    }
    // The panel is the CallAfter owner; sync deletion drops pending callbacks.
    delete pi->m_dldrpanel;
    pi->m_dldrpanel = nullptr;
  }

  if (pi->m_panel_host) {
    delete pi->m_panel_host;
    pi->m_panel_host = nullptr;
  }
}
