/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_temp_download.h"

#include "chartdldr_extract_common.h"

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

  // Publish the leaf into its containing directory: prefers atomic rename,
  // falls back to a sibling copy+rename across filesystems. Never overwrites a
  // directory; leaves the download temp intact on any failure.
  const wxFileName out_fn(output_path);
  return ChartDldrExtractCommon::PublishFileInto(
      out_fn.GetPath(), out_fn.GetFullName(), temp_path);
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
