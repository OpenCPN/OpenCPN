/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_archive_classify.h"

#include <wx/filename.h>

ChartDldrArchiveKind ChartDldrClassifyArchive(const wxString& path) {
  const wxString ext = wxFileName(path).GetExt().Lower();
  if (ext == wxT("zip")) {
    return ChartDldrArchiveKind::Zip;
  }
  if (ext == wxT("rar")) {
    return ChartDldrArchiveKind::Rar;
  }
  if (ext == wxT("tar") || ext == wxT("gz") || ext == wxT("gzip") ||
      ext == wxT("bz2") || ext == wxT("lzma") || ext == wxT("7z") ||
      ext == wxT("xz") || ext == wxT("tgz") || ext == wxT("tbz2")) {
    return ChartDldrArchiveKind::MultiFormat;
  }
  const wxString lower = path.Lower();
  if (lower.EndsWith(_T(".tar.gz")) || lower.EndsWith(_T(".tar.bz2")) ||
      lower.EndsWith(_T(".tar.xz"))) {
    return ChartDldrArchiveKind::MultiFormat;
  }
  return ChartDldrArchiveKind::Plain;
}
