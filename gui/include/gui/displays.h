/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Display utilities
 * Author:   David Register
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef OCPN_DISPLAYS_H
#define OCPN_DISPLAYS_H

#include <string>
#include <vector>

/// @brief Structure to hold information about a monitor
struct OCPN_MonitorInfo {
  /// @brief Name of the monitor
  std::string name;
  /// @brief Width of the monitor in millimeters
  size_t width_mm;
  /// @brief Height of the monitor in millimeters
  size_t height_mm;
  /// @brief Width of the monitor in pixels
  size_t width;
  /// @brief Height of the monitor in pixels
  size_t height;
  /// @brief Width of the monitor in physical pixels, on eg. Apple Retina displays this value differs
  size_t width_px;
  /// @brief Height of the monitor in physical pixels, on eg. Apple Retina displays this value differs
  size_t height_px;
  /// @brief Scaling factor in percent
  size_t scale;
};

/// @brief Number of monitors connected to the system
extern size_t g_num_monitors;
extern size_t g_current_monitor;
/// @brief Information about the monitors connected to the system  
extern std::vector<OCPN_MonitorInfo> g_monitor_info;

/// @brief Enumerate the monitors connected to the system
void EnumerateMonitors();

#endif  // OCPN_DISPLAYS_H
