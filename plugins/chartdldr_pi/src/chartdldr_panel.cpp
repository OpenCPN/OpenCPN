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

bool ChartDldrPanelOnOptionsPage(const chartdldr_pi* pi) {
  return pi && pi->m_dldrpanel && pi->m_pOptionsPage &&
         pi->m_dldrpanel->GetParent() == pi->m_pOptionsPage;
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
