/***************************************************************************
 *
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifndef _PLUGIN_LOADER_H_
#define _PLUGIN_LOADER_H_

#include <functional>

#include <wx/wx.h>
#include <wx/bitmap.h>
#include <wx/dynarray.h>
#include <wx/dynlib.h>

#include "config.h"

#include "catalog_parser.h"
#include "observable_evtvar.h"
#include "ocpn_plugin.h"
#include "plugin_blacklist.h"
#include "semantic_vers.h"

typedef struct {
  wxString name;      //!< name of the plugin
  int version_major;  //!< major version
  int version_minor;  //!< minor version
  /**
   * hard blacklist - if true, don't load it at all, if false, load
   * it and just warn the user
   */
  bool hard;
  /**
   * if true, blacklist also all the lower versions of the
   * plugin
   */
  bool all_lower;
  bool mute_dialog;  //!< if true, don't warn the user by dialog.
} BlackListedPlugin;

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
  PendingListRemoval
};

//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Container Specification
//
//-----------------------------------------------------------------------------------------------------
class PlugInContainer {
public:
  PlugInContainer();
  ~PlugInContainer() { delete m_bitmap; }

  opencpn_plugin* m_pplugin;
  bool m_bEnabled;
  bool m_bInitState;
  bool m_bToolboxPanel;
  int m_cap_flag;                    //!< PlugIn Capabilities descriptor
  wxString m_plugin_file;            //!< The full file path
  wxString m_plugin_filename;        //!< The short file path
  wxDateTime m_plugin_modification;  //!< used to detect upgraded plugins
  destroy_t* m_destroy_fn;
  wxDynamicLibrary m_library;
  wxString m_common_name;  //!< A common name string for the plugin
  wxString m_short_description;
  wxString m_long_description;
  int m_api_version;
  int m_version_major;
  int m_version_minor;
  PluginStatus m_pluginStatus;
  PluginMetadata m_ManagedMetadata;
  wxBitmap* m_bitmap;
  /**
   * Return version from plugin API. Older pre-117 plugins just
   * support major and minor version, newer plugins have
   * complete semantic version data.
   */
  SemanticVersion GetVersion();

  wxString m_version_str;  //!< Complete version as of semantic_vers
  std::string m_InstalledManagedVersion;  //!< As detected from manifest
};

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
  const std::string lib_path;            //<! Complete path to failing library
  const int api_version;                 //<! As determined from plugin API
  const SemanticVersion plugin_version;  //<! As determined from plugin API

  LoadError(Type t, const std::string& l, int av, SemanticVersion pv)
      : type(t), lib_path(l), api_version(av), plugin_version(pv) {}

  LoadError(Type t, const std::string& l, int av)
      : type(t),
        lib_path(l),
        api_version(av),
        plugin_version(SemanticVersion()) {}

  LoadError(Type t, const std::string& l)
      : type(t),
        lib_path(l),
        api_version(0),
        plugin_version(SemanticVersion()) {}
};

//    Declare an array of PlugIn Containers
WX_DEFINE_ARRAY_PTR(PlugInContainer*, ArrayOfPlugIns);

/**
 * PluginLoader is a backend module without any direct GUI functionality.
 * Instead, it generates events listed here. Each event is mirrored by a
 * public variable, by convention having the same name in lower case.
 *
 * The general usage pattern to process events, here using EVT_LOAD_PLUGIN:
 *
 *   PluginLoader::getInstance()->evt_load_plugin.listen(this, EVT_LOAD_PLUGIN)
 *   Bind(EVT_LOAD_PLUGIN, [&](ObservedEvt ev) {
 *          code to run on event...
 *   });
 *
 * The code in plugin_loader uses evt_load_plugin.Notify() to trigger the
 * event. Notify() might have a string or void* argument; these are
 * available as ev.GetString() or ev.GetClientData() in the Bind() lambda
 * function. There is a also a generic std::shared_ptr available as using
 * GetSharedPtr();
 *
 * Examples: PlugInManager::PlugInManager() in pluginmanager.cpp
 */
class PluginLoader {
public:
  static PluginLoader* getInstance();
  virtual ~PluginLoader() {}

  EventVar evt_blacklisted_plugin;

  EventVar evt_load_directory;
  EventVar evt_load_plugin;
  EventVar evt_plugin_unload;
  EventVar evt_pluglist_change;
  EventVar evt_unreadable_plugin;

  /**
   *  Carries a malloc'ed read-only copy of a PlugInContainer owned by listener.
   */
  EventVar evt_deactivate_plugin;

  EventVar evt_update_chart_types;

  /**
   * Emitted after all plugins are loaded. Event carries
   * a std::vector<LoadError> available though GetSharedPtr()
   */
  EventVar evt_plugin_loadall_finalize;

  EventVar evt_version_incompatible_plugin;

  bool LoadAllPlugIns(bool enabled_plugins);

  void SetPluginDefaultIcon(const wxBitmap* bitmap);

  /** Callback invoked in late stage on deactivating a plugin. */
  void SetOnDeactivateCb(std::function<void(const PlugInContainer*)> cb) {
    m_on_deactivate_cb = cb;
  }

  const wxBitmap* GetPluginDefaultIcon();

  /** Unload, delete and remove item ix in GetPlugInArray(). */
  bool UnLoadPlugIn(size_t ix);

  bool UnLoadAllPlugIns();
  bool DeactivateAllPlugIns();
  bool DeactivatePlugIn(PlugInContainer* pic);
  bool UpdatePlugIns();
  void UpdateManagedPlugins();
  PlugInContainer* LoadPlugIn(wxString plugin_file);
  PlugInContainer* LoadPlugIn(wxString plugin_file, PlugInContainer* pic);

  ArrayOfPlugIns* GetPlugInArray() { return &plugin_array; }
  bool IsPlugInAvailable(wxString commonName);
  bool CheckPluginCompatibility(wxString plugin_file);

private:
  PluginLoader();
  bool LoadPlugInDirectory(const wxString& plugin_dir, bool load_enabled);
  bool LoadPluginCandidate(wxString file_name, bool load_enabled);
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

#endif  // _PLUGIN_LOADER_H_
