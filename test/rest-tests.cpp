#include "config.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <thread>

#ifndef _WIN32
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#endif

#include <wx/app.h>
#include <wx/event.h>
#include <wx/fileconf.h>
#include <wx/jsonreader.h>
#include <wx/log.h>

#include <gtest/gtest.h>

#include "certificates.h"
#include "cli_platform.h"
#include "config_vars.h"
#include "mDNS_query.h"
#include "observable_confvar.h"
#include "ocpn_types.h"
#include "ocpn_utils.h"
#include "rest_server.h"
#include "routeman.h"

using namespace std::chrono_literals;

extern WayPointman* pWayPointMan;
extern RouteList* pRouteList;
extern Select* pSelect;
extern AbstractPlatform* g_BasePlatform;

static std::string s_result;
static int int_result0;

static void ConfigSetup() {
  const auto config_orig = fs::path(TESTDATA) / "opencpn.conf";
  const auto config_path = fs::path(CMAKE_BINARY_DIR) / "opencpn.conf";
  std::remove(config_path.string().c_str());
  fs::copy(config_orig, config_path);
  InitBaseConfig(new wxFileConfig("", "", config_path.string()));
}

#ifdef _WIN32
#define GetLocalAddresses()  get_local_ipv4_addresses()

#else
static std::vector<std::string> GetLocalAddresses() {
  struct ifaddrs* ifAddrStruct = 0;
  struct ifaddrs* ifa = 0;
  void* tmp_addr = 0;
  std::vector<std::string> retvals;

  getifaddrs(&ifAddrStruct);
  for (ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) continue;
    if (ifa->ifa_addr->sa_family == AF_INET) {
      tmp_addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      char address_buffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmp_addr, address_buffer, INET_ADDRSTRLEN);
      retvals.push_back(address_buffer);
    }
  }
  if (ifAddrStruct) freeifaddrs(ifAddrStruct);
  return retvals;
}
#endif   // _WIN32

static bool g_portable = false;
class RestServerApp : public wxAppConsole {
public:
  RestServerApp(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
          : wxAppConsole(),
            m_rest_server(ctx, route_ctx, portable) {}

  void Run() {
    ConfigSetup();
    std::vector<std::string> addresses;
    for (int i = 0; addresses.size() == 0 && i < 10; i++) {
       std::this_thread::sleep_for(500ms);
       addresses = GetLocalAddresses();
    } 
    if (!addresses.size()) {
        std::cerr << "Cannot get local IP address(!)\n";
        return;
    }
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


static std::string CmdString(const std::string s) {
#ifdef _MSC_VER
  return std::string("\"") + s + "\"";
#else
  return s;
#endif
}


class RestServerPingApp : public RestServerApp {
public:
  RestServerPingApp(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
      : RestServerApp(ctx, route_ctx, portable) {}

protected:
  void Work() {
    auto path = fs::path(CMAKE_BINARY_DIR) / "curl-result";
    {
      std::this_thread::sleep_for(50ms);

      fs::path curl_prog(CURLPROG);
      std::stringstream ss;
      ss << curl_prog.make_preferred() << " --insecure -o " << path
	  << " \"https://localhost:8443/api/ping?source=1.2.3.4&apikey=bad\"";
      system(CmdString(ss.str()).c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(path.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 5}");  // Bad api key
    }{
      fs::path curl_prog(CURLPROG);
      std::stringstream ss;
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      ss << curl_prog.make_preferred() << " --insecure -o " << path
         << " \"https://localhost:8443/api/ping?source=1.2.3.4&apikey=" << key
	 << "\"";
      system(CmdString(ss.str()).c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(path.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 0}");  // ok
    }
  }
};

class RestServer404App : public RestServerApp {
public:
  RestServer404App(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
      : RestServerApp(ctx, route_ctx, portable) {}

protected:
  void Work() {
    auto path = fs::path(CMAKE_BINARY_DIR) / "curl-result";
    std::this_thread::sleep_for(50ms);
    fs::path curl_prog(CURLPROG);
    std::stringstream ss;
    ss <<  curl_prog.make_preferred()  << " --insecure --max-time 3 -I -o "
       << path << " \"https://localhost:8443/api/pong\"";
    system(CmdString(ss.str()).c_str());
    std::this_thread::sleep_for(50ms);

    ProcessPendingEvents();
    std::ifstream f(path.string());
    std::string result;
    std::getline(f, result);
    auto words = ocpn::split(result.c_str(), " ");
    EXPECT_EQ(words[1], "404");  // ok
  }
};

class RestServerVersionApp : public RestServerApp {
public:
  RestServerVersionApp(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
      : RestServerApp(ctx, route_ctx, portable) {}

protected:
  void Work() {
    auto path = fs::path(CMAKE_BINARY_DIR) / "curl-result";

    {
      fs::path curl_prog(CURLPROG);
      std::stringstream ss;
      ss << curl_prog.make_preferred()  << " --insecure --max-time 3 -o "
         << path << " \"https://localhost:8443/api/get-version\"";
      system(CmdString(ss.str()).c_str());
    }
    std::this_thread::sleep_for(50ms);
    ProcessPendingEvents();
    std::ifstream f(path.string());
    std::stringstream ss;
    ss << f.rdbuf();
    wxJSONValue root;
    wxJSONReader reader;
    std::string reply = ss.str();
    int errors = reader.Parse(reply, &root);
    EXPECT_EQ(errors, 0);
    wxString version = root["version"].AsString();
    EXPECT_EQ(version, PACKAGE_VERSION);
  }
};

class RestServerObjectApp : public RestServerApp {
public:
  RestServerObjectApp(RestServerDlgCtx ctx, RouteCtx route_ctx, bool& portable)
      : RestServerApp(ctx, route_ctx, portable) {}

protected:
  void Work() override {
    auto colour_func = [] (wxString c) { return *wxBLACK; };
    pWayPointMan = new WayPointman(colour_func);
    pRouteList = new RouteList;
    g_BasePlatform = new CliPlatform();
    pSelect =  new Select();

    auto outpath = fs::path(CMAKE_BINARY_DIR) / "curl-result";
    auto datapath = fs::path(TESTDATA) / "foo.gpx";
    {
      // try to transfer route without api key
      fs::path curl_prog(CURLPROG);
      std::stringstream ss;
      ss << curl_prog.make_preferred() << " --insecure --silent --data @"
	 << datapath << " -o " << outpath <<  " -H \"Content-Type: text/xml\""
         << " \"https://localhost:8443/api/rx_object?source=1.2.3.4\"";
      system(CmdString(ss.str()).c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 5}");   // New pin required
    } {
      // Try to transfer using api key set up above.
      fs::path curl_prog(CURLPROG);
      std::stringstream ss;
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      ss << curl_prog.make_preferred() << " --insecure --silent --data @"
	 << datapath << " -o " << outpath <<  " -H \"Content-Type: text/xml\""
          << " \"https://localhost:8443/api/rx_object?source=1.2.3.4"
          << "&apikey=" << key << "\"";
      system(CmdString(ss.str()).c_str());
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
      fs::path curl_prog(CURLPROG);
      std::stringstream ss;
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      ss << curl_prog.make_preferred() << " --insecure --silent --data @"
         << datapath << " -o " << outpath <<  " -H \"Content-Type: text/xml\""
          << " \"https://localhost:8443/api/rx_object?source=1.2.3.4"
          << "&apikey=" << key << "\"";
      system(CmdString(ss.str()).c_str());
      std::this_thread::sleep_for(50ms);
      ProcessPendingEvents();
      std::ifstream f(outpath.string());
      std::string result;
      std::getline(f, result);
      EXPECT_EQ(result, "{\"result\": 3}");     // Duplicate rejected
    } {
      // Try to transfer same object using argument force
      fs::path curl_prog(CURLPROG);
      std::stringstream ss;
      auto key = m_rest_server.m_key_map["1.2.3.4"];
      ss << curl_prog.make_preferred() << " --insecure --silent --data @"
         << datapath << " -o " << outpath <<  " -H \"Content-Type: text/xml\""
          << " \"https://localhost:8443/api/rx_object?source=1.2.3.4"
          << "&force=1&apikey=" << key << "\"";
      system(CmdString(ss.str()).c_str());
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
    auto colour_func = [] (wxString c) { return *wxBLACK; };
    pWayPointMan = new WayPointman(colour_func);
    pRouteList = new RouteList;
    g_BasePlatform = new CliPlatform();
    pSelect = new Select();

    fs::path curl_prog(CURLPROG);
    auto datapath = fs::path(TESTDATA) / "foo.gpx";
    auto outpath = fs::path(CMAKE_BINARY_DIR) / "curl-result";
    {
      std::stringstream ss;
      ss << curl_prog.make_preferred() << " --insecure --silent --data @"
         << datapath << " -o " << outpath <<  " -H \"Content-Type: text/xml\""
        << " \"https://localhost:8443/api/writable?source=1.2.3.4"
        << "&apikey=" << "foobar"
        << "&guid=6a76a7e6-dc39-4a7d-964e-1eff3462c06c\"";

    // Try check our standard object, bad api key
      system(CmdString(ss.str()).c_str());
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
      ss << curl_prog.make_preferred() << " --insecure --silent --data @"
         << datapath << " -o " << outpath <<  " -H \"Content-Type: text/xml\""
        << " \"https://localhost:8443/api/writable?source=1.2.3.4"
        << "&apikey=" << key
        << "&guid=6a76a7e6-dc39-4a7d-964e-1eff3462c06c\"";
      system(CmdString(ss.str()).c_str());
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
      ss << curl_prog.make_preferred() << " --insecure --silent --data @"
	 << datapath << " -o " << outpath <<  " -H \"Content-Type: text/xml\""
        << " \"https://localhost:8443/api/writable?source=1.2.3.4"
        << "&apikey=" << key
        << "&guid=apikey6a76a7e6-dc39-4a7d-964e-1eff3462c06c\"";
      system(CmdString(ss.str()).c_str());
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
  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestServerApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
  EXPECT_EQ(1, 1);
};

TEST(RestServer, Ping) {
  wxDisableAsserts();
  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestServerPingApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
};

TEST(RestServer, Pong) {
  wxDisableAsserts();
  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestServer404App app(dialog_ctx, route_ctx, g_portable);
  app.Run();
};

TEST(RestServer, Version) {
  wxDisableAsserts();
  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestServerVersionApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
};

TEST(RestServer, Object) {
  wxDisableAsserts();
  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestServerObjectApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
}

TEST(RestServer, CheckWrite) {
  wxDisableAsserts();
  RestServerDlgCtx dialog_ctx;
  RouteCtx route_ctx;
  RestCheckWriteApp app(dialog_ctx, route_ctx, g_portable);
  app.Run();
  delete g_BasePlatform;
  g_BasePlatform = 0;
}
