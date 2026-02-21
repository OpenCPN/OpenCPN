/**************************************************************************
 *   Copyright (C) 2024 by David S. Register                               *
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
 * Display utilities
 */

#ifndef OCPN_DISPLAYS_H
#define OCPN_DISPLAYS_H

#include <string>
#include <vector>

///  Structure to hold information about a monitor
struct OCPN_MonitorInfo {
  ///  Name of the monitor
  std::string name;

  ///  Width of the monitor in millimeters
  size_t width_mm;

  ///  Height of the monitor in millimeters
  size_t height_mm;

  ///  Width of the monitor in pixels
  size_t width;

  ///  Height of the monitor in pixels
  size_t height;

  ///  Width of the monitor in physical pixels, on eg. Apple Retina
  /// displays this value differs
  size_t width_px;

  ///  Height of the monitor in physical pixels, on eg. Apple Retina
  /// displays this value differs
  size_t height_px;

  ///  Scaling factor in percent
  size_t scale;
};

///  Information about the monitors connected to the system
extern std::vector<OCPN_MonitorInfo> g_monitor_info;

///  Enumerate the monitors connected to the system
void EnumerateMonitors();

#endif  // OCPN_DISPLAYS_H
