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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef __TRACKPRINTOUT_H__
#define __TRACKPRINTOUT_H__

#include <wx/print.h>
#include <wx/datetime.h>
#include <wx/cmdline.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "dialog_input.h"
#include "model/ocpn_types.h"
#include "navutil.h"
#include "TrackPropDlg.h"
#include "printout_base.h"
#include "printtable.h"
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
class TrackPrintDialog : public InputDialog {
public:
  TrackPrintDialog(wxWindow* parent, const std::set<int>& options)
      : InputDialog(parent, _("Print Track").ToStdString()) {
    AddSelection(options, TrackPrintOptions::kTrackPosition,
                 _("Print Track Position").ToStdString());
    AddSelection(options, TrackPrintOptions::kTrackCourse,
                 _("Print Track Course").ToStdString());
    AddSelection(options, TrackPrintOptions::kTrackDistance,
                 _("Print Track Distance").ToStdString());
    AddSelection(options, TrackPrintOptions::kTrackTime,
                 _("Print Track Time").ToStdString());
    AddSelection(options, TrackPrintOptions::kTrackSpeed,
                 _("Print Track Speed").ToStdString());
  };
};

/**
 * Printout a table with track selected information.
 */
class TrackPrintout : public BasePrintout {
public:
  /**
   * Create track printout.
   * @param track Track to print.
   * @param options Selected print options.
   */
  TrackPrintout(Track* track, OCPNTrackListCtrl* lcPoints,
                std::set<int> options);

  void OnPreparePrinting() override;

protected:
  PrintTable m_table;
  Track* m_track;

  int m_text_offset_x;
  int m_text_offset_y;

  void DrawPage(wxDC* dc, int page) override;
};

#endif
