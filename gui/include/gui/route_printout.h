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
#ifndef __ROUTEPRINTOUT_H__
#define __ROUTEPRINTOUT_H__

#include <set>
#include <wx/print.h>
#include <wx/datetime.h>
#include <wx/cmdline.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "dialog_input.h"
#include "model/ocpn_types.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "printout_base.h"
#include "printtable.h"
#include "ui_utils.h"

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
class RoutePrintDialog : public InputDialog {
public:
  RoutePrintDialog(wxWindow* parent, const std::set<int>& options)
      : InputDialog(parent, _("Print Route").ToStdString()) {
    AddSelection(options, RoutePrintOptions::kWaypointName,
                 _("Print Waypoint Name").ToStdString());
    AddSelection(options, RoutePrintOptions::kWaypointPosition,
                 _("Print Waypoint Position").ToStdString());
    AddSelection(options, RoutePrintOptions::kWaypointCourse,
                 _("Print Waypoint Course to Next").ToStdString());
    AddSelection(options, RoutePrintOptions::kWaypointDistance,
                 _("Print Waypoint Distance to Next").ToStdString());
    AddSelection(options, RoutePrintOptions::kWaypointSpeed,
                 _("Waypoint Leg Speed").ToStdString());
    AddSelection(options, RoutePrintOptions::kWaypointETA,
                 _("Waypoint Estimated Time Arrival").ToStdString());
    AddSelection(options, RoutePrintOptions::kWaypointETD,
                 _("Waypoint Estimated Time Departure").ToStdString());
    AddSelection(options, RoutePrintOptions::kWaypointTideEvent,
                 _("Waypoint Next Tide Event").ToStdString());
    AddSelection(options, RoutePrintOptions::kWaypointDescription,
                 _("Print Waypoint Description").ToStdString());
  };
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
   * @param options Selected print options.
   * @param tz_selection Timezone selection.
   */
  RoutePrintout(Route* route, const std::set<int>& options,
                const int tz_selection);

  void OnPreparePrinting() override;

protected:
  PrintTable m_table;
  Route* m_route;

  int m_text_offset_x;
  int m_text_offset_y;

  void DrawPage(wxDC* dc, int page) override;
};

#endif
