
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

#include "manual.h"
#include "manual_dlg.h"
#include "std_filesystem.h"

static const char* const kOnlineRoot = "https://opencpn-manuals.github.io/main";
static const char* const kOnlinePlugroot =
    "https://opencpn-manuals.github.io/main/opencpn-plugins";

static bool replace(std::string& str, const std::string& from,
                    const std::string& to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos) return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

Manual::Manual(const std::string& path)
    : entrypoints(
          {{Type::Toc,
            {"@LOCAL_ROOT@/doc/index.html", "@ONLINE_ROOT@/index.html"}},
           {Type::Chartdldr,
            {"@LOCAL_ROOT@/doc/plugin-dev-manual/chartdldr/index.html",
             "@ONLINE_PLUGROOT@/chart_downloader_tab/"
             "chart_downloader_tab.html"}},
           {Type::Wmm,
            {"@LOCAL_ROOT@/doc/plugin-dev-manual/wmm/index.html",
             "@ONLINE_PLUGROOT@/wmm/wmm.html"}},
           {Type::Dashboard,
            {"@LOCAL_ROOT@/doc/plugin-dev-manual/dashboard/index.html",
             "@ONLINE_PLUGROOT@/dashboard/dashboard.html"}},
           {Type::Grib,
            {"@LOCAL_ROOT@/doc/plugin-dev-manual/grib/index.html",
             "@ONLINE_PLUGROOT@/grib_weather/grib_weather.html"}}}),
      manual_datadir(path) {}

void Manual::Launch(Type type) {
  std::string path(entrypoints[type].first);
  replace(path, "@LOCAL_ROOT@", manual_datadir);
  if (fs::exists(fs::path(path))) {
    wxLaunchDefaultBrowser(path);
  } else {
    path = entrypoints[type].second;
    replace(path, "@ONLINE_PLUGROOT@", kOnlinePlugroot);
    replace(path, "@ONLINE_ROOT@", kOnlineRoot);
    ManualDlg dlg(path);
    dlg.ShowModal();
  }
}
