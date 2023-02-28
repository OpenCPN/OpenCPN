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
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <memory>
#include <ostream>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <unordered_map>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/dir.h>
#include <wx/file.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/window.h>
#include <wx/uri.h>

#include <archive.h>
#include <archive_entry.h>
typedef __LA_INT64_T la_int64_t;  //  "older" libarchive versions support

#if defined(__MINGW32__) && defined(Yield)
#undef Yield  // from win.h, conflicts with mingw headers
#endif

#include "base_platform.h"
#include "catalog_handler.h"
#include "catalog_parser.h"
#include "config.h"
#include "downloader.h"
#include "gui_lib.h"
#include "logger.h"
#include "navutil.h"
#include "ocpn_utils.h"
#include "plugin_cache.h"
#include "plugin_handler.h"
#include "plugin_loader.h"
#include "plugin_paths.h"

#ifdef _WIN32
static std::string SEP("\\");
#else
static std::string SEP("/");
#endif

#ifndef F_OK  // windows: missing unistd.h.
#define F_OK 0
#endif

extern BasePlatform* g_BasePlatform;
extern wxString g_winPluginDir;
extern MyConfig* pConfig;
extern bool g_bportable;

extern wxString g_compatOS;
extern wxString g_compatOsVersion;

/** split s on first occurrence of delim, or return s in first result. */
static std::vector<std::string> split(const std::string& s,
                                      const std::string& delim) {
  std::vector<std::string> result;
  size_t pos = s.find(delim);
  if (pos == std::string::npos) {
    result.push_back(s);
    return result;
  }
  result.push_back(s.substr(0, pos));
  result.push_back(s.substr(pos + delim.length()));
  return result;
}

inline std::string basename(const std::string path) {
  wxFileName wxFile(path);
  return wxFile.GetFullName().ToStdString();
}

bool isRegularFile(const char* path) {
  wxFileName wxFile(path);
  return wxFile.FileExists() && !wxFile.IsDir();
}

static void mkdir(const std::string path) {
#if defined(_WIN32) && !defined(__MINGW32__)
  _mkdir(path.c_str());
#elif defined(__MINGW32__)
  mkdir(path.c_str());
#else
  mkdir(path.c_str(), 0755);
#endif
}

/**
 * Return index in ArrayOfPlugins for plugin with given name,
 * or -1 if not found
 */
static ssize_t PlugInIxByName(const std::string name, ArrayOfPlugIns* plugins) {
  for (unsigned i = 0; i < plugins->GetCount(); i += 1) {
    if (name == plugins->Item(i)->m_common_name.ToStdString()) {
      return i;
    }
  }
  return -1;
}

static std::string pluginsConfigDir() {
  std::string pluginDataDir = g_BasePlatform->GetPrivateDataDir().ToStdString();
  pluginDataDir += SEP + "plugins";
  if (!ocpn::exists(pluginDataDir)) {
    mkdir(pluginDataDir);
  }
  pluginDataDir += SEP + "install_data";
  if (!ocpn::exists(pluginDataDir)) {
    mkdir(pluginDataDir);
  }
  return pluginDataDir;
}

static std::string dirListPath(std::string name) {
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return pluginsConfigDir() + SEP + name + ".dirs";
}

std::string PluginHandler::pluginsInstallDataPath() {
  return pluginsConfigDir();
}

static std::vector<std::string> LoadLinesFromFile(const std::string& path) {
  std::vector<std::string> lines;
  std::ifstream src(path);
  while (!src.eof()) {
    char line[256];
    src.getline(line, sizeof(line));
    lines.push_back(line);
  }
  return lines;
}

/** Plugin ABI encapsulation. */
class Plugin {
public:
  Plugin(const PluginMetadata& metadata) {
    m_abi = metadata.target;
    m_abi_version = metadata.target_version;
    m_major_version = ocpn::split(m_abi_version.c_str(), ".")[0];
    m_name = metadata.name;
    wxLogDebug("Plugin: setting up, name: %s", m_name);
    wxLogDebug("Plugin: init: abi: %s, abi_version: %s, major ver: %s", m_abi,
               m_abi_version, m_major_version);
  }
  const std::string& abi() const { return m_abi; }
  const std::string& abi_version() const { return m_abi_version; }
  const std::string& major_version() const { return m_major_version; }
  const std::string& name() const { return m_name; }

private:
  std::string m_abi;
  std::string m_abi_version;
  std::string m_major_version;
  std::string m_name;
};

/** Host ABI encapsulation and plugin compatibility checks. */
class Host {
public:
  Host(CompatOs* compatOs) {
    m_abi = compatOs->name();
    m_abi_version = compatOs->version();
    m_major_version = ocpn::split(m_abi_version.c_str(), ".")[0];
    wxLogDebug("Host: init: abi: %s, abi_version: %s, major ver: %s", m_abi,
               m_abi_version, m_major_version);
  }

  bool is_version_compatible(const Plugin& plugin) const {
    if (ocpn::startswith(plugin.abi(), "ubuntu")) {
      return plugin.abi_version() == m_abi_version;
    }
    return plugin.major_version() == m_major_version;
  }

  // Test if plugin abi is a Debian version compatible with host's Ubuntu
  // abi version on a x86_64 platform.
  bool is_debian_plugin_compatible(const Plugin& plugin) const {
    if (!ocpn::startswith(m_abi, "ubuntu")) return false;
    static const std::vector<std::string> compat_versions = {
        // clang-format: off
        "debian-x86_64;11;ubuntu-gtk3-x86_64;20.04",
        "debian-wx32-x86_64;11;ubuntu-wx32-x86_64;22.04",
        "debian-x86_64;12;ubuntu-x86_64;23.04",
        "debian-x86_64;12;ubuntu-x86_64;23.10",
        "debian-x86_64;12;ubuntu-x86_64;24.04",
        "debian-x86_64;sid;ubuntu-x86_64;24.04"};  // clang-format: on
    if (ocpn::startswith(plugin.abi(), "debian")) {
      wxLogDebug("Checking for debian plugin on a ubuntu-x86_64 host");
      const std::string compat_version = plugin.abi() + ";" +
                                         plugin.major_version() + ";" + m_abi +
                                         ";" + m_abi_version;
      for (auto& cv : compat_versions) {
        if (compat_version == cv) {
          return true;
        }
      }
    }
    return false;
  }

  const std::string& abi() const { return m_abi; }

  const std::string& abi_version() const { return m_abi_version; }

  const std::string& major_version() const { return m_major_version; }

private:
  std::string m_abi;
  std::string m_abi_version;
  std::string m_major_version;
};

CompatOs* CompatOs::getInstance() {
  static std::string last_global_os("");
  static CompatOs* instance = 0;

  if (!instance || last_global_os != g_compatOS) {
    instance = new (CompatOs);
    last_global_os = g_compatOS;
  }
  return instance;
};

CompatOs::CompatOs() : _name(PKG_TARGET), _version(PKG_TARGET_VERSION) {
  // Get the specified system definition,
  //   from the environment override,
  //   or the config file override
  //   or the baked in (build system) values.

  std::string compatOS(_name);
  std::string compatOsVersion(_version);

  if (getenv("OPENCPN_COMPAT_TARGET") != 0) {
    _name = getenv("OPENCPN_COMPAT_TARGET");
    if (_name.find(':') != std::string::npos) {
      auto tokens = ocpn::split(_name.c_str(), ":");
      _name = tokens[0];
      _version = tokens[1];
    }
  } else if (g_compatOS != "") {
    // CompatOS and CompatOsVersion in opencpn.conf/.ini file.
    _name = g_compatOS;
    if (g_compatOsVersion != "") {
      _version = g_compatOsVersion;
    }
  } else if (ocpn::startswith(_name, "ubuntu") && (_version == "22.04")) {
    int wxv = wxMAJOR_VERSION * 10 + wxMINOR_VERSION;
    if (wxv >= 32) {
      auto tokens = ocpn::split(_name.c_str(), "-");
      _name = std::string(tokens[0]) + std::string("-wx32-") + tokens[1];
    }
  }

  _name = ocpn::tolower(_name);
  _version = ocpn::tolower(_version);
}

PluginHandler::PluginHandler() {}

bool PluginHandler::isCompatible(const PluginMetadata& metadata, const char* os,
                                 const char* os_version) {
  static const std::vector<std::string> simple_abis = {
      "msvc",        "msvc-wx32",     "darwin",
      "darwin-wx32", "android-armhf", "android-arm64"};

  auto compatOS = CompatOs::getInstance();
  Host host(compatOS);
  Plugin plugin(metadata);

  auto found = std::find(simple_abis.begin(), simple_abis.end(), plugin.abi());
  if (found != simple_abis.end()) {
    bool ok = plugin.abi() == host.abi();
    wxLogDebug("Returning %s for %s", (ok ? "ok" : "fail"), host.abi());
    wxLogDebug(" ");
    return ok;
  }
  bool rv = false;
  if (host.abi() == plugin.abi() && host.is_version_compatible(plugin)) {
    rv = true;
    wxLogDebug("Found matching abi version %s", plugin.abi_version());
  } else if (host.is_debian_plugin_compatible(plugin)) {
    rv = true;
    wxLogDebug("Found Debian version matching Ubuntu host");
  }
  DEBUG_LOG << "Plugin compatibility check Final: "
            << (rv ? "ACCEPTED: " : "REJECTED: ") << metadata.name;
  return rv;
}

std::string PluginHandler::fileListPath(std::string name) {
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return pluginsConfigDir() + SEP + name + ".files";
}

std::string PluginHandler::versionPath(std::string name) {
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return pluginsConfigDir() + SEP + name + ".version";
}

typedef std::unordered_map<std::string, std::string> pathmap_t;

/**
 * Platform-specific mapping of tarball paths to user-writable
 * installation directories.
 */
static pathmap_t getInstallPaths() {
  using namespace std;

  pathmap_t pathmap;
  PluginPaths* paths = PluginPaths::getInstance();
  pathmap["bin"] = paths->UserBindir();
  pathmap["lib"] = paths->UserLibdir();
  pathmap["lib64"] = paths->UserLibdir();
  pathmap["share"] = paths->UserDatadir();
  return pathmap;
}

static void saveFilelist(std::string filelist, std::string name) {
  using namespace std;
  string listpath = PluginHandler::fileListPath(name);
  ofstream diskfiles(listpath);
  if (!diskfiles.is_open()) {
    wxLogWarning("Cannot create installed files list.");
    return;
  }
  diskfiles << filelist;
}

static void saveDirlist(std::string name) {
  using namespace std;
  string path = dirListPath(name);
  ofstream dirs(path);
  if (!dirs.is_open()) {
    wxLogWarning("Cannot create installed files list.");
    return;
  }
  pathmap_t pathmap = getInstallPaths();
  unordered_map<string, string>::iterator it;
  for (it = pathmap.begin(); it != pathmap.end(); it++) {
    dirs << it->first << ": " << it->second << endl;
  }
}

static void saveVersion(const std::string& name, const std::string& version) {
  using namespace std;
  string path = PluginHandler::versionPath(name);
  ofstream stream(path);
  if (!stream.is_open()) {
    wxLogWarning("Cannot create version file.");
    return;
  }
  stream << version << endl;
}

static int copy_data(struct archive* ar, struct archive* aw) {
  int r;
  const void* buff;
  size_t size;
  la_int64_t offset;

  while (true) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) return (ARCHIVE_OK);
    if (r < ARCHIVE_OK) {
      std::string s(archive_error_string(ar));
      return (r);
    }
    r = archive_write_data_block(aw, buff, size, offset);
    if (r < ARCHIVE_OK) {
      std::string s(archive_error_string(aw));
      wxLogWarning("Error copying install data: %s", archive_error_string(aw));
      return (r);
    }
  }
}

static bool win_entry_set_install_path(struct archive_entry* entry,
                                       pathmap_t installPaths) {
  using namespace std;

  string path = archive_entry_pathname(entry);

  // Check # components, drop the single top-level path
  int slashes = count(path.begin(), path.end(), '/');
  if (slashes < 1) {
    archive_entry_set_pathname(entry, "");
    return true;
  }
  if (ocpn::startswith(path, "./")) {
    path = path.substr(1);
  }

  // Remove top-level directory part
  int slashpos = path.find_first_of('/', 1);
  if (slashpos < 0) {
    archive_entry_set_pathname(entry, "");
    return true;
  }

  string prefix = path.substr(0, slashpos);
  path = path.substr(prefix.size() + 1);

  // Map remaining path to installation directory
  if (ocpn::endswith(path, ".dll") || ocpn::endswith(path, ".exe")) {
    slashpos = path.find_first_of('/');
    path = path.substr(slashpos + 1);
    path = installPaths["bin"] + "\\" + path;
  } else if (ocpn::startswith(path, "share")) {
    // The "share" directory should be a direct sibling of "plugins" directory
    wxFileName fn(installPaths["share"].c_str(),
                  "");   // should point to .../opencpn/plugins
    fn.RemoveLastDir();  // should point to ".../opencpn
    path = fn.GetFullPath().ToStdString() + path;
  } else if (ocpn::startswith(path, "plugins")) {
    slashpos = path.find_first_of('/');
    // share path already ends in plugins/, drop prefix from archive entry.
    path = path.substr(slashpos + 1);
    path = installPaths["share"] + "\\" + path;

  } else if (archive_entry_filetype(entry) == AE_IFREG) {
    wxString msg(_T("PluginHandler::Invalid install path on file: "));
    msg += wxString(path.c_str());
    wxLogDebug(msg);
    return false;
  }
  wxString s(path);
  s.Replace("/", "\\");  // std::regex_replace FTBS on gcc 4.8.4
  s.Replace("\\\\", "\\");
  archive_entry_set_pathname(entry, s.c_str());
  return true;
}

static bool flatpak_entry_set_install_path(struct archive_entry* entry,
                                           pathmap_t installPaths) {
  using namespace std;

  string path = archive_entry_pathname(entry);
  int slashes = count(path.begin(), path.end(), '/');
  if (slashes < 2) {
    archive_entry_set_pathname(entry, "");
    return true;
  }
  if (ocpn::startswith(path, "./")) {
    path = path.substr(2);
  }
  int slashpos = path.find_first_of('/', 1);
  string prefix = path.substr(0, slashpos);
  path = path.substr(prefix.size() + 1);
  slashpos = path.find_first_of('/');
  string location = path.substr(0, slashpos);
  string suffix = path.substr(slashpos + 1);
  if (installPaths.find(location) == installPaths.end() &&
      archive_entry_filetype(entry) == AE_IFREG) {
    wxString msg(_T("PluginHandler::Invalid install path on file: "));
    msg += wxString(path.c_str());
    wxLogDebug(msg);
    return false;
  }
  string dest = installPaths[location] + "/" + suffix;
  archive_entry_set_pathname(entry, dest.c_str());

  return true;
}

static bool linux_entry_set_install_path(struct archive_entry* entry,
                                         pathmap_t installPaths) {
  using namespace std;

  string path = archive_entry_pathname(entry);
  int slashes = count(path.begin(), path.end(), '/');
  if (slashes < 2) {
    archive_entry_set_pathname(entry, "");
    return true;
  }

  int slashpos = path.find_first_of('/', 1);
  if (ocpn::startswith(path, "./"))
    slashpos = path.find_first_of('/', 2);  // skip the './'

  string prefix = path.substr(0, slashpos);
  path = path.substr(prefix.size() + 1);
  if (ocpn::startswith(path, "usr/")) {
    path = path.substr(strlen("usr/"));
  }
  if (ocpn::startswith(path, "local/")) {
    path = path.substr(strlen("local/"));
  }
  slashpos = path.find_first_of('/');
  string location = path.substr(0, slashpos);
  string suffix = path.substr(slashpos + 1);
  if (installPaths.find(location) == installPaths.end() &&
      archive_entry_filetype(entry) == AE_IFREG) {
    wxString msg(_T("PluginHandler::Invalid install path on file: "));
    msg += wxString(path.c_str());
    wxLogDebug(msg);
    return false;
  }

  string dest = installPaths[location] + "/" + suffix;

  if (g_bportable) {
    // A data dir?
    if (ocpn::startswith(location, "share") &&
        ocpn::startswith(suffix, "opencpn/plugins/")) {
      slashpos = suffix.find_first_of("opencpn/plugins/");
      suffix = suffix.substr(16);

      dest = g_BasePlatform->GetPrivateDataDir().ToStdString() + "/plugins/" +
             suffix;
    }
    if (ocpn::startswith(location, "lib") &&
        ocpn::startswith(suffix, "opencpn/")) {
      suffix = suffix.substr(8);

      dest = g_BasePlatform->GetPrivateDataDir().ToStdString() +
             "/plugins/lib/" + suffix;
    }
  }

  archive_entry_set_pathname(entry, dest.c_str());
  return true;
}

static bool apple_entry_set_install_path(struct archive_entry* entry,
                                         pathmap_t installPaths) {
  using namespace std;

  const string base = PluginPaths::getInstance()->Homedir() +
                      "/Library/Application Support/OpenCPN";

  string path = archive_entry_pathname(entry);
  if (ocpn::startswith(path, "./")) path = path.substr(2);

  string dest("");
  size_t slashes = count(path.begin(), path.end(), '/');
  if (slashes < 3) {
    archive_entry_set_pathname(entry, "");
    return true;
  }
  auto parts = split(path, "Contents/Resources");
  if (parts.size() >= 2) {
    dest = base + "/Contents/Resources" + parts[1];
  }
  if (dest == "") {
    parts = split(path, "Contents/SharedSupport");
    if (parts.size() >= 2) {
      dest = base + "/Contents/SharedSupport" + parts[1];
    }
  }
  if (dest == "") {
    parts = split(path, "Contents/PlugIns");
    if (parts.size() >= 2) {
      dest = base + "/Contents/PlugIns" + parts[1];
    }
  }
  if (dest == "" && archive_entry_filetype(entry) == AE_IFREG) {
    wxString msg(_T("PluginHandler::Invalid install path on file: "));
    msg += wxString(path.c_str());
    wxLogDebug(msg);
    return false;
  }
  archive_entry_set_pathname(entry, dest.c_str());
  return true;
}

static bool android_entry_set_install_path(struct archive_entry* entry,
                                           pathmap_t installPaths) {
  using namespace std;

  string path = archive_entry_pathname(entry);
  int slashes = count(path.begin(), path.end(), '/');
  if (slashes < 2) {
    archive_entry_set_pathname(entry, "");
    return true;
    ;
  }

  int slashpos = path.find_first_of('/', 1);
  if (ocpn::startswith(path, "./"))
    slashpos = path.find_first_of('/', 2);  // skip the './'

  string prefix = path.substr(0, slashpos);
  path = path.substr(prefix.size() + 1);
  if (ocpn::startswith(path, "usr/")) {
    path = path.substr(strlen("usr/"));
  }
  if (ocpn::startswith(path, "local/")) {
    path = path.substr(strlen("local/"));
  }
  slashpos = path.find_first_of('/');
  string location = path.substr(0, slashpos);
  string suffix = path.substr(slashpos + 1);
  if (installPaths.find(location) == installPaths.end() &&
      archive_entry_filetype(entry) == AE_IFREG) {
    wxString msg(_T("PluginHandler::Invalid install path on file: "));
    msg += wxString(path.c_str());
    wxLogDebug(msg);
    return false;
  }

  if ((location == "lib") && ocpn::startswith(suffix, "opencpn")) {
    auto parts = split(suffix, "/");
    if (parts.size() == 2) suffix = parts[1];
  }

  if ((location == "share") && ocpn::startswith(suffix, "opencpn")) {
    auto parts = split(suffix, "opencpn/");
    if (parts.size() == 2) suffix = parts[1];
  }

  /// storage/emulated/0/android/data/org.opencpn.opencpn/files/opencpn/plugins/oesenc_pi/data/LUPPatch3.xml
  string dest = installPaths[location] + "/" + suffix;

  archive_entry_set_pathname(entry, dest.c_str());
  return true;
}

static bool entry_set_install_path(struct archive_entry* entry,
                                   pathmap_t installPaths) {
  const std::string src = archive_entry_pathname(entry);
  bool rv;
#ifdef __OCPN__ANDROID__
  rv = android_entry_set_install_path(entry, installPaths);
#else
  const auto osSystemId = wxPlatformInfo::Get().GetOperatingSystemId();
  if (g_BasePlatform->isFlatpacked()) {
    rv = flatpak_entry_set_install_path(entry, installPaths);
  } else if (osSystemId & wxOS_UNIX_LINUX) {
    rv = linux_entry_set_install_path(entry, installPaths);
  } else if (osSystemId & wxOS_WINDOWS) {
    rv = win_entry_set_install_path(entry, installPaths);
  } else if (osSystemId & wxOS_MAC) {
    rv = apple_entry_set_install_path(entry, installPaths);
  } else {
    wxLogMessage("set_install_path() invoked, unsupported platform %s",
                 wxPlatformInfo::Get().GetOperatingSystemDescription());
    rv = false;
  }
#endif
  const std::string dest = archive_entry_pathname(entry);
  if (rv) {
    if (dest.size()) {
      DEBUG_LOG << "Installing " << src << " into " << dest << std::endl;
    }
  }
  return rv;
}

bool PluginHandler::archive_check(int r, const char* msg, struct archive* a) {
  if (r < ARCHIVE_OK) {
    std::string s(msg);

    if (archive_error_string(a)) s = s + ": " + archive_error_string(a);
    wxLogMessage(s.c_str());
    last_error_msg = s;
  }
  return r >= ARCHIVE_WARN;
}

bool PluginHandler::explodeTarball(struct archive* src, struct archive* dest,
                                   std::string& filelist,
                                   const std::string& metadata_path) {
  struct archive_entry* entry = 0;
  pathmap_t pathmap = getInstallPaths();
  while (true) {
    int r = archive_read_next_header(src, &entry);
    if (r == ARCHIVE_EOF) {
      return true;
    }
    if (!archive_check(r, "archive read header error", src)) {
      return false;
    }
    std::string path = archive_entry_pathname(entry);
    bool is_metadata = std::string::npos != path.find("metadata.xml");
    if (is_metadata) {
      if (metadata_path == "") continue;
      archive_entry_set_pathname(entry, metadata_path.c_str());
    } else if (!entry_set_install_path(entry, pathmap))
      continue;
    if (strlen(archive_entry_pathname(entry)) == 0) {
      continue;
    }
    if (!is_metadata) {
      filelist.append(std::string(archive_entry_pathname(entry)) + "\n");
    }
    r = archive_write_header(dest, entry);
    archive_check(r, "archive write install header error", dest);
    if (r >= ARCHIVE_OK && archive_entry_size(entry) > 0) {
      r = copy_data(src, dest);
      if (!archive_check(r, "archive copy data error", dest)) {
        return false;
      }
    }
    r = archive_write_finish_entry(dest);
    if (!archive_check(r, "archive finish write error", dest)) {
      return false;
    }
  }
  return false;  // notreached
}

/*
 * Extract tarball into platform-specific user directories.
 *
 * The installed tarball has paths like topdir/dest/suffix_path... e. g.
 * oesenc_pi_ubuntu_10_64/usr/local/share/opencpn/plugins/oesenc_pi/README.
 * In this path, the topdir part must exist but is discarded. Next parts
 * being being standard prefixes like /usr/local or /usr are also
 * discarded. The remaining path (here share) is mapped to a user
 * directory. On linux, it ends up in ~/.local/share. The suffix
 * part is then installed as-is into this directory.
 *
 * Windows tarballs has dll and binary files in the top directory. They
 * go to winInstallDir/Program Files. Message catalogs exists under a
 * share/ toplevel directory, they go in winInstallDir/share. The
 * plugin data is installed under winInstallDir/plugins/<plugin name>,
 * and must be looked up by the plugins using GetPluginDataDir(plugin);
 * Windows requires that PATH is set to include the binary dir and tha
 * a bindtextdomain call is invoked to define the message catalog paths.
 *
 * For linux, the expected destinations are bin, lib and share.
 *
 * Parameters:
 *   - path: path to tarball
 *   - filelist: On return contains a list of files installed.
 *   - last_error_msg: Updated when returning false.
 *
 */
bool PluginHandler::extractTarball(const std::string path,
                                   std::string& filelist,
                                   const std::string metadata_path) {
  struct archive* src = archive_read_new();
  archive_read_support_filter_gzip(src);
  archive_read_support_format_tar(src);
  int r = archive_read_open_filename(src, path.c_str(), 10240);
  if (r != ARCHIVE_OK) {
    std::ostringstream os;
    os << "Cannot read installation tarball: " << path;
    wxLogWarning(os.str().c_str());
    last_error_msg = os.str();
    return false;
  }
  struct archive* dest = archive_write_disk_new();
  archive_write_disk_set_options(dest, ARCHIVE_EXTRACT_TIME);
  bool ok = explodeTarball(src, dest, filelist, metadata_path);
  archive_read_free(src);
  archive_write_free(dest);
  return ok;
}

PluginHandler* PluginHandler::getInstance() {
  static PluginHandler* instance = 0;
  if (!instance) {
    instance = new (PluginHandler);
  }
  return instance;
}

bool PluginHandler::isPluginWritable(std::string name) {
  if (isRegularFile(PluginHandler::fileListPath(name).c_str())) {
    return true;
  }
  auto loader = PluginLoader::getInstance();
  return PlugInIxByName(name, loader->GetPlugInArray()) == -1;
}

static std::string computeMetadataPath(void) {
  std::string path = g_BasePlatform->GetPrivateDataDir().ToStdString();
  path += SEP;
  path += "ocpn-plugins.xml";
  if (ocpn::exists(path)) {
    return path;
  }

  // If default location for composit plugin metadata is not found,
  // we look in the plugin cache directory, which will normally contain
  // he last "master" catalog downloaded
  path = ocpn::lookup_metadata();
  if (path != "") {
    return path;
  }

  // And if that does not work, use the empty metadata file found in the
  // distribution "data" directory
  path = g_BasePlatform->GetSharedDataDir();
  path += SEP;
  path += "ocpn-plugins.xml";
  if (!ocpn::exists(path)) {
    wxLogWarning("Non-existing plugins file: %s", path);
  }
  return path;
}

std::string PluginHandler::getMetadataPath() {
  if (metadataPath.size() > 0) {
    return metadataPath;
  }
  metadataPath = computeMetadataPath();
  wxLogDebug("Using metadata path: %s", metadataPath.c_str());
  return metadataPath;
}

static void parseMetadata(const std::string path, CatalogCtx& ctx) {
  using namespace std;

  wxLogMessage("PluginHandler: using metadata path: %s", path);
  ctx.depth = 0;
  if (!ocpn::exists(path)) {
    wxLogWarning("Non-existing plugins metadata file: %s", path.c_str());
    return;
  }
  ifstream ifpath(path);
  std::string xml((istreambuf_iterator<char>(ifpath)),
                  istreambuf_iterator<char>());
  ParseCatalog(xml, &ctx);
}

const std::map<std::string, int> PluginHandler::getCountByTarget() {
  auto plugins = getInstalled();
  auto a = getAvailable();
  plugins.insert(plugins.end(), a.begin(), a.end());
  std::map<std::string, int> count_by_target;
  for (const auto& p : plugins) {
    if (p.target == "") {
      continue;  // Built-in plugins like  dashboard et. al.
    }
    auto key = p.target + ":" + p.target_version;
    if (count_by_target.find(key) == count_by_target.end()) {
      count_by_target[key] = 1;
    } else {
      count_by_target[key] += 1;
    }
  }
  return count_by_target;
}

void PluginHandler::cleanupFiles(const std::string& manifestFile,
                                 const std::string& plugname) {
  std::ifstream diskfiles(manifestFile);
  if (diskfiles.is_open()) {
    std::stringstream buffer;
    buffer << diskfiles.rdbuf();
    PluginHandler::cleanup(buffer.str(), plugname);
  }
}

/** Remove all empty dirs found from root containing string "opencpn". */
static void PurgeEmptyDirs(const std::string& root) {
  if (!wxFileName::IsDirWritable(root)) return;
  if (ocpn::tolower(root).find("opencpn") == std::string::npos) return;
  wxDir rootdir(root);
  if (!rootdir.IsOpened()) return;
  wxString dirname;
  bool cont = rootdir.GetFirst(&dirname, "", wxDIR_DIRS);
  while (cont) {
    PurgeEmptyDirs((rootdir.GetNameWithSep() + dirname).ToStdString());
    cont = rootdir.GetNext(&dirname);
  }
  rootdir.Close();
  rootdir.Open(root);
  if (!(rootdir.HasFiles() || rootdir.HasSubDirs())) {
    wxFileName::Rmdir(rootdir.GetName());
  }
}

void PluginHandler::cleanup(const std::string& filelist,
                            const std::string& plugname) {
  wxLogMessage("Cleaning up failed install of %s", plugname.c_str());

  std::vector<std::string> paths = LoadLinesFromFile(filelist);
  for (const auto& path : paths) {
    if (isRegularFile(path.c_str())) {
      int r = remove(path.c_str());
      if (r != 0) wxLogWarning("Cannot remove file %s: %s", path, strerror(r));
    }
  }
  for (const auto& path : paths) PurgeEmptyDirs(path);

  std::string path = PluginHandler::fileListPath(plugname);
  if (ocpn::exists(path)) remove(path.c_str());

  // Best effort tries, failures are non-critical
  remove(dirListPath(plugname).c_str());
  remove(PluginHandler::versionPath(plugname).c_str());
}

const std::vector<PluginMetadata> PluginHandler::getAvailable() {
  using namespace std;
  CatalogCtx ctx;

  auto catalogHandler = CatalogHandler::getInstance();

  std::string path = getMetadataPath();
  if (!ocpn::exists(path)) {
    return ctx.plugins;
  }
  std::ifstream file;
  file.open(path, std::ios::in);
  if (file.is_open()) {
    std::string xml((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    file.close();
    auto status = catalogHandler->DoParseCatalog(xml, &ctx);
    if (status == CatalogHandler::ServerStatus::OK) {
      catalogData.undef = false;
      catalogData.version = ctx.version;
      catalogData.date = ctx.date;
    }
  }

  return ctx.plugins;
}

const std::vector<PluginMetadata> PluginHandler::getInstalled() {
  using namespace std;
  vector<PluginMetadata> plugins;

  auto loader = PluginLoader::getInstance();
  ArrayOfPlugIns* mgr_plugins = loader->GetPlugInArray();
  for (unsigned int i = 0; i < mgr_plugins->GetCount(); i += 1) {
    PlugInContainer* p = mgr_plugins->Item(i);
    PluginMetadata plugin;
    auto name = string(p->m_common_name);
    // std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    plugin.name = name;
    std::stringstream ss;
    ss << p->m_version_major << "." << p->m_version_minor;
    plugin.version = ss.str();
    plugin.readonly = !isPluginWritable(plugin.name);
    string path = PluginHandler::versionPath(plugin.name);
    if (path != "" && wxFileName::IsFileReadable(path)) {
      std::ifstream stream;
      stream.open(path, ifstream::in);
      stream >> plugin.version;
    }
    plugins.push_back(plugin);
  }
  return plugins;
}

bool PluginHandler::installPlugin(PluginMetadata plugin, std::string path) {
  std::string filelist;
  if (!extractTarball(path, filelist)) {
    std::ostringstream os;
    os << "Cannot unpack plugin: " << plugin.name << " at " << path;
    last_error_msg = os.str();
    PluginHandler::cleanup(filelist, plugin.name);
    return false;
  }
  // remove(path.c_str());
  saveFilelist(filelist, plugin.name);
  saveDirlist(plugin.name);
  saveVersion(plugin.name, plugin.version);

  return true;
}

bool PluginHandler::installPlugin(PluginMetadata plugin) {
  std::string path;
  char fname[4096];

  if (tmpnam(fname) == NULL) {
    wxLogWarning("Cannot create temporary file");
    path = "";
    return false;
  }
  path = std::string(fname);
  std::ofstream stream;
  stream.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  DEBUG_LOG << "Downloading: " << plugin.name << std::endl;
  auto downloader = Downloader(plugin.tarball_url);
  downloader.download(&stream);

  return installPlugin(plugin, path);
}

bool PluginHandler::installPlugin(std::string path) {
  std::string filelist;
  std::string temp_path(tmpnam(0));
  if (!extractTarball(path, filelist, temp_path)) {
    std::ostringstream os;
    os << "Cannot unpack plugin tarball at : " << path;
    if (filelist != "") cleanup(filelist, "unknown_name");
    last_error_msg = os.str();
    return false;
  }
  struct CatalogCtx ctx;
  std::ifstream istream(temp_path);
  std::stringstream buff;
  buff << istream.rdbuf();
  remove(temp_path.c_str());

  auto xml = std::string("<plugins>") + buff.str() + "</plugins>";

  ParseCatalog(xml, &ctx);
  auto name = ctx.plugins[0].name;
  auto version = ctx.plugins[0].version;

  saveFilelist(filelist, name);
  saveDirlist(name);
  saveVersion(name, version);

  return true;
}

bool PluginHandler::uninstall(const std::string plugin_name) {
  using namespace std;

  auto loader = PluginLoader::getInstance();
  auto ix = PlugInIxByName(plugin_name, loader->GetPlugInArray());
  auto pic = loader->GetPlugInArray()->Item(ix);
  // g_pi_manager->ClosePlugInPanel(pic, wxID_OK);
  loader->UnLoadPlugIn(ix);
  string path = PluginHandler::fileListPath(plugin_name);
  if (!ocpn::exists(path)) {
    wxLogWarning("Cannot find installation data for %s (%s)",
                 plugin_name.c_str(), path);
    return false;
  }
  vector<string> plug_paths = LoadLinesFromFile(path);
  for (const auto& p : plug_paths) {
    if (isRegularFile(p.c_str())) {
      int r = remove(p.c_str());
      if (r != 0) {
        wxLogWarning("Cannot remove file %s: %s", p.c_str(), strerror(r));
      }
    }
  }
  for (const auto& p : plug_paths) PurgeEmptyDirs(p);
  int r = remove(path.c_str());
  if (r != 0) {
    wxLogWarning("Cannot remove file %s: %s", path.c_str(), strerror(r));
  }

  // Best effort tries, failures are OK.
  remove(dirListPath(plugin_name).c_str());
  remove(PluginHandler::versionPath(plugin_name).c_str());

  return true;
}

using PluginMap = std::unordered_map<std::string, std::vector<std::string>>;

/**
 * Look in plugin data paths for a matching directory. Safe bet if
 * existing, but all plugins does not have a data directory.
 * @return Matched directory name or "" if not found.
 */
static std::string FindMatchingDataDir(std::regex name_re) {
  using namespace std;
  wxString data_dirs(g_BasePlatform->GetPluginDataPath());
  wxStringTokenizer tokens(data_dirs, ";");
  while (tokens.HasMoreTokens()) {
    auto token = tokens.GetNextToken();
    wxFileName path(token);
    wxDir dir(path.GetFullPath());
    wxString filename;
    bool cont = dir.GetFirst(&filename, "", wxDIR_DIRS);
    while (cont) {
      smatch sm;
      string s(filename);
      if (regex_search(s, sm, name_re)) {
        stringstream ss;
        for (auto c : sm) ss << c;
        return ss.str();
      }
      cont = dir.GetNext(&filename);
    }
  }
  return "";
}

/**
 * Look in library dirs for matching .dll/.so/.dylib and use matched name.
 * @return matched part of found library name or "" if not found
 */
static std::string FindMatchingLibFile(std::regex name_re) {
  using namespace std;
  for (const auto& lib : PluginPaths::getInstance()->Libdirs()) {
    wxDir dir(lib);
    wxString filename;
    bool cont = dir.GetFirst(&filename, "", wxDIR_FILES);
    while (cont) {
      smatch sm;
      string s(filename);
      if (regex_search(s, sm, name_re)) {
        stringstream ss;
        for (auto c : sm) ss << c;
        return ss.str();
      }
      cont = dir.GetNext(&filename);
    }
  }
  return "";
}

/** Best effort to return plugin name with correct case. */
static std::string PluginNameCase(const std::string& name) {
  using namespace std;
  const string lc_name = ocpn::tolower(name);
  regex name_re(lc_name, regex_constants::icase | regex_constants::ECMAScript);

  // Look for matching plugin in list of installed and available.
  // This often fails since the lists are not yet available when
  // plugins are loaded, but is otherwise a safe bet.
  for (const auto& plugin : PluginHandler::getInstance()->getInstalled()) {
    if (ocpn::tolower(plugin.name) == lc_name) return plugin.name;
  }
  for (const auto& plugin : PluginHandler::getInstance()->getAvailable()) {
    if (ocpn::tolower(plugin.name) == lc_name) return plugin.name;
  }

  string match = FindMatchingDataDir(name_re);
  if (match != "") return match;

  match = FindMatchingLibFile(name_re);
  return match != "" ? match : name;
}

/**  map[key] = list-of-files given path to file containing paths. */
static void LoadPluginMapFile(PluginMap& map, const std::string& path) {
  std::ifstream f;
  f.open(path);
  if (f.fail()) {
    wxLogWarning("Cannot open %s: %s", path.c_str(), strerror(errno));
    return;
  }
  std::stringstream buf;
  buf << f.rdbuf();
  auto filelist = ocpn::split(buf.str().c_str(), "\n");
  for (auto& file : filelist) {
    file = wxFileName(file).GetFullName().ToStdString();
  }

  // key is basename with removed .files suffix and correct case.
  auto key = wxFileName(path).GetFullName().ToStdString();
  key = ocpn::split(key.c_str(), ".")[0];
  key = PluginNameCase(key);
  map[key] = filelist;
}

/** For each installed plugin: map[plugin name] = list of files. */
static void LoadPluginMap(PluginMap& map) {
  map.clear();
  wxDir root(PluginHandler::pluginsInstallDataPath());
  if (!root.IsOpened()) return;
  wxString filename;
  bool cont = root.GetFirst(&filename, "*.files", wxDIR_FILES);
  while (cont) {
    auto path = root.GetNameWithSep() + filename;
    LoadPluginMapFile(map, path.ToStdString());
    cont = root.GetNext(&filename);
  }
}

std::string PluginHandler::getPluginByLibrary(const std::string& filename) {
  auto basename = wxFileName(filename).GetFullName().ToStdString();
  if (files_by_plugin.size() == 0) LoadPluginMap(files_by_plugin);
  for (const auto& it : files_by_plugin) {
    auto found = std::find(it.second.begin(), it.second.end(), basename);
    if (found != it.second.end()) return it.first;
  }
  return "";
}

bool PluginHandler::installPluginFromCache(PluginMetadata plugin) {
  // Look for the desired file
  wxURI uri(wxString(plugin.tarball_url.c_str()));
  wxFileName fn(uri.GetPath());
  wxString tarballFile = fn.GetFullName();
  std::string cacheFile = ocpn::lookup_tarball(tarballFile);

#ifdef __WXOSX__
  // Depending on the browser settings, MacOS will sometimes automatically
  // de-compress the tar.gz file, leaving a simple ".tar" file in its expected
  // place. Check for this case, and "do the right thing"
  if (cacheFile == "") {
    fn.ClearExt();
    wxFileName fn1(fn.GetFullName());
    if (fn1.GetExt().IsSameAs("tar")) {
      tarballFile = fn.GetFullName();
      cacheFile = ocpn::lookup_tarball(tarballFile);
    }
  }
#endif

  if (cacheFile != "") {
    wxLogMessage("Installing %s from local cache", tarballFile.c_str());
    bool bOK = installPlugin(plugin, cacheFile);
    if (!bOK) {
      wxLogWarning("Cannot install tarball file %s", cacheFile.c_str());
      evt_download_failed.Notify(cacheFile);
      return false;
    }
    evt_download_ok.Notify(plugin.name + " " + plugin.version);
    return true;
  }
  return false;
}
