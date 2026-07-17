/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_impl.h"

#include "chartdldr_catalog_prep.h"
#include "chartdldr_pi.h"
#include "chartcatalog.h"

#include "ocpn_plugin.h"

#include <memory>

#include <wx/filename.h>
#include <wx/uri.h>

#ifdef __ANDROID__
#include "androidSupport.h"
#endif

void ChartDldrPanelImpl::AppendCatalog(std::unique_ptr<ChartSource>& cs) {
  long id = m_lbChartSources->GetItemCount();
  m_lbChartSources->InsertItem(id, cs->GetName());
  m_lbChartSources->SetItem(id, 1, _("(Please update first)"));
  m_lbChartSources->SetItem(id, 2, cs->GetDir());
  wxURI url(cs->GetUrl());
  if (url.IsReference()) {
    OCPNMessageBox_PlugIn(
        this, _("Error, the URL to the chart source data seems wrong."),
        _("Error"));
    return;
  }
  wxFileName fn;
  fn.SetFullName(ChartDldrCatalogFilenameFromUrl(cs->GetUrl()));
  fn.SetPath(cs->GetDir());
  wxString path = fn.GetFullPath();
  if (wxFileExists(path)) {
    if (pPlugIn->m_pChartCatalog.LoadFromFile(path, true)) {
      m_lbChartSources->SetItem(id, 0, pPlugIn->m_pChartCatalog.title);
      m_lbChartSources->SetItem(
          id, 1,
          pPlugIn->m_pChartCatalog.GetReleaseDate().Format(
              _T("%Y-%m-%d %H:%M")));
      m_lbChartSources->SetItem(id, 2, path);
#ifdef __ANDROID__
      m_lbChartSources->GetHandle()->resizeColumnToContents(0);
      m_lbChartSources->GetHandle()->resizeColumnToContents(1);
      m_lbChartSources->GetHandle()->resizeColumnToContents(2);
#endif
    }
  }
  MarkMacCatalogListNeedsRefresh();
}

void ChartDldrPanelImpl::DeleteSource(wxCommandEvent& event) {
  if (!m_lbChartSources->GetSelectedItemCount()) {
    return;
  }
  if (wxID_YES != OCPNMessageBox_PlugIn(
                      this,
                      _("Do you really want to remove the chart source?\nThe "
                        "local chart files will not be removed,\nbut you will "
                        "not be able to update the charts anymore."),
                      _("Chart Downloader"), wxYES_NO | wxCENTRE)) {
    return;
  }
  const int to_be_removed = GetSelectedCatalog();
  m_lbChartSources->SetItemState(to_be_removed, 0,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
  pPlugIn->m_ChartSources.erase(pPlugIn->m_ChartSources.begin() +
                                to_be_removed);
  m_lbChartSources->DeleteItem(to_be_removed);
  SetSource(-1);
  MarkMacCatalogListNeedsRefresh();
  pPlugIn->SaveConfig();
  event.Skip();
}

void ChartDldrPanelImpl::AddSource(wxCommandEvent& event) {
  ChartDldrGuiAddSourceDlg dialog(this);
  dialog.SetBasePath(pPlugIn->GetBaseChartDir());

  wxSize sz = GetParent()
                  ->GetGrandParent()
                  ->GetSize();  // This is the options panel true size
  dialog.SetSize(sz.GetWidth(), sz.GetHeight());
  dialog.Center();

#ifdef __ANDROID__
  androidDisableRotation();
#endif

  if (dialog.ShowModal() == wxID_OK) {
    std::unique_ptr<ChartSource> cs = std::make_unique<ChartSource>(
        dialog.m_tSourceName->GetValue(), dialog.m_tChartSourceUrl->GetValue(),
        dialog.m_tcChartDirectory->GetValue());
    AppendCatalog(cs);
    pPlugIn->m_ChartSources.push_back(std::move(cs));
    const int new_index = static_cast<int>(pPlugIn->m_ChartSources.size()) - 1;
    bool covered = false;
    for (size_t i = 0; i < GetChartDBDirArrayString().GetCount(); i++) {
      if (pPlugIn->m_ChartSources.at(new_index)->GetDir().StartsWith(
              (GetChartDBDirArrayString().Item(i)))) {
        covered = true;
        break;
      }
    }
    if (!covered) {
      wxString dir = pPlugIn->m_ChartSources.at(new_index)->GetDir();
      AddChartDirectory(dir);
    }

    SetSource(new_index);
    pPlugIn->SaveConfig();
  }
#ifdef __ANDROID__
  androidEnableRotation();
#endif

  event.Skip();
}

void ChartDldrPanelImpl::DoEditSource() {
  if (!m_lbChartSources->GetSelectedItemCount()) {
    return;
  }
  const int cat = GetSelectedCatalog();
  ChartDldrGuiAddSourceDlg dialog(this);
  dialog.SetBasePath(pPlugIn->GetBaseChartDir());
  dialog.SetSourceEdit(pPlugIn->m_ChartSources.at(cat));
  dialog.SetTitle(_("Edit Chart Source"));

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  pPlugIn->m_ChartSources.at(cat)->SetName(dialog.m_tSourceName->GetValue());
  pPlugIn->m_ChartSources.at(cat)->SetUrl(dialog.m_tChartSourceUrl->GetValue());
  pPlugIn->m_ChartSources.at(cat)->SetDir(
      dialog.m_tcChartDirectory->GetValue());

  m_lbChartSources->SetItem(cat, 0, pPlugIn->m_ChartSources.at(cat)->GetName());
  m_lbChartSources->SetItem(cat, 1, _("(Please update first)"));
  m_lbChartSources->SetItem(cat, 2, pPlugIn->m_ChartSources.at(cat)->GetDir());
  wxURI url(pPlugIn->m_ChartSources.at(cat)->GetUrl());
  wxFileName fn(url.GetPath());
  fn.SetPath(pPlugIn->m_ChartSources.at(cat)->GetDir());
  wxString path = fn.GetFullPath();
  if (wxFileExists(path)) {
    if (pPlugIn->m_pChartCatalog.LoadFromFile(path, true)) {
      m_lbChartSources->SetItem(cat, 0, pPlugIn->m_pChartCatalog.title);
      m_lbChartSources->SetItem(
          cat, 1,
          pPlugIn->m_pChartCatalog.GetReleaseDate().Format(
              _T("%Y-%m-%d %H:%M")));
      m_lbChartSources->SetItem(cat, 2, path);
    }
  }
  bool covered = false;
  for (size_t i = 0; i < GetChartDBDirArrayString().GetCount(); i++) {
    if (pPlugIn->m_ChartSources.at(cat)->GetDir().StartsWith(
            (GetChartDBDirArrayString().Item(i)))) {
      covered = true;
      break;
    }
  }
  if (!covered) {
    OCPNMessageBox_PlugIn(
        this,
        wxString::Format(
            _("Path %s seems not to be covered by your configured Chart "
              "Directories.\nTo see the charts you have to adjust the "
              "configuration on the 'Chart Files' tab."),
            pPlugIn->m_ChartSources.at(cat)->GetDir().c_str()),
        _("Chart Downloader"));
  }

  pPlugIn->SaveConfig();
  SetSource(cat);
}

void ChartDldrPanelImpl::EditSource(wxCommandEvent& event) {
  DoEditSource();
  event.Skip();
}

void ChartDldrPanelImpl::OnLeftDClick(wxMouseEvent& event) {
  DoEditSource();
  event.Skip();
}
