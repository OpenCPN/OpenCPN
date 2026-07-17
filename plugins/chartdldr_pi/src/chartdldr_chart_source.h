/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_CHART_SOURCE_H_
#define CHARTDLDR_CHART_SOURCE_H_

#include <map>
#include <string>

#include <wx/datetime.h>
#include <wx/string.h>
#include <wx/treectrl.h>

#define UPDATE_DATA_FILENAME "chartdldr_pi.dat"

class ChartSource : public wxTreeItemData {
public:
  ChartSource(wxString name, wxString url, wxString localdir);
  ~ChartSource();

  wxString GetName() const { return m_name; }
  wxString GetUrl() const { return m_url; }
  wxString GetDir() const { return m_dir; }
  void SetDir(wxString dir) { m_dir = dir; }
  void SetName(wxString name) { m_name = name; }
  void SetUrl(wxString url) { m_url = url; }
  bool ExistsLocaly(wxString filename) const;
  bool IsNewerThanLocal(wxString filename, wxDateTime validDate) const;
  bool UpdateLocalFiles() { return GetLocalFiles(); }

  bool UpdateDataExists() const;
  void LoadUpdateData();
  /** Persist m_update_data to the chart dir (Android: cache write + secure
   * publish). */
  bool SaveUpdateData();
  /**
   * Stamp by filename stem (same key GetLocalFiles reconciles). On persist
   * failure the in-memory map is left unchanged for that key.
   */
  bool ChartUpdated(wxString filename, time_t timestamp);

private:
  bool GetLocalFiles();
  wxString m_name;
  wxString m_url;
  wxString m_dir;
  /** Disk-reconciled install stamps keyed by filename stem. */
  std::map<std::string, time_t> m_update_data;
};

#endif  // CHARTDLDR_CHART_SOURCE_H_
