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
#include <wx/textfile.h>

namespace {

using ChartDldrExtractCommon::ExtractManifest;
using ChartDldrExtractCommon::IsPathInsideDir;
using ChartDldrExtractCommon::kBackupJournalName;
using ChartDldrExtractCommon::kExtractDirPrefix;
using ChartDldrExtractCommon::kManifestName;
using ChartDldrExtractCommon::kPhaseCommitted;
using ChartDldrExtractCommon::kPhasePublishing;
using ChartDldrExtractCommon::kRollbackDirName;
using ChartDldrExtractCommon::NormalizeDir;
using ChartDldrExtractCommon::ReadExtractManifest;
using ChartDldrExtractCommon::RestoreReplaceFromBackup;
using ChartDldrExtractCommon::SameNormalizedLiveRoot;
using ChartDldrExtractCommon::UndoOneOrphanCreate;

bool RecoverPublishingOrphan(const wxString& live_root,
                             const wxString& stage_root,
                             const ExtractManifest& manifest) {
  wxArrayString entries = manifest.publish_entries;
  const wxString journal_path =
      stage_root + wxFileName::GetPathSeparator() + kBackupJournalName;
  if (entries.IsEmpty() && wxFileExists(journal_path)) {
    wxTextFile journal;
    if (!journal.Open(journal_path)) {
      wxLogError(_T("chartdldr_pi: cannot read extract backup journal at %s"),
                 journal_path.c_str());
      return false;
    }
    for (size_t i = 0; i < journal.GetLineCount(); ++i) {
      const wxString line = journal.GetLine(i).Trim(true).Trim(false);
      if (!line.IsEmpty()) {
        entries.Add(line);
      }
    }
    journal.Close();
  }
  if (entries.IsEmpty()) {
    wxLogWarning(
        _T("chartdldr_pi: PUBLISHING extract manifest has no rollback entries ")
        _T("at %s"),
        stage_root.c_str());
    return false;
  }

  const wxString backup_root =
      stage_root + wxFileName::GetPathSeparator() + kRollbackDirName;
  bool all_ok = true;
  size_t entry_count = 0;
  for (size_t i = 0; i < entries.GetCount(); ++i) {
    const wxString line = entries.Item(i);
    wxString op;
    wxString rel;
    const int tab = line.Find('\t');
    if (tab == wxNOT_FOUND) {
      op = wxT("replace");
      rel = line;
    } else {
      op = line.Left(tab);
      rel = line.Mid(tab + 1);
    }
    if (rel.IsEmpty()) {
      continue;
    }
    ++entry_count;
    bool ok = false;
    if (op == wxT("replace")) {
      ok = RestoreReplaceFromBackup(live_root, backup_root, stage_root, rel);
    } else if (op == wxT("create")) {
      ok = UndoOneOrphanCreate(live_root, rel);
    } else {
      wxLogError(_T("chartdldr_pi: unknown extract journal op \"%s\" for %s"),
                 op.c_str(), rel.c_str());
      ok = false;
    }
    if (!ok) {
      all_ok = false;
      wxLogError(
          _T("chartdldr_pi: orphan extract recovery failed for %s under %s"),
          rel.c_str(), live_root.c_str());
    }
  }
  if (entry_count == 0) {
    wxLogError(
        _T("chartdldr_pi: PUBLISHING extract journal empty or truncated at %s"),
        journal_path.c_str());
    return false;
  }

  if (all_ok) {
    all_ok = wxFileName::Rmdir(stage_root,
                               wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
  }
  return all_ok;
}

bool RecoverOneOrphanExtractTree(const wxString& requested_live_root,
                                 const wxString& stage_root) {
  ExtractManifest manifest;
  if (!ReadExtractManifest(stage_root, manifest)) {
    wxLogWarning(
        _T("chartdldr_pi: leaving extract staging tree intact (missing or ")
        _T("invalid manifest): %s"),
        stage_root.c_str());
    return false;
  }

  const wxString manifest_live = NormalizeDir(manifest.live_root);
  if (!SameNormalizedLiveRoot(manifest_live, requested_live_root)) {
    wxLogWarning(
        _T("chartdldr_pi: skipping extract staging tree (manifest live_root ")
        _T("mismatch): %s"),
        stage_root.c_str());
    return false;
  }

  if (manifest.phase == kPhaseCommitted) {
    if (!wxFileName::Rmdir(stage_root,
                           wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL)) {
      wxLogWarning(
          _T("chartdldr_pi: could not remove committed extract staging tree ")
          _T("%s"),
          stage_root.c_str());
      return false;
    }
    return true;
  }

  if (manifest.phase == kPhasePublishing) {
    if (RecoverPublishingOrphan(requested_live_root, stage_root, manifest)) {
      return true;
    }
    wxLogError(
        _T("chartdldr_pi: leaving PUBLISHING extract staging tree intact: %s"),
        stage_root.c_str());
    return false;
  }

  wxLogWarning(
      _T("chartdldr_pi: leaving extract staging tree intact (phase %s): %s"),
      manifest.phase.c_str(), stage_root.c_str());
  return false;
}

}  // namespace

void ChartDldrRecoverOrphanExtractTrees(const wxString& live_or_parent) {
  if (live_or_parent.IsEmpty()) {
    return;
  }
  const wxString live_root = NormalizeDir(live_or_parent);
  if (live_root.IsEmpty()) {
    return;
  }

  wxFileName live_fn(live_root);
  const wxString parent = live_fn.GetPath();
  if (parent.IsEmpty() || !wxDirExists(parent)) {
    return;
  }

  wxDir dir(parent);
  if (!dir.IsOpened()) {
    return;
  }

  wxString name;
  bool cont = dir.GetFirst(&name, wxEmptyString, wxDIR_DIRS | wxDIR_HIDDEN);
  while (cont) {
    if (name.StartsWith(kExtractDirPrefix)) {
      const wxString stage_root =
          parent + wxFileName::GetPathSeparator() + name;
      RecoverOneOrphanExtractTree(live_root, stage_root);
    }
    cont = dir.GetNext(&name);
  }
}
