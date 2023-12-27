/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
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
 ***************************************************************************
 */

#include <fstream>

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/filefn.h>

#include "model/base_platform.h"
#include "model/ocpn_utils.h"
#include "model/plugin_cache.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

extern BasePlatform* g_BasePlatform;

static std::string cache_path() {
  wxFileName path;
  path.AssignDir(g_BasePlatform->GetPrivateDataDir());
  path.AppendDir("plugins");
  path.AppendDir("cache");
  return path.GetFullPath().ToStdString();
}

static std::string tarball_path(const char* basename, bool create = false) {
  wxFileName dirs(cache_path());
  dirs.AppendDir("tarballs");
  if (create) {
    dirs.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
  }
  static const auto kSeparator = wxFileName::GetPathSeparator();
  wxFileName path(dirs.GetFullPath() + kSeparator +  wxString(basename));
  return path.GetFullPath().ToStdString();
}

static bool copy_file(const char* src_path, const char* dest_path) {
#ifdef __ANDROID__
  return AndroidSecureCopyFile(src_path, dest_path);
#else
  return wxCopyFile(src_path, dest_path);
#endif
}

namespace ocpn {

std::string get_basename(const char* path) {
  wxString sep(wxFileName::GetPathSeparator());
  auto parts = ocpn::split(path, sep.ToStdString());
  return parts[parts.size() - 1];
}

static std::string metadata_path(const char* basename, bool create = false) {
  wxFileName dirs(cache_path());
  dirs.AppendDir("metadata");
  if (create) {
    dirs.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
  }
  wxFileName path(dirs.GetFullPath(), wxString(basename));
  return path.GetFullPath().ToStdString();
}

bool store_metadata(const char* path) {
  auto name = get_basename(path);
  std::string dest = metadata_path(name.c_str(), true);
  bool ok = ::copy_file(path, dest.c_str());
  wxLogDebug("Storing metadata %s at %s: %s", path, dest.c_str(),
             ok ? "ok" : "fail");
  return ok;
}

std::string lookup_metadata(const char* name) {
  if (name == 0) {
    name = "ocpn-plugins.xml";
  }
  auto path = metadata_path(name);
  return ocpn::exists(path) ? path : "";
}

bool store_tarball(const char* path, const char* basename) {
  std::string dest = tarball_path(basename, true);
  bool ok = ::copy_file(path, dest.c_str());
  wxLogDebug("Storing tarball %s at %s: %s", path, dest.c_str(),
             ok ? "ok" : "fail");
  return ok;
}

std::string lookup_tarball(const char* uri) {
  std::string basename = get_basename(uri);
  std::string path = tarball_path(basename.c_str());
  return ocpn::exists(path) ? path : "";
}

unsigned cache_file_count() {
  wxFileName dirs(cache_path());
  dirs.AppendDir("tarballs");
  if (!dirs.DirExists()) {
    return 0;
  }
  wxDir dir(dirs.GetFullPath());
  wxString file;
  unsigned count = 0;
  bool cont = dir.GetFirst(&file);
  while (cont) {
    count += 1;
    cont = dir.GetNext(&file);
  }
  return count;
}

unsigned long cache_size() {
  wxFileName dirs(cache_path());
  dirs.AppendDir("tarballs");
  if (!dirs.DirExists()) {
    return 0;
  }
  wxDir dir(dirs.GetFullPath());
  wxString file;
  wxULongLong total = 0;
  bool cont = dir.GetFirst(&file);
  while (cont) {
    dirs.SetFullName(file);
    wxFileName fn(dirs.GetFullPath());
    if (fn.FileExists()) {  // Consider only regular files.  Should be no
                            // directories here, but one never knows...
      auto size = fn.GetSize();
      if (size == wxInvalidSize) {
        wxLogMessage("Cannot stat file %s",
                     dirs.GetFullPath().ToStdString().c_str());
        continue;
      }

      total += size;
    }
    cont = dir.GetNext(&file);
  }
  total /= (1024 * 1024);
  return total.ToULong();
}

/* mock up definitions.*/
void cache_clear() {
  wxFileName dirs(cache_path());
  dirs.AppendDir("tarballs");
  if (!dirs.DirExists()) {
    return;
  }
  wxDir dir(dirs.GetFullPath());
  wxString file;
  bool cont = dir.GetFirst(&file);
  while (cont) {
    dirs.SetFullName(file);
    wxRemoveFile(dirs.GetFullPath());
    ;
    cont = dir.GetNext(&file);
  }
}

}  // namespace ocpn
