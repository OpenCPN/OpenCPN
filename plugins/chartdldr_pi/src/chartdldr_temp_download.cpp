/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_temp_download.h"

#include <wx/filename.h>

namespace {

_OCPN_DLStatus ResolveTempDownloadStatus(bool transfer_success, bool cancelled,
                                         bool finalize_ok) {
  if (!transfer_success || cancelled) {
    return cancelled ? OCPN_DL_ABORTED : OCPN_DL_FAILED;
  }
  return finalize_ok ? OCPN_DL_NO_ERROR : OCPN_DL_FAILED;
}

wxString ChartDldrTempFilesystemPath(const wxString& output_path) {
  wxFileName tfn = wxFileName::CreateTempFileName(output_path);
  return tfn.GetFullPath();
}

bool PublishViaSiblingTemp(const wxString& temp_path,
                           const wxString& output_path) {
  const wxString publish_tmp = output_path + wxT(".publish-tmp");
  if (wxFileExists(publish_tmp) && !wxRemoveFile(publish_tmp)) {
    return false;
  }
  if (!wxCopyFile(temp_path, publish_tmp)) {
    if (wxFileExists(publish_tmp)) {
      wxRemoveFile(publish_tmp);
    }
    return false;
  }
  if (!wxRenameFile(publish_tmp, output_path, true /*overwrite*/)) {
    if (wxFileExists(publish_tmp)) {
      wxRemoveFile(publish_tmp);
    }
    return false;
  }
  wxRemoveFile(temp_path);
  return true;
}

}  // namespace

wxString ChartDldrTempDownloadBackgroundTargetPath(
    const wxFileName& output_fn) {
#ifdef __ANDROID__
  wxString file_uri = output_fn.GetFullPath();
  if (!file_uri.StartsWith(_T("file://"))) {
    file_uri = _T("file://") + output_fn.GetFullPath();
  }
  return file_uri;
#else
  return output_fn.GetFullPath();
#endif
}

ChartDldrTempDownloadPaths ChartDldrTempDownloadPathsFor(
    const wxString& output_path) {
  ChartDldrTempDownloadPaths paths;
  paths.output_path = output_path;
  paths.temp_path = ChartDldrTempFilesystemPath(output_path);
  paths.download_target =
      ChartDldrTempDownloadBackgroundTargetPath(wxFileName(paths.temp_path));
  return paths;
}

bool ChartDldrFinalizeTempDownload(const wxString& temp_path,
                                   const wxString& output_path) {
  if (temp_path.empty() || temp_path == output_path) {
    return true;
  }
  if (!wxFileExists(temp_path)) {
    return false;
  }
  // Never publish a downloaded file over a directory path.
  if (wxDirExists(output_path)) {
    return false;
  }

  // Prefer atomic replace when temp and output share a filesystem.
  if (wxRenameFile(temp_path, output_path, true /*overwrite*/)) {
    return true;
  }

  // Cross-device fallback: copy to a sibling temp next to the destination,
  // then rename that over the live path. Leave the download temp intact on
  // any failure so the caller can retry without clobbering existing output.
  return PublishViaSiblingTemp(temp_path, output_path);
}

void ChartDldrRemoveTempDownload(const wxString& temp_path) {
  if (!temp_path.empty() && wxFileExists(temp_path)) {
    wxRemoveFile(temp_path);
  }
}

_OCPN_DLStatus ChartDldrCompleteTempDownloadPaths(
    const ChartDldrTempDownloadPaths& paths, bool transfer_success,
    bool cancelled, ChartDldrTempPublishGate publish_gate) {
  bool publish_ok = true;
  if (transfer_success && !cancelled) {
    if (publish_gate && !publish_gate(paths.temp_path)) {
      ChartDldrRemoveTempDownload(paths.temp_path);
      publish_ok = false;
    } else {
      publish_ok =
          ChartDldrFinalizeTempDownload(paths.temp_path, paths.output_path);
    }
  } else {
    ChartDldrRemoveTempDownload(paths.temp_path);
  }
  return ResolveTempDownloadStatus(transfer_success, cancelled, publish_ok);
}
