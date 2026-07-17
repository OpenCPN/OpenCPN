/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_temp_download.h"

#include "chartdldr_extract_common.h"

#include <wx/filename.h>

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

}  // namespace

TEST(ChartDldrTempDownload, SyncPathsUseTempFile) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_temp_dl_test"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString output_path = output_dir.GetPath() +
                               wxFileName::GetPathSeparator() +
                               wxT("catalog.xml");
  const ChartDldrTempDownloadPaths paths =
      ChartDldrTempDownloadPathsFor(output_path);

  EXPECT_FALSE(paths.download_target.empty());
  EXPECT_FALSE(paths.temp_path.empty());
  EXPECT_EQ(paths.output_path, output_path);
  EXPECT_NE(paths.download_target, output_path);
  EXPECT_NE(paths.temp_path, output_path);
#ifdef __ANDROID__
  EXPECT_TRUE(paths.download_target.StartsWith(wxT("file://")));
  EXPECT_NE(paths.download_target, paths.temp_path);
#endif

  ChartDldrRemoveTempDownload(paths.temp_path);
  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrTempDownload, FinalizeCopiesTempToOutput) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_temp_finalize"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString output_path = output_dir.GetPath() +
                               wxFileName::GetPathSeparator() +
                               wxT("catalog.xml");
  wxFileName temp_fn = wxFileName::CreateTempFileName(output_path);
  const wxString temp_path = temp_fn.GetFullPath();
  {
    wxFile file(temp_path, wxFile::write);
    file.Write("catalog");
  }

  EXPECT_TRUE(ChartDldrFinalizeTempDownload(temp_path, output_path));
  EXPECT_TRUE(wxFileExists(output_path));
  EXPECT_EQ(ReadFileContents(output_path), wxT("catalog"));
  EXPECT_FALSE(wxFileExists(temp_path));
  EXPECT_FALSE(wxFileExists(output_path + wxT(".publish-tmp")));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrTempDownload, FinalizeRenameReplacesExistingOutput) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_temp_rename"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString output_path = output_dir.GetPath() +
                               wxFileName::GetPathSeparator() +
                               wxT("catalog.xml");
  {
    wxFile file(output_path, wxFile::write);
    file.Write("old");
  }
  wxFileName temp_fn = wxFileName::CreateTempFileName(output_path);
  const wxString temp_path = temp_fn.GetFullPath();
  {
    wxFile file(temp_path, wxFile::write);
    file.Write("new");
  }

  EXPECT_TRUE(ChartDldrFinalizeTempDownload(temp_path, output_path));
  EXPECT_EQ(ReadFileContents(output_path), wxT("new"));
  EXPECT_FALSE(wxFileExists(temp_path));
  EXPECT_FALSE(wxFileExists(output_path + wxT(".publish-tmp")));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrTempDownload, BackgroundTargetPathUsesFileUriOnAndroid) {
  const wxString local_path = wxT("/tmp/chart.zip");
  const wxString target =
      ChartDldrTempDownloadBackgroundTargetPath(wxFileName(local_path));
#ifdef __ANDROID__
  EXPECT_TRUE(target.StartsWith(wxT("file://")));
#else
  EXPECT_EQ(target, wxFileName(local_path).GetFullPath());
#endif
}

TEST(ChartDldrTempDownload, CompletePathsFinalizeFailureReportsFailed) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_temp_complete"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString output_path = output_dir.GetPath() +
                               wxFileName::GetPathSeparator() +
                               wxT("catalog.xml");
  const ChartDldrTempDownloadPaths paths =
      ChartDldrTempDownloadPathsFor(output_path);
  wxRemoveFile(paths.temp_path);

  EXPECT_EQ(ChartDldrCompleteTempDownloadPaths(paths, true, false),
            OCPN_DL_FAILED);

  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrTempDownload, CompletePathsTransferFailureRemovesTemp) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_temp_complete_fail"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString output_path = output_dir.GetPath() +
                               wxFileName::GetPathSeparator() +
                               wxT("catalog.xml");
  const ChartDldrTempDownloadPaths paths =
      ChartDldrTempDownloadPathsFor(output_path);
  {
    wxFile file(paths.temp_path, wxFile::write);
    file.Write("catalog");
  }

  EXPECT_EQ(ChartDldrCompleteTempDownloadPaths(paths, false, true),
            OCPN_DL_ABORTED);
  EXPECT_FALSE(wxFileExists(paths.temp_path));

  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrTempDownload, TransferFailurePreservesExistingOutput) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_temp_preserve_fail"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString output_path =
      output_dir.GetPath() + wxFileName::GetPathSeparator() + wxT("chart.zip");
  {
    wxFile file(output_path, wxFile::write);
    file.Write("existing");
  }

  const ChartDldrTempDownloadPaths paths =
      ChartDldrTempDownloadPathsFor(output_path);
  {
    wxFile file(paths.temp_path, wxFile::write);
    file.Write("replacement");
  }

  EXPECT_EQ(ChartDldrCompleteTempDownloadPaths(paths, false, false),
            OCPN_DL_FAILED);
  EXPECT_EQ(ReadFileContents(output_path), wxT("existing"));
  EXPECT_FALSE(wxFileExists(paths.temp_path));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrTempDownload, FinalizeFailurePreservesExistingOutput) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_temp_preserve_finalize"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString output_path =
      output_dir.GetPath() + wxFileName::GetPathSeparator() + wxT("chart.zip");
  {
    wxFile file(output_path, wxFile::write);
    file.Write("existing");
  }

  ChartDldrTempDownloadPaths paths;
  paths.output_path = output_path;
  paths.temp_path = output_path + wxT(".missing-temp");

  EXPECT_EQ(ChartDldrCompleteTempDownloadPaths(paths, true, false),
            OCPN_DL_FAILED);
  EXPECT_EQ(ReadFileContents(output_path), wxT("existing"));

  wxRemoveFile(output_path);
  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}

#ifndef _WIN32
TEST(ChartDldrTempDownload, PublishFileIntoRejectsSymlinkedRelComponent) {
  wxFileName base_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_publish_symlink"));
  wxRemoveFile(base_dir.GetFullPath());
  const wxString root = base_dir.GetPath() + wxFileName::GetPathSeparator() +
                        wxT("chartdldr_publish_root");
  wxFileName::Mkdir(root, 0755, wxPATH_MKDIR_FULL);

  // An intermediate rel component ("sub") is a symlink escaping the root; a
  // publish through it must be rejected before any rename/copy follows it.
  const wxString escape_dir =
      base_dir.GetPath() + wxFileName::GetPathSeparator() + wxT("escape");
  wxFileName::Mkdir(escape_dir, 0755, wxPATH_MKDIR_FULL);
  const wxString link_dir = root + wxFileName::GetPathSeparator() + wxT("sub");
  ASSERT_EQ(symlink(escape_dir.fn_str(), link_dir.fn_str()), 0);

  wxFileName temp_fn = wxFileName::CreateTempFileName(
      base_dir.GetPath() + wxFileName::GetPathSeparator() + wxT("dl"));
  const wxString temp_path = temp_fn.GetFullPath();
  {
    wxFile file(temp_path, wxFile::write);
    file.Write("payload");
  }

  EXPECT_FALSE(ChartDldrExtractCommon::PublishFileInto(root, wxT("sub/chart.000"),
                                                       temp_path));
  EXPECT_FALSE(wxFileExists(escape_dir + wxFileName::GetPathSeparator() +
                            wxT("chart.000")));
  EXPECT_TRUE(wxFileExists(temp_path));

  ChartDldrRemoveTempDownload(temp_path);
  wxFileName::Rmdir(escape_dir, wxPATH_RMDIR_RECURSIVE);
  wxFileName::Rmdir(root, wxPATH_RMDIR_RECURSIVE);
}

TEST(ChartDldrTempDownload, PublishFileIntoMovesSourceIntoRoot) {
  wxFileName base_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_publish_move"));
  wxRemoveFile(base_dir.GetFullPath());
  const wxString root = base_dir.GetPath() + wxFileName::GetPathSeparator() +
                        wxT("chartdldr_publish_move_root");
  wxFileName::Mkdir(root, 0755, wxPATH_MKDIR_FULL);

  wxFileName temp_fn = wxFileName::CreateTempFileName(
      base_dir.GetPath() + wxFileName::GetPathSeparator() + wxT("dl"));
  const wxString temp_path = temp_fn.GetFullPath();
  {
    wxFile file(temp_path, wxFile::write);
    file.Write("chart");
  }

  EXPECT_TRUE(ChartDldrExtractCommon::PublishFileInto(root, wxT("nested/chart.000"),
                                                      temp_path));
  const wxString dest =
      root + wxFileName::GetPathSeparator() + wxT("nested") +
      wxFileName::GetPathSeparator() + wxT("chart.000");
  EXPECT_TRUE(wxFileExists(dest));
  EXPECT_EQ(ReadFileContents(dest), wxT("chart"));
  EXPECT_FALSE(wxFileExists(temp_path));

  wxFileName::Rmdir(root, wxPATH_RMDIR_RECURSIVE);
}
#endif  // _WIN32

TEST(ChartDldrTempDownload, FinalizeFailureKeepsTempAndPreservesOutput) {
  wxFileName output_dir =
      wxFileName::CreateTempFileName(wxT("chartdldr_temp_keep_temp"));
  wxRemoveFile(output_dir.GetFullPath());
  wxFileName::Mkdir(output_dir.GetPath(), 0755, wxPATH_MKDIR_FULL);

  const wxString output_path =
      output_dir.GetPath() + wxFileName::GetPathSeparator() + wxT("chart.zip");
  // Existing live path is a directory (with prior content). Rename/copy onto a
  // directory fails, so finalize must leave both the live path and temp alone.
  ASSERT_TRUE(wxFileName::Mkdir(output_path));
  const wxString marker_path =
      output_path + wxFileName::GetPathSeparator() + wxT("prior.txt");
  {
    wxFile file(marker_path, wxFile::write);
    file.Write("existing");
  }

  wxFileName temp_fn = wxFileName::CreateTempFileName(
      output_dir.GetPath() + wxFileName::GetPathSeparator() + wxT("dl"));
  const wxString temp_path = temp_fn.GetFullPath();
  {
    wxFile file(temp_path, wxFile::write);
    file.Write("replacement");
  }

  ChartDldrTempDownloadPaths paths;
  paths.output_path = output_path;
  paths.temp_path = temp_path;

  EXPECT_EQ(ChartDldrCompleteTempDownloadPaths(paths, true, false),
            OCPN_DL_FAILED);
  EXPECT_TRUE(wxDirExists(output_path));
  EXPECT_EQ(ReadFileContents(marker_path), wxT("existing"));
  EXPECT_TRUE(wxFileExists(temp_path));
  EXPECT_EQ(ReadFileContents(temp_path), wxT("replacement"));
  EXPECT_FALSE(wxFileExists(output_path + wxT(".publish-tmp")));

  ChartDldrRemoveTempDownload(temp_path);
  wxFileName::Rmdir(output_path, wxPATH_RMDIR_RECURSIVE);
  wxFileName::Rmdir(output_dir.GetPath(), wxPATH_RMDIR_RECURSIVE);
}
