/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_panel.h"

#include "chartdldr_pi.h"

#include <wx/panel.h>
#include <wx/sizer.h>

// A headless bulk coordinator could avoid this hidden panel; see chartdldr_pi.

bool ChartDldrPanelOnOptionsPage(const chartdldr_pi* pi) {
  return pi && pi->m_dldrpanel && pi->m_pOptionsPage &&
         pi->m_dldrpanel->GetParent() == pi->m_pOptionsPage;
}

bool ChartDldrEnsureDownloaderPanel(chartdldr_pi* pi) {
  if (!pi || pi->m_dldrpanel) {
    return pi && pi->m_dldrpanel;
  }
  if (!pi->m_parent_window) {
    return false;
  }

  if (!pi->m_panel_host) {
    pi->m_panel_host = new wxPanel(pi->m_parent_window, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxNO_BORDER);
    pi->m_panel_host->Hide();
  }

  pi->m_dldrpanel = new ChartDldrPanelImpl(pi, pi->m_panel_host, wxID_ANY,
                                          wxDefaultPosition, wxDefaultSize,
                                          wxDEFAULT_DIALOG_STYLE);
  pi->m_dldrpanel->Hide();

  wxBoxSizer* host_sizer = new wxBoxSizer(wxVERTICAL);
  pi->m_panel_host->SetSizer(host_sizer);
  host_sizer->Add(pi->m_dldrpanel, 1, wxEXPAND);
  pi->m_dldrpanel->SetBulkUpdate(pi->m_allow_bulk_update);
  return true;
}

void ChartDldrAttachDownloaderPanelToOptions(chartdldr_pi* pi,
                                             wxScrolledWindow* page) {
  if (!pi || !pi->m_dldrpanel || !page) {
    return;
  }

  if (pi->m_panel_host && pi->m_panel_host->GetSizer()) {
    pi->m_panel_host->GetSizer()->Detach(pi->m_dldrpanel);
  }

  wxSizer* old_sizer = page->GetSizer();
  if (old_sizer) {
    old_sizer->Detach(pi->m_dldrpanel);
    old_sizer->Clear(false);
    delete old_sizer;
  }

  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  page->SetSizer(sizer);
  pi->m_dldrpanel->Reparent(page);
  page->InvalidateBestSize();
  sizer->Add(pi->m_dldrpanel, 1, wxALL | wxEXPAND);
}

void ChartDldrDestroyDownloaderUI(chartdldr_pi* pi) {
  if (!pi) {
    return;
  }

  if (pi->m_dldrpanel) {
    pi->m_dldrpanel->CancelDownload();
#ifndef __ANDROID__
    OCPN_cancelDownloadFileBackground(0);
#endif
    if (pi->m_pOptionsPage &&
        pi->m_dldrpanel->GetParent() == pi->m_pOptionsPage) {
      wxSizer* sizer = pi->m_pOptionsPage->GetSizer();
      if (sizer) {
        sizer->Detach(pi->m_dldrpanel);
      }
    }
    delete pi->m_dldrpanel;
    pi->m_dldrpanel = NULL;
  }

  if (pi->m_panel_host) {
    delete pi->m_panel_host;
    pi->m_panel_host = NULL;
  }
}
