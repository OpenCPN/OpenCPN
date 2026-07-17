/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_EXTRACT_COMMON_H_
#define CHARTDLDR_EXTRACT_COMMON_H_

#include <wx/arrstr.h>
#include <wx/string.h>

namespace ChartDldrExtractCommon {

extern const wxChar* kRollbackDirName;
extern const wxChar* kBackupJournalName;
extern const wxChar* kManifestName;
extern const wxChar* kExtractDirPrefix;

extern const wxChar* kPhaseStaging;
extern const wxChar* kPhasePublishing;
extern const wxChar* kPhaseCommitted;

bool IsPathInsideDir(const wxString& target_dir, const wxString& entry_name,
                     wxString& out_full_path);

/**
 * True when joining `rel` under `root` would traverse an existing symlink or
 * reparse point in any parent directory component. IsPathInsideDir is purely
 * lexical: a real directory component that is actually a link pointing outside
 * the root passes containment yet escapes on the following rename/write.
 * Callers must reject such paths before any live-tree write, rename, restore,
 * or delete.
 */
bool PathTraversesSymlink(const wxString& root, const wxString& rel);

wxString NormalizeDir(const wxString& dir);

bool SameNormalizedLiveRoot(const wxString& a, const wxString& b);

wxString PortableRelPath(wxString rel);

bool EnsureParentDirsForPath(const wxString& path);

bool WriteTextFileAtomic(const wxString& path, const wxString& contents);

bool RestoreReplaceFromBackup(const wxString& live_root,
                              const wxString& backup_root,
                              const wxString& stage_root, const wxString& rel);

struct ExtractManifest {
  int version = 0;
  wxString live_root;
  wxString phase;
  wxArrayString publish_entries;
};

bool ReadExtractManifest(const wxString& stage_root, ExtractManifest& out);

bool UndoOneOrphanCreate(const wxString& live_root, const wxString& rel);

bool IsTxnMetaRelPath(const wxString& rel);

}  // namespace ChartDldrExtractCommon

#endif  // CHARTDLDR_EXTRACT_COMMON_H_
