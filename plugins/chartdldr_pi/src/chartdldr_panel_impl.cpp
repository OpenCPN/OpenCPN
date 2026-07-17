/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_impl.h"
#include "chartdldr_bulk_orchestrate.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_catalog_prep.h"
#include "chartdldr_download_cancel.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <memory>

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/uri.h>

#ifdef __ANDROID__
#include "androidSupport.h"
#endif

#ifdef __WXMAC__
#define CATALOGS_NAME_WIDTH 300
#define CATALOGS_DATE_WIDTH 120
#define CATALOGS_PATH_WIDTH 100
#else
#ifdef __ANDROID__
#define CATALOGS_NAME_WIDTH 350
#define CATALOGS_DATE_WIDTH 500
#define CATALOGS_PATH_WIDTH 1000
#else
#define CATALOGS_NAME_WIDTH 200
#define CATALOGS_DATE_WIDTH 130
#define CATALOGS_PATH_WIDTH 250
#endif
#endif  // __WXMAC__

void ChartDldrInitPanelBulkDownloadHooks();

namespace {

class ChartDldrPanelBulkNotifier final : public ChartDldrBulkNotifier {
public:
  explicit ChartDldrPanelBulkNotifier(ChartDldrPanelImpl &panel)
      : panel_(panel) {}

  bool ConfirmInteractiveStart() override {
    chartdldr_pi *const pi = panel_.GetPlugin();
    return pi && pi->ConfirmInteractiveBulkUpdate(&panel_);
  }

private:
  ChartDldrPanelImpl &panel_;
};

}  // namespace

void ChartDldrPanelImpl::MarkMacCatalogListNeedsRefresh() {
#ifdef __WXMAC__
  m_macCatalogListNeedsRefresh = true;
#endif
}

void ChartDldrPanelImpl::OnShowLocalDir(wxCommandEvent &event) {
  if (pPlugIn->m_pChartSource == NULL) return;
#ifdef __WXGTK__
  wxExecute(wxString::Format(_T("xdg-open %s"),
                             pPlugIn->m_pChartSource->GetDir().c_str()));
#endif
#ifdef __WXMAC__
  wxExecute(wxString::Format(_T("open %s"),
                             pPlugIn->m_pChartSource->GetDir().c_str()));
#endif
#ifdef __WXMSW__
  wxExecute(wxString::Format(_T("explorer %s"),
                             pPlugIn->m_pChartSource->GetDir().c_str()));
#endif
}

void ChartDldrPanelImpl::SetActiveCatalogContext(int catalog_index) {
  pPlugIn->SetSourceId(catalog_index);

  if (catalog_index >= 0 &&
      catalog_index < static_cast<int>(pPlugIn->m_ChartSources.size())) {
    pPlugIn->m_pChartSource = pPlugIn->m_ChartSources.at(catalog_index).get();
    // Active context owns the sources-list highlight.
    SelectCatalog(catalog_index);
  } else {
    pPlugIn->m_pChartSource = nullptr;
    const long selected = m_lbChartSources->GetNextItem(-1, wxLIST_NEXT_ALL,
                                                        wxLIST_STATE_SELECTED);
    if (selected >= 0) {
      m_lbChartSources->SetItemState(
          selected, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    }
  }
}

void ChartDldrPanelImpl::ApplyCatalogToolbarState() {
  const ChartDldrCatalogControlsState controls = ChartDldrCatalogControlsFor(
      bulk_->IsRunActive(),
      bulk_->Session().DownloadCancel().IsDownloadButtonCancelArmed());
  const int catalog_index = GetSelectedCatalog();
  const bool has_selection = catalog_index >= 0;
  const bool bulk_update = pPlugIn != nullptr && pPlugIn->m_allow_bulk_update;

  m_lbChartSources->Enable(controls.list_enabled);
#if defined(CHART_LIST)
  m_bSelectNew->Enable(controls.list_enabled);
  m_bSelectUpdated->Enable(controls.list_enabled);
  m_bSelectAll->Enable(controls.list_enabled);
  m_scrollWinChartList->Enable(controls.list_enabled);
#endif /* CHART_LIST */

  if (!controls.mutations_enabled) {
    m_bAddSource->Disable();
    m_bDeleteSource->Disable();
    m_bEditSource->Disable();
    m_bUpdateChartList->Disable();
    m_bUpdateAllCharts->Disable();
    m_bUpdateAllCharts->Show(bulk_update);
  } else {
    m_bAddSource->Enable();
    m_bDeleteSource->Enable(has_selection);
    m_bEditSource->Enable(has_selection);
    m_bUpdateChartList->Enable(has_selection);
    m_bUpdateAllCharts->Enable(bulk_update);
    m_bUpdateAllCharts->Show(bulk_update);
  }
  m_bDnldCharts->Enable(controls.download_button_enabled);
  Layout();
  if (m_parent != nullptr) {
    m_parent->Layout();
  }
}

void ChartDldrPanelImpl::LoadCatalogSelectionFromDisk(const wxString &path,
                                                      bool selnew,
                                                      bool selupd) {
  if (!pPlugIn->m_pChartCatalog.LoadFromFile(path)) {
    m_catalogSelection.Clear();
    return;
  }

  m_catalogSelection = ChartDldrBuildCatalogSelection(
      pPlugIn->m_pChartCatalog, *pPlugIn->m_pChartSource, selnew, selupd);
}

void ChartDldrPanelImpl::ReloadCatalogChartList(ChartSource &cs, bool selnew,
                                                bool selupd, bool materialize) {
  if (materialize) {
    CleanForm();
  } else {
    chart_list_view_.ClearWidgets();
  }
  ChartDldrPrepareChartSourceLocals(cs);
  pPlugIn->m_pChartSource = &cs;

  wxFileName fn;
  fn.SetFullName(ChartDldrCatalogFilenameFromUrl(cs.GetUrl()));
  fn.SetPath(cs.GetDir());
  if (wxFileExists(fn.GetFullPath())) {
    LoadCatalogSelectionFromDisk(fn.GetFullPath(), selnew, selupd);
    chart_list_view_.MaterializeWidgets(materialize);
  }
}

void ChartDldrPanelImpl::RefreshChartListForSource(
    int catalog_index, const ChartDldrCatalogUiPolicy &ui) {
  if (catalog_index < 0 ||
      catalog_index >= static_cast<int>(pPlugIn->m_ChartSources.size())) {
    return;
  }
  ChartSource &cs = *pPlugIn->m_ChartSources.at(catalog_index);
  ReloadCatalogChartList(cs, ui.preselect_new, ui.preselect_updated,
                         ui.materialize);
  UpdateChartsLabelForSource(cs);
}

void ChartDldrPanelImpl::UpdateChartsLabelForSource(const ChartSource &cs) {
  wxURI url(cs.GetUrl());
  m_chartsLabel->SetLabel(wxString::Format(
      _("Charts: %s"),
      (cs.GetName() + _(" from ") + url.BuildURI() + _T(" -> ") + cs.GetDir())
          .c_str()));
}

ChartDldrChartUpdateKind ChartDldrPanelImpl::ChartKindAt(int index) const {
  return m_catalogSelection.KindAt(index);
}

#if defined(CHART_LIST)
bool ChartDldrPanelImpl::isNew(int item) {
  return ChartKindAt(item) == ChartDldrChartUpdateKind::New;
}

bool ChartDldrPanelImpl::isUpdated(int item) {
  return ChartKindAt(item) == ChartDldrChartUpdateKind::Updated;
}
#endif /* CHART_LIST */

void ChartDldrPanelImpl::SetSource(int id) {
  // User clicks call SelectSource (list already selected). Bulk / mutation
  // paths go through SetActiveCatalogContext, which owns the list highlight.
  SetActiveCatalogContext(id);
  ApplyCatalogToolbarState();

  if (id >= 0 && id < (int)pPlugIn->m_ChartSources.size()) {
    const bool show_wait_cursor =
        wxPanel::IsShownOnScreen() && !bulk_->IsRunActive();
    if (show_wait_cursor) {
      ::wxBeginBusyCursor();
    }
    RefreshChartListForSource(
        id, ChartDldrBrowseCatalogUiPolicy(pPlugIn->m_preselect_new,
                                           pPlugIn->m_preselect_updated));
    if (show_wait_cursor && ::wxIsBusy()) {
      ::wxEndBusyCursor();
    }
  } else {
    CleanForm();
    m_chartsLabel->SetLabel(_("Charts"));
  }
}

void ChartDldrPanelImpl::SelectSource(wxListEvent &event) {
  int i = GetSelectedCatalog();
  if (i >= 0) SetSource(i);
  event.Skip();
}

void ChartDldrPanelImpl::RefreshCatalogToolbar() { ApplyCatalogToolbarState(); }

void ChartDldrPanelImpl::CleanForm() {
  m_catalogSelection.Clear();
  chart_list_view_.ClearWidgets();
}

int ChartDldrPanelImpl::GetSelectedCatalog() {
  long item =
      m_lbChartSources->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  return item;
}

void ChartDldrPanelImpl::SelectCatalog(int item) {
  m_lbChartSources->SetItemState(item, wxLIST_STATE_SELECTED,
                                 wxLIST_STATE_SELECTED);
}

#if defined(CHART_LIST)
void ChartDldrPanelImpl::OnSelectChartItem(wxDataViewEvent &event) {
  if (m_bInfoHold) {
    event.Skip();
    return;
  }
  chart_list_view_.CaptureSelectionFromWidgets();
}

void ChartDldrPanelImpl::OnChartListContextMenu(wxDataViewEvent &event) {
  wxMouseEvent mouse_event;
  OnContextMenu(mouse_event);
  event.Skip();
}
#endif /* CHART_LIST */
#if defined(CHART_LIST)
void ChartDldrPanelImpl::OnSelectNewCharts(wxCommandEvent &event) {
  CheckNewCharts(true);
}
#endif /* CHART_LIST */

#if defined(CHART_LIST)
void ChartDldrPanelImpl::OnSelectUpdatedCharts(wxCommandEvent &event) {
  CheckUpdatedCharts(true);
}
#endif /* CHART_LIST */

#if defined(CHART_LIST)
void ChartDldrPanelImpl::OnSelectAllCharts(wxCommandEvent &event) {
  if (m_bSelectAll->GetLabel() == _("Select All")) {
    CheckAllCharts(true);
    m_bSelectAll->SetLabel(_("Select None"));
    m_bSelectAll->SetToolTip(_("De-select all charts in the list."));
  } else {
    CheckAllCharts(false);
    m_bSelectAll->SetLabel(_("Select All"));
    m_bSelectAll->SetToolTip(_("Select all charts in the list."));
  }
}
#endif /* CHART_LIST */

int ChartDldrPanelImpl::GetChartCount() { return m_catalogSelection.Count(); }

int ChartDldrPanelImpl::GetCheckedChartCount() {
  return m_catalogSelection.CheckedCount();
}

bool ChartDldrPanelImpl::IsChartChecked(int i) const {
  wxASSERT_MSG(i >= 0,
               wxT("This function should be called with non-negative index."));
  return m_catalogSelection.IsChecked(i);
}

void ChartDldrPanelImpl::CheckAllCharts(bool value) {
  m_catalogSelection.SetAllChecked(value);
  chart_list_view_.SyncFromSelection();
}

void ChartDldrPanelImpl::CheckNewCharts(bool value) {
  m_catalogSelection.SetCheckedForKind(ChartDldrChartUpdateKind::New, value);
  chart_list_view_.SyncFromSelection();
}

void ChartDldrPanelImpl::CheckUpdatedCharts(bool value) {
  m_catalogSelection.SetCheckedForKind(ChartDldrChartUpdateKind::Updated,
                                       value);
  chart_list_view_.SyncFromSelection();
}

void ChartDldrPanelImpl::InvertCheckAllCharts() {
  m_catalogSelection.InvertAllChecked();
  chart_list_view_.SyncFromSelection();
}

void ChartDldrPanelImpl::UpdateCatalogListRow(int catalog_index,
                                              const wxString &title,
                                              const wxString &release_date,
                                              const wxString &local_path) {
  if (catalog_index < 0 || catalog_index >= m_lbChartSources->GetItemCount()) {
    return;
  }
  m_lbChartSources->SetItem(catalog_index, 0, title);
  m_lbChartSources->SetItem(catalog_index, 1, release_date);
  m_lbChartSources->SetItem(catalog_index, 2, local_path);
  MarkMacCatalogListNeedsRefresh();
}

void ChartDldrPanelImpl::FocusChartsDownloadTab() {
  m_DLoadNB->SetSelection(1);
}

void ChartDldrPanelImpl::SetDownloadChartsButtonLabel(const wxString &label) {
  m_bDnldCharts->SetLabel(label);
  SyncDownloadChartsButtonEnabled();
}

void ChartDldrPanelImpl::SyncDownloadCancelButton() {
  SetDownloadChartsButtonLabel(ChartDldrDownloadCancelButtonLabel(
      bulk_->Session().DownloadCancel().phase));
}

void ChartDldrPanelImpl::SyncDownloadChartsButtonEnabled() {
  m_bDnldCharts->Enable(
      ChartDldrCatalogControlsFor(
          bulk_->IsRunActive(),
          bulk_->Session().DownloadCancel().IsDownloadButtonCancelArmed())
          .download_button_enabled);
}

void ChartDldrPanelImpl::ArmChartDownloadCancelUi() {
  bulk_->Session().DownloadCancel().BeginActiveDownload();
  SyncDownloadCancelButton();
}

void ChartDldrPanelImpl::DisarmChartDownloadCancelUi() {
  bulk_->Session().DownloadCancel().EndActiveDownload();
  SyncDownloadCancelButton();
}

bool ChartDldrPanelImpl::IsBulkRunCancelled() const {
  return bulk_->Session().DownloadCancel().IsSessionCancelled();
}

void ChartDldrPanelImpl::UpdateChartList(wxCommandEvent &event) {
  (void)event;
  Bulk().RefreshCatalogManual(GetSelectedCatalog());
}

ChartDldrPanelImpl::~ChartDldrPanelImpl() {
  if (bulk_) {
    bulk_->Pump().InvalidatePending();
    bulk_->Pump().SetTransferStallTimerRunning(false);
  }

  // Unexpected options-page destroy (without ParkOnHost) lands here too:
  // tear down bulk explicitly so session state is not silently dropped.
  CancelBulkActivity(ChartDldrBulkCancelScope::PluginShutdown);
  if (pPlugIn && pPlugIn->m_dldrpanel == this) {
    pPlugIn->m_dldrpanel = nullptr;
  }

#ifndef __ANDROID__
  OCPN_cancelDownloadFileBackground(
      0);  // Stop the thread, is something like this needed on Android as well?
#endif
#if defined(CHART_LIST)
  clearChartList();
#endif /* CHART_LIST */
}

ChartDldrPanelImpl::ChartDldrPanelImpl(chartdldr_pi *plugin, wxWindow *parent,
                                       wxWindowID id, const wxPoint &pos,
                                       const wxSize &size, long style)
    : ChartDldrPanel(parent, id, pos, size, style),
      chart_list_view_(*this),
      bulk_(std::make_unique<ChartDldrBulkOrchestrate>(*this)),
      notifier_(std::make_unique<ChartDldrPanelBulkNotifier>(*this)) {
  ChartDldrInitPanelBulkDownloadHooks();
  BindPanelEventHandlers();
  m_lbChartSources->InsertColumn(0, _("Catalog"), wxLIST_FORMAT_LEFT,
                                 CATALOGS_NAME_WIDTH);
  m_lbChartSources->InsertColumn(1, _("Released"), wxLIST_FORMAT_LEFT,
                                 CATALOGS_DATE_WIDTH);
  m_lbChartSources->InsertColumn(2, _("Local path"), wxLIST_FORMAT_LEFT,
                                 CATALOGS_PATH_WIDTH);
  m_lbChartSources->Enable();
  m_bInfoHold = false;
  pPlugIn = plugin;
  m_populated = false;
  SetChartInfo(wxEmptyString);

  for (size_t i = 0; i < pPlugIn->m_ChartSources.size(); i++) {
    AppendCatalog(pPlugIn->m_ChartSources.at(i));
  }
  m_populated = true;
  ApplyCatalogToolbarState();
}
