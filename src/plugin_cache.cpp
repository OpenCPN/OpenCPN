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
#include <wx/filename.h>
#include <wx/filefn.h>

#include "plugin_cache.h"
#include "ocpn_utils.h"
#include "OCPNPlatform.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

extern OCPNPlatform*  g_Platform;


static std::string cache_path()
{
    wxFileName path;
    path.AssignDir(g_Platform->GetPrivateDataDir());
    path.AppendDir("plugins");
    path.AppendDir("cache");
    return path.GetFullPath().ToStdString();
}



static std::string tarball_path(const char* basename, bool create = false)
{
    wxFileName dirs(cache_path());
    dirs.AppendDir("tarballs");
    if (create) {
        dirs.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    wxFileName path(dirs.GetFullPath(), wxString(basename));
    return path.GetFullPath().ToStdString();
}


static bool copy_file(const char* src_path, const char* dest_path)
{
#ifdef __OCPN__ANDROID__
    return AndroidSecureCopyFile(src_path, dest_path);
#else
    return wxCopyFile(src_path, dest_path);
#endif
}


namespace ocpn {


std::string get_basename(const char* path)
{
    wxString sep(wxFileName::GetPathSeparator());
    auto parts = ocpn::split(path, sep.ToStdString());
    return parts[parts.size() - 1];
}


static std::string metadata_path(const char* basename, bool create = false)
{
    wxFileName dirs(cache_path());
    dirs.AppendDir("metadata");
    if (create) {
        dirs.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    wxFileName path(dirs.GetFullPath(), wxString(basename));
    return path.GetFullPath().ToStdString();
}


bool store_metadata(const char* path)
{
    auto name = get_basename(path);
    std::string dest = metadata_path(name.c_str(), true);
    bool ok = ::copy_file(path, dest.c_str());
    wxLogDebug("Storing metadata %s at %s: %s",
               path, dest.c_str(), ok ? "ok" : "fail");
    return ok;
}


std::string lookup_metadata(const char* name)
{
    if (name == 0) {
        name = "ocpn-plugins.xml";
    }
    auto path = metadata_path(name);
    return ocpn::exists(path) ? path : "";
}


bool store_tarball(const char* path, const char* basename)
{
    std::string dest = tarball_path(basename, true);
    bool ok = ::copy_file(path, dest.c_str());
    wxLogDebug("Storing tarball %s at %s: %s",
               path, dest.c_str(), ok ? "ok" : "fail");
    return ok;
}


std::string lookup_tarball(const char* uri)
{
    std::string basename = get_basename(uri);
    std::string path = tarball_path(basename.c_str());
    return ocpn::exists(path) ? path : "";
}


}  // namespace
