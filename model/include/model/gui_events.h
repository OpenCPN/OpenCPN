/**************************************************************************
 *   Copyright (C) 2025  Alec Leamas                                       *
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
 *  Misc GUI event vars, a singleton.
 */

#ifndef GUI_EVENTS_H
#define GUI_EVENTS_H

#include "observable_evtvar.h"

/** EventVar exchange point, a singleton. */
class GuiEvents {
public:
  static GuiEvents& GetInstance() {
    static GuiEvents instance;
    return instance;
  }

  GuiEvents(const GuiEvents&) = delete;
  GuiEvents& operator=(const GuiEvents&) = delete;

  /**
   * Notified when the day/dusk/night color scheme changes. GetInt() returns
   * a boolean "day" value, true if the new scheme uses dark text on light
   * background.
   */
  EventVar color_scheme_change;

  /**
   * Notified when the top level status bas should be updated by gFrame
   */
  EventVar gframe_update_status_bar;

  /**
   * Notified with a shared_ptr<const AisTargetData> when gFrame should center
   * the given AIS target
   */
  EventVar on_center_ais_target;

  /**
   * Notified without arguments when the global variables housing the Data
   * Monitor user defined colors are updated.
   */
  EventVar on_dm_colors_change;

  /** Notified when list of routes is updated (no data in event) */
  EventVar on_routes_update;

  /** Notified when waypoint(s) is updated (no data in event) */
  EventVar on_waypoint_update;

  /** Notified when Chart canvases need a full reload, no argsuments */
  EventVar on_reload_charts;

private:
  GuiEvents() = default;
};

#endif  //  GUI_EVENTS_H
