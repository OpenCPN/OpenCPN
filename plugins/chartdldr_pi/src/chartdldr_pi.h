/******************************************************************************
 * $Id: chartdldr_pi.h,v 1.0 2011/02/26 01:54:37 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Chart Downloader Plugin
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Pavel Kalian                                    *
 *   $EMAIL$                                                               *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _CHARTDLDRPI_H_
#define _CHARTDLDRPI_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <wx/event.h>

#include <wx/imaglist.h>

#include <map>

#include "version.h"

#define MY_API_VERSION_MAJOR 1
#define MY_API_VERSION_MINOR 13

#define USERDATA "{USERDATA}"

#include "ocpn_plugin.h"

#include "chartdldrgui.h"
#include "chartcatalog.h"
#include "chartdldr_bulk.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_schedule_config.h"
#include "chartdldr_chart_source.h"

// forward declarations
class ChartDldrPanelImpl;
class ChartDldrGuiAddSourceDlg;
class ChartDldrPrefsDlgImpl;

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

class chartdldr_pi : public opencpn_plugin_113 {
public:
  chartdldr_pi(void* ppimgr);

  //    The required PlugIn Methods
  int Init(void);
  bool DeInit(void);

  int GetAPIVersionMajor();
  int GetAPIVersionMinor();
  int GetPlugInVersionMajor();
  int GetPlugInVersionMinor();
  wxBitmap* GetPlugInBitmap();
  wxString GetCommonName();
  wxString GetShortDescription();
  wxString GetLongDescription();

  void OnSetupOptions(void);
  void OnCloseToolboxPanel(int page_sel, int ok_apply_cancel);

  //    The required override PlugIn Methods
  void ShowPreferencesDialog(wxWindow* parent);

  //    Other public methods
  bool SaveConfig(void);
  /** Persist schedule bookkeeping only; does not rewrite ChartSources. */
  bool SaveScheduleConfig(const ChartDldrScheduleConfig& schedule);
  bool ProcessFile(const wxString& aFile, const wxString& aTargetDir,
                   bool aStripPath = true,
                   wxDateTime aMTime = wxDateTime::Now(),
                   bool verbose_extract_log = true);
  bool ExtractZipFiles(const wxString& aZipFile, const wxString& aTargetDir,
                       bool aStripPath = true,
                       wxDateTime aMTime = wxDateTime::Now(),
                       bool aRemoveZip = false,
                       bool verbose_extract_log = true);
#ifdef DLDR_USE_LIBARCHIVE
  bool ExtractLibArchiveFiles(const wxString& aArchiveFile,
                              const wxString& aTargetDir,
                              bool aStripPath = true,
                              wxDateTime aMTime = wxDateTime::Now(),
                              bool aRemoveArchive = false,
                              bool verbose_extract_log = true);
#endif
#if defined(CHARTDLDR_RAR_UNARR) || !defined(DLDR_USE_LIBARCHIVE)
  bool ExtractUnarrFiles(const wxString& aRarFile, const wxString& aTargetDir,
                         bool aStripPath = true,
                         wxDateTime aMTime = wxDateTime::Now(),
                         bool aRemoveRar = false,
                         bool verbose_extract_log = true);
#endif

  void UpdatePrefs(ChartDldrPrefsDlgImpl* dialog);

  bool EnsureDownloaderPanel();
  void AttachDownloaderPanelToOptions(wxScrolledWindow* page);
  bool RequestManualScheduledBulkUpdate();
  bool ConfirmInteractiveBulkUpdate(wxWindow* parent) const;
  bool IsScheduledBulkRunActive() const;
  void ApplyChartDatabaseUpdate();
  bool HasChartSources() const { return !m_ChartSources.empty(); }
  ChartDldrBulkRequestInput MakeBulkRequestInput() const;

  ChartDldrScheduleConfig& ScheduleConfig() { return m_schedule; }
  const ChartDldrScheduleConfig& ScheduleConfig() const { return m_schedule; }
  ChartDldrPanelImpl* GetDownloaderPanel() { return m_dldrpanel; }
  ChartDldrScheduler& Scheduler() { return m_scheduler; }
  const ChartDldrScheduler& Scheduler() const { return m_scheduler; }

  //    Public properties
  std::vector<std::unique_ptr<ChartSource>> m_ChartSources;
  wxWindow* m_parent_window;
  ChartCatalog m_pChartCatalog;
  ChartSource* m_pChartSource;
  void SetSourceId(int id) { m_selected_source = id; }
  int GetSourceId() { return m_selected_source; }
  wxString GetBaseChartDir() { return m_base_chart_dir; }
  bool m_preselect_new;
  bool m_preselect_updated;
  bool m_allow_bulk_update;

  wxFileConfig* m_pconfig;
  wxScrolledWindow* m_pOptionsPage;
  bool LoadConfig(void);

  wxString m_schartdldr_sources;
  int m_selected_source;

  ChartDldrPanelImpl* m_dldrpanel;
  wxPanel* m_panel_host;
  wxString m_base_chart_dir;

  ChartDldrScheduleConfig m_schedule;
  ChartDldrScheduler m_scheduler;
};

class ChartDldrGuiAddSourceDlg : public AddSourceDlg {
protected:
  void OnChangeType(wxNotebookEvent& event);
  void OnSourceSelected(wxTreeEvent& event) override;
  void OnOkClick(wxCommandEvent& event) override;
  void OnCancelClick(wxCommandEvent& event) override;

  bool LoadSources();
  bool LoadSections(const wxTreeItemId& root, pugi::xml_node& node);
  bool LoadSection(const wxTreeItemId& root, pugi::xml_node& node);
  bool LoadCatalogs(const wxTreeItemId& root, pugi::xml_node& node);
  bool LoadCatalog(const wxTreeItemId& root, pugi::xml_node& node);

public:
  ChartDldrGuiAddSourceDlg(wxWindow* parent);
  ~ChartDldrGuiAddSourceDlg();
  void SetBasePath(const wxString path) { m_base_path = path; }
  void SetSourceEdit(std::unique_ptr<ChartSource>& cs);

private:
  bool ValidateUrl(const wxString Url, bool catalog_xml = true);
  wxString FixPath(wxString path);
  wxString m_base_path;
  wxString m_last_path;
  wxImageList* p_iconList;
#ifdef __OCPN__ANDROID__
  wxImageList* p_buttonIconList;
#endif /* __OCPN__ANDROID__ */
};

#endif
