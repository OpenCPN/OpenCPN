/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2022 Alec Leamas                                        *
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

#include "config.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>

#ifdef USE_LIBELF
#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#include <wordexp.h>
#endif

#ifndef WIN32
#include <cxxabi.h>
#endif

#include <wx/wx.h>  //  NOLINT
#include <wx/bitmap.h>
#include <wx/dir.h>
#include <wx/event.h>
#include <wx/hashset.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/window.h>
#include <wx/process.h>

#include "base_platform.h"
#include "config_vars.h"
#include "ocpn_utils.h"
#include "logger.h"
#include "observable_confvar.h"
#include "plugin_blacklist.h"
#include "plugin_cache.h"
#include "plugin_handler.h"
#include "plugin_loader.h"
#include "plugin_paths.h"
#include "safe_mode.h"
#include "chartdb.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#include <dlfcn.h>
#endif

#ifdef __WXMSW__
#include <Psapi.h>
#endif

extern BasePlatform* g_BasePlatform;
extern wxWindow* gFrame;
extern ChartDB* ChartData;
extern bool g_bportable;

static const std::vector<std::string> SYSTEM_PLUGINS = {
    "chartdownloader", "wmm", "dashboard", "grib"};

/** Return complete PlugInContainer matching pic. */
static PlugInContainer* GetContainer(const PlugInData& pd,
                                     const ArrayOfPlugIns& plugin_array) {
  for (size_t i = 0; i < plugin_array.GetCount(); i++) {
    const auto& p = plugin_array.Item(i);
    if (p->m_common_name == pd.m_common_name) return p;
  }
  return nullptr;
}

/** Return true if path "seems" to contain a system plugin */
static bool IsSystemPluginPath(const std::string& path) {
  static const std::vector<std::string> kPlugins = {
      "chartdldr_pi", "wmm_pi", "dashboard_pi", "grib_pi"};

  const std::string lc_path = ocpn::tolower(path);
  for (const auto& p : kPlugins)
    if (lc_path.find(p) != std::string::npos) return true;
  return false;
}

/** Return true if name is a valid system plugin name. */
static bool IsSystemPluginName(const std::string& name) {
  static const std::vector<std::string> kPlugins = {
      "chartdownloader", "wmm", "dashboard", "grib"};
  auto found =
      std::find(kPlugins.begin(), kPlugins.end(), ocpn::tolower(name));
  return found != kPlugins.end();
}


std::string PluginLoader::GetPluginVersion(
    const PlugInData pd,
    std::function<const PluginMetadata(const std::string&)> get_metadata) {
  auto loader = PluginLoader::getInstance();
  auto pic = GetContainer(pd, *loader->GetPlugInArray());
  if (!pic) {
    return SemanticVersion(0, 0, -1).to_string();
  }

  PluginMetadata metadata;
  metadata = pic->m_managed_metadata;
  if (metadata.version == "")
    metadata = get_metadata(pic->m_common_name.ToStdString());
  std::string import_suffix(metadata.is_imported ? _(" [Imported]") : "");

  int v_major(0);
  int v_minor(0);
  if (pic->m_pplugin) {
    v_major = pic->m_pplugin->GetPlugInVersionMajor();
    v_minor = pic->m_pplugin->GetPlugInVersionMinor();
  }
  auto p = dynamic_cast<opencpn_plugin_117*>(pic->m_pplugin);
  if (p) {
    // New style plugin, trust version available in the API.
    auto v = SemanticVersion(
        v_major, v_minor, p->GetPlugInVersionPatch(), p->GetPlugInVersionPost(),
        p->GetPlugInVersionPre(), p->GetPlugInVersionBuild());
    return v.to_string() + import_suffix;
  } else {
    return SemanticVersion(v_major, v_minor, -1).to_string() + import_suffix;
  }
}

PlugInContainer::PlugInContainer()
    : PlugInData(), m_pplugin(nullptr), m_library(), m_destroy_fn(nullptr) {}

PlugInData::PlugInData()
    : m_has_setup_options(false),
      m_enabled(false),
      m_init_state(false),
      m_toolbox_panel(false),
      m_cap_flag(0),
      m_api_version(0),
      m_version_major(0),
      m_version_minor(0),
      m_status(PluginStatus::Unknown) {}

PlugInData::PlugInData(const PluginMetadata& md) : PlugInData() {
  m_common_name = wxString(md.name);
  auto v = SemanticVersion::parse(md.version);
  m_version_major = v.major;
  m_version_minor = v.minor;
  m_managed_metadata = md;
  m_status = PluginStatus::ManagedInstallAvailable;
  m_enabled = false;
}

std::string PlugInData::Key() const {
  return std::string(m_status == PluginStatus::Managed ? "1" : "0") +
         m_common_name.ToStdString();
}

//-----------------------------------------------------------------------------------------------------
//
//          Plugin Loader Implementation
//
//-----------------------------------------------------------------------------------------------------

/** Events sent to GUI */

/**
 *
 * For linux, set up LD_LIBRARY_PATH to the same value as the path used
 * to load plugins, assuring helper binaries can load libraries installed
 * in the same directory as the plugin.mac is handled the same way using
 * DYLD_LIBRARY_PATH. For windows, setup PATH in the likewise.
 */
static void setLoadPath() {
  using namespace std;

  auto const osSystemId = wxPlatformInfo::Get().GetOperatingSystemId();
  auto dirs = PluginPaths::getInstance()->Libdirs();
  if (osSystemId & wxOS_UNIX_LINUX) {
    string path = ocpn::join(dirs, ':');
    wxString envPath;
    if (wxGetEnv("LD_LIBRARY_PATH", &envPath)) {
      path = path + ":" + envPath.ToStdString();
    }
    wxLogMessage("Using LD_LIBRARY_PATH: %s", path.c_str());
    wxSetEnv("LD_LIBRARY_PATH", path.c_str());
  } else if (osSystemId & wxOS_WINDOWS) {
    // On windows, Libdirs() and Bindirs() are the same.
    string path = ocpn::join(dirs, ';');
    wxString envPath;
    if (wxGetEnv("PATH", &envPath)) {
      path = path + ";" + envPath.ToStdString();
    }
    wxLogMessage("Using PATH: %s", path);
    wxSetEnv("PATH", path);
  } else if (osSystemId & wxOS_MAC) {
    string path = ocpn::join(dirs, ':');
    wxString envPath;
    if (wxGetEnv("DYLD_LIBRARY_PATH", &envPath)) {
      path = path + ":" + envPath.ToStdString();
    }
    wxLogMessage("Using DYLD_LIBRARY_PATH: %s", path.c_str());
    wxSetEnv("DYLD_LIBRARY_PATH", path.c_str());
  } else {
    wxString os_name = wxPlatformInfo::Get().GetPortIdName();
    if (os_name.Contains("wxQT")) {
      wxLogMessage("setLoadPath() using Android library path");
    } else
      wxLogWarning("SetLoadPath: Unsupported platform.");
  }
  if (osSystemId & wxOS_MAC || osSystemId & wxOS_UNIX_LINUX) {
    dirs = PluginPaths::getInstance()->Bindirs();
    string path = ocpn::join(dirs, ':');
    wxString envPath;
    wxGetEnv("PATH", &envPath);
    path = path + ":" + envPath.ToStdString();
    wxLogMessage("Using PATH: %s", path);
    wxSetEnv("PATH", path);
  }
}

static void ProcessLateInit(PlugInContainer* pic) {
  if (pic->m_cap_flag & WANTS_LATE_INIT) {
    wxString msg("PluginLoader: Calling LateInit PlugIn: ");
    msg += pic->m_plugin_file;
    wxLogMessage(msg);

    auto ppi = dynamic_cast<opencpn_plugin_110*>(pic->m_pplugin);
    if (ppi) ppi->LateInit();
  }
}

PluginLoader* PluginLoader::getInstance() {
  static PluginLoader* instance = nullptr;

  if (!instance) instance = new PluginLoader();
  return instance;
}

PluginLoader::PluginLoader()
    : m_blacklist(blacklist_factory()),
      m_default_plugin_icon(nullptr),
#ifdef __WXMSW__
      m_found_wxwidgets(false),
#endif
      m_on_deactivate_cb([](const PlugInContainer* pic) {}) {}

bool PluginLoader::IsPlugInAvailable(const wxString& commonName) {
  for (unsigned int i = 0; i < plugin_array.GetCount(); i++) {
    PlugInContainer* pic = plugin_array[i];
    if (pic && pic->m_enabled && (pic->m_common_name == commonName))
      return true;
  }
  return false;
}

void PluginLoader::ShowPreferencesDialog(const PlugInData& pd,
                                         wxWindow* parent) {
  auto loader = PluginLoader::getInstance();
  auto pic = GetContainer(pd, *loader->GetPlugInArray());
  if (pic) pic->m_pplugin->ShowPreferencesDialog(parent);
}

void PluginLoader::NotifySetupOptionsPlugin(const PlugInData* pd) {
  auto pic = GetContainer(*pd, *GetPlugInArray());
  if (!pic) return;
  if (pic->m_has_setup_options) return;
  pic->m_has_setup_options = true;
  if (pic->m_enabled && pic->m_init_state) {
    if (pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE) {
      switch (pic->m_api_version) {
        case 109:
        case 110:
        case 111:
        case 112:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118: {
          if (pic->m_pplugin) {
            opencpn_plugin_19 *ppi =
                dynamic_cast<opencpn_plugin_19 *>(pic->m_pplugin);
            if (ppi) {
              ppi->OnSetupOptions();
              auto loader = PluginLoader::getInstance();
              loader->SetToolboxPanel(pic->m_common_name, true);
            }
            break;
          }
        }
        default:
          break;
      }
    }
  }
}

void PluginLoader::SetEnabled(const wxString& common_name, bool enabled) {
  for (size_t i = 0; i < plugin_array.GetCount(); i++) {
    PlugInContainer* pic = plugin_array[i];
    if (pic->m_common_name == common_name) {
      pic->m_enabled = enabled;
      return;
    }
  }
}

void PluginLoader::SetToolboxPanel(const wxString& common_name, bool value) {
  for (size_t i = 0; i < plugin_array.GetCount(); i++) {
    PlugInContainer* pic = plugin_array[i];
    if (pic->m_common_name == common_name) {
      pic->m_toolbox_panel = value;
      return;
    }
  }
  wxLogMessage("Atttempt to update toolbox panel on non-existing plugin " +
               common_name);
}

const wxBitmap* PluginLoader::GetPluginDefaultIcon() {
  if (!m_default_plugin_icon) m_default_plugin_icon = new wxBitmap(32, 32);
  return m_default_plugin_icon;
}

void PluginLoader::SetPluginDefaultIcon(const wxBitmap* bitmap) {
  delete m_default_plugin_icon;
  m_default_plugin_icon = bitmap;
}

void PluginLoader::RemovePlugin(const PlugInData& pd) {
  auto pic = GetContainer(pd, plugin_array);
  if (!pic) {
    wxLogMessage("Attempt to remove non-existing plugin %s",
                 pd.m_common_name.ToStdString().c_str());
    return;
  }
  plugin_array.Remove(pic);
}

static int ComparePlugins(PlugInContainer** p1, PlugInContainer** p2) {
  return (*p1)->Key().compare((*p2)->Key());
}

void PluginLoader::SortPlugins(int (*cmp_func)(PlugInContainer**,
                                               PlugInContainer**)) {
  plugin_array.Sort(ComparePlugins);
}

bool PluginLoader::LoadAllPlugIns(bool load_enabled) {
  using namespace std;

  static const wxString sep = wxFileName::GetPathSeparator();
  vector<string> dirs = PluginPaths::getInstance()->Libdirs();
  wxLogMessage("PluginLoader: loading plugins from %s", ocpn::join(dirs, ';'));
  setLoadPath();
  bool any_dir_loaded = false;
  for (const auto& dir : dirs) {
    wxString wxdir(dir);
    wxLogMessage("Loading plugins from dir: %s", wxdir.mb_str().data());
    if (LoadPlugInDirectory(wxdir, load_enabled)) any_dir_loaded = true;
  }

  // Read the default ocpn-plugins.xml, and update/merge the plugin array
  // This only needs to happen when the entire universe (enabled and disabled)
  // of plugins are loaded for management.
  if (!load_enabled) UpdateManagedPlugins();

  // Some additional actions needed after all plugins are loaded.
  evt_update_chart_types.Notify();
  auto errors = std::make_shared<std::vector<LoadError>>(load_errors);
  evt_plugin_loadall_finalize.Notify(errors, "");
  load_errors.clear();

  return any_dir_loaded;
}

bool PluginLoader::LoadPluginCandidate(const wxString& file_name,
                                       bool load_enabled) {
  wxString plugin_file = wxFileName(file_name).GetFullName();
  wxLogMessage("Checking plugin candidate: %s", file_name.mb_str().data());
  wxDateTime plugin_modification = wxFileName(file_name).GetModificationTime();
  wxLog::FlushActive();

  // this gets called every time we switch to the plugins tab.
  // this allows plugins to be installed and enabled without restarting
  // opencpn. For this reason we must check that we didn't already load this
  // plugin
  bool loaded = false;
  PlugInContainer* loaded_pic = nullptr;
  for (unsigned int i = 0; i < plugin_array.GetCount(); i++) {
    PlugInContainer* pic_test = plugin_array[i];

    // Checking for dynamically updated plugins
    if (pic_test->m_plugin_filename == plugin_file) {
      // Do not re-load same-name plugins from different directories.  Certain
      // to crash...
      if (pic_test->m_plugin_file == file_name) {
        if (pic_test->m_plugin_modification != plugin_modification) {
          // modification times don't match, reload plugin
          plugin_array.Remove(pic_test);
          i--;

          DeactivatePlugIn(pic_test);
          pic_test->m_destroy_fn(pic_test->m_pplugin);

          delete pic_test;
        } else {
          loaded = true;
          loaded_pic = pic_test;
          break;
        }
      } else {
        loaded = true;
        loaded_pic = pic_test;
        break;
      }
    }
  }

  if (loaded) return true;

  // Avoid loading/testing legacy plugins installed in base plugin path.
  wxFileName fn_plugin_file(file_name);
  wxString plugin_file_path =
      fn_plugin_file.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
  wxString base_plugin_path = g_BasePlatform->GetPluginDir();
  if (!base_plugin_path.EndsWith(wxFileName::GetPathSeparator()))
    base_plugin_path += wxFileName::GetPathSeparator();

  if (!g_bportable) {
    if (base_plugin_path.IsSameAs(plugin_file_path)) {
      if (!IsSystemPluginPath(file_name.ToStdString())) {
        DEBUG_LOG << "Skipping plugin " << file_name << " in "
                  << g_BasePlatform->GetPluginDir();
        return false;
      }
    }
  }

  if (!IsSystemPluginPath(file_name.ToStdString()) && safe_mode::get_mode()) {
    DEBUG_LOG << "Skipping plugin " << file_name << " in safe mode";
    return false;
  }

  auto msg =
      std::string("Checking plugin compatibility: ") + file_name.ToStdString();
  wxLogMessage(msg.c_str());
  wxLog::FlushActive();

  bool b_compat = CheckPluginCompatibility(file_name);

  if (!b_compat) {
    msg =
        std::string("Incompatible plugin detected: ") + file_name.ToStdString();
    wxLogMessage(msg.c_str());
    if (m_blacklist->mark_unloadable(file_name.ToStdString())) {
      LoadError le(LoadError::Type::Unloadable, file_name.ToStdString());
      load_errors.push_back(le);
    }
    return false;
  }

  PlugInContainer* pic = LoadPlugIn(file_name);

  // Check the config file to see if this PlugIn is user-enabled,
  // only loading enabled plugins.
  // Make the check late enough to pick up incompatible plugins anyway
  const auto path = std::string("/PlugIns/") + plugin_file.ToStdString();
  ConfigVar<bool> enabled(path, "bEnabled", TheBaseConfig());
  if (load_enabled && !enabled.Get(true)) {
    delete pic;
    wxLogMessage("Skipping not enabled candidate.");
    return true;
  }

  if (pic) {
    if (pic->m_pplugin) {
      plugin_array.Add(pic);

      //    The common name is available without initialization and startup of
      //    the PlugIn
      pic->m_common_name = pic->m_pplugin->GetCommonName();
      pic->m_plugin_filename = plugin_file;
      pic->m_plugin_modification = plugin_modification;
      pic->m_enabled = enabled.Get(false);

      if (safe_mode::get_mode()) {
        pic->m_enabled = false;
        enabled.Set(false);
      }
#ifndef CLIAPP
      // The CLI has no graphics context, but plugins assumes there is.
      if (pic->m_enabled) {
        pic->m_cap_flag = pic->m_pplugin->Init();
        pic->m_init_state = true;
        evt_load_plugin.Notify(pic);
      }
#endif
      wxLog::FlushActive();

      std::string found_version;
      for (const auto& p : PluginHandler::getInstance()->getInstalled()) {
        if (p.name == pic->m_common_name.Lower()) {
          found_version = p.readonly ? "" : p.version;
          break;
        }
      }
      pic->m_version_str = found_version;
      pic->m_short_description = pic->m_pplugin->GetShortDescription();
      pic->m_long_description = pic->m_pplugin->GetLongDescription();
      pic->m_version_major = pic->m_pplugin->GetPlugInVersionMajor();
      pic->m_version_minor = pic->m_pplugin->GetPlugInVersionMinor();

      auto pbm0 = pic->m_pplugin->GetPlugInBitmap();
      if (!pbm0->IsOk()) {
        pbm0 = (wxBitmap *)GetPluginDefaultIcon();
      }
      pic->m_bitmap = wxBitmap(pbm0->GetSubBitmap(
          wxRect(0, 0, pbm0->GetWidth(), pbm0->GetHeight())));

      if (!pic->m_enabled && pic->m_destroy_fn) {
        pic->m_destroy_fn(pic->m_pplugin);
        pic->m_destroy_fn = nullptr;
        pic->m_pplugin = nullptr;
        pic->m_init_state = false;
        if (pic->m_library.IsLoaded()) pic->m_library.Unload();
      }

    } else {  //  No pic->m_pplugin
      wxLogMessage(
          "    PluginLoader: Unloading invalid PlugIn, API version %d ",
          pic->m_api_version);
      pic->m_destroy_fn(pic->m_pplugin);

      LoadError le(LoadError::Type::Unloadable, file_name.ToStdString());
      delete pic;
      load_errors.push_back(le);
      return false;
    }
  } else {  // pic == 0
    return false;
  }
  return true;
}

// Helper function: loads all plugins from a single directory
bool PluginLoader::LoadPlugInDirectory(const wxString& plugin_dir,
                                       bool load_enabled) {
  evt_load_directory.Notify();
  m_plugin_location = plugin_dir;

  wxString msg("PluginLoader searching for PlugIns in location ");
  msg += m_plugin_location;
  wxLogMessage(msg);

#ifdef __WXMSW__
  wxString pispec = "*_pi.dll";
#elif defined(__WXOSX__)
  wxString pispec = "*_pi.dylib";
#else
  wxString pispec = "*_pi.so";
#endif

  if (!::wxDirExists(m_plugin_location)) {
    msg = m_plugin_location;
    msg.Prepend("   Directory ");
    msg.Append(" does not exist.");
    wxLogMessage(msg);
    return false;
  }

  if (!g_BasePlatform->isPlatformCapable(PLATFORM_CAP_PLUGINS)) return false;

  wxArrayString file_list;

  int get_flags = wxDIR_FILES | wxDIR_DIRS;
#ifdef __WXMSW__
#ifdef _DEBUG
  get_flags = wxDIR_FILES;
#endif
#endif

#ifdef __ANDROID__
  get_flags = wxDIR_FILES;  // No subdirs, especially "/files" where PlugIns are
                            // initially placed in APK
#endif

  bool ret =
      false;  // return true if at least one new plugins gets loaded/unloaded
  wxDir::GetAllFiles(m_plugin_location, &file_list, pispec, get_flags);

  wxLogMessage("Found %d candidates", (int)file_list.GetCount());
  for (unsigned int i = 0; i < file_list.GetCount(); i++) {
    wxLog::FlushActive();

    wxString file_name = file_list[i];

    LoadPluginCandidate(file_name, load_enabled);
  }

  // Scrub the plugin array...
  // Here, looking for duplicates caused by new installation of a plugin
  // We want to remove the previous entry representing the uninstalled packaged
  // plugin metadata
  for (unsigned int i = 0; i < plugin_array.GetCount(); i++) {
    PlugInContainer* pic = plugin_array[i];
    for (unsigned int j = i + 1; j < plugin_array.GetCount(); j++) {
      PlugInContainer* pict = plugin_array[j];

      if (pic->m_common_name == pict->m_common_name) {
        if (pic->m_plugin_file.IsEmpty())
          plugin_array.Item(i)->m_status = PluginStatus::PendingListRemoval;
        else
          plugin_array.Item(j)->m_status = PluginStatus::PendingListRemoval;
      }
    }
  }

  //  Remove any list items marked
  size_t i = 0;
  while ((i >= 0) && (i < plugin_array.GetCount())) {
    PlugInContainer* pict = plugin_array.Item(i);
    if (pict->m_status == PluginStatus::PendingListRemoval) {
      plugin_array.RemoveAt(i);
      i = 0;
    } else
      i++;
  }

  return ret;
}

bool PluginLoader::UpdatePlugIns() {
  bool bret = false;

  for (unsigned int i = 0; i < plugin_array.GetCount(); i++) {
    PlugInContainer* pic = plugin_array[i];

    // Try to confirm that the m_pplugin member points to a valid plugin
    // image...
    if (pic->m_pplugin) {
      auto ppl = dynamic_cast<opencpn_plugin*>(pic->m_pplugin);
      if (!ppl) {
        pic->m_pplugin = nullptr;
        pic->m_init_state = false;
      }
    }

    // Installed and loaded?
    if (!pic->m_pplugin) {  // Needs a reload?
      if (pic->m_enabled) {
        PluginStatus stat = pic->m_status;
        PlugInContainer* newpic = LoadPlugIn(pic->m_plugin_file, pic);
        if (newpic) {
          pic->m_status = stat;
          pic->m_enabled = true;
        }
      } else
        continue;
    }

    if (pic->m_enabled && !pic->m_init_state && pic->m_pplugin) {
      wxString msg("PluginLoader: Initializing PlugIn: ");
      msg += pic->m_plugin_file;
      wxLogMessage(msg);

      pic->m_cap_flag = pic->m_pplugin->Init();
      pic->m_pplugin->SetDefaults();
      pic->m_init_state = true;
      ProcessLateInit(pic);
      pic->m_short_description = pic->m_pplugin->GetShortDescription();
      pic->m_long_description = pic->m_pplugin->GetLongDescription();
      pic->m_version_major = pic->m_pplugin->GetPlugInVersionMajor();
      pic->m_version_minor = pic->m_pplugin->GetPlugInVersionMinor();
      wxBitmap* pbm0 = pic->m_pplugin->GetPlugInBitmap();
      pic->m_bitmap = wxBitmap(pbm0->GetSubBitmap(
          wxRect(0, 0, pbm0->GetWidth(), pbm0->GetHeight())));
      bret = true;
    } else if (!pic->m_enabled && pic->m_init_state) {
      // Save a local copy of the plugin icon before unloading
      wxBitmap* pbm0 = pic->m_pplugin->GetPlugInBitmap();
      pic->m_bitmap = wxBitmap(pbm0->GetSubBitmap(
          wxRect(0, 0, pbm0->GetWidth(), pbm0->GetHeight())));

      bret = DeactivatePlugIn(pic);
      if (pic->m_pplugin) pic->m_destroy_fn(pic->m_pplugin);
      if (pic->m_library.IsLoaded()) pic->m_library.Unload();
      pic->m_pplugin = nullptr;
      pic->m_init_state = false;
    }
  }
  evt_update_chart_types.Notify();
  return bret;
}

bool PluginLoader::DeactivatePlugIn(PlugInContainer* pic) {
  if (!pic) return false;
  if (pic->m_init_state) {
    wxString msg("PluginLoader: Deactivating PlugIn: ");
    wxLogMessage(msg + pic->m_plugin_file);
    m_on_deactivate_cb(pic);
    pic->m_init_state = false;
    pic->m_pplugin->DeInit();
  }
  return true;
}

bool PluginLoader::DeactivatePlugIn(const PlugInData& pd) {
  auto pic = GetContainer(pd, plugin_array);
  if (!pic) {
    wxLogError("Attempt to deactivate non-existing plugin %s",
               pd.m_common_name.ToStdString());
    return false;
  }
  return DeactivatePlugIn(pic);
}

/**
 * Return list of available, unique and compatible plugins from
 * configured XML catalog.
 */
// FIXME: Move to PluginHandler.
static std::vector<PluginMetadata> getCompatiblePlugins() {
  /** Compare two PluginMetadata objects, a named c++ requirement. */
  struct metadata_compare {
    bool operator()(const PluginMetadata& lhs,
                    const PluginMetadata& rhs) const {
      return lhs.key() < rhs.key();
    }
  };

  std::vector<PluginMetadata> returnArray;

  std::set<PluginMetadata, metadata_compare> unique_plugins;
  for (const auto& plugin : PluginHandler::getInstance()->getAvailable()) {
    unique_plugins.insert(plugin);
  }
  for (const auto& plugin : unique_plugins) {
    if (PluginHandler::isCompatible(plugin)) {
      returnArray.push_back(plugin);
    }
  }
  return returnArray;
}

bool PluginLoader::UnLoadPlugIn(size_t ix) {
  if (ix >= plugin_array.GetCount()) {
    wxLogWarning("Attempt to remove non-existing plugin %d", ix);
    return false;
  }
  PlugInContainer* pic = plugin_array[ix];
  if (!DeactivatePlugIn(pic)) {
    return false;
  }
  if (pic->m_pplugin) {
    pic->m_destroy_fn(pic->m_pplugin);
  }

  delete pic;  // This will unload the PlugIn via DTOR of pic->m_library
  plugin_array.RemoveAt(ix);
  return true;
}

static std::string VersionFromManifest(const std::string& plugin_name) {
  std::string version;
  std::string path = PluginHandler::versionPath(plugin_name);
  if (!path.empty() && wxFileName::IsFileReadable(path)) {
    std::ifstream stream;
    stream.open(path, std::ifstream::in);
    stream >> version;
  }
  return version;
}

/** Find metadata for given plugin. */
PluginMetadata PluginLoader::MetadataByName(const std::string& name) {
  using namespace std;
  if (name.empty()) return {};

  auto available = getCompatiblePlugins();
  vector<PluginMetadata> matches;
  copy_if(available.begin(), available.end(), back_inserter(matches),
          [name](const PluginMetadata& md) { return md.name == name; });
  if (matches.size() == 0) return {};
  if (matches.size() == 1) return matches[0];  // only one found with given name

  auto version = VersionFromManifest(name);
  auto predicate = [version](const PluginMetadata& md) {
    return version == md.version;
  };
  auto found = find_if(matches.begin(), matches.end(), predicate);
  return found != matches.end() ? *found : matches[0];
}

/** Update PlugInContainer using data from PluginMetadata and manifest. */
void PluginLoader::UpdatePlugin(PlugInContainer* plugin,
                                const PluginMetadata& md) {
  auto found = std::find(SYSTEM_PLUGINS.begin(), SYSTEM_PLUGINS.end(),
                         plugin->m_common_name.Lower());
  bool is_system = found != SYSTEM_PLUGINS.end();

  std::string installed = VersionFromManifest(md.name);
  plugin->m_manifest_version = installed;
  auto installedVersion = SemanticVersion::parse(installed);
  auto metaVersion = SemanticVersion::parse(md.version);

  if (is_system)
    plugin->m_status = PluginStatus::System;
  else if (installedVersion < metaVersion)
    plugin->m_status = PluginStatus::ManagedInstalledUpdateAvailable;
  else if (installedVersion == metaVersion)
    plugin->m_status = PluginStatus::ManagedInstalledCurrentVersion;
  else
    plugin->m_status = PluginStatus::ManagedInstalledDowngradeAvailable;

  plugin->m_managed_metadata = md;
}

void PluginLoader::UpdateManagedPlugins() {
  std::vector<PlugInContainer*> loaded_plugins;
  for (size_t i = 0; i < plugin_array.GetCount(); i++)
    loaded_plugins.push_back(plugin_array.Item(i));

  // Initiate status to "unmanaged" or "system" on all plugins
  for (auto& p : loaded_plugins) {
    auto found = std::find(SYSTEM_PLUGINS.begin(), SYSTEM_PLUGINS.end(),
                           p->m_common_name.Lower().ToStdString());
    bool is_system = found != SYSTEM_PLUGINS.end();
    p->m_status = is_system ? PluginStatus::System : PluginStatus::Unmanaged;
  }

  // Remove any inactive/uninstalled managed plugins that are no longer
  // available in the current catalog Usually due to reverting from Alpha/Beta
  // catalog back to master
  auto predicate = [](const PlugInContainer* pd) -> bool {
    const auto md(
        PluginLoader::MetadataByName(pd->m_common_name.ToStdString()));
    return md.name.empty() && !pd->m_pplugin &&
        !IsSystemPluginName(pd->m_common_name.ToStdString());
  };
  auto end =
      std::remove_if(loaded_plugins.begin(), loaded_plugins.end(), predicate);
  loaded_plugins.erase(end, loaded_plugins.end());

  //  Update from the catalog metadata
  for (auto& plugin : loaded_plugins) {
    auto md = PluginLoader::MetadataByName(plugin->m_common_name.ToStdString());
    if (!md.name.empty()) {
      auto import_path = PluginHandler::ImportedMetadataPath(md.name.c_str());
      md.is_imported = isRegularFile(import_path.c_str());
      if (isRegularFile(PluginHandler::fileListPath(md.name).c_str())) {
        // This is an installed plugin
        PluginLoader::UpdatePlugin(plugin, md);
      } else if (IsSystemPluginName(md.name)) {
        plugin->m_status = PluginStatus::System;
      } else if (plugin->m_api_version) {
        // If the plugin is actually loaded, but the new plugin is known not
        // to be installed, then it must be a legacy plugin loaded.
        plugin->m_status = PluginStatus::LegacyUpdateAvailable;
        plugin->m_managed_metadata = md;
      } else {
        // Otherwise, this is an uninstalled managed plugin.
        plugin->m_status = PluginStatus::ManagedInstallAvailable;
      }
    }
  }

  plugin_array.Clear();
  for (const auto& p : loaded_plugins) plugin_array.Add(p);
  evt_pluglist_change.Notify();
}

bool PluginLoader::UnLoadAllPlugIns() {
  bool rv = true;
  while (plugin_array.GetCount()) {
    if (!UnLoadPlugIn(0)) {
      rv = false;
    }
  }
  return rv;
}

bool PluginLoader::DeactivateAllPlugIns() {
  for (unsigned int i = 0; i < plugin_array.GetCount(); i++) {
    PlugInContainer* pic = plugin_array[i];
    if (pic && pic->m_enabled && pic->m_init_state) DeactivatePlugIn(pic);
  }
  return true;
}

#ifdef __WXMSW__
/*Convert Virtual Address to File Offset */
DWORD Rva2Offset(DWORD rva, PIMAGE_SECTION_HEADER psh, PIMAGE_NT_HEADERS pnt) {
  size_t i = 0;
  PIMAGE_SECTION_HEADER pSeh;
  if (rva == 0) {
    return (rva);
  }
  pSeh = psh;
  for (i = 0; i < pnt->FileHeader.NumberOfSections; i++) {
    if (rva >= pSeh->VirtualAddress &&
        rva < pSeh->VirtualAddress + pSeh->Misc.VirtualSize) {
      break;
    }
    pSeh++;
  }
  return (rva - pSeh->VirtualAddress + pSeh->PointerToRawData);
}
#endif

class ModuleInfo {
public:
  ModuleInfo() : type_magic(0) {}
  WX_DECLARE_HASH_SET(wxString, wxStringHash, wxStringEqual, DependencySet);
  WX_DECLARE_HASH_MAP(wxString, wxString, wxStringHash, wxStringEqual,
                      DependencyMap);

  uint64_t type_magic;
  DependencyMap dependencies;
};

#ifdef USE_LIBELF
bool ReadModuleInfoFromELF(const wxString& file,
                           const ModuleInfo::DependencySet& dependencies,
                           ModuleInfo& info) {
  static bool b_libelf_initialized = false;
  static bool b_libelf_usable = false;

  if (b_libelf_usable) {
    // Nothing to do.
  } else if (b_libelf_initialized) {
    return false;
  } else if (elf_version(EV_CURRENT) == EV_NONE) {
    b_libelf_initialized = true;
    b_libelf_usable = false;
    wxLogError("LibELF is outdated.");
    return false;
  } else {
    b_libelf_initialized = true;
    b_libelf_usable = true;
  }

  int file_handle;
  Elf* elf_handle = nullptr;
  GElf_Ehdr elf_file_header;
  Elf_Scn* elf_section_handle = nullptr;

  file_handle = open(file, O_RDONLY);
  if (file_handle == -1) {
    wxLogMessage("Could not open file \"%s\" for reading: %s", file,
                 strerror(errno));
    goto FailureEpilogue;
  }

  elf_handle = elf_begin(file_handle, ELF_C_READ, nullptr);
  if (elf_handle == nullptr) {
    wxLogMessage("Could not get ELF structures from \"%s\".", file);
    goto FailureEpilogue;
  }

  if (gelf_getehdr(elf_handle, &elf_file_header) != &elf_file_header) {
    wxLogMessage("Could not get ELF file header from \"%s\".", file);
    goto FailureEpilogue;
  }

  switch (elf_file_header.e_type) {
    case ET_EXEC:
    case ET_DYN:
      break;
    default:
      wxLogMessage(wxString::Format(
          "Module \"%s\" is not an executable or shared library.", file));
      goto FailureEpilogue;
  }

  info.type_magic =
      (static_cast<uint64_t>(elf_file_header.e_ident[EI_CLASS])
       << 0) |  // ELF class (32/64).
      (static_cast<uint64_t>(elf_file_header.e_ident[EI_DATA])
       << 8) |  // Endianness.
      (static_cast<uint64_t>(elf_file_header.e_ident[EI_OSABI])
       << 16) |  // OS ABI (Linux, FreeBSD, etc.).
      (static_cast<uint64_t>(elf_file_header.e_ident[EI_ABIVERSION])
       << 24) |  // OS ABI version.
      (static_cast<uint64_t>(elf_file_header.e_machine)
       << 32) |  // Instruction set.
      0;

  while ((elf_section_handle = elf_nextscn(elf_handle, elf_section_handle)) !=
         nullptr) {
    GElf_Shdr elf_section_header;
    Elf_Data* elf_section_data = nullptr;
    size_t elf_section_entry_count = 0;

    if (gelf_getshdr(elf_section_handle, &elf_section_header) !=
        &elf_section_header) {
      wxLogMessage("Could not get ELF section header from \"%s\".", file);
      goto FailureEpilogue;
    } else if (elf_section_header.sh_type != SHT_DYNAMIC) {
      continue;
    }

    elf_section_data = elf_getdata(elf_section_handle, nullptr);
    if (elf_section_data == nullptr) {
      wxLogMessage("Could not get ELF section data from \"%s\".", file);
      goto FailureEpilogue;
    }

    if ((elf_section_data->d_size == 0) ||
        (elf_section_header.sh_entsize == 0)) {
      wxLogMessage("Got malformed ELF section metadata from \"%s\".", file);
      goto FailureEpilogue;
    }

    elf_section_entry_count =
        elf_section_data->d_size / elf_section_header.sh_entsize;
    for (size_t elf_section_entry_index = 0;
         elf_section_entry_index < elf_section_entry_count;
         ++elf_section_entry_index) {
      GElf_Dyn elf_dynamic_entry;
      const char* elf_dynamic_entry_name = nullptr;
      if (gelf_getdyn(elf_section_data,
                      static_cast<int>(elf_section_entry_index),
                      &elf_dynamic_entry) != &elf_dynamic_entry) {
        wxLogMessage("Could not get ELF dynamic_section entry from \"%s\".",
                     file);
        goto FailureEpilogue;
      } else if (elf_dynamic_entry.d_tag != DT_NEEDED) {
        continue;
      }
      elf_dynamic_entry_name = elf_strptr(
          elf_handle, elf_section_header.sh_link, elf_dynamic_entry.d_un.d_val);
      if (elf_dynamic_entry_name == nullptr) {
        wxLogMessage(wxString::Format("Could not get %s %s from \"%s\".", "ELF",
                                      "string entry", file));
        goto FailureEpilogue;
      }
      wxString name_full(elf_dynamic_entry_name);
      wxString name_part(elf_dynamic_entry_name,
                         strcspn(elf_dynamic_entry_name, "-."));
      if (dependencies.find(name_part) != dependencies.end()) {
        info.dependencies.insert(
            ModuleInfo::DependencyMap::value_type(name_part, name_full));
      }
    }
  }

  goto SuccessEpilogue;

SuccessEpilogue:
  elf_end(elf_handle);
  close(file_handle);
  return true;

FailureEpilogue:
  if (elf_handle != nullptr) elf_end(elf_handle);
  if (file_handle >= 0) close(file_handle);
  wxLog::FlushActive();
  return false;
}
#endif  // USE_LIBELF

bool PluginLoader::CheckPluginCompatibility(const wxString& plugin_file) {
  bool b_compat = false;

#ifdef __WXMSW__
  // For Windows we identify the dll file containing the core wxWidgets functions
  // Later we will compare this with the file containing the wxWidgets functions used
  // by plugins.  If these file names match exactly then we assume the plugin is compatible.
  // By using the file names we avoid having to hard code the file name into the OpenCPN sources.
  // This makes it easier to update wxWigets versions without editing sources.
  // NOTE: this solution may not follow symlinks but almost no one uses simlinks for wxWidgets dlls

  // Only go through this process once per instance of O.
  if (!m_found_wxwidgets) {
    DWORD myPid = GetCurrentProcessId();
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, myPid);
    if (hProcess == NULL) {
      wxLogMessage(wxString::Format("Cannot identify running process for %s",
                                    plugin_file.c_str()));
    } else {
      // Find namme of wxWidgets core DLL used by the current process
      // so we can compare it to the one used by the plugin
      HMODULE hMods[1024];
      DWORD cbNeeded;
      if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
          TCHAR szModName[MAX_PATH];
          if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
                                  sizeof(szModName) / sizeof(TCHAR))) {
            m_module_name = szModName;
            if (m_module_name.Find("wxmsw") != wxNOT_FOUND) {
              if (m_module_name.Find("_core_") != wxNOT_FOUND) {
                m_found_wxwidgets = true;
                wxLogMessage(wxString::Format(
                    "Found wxWidgets core DLL: %s",
                    m_module_name.c_str()));
                break;
              }
            }
          }
        }
      } else {
        wxLogMessage(wxString::Format("Cannot enumerate process modules for %s",
                                      plugin_file.c_str()));
      }
      if (hProcess) CloseHandle(hProcess);
    }
  }
  if (!m_found_wxwidgets) {
    wxLogMessage(wxString::Format("Cannot identify wxWidgets core DLL for %s",
                                  plugin_file.c_str()));
  } else {
    LPCWSTR fName = plugin_file.wc_str();
    HANDLE handle = CreateFile(fName, GENERIC_READ, 0, 0, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, 0);
    DWORD byteread, size = GetFileSize(handle, NULL);
    PVOID virtualpointer = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    bool status = ReadFile(handle, virtualpointer, size, &byteread, NULL);
    CloseHandle(handle);
    PIMAGE_NT_HEADERS ntheaders =
        (PIMAGE_NT_HEADERS)(PCHAR(virtualpointer) +
                            PIMAGE_DOS_HEADER(virtualpointer)->e_lfanew);
    PIMAGE_SECTION_HEADER pSech =
        IMAGE_FIRST_SECTION(ntheaders);  // Pointer to first section header
    PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor;  // Pointer to import descriptor
    if (ntheaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
            .Size !=
        0) /*if size of the table is 0 - Import Table does not exist */
    {
      pImportDescriptor =
          (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)virtualpointer +
                                     Rva2Offset(
                                         ntheaders->OptionalHeader
                                             .DataDirectory
                                                 [IMAGE_DIRECTORY_ENTRY_IMPORT]
                                             .VirtualAddress,
                                         pSech, ntheaders));
      LPSTR libname[256];
      size_t i = 0;
      // Walk until you reached an empty IMAGE_IMPORT_DESCRIPTOR or we find core wxWidgets DLL
      while (pImportDescriptor->Name != 0) {
        // Get the name of each DLL
        libname[i] =
            (PCHAR)((DWORD_PTR)virtualpointer +
                    Rva2Offset(pImportDescriptor->Name, pSech, ntheaders));
        // Check if the plugin DLL dependencey is same as main process wxWidgets core DLL
        if (m_module_name.Find(libname[i]) != wxNOT_FOUND) {
          // Match found - plugin is compatible
          b_compat = true;
          wxLogMessage(
              wxString::Format("Compatible wxWidgets plugin library found for %s: %s",
              plugin_file.c_str(), libname[i]));
          break;
        }
        pImportDescriptor++;  // advance to next IMAGE_IMPORT_DESCRIPTOR
        i++;
      }
    } else {
      wxLogMessage(
          wxString::Format("No Import Table! in %s", plugin_file.c_str()));
    }
    if (virtualpointer) VirtualFree(virtualpointer, size, MEM_DECOMMIT);
  }
#endif
#if defined(__WXGTK__) || defined(__WXQT__)
#if defined(USE_LIBELF)

  static bool b_own_info_queried = false;
  static bool b_own_info_usable = false;
  static ModuleInfo own_info;
  static ModuleInfo::DependencySet dependencies;

  if (!b_own_info_queried) {
    dependencies.insert("libwx_baseu");

    char exe_buf[100] = {0};
    ssize_t len = readlink("/proc/self/exe", exe_buf, 99);
    if (len > 0) {
      exe_buf[len] = '\0';
      wxString app_path(exe_buf);
      wxLogMessage("Executable path: %s", exe_buf);
      b_own_info_usable =
          ReadModuleInfoFromELF(app_path, dependencies, own_info);
      if (!b_own_info_usable) {
        wxLogMessage("Cannot get own info from: %s", exe_buf);
      }
    } else {
      wxLogMessage("Cannot get own executable path.");
    }
    b_own_info_queried = true;
  }

  if (b_own_info_usable) {
    bool b_pi_info_usable = false;
    ModuleInfo pi_info;
    b_pi_info_usable =
        ReadModuleInfoFromELF(plugin_file, dependencies, pi_info);
    if (b_pi_info_usable) {
      b_compat = (pi_info.type_magic == own_info.type_magic);

      // OSABI field on flatpak builds
      if ((pi_info.type_magic ^ own_info.type_magic) == 0x00030000) {
        b_compat = true;
      }

      if (!b_compat) {
        pi_info.dependencies.clear();
        wxLogMessage(
            wxString::Format("    Plugin \"%s\" is of another binary "
                             "flavor than the main module.",
                             plugin_file));
        wxLogMessage("host magic: %.8x, plugin magic: %.8x",
                     own_info.type_magic, pi_info.type_magic);
      }
      for (const auto& own_dependency : own_info.dependencies) {
        ModuleInfo::DependencyMap::const_iterator pi_dependency =
            pi_info.dependencies.find(own_dependency.first);
        if ((pi_dependency != pi_info.dependencies.end()) &&
            (pi_dependency->second != own_dependency.second)) {
          b_compat = false;
          wxLogMessage(
              "    Plugin \"%s\" depends on library \"%s\", but the main "
              "module was built for \"%s\".",
              plugin_file, pi_dependency->second, own_dependency.second);
          break;
        }
      }
    } else {
      b_compat = false;
      wxLogMessage(
          wxString::Format("    Plugin \"%s\" could not be reliably "
                           "checked for compatibility.",
                           plugin_file));
    }
  } else {
    // Allow any plugin when own info is not available.
    b_compat = true;
  }

  wxLogMessage("Plugin is compatible by elf library scan: %s",
               b_compat ? "true" : "false");

  wxLog::FlushActive();
  return b_compat;

#endif  // LIBELF

  //  But Android Plugins do not include the wxlib specification in their ELF
  //  file. So we assume Android Plugins are compatible....
#ifdef __ANDROID__
  return true;
#endif

  // If libelf is not available, then we must use a simplistic file scan method.
  // This is easily fooled if the wxWidgets version in use is not exactly
  // recognized. File scan is 3x faster than the ELF scan method

  FILE* f = fopen(plugin_file, "r");
  char strver[26];  // Enough space even for very big integers...

  sprintf(strver,
#if defined(__WXGTK3__)
          "libwx_gtk3u_core-%i.%i"
#elif defined(__WXGTK20__)
          "libwx_gtk2u_core-%i.%i"
#elif defined(__WXQT__)
          "libwx_qtu_core-%i.%i"
#else
#error undefined plugin platform
#endif
          ,
          wxMAJOR_VERSION, wxMINOR_VERSION);
  b_compat = false;

  size_t pos(0);
  size_t len(strlen(strver));
  int c;
  while ((c = fgetc(f)) != EOF) {
    if (c == strver[pos]) {
      if (++pos == len) {
        b_compat = true;
        break;
      }
    } else
      pos = 0;
  }
  fclose(f);
#endif  // __WXGTK__ or __WXQT__

  wxLogMessage("Plugin is compatible: %s", b_compat ? "true" : "false");
  return b_compat;
}

PlugInContainer* PluginLoader::LoadPlugIn(const wxString& plugin_file) {
  auto pic = new PlugInContainer;
  if (!LoadPlugIn(plugin_file, pic)) {
    delete pic;
    return nullptr;
  } else {
    return pic;
  }
}

PlugInContainer* PluginLoader::LoadPlugIn(const wxString& plugin_file,
                                          PlugInContainer* pic) {
  wxLogMessage(wxString("PluginLoader: Loading PlugIn: ") + plugin_file);

  if (plugin_file.empty()) {
    wxLogMessage("Ignoring loading of empty path");
    return nullptr;
  }

  if (!wxIsReadable(plugin_file)) {
    wxLogMessage("Ignoring unreadable plugin %s",
                 plugin_file.ToStdString().c_str());
    LoadError le(LoadError::Type::Unreadable, plugin_file.ToStdString());
    load_errors.push_back(le);
    return nullptr;
  }

  // Check if blacklisted, exit if so.
  auto sts =
      m_blacklist->get_status(pic->m_common_name.ToStdString(),
                              pic->m_version_major, pic->m_version_minor);
  if (sts != plug_status::unblocked) {
    wxLogDebug("Refusing to load blacklisted plugin: %s",
               pic->m_common_name.ToStdString().c_str());
    return nullptr;
  }
  auto data = m_blacklist->get_library_data(plugin_file.ToStdString());
  if (!data.name.empty()) {
    wxLogDebug("Refusing to load blacklisted library: %s",
               plugin_file.ToStdString().c_str());
    return nullptr;
  }
  pic->m_plugin_file = plugin_file;
  pic->m_status =
      PluginStatus::Unmanaged;  // Status is updated later, if necessary

  // load the library
  if (pic->m_library.IsLoaded()) pic->m_library.Unload();
  pic->m_library.Load(plugin_file);

  if (!pic->m_library.IsLoaded()) {
    //  Look in the Blacklist, try to match a filename, to give some kind of
    //  message extract the probable plugin name
    wxFileName fn(plugin_file);
    std::string name = fn.GetName().ToStdString();
    auto found = m_blacklist->get_library_data(name);
    if (m_blacklist->mark_unloadable(plugin_file.ToStdString())) {
      wxLogMessage("Ignoring blacklisted plugin %s", name.c_str());
      if (!found.name.empty()) {
        SemanticVersion v(found.major, found.minor);
        LoadError le(LoadError::Type::Unloadable, name, v);
        load_errors.push_back(le);
      } else {
        LoadError le(LoadError::Type::Unloadable, plugin_file.ToStdString());
        load_errors.push_back(le);
      }
    }
    wxLogMessage(wxString("   PluginLoader: Cannot load library: ") +
                 plugin_file);
    return nullptr;
  }

  // load the factory symbols
  const char* const FIX_LOADING =
      _("\n    Install/uninstall plugin or remove file to mute message");
  create_t* create_plugin = (create_t*)pic->m_library.GetSymbol("create_pi");
  if (nullptr == create_plugin) {
    std::string msg(_("   PluginLoader: Cannot load symbol create_pi: "));
    wxLogMessage(msg + plugin_file);
    if (m_blacklist->mark_unloadable(plugin_file.ToStdString())) {
      LoadError le(LoadError::Type::NoCreate, plugin_file.ToStdString());
      load_errors.push_back(le);
    }
    return nullptr;
  }

  destroy_t* destroy_plugin =
      (destroy_t*)pic->m_library.GetSymbol("destroy_pi");
  pic->m_destroy_fn = destroy_plugin;
  if (nullptr == destroy_plugin) {
    wxLogMessage("   PluginLoader: Cannot load symbol destroy_pi: " +
                 plugin_file);
    if (m_blacklist->mark_unloadable(plugin_file.ToStdString())) {
      LoadError le(LoadError::Type::NoDestroy, plugin_file.ToStdString());
      load_errors.push_back(le);
    }
    return nullptr;
  }

  // create an instance of the plugin class
  opencpn_plugin* plug_in = create_plugin(this);

  int api_major = plug_in->GetAPIVersionMajor();
  int api_minor = plug_in->GetAPIVersionMinor();
  int api_ver = (api_major * 100) + api_minor;
  pic->m_api_version = api_ver;

  int pi_major = plug_in->GetPlugInVersionMajor();
  int pi_minor = plug_in->GetPlugInVersionMinor();
  SemanticVersion pi_ver(pi_major, pi_minor, -1);

  wxString pi_name = plug_in->GetCommonName();

  wxLogDebug("blacklist: Get status for %s %d %d",
             pi_name.ToStdString().c_str(), pi_major, pi_minor);
  const auto status =
      m_blacklist->get_status(pi_name.ToStdString(), pi_major, pi_minor);
  if (status != plug_status::unblocked) {
    wxLogDebug("Ignoring blacklisted plugin.");
    if (status != plug_status::unloadable) {
      SemanticVersion v(pi_major, pi_minor);
      LoadError le(LoadError::Type::Blacklisted, pi_name.ToStdString(), v);
      load_errors.push_back(le);
    }
    return nullptr;
  }

  switch (api_ver) {
    case 105:
      pic->m_pplugin = dynamic_cast<opencpn_plugin*>(plug_in);
      break;

    case 106:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_16*>(plug_in);
      break;

    case 107:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_17*>(plug_in);
      break;

    case 108:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_18*>(plug_in);
      break;

    case 109:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_19*>(plug_in);
      break;

    case 110:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_110*>(plug_in);
      break;

    case 111:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_111*>(plug_in);
      break;

    case 112:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_112*>(plug_in);
      break;

    case 113:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_113*>(plug_in);
      break;

    case 114:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_114*>(plug_in);
      break;
    case 115:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_115*>(plug_in);
      break;

    case 116:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_116*>(plug_in);
      break;

    case 117:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_117*>(plug_in);
      do /* force a local scope */ {
        auto p = dynamic_cast<opencpn_plugin_117*>(plug_in);
        pi_ver =
            SemanticVersion(pi_major, pi_minor, p->GetPlugInVersionPatch(),
                            p->GetPlugInVersionPost(), p->GetPlugInVersionPre(),
                            p->GetPlugInVersionBuild());
      } while (false);  // NOLINT
      break;
    case 118:
      pic->m_pplugin = dynamic_cast<opencpn_plugin_118*>(plug_in);
      do /* force a local scope */ {
        auto p = dynamic_cast<opencpn_plugin_118*>(plug_in);
        pi_ver =
            SemanticVersion(pi_major, pi_minor, p->GetPlugInVersionPatch(),
                            p->GetPlugInVersionPost(), p->GetPlugInVersionPre(),
                            p->GetPlugInVersionBuild());
      } while (false);  // NOLINT
      break;

    default:
      break;
  }

  if (!pic->m_pplugin) {
    INFO_LOG << _("Incompatible plugin detected: ") << plugin_file << "\n";
    INFO_LOG << _("        API Version detected: ");
    INFO_LOG << api_major << "." << api_minor << "\n";
    INFO_LOG << _("        PlugIn Version detected: ") << pi_ver << "\n";
    if (m_blacklist->mark_unloadable(pi_name.ToStdString(), pi_ver.major,
                                     pi_ver.minor)) {
      LoadError le(LoadError::Type::Incompatible, pi_name.ToStdString(),
                   pi_ver);
      load_errors.push_back(le);
    }
    return nullptr;
  }
  return pic;
}
