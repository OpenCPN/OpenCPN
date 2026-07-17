/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_EXTRACT_TXN_H_
#define CHARTDLDR_EXTRACT_TXN_H_

#include <wx/arrstr.h>
#include <wx/datetime.h>
#include <wx/string.h>

/** Drop the leading directory when strip_path is true (all extract backends).
 */
wxString ChartDldrArchiveEntryRelPath(wxString name, bool strip_path);

/**
 * Scan the parent of live_root for `.chartdldr-extract-*` trees. Recover only
 * trees whose manifest live_root matches (normalized). Skip missing/mismatched
 * manifests (log + leave intact) so sibling chart sources cannot steal files.
 */
void ChartDldrRecoverOrphanExtractTrees(const wxString& live_root);

/**
 * One extract transaction for every backend: write into a sibling staging
 * tree, verify each file, then publish with backup/rollback and stamp only
 * after the full publish succeeds (live charts stay intact on failure).
 */
class ChartDldrExtractTxn {
public:
  ChartDldrExtractTxn() = default;
  ~ChartDldrExtractTxn();

  ChartDldrExtractTxn(const ChartDldrExtractTxn&) = delete;
  ChartDldrExtractTxn& operator=(const ChartDldrExtractTxn&) = delete;

  bool Begin(const wxString& live_target);

  /** Map an archive-relative path into the staging tree. */
  bool MapEntry(const wxString& entry_rel, wxString& stage_path) const;

  bool EnsureParentDirs(const wxString& path) const;

  /**
   * After bytes are fully written to stage_path: verify size (when known) and
   * record the file for commit. Shared completeness gate for all backends.
   */
  bool AcceptWrittenFile(const wxString& stage_path,
                         wxFileOffset expected_size);

  bool Finalize(bool success, const wxDateTime& mtime);

  /** Staging root after Begin(); used by platforms that extract via an external
   * helper. */
  const wxString& StageRoot() const { return stage_root_; }

  /**
   * After an external extractor wrote into StageRoot(), record every staged
   * file for commit (size unknown — completeness was the extractor's job).
   */
  bool AcceptAllStagedFiles();

#ifdef UNIT_TESTS
  /**
   * Test-only: after a live→backup move, force stage→live and any restore
   * rename to fail so rollback must keep the staging/backup tree.
   */
  void SetFailPublishAndRestore(bool enable) {
    fail_publish_and_restore_ = enable;
  }
  void SetFailCommittedManifest(bool enable) {
    fail_committed_manifest_ = enable;
  }
  void SetFailStageRemoval(bool enable) { fail_stage_removal_ = enable; }
#endif

private:
  static const wxChar* RollbackDirName();
  static const wxChar* ManifestName();
  wxString BackupRoot() const;
  wxString ManifestPath() const;

  bool WriteManifest(const wxString& phase) const;

  /** Record a successful live→backup move for in-memory rollback. */
  void NoteBackedUp(const wxString& rel);

  /** Drop rel from backed_up_ after a successful immediate restore. */
  void ForgetBackedUp(const wxString& rel);

  /** Restore one published path. Leaves backups in place on failure. */
  bool RestorePublishedPath(const wxString& rel, const wxString& backup_root);

  /**
   * Roll back published files and any in-flight backups. Removes the stage
   * tree only when every restore succeeds; otherwise keeps stage/backups
   * rather than incinerating them.
   */
  void RollbackPublish(const wxArrayString& published);

  bool Commit(const wxDateTime& mtime);
  void Abort();
  bool RemoveStageTree();

  wxString live_root_;
  wxString stage_root_;
  wxArrayString rel_paths_;
  /** Relative paths successfully moved live→backup during Commit. */
  wxArrayString backed_up_;
  bool active_ = false;
  bool finished_ = false;
#ifdef UNIT_TESTS
  bool fail_publish_and_restore_ = false;
  bool fail_committed_manifest_ = false;
  bool fail_stage_removal_ = false;
#endif
};

#endif  // CHARTDLDR_EXTRACT_TXN_H_
