/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_CHART_SOURCE_H_
#define CHARTDLDR_CHART_SOURCE_H_

#include <map>
#include <string>
#include <vector>

#include <wx/arrstr.h>
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
  bool ExistsLocaly(wxString chart_number, wxString filename) const;
  bool IsNewerThanLocal(wxString chart_number, wxString filename,
                        wxDateTime validDate) const;
  void UpdateLocalFiles() { GetLocalFiles(); }

  bool UpdateDataExists() const;
  void LoadUpdateData();
  void SaveUpdateData();
  void ChartUpdated(wxString chart_number, time_t timestamp);

private:
  wxArrayString m_localfiles;
  std::vector<wxDateTime> m_localdt;
  void GetLocalFiles();
  wxString m_name;
  wxString m_url;
  wxString m_dir;
  std::map<std::string, time_t> m_update_data;
};

#endif  // CHARTDLDR_CHART_SOURCE_H_
