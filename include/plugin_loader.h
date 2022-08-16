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

#include <wx/wx.h>
#include <wx/bitmap.h>
#include <wx/dynarray.h>
#include <wx/dynlib.h>

#include "config.h"

#include "catalog_parser.h"
#include "observable_evtvar.h"
#include "ocpn_plugin.h"
#include "semantic_vers.h"



typedef struct {
  wxString name;      // name of the plugin
  int version_major;  // major version
  int version_minor;  // minor version
  bool hard;  // hard blacklist - if true, don't load it at all, if false, load
              // it and just warn the user
  bool all_lower;  // if true, blacklist also all the lower versions of the
                   // plugin
  bool mute_dialog;  // if true, don't warn the user by dialog.
} BlackListedPlugin;

const BlackListedPlugin PluginBlacklist[] = {
    {_T("aisradar_pi"), 0, 95, true, true},
    {_T("radar_pi"), 0, 95, true, true},  // GCC alias for aisradar_pi
    {_T("watchdog_pi"), 1, 00, true, true},
    {_T("squiddio_pi"), 0, 2, true, true},
    {_T("objsearch_pi"), 0, 3, true, true},
#ifdef __WXOSX__
    {_T("s63_pi"), 0, 6, true, true},
#endif
    {_T("oesenc_pi"), 4, 2, true, true},

};


enum class PluginStatus {
  System,     // One of the four system plugins, unmanaged.
  Managed,    // Managed by installer.
  Unmanaged,  // Unmanaged, probably a package.
  Ghost,      // Managed, shadowing another (packaged?) plugin.
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

  opencpn_plugin* m_pplugin;
  bool m_bEnabled;
  bool m_bInitState;
  bool m_bToolboxPanel;
  int m_cap_flag;                    // PlugIn Capabilities descriptor
  wxString m_plugin_file;            // The full file path
  wxString m_plugin_filename;        // The short file path
  wxDateTime m_plugin_modification;  // used to detect upgraded plugins
  destroy_t* m_destroy_fn;
  wxDynamicLibrary m_library;
  wxString m_common_name;  // A common name string for the plugin
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
  wxString m_version_str;  // Complete version as of
                           // semantic_vers
  std::string m_InstalledManagedVersion;  // As detected from manifest
};

//    Declare an array of PlugIn Containers
WX_DEFINE_ARRAY_PTR(PlugInContainer* , ArrayOfPlugIns);

//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Manager Specification
//
//-----------------------------------------------------------------------------------------------------

/**
 * PluginLoader is a backend module without any direct GUI functionality.
 * Instead, it generates events listed here. Each event is mirrored by a
 * public variable, by convention having the same name in lower case.
 *
 * The general usage pattern to process events, here using EVT_LOAD_PLUGIN:
 *
 *   PluginLoader::getInstance()->evt_load_plugin.listen(this, EVT_LOAD_PLUGIN)
 *   Bind(EVT_LOAD_PLUGIN, [&](wxCommandEvent ev) {
 *          code to run on event...
 *   });
 *
 * The code in plugin_loader uses evt_load_plugin.notify() to trigger the
 * event. notify() might have a string or void* argument; these are
 * available as ev.GetString() or ev.GetClientData() in the Bind() lambda
 * function.
 *
 * Examples: PlugInManager::PlugInManager() in pluginmanager.cpp
 */

class PluginLoader {
public:
  EventVar evt_blacklisted_plugin;

  /** Receives a malloc'ed copy of a PlugInContainer owned by listener. */
  EventVar evt_deactivate_plugin;

  EventVar evt_incompatible_plugin;
  EventVar evt_load_directory;
  EventVar evt_load_plugin;
  EventVar evt_plugin_unload;
  EventVar evt_pluglist_change;
  EventVar evt_unreadable_plugin;
  EventVar evt_update_chart_types;
  EventVar evt_plugin_loadall_finalize;
  EventVar evt_version_incompatible_plugin;


  static PluginLoader* getInstance();
  virtual ~PluginLoader() {}

  bool LoadAllPlugIns(bool enabled_plugins);

  void SetPluginDefaultIcon(const wxBitmap* bitmap);
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
  bool CheckBlacklistedPlugin(opencpn_plugin* plugin);

private:
  PluginLoader();
  bool LoadPlugInDirectory(const wxString &plugin_dir, bool load_enabled);
  bool LoadPluginCandidate(wxString file_name, bool load_enabled);
  ArrayOfPlugIns plugin_array;
  wxString m_last_error_string;
  wxString m_plugin_location;
  const wxBitmap* m_default_plugin_icon;
};

#endif  // _PLUGIN_LOADER_H_
