/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_chart_source.h"

#include <fstream>

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>

#ifdef __ANDROID__
#include "androidSupport.h"
#endif

ChartSource::ChartSource(wxString name, wxString url, wxString localdir) {
  m_name = name;
  m_url = url;
  m_dir = localdir;
  m_update_data.clear();
}

ChartSource::~ChartSource() { m_update_data.clear(); }

bool ChartSource::ExistsLocaly(wxString chart_number,
                               wxString filename) const {
  wxASSERT(this);

  wxStringTokenizer tk(filename, _T("."));
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

bool ChartSource::IsNewerThanLocal(wxString chart_number, wxString filename,
                                   wxDateTime validDate) const {
  wxStringTokenizer tk(filename, _T("."));
  wxString file = tk.GetNextToken().MakeLower();
  if (!m_update_data.empty()) {
    const auto chart_it =
        m_update_data.find(std::string(chart_number.Lower().mbc_str()));
    const auto file_it = m_update_data.find(std::string(file.mbc_str()));
    const time_t chart_ticks =
        chart_it != m_update_data.end() ? chart_it->second : 0;
    const time_t file_ticks =
        file_it != m_update_data.end() ? file_it->second : 0;
    if (chart_ticks < validDate.GetTicks() && file_ticks < validDate.GetTicks()) {
      return true;
    }
    return false;
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

void ChartSource::GetLocalFiles() {
  if (!UpdateDataExists() || m_update_data.empty()) {
    wxArrayString* allFiles = new wxArrayString;
    if (wxDirExists(GetDir())) wxDir::GetAllFiles(GetDir(), allFiles);
    m_localdt.clear();
    m_localfiles.Clear();
    wxDateTime ct, mt, at;
    wxString name;
    for (size_t i = 0; i < allFiles->Count(); i++) {
      wxFileName fn(allFiles->Item(i));
      name = fn.GetFullName().Lower();
      if (!ExistsLocaly(wxEmptyString, name)) {
        fn.GetTimes(&at, &mt, &ct);
        m_localdt.push_back(mt);
        m_localfiles.Add(fn.GetName().Lower());

        wxStringTokenizer tk(name, _T("."));
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

bool ChartSource::UpdateDataExists() const {
  return wxFileExists(GetDir() + wxFileName::GetPathSeparator() +
                      _T(UPDATE_DATA_FILENAME));
}

void ChartSource::LoadUpdateData() {
  m_update_data.clear();
  wxString fn =
      GetDir() + wxFileName::GetPathSeparator() + _T(UPDATE_DATA_FILENAME);

  if (!wxFileExists(fn)) return;

  std::ifstream infile(fn.mb_str());

  std::string key;
  long value;

  while (infile >> key >> value) m_update_data[key] = value;

  infile.close();
}

void ChartSource::SaveUpdateData() {
  wxString fn;
  fn = GetDir() + wxFileName::GetPathSeparator() + _T(UPDATE_DATA_FILENAME);

#ifdef __ANDROID__
  fn = AndroidGetCacheDir() + wxFileName::GetPathSeparator() +
       _T(UPDATE_DATA_FILENAME);
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
      fn, GetDir() + wxFileName::GetPathSeparator() + _T(UPDATE_DATA_FILENAME));
#endif
}

void ChartSource::ChartUpdated(wxString chart_number, time_t timestamp) {
  m_update_data[std::string(chart_number.Lower().mb_str())] = timestamp;
  SaveUpdateData();
}
