/**************************************************************************
 *   Copyright (C) 2019 - 2025 Alec Leamas                                 *
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
 ***************************************************************************/

/**
 * \file
 * PLugin remote repositories installation and Uninstall/list operations
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

#include "model/catalog_parser.h"
#include "observable_evtvar.h"

bool isRegularFile(const char* path);

/** Internal helper wrapping host OS and version. */
class CompatOs {
public:
  static CompatOs* GetInstance();
  std::string name() const { return _name; }
  std::string version() const { return _version; }

private:
  CompatOs();
  std::string _name;
  std::string _version;
};

/**
 * Handle plugin install from remote repositories and local operations
 * to Uninstall and list plugins. The plugin maintains an internal list
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
 */
class PluginHandler {
public:
  /** Singleton factory. */
  static PluginHandler* GetInstance();

  PluginHandler(const PluginHandler&) = delete;
  PluginHandler& operator=(const PluginHandler&) = delete;

  /**
   * Notified with plugin name + version string after successful download
   * from repository
   */
  EventVar evt_download_ok;

  /** Notified with plugin name after failed download attempt. */
  EventVar evt_download_failed;

  /** Cleanup failed installation attempt using filelist for plugin. */
  static void Cleanup(const std::string& filelist, const std::string& plugname);

  static void CleanupFiles(const std::string& manifestFile,
                           const std::string& plugname);

  /** Return base directory for installation data. */
  static std::string PluginsInstallDataPath();

  /** Return path to installation manifest for given plugin. */
  static std::string FileListPath(std::string name);

  /** Return path to file containing version for given plugin. */
  static std::string VersionPath(std::string name);

  /** Return path to imported metadata for given plugin. */
  static std::string ImportedMetadataPath(std::string name);

  /** List of paths for imported plugins metadata. */
  static std::vector<std::string> GetImportPaths();

  /** Return true if given plugin is loadable on given os/version. */
  static bool IsCompatible(const PluginMetadata& metadata,
                           const char* os = PKG_TARGET,
                           const char* os_version = PKG_TARGET_VERSION);

  /** Check if given plugin can be installed/updated. */
  bool IsPluginWritable(std::string name);

  /** Return list of all installed  and loaded plugins. */
  const std::vector<PluginMetadata> GetInstalled();

  /**
   *  Return list of installed plugins lower case names, not necessarily
   *  loaded
   */
  std::vector<std::string> GetInstalldataPlugins();

  /** Set metadata for an installed plugin */
  void SetInstalledMetadata(const PluginMetadata& pm);

  /** Update catalog and return list of available, not installed plugins. */
  const std::vector<PluginMetadata> GetAvailable();

  /**
   * Return list of available, unique and compatible plugins from
   * configured XML catalog.
   */
  std::vector<PluginMetadata> getCompatiblePlugins();

  /** Map of available plugin targets -> number of occurences. */
  const std::map<std::string, int> GetCountByTarget();

  /** Return plugin containing given filename or "" if not found. */
  std::string GetPluginByLibrary(const std::string& filename);

  /** Return path to metadata XML file. */
  std::string GetMetadataPath();

  /** Return path to user, writable metadata XML file. */
  std::string GetUserMetadataPath();

  /** Set path to metadata XML file. */
  void setMetadata(std::string path) { metadataPath = path; }

  /** Download and install a new, not installed plugin. */
  bool InstallPlugin(PluginMetadata plugin);

  /** Install a new, downloaded but not installed plugin tarball. */
  bool InstallPlugin(PluginMetadata plugin, std::string path);

  /** Extract metadata in given tarball path. */
  bool ExtractMetadata(const std::string& path, PluginMetadata& metadata);

  /* Install a new, downloaded but not installed plugin tarball. */
  bool InstallPlugin(const std::string& path);

  /** Uninstall an installed  and loaded plugin. */
  bool Uninstall(const std::string plugin);

  /** Remove installation data for not loaded plugin. */
  bool ClearInstallData(const std::string plugin_name);

  /** Install plugin tarball from local cache. */
  bool InstallPluginFromCache(PluginMetadata plugin);

  CatalogData* GetCatalogData() { return &catalogData; }

protected:
  PluginHandler();

private:
  std::string metadataPath;
  std::vector<PluginMetadata> installed;
  CatalogData catalogData;
  std::string last_error_msg;
  bool InstallPlugin(const std::string& path, std::string& filelist,
                     const std::string metadata_path, bool only_metadata);

  /**
   * Internal helper function to extract a tarball into platform-specific user
   * directories.
   *
   *   @param path: Path to tarball.
   *   @param filelist: On return contains a list of files installed.
   *   @param metadata_path: If non-empty, location where to extract plugin
   * metadata.xml file.
   *   @param only_metadata: If true don't install any files, just extract
   *                         the metadata.xml file.
   *   @return true if tarball could be extracted and contains metadata.xml
   * file. false otherwise.
   */
  bool ExplodeTarball(struct archive* src, struct archive* dest,
                      std::string& filelist, const std::string& metadata_path,
                      bool only_metadata);
  /**
   * Extract a tarball into platform-specific user directories.
   *
   *   @param path: Path to tarball.
   *   @param filelist: On return contains a list of files installed.
   *   @param metadata_path: If non-empty, location where to extract plugin
   * metadata.xml file.
   *   @param only_metadata: If true don't install any files, just extract
   *                         the metadata.xml file.
   *   @return true if tarball could be extracted and contains metadata.xml
   * file. false otherwise.
   */
  bool ExtractTarball(const std::string path, std::string& filelist,
                      const std::string metadata_path = "",
                      bool only_metadata = false);
  bool ArchiveCheck(int r, const char* msg, struct archive* a);

  std::unordered_map<std::string, std::vector<std::string>> FilesByPlugin;

  bool DoClearInstallData(const std::string plugin_name);
};

#endif  // PLUGIN_HANDLER_H__
