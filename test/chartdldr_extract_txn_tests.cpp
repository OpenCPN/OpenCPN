/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_extract_txn.h"

#include <wx/file.h>
#include <wx/filename.h>

#ifdef __UNIX__
#include <climits>
#include <cstdlib>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

namespace {

wxString ReadFileContents(const wxString& path) {
  wxFile file(path);
  if (!file.IsOpened()) {
    return wxString();
  }
  wxString contents;
  file.ReadAll(&contents);
  return contents;
}

bool WriteFileContents(const wxString& path, const wxString& contents) {
  wxFileName::Mkdir(wxFileName(path).GetPath(), 0755, wxPATH_MKDIR_FULL);
  wxFile file(path, wxFile::write);
  if (!file.IsOpened()) {
    return false;
  }
  return file.Write(contents);
}

wxString MakeTempDir(const wxString& prefix) {
  wxFileName marker = wxFileName::CreateTempFileName(prefix);
  const wxString path = marker.GetPath() + wxFileName::GetPathSeparator() +
                        marker.GetName() + wxT("_dir");
  wxRemoveFile(marker.GetFullPath());
  wxFileName::Mkdir(path, 0755, wxPATH_MKDIR_FULL);
  return path;
}

/** Match ChartDldrExtractTxn's live_root normalization (incl. realpath). */
wxString CanonicalLiveRoot(const wxString& dir) {
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

wxString SiblingStageRoot(const wxString& live_root, int tag) {
  wxFileName live_fn(live_root);
  const wxString parent = live_fn.GetPath();
  return parent + wxFileName::GetPathSeparator() +
         wxString::Format(wxT(".chartdldr-extract-%lu-%d"),
                          static_cast<unsigned long>(wxGetProcessId()), tag);
}

bool WritePublishingOrphan(const wxString& stage_root,
                           const wxString& live_root, const wxString& chart_rel,
                           const wxString& journal_line,
                           const wxString& backup_bytes = wxString()) {
  if (!wxFileName::Mkdir(stage_root, 0755, wxPATH_MKDIR_FULL)) {
    return false;
  }
  if (!backup_bytes.IsEmpty()) {
    const wxString backup_path = stage_root + wxFileName::GetPathSeparator() +
                                 wxT("__chartdldr_rollback__") +
                                 wxFileName::GetPathSeparator() + chart_rel;
    if (!WriteFileContents(backup_path, backup_bytes)) {
      return false;
    }
  }
  const wxString journal_path = stage_root + wxFileName::GetPathSeparator() +
                                wxT("__chartdldr_backup_journal__");
  const wxString manifest_path = stage_root + wxFileName::GetPathSeparator() +
                                 wxT("__chartdldr_manifest__");
  if (!WriteFileContents(journal_path, journal_line)) {
    return false;
  }
  return WriteFileContents(
      manifest_path,
      wxString::Format(wxT("version=1\nlive_root=%s\nphase=PUBLISHING\n"),
                       CanonicalLiveRoot(live_root).c_str()));
}

}  // namespace

TEST(ChartDldrExtractTxn, KeepsBackupWhenPublishAndRestoreFail) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_live"));
  const wxString chart_rel = wxT("charts/test.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  const wxString original = wxT("original-chart-bytes");
  const wxString staged = wxT("staged-chart-bytes");

  ASSERT_TRUE(WriteFileContents(live_path, original));

  ChartDldrExtractTxn txn;
  ASSERT_TRUE(txn.Begin(live_root));
  const wxString stage_root = txn.StageRoot();
  ASSERT_FALSE(stage_root.IsEmpty());
  ASSERT_TRUE(wxDirExists(stage_root));

  wxString stage_path;
  ASSERT_TRUE(txn.MapEntry(chart_rel, stage_path));
  ASSERT_TRUE(WriteFileContents(stage_path, staged));
  ASSERT_TRUE(txn.AcceptWrittenFile(stage_path, staged.length()));

  txn.SetFailPublishAndRestore(true);
  EXPECT_FALSE(txn.Finalize(true, wxDateTime()));

  const wxString backup_path = stage_root + wxFileName::GetPathSeparator() +
                               wxT("__chartdldr_rollback__") +
                               wxFileName::GetPathSeparator() + chart_rel;
  const wxString manifest_path = stage_root + wxFileName::GetPathSeparator() +
                                 wxT("__chartdldr_manifest__");

  EXPECT_TRUE(wxDirExists(stage_root));
  EXPECT_TRUE(wxFileExists(backup_path));
  EXPECT_EQ(ReadFileContents(backup_path), original);
  EXPECT_TRUE(wxFileExists(manifest_path));
  EXPECT_TRUE(
      ReadFileContents(manifest_path).Contains(wxT("phase=PUBLISHING")));
  EXPECT_TRUE(ReadFileContents(manifest_path)
                  .Contains(wxT("entry=replace\t") + chart_rel));

  if (wxFileExists(live_path)) {
    EXPECT_EQ(ReadFileContents(live_path), original);
  }

  wxFileName::Rmdir(stage_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, SuccessfulPublishReplacesLive) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_ok"));
  const wxString chart_rel = wxT("chart.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  ASSERT_TRUE(WriteFileContents(live_path, wxT("old")));

  ChartDldrExtractTxn txn;
  ASSERT_TRUE(txn.Begin(live_root));
  const wxString stage_root = txn.StageRoot();

  wxString stage_path;
  ASSERT_TRUE(txn.MapEntry(chart_rel, stage_path));
  ASSERT_TRUE(WriteFileContents(stage_path, wxT("new")));
  ASSERT_TRUE(txn.AcceptWrittenFile(stage_path, 3));

  EXPECT_TRUE(txn.Finalize(true, wxDateTime()));
  EXPECT_FALSE(wxDirExists(stage_root));
  EXPECT_TRUE(wxFileExists(live_path));
  EXPECT_EQ(ReadFileContents(live_path), wxT("new"));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, RecoverOrphanStagingRestoresBackup) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_orphan"));
  const wxString chart_rel = wxT("charts/orphan.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  const wxString original = wxT("original-orphan-bytes");
  const wxString stage_root = SiblingStageRoot(live_root, 99);

  ASSERT_TRUE(WritePublishingOrphan(stage_root, live_root, chart_rel,
                                    wxT("replace\t") + chart_rel + wxT("\n"),
                                    original));
  EXPECT_FALSE(wxFileExists(live_path));

  ChartDldrRecoverOrphanExtractTrees(live_root);

  EXPECT_TRUE(wxFileExists(live_path));
  EXPECT_EQ(ReadFileContents(live_path), original);
  EXPECT_FALSE(wxDirExists(stage_root));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, RecoverOrphanCreateRemovesLiveNewFile) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_create"));
  const wxString chart_rel = wxT("new_chart.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  ASSERT_TRUE(WriteFileContents(live_path, wxT("partially-published")));
  const wxString stage_root = SiblingStageRoot(live_root, 98);

  ASSERT_TRUE(WritePublishingOrphan(stage_root, live_root, chart_rel,
                                    wxT("create\t") + chart_rel + wxT("\n")));

  ChartDldrRecoverOrphanExtractTrees(live_root);

  EXPECT_FALSE(wxFileExists(live_path));
  EXPECT_FALSE(wxDirExists(stage_root));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, RecoverOrphanDoesNotCrossSiblingLiveRoots) {
  const wxString parent = MakeTempDir(wxT("chartdldr_extract_siblings"));
  const wxString live_a =
      parent + wxFileName::GetPathSeparator() + wxT("source_a");
  const wxString live_b =
      parent + wxFileName::GetPathSeparator() + wxT("source_b");
  ASSERT_TRUE(wxFileName::Mkdir(live_a, 0755, wxPATH_MKDIR_FULL));
  ASSERT_TRUE(wxFileName::Mkdir(live_b, 0755, wxPATH_MKDIR_FULL));

  const wxString chart_rel = wxT("shared_name.kap");
  const wxString live_a_path =
      live_a + wxFileName::GetPathSeparator() + chart_rel;
  const wxString original_b = wxT("belongs-to-b");
  const wxString stage_root = SiblingStageRoot(live_a, 97);

  ASSERT_TRUE(WritePublishingOrphan(stage_root, live_b, chart_rel,
                                    wxT("replace\t") + chart_rel + wxT("\n"),
                                    original_b));

  // Recovery keyed to A must ignore B's orphan tree.
  ChartDldrRecoverOrphanExtractTrees(live_a);
  EXPECT_FALSE(wxFileExists(live_a_path));
  EXPECT_TRUE(wxDirExists(stage_root));

  const wxString live_b_path =
      live_b + wxFileName::GetPathSeparator() + chart_rel;
  ChartDldrRecoverOrphanExtractTrees(live_b);
  EXPECT_TRUE(wxFileExists(live_b_path));
  EXPECT_EQ(ReadFileContents(live_b_path), original_b);
  EXPECT_FALSE(wxFileExists(live_a_path));
  EXPECT_FALSE(wxDirExists(stage_root));

  wxFileName::Rmdir(parent, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, RecoverCommittedPhaseRemovesStageOnly) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_committed"));
  const wxString chart_rel = wxT("done.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  ASSERT_TRUE(WriteFileContents(live_path, wxT("already-live")));
  const wxString stage_root = SiblingStageRoot(live_root, 96);
  ASSERT_TRUE(wxFileName::Mkdir(stage_root, 0755, wxPATH_MKDIR_FULL));
  const wxString manifest_path = stage_root + wxFileName::GetPathSeparator() +
                                 wxT("__chartdldr_manifest__");
  ASSERT_TRUE(WriteFileContents(
      manifest_path,
      wxString::Format(wxT("version=1\nlive_root=%s\nphase=COMMITTED\n"),
                       CanonicalLiveRoot(live_root).c_str())));

  ChartDldrRecoverOrphanExtractTrees(live_root);
  EXPECT_FALSE(wxDirExists(stage_root));
  EXPECT_EQ(ReadFileContents(live_path), wxT("already-live"));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, PublishingAfterAllFilesMovedStillRollsBack) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_all_moved"));
  const wxString chart_rel = wxT("published.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  ASSERT_TRUE(WriteFileContents(live_path, wxT("new")));

  const wxString stage_root = SiblingStageRoot(live_root, 94);
  const wxString backup_path = stage_root + wxFileName::GetPathSeparator() +
                               wxT("__chartdldr_rollback__") +
                               wxFileName::GetPathSeparator() + chart_rel;
  ASSERT_TRUE(WriteFileContents(backup_path, wxT("old")));
  ASSERT_TRUE(WriteFileContents(
      stage_root + wxFileName::GetPathSeparator() +
          wxT("__chartdldr_manifest__"),
      wxString::Format(wxT("version=2\nlive_root=%s\nphase=PUBLISHING\n")
                           wxT("entry=replace\t%s\n"),
                       CanonicalLiveRoot(live_root).c_str(),
                       chart_rel.c_str())));

  ChartDldrRecoverOrphanExtractTrees(live_root);

  EXPECT_EQ(ReadFileContents(live_path), wxT("old"));
  EXPECT_FALSE(wxDirExists(stage_root));
  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn,
     RecoverPartialReplaceRestoresOriginalsWhenLiveHoldsNewBytes) {
  // Crash after A and B published (live=new, backup=old) but before C:
  // recovery must restore A/B originals, not treat "live exists" as success.
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_first_pass"));
  const wxString a_rel = wxT("a.kap");
  const wxString b_rel = wxT("b.kap");
  const wxString c_rel = wxT("c.kap");
  const wxString a_live = live_root + wxFileName::GetPathSeparator() + a_rel;
  const wxString b_live = live_root + wxFileName::GetPathSeparator() + b_rel;
  const wxString c_live = live_root + wxFileName::GetPathSeparator() + c_rel;
  ASSERT_TRUE(WriteFileContents(a_live, wxT("new-a")));
  ASSERT_TRUE(WriteFileContents(b_live, wxT("new-b")));
  ASSERT_TRUE(WriteFileContents(c_live, wxT("old-c")));

  const wxString stage_root = SiblingStageRoot(live_root, 93);
  ASSERT_TRUE(wxFileName::Mkdir(stage_root, 0755, wxPATH_MKDIR_FULL));
  const wxString rollback = stage_root + wxFileName::GetPathSeparator() +
                            wxT("__chartdldr_rollback__");
  ASSERT_TRUE(WriteFileContents(
      rollback + wxFileName::GetPathSeparator() + a_rel, wxT("old-a")));
  ASSERT_TRUE(WriteFileContents(
      rollback + wxFileName::GetPathSeparator() + b_rel, wxT("old-b")));

  ASSERT_TRUE(WriteFileContents(
      stage_root + wxFileName::GetPathSeparator() +
          wxT("__chartdldr_manifest__"),
      wxString::Format(wxT("version=2\nlive_root=%s\n")
                           wxT("phase=PUBLISHING\n") wxT("entry=replace\t%s\n")
                               wxT("entry=replace\t%s\n")
                                   wxT("entry=replace\t%s\n"),
                       CanonicalLiveRoot(live_root).c_str(), a_rel.c_str(),
                       b_rel.c_str(), c_rel.c_str())));

  ChartDldrRecoverOrphanExtractTrees(live_root);

  EXPECT_EQ(ReadFileContents(a_live), wxT("old-a"));
  EXPECT_EQ(ReadFileContents(b_live), wxT("old-b"));
  EXPECT_EQ(ReadFileContents(c_live), wxT("old-c"));
  EXPECT_FALSE(wxDirExists(stage_root));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, EmptyV2PublishingManifestIsRejected) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_empty_v2"));
  const wxString chart_rel = wxT("keep.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  ASSERT_TRUE(WriteFileContents(live_path, wxT("live")));

  const wxString stage_root = SiblingStageRoot(live_root, 92);
  ASSERT_TRUE(wxFileName::Mkdir(stage_root, 0755, wxPATH_MKDIR_FULL));
  ASSERT_TRUE(WriteFileContents(
      stage_root + wxFileName::GetPathSeparator() +
          wxT("__chartdldr_manifest__"),
      wxString::Format(wxT("version=2\nlive_root=%s\nphase=PUBLISHING\n"),
                       CanonicalLiveRoot(live_root).c_str())));

  ChartDldrRecoverOrphanExtractTrees(live_root);

  EXPECT_TRUE(wxDirExists(stage_root));
  EXPECT_EQ(ReadFileContents(live_path), wxT("live"));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn,
     RecoverPublishingIsIdempotentAfterInterruptedFirstPass) {
  // State after a first recovery restored A and was interrupted: A's backup
  // was consumed, B remains published with a backup, C was never reached.
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_partial"));
  const wxString a_rel = wxT("a.kap");
  const wxString b_rel = wxT("b.kap");
  const wxString c_rel = wxT("c.kap");
  const wxString a_live = live_root + wxFileName::GetPathSeparator() + a_rel;
  const wxString b_live = live_root + wxFileName::GetPathSeparator() + b_rel;
  const wxString c_live = live_root + wxFileName::GetPathSeparator() + c_rel;
  ASSERT_TRUE(WriteFileContents(a_live, wxT("old-a")));
  ASSERT_TRUE(WriteFileContents(b_live, wxT("new-b")));
  ASSERT_TRUE(WriteFileContents(c_live, wxT("old-c")));

  const wxString stage_root = SiblingStageRoot(live_root, 95);
  ASSERT_TRUE(wxFileName::Mkdir(stage_root, 0755, wxPATH_MKDIR_FULL));
  const wxString rollback = stage_root + wxFileName::GetPathSeparator() +
                            wxT("__chartdldr_rollback__");
  ASSERT_TRUE(WriteFileContents(
      rollback + wxFileName::GetPathSeparator() + b_rel, wxT("old-b")));

  ASSERT_TRUE(WriteFileContents(
      stage_root + wxFileName::GetPathSeparator() +
          wxT("__chartdldr_manifest__"),
      wxString::Format(wxT("version=2\nlive_root=%s\n")
                           wxT("phase=PUBLISHING\n") wxT("entry=replace\t%s\n")
                               wxT("entry=replace\t%s\n")
                                   wxT("entry=replace\t%s\n"),
                       CanonicalLiveRoot(live_root).c_str(), a_rel.c_str(),
                       b_rel.c_str(), c_rel.c_str())));

  ChartDldrRecoverOrphanExtractTrees(live_root);

  EXPECT_EQ(ReadFileContents(a_live), wxT("old-a"));
  EXPECT_EQ(ReadFileContents(b_live), wxT("old-b"));
  EXPECT_EQ(ReadFileContents(c_live), wxT("old-c"));
  EXPECT_FALSE(wxDirExists(stage_root));

  // A subsequent startup pass is a no-op and preserves restored bytes.
  ChartDldrRecoverOrphanExtractTrees(live_root);
  EXPECT_EQ(ReadFileContents(a_live), wxT("old-a"));
  EXPECT_EQ(ReadFileContents(b_live), wxT("old-b"));
  EXPECT_EQ(ReadFileContents(c_live), wxT("old-c"));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, CommittedMarkerFailureRollsBackPublishedBytes) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_marker_fail"));
  const wxString chart_rel = wxT("chart.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  ASSERT_TRUE(WriteFileContents(live_path, wxT("old")));

  ChartDldrExtractTxn txn;
  ASSERT_TRUE(txn.Begin(live_root));
  const wxString stage_root = txn.StageRoot();
  wxString stage_path;
  ASSERT_TRUE(txn.MapEntry(chart_rel, stage_path));
  ASSERT_TRUE(WriteFileContents(stage_path, wxT("new")));
  ASSERT_TRUE(txn.AcceptWrittenFile(stage_path, 3));
  txn.SetFailCommittedManifest(true);

  EXPECT_FALSE(txn.Finalize(true, wxDateTime()));
  EXPECT_EQ(ReadFileContents(live_path), wxT("old"));
  EXPECT_FALSE(wxDirExists(stage_root));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

#ifndef _WIN32
TEST(ChartDldrExtractTxn, PublishRefusesSymlinkedDirComponentEscape) {
  // An archive whose entry passes lexical containment but whose parent
  // directory component is a symlink pointing outside the chart root must not
  // let bytes land outside the root.
  const wxString parent = MakeTempDir(wxT("chartdldr_extract_symlink"));
  const wxString live_root =
      parent + wxFileName::GetPathSeparator() + wxT("charts");
  const wxString outside =
      parent + wxFileName::GetPathSeparator() + wxT("outside");
  ASSERT_TRUE(wxFileName::Mkdir(live_root, 0755, wxPATH_MKDIR_FULL));
  ASSERT_TRUE(wxFileName::Mkdir(outside, 0755, wxPATH_MKDIR_FULL));

  const wxString link_path =
      live_root + wxFileName::GetPathSeparator() + wxT("escape");
  ASSERT_EQ(0, symlink(outside.fn_str(), link_path.fn_str()));

  const wxString chart_rel = wxT("escape/pwned.kap");
  const wxString escaped_target =
      outside + wxFileName::GetPathSeparator() + wxT("pwned.kap");

  ChartDldrExtractTxn txn;
  ASSERT_TRUE(txn.Begin(live_root));
  wxString stage_path;
  ASSERT_TRUE(txn.MapEntry(chart_rel, stage_path));
  ASSERT_TRUE(WriteFileContents(stage_path, wxT("malicious")));
  ASSERT_TRUE(txn.AcceptWrittenFile(stage_path, 9));

  EXPECT_FALSE(txn.Finalize(true, wxDateTime()));
  EXPECT_FALSE(wxFileExists(escaped_target));

  wxFileName::Rmdir(parent, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}

TEST(ChartDldrExtractTxn, RecoverRefusesRestoreThroughSymlinkedComponent) {
  // Orphan recovery must not restore backup bytes through a symlinked live
  // directory component either.
  const wxString parent = MakeTempDir(wxT("chartdldr_recover_symlink"));
  const wxString live_root =
      parent + wxFileName::GetPathSeparator() + wxT("charts");
  const wxString outside =
      parent + wxFileName::GetPathSeparator() + wxT("outside");
  ASSERT_TRUE(wxFileName::Mkdir(live_root, 0755, wxPATH_MKDIR_FULL));
  ASSERT_TRUE(wxFileName::Mkdir(outside, 0755, wxPATH_MKDIR_FULL));

  const wxString link_path =
      live_root + wxFileName::GetPathSeparator() + wxT("escape");
  ASSERT_EQ(0, symlink(outside.fn_str(), link_path.fn_str()));

  const wxString chart_rel = wxT("escape/pwned.kap");
  const wxString escaped_target =
      outside + wxFileName::GetPathSeparator() + wxT("pwned.kap");
  const wxString stage_root = SiblingStageRoot(live_root, 91);

  ASSERT_TRUE(WritePublishingOrphan(stage_root, live_root, chart_rel,
                                    wxT("replace\t") + chart_rel + wxT("\n"),
                                    wxT("malicious")));

  ChartDldrRecoverOrphanExtractTrees(live_root);

  EXPECT_FALSE(wxFileExists(escaped_target));
  // Recovery could not complete safely, so the staging tree is preserved.
  EXPECT_TRUE(wxDirExists(stage_root));

  wxFileName::Rmdir(parent, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}
#endif  // _WIN32

TEST(ChartDldrExtractTxn, CommittedTreeSurvivesCleanupFailureAndRecovers) {
  const wxString live_root = MakeTempDir(wxT("chartdldr_extract_cleanup"));
  const wxString chart_rel = wxT("chart.kap");
  const wxString live_path =
      live_root + wxFileName::GetPathSeparator() + chart_rel;
  ASSERT_TRUE(WriteFileContents(live_path, wxT("old")));

  ChartDldrExtractTxn txn;
  ASSERT_TRUE(txn.Begin(live_root));
  const wxString stage_root = txn.StageRoot();
  wxString stage_path;
  ASSERT_TRUE(txn.MapEntry(chart_rel, stage_path));
  ASSERT_TRUE(WriteFileContents(stage_path, wxT("new")));
  ASSERT_TRUE(txn.AcceptWrittenFile(stage_path, 3));
  txn.SetFailStageRemoval(true);

  EXPECT_TRUE(txn.Finalize(true, wxDateTime()));
  ASSERT_TRUE(wxDirExists(stage_root));
  EXPECT_TRUE(ReadFileContents(stage_root + wxFileName::GetPathSeparator() +
                               wxT("__chartdldr_manifest__"))
                  .Contains(wxT("phase=COMMITTED")));
  EXPECT_EQ(ReadFileContents(live_path), wxT("new"));

  ChartDldrRecoverOrphanExtractTrees(live_root);
  EXPECT_FALSE(wxDirExists(stage_root));
  EXPECT_EQ(ReadFileContents(live_path), wxT("new"));

  wxFileName::Rmdir(live_root, wxPATH_RMDIR_RECURSIVE | wxPATH_RMDIR_FULL);
}
