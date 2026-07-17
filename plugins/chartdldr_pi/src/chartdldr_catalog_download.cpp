/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_catalog_download.h"

#include "chartdldr_bulk_lifecycle.h"
#include "chartcatalog.h"

#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/url.h>

ChartDldrCatalogPublishSurvey ChartDldrSurveyCatalogPublishable(
    const ChartCatalog& catalog) {
  ChartDldrCatalogPublishSurvey survey;
  if (catalog.title.IsEmpty()) {
    return survey;
  }
  if (!catalog.dt_valid.IsValid() &&
      !(catalog.date_created.IsValid() && catalog.time_created.IsValid())) {
    return survey;
  }
  for (const auto& chart : catalog.charts) {
    ++survey.total_cells;
    if (!chart || chart->GetChartTitle().IsEmpty()) {
      ++survey.skipped_cells;
      continue;
    }
    if (chart->NeedsManualDownload()) {
      if (!chart->GetManualDownloadUrl().IsEmpty()) {
        ++survey.usable_cells;
      }
    } else if (!chart->GetDownloadLocation().IsEmpty()) {
      ++survey.usable_cells;
    }
  }
  survey.publishable = survey.usable_cells > 0;
  return survey;
}

bool ChartDldrCatalogTempIsPublishable(const wxString& temp_path) {
  ChartCatalog catalog;
  if (!catalog.LoadFromFile(temp_path, false)) {
    return false;
  }
  const ChartDldrCatalogPublishSurvey survey =
      ChartDldrSurveyCatalogPublishable(catalog);
  if (survey.skipped_cells > 0) {
    wxLogWarning(wxString::Format(
        wxT("chartdldr_pi: catalog %s has %d of %d chart cells without a "
            "usable download URL; publishing %d usable cells"),
        temp_path.c_str(), survey.skipped_cells, survey.total_cells,
        survey.usable_cells));
  }
  return survey.publishable;
}

ChartDldrCatalogDownloadOutcome ChartDldrCatalogDownloadOutcomeFor(
    _OCPN_DLStatus status, const wxString& url) {
  ChartDldrCatalogDownloadOutcome outcome;
  switch (status) {
    case OCPN_DL_NO_ERROR:
      outcome.reload_catalog = true;
      break;
    case OCPN_DL_FAILED:
      outcome.report_failure = true;
      outcome.failure_message = wxString::Format(
          _("Failed to Download Catalog: %s \nVerify there is a working "
            "Internet connection."),
          url.c_str());
      break;
    case OCPN_DL_USER_TIMEOUT:
    case OCPN_DL_ABORTED:
      break;
    case OCPN_DL_UNKNOWN:
    case OCPN_DL_STARTED:
      break;
    default:
      wxASSERT(false);
      break;
  }
  return outcome;
}

bool ChartDldrCatalogRefreshSucceeded(_OCPN_DLStatus status, bool cancelled) {
  return status == OCPN_DL_NO_ERROR && !cancelled;
}

bool ChartDldrPublishValidatedCatalog(const wxString& temp_path,
                                      const wxString& output_path) {
  if (!ChartDldrCatalogTempIsPublishable(temp_path)) {
    ChartDldrRemoveTempDownload(temp_path);
    return false;
  }
  return ChartDldrFinalizeTempDownload(temp_path, output_path);
}

_OCPN_DLStatus ChartDldrCompleteValidatedCatalogDownloadPaths(
    const ChartDldrTempDownloadPaths& paths, bool transfer_success,
    bool cancelled) {
  return ChartDldrCompleteTempDownloadPaths(paths, transfer_success, cancelled,
                                            ChartDldrCatalogTempIsPublishable);
}

void ChartDldrReportCatalogPreparePathFailure(wxWindow* parent,
                                              ChartDldrErrorReporting reporting,
                                              const wxURI& url,
                                              const wxFileName& output_fn) {
  if (url.IsReference()) {
    const wxString msg =
        _("Error, the URL to the chart source data seems wrong.");
    ChartDldrReportBulkError(parent, reporting, msg, _("Error"));
    return;
  }
  const wxString msg = wxString::Format(_("Directory %s can't be created."),
                                        output_fn.GetPath().c_str());
  ChartDldrReportBulkError(parent, reporting, msg, _("Chart Downloader"));
}
