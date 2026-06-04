/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_PREFS_TIME_H_
#define CHARTDLDR_PREFS_TIME_H_

#include <wx/string.h>

class wxStaticText;
class wxTextCtrl;
class wxWindow;

bool ChartDldrParseScheduledTimeEntry(const wxString& text, int& hour,
                                      int& minute);

wxString ChartDldrFormatScheduledTimeEntry(int hour, int minute);

/** Locale preview label, or empty when preview should be hidden (24-hour). */
wxString ChartDldrFormatScheduledTimePreview(int hour, int minute);

bool ChartDldrShowsScheduledTimePreview();

void ChartDldrUpdateScheduledTimePreviewWidgets(wxTextCtrl* entry,
                                                wxStaticText* preview);

#endif  // CHARTDLDR_PREFS_TIME_H_
