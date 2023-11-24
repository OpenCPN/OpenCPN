#include "config.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <wx/event.h>
#include <wx/app.h>
#include <wx/fileconf.h>
#include <wx/log.h>

#include <gtest/gtest.h>

#include "base_platform.h"
#include "certificates.h"
#include "config_vars.h"
#include "mDNS_query.h"
#include "observable_confvar.h"
#include "ocpn_types.h"
#include "rest_server.h"
#include "routeman.h"

namespace fs = std::filesystem;
using namespace std::chrono_literals;

extern WayPointman* pWayPointMan;
extern RouteList* pRouteList;
extern Select* pSelect;
extern BasePlatform* g_BasePlatform;

static std::string s_result;
static int int_result0;

static void ConfigSetup() {
  const auto config_orig = fs::path(TESTDATA) / "opencpn.conf";
  const auto config_path = fs::path(CMAKE_BINARY_DIR) / "opencpn.conf";
  std::remove(config_path.string().c_str());
  fs::copy(config_orig, config_path);
  InitBaseConfig(new wxFileConfig("", "", config_path.string()));
}

static bool g_portable = false;
class RestServerApp : public wxAppConsole {
public:
  RestServerApp(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
          : wxAppConsole(),
            m_rest_server(ctx, route_ctx, portable) {}

  void Run() {
    std::vector<std::string> addresses  = get_local_ipv4_addresses();
    auto local_address = addresses[0];
    fs::path dirpath(std::string(CMAKE_BINARY_DIR) + "/certs");
    if (!fs::exists(dirpath)) fs::create_directory(dirpath);
    // Handle buggy make_certificate:
    make_certificate(local_address, dirpath.string() + "/");
    m_rest_server.StartServer(dirpath.string());
    Work();
    ProcessPendingEvents();
    m_rest_server.StopServer();
  }

protected:
  virtual void Work() { std::this_thread::sleep_for(50ms); }
  RestServer m_rest_server;
};

class RestServerPingApp : public RestServerApp {
public:
  RestServerPingApp(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
      : RestServerApp(ctx, route_ctx, portable) {}

protected: 
  void Work() {
    auto path = fs::path(CMAKE_BINARY_DIR) / "curl-result";
    {
      std::stringstream ss;
      ss << CURLPROG << " --insecure -o " << path
        << " https://localhost:8443/api/ping?source=1.2.3.4&apikey=bad";
      system(ss.str().c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(path.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 5}");  // Bad api key
    }{
      std::stringstream ss;
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      ss << CURLPROG << " --insecure -o " << path
        << " \"https://localhost:8443/api/ping?source=1.2.3.4&apikey="
        << key << "\"";
      system(ss.str().c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(path.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 0}");  // ok
    }
  }
};

class RestServerObjectApp : public RestServerApp {
public:
  RestServerObjectApp(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
      : RestServerApp(ctx, route_ctx, portable) {}

protected: 
  void Work() override {
    auto outpath = fs::path(CMAKE_BINARY_DIR) / "curl-result";
    auto datapath = fs::path(TESTDATA) / "foo.gpx";
    {
      // try to transfer route without api key
      std::stringstream ss;
      ss << CURLPROG << " --insecure --silent --data @" << datapath
          << " -o " << outpath <<  " -H \"Content-Type: text/xml\""
          << " https://localhost:8443/api/rx_object?source=1.2.3.4";
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      system(ss.str().c_str());
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 5}");   // New pin required
    } {
      // Try to transfer using api key set up above.
      std::stringstream ss;
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      ss << CURLPROG << " --insecure --silent --data @" << datapath << " -o "
          << outpath <<  " -H \"Content-Type: text/xml\""
          << " \"https://localhost:8443/api/rx_object?source=1.2.3.4"
          << "&apikey=" << key << "\"";
      system(ss.str().c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 0}");     // Ok
    } {
      // Set "find duplicate guid" callback to return true;
      m_rest_server.m_route_ctx.find_route_by_guid = [](wxString guid) {
        auto r = new Route;
        r->m_GUID = guid;
        return r;
      };
      // Set the user "accept overwrite" cb to return false
      m_rest_server.m_dlg_ctx.run_accept_object_dlg =
        [](const wxString&, const wxString&) {
            return AcceptObjectDlgResult(ID_STG_CANCEL, true); };

      // Try to transfer same object
      std::stringstream ss;
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      ss << CURLPROG << " --insecure --silent --data @" << datapath << " -o "
          << outpath <<  " -H \"Content-Type: text/xml\""
          << " \"https://localhost:8443/api/rx_object?source=1.2.3.4"
          << "&apikey=" << key << "\"";
      system(ss.str().c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 3}");     // Duplicate rejected
    } {
      // Try to transfer same object using argument force
      std::stringstream ss;
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      ss << CURLPROG << " --insecure --silent --data @" << datapath << " -o "
          << outpath <<  " -H \"Content-Type: text/xml\""
          << " \"https://localhost:8443/api/rx_object?source=1.2.3.4"
          << "&force=1&apikey=" << key << "\"";
      system(ss.str().c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 0}");     // Ok
    }
  }
};

class RestCheckWriteApp : public RestServerApp {
public:
  RestCheckWriteApp(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
      : RestServerApp(ctx, route_ctx, portable) {}

protected:

  void Work() override {

    auto datapath = fs::path(TESTDATA) / "foo.gpx";
    auto outpath = fs::path(CMAKE_BINARY_DIR) / "curl-result";
    {
      std::stringstream ss;
      ss << CURLPROG << " --insecure --silent --data @" << datapath << " -o "
        << outpath <<  " -H \"Content-Type: text/xml\""
        << " \"https://localhost:8443/api/writable?source=1.2.3.4"
        << "&apikey=" << "foobar"
        << "&guid=6a76a7e6-dc39-4a7d-964e-1eff3462c06c\"";

    // Try check our standard object, bad api key
    std::cout << "Running command; " << ss.str() << "\n";
      system(ss.str().c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 5}");     // New pin required
    } {
      // Try check our standard object, fix the api key
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      std::stringstream  ss;
      ss << CURLPROG << " --insecure --silent --data @" << datapath << " -o "
        << outpath <<  " -H \"Content-Type: text/xml\""
        << " \"https://localhost:8443/api/writable?source=1.2.3.4"
        << "&apikey=" << key
        << "&guid=6a76a7e6-dc39-4a7d-964e-1eff3462c06c\"";
      system(ss.str().c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 0}");     // Ok
    } {
      // Set "find duplicate guid" callback to return true;
      m_rest_server.m_route_ctx.find_route_by_guid = [](wxString guid) {
        auto r = new Route;
        r->m_GUID = guid;
        return r;
      };
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      std::stringstream  ss;
      ss << CURLPROG << " --insecure --silent --data @" << datapath << " -o "
        << outpath <<  " -H \"Content-Type: text/xml\""
        << " \"https://localhost:8443/api/writable?source=1.2.3.4"
        << "&apikey=" << key
        << "&guid=apikey6a76a7e6-dc39-4a7d-964e-1eff3462c06c\"";
      system(ss.str().c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 3}");     // Duplicate reject
    }
  }
};

TEST(RestServer, start_stop) {
  wxInitializer initializer;
  ConfigSetup();
  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestServerApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
  EXPECT_EQ(1, 1);
};

TEST(RestServer, Ping) {
  wxInitializer initializer;
  ConfigSetup();
  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestServerPingApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
};

TEST(RestServer, Object) {
  wxInitializer initializer;
  ConfigSetup();
  auto colour_func = [] (wxString c) { return *wxBLACK; };
  pWayPointMan = new WayPointman(colour_func);
  pRouteList = new RouteList;
  g_BasePlatform = new BasePlatform();
  pSelect =  new Select();

  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestServerObjectApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
}
TEST(RestServer, CheckWrite) {
  wxInitializer initializer;
  ConfigSetup();
  auto colour_func = [] (wxString c) { return *wxBLACK; };
  pWayPointMan = new WayPointman(colour_func);
  pRouteList = new RouteList;
  g_BasePlatform = new BasePlatform();
  pSelect = new Select();

  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestCheckWriteApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
}
