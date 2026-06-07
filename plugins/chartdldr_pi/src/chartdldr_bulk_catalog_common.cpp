/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_catalog_internal.h"

#include <wx/filename.h>
#include <wx/url.h>

void ChartDldrPanelImpl::PrepareBulkCatalog(int catalog_index,
                                            bool sync_list_selection) {
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }

  if (sync_list_selection && catalog_index < m_lbChartSources->GetItemCount()) {
    SelectCatalog(catalog_index);
  }
  SetSource(catalog_index);
}

void ChartDldrPanelImpl::HandleCatalogDownloadResult(
    int catalog_index, _OCPN_DLStatus ret, const wxString& url,
    const ChartDldrBulkModeProfile& profile) {
  switch (ret) {
    case OCPN_DL_NO_ERROR: {
      if (catalog_index < m_lbChartSources->GetItemCount()) {
        std::unique_ptr<ChartSource>& cs =
            pPlugIn->m_ChartSources.at(catalog_index);
        m_lbChartSources->SetItem(catalog_index, 0,
                                  pPlugIn->m_pChartCatalog.title);
        m_lbChartSources->SetItem(
            catalog_index, 1,
            pPlugIn->m_pChartCatalog.GetReleaseDate().Format(
                _T("%Y-%m-%d %H:%M")));
        m_lbChartSources->SetItem(catalog_index, 2, cs->GetDir());
      }
      if (profile.focus_charts_tab_after_catalog) {
        m_DLoadNB->SetSelection(1);
      }
      break;
    }
    case OCPN_DL_FAILED: {
      const wxString msg = wxString::Format(
          _("Failed to Download Catalog: %s \nVerify there is a working "
            "Internet connection."),
          url.c_str());
      ChartDldrReportBulkError(this, profile, msg, _("Chart Downloader"));
      break;
    }
    case OCPN_DL_USER_TIMEOUT:
    case OCPN_DL_ABORTED:
      cancelled = true;
      break;
    case OCPN_DL_UNKNOWN:
    case OCPN_DL_STARTED:
      break;
    default:
      wxASSERT(false);
  }
}

void ChartDldrPanelImpl::UpdateChartList(wxCommandEvent& event) {
  bulk_.RefreshCatalogManual(GetSelectedCatalog(), event);
}

void ChartDldrPanelImpl::UpdateAllCharts(wxCommandEvent& event) {
  ChartDldrBulkRunStats stats;
  Bulk().RunInteractive(ChartDldrBulkRunKind::Interactive, event, stats);
}
