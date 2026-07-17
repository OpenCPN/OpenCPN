/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_extract_common.h"

#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef __UNIX__
#include <climits>
#include <cstdlib>
#endif

namespace ChartDldrExtractCommon {

const wxChar* kRollbackDirName = _T("__chartdldr_rollback__");
const wxChar* kBackupJournalName = _T("__chartdldr_backup_journal__");
const wxChar* kManifestName = _T("__chartdldr_manifest__");
const wxChar* kExtractDirPrefix = _T(".chartdldr-extract-");

const wxChar* kPhaseStaging = _T("STAGING");
const wxChar* kPhasePublishing = _T("PUBLISHING");
const wxChar* kPhaseCommitted = _T("COMMITTED");

bool IsPathInsideDir(const wxString& target_dir, const wxString& entry_name,
                     wxString& out_full_path) {
  wxString combined_path = target_dir;
  if (!combined_path.EndsWith(wxFileName::GetPathSeparator())) {
    combined_path += wxFileName::GetPathSeparator();
  }
  combined_path += entry_name;

  wxFileName fn(combined_path);
  fn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG);
  out_full_path = fn.GetFullPath();

  wxFileName target_fn(target_dir);
  target_fn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE |
                      wxPATH_NORM_LONG);
  wxString normalized_target = target_fn.GetFullPath();

  if (!normalized_target.EndsWith(wxFileName::GetPathSeparator())) {
    normalized_target += wxFileName::GetPathSeparator();
  }

  if (out_full_path.StartsWith(normalized_target)) {
    return true;
  }
  if (out_full_path == target_fn.GetFullPath()) {
    return true;
  }
  return false;
}

bool PathTraversesSymlink(const wxString& root, const wxString& rel) {
  wxString base = root;
  while (!base.IsEmpty() && base.EndsWith(wxFileName::GetPathSeparator())) {
    base.RemoveLast();
  }
  const wxArrayString parts =
      wxStringTokenize(PortableRelPath(rel), wxT("/"), wxTOKEN_STRTOK);
  wxString current = base;
  // Only parent components matter: the leaf is replaced atomically by rename,
  // which does not follow a symlink at the final path. A symlinked *directory*
  // component, however, is the traversal escape we must reject.
  for (size_t i = 0; i + 1 < parts.GetCount(); ++i) {
    current += wxFileName::GetPathSeparator();
    current += parts.Item(i);
    if (wxFileName::Exists(current, wxFILE_EXISTS_SYMLINK)) {
      return true;
    }
  }
  return false;
}

wxString NormalizeDir(const wxString& dir) {
  wxFileName fn(dir);
  fn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG);
  wxString path = fn.GetFullPath();
#ifdef __UNIX__
  char resolved[PATH_MAX];
  if (realpath(path.fn_str(), resolved) != nullptr) {
    path = wxString(resolved);
  }
#endif
  return path;
}

bool SameNormalizedLiveRoot(const wxString& a, const wxString& b) {
  if (a == b) {
    return true;
  }
  if (wxDirExists(a) && wxDirExists(b) && wxFileName(a).SameAs(wxFileName(b))) {
    return true;
  }
  return false;
}

wxString PortableRelPath(wxString rel) {
  rel.Replace(wxT("\\"), wxT("/"));
  return rel;
}

bool EnsureParentDirsForPath(const wxString& path) {
  wxFileName fn(path);
  if (fn.GetPath().IsEmpty() || wxDirExists(fn.GetPath())) {
    return true;
  }
  return wxFileName::Mkdir(fn.GetPath(), 0755, wxPATH_MKDIR_FULL);
}

bool WriteTextFileAtomic(const wxString& path, const wxString& contents) {
  const wxString tmp_path = path + wxT(".tmp");
  wxRemoveFile(tmp_path);
  wxFile file;
  if (!file.Create(tmp_path, true)) {
    return false;
  }
  const wxCharBuffer utf8 = contents.utf8_str();
  const size_t nbytes = utf8.length();
  if (file.Write(utf8.data(), nbytes) != nbytes) {
    file.Close();
    wxRemoveFile(tmp_path);
    return false;
  }
  if (!file.Flush()) {
    file.Close();
    wxRemoveFile(tmp_path);
    return false;
  }
  file.Close();
  if (!wxRenameFile(tmp_path, path, true)) {
    wxRemoveFile(tmp_path);
    return false;
  }
#ifndef _WIN32
  const wxCharBuffer parent = wxFileName(path).GetPath().utf8_str();
  int flags = O_RDONLY;
#ifdef O_DIRECTORY
  flags |= O_DIRECTORY;
#endif
  const int dir_fd = open(parent.data(), flags);
  if (dir_fd < 0 || fsync(dir_fd) != 0) {
    if (dir_fd >= 0) {
      close(dir_fd);
    }
    return false;
  }
  close(dir_fd);
#endif
  return true;
}

bool RestoreReplaceFromBackup(const wxString& live_root,
                              const wxString& backup_root,
                              const wxString& stage_root, const wxString& rel) {
  wxString live_path;
  if (!IsPathInsideDir(live_root, rel, live_path) ||
      PathTraversesSymlink(live_root, rel)) {
    return false;
  }

  wxString backup_path;
  const bool has_backup = IsPathInsideDir(backup_root, rel, backup_path) &&
                          wxFileExists(backup_path);
  if (!has_backup) {
    return wxFileExists(live_path);
  }

  wxString trash_path;
  bool moved_to_trash = false;
  if (wxFileExists(live_path)) {
    const wxString trash_root =
        stage_root + wxFileName::GetPathSeparator() + _T("__chartdldr_trash__");
    if (!IsPathInsideDir(trash_root, rel, trash_path) ||
        !EnsureParentDirsForPath(trash_path) ||
        !wxRenameFile(live_path, trash_path)) {
      return false;
    }
    moved_to_trash = true;
  }

  if (!EnsureParentDirsForPath(live_path) ||
      !wxRenameFile(backup_path, live_path)) {
    if (moved_to_trash && wxFileExists(trash_path)) {
      EnsureParentDirsForPath(live_path);
      wxRenameFile(trash_path, live_path);
    }
    return false;
  }
  return true;
}

bool ReadExtractManifest(const wxString& stage_root, ExtractManifest& out) {
  const wxString path =
      stage_root + wxFileName::GetPathSeparator() + kManifestName;
  if (!wxFileExists(path)) {
    return false;
  }
  wxTextFile file;
  if (!file.Open(path)) {
    return false;
  }
  ExtractManifest parsed;
  for (size_t i = 0; i < file.GetLineCount(); ++i) {
    const wxString line = file.GetLine(i).Trim(true).Trim(false);
    if (line.IsEmpty()) {
      continue;
    }
    const int eq = line.Find('=');
    if (eq == wxNOT_FOUND) {
      continue;
    }
    const wxString key = line.Left(eq);
    const wxString value = line.Mid(eq + 1);
    if (key == wxT("version")) {
      long v = 0;
      if (!value.ToLong(&v)) {
        file.Close();
        return false;
      }
      parsed.version = static_cast<int>(v);
    } else if (key == wxT("live_root")) {
      parsed.live_root = value;
    } else if (key == wxT("phase")) {
      parsed.phase = value;
    } else if (key == wxT("entry")) {
      parsed.publish_entries.Add(value);
    }
  }
  file.Close();
  if ((parsed.version != 1 && parsed.version != 2) ||
      parsed.live_root.IsEmpty() || parsed.phase.IsEmpty()) {
    return false;
  }
  out = parsed;
  return true;
}

bool UndoOneOrphanCreate(const wxString& live_root, const wxString& rel) {
  wxString live_path;
  if (!IsPathInsideDir(live_root, rel, live_path) ||
      PathTraversesSymlink(live_root, rel)) {
    return false;
  }
  if (!wxFileExists(live_path)) {
    return true;
  }
  return wxRemoveFile(live_path);
}

bool IsTxnMetaRelPath(const wxString& rel) {
  return rel == kManifestName || rel == kBackupJournalName ||
         rel.StartsWith(wxString(kRollbackDirName)) ||
         rel.StartsWith(wxT("__chartdldr_"));
}

}  // namespace ChartDldrExtractCommon
