
/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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

#ifndef MANUAL_H_
#define MANUAL_H_

#include <string>
#include <unordered_map>

class Manual {
public:
  /** Manual entry points. */
  enum class Type { Toc, Chartdldr, Wmm, Dashboard, Grib };

  /**
   * Construct a Manual object.
   *
   * @param path  Manual plugin data directory as returned by
   *              GetPluginDataDir("manual")
   */
  Manual(const std::string& path);

  /**
   * Launch a web browser displaying a manual entry point. If the manual
   * plugin is installed use that, otherwise offer user to use the on-line
   * manual.
   *
   * @param type Entry point to display.
   */
  void Launch(Type type);

private:
  std::unordered_map<Type, std::pair<std::string, std::string>> entrypoints;
  std::string manual_datadir;
};

#endif  // MANUAL_H_
