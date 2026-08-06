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
 * Chart Downloader Plugin -- plugin implementation header
 */

#ifndef ChartDLdrpI_H_
#define ChartDLdrpI_H_

#include <map>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/event.h>
#include <wx/fileconf.h>
#include <wx/tokenzr.h>

#include "chartcatalog.h"
#include "chartdldrgui.h"
#include "ocpn_plugin.h"

#define MY_API_VERSION_MAJOR 1
#define MY_API_VERSION_MINOR 13
#define USERDATA "{USERDATA}"
#define UPDATE_DATA_FILENAME "chartdldr_pi.dat"

// forward declarations
class ChartSource;               // forward
class ChartDldrPanelImpl;        // in chartdldr.h
class ChartDldrGuiAddSourceDlg;  // in chartdldr.h
class ChartDldrPrefsDlgImpl;     // in chartdldr.h

/**
 *    The PlugIn Class Definition
 */
class chartdldr_pi : public opencpn_plugin_113 {
public:
  explicit chartdldr_pi(void* ppimgr);

  //    The required PlugIn Methods
  int Init() override;
  bool DeInit() override;

  //    The required override PlugIn Methods
  int GetAPIVersionMajor() override;
  int GetAPIVersionMinor() override;
  int GetPlugInVersionMajor() override;
  int GetPlugInVersionMinor() override;
  wxBitmap* GetPlugInBitmap() override;
  wxString GetCommonName() override;
  wxString GetShortDescription() override;
  wxString GetLongDescription() override;

  //    Other public methods
  void OnSetupOptions() override;
  void OnCloseToolboxPanel(int page_sel, int ok_apply_cancel) override;

  void ShowPreferencesDialog(wxWindow* parent) override;

  bool SaveConfig();
  bool ProcessFile(const wxString& aFile, const wxString& aTargetDir,
                   bool aStripPath = true,
                   wxDateTime aMTime = wxDateTime::Now());
  bool ExtractZipFiles(const wxString& aZipFile, const wxString& aTargetDir,
                       bool aStripPath = true,
                       wxDateTime aMTime = wxDateTime::Now(),
                       bool aRemoveZip = false);
#ifdef DLDR_USE_LIBARCHIVE
  bool ExtractLibArchiveFiles(const wxString& aArchiveFile,
                              const wxString& aTargetDir,
                              bool aStripPath = true,
                              wxDateTime aMTime = wxDateTime::Now(),
                              bool aRemoveArchive = false);
#endif
#if defined(CHARTDLDR_RAR_UNARR) || !defined(DLDR_USE_LIBARCHIVE)
  bool ExtractUnarrFiles(const wxString& aRarFile, const wxString& aTargetDir,
                         bool aStripPath = true,
                         wxDateTime aMTime = wxDateTime::Now(),
                         bool aRemoveRar = false);
#endif

  void UpdatePrefs(ChartDldrPrefsDlgImpl* dialog);

  //    Public properties
  std::vector<std::unique_ptr<ChartSource>> m_ChartSources;
  wxWindow* m_parent_window;
  ChartCatalog m_chart_catalog;
  ChartSource* m_chart_source;
  void SetSourceId(int id) { m_selected_source = id; }
  [[nodiscard]] int GetSourceId() const { return m_selected_source; }
  wxString GetBaseChartDir() { return m_base_chart_dir; }
  bool m_reselect_new;
  bool m_reselect_updated;
  bool m_allow_bulk_update;

private:
  wxFileConfig* m_config;
  wxScrolledWindow* m_options_page;
  wxString m_schartdldr_sources;
  int m_selected_source;
  ChartDldrPanelImpl* m_dldrpanel;
  wxString m_base_chart_dir;

  bool LoadConfig();
};

class ChartSource : public wxTreeItemData {
public:
  ChartSource(const wxString& name, const wxString& url,
              const wxString& localdir);
  ~ChartSource() override;

  wxString GetName() { return m_name; }
  wxString GetUrl() { return m_url; }
  wxString GetDir() { return m_dir; }
  void SetDir(const wxString& dir) { m_dir = dir; }
  void SetName(const wxString& name) { m_name = name; }
  void SetUrl(const wxString& url) { m_url = url; }
  bool ExistsLocally(const wxString& chart_number, const wxString& filename);
  bool IsNewerThanLocal(const wxString& chart_number, const wxString& filename,
                        const wxDateTime& validDate);
  void UpdateLocalFiles() { GetLocalFiles(); }

  bool UpdateDataExists();
  void LoadUpdateData();
  void SaveUpdateData();
  void ChartUpdated(const wxString& chart_number, time_t timestamp);

private:
  wxArrayString m_localfiles;
  std::vector<wxDateTime> m_localdt;
  wxString m_name;
  wxString m_url;
  wxString m_dir;
  std::map<std::string, time_t> m_update_data;

  void GetLocalFiles();
};

/** Implementing ChartDldrPanel */
class ChartDldrPanelImpl : public ChartDldrPanel {
  friend class chartdldr_pi;

private:
  int m_to_download;
  int m_updating_all;
  bool m_cancelled;
  bool m_download_is_cancel;
  chartdldr_pi* m_plugin;
  bool m_is_populated;
  bool m_is_transfer_complete;
  bool m_is_transfer_ok;
  long m_total_size;
  long m_transferred_size;
  int m_failed_downloads;
  bool m_is_connected;
  bool m_hold_info;  // Don't update chart selection stats right now
  size_t m_new_charts;
  size_t m_updated_charts;
  int m_downloading;

  bool DownloadChart(const wxString& url, const wxString& file,
                     const wxString& title);
  void OnPopupClick(wxCommandEvent& evt);
  int GetSelectedCatalog();
  void AppendCatalog(std::unique_ptr<ChartSource>& cs);
  void DoEditSource();

  void DisableForDownload(bool enabled);

protected:
  // Handlers for ChartDldrPanel events.
  void SetSource(int id);
  void SelectSource(wxListEvent& event) override;
  void AddSource(wxCommandEvent& event) override;
  void DeleteSource(wxCommandEvent& event) override;
  void EditSource(wxCommandEvent& event) override;
  void UpdateChartList(wxCommandEvent& event) override;
  void OnDownloadCharts(wxCommandEvent& event) override;

#if defined(CHART_LIST)
  void OnSelectChartItem(wxCommandEvent& event);
  void OnSelectNewCharts(wxCommandEvent& event);
  void OnSelectUpdatedCharts(wxCommandEvent& event);
  void OnSelectAllCharts(wxCommandEvent& event);
#endif

  void DownloadCharts();
  void DoHelp(wxCommandEvent& event) override {
#ifdef __WXMSW__
    wxLaunchDefaultBrowser("file:///" + *GetpSharedDataLocation() +
                           "plugins/chartdldr_pi/data/doc/index.html");
#else
    wxLaunchDefaultBrowser("file://" + *GetpSharedDataLocation() +
                           "plugins/chartdldr_pi/data/doc/index.html");
#endif
  }
  void UpdateAllCharts(wxCommandEvent& event) override;
  void OnShowLocalDir(wxCommandEvent& event) override;
  void OnPaint(wxPaintEvent& event) override;
  void OnLeftDClick(wxMouseEvent& event) override;

  void CleanForm();
  void FillFromFile(const wxString& url, const wxString& dir,
                    bool selnew = false, bool selupd = false);

  void SetBulkUpdate(bool bulk_update);

  int GetChartCount();
  int GetCheckedChartCount();
  bool isChartChecked(int i);
  void CheckAllCharts(bool value);
  void InvertCheckAllCharts();

  void CheckNewCharts(bool value);
  void CheckUpdatedCharts(bool value);

public:
  ~ChartDldrPanelImpl() override;
  explicit ChartDldrPanelImpl(chartdldr_pi* plugin = nullptr,
                              wxWindow* parent = nullptr,
                              wxWindowID id = wxID_ANY,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxDEFAULT_DIALOG_STYLE |
                                           wxRESIZE_BORDER);
  void SelectCatalog(int item);
  void onDLEvent(OCPN_downloadEvent& ev);
  void CancelDownload() {
    Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent);
    m_cancelled = true;
    m_is_connected = false;
  }
  void OnContextMenu(wxMouseEvent& event) override;

private:
  DECLARE_EVENT_TABLE()
};

class ChartDldrGuiAddSourceDlg : public AddSourceDlg {
protected:
  void OnChangeType(wxCommandEvent& event);
  void OnSourceSelected(wxTreeEvent& event) override;
  void OnOkClick(wxCommandEvent& event) override;
  void OnCancelClick(wxCommandEvent& event) override;

  bool LoadSources();
  bool LoadSections(const wxTreeItemId& root, pugi::xml_node& node);
  bool LoadSection(const wxTreeItemId& root, pugi::xml_node& node);
  bool LoadCatalogs(const wxTreeItemId& root, pugi::xml_node& node);
  bool LoadCatalog(const wxTreeItemId& root, pugi::xml_node& node);

public:
  explicit ChartDldrGuiAddSourceDlg(wxWindow* parent);
  ~ChartDldrGuiAddSourceDlg() override;
  void SetBasePath(const wxString& path) { m_base_path = path; }
  void SetSourceEdit(std::unique_ptr<ChartSource>& cs);

private:
  wxString m_base_path;
  wxString m_last_path;
  wxImageList* p_iconList;
#ifdef __ANDROID__
  wxImageList* p_buttonIconList;
#endif /* __ANDROID__ */

  bool ValidateUrl(const wxString& Url, bool catalog_xml = true);
  wxString FixPath(const wxString& path);
};

class ChartDldrPrefsDlgImpl : public ChartDldrPrefsDlg {
protected:
  void OnOkClick(wxCommandEvent& event) override;

public:
  explicit ChartDldrPrefsDlgImpl(wxWindow* parent);
  ~ChartDldrPrefsDlgImpl() override;
  [[nodiscard]] wxString GetPath() const { return m_tcDefaultDir->GetValue(); }
  void SetPath(const wxString& path);
  void GetPreferences(bool& preselect_new, bool& preselect_updated,
                      bool& bulk_update);
  void SetPreferences(bool preselect_new, bool preselect_updated,
                      bool bulk_update);
};

#endif  // ChartDLdrpI_H_
