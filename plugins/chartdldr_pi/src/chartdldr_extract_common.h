/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_EXTRACT_COMMON_H_
#define CHARTDLDR_EXTRACT_COMMON_H_

#include <wx/arrstr.h>
#include <wx/string.h>

namespace ChartDldrExtractCommon {

extern const wxChar* kRollbackDirName;
extern const wxChar* kManifestName;
extern const wxChar* kExtractDirPrefix;

extern const wxChar* kPhaseStaging;
extern const wxChar* kPhasePublishing;
extern const wxChar* kPhaseCommitted;

/**
 * True when joining `entry_name` under `target_dir` stays inside that root.
 * Normalizes the existing root (including Windows long paths), then applies
 * only lexical DOTS/ABSOLUTE normalization to the joined path so a not-yet-
 * created leaf is not rejected by short/long-path mismatch.
 */
bool IsPathInsideDir(const wxString& target_dir, const wxString& entry_name,
                     wxString& out_full_path);

/**
 * True when joining `rel` under `root` would traverse an existing symlink or
 * reparse point in any parent directory component. IsPathInsideDir is purely
 * lexical after root normalization: a real directory component that is
 * actually a link pointing outside the root passes containment yet escapes on
 * the following rename/write. Callers must reject such paths before any
 * live-tree write, rename, restore, or delete.
 */
bool PathTraversesSymlink(const wxString& root, const wxString& rel);

/**
 * Publish `source_path` to `root`/`rel`, rejecting traversal escapes. Rejects
 * when the join leaves `root` (!IsPathInsideDir) or traverses a symlinked
 * intermediate component of `rel` (PathTraversesSymlink) — the escape an
 * untrusted relative path can use to write outside the managed root. Ensures
 * parent dirs, then moves the source into place: prefers an atomic rename,
 * falls back to a sibling copy+rename across filesystems. Never overwrites a
 * directory; on failure `source_path` is left intact.
 */
bool PublishFileInto(const wxString& root, const wxString& rel,
                     const wxString& source_path);

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
