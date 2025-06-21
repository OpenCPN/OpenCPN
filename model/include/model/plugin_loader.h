/**************************************************************************
 *   Copyright (C) 2010 - 2023 by David S. Register                        *
 *   Copyright (C) 2023 - 2025  Alec Leamas                                *
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

/**
 * \file
 * Low level code to load plugins from disk, notably the PluginLoader class
 */

#ifndef PLUGIN_LOADER_H_GUARD
#define PLUGIN_LOADER_H_GUARD

#include <functional>

#include <wx/wx.h>
#include <wx/bitmap.h>
#include <wx/dynarray.h>
#include <wx/dynlib.h>

#include "config.h"

#include "model/catalog_parser.h"
#include "model/plugin_blacklist.h"
#include "model/semantic_vers.h"
#include "observable_evtvar.h"
#include "ocpn_plugin.h"

enum class PluginStatus {
  System,     //!< One of the four system plugins, unmanaged.
  Managed,    //!< Managed by installer.
  Unmanaged,  //!< Unmanaged, probably a package.
  Ghost,      //!< Managed, shadowing another (packaged?) plugin.
  Unknown,
  LegacyUpdateAvailable,
  ManagedInstallAvailable,
  ManagedInstalledUpdateAvailable,
  ManagedInstalledCurrentVersion,
  ManagedInstalledDowngradeAvailable,
  Imported,
  PendingListRemoval
};

class PlugInContainer;  // forward

/** Basic data for a loaded plugin, trivially copyable */
class PlugInData {
public:
  /** Create a container with applicable fields defined from metadata. */
  explicit PlugInData(const PluginMetadata& md);

  PlugInData();

  bool m_has_setup_options;  //!< Has run NotifySetupOptionsPlugin()
  bool m_enabled;
  bool m_init_state;
  bool m_toolbox_panel;
  int m_cap_flag;                    //!< PlugIn Capabilities descriptor
  wxString m_plugin_file;            //!< The full file path
  wxString m_plugin_filename;        //!< The short file path
  wxDateTime m_plugin_modification;  //!< used to detect upgraded plugins
  wxString m_common_name;            //!< A common name string for the plugin
  wxString m_short_description;
  wxString m_long_description;
  int m_api_version;
  int m_version_major;
  int m_version_minor;
  PluginStatus m_status;
  PluginMetadata m_managed_metadata;
  wxBitmap m_bitmap;
  wxString m_version_str;          //!< Complete version as of semantic_vers
  std::string m_manifest_version;  //!< As detected from manifest

  /** sort key. */
  std::string Key() const;
};

/**
 * Data for a loaded plugin, including dl-loaded library.
 * Due to the library it is not copyable.
 */
class PlugInContainer : public PlugInData {
public:
  PlugInContainer();

  ~PlugInContainer() = default;

  opencpn_plugin* m_pplugin;
  wxDynamicLibrary m_library;
  destroy_t* m_destroy_fn;
};

/** Error condition when loading a plugin. */
class LoadError {
public:
  enum class Type {
    Unloadable,  //<! wrong magic, wrong type of binary...
    Unreadable,
    Incompatible,
    NoCreate,   //<! Missing linkage (is this a plugin?)
    NoDestroy,  //<! Missing linkage (is this a plugin?)
    Blacklisted
  } type;
  const std::string lib_path;  //<! Complete path to failing library
  //<! As determined from plugin API
  const SemanticVersion plugin_version;  //<! As determined from plugin API

  LoadError(Type t, const std::string& l, SemanticVersion pv)
      : type(t), lib_path(l), plugin_version(pv) {}

  LoadError(Type t, std::string l)
      : type(t), lib_path(std::move(l)), plugin_version(SemanticVersion()) {}
};

WX_DEFINE_ARRAY_PTR(PlugInContainer*, ArrayOfPlugIns);

/**
 * PluginLoader is a backend module without any direct GUI functionality.
 * Instead, it generates events listed here. Each event is mirrored by a
 * public variable, by convention having the same name in lower case.
 *
 * The general usage pattern to process events, here using EVT_LOAD_PLUGIN:
 *
 *   PluginLoader::GetInstance()->evt_load_plugin.listen(this, EVT_LOAD_PLUGIN)
 *   Bind(EVT_LOAD_PLUGIN, [&](ObservedEvt ev) {
 *          code to run on event...
 *   });
 *
 * The code in plugin_loader uses evt_load_plugin.Notify() to trigger the
 * event. Notify() might have a string or void* argument; these are
 * available as ev.GetString() or ev.GetClientData() . There is a also a
 * generic std::shared_ptr available as using GetSharedPtr();
 *
 * See: PlugInManager::PlugInManager() in pluginmanager.cpp
 */
class PluginLoader {
public:
  static PluginLoader* GetInstance();
  virtual ~PluginLoader() = default;

  /**
   * Mark a library file (complete path) as loadable i. e., remove possible
   * stamp
   */
  static void MarkAsLoadable(const std::string& library_path);
  /**
   *  Update PlugInContainer status using data from PluginMetadata and manifest.
   */
  static void UpdatePlugin(PlugInContainer* plugin, const PluginMetadata& md);

  /** Return version string for a plugin, possibly with an "Imported" suffix. */
  static std::string GetPluginVersion(
      const PlugInData pd,
      std::function<const PluginMetadata(const std::string&)> get_metadata);

  /** Find metadata for given plugin. */
  static PluginMetadata MetadataByName(const std::string& name);

  /** Find highest versioned metadata for given plugin. */
  static PluginMetadata LatestMetadataByName(const std::string& name);

  /** Notified without data when loader starts loading from a new directory. */
  EventVar evt_load_directory;

  /**
   * Notified with a PlugInContainer* pointer when a plugin is loaded.
   * The pointer should be treated as const and is owned by loader
   */
  EventVar evt_load_plugin;

  /** Notified without data when the GetPlugInArray() list is changed. */
  EventVar evt_pluglist_change;

  /**
   * Carries a malloc'ed read-only copy of a PlugInContainer owned: by
   * listener.
   */
  EventVar evt_deactivate_plugin;

  /** Notified without data after all plugins loaded ot updated. */
  EventVar evt_update_chart_types;

  /**
   * Emitted after all plugins are loaded. Event carries
   * a std::vector<LoadError> available though GetSharedPtr()
   */
  EventVar evt_plugin_loadall_finalize;

  /**
   * Update catalog with imported metadata and load all plugin library files.
   *
   * @param enabled_plugins If true, only load enabled plugins
   * @param keep_orphans If true, don't scrub plugins not available in the
   *                     catalog.
   * @return false on load errors, else true.
   */
  bool LoadAllPlugIns(bool enabled_plugins, bool keep_orphans = false);

  const wxBitmap* GetPluginDefaultIcon();
  void SetPluginDefaultIcon(const wxBitmap* bitmap);

  /** Callback invoked in late stage on deactivating a plugin. */
  void SetOnDeactivateCb(std::function<void(const PlugInContainer*)> cb) {
    m_on_deactivate_cb = cb;
  }
  /** Display the preferences dialog for a plugin. */
  void ShowPreferencesDialog(const PlugInData& pd, wxWindow* parent);

  void NotifySetupOptionsPlugin(const PlugInData* pic);

  /** Remove a plugin from *GetPluginArray().  */
  void RemovePlugin(const PlugInData& pd);

  /** Sort GetPluginArray(). */
  void SortPlugins(int (*cmp_func)(PlugInContainer**, PlugInContainer**));

  /** Unload, delete and remove item ix in GetPlugInArray(). */
  bool UnLoadPlugIn(size_t ix);

  /** Unload allplugins i. e., release the dynamic libraries. */
  bool UnLoadAllPlugIns();

  /** Deactivate all plugins. */
  bool DeactivateAllPlugIns();

  /** Deactivate given plugin. */
  bool DeactivatePlugIn(PlugInContainer* pic);

  /** Deactivate given plugin. */
  bool DeactivatePlugIn(const PlugInData& pic);

  /** Update the GetPlugInArray() list by reloading all plugins from disk. */
  bool UpdatePlugIns();

  /**
   * Update all managed plugins i. e., everything besides system, imported
   * or legacy plugins.
   * @param keep_orphans If true, Keep any inactive/uninstalled managed plugins
   * that are no longer available in the current catalog. Otherwise, remove
   * them. Orphans usuall occur after reverting from Alpha/Beta catalog back
   * to master.
   */
  void UpdateManagedPlugins(bool keep_orphans);

  /** Load given plugin file from disk into GetPlugInArray() list. */
  PlugInContainer* LoadPlugIn(const wxString& plugin_file);

  /** Load given plugin file from disk into GetPlugInArray() list. */
  PlugInContainer* LoadPlugIn(const wxString& plugin_file,
                              PlugInContainer* pic);

  /** Return list of currently loaded plugins. */
  const ArrayOfPlugIns* GetPlugInArray() { return &plugin_array; }

  /** Return true if a plugin with given name exists in GetPlugInArray() */
  bool IsPlugInAvailable(const wxString& commonName);

  /**
   * Check plugin compatibiliet w r t library type. Very platform
   * dependent.
   * @return true if library is deemed compatible.
   */
  bool CheckPluginCompatibility(const wxString& plugin_file);

  /** Update enabled/disabled state for plugin with given name. */
  void SetEnabled(const wxString& common_name, bool enabled);

  /** Update m_toolbox_panel state for plugin with given name. */
  void SetToolboxPanel(const wxString& common_name, bool value);

  /** Update m_has_setup_options state for plugin with given name. */
  void SetSetupOptions(const wxString& common_name, bool value);

private:
  PluginLoader();
  bool LoadPlugInDirectory(const wxString& plugin_dir, bool load_enabled);
  bool LoadPluginCandidate(const wxString& file_name, bool load_enabled);
  std::unique_ptr<AbstractBlacklist> m_blacklist;
  ArrayOfPlugIns plugin_array;
  wxString m_last_error_string;
  wxString m_plugin_location;

#ifdef __WXMSW__
  wxString m_module_name;
  bool m_found_wxwidgets;
#endif

  const wxBitmap* m_default_plugin_icon;
  std::function<void(const PlugInContainer*)> m_on_deactivate_cb;

  std::vector<LoadError> load_errors;
};

#endif  // PLUGIN_LOADER_H_GUARD
