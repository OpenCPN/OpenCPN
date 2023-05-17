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

/**
 * Handle plugin install from remote repositories and local operations
 * to uninstall and list plugins. The plugin maintains an internal list
 * and is a singleton.
 *
 * Remote repositories are based on XML files describing metadata and
 * addresses to tarballs with compiled and installed files, basically
 * under there different directories.
 *
 *    - The plugin directory e. g., /usr/lib/opencpn/
 *    - The data directory e. g., /usr/share/data/opencpn/plugins.
 *    - The binary directory (additional program) e. g, /usr/bin.
 *
 * The compiled plugins are installed to user-writable location(s)
 *
 *    - Linux: under ~/.local, possibly relocated by environment variables.
 *    - Windows: under &GetWinPluginBaseDir() which is configurable,
 *      defaults to %APPDATA%/Local/opencpn
 *    - Flatpak: under ~/.var/app/org.opencpn.OpenCPN
 *
 * However, plugins are loaded from multiple locations basically
 * corresponding to new and old lacations:
 *
 *    - Windows: GetSharedDataDirPtr():GetWinPluginBaseDir()
 *    - linux: As defined  by XDG_DATA_DIRS, defaulting to
 *      ~/.local/lib/opencpn:/usr/local/lib/opencpn:/usr/lib/opencpn
 *    - flatpak:
 *        /app/lib/opencpn:/app/extensions/lib/opencpn:~/.var/app/opencpn/lib
 *
 */

#ifndef PLUGIN_HANDLER_H__
#define PLUGIN_HANDLER_H__

#include "config.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <wx/cmdline.h>

#include <archive.h>

#include "catalog_parser.h"
#include "observable_evtvar.h"

bool isRegularFile(const char* path);

class CompatOs {
public:
  static CompatOs* getInstance();
  std::string name() const { return _name; }
  std::string version() const { return _version; }

private:
  CompatOs();
  std::string _name;
  std::string _version;
};

class PluginHandler {
public:

  EventVar evt_download_ok;
  EventVar evt_download_failed;

  static PluginHandler* getInstance();

  /** Cleanup failed installation attempt using filelist for plugin. */
  static void cleanup(const std::string& filelist, const std::string& plugname);
  static void cleanupFiles(const std::string& manifestFile,
                           const std::string& plugname);

  /** Return base directory for installation data. */
  static std::string pluginsInstallDataPath();

  /** Return path to installation manifest for given plugin. */
  static std::string fileListPath(std::string name);

  /** Return path to file containing version for given plugin. */
  static std::string versionPath(std::string name);

  /** Return path to imported metadata for given plugin. */
  static std::string ImportedMetadataPath(std::string name);

  /** List of paths for imported plugins metadata. */
  static std::vector<std::string> GetImportPaths();

  /** Return true if given plugin is loadable on given os/version. */
  static bool isCompatible(const PluginMetadata& metadata,
                           const char* os = PKG_TARGET,
                           const char* os_version = PKG_TARGET_VERSION);

  /** Check if given plugin can be installed/updated. */
  bool isPluginWritable(std::string name);

  /** Return list of all installed plugins. */
  const std::vector<PluginMetadata> getInstalled();

  /** Set metadata for an installed plugin */
  void SetInstalledMetadata(const PluginMetadata& pm);

  /** Update catalog and return list of available, not installed plugins. */
  const std::vector<PluginMetadata> getAvailable();

  /** Map of available plugin targets -> number of occurences. */
  const std::map<std::string, int> getCountByTarget();


  /** Return plugin containing given filename or "" if not found. */
  std::string getPluginByLibrary(const std::string& filename);

  /** Return path to metadata XML file. */
  std::string getMetadataPath();

  /** Set path to metadata XML file. */
  void setMetadata(std::string path) { metadataPath = path; }

  /** Download and install a new, not installed plugin. */
  bool installPlugin(PluginMetadata plugin);

  /** Install a new, downloaded but not installed plugin tarball. */
  bool installPlugin(PluginMetadata plugin, std::string path);

  /**
   * Install a new, downloaded but not installed plugin tarball,
   * returning metadata for installed plugin.
   */
  bool installPlugin(const std::string& path, PluginMetadata& metadata);

  /* Install a new, downloaded but not installed plugin tarball. */
  bool installPlugin(const std::string& path);

  /** Uninstall an installed plugin. */
  bool uninstall(const std::string plugin);

  /** Install plugin tarball from local cache. */
  bool installPluginFromCache(PluginMetadata plugin);

  std::string getLastErrorMsg() { return last_error_msg; }

  CatalogData* GetCatalogData() { return &catalogData; }

protected:
  PluginHandler();

private:
  std::string metadataPath;
  std::vector<PluginMetadata> installed;
  CatalogData catalogData;
  std::string last_error_msg;
  bool explodeTarball(struct archive* src, struct archive* dest,
                      std::string& filelist,
                      const std::string& metadata_path);
  bool extractTarball(const std::string path, std::string& filelist,
                      const std::string metadata_path = "");
  bool archive_check(int r, const char* msg, struct archive* a);
  std::unordered_map<std::string, std::vector<std::string>> files_by_plugin;
};

#endif  // PLUGIN_HANDLER_H__
