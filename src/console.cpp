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

#include "config.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
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
#include <wx/string.h>

#include "BasePlatform.h"
#include "catalog_handler.h"
#include "ocpn_utils.h"
#include "Downloader.h"
#include "observable_msg.h"
#include "drivers.h"
#include "plugin_loader.h"
#include "PluginHandler.h"
#include "commTransport.h"

BasePlatform* g_BasePlatform = 0;
bool g_bportable = false;
wxString g_winPluginDir;
wxConfigBase* pBaseConfig = 0;
void* g_pi_manager = reinterpret_cast<void*>(1L);
wxString g_compatOS = PKG_TARGET;
wxString g_compatOsVersion = PKG_TARGET_VERSION;

wxString g_catalog_custom_url;
wxString g_catalog_channel;
wxLog* g_logger;

namespace safe_mode { bool get_mode() { return false; } }

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

)""";

static const char *const DOWNLOAD_REPO_PROTO =
    "https://raw.githubusercontent.com/OpenCPN/plugins/@branch@/"
    "ocpn-plugins.xml";

wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
wxDEFINE_EVENT(EVT_BAR, wxCommandEvent);


class Message {
public:
  Message() : payload("foobar") {};
  virtual  ~Message() { std::cout << "Message: DTOR\n"; }
  const std::string payload;
};


class MessagePtr {
public:
  MessagePtr(std::unique_ptr<Message> p) {ptr = std::move(p); }
  std::unique_ptr<Message> ptr;
  virtual  ~MessagePtr() { std::cout << "MessagePtr: DTOR\n"; }
};


class MsgSink:  public wxEvtHandler {
private:
  ObservedVarListener listener;
public:
  MsgSink() {
    ObservableMsg observable("1234");
    listener = observable.get_listener(this, EVT_BAR);
    Bind(EVT_BAR, [&](wxCommandEvent ev) {
      auto msg = get_message_ptr(ev);
      std::cout << (msg->bus == NavBus::n2k ? "Type: N2k\n" : "wrong type\n");
      auto n2000_msg = std::dynamic_pointer_cast<Nmea2000Msg>(msg);
      std::string s(n2000_msg->payload.begin(), n2000_msg->payload.end());
      std::cout << "Payload: " << s << "\n";
    });
  }
};


class TransportSource {
public:
  TransportSource(wxEvtHandler& sink) {
    std::string s("payload data");
    auto payload = std::vector<unsigned char>(s.begin(), s.end());
    auto msg = new Nmea2000Msg(static_cast<uint64_t>(1234), payload);
    auto t = Transport::getInstance();
    t->notify(*msg);
  }
};


class TransportSink: public wxEvtHandler {
public:
  TransportSink() {
    auto t = Transport::getInstance();
    Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234));
    listener = t->get_listener(EVT_FOO, this, n2k_msg.key());

    Bind(EVT_FOO, [&](wxCommandEvent ev) {
      std::cout << "EVT_FOO: received\n" ;
      auto message = get_message_ptr(ev);
      auto n2k_msg = std::dynamic_pointer_cast<Nmea2000Msg>(message);
      std::string s(n2k_msg->payload.begin(), n2k_msg->payload.end());
      std::cout << "payload: " + s + "\n";
    });
  }
  ObservedVarListener listener;
};


class MsgSource {
public:
  MsgSource(wxEvtHandler& sink) {
    std::string s("payload data");
    auto payload = std::vector<unsigned char>(s.begin(), s.end());
    Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234), payload);

    ObservableMsg observable("1234");
    observable.notify(n2k_msg);
  }
};

class Sink: public wxEvtHandler {
public:
  Sink() {
    Bind(EVT_FOO, [&](wxCommandEvent ev) {
      std::cout << "EVT_FOO: received\n" ;
      auto message = static_cast<MessagePtr*>(ev.GetClientData());
      auto ptr = std::move(message->ptr);
      std::cout << "payload: " + ptr->payload + "\n";
      delete message;
    });
  }
};

class Source {
public:
  Source(wxEvtHandler& sink) {
    auto evt = new wxCommandEvent(EVT_FOO);
    evt->SetString("FOO string");
    auto message = new Message();
    auto ptr = std::unique_ptr<Message>(message);
    auto message_ptr = new MessagePtr(std::move(ptr));
    evt->SetClientData(message_ptr);
    wxQueueEvent(&sink, evt);
  }
};

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
    parser.AddParam("<command>",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
    parser.AddParam("[arg]", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

    wxLog::SetActiveTarget(new wxLogStderr);
    wxLog::SetTimestamp("");
    wxLog::SetLogLevel(wxLOG_Warning);

    g_BasePlatform = new BasePlatform();
    auto config_file = g_BasePlatform->GetConfigFileName();
    pBaseConfig = new wxFileConfig("", "", config_file);
  }

  void list_plugins() {
    using namespace std;
    wxImage::AddHandler(new wxPNGHandler());
    g_BasePlatform->GetSharedDataDir();    // See #2619
    PluginLoader::getInstance()->LoadAllPlugIns(false);
    auto plugins = PluginHandler::getInstance()->getInstalled();
    for (const auto& p : plugins) {
      if (p.version == "0.0") continue;
      cout << left << setw(25) << p.name << p.version << "\n";
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
    g_BasePlatform->GetSharedDataDir();    // See #2619
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
    PluginHandler::getInstance()->installPlugin(tarball_path);
    exit(0);
  }

  void install_plugin(const std::string& plugin) {
    using namespace std;
    g_BasePlatform->GetSharedDataDir();   // See #2619
    wxImage::AddHandler(new wxPNGHandler());
    auto handler = PluginHandler::getInstance();
    auto plugins = handler->getAvailable();
    vector<PluginMetadata> found;
    copy_if(plugins.begin(), plugins.end(), back_inserter(found),
            [plugin, handler](const PluginMetadata& m) {
                return m.name == plugin && handler->isCompatible(m); });
    if (found.size() == 0) {
      cerr << "No such plugin available\n";
      exit(2);
    }
    Downloader downloader(found[0].tarball_url);
    string path;
    bool ok = downloader.download(path);
    if (!ok)  {
      cerr << "Cannot download data from " << found[0].tarball_url << "\n";
      exit(1);
    }
    PluginHandler::getInstance()->installPlugin(path);
    remove(path.c_str());
    exit(0);
  }

  bool test_transport() {
    TransportSink sink;
    TransportSource source(sink);
    ProcessPendingEvents();
    exit(0);
  }

  bool test_voidptr() {
    Sink sink;
    Source source(sink);
    ProcessPendingEvents();
    exit(0);
  }

  bool test_observable() {
    MsgSink sink;
    MsgSource source(sink);
    ProcessPendingEvents();
    exit(0);
  }

  bool load_plugin(const std::string& plugin) {
    auto loader = PluginLoader::getInstance();
    wxImage::AddHandler(new wxPNGHandler());
    g_BasePlatform->GetSharedDataDir();   // See #2619
    wxDEFINE_EVENT(EVT_FILE_NOTFOUND, wxCommandEvent);
    auto file_notfound_listener =
        loader->evt_unreadable_plugin.get_listener(this, EVT_FILE_NOTFOUND);
    Bind(EVT_FILE_NOTFOUND, [&](wxCommandEvent ev) {
      std::cerr << "Cannot open file: " << ev.GetString() << "\n";
    });

    wxDEFINE_EVENT(EVT_BAD_VERSION, wxCommandEvent);
    auto bad_version_listener =
        loader->evt_version_incompatible_plugin.get_listener(this,
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

  bool OnCmdLineParsed(wxCmdLineParser& parser) {
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
    if (command == "test-voidptr") {
      check_param_count(parser, 0);
      test_voidptr();
    }
    else if (command == "test-transport") {
      check_param_count(parser, 0);
      test_transport();
    }
    else if (command == "test-observable") {
      check_param_count(parser, 0);
      test_observable();
    }
    else if (command == "load-plugin") {
      check_param_count(parser, 2);
      load_plugin(parser.GetParam(1).ToStdString());
    }
    else if (command == "uninstall-plugin") {
      check_param_count(parser, 2);
      uninstall_plugin(parser.GetParam(1).ToStdString());
    }
    else if (command == "import-plugin") {
      check_param_count(parser, 2);
      import_plugin(parser.GetParam(1).ToStdString());
    }
    else if (command == "install-plugin") {
      check_param_count(parser, 2);
      install_plugin(parser.GetParam(1).ToStdString());
    }
    else if (command == "list-plugins") {
      check_param_count(parser, 1);
      list_plugins();
    }
    else if (command == "list-available") {
      check_param_count(parser, 1);
      list_available();
    }
    else if (command == "print-abi") {
      check_param_count(parser, 1);
      std::cout << PKG_TARGET << ":" << PKG_TARGET_VERSION "\n";
    }
    else if (command == "update-catalog") {
      check_param_count(parser, 1);
      update_catalog();
    }
    else {
      std::cerr << USAGE << "\n";
      exit(2);
    }
    exit(0);
  }
};

wxIMPLEMENT_APP_CONSOLE(CliApp);
