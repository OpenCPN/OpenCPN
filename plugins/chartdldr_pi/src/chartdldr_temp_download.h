/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_TEMP_DOWNLOAD_H_
#define CHARTDLDR_TEMP_DOWNLOAD_H_

#include "ocpn_plugin.h"

#include <wx/string.h>

class wxFileName;

/** Resolved paths for a background download via temp file. */
struct ChartDldrTempDownloadPaths {
  wxString download_target;
  wxString temp_path;
  wxString output_path;
};

/** Background download target URI/path for a local filesystem path. */
wxString ChartDldrTempDownloadBackgroundTargetPath(const wxFileName& output_fn);

ChartDldrTempDownloadPaths ChartDldrTempDownloadPathsFor(
    const wxString& output_path);

/**
 * Publish temp download to the final path.
 * Prefers atomic rename; falls back to sibling copy+rename. On failure the
 * download temp is left intact and existing output is left untouched.
 */
bool ChartDldrFinalizeTempDownload(const wxString& temp_path,
                                   const wxString& output_path);

void ChartDldrRemoveTempDownload(const wxString& temp_path);

/** Optional gate: temp must pass before publish (catalog XML validation). */
using ChartDldrTempPublishGate = bool (*)(const wxString& temp_path);

/** Finalize or discard a temp download and map to transfer status. */
_OCPN_DLStatus ChartDldrCompleteTempDownloadPaths(
    const ChartDldrTempDownloadPaths& paths, bool transfer_success,
    bool cancelled, ChartDldrTempPublishGate publish_gate = nullptr);

#endif  // CHARTDLDR_TEMP_DOWNLOAD_H_
