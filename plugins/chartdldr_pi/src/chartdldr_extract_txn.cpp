/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_extract_txn.h"

#include "chartdldr_extract_common.h"

#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/log.h>

using ChartDldrExtractCommon::EnsureParentDirsForPath;
using ChartDldrExtractCommon::IsPathInsideDir;
using ChartDldrExtractCommon::IsTxnMetaRelPath;
using ChartDldrExtractCommon::kManifestName;
using ChartDldrExtractCommon::kPhaseCommitted;
using ChartDldrExtractCommon::kPhasePublishing;
using ChartDldrExtractCommon::kPhaseStaging;
using ChartDldrExtractCommon::kRollbackDirName;
using ChartDldrExtractCommon::NormalizeDir;
using ChartDldrExtractCommon::PathTraversesSymlink;
using ChartDldrExtractCommon::PortableRelPath;
using ChartDldrExtractCommon::RestoreReplaceFromBackup;
using ChartDldrExtractCommon::WriteTextFileAtomic;

wxString ChartDldrArchiveEntryRelPath(wxString name, bool strip_path) {
  if (!strip_path) {
    return name;
  }
  wxFileName fn(name);
  if (fn.GetDirCount() > 0) {
    fn.RemoveDir(0);
    return fn.GetFullPath();
  }
  return name;
}

ChartDldrExtractTxn::~ChartDldrExtractTxn() { Abort(); }

bool ChartDldrExtractTxn::Begin(const wxString& live_target) {
  Abort();
  live_root_ = NormalizeDir(live_target);
  if (live_root_.IsEmpty()) {
    return false;
  }
  if (!wxDirExists(live_root_) &&
      !wxFileName::Mkdir(live_root_, 0755, wxPATH_MKDIR_FULL)) {
    return false;
  }

  ChartDldrRecoverOrphanExtractTrees(live_root_);

  wxFileName live_fn(live_root_);
  const wxString parent = live_fn.GetPath();
  if (parent.IsEmpty()) {
    return false;
  }
  const unsigned long pid = static_cast<unsigned long>(wxGetProcessId());
  for (int attempt = 0; attempt < 64; ++attempt) {
    const wxString candidate =
        parent + wxFileName::GetPathSeparator() +
        wxString::Format(_T(".chartdldr-extract-%lu-%d"), pid, attempt);
    if (wxFileName::Exists(candidate)) {
      continue;
    }
    if (wxFileName::Mkdir(candidate, 0755, wxPATH_MKDIR_FULL)) {
      stage_root_ = NormalizeDir(candidate);
      active_ = true;
      finished_ = false;
      rel_paths_.Clear();
      backed_up_.Clear();
      if (!WriteManifest(kPhaseStaging)) {
        RemoveStageTree();
        active_ = false;
        return false;
      }
      return true;
    }
  }
  return false;
}

bool ChartDldrExtractTxn::MapEntry(const wxString& entry_rel,
                                   wxString& stage_path) const {
  return IsPathInsideDir(stage_root_, entry_rel, stage_path);
}

bool ChartDldrExtractTxn::EnsureParentDirs(const wxString& path) const {
  return EnsureParentDirsForPath(path);
}

bool ChartDldrExtractTxn::AcceptWrittenFile(const wxString& stage_path,
                                            wxFileOffset expected_size) {
  wxStructStat st;
  if (wxStat(stage_path, &st) != 0) {
    wxRemoveFile(stage_path);
    return false;
  }
  if (expected_size != wxInvalidOffset &&
      static_cast<wxFileOffset>(st.st_size) != expected_size) {
    wxRemoveFile(stage_path);
    return false;
  }

  wxString root = stage_root_;
  if (!root.EndsWith(wxFileName::GetPathSeparator())) {
    root += wxFileName::GetPathSeparator();
  }
  if (!stage_path.StartsWith(root)) {
    wxRemoveFile(stage_path);
    return false;
  }
  const wxString rel = PortableRelPath(stage_path.Mid(root.Length()));
  if (IsTxnMetaRelPath(rel)) {
    return true;
  }
  rel_paths_.Add(rel);
  return true;
}

bool ChartDldrExtractTxn::Finalize(bool success, const wxDateTime& mtime) {
  if (!success) {
    Abort();
    return false;
  }
  return Commit(mtime);
}

bool ChartDldrExtractTxn::AcceptAllStagedFiles() {
  if (!active_ || stage_root_.IsEmpty() || !wxDirExists(stage_root_)) {
    return false;
  }
  wxArrayString files;
  wxDir::GetAllFiles(stage_root_, &files);
  for (size_t i = 0; i < files.GetCount(); ++i) {
    if (!AcceptWrittenFile(files.Item(i), wxInvalidOffset)) {
      return false;
    }
  }
  return true;
}

const wxChar* ChartDldrExtractTxn::RollbackDirName() {
  return kRollbackDirName;
}

const wxChar* ChartDldrExtractTxn::ManifestName() { return kManifestName; }

wxString ChartDldrExtractTxn::BackupRoot() const {
  return stage_root_ + wxFileName::GetPathSeparator() + RollbackDirName();
}

wxString ChartDldrExtractTxn::ManifestPath() const {
  return stage_root_ + wxFileName::GetPathSeparator() + ManifestName();
}

bool ChartDldrExtractTxn::WriteManifest(const wxString& phase) const {
#ifdef UNIT_TESTS
  if (phase == kPhaseCommitted && fail_committed_manifest_) {
    return false;
  }
#endif
  wxString body = wxString::Format(_T("version=2\nlive_root=%s\nphase=%s\n"),
                                   live_root_.c_str(), phase.c_str());
  if (phase == kPhasePublishing) {
    for (size_t i = 0; i < rel_paths_.GetCount(); ++i) {
      const wxString& rel = rel_paths_.Item(i);
      wxString live_path;
      if (!IsPathInsideDir(live_root_, rel, live_path) ||
          rel.Contains(wxT("\n")) || rel.Contains(wxT("\r"))) {
        return false;
      }
      const wxString op =
          wxFileExists(live_path) ? wxT("replace") : wxT("create");
      body += wxT("entry=");
      body += op;
      body += wxT("\t");
      body += rel;
      body += wxT("\n");
    }
  }
  return WriteTextFileAtomic(ManifestPath(), body);
}

void ChartDldrExtractTxn::NoteBackedUp(const wxString& rel) {
  backed_up_.Add(rel);
}

void ChartDldrExtractTxn::ForgetBackedUp(const wxString& rel) {
  const int idx = backed_up_.Index(rel);
  if (idx != wxNOT_FOUND) {
    backed_up_.RemoveAt(static_cast<size_t>(idx));
  }
}

bool ChartDldrExtractTxn::RestorePublishedPath(const wxString& rel,
                                               const wxString& backup_root) {
#ifdef UNIT_TESTS
  if (fail_publish_and_restore_) {
    return false;
  }
#endif

  wxString live_path;
  if (!IsPathInsideDir(live_root_, rel, live_path) ||
      PathTraversesSymlink(live_root_, rel)) {
    return false;
  }

  wxString backup_path;
  const bool has_backup = IsPathInsideDir(backup_root, rel, backup_path) &&
                          wxFileExists(backup_path);

  if (!has_backup) {
    return !wxFileExists(live_path) || wxRemoveFile(live_path);
  }

  return RestoreReplaceFromBackup(live_root_, backup_root, stage_root_, rel);
}

void ChartDldrExtractTxn::RollbackPublish(const wxArrayString& published) {
  wxArrayString to_restore = published;
  for (size_t i = 0; i < backed_up_.GetCount(); ++i) {
    if (to_restore.Index(backed_up_.Item(i)) == wxNOT_FOUND) {
      to_restore.Add(backed_up_.Item(i));
    }
  }

  const wxString backup_root = BackupRoot();
  bool all_restored = true;
  for (size_t i = to_restore.GetCount(); i-- > 0;) {
    if (!RestorePublishedPath(to_restore.Item(i), backup_root)) {
      all_restored = false;
      wxLogError(
          _T("chartdldr_pi: Failed to restore chart after extract rollback; ")
          _T("leaving staging tree intact."));
    }
  }
  if (all_restored) {
    RemoveStageTree();
  } else {
    wxLogError(
        _T("chartdldr_pi: Extract rollback incomplete; leaving staging and ")
        _T("backup tree intact for recovery."));
  }
  active_ = false;
}

bool ChartDldrExtractTxn::Commit(const wxDateTime& mtime) {
  if (!active_ || finished_) {
    return false;
  }
  finished_ = true;
  if (rel_paths_.IsEmpty()) {
    RemoveStageTree();
    active_ = false;
    return false;
  }

  if (!WriteManifest(kPhasePublishing)) {
    wxLogError(_T("chartdldr_pi: failed to mark extract phase PUBLISHING"));
    RemoveStageTree();
    active_ = false;
    return false;
  }

  const wxString backup_root = BackupRoot();
  wxArrayString published;

  for (size_t i = 0; i < rel_paths_.GetCount(); ++i) {
    const wxString& rel = rel_paths_.Item(i);
    wxString stage_path;
    wxString live_path;
    if (!IsPathInsideDir(stage_root_, rel, stage_path) ||
        !IsPathInsideDir(live_root_, rel, live_path) ||
        PathTraversesSymlink(live_root_, rel)) {
      wxLogError(
          _T("chartdldr_pi: refusing to publish %s through a symlinked path ")
          _T("component under the chart root"),
          rel.c_str());
      RollbackPublish(published);
      return false;
    }
    if (!EnsureParentDirs(live_path)) {
      RollbackPublish(published);
      return false;
    }

    wxString backup_path;
    const bool had_live = wxFileExists(live_path);
    if (had_live) {
      if (!IsPathInsideDir(backup_root, rel, backup_path) ||
          !EnsureParentDirs(backup_path)) {
        RollbackPublish(published);
        return false;
      }
      if (!wxRenameFile(live_path, backup_path)) {
        RollbackPublish(published);
        return false;
      }
      NoteBackedUp(rel);
    }

    bool published_ok = false;
#ifdef UNIT_TESTS
    if (fail_publish_and_restore_) {
      published_ok = false;
    } else
#endif
    {
      published_ok = wxRenameFile(stage_path, live_path);
    }

    if (!published_ok) {
      RollbackPublish(published);
      return false;
    }
    published.Add(rel);
  }

  if (!WriteManifest(kPhaseCommitted)) {
    wxLogError(_T("chartdldr_pi: failed to mark extract phase COMMITTED"));
    RollbackPublish(published);
    return false;
  }

  if (mtime.IsValid()) {
    for (size_t i = 0; i < published.GetCount(); ++i) {
      wxString live_path;
      if (!IsPathInsideDir(live_root_, published.Item(i), live_path)) {
        wxLogWarning(
            _T("chartdldr_pi: could not stamp committed chart path %s"),
            published.Item(i).c_str());
        continue;
      }
      wxFileName stamped(live_path);
      if (!stamped.SetTimes(&mtime, &mtime, &mtime)) {
        wxLogWarning(_T("chartdldr_pi: could not stamp committed chart %s"),
                     live_path.c_str());
      }
    }
  }

  if (!RemoveStageTree()) {
    wxLogWarning(
        _T("chartdldr_pi: committed extract tree remains for orphan cleanup: ")
        _T("%s"),
        stage_root_.c_str());
  }
  active_ = false;
  return true;
}

void ChartDldrExtractTxn::Abort() {
  if (!active_ || finished_) {
    return;
  }
  finished_ = true;
  RemoveStageTree();
  rel_paths_.Clear();
  backed_up_.Clear();
  active_ = false;
}

bool ChartDldrExtractTxn::RemoveStageTree() {
  if (stage_root_.IsEmpty() || !wxDirExists(stage_root_)) {
    stage_root_.Clear();
    return true;
  }
#ifdef UNIT_TESTS
  if (fail_stage_removal_) {
    return false;
  }
#endif
  if (!wxFileName::Rmdir(stage_root_,
                         wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL)) {
    return false;
  }
  stage_root_.Clear();
  return true;
}
