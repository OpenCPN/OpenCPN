/******************************************************************************
 *
 * Project:  OpenCPN
 *
 * Purpose:  Simple CLI application.
 *
 ***************************************************************************
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
 ***************************************************************************
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "config.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/app.h>
#include <wx/bitmap.h>
#include <wx/cmdline.h>
#include <wx/dynlib.h>
#include <wx/fileconf.h>
#include <wx/init.h>
#include <wx/string.h>

#include "catalog_handler.h"
#include "cli_platform.h"
#include "comm_appmsg_bus.h"
#include "comm_driver.h"
#include "comm_navmsg_bus.h"
#include "config_vars.h"
#include "downloader.h"
#include "nmea_log.h"
#include "observable_evtvar.h"
#include "ocpn_utils.h"
#include "plugin_handler.h"
#include "plugin_loader.h"
#include "routeman.h"
#include "S57ClassRegistrar.h"
#include "select.h"
#include "track.h"

class AISTargetAlertDialog;
class Multiplexer;
class Select;

BasePlatform* g_BasePlatform = 0;
void* g_pi_manager = reinterpret_cast<void*>(1L);
wxString g_compatOS = PKG_TARGET;
wxString g_compatOsVersion = PKG_TARGET_VERSION;

wxString g_catalog_custom_url;
wxString g_catalog_channel;
wxLog* g_logger;

bool g_bAIS_ACK_Timeout;
bool g_bAIS_CPA_Alert_Suppress_Moored;
bool g_bCPAMax;
bool g_bCPAWarn;
bool g_bHideMoored;
bool g_bTCPA_Max;
double g_AckTimeout_Mins;
double g_CPAMax_NM;
double g_CPAWarn_NM;
double g_ShowMoored_Kts;
double g_TCPA_Max;
bool g_bShowMag;
bool g_bShowTrue;
bool bGPSValid;
bool g_bInlandEcdis;
bool g_bRemoveLost;
bool g_bMarkLost;
bool g_bShowScaled;
bool g_bAllowShowScaled;
bool g_bAISRolloverShowCOG;
bool g_bAISRolloverShowCPA;
bool g_bAISShowTracks;
bool g_bAISRolloverShowClass;
bool g_bAIS_CPA_Alert;
double g_RemoveLost_Mins;
double g_MarkLost_Mins;
double g_AISShowTracks_Mins;
float g_selection_radius_mm;
float g_selection_radius_touch_mm;
int g_nCOMPortCheck = 32;
bool g_benableUDPNullHeader;

S57ClassRegistrar *g_poRegistrar;

std::vector<Track*> g_TrackList;
wxString AISTargetNameFileName;
Route* pAISMOBRoute;
int g_WplAction;
Select* pSelectAIS;

/* comm_bridge context. */

Select* pSelect;
double g_n_arrival_circle_radius;
double g_PlanSpeed;
bool g_bTrackDaily;
int g_trackFilterMax;
wxString g_default_routepoint_icon;
double g_TrackDeltaDistance;
float g_fWaypointRangeRingsStep;
float g_ChartScaleFactorExp;
float g_MarkScaleFactorExp;
wxString g_default_wp_icon;
bool g_btouch;
int g_iWaypointRangeRingsNumber;
int g_iWaypointRangeRingsStepUnits;
wxColour g_colourWaypointRangeRingsColour;
bool g_bUseWptScaMin;
bool g_bShowWptName;
int g_iWpt_ScaMin;
int g_LayerIdx;
bool g_bOverruleScaMin;
int g_nTrackPrecision;
bool g_bIsNewLayer;
RouteList* pRouteList;
WayPointman* pWayPointMan;
int g_route_line_width;
int g_track_line_width;
RoutePoint* pAnchorWatchPoint1 = 0;
RoutePoint* pAnchorWatchPoint2 = 0;
bool g_bAllowShipToActive;
wxRect g_blink_rect;
bool g_bMagneticAPB;

Routeman* g_pRouteMan;

class NmeaLogDummy: public NmeaLog {
  bool Active() const { return false; }
  void Add(const wxString& s) {};
};

static void InitRouteman() {
  struct RoutePropDlgCtx ctx;
  auto RouteMgrDlgUpdateListCtrl = [&]() {};
  static  NmeaLogDummy dummy_log;
  g_pRouteMan = new Routeman(ctx, RoutemanDlgCtx(), dummy_log);
}

namespace safe_mode {
bool get_mode() { return false; }
}  // namespace safe_mode

static const char* USAGE = R"""(
Usage: opencpn-cli [options] <command>

Options:

  -v, --verbose:
      Verbose logging.

  -a, --abi <abi-spec>
      Use non-default ABI. <abi-spec> is a string platform:version for
      example 'ubuntu-gtk3-x86_64:20.04'.

Commands:
  load-plugin <plugin library file>:
      Try to load a given library file, report possible errors.

  install-plugin <plugin name>:
      Download and install given plugin

  uninstall-plugin <plugin name>:
      Uninstall given plugin

  list-plugins:
      List name and version for all installed plugins

  list-available:
      List name and version for available plugins in catalog.

  import-plugin <tarball file>
      Import a given tarball file

  print-abi:
      print default ABI spec  platform:version

  update-catalog:
     Download latest master catalog.

  plugin-by-file <filename>
     Print name of a plugin containing file or "not found"

)""";

static const char* const DOWNLOAD_REPO_PROTO =
    "https://raw.githubusercontent.com/OpenCPN/plugins/@branch@/"
    "ocpn-plugins.xml";

wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
wxDEFINE_EVENT(EVT_BAR, wxCommandEvent);

class CliApp : public wxAppConsole {
public:
  CliApp() : wxAppConsole() {
    CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");
    SetAppName("opencpn");
  }

  void OnInitCmdLine(wxCmdLineParser& parser) override {
    parser.AddOption("a", "abi", "abi:version e. g., \"ubuntu-x86_64:20.04\"");
    parser.AddSwitch("v", "verbose", "Verbose logging");
    parser.AddSwitch("h", "help", "Print help");
    parser.AddParam("<command>", wxCMD_LINE_VAL_STRING,
                    wxCMD_LINE_PARAM_OPTIONAL);
    parser.AddParam("[arg]", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

    wxLog::SetActiveTarget(new wxLogStderr);
    wxLog::SetTimestamp("");
    wxLog::SetLogLevel(wxLOG_Warning);

    g_BasePlatform = new BasePlatform();
    auto config_file = g_BasePlatform->GetConfigFileName();
    InitBaseConfig(new wxFileConfig("", "", config_file));
    pSelect = new Select();
    pRouteList = new RouteList;
    InitRouteman();
    auto colour_func = [] (wxString c) { return *wxBLACK; };
    pWayPointMan = new WayPointman(colour_func);
  }

  void list_plugins() {
    using namespace std;
    wxImage::AddHandler(new wxPNGHandler());
    g_BasePlatform->GetSharedDataDir();  // See #2619
    PluginLoader::getInstance()->LoadAllPlugIns(false);
    auto plugins = PluginHandler::getInstance()->getInstalled();
    for (const auto& p : plugins) {
      if (p.version == "0.0") continue;
      auto path = PluginHandler::ImportedMetadataPath(p.name);
      std::string suffix(ocpn::exists(path) ? "[imported]" : "");
      cout << left << setw(25) << p.name << p.version << suffix  << "\n";
    }
  }

  void list_available() {
    using namespace std;
    auto handler = PluginHandler::getInstance();
    auto plugins = handler->getAvailable();
    for (const auto& p : plugins) {
      if (handler->isCompatible(p)) {
        cout << left << setw(25) << p.name << p.version << "\n";
      }
    }
  }

  void uninstall_plugin(const std::string& plugin) {
    using namespace std;
    g_BasePlatform->GetSharedDataDir();  // See #2619
    PluginLoader::getInstance()->LoadAllPlugIns(false);
    auto plugins = PluginHandler::getInstance()->getInstalled();
    vector<PluginMetadata> found;
    copy_if(plugins.begin(), plugins.end(), back_inserter(found),
            [plugin](const PluginMetadata& m) { return m.name == plugin; });
    if (found.size() == 0) {
      cerr << "No such plugin installed\n";
      exit(2);
    }
    PluginHandler::getInstance()->uninstall(found[0].name);
  }

  void import_plugin(const std::string& tarball_path) {
    auto handler = PluginHandler::getInstance();
    PluginMetadata metadata;
    bool ok = handler->ExtractMetadata(tarball_path, metadata);
    if (!ok) {
      std::cerr << "Cannot extract metadata (malformed tarball?)\n";
      exit(2);
    }
    if (!PluginHandler::isCompatible(metadata)) {
      std::cerr << "Incompatible plugin detected\n";
      exit(2);
    }
    ok = handler->installPlugin(metadata, tarball_path);
    if (!ok) {
      std::cerr << "Error extracting import plugin tarball.\n";
      exit(2);
    }
    metadata.is_imported = true;
    auto metadata_path = PluginHandler::ImportedMetadataPath(metadata.name);
    std::ofstream file(metadata_path);
    file << metadata.to_string();
    if (!file.good()) {
       std::cerr << "Error saving metadata file: " << metadata_path
                << " for imported plugin: " << metadata.name;
       exit(2);
    }
    exit(0);
  }

  void install_plugin(const std::string& plugin) {
    using namespace std;
    g_BasePlatform->GetSharedDataDir();  // See #2619
    wxImage::AddHandler(new wxPNGHandler());
    auto handler = PluginHandler::getInstance();
    auto plugins = handler->getAvailable();
    vector<PluginMetadata> found;
    copy_if(plugins.begin(), plugins.end(), back_inserter(found),
            [plugin, handler](const PluginMetadata& m) {
              return m.name == plugin && handler->isCompatible(m);
            });
    if (found.size() == 0) {
      cerr << "No such plugin available\n";
      exit(2);
    }
    Downloader downloader(found[0].tarball_url);
    string path;
    bool ok = downloader.download(path);
    if (!ok) {
      cerr << "Cannot download data from " << found[0].tarball_url << "\n";
      exit(1);
    }
    PluginHandler::getInstance()->installPlugin(path);
    remove(path.c_str());
    exit(0);
  }

  void plugin_by_file(const std::string& filename) {
    auto plugin = PluginHandler::getInstance()->getPluginByLibrary(filename);
    std::cout << (plugin != "" ? plugin : "Not found") << "\n";
  }

  bool load_plugin(const std::string& plugin) {
    auto loader = PluginLoader::getInstance();
    wxImage::AddHandler(new wxPNGHandler());
    g_BasePlatform->GetSharedDataDir();  // See #2619
    wxDEFINE_EVENT(EVT_FILE_NOTFOUND, wxCommandEvent);
    ObservableListener file_notfound_listener;
    file_notfound_listener.Listen(loader->evt_unreadable_plugin, this,
                                  EVT_FILE_NOTFOUND);
    Bind(EVT_FILE_NOTFOUND, [&](wxCommandEvent ev) {
      std::cerr << "Cannot open file: " << ev.GetString() << "\n";
    });

    wxDEFINE_EVENT(EVT_BAD_VERSION, wxCommandEvent);
    ObservableListener bad_version_listener;
    bad_version_listener.Listen(loader->evt_version_incompatible_plugin, this,
                                EVT_BAD_VERSION);
    Bind(EVT_BAD_VERSION, [&](wxCommandEvent ev) {
      std::cerr << "Incompatible plugin version " << ev.GetString() << "\n";
    });

    auto container = loader->LoadPlugIn(plugin.c_str());
    ProcessPendingEvents();
    std::cout << (container ? "File loaded OK\n" : "Load error\n");
    return container ? true : false;
  }

  void check_param_count(const wxCmdLineParser& parser, size_t count) {
    if (parser.GetParamCount() < count) {
      std::cerr << USAGE << "\n";
      exit(1);
    }
  }

  void update_catalog() {
    std::string catalog(g_catalog_channel == "" ? "master" : g_catalog_channel);
    std::string url(g_catalog_custom_url);
    if (catalog != "custom") {
      url = std::string(DOWNLOAD_REPO_PROTO);
      ocpn::replace(url, "@branch@", catalog);
    }
    auto path = PluginHandler::getInstance()->getMetadataPath();
    auto cat_handler = CatalogHandler::getInstance();
    auto status = cat_handler->DownloadCatalog(path, url);
    if (status != CatalogHandler::ServerStatus::OK) {
      std::cout << "Cannot update catalog\n";
      exit(1);
    }
  }

  bool OnCmdLineParsed(wxCmdLineParser& parser) override {
    wxInitializer initializer;
    if (!initializer) {
      std::cerr << "Failed to initialize the wxWidgets library, aborting.";
      exit(1);
    }
    wxAppConsole::OnCmdLineParsed(parser);
    if (argc == 1) {
      std::cout << "OpenCPN CLI application. Use -h for help\n";
      exit(0);
    }
    wxString option_val;
    if (parser.Found("help")) {
      std::cout << USAGE << "\n";
      exit(0);
    }
    if (parser.Found("verbose")) wxLog::SetLogLevel(wxLOG_Debug);
    if (parser.Found("abi", &option_val)) {
      auto abi_vers = ocpn::split(option_val, ":");
      g_compatOS = abi_vers[0];
      if (abi_vers.size() > 1) g_compatOsVersion = abi_vers[1];
    }
    if (parser.GetParamCount() < 1) {
      std::cerr << USAGE << "\n";
      exit(1);
    }
    std::string command(parser.GetParam(0));
    if (command == "load-plugin") {
      check_param_count(parser, 2);
      load_plugin(parser.GetParam(1).ToStdString());
    } else if (command == "uninstall-plugin") {
      check_param_count(parser, 2);
      uninstall_plugin(parser.GetParam(1).ToStdString());
    } else if (command == "import-plugin") {
      check_param_count(parser, 2);
      import_plugin(parser.GetParam(1).ToStdString());
    } else if (command == "install-plugin") {
      check_param_count(parser, 2);
      install_plugin(parser.GetParam(1).ToStdString());
    } else if (command == "list-plugins") {
      check_param_count(parser, 1);
      list_plugins();
    } else if (command == "list-available") {
      check_param_count(parser, 1);
      list_available();
    } else if (command == "print-abi") {
      check_param_count(parser, 1);
      std::cout << PKG_TARGET << ":" << PKG_TARGET_VERSION "\n";
    } else if (command == "update-catalog") {
      check_param_count(parser, 1);
      update_catalog();
    } else if (command == "plugin-by-file") {
      check_param_count(parser, 2);
      plugin_by_file(parser.GetParam(1).ToStdString());
    } else {
      std::cerr << USAGE << "\n";
      exit(2);
    }
    exit(0);
  }
};

wxIMPLEMENT_APP_CONSOLE(CliApp);
