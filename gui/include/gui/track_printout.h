/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2025 by NoCodeHummel                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Track print dialog
 */

#ifndef TRACKPRINTOUT_H_
#define TRACKPRINTOUT_H_

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "navutil.h"
#include "printout_base.h"
#include "printtable.h"
#include "track_prop_dlg.h"
#include "ui_utils.h"

enum class TrackPrintOptions {
  kTrackPosition,
  kTrackCourse,
  kTrackDistance,
  kTrackTime,
  kTrackSpeed
};

/**
 * Input dialog with track print selection.
 */
class TrackPrintDlg : public wxDialog {
public:
  explicit TrackPrintDlg(wxWindow* parent);

  bool IsEnabled(TrackPrintOptions option) const;

private:
  std::unordered_map<TrackPrintOptions, int> IdByOption;
};

/**
 * Printout a table with track selected information.
 */
class TrackPrintout : public BasePrintout {
public:
  /**
   * Create track printout.
   * @param track Track to print.
   * @param dlg A TrackPrintDlg initiated by ShowModal().
   */
  TrackPrintout(Track* track, OCPNTrackListCtrl* lcPoints,
                const TrackPrintDlg& dlg);

  void OnPreparePrinting() override;

protected:
  PrintTable m_table;
  Track* m_track;
  int m_text_offset_x;
  int m_text_offset_y;

  void DrawPage(wxDC* dc, int page) override;
};

#endif
