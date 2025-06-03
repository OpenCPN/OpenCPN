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

#if (defined(OCPN_GHC_FILESYSTEM) || \
     (defined(__clang_major__) && (__clang_major__ < 15)))
// MacOS 1.13
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#else
#include <filesystem>
#include <utility>
namespace fs = std::filesystem;
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>
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

#include "config.h"

#include "model/base_platform.h"
#include "model/catalog_handler.h"
#include "model/catalog_parser.h"
#include "model/config_vars.h"
#include "model/cmdline.h"
#include "model/downloader.h"
#include "model/logger.h"
#include "model/ocpn_utils.h"
#include "model/plugin_cache.h"
#include "model/plugin_handler.h"
#include "model/plugin_loader.h"
#include "model/plugin_paths.h"

#ifdef _WIN32
static std::string SEP("\\");
#else
static std::string SEP("/");
#endif

#ifndef F_OK  // windows: missing unistd.h.
#define F_OK 0
#endif

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

static std::vector<std::string> glob_dir(const std::string& dir_path,
                                         const std::string& pattern) {
  std::vector<std::string> found;
  wxString s;
  wxDir dir(dir_path);
  auto match = dir.GetFirst(&s, pattern);
  while (match) {
    static const std::string SEP =
        wxString(wxFileName::GetPathSeparator()).ToStdString();
    found.push_back(dir_path + SEP + s.ToStdString());
    match = dir.GetNext(&s);
  }
  return found;
}

/**
 * Return index in ArrayOfPlugins for plugin with given name,
 * or -1 if not found
 */
static ssize_t PlugInIxByName(const std::string& name,
                              const ArrayOfPlugIns* plugins) {
  const auto lc_name = ocpn::tolower(name);
  for (unsigned i = 0; i < plugins->GetCount(); i += 1) {
    if (lc_name == plugins->Item(i)->m_common_name.Lower().ToStdString()) {
      return i;
    }
  }
  return -1;
}

static std::string pluginsConfigDir() {
  auto pluginDataDir = g_BasePlatform->DefaultPrivateDataDir().ToStdString();
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

static std::string importsDir() {
  auto path = pluginsConfigDir();
  path = path + SEP + "imports";
  if (!ocpn::exists(path)) {
    mkdir(path);
  }
  return path;
}

static std::string dirListPath(std::string name) {
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return pluginsConfigDir() + SEP + name + ".dirs";
}

std::string PluginHandler::PluginsInstallDataPath() {
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

#ifdef _WIN32
static std::string tmpfile_path() {
  /** Use old poorly defined fname. */
  char fname[4096];
  if (tmpnam(fname) == NULL) {
    MESSAGE_LOG << "Cannot create temporary file";
    return "";
  }
  return std::string(fname);
}

#else
static std::string tmpfile_path() {
  /** Use mkstemp to avoid annoying linker warning */
  fs::path tmp_path = fs::temp_directory_path() / "ocpn-tmpXXXXXX";
  char buff[PATH_MAX];
  strncpy(buff, tmp_path.c_str(), PATH_MAX - 1);
  int fd = mkstemp(buff);
  if (fd == -1) {
    MESSAGE_LOG << "Cannot create temporary file: " << strerror(errno);
    return "";
  }
  assert(close(fd) == 0 && "Cannot close file?!");
  return std::string(buff);
}
#endif  // _WIN32

/** Plugin ABI encapsulation. */
class Plugin {
public:
  Plugin(const PluginMetadata& metadata) {
    m_abi = metadata.target;
    m_abi_version = metadata.target_version;
    m_major_version = ocpn::split(m_abi_version.c_str(), ".")[0];
    m_name = metadata.name;
    DEBUG_LOG << "Plugin: setting up, name: " << m_name;
    DEBUG_LOG << "Plugin: init: abi: " << m_abi
              << ", abi_version: " << m_abi_version
              << ", major ver: " << m_major_version;
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
    DEBUG_LOG << "Host: init: abi: " << m_abi
              << ", abi_version: " << m_abi_version
              << ", major ver: " << m_major_version;
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
        "debian-x86_64;sid;ubuntu-x86_64;24.04",

        "debian-arm64;11;ubuntu-gtk3-arm64;20.04",
        "debian-wx32-arm64;11;ubuntu-wx32-arm64;22.04",
        "debian-arm64;12;ubuntu-arm64;23.04",
        "debian-arm64;12;ubuntu-arm64;23.10",
        "debian-arm64;12;ubuntu-arm64;24.04",
        "debian-arm64;sid;ubuntu-arm64;24.04",

        "debian-armhf;10;ubuntu-armhf;18.04",
        "debian-gtk3-armhf;10;ubuntu-gtk3-armhf;18.04",
        "debian-armhf;11;ubuntu-gtk3-armhf;20.04",
        "debian-wx32-armhf;11;ubuntu-wx32-armhf;22.04",
        "debian-armhf;12;ubuntu-armhf;23.04",
        "debian-armhf;12;ubuntu-armhf;23.10",
        "debian-armhf;12;ubuntu-armhf;24.04",
        "debian-armhf;sid;ubuntu-armhf;24.04"};  // clang-format: on

    if (ocpn::startswith(plugin.abi(), "debian")) {
      DEBUG_LOG << "Checking for debian plugin on a ubuntu host";
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

CompatOs* CompatOs::GetInstance() {
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
      _name = std::string(tokens[0]) + std::string("-wx32");
      if (tokens.size() > 1) _name = _name + std::string("-") + tokens[1];
    }
  }

  _name = ocpn::tolower(_name);
  _version = ocpn::tolower(_version);
}

PluginHandler::PluginHandler() {}

bool PluginHandler::IsCompatible(const PluginMetadata& metadata, const char* os,
                                 const char* os_version) {
  static const SemanticVersion kMinApi = SemanticVersion(1, 16);
  static const SemanticVersion kMaxApi = SemanticVersion(1, 20);
  auto plugin_api = SemanticVersion::parse(metadata.api_version);
  if (plugin_api.major == -1) {
    DEBUG_LOG << "Cannot parse API version \"" << metadata.api_version << "\"";
    return false;
  }
  if (plugin_api < kMinApi || plugin_api > kMaxApi) {
    DEBUG_LOG << "Incompatible API version \"" << metadata.api_version << "\"";
    return false;
  }

  static const std::vector<std::string> simple_abis = {
      "msvc", "msvc-wx32", "android-armhf", "android-arm64"};

  Plugin plugin(metadata);
  if (plugin.abi() == "all") {
    DEBUG_LOG << "Returning true for plugin abi \"all\"";
    return true;
  }
  auto compatOS = CompatOs::GetInstance();
  Host host(compatOS);

  auto found = std::find(simple_abis.begin(), simple_abis.end(), plugin.abi());
  if (found != simple_abis.end()) {
    bool ok = plugin.abi() == host.abi();
    DEBUG_LOG << "Returning " << (ok ? "ok" : "fail") << " for " << host.abi();
    return ok;
  }
  bool rv = false;
  if (host.abi() == plugin.abi() && host.is_version_compatible(plugin)) {
    rv = true;
    DEBUG_LOG << "Found matching abi version " << plugin.abi_version();
  } else if (host.is_debian_plugin_compatible(plugin)) {
    rv = true;
    DEBUG_LOG << "Found Debian version matching Ubuntu host";
  }
  // macOS is an exception as packages with universal binaries can support both
  // x86_64 and arm64 at the same time
  if (host.abi() == "darwin-wx32" && plugin.abi() == "darwin-wx32") {
    OCPN_OSDetail* detail = g_BasePlatform->GetOSDetail();
    auto found = metadata.target_arch.find(detail->osd_arch);
    if (found != std::string::npos) {
      rv = true;
    }
  }
  DEBUG_LOG << "Plugin compatibility check Final: "
            << (rv ? "ACCEPTED: " : "REJECTED: ") << metadata.name;
  return rv;
}

std::string PluginHandler::FileListPath(std::string name) {
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return pluginsConfigDir() + SEP + name + ".files";
}

std::string PluginHandler::VersionPath(std::string name) {
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return pluginsConfigDir() + SEP + name + ".version";
}

std::string PluginHandler::ImportedMetadataPath(std::string name) {
  ;
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return importsDir() + SEP + name + ".xml";
}

typedef std::unordered_map<std::string, std::string> pathmap_t;

/**
 * Platform-specific mapping of tarball paths to user-writable
 * installation directories.
 */
static pathmap_t getInstallPaths() {
  using namespace std;

  pathmap_t pathmap;
  PluginPaths* paths = PluginPaths::GetInstance();
  pathmap["bin"] = paths->UserBindir();
  pathmap["lib"] = paths->UserLibdir();
  pathmap["lib64"] = paths->UserLibdir();
  pathmap["share"] = paths->UserDatadir();
  return pathmap;
}

static void saveFilelist(std::string filelist, std::string name) {
  using namespace std;
  string listpath = PluginHandler::FileListPath(name);
  ofstream diskfiles(listpath);
  if (!diskfiles.is_open()) {
    MESSAGE_LOG << "Cannot create installed files list.";
    return;
  }
  diskfiles << filelist;
}

static void saveDirlist(std::string name) {
  using namespace std;
  string path = dirListPath(name);
  ofstream dirs(path);
  if (!dirs.is_open()) {
    MESSAGE_LOG << "Cannot create installed files list.";
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
  string path = PluginHandler::VersionPath(name);
  ofstream stream(path);
  if (!stream.is_open()) {
    MESSAGE_LOG << "Cannot create version file.";
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
      MESSAGE_LOG << "Error copying install data: " << archive_error_string(aw);
      return (r);
    }
  }
}

static bool win_entry_set_install_path(struct archive_entry* entry,
                                       pathmap_t installPaths) {
  using namespace std;

  string path = archive_entry_pathname(entry);
  bool is_library = false;

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
    is_library = true;
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
    DEBUG_LOG << msg;
    return false;
  }
  if (is_library) {
    wxFileName nm(path);
    PluginLoader::MarkAsLoadable(nm.GetName().ToStdString());
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
    DEBUG_LOG << msg;
    return false;
  }
  string dest = installPaths[location] + "/" + suffix;
  archive_entry_set_pathname(entry, dest.c_str());

  PluginPaths* paths = PluginPaths::GetInstance();
  if (dest.find(paths->UserLibdir()) != std::string::npos) {
    wxFileName nm(path);
    PluginLoader::MarkAsLoadable(nm.GetName().ToStdString());
  }

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
    DEBUG_LOG << msg;
    return false;
  }

  bool is_library = false;
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
      is_library = true;
    }
  } else {
    if (ocpn::startswith(location, "lib") &&
        ocpn::startswith(suffix, "opencpn/") && ocpn::endswith(suffix, ".so")) {
      is_library = true;
    }
  }

  if (is_library) {
    wxFileName nm(suffix);
    PluginLoader::MarkAsLoadable(nm.GetName().ToStdString());
  }

  archive_entry_set_pathname(entry, dest.c_str());
  return true;
}

static bool apple_entry_set_install_path(struct archive_entry* entry,
                                         pathmap_t installPaths) {
  using namespace std;

  const string base = PluginPaths::GetInstance()->Homedir() +
                      "/Library/Application Support/OpenCPN";

  string path = archive_entry_pathname(entry);
  if (ocpn::startswith(path, "./")) path = path.substr(2);
  bool is_library = false;

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
      is_library = true;
    }
  }
  if (dest == "" && archive_entry_filetype(entry) == AE_IFREG) {
    wxString msg(_T("PluginHandler::Invalid install path on file: "));
    msg += wxString(path.c_str());
    DEBUG_LOG << msg;
    return false;
  }
  archive_entry_set_pathname(entry, dest.c_str());
  if (is_library) {
    wxFileName nm(dest);
    PluginLoader::MarkAsLoadable(nm.GetName().ToStdString());
  }

  return true;
}

static bool android_entry_set_install_path(struct archive_entry* entry,
                                           pathmap_t installPaths) {
  using namespace std;

  bool is_library = false;
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
    DEBUG_LOG << msg;
    return false;
  }

  if ((location == "lib") && ocpn::startswith(suffix, "opencpn")) {
    auto parts = split(suffix, "/");
    if (parts.size() == 2) suffix = parts[1];
    is_library = true;
  }

  if ((location == "share") && ocpn::startswith(suffix, "opencpn")) {
    auto parts = split(suffix, "opencpn/");
    if (parts.size() == 2) suffix = parts[1];
  }

  /// storage/emulated/0/android/data/org.opencpn.opencpn/files/opencpn/plugins/oesenc_pi/data/LUPPatch3.xml
  string dest = installPaths[location] + "/" + suffix;

  archive_entry_set_pathname(entry, dest.c_str());
  if (is_library) {
    wxFileName nm(suffix);
    PluginLoader::MarkAsLoadable(nm.GetName().ToStdString());
  }
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
    MESSAGE_LOG << "set_install_path() invoked, unsupported platform "
                << wxPlatformInfo::Get().GetOperatingSystemDescription();
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

bool PluginHandler::ArchiveCheck(int r, const char* msg, struct archive* a) {
  if (r < ARCHIVE_OK) {
    std::string s(msg);

    if (archive_error_string(a)) s = s + ": " + archive_error_string(a);
    MESSAGE_LOG << s;
    last_error_msg = s;
  }
  return r >= ARCHIVE_WARN;
}

bool PluginHandler::ExplodeTarball(struct archive* src, struct archive* dest,
                                   std::string& filelist,
                                   const std::string& metadata_path,
                                   bool only_metadata) {
  struct archive_entry* entry = 0;
  pathmap_t pathmap = getInstallPaths();
  bool is_metadata_ok = false;
  while (true) {
    int r = archive_read_next_header(src, &entry);
    if (r == ARCHIVE_EOF) {
      if (!is_metadata_ok) {
        MESSAGE_LOG << "Plugin tarball does not contain metadata.xml";
      }
      return is_metadata_ok;
    }
    if (!ArchiveCheck(r, "archive read header error", src)) {
      return false;
    }
    std::string path = archive_entry_pathname(entry);
    bool is_metadata = std::string::npos != path.find("metadata.xml");
    if (is_metadata) {
      is_metadata_ok = true;
      if (metadata_path == "") {
        continue;
      } else {
        archive_entry_set_pathname(entry, metadata_path.c_str());
        DEBUG_LOG << "Extracted metadata.xml to " << metadata_path;
      }
    } else if (!entry_set_install_path(entry, pathmap))
      continue;
    if (strlen(archive_entry_pathname(entry)) == 0) {
      continue;
    }
    if (!is_metadata && only_metadata) {
      continue;
    }
    if (!is_metadata) {
      filelist.append(std::string(archive_entry_pathname(entry)) + "\n");
    }
    r = archive_write_header(dest, entry);
    ArchiveCheck(r, "archive write install header error", dest);
    if (r >= ARCHIVE_OK && archive_entry_size(entry) > 0) {
      r = copy_data(src, dest);
      if (!ArchiveCheck(r, "archive copy data error", dest)) {
        return false;
      }
    }
    r = archive_write_finish_entry(dest);
    if (!ArchiveCheck(r, "archive finish write error", dest)) {
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
 *   @param path path to tarball
 *   @param filelist: On return contains a list of files installed.
 *   @param metadata_path: if non-empty, location where to store metadata,
 *   @param only_metadata: If true don't install any files, just extract
 *                         metadata.
 *   @return true if tarball contains metadata.xml file, false otherwise.
 *
 */
bool PluginHandler::ExtractTarball(const std::string path,
                                   std::string& filelist,
                                   const std::string metadata_path,
                                   bool only_metadata) {
  struct archive* src = archive_read_new();
  archive_read_support_filter_gzip(src);
  archive_read_support_format_tar(src);
  int r = archive_read_open_filename(src, path.c_str(), 10240);
  if (r != ARCHIVE_OK) {
    std::ostringstream os;
    os << "Cannot read installation tarball: " << path;
    MESSAGE_LOG << os.str();
    last_error_msg = os.str();
    return false;
  }
  struct archive* dest = archive_write_disk_new();
  archive_write_disk_set_options(dest, ARCHIVE_EXTRACT_TIME);
  bool ok = ExplodeTarball(src, dest, filelist, metadata_path, only_metadata);
  archive_read_free(src);
  archive_write_free(dest);
  return ok;
}

PluginHandler* PluginHandler::GetInstance() {
  static PluginHandler* instance = 0;
  if (!instance) {
    instance = new (PluginHandler);
  }
  return instance;
}

bool PluginHandler::IsPluginWritable(std::string name) {
  if (isRegularFile(PluginHandler::FileListPath(name).c_str())) {
    return true;
  }
  auto loader = PluginLoader::GetInstance();
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
    MESSAGE_LOG << "Non-existing plugins file: " << path;
  }
  return path;
}

static void parseMetadata(const std::string path, CatalogCtx& ctx) {
  using namespace std;

  MESSAGE_LOG << "PluginHandler: using metadata path: " << path;
  ctx.depth = 0;
  if (!ocpn::exists(path)) {
    MESSAGE_LOG << "Non-existing plugins metadata file: " << path;
    return;
  }
  ifstream ifpath(path);
  std::string xml((istreambuf_iterator<char>(ifpath)),
                  istreambuf_iterator<char>());
  ParseCatalog(xml, &ctx);
}

bool PluginHandler::InstallPlugin(const std::string& path,
                                  std::string& filelist,
                                  const std::string metadata_path,
                                  bool only_metadata) {
  if (!ExtractTarball(path, filelist, metadata_path, only_metadata)) {
    std::ostringstream os;
    os << "Cannot unpack plugin tarball at : " << path;
    MESSAGE_LOG << os.str();
    if (filelist != "") Cleanup(filelist, "unknown_name");
    last_error_msg = os.str();
    return false;
  }
  if (only_metadata) {
    return true;
  }
  struct CatalogCtx ctx;
  std::ifstream istream(metadata_path);
  std::stringstream buff;
  buff << istream.rdbuf();

  auto xml = std::string("<plugins>") + buff.str() + "</plugins>";
  ParseCatalog(xml, &ctx);
  auto name = ctx.plugins[0].name;
  auto version = ctx.plugins[0].version;
  saveFilelist(filelist, name);
  saveDirlist(name);
  saveVersion(name, version);

  return true;
}

std::string PluginHandler::GetMetadataPath() {
  if (metadataPath.size() > 0) {
    return metadataPath;
  }
  metadataPath = computeMetadataPath();
  DEBUG_LOG << "Using metadata path: " << metadataPath;
  return metadataPath;
}

std::string PluginHandler::GetUserMetadataPath() {
  std::string path = g_BasePlatform->GetPrivateDataDir().ToStdString();
  path += SEP;
  return path + "ocpn-plugins.xml";
}

const std::map<std::string, int> PluginHandler::GetCountByTarget() {
  auto plugins = GetInstalled();
  auto a = GetAvailable();
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

std::vector<std::string> PluginHandler::GetImportPaths() {
  return glob_dir(importsDir(), "*.xml");
}

void PluginHandler::CleanupFiles(const std::string& manifestFile,
                                 const std::string& plugname) {
  std::ifstream diskfiles(manifestFile);
  if (diskfiles.is_open()) {
    std::stringstream buffer;
    buffer << diskfiles.rdbuf();
    PluginHandler::Cleanup(buffer.str(), plugname);
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

void PluginHandler::Cleanup(const std::string& filelist,
                            const std::string& plugname) {
  MESSAGE_LOG << "Cleaning up failed install of " << plugname;

  std::vector<std::string> paths = LoadLinesFromFile(filelist);
  for (const auto& path : paths) {
    if (isRegularFile(path.c_str())) {
      int r = remove(path.c_str());
      if (r != 0) {
        MESSAGE_LOG << "Cannot remove file " << path << ": " << strerror(r);
      }
    }
  }
  for (const auto& path : paths) PurgeEmptyDirs(path);

  std::string path = PluginHandler::FileListPath(plugname);
  if (ocpn::exists(path)) remove(path.c_str());

  // Best effort tries, failures are non-critical
  remove(dirListPath(plugname).c_str());
  remove(PluginHandler::VersionPath(plugname).c_str());
}

/**
 * Return list of available, unique and compatible plugins from
 * configured XML catalog.
 */
std::vector<PluginMetadata> PluginHandler::getCompatiblePlugins() {
  /** Compare two PluginMetadata objects, a named c++ requirement. */
  struct metadata_compare {
    bool operator()(const PluginMetadata& lhs,
                    const PluginMetadata& rhs) const {
      return lhs.key() < rhs.key();
    }
  };

  std::vector<PluginMetadata> returnArray;

  std::set<PluginMetadata, metadata_compare> unique_plugins;
  for (const auto& plugin : GetAvailable()) {
    unique_plugins.insert(plugin);
  }
  for (const auto& plugin : unique_plugins) {
    if (IsCompatible(plugin)) {
      returnArray.push_back(plugin);
    }
  }
  return returnArray;
}

const std::vector<PluginMetadata> PluginHandler::GetAvailable() {
  using namespace std;
  CatalogCtx* ctx;

  auto catalogHandler = CatalogHandler::GetInstance();

  ctx = catalogHandler->GetActiveCatalogContext();
  auto status = catalogHandler->GetCatalogStatus();

  if (status == CatalogHandler::ServerStatus::OK) {
    catalogData.undef = false;
    catalogData.version = ctx->version;
    catalogData.date = ctx->date;
  }
  return ctx->plugins;
}

std::vector<std::string> PluginHandler::GetInstalldataPlugins() {
  std::vector<std::string> names;
  fs::path dirpath(PluginsInstallDataPath());
  for (const auto& entry : fs::directory_iterator(dirpath)) {
    const std::string name(entry.path().filename().string());
    if (ocpn::endswith(name, ".files"))
      names.push_back(ocpn::split(name.c_str(), ".")[0]);
  }
  return names;
}

const std::vector<PluginMetadata> PluginHandler::GetInstalled() {
  using namespace std;
  vector<PluginMetadata> plugins;

  auto loader = PluginLoader::GetInstance();
  for (unsigned int i = 0; i < loader->GetPlugInArray()->GetCount(); i += 1) {
    const PlugInContainer* p = loader->GetPlugInArray()->Item(i);
    PluginMetadata plugin;
    auto name = string(p->m_common_name);
    // std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    plugin.name = name;
    std::stringstream ss;
    ss << p->m_version_major << "." << p->m_version_minor;
    plugin.version = ss.str();
    plugin.readonly = !IsPluginWritable(plugin.name);
    string path = PluginHandler::VersionPath(plugin.name);
    if (path != "" && wxFileName::IsFileReadable(path)) {
      std::ifstream stream;
      stream.open(path, ifstream::in);
      stream >> plugin.version;
    }
    plugins.push_back(plugin);
  }
  return plugins;
}

void PluginHandler::SetInstalledMetadata(const PluginMetadata& pm) {
  auto loader = PluginLoader::GetInstance();
  ssize_t ix = PlugInIxByName(pm.name, loader->GetPlugInArray());
  if (ix == -1) return;  // no such plugin

  auto plugins = *loader->GetPlugInArray();
  plugins[ix]->m_managed_metadata = pm;
}

bool PluginHandler::InstallPlugin(PluginMetadata plugin, std::string path) {
  std::string filelist;
  if (!ExtractTarball(path, filelist)) {
    std::ostringstream os;
    os << "Cannot unpack plugin: " << plugin.name << " at " << path;
    MESSAGE_LOG << os.str();
    last_error_msg = os.str();
    PluginHandler::Cleanup(filelist, plugin.name);
    return false;
  }
  saveFilelist(filelist, plugin.name);
  saveDirlist(plugin.name);
  saveVersion(plugin.name, plugin.version);
  return true;
}

bool PluginHandler::InstallPlugin(PluginMetadata plugin) {
  std::string path = tmpfile_path();
  if (path.empty()) {
    MESSAGE_LOG << "Cannot create temporary file";
    path = "";
    return false;
  }
  std::ofstream stream;
  stream.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  DEBUG_LOG << "Downloading: " << plugin.name << std::endl;
  auto downloader = Downloader(plugin.tarball_url);
  downloader.download(&stream);

  return InstallPlugin(plugin, path);
}

bool PluginHandler::InstallPlugin(const std::string& path) {
  PluginMetadata metadata;
  if (!ExtractMetadata(path, metadata)) {
    MESSAGE_LOG << "Cannot extract metadata from tarball";
    return false;
  }
  return InstallPlugin(metadata, path);
}

bool PluginHandler::ExtractMetadata(const std::string& path,
                                    PluginMetadata& metadata) {
  std::string filelist;
  std::string temp_path = tmpfile_path();
  if (!ExtractTarball(path, filelist, temp_path, true)) {
    std::ostringstream os;
    os << "Cannot unpack plugin " << metadata.name << " tarball at: " << path;
    MESSAGE_LOG << os.str();
    if (filelist != "") Cleanup(filelist, "unknown_name");
    last_error_msg = os.str();
    return false;
  }
  if (!isRegularFile(temp_path.c_str())) {
    // This could happen if the tarball does not contain the metadata.xml file
    // or the metadata.xml file could not be extracted.
    return false;
  }

  struct CatalogCtx ctx;
  std::ifstream istream(temp_path);
  std::stringstream buff;
  buff << istream.rdbuf();
  int r = remove(temp_path.c_str());
  if (r != 0) {
    MESSAGE_LOG << "Cannot remove file " << temp_path << ":" << strerror(r);
  }
  auto xml = std::string("<plugins>") + buff.str() + "</plugins>";
  ParseCatalog(xml, &ctx);
  metadata = ctx.plugins[0];
  if (metadata.name.empty()) {
    MESSAGE_LOG << "Plugin metadata is empty";
  }
  return !metadata.name.empty();
}

bool PluginHandler::ClearInstallData(const std::string plugin_name) {
  auto ix = PlugInIxByName(plugin_name,
                           PluginLoader::GetInstance()->GetPlugInArray());
  if (ix != -1) {
    MESSAGE_LOG << "Attempt to remove installation data for loaded plugin";
    return false;
  }
  return DoClearInstallData(plugin_name);
}

bool PluginHandler::DoClearInstallData(const std::string plugin_name) {
  std::string path = PluginHandler::FileListPath(plugin_name);
  if (!ocpn::exists(path)) {
    MESSAGE_LOG << "Cannot find installation data for " << plugin_name << " ("
                << path << ")";
    return false;
  }
  std::vector<std::string> plug_paths = LoadLinesFromFile(path);
  for (const auto& p : plug_paths) {
    if (isRegularFile(p.c_str())) {
      int r = remove(p.c_str());
      if (r != 0) {
        MESSAGE_LOG << "Cannot remove file " << p << ": " << strerror(r);
      }
    }
  }
  for (const auto& p : plug_paths) PurgeEmptyDirs(p);
  int r = remove(path.c_str());
  if (r != 0) {
    MESSAGE_LOG << "Cannot remove file " << path << ": " << strerror(r);
  }
  // Best effort tries, failures are OK.
  remove(dirListPath(plugin_name).c_str());
  remove(PluginHandler::VersionPath(plugin_name).c_str());
  remove(PluginHandler::ImportedMetadataPath(plugin_name).c_str());
  return true;
}

bool PluginHandler::Uninstall(const std::string plugin) {
  using namespace std;

  auto loader = PluginLoader::GetInstance();
  auto ix = PlugInIxByName(plugin, loader->GetPlugInArray());
  if (ix < 0) {
    MESSAGE_LOG << "trying to Uninstall non-existing plugin " << plugin;
    return false;
  }
  auto pic = loader->GetPlugInArray()->Item(ix);

  // Capture library file name before pic dies.
  string libfile = pic->m_plugin_file.ToStdString();
  loader->UnLoadPlugIn(ix);

  bool ok = DoClearInstallData(plugin);

  //  If this is an orphan plugin, there may be no installation record
  //  So make sure that the library file (.so/.dylib/.dll) is removed
  //  as a minimum best effort requirement
  if (isRegularFile(libfile.c_str())) {
    remove(libfile.c_str());
  }

  return ok;
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
    if (dir.IsOpened()) {
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
  }
  return "";
}

/**
 * Look in library dirs for matching .dll/.so/.dylib and use matched name.
 * @return matched part of found library name or "" if not found
 */
static std::string FindMatchingLibFile(std::regex name_re) {
  using namespace std;
  for (const auto& lib : PluginPaths::GetInstance()->Libdirs()) {
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
  for (const auto& plugin : PluginHandler::GetInstance()->GetInstalled()) {
    if (ocpn::tolower(plugin.name) == lc_name) return plugin.name;
  }
  for (const auto& plugin : PluginHandler::GetInstance()->GetAvailable()) {
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
    MESSAGE_LOG << "Cannot open " << path << ": " << strerror(errno);
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
  wxDir root(PluginHandler::PluginsInstallDataPath());
  if (!root.IsOpened()) return;
  wxString filename;
  bool cont = root.GetFirst(&filename, "*.files", wxDIR_FILES);
  while (cont) {
    auto path = root.GetNameWithSep() + filename;
    LoadPluginMapFile(map, path.ToStdString());
    cont = root.GetNext(&filename);
  }
}

std::string PluginHandler::GetPluginByLibrary(const std::string& filename) {
  auto basename = wxFileName(filename).GetFullName().ToStdString();
  if (FilesByPlugin.size() == 0) LoadPluginMap(FilesByPlugin);
  for (const auto& it : FilesByPlugin) {
    auto found = std::find(it.second.begin(), it.second.end(), basename);
    if (found != it.second.end()) return it.first;
  }
  return "";
}

bool PluginHandler::InstallPluginFromCache(PluginMetadata plugin) {
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
    MESSAGE_LOG << "Installing " << tarballFile << " from local cache";
    bool bOK = InstallPlugin(plugin, cacheFile);
    if (!bOK) {
      evt_download_failed.Notify(cacheFile);
      return false;
    }
    evt_download_ok.Notify(plugin.name + " " + plugin.version);
    return true;
  }
  return false;
}
