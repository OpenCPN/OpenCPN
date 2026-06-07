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

#include "ocpn_plugin.h"

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

#define ID_MNU_SELALL 2001
#define ID_MNU_DELALL 2002
#define ID_MNU_INVSEL 2003
#define ID_MNU_SELUPD 2004
#define ID_MNU_SELNEW 2005

enum { ThreadId = wxID_HIGHEST + 1 };

BEGIN_EVENT_TABLE(ChartDldrPanelImpl, ChartDldrPanel)
END_EVENT_TABLE()

void ChartDldrPanelImpl::OnPopupClick(wxCommandEvent &evt) {
  switch (evt.GetId()) {
    case ID_MNU_SELALL:
      CheckAllCharts(true);
      break;
    case ID_MNU_DELALL:
      CheckAllCharts(false);
      break;
    case ID_MNU_INVSEL:
      InvertCheckAllCharts();
      break;
    case ID_MNU_SELUPD:
      CheckUpdatedCharts(true);
      break;
    case ID_MNU_SELNEW:
      CheckNewCharts(true);
      break;
  }
}

void ChartDldrPanelImpl::OnContextMenu(wxMouseEvent &event) {
  wxMenu menu;

  wxPoint mouseScreen = wxGetMousePosition();
  wxPoint mouseClient = ScreenToClient(mouseScreen);

#ifdef __ANDROID__
  wxFont *pf = OCPNGetFont(_("Menu"));

  // add stuff
  wxMenuItem *item1 = new wxMenuItem(&menu, ID_MNU_SELALL, _("Select all"));
  item1->SetFont(*pf);
  menu.Append(item1);

  wxMenuItem *item2 = new wxMenuItem(&menu, ID_MNU_DELALL, _("Deselect all"));
  item2->SetFont(*pf);
  menu.Append(item2);

  wxMenuItem *item3 =
      new wxMenuItem(&menu, ID_MNU_INVSEL, _("Invert selection"));
  item3->SetFont(*pf);
  menu.Append(item3);

  wxMenuItem *item4 = new wxMenuItem(&menu, ID_MNU_SELUPD, _("Select updated"));
  item4->SetFont(*pf);
  menu.Append(item4);

  wxMenuItem *item5 = new wxMenuItem(&menu, ID_MNU_SELNEW, _("Select new"));
  item5->SetFont(*pf);
  menu.Append(item5);

#else

  menu.Append(ID_MNU_SELALL, _("Select all"), wxT(""));
  menu.Append(ID_MNU_DELALL, _("Deselect all"), wxT(""));
  menu.Append(ID_MNU_INVSEL, _("Invert selection"), wxT(""));
  menu.Append(ID_MNU_SELUPD, _("Select updated"), wxT(""));
  menu.Append(ID_MNU_SELNEW, _("Select new"), wxT(""));

#endif

  menu.Connect(wxEVT_COMMAND_MENU_SELECTED,
               (wxObjectEventFunction)&ChartDldrPanelImpl::OnPopupClick, NULL,
               this);
  // and then display
  PopupMenu(&menu, mouseClient.x, mouseClient.y);
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

void ChartDldrPanelImpl::SetSource(int id) {
  pPlugIn->SetSourceId(id);

  m_bDeleteSource->Enable(id >= 0);
  m_bUpdateChartList->Enable(id >= 0);
  m_bEditSource->Enable(id >= 0);

  // TODO: DAN - Need to optimze to only update the chart list if needed.
  //             Right now it updates multiple times unnecessarily.
  CleanForm();
  if (id >= 0 && id < (int)pPlugIn->m_ChartSources.size()) {
    const bool show_wait_cursor =
        IsShownOnScreen() && !pPlugIn->m_bulk_run_active;
    if (show_wait_cursor) {
      ::wxBeginBusyCursor();
    }
    std::unique_ptr<ChartSource> &cs = pPlugIn->m_ChartSources.at(id);
    cs->LoadUpdateData();
    cs->UpdateLocalFiles();
    pPlugIn->m_pChartSource = cs.get();
    FillFromFile(cs->GetUrl(), cs->GetDir(), pPlugIn->m_preselect_new,
                 pPlugIn->m_preselect_updated);
    if (show_wait_cursor) {
      wxURI url(cs->GetUrl());
      m_chartsLabel->SetLabel(wxString::Format(
          _("Charts: %s"), (cs->GetName() + _(" from ") + url.BuildURI() +
                            _T(" -> ") + cs->GetDir())
                               .c_str()));
      if (::wxIsBusy()) {
        ::wxEndBusyCursor();
      }
    }
  } else {
    pPlugIn->m_pChartSource = NULL;
    m_chartsLabel->SetLabel(_("Charts"));
  }
}

void ChartDldrPanelImpl::SelectSource(wxListEvent &event) {
  int i = GetSelectedCatalog();
  if (i >= 0) SetSource(i);
  event.Skip();
}

void ChartDldrPanelImpl::SetBulkUpdate(bool bulk_update) {
  m_bUpdateAllCharts->Enable(bulk_update);
  m_bUpdateAllCharts->Show(bulk_update);
  Layout();
  m_parent->Layout();
}

void ChartDldrPanelImpl::CleanForm() {
#if defined(CHART_LIST)
  clearChartList();
#else
  m_scrollWinChartList->ClearBackground();
#endif /* CHART_LIST */
       // m_stCatalogInfo->Show( false );
}

void ChartDldrPanelImpl::FillFromFile(wxString url, wxString dir, bool selnew,
                                      bool selupd) {
  // load if exists
  wxStringTokenizer tk(url, _T("/"));
  wxString file;
  do {
    file = tk.GetNextToken();
  } while (tk.HasMoreTokens());
  wxFileName fn;
  fn.SetFullName(file);
  fn.SetPath(dir);
  wxString path = fn.GetFullPath();
  if (wxFileExists(path)) {
    pPlugIn->m_pChartCatalog.LoadFromFile(path);
    //            m_tChartSourceInfo->SetValue(pPlugIn->m_pChartCatalog.GetDescription());
    // fill in the rest of the form

    m_updatedCharts = 0;
    m_newCharts = 0;

#if !defined(CHART_LIST)
    // Clear any existing panels
    m_panelArray.clear();
    m_scrollWinChartList->ClearBackground();
#endif /* CHART_LIST */

    for (size_t i = 0; i < pPlugIn->m_pChartCatalog.charts.size(); i++) {
      wxString status;
      wxString latest;
      bool bcheck = false;
      wxString file_ =
          pPlugIn->m_pChartCatalog.charts.at(i)->GetChartFilename(true);
      if (!pPlugIn->m_pChartSource->ExistsLocaly(
              pPlugIn->m_pChartCatalog.charts.at(i)->number, file_)) {
        m_newCharts++;
        status = _("New");
        if (selnew) bcheck = true;
      } else {
        if (pPlugIn->m_pChartSource->IsNewerThanLocal(
                pPlugIn->m_pChartCatalog.charts.at(i)->number, file_,
                pPlugIn->m_pChartCatalog.charts.at(i)->GetUpdateDatetime())) {
          m_updatedCharts++;
          status = _("Out of date");
          if (selupd) bcheck = true;
        } else {
          status = _("Up to date");
        }
      }
      latest =
          pPlugIn->m_pChartCatalog.charts.at(i)->GetUpdateDatetime().Format(
              _T("%Y-%m-%d"));

#if defined(CHART_LIST)
      wxVector<wxVariant> data;
      data.push_back(wxVariant(bcheck));
      data.push_back(wxVariant(status));
      data.push_back(wxVariant(latest));
      data.push_back(
          wxVariant(pPlugIn->m_pChartCatalog.charts.at(i)->GetChartTitle()));
      getChartList()->AppendItem(data);
#else
      auto pC = std::make_unique<ChartPanel>(
          m_scrollWinChartList, wxID_ANY, wxDefaultPosition, wxSize(-1, -1),
          pPlugIn->m_pChartCatalog.charts.at(i)->GetChartTitle(), status,
          latest, this, bcheck);
      pC->Connect(wxEVT_RIGHT_DOWN,
                  wxMouseEventHandler(ChartDldrPanel::OnContextMenu), NULL,
                  this);

      m_boxSizerCharts->Add(pC.get(), 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
      m_panelArray.push_back(std::move(pC));
#endif /* CHART_LIST */
    }

#if !defined(CHART_LIST)  // wxDataViewListCtrl handles all of this AFAIK: Dan
    m_scrollWinChartList->ClearBackground();
    m_scrollWinChartList->FitInside();
    m_scrollWinChartList->GetSizer()->Layout();
    Layout();
    m_scrollWinChartList->ClearBackground();
    SetChartInfo(wxString::Format(_("%lu charts total, %lu updated, %lu new"),
                                  pPlugIn->m_pChartCatalog.charts.size(),
                                  m_updatedCharts, m_newCharts));
#else
    SetChartInfo(wxString::Format(
        _("%lu charts total, %lu updated, %lu new, %lu selected"),
        pPlugIn->m_pChartCatalog.charts.size(), m_updatedCharts, m_newCharts,
        GetCheckedChartCount()));
#endif /* CHART_LIST */
  }
}

int ChartDldrPanelImpl::GetSelectedCatalog() {
  long item =
      m_lbChartSources->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  return item;
}

void ChartDldrPanelImpl::SelectCatalog(int item) {
  if (item >= 0) {
    m_bDeleteSource->Enable();
    m_bEditSource->Enable();
    m_bUpdateChartList->Enable();
  } else {
    m_bDeleteSource->Disable();
    m_bEditSource->Disable();
    m_bUpdateChartList->Disable();
  }
  m_lbChartSources->SetItemState(item, wxLIST_STATE_SELECTED,
                                 wxLIST_STATE_SELECTED);
}

void ChartDldrPanelImpl::AppendCatalog(std::unique_ptr<ChartSource> &cs) {
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
  wxFileName fn(url.GetPath());
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
}

bool ChartDldrPanelImpl::DownloadChart(wxString url, wxString file,
                                       wxString title) {
  return false;
}

void ChartDldrPanelImpl::DisableForDownload(bool enabled) {
  m_bAddSource->Enable(enabled);
  m_bDeleteSource->Enable(enabled);
  m_bEditSource->Enable(enabled);
  m_bUpdateAllCharts->Enable(enabled);
  m_bUpdateChartList->Enable(enabled);
  m_lbChartSources->Enable(enabled);
#if defined(CHART_LIST)
  m_bSelectNew->Enable(enabled);
  m_bSelectUpdated->Enable(enabled);
  m_bSelectAll->Enable(enabled);
#endif /* CHART_LIST */
}

void ChartDldrPanelImpl::OnDownloadCharts(wxCommandEvent &event) {
  if (DownloadIsCancel) {
    cancelled = true;
    return;
  }
  DownloadCharts(ChartDldrSelectedChartsDownloadProfile());
}
#if defined(CHART_LIST)
void ChartDldrPanelImpl::OnSelectChartItem(wxCommandEvent &event) {
  if (!m_bInfoHold)
    SetChartInfo(wxString::Format(
        _("%lu charts total, %lu updated, %lu new, %lu selected"),
        pPlugIn->m_pChartCatalog.charts.size(), m_updatedCharts, m_newCharts,
        GetCheckedChartCount()));
  else
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

int ChartDldrPanelImpl::GetChartCount() {
#if defined(CHART_LIST)
  return getChartList()->GetItemCount();
#else
  return m_panelArray.size();
#endif /* CHART_LIST*/
}

int ChartDldrPanelImpl::GetCheckedChartCount() {
#if defined(CHART_LIST)
  int cnt = 0;
  int chartCnt = GetChartCount();
  for (int i = 0; i < chartCnt; i++)
    if (isChartChecked(i)) cnt++;
#else
  int cnt = 0;
  for (int i = 0; i < GetChartCount(); i++) {
    if (m_panelArray.at(i)->GetCB()->IsChecked()) cnt++;
  }
#endif /* CHART_LIST*/
  return cnt;
}

bool ChartDldrPanelImpl::isChartChecked(int i) {
  wxASSERT_MSG(i >= 0,
               wxT("This function should be called with non-negative index."));
  if (i <= GetChartCount())
#if defined(CHART_LIST)
    return getChartList()->GetToggleValue(i, 0);
#else
    return m_panelArray.at(i)->GetCB()->IsChecked();
#endif /* CHART_LIST*/
  else
    return false;
}

void ChartDldrPanelImpl::CheckAllCharts(bool value) {
#if defined(CHART_LIST)
  m_bInfoHold = true;
#endif /* CHART_LIST */

  for (int i = 0; i < GetChartCount(); i++) {
#if defined(CHART_LIST)
    getChartList()->SetToggleValue(value, i, 0);
#else
    m_panelArray.at(i)->GetCB()->SetValue(value);
#endif /* CHART_LIST*/
  }
#if defined(CHART_LIST)
  SetChartInfo(wxString::Format(
      _("%lu charts total, %lu updated, %lu new, %lu selected"),
      pPlugIn->m_pChartCatalog.charts.size(), m_updatedCharts, m_newCharts,
      GetCheckedChartCount()));
  m_bInfoHold = false;
#endif /* CHART_LIST */
}

void ChartDldrPanelImpl::CheckNewCharts(bool value) {
  for (int i = 0; i < GetChartCount(); i++) {
#if defined(CHART_LIST)
    if (isNew(i)) getChartList()->SetToggleValue(true, i, 0);
#else
    if (m_panelArray.at(i)->isNew())
      m_panelArray.at(i)->GetCB()->SetValue(value);
#endif /* CHART_LIST*/
  }
#if defined(CHART_LIST)
  SetChartInfo(wxString::Format(
      _("%lu charts total, %lu updated, %lu new, %lu selected"),
      pPlugIn->m_pChartCatalog.charts.size(), m_updatedCharts, m_newCharts,
      GetCheckedChartCount()));
#endif /* CHART_LIST */
}

void ChartDldrPanelImpl::CheckUpdatedCharts(bool value) {
  for (int i = 0; i < GetChartCount(); i++) {
#if defined(CHART_LIST)
    if (isUpdated(i)) getChartList()->SetToggleValue(value, i, 0);
#else
    if (m_panelArray.at(i)->isUpdated())
      m_panelArray.at(i)->GetCB()->SetValue(value);
#endif /* CHART_LIST */
  }
#if defined(CHART_LIST)
  SetChartInfo(wxString::Format(
      _("%lu charts total, %lu updated, %lu new, %lu selected"),
      pPlugIn->m_pChartCatalog.charts.size(), m_updatedCharts, m_newCharts,
      GetCheckedChartCount()));
#endif /* CHART_LIST */
}

void ChartDldrPanelImpl::InvertCheckAllCharts() {
#if defined(CHART_LIST)
  m_bInfoHold = true;
#endif /* CHART_LIST */
  for (int i = 0; i < GetChartCount(); i++)
#if defined(CHART_LIST)
    getChartList()->SetToggleValue(!isChartChecked(i), i, 0);
#else
    m_panelArray.at(i)->GetCB()->SetValue(!isChartChecked(i));
#endif /* CHART_LIST */
#if defined(CHART_LIST)
  m_bInfoHold = false;
  SetChartInfo(wxString::Format(
      _("%lu charts total, %lu updated, %lu new, %lu selected"),
      pPlugIn->m_pChartCatalog.charts.size(), m_updatedCharts, m_newCharts,
      GetCheckedChartCount()));
#endif /* CHART_LIST */
}

ChartDldrPanelImpl::~ChartDldrPanelImpl() {
  Disconnect(
      wxEVT_DOWNLOAD_EVENT,
      (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
  m_bconnected = false;

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
    : ChartDldrPanel(parent, id, pos, size, style), bulk_(this) {
  m_bDeleteSource->Disable();
  m_bUpdateChartList->Disable();
  m_bEditSource->Disable();
  m_lbChartSources->InsertColumn(0, _("Catalog"), wxLIST_FORMAT_LEFT,
                                 CATALOGS_NAME_WIDTH);
  m_lbChartSources->InsertColumn(1, _("Released"), wxLIST_FORMAT_LEFT,
                                 CATALOGS_DATE_WIDTH);
  m_lbChartSources->InsertColumn(2, _("Local path"), wxLIST_FORMAT_LEFT,
                                 CATALOGS_PATH_WIDTH);
  m_lbChartSources->Enable();
  m_bInfoHold = false;
  cancelled = true;
  to_download = -1;
  m_downloading = -1;
  pPlugIn = plugin;
  m_populated = false;
  DownloadIsCancel = false;
  m_failed_downloads = 0;
  m_bulkDownloadedNew = 0;
  m_bulkDownloadedUpdated = 0;
  SetChartInfo(wxEmptyString);
  m_transfer.Reset();

  Connect(
      wxEVT_DOWNLOAD_EVENT,
      (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
  m_bconnected = true;

  for (size_t i = 0; i < pPlugIn->m_ChartSources.size(); i++) {
    AppendCatalog(pPlugIn->m_ChartSources.at(i));
  }
  m_populated = true;
}

void ChartDldrPanelImpl::OnPaint(wxPaintEvent &event) {
  if (!m_populated) {
    m_populated = true;
    for (size_t i = 0; i < pPlugIn->m_ChartSources.size(); i++) {
      AppendCatalog(pPlugIn->m_ChartSources.at(i));
    }
  }
#ifdef __WXMAC__
  // Mojave does not paint the controls correctly without this.
  m_lbChartSources->Refresh(true);
#endif
  event.Skip();
}

void ChartDldrPanelImpl::DeleteSource(wxCommandEvent &event) {
  if (!m_lbChartSources->GetSelectedItemCount()) return;
  if (wxID_YES != OCPNMessageBox_PlugIn(
                      this,
                      _("Do you really want to remove the chart source?\nThe "
                        "local chart files will not be removed,\nbut you will "
                        "not be able to update the charts anymore."),
                      _("Chart Downloader"), wxYES_NO | wxCENTRE))
    return;
  int ToBeRemoved = GetSelectedCatalog();
  m_lbChartSources->SetItemState(ToBeRemoved, 0,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
  pPlugIn->m_ChartSources.erase(pPlugIn->m_ChartSources.begin() + ToBeRemoved);
  m_lbChartSources->DeleteItem(ToBeRemoved);
  CleanForm();
  pPlugIn->SetSourceId(-1);
  SelectCatalog(-1);
  pPlugIn->SaveConfig();
  event.Skip();
}

void ChartDldrPanelImpl::AddSource(wxCommandEvent &event) {
  ChartDldrGuiAddSourceDlg *dialog = new ChartDldrGuiAddSourceDlg(this);
  dialog->SetBasePath(pPlugIn->GetBaseChartDir());

  wxSize sz = GetParent()
                  ->GetGrandParent()
                  ->GetSize();  // This is the options panel true size
  dialog->SetSize(sz.GetWidth(), sz.GetHeight());
  dialog->Center();

#ifdef __ANDROID__
  androidDisableRotation();
#endif

  if (dialog->ShowModal() == wxID_OK) {
    std::unique_ptr<ChartSource> cs =
        std::make_unique<ChartSource>(dialog->m_tSourceName->GetValue(),
                                      dialog->m_tChartSourceUrl->GetValue(),
                                      dialog->m_tcChartDirectory->GetValue());
    dialog->Destroy();
    AppendCatalog(cs);
    bool covered = false;
    for (size_t i = 0; i < GetChartDBDirArrayString().GetCount(); i++) {
      if (cs->GetDir().StartsWith((GetChartDBDirArrayString().Item(i)))) {
        covered = true;
        break;
      }
    }
    if (!covered) {
      wxString dir = cs->GetDir();
      AddChartDirectory(dir);
    }

    long itemSelectedNow = GetSelectedCatalog();
    m_lbChartSources->SetItemState(itemSelectedNow, 0, wxLIST_STATE_SELECTED);

    SelectCatalog(m_lbChartSources->GetItemCount() - 1);
    pPlugIn->m_ChartSources.push_back(std::move(cs));
    pPlugIn->SaveConfig();
  }
#ifdef __ANDROID__
  androidEnableRotation();
#endif

  event.Skip();
}

void ChartDldrPanelImpl::DoEditSource() {
  if (!m_lbChartSources->GetSelectedItemCount()) return;
  int cat = GetSelectedCatalog();
  ChartDldrGuiAddSourceDlg *dialog = new ChartDldrGuiAddSourceDlg(this);
  dialog->SetBasePath(pPlugIn->GetBaseChartDir());
  dialog->SetSourceEdit(pPlugIn->m_ChartSources.at(cat));
  dialog->SetTitle(_("Edit Chart Source"));

  dialog->ShowModal();
  int retcode = dialog->GetReturnCode();
  {
    if (retcode == wxID_OK) {
      pPlugIn->m_ChartSources.at(cat)->SetName(
          dialog->m_tSourceName->GetValue());
      pPlugIn->m_ChartSources.at(cat)->SetUrl(
          dialog->m_tChartSourceUrl->GetValue());
      pPlugIn->m_ChartSources.at(cat)->SetDir(
          dialog->m_tcChartDirectory->GetValue());

      m_lbChartSources->SetItem(cat, 0,
                                pPlugIn->m_ChartSources.at(cat)->GetName());
      m_lbChartSources->SetItem(cat, 1, _("(Please update first)"));
      m_lbChartSources->SetItem(cat, 2,
                                pPlugIn->m_ChartSources.at(cat)->GetDir());
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
      if (!covered)
        OCPNMessageBox_PlugIn(
            this,
            wxString::Format(
                _("Path %s seems not to be covered by your configured Chart "
                  "Directories.\nTo see the charts you have to adjust the "
                  "configuration on the 'Chart Files' tab."),
                pPlugIn->m_ChartSources.at(cat)->GetDir().c_str()),
            _("Chart Downloader"));

      pPlugIn->SaveConfig();
      SetSource(cat);
    }
  }
}

void ChartDldrPanelImpl::EditSource(wxCommandEvent &event) {
  DoEditSource();
  event.Skip();
}

void ChartDldrPanelImpl::OnLeftDClick(wxMouseEvent &event) {
  DoEditSource();
  event.Skip();
}

void ChartDldrPanelImpl::onDLEvent(OCPN_downloadEvent &ev) {
  if (m_transfer.owner == ChartDldrBulkTransferOwner::None) {
    return;
  }

  switch (ev.getDLEventCondition()) {
    case OCPN_DL_EVENT_TYPE_END:
      m_transfer.OnEnd(ev.getDLEventStatus() == OCPN_DL_NO_ERROR);
      break;

    case OCPN_DL_EVENT_TYPE_PROGRESS:
      m_transfer.OnProgress(ev.getTotal(), ev.getTransferred());
      break;
    default:
      break;
  }
  wxYieldIfNeeded();
}

void ChartDldrPanelImpl::EnsureDownloadHandlerConnected() {
  if (m_bconnected) {
    return;
  }
  Connect(
      wxEVT_DOWNLOAD_EVENT,
      (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
  m_bconnected = true;
}

ChartDldrBulkRunStats ChartDldrPanelImpl::TakeBulkCatalogStats() const {
  ChartDldrBulkRunStats stats;
  stats.attempted = m_downloading;
  stats.failed = m_failed_downloads;
  stats.new_downloads = m_bulkDownloadedNew;
  stats.updated_downloads = m_bulkDownloadedUpdated;
  return stats;
}
