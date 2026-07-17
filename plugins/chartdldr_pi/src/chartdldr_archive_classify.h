/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_ARCHIVE_CLASSIFY_H_
#define CHARTDLDR_ARCHIVE_CLASSIFY_H_

#include <wx/string.h>

enum class ChartDldrArchiveKind { Zip, Rar, MultiFormat, Plain };

ChartDldrArchiveKind ChartDldrClassifyArchive(const wxString& path);

#endif  // CHARTDLDR_ARCHIVE_CLASSIFY_H_
