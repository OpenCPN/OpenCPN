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
#include "model/plugin_blacklist.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <unordered_map>
#include <vector>

#include <wx/translation.h>
#include <wx/log.h>

#include "model/logger.h"


// Work around gnu's major() and minor() macros
#ifdef major
#undef major
#undef minor
#endif


#ifdef _WIN32
static constexpr char SEP = '\\';
#else
static const char SEP = '/';
#endif

/** Hardcoded representation of a blocked plugin. */
typedef struct config_block {
  const char* name;      /** Official plugin name as of GetCommonName(). */
  int version_major;
  int version_minor;
  bool hard;             /** If true, unconditional hard block; else load
                             plugin with a warning.  */
  const char* message;
} config_block;

static const char* const STD_HARD_MSG = _(R"(
PlugIn %s, version %i.%i was detected.
This version is known to be unstable and will not be loaded.
Please update this PlugIn using the PlugIn manager master catalog.
)");

static const char* const STD_SOFT_MSG = _(R"(
PlugIn %s, version %i.%i was detected.
This version is known to be unstable.
Please update this PlugIn using the PlugIn manager master catalog.
)");

static const char* const OCHART_OBSOLETED_MSG = _(R"(
PlugIn %s, version %i.%i was detected.
This plugin is obsolete, the o-charts plugin should be used
instead. Please uninstall this plugin and install o-charts
using the PlugIn manager master catalog.
)");


static const config_block plugin_blacklist[] = {
  { "Radar",     0, 95, true, STD_HARD_MSG},
  { "Watchdog",  1, 0,  true, STD_HARD_MSG},
  { "squiddio",  0, 2,  true, STD_HARD_MSG},
  { "ObjSearch", 0, 3,  true, STD_HARD_MSG},
#ifdef __WXOSX__
  { "S63",       0, 6,  true, STD_HARD_MSG},
#endif
  { "oeSENC",   99, 99, true, OCHART_OBSOLETED_MSG},
  { "oernc_pi", 99, 99, true, OCHART_OBSOLETED_MSG},
  { "oesenc_pi", 99, 99, true, OCHART_OBSOLETED_MSG}
};


/** Runtime representation of a plugin block. */
typedef struct block {
  int major;
  int minor;
  plug_status status;
  const char* message;

  block()
    : major(0), minor(0), status(plug_status::unblocked), message("") {};

  block(int _major, int _minor)
    : major(_major), minor(_minor), status(plug_status::unblocked),
      message("")
    {};

  block(const struct config_block& cb)
    : major(cb.version_major), minor(cb.version_minor),
      status(cb.hard ? plug_status::hard : plug_status::soft),
      message(cb.message)
    {};

  /** Return true if _major/_minor matches the blocked plugin. */
  bool is_matching(int _major, int _minor) const {
    if (major == -1) return true;
    if (_major == -1) return false;
    if (_major > major) return false;
    if (_minor > minor) return false;
    return true;
  };

  plug_data to_plug_data(std::string name) {
    return plug_data(name, major, minor);
  }

} block;


/** Drop possible directory, unix-style lib prefix and extension suffix. */
static inline std::string normalize_lib(const std::string& name) {
  auto libname(name);
  auto slashpos = libname.rfind(SEP);
  if (slashpos != std::string::npos) libname = libname.substr(slashpos + 1);
#if defined(__WXGTK__) || defined(__WXOSX__)
  if (libname.find("lib") == 0) libname = libname.substr(3);
#endif
  auto dotpos = libname.rfind('.');
  if (dotpos != std::string::npos) libname = libname.substr(0, dotpos);
  return libname;
}

static std::string to_lower(const std::string& arg) {
  std::string s(arg);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c){ return std::tolower(c); });
  return s;
}


class PlugBlacklist: public AbstractBlacklist {

friend std::unique_ptr<AbstractBlacklist> blacklist_factory();

typedef std::unordered_map<std::string, block> block_map;

private:
  PlugBlacklist() {
    constexpr int list_len = sizeof(plugin_blacklist)/sizeof(config_block);
    for (int i = 0; i < list_len; i += 1) {
      m_blocks[plugin_blacklist[i].name] = block(plugin_blacklist[i]);
    }
  }

  block_map m_blocks;

  block_map::iterator find_block(const std::string& name) {
    const auto s = to_lower(name);
    for (auto it = m_blocks.begin(); it != m_blocks.end(); it++) {
      if (to_lower(it->first) == s) return it;
    }
    return m_blocks.end();
  }

  bool update_block(const std::string& name, int major, int minor) {
    bool  new_block = false;
    if (m_blocks.find(name) == m_blocks.end()) {
      m_blocks[name] = block(major, minor);
      new_block = true;
    }
    m_blocks[name].status = plug_status::unloadable;
    m_blocks[name].major = major;
    m_blocks[name].minor = minor;
    return new_block;
  }

  /** Avoid pulling in wx libraries in low-level model code. */
  std::string format_message(const std::string msg, const  plug_data& data) {
    int size = std::snprintf(nullptr, 0, msg.c_str(),
                             data.name.c_str(), data.major, data.minor);
    if (size < 0) {
      wxLogWarning("Cannot format message for %s", data.name.c_str());
      return "Internal error: Cannot format message(!)";
    }
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, msg.c_str(),
                  data.name.c_str(), data.major, data.minor);
    return std::string(buf.get(), buf.get() + size - 1);
  }

public:

  virtual plug_data get_library_data(const std::string& library_file) {
    std::string filename(normalize_lib(library_file));
    auto found = find_block(filename);
    if (found == m_blocks.end()) return plug_data("", -1, -1);
    return plug_data(found->first, found->second.major, found->second.minor);
  }

  plug_status get_status(const std::string& name, int major, int minor) {
    if (m_blocks.find(name) == m_blocks.end()) return plug_status::unblocked;
    const auto& b = m_blocks[name];
    return b.is_matching(major, minor) ? b.status : plug_status::unblocked;
  }

  plug_status get_status(const plug_data pd) {
    return get_status(pd.name, pd.major, pd.minor);
  }

  virtual bool mark_unloadable(const std::string& name,
		               int major, int minor) {
    return update_block(name, major, minor);
  }

  /** Given a path, mark filename as unloadable. */
  bool mark_unloadable(const std::string& path) {
    auto filename(path);
    auto slashpos = filename.rfind(SEP);
    if (slashpos != std::string::npos)
      filename = filename.substr(slashpos + 1);
    return update_block(filename, -1, -1);
  }

  bool is_loadable(const std::string path) {
    auto filename(path);
    auto slashpos = filename.rfind(SEP);
    if (slashpos != std::string::npos) filename = filename.substr(slashpos + 1);

    if (m_blocks.find(filename) == m_blocks.end()) return true;
    return m_blocks[filename].status != plug_status::unloadable;
  }

// gcc 12 bogus regex warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
  std::string get_message(plug_status status, const plug_data& data) {
    if (status == plug_status::unloadable) {
      std::string msg(_("Plugin library %s can not be loaded"));
      msg = std::regex_replace(msg, std::regex("%s"), data.name);
      return msg;
    }
    if (status == plug_status::unblocked) {
      wxLogMessage("Attempt to get message for unblocked plugin %s",
                   data.name.c_str());
      return "No applicable message";
    }
    auto found = find_block(data.name);
    if (found == m_blocks.end())
      return format_message("No known message for %s version %d.%d", data);
    else
      return format_message(found->second.message, data);
  }
#pragma GCC diagnostic pop
};


std::unique_ptr<AbstractBlacklist> blacklist_factory() {
  return std::unique_ptr<AbstractBlacklist>(new PlugBlacklist());
};

#ifdef BLACKLIST_TEST
//    $ export CPPFLAGS="-g -I../include -DBLACKLIST_TEST -D__WXGTK__"
//    $ make plugin_blacklist
//    $ ./plugin_blacklist aisradar_pi 0 96
//    unblocked

#include <iostream>
int main(int argc, char** argv) {

  const std::string name(argv[1]);
  int major = atoi(argv[2]);
  int minor = atoi(argv[3]);
  auto blacklist = blacklist_factory();
  blacklist->mark_unloadable("foo");
  auto s = blacklist->get_status(name, major, minor);
  switch (s) {
      case plug_status::unloadable: std::cout << "unloadable\n"; break;
      case plug_status::unblocked: std::cout << "unblocked\n"; break;
      case plug_status::hard: std::cout << "hard\n"; break;
      case plug_status::soft: std::cout << "soft\n"; break;
  }
  auto lib = blacklist->plugin_by_libname(name);
  std::cout << "found plugin: \"" << lib.name << "\" version: " << lib.major
      << "." << lib.minor << "\n";
  exit(0);
}

#endif    // BLACKLIST_TEST
