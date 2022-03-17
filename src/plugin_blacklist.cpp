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
#include "plugin_blacklist.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <vector>

#include "logger.h"


// Work around gnu's major() and minor() macros
#ifdef major
#undef major
#undef minor
#endif


#ifdef _WIN32
static const char SEP = '\\';
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
} config_block;


static const config_block plugin_blacklist[] = {
  { "AIS Radar view",  0, 95, true},
  { "Radar",     0, 95, true},
  { "Watchdog",  1, 0,  true},
  { "squiddio",  0, 2,  true},
  { "ObjSearch", 0, 3 , true},
#ifdef __WXOSX__
  { "S63",       0, 6, true},
#endif
  { "oeSENC",    4, 2, true}
};

static std::string status2string(plug_status sts){
   std::string rval;
   switch (sts) {
       case plug_status::unblocked:  rval = "unblocked"; break;
       case plug_status::unloadable: rval = "unloadable"; break;
       case plug_status::hard: rval = "hard"; break;
       case plug_status::soft: rval = "soft"; break;
   }
   return rval;
}


/** Runtime representation of a plugin block. */
typedef struct block {
  int major;
  int minor;
  plug_status status;

  block()
    : major(0), minor(0), status(plug_status::unblocked) {};

  block(int _major, int _minor, bool _exact)
    : major(_major), minor(_minor), status(plug_status::unblocked) {};

  block(const struct config_block& cb)
    : major(cb.version_major), minor(cb.version_minor),
      status(cb.hard ? plug_status::hard : plug_status::soft) {};

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


static std::unordered_map<std::string, block>::iterator
  find_block(std::unordered_map<std::string, block>& blocks,
             const std::string& needle)
{
  const auto s = to_lower(needle);
  for (auto it = blocks.begin(); it != blocks.end(); it++) {
    if (to_lower(it->first) == s) return it;
  }
  return blocks.end();
}

class PlugBlacklist: public AbstractBlacklist {

friend std::unique_ptr<AbstractBlacklist> blacklist_factory();

private:
  PlugBlacklist() {
    constexpr int list_len = sizeof(plugin_blacklist)/sizeof(config_block);
    for (int i = 0; i < list_len; i += 1) {
      m_blocks[plugin_blacklist[i].name] = block(plugin_blacklist[i]);
    }
  }

  std::unordered_map<std::string, block> m_blocks;

public:
  virtual plug_data get_library_data(const std::string library_file) {
    std::string filename(normalize_lib(library_file));
    auto found = find_block(m_blocks, filename);
    if (found == m_blocks.end()) return plug_data("", -1, -1);
    return plug_data(found->first, found->second.major, found->second.minor);
  }



  void update_blocks(const std::string& name, int major, int minor) {
    if (m_blocks.find(name) == m_blocks.end())
      m_blocks[name] = block(major, minor, true);
    m_blocks[name].status = plug_status::unloadable;
    m_blocks[name].major = major;
    m_blocks[name].minor = minor;
  }

  plug_status get_status(const std::string& name, int major, int minor) {
    if (m_blocks.find(name) == m_blocks.end()) return plug_status::unblocked;
    const auto& b = m_blocks[name];
    return b.is_matching(major, minor) ? b.status : plug_status::unblocked;
  }

  plug_status get_status(const plug_data pd) {
    return get_status(pd.name, pd.major, pd.minor);
  }

  void mark_unloadable(const std::string& path) {
    auto filename(path);
    auto slashpos = filename.rfind(SEP);
    if (slashpos != std::string::npos)
      filename = filename.substr(slashpos + 1);
    update_blocks(filename, -1, -1);
  }

  bool is_loadable(const std::string path) {
    auto filename(path);
    auto slashpos = filename.rfind(SEP);
    if (slashpos != std::string::npos) filename = filename.substr(slashpos + 1);

    if (m_blocks.find(filename) == m_blocks.end()) return true;
    return m_blocks[filename].status != plug_status::unloadable;
  }

  void get_message(plug_status status, const plug_data& data) {

  }

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
