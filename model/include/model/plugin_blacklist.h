/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Plugin blacklist for plugins which can or should not be loaded
 * Author:   Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by Alec Leamas                                     *
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

#ifndef PLUGIN_BLACKLIST_H
#define PLUGIN_BLACKLIST_H

#include <string>
#include <sstream>
#include <memory>

// Handle gnu's major/minor macros
#ifdef major
#define gnu_major major
#define gnu_minor minor
#undef major
#undef minor
#endif

typedef enum class plug_status {
  unblocked,  /** Not blocked for any reason */
  unloadable, /** Tried with load error */
  hard,       /** Hard block from code or configuration */
  soft        /** Soft block: load with a warning. */
} plug_status;

typedef struct plug_data {
  std::string name;
  int major;
  int minor;

  plug_data(std::string n, int _major, int _minor)
      : name(n), major(_major), minor(_minor) {}

} plug_data;

/**
 * Plugins could be blacklisted in runtime if they are unloadable or in
 * hardcoded, compile-time list.
 *
 * Unloadable plugins are blocked using the complete filename (basename).
 *
 * Loadable plugins are blacklisted using the official name and a version,
 * possibly covering also all older versions.
 */
class AbstractBlacklist {
public:
  virtual ~AbstractBlacklist() = default;

  /** Return status for given official plugin name and version. */
  virtual plug_status get_status(const std::string& name, int _major,
                                 int _minor) = 0;

  /** Return status for given official plugin name and version. */
  virtual plug_status get_status(const plug_data pd) = 0;

  /** Best effort attempt to get data for a library file. */
  virtual plug_data get_library_data(const std::string& library_file) = 0;

  /**
   *  Given a path, mark filename as unloadable.
   *  @return true if filename was already marked, else false.
   */
  virtual bool mark_unloadable(const std::string& path) = 0;

  /**
   *  Given plugin name and version mark it as unloadable.
   *  @return true if plugin was already marked, else false.
   **/
  virtual bool mark_unloadable(const std::string& name, int major,
                               int minor) = 0;

  /** Return true iff plugin (a path) is loadable. */
  virtual bool is_loadable(const std::string path) = 0;

  /** Return plugin-specific message, possibly "". */
  virtual std::string get_message(plug_status sts, const plug_data& data) = 0;
};

std::unique_ptr<AbstractBlacklist> blacklist_factory();

#ifdef gnu_major
#define major gnu_major
#define minor gnu_minor
#endif

#endif  // PLUGIN_BLACKLIST_H
