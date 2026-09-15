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
 * Route print dialog
 */

#ifndef ROUTEPRINTOUT_H_
#define ROUTEPRINTOUT_H_

#include <set>

#include <wx/print.h>
#include <wx/window.h>

#include <wx/dc.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "libgui/ui_utils.h"

#include "printout_base.h"
#include "printtable.h"

enum class RoutePrintOptions {
  kWaypointName,
  kWaypointPosition,
  kWaypointCourse,
  kWaypointDistance,
  kWaypointDescription,
  kWaypointSpeed,
  kWaypointETA,
  kWaypointETD,
  kWaypointTideEvent
};

/**
 * Input dialog with route print selection.
 */
class RoutePrintDlg : public wxDialog {
public:
  RoutePrintDlg(wxWindow* parent);
  bool IsEnabled(RoutePrintOptions option) const;

private:
  std::unordered_map<RoutePrintOptions, int> IdByOption;
};

/**
 * Printout route information and a table with
 * selected route point information.
 */
class RoutePrintout : public BasePrintout {
public:
  /**
   * Create route prinout.
   * @param route Route to print.
   * @param dlg Selected print options after running ShowModal().
   * @param tz_selection Timezone selection.
   */
  RoutePrintout(Route* route, const RoutePrintDlg& dlg, const int tz_selection);

  void OnPreparePrinting() override;

protected:
  PrintTable m_table;
  Route* m_route;

  int m_text_offset_x;
  int m_text_offset_y;

  void DrawPage(wxDC* dc, int page) override;
};

#endif
