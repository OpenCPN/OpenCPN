/***************************************************************************
 *   Copyright (C) 2025 Alec Leamas                                        *
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
 * Implement filters_on_disk.h -- Data Monitor filter storage routines
 */

#include <fstream>

#include "model/filters_on_disk.h"
#include "model/base_platform.h"
#include "std_filesystem.h"

extern BasePlatform* g_BasePlatform;

static fs::path UserPath() {
  auto userdir = g_BasePlatform->GetPrivateDataDir().ToStdString();
  fs::path path(userdir);
  path /= "filters";
  if (!fs::exists(path)) fs::create_directories(path);
  return path;
}

static fs::path SystemPath() {
  auto systemdir = g_BasePlatform->GetSharedDataDir().ToStdString();
  fs::path path(systemdir);
  path /= "filters";
  assert(fs::exists(path) && "System filters not found");
  return path;
}

namespace filters_on_disk {

std::vector<std::string> List(bool include_system) {
  std::vector<std::string> v;
  for (const auto& entry : fs::directory_iterator(UserPath()))
    v.push_back(entry.path().filename().string());
  if (include_system) {
    for (const auto& entry : fs::directory_iterator(SystemPath()))
      v.push_back(entry.path().stem().string());
  }
  for (auto& filter : v) filter = fs::path(filter).stem().string();
  return v;
}

bool Exists(const std::string& name) {
  const std::string filename = name + ".json";
  if (fs::exists(UserPath() / filename)) return true;
  return fs::exists(SystemPath() / filename);
}

bool Remove(const std::string& name) {
  const std::string filename = name + ".json";
  fs::path path(UserPath() / filename);
  fs::remove(path);
  return !fs::exists(path);
}

bool Rename(const std::string& old_name, const std::string& new_name) {
  const std::string old_filename = old_name + ".json";
  fs::path old_path(UserPath() / old_filename);
  if (!fs::exists(old_path)) return false;
  const std::string new_filename = new_name + ".json";
  fs::path new_path(UserPath() / new_filename);
  try {
    fs::rename(old_path, new_path);
  } catch (fs::filesystem_error& e) {
    return false;
  }
  return !fs::exists(old_path) && fs::exists(new_path);
}

bool Write(const NavmsgFilter& filter, const std::string& name) {
  std::string json = filter.to_string();
  const std::string filename = name + ".json";
  fs::path path(UserPath() / filename);
  std::ofstream stream(path);
  if (!stream.is_open()) return false;
  stream << json;
  return true;
}

NavmsgFilter Read(const std::string& name) {
  const std::string filename = name + ".json";
  fs::path path(UserPath() / filename);
  if (!fs::exists(path)) path = SystemPath() / filename;
  std::ifstream stream(path);
  if (stream.bad()) return NavmsgFilter();
  std::stringstream ss;
  ss << stream.rdbuf();
  return NavmsgFilter::Parse(ss.str());
}

}  // namespace filters_on_disk
