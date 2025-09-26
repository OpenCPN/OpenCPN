/***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 * WaypointMan drawing stuff
 */

#ifndef WAYPOINT_GUI_H_
#define WAYPOINT_GUI_H_

#include "model/routeman.h"
#include "styles.h"

class WayPointmanGui {
public:
  WayPointmanGui(WayPointman &waypoint_man) : m_waypoint_man(waypoint_man) {}

  void SetColorScheme(ColorScheme cs, double displayDPmm);
  void ReloadAllIcons(double displayDPmm);
  void ProcessDefaultIcons(double displayDPmm);
  void ProcessIcons(ocpnStyle::Style *style, double displayDPmm);
  void ProcessUserIcons(ocpnStyle::Style *style, double displayDPmm);
  MarkIcon *ProcessIcon(wxImage image, const wxString &key,
                        const wxString &description, bool add_in_front = false);
  unsigned int GetIconTexture(const wxBitmap *pmb, int &glw, int &glh);

  void ReloadRoutepointIcons();

private:
  MarkIcon *ProcessLegacyIcon(wxString fileName, const wxString &key,
                              const wxString &description, double displayDPmm);
  MarkIcon *ProcessExtendedIcon(wxImage &image, const wxString &key,
                                const wxString &description);
  wxRect CropImageOnAlpha(wxImage &image);
  WayPointman &m_waypoint_man;
};

#endif  // WAYPOINT_GUI_H_
