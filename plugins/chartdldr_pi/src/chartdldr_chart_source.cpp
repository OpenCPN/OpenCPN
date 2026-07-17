/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_chart_source.h"

#include <fstream>
#include <string>

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/tokenzr.h>

#ifdef __ANDROID__
#include "androidSupport.h"
#endif

namespace {

/** Lowercased filename stem before the first '.' — sole key into m_update_data.
 */
std::string FilenameStemUpdateKey(const wxString& filename) {
  wxStringTokenizer tk(filename, _T("."));
  return std::string(tk.GetNextToken().MakeLower().mb_str());
}

bool HasStamp(const std::map<std::string, time_t>& stamps,
              const std::string& key) {
  return !key.empty() && stamps.find(key) != stamps.end();
}

wxString ChartUpdateDataPath(const wxString& chart_dir) {
  return chart_dir + wxFileName::GetPathSeparator() + _T(UPDATE_DATA_FILENAME);
}

}  // namespace

ChartSource::ChartSource(wxString name, wxString url, wxString localdir) {
  m_name = name;
  m_url = url;
  m_dir = localdir;
  m_update_data.clear();
}

ChartSource::~ChartSource() { m_update_data.clear(); }

bool ChartSource::ExistsLocaly(wxString filename) const {
  wxASSERT(this);
  // Presence is ownership of m_update_data only: GetLocalFiles rebuilds it from
  // disk, ChartUpdated adds stamps for installs. Filename stem is the only key.
  return HasStamp(m_update_data, FilenameStemUpdateKey(filename));
}

bool ChartSource::IsNewerThanLocal(wxString filename,
                                   wxDateTime validDate) const {
  const auto file_it = m_update_data.find(FilenameStemUpdateKey(filename));
  const time_t local_ticks =
      file_it != m_update_data.end() ? file_it->second : 0;
  return local_ticks < validDate.GetTicks();
}

bool ChartSource::GetLocalFiles() {
  // Preserve install stamps across the scan when already loaded, or pull them
  // from disk once when the map is still empty.
  if (UpdateDataExists() && m_update_data.empty()) {
    LoadUpdateData();
  }
  const std::map<std::string, time_t> previous_stamps = m_update_data;

  wxArrayString allFiles;
  if (wxDirExists(GetDir())) {
    wxDir::GetAllFiles(GetDir(), &allFiles);
  }

  m_update_data.clear();

  const wxString update_data_name(_T(UPDATE_DATA_FILENAME));
  for (size_t i = 0; i < allFiles.Count(); i++) {
    wxFileName fn(allFiles.Item(i));
    if (fn.GetFullName() == update_data_name) {
      continue;
    }

    const std::string key = FilenameStemUpdateKey(fn.GetFullName().Lower());
    if (key.empty() || m_update_data.find(key) != m_update_data.end()) {
      continue;
    }

    wxDateTime ct, mt, at;
    fn.GetTimes(&at, &mt, &ct);
    const auto prev = previous_stamps.find(key);
    m_update_data[key] =
        prev != previous_stamps.end() ? prev->second : mt.GetTicks();
  }

  return SaveUpdateData();
}

bool ChartSource::UpdateDataExists() const {
  return wxFileExists(ChartUpdateDataPath(GetDir()));
}

void ChartSource::LoadUpdateData() {
  m_update_data.clear();
  const wxString fn = ChartUpdateDataPath(GetDir());

  if (!wxFileExists(fn)) return;

  std::ifstream infile(fn.mb_str());

  std::string key;
  long value;

  while (infile >> key >> value) m_update_data[key] = value;

  infile.close();
}

bool ChartSource::SaveUpdateData() {
  const wxString published = ChartUpdateDataPath(GetDir());
  wxString write_path = published;

#ifdef __ANDROID__
  write_path = AndroidGetCacheDir() + wxFileName::GetPathSeparator() +
               _T(UPDATE_DATA_FILENAME);
#else
  write_path = published + wxT(".publish-tmp");
#endif

  std::ofstream outfile(write_path.mb_str());
  if (!outfile.is_open()) {
    wxLogWarning(
        wxT("chartdldr_pi: failed to write chart update metadata \"%s\""),
        write_path.c_str());
    return false;
  }

  for (std::map<std::string, time_t>::const_iterator iter =
           m_update_data.begin();
       iter != m_update_data.end(); ++iter) {
    if (iter->first.find(" ") == std::string::npos && !iter->first.empty()) {
      outfile << iter->first << " " << iter->second << "\n";
    }
  }

  outfile.flush();
  const bool write_ok = static_cast<bool>(outfile);
  outfile.close();
  if (!write_ok) {
    wxLogWarning(
        wxT("chartdldr_pi: failed to write chart update metadata \"%s\""),
        write_path.c_str());
    wxRemoveFile(write_path);
    return false;
  }

#ifdef __ANDROID__
  if (!AndroidSecureCopyFile(write_path, published)) {
    wxLogWarning(
        wxT("chartdldr_pi: failed to publish chart update metadata \"%s\""),
        published.c_str());
    wxRemoveFile(write_path);
    return false;
  }
  wxRemoveFile(write_path);
#else
  if (!wxRenameFile(write_path, published, true /*overwrite*/)) {
    wxLogWarning(
        wxT("chartdldr_pi: failed to publish chart update metadata \"%s\""),
        published.c_str());
    wxRemoveFile(write_path);
    return false;
  }
#endif
  return true;
}

bool ChartSource::ChartUpdated(wxString filename, time_t timestamp) {
  const std::string file = FilenameStemUpdateKey(filename);
  if (file.empty()) {
    return SaveUpdateData();
  }

  const auto previous = m_update_data.find(file);
  const bool had_previous = previous != m_update_data.end();
  const time_t previous_stamp = had_previous ? previous->second : 0;

  m_update_data[file] = timestamp;
  if (SaveUpdateData()) {
    return true;
  }

  if (had_previous) {
    m_update_data[file] = previous_stamp;
  } else {
    m_update_data.erase(file);
  }
  return false;
}
