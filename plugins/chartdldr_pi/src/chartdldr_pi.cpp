/**************************************************************************
 *   Copyright (C) 2011 by Pavel Kalian                                    *
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
 **************************************************************************/

/**
 * \file
 *
 * Chart Downloader Plugin -- plugin implementation
 */

#ifdef __ANDROID__
#define _LIBCPP_HAS_NO_OFF_T_FUNCTIONS
#endif

#include "chartdldr_pi.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <memory>

#ifdef DLDR_USE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#ifdef CHARTDLDR_RAR_UNARR
#include "unarr.h"
#endif
#else
#include "unarr.h"
#endif

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/debug.h>
#include <wx/dir.h>
#include <wx/filename.h>
// #include <wx/filesys.h>
#include <wx/listctrl.h>
#include <wx/progdlg.h>
#include <wx/regex.h>
#include <wx/sstream.h>
// #include <wx/stdpaths.h>
#include <wx/url.h>
#include <wx/wfstream.h>
// #include <wx/wfstream.h>
#include <wxWTranslateCatalog.h>
#include <wx/zipstrm.h>

#include "icons.h"
#include "version.h"

#ifdef __ANDROID__
#include <QtAndroidExtras/QAndroidJniObject>
#include "qdebug.h"
#include "android_support.h"
#include "android_jvm.h"
#include <jni.h>
#endif

#ifdef __WXMAC__
#define CATALOGS_NAME_WIDTH 300
#define CATALOGS_DATE_WIDTH 120
#define CATALOGS_PATH_WIDTH 100
#define CHARTS_NAME_WIDTH 300
#define CHARTS_STATUS_WIDTH 100
#define CHARTS_DATE_WIDTH 120
#else
#ifdef __ANDROID__

#define CATALOGS_NAME_WIDTH 350
#define CATALOGS_DATE_WIDTH 500
#define CATALOGS_PATH_WIDTH 1000
#define CHARTS_NAME_WIDTH 520
#define CHARTS_STATUS_WIDTH 150
#define CHARTS_DATE_WIDTH 200

#else

#define CATALOGS_NAME_WIDTH 200
#define CATALOGS_DATE_WIDTH 130
#define CATALOGS_PATH_WIDTH 250
#define CHARTS_NAME_WIDTH 320
#define CHARTS_STATUS_WIDTH 150
#define CHARTS_DATE_WIDTH 130

#endif
#endif  // __WXMAC__

#define CHART_DIR "Charts"

static wxString FormatBytes(long bytes);

namespace {

constexpr double kStarterChartLat = 33.358;
constexpr double kStarterChartLon = -79.282;
constexpr double kStarterChartScalePpm = 0.25;

const std::string kStarterCatalogUrl =
    "https://www.charts.noaa.gov/ENCs/SC_ENCProdCat.xml";

bool PanelContainsPosition(const Panel &panel, double lat, double lon) {
  const auto &vertices = panel.vertexes;
  if (vertices.size() < 3) return false;

  bool inside = false;
  for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
    const Vertex &a = vertices[i];
    const Vertex &b = vertices[j];
    const bool crosses_latitude = (a.lat > lat) != (b.lat > lat);
    if (crosses_latitude &&
        lon < (b.lon - a.lon) * (lat - a.lat) / (b.lat - a.lat) + a.lon) {
      inside = !inside;
    }
  }
  return inside;
}

bool ChartContainsPosition(const Chart &chart, double lat, double lon) {
  return std::any_of(chart.coverage.begin(), chart.coverage.end(),
                     [lat, lon](const std::unique_ptr<Panel> &panel) {
                       return panel && PanelContainsPosition(*panel, lat, lon);
                     });
}

struct StarterChartChoice {
  size_t catalog_index;
  std::string label;
};

std::vector<StarterChartChoice> FindStarterCharts(const ChartCatalog &catalog) {
  std::vector<size_t> candidate_indexes;
  for (size_t i = 0; i < catalog.charts.size(); ++i) {
    Chart *chart = catalog.charts[i].get();
    const auto *enc = dynamic_cast<const EncCell *>(chart);
    if (!enc || enc->cscale <= 0 || enc->status.CmpNoCase("Canceled") == 0 ||
        chart->NeedsManualDownload() ||
        chart->GetDownloadLocation().IsEmpty()) {
      continue;
    }
    if (ChartContainsPosition(*chart, kStarterChartLat, kStarterChartLon)) {
      candidate_indexes.push_back(i);
    }
  }

  std::stable_sort(
      candidate_indexes.begin(), candidate_indexes.end(),
      [&catalog](size_t left, size_t right) {
        const auto *left_cell =
            dynamic_cast<const EncCell *>(catalog.charts[left].get());
        const auto *right_cell =
            dynamic_cast<const EncCell *>(catalog.charts[right].get());
        return left_cell->cscale < right_cell->cscale;
      });

  std::vector<StarterChartChoice> choices;
  for (size_t index : candidate_indexes) {
    const Chart &chart = *catalog.charts[index];
    const auto &enc = dynamic_cast<const EncCell &>(chart);
    std::string label =
        wxString::Format("%s - %s (1:%d, %s)", chart.number.c_str(),
                         chart.title.c_str(), enc.cscale,
                         FormatBytes(chart.zipfile_size).c_str())
            .ToStdString();
    if (choices.empty()) {
      label = wxString::Format(_("%s (recommended)"), wxString(label))
                  .ToStdString();
    }
    choices.push_back({index, label});
  }
  return choices;
}

class StarterChartDialog final : public wxDialog {
public:
  StarterChartDialog(wxWindow *parent,
                     const std::vector<StarterChartChoice> &choices)
      : wxDialog(parent, wxID_ANY, _("Free starter chart"), wxDefaultPosition,
                 wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
        m_choices(choices),
        m_catalog_index(choices.front().catalog_index),
        m_chart_list(nullptr) {
    // Size from font metrics, capped to the display, instead of fixed
    // pixels, so the dialog fits small and high-DPI screens alike.
    const int text_width =
        std::min(80 * GetCharWidth(), wxGetDisplaySize().GetWidth() * 85 / 100);
    const int list_height = 10 * GetCharHeight();
    auto *top_sizer = new wxBoxSizer(wxVERTICAL);

    auto *heading =
        new wxStaticText(this, wxID_ANY, _("Try OpenCPN with a real chart"));
    wxFont heading_font = heading->GetFont();
    heading_font.SetWeight(wxFONTWEIGHT_BOLD);
    heading_font.SetPointSize(heading_font.GetPointSize() + 2);
    heading->SetFont(heading_font);
    top_sizer->Add(heading, 0, wxLEFT | wxRIGHT | wxTOP, 12);

    auto *intro = new wxStaticText(
        this, wxID_ANY,
        _("Chart Downloader found current free NOAA charts around OpenCPN's "
          "sample location in Georgetown, South Carolina. Choose one to "
          "download and install automatically."));
    intro->Wrap(text_width);
    top_sizer->Add(intro, 0, wxEXPAND | wxALL, 12);

    wxArrayString labels;
    for (const auto &choice : choices) labels.Add(choice.label);
    m_chart_list = new wxListBox(this, wxID_ANY, wxDefaultPosition,
                                 wxSize(text_width, list_height), labels);
    m_chart_list->SetSelection(0);
    top_sizer->Add(m_chart_list, 1, wxEXPAND | wxLEFT | wxRIGHT, 12);

    auto *more_charts = new wxStaticText(
        this, wxID_ANY,
        _("More free charts are available in Options > Charts > Chart "
          "Downloader. Commercial charts can be purchased and installed "
          "using O-Charts or other compatible providers."));
    more_charts->Wrap(text_width);
    top_sizer->Add(more_charts, 0, wxEXPAND | wxALL, 12);

    auto *buttons = new wxStdDialogButtonSizer();
    auto *cancel = new wxButton(this, wxID_CANCEL, _("Not now"));
    auto *download = new wxButton(this, wxID_OK, _("Download chart"));
    buttons->AddButton(cancel);
    buttons->AddButton(download);
    buttons->Realize();
    top_sizer->Add(buttons, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    SetAffirmativeId(wxID_OK);
    SetEscapeId(wxID_CANCEL);
    SetSizerAndFit(top_sizer);
    SetMinSize(GetSize());
    CentreOnParent();

    download->Bind(wxEVT_BUTTON,
                   [this](wxCommandEvent &) { ConfirmSelection(); });
    m_chart_list->Bind(wxEVT_LISTBOX_DCLICK,
                       [this](wxCommandEvent &) { ConfirmSelection(); });
  }

  size_t GetCatalogIndex() const { return m_catalog_index; }

private:
  void ConfirmSelection() {
    int selection = m_chart_list->GetSelection();
    if (selection == wxNOT_FOUND ||
        static_cast<size_t>(selection) >= m_choices.size()) {
      selection = 0;
    }
    m_catalog_index = m_choices[selection].catalog_index;
    EndModal(wxID_OK);
  }

  const std::vector<StarterChartChoice> m_choices;
  size_t m_catalog_index;
  wxListBox *m_chart_list;
};

bool IsDirectoryCovered(const wxArrayString &chart_dirs,
                        const wxString &directory) {
  // wxPATH_NORM_CASE lowercases only on case-insensitive filesystems.
  const int norm_flags = wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE |
                         wxPATH_NORM_LONG | wxPATH_NORM_CASE;
  wxFileName child(directory, wxEmptyString);
  child.Normalize(norm_flags);
  const wxString child_path = child.GetPath();

  for (const auto &configured : chart_dirs) {
    wxFileName parent(configured, wxEmptyString);
    parent.Normalize(norm_flags);
    wxString parent_path = parent.GetPath();
    if (child_path == parent_path) return true;
    if (!parent_path.EndsWith(wxFileName::GetPathSeparator())) {
      parent_path += wxFileName::GetPathSeparator();
    }
    if (child_path.StartsWith(parent_path)) return true;
  }
  return false;
}

}  // namespace

extern "C" DECL_EXP opencpn_plugin *create_pi(void *ppimgr) {
  return new chartdldr_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin *p) { delete p; }

double g_androidDPmm;
chartdldr_pi *g_pi;

#ifdef __ANDROID__
int g_Android_SDK_Version;
#endif
// the class factories, used to create and destroy instances of the PlugIn

bool getDisplayMetrics();  // External in chartdldr_pi.h

// Helper function to check if a path is safely inside the target directory
// Returns true if normalizedPath is inside targetDir, false otherwise (path
// traversal attempt)
static bool IsPathInsideDir(const wxString &targetDir,
                            const wxString &entryName, wxString &outFullPath) {
  // Construct the full path
  wxString combinedPath = targetDir;
  if (!combinedPath.EndsWith(wxFileName::GetPathSeparator())) {
    combinedPath += wxFileName::GetPathSeparator();
  }
  combinedPath += entryName;

  // Normalize the combined path to resolve any ".." components
  wxFileName fn(combinedPath);
  fn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG);
  outFullPath = fn.GetFullPath();

  // Normalize target dir for comparison
  wxFileName targetFn(targetDir);
  targetFn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE |
                     wxPATH_NORM_LONG);
  wxString normalizedTarget = targetFn.GetFullPath();

  // Ensure target ends with separator for proper prefix matching
  if (!normalizedTarget.EndsWith(wxFileName::GetPathSeparator())) {
    normalizedTarget += wxFileName::GetPathSeparator();
  }

  // Check if the normalized path starts with the target directory
  // This catches all path traversal attempts including "../", absolute paths,
  // etc.
  if (outFullPath.StartsWith(normalizedTarget)) {
    return true;
  }

  // Also allow if it's exactly the target directory (for directory entries)
  if (outFullPath == targetFn.GetFullPath()) {
    return true;
  }

  return false;
}

static wxString FormatBytes(double bytes) {
  if (bytes <= 0) return "?";
  return wxString::Format("%.1fMB", bytes / 1024 / 1024);
}

static wxString FormatBytes(long bytes) {
  return FormatBytes(static_cast<double>(bytes));
}

static bool IsDLDirWritable(const wxFileName &fn) {
#ifndef __ANDROID__
  return fn.IsDirWritable();
#else
  if (g_Android_SDK_Version >= 30) {  // scoped storage?
    // Use a simple test here
    return (fn.GetFullPath().Contains("org.opencpn.opencpn"));  // fast test
  } else
    return fn.IsDirWritable();
#endif
}

static void SetBackColor(wxWindow *ctrl, const wxColour &col) {
  static int depth = 0;  // recursion count
  if (depth == 0) {      // only for the window root, not for every child

    ctrl->SetBackgroundColour(col);
  }

  wxWindowList kids = ctrl->GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode *node = kids.Item(i);
    wxWindow *win = node->GetData();

    if (dynamic_cast<wxListBox *>(win))
      dynamic_cast<wxListBox *>(win)->SetBackgroundColour(col);

    else if (dynamic_cast<wxTextCtrl *>(win))
      dynamic_cast<wxTextCtrl *>(win)->SetBackgroundColour(col);

    //        else if( win->IsKindOf( CLASSINFO(wxStaticText) ) )
    //            ( (wxStaticText*) win )->SetForegroundColour( uitext );

    else if (dynamic_cast<wxChoice *>(win))
      dynamic_cast<wxChoice *>(win)->SetBackgroundColour(col);

    else if (dynamic_cast<wxComboBox *>(win))
      dynamic_cast<wxComboBox *>(win)->SetBackgroundColour(col);

    else if (dynamic_cast<wxRadioButton *>(win))
      dynamic_cast<wxRadioButton *>(win)->SetBackgroundColour(col);

    else if (dynamic_cast<wxScrolledWindow *>(win)) {
      dynamic_cast<wxScrolledWindow *>(win)->SetBackgroundColour(col);
    }

    else if (dynamic_cast<wxButton *>(win)) {
      dynamic_cast<wxButton *>(win)->SetBackgroundColour(col);
    }

    else {
      ;
    }

    if (win->GetChildren().GetCount() > 0) {
      depth++;
      wxWindow *w = win;
      SetBackColor(w, col);
      depth--;
    }
  }
}

//---------------------------------------------------------------------------------------------------------
//
//    ChartDldr PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

chartdldr_pi::chartdldr_pi(void *ppimgr) : opencpn_plugin_113(ppimgr) {
  m_alive = std::make_shared<bool>(true);
  // Create the PlugIn icons
  initialize_images();

  m_parent_window = nullptr;
  m_chart_source = nullptr;
  m_config = nullptr;
  m_reselect_new = false;
  m_reselect_updated = false;
  m_allow_bulk_update = false;
  m_options_page = nullptr;
  m_selected_source = -1;
  m_dldrpanel = nullptr;
  m_schartdldr_sources = "";

  g_pi = this;
}

int chartdldr_pi::Init() {
  AddLocaleCatalog(PLUGIN_CATALOG_NAME);

  //    Get a pointer to the opencpn display canvas, to use as a parent for the
  //    POI Manager dialog
  m_parent_window = GetOCPNCanvasWindow();

  //    Get a pointer to the opencpn configuration object
  m_config = GetOCPNConfigObject();
  m_options_page = nullptr;

  m_chart_source = nullptr;

#ifdef __ANDROID__
  androidGetSDKVersion();
#endif

  //    And load the configuration items
  LoadConfig();

  getDisplayMetrics();

  wxStringTokenizer st(m_schartdldr_sources, "|", wxTOKEN_DEFAULT);
  while (st.HasMoreTokens()) {
    wxString s1 = st.GetNextToken();
    wxString s2 = st.GetNextToken();
    wxString s3 = st.GetNextToken();
    if (!s2.IsEmpty())  // scrub empty sources.
      m_ChartSources.push_back(std::make_unique<ChartSource>(s1, s2, s3));
  }
#ifndef __ANDROID__
  // Ask the host to report startup events, used for the starter chart
  // offer on a clean installation.
  auto host_api = GetHostApi();
  auto *api = dynamic_cast<HostApi122 *>(host_api.get());
  assert(api && "HostApi122 not available");
  api->RegisterApiEventCallback(
      GetCommonName().ToStdString(),
      [this](HostApi122::EventType what) { OnHostApiEvent(what); });
#endif

  return (WANTS_PREFERENCES | WANTS_CONFIG | INSTALLS_TOOLBOX_PAGE);
}

void chartdldr_pi::OnHostApiEvent(HostApi122::EventType what) {
  if (what != HostApi122::EventType::kInitialStart) return;
  wxLogMessage("chartdldr_pi: Initial start reported.");
  // The event callback must return promptly and cannot run a nested event
  // loop, so present the starter chart offer from the main loop instead.
  // Guard against the plugin being deactivated before the offer runs.
  std::weak_ptr<bool> alive = m_alive;
  wxTheApp->CallAfter([this, alive] {
    if (alive.expired()) return;
    OnInitialStart();
  });
}

void chartdldr_pi::OnInitialStart() {
  if (!GetChartDBDirArrayString().IsEmpty()) {
    wxLogMessage(
        "chartdldr_pi: Charts are already configured; skipping starter "
        "chart offer.");
    return;
  }
  // Ask before going online: no network access without user consent.
  const int answer = OCPNMessageBox_PlugIn(
      m_parent_window,
      _("No charts are installed yet.\n\n"
        "Check charts.noaa.gov for a current free starter chart? The chart "
        "can be downloaded and installed automatically."),
      _("Chart Downloader"), wxYES_NO | wxICON_QUESTION);
  if (answer != wxID_YES) {
    wxLogMessage("chartdldr_pi: Starter chart offer declined.");
    return;
  }
  OfferStarterChart();
}

void chartdldr_pi::OfferStarterChart() {
  const wxString temp_catalog =
      wxFileName::CreateTempFileName("opencpn-starter-catalog");
  const long download_style =
      OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
      OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE |
      OCPN_DLDS_URL | OCPN_DLDS_CAN_ABORT | OCPN_DLDS_AUTO_CLOSE;
  const _OCPN_DLStatus status = OCPN_downloadFile(
      kStarterCatalogUrl, temp_catalog, _("Looking for starter charts"),
      _("Retrieving the current list of free charts"), wxNullBitmap,
      m_parent_window, download_style, 10);

  if (status != OCPN_DL_NO_ERROR ||
      !m_chart_catalog.LoadFromFile(temp_catalog)) {
    wxLogMessage(
        "chartdldr_pi: Starter chart catalog unavailable; skipping offer.");
    if (wxFileExists(temp_catalog)) wxRemoveFile(temp_catalog);
    return;
  }

  const auto choices = FindStarterCharts(m_chart_catalog);
  if (choices.empty()) {
    wxLogMessage(
        "chartdldr_pi: Starter chart catalog contains no suitable charts; "
        "skipping offer.");
    wxRemoveFile(temp_catalog);
    return;
  }

  StarterChartDialog dialog(m_parent_window, choices);
  if (dialog.ShowModal() == wxID_OK) {
    const size_t catalog_index = dialog.GetCatalogIndex();
    if (catalog_index >= m_chart_catalog.charts.size()) {
      wxLogWarning("chartdldr_pi: Invalid starter chart selection.");
    } else {
      Chart &selected_chart = *m_chart_catalog.charts[catalog_index];
      wxLogMessage("chartdldr_pi: Installing selected starter chart %s.",
                   selected_chart.number.c_str());
      InstallStarterChart(selected_chart, temp_catalog);
    }
  }
  wxRemoveFile(temp_catalog);
}

bool chartdldr_pi::InstallStarterChart(Chart &chart,
                                       const wxString &catalog_path) {
  wxString chart_directory;
  ChartSource *source = nullptr;
  size_t source_index = 0;
  for (size_t i = 0; i < m_ChartSources.size(); ++i) {
    if (m_ChartSources[i]->GetUrl().ToStdString() == kStarterCatalogUrl) {
      source = m_ChartSources[i].get();
      source_index = i;
      chart_directory = source->GetDir();
      break;
    }
  }

  if (chart_directory.IsEmpty()) {
    wxFileName directory(m_base_chart_dir, wxEmptyString);
    directory.AppendDir("ENC");
    directory.AppendDir("US_SC");
    chart_directory = directory.GetPath();
  }

  bool created_directory = false;
  if (!wxDirExists(chart_directory)) {
    if (!wxFileName::Mkdir(chart_directory, 0755, wxPATH_MKDIR_FULL)) {
      OCPNMessageBox_PlugIn(
          m_parent_window,
          wxString::Format(_("Directory %s can't be created."),
                           chart_directory.c_str()),
          _("Chart Downloader"), wxOK | wxICON_ERROR);
      return false;
    }
    created_directory = true;
  }

  wxFileName writable_directory(chart_directory, wxEmptyString);
  if (!IsDLDirWritable(writable_directory)) {
    OCPNMessageBox_PlugIn(m_parent_window,
                          wxString::Format(_("Directory %s is not writable."),
                                           chart_directory.c_str()),
                          _("Chart Downloader"), wxOK | wxICON_ERROR);
    return false;
  }

  wxURI chart_url(chart.GetDownloadLocation());
  wxFileName archive_name(chart.GetChartFilename(false));
  if (chart_url.IsReference() || archive_name.GetFullName().IsEmpty()) {
    wxLogMessage("chartdldr_pi: Invalid starter chart download metadata.");
    return false;
  }

  archive_name.SetPath(chart_directory);
  const wxString archive_path = archive_name.GetFullPath();
  if (wxFileExists(archive_path)) wxRemoveFile(archive_path);

  const long download_style =
      OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
      OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE |
      OCPN_DLDS_URL | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT |
      OCPN_DLDS_AUTO_CLOSE;
  const _OCPN_DLStatus status = OCPN_downloadFile(
      chart_url.BuildURI(), archive_path, _("Downloading starter chart"),
      chart.GetChartTitle(), wxNullBitmap, m_parent_window, download_style, 30);
  if (status != OCPN_DL_NO_ERROR ||
      !ProcessFile(archive_path, chart_directory, true,
                   chart.GetUpdateDatetime())) {
    if (wxFileExists(archive_path)) wxRemoveFile(archive_path);
    if (created_directory) wxFileName::Rmdir(chart_directory);
    if (status != OCPN_DL_ABORTED) {
      OCPNMessageBox_PlugIn(
          m_parent_window,
          _("The starter chart could not be downloaded and installed.\n\n"
            "You can retry later using the Chart Downloader in Options > "
            "Charts."),
          _("Chart Downloader"), wxOK | wxICON_ERROR);
    }
    return false;
  }

  wxURI catalog_url(kStarterCatalogUrl);
  wxFileName local_catalog(catalog_url.GetPath());
  local_catalog.SetPath(chart_directory);
  if (!wxCopyFile(catalog_path, local_catalog.GetFullPath(), true)) {
    wxLogWarning("chartdldr_pi: Could not retain the starter chart catalog.");
  }

  if (!source) {
    m_ChartSources.push_back(std::make_unique<ChartSource>(
        _("SC - South Carolina"), kStarterCatalogUrl, chart_directory));
    source_index = m_ChartSources.size() - 1;
    source = m_ChartSources.back().get();
  } else {
    source->SetDir(chart_directory);
  }
  source->ChartUpdated(chart.number, chart.GetUpdateDatetime().GetTicks());
  m_selected_source = static_cast<int>(source_index);
  m_chart_source = source;
  SaveConfig();

  wxArrayString chart_dirs = GetChartDBDirArrayString();
  if (!IsDirectoryCovered(chart_dirs, chart_directory)) {
    chart_dirs.Add(chart_directory);
  }
  UpdateChartDBInplace(chart_dirs, false, true);
  JumpToPosition(kStarterChartLat, kStarterChartLon, kStarterChartScalePpm);
  return true;
}

bool chartdldr_pi::DeInit() {
  wxLogMessage("chartdldr_pi: DeInit");

  m_alive.reset();  // drop any pending deferred starter-chart offer
  m_ChartSources.clear();
  // wxDELETE(m_pChartSource);
  /* TODO: Seth */
  //      dialog->Close();
  //      dialog->Destroy();
  //      wxDELETE(dialog);
  /* We must delete remaining page if the plugin is disabled while in Options
   * dialog */
  if (m_options_page) {
    if (DeleteOptionsPage(m_options_page)) m_options_page = nullptr;
    // TODO: any other memory leak?
  }
  return true;
}

int chartdldr_pi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int chartdldr_pi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

int chartdldr_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int chartdldr_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap *chartdldr_pi::GetPlugInBitmap() { return _img_chartdldr_pi; }

wxString chartdldr_pi::GetCommonName() { return _("ChartDownloader"); }

wxString chartdldr_pi::GetShortDescription() {
  return _("Chart Downloader PlugIn for OpenCPN");
}

wxString chartdldr_pi::GetLongDescription() {
  return _(
      "Chart Downloader PlugIn for OpenCPN\n\
Manages chart downloads and updates from sources supporting\n\
NOAA Chart Catalog format");
}

void chartdldr_pi::OnSetupOptions() {
  m_options_page =
      AddOptionsPage(PI_OPTIONS_PARENT_CHARTS, _("Chart Downloader"));
  if (!m_options_page) {
    wxLogMessage("Error: chartdldr_pi::OnSetupOptions AddOptionsPage failed!");
    return;
  }
  auto *sizer = new wxBoxSizer(wxVERTICAL);
  m_options_page->SetSizer(sizer);

  m_dldrpanel =
      new ChartDldrPanelImpl(this, m_options_page, wxID_ANY, wxDefaultPosition,
                             wxDefaultSize, wxDEFAULT_DIALOG_STYLE);

  m_options_page->InvalidateBestSize();
  sizer->Add(m_dldrpanel, 1, wxALL | wxEXPAND);
  m_dldrpanel->SetBulkUpdate(m_allow_bulk_update);
  m_dldrpanel->FitInside();
}

void chartdldr_pi::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel) {
  /* TODO: Seth */
  m_dldrpanel->CancelDownload();
#ifndef __ANDROID__
  OCPN_cancelDownloadFileBackground(
      0);  // Stop the thread, is something like this needed on Android as well?
#endif
  m_selected_source = m_dldrpanel->GetSelectedCatalog();
  SaveConfig();
}

bool chartdldr_pi::LoadConfig() {
  auto *pConf = (wxFileConfig *)m_config;

  if (pConf) {
    pConf->SetPath("/Settings/ChartDnldr");
    pConf->Read("ChartSources", &m_schartdldr_sources, "");
    pConf->Read("Source", &m_selected_source, -1);

    wxFileName fn(GetWritableDocumentsDir(), "");
    fn.AppendDir(CHART_DIR);

    pConf->Read("BaseChartDir", &m_base_chart_dir, fn.GetPath());
    wxLogMessage("chartdldr_pi:m_base_chart_dir: " + m_base_chart_dir);

    // Check to see if the directory is writeable, esp. on App updates.
    wxFileName testFN(m_base_chart_dir);
    if (!IsDLDirWritable(testFN)) {
      wxLogMessage(
          "Cannot write to m_base_chart_dir, override to "
          "GetWritableDocumentsDir()");
      m_base_chart_dir = fn.GetPath();
      wxLogMessage("chartdldr_pi: Corrected: " + m_base_chart_dir);
    }

    pConf->Read("PreselectNew", &m_reselect_new, true);
    pConf->Read("PreselectUpdated", &m_reselect_updated, true);
    pConf->Read("AllowBulkUpdate", &m_allow_bulk_update, false);
    return true;
  } else
    return false;
}

bool chartdldr_pi::SaveConfig() {
  auto *pConf = (wxFileConfig *)m_config;

  m_schartdldr_sources.Clear();

  for (const std::unique_ptr<ChartSource> &cs : m_ChartSources) {
    m_schartdldr_sources.Append(
        wxString::Format("%s|%s|%s|", cs->GetName().c_str(),
                         cs->GetUrl().c_str(), cs->GetDir().c_str()));
  }

  if (pConf) {
    pConf->SetPath("/Settings/ChartDnldr");
    pConf->Write("ChartSources", m_schartdldr_sources);
    pConf->Write("Source", m_selected_source);
    pConf->Write("BaseChartDir", m_base_chart_dir);
    pConf->Write("PreselectNew", m_reselect_new);
    pConf->Write("PreselectUpdated", m_reselect_updated);
    pConf->Write("AllowBulkUpdate", m_allow_bulk_update);

    return true;
  } else
    return false;
}

void chartdldr_pi::ShowPreferencesDialog(wxWindow *parent) {
  auto *dialog = new ChartDldrPrefsDlgImpl(parent);

  wxFont fo = GetOCPNGUIScaledFont_PlugIn(_("Dialog"));
  dialog->SetFont(fo);

#ifdef __ANDROID__
  if (m_parent_window) {
    int xmax = m_parent_window->GetSize().GetWidth();
    int ymax = m_parent_window->GetParent()
                   ->GetSize()
                   .GetHeight();  // This would be the Options dialog itself
    dialog->SetSize(xmax, ymax);
    dialog->Layout();

    dialog->Move(0, 0);
  }

  wxColour cl = wxColour(214, 218, 222);
  SetBackColor(dialog, cl);
#endif

  dialog->SetPath(m_base_chart_dir);
  dialog->SetPreferences(m_reselect_new, m_reselect_updated,
                         m_allow_bulk_update);

  dialog->ShowModal();
  dialog->Destroy();
}

void chartdldr_pi::UpdatePrefs(ChartDldrPrefsDlgImpl *dialog) {
  m_base_chart_dir = dialog->GetPath();
  dialog->GetPreferences(m_reselect_new, m_reselect_updated,
                         m_allow_bulk_update);
  SaveConfig();
  if (m_dldrpanel) m_dldrpanel->SetBulkUpdate(m_allow_bulk_update);
}

bool getDisplayMetrics() {
#ifdef __ANDROID__

  g_androidDPmm = 4.0;  // nominal default

  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    return false;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod("getDisplayMetrics", "()Ljava/lang/String;");

  wxString return_string;
  jstring s = data.object<jstring>();

  //  Need a Java environment to decode the resulting string
  JNIEnv *jenv;
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  //  Return string may have commas instead of periods, if using Euro locale
  //  We just fix it here...
  return_string.Replace(",", ".");

  //     wxLogMessage("Metrics:" + return_string);
  //     wxSize screen_size = ::wxGetDisplaySize();
  //     wxString msg;
  //     msg.Printf("wxGetDisplaySize(): %d %d", screen_size.x,
  //     screen_size.y); wxLogMessage(msg);

  double density = 1.0;
  wxStringTokenizer tk(return_string, ";");
  if (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();  // xdpi
    token = tk.GetNextToken();           // density

    long b = ::wxGetDisplaySize().y;
    token.ToDouble(&density);

    token = tk.GetNextToken();  // ldpi

    token = tk.GetNextToken();  // width
    token = tk.GetNextToken();  // height - statusBarHeight
    token = tk.GetNextToken();  // width
    token = tk.GetNextToken();  // height
    token = tk.GetNextToken();  // dm.widthPixels
    token = tk.GetNextToken();  // dm.heightPixels

    token = tk.GetNextToken();  // actionBarHeight
    long abh;
    token.ToLong(&abh);
    //        g_ActionBarHeight = wxMax(abh, 50);

    //        qDebug() << "g_ActionBarHeight" << abh << g_ActionBarHeight;
  }

  double ldpi = 160. * density;

  //    double maxDim = wxMax(::wxGetDisplaySize().x, ::wxGetDisplaySize().y);
  //    ret = (maxDim / ldpi) * 25.4;

  //    msg.Printf("Android Auto Display Size (mm, est.): %g", ret);
  //    wxLogMessage(msg);

  //  Save some items as global statics for convenience
  g_androidDPmm = ldpi / 25.4;
  //    g_androidDensity = density;

  // qDebug() << "PI Metrics" << g_androidDPmm << density;
  return true;
#else

  return true;
#endif
}

ChartSource::ChartSource(const wxString &name, const wxString &url,
                         const wxString &localdir) {
  m_name = name;
  m_url = url;
  m_dir = localdir;
  m_update_data.clear();
}

ChartSource::~ChartSource() { m_update_data.clear(); }

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
    default:
      assert(false && "Illegal popup menu id");
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

  menu.Append(ID_MNU_SELALL, _("Select all"), "");
  menu.Append(ID_MNU_DELALL, _("Deselect all"), "");
  menu.Append(ID_MNU_INVSEL, _("Invert selection"), "");
  menu.Append(ID_MNU_SELUPD, _("Select updated"), "");
  menu.Append(ID_MNU_SELNEW, _("Select new"), "");

#endif

  menu.Connect(wxEVT_COMMAND_MENU_SELECTED,
               (wxObjectEventFunction)&ChartDldrPanelImpl::OnPopupClick,
               nullptr, this);
  // and then display
  PopupMenu(&menu, mouseClient.x, mouseClient.y);
}

void ChartDldrPanelImpl::OnShowLocalDir(wxCommandEvent &event) {
  if (!m_plugin->m_chart_source) return;
#ifdef __WXGTK__
  wxExecute(wxString::Format("xdg-open %s",
                             m_plugin->m_chart_source->GetDir().c_str()));
#endif
#ifdef __WXMAC__
  wxExecute(
      wxString::Format("open %s", m_plugin->m_chart_source->GetDir().c_str()));
#endif
#ifdef __WXMSW__
  wxExecute(wxString::Format("explorer %s",
                             m_plugin->m_chart_source->GetDir().c_str()));
#endif
}

void ChartDldrPanelImpl::SetSource(int id) {
  m_plugin->SetSourceId(id);

  m_bDeleteSource->Enable(id >= 0);
  m_bUpdateChartList->Enable(id >= 0);
  m_bEditSource->Enable(id >= 0);

  // TODO: DAN - Need to optimze to only update the chart list if needed.
  //             Right now it updates multiple times unnecessarily.
  CleanForm();
  if (id >= 0 && id < (int)m_plugin->m_ChartSources.size()) {
    ::wxBeginBusyCursor();  // wxSetCursor(wxCURSOR_WAIT);
    //        wxYield();
    std::unique_ptr<ChartSource> &cs = m_plugin->m_ChartSources.at(id);
    cs->LoadUpdateData();
    cs->UpdateLocalFiles();
    m_plugin->m_chart_source = cs.get();
    FillFromFile(cs->GetUrl(), cs->GetDir(), m_plugin->m_reselect_new,
                 m_plugin->m_reselect_updated);
    wxURI url(cs->GetUrl());
    m_chartsLabel->SetLabel(wxString::Format(
        _("Charts: %s"),
        (cs->GetName() + _(" from ") + url.BuildURI() + " -> " + cs->GetDir())
            .c_str()));
    if (::wxIsBusy()) ::wxEndBusyCursor();
  } else {
    m_plugin->m_chart_source = nullptr;
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

void ChartDldrPanelImpl::FillFromFile(const wxString &url, const wxString &dir,
                                      bool selnew, bool selupd) {
  // load if exists
  wxStringTokenizer tk(url, "/");
  wxString file;
  do {
    file = tk.GetNextToken();
  } while (tk.HasMoreTokens());
  wxFileName fn;
  fn.SetFullName(file);
  fn.SetPath(dir);
  wxString path = fn.GetFullPath();
  if (wxFileExists(path)) {
    m_plugin->m_chart_catalog.LoadFromFile(path);
    //            m_tChartSourceInfo->SetValue(pPlugIn->m_pChartCatalog.GetDescription());
    // fill in the rest of the form

    m_updated_charts = 0;
    m_new_charts = 0;

#if !defined(CHART_LIST)
    // Clear any existing panels
    m_panelArray.clear();
    m_scrollWinChartList->ClearBackground();
#endif /* CHART_LIST */

    for (size_t i = 0; i < m_plugin->m_chart_catalog.charts.size(); i++) {
      wxString status;
      wxString latest;
      bool bcheck = false;
      wxString file_ =
          m_plugin->m_chart_catalog.charts.at(i)->GetChartFilename(true);
      if (!m_plugin->m_chart_source->ExistsLocally(
              m_plugin->m_chart_catalog.charts.at(i)->number, file_)) {
        m_new_charts++;
        status = _("New");
        if (selnew) bcheck = true;
      } else {
        if (m_plugin->m_chart_source->IsNewerThanLocal(
                m_plugin->m_chart_catalog.charts.at(i)->number, file_,
                m_plugin->m_chart_catalog.charts.at(i)->GetUpdateDatetime())) {
          m_updated_charts++;
          status = _("Out of date");
          if (selupd) bcheck = true;
        } else {
          status = _("Up to date");
        }
      }
      latest =
          m_plugin->m_chart_catalog.charts.at(i)->GetUpdateDatetime().Format(
              "%Y-%m-%d");

#if defined(CHART_LIST)
      wxVector<wxVariant> data;
      data.push_back(wxVariant(bcheck));
      data.push_back(wxVariant(status));
      data.push_back(wxVariant(latest));
      data.push_back(
          wxVariant(m_plugin->m_chart_catalog.charts.at(i)->GetChartTitle()));
      getChartList()->AppendItem(data);
#else
      auto pC = std::make_unique<ChartPanel>(
          m_scrollWinChartList, wxID_ANY, wxDefaultPosition, wxSize(-1, -1),
          m_plugin->m_chart_catalog.charts.at(i)->GetChartTitle(), status,
          latest, this, bcheck);
      pC->Connect(wxEVT_RIGHT_DOWN,
                  wxMouseEventHandler(ChartDldrPanel::OnContextMenu), nullptr,
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
                                  m_plugin->m_chart_catalog.charts.size(),
                                  m_updated_charts, m_new_charts));
#else
    SetChartInfo(wxString::Format(
        _("%lu charts total, %lu updated, %lu new, %lu selected"),
        m_plugin->m_chart_catalog.charts.size(), m_updated_charts, m_new_charts,
        GetCheckedChartCount()));
#endif /* CHART_LIST */
  }
}

bool ChartSource::ExistsLocally(const wxString &chart_number,
                                const wxString &filename) {
  wxASSERT(this);

  wxStringTokenizer tk(filename, ".");
  wxString file = tk.GetNextToken().MakeLower();

  if (!m_update_data.empty()) {
    return m_update_data.find(std::string(chart_number.Lower().mb_str())) !=
               m_update_data.end() ||
           m_update_data.find(std::string(file.mb_str())) !=
               m_update_data.end();
  }
  for (size_t i = 0; i < m_localfiles.Count(); i++) {
    if (m_localfiles.Item(i) == file) return true;
  }
  return false;
}

bool ChartSource::IsNewerThanLocal(const wxString &chart_number,
                                   const wxString &filename,
                                   const wxDateTime &validDate) {
  wxStringTokenizer tk(filename, ".");
  wxString file = tk.GetNextToken().MakeLower();
  time_t validTime = validDate.GetTicks();
  if (!m_update_data.empty()) {
    time_t chartNumberTime =
        m_update_data[std::string(chart_number.Lower().mbc_str())];
    time_t updateFileTime = m_update_data[std::string(file.mbc_str())];
    bool needsUpdate =
        chartNumberTime < validTime && updateFileTime < validTime;
    if (wxLOG_Debug <= wxLog::GetLogLevel()) {
      // Show these only if user has selected loglevel debug, otherwise save a
      // few cpu cycles
      wxLogInfo("Latest Zip File Date: %sZ",
                validDate.ToUTC().FormatISOCombined());
      wxLogInfo("Local File: %s, Date: %sZ", filename,
                wxDateTime(updateFileTime).ToUTC().FormatISOCombined());
      wxLogInfo("Chart Number %s DOB: Date: %sZ", chart_number,
                wxDateTime(chartNumberTime).ToUTC().FormatISOCombined());
      wxLogInfo("Chart Number %s Needs update: %s", chart_number,
                needsUpdate ? wxString("true") : wxString("false"));
    }
    return needsUpdate;
  }

  bool update_candidate = false;

  for (size_t i = 0; i < m_localfiles.Count(); i++) {
    if (m_localfiles.Item(i) == file) {
      if (validDate.IsLaterThan(m_localdt.at(i))) {
        update_candidate = true;
      } else
        return false;
    }
  }
  return update_candidate;
}

int ChartDldrPanelImpl::GetSelectedCatalog() {
  long item =
      m_lbChartSources->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  return static_cast<int>(item);
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
    if (m_plugin->m_chart_catalog.LoadFromFile(path, true)) {
      m_lbChartSources->SetItem(id, 0, m_plugin->m_chart_catalog.title);
      m_lbChartSources->SetItem(
          id, 1,
          m_plugin->m_chart_catalog.GetReleaseDate().Format("%Y-%m-%d %H:%M"));
      m_lbChartSources->SetItem(id, 2, path);
#ifdef __ANDROID__
      m_lbChartSources->GetHandle()->resizeColumnToContents(0);
      m_lbChartSources->GetHandle()->resizeColumnToContents(1);
      m_lbChartSources->GetHandle()->resizeColumnToContents(2);
#endif
    }
  }
}

void ChartDldrPanelImpl::UpdateAllCharts(wxCommandEvent &event) {
  int failed_to_update = 0;
  int attempted_to_update = 0;
  if ((m_plugin->m_reselect_new) && (m_plugin->m_reselect_updated)) {
    wxMessageDialog mess(
        this,
        _("You have chosen to update all chart catalogs.\nThen download all "
          "new and updated charts.\nThis may take a long time."),
        _("Chart Downloader"), wxOK | wxCANCEL);
    if (mess.ShowModal() == wxID_CANCEL) return;
  } else if (m_plugin->m_reselect_new) {
    wxMessageDialog mess(
        this,
        _("You have chosen to update all chart catalogs.\nThen download only "
          "new (but not updated) charts.\nThis may take a long time."),
        _("Chart Downloader"), wxOK | wxCANCEL);
    if (mess.ShowModal() == wxID_CANCEL) return;
  } else if (m_plugin->m_reselect_updated) {
    wxMessageDialog mess(
        this,
        _("You have chosen to update all chart catalogs.\nThen download only "
          "updated (but not new) charts.\nThis may take a long time."),
        _("Chart Downloader"), wxOK | wxCANCEL);
    if (mess.ShowModal() == wxID_CANCEL) return;
  }
  m_updating_all = true;
  m_cancelled = false;
  // Flip to the list of charts so user can observe the download progress
  int oldPage = m_DLoadNB->SetSelection(1);
  for (long chartIndex = 0; chartIndex < m_lbChartSources->GetItemCount();
       chartIndex++) {
    m_lbChartSources->SetItemState(chartIndex, wxLIST_STATE_SELECTED,
                                   wxLIST_STATE_SELECTED);
    if (m_cancelled) break;
    UpdateChartList(event);
    DownloadCharts();
    attempted_to_update += m_downloading;
    failed_to_update += m_failed_downloads;
  }
  wxLogMessage(wxString::Format(
      "chartdldr_pi::UpdateAllCharts() downloaded %d out of %d charts.",
      attempted_to_update - failed_to_update, attempted_to_update));
  if (failed_to_update > 0)
    OCPNMessageBox_PlugIn(
        this,
        wxString::Format(_("%d out of %d charts failed to download.\nCheck the "
                           "list, verify there is a working Internet "
                           "connection and repeat the operation if needed."),
                         failed_to_update, attempted_to_update),
        _("Chart Downloader"), wxOK | wxICON_ERROR);
  if (attempted_to_update > failed_to_update) ForceChartDBUpdate();
  m_updating_all = false;
  m_cancelled = false;
  // Flip back to the original page
  m_DLoadNB->SetSelection(oldPage);
}

void ChartDldrPanelImpl::UpdateChartList(wxCommandEvent &event) {
  // TODO: check if everything exists and we can write to the output dir etc.
  if (!m_lbChartSources->GetSelectedItemCount()) return;
  std::unique_ptr<ChartSource> &cs =
      m_plugin->m_ChartSources.at(GetSelectedCatalog());
  wxURI url(cs->GetUrl());
  if (url.IsReference()) {
    OCPNMessageBox_PlugIn(
        this, _("Error, the URL to the chart source data seems wrong."),
        _("Error"));
    return;
  }

  wxStringTokenizer tk(url.GetPath(), "/");
  wxString file;
  do {
    file = tk.GetNextToken();
  } while (tk.HasMoreTokens());
  wxFileName fn;
  fn.SetFullName(file);
  fn.SetPath(cs->GetDir());
  if (!wxDirExists(cs->GetDir())) {
    if (!wxFileName::Mkdir(cs->GetDir(), 0755, wxPATH_MKDIR_FULL)) {
      OCPNMessageBox_PlugIn(
          this,
          wxString::Format(_("Directory %s can't be created."),
                           cs->GetDir().c_str()),
          _("Chart Downloader"));
      return;
    }
  }

  bool bok = false;

#ifdef __ANDROID__
  wxString file_URI = "file://" + fn.GetFullPath();

  //     wxFile testFile(tfn.GetFullPath().c_str(), wxFile::write);
  //     if(!testFile.IsOpened()){
  //         wxMessageBox(this, wxString::Format(_("File  %s can't be written.
  //         \nChoose a writable folder for Chart Downloader file storage."),
  //         tfn.GetFullPath().c_str()), _("Chart Downloader")); return;
  //     }
  //     testFile.Close();
  //     ::wxRemoveFile(tfn.GetFullPath());

  _OCPN_DLStatus ret = OCPN_downloadFile(
      cs->GetUrl(), file_URI, _("Downloading file"),
      _("Reading Headers: ") + url.BuildURI(), wxNullBitmap, this,
      OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
          OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE |
          OCPN_DLDS_URL | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT |
          OCPN_DLDS_AUTO_CLOSE,
      10);
  bok = true;

#else
  wxFileName tfn = wxFileName::CreateTempFileName(fn.GetFullPath());
  wxString file_URI = tfn.GetFullPath();

  _OCPN_DLStatus ret = OCPN_downloadFile(
      cs->GetUrl(), file_URI, _("Downloading file"),
      _("Reading Headers: ") + url.BuildURI(), wxNullBitmap, this,
      OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
          OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE |
          OCPN_DLDS_URL | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT |
          OCPN_DLDS_AUTO_CLOSE,
      10);

  bok = wxCopyFile(tfn.GetFullPath(), fn.GetFullPath());
  wxRemoveFile(tfn.GetFullPath());

#endif

  //    wxLogMessage("chartdldr_pi:  OCPN_downloadFile done:");

  switch (ret) {
    case OCPN_DL_NO_ERROR: {
      if (bok) {
        int id = GetSelectedCatalog();
        SetSource(id);

        m_lbChartSources->SetItem(id, 0, m_plugin->m_chart_catalog.title);
        m_lbChartSources->SetItem(
            id, 1,
            m_plugin->m_chart_catalog.GetReleaseDate().Format(
                "%Y-%m-%d %H:%M"));
        m_lbChartSources->SetItem(id, 2, cs->GetDir());

      } else
        OCPNMessageBox_PlugIn(
            this,
            wxString::Format(_("Failed to Find New Catalog: %s "),
                             url.BuildURI().c_str()),
            _("Chart Downloader"), wxOK | wxICON_ERROR);
      break;
    }
    case OCPN_DL_FAILED: {
      OCPNMessageBox_PlugIn(
          this,
          wxString::Format(_("Failed to Download Catalog: %s \nVerify there is "
                             "a working Internet connection."),
                           url.BuildURI().c_str()),
          _("Chart Downloader"), wxOK | wxICON_ERROR);
      break;
    }

    case OCPN_DL_USER_TIMEOUT:
    case OCPN_DL_ABORTED: {
      m_cancelled = true;
      break;
    }

    case OCPN_DL_UNKNOWN:
    case OCPN_DL_STARTED: {
      break;
    }

    default:
      wxASSERT(false);  // This should never happen because we handle all
                        // possible cases of ret
  }

  if ((ret == OCPN_DL_NO_ERROR) && bok) m_DLoadNB->SetSelection(1);
}

void ChartSource::GetLocalFiles() {
  if (!UpdateDataExists() || m_update_data.empty()) {
    auto *allFiles = new wxArrayString;
    if (wxDirExists(GetDir())) wxDir::GetAllFiles(GetDir(), allFiles);
    m_localdt.clear();
    m_localfiles.Clear();
    wxDateTime ct, mt, at;
    wxString name;
    for (size_t i = 0; i < allFiles->Count(); i++) {
      wxFileName fn(allFiles->Item(i));
      name = fn.GetFullName().Lower();
      // Only add unique files names to the local list.
      // This is safe because all chart names within a catalog
      // are necessarily unique.
      if (!ExistsLocally("", name)) {
        fn.GetTimes(&at, &mt, &ct);
        m_localdt.push_back(mt);
        m_localfiles.Add(fn.GetName().Lower());

        wxStringTokenizer tk(name, ".");
        wxString file = tk.GetNextToken().MakeLower();
        m_update_data[std::string(file.mbc_str())] = mt.GetTicks();
      }
    }
    allFiles->Clear();
    wxDELETE(allFiles);
    SaveUpdateData();
  } else {
    LoadUpdateData();
  }
}

bool ChartSource::UpdateDataExists() {
  return wxFileExists(GetDir() + wxFileName::GetPathSeparator() +
                      UPDATE_DATA_FILENAME);
}

void ChartSource::LoadUpdateData() {
  m_update_data.clear();
  wxString fn =
      GetDir() + wxFileName::GetPathSeparator() + UPDATE_DATA_FILENAME;

  if (!wxFileExists(fn)) return;

  std::ifstream infile(fn.mb_str());

  std::string key;
  time_t value(0);

  while (infile >> key >> value) m_update_data[key] = value;

  infile.close();
}

void ChartSource::SaveUpdateData() {
  wxString fn;
  fn = GetDir() + wxFileName::GetPathSeparator() + UPDATE_DATA_FILENAME;

#ifdef __ANDROID__
  fn = AndroidGetCacheDir() + wxFileName::GetPathSeparator() +
       UPDATE_DATA_FILENAME;
#endif

  std::ofstream outfile(fn.mb_str());
  if (!outfile.is_open()) return;

  std::map<std::string, time_t>::iterator iter;
  for (iter = m_update_data.begin(); iter != m_update_data.end(); ++iter) {
    if (iter->first.find(" ") == std::string::npos)
      if (!iter->first.empty())
        outfile << iter->first << " " << iter->second << "\n";
  }

  outfile.close();

#ifdef __ANDROID__
  AndroidSecureCopyFile(
      fn, GetDir() + wxFileName::GetPathSeparator() + UPDATE_DATA_FILENAME);
#endif
}

void ChartSource::ChartUpdated(const wxString &chart_number, time_t timestamp) {
  m_update_data[std::string(chart_number.Lower().mb_str())] = timestamp;
  SaveUpdateData();
}

bool ChartDldrPanelImpl::DownloadChart(const wxString &url,
                                       const wxString &file,
                                       const wxString &title) {
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
  if (m_download_is_cancel) {
    m_cancelled = true;
    return;
  }
  DownloadCharts();
}
#if defined(CHART_LIST)
void ChartDldrPanelImpl::OnSelectChartItem(wxCommandEvent &event) {
  if (!m_hold_info)
    SetChartInfo(wxString::Format(
        _("%lu charts total, %lu updated, %lu new, %lu selected"),
        m_plugin->m_chart_catalog.charts.size(), m_updated_charts, m_new_charts,
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
  return static_cast<int>(m_panelArray.size());
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
               "This function should be called with non-negative index.");
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
  m_hold_info = true;
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
      m_plugin->m_chart_catalog.charts.size(), m_updated_charts, m_new_charts,
      GetCheckedChartCount()));
  m_hold_info = false;
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
      m_plugin->m_chart_catalog.charts.size(), m_updated_charts, m_new_charts,
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
      m_plugin->m_chart_catalog.charts.size(), m_updated_charts, m_new_charts,
      GetCheckedChartCount()));
#endif /* CHART_LIST */
}

void ChartDldrPanelImpl::InvertCheckAllCharts() {
#if defined(CHART_LIST)
  m_hold_info = true;
#endif /* CHART_LIST */
  for (int i = 0; i < GetChartCount(); i++)
#if defined(CHART_LIST)
    getChartList()->SetToggleValue(!isChartChecked(i), i, 0);
#else
    m_panelArray.at(i)->GetCB()->SetValue(!isChartChecked(i));
#endif /* CHART_LIST */
#if defined(CHART_LIST)
  m_hold_info = false;
  SetChartInfo(wxString::Format(
      _("%lu charts total, %lu updated, %lu new, %lu selected"),
      m_plugin->m_chart_catalog.charts.size(), m_updated_charts, m_new_charts,
      GetCheckedChartCount()));
#endif /* CHART_LIST */
}

void ChartDldrPanelImpl::DownloadCharts() {
  if (!m_is_connected) {
    Connect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
    m_is_connected = true;
  }

  if (!GetCheckedChartCount() && !m_updating_all) {
    OCPNMessageBox_PlugIn(this, _("No charts selected for download."));
    return;
  }
  std::unique_ptr<ChartSource> &cs =
      m_plugin->m_ChartSources.at(GetSelectedCatalog());

  m_cancelled = false;
  m_to_download = GetCheckedChartCount();
  m_downloading = 0;
  m_failed_downloads = 0;
  DisableForDownload(false);
  // wxString old_label = m_bDnldCharts->GetLabel();     // Broken on Android??
  m_bDnldCharts->SetLabel(_("Abort download"));
  m_download_is_cancel = true;

  wxFileName downloaded_p;
  int idx = -1;

  for (int i = 0; i < GetChartCount() && m_to_download; i++) {
    int index = i;
    if (m_cancelled) break;
    // Prepare download queues
    if (!isChartChecked(i)) continue;
    m_is_transfer_complete = false;
    m_is_transfer_ok = true;
    m_total_size = -1;
    m_transferred_size = 0;
    m_downloading++;
    if (m_plugin->m_chart_catalog.charts.at(index)->NeedsManualDownload()) {
      if (wxID_YES ==
          OCPNMessageBox_PlugIn(
              this,
              wxString::Format(
                  _("The selected chart '%s' can't be downloaded automatically, do you want me to open a browser window and download them manually?\n\n \
After downloading the charts, please extract them to %s"),
                  m_plugin->m_chart_catalog.charts.at(index)->title.c_str(),
                  m_plugin->m_chart_source->GetDir().c_str()),
              _("Chart Downloader"), wxYES_NO | wxCENTRE | wxICON_QUESTION)) {
        wxLaunchDefaultBrowser(
            m_plugin->m_chart_catalog.charts.at(index)->GetManualDownloadUrl());
      }
      continue;
    }

    // download queue
    wxURI url(
        m_plugin->m_chart_catalog.charts.at(index)->GetDownloadLocation());
    if (url.IsReference()) {
      OCPNMessageBox_PlugIn(
          this,
          wxString::Format(
              _("Error, the URL to the chart (%s) data seems wrong."),
              url.BuildURI().c_str()),
          _("Error"));
      this->Enable();
      /// XXX undo anything? return or break?
      return;
    }
    // construct local file path
    wxString file =
        m_plugin->m_chart_catalog.charts.at(index)->GetChartFilename(false);
    wxFileName fn;
    fn.SetFullName(file);
    fn.SetPath(cs->GetDir());
    wxString path = fn.GetFullPath();
    if (wxFileExists(path)) wxRemoveFile(path);
    wxString title =
        m_plugin->m_chart_catalog.charts.at(index)->GetChartTitle();

    //  Ready to start download
#ifdef __ANDROID__
    wxString file_path = "file://" + fn.GetFullPath();
#else
    wxString file_path = fn.GetFullPath();
#endif

    long handle;
    OCPN_downloadFileBackground(url.BuildURI(), file_path, this, &handle);

    if (idx >= 0) {
      if (m_plugin->ProcessFile(
              downloaded_p.GetFullPath(), downloaded_p.GetPath(), true,
              m_plugin->m_chart_catalog.charts.at(idx)->GetUpdateDatetime())) {
        cs->ChartUpdated(m_plugin->m_chart_catalog.charts.at(idx)->number,
                         m_plugin->m_chart_catalog.charts.at(idx)
                             ->GetUpdateDatetime()
                             .GetTicks());
      } else {
        m_failed_downloads++;
      }
      idx = -1;
    }

    while (!m_is_transfer_complete && m_is_transfer_ok && !m_cancelled) {
      if (m_failed_downloads)
        SetChartInfo(wxString::Format(
            _("Downloading chart %u of %u, %u downloads failed (%s / %s)"),
            m_downloading, m_to_download, m_failed_downloads,
            FormatBytes(m_transferred_size), FormatBytes(m_total_size)));
      else
        SetChartInfo(wxString::Format(_("Downloading chart %u of %u (%s / %s)"),
                                      m_downloading, m_to_download,
                                      FormatBytes(m_transferred_size),
                                      FormatBytes(m_total_size)));

      Update();
      Refresh();

      wxTheApp->ProcessPendingEvents();
      wxYield();
      wxMilliSleep(20);
    }

    if (m_cancelled) {
      idx = -1;
      OCPN_cancelDownloadFileBackground(handle);
    }

    if (m_is_transfer_ok && !m_cancelled) {
      idx = index;
      downloaded_p = path;
    } else {
      idx = -1;
      if (wxFileExists(path)) wxRemoveFile(path);
      m_failed_downloads++;
    }
  }
  if (idx >= 0) {
    if (m_plugin->ProcessFile(
            downloaded_p.GetFullPath(), downloaded_p.GetPath(), true,
            m_plugin->m_chart_catalog.charts.at(idx)->GetUpdateDatetime())) {
      cs->ChartUpdated(m_plugin->m_chart_catalog.charts.at(idx)->number,
                       m_plugin->m_chart_catalog.charts.at(idx)
                           ->GetUpdateDatetime()
                           .GetTicks());
    } else {
      m_failed_downloads++;
    }
  }
  DisableForDownload(true);
  m_bDnldCharts->SetLabel(_("Download selected charts"));
  m_download_is_cancel = false;
  SetSource(GetSelectedCatalog());
  if (m_failed_downloads > 0 && !m_updating_all && !m_cancelled)
    OCPNMessageBox_PlugIn(
        this,
        wxString::Format(_("%d out of %d charts failed to download.\nCheck the "
                           "list, verify there is a working Internet "
                           "connection and repeat the operation if needed."),
                         m_failed_downloads, m_downloading),
        _("Chart Downloader"), wxOK | wxICON_ERROR);

  if (m_cancelled)
    OCPNMessageBox_PlugIn(this, _("Chart download cancelled."),
                          _("Chart Downloader"), wxOK | wxICON_INFORMATION);

  if ((m_downloading - m_failed_downloads > 0) && !m_updating_all)
    ForceChartDBUpdate();
}

ChartDldrPanelImpl::~ChartDldrPanelImpl() {
  Disconnect(
      wxEVT_DOWNLOAD_EVENT,
      (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
  m_is_connected = false;

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
    : ChartDldrPanel(parent, id, pos, size, style) {
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
  m_hold_info = false;
  m_cancelled = true;
  m_to_download = -1;
  m_downloading = -1;
  m_updating_all = false;
  m_plugin = plugin;
  m_is_populated = false;
  m_download_is_cancel = false;
  m_failed_downloads = 0;
  ChartDldrPanelImpl::SetChartInfo("");
  m_is_transfer_complete = true;
  m_is_transfer_ok = true;

  Connect(
      wxEVT_DOWNLOAD_EVENT,
      (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
  m_is_connected = true;

  for (size_t i = 0; i < m_plugin->m_ChartSources.size(); i++) {
    AppendCatalog(m_plugin->m_ChartSources.at(i));
  }
  m_is_populated = true;
}

void ChartDldrPanelImpl::OnPaint(wxPaintEvent &event) {
  if (!m_is_populated) {
    m_is_populated = true;
    for (size_t i = 0; i < m_plugin->m_ChartSources.size(); i++) {
      AppendCatalog(m_plugin->m_ChartSources.at(i));
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
  m_plugin->m_ChartSources.erase(m_plugin->m_ChartSources.begin() +
                                 ToBeRemoved);
  m_lbChartSources->DeleteItem(ToBeRemoved);
  CleanForm();
  m_plugin->SetSourceId(-1);
  SelectCatalog(-1);
  m_plugin->SaveConfig();
  event.Skip();
}

void ChartDldrPanelImpl::AddSource(wxCommandEvent &event) {
  auto *dialog = new ChartDldrGuiAddSourceDlg(this);
  dialog->SetBasePath(m_plugin->GetBaseChartDir());

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
    m_plugin->m_ChartSources.push_back(std::move(cs));
    m_plugin->SaveConfig();
  }
#ifdef __ANDROID__
  androidEnableRotation();
#endif

  event.Skip();
}

void ChartDldrPanelImpl::DoEditSource() {
  if (!m_lbChartSources->GetSelectedItemCount()) return;
  int cat = GetSelectedCatalog();
  auto *dialog = new ChartDldrGuiAddSourceDlg(this);
  dialog->SetBasePath(m_plugin->GetBaseChartDir());
  dialog->SetSourceEdit(m_plugin->m_ChartSources.at(cat));
  dialog->SetTitle(_("Edit Chart Source"));

  dialog->ShowModal();
  int retcode = dialog->GetReturnCode();
  {
    if (retcode == wxID_OK) {
      m_plugin->m_ChartSources.at(cat)->SetName(
          dialog->m_tSourceName->GetValue());
      m_plugin->m_ChartSources.at(cat)->SetUrl(
          dialog->m_tChartSourceUrl->GetValue());
      m_plugin->m_ChartSources.at(cat)->SetDir(
          dialog->m_tcChartDirectory->GetValue());

      m_lbChartSources->SetItem(cat, 0,
                                m_plugin->m_ChartSources.at(cat)->GetName());
      m_lbChartSources->SetItem(cat, 1, _("(Please update first)"));
      m_lbChartSources->SetItem(cat, 2,
                                m_plugin->m_ChartSources.at(cat)->GetDir());
      wxURI url(m_plugin->m_ChartSources.at(cat)->GetUrl());
      wxFileName fn(url.GetPath());
      fn.SetPath(m_plugin->m_ChartSources.at(cat)->GetDir());
      wxString path = fn.GetFullPath();
      if (wxFileExists(path)) {
        if (m_plugin->m_chart_catalog.LoadFromFile(path, true)) {
          m_lbChartSources->SetItem(cat, 0, m_plugin->m_chart_catalog.title);
          m_lbChartSources->SetItem(
              cat, 1,
              m_plugin->m_chart_catalog.GetReleaseDate().Format(
                  "%Y-%m-%d %H:%M"));
          m_lbChartSources->SetItem(cat, 2, path);
        }
      }
      bool covered = false;
      for (size_t i = 0; i < GetChartDBDirArrayString().GetCount(); i++) {
        if (m_plugin->m_ChartSources.at(cat)->GetDir().StartsWith(
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
                m_plugin->m_ChartSources.at(cat)->GetDir().c_str()),
            _("Chart Downloader"));

      m_plugin->SaveConfig();
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

bool chartdldr_pi::ProcessFile(const wxString &aFile,
                               const wxString &aTargetDir, bool aStripPath,
                               wxDateTime aMTime) {
  if (aFile.Lower().EndsWith("zip"))  // Zip compressed
  {
    bool ret = ExtractZipFiles(aFile, aTargetDir, aStripPath, aMTime, false);
    if (ret)
      wxRemoveFile(aFile);
    else
      wxLogError("chartdldr_pi: Unable to extract: " + aFile);
    return ret;
  }
#ifdef DLDR_USE_LIBARCHIVE
  else if (aFile.Lower().EndsWith("rar")) {
#ifdef CHARTDLDR_RAR_UNARR
    bool ret = ExtractUnarrFiles(aFile, aTargetDir, aStripPath, aMTime, false);
#else
    bool ret =
        ExtractLibArchiveFiles(aFile, aTargetDir, aStripPath, aMTime, false);
#endif
    if (ret)
      wxRemoveFile(aFile);
    else
      wxLogError("chartdldr_pi: Unable to extract: " + aFile);
    return ret;
  } else if (aFile.Lower().EndsWith("tar") || aFile.Lower().EndsWith("gz") ||
             aFile.Lower().EndsWith("bz2") || aFile.Lower().EndsWith("lzma") ||
             aFile.Lower().EndsWith("7z") || aFile.Lower().EndsWith("xz")) {
    bool ret =
        ExtractLibArchiveFiles(aFile, aTargetDir, aStripPath, aMTime, false);
    if (ret)
      wxRemoveFile(aFile);
    else
      wxLogError("chartdldr_pi: Unable to extract: " + aFile);
    return ret;
  }
#else
  else if (aFile.Lower().EndsWith("rar") || aFile.Lower().EndsWith("tar")
#ifdef HAVE_BZIP2
           || aFile.Lower().EndsWith("bz2")
#endif
#ifdef HAVE_ZLIB
           || aFile.Lower().EndsWith("gz")
#endif
#ifdef HAVE_7Z
           ||
           aFile.Lower().EndsWith("7z")  // TODO: Could it actually extract more
                                         // formats the LZMA SDK supports?
#endif
  ) {
    bool ret = ExtractUnarrFiles(aFile, aTargetDir, aStripPath, aMTime, false);
    if (ret)
      wxRemoveFile(aFile);
    else
      wxLogError("chartdldr_pi: Unable to extract: " + aFile);
    return ret;
  }
#endif

#ifdef __ANDROID__
  else if (aFile.Lower().EndsWith("tar") || aFile.Lower().EndsWith("gz") ||
           aFile.Lower().EndsWith("bz2") || aFile.Lower().EndsWith("lzma") ||
           aFile.Lower().EndsWith("7z") || aFile.Lower().EndsWith("xz")) {
    int nStrip = 0;
    if (aStripPath) nStrip = 1;

    if (m_dldrpanel) m_dldrpanel->SetChartInfo(_("Installing charts."));

    androidShowBusyIcon();
    bool ret = AndroidUnzip(aFile, aTargetDir, nStrip, true);
    androidHideBusyIcon();

    return ret;
  }
#endif

  else  // Uncompressed
  {
    wxFileName fn(aFile);
    if (fn.GetPath() != aTargetDir)  // We have to move the file somewhere
    {
      if (!wxDirExists(aTargetDir)) {
        if (wxFileName::Mkdir(aTargetDir, 0755, wxPATH_MKDIR_FULL)) {
          if (!wxRenameFile(aFile, aTargetDir)) return false;
        } else
          return false;
      }
    }
    wxString name = fn.GetFullName();
    fn.Clear();
    fn.Assign(aTargetDir, name);
    fn.SetTimes(&aMTime, &aMTime, &aMTime);
  }
  return true;
}

#ifdef DLDR_USE_LIBARCHIVE
#ifndef __ANDROID__
static int copy_data(struct archive *ar, struct archive *aw) {
  int r;
  const void *buff;
  size_t size;
  __LA_INT64_T offset;

  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) return (ARCHIVE_OK);
    if (r < ARCHIVE_OK) return (r);
    r = static_cast<int>(archive_write_data_block(aw, buff, size, offset));
    if (r < ARCHIVE_OK) {
      // fprintf(stderr, "%s\n", archive_error_string(aw));
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(aw)));
      return (r);
    }
  }
}
#endif

bool chartdldr_pi::ExtractLibArchiveFiles(const wxString &aArchiveFile,
                                          const wxString &aTargetDir,
                                          bool aStripPath, wxDateTime aMTime,
                                          bool aRemoveArchive) {
#ifndef __ANDROID__
  struct archive *a = nullptr;
  struct archive *ext = nullptr;
  bool ok = false;

  int flags = ARCHIVE_EXTRACT_TIME;
#ifdef ARCHIVE_EXTRACT_SECURE_NODOTDOT
  flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;
#endif
#ifdef ARCHIVE_EXTRACT_SECURE_SYMLINKS
  flags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;
#endif

  a = archive_read_new();
  ext = archive_write_disk_new();

  if (!a || !ext) {
    wxLogError("Chartdldr_pi: Failed to create libarchive objects.");
    goto cleanup;
  }

  archive_read_support_format_all(a);
  archive_read_support_filter_all(a);
#if !defined(__clang__)
  archive_read_support_compression_all(a);
#endif

  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

#ifdef _WIN32
  if (archive_read_open_filename_w(a, aArchiveFile.wc_str(), 10240) !=
      ARCHIVE_OK) {
    wxLogError(wxString::Format("Chartdldr_pi: LibArchive open error: %s",
                                archive_error_string(a)));
    goto cleanup;
  }
#else
  {
    if (archive_read_open_filename(a, aArchiveFile.mb_str().data(), 10240) !=
        ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive open error: %s",
                                  archive_error_string(a)));
      goto cleanup;
    }
  }
#endif

  for (;;) {
    struct archive_entry *entry = nullptr;
    int r = archive_read_next_header(a, &entry);

    if (r == ARCHIVE_EOF) {
      break;
    }

    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(a)));
    }
    if (r < ARCHIVE_WARN) {
      goto cleanup;
    }

    wxString entryName;
#ifdef _WIN32
    const char *rawUtf8 = archive_entry_pathname_utf8(entry);
    if (rawUtf8 && *rawUtf8) {
      entryName = wxString::FromUTF8(rawUtf8);
    } else {
      const wchar_t *rawWide = archive_entry_pathname_w(entry);
      if (rawWide && *rawWide) entryName = wxString(rawWide);
    }
#else
    const char *rawPath = archive_entry_pathname(entry);
    if (rawPath && *rawPath) {
      entryName = wxString::FromUTF8(rawPath);
      if (entryName.IsEmpty()) {
        entryName = wxString::From8BitData(rawPath);
      }
    }
#endif

    if (entryName.IsEmpty()) {
      wxLogWarning("Skipping archive entry with empty pathname.");
      continue;
    }

    if (aStripPath) {
      wxFileName stripped(entryName);
      entryName = stripped.GetFullName();

      if (entryName.IsEmpty()) {
        continue;
      }
    }

    wxString outputPath = entryName;
    if (aTargetDir.empty()) {
      if (!IsPathInsideDir(aTargetDir, entryName, outputPath)) {
        wxLogWarning("Skipping archive entry with path traversal attempt: " +
                     entryName);
        continue;
      }
    }

#ifdef _WIN32
    archive_entry_copy_pathname_w(entry, outputPath.wc_str());
#else
    archive_entry_copy_pathname(entry, outputPath.fn_str().data());
#endif

    if (aMTime.IsValid()) {
      archive_entry_set_mtime(entry, static_cast<time_t>(aMTime.GetTicks()), 0);
    }

    r = archive_write_header(ext, entry);
    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(ext)));
    }
    if (r < ARCHIVE_WARN) {
      goto cleanup;
    }

    if (archive_entry_size(entry) > 0) {
      r = copy_data(a, ext);
      if (r < ARCHIVE_OK) {
        wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                    archive_error_string(ext)));
      }
      if (r < ARCHIVE_WARN) {
        goto cleanup;
      }
    }

    r = archive_write_finish_entry(ext);
    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(ext)));
    }
    if (r < ARCHIVE_WARN) {
      goto cleanup;
    }
  }

  ok = true;

cleanup:
  if (a) {
    archive_read_close(a);
    archive_read_free(a);
  }
  if (ext) {
    archive_write_close(ext);
    archive_write_free(ext);
  }

  if (ok && aRemoveArchive) wxRemoveFile(aArchiveFile);
  return ok;

#else
  wxUnusedVar(aArchiveFile);
  wxUnusedVar(aTargetDir);
  wxUnusedVar(aStripPath);
  wxUnusedVar(aMTime);
  wxUnusedVar(aRemoveArchive);
  return false;
#endif
}
#endif  // DLDR_USE_LIBARCHIVE

#if defined(CHARTDLDR_RAR_UNARR) || !defined(DLDR_USE_LIBARCHIVE)
ar_archive *ar_open_any_archive(ar_stream *stream, const char *fileext) {
  ar_archive *ar = ar_open_rar_archive(stream);
  if (!ar)
    ar =
        ar_open_zip_archive(stream, fileext && (strcmp(fileext, ".xps") == 0 ||
                                                strcmp(fileext, ".epub") == 0));
  if (!ar) ar = ar_open_7z_archive(stream);
  if (!ar) ar = ar_open_tar_archive(stream);
  return ar;
}

bool chartdldr_pi::ExtractUnarrFiles(const wxString &aRarFile,
                                     const wxString &aTargetDir,
                                     bool aStripPath, wxDateTime aMTime,
                                     bool aRemoveRar) {
  ar_stream *stream = NULL;
  ar_archive *ar = NULL;
  int entry_count = 1;
  int entry_skips = 0;
  int error_step = 1;
  bool ret = true;

  stream = ar_open_file(aRarFile.c_str());
  if (!stream) {
    wxLogError("Can not open file '" + aRarFile + "'.");
    ar_close_archive(ar);
    ar_close(stream);
    return false;
  }
  ar = ar_open_any_archive(stream, strrchr(aRarFile.c_str(), '.'));
  if (!ar) {
    wxLogError("Can not open archive '" + aRarFile + "'.");
    ar_close_archive(ar);
    ar_close(stream);
    return false;
  }
  while (ar_parse_entry(ar)) {
    size_t size = ar_entry_get_size(ar);
    wxString name = ar_entry_get_name(ar);
    wxString originalName = name;  // Save for logging
    if (aStripPath) {
      wxFileName fn(name);
      /* We can completly replace the entry path */
      // fn.SetPath(aTargetDir);
      // name = fn.GetFullPath();
      /* Or only remove the first dir (eg. ENC_ROOT) */
      if (fn.GetDirCount() > 0) {
        fn.RemoveDir(0);
        name = fn.GetFullPath();
      }
    }

    // Path traversal protection: validate path stays inside target directory
    wxString fullPath;
    if (!IsPathInsideDir(aTargetDir, name, fullPath)) {
      wxLogWarning("Skipping archive entry with path traversal attempt: " +
                   originalName);
      continue;
    }
    name = fullPath;

    wxFileName fn(name);
    if (!fn.DirExists()) {
      if (!wxFileName::Mkdir(fn.GetPath())) {
        wxLogError("Can not create directory '" + fn.GetPath() + "'.");
        ret = false;
        break;
      }
    }
    wxFileOutputStream file(name);
    if (!file) {
      wxLogError("Can not create file '" + name + "'.");
      ret = false;
      break;
    }
    while (size > 0) {
      unsigned char buffer[1024];
      size_t count = size < sizeof(buffer) ? size : sizeof(buffer);
      if (!ar_entry_uncompress(ar, buffer, count)) break;
      file.Write(buffer, count);
      size -= count;
    }
    file.Close();
    fn.SetTimes(&aMTime, &aMTime, &aMTime);
    if (size > 0) {
      wxLogError("Warning: Failed to uncompress... skipping");
      entry_skips++;
      ret = false;
    }
  }
  if (!ar_at_eof(ar)) {
    wxLogError("Error: Failed to parse entry %d!", entry_count);
    ret = false;
  }
  ar_close_archive(ar);
  ar_close(stream);

  if (aRemoveRar) wxRemoveFile(aRarFile);

#ifdef _UNIX
  // reset LC_NUMERIC locale, some locales use a comma for decimal point
  // and it corrupts navobj.xml file
  setlocale(LC_NUMERIC, "C");
#endif

  return ret;
}
#endif

bool chartdldr_pi::ExtractZipFiles(const wxString &aZipFile,
                                   const wxString &aTargetDir, bool aStripPath,
                                   wxDateTime aMTime, bool aRemoveZip) {
  bool ret = true;

#ifdef __ANDROID__
  int nStrip = 0;
  if (aStripPath) nStrip = 1;

  ret = AndroidUnzip(aZipFile, aTargetDir, nStrip, true);
#else
  std::unique_ptr<wxZipEntry> entry(new wxZipEntry());

  do {
    wxLogMessage("chartdldr_pi: Going to extract '" + aZipFile + "'.");
    wxFileInputStream in(aZipFile);

    if (!in) {
      wxLogMessage("Can not open file '" + aZipFile + "'.");
      ret = false;
      break;
    }
    wxZipInputStream zip(in);
    ret = false;

    while (entry.reset(zip.GetNextEntry()), entry) {
      // access meta-data
      wxString name = entry->GetName();
      wxString fullPath;
      if (aStripPath) {
        wxFileName fn(name);
        /* We can completly replace the entry path */
        // fn.SetPath(aTargetDir);
        // name = fn.GetFullPath();
        /* Or only remove the first dir (eg. ENC_ROOT) */
        if (fn.GetDirCount() > 0) fn.RemoveDir(0);
        name = fn.GetFullPath();
      }

      // Path traversal protection: validate path stays inside target directory
      if (!IsPathInsideDir(aTargetDir, name, fullPath)) {
        wxLogWarning("Skipping zip entry with path traversal attempt: " +
                     entry->GetName());
        continue;
      }
      name = fullPath;

      // read 'zip' to access the entry's data
      if (entry->IsDir()) {
        int perm = entry->GetMode();
        if (!wxFileName::Mkdir(name, perm, wxPATH_MKDIR_FULL)) {
          wxLogMessage("Can not create directory '" + name + "'.");
          ret = false;
          break;
        }
      } else {
        if (!zip.OpenEntry(*entry)) {
          wxLogMessage("Can not open zip entry '" + entry->GetName() + "'.");
          ret = false;
          break;
        }
        if (!zip.CanRead()) {
          wxLogMessage("Can not read zip entry '" + entry->GetName() + "'.");
          ret = false;
          break;
        }

        wxFileName fn(name);
        if (!fn.DirExists()) {
          if (!wxFileName::Mkdir(fn.GetPath())) {
            wxLogMessage("Can not create directory '" + fn.GetPath() + "'.");
            ret = false;
            break;
          }
        }

        wxFileOutputStream file(name);

        if (!file) {
          wxLogMessage("Can not create file '" + name + "'.");
          ret = false;
          break;
        }
        zip.Read(file);
        fn.SetTimes(&aMTime, &aMTime, &aMTime);
        ret = true;
      }
    }

  } while (false);

  if (aRemoveZip) wxRemoveFile(aZipFile);
#endif  //  __ANDROID__

  return ret;
}

ChartDldrGuiAddSourceDlg::ChartDldrGuiAddSourceDlg(wxWindow *parent)
    : AddSourceDlg(parent) {
  wxFileName fn;
  fn.SetPath(*GetpSharedDataLocation());
  fn.AppendDir("plugins");
  fn.AppendDir("chartdldr_pi");
  fn.AppendDir("data");

  int w = 16;  // default for desktop
  int h = 16;

#ifdef __ANDROID__
  w = 6 * g_androidDPmm;  // mm nominal size
  h = w;

  p_buttonIconList = new wxImageList(w, h);

  fn.SetFullName("button_right.png");
  wxImage im1(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  im1.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_buttonIconList->Add(im1);

  fn.SetFullName("button_right.png");
  wxImage im2(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  im2.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_buttonIconList->Add(im2);

  fn.SetFullName("button_down.png");
  wxImage im3(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  im3.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_buttonIconList->Add(im3);

  fn.SetFullName("button_down.png");
  wxImage im4(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  im4.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_buttonIconList->Add(im4);

  m_treeCtrlPredefSrcs->AssignButtonsImageList(p_buttonIconList);
#else
  p_iconList = new wxImageList(w, h);

  fn.SetFullName("folder.png");
  wxImage ima(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  ima.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_iconList->Add(ima);

  fn.SetFullName("file.png");
  wxImage imb(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  imb.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_iconList->Add(imb);

  m_treeCtrlPredefSrcs->AssignImageList(p_iconList);
#endif /* __ANDROID__ */

  m_treeCtrlPredefSrcs->SetIndent(w);

  m_base_path = "";
  m_last_path = "";
  LoadSources();
  m_nbChoice->SetSelection(0);
  // m_treeCtrlPredefSrcs->ExpandAll();

  wxWindow::Fit();

  applyStyle();
}

bool ChartDldrGuiAddSourceDlg::LoadSources() {
  wxTreeItemId tree = m_treeCtrlPredefSrcs->AddRoot("root");

  wxFileName fn;
  fn.SetPath(*GetpPrivateApplicationDataLocation());
  fn.SetFullName("chartdldr_pi-chart_sources.xml");
  if (!fn.FileExists()) {
    fn.SetPath(*GetpSharedDataLocation());
    fn.AppendDir("plugins");
    fn.AppendDir("chartdldr_pi");
    fn.AppendDir("data");
    fn.SetFullName("chart_sources.xml");
    if (!fn.FileExists()) {
      wxLogMessage(
          wxString::Format("Error: chartdldr_pi::LoadSources() %s not found!",
                           fn.GetFullPath().c_str()));
      return false;
    }
  }
  wxString path = fn.GetFullPath();

  auto *doc = new pugi::xml_document;
  bool ret = doc->load_file(path.mb_str());
  if (ret) {
    pugi::xml_node root = doc->first_child();

    for (pugi::xml_node element = root.first_child(); element;
         element = element.next_sibling()) {
      if (!strcmp(element.name(), "sections")) {
        LoadSections(tree, element);
      }
    }
  }
  wxDELETE(doc);
  return true;
}

bool ChartDldrGuiAddSourceDlg::LoadSections(const wxTreeItemId &root,
                                            pugi::xml_node &node) {
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "section")) {
      LoadSection(root, element);
    }
  }
  return true;
}

bool ChartDldrGuiAddSourceDlg::LoadSection(const wxTreeItemId &root,
                                           pugi::xml_node &node) {
  wxTreeItemId item;
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "name")) {
      item = m_treeCtrlPredefSrcs->AppendItem(
          root, wxString::FromUTF8(element.first_child().value()), 0, 0);

      wxFont *pFont = OCPNGetFont(_("Dialog"));
      if (pFont) m_treeCtrlPredefSrcs->SetItemFont(item, *pFont);
    }
    if (!strcmp(element.name(), "sections")) LoadSections(item, element);
    if (!strcmp(element.name(), "catalogs")) LoadCatalogs(item, element);
  }

  return true;
}

bool ChartDldrGuiAddSourceDlg::LoadCatalogs(const wxTreeItemId &root,
                                            pugi::xml_node &node) {
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "catalog")) LoadCatalog(root, element);
  }

  return true;
}

bool ChartDldrGuiAddSourceDlg::LoadCatalog(const wxTreeItemId &root,
                                           pugi::xml_node &node) {
  wxString name, location, dir;
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "name"))
      name = wxString::FromUTF8(element.first_child().value());
    else if (!strcmp(element.name(), "location"))
      location = wxString::FromUTF8(element.first_child().value());
    else if (!strcmp(element.name(), "dir"))
      dir = wxString::FromUTF8(element.first_child().value());
  }
  auto *cs = new ChartSource(name, location, dir);
  wxTreeItemId id = m_treeCtrlPredefSrcs->AppendItem(root, name, 1, 1, cs);

  wxFont *pFont = OCPNGetFont(_("Dialog"));
  if (pFont) m_treeCtrlPredefSrcs->SetItemFont(id, *pFont);

  return true;
}

ChartDldrGuiAddSourceDlg::~ChartDldrGuiAddSourceDlg() = default;

wxString ChartDldrGuiAddSourceDlg::FixPath(const wxString &path) {
  wxString sep(wxFileName::GetPathSeparator());
  wxString s = path;
  s.Replace("/", sep, true);
  s.Replace(USERDATA, m_base_path);
  s.Replace(sep + sep, sep);
  return s;
}

void ChartDldrGuiAddSourceDlg::OnChangeType(wxCommandEvent &event) {
  m_treeCtrlPredefSrcs->Enable(m_nbChoice->GetSelection() == 0);
  m_tSourceName->Enable(m_nbChoice->GetSelection() == 1);
  m_tChartSourceUrl->Enable(m_nbChoice->GetSelection() == 1);
}

void ChartDldrGuiAddSourceDlg::OnSourceSelected(wxTreeEvent &event) {
  wxTreeItemId item = m_treeCtrlPredefSrcs->GetSelection();
  auto *cs = (ChartSource *)(m_treeCtrlPredefSrcs->GetItemData(item));
  if (cs) {
    m_dirExpanded = FixPath(cs->GetDir());

    m_tSourceName->SetValue(cs->GetName());
    m_tChartSourceUrl->SetValue(cs->GetUrl());
    if (m_tcChartDirectory->GetValue() == m_last_path) {
      m_tcChartDirectory->SetValue(FixPath(cs->GetDir()));
      m_panelChartDirectory->SetText(FixPath(cs->GetDir()));

      m_buttonChartDirectory->Enable();
      m_last_path = m_tcChartDirectory->GetValue();
    }
  }
  event.Skip();
}

void ChartDldrGuiAddSourceDlg::SetSourceEdit(std::unique_ptr<ChartSource> &cs) {
  m_nbChoice->SetSelection(1);
  m_tChartSourceUrl->Enable();
  m_treeCtrlPredefSrcs->Disable();
  m_tSourceName->SetValue(cs->GetName());
  m_tChartSourceUrl->SetValue(cs->GetUrl());
  m_tcChartDirectory->SetValue(FixPath(cs->GetDir()));
  m_panelChartDirectory->SetText(FixPath(cs->GetDir()));

  m_buttonChartDirectory->Enable();
}

ChartDldrPrefsDlgImpl::ChartDldrPrefsDlgImpl(wxWindow *parent)
    : ChartDldrPrefsDlg(parent) {}

ChartDldrPrefsDlgImpl::~ChartDldrPrefsDlgImpl() = default;

void ChartDldrPrefsDlgImpl::SetPath(const wxString &path) {
  // if( !wxDirExists(path) )
  // if( !wxFileName::Mkdir(path, 0755, wxPATH_MKDIR_FULL) )
  //{
  //    OCPNMessageBox_PlugIn(this, wxString::Format(_("Directory %s can't be
  //    created."), m_dpDefaultDir->GetTextCtrlValue().c_str()), _("Chart
  //    Downloader")); return;
  //}
  m_tcDefaultDir->SetValue(path);
}

void ChartDldrPrefsDlgImpl::GetPreferences(bool &preselect_new,
                                           bool &preselect_updated,
                                           bool &bulk_update) {
  preselect_new = m_cbSelectNew->GetValue();
  preselect_updated = m_cbSelectUpdated->GetValue();
  bulk_update = m_cbBulkUpdate->GetValue();
}
void ChartDldrPrefsDlgImpl::SetPreferences(bool preselect_new,
                                           bool preselect_updated,
                                           bool bulk_update) {
  m_cbSelectNew->SetValue(preselect_new);
  m_cbSelectUpdated->SetValue(preselect_updated);
  m_cbBulkUpdate->SetValue(bulk_update);
}

void ChartDldrGuiAddSourceDlg::OnOkClick(wxCommandEvent &event) {
  wxString msg = "";

  if (m_nbChoice->GetSelection() == 0) {
    wxTreeItemId item = m_treeCtrlPredefSrcs->GetSelection();
    if (m_treeCtrlPredefSrcs->GetSelection().IsOk()) {
      auto *cs = (ChartSource *)(m_treeCtrlPredefSrcs->GetItemData(item));
      if (!cs)
        msg +=
            _("You must select one of the predefined chart sources or create "
              "one of your own.\n");
    } else
      msg +=
          _("You must select one of the predefined chart sources or create one "
            "of your own.\n");
  }
  if (m_nbChoice->GetSelection() == 1 && m_tSourceName->GetValue().empty())
    msg += _("The chart source must have a name.\n");
  wxURI url(m_tChartSourceUrl->GetValue());
  if (m_nbChoice->GetSelection() == 1 &&
      (m_tChartSourceUrl->GetValue().empty() ||
       !ValidateUrl(m_tChartSourceUrl->GetValue())))
    msg += _("The chart source must have a valid URL.\n");
  if (m_tcChartDirectory->GetValue().empty())
    msg += _("You must select a local folder to store the charts.\n");
  else if (!wxDirExists(m_tcChartDirectory->GetValue()))
    if (!wxFileName::Mkdir(m_tcChartDirectory->GetValue(), 0755,
                           wxPATH_MKDIR_FULL))
      msg += wxString::Format(_("Directory %s can't be created."),
                              m_tcChartDirectory->GetValue().c_str()) +
             "\n";

  if (!msg.empty())
    OCPNMessageBox_PlugIn(this, msg, _("Chart source definition problem"),
                          wxOK | wxCENTRE | wxICON_ERROR);
  else {
    event.Skip();
    SetReturnCode(wxID_OK);
    EndModal(wxID_OK);
  }
}

void ChartDldrGuiAddSourceDlg::OnCancelClick(wxCommandEvent &event) {
  SetReturnCode(wxID_CANCEL);
  EndModal(wxID_CANCEL);
}

void ChartDldrPrefsDlgImpl::OnOkClick(wxCommandEvent &event) {
  if (!wxDirExists(m_tcDefaultDir->GetValue())) {
    if (!wxFileName::Mkdir(m_tcDefaultDir->GetValue(), 0755,
                           wxPATH_MKDIR_FULL)) {
      OCPNMessageBox_PlugIn(
          this,
          wxString::Format(_("Directory %s can't be created."),
                           m_tcDefaultDir->GetValue().c_str()),
          _("Chart Downloader"));
      return;
    }
  }

  if (g_pi) {
    g_pi->UpdatePrefs(this);
  }

  event.Skip();
  EndModal(wxID_OK);

  // Hide();
  // Close();
}

void ChartDldrPrefsDlg::OnCancelClick(wxCommandEvent &event) {
  event.Skip();
  EndModal(wxID_CANCEL);
  // Close();
}

void ChartDldrPrefsDlg::OnOkClick(wxCommandEvent &event) {
  event.Skip();
  // Close();
}

bool ChartDldrGuiAddSourceDlg::ValidateUrl(const wxString &Url,
                                           bool catalog_xml) {
  wxRegEx re;
  if (catalog_xml)
    re.Compile(
        "^https?\\://[a-zA-Z0-9\\./_-]*\\.[xX][mM][lL]$");  // TODO: wxRegEx
                                                            // sucks a bit,
                                                            // this RE is
                                                            // way too naive
  else
    re.Compile(
        "^https?\\://[a-zA-Z0-9\\./_-]*$");  // TODO: wxRegEx sucks a bit,
                                             // this RE is way too naive
  return re.Matches(Url);
}

void ChartDldrPanelImpl::onDLEvent(OCPN_downloadEvent &ev) {
  //    wxString msg;
  //    msg.Printf("onDLEvent  %d %d",ev.getDLEventCondition(),
  //    ev.getDLEventStatus()); wxLogMessage(msg);

  switch (ev.getDLEventCondition()) {
    case OCPN_DL_EVENT_TYPE_END:
      m_is_transfer_complete = true;
      m_is_transfer_ok =
          (ev.getDLEventStatus() == OCPN_DL_NO_ERROR) ? true : false;
      break;

    case OCPN_DL_EVENT_TYPE_PROGRESS:
      if (ev.getTransferred() > m_transferred_size) {
        m_total_size = ev.getTotal();
        m_transferred_size = ev.getTransferred();
      }

      break;
    default:
      break;
  }
  wxYieldIfNeeded();
}
