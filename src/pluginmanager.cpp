/***************************************************************************
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

#include <config.h>

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <typeinfo>
#if defined(__linux__) && !defined(__OCPN__ANDROID__)
#include <wordexp.h>
#endif
#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/event.h>
#include <wx/filename.h>
#include <wx/aui/aui.h>
#include <wx/platinfo.h>
#include <wx/popupwin.h>
#include <wx/progdlg.h>
#include <wx/statline.h>
#include <wx/tokenzr.h>
#include <wx/tooltip.h>
#include <wx/app.h>
#include <wx/hashset.h>
#include <wx/hashmap.h>
#include <wx/uri.h>
#include <wx/zipstrm.h>
#include <wx/zstream.h>
#include <wx/tarstrm.h>
#include <wx/textwrapper.h>

#ifndef __WXMSW__
#include <cxxabi.h>
#endif  // __WXMSW__


#include <algorithm>
#include <archive.h>
#include <cstdio>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stdint.h>
#include <string>
#include <unordered_map>

#include <archive_entry.h>
typedef __LA_INT64_T la_int64_t;  //  "older" libarchive versions support

#ifdef USE_LIBELF
#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#endif


#include "AIS_Decoder.h"
#include "ais.h"
#include "AIS_Target_Data.h"
#include "canvasMenu.h"
#include "catalog_handler.h"
#include "cat_settings.h"
#include "chartbase.h"  // for ChartPlugInWrapper
#include "chartdb.h"
#include "chartdbs.h"
#include "chcanv.h"
#include "config.h"
#include "Downloader.h"
#include "download_mgr.h"
#include "dychart.h"
#include "FontMgr.h"
#include "georef.h"
#include "ocpn_pixel.h"
#include "gshhs.h"
#include "logger.h"
#include "multiplexer.h"
#include "mygeom.h"
#include "NavObjectCollection.h"
#include "navutil.h"
#include "observable.h"
#include "OCPN_AUIManager.h"
#include "OCPN_DataStreamEvent.h"
#include "ocpndc.h"
#include "ocpn_pixel.h"
#include "OCPNPlatform.h"
#include "OCPNRegion.h"
#include "ocpn_utils.h"
#include "options.h"
#include "piano.h"
#include "plugin_cache.h"
#include "PluginHandler.h"
#include "plugin_loader.h"
#include "pluginmanager.h"
#include "PluginPaths.h"
#include "Route.h"
#include "routemanagerdialog.h"
#include "routeman.h"
#include "s52plib.h"
#include "s52utils.h"
#include "safe_mode.h"
#include "semantic_vers.h"
#include "SoundFactory.h"
#include "SystemCmdSound.h"
#include "styles.h"
#include "toolbar.h"
#include "Track.h"
#include "update_mgr.h"
//#include "cat_settings.h"
#include "svg_utils.h"
//#include "observable.h"

#ifdef __OCPN__ANDROID__
#include <dlfcn.h>
#include "androidUTIL.h"
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>

struct sigaction sa_all_PIM;
struct sigaction sa_all_PIM_previous;

sigjmp_buf env_PIM;  // the context saved by sigsetjmp();

void catch_signals_PIM(int signo) {
  switch (signo) {
    case SIGSEGV:
      siglongjmp(env_PIM, 1);  // jump back to the setjmp() point
      break;

    default:
      break;
  }
}

#endif

extern MyConfig *pConfig;
extern AIS_Decoder *g_pAIS;
extern OCPN_AUIManager *g_pauimgr;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
extern wxLocale *plocale_def_lang;
#endif

extern OCPNPlatform *g_Platform;
extern ChartDB *ChartData;
extern MyFrame *gFrame;
extern ocpnStyle::StyleManager *g_StyleManager;
extern options *g_pOptions;
extern Multiplexer *g_pMUX;
extern bool g_bShowChartBar;
extern Routeman *g_pRouteMan;
extern WayPointman *pWayPointMan;
extern Select *pSelect;
extern RouteManagerDialog *pRouteManagerDialog;
extern RouteList *pRouteList;
extern std::vector<Track*> g_TrackList;
extern PlugInManager *g_pi_manager;
extern s52plib *ps52plib;
extern wxString ChartListFileName;
extern bool g_boptionsactive;
extern options *g_options;
extern ColorScheme global_color_scheme;
extern wxArrayString g_locale_catalog_array;
extern int g_GUIScaleFactor;
extern int g_ChartScaleFactor;
extern wxString g_locale;
extern bool g_btouch;
extern ocpnFloatingToolbarDialog *g_MainToolbar;

extern int g_chart_zoom_modifier;
extern int g_chart_zoom_modifier_vector;
extern double g_display_size_mm;
extern bool g_bopengl;

extern ChartGroupArray *g_pGroupArray;
extern unsigned int g_canvasConfig;

extern wxString g_CmdSoundString;

extern int g_iSDMMFormat;

unsigned int gs_plib_flags;
wxString g_lastPluginMessage;
extern ChartCanvas *g_focusCanvas;
extern ChartCanvas *g_overlayCanvas;
extern bool g_bquiting;
extern wxString g_ownshipMMSI_SK;
extern wxString g_catalog_channel;
extern wxString g_catalog_custom_url;

WX_DEFINE_ARRAY_PTR(ChartCanvas *, arrayofCanvasPtr);
extern arrayofCanvasPtr g_canvasArray;

const char *const LINUX_LOAD_PATH = "~/.local/lib:/usr/local/lib:/usr/lib";
const char *const FLATPAK_LOAD_PATH = "~/.var/app/org.opencpn.OpenCPN/lib";

void NotifySetupOptionsPlugin(const PlugInContainer *pic);

enum { CurlThreadId = wxID_HIGHEST + 1 };

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(Plugin_WaypointList);
WX_DEFINE_LIST(Plugin_HyperlinkList);

wxDEFINE_EVENT(EVT_N0183_PLUGIN, wxCommandEvent);

/**
 * Handle messages for blacklisted plugins. Messages are deferred until
 * show_deferred_messages() is invoked, signaling that the UI is ready.
 */
class BlacklistUI {
public:

  void message(const std::string& message) {
    if (m_defer)
      m_deferred_messages.push_back(message);
    else
      show_msg(message);
  }

  void show_deferred_messages() {
    for (auto m : m_deferred_messages) show_msg(m);
    m_defer = false;
  }

  BlacklistUI() : m_defer(true) {};

private:
  void show_msg(wxString msg) {
    OCPNMessageBox(NULL, msg, wxString(_("OpenCPN Info")),
                   wxICON_INFORMATION | wxOK, 10);  // 10 second timeout
  }

  bool m_defer;            // defer dialogs until setup completed
  std::vector<wxString>  m_deferred_messages;
};

class PanelHardBreakWrapper : public wxTextWrapper {
public:
  PanelHardBreakWrapper(wxWindow *win, const wxString &text, int widthMax) {
    m_lineCount = 0;
    Wrap(win, text, widthMax);
  }
  wxString const &GetWrapped() const { return m_wrapped; }
  int const GetLineCount() const { return m_lineCount; }
  wxArrayString GetLineArray() { return m_array; }

protected:
  virtual void OnOutputLine(const wxString &line) {
    m_wrapped += line;
    m_array.Add(line);
  }
  virtual void OnNewLine() {
    m_wrapped += '\n';
    m_lineCount++;
  }

private:
  wxString m_wrapped;
  int m_lineCount;
  wxArrayString m_array;
};

static const std::vector<std::string> SYSTEM_PLUGINS = {
    "chartdownloader", "wmm", "dashboard", "grib"};

struct EnumClassHash {
  template <typename T>
  std::size_t operator()(T t) const {
    return static_cast<std::size_t>(t);
  }
};

wxString message_by_status(PluginStatus stat) {
  switch (stat) {
    case PluginStatus::System:
      return _("Plugin is a standard system plugin");
    case PluginStatus::Managed:
      return _("Plugin is managed by OpenCPN");
    case PluginStatus::Unmanaged:
      return _("Plugin is not managed by OpenCPN");
    case PluginStatus::Ghost:
      return ("");
    case PluginStatus::Unknown:
      return _("Plugin status unknown");
    case PluginStatus::LegacyUpdateAvailable:
      return _("Update to managed Plugin is available");
    case PluginStatus::ManagedInstallAvailable:
      return _("New managed Plugin installation available");
    case PluginStatus::ManagedInstalledUpdateAvailable:
      return _("Update to installed Plugin is available");
    case PluginStatus::ManagedInstalledCurrentVersion:
      return _("Plugin is latest available");
    case PluginStatus::ManagedInstalledDowngradeAvailable:
      return ("");
    case PluginStatus::PendingListRemoval:
      return ("");
    default:
      return ("");
  }
}

static std::unordered_map<PluginStatus, const char *, EnumClassHash>
    icon_by_status(
        {{PluginStatus::System, "emblem-system.svg"},
         {PluginStatus::Managed, "emblem-default.svg"},
         {PluginStatus::Unmanaged, "emblem-unmanaged.svg"},
         {PluginStatus::Ghost, "ghost.svg"},
         {PluginStatus::Unknown, "emblem-unmanaged.svg"},
         {PluginStatus::LegacyUpdateAvailable, "emblem-legacy-update.svg"},
         {PluginStatus::ManagedInstallAvailable, "emblem-default.svg"},
         {PluginStatus::ManagedInstalledUpdateAvailable,
          "emblem-legacy-update.svg"},
         {PluginStatus::ManagedInstalledCurrentVersion, "emblem-default.svg"},
         {PluginStatus::ManagedInstalledDowngradeAvailable,
          "emblem-default.svg"},
         {PluginStatus::PendingListRemoval, "emblem-default.svg"}

        });

static std::unordered_map<PluginStatus, const char *, EnumClassHash>
    literalstatus_by_status(
        {{PluginStatus::System, "System"},
         {PluginStatus::Managed, "Managed"},
         {PluginStatus::Unmanaged, "Unmanaged"},
         {PluginStatus::Ghost, "Ghost"},
         {PluginStatus::Unknown, "Unknown"},
         {PluginStatus::LegacyUpdateAvailable, "LegacyUpdateAvailable"},
         {PluginStatus::ManagedInstallAvailable, "ManagedInstallAvailable"},
         {PluginStatus::ManagedInstalledUpdateAvailable,
          "ManagedInstalledUpdateAvailable"},
         {PluginStatus::ManagedInstalledCurrentVersion,
          "ManagedInstalledCurrentVersion"},
         {PluginStatus::ManagedInstalledDowngradeAvailable,
          "ManagedInstalledDowngradeAvailable"},
         {PluginStatus::PendingListRemoval, "PendingListRemoval"}

        });

/**
 * Return list of available, unique and compatible plugins from
 * configured XML catalog.
 */
static std::vector<PluginMetadata> getCompatiblePlugins() {
  /** Compare two PluginMetadata objects, a named c++ requirement. */
  struct metadata_compare {
    bool operator()(const PluginMetadata &lhs,
                    const PluginMetadata &rhs) const {
      return lhs.key() < rhs.key();
    }
  };

  std::vector<PluginMetadata> returnArray;

  std::set<PluginMetadata, metadata_compare> unique_plugins;
  for (auto plugin : PluginHandler::getInstance()->getAvailable()) {
    unique_plugins.insert(plugin);
  }
  for (auto plugin : unique_plugins) {
    if (PluginHandler::isCompatible(plugin)) {
      returnArray.push_back(plugin);
    }
  }
  return returnArray;
}

static SemanticVersion metadata_version(const PluginMetadata pm) {
  return SemanticVersion::parse(pm.name);
}

// Get installed version from manifest for given plugin. For
// older plugins this contains more detailed info then the
// plugin API. From API level 117 the API should contain the
// same info.
//
// TODO: Get version from API for api level 117+
SemanticVersion getInstalledVersion(const std::string name) {
  std::string installed;
  std::string path = PluginHandler::versionPath(name);
  if (path == "" || !wxFileName::IsFileReadable(path)) {
    return SemanticVersion(-1, -1);
  }
  std::ifstream stream;
  stream.open(path, std::ifstream::in);
  stream >> installed;
  return SemanticVersion::parse(installed);
}

/**
 * Return list of all versions of given plugin name besides installed
 * one.
 */
static std::vector<PluginMetadata> getUpdates(const char *name) {
  auto updates = getCompatiblePlugins();
  updates.erase(
      std::remove_if(updates.begin(), updates.end(),
                     [&](const PluginMetadata m) { return m.name != name; }),
      updates.end());

  auto inst_vers = getInstalledVersion(name);
  if (inst_vers.major == -1) {
    return updates;
  }

  // Drop already installed plugin, it has its own update options.
  updates.erase(std::remove_if(updates.begin(), updates.end(),
                               [&](const PluginMetadata m) {
                                 return metadata_version(m) == inst_vers;
                               }),
                updates.end());
  return updates;
}

/**
 * Return number of existing files named filename in the list of
 * dirs.
 */
static int count_files_in_dirs(const char *filename,
                               const std::vector<std::string> dirs) {
  int count = 0;
  for (auto dir : dirs) {
    const std::string sep(1, wxFileName::GetPathSeparator());
    auto path = dir + sep + filename;
    if (ocpn::exists(path)) {
      count += 1;
    }
  }
  return count;
}

static PluginMetadata getLatestUpdate() {
  auto updates = getCompatiblePlugins();
  if (updates.size() == 0) {
    PluginMetadata metadata;
    return metadata;
  }
  std::sort(updates.begin(), updates.end(),
            [](PluginMetadata m1, PluginMetadata m2) {
              return !(m1.version < m2.version);
            });
  return updates[0];
}

/** Remove plugin and update GUI elements. */
static void gui_uninstall(PlugInContainer *pic, const char *plugin) {
  g_Platform->ShowBusySpinner();
  PluginLoader::getInstance()->DeactivatePlugIn(pic);
  pic->m_bEnabled = false;
  PluginLoader::getInstance()->UpdatePlugIns();

  wxLogMessage("Uninstalling %s", plugin);
  PluginHandler::getInstance()->uninstall(plugin);
  PluginLoader::getInstance()->UpdatePlugIns();
  g_Platform->HideBusySpinner();
}

static bool LoadAllPlugIns(bool load_enabled) {
  g_Platform->ShowBusySpinner();
  bool b = PluginLoader::getInstance()->LoadAllPlugIns(load_enabled);
  g_Platform->HideBusySpinner();
  return b;
}

static void run_update_dialog(PluginListPanel *parent, PlugInContainer *pic,
                              bool uninstall, const char *name = 0,
                              bool b_forceEnable = false) {
  wxString pluginName = pic->m_common_name;
  const char *plugin = name == 0 ? pic->m_common_name.mb_str().data() : name;
  auto updates = getUpdates(plugin);
  auto parent_dlg = dynamic_cast<wxScrolledWindow *>(parent);
  wxASSERT(parent_dlg != 0);
  UpdateDialog dialog(parent_dlg, updates);
  auto status = dialog.ShowModal();
  status = dialog.GetReturnCode();
  if (status != wxID_OK) {
    return;
  }

  auto update = dialog.GetUpdate();
  if (!g_pi_manager->CheckBlacklistedPlugin(update)) {
    return;
  }

  wxLogMessage("Installing %s", update.name.c_str());

  auto pluginHandler = PluginHandler::getInstance();
  auto path = ocpn::lookup_tarball(update.tarball_url.c_str());
  if (uninstall && path != "") {
    gui_uninstall(pic, update.name.c_str());
  }
  bool cacheResult = pluginHandler->installPluginFromCache(update);

  if (!cacheResult) {
    g_Platform->ShowBusySpinner();  // Will be cancelled in downloader->run()
    wxYield();

    auto downloader = new GuiDownloader(parent_dlg, update);
    std::string tempTarballPath = downloader->run(parent_dlg, uninstall);

    if (!tempTarballPath.size())  // Error, dialog already presented
      return;

    // Provisional error check
    bool bOK = true;
    std::string manifestPath = PluginHandler::fileListPath(update.name);
    if (!isRegularFile(manifestPath.c_str())) {
      wxLogMessage("Installation of %s failed", update.name.c_str());
      PluginHandler::cleanupFiles(manifestPath, update.name);
      bOK = false;
    }

    //  On successful installation, copy the temp tarball to the local cache
    if (bOK) {
      wxLogMessage("Installation of %s successful", update.name.c_str());
      wxURI uri(wxString(update.tarball_url.c_str()));
      wxFileName fn(uri.GetPath());
      std::string basename = fn.GetFullName().ToStdString();

      if (ocpn::store_tarball(tempTarballPath.c_str(), basename.c_str())) {
        wxLogDebug("Copied %s to local cache at %s", tempTarballPath.c_str(),
                   basename);
        remove(tempTarballPath.c_str());
      }
    }
  }

  //  Check the library compatibility of the subject plugin
  //  Find the plugin library file, looking for "_pi.{dll/so/dylib file}
#ifdef __WXMSW__
  wxString pispec = _T("_pi.dll");
#elif defined(__WXOSX__)
  wxString pispec = _T("_pi.dylib");
#else
  wxString pispec = _T("_pi.so");
#endif

  std::string manifestPath = PluginHandler::fileListPath(update.name);
  wxTextFile manifest_file(manifestPath);
  wxString pluginFile;
  if (manifest_file.Open()) {
    wxString val;
    for (wxString str = manifest_file.GetFirstLine(); !manifest_file.Eof();
         str = manifest_file.GetNextLine()) {
      if (str.Contains(pispec)) {
        if (getenv("OCPN_KEEP_PLUGINS")) {
          // Undocumented debug hook
          continue;
        }
        auto loader = PluginLoader::getInstance();
        if (!loader->CheckPluginCompatibility(str)) {
          wxString msg =
              _("The plugin is not compatible with this version of OpenCPN, "
                "and will be uninstalled.");
          OCPNMessageBox(NULL, msg, wxString(_("OpenCPN Info")),
                         wxICON_INFORMATION | wxOK, 10);

          PluginHandler::cleanupFiles(manifestPath, update.name);
        } else {
          pluginFile = str;
        }
        break;
      }
    }
  }

  if (b_forceEnable && pluginFile.Length()) {
    wxString config_section = (_T ( "/PlugIns/" ));
    wxFileName fn(pluginFile);
    config_section += fn.GetFullName();
    pConfig->SetPath(config_section);
    pConfig->Write(_T ( "bEnabled" ), true);
  }

  //  Reload all plugins, which will bring in the action results.
  LoadAllPlugIns(false);

  // Check to see if this plugin needs an options instance reload
  if (g_options) {
    bool b_newOptions = false;
    auto loader = PluginLoader::getInstance();
    for (unsigned int i = 0; i < loader->GetPlugInArray()->GetCount();
         i++) {
      PlugInContainer *pic = loader->GetPlugInArray()->Item(i);
      wxString cname = pic->m_common_name;
      if (pic->m_bInitState && (pluginName == cname)) {
        if ((pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE)) {
          g_options->SetNeedNew(true);

          NotifySetupOptionsPlugin(pic);
        }
      }
    }
  }

  parent->ReloadPluginPanels();
  // wxString name(plugin);
  // g_pi_manager->GetListPanelPtr()->SelectByName(name);
}

//    Some static helper funtions
//    Scope is local to this module

PlugIn_ViewPort CreatePlugInViewport(const ViewPort &vp) {
  //    Create a PlugIn Viewport
  ViewPort tvp = vp;
  PlugIn_ViewPort pivp;

  pivp.clat = tvp.clat;  // center point
  pivp.clon = tvp.clon;
  pivp.view_scale_ppm = tvp.view_scale_ppm;
  pivp.skew = tvp.skew;
  pivp.rotation = tvp.rotation;
  pivp.chart_scale = tvp.chart_scale;
  pivp.pix_width = tvp.pix_width;
  pivp.pix_height = tvp.pix_height;
  pivp.rv_rect = tvp.rv_rect;
  pivp.b_quilt = tvp.b_quilt;
  pivp.m_projection_type = tvp.m_projection_type;

  pivp.lat_min = tvp.GetBBox().GetMinLat();
  pivp.lat_max = tvp.GetBBox().GetMaxLat();
  pivp.lon_min = tvp.GetBBox().GetMinLon();
  pivp.lon_max = tvp.GetBBox().GetMaxLon();

  pivp.bValid = tvp.IsValid();  // This VP is valid

  return pivp;
}

ViewPort CreateCompatibleViewport(const PlugIn_ViewPort &pivp) {
  //    Create a system ViewPort
  ViewPort vp;

  vp.clat = pivp.clat;  // center point
  vp.clon = pivp.clon;
  vp.view_scale_ppm = pivp.view_scale_ppm;
  vp.skew = pivp.skew;
  vp.rotation = pivp.rotation;
  vp.chart_scale = pivp.chart_scale;
  vp.pix_width = pivp.pix_width;
  vp.pix_height = pivp.pix_height;
  vp.rv_rect = pivp.rv_rect;
  vp.b_quilt = pivp.b_quilt;
  vp.m_projection_type = pivp.m_projection_type;

  if (gFrame->GetPrimaryCanvas())
    vp.ref_scale = gFrame->GetPrimaryCanvas()->GetVP().ref_scale;
  else
    vp.ref_scale = vp.chart_scale;

  vp.SetBoxes();
  vp.Validate();  // This VP is valid

  return vp;
}

class pluginUtilHandler : public wxEvtHandler {
public:
  pluginUtilHandler();
  ~pluginUtilHandler() {}

  void OnPluginUtilAction(wxCommandEvent &event);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(pluginUtilHandler, wxEvtHandler)
EVT_BUTTON(ID_CMD_BUTTON_PERFORM_ACTION, pluginUtilHandler::OnPluginUtilAction)
END_EVENT_TABLE()

pluginUtilHandler::pluginUtilHandler() {}

void pluginUtilHandler::OnPluginUtilAction(wxCommandEvent &event) {
  auto panel = static_cast<PluginPanel *>(event.GetClientData());
  PluginListPanel *plugin_list_panel =
      dynamic_cast<PluginListPanel *>(panel->GetParent());
  wxASSERT(plugin_list_panel != 0);

  auto actionPIC = panel->GetPlugin();
  wxString name = actionPIC->m_common_name;

  // Perform the indicated action according to the verb...
  switch (panel->GetAction()) {
    case ActionVerb::UPGRADE_TO_MANAGED_VERSION: {
      auto loader = PluginLoader::getInstance();

      // capture the plugin name
      std::string pluginName = actionPIC->m_ManagedMetadata.name;

      wxLogMessage("Installing managed plugin: %s", pluginName.c_str());
      auto downloader =
          new GuiDownloader(plugin_list_panel, actionPIC->m_ManagedMetadata);
      downloader->run(plugin_list_panel, false);

      // Provisional error check
      std::string manifestPath = PluginHandler::fileListPath(pluginName);
      if (isRegularFile(manifestPath.c_str())) {
        // dynamically deactivate the legacy plugin, making way for the upgrade.
        for (unsigned i = 0; i < loader->GetPlugInArray()->GetCount();
             i += 1) {
          if (actionPIC->m_ManagedMetadata.name ==
              loader->GetPlugInArray()
                  ->Item(i)
                  ->m_common_name.ToStdString()) {
            loader->UnLoadPlugIn(i);
            break;
          }
        }

        //  Reload all plugins, which will bring in the new, managed version.
        LoadAllPlugIns(false);
      } else {
        PluginHandler::cleanupFiles(manifestPath,
                                    actionPIC->m_ManagedMetadata.name);
      }
      plugin_list_panel->ReloadPluginPanels();
      plugin_list_panel->SelectByName(name);

      break;
    }

    case ActionVerb::UPGRADE_INSTALLED_MANAGED_VERSION:
    case ActionVerb::REINSTALL_MANAGED_VERSION:
    case ActionVerb::DOWNGRADE_INSTALLED_MANAGED_VERSION: {
      // Grab a copy of the managed metadata
      auto metaSave = actionPIC->m_ManagedMetadata;
      run_update_dialog(plugin_list_panel, actionPIC, true,
                        metaSave.name.c_str());
      break;
    }

    case ActionVerb::INSTALL_MANAGED_VERSION: {
      wxLogMessage("Installing new managed plugin.");
      run_update_dialog(plugin_list_panel, actionPIC, false);
      break;
    }

    case ActionVerb::UNINSTALL_MANAGED_VERSION: {
      PluginLoader::getInstance()->DeactivatePlugIn(actionPIC);

      // Capture the confirmation dialog contents before the plugin goes away
      wxString message;
      message.Printf("%s %s\n", actionPIC->m_ManagedMetadata.name.c_str(),
                     actionPIC->m_ManagedMetadata.version.c_str());
      message += _("successfully un-installed");

      wxLogMessage("Uninstalling %s",
                   actionPIC->m_ManagedMetadata.name.c_str());

      PluginHandler::getInstance()->uninstall(
          actionPIC->m_ManagedMetadata.name);

      //  Reload all plugins, which will bring in the action results.
      auto loader =  PluginLoader::getInstance();
      LoadAllPlugIns(false);
      plugin_list_panel->ReloadPluginPanels();

      OCPNMessageBox(gFrame, message, _("Un-Installation complete"),
                     wxICON_INFORMATION | wxOK);

      break;
    }

    case ActionVerb::NOP:
    default:
      break;
  }
}

#if 0
/**
 * A svg status icon, scaled to about 1/3 of available space
 *
 * Load icons from .../uidata/.
 *
 * Dont unbind in destructor:
 *      https://forums.wxwidgets.org/viewtopic.php?t=36399
 */
class StatusIconPanel: public wxPanel
{
    public:
        StatusIconPanel(wxWindow* parent, const PlugInContainer* pic)
            :wxPanel(parent)
        {
            m_parent = wxDynamicCast(parent, PluginPanel);

            m_stat = pic->m_pluginStatus; //::Unknown;
            SetToolTip(message_by_status[m_stat]);
            m_icon_name = icon_by_status[m_stat];

            m_penWidthUnselected = g_Platform->GetDisplayDPmm() * .25;
            m_penWidthSelected = g_Platform->GetDisplayDPmm() * .5;

            //SetBackgroundColour(GetGlobalColor(_T("DILG0")));
            //auto minsize = wxSize(GetCharWidth() * 5, GetCharWidth() * 10);
            //SetSize(minsize);
            SetMinSize(wxSize(GetCharWidth() * 5, -1));
            Bind(wxEVT_PAINT, &StatusIconPanel::OnPaint, this);
            Bind(wxEVT_LEFT_DOWN, &StatusIconPanel::OnIconSelected, this);


       }

        void OnPaint(wxPaintEvent& event)
        {
            auto size = GetClientSize();
            int minsize = GetCharWidth() * 3;
            auto offset = minsize / 4;

            LoadIcon(m_icon_name.c_str(), m_bitmap,   wxMax(1, minsize));
            wxPaintDC dc(this);
            if (!m_bitmap.IsOk()) {
                wxLogMessage("StatusPluginPanel: bitmap is not OK!");
                return;
            }

            int penWidth = m_penWidthUnselected;
            wxColour border = GetDialogColor(DLG_UNSELECTED_ACCENT);

            if(m_parent->GetSelected()){
                penWidth = m_penWidthSelected;
                border = GetDialogColor(DLG_SELECTED_ACCENT);
            }

            wxBrush b(m_parent->GetBackgroundColour(), wxSOLID);
            dc.SetBrush(b);
            dc.SetPen( wxPen(border, penWidth) );

            dc.DrawRoundedRectangle(-20, 5, 20 + GetSize().x-10, GetSize().y-10, 5);
            //dc.DrawRectangle(0, 0, GetSize().x, GetSize().y);
            dc.DrawBitmap(m_bitmap, offset * 3 / 4, offset*3, true);


            //dc.DrawText(_T("PluginStatus"), 0, 0);
            //dc.DrawText(literalstatus_by_status[m_stat], 4 * GetCharWidth(), GetCharHeight());
         }

         void SetStatus(PluginStatus stat)
         {
            m_stat = stat;
            SetToolTip(message_by_status[m_stat]);
            m_icon_name = icon_by_status[m_stat];
            Refresh();
         }

         void OnIconSelected( wxMouseEvent &event )
         {
             if(m_parent){
                 m_parent->OnPluginSelected(event);
             }
         }


    protected:
        wxBitmap m_bitmap;
        std::string m_icon_name;
        PluginStatus m_stat;
        PluginPanel *m_parent;
        int m_penWidthUnselected;
        int m_penWidthSelected;

        void LoadIcon(const char* icon_name, wxBitmap& bitmap, int size=32)
        {
            wxFileName path(g_Platform->GetSharedDataDir(), icon_name);
            path.AppendDir("uidata");
            path.AppendDir("traditional");
            bool ok = false;


            if (path.IsFileReadable()) {
                bitmap = LoadSVG(path.GetFullPath(), size, size);
                ok = bitmap.IsOk();
            }

            if (!ok) {
                auto style = g_StyleManager->GetCurrentStyle();
                bitmap = wxBitmap(style->GetIcon( _T("default_pi"), size, size));
                wxLogMessage("Icon: %s not found.", path.GetFullPath());
            }
        }
};
#endif

//------------------------------------------------------------------------------
//    NMEA Event Implementation
//    PlugIn Messaging scheme Event
//------------------------------------------------------------------------------

const wxEventType wxEVT_OCPN_MSG = wxNewEventType();

OCPN_MsgEvent::OCPN_MsgEvent(wxEventType commandType, int id)
    : wxEvent(id, commandType) {}

OCPN_MsgEvent::~OCPN_MsgEvent() {}

wxEvent *OCPN_MsgEvent::Clone() const {
  OCPN_MsgEvent *newevent = new OCPN_MsgEvent(*this);
  newevent->m_MessageID =
      this->m_MessageID
          .c_str();  // this enforces a deep copy of the string data
  newevent->m_MessageText = this->m_MessageText.c_str();
  return newevent;
}

//------------------------------------------------------------------------------------------------
//
//          The PlugInToolbarToolContainer Implementation
//
//------------------------------------------------------------------------------------------------
PlugInToolbarToolContainer::PlugInToolbarToolContainer() {
  bitmap_dusk = NULL;
  bitmap_night = NULL;
  bitmap_day = NULL;
  bitmap_Rollover_day = NULL;
  bitmap_Rollover_dusk = NULL;
  bitmap_Rollover_night = NULL;
}

PlugInToolbarToolContainer::~PlugInToolbarToolContainer() {
  delete bitmap_dusk;
  delete bitmap_night;
  delete bitmap_day;
  delete bitmap_Rollover_day;
  delete bitmap_Rollover_dusk;
  delete bitmap_Rollover_night;
}

//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Manager Implementation
//
//-----------------------------------------------------------------------------------------------------
PlugInManager *s_ppim;

BEGIN_EVENT_TABLE(PlugInManager, wxEvtHandler)
#if !defined(__OCPN__ANDROID__) && defined(OCPN_USE_CURL)
EVT_CURL_END_PERFORM(CurlThreadId, PlugInManager::OnEndPerformCurlDownload)
EVT_CURL_DOWNLOAD(CurlThreadId, PlugInManager::OnCurlDownload)
#endif
END_EVENT_TABLE()

static void event_message_box(const wxString& msg) {
  OCPNMessageBox(NULL, msg, wxString(_("OpenCPN Info")),
                 wxICON_INFORMATION | wxOK, 10);  // 10 second timeout
}

static void event_message_box(const wxString& msg, wxCommandEvent ev) {
  auto s = wxString::Format(msg, ev.GetString());
  event_message_box(s);
}

static void OnLoadPlugin(const PlugInContainer* pic) {
  if (g_options) {
    if ((pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE)) {
      if (!pic->m_bToolboxPanel) NotifySetupOptionsPlugin(pic);
    }
  }
}


PlugInManager::PlugInManager(MyFrame *parent) {
#if !defined(__OCPN__ANDROID__) && defined(OCPN_USE_CURL)
  m_pCurlThread = NULL;
  m_pCurl = 0;
#endif
  pParent = parent;
  s_ppim = this;

  MyFrame *pFrame = GetParentFrame();
  if (pFrame) {
    m_plugin_menu_item_id_next = CanvasMenuHandler::GetNextContextMenuId();
    m_plugin_tool_id_next = pFrame->GetNextToolbarToolId();
  }

#ifdef __OCPN__ANDROID__
  //  Due to the oddball mixed static/dynamic linking model used in the Android
  //  architecture, all classes used in PlugIns must be present in the core,
  //  even if stubs.
  //
  //  Here is where we do that....
  if (pFrame) {
    wxArrayString as;
    as.Add(_T("Item0"));
    wxRadioBox *box =
        new wxRadioBox(pFrame, -1, _T(""), wxPoint(0, 0), wxSize(-1, -1), as);
    delete box;
  }

#endif

#if !defined(__OCPN__ANDROID__) && defined (OCPN_USE_CURL)
  wxCurlBase::Init();
  m_last_online = false;
  m_last_online_chk = -1;
#endif

  m_utilHandler = new pluginUtilHandler();
  m_listPanel = NULL;
  m_blacklist = blacklist_factory();
  m_blacklist_ui = std::unique_ptr<BlacklistUI>(new BlacklistUI());
  HandlePluginLoaderEvents();
  auto loader = PluginLoader::getInstance();
}
PlugInManager::~PlugInManager() {
#if !defined(__OCPN__ANDROID__) && defined (OCPN_USE_CURL)
  wxCurlBase::Shutdown();
#endif
}

void PlugInManager::InitCommListeners(void) {

  // Initialize the comm listener to support
  // void SetNMEASentence(wxString &sentence);

  auto& msgbus = NavMsgBus::getInstance();

  //NMEA0183, all
  Nmea0183Msg n0183_msg("", "");
  listener_N0183 = msgbus.get_listener(EVT_N0183_PLUGIN, this, n0183_msg.key());

  Bind(EVT_N0183_PLUGIN, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183( n0183_msg );
      });
}

void PlugInManager::HandleN0183( std::shared_ptr <const Nmea0183Msg> n0183_msg ) {

}

/**
 * Set up actions to perform for messages generated by PluginLoader's
 * evt_foo.notify() calls.
 */

wxDEFINE_EVENT(EVT_BLACKLISTED_PLUGIN, wxCommandEvent);
wxDEFINE_EVENT(EVT_DEACTIVATE_PLUGIN, wxCommandEvent);
wxDEFINE_EVENT(EVT_INCOMPATIBLE_PLUGIN, wxCommandEvent);
wxDEFINE_EVENT(EVT_LOAD_DIRECTORY, wxCommandEvent);
wxDEFINE_EVENT(EVT_LOAD_PLUGIN, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLUGIN_UNLOAD, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLUGLIST_CHANGE, wxCommandEvent);
wxDEFINE_EVENT(EVT_UNREADABLE_PLUGIN, wxCommandEvent);
wxDEFINE_EVENT(EVT_UPDATE_CHART_TYPES, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLUGIN_LOADALL_FINALIZE, wxCommandEvent);
wxDEFINE_EVENT(EVT_VERSION_INCOMPATIBLE_PLUGIN, wxCommandEvent);

void PlugInManager::HandlePluginLoaderEvents() {
  auto loader = PluginLoader::getInstance();

  evt_blacklisted_plugin_listener =
    loader->evt_blacklisted_plugin.get_listener(this, EVT_BLACKLISTED_PLUGIN);
  Bind(EVT_BLACKLISTED_PLUGIN, [&](wxCommandEvent& ev) {
    m_blacklist_ui->message(ev.GetString().ToStdString()); });

  evt_deactivate_plugin_listener =
    loader->evt_deactivate_plugin.get_listener(this, EVT_DEACTIVATE_PLUGIN);
  Bind(EVT_DEACTIVATE_PLUGIN, [&](wxCommandEvent& ev) {
    auto pic = static_cast<const PlugInContainer*>(ev.GetClientData());
    OnPluginDeactivate(pic); });

  evt_incompatible_plugin_listener =
    loader->evt_incompatible_plugin.get_listener(this,
                                                 EVT_INCOMPATIBLE_PLUGIN);
  Bind(EVT_INCOMPATIBLE_PLUGIN,
       [&](wxCommandEvent& ev) { event_message_box(ev.GetString()); });

  evt_pluglist_change_listener =
    loader->evt_pluglist_change.get_listener(this, EVT_PLUGLIST_CHANGE);
  Bind(EVT_PLUGLIST_CHANGE, [&](wxCommandEvent&) {
    if (m_listPanel) m_listPanel->ReloadPluginPanels();
    g_options->itemBoxSizerPanelPlugins->Layout(); });

  evt_load_directory_listener =
    loader->evt_load_directory.get_listener(this, EVT_LOAD_DIRECTORY);
  Bind(EVT_LOAD_DIRECTORY, [&](wxCommandEvent&) {
    pConfig->SetPath("/PlugIns/");
    SetPluginOrder(pConfig->Read("PluginOrder", wxEmptyString)); });

  evt_load_plugin_listener =
    loader->evt_load_plugin.get_listener(this, EVT_LOAD_PLUGIN);
  Bind(EVT_LOAD_PLUGIN, [&](wxCommandEvent& ev) {
    auto pic = static_cast<const PlugInContainer*>(ev.GetClientData());
    OnLoadPlugin(pic); });

  evt_version_incompatible_plugin_listener =
    loader->evt_version_incompatible_plugin.get_listener(
      this,
      EVT_VERSION_INCOMPATIBLE_PLUGIN);
  Bind(EVT_VERSION_INCOMPATIBLE_PLUGIN, [&](wxCommandEvent& ev) {
    static const wxString msg =
      _("The plugin %s is not compatible with this version "
        "of OpenCPN, please get an updated version.");
    event_message_box(msg, ev); });

  evt_unreadable_plugin_listener =
    loader->evt_blacklisted_plugin.get_listener(this, EVT_UNREADABLE_PLUGIN);
  Bind(EVT_UNREADABLE_PLUGIN, [&](wxCommandEvent& ev) {
    static const wxString msg =
      _("Unreadable Plugin library %s detected, check file permissions:\n\n");
    event_message_box(msg, ev); });

  evt_incompatible_plugin_listener =
    loader->evt_incompatible_plugin.get_listener(this,
                                                 EVT_INCOMPATIBLE_PLUGIN);
  Bind(EVT_INCOMPATIBLE_PLUGIN,
       [&](wxCommandEvent& ev) { event_message_box(ev.GetString()); });

  evt_update_chart_types_listener =
    loader->evt_update_chart_types.get_listener(this, EVT_UPDATE_CHART_TYPES);
  Bind(EVT_UPDATE_CHART_TYPES,
       [&](wxCommandEvent& ev) { UpDateChartDataTypes(); });

  evt_plugin_loadall_finalize_listener =
    loader->evt_plugin_loadall_finalize.get_listener(this, EVT_PLUGIN_LOADALL_FINALIZE);
  Bind(EVT_PLUGIN_LOADALL_FINALIZE,
       [&](wxCommandEvent& ev) { FinalizePluginLoadall(); });

}

/**
 * Set up actions to perform for messages generated by PluginHandlers's
 * evt_foo.notify() calls.
 */
wxDEFINE_EVENT(EVT_DOWNLOAD_FAILED, wxCommandEvent);
wxDEFINE_EVENT(EVT_DOWNLOAD_OK, wxCommandEvent);

void PlugInManager::HandlePluginHandlerEvents() {
  auto loader = PluginLoader::getInstance();

  evt_download_failed_listener =
    loader->evt_update_chart_types.get_listener(this, EVT_DOWNLOAD_FAILED);
  Bind(EVT_DOWNLOAD_FAILED, [&](wxCommandEvent& ev) {
      wxString message = _("Please check system log for more info.");
      OCPNMessageBox(gFrame, message, _("Installation error"),
                     wxICON_ERROR | wxOK | wxCENTRE); });

  evt_download_ok_listener =
    loader->evt_update_chart_types.get_listener(this, EVT_DOWNLOAD_OK);
  Bind(EVT_DOWNLOAD_OK, [&](wxCommandEvent& ev) {
    wxString message(ev.GetString());
    message += _(" successfully installed from cache");
    OCPNMessageBox(gFrame, message, _("Installation complete"),
                   wxICON_INFORMATION | wxOK | wxCENTRE); });
}

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
  vector<string> dirs = PluginPaths::getInstance()->Libdirs();
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
    if (os_name.Contains(_T("wxQT"))) {
      wxLogMessage(_T("setLoadPath() using Android library path"));
    } else
      wxLogWarning("SetLoadPath: Unsupported platform.");
  }
  if (osSystemId & wxOS_MAC || osSystemId & wxOS_UNIX_LINUX) {
    vector<string> dirs = PluginPaths::getInstance()->Bindirs();
    string path = ocpn::join(dirs, ':');
    wxString envPath;
    wxGetEnv("PATH", &envPath);
    path = path + ":" + envPath.ToStdString();
    wxLogMessage("Using PATH: %s", path);
    wxSetEnv("PATH", path);
  }
}

bool PlugInManager::CallLateInit(void) {
  bool bret = true;

  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = (*plugin_array)[i];

    switch (pic->m_api_version) {
      case 110:
      case 111:
      case 112:
      case 113:
      case 114:
      case 115:
      case 116:
      case 117:
        ProcessLateInit(pic);
        break;
    }
  }

  return bret;
}

void PlugInManager::ProcessLateInit(PlugInContainer *pic) {
  if (pic->m_cap_flag & WANTS_LATE_INIT) {
    wxString msg(_T("PlugInManager: Calling LateInit PlugIn: "));
    msg += pic->m_plugin_file;
    wxLogMessage(msg);

    opencpn_plugin_110 *ppi =
        dynamic_cast<opencpn_plugin_110 *>(pic->m_pplugin);
    if (ppi) ppi->LateInit();
  }
}

void PlugInManager::OnPluginDeactivate(const PlugInContainer* pic) {
  // Unload chart cache if this plugin is responsible for any charts
  if ((pic->m_cap_flag & INSTALLS_PLUGIN_CHART) ||
      (pic->m_cap_flag & INSTALLS_PLUGIN_CHART_GL)) {
    ChartData->PurgeCachePlugins();
    gFrame->InvalidateAllQuilts();

  }
  //    Deactivate (Remove) any ToolbarTools added by this PlugIn
  for (unsigned int i = 0; i < m_PlugInToolbarTools.GetCount(); i++) {
    PlugInToolbarToolContainer* pttc = m_PlugInToolbarTools[i];

    if (pttc->m_pplugin == pic->m_pplugin) {
      m_PlugInToolbarTools.Remove(pttc);
      delete pttc;
    }
  }

  //    Deactivate (Remove) any ContextMenu items addded by this PlugIn
  for (unsigned int i = 0; i < m_PlugInMenuItems.GetCount(); i++) {
    PlugInMenuItemContainer* pimis = m_PlugInMenuItems[i];
    if (pimis->m_pplugin == pic->m_pplugin) {
      m_PlugInMenuItems.Remove(pimis);
      delete pimis;
    }
  }
  // *pic is a malloc'ed copy of the original *pic, owned by us.
  free(const_cast<PlugInContainer*>(pic));
}


void PlugInManager::SendVectorChartObjectInfo(const wxString &chart,
                                              const wxString &feature,
                                              const wxString &objname,
                                              double &lat, double &lon,
                                              double &scale, int &nativescale) {
  wxString decouple_chart(chart);
  wxString decouple_feature(feature);
  wxString decouple_objname(objname);
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = (*plugin_array)[i];
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_VECTOR_CHART_OBJECT_INFO) {
        switch (pic->m_api_version) {
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117: {
            opencpn_plugin_112 *ppi =
                dynamic_cast<opencpn_plugin_112 *>(pic->m_pplugin);
            if (ppi)
              ppi->SendVectorChartObjectInfo(decouple_chart, decouple_feature,
                                             decouple_objname, lat, lon, scale,
                                             nativescale);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

bool PlugInManager::IsAnyPlugInChartEnabled() {
  //  Is there a PlugIn installed and active that implements PlugIn Chart
  //  type(s)?
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = (*plugin_array)[i];
    if (pic->m_bEnabled && pic->m_bInitState) {
      if ((pic->m_cap_flag & INSTALLS_PLUGIN_CHART) ||
          (pic->m_cap_flag & INSTALLS_PLUGIN_CHART_GL))
        return true;
    }
  }
  return false;
}

void PlugInManager::UpdateManagedPlugins() {
  PlugInContainer *pict;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  // Clear the status (to "unmanaged") on all plugins
  for (size_t i = 0; i < plugin_array->GetCount(); i++) {
    pict = plugin_array->Item(i);
    plugin_array->Item(i)->m_pluginStatus = PluginStatus::Unmanaged;

    // Pre-mark the default "system" plugins
    auto r =
        std::find(SYSTEM_PLUGINS.begin(), SYSTEM_PLUGINS.end(),
                  plugin_array->Item(i)->m_common_name.Lower().ToStdString());
    if (r != SYSTEM_PLUGINS.end())
      plugin_array->Item(i)->m_pluginStatus = PluginStatus::System;
  }

  std::vector<PluginMetadata> available = getCompatiblePlugins();

  // Traverse the list again
  // Remove any inactive/uninstalled managed plugins that are no longer
  // available in the current catalog Usually due to reverting from Alpha/Beta
  // catalog back to master
  for (size_t i = 0; i < plugin_array->GetCount(); i++) {
    pict = plugin_array->Item(i);
    if (pict->m_ManagedMetadata.name
            .size()) {  // If metadata is good, must be a managed plugin
      bool bfound = false;
      for (auto plugin : available) {
        if (pict->m_common_name.IsSameAs(wxString(plugin.name.c_str()))) {
          bfound = true;
          break;
        }
      }
      if (!bfound) {
        if (!pict->m_pplugin) {  // Only remove inactive plugins
          plugin_array->Item(i)->m_pluginStatus =
              PluginStatus::PendingListRemoval;
        }
      }
    }
  }

  //  Remove any list items marked
  size_t i = 0;
  while ((i >= 0) && (i < plugin_array->GetCount())) {
    pict = plugin_array->Item(i);
    if (pict->m_pluginStatus == PluginStatus::PendingListRemoval) {
      plugin_array->RemoveAt(i);
      i = 0;
    } else
      i++;
  }

  for (size_t i = 0; i < plugin_array->GetCount(); i++) {
    pict = plugin_array->Item(i);
    int yyp = 4;
  }

  //  Now merge and update from the catalog
  for (auto plugin : available) {
    PlugInContainer *pic = NULL;
    // Search for an exact name match in the existing plugin array
    bool bfound = false;
    for (size_t i = 0; i < plugin_array->GetCount(); i++) {
      pic = plugin_array->Item(i);
      if (plugin_array->Item(i)->m_common_name.IsSameAs(
              wxString(plugin.name.c_str()))) {
        bfound = true;
        break;
      }
    }

    //  No match found, so add a container, and populate it
    if (!bfound) {
      PlugInContainer *new_pic = new PlugInContainer;
      new_pic->m_common_name = wxString(plugin.name.c_str());
      new_pic->m_pluginStatus = PluginStatus::ManagedInstallAvailable;
      new_pic->m_ManagedMetadata = plugin;
      new_pic->m_version_major = 0;
      new_pic->m_version_minor = 0;

      // In safe mode, check to see if the plugin appears to be installed
      // If so, set the status to "ManagedInstalledCurrentVersion", thus
      // enabling the "uninstall" button.
      if (safe_mode::get_mode()) {
        std::string installed;
        if (isRegularFile(PluginHandler::fileListPath(plugin.name).c_str())) {
          // Get the installed version from the manifest
          std::string path = PluginHandler::versionPath(plugin.name);
          if (path != "" && wxFileName::IsFileReadable(path)) {
            std::ifstream stream;
            stream.open(path, std::ifstream::in);
            stream >> installed;
          }
        }
        if (!installed.empty())
          new_pic->m_pluginStatus =
              PluginStatus::ManagedInstalledCurrentVersion;
        else
          new_pic->m_pluginStatus = PluginStatus::Unknown;
      }

      plugin_array->Add(new_pic);

    }
    // Match found, so merge the info and determine the plugin status
    else {
      // If the managed plugin is installed, the fileList (manifest) will be
      // present
      if (isRegularFile(PluginHandler::fileListPath(plugin.name).c_str())) {
        // Get the installed version from the manifest
        std::string installed;
        std::string path = PluginHandler::versionPath(plugin.name);
        if (path != "" && wxFileName::IsFileReadable(path)) {
          std::ifstream stream;
          stream.open(path, std::ifstream::in);
          stream >> installed;
        }
        pic->m_InstalledManagedVersion = installed;
        auto installedVersion = SemanticVersion::parse(installed);

        // Compare to the version reported in metadata
        auto metaVersion = SemanticVersion::parse(plugin.version);
        if (installedVersion < metaVersion)
          pic->m_pluginStatus = PluginStatus::ManagedInstalledUpdateAvailable;
        else if (installedVersion == metaVersion)
          pic->m_pluginStatus = PluginStatus::ManagedInstalledCurrentVersion;
        else if (installedVersion > metaVersion)
          pic->m_pluginStatus =
              PluginStatus::ManagedInstalledDowngradeAvailable;

        pic->m_ManagedMetadata = plugin;
      }

      // If the new plugin is not installed....
      else {
        // If the plugin is actually loaded, but the new plugin is known not to
        // be installed,
        //  then there must be a legacy plugin loaded.
        //  and the new status must be "PluginStatus::LegacyUpdateAvailable"
        if (pic->m_api_version) {
          pic->m_pluginStatus = PluginStatus::LegacyUpdateAvailable;
          pic->m_ManagedMetadata = plugin;
        }
        // Otherwise, this is an uninstalled managed plugin.
        else {
          pic->m_pluginStatus = PluginStatus::ManagedInstallAvailable;
        }
      }
    }
  }

  // Sort the list

  // Detach and hold the uninstalled, managed plugins
  std::map<std::string, PlugInContainer *> sortmap;
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_pluginStatus == PluginStatus::ManagedInstallAvailable) {
      plugin_array->Remove(pic);

      // Sort by name, lower cased.
      std::string name = pic->m_ManagedMetadata.name;
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      sortmap[name] = pic;
      i = 0;  // Restart the list
    }
  }

  // Add the detached plugins back at the top of the list.
  //  Later, the list will be populated in reverse order...Why??
  for (std::map<std::string, PlugInContainer *>::iterator i = sortmap.begin();
       i != sortmap.end(); i++) {
    PlugInContainer *pic = i->second;
    plugin_array->Insert(pic, 0);
  }

  if (m_listPanel) m_listPanel->ReloadPluginPanels();

  g_options->itemBoxSizerPanelPlugins->Layout();
}

bool PlugInManager::UpDateChartDataTypes() {
  bool bret = false;
  if (NULL == ChartData) return bret;

  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);

    if (pic->m_bInitState) {
      if ((pic->m_cap_flag & INSTALLS_PLUGIN_CHART) ||
          (pic->m_cap_flag & INSTALLS_PLUGIN_CHART_GL))
        bret = true;
    }
  }

  if (bret) ChartData->UpdateChartClassDescriptorArray();

  return bret;
}

void PlugInManager::FinalizePluginLoadall() {

  //FIXME
  //  Maybe this does not need to be done for CLI instance?
  // Inform plugins of the current color scheme
  SetColorSchemeForAllPlugIns( global_color_scheme );

  // Tell all the PlugIns about the current OCPN configuration
  SendBaseConfigToAllPlugIns();
  SendS52ConfigToAllPlugIns( true );

  // Inform Plugins of OpenGL configuration, if enabled
  if(g_bopengl){
      if(gFrame->GetPrimaryCanvas()->GetglCanvas())
          gFrame->GetPrimaryCanvas()->GetglCanvas()->SendJSONConfigMessage();
  }

  //  And then reload all catalogs.
  ReloadLocale();

}

void PlugInManager::SetPluginOrder(wxString serialized_names) {
  m_plugin_order.Empty();
  wxStringTokenizer tokenizer(serialized_names, _T(";"));
  while (tokenizer.HasMoreTokens()) {
    m_plugin_order.Add(tokenizer.GetNextToken());
  }
}

wxString PlugInManager::GetPluginOrder() {
  wxString plugins = wxEmptyString;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    plugins.Append(plugin_array->Item(i)->m_common_name);
    if (i < plugin_array->GetCount() - 1) plugins.Append(';');
  }
  return plugins;
}

bool PlugInManager::UpdateConfig() {
  //    pConfig->SetPath( _T("/PlugIns/") );
  //    pConfig->Write( _T("PluginOrder"), GetPluginOrder() );

  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);

    if (pic) {
      wxString config_section = (_T ( "/PlugIns/" ));
      config_section += pic->m_plugin_filename;
      pConfig->SetPath(config_section);
      pConfig->Write(_T ( "bEnabled" ), pic->m_bEnabled);
    }
  }

  return true;
}

void PlugInManager::ShowDeferredBlacklistMessages() {
  m_blacklist_ui->show_deferred_messages();
}

bool PlugInManager::CheckBlacklistedPlugin(const PluginMetadata plugin) {
  auto v = SemanticVersion::parse(plugin.version);
  return CheckBlacklistedPlugin(wxString(plugin.name), v.major, v.minor);
}

bool PlugInManager::CheckBlacklistedPlugin(opencpn_plugin *plugin) {
  int major = plugin->GetPlugInVersionMajor();
  int minor = plugin->GetPlugInVersionMinor();

#ifdef __WXMSW__
  wxString name = wxString::FromAscii(typeid(*plugin).name());
  name.Replace("class ", wxEmptyString);
#else
  const std::type_info &ti = typeid(*plugin);
  int status;
  char *realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
  wxString name = wxString::FromAscii(realname);
  free(realname);
#endif  // __WXMSW__
  return CheckBlacklistedPlugin(name, major, minor);
}

bool PlugInManager::CheckBlacklistedPlugin(wxString name, int major, int minor) {
  auto block_status = m_blacklist->get_status(name.ToStdString(), major, minor);
  if (block_status == plug_status::unblocked) return true;
  plug_data data(name.ToStdString(), major, minor);
  auto msg = m_blacklist->get_message(block_status, data);
  m_blacklist_ui->message(msg);
  return false;
}

bool PlugInManager::RenderAllCanvasOverlayPlugIns(ocpnDC &dc,
                                                  const ViewPort &vp,
                                                  int canvasIndex) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_OVERLAY_CALLBACK) {
        PlugIn_ViewPort pivp = CreatePlugInViewport(vp);

        wxDC *pdc = dc.GetDC();
        if (pdc)  // not in OpenGL mode
        {
          switch (pic->m_api_version) {
            case 106: {
              opencpn_plugin_16 *ppi =
                  dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
              if (ppi) ppi->RenderOverlay(*pdc, &pivp);
              break;
            }
            case 107: {
              opencpn_plugin_17 *ppi =
                  dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
              if (ppi) ppi->RenderOverlay(*pdc, &pivp);
              break;
            }
            case 108:
            case 109:
            case 110:
            case 111:
            case 112:
            case 113:
            case 114:
            case 115: {
              opencpn_plugin_18 *ppi =
                  dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
              if (ppi) ppi->RenderOverlay(*pdc, &pivp);
              break;
            }
            case 116:
            case 117: {
              opencpn_plugin_18 *ppi =
                  dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
              if (ppi) {
                ppi->RenderOverlay(*pdc, &pivp);
              }
              opencpn_plugin_116 *ppi116 =
                  dynamic_cast<opencpn_plugin_116 *>(pic->m_pplugin);
              if (ppi116)
                ppi116->RenderOverlayMultiCanvas(*pdc, &pivp, canvasIndex);
              break;
            }
            default:
              break;
          }
        } else {
          //    If in OpenGL mode, and the PlugIn has requested OpenGL render
          //    callbacks, then there is no need to render by wxDC here.
          if (pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK) continue;

          if ((m_cached_overlay_bm.GetWidth() != vp.pix_width) ||
              (m_cached_overlay_bm.GetHeight() != vp.pix_height))
            m_cached_overlay_bm.Create(vp.pix_width, vp.pix_height, -1);

          wxMemoryDC mdc;
          mdc.SelectObject(m_cached_overlay_bm);
          mdc.SetBackground(*wxBLACK_BRUSH);
          mdc.Clear();

          bool b_rendered = false;

          switch (pic->m_api_version) {
            case 106: {
              opencpn_plugin_16 *ppi =
                  dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
              if (ppi) b_rendered = ppi->RenderOverlay(mdc, &pivp);
              break;
            }
            case 107: {
              opencpn_plugin_17 *ppi =
                  dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
              if (ppi) b_rendered = ppi->RenderOverlay(mdc, &pivp);
              break;
            }
            case 108:
            case 109:
            case 110:
            case 111:
            case 112:
            case 113:
            case 114:
            case 115: {
              opencpn_plugin_18 *ppi =
                  dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
              if (ppi) b_rendered = ppi->RenderOverlay(*pdc, &pivp);
              break;
            }
            case 116:
            case 117: {
              opencpn_plugin_18 *ppi =
                  dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
              if (ppi) {
                b_rendered = ppi->RenderOverlay(*pdc, &pivp);
              }
              opencpn_plugin_116 *ppi116 =
                  dynamic_cast<opencpn_plugin_116 *>(pic->m_pplugin);
              if (ppi116)
                b_rendered = ppi116->RenderOverlayMultiCanvas(*pdc, &pivp,
                                                              g_canvasConfig);
              break;
            }
            default: {
              b_rendered = pic->m_pplugin->RenderOverlay(&mdc, &pivp);
              break;
            }
          }

          mdc.SelectObject(wxNullBitmap);

          if (b_rendered) {
            wxMask *p_msk = new wxMask(m_cached_overlay_bm, wxColour(0, 0, 0));
            m_cached_overlay_bm.SetMask(p_msk);

            dc.DrawBitmap(m_cached_overlay_bm, 0, 0, true);
          }
        }
      } else if (pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK) {
      }
    }
  }

  return true;
}

bool PlugInManager::RenderAllGLCanvasOverlayPlugIns(wxGLContext *pcontext,
                                                    const ViewPort &vp,
                                                    int canvasIndex) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK) {
        PlugIn_ViewPort pivp = CreatePlugInViewport(vp);

        switch (pic->m_api_version) {
          case 107: {
            opencpn_plugin_17 *ppi =
                dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
            if (ppi) ppi->RenderGLOverlay(pcontext, &pivp);
            break;
          }

          case 108:
          case 109:
          case 110:
          case 111:
          case 112:
          case 113:
          case 114:
          case 115: {
            opencpn_plugin_18 *ppi =
                dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
            if (ppi) ppi->RenderGLOverlay(pcontext, &pivp);
            break;
          }
          case 116:
          case 117: {
            opencpn_plugin_18 *ppi =
                dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
            if (ppi) {
              ppi->RenderGLOverlay(pcontext, &pivp);
            }
            opencpn_plugin_116 *ppi116 =
                dynamic_cast<opencpn_plugin_116 *>(pic->m_pplugin);
            if (ppi116) {
              ppi116->RenderGLOverlayMultiCanvas(pcontext, &pivp, canvasIndex);
            }
            break;
          }
          default:
            break;
        }
      }
    }
  }

  return true;
}

bool PlugInManager::SendMouseEventToPlugins(wxMouseEvent &event) {
  bool bret = false;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_MOUSE_EVENTS) {
        switch (pic->m_api_version) {
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117: {
            opencpn_plugin_112 *ppi =
                dynamic_cast<opencpn_plugin_112 *>(pic->m_pplugin);
            if (ppi)
              if (ppi->MouseEventHook(event)) bret = true;
            break;
          }
          default:
            break;
        }
      }
    }
  }

  return bret;
}

bool PlugInManager::SendKeyEventToPlugins(wxKeyEvent &event) {
  bool bret = false;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_KEYBOARD_EVENTS) {
        {
          switch (pic->m_api_version) {
            case 113:
            case 114:
            case 115:
            case 116:
            case 117: {
              opencpn_plugin_113 *ppi =
                  dynamic_cast<opencpn_plugin_113 *>(pic->m_pplugin);
              if (ppi && ppi->KeyboardEventHook(event)) bret = true;
              break;
            }
            default:
              break;
          }
        }
      }
    }
  }

  return bret;
  ;
}

void PlugInManager::SendViewPortToRequestingPlugIns(ViewPort &vp) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_ONPAINT_VIEWPORT) {
        PlugIn_ViewPort pivp = CreatePlugInViewport(vp);
        if (pic->m_pplugin) pic->m_pplugin->SetCurrentViewPort(pivp);
      }
    }
  }
}

void PlugInManager::SendCursorLatLonToAllPlugIns(double lat, double lon) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_CURSOR_LATLON)
        if (pic->m_pplugin) pic->m_pplugin->SetCursorLatLon(lat, lon);
    }
  }
}

void NotifySetupOptionsPlugin(const PlugInContainer *pic) {
  if (pic->m_bEnabled && pic->m_bInitState) {
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
        case 117: {
          opencpn_plugin_19 *ppi =
              dynamic_cast<opencpn_plugin_19 *>(pic->m_pplugin);
          if (ppi) {
            ppi->OnSetupOptions();
            const_cast<PlugInContainer*>(pic)->m_bToolboxPanel = true;
          }
          break;
        }
        default:
          break;
      }
    }
  }
}

void PlugInManager::NotifySetupOptions() {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    NotifySetupOptionsPlugin(pic);
  }
}

void PlugInManager::ClosePlugInPanel(PlugInContainer *pic,
                                     int ok_apply_cancel) {
  if (pic->m_bEnabled && pic->m_bInitState) {
    if ((pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE) && pic->m_bToolboxPanel) {
      pic->m_pplugin->OnCloseToolboxPanel(0, ok_apply_cancel);
      pic->m_bToolboxPanel = false;
    }
  }
}

void PlugInManager::CloseAllPlugInPanels(int ok_apply_cancel) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic) {
      ClosePlugInPanel(pic, ok_apply_cancel);
    }
  }
}

int PlugInManager::AddCanvasContextMenuItem(wxMenuItem *pitem,
                                            opencpn_plugin *pplugin,
                                            const char *name) {
  PlugInMenuItemContainer *pmic = new PlugInMenuItemContainer;
  pmic->pmenu_item = pitem;
  pmic->m_pplugin = pplugin;
  pmic->id = pitem->GetId() == wxID_SEPARATOR ? wxID_SEPARATOR
                                              : m_plugin_menu_item_id_next;
  pmic->b_viz = true;
  pmic->b_grey = false;
  pmic->m_in_menu = name;

  m_PlugInMenuItems.Add(pmic);

  m_plugin_menu_item_id_next++;

  return pmic->id;
}

void PlugInManager::RemoveCanvasContextMenuItem(int item, const char *name) {
  for (unsigned int i = 0; i < m_PlugInMenuItems.GetCount(); i++) {
    PlugInMenuItemContainer *pimis = m_PlugInMenuItems[i];
    {
      if (pimis->id == item && !strcmp(name, pimis->m_in_menu)) {
        m_PlugInMenuItems.Remove(pimis);
        delete pimis;
        break;
      }
    }
  }
}

void PlugInManager::SetCanvasContextMenuItemViz(int item, bool viz,
                                                const char *name) {
  for (unsigned int i = 0; i < m_PlugInMenuItems.GetCount(); i++) {
    PlugInMenuItemContainer *pimis = m_PlugInMenuItems[i];
    {
      if (pimis->id == item && !strcmp(name, pimis->m_in_menu)) {
        pimis->b_viz = viz;
        break;
      }
    }
  }
}

void PlugInManager::SetCanvasContextMenuItemGrey(int item, bool grey,
                                                 const char *name) {
  for (unsigned int i = 0; i < m_PlugInMenuItems.GetCount(); i++) {
    PlugInMenuItemContainer *pimis = m_PlugInMenuItems[i];
    {
      if (pimis->id == item && !strcmp(name, pimis->m_in_menu)) {
        pimis->b_grey = grey;
        break;
      }
    }
  }
}

void PlugInManager::SendNMEASentenceToAllPlugIns(const wxString &sentence) {
  wxString decouple_sentence(
      sentence);  // decouples 'const wxString &' and 'wxString &' to keep bin
                  // compat for plugins
#ifndef __WXMSW__
  // Set up a framework to catch (some) sigsegv faults from plugins.
  sigaction(SIGSEGV, NULL, &sa_all_PIM_previous);  // save existing
                                                   // action for this signal
  struct sigaction temp;
  sigaction(SIGSEGV, NULL, &temp);  // inspect existing action for this signal

  temp.sa_handler = catch_signals_PIM;  // point to my handler
  sigemptyset(&temp.sa_mask);           // make the blocking set
                                        // empty, so that all
                                        // other signals will be
                                        // unblocked during my handler
  temp.sa_flags = 0;
  sigaction(SIGSEGV, &temp, NULL);
#endif
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_NMEA_SENTENCES) {
#ifndef __WXMSW__
        if (sigsetjmp(env_PIM,
                      1)) {  //  Something in the "else" code block faulted.

          // Probably safest to assume that all variables in this method are
          // trash.. So, simply clean up and return.
          sigaction(SIGSEGV, &sa_all_PIM_previous,
                    NULL);  // reset signal handler
          return;
        } else
#endif
        {
          // volatile int *x = 0;
          //*x = 0;
          if (pic->m_pplugin)
            pic->m_pplugin->SetNMEASentence(decouple_sentence);
        }
      }
    }
  }

#ifndef __WXMSW__
  sigaction(SIGSEGV, &sa_all_PIM_previous, NULL);  // reset signal handler
#endif
}

int PlugInManager::GetJSONMessageTargetCount() {
  int rv = 0;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState &&
        (pic->m_cap_flag & WANTS_PLUGIN_MESSAGING))
      rv++;
  }
  return rv;
}

void PlugInManager::SendJSONMessageToAllPlugins(const wxString &message_id,
                                                wxJSONValue v) {
  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendMessageToAllPlugins(message_id, out);
  wxLogDebug(message_id);
  wxLogDebug(out);
}

void PlugInManager::SendMessageToAllPlugins(const wxString &message_id,
                                            const wxString &message_body) {
  g_lastPluginMessage = message_body;

  wxString decouple_message_id(
      message_id);  // decouples 'const wxString &' and 'wxString &' to keep bin
                    // compat for plugins
  wxString decouple_message_body(
      message_body);  // decouples 'const wxString &' and 'wxString &' to keep
                      // bin compat for plugins
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_PLUGIN_MESSAGING) {
        switch (pic->m_api_version) {
          case 106: {
            opencpn_plugin_16 *ppi =
                dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
            if (ppi)
              ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
            break;
          }
          case 107: {
            opencpn_plugin_17 *ppi =
                dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
            if (ppi)
              ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
            break;
          }
          case 108:
          case 109:
          case 110:
          case 111:
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117: {
            opencpn_plugin_18 *ppi =
                dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
            if (ppi)
              ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

void PlugInManager::SendAISSentenceToAllPlugIns(const wxString &sentence) {
  wxString decouple_sentence(
      sentence);  // decouples 'const wxString &' and 'wxString &' to keep bin
                  // compat for plugins
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_AIS_SENTENCES)
        pic->m_pplugin->SetAISSentence(decouple_sentence);
    }
  }
}

void PlugInManager::SendPositionFixToAllPlugIns(GenericPosDatEx *ppos) {
  //    Send basic position fix
  PlugIn_Position_Fix pfix;
  pfix.Lat = ppos->kLat;
  pfix.Lon = ppos->kLon;
  pfix.Cog = ppos->kCog;
  pfix.Sog = ppos->kSog;
  pfix.Var = ppos->kVar;
  pfix.FixTime = ppos->FixTime;
  pfix.nSats = ppos->nSats;

  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_NMEA_EVENTS)
        if (pic->m_pplugin) pic->m_pplugin->SetPositionFix(pfix);
    }
  }

  //    Send extended position fix to PlugIns at API 108 and later
  PlugIn_Position_Fix_Ex pfix_ex;
  pfix_ex.Lat = ppos->kLat;
  pfix_ex.Lon = ppos->kLon;
  pfix_ex.Cog = ppos->kCog;
  pfix_ex.Sog = ppos->kSog;
  pfix_ex.Var = ppos->kVar;
  pfix_ex.FixTime = ppos->FixTime;
  pfix_ex.nSats = ppos->nSats;
  pfix_ex.Hdt = ppos->kHdt;
  pfix_ex.Hdm = ppos->kHdm;

  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_NMEA_EVENTS) {
        switch (pic->m_api_version) {
          case 108:
          case 109:
          case 110:
          case 111:
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117: {
            opencpn_plugin_18 *ppi =
                dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
            if (ppi) ppi->SetPositionFixEx(pfix_ex);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

void PlugInManager::SendActiveLegInfoToAllPlugIns(ActiveLegDat *leg_info) {
  Plugin_Active_Leg_Info leg;
  leg.Btw = leg_info->Btw;
  leg.Dtw = leg_info->Dtw;
  leg.wp_name = leg_info->wp_name;
  leg.Xte = leg_info->Xte;
  leg.arrival = leg_info->arrival;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & WANTS_NMEA_EVENTS) {
        switch (pic->m_api_version) {
          case 108:
          case 109:
          case 110:
          case 111:
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
            break;
          case 117: {
            opencpn_plugin_117 *ppi =
                dynamic_cast<opencpn_plugin_117 *>(pic->m_pplugin);
            if (ppi) ppi->SetActiveLegInfo(leg);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

void PlugInManager::SendResizeEventToAllPlugIns(int x, int y) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState)
      pic->m_pplugin->ProcessParentResize(x, y);
  }
}

void PlugInManager::SetColorSchemeForAllPlugIns(ColorScheme cs) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState)
      pic->m_pplugin->SetColorScheme((PI_ColorScheme)cs);
  }
}

void PlugInManager::PrepareAllPluginContextMenus() {
  int canvasIndex = gFrame->GetCanvasIndexUnderMouse();
  if (canvasIndex < 0) return;

  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState) {
      if (pic->m_cap_flag & INSTALLS_CONTEXTMENU_ITEMS) {
        switch (pic->m_api_version) {
          case 116:
          case 117: {
            opencpn_plugin_116 *ppi =
                dynamic_cast<opencpn_plugin_116 *>(pic->m_pplugin);
            if (ppi) ppi->PrepareContextMenu(canvasIndex);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

void PlugInManager::SendSKConfigToAllPlugIns() {
  // Send the current ownship MMSI, encoded as sK,  to all PlugIns
  wxJSONValue v;
  v[_T("self")] = g_ownshipMMSI_SK;

  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendMessageToAllPlugins(wxString(_T("OCPN_CORE_SIGNALK")), out);
}

void PlugInManager::SendBaseConfigToAllPlugIns() {
  // Send the current run-time configuration to all PlugIns
  wxJSONValue v;
  v[_T("OpenCPN Version Major")] = VERSION_MAJOR;
  v[_T("OpenCPN Version Minor")] = VERSION_MINOR;
  v[_T("OpenCPN Version Patch")] = VERSION_PATCH;
  v[_T("OpenCPN Version Date")] = VERSION_DATE;
  v[_T("OpenCPN Version Full")] = VERSION_FULL;

  // Some useful display metrics
  if (g_MainToolbar) {
    v[_T("OpenCPN Toolbar Width")] = g_MainToolbar->GetSize().x;
    v[_T("OpenCPN Toolbar Height")] = g_MainToolbar->GetSize().y;
    v[_T("OpenCPN Toolbar PosnX")] = g_MainToolbar->GetPosition().x;
    v[_T("OpenCPN Toolbar PosnY")] = g_MainToolbar->GetPosition().y;
  }

  // Some rendering parameters
  v[_T("OpenCPN Zoom Mod Vector")] = g_chart_zoom_modifier_vector;
  v[_T("OpenCPN Zoom Mod Other")] = g_chart_zoom_modifier;
  v[_T("OpenCPN Display Width")] = (int)g_display_size_mm;

  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendMessageToAllPlugins(wxString(_T("OpenCPN Config")), out);
}

void PlugInManager::SendS52ConfigToAllPlugIns(bool bReconfig) {
  // Send the current run-time configuration to all PlugIns
  wxJSONValue v;
  v[_T("OpenCPN Version Major")] = VERSION_MAJOR;
  v[_T("OpenCPN Version Minor")] = VERSION_MINOR;
  v[_T("OpenCPN Version Patch")] = VERSION_PATCH;
  v[_T("OpenCPN Version Date")] = VERSION_DATE;
  v[_T("OpenCPN Version Full")] = VERSION_FULL;

  //  S52PLIB state
  if (ps52plib) {
    //         v[_T("OpenCPN S52PLIB ShowText")] = ps52plib->GetShowS57Text();
    //         v[_T("OpenCPN S52PLIB ShowSoundings")] =
    //         ps52plib->GetShowSoundings(); v[_T("OpenCPN S52PLIB ShowLights")]
    //         = !ps52plib->GetLightsOff();
    v[_T("OpenCPN S52PLIB ShowAnchorConditions")] = ps52plib->GetAnchorOn();
    v[_T("OpenCPN S52PLIB ShowQualityOfData")] = ps52plib->GetQualityOfData();
    //         v[_T("OpenCPN S52PLIB DisplayCategory")] =
    //         ps52plib->GetDisplayCategory();

    // Global parameters
    v[_T("OpenCPN S52PLIB MetaDisplay")] = ps52plib->m_bShowMeta;
    v[_T("OpenCPN S52PLIB DeclutterText")] = ps52plib->m_bDeClutterText;
    v[_T("OpenCPN S52PLIB ShowNationalText")] = ps52plib->m_bShowNationalTexts;
    v[_T("OpenCPN S52PLIB ShowImportantTextOnly")] =
        ps52plib->m_bShowS57ImportantTextOnly;
    v[_T("OpenCPN S52PLIB UseSCAMIN")] = ps52plib->m_bUseSCAMIN;
    v[_T("OpenCPN S52PLIB SymbolStyle")] = ps52plib->m_nSymbolStyle;
    v[_T("OpenCPN S52PLIB BoundaryStyle")] = ps52plib->m_nBoundaryStyle;
    v[_T("OpenCPN S52PLIB ColorShades")] =
        S52_getMarinerParam(S52_MAR_TWO_SHADES);
  }

  // Notify plugins that S52PLIB may have reconfigured global options
  v[_T("OpenCPN S52PLIB GlobalReconfig")] = bReconfig;

  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendMessageToAllPlugins(wxString(_T("OpenCPN Config")), out);
}

void PlugInManager::NotifyAuiPlugIns(void) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState &&
        (pic->m_cap_flag & USES_AUI_MANAGER))
      pic->m_pplugin->UpdateAuiStatus();
  }
}

int PlugInManager::AddToolbarTool(wxString label, wxBitmap *bitmap,
                                  wxBitmap *bmpRollover, wxItemKind kind,
                                  wxString shortHelp, wxString longHelp,
                                  wxObject *clientData, int position,
                                  int tool_sel, opencpn_plugin *pplugin) {
  PlugInToolbarToolContainer *pttc = new PlugInToolbarToolContainer;
  pttc->label = label;

  if (!bitmap->IsOk()) {
    ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
    pttc->bitmap_day = new wxBitmap(style->GetIcon(_T("default_pi")));
  } else {
    //  Force a non-reference copy of the bitmap from the PlugIn
    pttc->bitmap_day = new wxBitmap(*bitmap);
    pttc->bitmap_day->UnShare();
  }

  if (!bmpRollover->IsOk()) {
    ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
    pttc->bitmap_Rollover_day = new wxBitmap(style->GetIcon(_T("default_pi")));
  } else {
    //  Force a non-reference copy of the bitmap from the PlugIn
    pttc->bitmap_Rollover_day = new wxBitmap(*bmpRollover);
    pttc->bitmap_Rollover_day->UnShare();
  }

  pttc->bitmap_dusk = BuildDimmedToolBitmap(pttc->bitmap_day, 128);
  pttc->bitmap_night = BuildDimmedToolBitmap(pttc->bitmap_day, 32);
  pttc->bitmap_Rollover_dusk =
      BuildDimmedToolBitmap(pttc->bitmap_Rollover_day, 128);
  pttc->bitmap_Rollover_night =
      BuildDimmedToolBitmap(pttc->bitmap_Rollover_day, 32);

  pttc->kind = kind;
  pttc->shortHelp = shortHelp;
  pttc->longHelp = longHelp;
  pttc->clientData = clientData;
  pttc->position = position;
  pttc->m_pplugin = pplugin;
  pttc->tool_sel = tool_sel;
  pttc->b_viz = true;
  pttc->b_toggle = false;
  pttc->id = m_plugin_tool_id_next;

  m_PlugInToolbarTools.Add(pttc);

  m_plugin_tool_id_next++;

  return pttc->id;
}

int PlugInManager::AddToolbarTool(wxString label, wxString SVGfile,
                                  wxString SVGRolloverfile,
                                  wxString SVGToggledfile, wxItemKind kind,
                                  wxString shortHelp, wxString longHelp,
                                  wxObject *clientData, int position,
                                  int tool_sel, opencpn_plugin *pplugin) {
  PlugInToolbarToolContainer *pttc = new PlugInToolbarToolContainer;
  pttc->label = label;

  pttc->pluginNormalIconSVG = SVGfile;
  pttc->pluginRolloverIconSVG = SVGRolloverfile;
  pttc->pluginToggledIconSVG = SVGToggledfile;

  // Build a set of bitmaps based on the generic "puzzle piece" icon,
  // In case there is some problem with the SVG file(s) specified.
  ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
  pttc->bitmap_day = new wxBitmap(style->GetIcon(_T("default_pi")));
  pttc->bitmap_Rollover_day = new wxBitmap(style->GetIcon(_T("default_pi")));

  pttc->bitmap_dusk = BuildDimmedToolBitmap(pttc->bitmap_day, 128);
  pttc->bitmap_night = BuildDimmedToolBitmap(pttc->bitmap_day, 32);
  pttc->bitmap_Rollover_day = new wxBitmap(*pttc->bitmap_day);
  pttc->bitmap_Rollover_dusk =
      BuildDimmedToolBitmap(pttc->bitmap_Rollover_day, 128);
  pttc->bitmap_Rollover_night =
      BuildDimmedToolBitmap(pttc->bitmap_Rollover_day, 32);

  pttc->kind = kind;
  pttc->shortHelp = shortHelp;
  pttc->longHelp = longHelp;
  pttc->clientData = clientData;
  pttc->position = position;
  pttc->m_pplugin = pplugin;
  pttc->tool_sel = tool_sel;
  pttc->b_viz = true;
  pttc->b_toggle = false;
  pttc->id = m_plugin_tool_id_next;

  m_PlugInToolbarTools.Add(pttc);

  m_plugin_tool_id_next++;

  return pttc->id;
}

void PlugInManager::RemoveToolbarTool(int tool_id) {
  for (unsigned int i = 0; i < m_PlugInToolbarTools.GetCount(); i++) {
    PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools[i];
    {
      if (pttc->id == tool_id) {
        m_PlugInToolbarTools.Remove(pttc);
        delete pttc;
        break;
      }
    }
  }

  pParent->RequestNewToolbars();
}

void PlugInManager::SetToolbarToolViz(int item, bool viz) {
  for (unsigned int i = 0; i < m_PlugInToolbarTools.GetCount(); i++) {
    PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools[i];
    {
      if (pttc->id == item) {
        pttc->b_viz = viz;

        //      Apply the change
        pParent->RequestNewToolbars();

        break;
      }
    }
  }
}

void PlugInManager::SetToolbarItemState(int item, bool toggle) {
  for (unsigned int i = 0; i < m_PlugInToolbarTools.GetCount(); i++) {
    PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools[i];
    {
      if (pttc->id == item) {
        pttc->b_toggle = toggle;
        pParent->SetMasterToolbarItemState(item, toggle);
        break;
      }
    }
  }
}

void PlugInManager::SetToolbarItemBitmaps(int item, wxBitmap *bitmap,
                                          wxBitmap *bmpRollover) {
  for (unsigned int i = 0; i < m_PlugInToolbarTools.GetCount(); i++) {
    PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools[i];
    {
      if (pttc->id == item) {
        delete pttc->bitmap_day;
        delete pttc->bitmap_dusk;
        delete pttc->bitmap_night;
        delete pttc->bitmap_Rollover_day;

        if (!bitmap->IsOk()) {
          ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
          pttc->bitmap_day = new wxBitmap(style->GetIcon(_T("default_pi")));
        } else {
          //  Force a non-reference copy of the bitmap from the PlugIn
          pttc->bitmap_day = new wxBitmap(*bitmap);
          pttc->bitmap_day->UnShare();
        }

        if (!bmpRollover->IsOk()) {
          ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
          pttc->bitmap_Rollover_day =
              new wxBitmap(style->GetIcon(_T("default_pi")));
        } else {
          //  Force a non-reference copy of the bitmap from the PlugIn
          pttc->bitmap_Rollover_day = new wxBitmap(*bmpRollover);
          pttc->bitmap_Rollover_day->UnShare();
        }

        pttc->bitmap_dusk = BuildDimmedToolBitmap(pttc->bitmap_day, 128);
        pttc->bitmap_night = BuildDimmedToolBitmap(pttc->bitmap_day, 32);

        pParent->SetToolbarItemBitmaps(item, pttc->bitmap_day,
                                       pttc->bitmap_Rollover_day);
        break;
      }
    }
  }
}

void PlugInManager::SetToolbarItemBitmaps(int item, wxString SVGfile,
                                          wxString SVGfileRollover,
                                          wxString SVGfileToggled) {
  for (unsigned int i = 0; i < m_PlugInToolbarTools.GetCount(); i++) {
    PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools[i];
    {
      if (pttc->id == item) {
        pttc->pluginNormalIconSVG = SVGfile;
        pttc->pluginRolloverIconSVG = SVGfileRollover;
        pttc->pluginToggledIconSVG = SVGfileToggled;
        pParent->SetToolbarItemSVG(item, pttc->pluginNormalIconSVG,
                                   pttc->pluginRolloverIconSVG,
                                   pttc->pluginToggledIconSVG);
        break;
      }
    }
  }
}

opencpn_plugin *PlugInManager::FindToolOwner(const int id) {
  for (unsigned int i = 0; i < m_PlugInToolbarTools.GetCount(); i++) {
    PlugInToolbarToolContainer *pc = m_PlugInToolbarTools[i];
    if (id == pc->id) return pc->m_pplugin;
  }

  return NULL;
}

wxString PlugInManager::GetToolOwnerCommonName(const int id) {
  opencpn_plugin *ppi = FindToolOwner(id);
  if (ppi) {
    auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
    for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
      PlugInContainer *pic = plugin_array->Item(i);
      if (pic && (pic->m_pplugin == ppi)) return pic->m_common_name;
    }
  }

  return wxEmptyString;
}

wxString PlugInManager::GetLastError() { return m_last_error_string; }

wxBitmap *PlugInManager::BuildDimmedToolBitmap(wxBitmap *pbmp_normal,
                                               unsigned char dim_ratio) {
  wxImage img_dup = pbmp_normal->ConvertToImage();

  if (!img_dup.IsOk()) return NULL;

  if (dim_ratio < 200) {
    //  Create a dimmed version of the image/bitmap
    int gimg_width = img_dup.GetWidth();
    int gimg_height = img_dup.GetHeight();

    double factor = (double)(dim_ratio) / 256.0;

    for (int iy = 0; iy < gimg_height; iy++) {
      for (int ix = 0; ix < gimg_width; ix++) {
        if (!img_dup.IsTransparent(ix, iy)) {
          wxImage::RGBValue rgb(img_dup.GetRed(ix, iy),
                                img_dup.GetGreen(ix, iy),
                                img_dup.GetBlue(ix, iy));
          wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
          hsv.value = hsv.value * factor;
          wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
          img_dup.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
        }
      }
    }
  }

  //  Make a bitmap
  wxBitmap *ptoolBarBitmap;

#ifdef __WXMSW__
  wxBitmap tbmp(img_dup.GetWidth(), img_dup.GetHeight(), -1);
  wxMemoryDC dwxdc;
  dwxdc.SelectObject(tbmp);

  ptoolBarBitmap = new wxBitmap(img_dup, (wxDC &)dwxdc);
#else
  ptoolBarBitmap = new wxBitmap(img_dup);
#endif

  // store it
  return ptoolBarBitmap;
}

wxArrayString PlugInManager::GetPlugInChartClassNameArray(void) {
  wxArrayString array;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer *pic = plugin_array->Item(i);
    if (pic->m_bEnabled && pic->m_bInitState &&
        ((pic->m_cap_flag & INSTALLS_PLUGIN_CHART) ||
         (pic->m_cap_flag & INSTALLS_PLUGIN_CHART_GL))) {
      wxArrayString carray = pic->m_pplugin->GetDynamicChartClassNameArray();

      for (unsigned int j = 0; j < carray.GetCount(); j++) {
        array.Add(carray[j]);
      }
    }
  }

  //    Scrub the list for duplicates
  //    Corrects a flaw in BSB4 and NVC PlugIns
  unsigned int j = 0;
  while (j < array.GetCount()) {
    wxString test = array[j];
    unsigned int k = j + 1;
    while (k < array.GetCount()) {
      if (test == array[k]) {
        array.RemoveAt(k);
        j = -1;
        break;
      } else
        k++;
    }

    j++;
  }

  return array;
}

//----------------------------------------------------------------------------------------------------------
//    The PlugIn CallBack API Implementation
//    The definitions of this API are found in ocpn_plugin.h
//----------------------------------------------------------------------------------------------------------

int InsertPlugInTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpRollover,
                     wxItemKind kind, wxString shortHelp, wxString longHelp,
                     wxObject *clientData, int position, int tool_sel,
                     opencpn_plugin *pplugin) {
  if (s_ppim)
    return s_ppim->AddToolbarTool(label, bitmap, bmpRollover, kind, shortHelp,
                                  longHelp, clientData, position, tool_sel,
                                  pplugin);
  else
    return -1;
}

void RemovePlugInTool(int tool_id) {
  if (s_ppim) s_ppim->RemoveToolbarTool(tool_id);
}

void SetToolbarToolViz(int item, bool viz) {
  if (s_ppim) s_ppim->SetToolbarToolViz(item, viz);
}

void SetToolbarItemState(int item, bool toggle) {
  if (s_ppim) s_ppim->SetToolbarItemState(item, toggle);
}

void SetToolbarToolBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmpRollover) {
  if (s_ppim) s_ppim->SetToolbarItemBitmaps(item, bitmap, bmpRollover);
}

int InsertPlugInToolSVG(wxString label, wxString SVGfile,
                        wxString SVGfileRollover, wxString SVGfileToggled,
                        wxItemKind kind, wxString shortHelp, wxString longHelp,
                        wxObject *clientData, int position, int tool_sel,
                        opencpn_plugin *pplugin) {
  if (s_ppim)
    return s_ppim->AddToolbarTool(label, SVGfile, SVGfileRollover,
                                  SVGfileToggled, kind, shortHelp, longHelp,
                                  clientData, position, tool_sel, pplugin);
  else
    return -1;
}

void SetToolbarToolBitmapsSVG(int item, wxString SVGfile,
                              wxString SVGfileRollover,
                              wxString SVGfileToggled) {
  if (s_ppim)
    s_ppim->SetToolbarItemBitmaps(item, SVGfile, SVGfileRollover,
                                  SVGfileToggled);
}

int AddCanvasMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin,
                      const char *name) {
  if (s_ppim)
    return s_ppim->AddCanvasContextMenuItem(pitem, pplugin, name);
  else
    return -1;
}

void SetCanvasMenuItemViz(int item, bool viz, const char *name) {
  if (s_ppim) s_ppim->SetCanvasContextMenuItemViz(item, viz, name);
}

void SetCanvasMenuItemGrey(int item, bool grey, const char *name) {
  if (s_ppim) s_ppim->SetCanvasContextMenuItemGrey(item, grey, name);
}

void RemoveCanvasMenuItem(int item, const char *name) {
  if (s_ppim) s_ppim->RemoveCanvasContextMenuItem(item, name);
}

int AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin) {
  /* main context popup menu */
  return AddCanvasMenuItem(pitem, pplugin, "");
}

void SetCanvasContextMenuItemViz(int item, bool viz) {
  SetCanvasMenuItemViz(item, viz);
}

void SetCanvasContextMenuItemGrey(int item, bool grey) {
  SetCanvasMenuItemGrey(item, grey);
}

void RemoveCanvasContextMenuItem(int item) { RemoveCanvasMenuItem(item); }

wxFileConfig *GetOCPNConfigObject(void) {
  if (s_ppim)
    return pConfig;  // return the global application config object
  else
    return NULL;
}

wxWindow *GetOCPNCanvasWindow() {
  wxWindow *pret = NULL;
  if (s_ppim) {
    MyFrame *pFrame = s_ppim->GetParentFrame();
    pret = (wxWindow *)pFrame->GetPrimaryCanvas();
  }
  return pret;
}

void RequestRefresh(wxWindow *win) {
  if (win) win->Refresh();
}

void GetCanvasPixLL(PlugIn_ViewPort *vp, wxPoint *pp, double lat, double lon) {
  //    Make enough of an application viewport to run its method....
  ViewPort ocpn_vp;
  ocpn_vp.clat = vp->clat;
  ocpn_vp.clon = vp->clon;
  ocpn_vp.m_projection_type = vp->m_projection_type;
  ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
  ocpn_vp.skew = vp->skew;
  ocpn_vp.rotation = vp->rotation;
  ocpn_vp.pix_width = vp->pix_width;
  ocpn_vp.pix_height = vp->pix_height;

  wxPoint ret = ocpn_vp.GetPixFromLL(lat, lon);
  pp->x = ret.x;
  pp->y = ret.y;
}

void GetDoubleCanvasPixLL(PlugIn_ViewPort *vp, wxPoint2DDouble *pp, double lat,
                          double lon) {
  //    Make enough of an application viewport to run its method....
  ViewPort ocpn_vp;
  ocpn_vp.clat = vp->clat;
  ocpn_vp.clon = vp->clon;
  ocpn_vp.m_projection_type = vp->m_projection_type;
  ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
  ocpn_vp.skew = vp->skew;
  ocpn_vp.rotation = vp->rotation;
  ocpn_vp.pix_width = vp->pix_width;
  ocpn_vp.pix_height = vp->pix_height;

  *pp = ocpn_vp.GetDoublePixFromLL(lat, lon);
}

void GetCanvasLLPix(PlugIn_ViewPort *vp, wxPoint p, double *plat,
                    double *plon) {
  //    Make enough of an application viewport to run its method....
  ViewPort ocpn_vp;
  ocpn_vp.clat = vp->clat;
  ocpn_vp.clon = vp->clon;
  ocpn_vp.m_projection_type = vp->m_projection_type;
  ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
  ocpn_vp.skew = vp->skew;
  ocpn_vp.rotation = vp->rotation;
  ocpn_vp.pix_width = vp->pix_width;
  ocpn_vp.pix_height = vp->pix_height;

  return ocpn_vp.GetLLFromPix(p, plat, plon);
}

bool GetGlobalColor(wxString colorName, wxColour *pcolour) {
  wxColour c = GetGlobalColor(colorName);
  *pcolour = c;

  return true;
}

wxFont *OCPNGetFont(wxString TextElement, int default_size) {
  return FontMgr::Get().GetFont(TextElement, default_size);
}

wxFont *GetOCPNScaledFont_PlugIn(wxString TextElement, int default_size) {
  return GetOCPNScaledFont(TextElement, default_size);
}

double GetOCPNGUIToolScaleFactor_PlugIn(int GUIScaleFactor) {
  return g_Platform->GetToolbarScaleFactor(GUIScaleFactor);
}

double GetOCPNGUIToolScaleFactor_PlugIn() {
  return g_Platform->GetToolbarScaleFactor(g_GUIScaleFactor);
}

float GetOCPNChartScaleFactor_Plugin() {
  return g_Platform->getChartScaleFactorExp(g_ChartScaleFactor);
}

wxFont GetOCPNGUIScaledFont_PlugIn(wxString item) {
  return GetOCPNGUIScaledFont(item);
}

bool AddPersistentFontKey(wxString TextElement) {
  return FontMgr::Get().AddAuxKey(TextElement);
}

wxString GetActiveStyleName() {
  if (g_StyleManager)
    return g_StyleManager->GetCurrentStyle()->name;
  else
    return _T("");
}

wxBitmap GetBitmapFromSVGFile(wxString filename, unsigned int width,
                              unsigned int height) {
  return LoadSVG(filename, width, height);
}

bool IsTouchInterface_PlugIn(void) { return g_btouch; }

wxColour GetFontColour_PlugIn(wxString TextElement) {
  return FontMgr::Get().GetFontColor(TextElement);
}

wxString *GetpSharedDataLocation(void) {
  return g_Platform->GetSharedDataDirPtr();
}

wxString *GetpPrivateApplicationDataLocation(void) {
  return g_Platform->GetPrivateDataDirPtr();
}

ArrayOfPlugIn_AIS_Targets *GetAISTargetArray(void) {
  if (!g_pAIS) return NULL;

  ArrayOfPlugIn_AIS_Targets *pret = new ArrayOfPlugIn_AIS_Targets;

  //      Iterate over the AIS Target Hashmap
  for (const auto &it : g_pAIS->GetTargetList()) {
    AIS_Target_Data *td = it.second;
    PlugIn_AIS_Target *ptarget = Create_PI_AIS_Target(td);
    pret->Add(ptarget);
  }

//  Test one alarm target
#if 0
    AIS_Target_Data td;
    td.n_alarm_state = AIS_ALARM_SET;
    PlugIn_AIS_Target *ptarget = Create_PI_AIS_Target(&td);
    pret->Add(ptarget);
#endif
  return pret;
}

wxAuiManager *GetFrameAuiManager(void) { return g_pauimgr; }

bool AddLocaleCatalog(wxString catalog) {
#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)

  if (plocale_def_lang) {
    // Add this catalog to the persistent catalog array
    g_locale_catalog_array.Add(catalog);

    return plocale_def_lang->AddCatalog(catalog);
  } else
#endif
    return false;
}

void PushNMEABuffer(wxString buf) {
  OCPN_DataStreamEvent event(wxEVT_OCPN_DATASTREAM, 0);
  std::string s = std::string(buf.mb_str());
  event.SetNMEAString(s);
  event.SetStream(NULL);

  g_pMUX->AddPendingEvent(event);
}

wxXmlDocument GetChartDatabaseEntryXML(int dbIndex, bool b_getGeom) {
  wxXmlDocument doc = ChartData->GetXMLDescription(dbIndex, b_getGeom);

  return doc;
}

bool UpdateChartDBInplace(wxArrayString dir_array, bool b_force_update,
                          bool b_ProgressDialog) {
  //    Make an array of CDI
  ArrayOfCDI ChartDirArray;
  for (unsigned int i = 0; i < dir_array.GetCount(); i++) {
    wxString dirname = dir_array[i];
    ChartDirInfo cdi;
    cdi.fullpath = dirname;
    cdi.magic_number = _T("");
    ChartDirArray.Add(cdi);
  }

  bool b_ret = gFrame->UpdateChartDatabaseInplace(ChartDirArray, b_force_update,
                                                  b_ProgressDialog,
                                                  ChartData->GetDBFileName());

  gFrame->ChartsRefresh();

  return b_ret;
}

wxArrayString GetChartDBDirArrayString() {
  return ChartData->GetChartDirArrayString();
}

int AddChartToDBInPlace(wxString &full_path, bool b_RefreshCanvas) {
  // extract the path from the chart name
  wxFileName fn(full_path);
  wxString fdir = fn.GetPath();

  bool bret = false;
  if (ChartData) {
    bret = ChartData->AddSingleChart(full_path);

    if (bret) {
      // Save to disk
      pConfig->UpdateChartDirs(ChartData->GetChartDirArray());
      ChartData->SaveBinary(ChartListFileName);

      //  Completely reload the chart database, for a fresh start
      ArrayOfCDI XnewChartDirArray;
      pConfig->LoadChartDirArray(XnewChartDirArray);
      delete ChartData;
      ChartData = new ChartDB();
      ChartData->LoadBinary(ChartListFileName, XnewChartDirArray);

      // Update group contents
      if (g_pGroupArray) ChartData->ApplyGroupArray(g_pGroupArray);

      if (g_boptionsactive) {
        g_options->UpdateDisplayedChartDirList(ChartData->GetChartDirArray());
      }

      if (b_RefreshCanvas || !gFrame->GetPrimaryCanvas()->GetQuiltMode()) {
        gFrame->ChartsRefresh();
      }
    }
  }
  return bret;
}

int RemoveChartFromDBInPlace(wxString &full_path) {
  bool bret = false;
  if (ChartData) {
    bret = ChartData->RemoveSingleChart(full_path);

    // Save to disk
    pConfig->UpdateChartDirs(ChartData->GetChartDirArray());
    ChartData->SaveBinary(ChartListFileName);

    //  Completely reload the chart database, for a fresh start
    ArrayOfCDI XnewChartDirArray;
    pConfig->LoadChartDirArray(XnewChartDirArray);
    delete ChartData;
    ChartData = new ChartDB();
    ChartData->LoadBinary(ChartListFileName, XnewChartDirArray);

    // Update group contents
    if (g_pGroupArray) ChartData->ApplyGroupArray(g_pGroupArray);

    if (g_boptionsactive) {
      g_options->UpdateDisplayedChartDirList(ChartData->GetChartDirArray());
    }

    gFrame->ChartsRefresh();
  }

  return bret;
}

wxString GetLocaleCanonicalName() { return g_locale; }

void SendPluginMessage(wxString message_id, wxString message_body) {
  s_ppim->SendMessageToAllPlugins(message_id, message_body);

  //  We will send an event to the main application frame (gFrame)
  //  for informational purposes.
  //  Of course, gFrame is encouraged to use any or all the
  //  data flying by if judged useful and dependable....

  OCPN_MsgEvent Nevent(wxEVT_OCPN_MSG, 0);
  Nevent.SetID(message_id);
  Nevent.SetJSONText(message_body);
  gFrame->GetEventHandler()->AddPendingEvent(Nevent);
}

void DimeWindow(wxWindow *win) { DimeControl(win); }

void JumpToPosition(double lat, double lon, double scale) {
  gFrame->JumpToPosition(gFrame->GetFocusCanvas(), lat, lon, scale);
}

/* API 1.9 */
wxScrolledWindow *AddOptionsPage(OptionsParentPI parent, wxString title) {
  if (!g_pOptions) return NULL;

  size_t parentid;
  switch (parent) {
    case PI_OPTIONS_PARENT_DISPLAY:
      parentid = g_pOptions->m_pageDisplay;
      break;
    case PI_OPTIONS_PARENT_CONNECTIONS:
      parentid = g_pOptions->m_pageConnections;
      break;
    case PI_OPTIONS_PARENT_CHARTS:
      parentid = g_pOptions->m_pageCharts;
      break;
    case PI_OPTIONS_PARENT_SHIPS:
      parentid = g_pOptions->m_pageShips;
      break;
    case PI_OPTIONS_PARENT_UI:
      parentid = g_pOptions->m_pageUI;
      break;
    case PI_OPTIONS_PARENT_PLUGINS:
      parentid = g_pOptions->m_pagePlugins;
      break;
    default:
      wxLogMessage(
          _T("Error in PluginManager::AddOptionsPage: Unknown parent"));
      return NULL;
      break;
  }

  return g_pOptions->AddPage(parentid, title);
}

bool DeleteOptionsPage(wxScrolledWindow *page) {
  if (!g_pOptions) return false;
  return g_pOptions->DeletePluginPage(page);
}

bool DecodeSingleVDOMessage(const wxString &str, PlugIn_Position_Fix_Ex *pos,
                            wxString *accumulator) {
  if (!pos) return false;

  GenericPosDatEx gpd;
  AIS_Error nerr = AIS_GENERIC_ERROR;
  if (g_pAIS) nerr = g_pAIS->DecodeSingleVDO(str, &gpd, accumulator);
  if (nerr == AIS_NoError) {
    pos->Lat = gpd.kLat;
    pos->Lon = gpd.kLon;
    pos->Cog = gpd.kCog;
    pos->Sog = gpd.kSog;
    pos->Hdt = gpd.kHdt;

    //  Fill in the dummy values
    pos->FixTime = 0;
    pos->Hdm = 1000;
    pos->Var = 1000;
    pos->nSats = 0;

    return true;
  }

  return false;
}

int GetChartbarHeight(void) {
  int val = 0;
  if (g_bShowChartBar) {
    ChartCanvas *cc = gFrame->GetPrimaryCanvas();
    if (cc && cc->GetPiano()) {
      val = cc->GetPiano()->GetHeight();
    }
  }
  return val;
}

bool GetRoutepointGPX(RoutePoint *pRoutePoint, char *buffer,
                      unsigned int buffer_length) {
  bool ret = false;

  NavObjectCollection1 *pgpx = new NavObjectCollection1;
  pgpx->AddGPXWaypoint(pRoutePoint);
  wxString gpxfilename = wxFileName::CreateTempFileName(wxT("gpx"));
  pgpx->SaveFile(gpxfilename);
  delete pgpx;

  wxFFile gpxfile(gpxfilename);
  wxString s;
  if (gpxfile.ReadAll(&s)) {
    if (s.Length() < buffer_length) {
      strncpy(buffer, (const char *)s.mb_str(wxConvUTF8), buffer_length - 1);
      ret = true;
    }
  }

  gpxfile.Close();
  ::wxRemoveFile(gpxfilename);

  return ret;
}

bool GetActiveRoutepointGPX(char *buffer, unsigned int buffer_length) {
  if (g_pRouteMan->IsAnyRouteActive())
    return GetRoutepointGPX(g_pRouteMan->GetpActivePoint(), buffer,
                            buffer_length);
  else
    return false;
}

void PositionBearingDistanceMercator_Plugin(double lat, double lon, double brg,
                                            double dist, double *dlat,
                                            double *dlon) {
  PositionBearingDistanceMercator(lat, lon, brg, dist, dlat, dlon);
}

void DistanceBearingMercator_Plugin(double lat0, double lon0, double lat1,
                                    double lon1, double *brg, double *dist) {
  DistanceBearingMercator(lat0, lon0, lat1, lon1, brg, dist);
}

double DistGreatCircle_Plugin(double slat, double slon, double dlat,
                              double dlon) {
  return DistGreatCircle(slat, slon, dlat, dlon);
}

void toTM_Plugin(float lat, float lon, float lat0, float lon0, double *x,
                 double *y) {
  toTM(lat, lon, lat0, lon0, x, y);
}

void fromTM_Plugin(double x, double y, double lat0, double lon0, double *lat,
                   double *lon) {
  fromTM(x, y, lat0, lon0, lat, lon);
}

void toSM_Plugin(double lat, double lon, double lat0, double lon0, double *x,
                 double *y) {
  toSM(lat, lon, lat0, lon0, x, y);
}

void fromSM_Plugin(double x, double y, double lat0, double lon0, double *lat,
                   double *lon) {
  fromSM(x, y, lat0, lon0, lat, lon);
}

void toSM_ECC_Plugin(double lat, double lon, double lat0, double lon0,
                     double *x, double *y) {
  toSM_ECC(lat, lon, lat0, lon0, x, y);
}

void fromSM_ECC_Plugin(double x, double y, double lat0, double lon0,
                       double *lat, double *lon) {
  fromSM_ECC(x, y, lat0, lon0, lat, lon);
}

double toUsrDistance_Plugin(double nm_distance, int unit) {
  return toUsrDistance(nm_distance, unit);
}

double fromUsrDistance_Plugin(double usr_distance, int unit) {
  return fromUsrDistance(usr_distance, unit);
}

double toUsrSpeed_Plugin(double kts_speed, int unit) {
  return toUsrSpeed(kts_speed, unit);
}

double fromUsrSpeed_Plugin(double usr_speed, int unit) {
  return fromUsrSpeed(usr_speed, unit);
}

double toUsrTemp_Plugin(double cel_temp, int unit) {
  return toUsrTemp(cel_temp, unit);
}

double fromUsrTemp_Plugin(double usr_temp, int unit) {
  return fromUsrTemp(usr_temp, unit);
}

wxString getUsrDistanceUnit_Plugin(int unit) {
  return getUsrDistanceUnit(unit);
}

wxString getUsrSpeedUnit_Plugin(int unit) { return getUsrSpeedUnit(unit); }

wxString getUsrTempUnit_Plugin(int unit) { return getUsrTempUnit(unit); }

bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1, double lat2,
                              double lon2) {
  static bool loaded = false;
  if (!loaded) {
    gshhsCrossesLandInit();
    loaded = true;
  }

  return gshhsCrossesLand(lat1, lon1, lat2, lon2);
}

void PlugInPlaySound(wxString &sound_file) {
  PlugInPlaySoundEx(sound_file, -1);
}

// API 1.10 Route and Waypoint Support
// wxBitmap *FindSystemWaypointIcon( wxString& icon_name );

//      PlugInWaypoint implementation
PlugIn_Waypoint::PlugIn_Waypoint() { m_HyperlinkList = NULL; }

PlugIn_Waypoint::PlugIn_Waypoint(double lat, double lon,
                                 const wxString &icon_ident,
                                 const wxString &wp_name,
                                 const wxString &GUID) {
  wxDateTime now = wxDateTime::Now();
  m_CreateTime = now.ToUTC();
  m_HyperlinkList = NULL;

  m_lat = lat;
  m_lon = lon;
  m_IconName = icon_ident;
  m_MarkName = wp_name;
  m_GUID = GUID;
}

PlugIn_Waypoint::~PlugIn_Waypoint() {}

//      PlugInRoute implementation
PlugIn_Route::PlugIn_Route(void) { pWaypointList = new Plugin_WaypointList; }

PlugIn_Route::~PlugIn_Route(void) {
  pWaypointList->DeleteContents(false);  // do not delete Waypoints
  pWaypointList->Clear();

  delete pWaypointList;
}

//      PlugInTrack implementation
PlugIn_Track::PlugIn_Track(void) { pWaypointList = new Plugin_WaypointList; }

PlugIn_Track::~PlugIn_Track(void) {
  pWaypointList->DeleteContents(false);  // do not delete Waypoints
  pWaypointList->Clear();

  delete pWaypointList;
}

wxString GetNewGUID(void) { return GpxDocument::GetUUID(); }

bool AddCustomWaypointIcon(wxBitmap *pimage, wxString key,
                           wxString description) {
  pWayPointMan->ProcessIcon(*pimage, key, description);
  return true;
}

static void cloneHyperlinkList(RoutePoint *dst, const PlugIn_Waypoint *src) {
  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList == nullptr) return;

  if (src->m_HyperlinkList->GetCount() > 0) {
    wxPlugin_HyperlinkListNode *linknode = src->m_HyperlinkList->GetFirst();
    while (linknode) {
      Plugin_Hyperlink *link = linknode->GetData();

      Hyperlink *h = new Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->LType = link->Type;

      dst->m_HyperlinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }
}

bool AddSingleWaypoint(PlugIn_Waypoint *pwaypoint, bool b_permanent) {
  //  Validate the waypoint parameters a little bit

  //  GUID
  //  Make sure that this GUID is indeed unique in the Routepoint list
  bool b_unique = true;
  wxRoutePointListNode *prpnode = pWayPointMan->GetWaypointList()->GetFirst();
  while (prpnode) {
    RoutePoint *prp = prpnode->GetData();

    if (prp->m_GUID == pwaypoint->m_GUID) {
      b_unique = false;
      break;
    }
    prpnode = prpnode->GetNext();  // RoutePoint
  }

  if (!b_unique) return false;

  RoutePoint *pWP =
      new RoutePoint(pwaypoint->m_lat, pwaypoint->m_lon, pwaypoint->m_IconName,
                     pwaypoint->m_MarkName, pwaypoint->m_GUID);

  pWP->m_bIsolatedMark = true;  // This is an isolated mark

  cloneHyperlinkList(pWP, pwaypoint);

  pWP->m_MarkDescription = pwaypoint->m_MarkDescription;

  if (pwaypoint->m_CreateTime.IsValid())
    pWP->SetCreateTime(pwaypoint->m_CreateTime);
  else {
    wxDateTime dtnow(wxDateTime::Now());
    pWP->SetCreateTime(dtnow);
  }

  pWP->m_btemp = (b_permanent == false);

  pSelect->AddSelectableRoutePoint(pwaypoint->m_lat, pwaypoint->m_lon, pWP);
  if (b_permanent) pConfig->AddNewWayPoint(pWP, -1);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();

  return true;
}

bool DeleteSingleWaypoint(wxString &GUID) {
  //  Find the RoutePoint
  bool b_found = false;
  RoutePoint *prp = pWayPointMan->FindRoutePointByGUID(GUID);

  if (prp) b_found = true;

  if (b_found) {
    pWayPointMan->DestroyWaypoint(prp);
    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateWptListCtrl();
  }

  return b_found;
}

bool UpdateSingleWaypoint(PlugIn_Waypoint *pwaypoint) {
  //  Find the RoutePoint
  bool b_found = false;
  RoutePoint *prp = pWayPointMan->FindRoutePointByGUID(pwaypoint->m_GUID);

  if (prp) b_found = true;

  if (b_found) {
    double lat_save = prp->m_lat;
    double lon_save = prp->m_lon;

    prp->m_lat = pwaypoint->m_lat;
    prp->m_lon = pwaypoint->m_lon;
    prp->SetIconName(pwaypoint->m_IconName);
    prp->SetName(pwaypoint->m_MarkName);
    prp->m_MarkDescription = pwaypoint->m_MarkDescription;
    prp->SetVisible(pwaypoint->m_IsVisible);
    if (pwaypoint->m_CreateTime.IsValid())
      prp->SetCreateTime(pwaypoint->m_CreateTime);

    //  Transcribe (clone) the html HyperLink List, if present

    if (pwaypoint->m_HyperlinkList) {
      prp->m_HyperlinkList->Clear();
      if (pwaypoint->m_HyperlinkList->GetCount() > 0) {
        wxPlugin_HyperlinkListNode *linknode =
            pwaypoint->m_HyperlinkList->GetFirst();
        while (linknode) {
          Plugin_Hyperlink *link = linknode->GetData();

          Hyperlink *h = new Hyperlink();
          h->DescrText = link->DescrText;
          h->Link = link->Link;
          h->LType = link->Type;

          prp->m_HyperlinkList->Append(h);

          linknode = linknode->GetNext();
        }
      }
    }

    if (prp) prp->ReLoadIcon();

    SelectItem *pFind = pSelect->FindSelection(
        gFrame->GetPrimaryCanvas(), lat_save, lon_save, SELTYPE_ROUTEPOINT);
    if (pFind) {
      pFind->m_slat = pwaypoint->m_lat;  // update the SelectList entry
      pFind->m_slon = pwaypoint->m_lon;
    }

    if (!prp->m_btemp) pConfig->UpdateWayPoint(prp);

    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateWptListCtrl();
  }

  return b_found;
}

// translate O route class to Plugin one
static void PlugInFromRoutePoint(PlugIn_Waypoint *dst,
                                 /* const*/ RoutePoint *src) {
  dst->m_lat = src->m_lat;
  dst->m_lon = src->m_lon;
  dst->m_IconName = src->GetIconName();
  dst->m_MarkName = src->GetName();
  dst->m_MarkDescription = src->m_MarkDescription;
  dst->m_IsVisible = src->IsVisible();
  dst->m_CreateTime = src->GetCreateTime();  // not const
  dst->m_GUID = src->m_GUID;

  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList == nullptr) return;

  delete dst->m_HyperlinkList;
  dst->m_HyperlinkList = nullptr;

  if (src->m_HyperlinkList->GetCount() > 0) {
    dst->m_HyperlinkList = new Plugin_HyperlinkList;

    wxHyperlinkListNode *linknode = src->m_HyperlinkList->GetFirst();
    while (linknode) {
      Hyperlink *link = linknode->GetData();

      Plugin_Hyperlink *h = new Plugin_Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->Type = link->LType;

      dst->m_HyperlinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }
}

bool GetSingleWaypoint(wxString GUID, PlugIn_Waypoint *pwaypoint) {
  //  Find the RoutePoint
  RoutePoint *prp = pWayPointMan->FindRoutePointByGUID(GUID);

  if (!prp) return false;

  PlugInFromRoutePoint(pwaypoint, prp);

  return true;
}

wxArrayString GetWaypointGUIDArray(void) {
  wxArrayString result;
  RoutePointList *list = pWayPointMan->GetWaypointList();

  wxRoutePointListNode *prpnode = list->GetFirst();
  while (prpnode) {
    RoutePoint *prp = prpnode->GetData();
    result.Add(prp->m_GUID);

    prpnode = prpnode->GetNext();  // RoutePoint
  }

  return result;
}

wxArrayString GetRouteGUIDArray(void) {
  wxArrayString result;
  RouteList *list = pRouteList;

  wxRouteListNode *prpnode = list->GetFirst();
  while (prpnode) {
    Route *proute = prpnode->GetData();
    result.Add(proute->m_GUID);

    prpnode = prpnode->GetNext();  // Route
  }

  return result;
}

wxArrayString GetTrackGUIDArray(void) {
  wxArrayString result;
  for (Track *ptrack : g_TrackList) {
    result.Add(ptrack->m_GUID);
  }

  return result;
}

wxArrayString GetIconNameArray(void) {
  wxArrayString result;

  for (int i = 0; i < pWayPointMan->GetNumIcons(); i++) {
    wxString *ps = pWayPointMan->GetIconKey(i);
    result.Add(*ps);
  }
  return result;
}

bool AddPlugInRoute(PlugIn_Route *proute, bool b_permanent) {
  Route *route = new Route();

  PlugIn_Waypoint *pwp;
  RoutePoint *pWP_src;
  int ip = 0;
  wxDateTime plannedDeparture;

  wxPlugin_WaypointListNode *pwpnode = proute->pWaypointList->GetFirst();
  while (pwpnode) {
    pwp = pwpnode->GetData();

    RoutePoint *pWP = new RoutePoint(pwp->m_lat, pwp->m_lon, pwp->m_IconName,
                                     pwp->m_MarkName, pwp->m_GUID);

    //  Transcribe (clone) the html HyperLink List, if present
    cloneHyperlinkList(pWP, pwp);
    pWP->m_MarkDescription = pwp->m_MarkDescription;
    pWP->m_bShowName = false;
    pWP->SetCreateTime(pwp->m_CreateTime);

    route->AddPoint(pWP);

    pSelect->AddSelectableRoutePoint(pWP->m_lat, pWP->m_lon, pWP);

    if (ip > 0)
      pSelect->AddSelectableRouteSegment(pWP_src->m_lat, pWP_src->m_lon,
                                         pWP->m_lat, pWP->m_lon, pWP_src, pWP,
                                         route);
    else
      plannedDeparture = pwp->m_CreateTime;
    ip++;
    pWP_src = pWP;

    pwpnode = pwpnode->GetNext();  // PlugInWaypoint
  }

  route->m_PlannedDeparture = plannedDeparture;

  route->m_RouteNameString = proute->m_NameString;
  route->m_RouteStartString = proute->m_StartString;
  route->m_RouteEndString = proute->m_EndString;
  if (!proute->m_GUID.IsEmpty()) {
    route->m_GUID = proute->m_GUID;
  }
  route->m_btemp = (b_permanent == false);

  pRouteList->Append(route);

  if (b_permanent) pConfig->AddNewRoute(route);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateRouteListCtrl();

  return true;
}

bool DeletePlugInRoute(wxString &GUID) {
  bool b_found = false;

  //  Find the Route
  Route *pRoute = g_pRouteMan->FindRouteByGUID(GUID);
  if (pRoute) {
    g_pRouteMan->DeleteRoute(pRoute);
    b_found = true;
  }
  return b_found;
}

bool UpdatePlugInRoute(PlugIn_Route *proute) {
  bool b_found = false;

  //  Find the Route
  Route *pRoute = g_pRouteMan->FindRouteByGUID(proute->m_GUID);
  if (pRoute) b_found = true;

  if (b_found) {
    bool b_permanent = (pRoute->m_btemp == false);
    g_pRouteMan->DeleteRoute(pRoute);

    b_found = AddPlugInRoute(proute, b_permanent);
  }

  return b_found;
}

bool AddPlugInTrack(PlugIn_Track *ptrack, bool b_permanent) {
  Track *track = new Track();

  PlugIn_Waypoint *pwp = 0;
  TrackPoint *pWP_src = 0;
  int ip = 0;

  wxPlugin_WaypointListNode *pwpnode = ptrack->pWaypointList->GetFirst();
  while (pwpnode) {
    pwp = pwpnode->GetData();

    TrackPoint *pWP = new TrackPoint(pwp->m_lat, pwp->m_lon);
    pWP->SetCreateTime(pwp->m_CreateTime);

    track->AddPoint(pWP);

    if (ip > 0)
      pSelect->AddSelectableTrackSegment(pWP_src->m_lat, pWP_src->m_lon,
                                         pWP->m_lat, pWP->m_lon, pWP_src, pWP,
                                         track);
    ip++;
    pWP_src = pWP;

    pwpnode = pwpnode->GetNext();  // PlugInWaypoint
  }

  track->SetName(ptrack->m_NameString);
  track->m_TrackStartString = ptrack->m_StartString;
  track->m_TrackEndString = ptrack->m_EndString;
  track->m_GUID = ptrack->m_GUID;
  track->m_btemp = (b_permanent == false);

  g_TrackList.push_back(track);

  if (b_permanent) pConfig->AddNewTrack(track);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateTrkListCtrl();

  return true;
}

bool DeletePlugInTrack(wxString &GUID) {
  bool b_found = false;

  //  Find the Route
  Track *pTrack = g_pRouteMan->FindTrackByGUID(GUID);
  if (pTrack) {
    g_pRouteMan->DeleteTrack(pTrack);
    b_found = true;
  }

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateTrkListCtrl();

  return b_found;
}

bool UpdatePlugInTrack(PlugIn_Track *ptrack) {
  bool b_found = false;

  //  Find the Track
  Track *pTrack = g_pRouteMan->FindTrackByGUID(ptrack->m_GUID);
  if (pTrack) b_found = true;

  if (b_found) {
    bool b_permanent = (pTrack->m_btemp == false);
    g_pRouteMan->DeleteTrack(pTrack);

    b_found = AddPlugInTrack(ptrack, b_permanent);
  }

  return b_found;
}

bool PlugInHasNormalizedViewPort(PlugIn_ViewPort *vp) {
#ifdef ocpnUSE_GL
  ViewPort ocpn_vp;
  ocpn_vp.m_projection_type = vp->m_projection_type;

  return glChartCanvas::HasNormalizedViewPort(ocpn_vp);
#else
  return false;
#endif
}

void PlugInMultMatrixViewport(PlugIn_ViewPort *vp, float lat, float lon) {
#ifdef ocpnUSE_GL
  ViewPort ocpn_vp;
  ocpn_vp.clat = vp->clat;
  ocpn_vp.clon = vp->clon;
  ocpn_vp.m_projection_type = vp->m_projection_type;
  ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
  ocpn_vp.skew = vp->skew;
  ocpn_vp.rotation = vp->rotation;
  ocpn_vp.pix_width = vp->pix_width;
  ocpn_vp.pix_height = vp->pix_height;

// TODO fix for multicanvas    glChartCanvas::MultMatrixViewPort(ocpn_vp, lat,
// lon);
#endif
}

void PlugInNormalizeViewport(PlugIn_ViewPort *vp, float lat, float lon) {
#ifdef ocpnUSE_GL
  ViewPort ocpn_vp;
  glChartCanvas::NormalizedViewPort(ocpn_vp, lat, lon);

  vp->clat = ocpn_vp.clat;
  vp->clon = ocpn_vp.clon;
  vp->view_scale_ppm = ocpn_vp.view_scale_ppm;
  vp->rotation = ocpn_vp.rotation;
  vp->skew = ocpn_vp.skew;
#endif
}

//          Helper and interface classes

//-------------------------------------------------------------------------------
//    PlugIn_AIS_Target Implementation
//-------------------------------------------------------------------------------

PlugIn_AIS_Target *Create_PI_AIS_Target(AIS_Target_Data *ptarget) {
  PlugIn_AIS_Target *pret = new PlugIn_AIS_Target;

  pret->MMSI = ptarget->MMSI;
  pret->Class = ptarget->Class;
  pret->NavStatus = ptarget->NavStatus;
  pret->SOG = ptarget->SOG;
  pret->COG = ptarget->COG;
  pret->HDG = ptarget->HDG;
  pret->Lon = ptarget->Lon;
  pret->Lat = ptarget->Lat;
  pret->ROTAIS = ptarget->ROTAIS;
  pret->ShipType = ptarget->ShipType;
  pret->IMO = ptarget->IMO;

  pret->Range_NM = ptarget->Range_NM;
  pret->Brg = ptarget->Brg;

  //      Per target collision parameters
  pret->bCPA_Valid = ptarget->bCPA_Valid;
  pret->TCPA = ptarget->TCPA;  // Minutes
  pret->CPA = ptarget->CPA;    // Nautical Miles

  pret->alarm_state = (plugin_ais_alarm_type)ptarget->n_alert_state;

  memcpy(pret->CallSign, ptarget->CallSign, CALL_SIGN_LEN);
  memcpy(pret->ShipName, ptarget->ShipName, SHIP_NAME_LEN);

  return pret;
}

//-------------------------------------------------------------------------------
//    PluginListPanel & PluginPanel Implementation
//-------------------------------------------------------------------------------

#define DISABLED_SETTINGS_MSG                                               \
  _("These settings might destabilize OpenCPN and are by default disabled." \
    " To despite the dangers enable them manually add a CatalogExpert=1"    \
    " line in the [PlugIns] section in the configuration file.")

/*
 * Panel with buttons to control plugin catalog management.
 */
CatalogMgrPanel::CatalogMgrPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize),
      m_parent(parent) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(topSizer);

  topSizer->Add(new wxStaticLine(this), 0, wxGROW | wxLEFT | wxRIGHT, 4);

  wxStaticBox *itemStaticBoxSizer4Static =
      new wxStaticBox(this, wxID_ANY, _("Plugin Catalog"));
  wxStaticBoxSizer *itemStaticBoxSizer4 =
      new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
  topSizer->Add(itemStaticBoxSizer4, 1, wxEXPAND | wxALL, 2);

#ifndef __OCPN__ANDROID__
  // First line
  m_catalogText = new wxStaticText(this, wxID_STATIC, _T(""));
  itemStaticBoxSizer4->Add(m_catalogText,
                           wxSizerFlags().Border().Proportion(1));
  m_catalogText->SetLabel(GetCatalogText(false));

  // Next line
  wxBoxSizer *rowSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer4->Add(rowSizer2,
                           wxSizerFlags().Expand().Border().Proportion(1));

  m_updateButton = new wxButton(this, wxID_ANY, _("Update Plugin Catalog"),
                                wxDefaultPosition, wxDefaultSize, 0);
  rowSizer2->Add(m_updateButton, 0, wxALIGN_LEFT);
  m_updateButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                       &CatalogMgrPanel::OnUpdateButton, this);
  rowSizer2->AddSpacer(4 * GetCharWidth());
  m_tarballButton = new wxButton(this, wxID_ANY, _("Import plugin..."),
                                 wxDefaultPosition, wxDefaultSize, 0);
  rowSizer2->Add(m_tarballButton, 0, wxALIGN_LEFT | wxLEFT, 2 * GetCharWidth());
  m_tarballButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                        &CatalogMgrPanel::OnTarballButton, this);

  rowSizer2->AddSpacer(4 * GetCharWidth());
  m_adv_button = new wxButton(this, wxID_ANY, _("Settings..."),
                              wxDefaultPosition, wxDefaultSize, 0);
  ConfigVar<bool> expert("/PlugIns", "CatalogExpert", pConfig);
  if (expert.get(false)) {
    m_adv_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                       &CatalogMgrPanel::OnPluginSettingsButton, this);
  } else {
    m_adv_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent &) {
      wxMessageBox(DISABLED_SETTINGS_MSG, _("Disabled"));
    });
  }
  rowSizer2->AddSpacer(4 * GetCharWidth());
  rowSizer2->Add(m_adv_button, 0, wxALIGN_LEFT);

  SetUpdateButtonLabel();

  // Next line
  wxBoxSizer *rowSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer4->Add(rowSizer3, 0, wxEXPAND | wxALL, 4);

  SetMinSize(wxSize(m_parent->GetClientSize().x - (4 * GetCharWidth()), -1));
  Fit();

  GlobalVar<wxString> catalog(&g_catalog_channel);
  wxDEFINE_EVENT(EVT_CATALOG_CHANGE, wxCommandEvent);
  catalog_listener = catalog.get_listener(this, EVT_CATALOG_CHANGE);
  Bind(EVT_CATALOG_CHANGE, [&](wxCommandEvent &) { SetUpdateButtonLabel(); });

#else  // Android
  SetBackgroundColour(wxColour(0x7c, 0xb0, 0xe9));  // light blue
  ConfigVar<bool> expert("/PlugIns", "CatalogExpert", pConfig);
  if (!expert.get(false)) {
    m_updateButton =
        new wxButton(this, wxID_ANY, _("Update Plugin Catalog: master"),
                     wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer4->Add(m_updateButton, 0, wxALIGN_LEFT);
    m_updateButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                         &CatalogMgrPanel::OnUpdateButton, this);
    SetUpdateButtonLabel();
    m_tarballButton = NULL;
    m_adv_button = NULL;
  } else {
    // First line
    m_catalogText = new wxStaticText(this, wxID_STATIC, _T(""));
    itemStaticBoxSizer4->Add(m_catalogText,
                             wxSizerFlags().Border().Proportion(1));
    m_catalogText->SetLabel(GetCatalogText(false));

    m_updateButton =
        new wxButton(this, wxID_ANY, _("Update Plugin Catalog:master"),
                     wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer4->Add(m_updateButton, 0, wxALIGN_LEFT);
    m_updateButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                         &CatalogMgrPanel::OnUpdateButton, this);
    SetUpdateButtonLabel();

    // Next line
    m_adv_button = new wxButton(this, wxID_ANY, _("Settings..."),
                                wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer4->Add(m_adv_button, 0, wxALIGN_LEFT);
    m_adv_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                       &CatalogMgrPanel::OnPluginSettingsButton, this);

    // Next line
    m_tarballButton = new wxButton(this, wxID_ANY, _("Import plugin..."),
                                   wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer4->Add(m_tarballButton, 0, wxALIGN_LEFT | wxLEFT,
                             2 * GetCharWidth());
    m_tarballButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                          &CatalogMgrPanel::OnTarballButton, this);
  }

#endif
}

CatalogMgrPanel::~CatalogMgrPanel() {
  m_updateButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,
                         &CatalogMgrPanel::OnUpdateButton, this);
  if (m_tarballButton)
    m_tarballButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,
                            &CatalogMgrPanel::OnTarballButton, this);
}

static const char *const DOWNLOAD_REPO_PROTO =
    "https://raw.githubusercontent.com/OpenCPN/plugins/@branch@/"
    "ocpn-plugins.xml";

void CatalogMgrPanel::OnUpdateButton(wxCommandEvent &event) {
  // Craft the url
  std::string catalog(g_catalog_channel == "" ? "master" : g_catalog_channel);
  std::string url(g_catalog_custom_url);
  if (catalog != "custom") {
    url = std::string(DOWNLOAD_REPO_PROTO);
    ocpn::replace(url, "@branch@", catalog);
  }
  // Download to a temp file
  std::string filePath =
      wxFileName::CreateTempFileName("ocpn_dl").ToStdString();

  auto catalogHdlr = CatalogHandler::getInstance();

  g_Platform->ShowBusySpinner();
  auto status = catalogHdlr->DownloadCatalog(filePath, url);
  g_Platform->HideBusySpinner();

  std::string message;
  if (status != CatalogHandler::ServerStatus::OK) {
    message = _("Cannot download data from url");
    OCPNMessageBox(this, message, _("OpenCPN Catalog update"),
                   wxICON_ERROR | wxOK);
    return;
  }

  // TODO Validate xml using xsd here....
#ifdef __OCPN__ANDROID__
  if (!AndroidSecureCopyFile(wxString(filePath.c_str()),
                             g_Platform->GetPrivateDataDir() +
                                 wxFileName::GetPathSeparator() +
                                 _T("ocpn-plugins.xml"))) {
    OCPNMessageBox(this, _("Unable to copy catalog file"),
                   _("OpenCPN Catalog update"), wxICON_ERROR | wxOK);
    return;
  }
#else
  // Copy the downloaded file to proper local location
  if (!wxCopyFile(wxString(filePath.c_str()),
                  g_Platform->GetPrivateDataDir() +
                      wxFileName::GetPathSeparator() +
                      _T("ocpn-plugins.xml"))) {
    OCPNMessageBox(this, _("Unable to copy catalog file"),
                   _("OpenCPN Catalog update"), wxICON_ERROR | wxOK);
    return;
  }
#endif

  // If this is the "master" catalog, also copy to plugin cache
  if (catalog == "master") {
    if (!ocpn::store_metadata(filePath.c_str())) {
      OCPNMessageBox(this, _("Unable to copy catalog file to cache"),
                     _("OpenCPN Catalog update"), wxICON_ERROR | wxOK);
      return;
    }
  }

  // Record in the config file the name of the catalog downloaded
  pConfig->SetPath(_T("/PlugIns/"));
  pConfig->Write("LatestCatalogDownloaded", catalog.c_str());
  pConfig->Flush();

  // Reset the PluginHandler catalog file source.
  // This will case the Handler to find, load, and parse the just-downloaded
  // catalog as copied to g_Platform->GetPrivateDataDir()...
  auto pluginHandler = PluginHandler::getInstance();
  pluginHandler->setMetadata("");

  //  Reload all plugins, which will also update the status fields
  LoadAllPlugIns(false);

  // Update this Panel, and the entire list.
#ifndef __OCPN__ANDROID__
  m_catalogText->SetLabel(GetCatalogText(true));
#endif
  if (m_PluginListPanel)
    m_PluginListPanel->ReloadPluginPanels();
  OCPNMessageBox(this, _("Catalog update successful"),
                 _("OpenCPN Catalog update"), wxICON_INFORMATION | wxOK);
}

static bool parsePluginNode(pugi::xml_node &pluginRoot,
                            PluginMetadata &plugin) {
  for (pugi::xml_node element = pluginRoot.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "name")) {
      plugin.name = element.first_child().value();
    } else if (!strcmp(element.name(), "version")) {
      plugin.version = element.first_child().value();
    } else if (!strcmp(element.name(), "release")) {
      plugin.release = element.first_child().value();
    } else if (!strcmp(element.name(), "summary")) {
      plugin.summary = element.first_child().value();
    } else if (!strcmp(element.name(), "api-version")) {
      plugin.api_version = element.first_child().value();
    } else if (!strcmp(element.name(), "open-source")) {
      plugin.openSource = element.first_child().value();
    } else if (!strcmp(element.name(), "author")) {
      plugin.author = element.first_child().value();
    } else if (!strcmp(element.name(), "source")) {
      plugin.source = element.first_child().value();
    } else if (!strcmp(element.name(), "info-url")) {
      plugin.info_url = element.first_child().value();
    } else if (!strcmp(element.name(), "description")) {
      plugin.description = element.first_child().value();
    } else if (!strcmp(element.name(), "target")) {
      plugin.target = element.first_child().value();
    } else if (!strcmp(element.name(), "target-version")) {
      plugin.target_version = element.first_child().value();
    } else if (!strcmp(element.name(), "target-arch")) {
      plugin.target_arch = element.first_child().value();
    } else if (!strcmp(element.name(), "tarball-url")) {
      plugin.tarball_url = element.first_child().value();
    } else if (!strcmp(element.name(), "build-gtk")) {
      plugin.build_gtk = element.first_child().value();
    }
  }

  return true;
}

static void populatePluginNode(pugi::xml_node &pluginNode,
                               PluginMetadata &workingMetadata) {
  pugi::xml_node child;

  child = pluginNode.append_child("name");
  child.append_child(pugi::node_pcdata).set_value(workingMetadata.name.c_str());

  child = pluginNode.append_child("version");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.version.c_str());

  child = pluginNode.append_child("release");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.release.c_str());

  child = pluginNode.append_child("summary");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.summary.c_str());

  child = pluginNode.append_child("api-version");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.api_version.c_str());

  child = pluginNode.append_child("open-source");
  char b[2];
  sprintf(b, "%1d", workingMetadata.openSource);
  child.append_child(pugi::node_pcdata).set_value(b);

  child = pluginNode.append_child("author");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.author.c_str());

  child = pluginNode.append_child("source");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.source.c_str());

  child = pluginNode.append_child("info-url");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.info_url.c_str());

  child = pluginNode.append_child("description");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.description.c_str());

  child = pluginNode.append_child("target");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.target.c_str());

  child = pluginNode.append_child("target-version");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.target_version.c_str());

  child = pluginNode.append_child("target-arch");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.target_arch.c_str());

  child = pluginNode.append_child("tarball-url");
  child.append_child(pugi::node_pcdata)
      .set_value(workingMetadata.tarball_url.c_str());
}

void CatalogMgrPanel::OnPluginSettingsButton(wxCommandEvent &event) {
  auto dialog = new CatalogSettingsDialog(this);

#ifdef __OCPN__ANDROID__
  androidDisableRotation();
#endif

  dialog->ShowModal();

#ifdef __OCPN__ANDROID__
  androidEnableRotation();
#endif
}

void CatalogMgrPanel::OnTarballButton(wxCommandEvent &event) {
  // Present a file selector dialog to get the file name..
  wxString tarballPath;
  int response = g_Platform->DoFileSelectorDialog(
      this, &tarballPath, _("Select tarball file"), GetImportInitDir(),
      wxEmptyString, wxT("tar files (*.tar.gz)|*.tar.gz|All Files (*.*)|*.*"));

  if (response == wxID_OK) {
    // Record the path to the last import file for next time
    wxFileName f = tarballPath;
    wxString used_path = f.GetPath(wxPATH_GET_VOLUME | wxPATH_NO_SEPARATOR);
    if (used_path != wxEmptyString) {
      pConfig->SetPath(_T("/PlugIns/"));
      pConfig->Write("LatestImportDir", used_path);
      pConfig->Flush();
    }

    // Traverse the tarball to find the required "metadata.xml file

    // Store the metadata file in temp location
    wxString tmpMetadata = wxFileName::CreateTempFileName("meta");

    struct archive *src = archive_read_new();
    archive_read_support_filter_gzip(src);
    archive_read_support_format_tar(src);
    int r = archive_read_open_filename(src, tarballPath.ToStdString().c_str(),
                                       10240);
    if (r != ARCHIVE_OK) {
      std::ostringstream os;
      // os << "Cannot read installation tarball: " << path;
      wxLogWarning(os.str().c_str());
      // last_error_msg = os.str();
      return;
    }

    struct archive *dest = archive_write_disk_new();
    archive_write_disk_set_options(dest, ARCHIVE_EXTRACT_TIME);

    bool bFoundMetadata = false;
    struct archive_entry *entry = 0;
    while (true) {
      int r = archive_read_next_header(src, &entry);
      if (r == ARCHIVE_EOF) {
        break;
      }
      if (r < ARCHIVE_OK) {
        break;
      }

      //  Find the file "metadata.xml"
      std::string path = archive_entry_pathname(entry);
      if (std::string::npos != path.find("metadata.xml")) {
        bFoundMetadata = true;
        archive_entry_set_pathname(entry, tmpMetadata.mb_str());

        if (r >= ARCHIVE_OK && archive_entry_size(entry) > 0) {
          const void *buff;
          size_t size;
          la_int64_t offset;

          r = archive_write_header(dest, entry);
          if (r < ARCHIVE_OK) {
            break;
          }

          while (true) {
            r = archive_read_data_block(src, &buff, &size, &offset);
            if (r == ARCHIVE_EOF) {
              break;
            }
            if (r < ARCHIVE_OK) {
              break;
            }
            r = archive_write_data_block(dest, buff, size, offset);
            if (r < ARCHIVE_OK) {
              wxLogWarning("Error copying install data: %s",
                           archive_error_string(dest));

              break;
            }
          }
          if (r < ARCHIVE_OK) {
            break;
          }

          r = archive_write_finish_entry(dest);
          if (r < ARCHIVE_OK) {
            break;
          }
        }
      } else {
        continue;
      }
    }
    archive_read_free(src);
    archive_write_free(dest);

    // Any tarball extraction problem?
    if (r < ARCHIVE_OK) {
      OCPNMessageBox(this, _("Error extracting import plugin tarball."),
                     _("OpenCPN Plugin Import Error"));
      return;
    }

    // Found the metadata?
    if (!bFoundMetadata) {
      OCPNMessageBox(
          this,
          _("Error, import plugin tarball does not contain required metadata."),
          _("OpenCPN Plugin Import Error"));
      return;
    }

    // Parse the import metadata
    PluginMetadata importPlugin;

    pugi::xml_document doc;
    bool ret = doc.load_file(tmpMetadata.mb_str());
    if (ret) {
      pugi::xml_node pluginRoot = doc.first_child();

      if (!parsePluginNode(pluginRoot, importPlugin)) {
        OCPNMessageBox(this, _("Error processing import plugin metadata."),
                       _("OpenCPN Plugin Import Error"));
        return;
      }
    }

    //  TODO Validate the metadata, in some simplistic way...
    if (!PluginHandler::isCompatible(importPlugin)) {
      OCPNMessageBox(this, _("Incompatible import plugin detected."),
                     _("OpenCPN Plugin Import Error"));
      return;
    }

    // Load and parse a working copy of the currently active catalog...
    std::vector<PluginMetadata> pluginArray;

    pugi::xml_document catalog;
    wxString currentCatalog = g_Platform->GetPrivateDataDir() +
                              wxFileName::GetPathSeparator() +
                              _T("ocpn-plugins.xml");
    bool badCatalog = false;
    if (!wxFileExists(currentCatalog)) {
      badCatalog = true;
    } else {
      ret = catalog.load_file(currentCatalog.mb_str());
      if (!ret) {
        badCatalog = true;
      }
    }

    // If current default catalog is corrupt, or missing,
    // then create a new "stub" catalog

    if (badCatalog) {
      if (wxFileExists(currentCatalog)) wxRemoveFile(currentCatalog);

      wxTextFile stubCatalog(currentCatalog);
      stubCatalog.Create();
      stubCatalog.AddLine(_T("<?xml version=\"1.0\" ?>"));
      stubCatalog.AddLine(_T("<plugins>"));
      stubCatalog.AddLine(_T("<version>0.0.0</version>"));
      stubCatalog.AddLine(_T("<date>2021-06-01 00:01</date>"));
      stubCatalog.AddLine(_T("</plugins>"));
      stubCatalog.Write();
      stubCatalog.Close();

      catalog.load_file(currentCatalog.mb_str());
    }

    pugi::xml_node catalogRoot = catalog.first_child();
    for (pugi::xml_node element = catalogRoot.first_child(); element;
         element = element.next_sibling()) {
      if (!strcmp(element.name(), "plugin")) {
        PluginMetadata catalogPlugin;
        parsePluginNode(element, catalogPlugin);
        pluginArray.push_back(catalogPlugin);
      }
    }

    //  Merge the import plugin metadata with the current catalog
    //  By merge, we mean this:
    //  1.  If the import metadata does not exist in the catalog, append it
    //  2.  If the import metadata does exist (name, target, and target-version
    //  match), then..
    //      update the catalog version with the import version
    bool bmerge = false;
    for (size_t i = 0; i < pluginArray.size(); i++) {
      PluginMetadata candidate = pluginArray[i];
      if (importPlugin.name == candidate.name) {
        if (importPlugin.target == candidate.target) {
          if (importPlugin.target_version == candidate.target_version) {
            bmerge = true;
            pluginArray[i].version = importPlugin.version;
            pluginArray[i].release = importPlugin.release;
            pluginArray[i].summary = importPlugin.summary;
            pluginArray[i].description = importPlugin.description;
            pluginArray[i].tarball_url = importPlugin.tarball_url;
            break;
          }
        }
      }
    }
    // If there was no exact merge, then simply add the importPlugin metadata
    // (case 1)
    if (!bmerge) pluginArray.push_back(importPlugin);

    // Write out the newly merged catalog, replacing the currently active
    // catalog
    pugi::xml_document newCatalog;
    pugi::xml_node pluginsNode = newCatalog.append_child("plugins");

    pugi::xml_node childT = pluginsNode.append_child("version");
    childT.append_child(pugi::node_pcdata).set_value("0.0.0");
    childT = pluginsNode.append_child("date");
    wxDateTime now = wxDateTime::GetTimeNow();
    wxString timeFormat = now.FormatISOCombined(' ');
    childT.append_child(pugi::node_pcdata).set_value(timeFormat.mb_str());

    for (size_t i = 0; i < pluginArray.size(); i++) {
      PluginMetadata workingMetadata = pluginArray[i];

      pugi::xml_node pluginNode = pluginsNode.append_child("plugin");
      pugi::xml_attribute version = pluginNode.append_attribute("version");
      version.set_value("1");

      populatePluginNode(pluginNode, workingMetadata);
    }

    wxString catalogName = g_Platform->GetPrivateDataDir() +
                           wxFileName::GetPathSeparator() +
                           _T("ocpn-plugins.xml");
    newCatalog.save_file(catalogName.mb_str(), "  ");

    // Copy the metadata to the tarball cache

    wxFileName fn(tarballPath);
    if (ocpn::store_tarball(tarballPath.ToStdString().c_str(),
                            fn.GetFullName().ToStdString().c_str())) {
      wxLogMessage("Copied %s to local cache",
                   tarballPath.ToStdString().c_str());
    }

    // Ready to load and process the merged catalog...

    // Reset the PluginHandler catalog file source.
    // This will cause the Handler to find, load, and parse the just-merged
    // catalog as copied to g_Platform->GetPrivateDataDir()...
    auto pluginHandler = PluginHandler::getInstance();
    pluginHandler->setMetadata("");

    //  Reload all plugins, which will also update the status fields
    LoadAllPlugIns(false);

    // Update this Panel, and the entire list.
    if (m_PluginListPanel) {
      auto loader = PluginLoader::getInstance();
      m_PluginListPanel->ReloadPluginPanels();
    }

    // Success!
    wxString msg = _("Plugin imported successfully");
    msg += _T("\n");
    msg += _("Active catalog updated.");
    msg += _T("\n");
    msg += _("Plugin may be installed or updated now.");
    OCPNMessageBox(this, msg, _("OpenCPN Plugin Import Successful"));
  }
}

wxString CatalogMgrPanel::GetCatalogText(bool updated) {
  wxString catalog;
  catalog = updated ? _("Active Catalog") : _("Last Catalog");
  catalog += _T(": ");

  // Check the config file to learn what was the last catalog downloaded.
  pConfig->SetPath(_T("/PlugIns/"));
  wxString latestCatalog =
      pConfig->Read(_T("LatestCatalogDownloaded"), _T("default"));
  catalog += latestCatalog;

  //  Get the version from the currently active catalog, by which we mean
  //  the latest catalog parsed.
  auto pluginHandler = PluginHandler::getInstance();
  std::string date = pluginHandler->GetCatalogData()->date;

  catalog += wxString("  ") + _("Last change: ") + " " + date;
  if (!updated) catalog += _T("  : ") + _("Please Update Plugin Catalog.");

  return catalog;
}

void CatalogMgrPanel::SetUpdateButtonLabel() {
  wxString label = _("Update Plugin Catalog");
  label += _T(": ");
  label += g_catalog_channel == "" ? "master" : g_catalog_channel;
  m_updateButton->SetLabel(label);
  Layout();
}

wxString CatalogMgrPanel::GetImportInitDir() {
  // Check the config file for the last Import path.
  pConfig->SetPath(_T("/PlugIns/"));
  wxString lastImportDir;
  lastImportDir = pConfig->Read(_T("LatestImportDir"),
                                g_Platform->GetWritableDocumentsDir());
  if (wxDirExists(lastImportDir)) {
    return lastImportDir;
  }
  return (g_Platform->GetWritableDocumentsDir());
}

BEGIN_EVENT_TABLE(PluginListPanel, wxScrolledWindow)
// EVT_BUTTON( ID_CMD_BUTTON_PERFORM_ACTION,
// PluginListPanel::OnPluginPanelAction )
END_EVENT_TABLE()

PluginListPanel::PluginListPanel(wxWindow *parent, wxWindowID id,
                                 const wxPoint &pos, const wxSize &size,
                                 ArrayOfPlugIns *pPluginArray)
    : wxScrolledWindow(parent, id, pos, size, wxTAB_TRAVERSAL | wxVSCROLL),
      m_PluginSelected(0) {
  SetSizer(new wxBoxSizer(wxVERTICAL));
  ReloadPluginPanels();
  // SetScrollRate(0, 1);
}

void PluginListPanel::SelectByName(wxString &name) {
  for (auto it = GetChildren().GetFirst(); it; it = it->GetNext()) {
    auto pluginPanel = dynamic_cast<PluginPanel *>(it->GetData());
    if (pluginPanel) {
      if (pluginPanel->GetPluginPtr()->m_common_name.IsSameAs(name)) {
        pluginPanel->SetSelected(true);
        pluginPanel->Layout();
        SelectPlugin(pluginPanel);
        break;
      }
    }
  }
}

void PluginListPanel::ReloadPluginPanels() {
  auto plugins = PluginLoader::getInstance()->GetPlugInArray();
  m_pPluginArray = plugins;
  m_PluginItems.Clear();

  wxWindowList kids = GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode *node = kids.Item(i);
    wxWindow *win = node->GetData();
    PluginPanel *pp = dynamic_cast<PluginPanel *>(win);
    if (pp) win->Destroy();
  }

  GetSizer()->Clear();

  Hide();
  m_PluginSelected = 0;
  for (size_t i = m_pPluginArray->GetCount(); i > 0; i -= 1) {
    PlugInContainer *pic = m_pPluginArray->Item(i - 1);
    AddPlugin(pic);
  }
  Show();
  Layout();
  Refresh(true);

  Scroll(0, 0);
}

void PluginListPanel::AddPlugin(PlugInContainer *pic) {
  auto pPluginPanel =
      new PluginPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pic);
  pPluginPanel->SetSelected(false);
  GetSizer()->Add(pPluginPanel, 0, wxEXPAND);
  m_PluginItems.Add(pPluginPanel);

  m_pluginSpacer = g_Platform->GetDisplayDPmm() * 1.0;
  GetSizer()->AddSpacer(m_pluginSpacer);

  //    wxStaticLine* itemStaticLine = new wxStaticLine( m_panel, wxID_ANY,
  //    wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  //    m_pitemBoxSizer01->Add( itemStaticLine, wxSizerFlags().Expand());
}

//    When a child Panel is selected, its size grows to include "Preferences"
//    and Enable" buttons.  As a consequence, the vertical size of the
//    ListPanel grows as well.Calculate and add a spacer to bottom of
//    ListPanel so that initial ListPanel minimum size calculations account
//    for selected Panel size growth. Sadly, this does not work right on wxQt.
//    So, just punt for now...
int PluginListPanel::ComputePluginSpace(ArrayOfPluginPanel plugins,
                                        wxBoxSizer *sizer) {
  int max_dy = 0;
  for (size_t i = 0; i < plugins.GetCount(); i++) {
    auto panel = plugins.Item(i);
    bool was_selected = panel->GetSelected();
    panel->SetSelected(false);
    sizer->Layout();
    wxSize unselected = panel->GetSize();

    panel->SetSelected(true);  // switch to selected, a bit bigger
    sizer->Layout();
    wxSize selected = panel->GetSize();

    int dy = selected.GetHeight() - unselected.GetHeight();
    max_dy = wxMax(max_dy, dy);
    panel->SetSelected(was_selected);
  }
  return max_dy;
}

void PluginListPanel::UpdatePluginsOrder() {
  m_pPluginArray->Clear();
  for (unsigned int i = 0; i < m_PluginItems.GetCount(); i++) {
    m_pPluginArray->Insert(m_PluginItems[i]->GetPluginPtr(), 0);
  }
}

PluginListPanel::~PluginListPanel() {}

void PluginListPanel::UpdateSelections() {
  for (unsigned int i = 0; i < m_PluginItems.GetCount(); i++) {
    PluginPanel *pPluginPanel = m_PluginItems[i];
    if (pPluginPanel) {
      pPluginPanel->SetSelected(pPluginPanel->GetSelected());
    }
  }
}

void PluginListPanel::SelectPlugin(PluginPanel *pi) {
  int xs, ys;
  GetViewStart(&xs, &ys);
  Scroll(0, 0);

  if (m_PluginSelected) {
    m_PluginSelected->SetSelected(false);
    m_PluginSelected->Layout();
  }

  if (pi == NULL) m_PluginSelected->SetSelected(false);

  m_PluginSelected = pi;

  GetSizer()->Layout();
  Refresh(false);
  wxSize size = GetBestVirtualSize();
  SetVirtualSize(size);

  //  Measure, and ensure that the selected item is fully visible in the
  //  vertical scroll box.
  int htop = 0;
  for (unsigned int i = 0; i < m_PluginItems.GetCount(); i++) {
    PluginPanel *pPluginPanel = m_PluginItems[i];
    int yd = pPluginPanel->GetSize().y;
    htop += yd;
    htop += m_pluginSpacer;
    if (pPluginPanel == pi) {
      int piBottom = htop - (ys * g_options->GetScrollRate());
      if (piBottom > GetClientSize().y) {
        ys += (piBottom - GetClientSize().y) / g_options->GetScrollRate();
      }
      break;
    }
  }

  Scroll(xs, ys);
}

void PluginListPanel::MoveUp(PluginPanel *pi) {
  int pos = m_PluginItems.Index(pi);
  if (pos == 0)  // The first one can't be moved further up
    return;
  m_PluginItems.RemoveAt(pos);
  // m_pitemBoxSizer01->Remove( pos * 2 + 1 );
  // m_pitemBoxSizer01->Remove( pos * 2 );
  m_PluginItems.Insert(pi, pos - 1);
  wxStaticLine *itemStaticLine = new wxStaticLine(
      this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  // m_pitemBoxSizer01->Insert( (pos - 1) * 2, itemStaticLine, 0,
  // wxEXPAND|wxALL, 0 ); m_pitemBoxSizer01->Insert( (pos - 1) * 2, pi, 0,
  // wxEXPAND|wxALL, 0 );

  m_PluginSelected = pi;

  GetSizer()->Layout();
  m_parent->Layout();
  Refresh(true);
}

void PluginListPanel::MoveDown(PluginPanel *pi) {
  int pos = m_PluginItems.Index(pi);
  if (pos == (int)m_PluginItems.Count() -
                 1)  // The last one can't be moved further down
    return;
  m_PluginItems.RemoveAt(pos);
  // m_pitemBoxSizer01->Remove( pos * 2 + 1 );
  // m_pitemBoxSizer01->Remove( pos * 2 );
  m_PluginItems.Insert(pi, pos + 1);
  wxStaticLine *itemStaticLine = new wxStaticLine(
      this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  // m_pitemBoxSizer01->Insert( (pos + 1) * 2 - 1, itemStaticLine, 0,
  // wxEXPAND|wxALL, 0 ); m_pitemBoxSizer01->Insert( (pos + 1) * 2, pi, 0,
  // wxEXPAND|wxALL, 0 );

  m_PluginSelected = pi;

  GetSizer()->Layout();
  m_parent->Layout();
  Refresh(false);
}

static bool canUninstall(std::string name) {
  PluginHandler *pluginHandler = PluginHandler::getInstance();
  // std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  for (auto plugin : pluginHandler->getInstalled()) {
    if (plugin.name == name) {
      if (safe_mode::get_mode())
        return true;
      else
        return !plugin.readonly;
    }
  }
  return false;
}

BEGIN_EVENT_TABLE(PluginPanel, wxPanel)
EVT_PAINT(PluginPanel::OnPaint)
END_EVENT_TABLE()

PluginPanel::PluginPanel(wxPanel *parent, wxWindowID id, const wxPoint &pos,
                         const wxSize &size, PlugInContainer *p_plugin)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE) {
  m_PluginListPanel = (PluginListPanel *)parent;  //->GetParent();
  m_PluginListPanel = dynamic_cast<PluginListPanel *>(parent /*->GetParent()*/);
  wxASSERT(m_PluginListPanel != 0);

  m_pPlugin = p_plugin;
  m_bSelected = false;
  m_penWidthUnselected = g_Platform->GetDisplayDPmm() * .25;
  m_penWidthSelected = g_Platform->GetDisplayDPmm() * .5;

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(topSizer);

  wxBoxSizer *itemBoxSizer01 = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(itemBoxSizer01, 0, wxEXPAND);
  Bind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
  Bind(wxEVT_LEFT_UP, &PluginPanel::OnPluginSelectedUp, this);

  double iconSize = GetCharWidth() * 4;
  wxImage plugin_icon;
  ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
  if (m_pPlugin->m_bitmap && m_pPlugin->m_bitmap->IsOk()) {
    plugin_icon = m_pPlugin->m_bitmap->ConvertToImage();
  }
  wxBitmap bitmap;
  if (plugin_icon.IsOk()) {
    int nowSize = plugin_icon.GetWidth();
    if ((nowSize > iconSize) || ((iconSize / nowSize) > 1.5))
      plugin_icon.Rescale(iconSize, iconSize, wxIMAGE_QUALITY_HIGH);
    bitmap = wxBitmap(plugin_icon);
  } else if (m_pPlugin->m_pluginStatus ==
             PluginStatus::ManagedInstallAvailable) {
    wxFileName path(g_Platform->GetSharedDataDir(), "packageBox.svg");
    path.AppendDir("uidata");
    path.AppendDir("traditional");
    bitmap = LoadSVG(path.GetFullPath(), iconSize, iconSize);
  } else {
    bitmap = wxBitmap(style->GetIcon(_T("default_pi"), iconSize, iconSize));
  }
  m_itemStaticBitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);

  itemBoxSizer01->Add(m_itemStaticBitmap, 0, wxEXPAND | wxALL, 10);
  m_itemStaticBitmap->Bind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected,
                           this);
  m_itemStaticBitmap->Bind(wxEVT_LEFT_UP, &PluginPanel::OnPluginSelectedUp,
                           this);

  wxBoxSizer *itemBoxSizer02 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer01->Add(itemBoxSizer02, 1, wxEXPAND | wxALL, 0);

  // Calculate character width available
  int nChars = g_options->GetSize().x / GetCharWidth();
  bool bCompact = false;
  if (nChars < 60)  // Arbitrary, detecting mobile devices in portrait mode.
    bCompact = true;

  if (bCompact) {
    // Might need to shorten the Plugin name string
    wxString nameString = m_pPlugin->m_common_name;
    int maxWidth = g_Platform->getDisplaySize().x * 3 / 10;
    wxScreenDC dc;
    int nameWidth;
    dc.GetTextExtent(m_pPlugin->m_common_name, &nameWidth, NULL);
    if (nameWidth > maxWidth) {
      nameString = wxControl::Ellipsize(m_pPlugin->m_common_name, dc,
                                        wxELLIPSIZE_END, maxWidth);
    }
    m_pName = new wxStaticText(this, wxID_ANY, nameString);
    m_pName->Bind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
    m_pName->Bind(wxEVT_LEFT_UP, &PluginPanel::OnPluginSelectedUp, this);
    itemBoxSizer02->Add(m_pName, 0, /*wxEXPAND|*/ wxALL, 5);

    wxFlexGridSizer *sl1 = new wxFlexGridSizer(2, 0, 0);
    sl1->AddGrowableCol(1);
    itemBoxSizer02->Add(sl1, 0, wxEXPAND);

    m_pVersion = new wxStaticText(this, wxID_ANY, _T("X.YY.ZZ.AA"));
    sl1->Add(m_pVersion, 0, /*wxEXPAND|*/ wxALL, 5);
    if (m_pPlugin->m_pluginStatus == PluginStatus::ManagedInstallAvailable) {
      m_pVersion->Hide();
    }
    m_pVersion->Bind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
    m_pVersion->Bind(wxEVT_LEFT_UP, &PluginPanel::OnPluginSelectedUp, this);

    m_cbEnable = new wxCheckBox(this, wxID_ANY, _("Enabled"));
    sl1->Add(m_cbEnable, 1, wxALIGN_RIGHT | wxTOP, 5);
    m_cbEnable->Bind(wxEVT_CHECKBOX, &PluginPanel::OnPluginEnableToggle, this);

    // Might need to shorten the Plugin description string
    wxString descriptionString = m_pPlugin->m_short_description;
    int maxDescriptionWidth = g_Platform->getDisplaySize().x - (iconSize * 4);
    int descriptionWidth;
    dc.GetTextExtent(m_pPlugin->m_short_description, &descriptionWidth, NULL);
    if (descriptionWidth > maxDescriptionWidth)
      descriptionString =
          wxControl::Ellipsize(m_pPlugin->m_short_description, dc,
                               wxELLIPSIZE_END, maxDescriptionWidth);

    // This invocation has the effect of setting the minimum width of the
    // descriptor field.
    m_pDescription =
        new wxStaticText(this, wxID_ANY, descriptionString, wxDefaultPosition,
                         wxSize(maxDescriptionWidth, -1), wxST_NO_AUTORESIZE);
    itemBoxSizer02->Add(m_pDescription, 0, wxEXPAND | wxALL, 5);
    m_pDescription->Bind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
    m_pDescription->Bind(wxEVT_LEFT_UP, &PluginPanel::OnPluginSelectedUp, this);

  } else {
    wxFlexGridSizer *itemBoxSizer03 = new wxFlexGridSizer(4, 0, 0);
    itemBoxSizer03->AddGrowableCol(2);
    itemBoxSizer02->Add(itemBoxSizer03, 0, wxEXPAND);

    wxString nameString = m_pPlugin->m_common_name;
    m_pName = new wxStaticText(this, wxID_ANY, nameString);
    m_pName->Bind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
    m_pName->Bind(wxEVT_LEFT_UP, &PluginPanel::OnPluginSelectedUp, this);

    // Avoid known bug in wxGTK3
#ifndef __WXGTK3__
    wxFont font = GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    m_pName->SetFont(font);
#endif

    itemBoxSizer03->Add(m_pName, 0, /*wxEXPAND|*/ wxALL, 10);

    m_pVersion = new wxStaticText(this, wxID_ANY, _T("X.YY.ZZ.AA"));
    itemBoxSizer03->Add(m_pVersion, 0, /*wxEXPAND|*/ wxALL, 10);
    if (m_pPlugin->m_pluginStatus == PluginStatus::ManagedInstallAvailable) {
      m_pVersion->Hide();
    }
    m_pVersion->Bind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
    m_pVersion->Bind(wxEVT_LEFT_UP, &PluginPanel::OnPluginSelectedUp, this);

    m_cbEnable = new wxCheckBox(this, wxID_ANY, _("Enabled"));
    itemBoxSizer03->Add(m_cbEnable, 1, wxALIGN_RIGHT | wxTOP, 10);
    m_cbEnable->Bind(wxEVT_CHECKBOX, &PluginPanel::OnPluginEnableToggle, this);

    itemBoxSizer03->Add(5 * GetCharWidth(), 1, 0, wxALIGN_RIGHT | wxTOP, 10);

    m_pDescription = new wxStaticText(
        this, wxID_ANY, m_pPlugin->m_short_description, wxDefaultPosition,
        wxSize(-1, -1) /*, wxST_NO_AUTORESIZE*/);
    itemBoxSizer02->Add(m_pDescription, 1, wxEXPAND | wxALL, 5);
    m_pDescription->Bind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
    m_pDescription->Bind(wxEVT_LEFT_UP, &PluginPanel::OnPluginSelectedUp, this);
  }

  if (!bCompact) {
    m_info_btn = new WebsiteButton(this, "https:\\opencpn.org");
    m_info_btn->Hide();
    itemBoxSizer02->Add(m_info_btn, 0);

    m_pButtons = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer02->Add(m_pButtons, 0, /*wxEXPAND|*/ wxALL, 0);
    m_pButtonPreferences = new wxButton(this, wxID_ANY, _("Preferences"),
                                        wxDefaultPosition, wxDefaultSize, 0);
    m_pButtons->Add(m_pButtonPreferences, 0, wxALIGN_LEFT | wxALL, 2);

    m_pButtons->AddSpacer(3 * GetCharWidth());

    m_pButtonAction =
        new wxButton(this, wxID_ANY, "Upgrade to Version XX.XX.XX",
                     wxDefaultPosition, wxDefaultSize, 0);
    m_pButtons->Add(m_pButtonAction, 0, wxALIGN_LEFT | wxALL, 2);

    m_pButtonUninstall = new wxButton(this, wxID_ANY, _("Uninstall"),
                                      wxDefaultPosition, wxDefaultSize, 0);
    m_pButtons->Add(m_pButtonUninstall, 0, wxALIGN_LEFT | wxALL, 2);
  } else {
    m_pButtons = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer02->Add(m_pButtons, 0, /*wxEXPAND|*/ wxALL, 0);

    wxBoxSizer *tline = new wxBoxSizer(wxHORIZONTAL);
    m_pButtons->Add(tline, 0, wxALL, 2);

    m_pButtonPreferences = new wxButton(this, wxID_ANY, _("Preferences"),
                                        wxDefaultPosition, wxDefaultSize, 0);
    tline->Add(m_pButtonPreferences, 0, wxALIGN_LEFT | wxALL, 0);

    tline->AddSpacer(3 * GetCharWidth());

    m_info_btn = new WebsiteButton(this, "https:\\opencpn.org");
    m_info_btn->Hide();
    tline->Add(m_info_btn, 0);

    m_pButtonAction =
        new wxButton(this, wxID_ANY, "Upgrade to Version XX.XX.XX",
                     wxDefaultPosition, wxDefaultSize);
    m_pButtons->Add(m_pButtonAction, 0, wxALIGN_LEFT | wxALL, 2);

    m_pButtonUninstall = new wxButton(this, wxID_ANY, _("Uninstall"),
                                      wxDefaultPosition, wxDefaultSize, 0);
    m_pButtons->Add(m_pButtonUninstall, 0, wxALIGN_LEFT | wxALL, 2);
  }

  wxBitmap statusBitmap;
  auto stat = p_plugin->m_pluginStatus;
  auto icon_name = icon_by_status[stat];

  wxFileName path(g_Platform->GetSharedDataDir(), icon_name);
  path.AppendDir("uidata");
  path.AppendDir("traditional");
  bool ok = false;
  int bmsize = GetCharWidth() * 3;
  if (path.IsFileReadable()) {
    statusBitmap = LoadSVG(path.GetFullPath(), bmsize, bmsize);
    ok = statusBitmap.IsOk();
  }
  if (!ok) {
    auto style = g_StyleManager->GetCurrentStyle();
    statusBitmap = wxBitmap(style->GetIcon(_T("default_pi"), bmsize, bmsize));
    wxLogMessage("Icon: %s not found.", path.GetFullPath());
  }

  m_itemStatusIconBitmap = new wxStaticBitmap(this, wxID_ANY, statusBitmap);
  m_itemStatusIconBitmap->SetToolTip(message_by_status(stat));

  itemBoxSizer01->Add(m_itemStatusIconBitmap, 0, wxEXPAND | wxALL, 20);

  itemBoxSizer02->AddSpacer(GetCharWidth());

  m_pButtonPreferences->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                             &PluginPanel::OnPluginPreferences, this);
  m_pButtonUninstall->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                           &PluginPanel::OnPluginUninstall, this);
  m_pButtonAction->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                        &PluginPanel::OnPluginAction, this);

  SetSelected(m_bSelected);
  SetAutoLayout(true);
  // FitInside();
  Fit();
}

PluginPanel::~PluginPanel() {
  Unbind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
  m_itemStaticBitmap->Unbind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected,
                             this);
  m_pName->Unbind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
  m_pVersion->Unbind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
  m_pDescription->Unbind(wxEVT_LEFT_DOWN, &PluginPanel::OnPluginSelected, this);
  if (m_pButtonAction) {
    m_pButtonAction->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,
                            &PluginPanel::OnPluginAction, this);
  }
  m_pButtonPreferences->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,
                               &PluginPanel::OnPluginPreferences, this);
  m_cbEnable->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,
                     &PluginPanel::OnPluginEnableToggle, this);
}

void PluginPanel::SetActionLabel(wxString &label) {
  m_pButtonAction->SetLabel(label);
  Refresh();
}

static wxStopWatch swclick;
static int downx, downy;

void PluginPanel::OnPluginSelected(wxMouseEvent &event) {
#ifdef __OCPN__ANDROID__
  swclick.Start();
  event.GetPosition(&downx, &downy);
#else
  DoPluginSelect();
#endif
}

void PluginPanel::OnPluginSelectedUp(wxMouseEvent &event) {
#ifdef __OCPN__ANDROID__
  qDebug() << swclick.Time();
  if (swclick.Time() < 200) {
    int upx, upy;
    event.GetPosition(&upx, &upy);
    if ((fabs(upx - downx) < GetCharWidth()) &&
        (fabs(upy - downy) < GetCharWidth())) {
      DoPluginSelect();
    }
  }
  swclick.Start();
#endif
}

void PluginPanel::DoPluginSelect() {
  if (m_pPlugin->m_pluginStatus == PluginStatus::ManagedInstallAvailable) {
    // auto dialog = dynamic_cast<PluginListPanel*>(GetParent());
    // auto dialog = dynamic_cast<PluginListPanel*>(m_parent);
    // wxASSERT(dialog != 0);

    // Install the new plugin, auto-enabling as a convenience measure.
    run_update_dialog(m_PluginListPanel, m_pPlugin, false, 0, true);
  } else if (m_bSelected) {
    SetSelected(false);
    m_PluginListPanel->SelectPlugin(NULL);
  } else {
    SetSelected(true);
    m_PluginListPanel->SelectPlugin(this);
  }
}

void PluginPanel::SetSelected(bool selected) {
  m_bSelected = selected;

  m_pVersion->SetLabel(m_pPlugin->GetVersion().to_string());

  if (m_pPlugin->m_ManagedMetadata.version.size()) {
    // Is this a fully managed and current plugin?
    // If so, as a special case...
    //  We show the version from the metadata, thus handling managed plugins
    //  with API < 117

    if (m_pPlugin->m_pluginStatus ==
        PluginStatus::ManagedInstalledCurrentVersion)
      m_pVersion->SetLabel(m_pPlugin->m_ManagedMetadata.version);

    if (m_pPlugin->m_pluginStatus ==
        PluginStatus::ManagedInstalledUpdateAvailable)
      m_pVersion->SetLabel(wxString(m_pPlugin->m_InstalledManagedVersion));
  }

  if (selected) {
    SetBackgroundColour(GetDialogColor(DLG_SELECTED_BACKGROUND));
    m_pButtons->Show(true);
    bool unInstallPossible =
        canUninstall(m_pPlugin->m_common_name.ToStdString());

    // Directly mark Legacy and system plugins as "not uninstallable"
    if (m_pPlugin->m_pluginStatus == PluginStatus::LegacyUpdateAvailable ||
        m_pPlugin->m_pluginStatus == PluginStatus::Unmanaged ||
        m_pPlugin->m_pluginStatus == PluginStatus::System)
      unInstallPossible = false;

    m_pButtonUninstall->Show(unInstallPossible);

    if (m_pPlugin->m_ManagedMetadata.info_url.size()) {
      m_info_btn->SetURL(m_pPlugin->m_ManagedMetadata.info_url.c_str());
      m_info_btn->Show();
    }

    m_cbEnable->Show(true);

    // Configure the "Action" button
    wxString label;
    SemanticVersion newVersion;
    switch (m_pPlugin->m_pluginStatus) {
      case PluginStatus::LegacyUpdateAvailable:
        label = _("Upgrade to Version ");
        label += wxString(m_pPlugin->m_ManagedMetadata.version.c_str());
        m_action = ActionVerb::UPGRADE_TO_MANAGED_VERSION;
        m_pButtonAction->Enable();
        break;

      case PluginStatus::ManagedInstallAvailable:
        label = _("Install...");
        m_action = ActionVerb::INSTALL_MANAGED_VERSION;
        m_pButtonAction->Enable();
        break;

      case PluginStatus::ManagedInstalledUpdateAvailable:
        label = _("Update to ");
        label += wxString(m_pPlugin->m_ManagedMetadata.version.c_str());
        m_action = ActionVerb::UPGRADE_INSTALLED_MANAGED_VERSION;
        m_pButtonAction->Enable();
        break;

      case PluginStatus::ManagedInstalledCurrentVersion:
        label = _("Reinstall");
        m_action = ActionVerb::REINSTALL_MANAGED_VERSION;
        m_pButtonAction->Enable();
        break;

      case PluginStatus::ManagedInstalledDowngradeAvailable:
        label = _("Downgrade");
        m_action = ActionVerb::DOWNGRADE_INSTALLED_MANAGED_VERSION;
        m_pButtonAction->Enable();
        break;

      case PluginStatus::Unmanaged:
        m_action = ActionVerb::NOP;
        m_pButtonAction->Hide();
        break;

      case PluginStatus::System:
        m_action = ActionVerb::NOP;
        m_pButtonAction->Hide();
        break;

      default:
        label = "TBD";
        m_action = ActionVerb::NOP;
        break;
    }
    SetActionLabel(label);

    Layout();
  } else {
    SetBackgroundColour(GetDialogColor(DLG_UNSELECTED_BACKGROUND));
    // m_pDescription->SetLabel( m_pPlugin->m_short_description );
#ifndef __WXQT__
    // m_pButtons->Show(false);
#else
    // m_pButtons->Show(true);
#endif
    //();

    m_pButtons->Show(false);
    m_info_btn->Hide();

    if (m_pPlugin->m_pluginStatus == PluginStatus::ManagedInstallAvailable)
      m_cbEnable->Show(false);

    Layout();
  }

  // m_pButtons->Show(selected);   // For most platforms, show buttons if
  // selected m_pButtonsUpDown->Show(selected);
#ifdef __OCPN__ANDROID__
  // Some Android devices (e.g. Kyocera) have trouble with  wxBitmapButton...
  // m_pButtonsUpDown->Show(false);
  // m_pButtons->Show(true);     // Always enable buttons for Android
#endif

  Layout();

  if (selected) {
    SetBackgroundColour(GetDialogColor(DLG_SELECTED_BACKGROUND));
  } else {
    SetBackgroundColour(GetDialogColor(DLG_UNSELECTED_BACKGROUND));
  }

  SetEnabled(m_pPlugin->m_bEnabled);

#ifdef __OCPN__ANDROID__
  // Android (wxQT) sizers have troubles...
  // So we set some layout factors to avoid re-sizing on select/deselect.
  //    m_rgSizer->Show(true);
  // m_pButtons->Show(true);
  // m_pButtonAction->Hide();
  // m_pButtonUninstall->Hide();

  Fit();
  // m_PluginListPanel->m_pitemBoxSizer01->Layout();
#endif
}

void PluginPanel::OnPaint(wxPaintEvent &event) {
  wxPaintDC dc(this);

  int penWidth = m_penWidthUnselected;
  wxColour color = GetDialogColor(DLG_UNSELECTED_BACKGROUND);
  wxColour border = GetDialogColor(DLG_UNSELECTED_ACCENT);

  if (m_bSelected) {
    penWidth = m_penWidthSelected;
    color = GetDialogColor(DLG_SELECTED_BACKGROUND);
    border = GetDialogColor(DLG_SELECTED_ACCENT);
  }

  wxBrush b(color, wxSOLID);
  dc.SetBrush(b);
  dc.SetPen(wxPen(border, penWidth));

  dc.DrawRoundedRectangle(5, 5, GetSize().x - 10, GetSize().y - 10, 5);
}

void PluginPanel::OnPluginPreferences(wxCommandEvent &event) {
  if (m_pPlugin->m_bEnabled && m_pPlugin->m_bInitState &&
      (m_pPlugin->m_cap_flag & WANTS_PREFERENCES)) {
#ifdef __OCPN__ANDROID__
    androidDisableRotation();
    m_pPlugin->m_pplugin->ShowPreferencesDialog(
        GetGrandParent());  // GrandParent will be the entire list panel, not
                            // the plugin panel Ensures better centering on
                            // small screens
#else
    m_pPlugin->m_pplugin->ShowPreferencesDialog(this);
#endif
  }
}

void PluginPanel::OnPluginEnableToggle(wxCommandEvent &event) {
  SetEnabled(!m_pPlugin->m_bEnabled);
  if (m_pVersion->GetLabel().IsEmpty())
    m_pVersion->SetLabel(m_pPlugin->GetVersion().to_string());
}

void PluginPanel::OnPluginUninstall(wxCommandEvent &event) {
  m_action = ActionVerb::UNINSTALL_MANAGED_VERSION;

  //  Chain up to the utility event handler
  wxCommandEvent actionEvent(wxEVT_COMMAND_BUTTON_CLICKED);
  actionEvent.SetId(ID_CMD_BUTTON_PERFORM_ACTION);
  actionEvent.SetClientData(this);
  g_pi_manager->GetUtilHandler()->AddPendingEvent(actionEvent);
}

void PluginPanel::OnPluginAction(wxCommandEvent &event) {
  //  Chain up to the utility event handler
  wxCommandEvent actionEvent(wxEVT_COMMAND_BUTTON_CLICKED);
  actionEvent.SetId(ID_CMD_BUTTON_PERFORM_ACTION);
  actionEvent.SetClientData(this);
  g_pi_manager->GetUtilHandler()->AddPendingEvent(actionEvent);

  return;
}

void PluginPanel::SetEnabled(bool enabled) {
  if (m_pPlugin->m_bEnabled != enabled) {
    m_pPlugin->m_bEnabled = enabled;
    PluginLoader::getInstance()->UpdatePlugIns();
    NotifySetupOptionsPlugin(m_pPlugin);
  }
  if (!enabled && !m_bSelected) {
    m_pName->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    m_pVersion->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    m_pDescription->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
#ifdef x__OCPN__ANDROID__
    m_pName->Disable();
    m_pVersion->Disable();
    m_pDescription->Disable();
#endif
  } else {
    m_pName->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    m_pVersion->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    m_pDescription->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
#ifdef x__OCPN__ANDROID__
    m_pName->Enable();
    m_pVersion->Enable();
    m_pDescription->Enable();
#endif
  }

#ifdef __OCPN__ANDROID__
  m_pName->Enable(enabled || m_bSelected);
  m_pVersion->Enable(enabled || m_bSelected);
  m_pDescription->Enable(enabled || m_bSelected);
#endif

  if (m_bSelected) {
    wxString description = m_pPlugin->m_long_description;
    if (description.IsEmpty())
      description = wxString(m_pPlugin->m_ManagedMetadata.description.c_str());

    PanelHardBreakWrapper wrapper(this, description,
                                  g_options->GetSize().x * 7 / 10);
    m_pDescription->SetLabel(wrapper.GetWrapped());
    if (m_pPlugin->m_ManagedMetadata.info_url.size()) {
      m_info_btn->SetURL(m_pPlugin->m_ManagedMetadata.info_url.c_str());
      m_info_btn->Show();
    }
  } else {
    wxString description = m_pPlugin->m_short_description;
    if (description.IsEmpty())
      description = wxString(m_pPlugin->m_ManagedMetadata.summary.c_str());
    PanelHardBreakWrapper wrapper(this, description,
                                  g_options->GetSize().x * 7 / 10);
    m_pDescription->SetLabel(wrapper.GetWrapped());
  }

  m_pButtonPreferences->Enable(enabled &&
                               (m_pPlugin->m_cap_flag & WANTS_PREFERENCES));
  m_cbEnable->SetValue(enabled);
}

void PluginPanel::OnPluginUp(wxCommandEvent &event) {
  m_PluginListPanel->MoveUp(this);
}

void PluginPanel::OnPluginDown(wxCommandEvent &event) {
  m_PluginListPanel->MoveDown(this);
}

/** Invokes client browser on plugin info_url when clicked. */
WebsiteButton::WebsiteButton(wxWindow *parent, const char *url)
    : wxPanel(parent), m_url(url) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto button = new wxButton(this, wxID_ANY, _("Website"));
  button->Enable(strlen(url) > 0);
  vbox->Add(button);
  SetSizer(vbox);
  Bind(wxEVT_COMMAND_BUTTON_CLICKED,
       [=](wxCommandEvent &) { wxLaunchDefaultBrowser(m_url); });
}

// ----------------------------------------------------------------------------
// PlugInChartBase Implmentation
//  This class is the base class for Plug-able chart types
// ----------------------------------------------------------------------------

PlugInChartBase::PlugInChartBase() { m_Chart_Error_Factor = 0.; }

PlugInChartBase::~PlugInChartBase() {}

wxString PlugInChartBase::GetFileSearchMask(void) { return _T(""); }

int PlugInChartBase::Init(const wxString &name, int init_flags) { return 0; }

//    Accessors

double PlugInChartBase::GetNormalScaleMin(double canvas_scale_factor,
                                          bool b_allow_overzoom) {
  return 1.0;
}

double PlugInChartBase::GetNormalScaleMax(double canvas_scale_factor,
                                          int canvas_width) {
  return 2.0e7;
}

bool PlugInChartBase::GetChartExtent(ExtentPI *pext) { return false; }

wxBitmap &PlugInChartBase::RenderRegionView(const PlugIn_ViewPort &VPoint,
                                            const wxRegion &Region) {
  return wxNullBitmap;
}

bool PlugInChartBase::AdjustVP(PlugIn_ViewPort &vp_last,
                               PlugIn_ViewPort &vp_proposed) {
  return false;
}

void PlugInChartBase::GetValidCanvasRegion(const PlugIn_ViewPort &VPoint,
                                           wxRegion *pValidRegion) {}

void PlugInChartBase::SetColorScheme(int cs, bool bApplyImmediate) {}

double PlugInChartBase::GetNearestPreferredScalePPM(double target_scale_ppm) {
  return 1.0;
}

wxBitmap *PlugInChartBase::GetThumbnail(int tnx, int tny, int cs) {
  return NULL;
}

void PlugInChartBase::ComputeSourceRectangle(const PlugIn_ViewPort &vp,
                                             wxRect *pSourceRect) {}

double PlugInChartBase::GetRasterScaleFactor() { return 1.0; }

bool PlugInChartBase::GetChartBits(wxRect &source, unsigned char *pPix,
                                   int sub_samp) {
  return false;
}

int PlugInChartBase::GetSize_X() { return 1; }

int PlugInChartBase::GetSize_Y() { return 1; }

void PlugInChartBase::latlong_to_chartpix(double lat, double lon, double &pixx,
                                          double &pixy) {}

void PlugInChartBase::chartpix_to_latlong(double pixx, double pixy,
                                          double *plat, double *plon) {}

// ----------------------------------------------------------------------------
// PlugInChartBaseGL Implementation
//
// ----------------------------------------------------------------------------

PlugInChartBaseGL::PlugInChartBaseGL() {}

PlugInChartBaseGL::~PlugInChartBaseGL() {}

int PlugInChartBaseGL::RenderRegionViewOnGL(const wxGLContext &glc,
                                            const PlugIn_ViewPort &VPoint,
                                            const wxRegion &Region,
                                            bool b_use_stencil) {
  return 0;
}

ListOfPI_S57Obj *PlugInChartBaseGL::GetObjRuleListAtLatLon(
    float lat, float lon, float select_radius, PlugIn_ViewPort *VPoint) {
  return NULL;
}

wxString PlugInChartBaseGL::CreateObjDescriptions(ListOfPI_S57Obj *obj_list) {
  return _T("");
}

int PlugInChartBaseGL::GetNoCOVREntries() { return 0; }

int PlugInChartBaseGL::GetNoCOVRTablePoints(int iTable) { return 0; }

int PlugInChartBaseGL::GetNoCOVRTablenPoints(int iTable) { return 0; }

float *PlugInChartBaseGL::GetNoCOVRTableHead(int iTable) { return 0; }

// ----------------------------------------------------------------------------
// PlugInChartBaseExtended Implementation
//
// ----------------------------------------------------------------------------

PlugInChartBaseExtended::PlugInChartBaseExtended() {}

PlugInChartBaseExtended::~PlugInChartBaseExtended() {}

int PlugInChartBaseExtended::RenderRegionViewOnGL(const wxGLContext &glc,
                                                  const PlugIn_ViewPort &VPoint,
                                                  const wxRegion &Region,
                                                  bool b_use_stencil) {
  return 0;
}

int PlugInChartBaseExtended::RenderRegionViewOnGLNoText(
    const wxGLContext &glc, const PlugIn_ViewPort &VPoint,
    const wxRegion &Region, bool b_use_stencil) {
  return 0;
}

int PlugInChartBaseExtended::RenderRegionViewOnGLTextOnly(
    const wxGLContext &glc, const PlugIn_ViewPort &VPoint,
    const wxRegion &Region, bool b_use_stencil) {
  return 0;
}

wxBitmap &PlugInChartBaseExtended::RenderRegionViewOnDCNoText(
    const PlugIn_ViewPort &VPoint, const wxRegion &Region) {
  return wxNullBitmap;
}

bool PlugInChartBaseExtended::RenderRegionViewOnDCTextOnly(
    wxMemoryDC &dc, const PlugIn_ViewPort &VPoint, const wxRegion &Region) {
  return false;
}

ListOfPI_S57Obj *PlugInChartBaseExtended::GetObjRuleListAtLatLon(
    float lat, float lon, float select_radius, PlugIn_ViewPort *VPoint) {
  return NULL;
}

wxString PlugInChartBaseExtended::CreateObjDescriptions(
    ListOfPI_S57Obj *obj_list) {
  return _T("");
}

int PlugInChartBaseExtended::GetNoCOVREntries() { return 0; }

int PlugInChartBaseExtended::GetNoCOVRTablePoints(int iTable) { return 0; }

int PlugInChartBaseExtended::GetNoCOVRTablenPoints(int iTable) { return 0; }

float *PlugInChartBaseExtended::GetNoCOVRTableHead(int iTable) { return 0; }

void PlugInChartBaseExtended::ClearPLIBTextList() {}

// ----------------------------------------------------------------------------
// PlugInChartBaseExtendedPlus2 Implementation
//
// ----------------------------------------------------------------------------

PlugInChartBaseExtendedPlus2::PlugInChartBaseExtendedPlus2() {}

PlugInChartBaseExtendedPlus2::~PlugInChartBaseExtendedPlus2() {}

ListOfPI_S57Obj *
PlugInChartBaseExtendedPlus2::GetLightsObjRuleListVisibleAtLatLon(
    float lat, float lon, PlugIn_ViewPort *VPoint) {
  return NULL;
}

// ----------------------------------------------------------------------------
// PlugInChartBaseGLPlus2 Implementation
//
// ----------------------------------------------------------------------------

PlugInChartBaseGLPlus2::PlugInChartBaseGLPlus2() {}

PlugInChartBaseGLPlus2::~PlugInChartBaseGLPlus2() {}

ListOfPI_S57Obj *PlugInChartBaseGLPlus2::GetLightsObjRuleListVisibleAtLatLon(
    float lat, float lon, PlugIn_ViewPort *VPoint) {
  return NULL;
}

// ----------------------------------------------------------------------------
// ChartPlugInWrapper Implementation
//    This class is a wrapper/interface to PlugIn charts(PlugInChartBase)
// ----------------------------------------------------------------------------

ChartPlugInWrapper::ChartPlugInWrapper() {}

ChartPlugInWrapper::ChartPlugInWrapper(const wxString &chart_class) {
  m_ppo = ::wxCreateDynamicObject(chart_class);
  m_ppicb = wxDynamicCast(m_ppo, PlugInChartBase);
}

ChartPlugInWrapper::~ChartPlugInWrapper() {
  if (m_ppicb) delete m_ppicb;
}

wxString ChartPlugInWrapper::GetFileSearchMask(void) {
  if (m_ppicb)
    return m_ppicb->GetFileSearchMask();
  else
    return _T("");
}

InitReturn ChartPlugInWrapper::Init(const wxString &name,
                                    ChartInitFlag init_flags) {
  if (m_ppicb) {
    wxWindow *pa = wxWindow::FindFocus();

    InitReturn ret_val = (InitReturn)m_ppicb->Init(name, (int)init_flags);

    //    Here we transcribe all the required wrapped member elements up into
    //    the chartbase object which is the parent of this class
    if (ret_val == INIT_OK) {
      m_FullPath = m_ppicb->GetFullPath();
      m_ChartType = (ChartTypeEnum)m_ppicb->GetChartType();
      m_ChartFamily = (ChartFamilyEnum)m_ppicb->GetChartFamily();
      m_projection = (OcpnProjType)m_ppicb->GetChartProjection();
      m_EdDate = m_ppicb->GetEditionDate();
      m_Name = m_ppicb->GetName();
      m_ID = m_ppicb->GetID();
      m_DepthUnits = m_ppicb->GetDepthUnits();
      m_SoundingsDatum = m_ppicb->GetSoundingsDatum();
      m_datum_str = m_ppicb->GetDatumString();
      m_SE = m_ppicb->GetSE();
      m_EdDate = m_ppicb->GetEditionDate();
      m_ExtraInfo = m_ppicb->GetExtraInfo();
      Chart_Error_Factor = m_ppicb->GetChartErrorFactor();
      m_depth_unit_id = (ChartDepthUnitType)m_ppicb->GetDepthUnitId();
      m_Chart_Skew = m_ppicb->GetChartSkew();
      m_Chart_Scale = m_ppicb->GetNativeScale();

      // We estimate ppm_avg as needed by raster texture cache logic...
      // This number works for average BSB charts, scanned with average
      // resolution
      m_ppm_avg = 10000. / m_ppicb->GetNativeScale();  // fallback value

      // Calcuculate a "better" ppm from the chart geo extent and raster size.
      if ((fabs(m_Chart_Skew) < .01) &&
          (CHART_FAMILY_RASTER == m_ChartFamily)) {
        Extent extent;
        if (GetChartExtent(&extent)) {
          double lon_range = extent.ELON - extent.WLON;
          if ((lon_range > 0) &&
              (lon_range < 90.0))  // Be safe about IDL crossing and huge charts
            m_ppm_avg = GetSize_X() / (lon_range * 1852 * 60);
        }
      }

      m_overlayENC = false;
      if (m_ChartFamily == (ChartFamilyEnum)PI_CHART_FAMILY_VECTOR) {
        wxCharBuffer buf = m_FullPath.ToUTF8();
        m_overlayENC = s57chart::IsCellOverlayType(buf.data());
      }

      bReadyToRender = m_ppicb->IsReadyToRender();
    } else {
      //  Mark the chart as unable to render
      m_ChartType = CHART_TYPE_UNKNOWN;
      m_ChartFamily = CHART_FAMILY_UNKNOWN;
    }

    //  PlugIn may invoke wxExecute(), which steals the keyboard focus
    //  So take it back
    ChartCanvas *pc = wxDynamicCast(pa, ChartCanvas);
    if (pc) pc->SetFocus();

    return ret_val;
  } else
    return INIT_FAIL_REMOVE;
}

//    Accessors
int ChartPlugInWrapper::GetCOVREntries() {
  if (m_ppicb)
    return m_ppicb->GetCOVREntries();
  else
    return 0;
}

int ChartPlugInWrapper::GetCOVRTablePoints(int iTable) {
  if (m_ppicb)
    return m_ppicb->GetCOVRTablePoints(iTable);
  else
    return 0;
}

int ChartPlugInWrapper::GetCOVRTablenPoints(int iTable) {
  if (m_ppicb)
    return m_ppicb->GetCOVRTablenPoints(iTable);
  else
    return 0;
}

float *ChartPlugInWrapper::GetCOVRTableHead(int iTable) {
  if (m_ppicb)
    return m_ppicb->GetCOVRTableHead(iTable);
  else
    return 0;
}

//      TODO
//      PlugIn chart types do not properly support NoCovr Regions
//      Proper fix is to update PlugIn Chart Type API
//      Derive an extended PlugIn chart class from existing class,
//      and use some kind of RTTI to figure out which class to call.
int ChartPlugInWrapper::GetNoCOVREntries() {
  if (m_ppicb) {
    PlugInChartBaseGL *ppicbgl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
    if (ppicbgl) {
      return ppicbgl->GetNoCOVREntries();
    }
  }
  return 0;
}

int ChartPlugInWrapper::GetNoCOVRTablePoints(int iTable) {
  if (m_ppicb) {
    PlugInChartBaseGL *ppicbgl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
    if (ppicbgl) {
      return ppicbgl->GetNoCOVRTablePoints(iTable);
    }
  }
  return 0;
}

int ChartPlugInWrapper::GetNoCOVRTablenPoints(int iTable) {
  if (m_ppicb) {
    PlugInChartBaseGL *ppicbgl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
    if (ppicbgl) {
      return ppicbgl->GetNoCOVRTablenPoints(iTable);
    }
  }
  return 0;
}

float *ChartPlugInWrapper::GetNoCOVRTableHead(int iTable) {
  if (m_ppicb) {
    PlugInChartBaseGL *ppicbgl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
    if (ppicbgl) {
      return ppicbgl->GetNoCOVRTableHead(iTable);
    }
  }
  return 0;
}

bool ChartPlugInWrapper::GetChartExtent(Extent *pext) {
  if (m_ppicb) {
    ExtentPI xpi;
    if (m_ppicb->GetChartExtent(&xpi)) {
      pext->NLAT = xpi.NLAT;
      pext->SLAT = xpi.SLAT;
      pext->ELON = xpi.ELON;
      pext->WLON = xpi.WLON;

      return true;
    } else
      return false;
  } else
    return false;
}

ThumbData *ChartPlugInWrapper::GetThumbData(int tnx, int tny, float lat,
                                            float lon) {
  if (m_ppicb) {
    //    Create the bitmap if needed, doing a deep copy from the Bitmap owned
    //    by the PlugIn Chart
    if (!pThumbData->pDIBThumb) {
      wxBitmap *pBMPOwnedByChart =
          m_ppicb->GetThumbnail(tnx, tny, m_global_color_scheme);
      if (pBMPOwnedByChart) {
        wxImage img = pBMPOwnedByChart->ConvertToImage();
        pThumbData->pDIBThumb = new wxBitmap(img);
      } else
        pThumbData->pDIBThumb = NULL;
    }

    pThumbData->Thumb_Size_X = tnx;
    pThumbData->Thumb_Size_Y = tny;

    /*
    //    Plot the supplied Lat/Lon on the thumbnail
                int divx = m_ppicb->Size_X / tnx;
                int divy = m_ppicb->Size_Y / tny;

                int div_factor = __min(divx, divy);

                int pixx, pixy;


          //    Using a temporary synthetic ViewPort and source rectangle,
          //    calculate the ships position on the thumbnail
                ViewPort tvp;
                tvp.pix_width = tnx;
                tvp.pix_height = tny;
                tvp.view_scale_ppm = GetPPM() / div_factor;
                wxRect trex = Rsrc;
                Rsrc.x = 0;
                Rsrc.y = 0;
                latlong_to_pix_vp(lat, lon, pixx, pixy, tvp);
                Rsrc = trex;

                pThumbData->ShipX = pixx;// / div_factor;
                pThumbData->ShipY = pixy;// / div_factor;
    */
    pThumbData->ShipX = 0;
    pThumbData->ShipY = 0;

    return pThumbData;
  } else
    return NULL;
}

ThumbData *ChartPlugInWrapper::GetThumbData() { return pThumbData; }

bool ChartPlugInWrapper::UpdateThumbData(double lat, double lon) {
  return true;
}

double ChartPlugInWrapper::GetNormalScaleMin(double canvas_scale_factor,
                                             bool b_allow_overzoom) {
  if (m_ppicb)
    return m_ppicb->GetNormalScaleMin(canvas_scale_factor, b_allow_overzoom);
  else
    return 1.0;
}

double ChartPlugInWrapper::GetNormalScaleMax(double canvas_scale_factor,
                                             int canvas_width) {
  if (m_ppicb)
    return m_ppicb->GetNormalScaleMax(canvas_scale_factor, canvas_width);
  else
    return 2.0e7;
}

/*              RectRegion:
 *                      This is the Screen region desired to be updated.  Will
 * be either 1 rectangle(full screen) or two rectangles (panning with FBO
 * accelerated pan logic)
 *
 *              Region:
 *                      This is the LLRegion describing the quilt active region
 * for this chart.
 *
 *              So, Actual rendering area onscreen should be clipped to the
 * intersection of the two regions.
 */

bool ChartPlugInWrapper::RenderRegionViewOnGL(const wxGLContext &glc,
                                              const ViewPort &VPoint,
                                              const OCPNRegion &RectRegion,
                                              const LLRegion &Region) {
#ifdef ocpnUSE_GL
  if (m_ppicb) {
    ViewPort vp = VPoint;  // non-const copy

    gs_plib_flags = 0;  // reset the CAPs flag
    PlugInChartBaseGL *ppicb_gl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
    PlugInChartBaseExtended *ppicb_x =
        dynamic_cast<PlugInChartBaseExtended *>(m_ppicb);
    if (!Region.Empty() && (ppicb_gl || ppicb_x)) {
      wxRegion *r = RectRegion.GetNew_wxRegion();
      for (OCPNRegionIterator upd(RectRegion); upd.HaveRects();
           upd.NextRect()) {
        LLRegion chart_region = vp.GetLLRegion(upd.GetRect());
        chart_region.Intersect(Region);

        if (!chart_region.Empty()) {
          ViewPort cvp = glChartCanvas::ClippedViewport(VPoint, chart_region);

          glChartCanvas::SetClipRect(cvp, upd.GetRect(), false);

          ps52plib->m_last_clip_rect = upd.GetRect();

#ifndef USE_ANDROID_GLES2
          glPushMatrix();  //    Adjust for rotation
#endif
          glChartCanvas::RotateToViewPort(VPoint);

          PlugIn_ViewPort pivp = CreatePlugInViewport(cvp);
          if (ppicb_x)
            ppicb_x->RenderRegionViewOnGL(glc, pivp, *r,
                                          glChartCanvas::s_b_useStencil);
          else if (ppicb_gl)
            ppicb_gl->RenderRegionViewOnGL(glc, pivp, *r,
                                           glChartCanvas::s_b_useStencil);

#ifndef USE_ANDROID_GLES2
          glPopMatrix();
#endif
          glChartCanvas::DisableClipRegion();

        }  //! empty
      }    // for
      delete r;
    }
  } else
    return false;
#endif
  return true;
}

// int indexrr;

bool ChartPlugInWrapper::RenderRegionViewOnGLNoText(
    const wxGLContext &glc, const ViewPort &VPoint,
    const OCPNRegion &RectRegion, const LLRegion &Region) {
#ifdef ocpnUSE_GL
  if (m_ppicb) {
    //        printf("\nCPIW::RRVOGLNT  %d %d \n", indexrr++, m_Chart_Scale);

    gs_plib_flags = 0;  // reset the CAPs flag
    PlugInChartBaseExtended *ppicb_x =
        dynamic_cast<PlugInChartBaseExtended *>(m_ppicb);
    PlugInChartBaseGL *ppicb = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
    if (!Region.Empty() && ppicb_x) {
#ifndef USE_ANDROID_GLES2
      glPushMatrix();  //    Adjust for rotation
#endif

      // Start with a clean slate
      glChartCanvas::SetClipRect(VPoint, VPoint.rv_rect, false);
      glChartCanvas::DisableClipRegion();

      glChartCanvas::RotateToViewPort(VPoint);

      PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
      wxRegion *r = RectRegion.GetNew_wxRegion();

      ppicb_x->RenderRegionViewOnGLNoText(glc, pivp, *r,
                                          glChartCanvas::s_b_useStencil);

#ifndef USE_ANDROID_GLES2
      glPopMatrix();
#endif
      delete r;

    }

    else if (!Region.Empty() &&
             ppicb)  // Legacy Vector GL Plugin chart (e.g.S63)
    {
      ViewPort vp = VPoint;  // non-const copy
      wxRegion *r = RectRegion.GetNew_wxRegion();
      for (OCPNRegionIterator upd(RectRegion); upd.HaveRects();
           upd.NextRect()) {
        LLRegion chart_region = vp.GetLLRegion(upd.GetRect());
        chart_region.Intersect(Region);

        if (!chart_region.Empty()) {
          ViewPort cvp = glChartCanvas::ClippedViewport(VPoint, chart_region);

          glChartCanvas::SetClipRect(cvp, upd.GetRect(), false);

          ps52plib->m_last_clip_rect = upd.GetRect();
#ifndef USE_ANDROID_GLES2
          glPushMatrix();  //    Adjust for rotation
#endif
          glChartCanvas::RotateToViewPort(VPoint);

          PlugIn_ViewPort pivp = CreatePlugInViewport(cvp);
          ppicb->RenderRegionViewOnGL(glc, pivp, *r,
                                      glChartCanvas::s_b_useStencil);

#ifndef USE_ANDROID_GLES2
          glPopMatrix();
#endif
          glChartCanvas::DisableClipRegion();

        }  //! empty
      }    // for
      delete r;
    }

  } else
    return false;
#endif
  return true;
}

bool ChartPlugInWrapper::RenderRegionViewOnGLTextOnly(
    const wxGLContext &glc, const ViewPort &VPoint, const OCPNRegion &Region) {
#ifdef ocpnUSE_GL
  if (m_ppicb) {
    gs_plib_flags = 0;  // reset the CAPs flag
    PlugInChartBaseExtended *ppicb_x =
        dynamic_cast<PlugInChartBaseExtended *>(m_ppicb);
    if (!Region.Empty() && ppicb_x) {
      wxRegion *r = Region.GetNew_wxRegion();
      for (OCPNRegionIterator upd(Region); upd.HaveRects(); upd.NextRect()) {
#ifndef USE_ANDROID_GLES2
        glPushMatrix();  //    Adjust for rotation
#endif
        glChartCanvas::RotateToViewPort(VPoint);

        PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
        ppicb_x->RenderRegionViewOnGLTextOnly(glc, pivp, *r,
                                              glChartCanvas::s_b_useStencil);

#ifndef USE_ANDROID_GLES2
        glPopMatrix();
#endif

      }  // for
      delete r;
    }
  } else
    return false;
#endif
  return true;
}

bool ChartPlugInWrapper::RenderRegionViewOnDC(wxMemoryDC &dc,
                                              const ViewPort &VPoint,
                                              const OCPNRegion &Region) {
  if (m_ppicb) {
    gs_plib_flags = 0;  // reset the CAPs flag
    PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
    if (Region.IsOk()) {
      wxRegion *r = Region.GetNew_wxRegion();
      if (!m_overlayENC)
        dc.SelectObject(m_ppicb->RenderRegionView(pivp, *r));
      else {
        wxBitmap &obmp = m_ppicb->RenderRegionView(pivp, *r);

        //    Create a mask to remove the NODTA areas from overlay cells.
        wxColour nodat = GetGlobalColor(_T ( "NODTA" ));
        wxColour nodat_sub = nodat;

#ifdef ocpnUSE_ocpnBitmap
        nodat_sub = wxColour(nodat.Blue(), nodat.Green(), nodat.Red());
#endif
        m_pMask = new wxMask(obmp, nodat_sub);
        obmp.SetMask(m_pMask);

        dc.SelectObject(obmp);
      }

      delete r;
      return true;
    } else
      return false;
  } else
    return false;
}

bool ChartPlugInWrapper::RenderRegionViewOnDCNoText(wxMemoryDC &dc,
                                                    const ViewPort &VPoint,
                                                    const OCPNRegion &Region) {
  if (m_ppicb) {
    gs_plib_flags = 0;  // reset the CAPs flag
    PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);

    PlugInChartBaseExtended *pCBx =
        dynamic_cast<PlugInChartBaseExtended *>(m_ppicb);
    PlugInChartBase *ppicb = dynamic_cast<PlugInChartBase *>(m_ppicb);

    if (Region.IsOk() && (pCBx || ppicb)) {
      wxRegion *r = Region.GetNew_wxRegion();

      if (pCBx)
        dc.SelectObject(pCBx->RenderRegionViewOnDCNoText(pivp, *r));
      else if (ppicb)
        dc.SelectObject(ppicb->RenderRegionView(pivp, *r));

      delete r;
      return true;
    } else
      return false;
  } else
    return false;
}

bool ChartPlugInWrapper::RenderRegionViewOnDCTextOnly(
    wxMemoryDC &dc, const ViewPort &VPoint, const OCPNRegion &Region) {
  if (m_ppicb) {
    bool ret_val = false;
    gs_plib_flags = 0;  // reset the CAPs flag
    PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
    if (Region.IsOk()) {
      wxRegion *r = Region.GetNew_wxRegion();

      PlugInChartBaseExtended *pCBx =
          dynamic_cast<PlugInChartBaseExtended *>(m_ppicb);
      if (pCBx) ret_val = pCBx->RenderRegionViewOnDCTextOnly(dc, pivp, *r);

      delete r;
      return ret_val;
    } else
      return false;
  } else
    return false;
}

void ChartPlugInWrapper::ClearPLIBTextList() {
  if (m_ppicb) {
    PlugInChartBaseExtended *pCBx =
        dynamic_cast<PlugInChartBaseExtended *>(m_ppicb);
    if (pCBx) pCBx->ClearPLIBTextList();
  }
}

bool ChartPlugInWrapper::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed) {
  if (m_ppicb) {
    PlugIn_ViewPort pivp_last = CreatePlugInViewport(vp_last);
    PlugIn_ViewPort pivp_proposed = CreatePlugInViewport(vp_proposed);
    return m_ppicb->AdjustVP(pivp_last, pivp_proposed);
  } else
    return false;
}

void ChartPlugInWrapper::GetValidCanvasRegion(const ViewPort &VPoint,
                                              OCPNRegion *pValidRegion) {
  if (m_ppicb) {
    PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
    // currently convert using wxRegion,
    // this should be changed as wxRegion is proven unstable/buggy on various
    // platforms
    wxRegion region;
    m_ppicb->GetValidCanvasRegion(pivp, &region);
    *pValidRegion = OCPNRegion(region);
  }

  return;
}

void ChartPlugInWrapper::SetColorScheme(ColorScheme cs, bool bApplyImmediate) {
  if (m_ppicb) {
    m_ppicb->SetColorScheme(cs, bApplyImmediate);
  }
  m_global_color_scheme = cs;
  //      Force a new thumbnail
  if (pThumbData) pThumbData->pDIBThumb = NULL;
}

double ChartPlugInWrapper::GetNearestPreferredScalePPM(
    double target_scale_ppm) {
  if (m_ppicb)
    return m_ppicb->GetNearestPreferredScalePPM(target_scale_ppm);
  else
    return 1.0;
}

void ChartPlugInWrapper::ComputeSourceRectangle(const ViewPort &VPoint,
                                                wxRect *pSourceRect) {
  if (m_ppicb) {
    PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
    m_ppicb->ComputeSourceRectangle(pivp, pSourceRect);
  }
}

double ChartPlugInWrapper::GetRasterScaleFactor(const ViewPort &vp) {
  if (m_ppicb) {
    return (wxRound(100000 * GetPPM() / vp.view_scale_ppm)) / 100000.;
  } else
    return 1.0;
}

bool ChartPlugInWrapper::GetChartBits(wxRect &source, unsigned char *pPix,
                                      int sub_samp) {
  wxCriticalSectionLocker locker(m_critSect);

  if (m_ppicb)

    return m_ppicb->GetChartBits(source, pPix, sub_samp);
  else
    return false;
}

int ChartPlugInWrapper::GetSize_X() {
  if (m_ppicb)
    return m_ppicb->GetSize_X();
  else
    return 1;
}

int ChartPlugInWrapper::GetSize_Y() {
  if (m_ppicb)
    return m_ppicb->GetSize_Y();
  else
    return 1;
}

void ChartPlugInWrapper::latlong_to_chartpix(double lat, double lon,
                                             double &pixx, double &pixy) {
  if (m_ppicb) m_ppicb->latlong_to_chartpix(lat, lon, pixx, pixy);
}

void ChartPlugInWrapper::chartpix_to_latlong(double pixx, double pixy,
                                             double *plat, double *plon) {
  if (m_ppicb) m_ppicb->chartpix_to_latlong(pixx, pixy, plat, plon);
}

/* API 1.11  */

/* API 1.11  adds some more common functions to avoid unnecessary code
 * duplication */

wxString toSDMM_PlugIn(int NEflag, double a, bool hi_precision) {
  return toSDMM(NEflag, a, hi_precision);
}

wxColour GetBaseGlobalColor(wxString colorName) {
  return GetGlobalColor(colorName);
}

int OCPNMessageBox_PlugIn(wxWindow *parent, const wxString &message,
                          const wxString &caption, int style, int x, int y) {
  return OCPNMessageBox(parent, message, caption, style, 100, x, y);
}

wxString GetOCPN_ExePath(void) { return g_Platform->GetExePath(); }

wxString *GetpPlugInLocation() { return g_Platform->GetPluginDirPtr(); }

wxString GetWritableDocumentsDir(void) {
  return g_Platform->GetWritableDocumentsDir();
}

wxString GetPlugInPath(opencpn_plugin *pplugin) {
  wxString ret_val;
  ArrayOfPlugIns *pi_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < pi_array->GetCount(); i++) {
    PlugInContainer *pic = pi_array->Item(i);
    if (pic->m_pplugin == pplugin) {
      ret_val = pic->m_plugin_file;
      break;
    }
  }

  return ret_val;
}

//      API 1.11 Access to Vector PlugIn charts

ListOfPI_S57Obj *PlugInManager::GetPlugInObjRuleListAtLatLon(
    ChartPlugInWrapper *target, float zlat, float zlon, float SelectRadius,
    const ViewPort &vp) {
  ListOfPI_S57Obj *list = NULL;
  if (target) {
    PlugInChartBaseGL *picbgl =
        dynamic_cast<PlugInChartBaseGL *>(target->GetPlugInChart());
    if (picbgl) {
      PlugIn_ViewPort pi_vp = CreatePlugInViewport(vp);
      list = picbgl->GetObjRuleListAtLatLon(zlat, zlon, SelectRadius, &pi_vp);

      return list;
    }
    PlugInChartBaseExtended *picbx =
        dynamic_cast<PlugInChartBaseExtended *>(target->GetPlugInChart());
    if (picbx) {
      PlugIn_ViewPort pi_vp = CreatePlugInViewport(vp);
      list = picbx->GetObjRuleListAtLatLon(zlat, zlon, SelectRadius, &pi_vp);

      return list;
    } else
      return list;
  } else
    return list;
}

wxString PlugInManager::CreateObjDescriptions(ChartPlugInWrapper *target,
                                              ListOfPI_S57Obj *rule_list) {
  wxString ret_str;
  if (target) {
    PlugInChartBaseGL *picbgl =
        dynamic_cast<PlugInChartBaseGL *>(target->GetPlugInChart());
    if (picbgl) {
      ret_str = picbgl->CreateObjDescriptions(rule_list);
    } else {
      PlugInChartBaseExtended *picbx =
          dynamic_cast<PlugInChartBaseExtended *>(target->GetPlugInChart());
      if (picbx) {
        ret_str = picbx->CreateObjDescriptions(rule_list);
      }
    }
  }
  return ret_str;
}

//      API 1.11 Access to S52 PLIB
wxString PI_GetPLIBColorScheme() {
  return _T("");  // ps52plib->GetPLIBColorScheme()
}

int PI_GetPLIBDepthUnitInt() {
  if (ps52plib)
    return ps52plib->m_nDepthUnitDisplay;
  else
    return 0;
}

int PI_GetPLIBSymbolStyle() {
  if (ps52plib)
    return ps52plib->m_nSymbolStyle;
  else
    return 0;
}

int PI_GetPLIBBoundaryStyle() {
  if (ps52plib)
    return ps52plib->m_nBoundaryStyle;
  else
    return 0;
}

bool PI_PLIBObjectRenderCheck(PI_S57Obj *pObj, PlugIn_ViewPort *vp) {
  if (ps52plib) {
    //  Create and populate a compatible s57 Object
    S57Obj cobj;
    chart_context ctx;
    CreateCompatibleS57Object(pObj, &cobj, &ctx);

    ViewPort cvp = CreateCompatibleViewport(*vp);

    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

    //  Create and populate a minimally compatible object container
    ObjRazRules rzRules;
    rzRules.obj = &cobj;
    rzRules.LUP = pContext->LUP;
    rzRules.sm_transform_parms = 0;
    rzRules.child = NULL;
    rzRules.next = NULL;

    if (pContext->LUP)
      return ps52plib->ObjectRenderCheck(&rzRules, &cvp);
    else
      return false;
  } else
    return false;
}

int PI_GetPLIBStateHash() {
  if (ps52plib)
    return ps52plib->GetStateHash();
  else
    return 0;
}

void CreateCompatibleS57Object(PI_S57Obj *pObj, S57Obj *cobj,
                               chart_context *pctx) {
  strncpy(cobj->FeatureName, pObj->FeatureName, 8);
  cobj->Primitive_type = (GeoPrim_t)pObj->Primitive_type;
  cobj->att_array = pObj->att_array;
  cobj->attVal = pObj->attVal;
  cobj->n_attr = pObj->n_attr;

  cobj->x = pObj->x;
  cobj->y = pObj->y;
  cobj->z = pObj->z;
  cobj->npt = pObj->npt;

  cobj->iOBJL = pObj->iOBJL;
  cobj->Index = pObj->Index;

  cobj->geoPt = (pt *)pObj->geoPt;
  cobj->geoPtz = pObj->geoPtz;
  cobj->geoPtMulti = pObj->geoPtMulti;

  cobj->m_lat = pObj->m_lat;
  cobj->m_lon = pObj->m_lon;

  cobj->m_DisplayCat = (DisCat)pObj->m_DisplayCat;
  cobj->x_rate = pObj->x_rate;
  cobj->y_rate = pObj->y_rate;
  cobj->x_origin = pObj->x_origin;
  cobj->y_origin = pObj->y_origin;

  cobj->Scamin = pObj->Scamin;
  cobj->nRef = pObj->nRef;
  cobj->bIsAton = pObj->bIsAton;
  cobj->bIsAssociable = pObj->bIsAssociable;

  cobj->m_n_lsindex = pObj->m_n_lsindex;
  cobj->m_lsindex_array = pObj->m_lsindex_array;
  cobj->m_n_edge_max_points = pObj->m_n_edge_max_points;

  if (gs_plib_flags & PLIB_CAPS_OBJSEGLIST) {
    cobj->m_ls_list_legacy =
        (PI_line_segment_element *)
            pObj->m_ls_list;  // note the cast, assumes in-sync layout
  } else
    cobj->m_ls_list_legacy = 0;
  cobj->m_ls_list = 0;

  if (gs_plib_flags & PLIB_CAPS_OBJCATMUTATE)
    cobj->m_bcategory_mutable = pObj->m_bcategory_mutable;
  else
    cobj->m_bcategory_mutable = true;  // assume all objects are mutable

  cobj->m_DPRI = -1;  // default is unassigned, fixed at render time
  if (gs_plib_flags & PLIB_CAPS_OBJCATMUTATE) {
    if (pObj->m_DPRI == -1) {
      S52PLIB_Context *pCtx = (S52PLIB_Context *)pObj->S52_Context;
      if (pCtx->LUP) cobj->m_DPRI = pCtx->LUP->DPRI - '0';
    } else
      cobj->m_DPRI = pObj->m_DPRI;
  }

  cobj->pPolyTessGeo = (PolyTessGeo *)pObj->pPolyTessGeo;
  cobj->m_chart_context = (chart_context *)pObj->m_chart_context;

  if (pObj->auxParm3 != 1234) {
    pObj->auxParm3 = 1234;
    pObj->auxParm0 = -99;
  }

  cobj->auxParm0 = pObj->auxParm0;
  cobj->auxParm1 = 0;
  cobj->auxParm2 = 0;
  cobj->auxParm3 = 0;

  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

  if (pContext->bBBObj_valid)
    // this is ugly because plugins still use wxBoundingBox
    cobj->BBObj.Set(pContext->BBObj.GetMinY(), pContext->BBObj.GetMinX(),
                    pContext->BBObj.GetMaxY(), pContext->BBObj.GetMaxX());

  cobj->CSrules = pContext->CSrules;
  cobj->bCS_Added = pContext->bCS_Added;

  cobj->FText = pContext->FText;
  cobj->bFText_Added = pContext->bFText_Added;
  cobj->rText = pContext->rText;

  cobj->bIsClone = true;  // Protect cloned object pointers in S57Obj dtor

  if (pctx) {
    cobj->m_chart_context = pctx;
    chart_context *ppctx = (chart_context *)pObj->m_chart_context;

    if (ppctx) {
      cobj->m_chart_context->m_pvc_hash = ppctx->m_pvc_hash;
      cobj->m_chart_context->m_pve_hash = ppctx->m_pve_hash;
      cobj->m_chart_context->ref_lat = ppctx->ref_lat;
      cobj->m_chart_context->ref_lon = ppctx->ref_lon;
      cobj->m_chart_context->pFloatingATONArray = ppctx->pFloatingATONArray;
      cobj->m_chart_context->pRigidATONArray = ppctx->pRigidATONArray;
      cobj->m_chart_context->safety_contour = ppctx->safety_contour;
      cobj->m_chart_context->vertex_buffer = ppctx->vertex_buffer;
    }
    cobj->m_chart_context->chart =
        0;  // note bene, this is always NULL for a PlugIn chart
  }
}

bool PI_PLIBSetContext(PI_S57Obj *pObj) {
  S52PLIB_Context *ctx;
  if (!pObj->S52_Context) {
    ctx = new S52PLIB_Context;
    pObj->S52_Context = ctx;
  }

  ctx = (S52PLIB_Context *)pObj->S52_Context;

  S57Obj cobj;
  CreateCompatibleS57Object(pObj, &cobj, NULL);

  LUPname LUP_Name = PAPER_CHART;

  //      Force a re-evaluation of CS rules
  ctx->CSrules = NULL;
  ctx->bCS_Added = false;

  //      Clear the rendered text cache
  if (ctx->bFText_Added) {
    ctx->bFText_Added = false;
    delete ctx->FText;
    ctx->FText = NULL;
  }

  //  Reset object selection box
  ctx->bBBObj_valid = true;
  ctx->BBObj.SetMin(pObj->lon_min, pObj->lat_min);
  ctx->BBObj.SetMax(pObj->lon_max, pObj->lat_max);

  //      This is where Simplified or Paper-Type point features are selected
  switch (cobj.Primitive_type) {
    case GEO_POINT:
    case GEO_META:
    case GEO_PRIM:

      if (PAPER_CHART == ps52plib->m_nSymbolStyle)
        LUP_Name = PAPER_CHART;
      else
        LUP_Name = SIMPLIFIED;

      break;

    case GEO_LINE:
      LUP_Name = LINES;
      break;

    case GEO_AREA:
      if (PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle)
        LUP_Name = PLAIN_BOUNDARIES;
      else
        LUP_Name = SYMBOLIZED_BOUNDARIES;

      break;
  }

  LUPrec *lup = ps52plib->S52_LUPLookup(LUP_Name, cobj.FeatureName, &cobj);
  ctx->LUP = lup;

  //              Convert LUP to rules set
  ps52plib->_LUP2rules(lup, &cobj);

  ctx->MPSRulesList = NULL;

  return true;
}

void PI_UpdateContext(PI_S57Obj *pObj) {
  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
  if (pContext) {
    pContext->bBBObj_valid = true;
    pContext->BBObj.SetMin(pObj->lon_min, pObj->lat_min);
    pContext->BBObj.SetMax(pObj->lon_max, pObj->lat_max);
  }
}

void UpdatePIObjectPlibContext(PI_S57Obj *pObj, S57Obj *cobj,
                               ObjRazRules *rzRules) {
  //  Update the PLIB context after the render operation
  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

  pContext->CSrules = cobj->CSrules;
  pContext->bCS_Added = cobj->bCS_Added;

  pContext->FText = cobj->FText;
  pContext->bFText_Added = cobj->bFText_Added;
  pContext->rText = cobj->rText;

  if (cobj->BBObj.GetValid()) {
    // ugly as plugins still use wxBoundingBox
    pContext->BBObj =
        wxBoundingBox(cobj->BBObj.GetMinLon(), cobj->BBObj.GetMinLat(),
                      cobj->BBObj.GetMaxLon(), cobj->BBObj.GetMaxLat());
    pContext->bBBObj_valid = true;
  }

  //  Render operation may have promoted the object's display category
  //  (e.g.WRECKS)
  pObj->m_DisplayCat = (PI_DisCat)cobj->m_DisplayCat;

  if (gs_plib_flags & PLIB_CAPS_OBJCATMUTATE) pObj->m_DPRI = cobj->m_DPRI;

  pContext->ChildRazRules = rzRules->child;
  pContext->MPSRulesList = rzRules->mps;

  pObj->auxParm0 = cobj->auxParm0;
}

bool PI_GetObjectRenderBox(PI_S57Obj *pObj, double *lat_min, double *lat_max,
                           double *lon_min, double *lon_max) {
  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
  if (pContext) {
    if (lat_min) *lat_min = pContext->BBObj.GetMinY();
    if (lat_max) *lat_max = pContext->BBObj.GetMaxY();
    if (lon_min) *lon_min = pContext->BBObj.GetMinX();
    if (lon_max) *lon_max = pContext->BBObj.GetMaxX();
    return pContext->bBBObj_valid;
  } else
    return false;
}

PI_LUPname PI_GetObjectLUPName(PI_S57Obj *pObj) {
  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
  if (pContext) {
    LUPrec *lup = pContext->LUP;
    if (lup) return (PI_LUPname)(lup->TNAM);
  }
  return (PI_LUPname)(-1);
}

PI_DisPrio PI_GetObjectDisplayPriority(PI_S57Obj *pObj) {
  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
  if (pContext) {
    LUPrec *lup = pContext->LUP;
    if (lup) return (PI_DisPrio)(lup->DPRI);
  }

  return (PI_DisPrio)(-1);
}

PI_DisCat PI_GetObjectDisplayCategory(PI_S57Obj *pObj) {
  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
  if (pContext) {
    LUPrec *lup = pContext->LUP;
    if (lup) return (PI_DisCat)(lup->DISC);
  }
  return (PI_DisCat)(-1);
}
double PI_GetPLIBMarinerSafetyContour() {
  return S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);
}

void PI_PLIBSetLineFeaturePriority(PI_S57Obj *pObj, int prio) {
  //  Create and populate a compatible s57 Object
  S57Obj cobj;
  chart_context ctx;
  CreateCompatibleS57Object(pObj, &cobj, &ctx);

  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

  //  Create and populate a minimally compatible object container
  ObjRazRules rzRules;
  rzRules.obj = &cobj;
  rzRules.LUP = pContext->LUP;
  rzRules.sm_transform_parms = 0;
  rzRules.child = NULL;
  rzRules.next = NULL;
  rzRules.mps = pContext->MPSRulesList;

  if (pContext->LUP) {
    ps52plib->SetLineFeaturePriority(&rzRules, prio);

    //  Update the PLIB context after the render operation
    UpdatePIObjectPlibContext(pObj, &cobj, &rzRules);
  }
}

void PI_PLIBPrepareForNewRender(void) {
  if (ps52plib) {
    ps52plib->PrepareForRender();
    ps52plib->ClearTextList();

    if (gs_plib_flags & PLIB_CAPS_LINE_BUFFER)
      ps52plib->EnableGLLS(true);  // Newer PlugIns can use GLLS
    else
      ps52plib->EnableGLLS(false);  // Older cannot
  }
}

void PI_PLIBSetRenderCaps(unsigned int flags) { gs_plib_flags = flags; }

void PI_PLIBFreeContext(void *pContext) {
  S52PLIB_Context *pctx = (S52PLIB_Context *)pContext;

  if (pctx->ChildRazRules) {
    ObjRazRules *ctop = pctx->ChildRazRules;
    while (ctop) {
      delete ctop->obj;

      if (ps52plib) ps52plib->DestroyLUP(ctop->LUP);
      delete ctop->LUP;

      ObjRazRules *cnxx = ctop->next;
      delete ctop;
      ctop = cnxx;
    }
  }

  if (pctx->MPSRulesList) {
    if (ps52plib && pctx->MPSRulesList->cs_rules) {
      for (unsigned int i = 0; i < pctx->MPSRulesList->cs_rules->GetCount();
           i++) {
        Rules *top = pctx->MPSRulesList->cs_rules->Item(i);
        ps52plib->DestroyRulesChain(top);
      }
      delete pctx->MPSRulesList->cs_rules;
    }
    free(pctx->MPSRulesList);
  }

  delete pctx->FText;

  delete pctx;
}

int PI_PLIBRenderObjectToDC(wxDC *pdc, PI_S57Obj *pObj, PlugIn_ViewPort *vp) {
  //  Create and populate a compatible s57 Object
  S57Obj cobj;
  chart_context ctx;
  CreateCompatibleS57Object(pObj, &cobj, &ctx);

  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

  //  Set up object SM rendering constants
  sm_parms transform;
  toSM(vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon,
       &transform.easting_vp_center, &transform.northing_vp_center);

  //  Create and populate a minimally compatible object container
  ObjRazRules rzRules;
  rzRules.obj = &cobj;
  rzRules.LUP = pContext->LUP;
  rzRules.sm_transform_parms = &transform;
  rzRules.child = pContext->ChildRazRules;
  rzRules.next = NULL;
  rzRules.mps = pContext->MPSRulesList;

  if (pContext->LUP) {
    ViewPort cvp = CreateCompatibleViewport(*vp);

    //  Do the render
    ps52plib->RenderObjectToDC(pdc, &rzRules, &cvp);

    //  Update the PLIB context after the render operation
    UpdatePIObjectPlibContext(pObj, &cobj, &rzRules);
  }

  return 1;
}

int PI_PLIBRenderAreaToDC(wxDC *pdc, PI_S57Obj *pObj, PlugIn_ViewPort *vp,
                          wxRect rect, unsigned char *pixbuf) {
  //  Create a compatible render canvas
  render_canvas_parms pb_spec;

  pb_spec.depth = BPP;
  pb_spec.pb_pitch = ((rect.width * pb_spec.depth / 8));
  pb_spec.lclip = rect.x;
  pb_spec.rclip = rect.x + rect.width - 1;
  pb_spec.pix_buff = pixbuf;  // the passed buffer
  pb_spec.width = rect.width;
  pb_spec.height = rect.height;
  pb_spec.x = rect.x;
  pb_spec.y = rect.y;
#ifdef ocpnUSE_ocpnBitmap
  pb_spec.b_revrgb = true;
#else
  pb_spec.b_revrgb = false;
#endif

  pb_spec.b_revrgb = false;

  //  Create and populate a compatible s57 Object
  S57Obj cobj;
  chart_context ctx;
  CreateCompatibleS57Object(pObj, &cobj, &ctx);

  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

  //  Set up object SM rendering constants
  sm_parms transform;
  toSM(vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon,
       &transform.easting_vp_center, &transform.northing_vp_center);

  //  Create and populate a minimally compatible object container
  ObjRazRules rzRules;
  rzRules.obj = &cobj;
  rzRules.LUP = pContext->LUP;
  rzRules.sm_transform_parms = &transform;
  rzRules.child = pContext->ChildRazRules;
  rzRules.next = NULL;
  rzRules.mps = pContext->MPSRulesList;

  ViewPort cvp = CreateCompatibleViewport(*vp);

  //  If the PlugIn does not support it nativiely, build a fully described
  //  Geomoetry
  if (!(gs_plib_flags & PLIB_CAPS_SINGLEGEO_BUFFER)) {
    if (!pObj->geoPtMulti) {  // do this only once
      PolyTessGeo *tess = (PolyTessGeo *)pObj->pPolyTessGeo;

      if (!tess) return 1;  // bail on empty data

      PolyTriGroup *ptg = new PolyTriGroup;
      ptg->tri_prim_head =
          tess->Get_PolyTriGroup_head()->tri_prim_head;  // tph;
      ptg->bsingle_alloc = false;
      ptg->data_type = DATA_TYPE_DOUBLE;
      tess->Set_PolyTriGroup_head(ptg);

      double *pd = (double *)malloc(sizeof(double));
      pObj->geoPtMulti = pd;  // Hack hack
    }
  }

  if (pContext->LUP) {
    //  Do the render
    ps52plib->RenderAreaToDC(pdc, &rzRules, &cvp, &pb_spec);

    //  Update the PLIB context after the render operation
    UpdatePIObjectPlibContext(pObj, &cobj, &rzRules);
  }

  return 1;
}

int PI_PLIBRenderAreaToGL(const wxGLContext &glcc, PI_S57Obj *pObj,
                          PlugIn_ViewPort *vp, wxRect &render_rect) {
#ifdef ocpnUSE_GL
  //  Create and populate a compatible s57 Object
  S57Obj cobj;
  chart_context ctx;
  CreateCompatibleS57Object(pObj, &cobj, &ctx);

  //    chart_context *pct = (chart_context *)pObj->m_chart_context;

  //  If the PlugIn does not support it nativiely, build a fully described
  //  Geomoetry

  if (!(gs_plib_flags & PLIB_CAPS_SINGLEGEO_BUFFER)) {
    if (!pObj->geoPtMulti) {  // only do this once
      PolyTessGeo *tess = (PolyTessGeo *)pObj->pPolyTessGeo;

      if (!tess) return 1;  // bail on empty data

      PolyTriGroup *ptg =
          new PolyTriGroup;  // this will leak a little, but is POD
      ptg->tri_prim_head = tess->Get_PolyTriGroup_head()->tri_prim_head;
      ptg->bsingle_alloc = false;
      ptg->data_type = DATA_TYPE_DOUBLE;
      tess->Set_PolyTriGroup_head(ptg);

      //  Mark this object using geoPtMulti
      //  The malloc will get free'ed when the object is deleted.
      double *pd = (double *)malloc(sizeof(double));
      pObj->geoPtMulti = pd;  // Hack hack
    }
    cobj.auxParm0 = -6;  // signal that this object render cannot use VBO
    cobj.auxParm1 = -1;  // signal that this object render cannot have single
                         // buffer conversion done
  } else {  // it is a newer PLugIn, so can do single buffer conversion and VBOs
    if (pObj->auxParm0 < 1)
      cobj.auxParm0 = -7;  // signal that this object render can use a
                           // persistent VBO for area triangle vertices
  }

  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

  //  Set up object SM rendering constants
  sm_parms transform;
  toSM(vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon,
       &transform.easting_vp_center, &transform.northing_vp_center);

  //  Create and populate a minimally compatible object container
  ObjRazRules rzRules;
  rzRules.obj = &cobj;
  rzRules.LUP = pContext->LUP;
  rzRules.sm_transform_parms = &transform;
  rzRules.child = pContext->ChildRazRules;
  rzRules.next = NULL;
  rzRules.mps = pContext->MPSRulesList;

  if (pContext->LUP) {
    ViewPort cvp = CreateCompatibleViewport(*vp);

    //  Do the render
    ps52plib->RenderAreaToGL(glcc, &rzRules, &cvp);

    //  Update the PLIB context after the render operation
    UpdatePIObjectPlibContext(pObj, &cobj, &rzRules);
  }

#endif
  return 1;
}

int PI_PLIBRenderObjectToGL(const wxGLContext &glcc, PI_S57Obj *pObj,
                            PlugIn_ViewPort *vp, wxRect &render_rect) {
  //  Create and populate a compatible s57 Object
  S57Obj cobj;
  chart_context ctx;
  CreateCompatibleS57Object(pObj, &cobj, &ctx);

  S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

  //  Set up object SM rendering constants
  sm_parms transform;
  toSM(vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon,
       &transform.easting_vp_center, &transform.northing_vp_center);

  //  Create and populate a minimally compatible object container
  ObjRazRules rzRules;
  rzRules.obj = &cobj;
  rzRules.LUP = pContext->LUP;
  rzRules.sm_transform_parms = &transform;
  rzRules.child = pContext->ChildRazRules;
  rzRules.next = NULL;
  rzRules.mps = pContext->MPSRulesList;

  if (pContext->LUP) {
    ViewPort cvp = CreateCompatibleViewport(*vp);

    //  Do the render
    ps52plib->RenderObjectToGL(glcc, &rzRules, &cvp);

    //  Update the PLIB context after the render operation
    UpdatePIObjectPlibContext(pObj, &cobj, &rzRules);
  }

  return 1;
}

/* API 1.13  */

/* API 1.13  adds some more common functions to avoid unnecessary code
 * duplication */

double fromDMM_Plugin(wxString sdms) { return fromDMM(sdms); }

void SetCanvasRotation(double rotation) {
  gFrame->GetPrimaryCanvas()->DoRotateCanvas(rotation);
}

double GetCanvasTilt() { return gFrame->GetPrimaryCanvas()->GetVPTilt(); }

void SetCanvasTilt(double tilt) {
  gFrame->GetPrimaryCanvas()->DoTiltCanvas(tilt);
}

void SetCanvasProjection(int projection) {
  gFrame->GetPrimaryCanvas()->SetVPProjection(projection);
}

OcpnSound *g_PluginSound = SoundFactory();
static void onPlugInPlaySoundExFinished(void *ptr) {}

// Start playing a sound to a given device and return status to plugin
bool PlugInPlaySoundEx(wxString &sound_file, int deviceIndex) {
  bool ok = g_PluginSound->Load(sound_file, deviceIndex);
  if (!ok) {
    wxLogWarning("Cannot load sound file: %s", sound_file);
    return false;
  }
  auto cmd_sound = dynamic_cast<SystemCmdSound *>(g_PluginSound);
  if (cmd_sound) cmd_sound->SetCmd(g_CmdSoundString.mb_str(wxConvUTF8));

  g_PluginSound->SetFinishedCallback(onPlugInPlaySoundExFinished, NULL);
  ok = g_PluginSound->Play();
  if (!ok) {
    wxLogWarning("Cannot play sound file: %s", sound_file);
  }
  return ok;
}

bool CheckEdgePan_PlugIn(int x, int y, bool dragging, int margin, int delta) {
  return gFrame->GetPrimaryCanvas()->CheckEdgePan(x, y, dragging, margin,
                                                  delta);
}

wxBitmap GetIcon_PlugIn(const wxString &name) {
  ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
  return style->GetIcon(name);
}

void SetCursor_PlugIn(wxCursor *pCursor) {
  gFrame->GetPrimaryCanvas()->pPlugIn_Cursor = pCursor;
}

void AddChartDirectory(wxString &path) {
  if (g_options) {
    g_options->AddChartDir(path);
  }
}

void ForceChartDBUpdate() {
  if (g_options) {
    g_options->pScanCheckBox->SetValue(true);
    g_options->pUpdateCheckBox->SetValue(true);
  }
}

void ForceChartDBRebuild() {
  if (g_options) {
    g_options->pUpdateCheckBox->SetValue(true);
  }
}

wxDialog *GetActiveOptionsDialog() { return g_options; }

int PlatformDirSelectorDialog(wxWindow *parent, wxString *file_spec,
                              wxString Title, wxString initDir) {
  return g_Platform->DoDirSelectorDialog(parent, file_spec, Title, initDir);
}

int PlatformFileSelectorDialog(wxWindow *parent, wxString *file_spec,
                               wxString Title, wxString initDir,
                               wxString suggestedName, wxString wildcard) {
  return g_Platform->DoFileSelectorDialog(parent, file_spec, Title, initDir,
                                          suggestedName, wildcard);
}

//      http File Download Support

//      OCPN_downloadEvent Implementation

OCPN_downloadEvent::OCPN_downloadEvent(wxEventType commandType, int id)
    : wxEvent(id, commandType) {
  m_stat = OCPN_DL_UNKNOWN;
  m_condition = OCPN_DL_EVENT_TYPE_UNKNOWN;
  m_b_complete = false;
  m_sofarBytes = 0;
}

OCPN_downloadEvent::~OCPN_downloadEvent() {}

wxEvent *OCPN_downloadEvent::Clone() const {
  OCPN_downloadEvent *newevent = new OCPN_downloadEvent(*this);
  newevent->m_stat = this->m_stat;
  newevent->m_condition = this->m_condition;

  newevent->m_totalBytes = this->m_totalBytes;
  newevent->m_sofarBytes = this->m_sofarBytes;
  newevent->m_b_complete = this->m_b_complete;

  return newevent;
}

// const wxEventType wxEVT_DOWNLOAD_EVENT = wxNewEventType();
DECL_EXP wxEventType wxEVT_DOWNLOAD_EVENT = wxNewEventType();

_OCPN_DLStatus g_download_status;
_OCPN_DLCondition g_download_condition;

#define DL_EVENT_TIMER 4388

class PI_DLEvtHandler : public wxEvtHandler {
public:
  PI_DLEvtHandler();
  ~PI_DLEvtHandler();

  void onDLEvent(OCPN_downloadEvent &event);
  void setBackgroundMode(long ID, wxEvtHandler *handler);
  void clearBackgroundMode();
  void onTimerEvent(wxTimerEvent &event);

  long m_id;
  wxTimer m_eventTimer;
  wxEvtHandler *m_download_evHandler;

  long m_sofarBytes;
  long m_totalBytes;
};

PI_DLEvtHandler::PI_DLEvtHandler() {
  g_download_status = OCPN_DL_UNKNOWN;
  g_download_condition = OCPN_DL_EVENT_TYPE_UNKNOWN;

  m_download_evHandler = NULL;
  m_id = -1;
  m_sofarBytes = 0;
  m_totalBytes = 0;
}

PI_DLEvtHandler::~PI_DLEvtHandler() {
  m_eventTimer.Stop();
  Disconnect(
      wxEVT_TIMER,
      (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onTimerEvent);
}

void PI_DLEvtHandler::onDLEvent(OCPN_downloadEvent &event) {
  //    qDebug() << "Got Event " << (int)event.getDLEventStatus() <<
  //    (int)event.getDLEventCondition();

  g_download_status = event.getDLEventStatus();
  g_download_condition = event.getDLEventCondition();

  // This is an END event, happening at the end of BACKGROUND file download
  if (m_download_evHandler &&
      (OCPN_DL_EVENT_TYPE_END == event.getDLEventCondition())) {
    OCPN_downloadEvent ev(wxEVT_DOWNLOAD_EVENT, 0);
    ev.setComplete(true);
    ev.setTransferred(m_sofarBytes);
    ev.setTotal(m_totalBytes);

    ev.setDLEventStatus(event.getDLEventStatus());
    ev.setDLEventCondition(OCPN_DL_EVENT_TYPE_END);

    m_download_evHandler->AddPendingEvent(ev);
    m_eventTimer.Stop();
#ifdef __OCPN__ANDROID__
    finishAndroidFileDownload();
#endif
  }

  event.Skip();
}

void PI_DLEvtHandler::setBackgroundMode(long ID, wxEvtHandler *handler) {
  m_id = ID;
  m_download_evHandler = handler;

  m_eventTimer.SetOwner(this, DL_EVENT_TIMER);

  Connect(
      wxEVT_TIMER,
      (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onTimerEvent);
  m_eventTimer.Start(1000, wxTIMER_CONTINUOUS);
}

void PI_DLEvtHandler::clearBackgroundMode() {
  m_download_evHandler = NULL;
  m_eventTimer.Stop();
}

void PI_DLEvtHandler::onTimerEvent(wxTimerEvent &event) {
#ifdef __OCPN__ANDROID__
  //   Query the download status, and post to the original requestor
  //   This method only happens on Background file downloads

  wxString sstat;
  int stat = queryAndroidFileDownload(m_id, &sstat);

  OCPN_downloadEvent ev(wxEVT_DOWNLOAD_EVENT, 0);
  long sofarBytes = 0;
  long totalBytes = -1;
  long state = -1;

  if (stat) {  // some error
    qDebug() << "Error on queryAndroidFileDownload, ending download";
    ev.setComplete(true);
    ev.setTransferred(sofarBytes);
    ev.setTotal(totalBytes);

    ev.setDLEventStatus(OCPN_DL_FAILED);
    ev.setDLEventCondition(OCPN_DL_EVENT_TYPE_END);
  } else {
    wxStringTokenizer tk(sstat, _T(";"));
    if (tk.HasMoreTokens()) {
      wxString token = tk.GetNextToken();
      token.ToLong(&state);
      token = tk.GetNextToken();
      token.ToLong(&sofarBytes);
      token = tk.GetNextToken();
      token.ToLong(&totalBytes);
    }

    qDebug() << state << sofarBytes << totalBytes;

    m_sofarBytes = sofarBytes;
    m_totalBytes = totalBytes;

    ev.setTransferred(sofarBytes);
    ev.setTotal(totalBytes);

    if (state == 16) {  // error
      qDebug() << "Event OCPN_DL_FAILED/OCPN_DL_EVENT_TYPE_END";
      ev.setComplete(true);
      ev.setDLEventStatus(OCPN_DL_FAILED);
      ev.setDLEventCondition(OCPN_DL_EVENT_TYPE_END);
    } else if (state == 8) {  // Completed OK
      qDebug() << "Event OCPN_DL_NO_ERROR/OCPN_DL_EVENT_TYPE_END";
      ev.setComplete(true);
      ev.setDLEventStatus(OCPN_DL_NO_ERROR);
      ev.setDLEventCondition(OCPN_DL_EVENT_TYPE_END);
    } else {
      ev.setComplete(false);
      ev.setDLEventStatus(OCPN_DL_UNKNOWN);
      ev.setDLEventCondition(OCPN_DL_EVENT_TYPE_PROGRESS);
    }

    // 2;0;148686
  }

  if (m_download_evHandler) {
    //        qDebug() << "Sending event on timer...";
    m_download_evHandler->AddPendingEvent(ev);
  }

  //  Background download is all done.
  if (OCPN_DL_EVENT_TYPE_END == ev.getDLEventCondition()) {
    m_eventTimer.Stop();
    finishAndroidFileDownload();
  }

#endif
}

PI_DLEvtHandler *g_piEventHandler;

//  Blocking download of single file
_OCPN_DLStatus OCPN_downloadFile(const wxString &url,
                                 const wxString &outputFile,
                                 const wxString &title, const wxString &message,
                                 const wxBitmap &bitmap, wxWindow *parent,
                                 long style, int timeout_secs) {
#ifdef __OCPN__ANDROID__

  wxString msg = _T("Downloading file synchronously: ");
  msg += url;
  msg += _T(" to: ");
  msg += outputFile;
  wxLogMessage(msg);

  // Validate the write location
  int vres = validateAndroidWriteLocation(outputFile);
  if (vres == 0)  // Pending permission dialog
    return OCPN_DL_ABORTED;

  //  Create a single event handler to receive status events
  if (!g_piEventHandler) g_piEventHandler = new PI_DLEvtHandler;

  //  Reset global status indicators
  g_download_status = OCPN_DL_UNKNOWN;
  g_download_condition = OCPN_DL_EVENT_TYPE_UNKNOWN;

  //  Create a connection for the expected events from Android Activity
  g_piEventHandler->Connect(
      wxEVT_DOWNLOAD_EVENT,
      (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);

  long dl_ID = -1;

  // Make sure the outputfile is a file URI
  wxString fURI = outputFile;
  if (!fURI.StartsWith(_T("file://"))) {
    fURI.Prepend(_T("file://"));
  }

  int res = startAndroidFileDownload(url, fURI, g_piEventHandler, &dl_ID);
  //  Started OK?
  if (res) {
    finishAndroidFileDownload();
    g_piEventHandler->Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
    // delete g_piEventHandler;
    return OCPN_DL_FAILED;
  }

  wxDateTime dl_start_time = wxDateTime::Now();

  //  Spin, waiting for timeout or event from downstream, and checking status
  while (1) {
    wxTimeSpan dt = wxDateTime::Now() - dl_start_time;
    qDebug() << "Spin.." << dt.GetSeconds().GetLo();

    if (dt.GetSeconds() > timeout_secs) {
      qDebug() << "USER_TIMOUT";
      finishAndroidFileDownload();
      g_piEventHandler->Disconnect(
          wxEVT_DOWNLOAD_EVENT,
          (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
      // delete g_piEventHandler;
      return (OCPN_DL_USER_TIMEOUT);
    }

    if (g_download_condition != OCPN_DL_EVENT_TYPE_UNKNOWN) {
      if (OCPN_DL_EVENT_TYPE_END == g_download_condition) {
        _OCPN_DLStatus ss = g_download_status;
        finishAndroidFileDownload();
        g_piEventHandler->Disconnect(
            wxEVT_DOWNLOAD_EVENT,
            (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::
                onDLEvent);
        // delete g_piEventHandler;
        qDebug() << "RETURN DL_END" << (int)ss;
        return ss;  // The actual return code
      }
    }

    wxString sstat;
    int stat = queryAndroidFileDownload(dl_ID, &sstat);
    if (stat) {  // some error
      qDebug() << "Error on queryAndroidFileDownload";
      finishAndroidFileDownload();
      g_piEventHandler->Disconnect(
          wxEVT_DOWNLOAD_EVENT,
          (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
      // delete g_piEventHandler;

      return OCPN_DL_FAILED;  // so abort
    }

    wxSleep(1);
    wxSafeYield();
  }

#elif defined(OCPN_USE_CURL)
  wxFileName tfn = wxFileName::CreateTempFileName(outputFile);
  wxFileOutputStream output(tfn.GetFullPath());

  wxCurlDownloadDialog ddlg(url, &output, title, message + url, bitmap, parent,
                            style);
  wxCurlDialogReturnFlag ret = ddlg.RunModal();
  output.Close();

  _OCPN_DLStatus result = OCPN_DL_UNKNOWN;

  switch (ret) {
    case wxCDRF_SUCCESS: {
      if (wxCopyFile(tfn.GetFullPath(), outputFile))
        result = OCPN_DL_NO_ERROR;
      else
        result = OCPN_DL_FAILED;
      break;
    }
    case wxCDRF_FAILED: {
      result = OCPN_DL_FAILED;
      break;
    }
    case wxCDRF_USER_ABORTED: {
      result = OCPN_DL_ABORTED;
      break;
    }
    default:
      wxASSERT(false);  // This should never happen because we handle all
                        // possible cases of ret
  }
  if (wxFileExists(tfn.GetFullPath())) wxRemoveFile(tfn.GetFullPath());
  return result;

#else
  return OCPN_DL_FAILED;
#endif
}

//  Non-Blocking download of single file
_OCPN_DLStatus OCPN_downloadFileBackground(const wxString &url,
                                           const wxString &outputFile,
                                           wxEvtHandler *handler,
                                           long *handle) {
#ifdef __OCPN__ANDROID__
  wxString msg = _T("Downloading file asynchronously: ");
  msg += url;
  msg += _T(" to: ");
  msg += outputFile;
  wxLogMessage(msg);

  //  Create a single event handler to receive status events

  if (!g_piEventHandler) g_piEventHandler = new PI_DLEvtHandler;

  long dl_ID = -1;

  int res = startAndroidFileDownload(url, outputFile, NULL /*g_piEventHandler*/,
                                     &dl_ID);
  //  Started OK?
  if (res) {
    finishAndroidFileDownload();
    return OCPN_DL_FAILED;
  }

  //  configure the local event handler for background transfer
  g_piEventHandler->setBackgroundMode(dl_ID, handler);

  if (handle) *handle = dl_ID;

  return OCPN_DL_STARTED;

#elif defined(OCPN_USE_CURL)
  if (g_pi_manager->m_pCurlThread)  // We allow just one download at a time. Do
                                    // we want more? Or at least return some
                                    // other status in this case?
    return OCPN_DL_FAILED;
  g_pi_manager->m_pCurlThread =
      new wxCurlDownloadThread(g_pi_manager, CurlThreadId);
  bool http = (url.StartsWith(wxS("http:")) || url.StartsWith(wxS("https:")));
  bool keep = false;
  if (http && g_pi_manager->m_pCurl &&
      dynamic_cast<wxCurlHTTP *>(g_pi_manager->m_pCurl.get())) {
    keep = true;
  }
  if (!keep) {
    g_pi_manager->m_pCurl = 0;
  }

  bool failed = false;
  if (!g_pi_manager->HandleCurlThreadError(
          g_pi_manager->m_pCurlThread->SetURL(url, g_pi_manager->m_pCurl),
          g_pi_manager->m_pCurlThread, url))
    failed = true;
  if (!failed) {
    g_pi_manager->m_pCurl = g_pi_manager->m_pCurlThread->GetCurlSharedPtr();
    if (!g_pi_manager->HandleCurlThreadError(
            g_pi_manager->m_pCurlThread->SetOutputStream(
                new wxFileOutputStream(outputFile)),
            g_pi_manager->m_pCurlThread))
      failed = true;
  }
  if (!failed) {
    g_pi_manager->m_download_evHandler = handler;
    g_pi_manager->m_downloadHandle = handle;

    wxCurlThreadError err = g_pi_manager->m_pCurlThread->Download();
    if (err != wxCTE_NO_ERROR) {
      g_pi_manager->HandleCurlThreadError(
          err, g_pi_manager->m_pCurlThread);  // shows a message to the user
      g_pi_manager->m_pCurlThread->Abort();
      failed = true;
    }
  }

  if (!failed) return OCPN_DL_STARTED;

  if (g_pi_manager->m_pCurlThread) {
    if (g_pi_manager->m_pCurlThread->IsAlive())
      g_pi_manager->m_pCurlThread->Abort();
    if (g_pi_manager->m_pCurlThread->GetOutputStream())
      delete (g_pi_manager->m_pCurlThread->GetOutputStream());
    wxDELETE(g_pi_manager->m_pCurlThread);
    g_pi_manager->m_download_evHandler = NULL;
    g_pi_manager->m_downloadHandle = NULL;
    return OCPN_DL_STARTED;
  }
  g_pi_manager->m_pCurl = 0;
  return OCPN_DL_FAILED;

#else
  return OCPN_DL_FAILED;
#endif
}

void OCPN_cancelDownloadFileBackground(long handle) {
#ifdef OCPN_USE_CURL

#ifdef __OCPN__ANDROID__
  cancelAndroidFileDownload(handle);
  finishAndroidFileDownload();
  if (g_piEventHandler) g_piEventHandler->clearBackgroundMode();
#else
  if (g_pi_manager->m_pCurlThread) {
    g_pi_manager->m_pCurlThread->Abort();
    delete (g_pi_manager->m_pCurlThread->GetOutputStream());
    wxDELETE(g_pi_manager->m_pCurlThread);
    g_pi_manager->m_download_evHandler = NULL;
    g_pi_manager->m_downloadHandle = NULL;
  }
  g_pi_manager->m_pCurl = 0;
#endif
#endif
}

_OCPN_DLStatus OCPN_postDataHttp(const wxString &url,
                                 const wxString &parameters, wxString &result,
                                 int timeout_secs) {

#ifdef __OCPN__ANDROID__
  wxString lparms = parameters;
  wxString postResult = doAndroidPOST(url, lparms, timeout_secs * 1000);
  if (postResult.IsSameAs(_T("NOK"))) return OCPN_DL_FAILED;

  result = postResult;
  return OCPN_DL_NO_ERROR;

#elif  defined(OCPN_USE_CURL)
  wxCurlHTTP post;
  post.SetOpt(CURLOPT_TIMEOUT, timeout_secs);
  size_t res = post.Post(parameters.ToAscii(), parameters.Len(), url);

  if (res) {
    result = wxString(post.GetResponseBody().c_str(), wxConvUTF8);
    return OCPN_DL_NO_ERROR;
  } else
    result = wxEmptyString;

  return OCPN_DL_FAILED;

#else
  return OCPN_DL_FAILED;
#endif
}

bool OCPN_isOnline() {
#ifdef __OCPN__ANDROID__
  return androidCheckOnline();
#endif

#if !defined(__OCPN__ANDROID__) && defined(OCPN_USE_CURL)
  if (wxDateTime::GetTimeNow() >
      g_pi_manager->m_last_online_chk + ONLINE_CHECK_RETRY) {
    wxCurlHTTP get;
    get.Head(_T("http://yahoo.com/"));
    g_pi_manager->m_last_online = get.GetResponseCode() > 0;

    g_pi_manager->m_last_online_chk = wxDateTime::GetTimeNow();
  }
  return g_pi_manager->m_last_online;
#else
  return false;
#endif
}

#if !defined(__OCPN__ANDROID__) && defined(OCPN_USE_CURL)
void PlugInManager::OnEndPerformCurlDownload(wxCurlEndPerformEvent &ev) {
  OCPN_downloadEvent event(wxEVT_DOWNLOAD_EVENT, 0);
  if (ev.IsSuccessful()) {
    event.setDLEventStatus(OCPN_DL_NO_ERROR);
  } else {
    g_pi_manager->m_pCurl = 0;
    event.setDLEventStatus(OCPN_DL_FAILED);
  }
  event.setDLEventCondition(OCPN_DL_EVENT_TYPE_END);
  event.setComplete(true);

  if (m_download_evHandler) {
    m_download_evHandler->AddPendingEvent(event);
    m_download_evHandler = NULL;
    m_downloadHandle = NULL;
  }

  if (m_pCurlThread) {
    m_pCurlThread->Wait();
    if (!m_pCurlThread->IsAborting()) {
      delete (m_pCurlThread->GetOutputStream());
      wxDELETE(m_pCurlThread);
    }
  }
}

void PlugInManager::OnCurlDownload(wxCurlDownloadEvent &ev) {
  OCPN_downloadEvent event(wxEVT_DOWNLOAD_EVENT, 0);
  event.setDLEventStatus(OCPN_DL_UNKNOWN);
  event.setDLEventCondition(OCPN_DL_EVENT_TYPE_PROGRESS);
  event.setTotal(ev.GetTotalBytes());
  event.setTransferred(ev.GetDownloadedBytes());
  event.setComplete(false);

  if (m_download_evHandler) {
    m_download_evHandler->AddPendingEvent(event);
  }
}

bool PlugInManager::HandleCurlThreadError(wxCurlThreadError err,
                                          wxCurlBaseThread *p,
                                          const wxString &url) {
  switch (err) {
    case wxCTE_NO_ERROR:
      return true;  // ignore this

    case wxCTE_NO_RESOURCE:
      wxLogError(
          wxS("Insufficient resources for correct execution of the program."));
      break;

    case wxCTE_ALREADY_RUNNING:
      wxFAIL;  // should never happen!
      break;

    case wxCTE_INVALID_PROTOCOL:
      wxLogError(wxS("The URL '%s' uses an unsupported protocol."),
                 url.c_str());
      break;

    case wxCTE_NO_VALID_STREAM:
      wxFAIL;  // should never happen - the user streams should always be valid!
      break;

    case wxCTE_ABORTED:
      return true;  // ignore this

    case wxCTE_CURL_ERROR: {
      wxString err = wxS("unknown");
      if (p->GetCurlSession())
        err =
            wxString(p->GetCurlSession()->GetErrorString().c_str(), wxConvUTF8);
      wxLogError(wxS("Network error: %s"), err.c_str());
    } break;
  }

  // stop the thread
  if (p->IsAlive()) p->Abort();

  // this is an unrecoverable error:
  return false;
}
#endif

bool LaunchDefaultBrowser_Plugin(wxString url) {
  if (g_Platform) g_Platform->platformLaunchDefaultBrowser(url);

  return true;
}

/* API 1.14 */

void PlugInAISDrawGL(wxGLCanvas *glcanvas, const PlugIn_ViewPort &vp) {
  ViewPort ocpn_vp = CreateCompatibleViewport(vp);

  ocpnDC dc(*glcanvas);

  AISDraw(dc, ocpn_vp, NULL);
}

bool PlugInSetFontColor(const wxString TextElement, const wxColour color) {
  return FontMgr::Get().SetFontColor(TextElement, color);
}

/* API 1.15 */

double PlugInGetDisplaySizeMM() { return g_Platform->GetDisplaySizeMM(); }

wxFont *FindOrCreateFont_PlugIn(int point_size, wxFontFamily family,
                                wxFontStyle style, wxFontWeight weight,
                                bool underline, const wxString &facename,
                                wxFontEncoding encoding) {
  return FontMgr::Get().FindOrCreateFont(point_size, family, style, weight,
                                         underline, facename, encoding);
}

int PluginGetMinAvailableGshhgQuality() {
  return gFrame->GetPrimaryCanvas()->GetMinAvailableGshhgQuality();
}
int PluginGetMaxAvailableGshhgQuality() {
  return gFrame->GetPrimaryCanvas()->GetMaxAvailableGshhgQuality();
}

// disable builtin console canvas, and autopilot nmea sentences
void PlugInHandleAutopilotRoute(bool enable) {
  g_bPluginHandleAutopilotRoute = enable;
}

/* API 1.16 */
wxString GetSelectedWaypointGUID_Plugin() {
  ChartCanvas *cc = gFrame->GetFocusCanvas();
  if (cc && cc->GetSelectedRoutePoint()) {
    return cc->GetSelectedRoutePoint()->m_GUID;
  }
  return wxEmptyString;
}

wxString GetSelectedRouteGUID_Plugin() {
  ChartCanvas *cc = gFrame->GetFocusCanvas();
  if (cc && cc->GetSelectedRoute()) {
    return cc->GetSelectedRoute()->m_GUID;
  }
  return wxEmptyString;
}

wxString GetSelectedTrackGUID_Plugin() {
  ChartCanvas *cc = gFrame->GetFocusCanvas();
  if (cc && cc->GetSelectedTrack()) {
    return cc->GetSelectedTrack()->m_GUID;
  }
  return wxEmptyString;
}

std::unique_ptr<PlugIn_Waypoint> GetWaypoint_Plugin(const wxString &GUID) {
  std::unique_ptr<PlugIn_Waypoint> w(new PlugIn_Waypoint);
  GetSingleWaypoint(GUID, w.get());
  return w;
}

std::unique_ptr<PlugIn_Route> GetRoute_Plugin(const wxString &GUID) {
  std::unique_ptr<PlugIn_Route> r;
  Route *route = g_pRouteMan->FindRouteByGUID(GUID);
  if (route == nullptr) return r;

  r = std::unique_ptr<PlugIn_Route>(new PlugIn_Route);
  PlugIn_Route *dst_route = r.get();

  // PlugIn_Waypoint *pwp;
  RoutePoint *src_wp;
  wxRoutePointListNode *node = route->pRoutePointList->GetFirst();

  while (node) {
    src_wp = node->GetData();

    PlugIn_Waypoint *dst_wp = new PlugIn_Waypoint();
    PlugInFromRoutePoint(dst_wp, src_wp);

    dst_route->pWaypointList->Append(dst_wp);

    node = node->GetNext();
  }
  dst_route->m_NameString = route->m_RouteNameString;
  dst_route->m_StartString = route->m_RouteStartString;
  dst_route->m_EndString = route->m_RouteEndString;
  dst_route->m_GUID = route->m_GUID;

  return r;
}

std::unique_ptr<PlugIn_Track> GetTrack_Plugin(const wxString &GUID) {
  std::unique_ptr<PlugIn_Track> t;
  //  Find the Track
  Track *pTrack = g_pRouteMan->FindTrackByGUID(GUID);
  if (!pTrack) return t;

  std::unique_ptr<PlugIn_Track> tk =
      std::unique_ptr<PlugIn_Track>(new PlugIn_Track);
  PlugIn_Track *dst_track = tk.get();
  dst_track->m_NameString = pTrack->GetName();
  dst_track->m_StartString = pTrack->m_TrackStartString;
  dst_track->m_EndString = pTrack->m_TrackEndString;
  dst_track->m_GUID = pTrack->m_GUID;

  for (int i = 0; i < pTrack->GetnPoints(); i++) {
    TrackPoint *ptp = pTrack->GetPoint(i);

    PlugIn_Waypoint *dst_wp = new PlugIn_Waypoint();

    dst_wp->m_lat = ptp->m_lat;
    dst_wp->m_lon = ptp->m_lon;
    dst_wp->m_CreateTime = ptp->GetCreateTime();  // not const

    dst_track->pWaypointList->Append(dst_wp);
  }

  return tk;
}

wxWindow *PluginGetFocusCanvas() { return g_focusCanvas; }

wxWindow *PluginGetOverlayRenderCanvas() {
  // if(g_overlayCanvas)
  return g_overlayCanvas;
  // else
}

void CanvasJumpToPosition(wxWindow *canvas, double lat, double lon,
                          double scale) {
  ChartCanvas *oCanvas = wxDynamicCast(canvas, ChartCanvas);
  if (oCanvas) gFrame->JumpToPosition(oCanvas, lat, lon, scale);
}

bool ShuttingDown(void) { return g_bquiting; }

wxWindow *GetCanvasUnderMouse(void) { return gFrame->GetCanvasUnderMouse(); }

int GetCanvasIndexUnderMouse(void) {
  ChartCanvas *l_canvas = gFrame->GetCanvasUnderMouse();
  if (l_canvas) {
    for (unsigned int i = 0; i < g_canvasArray.GetCount(); ++i) {
      if (l_canvas == g_canvasArray[i]) return i;
    }
  }
  return 0;
}

// std::vector<wxWindow *> GetCanvasArray()
// {
//     std::vector<wxWindow *> rv;
//     for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
//         ChartCanvas *cc = g_canvasArray.Item(i);
//         rv.push_back(cc);
//     }
//
//     return rv;
// }

wxWindow *GetCanvasByIndex(int canvasIndex) {
  if (g_canvasConfig == 0)
    return gFrame->GetPrimaryCanvas();
  else {
    if ((canvasIndex >= 0) && g_canvasArray[canvasIndex]) {
      return g_canvasArray[canvasIndex];
    }
  }
  return NULL;
}

bool CheckMUIEdgePan_PlugIn(int x, int y, bool dragging, int margin, int delta,
                            int canvasIndex) {
  if (g_canvasConfig == 0)
    return gFrame->GetPrimaryCanvas()->CheckEdgePan(x, y, dragging, margin,
                                                    delta);
  else {
    if ((canvasIndex >= 0) && g_canvasArray[canvasIndex]) {
      return g_canvasArray[canvasIndex]->CheckEdgePan(x, y, dragging, margin,
                                                      delta);
    }
  }

  return false;
}

void SetMUICursor_PlugIn(wxCursor *pCursor, int canvasIndex) {
  if (g_canvasConfig == 0)
    gFrame->GetPrimaryCanvas()->pPlugIn_Cursor = pCursor;
  else {
    if ((canvasIndex >= 0) && g_canvasArray[canvasIndex]) {
      g_canvasArray[canvasIndex]->pPlugIn_Cursor = pCursor;
    }
  }
}

int GetCanvasCount() {
  if (g_canvasConfig == 1) return 2;
  //     else
  return 1;
}

int GetLatLonFormat() { return g_iSDMMFormat; }

wxRect GetMasterToolbarRect() {
  if (g_MainToolbar)
    return g_MainToolbar->GetRect();
  else
    return wxRect(0, 0, 1, 1);
}

/* API 1.17 */

void ZeroXTE() {
  if (g_pRouteMan) {
    g_pRouteMan->ZeroCurrentXTEToActivePoint();
  }
}

ListOfPI_S57Obj *PlugInManager::GetLightsObjRuleListVisibleAtLatLon(
    ChartPlugInWrapper *target, float zlat, float zlon, const ViewPort &vp) {
  ListOfPI_S57Obj *list = NULL;
  if (target) {
    PlugInChartBaseGLPlus2 *picbgl =
        dynamic_cast<PlugInChartBaseGLPlus2 *>(target->GetPlugInChart());
    if (picbgl) {
      PlugIn_ViewPort pi_vp = CreatePlugInViewport(vp);
      list = picbgl->GetLightsObjRuleListVisibleAtLatLon(zlat, zlon, &pi_vp);

      return list;
    }
    PlugInChartBaseExtendedPlus2 *picbx =
        dynamic_cast<PlugInChartBaseExtendedPlus2 *>(target->GetPlugInChart());
    if (picbx) {
      PlugIn_ViewPort pi_vp = CreatePlugInViewport(vp);
      list = picbx->GetLightsObjRuleListVisibleAtLatLon(zlat, zlon, &pi_vp);

      return list;
    } else
      return list;
  } else
    return list;
}

//      PlugInWaypointEx implementation
WX_DEFINE_LIST(Plugin_WaypointExList);

//  The class implementations
PlugIn_Waypoint_Ex::PlugIn_Waypoint_Ex() { InitDefaults(); }

PlugIn_Waypoint_Ex::PlugIn_Waypoint_Ex(
    double lat, double lon, const wxString &icon_ident, const wxString &wp_name,
    const wxString &GUID, const double ScaMin, const bool bNameVisible,
    const int nRangeRings, const double RangeDistance,
    const wxColor RangeColor) {
  InitDefaults();

  wxDateTime now = wxDateTime::Now();
  m_CreateTime = now.ToUTC();
  m_HyperlinkList = NULL;

  m_lat = lat;
  m_lon = lon;
  IconName = icon_ident;
  m_MarkName = wp_name;
  m_GUID = GUID;
  scamin = ScaMin;
  IsNameVisible = bNameVisible;
  nrange_rings = nRangeRings;
  RangeRingSpace = RangeDistance;
  RangeRingColor = RangeColor;
}

void PlugIn_Waypoint_Ex::InitDefaults() {
  m_HyperlinkList = NULL;
  scamin = 1e9;
  b_useScamin = false;
  nrange_rings = 0;
  RangeRingSpace = 1;
  IsNameVisible = false;
  IsVisible = true;
  RangeRingColor = *wxBLACK;
  m_CreateTime = wxDateTime::Now();
  IsActive = false;
  m_lat = 0;
  m_lon = 0;
}

bool PlugIn_Waypoint_Ex::GetFSStatus() {
  RoutePoint *prp = pWayPointMan->FindRoutePointByGUID(m_GUID);
  if (!prp) return false;

  if (prp->m_bIsInRoute && !prp->IsShared()) return false;

  return true;
}

int PlugIn_Waypoint_Ex::GetRouteMembershipCount() {
  // Search all routes to count the membership of this point
  RoutePoint *pWP = pWayPointMan->FindRoutePointByGUID(m_GUID);
  if (!pWP) return 0;

  int nCount = 0;
  wxRouteListNode *node = pRouteList->GetFirst();
  while (node) {
    Route *proute = node->GetData();
    wxRoutePointListNode *pnode = (proute->pRoutePointList)->GetFirst();
    while (pnode) {
      RoutePoint *prp = pnode->GetData();
      if (prp == pWP) nCount++;
      pnode = pnode->GetNext();
    }

    node = node->GetNext();
  }

  return nCount;
}

PlugIn_Waypoint_Ex::~PlugIn_Waypoint_Ex() {}

//      PlugInRouteExtended implementation
PlugIn_Route_Ex::PlugIn_Route_Ex(void) {
  pWaypointList = new Plugin_WaypointExList;
}

PlugIn_Route_Ex::~PlugIn_Route_Ex(void) {
  pWaypointList->DeleteContents(false);  // do not delete Waypoints
  pWaypointList->Clear();

  delete pWaypointList;
}

//  The utility methods implementations

// translate O route class to PlugIn_Waypoint_Ex
static void PlugInExFromRoutePoint(PlugIn_Waypoint_Ex *dst,
                                   /* const*/ RoutePoint *src) {
  dst->m_lat = src->m_lat;
  dst->m_lon = src->m_lon;
  dst->IconName = src->GetIconName();
  dst->m_MarkName = src->GetName();
  dst->m_MarkDescription = src->GetDescription();
  dst->IconDescription = pWayPointMan->GetIconDescription(src->GetIconName());
  dst->IsVisible = src->IsVisible();
  dst->m_CreateTime = src->GetCreateTime();  // not const
  dst->m_GUID = src->m_GUID;

  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList == nullptr) return;

  delete dst->m_HyperlinkList;
  dst->m_HyperlinkList = nullptr;

  if (src->m_HyperlinkList->GetCount() > 0) {
    dst->m_HyperlinkList = new Plugin_HyperlinkList;

    wxHyperlinkListNode *linknode = src->m_HyperlinkList->GetFirst();
    while (linknode) {
      Hyperlink *link = linknode->GetData();

      Plugin_Hyperlink *h = new Plugin_Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->Type = link->LType;

      dst->m_HyperlinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }

  // Get the range ring info
  dst->nrange_rings = src->m_iWaypointRangeRingsNumber;
  dst->RangeRingSpace = src->m_fWaypointRangeRingsStep;
  dst->RangeRingColor = src->m_wxcWaypointRangeRingsColour;

  // Get other extended info
  dst->IsNameVisible = src->m_bShowName;
  dst->scamin = src->GetScaMin();
  dst->b_useScamin = src->GetUseSca();
  dst->IsActive = src->m_bIsActive;
}

static void cloneHyperlinkListEx(RoutePoint *dst,
                                 const PlugIn_Waypoint_Ex *src) {
  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList == nullptr) return;

  if (src->m_HyperlinkList->GetCount() > 0) {
    wxPlugin_HyperlinkListNode *linknode = src->m_HyperlinkList->GetFirst();
    while (linknode) {
      Plugin_Hyperlink *link = linknode->GetData();

      Hyperlink *h = new Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->LType = link->Type;

      dst->m_HyperlinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }
}

RoutePoint *CreateNewPoint(const PlugIn_Waypoint_Ex *src, bool b_permanent) {
  RoutePoint *pWP = new RoutePoint(src->m_lat, src->m_lon, src->IconName,
                                   src->m_MarkName, src->m_GUID);

  pWP->m_bIsolatedMark = true;  // This is an isolated mark

  cloneHyperlinkListEx(pWP, src);

  pWP->m_MarkDescription = src->m_MarkDescription;

  if (src->m_CreateTime.IsValid())
    pWP->SetCreateTime(src->m_CreateTime);
  else {
    wxDateTime dtnow(wxDateTime::Now());
    pWP->SetCreateTime(dtnow);
  }

  pWP->m_btemp = (b_permanent == false);

  // Extended fields
  pWP->SetIconName(src->IconName);
  pWP->SetWaypointRangeRingsNumber(src->nrange_rings);
  pWP->SetWaypointRangeRingsStep(src->RangeRingSpace);
  pWP->SetWaypointRangeRingsColour(src->RangeRingColor);
  pWP->SetScaMin(src->scamin);
  pWP->SetUseSca(src->b_useScamin);
  pWP->SetNameShown(src->IsNameVisible);
  pWP->SetVisible(src->IsVisible);

  return pWP;
}
bool GetSingleWaypointEx(wxString GUID, PlugIn_Waypoint_Ex *pwaypoint) {
  //  Find the RoutePoint
  RoutePoint *prp = pWayPointMan->FindRoutePointByGUID(GUID);

  if (!prp) return false;

  PlugInExFromRoutePoint(pwaypoint, prp);

  return true;
}

bool AddSingleWaypointEx(PlugIn_Waypoint_Ex *pwaypointex, bool b_permanent) {
  //  Validate the waypoint parameters a little bit

  //  GUID
  //  Make sure that this GUID is indeed unique in the Routepoint list
  bool b_unique = true;
  wxRoutePointListNode *prpnode = pWayPointMan->GetWaypointList()->GetFirst();
  while (prpnode) {
    RoutePoint *prp = prpnode->GetData();

    if (prp->m_GUID == pwaypointex->m_GUID) {
      b_unique = false;
      break;
    }
    prpnode = prpnode->GetNext();  // RoutePoint
  }

  if (!b_unique) return false;

  RoutePoint *pWP = CreateNewPoint(pwaypointex, b_permanent);

  pSelect->AddSelectableRoutePoint(pWP->m_lat, pWP->m_lon, pWP);
  if (b_permanent) pConfig->AddNewWayPoint(pWP, -1);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();

  return true;
}

bool UpdateSingleWaypointEx(PlugIn_Waypoint_Ex *pwaypoint) {
  //  Find the RoutePoint
  bool b_found = false;
  RoutePoint *prp = pWayPointMan->FindRoutePointByGUID(pwaypoint->m_GUID);

  if (prp) b_found = true;

  if (b_found) {
    double lat_save = prp->m_lat;
    double lon_save = prp->m_lon;

    prp->m_lat = pwaypoint->m_lat;
    prp->m_lon = pwaypoint->m_lon;
    prp->SetIconName(pwaypoint->IconName);
    prp->SetName(pwaypoint->m_MarkName);
    prp->m_MarkDescription = pwaypoint->m_MarkDescription;
    prp->SetVisible(pwaypoint->IsVisible);
    if (pwaypoint->m_CreateTime.IsValid())
      prp->SetCreateTime(pwaypoint->m_CreateTime);

    //  Transcribe (clone) the html HyperLink List, if present

    if (pwaypoint->m_HyperlinkList) {
      prp->m_HyperlinkList->Clear();
      if (pwaypoint->m_HyperlinkList->GetCount() > 0) {
        wxPlugin_HyperlinkListNode *linknode =
            pwaypoint->m_HyperlinkList->GetFirst();
        while (linknode) {
          Plugin_Hyperlink *link = linknode->GetData();

          Hyperlink *h = new Hyperlink();
          h->DescrText = link->DescrText;
          h->Link = link->Link;
          h->LType = link->Type;

          prp->m_HyperlinkList->Append(h);

          linknode = linknode->GetNext();
        }
      }

      // Extended fields
      prp->SetWaypointRangeRingsNumber(pwaypoint->nrange_rings);
      prp->SetWaypointRangeRingsStep(pwaypoint->RangeRingSpace);
      prp->SetWaypointRangeRingsColour(pwaypoint->RangeRingColor);
      prp->SetScaMin(pwaypoint->scamin);
      prp->SetUseSca(pwaypoint->b_useScamin);
      prp->SetNameShown(pwaypoint->IsNameVisible);
    }

    if (prp) prp->ReLoadIcon();

    SelectItem *pFind = pSelect->FindSelection(
        gFrame->GetPrimaryCanvas(), lat_save, lon_save, SELTYPE_ROUTEPOINT);
    if (pFind) {
      pFind->m_slat = pwaypoint->m_lat;  // update the SelectList entry
      pFind->m_slon = pwaypoint->m_lon;
    }

    if (!prp->m_btemp) pConfig->UpdateWayPoint(prp);

    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateWptListCtrl();
  }

  return b_found;
}

bool AddPlugInRouteEx(PlugIn_Route_Ex *proute, bool b_permanent) {
  Route *route = new Route();

  PlugIn_Waypoint_Ex *pwaypointex;
  RoutePoint *pWP, *pWP_src;
  int ip = 0;
  wxDateTime plannedDeparture;

  wxPlugin_WaypointExListNode *pwpnode = proute->pWaypointList->GetFirst();
  while (pwpnode) {
    pwaypointex = pwpnode->GetData();

    pWP = pWayPointMan->FindRoutePointByGUID(pwaypointex->m_GUID);
    if (!pWP) {
      pWP = CreateNewPoint(pwaypointex, b_permanent);
      pWP->m_bIsolatedMark = false;
    }

    route->AddPoint(pWP);

    pSelect->AddSelectableRoutePoint(pWP->m_lat, pWP->m_lon, pWP);

    if (ip > 0)
      pSelect->AddSelectableRouteSegment(pWP_src->m_lat, pWP_src->m_lon,
                                         pWP->m_lat, pWP->m_lon, pWP_src, pWP,
                                         route);

    plannedDeparture = pwaypointex->m_CreateTime;
    ip++;
    pWP_src = pWP;

    pwpnode = pwpnode->GetNext();  // PlugInWaypoint
  }

  route->m_PlannedDeparture = plannedDeparture;

  route->m_RouteNameString = proute->m_NameString;
  route->m_RouteStartString = proute->m_StartString;
  route->m_RouteEndString = proute->m_EndString;
  if (!proute->m_GUID.IsEmpty()) {
    route->m_GUID = proute->m_GUID;
  }
  route->m_btemp = (b_permanent == false);

  pRouteList->Append(route);

  if (b_permanent) pConfig->AddNewRoute(route);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateRouteListCtrl();

  return true;
}

bool UpdatePlugInRouteEx(PlugIn_Route_Ex *proute) {
  bool b_found = false;

  //  Find the Route
  Route *pRoute = g_pRouteMan->FindRouteByGUID(proute->m_GUID);
  if (pRoute) b_found = true;

  if (b_found) {
    bool b_permanent = (pRoute->m_btemp == false);
    g_pRouteMan->DeleteRoute(pRoute);

    b_found = AddPlugInRouteEx(proute, b_permanent);
  }

  return b_found;
}

// std::unique_ptr<PlugIn_Waypoint_Ex> GetWaypointEx_Plugin(const wxString &)
// {
// }

// std::unique_ptr<PlugIn_Route_Ex> GetRouteEx_Plugin(const wxString &)
// {
// }

std::unique_ptr<PlugIn_Waypoint_Ex> GetWaypointEx_Plugin(const wxString &GUID) {
  std::unique_ptr<PlugIn_Waypoint_Ex> w(new PlugIn_Waypoint_Ex);
  GetSingleWaypointEx(GUID, w.get());
  return w;
}

std::unique_ptr<PlugIn_Route_Ex> GetRouteEx_Plugin(const wxString &GUID) {
  std::unique_ptr<PlugIn_Route_Ex> r;
  Route *route = g_pRouteMan->FindRouteByGUID(GUID);
  if (route == nullptr) return r;

  r = std::unique_ptr<PlugIn_Route_Ex>(new PlugIn_Route_Ex);
  PlugIn_Route_Ex *dst_route = r.get();

  // PlugIn_Waypoint *pwp;
  RoutePoint *src_wp;
  wxRoutePointListNode *node = route->pRoutePointList->GetFirst();

  while (node) {
    src_wp = node->GetData();

    PlugIn_Waypoint_Ex *dst_wp = new PlugIn_Waypoint_Ex();
    PlugInExFromRoutePoint(dst_wp, src_wp);

    dst_route->pWaypointList->Append(dst_wp);

    node = node->GetNext();
  }
  dst_route->m_NameString = route->m_RouteNameString;
  dst_route->m_StartString = route->m_RouteStartString;
  dst_route->m_EndString = route->m_RouteEndString;
  dst_route->m_GUID = route->m_GUID;
  dst_route->m_isActive = g_pRouteMan->GetpActiveRoute() == route;

  return r;
}

wxString GetActiveWaypointGUID(
    void) {  // if no active waypoint, returns wxEmptyString
  RoutePoint *rp = g_pRouteMan->GetpActivePoint();
  if (!rp)
    return wxEmptyString;
  else
    return rp->m_GUID;
}

wxString GetActiveRouteGUID(
    void) {  // if no active route, returns wxEmptyString
  Route *rt = g_pRouteMan->GetpActiveRoute();
  if (!rt)
    return wxEmptyString;
  else
    return rt->m_GUID;
}
