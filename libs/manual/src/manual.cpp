
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

/** \file manual.cpp Implement manual.h */

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <wx/log.h>
#include <wx/jsonreader.h>
#include <wx/string.h>

#include "manual.h"
#include "manual_dlg.h"
#include "std_filesystem.h"

static const char* const kOnlineRoot = "https://opencpn-manuals.github.io/main";
static const char* const kOnlinePlugroot =
    "https://opencpn-manuals.github.io/main/opencpn-plugins";
static const char* const kWikiRoot =
    "https://opencpn.org/wiki/dokuwiki/doku.php";


static const std::unordered_map<std::string, std::string> kOnlineEntries = {
  {"Toc", "@ONLINE_ROOT@/index.html"},
  {"Chartdldr",
     "@ONLINE_PLUGROOT@/chart_downloader_tab/chart_downloader_tab.html"},
  {"Wmm",  "@ONLINE_PLUGROOT@/wmm/wmm.html"},
  {"Dashboard", "@ONLINE_PLUGROOT@/dashboard/dashboard.html"},
  {"Grib", "@ONLINE_PLUGROOT@/grib_weather/grib_weather.html"},
  {"Hotkeys",
    "@WIKI_ROOT@?id=opencpn:manual_basic:ui_user_interface:keyboard_shortcuts" }
};

static bool replace(std::string& str, const std::string& from,
                    const std::string& to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos) return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

Manual::Manual(const std::string& path) : m_datadir(path) {
  auto datadir_path(fs::path(m_datadir) / "data" / "entrypoints.json");
  if (!fs::exists(datadir_path)) {
    wxLogDebug("Manual plugin is not installed");
    return;
  }
  std::ifstream stream(datadir_path.string());
  std::stringstream ss;
  ss << stream.rdbuf();
  wxJSONReader reader;
  int err_count = reader.Parse(ss.str(), &m_root);
  if (err_count != 0) {
    wxLogWarning("Cannot parse entrypoints.json from manual plugin");
  }
}

bool Manual::Launch(const std::string& entrypoint) {
  std::string path(m_root[entrypoint].AsString());
  replace(path, "@LOCAL_ROOT@", m_datadir);
  if (fs::exists(fs::path(path))) {
    wxLaunchDefaultBrowser(path);
    return true;
  }
  auto found = kOnlineEntries.find(entrypoint);
  if (found == kOnlineEntries.end()) {
    wxLogWarning("Using illegal manual entry point: %s", entrypoint.c_str());
    return false;
  }
  path = found->second;
  replace(path, "@ONLINE_PLUGROOT@", kOnlinePlugroot);
  replace(path, "@ONLINE_ROOT@", kOnlineRoot);
  replace(path, "@WIKI_ROOT@", kWikiRoot);
  ManualDlg dlg(path);
  dlg.ShowModal();
  return true;
}
