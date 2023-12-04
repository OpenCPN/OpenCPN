#include "config.h"

#include <chrono>

#include <wx/app.h>
#include <wx/fileconf.h>
#include <wx/event.h>
#include <wx/evtloop.h>

#include <gtest/gtest.h>

#include <gio/gio.h>

#include "base_platform.h"
#include "dbus_client.h"
#include "logger.h"
#include "ocpn_utils.h"

using namespace std::literals::chrono_literals;

BasePlatform* g_BasePlatform = 0;
bool g_bportable;
float g_selection_radius_mm;
float g_selection_radius_touch_mm;
bool g_btouch;
wxString g_winPluginDir;

static bool bool_result0;
static std::string s_result;

static void EnsureHomedir() {
#ifdef _MSC_VER
  wxFileName path("/ProgramData/opencpn/")
#else
  wxFileName path("~/.opencpn");
  path.Normalize(wxPATH_NORM_TILDE);
#endif
  if (!path.DirExists()) path.Mkdir();
}

class DbusRaise : public wxAppConsole {
public:

  class ObsListener : public wxEvtHandler {
  public:
    ObsListener(const KeyProvider& kp): wxEvtHandler() {
      wxDEFINE_EVENT(EVT_OBS_RAISE, ObservedEvt);
      m_listener.Listen(kp.GetKey(), this, EVT_OBS_RAISE);
      Bind(EVT_OBS_RAISE, [&](ObservedEvt& o) { bool_result0 = true;});
    }
    ObservableListener m_listener;
  };

  DbusRaise() {
    bool_result0 = false;
    DbusServer& dbus_server = DbusServer::GetInstance();
    ObsListener obs_listener(dbus_server.on_raise);

    constexpr const char* const kDbusSendCmd =
        "dbus-send --type=method_call --print-reply --dest=org.opencpn.OpenCPN"
        " /org/opencpn/OpenCPN opencpn.desktop.Raise";
    FILE* f = popen(kDbusSendCmd, "r");
    std::this_thread::sleep_for(50ms);    // Need some time to settle input
    char buff[1024];
    char* line;
    do { line = fgets(buff, sizeof(buff), f); } while (line);
    int r = pclose(f);
    EXPECT_EQ(r, 0);
    EXPECT_TRUE(HasPendingEvents());
    ProcessPendingEvents();
  }
};

class DbusQuit : public wxAppConsole {
public:
  class ObsListener : public wxEvtHandler {
  public:
    ObsListener(DbusServer& dh): wxEvtHandler() {
      wxDEFINE_EVENT(EVT_OBS_QUIT, ObservedEvt);
      m_listener.Listen(dh.on_quit, this, EVT_OBS_QUIT);
      Bind(EVT_OBS_QUIT, [&](ObservedEvt&) { bool_result0 = true; });
    }

  private:
    ObservableListener m_listener;
  };

  DbusQuit() {
    bool_result0 = false;
    DbusServer& dbus_server = DbusServer::GetInstance();
    ObsListener listener(dbus_server);
    constexpr const char* const kDbusSendCmd =
       "dbus-send --type=method_call --dest=org.opencpn.OpenCPN --print-reply"
       " /org/opencpn/OpenCPN opencpn.desktop.Quit";
    FILE* f = popen(kDbusSendCmd, "r");
    char buff[1024];
    char* line;
    do { line = fgets(buff, sizeof(buff), f); } while (line);
    int r = pclose(f);
    EXPECT_EQ(r, 0);
    std::this_thread::sleep_for(25ms);    // Need some time to settle input
    EXPECT_TRUE(HasPendingEvents());
    ProcessPendingEvents();
    EXPECT_TRUE(bool_result0);
  };
};

TEST(DbusServer, Ping) {
  auto  main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);
  DbusServer& dbus_server = DbusServer::GetInstance();

  constexpr const char* const kDbusSendCmd =
     "dbus-send --type=method_call --print-reply --dest=org.opencpn.OpenCPN"
     " /org/opencpn/OpenCPN opencpn.desktop.Ping";
  FILE* f = popen(kDbusSendCmd, "r");
  char buff[1024];
  char* line = fgets(buff, sizeof(buff), f);   // initial line, throw.
std::cout << "Initial line: " << line << "\n";
  EXPECT_TRUE(line);

  line = fgets(buff, sizeof(buff), f);  //  "   uint32 21614"
  EXPECT_TRUE(line);
  auto s = ocpn::trim(std::string(line));
  auto words = ocpn::split(s.c_str(), " ");
  EXPECT_EQ(words.size(), 2);
  EXPECT_EQ(words[1], "21614");

  line = fgets(buff, sizeof(buff), f);  //  "    boolean true"
  EXPECT_TRUE(line);
  s = ocpn::trim(std::string(line));
  words = ocpn::split(s.c_str(), " ");
  EXPECT_EQ(words.size(), 2);
  EXPECT_EQ(words[1], "true");

  pclose(f);
  g_main_loop_unref(main_loop);
  g_main_loop_quit(main_loop);
  t.join();
}

TEST(DbusServer, Raise) {
  auto  main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);
  auto logfile = std::string(CMAKE_BINARY_DIR) + "/unittests.log";
  wxLog::SetActiveTarget(new OcpnLog(logfile.c_str()));
  wxLog::SetLogLevel(wxLOG_Debug);
  wxLogMessage("Starting DbusServer::Raise");
  wxLog::FlushActive();

  bool_result0 = false;
  DbusRaise dbus_raise;
  std::this_thread::sleep_for(100ms);
  EXPECT_TRUE(bool_result0);
  g_main_loop_unref(main_loop);
  g_main_loop_quit(main_loop);
  t.join();
}

TEST(DbusServer, Quit) {
  auto  main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);
  bool_result0 = false;
  DbusQuit dbus_quit;
  std::this_thread::sleep_for(100ms);
  EXPECT_TRUE(bool_result0);
  g_main_loop_unref(main_loop);
  g_main_loop_quit(main_loop);
  t.join();
}

TEST(DbusServer, Open) {
  s_result = "";
  bool_result0 = false;
  auto  main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);

  DbusServer& dbus_server = DbusServer::GetInstance();
  dbus_server.open_file_cb = [](const std::string& s) {
      s_result = s;
      bool_result0 = true;
      return true; };

  constexpr const char* const kDbusSendCmd =
     "dbus-send --type=method_call --print-reply --dest=org.opencpn.OpenCPN"
     " /org/opencpn/OpenCPN opencpn.desktop.Open string:/foo/bar.gpx";
  FILE* f = popen(kDbusSendCmd, "r");
  char buff[1024];
  char* line = fgets(buff, sizeof(buff), f);   // initial line, throw.
  EXPECT_TRUE(line);

  line = fgets(buff, sizeof(buff), f);  //  "   boolean true"
  EXPECT_TRUE(line);
  auto s = ocpn::trim(std::string(line));
  auto words = ocpn::split(s.c_str(), " ");
  EXPECT_EQ(words.size(), 2);
  EXPECT_EQ(words[1], "true");

  int r = pclose(f);
  EXPECT_EQ(r, 0);

  std::this_thread::sleep_for(100ms);
  EXPECT_EQ(s_result, "/foo/bar.gpx");
  EXPECT_TRUE(bool_result0);

  g_main_loop_unref(main_loop);
  g_main_loop_quit(main_loop);
  t.join();
}

TEST(DbusServer, GetRestEndpoint) {
  s_result = "";
  bool_result0 = false;
  auto  main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);

  DbusServer& dbus_server = DbusServer::GetInstance();

  dbus_server.get_rest_api_endpoint_cb = []() { return "2.2.2.2/3333"; };

  constexpr const char* const kDbusSendCmd =
     "dbus-send --type=method_call --print-reply --dest=org.opencpn.OpenCPN"
     " /org/opencpn/OpenCPN opencpn.desktop.GetRestEndpoint";
  FILE* f = popen(kDbusSendCmd, "r");
  char buff[1024];
  char* line = fgets(buff, sizeof(buff), f);   // initial line, throw.
  EXPECT_TRUE(line);

  line = fgets(buff, sizeof(buff), f);  //  "   string 2.2.2.2/3333"
  EXPECT_TRUE(line);
  auto s = ocpn::trim(std::string(line));
  auto words = ocpn::split(s.c_str(), " ");
  EXPECT_EQ(words.size(), 2);
  EXPECT_EQ(words[1], "\"2.2.2.2/3333\"");

  int r = pclose(f);
  EXPECT_EQ(r, 0);

  g_main_loop_unref(main_loop);
  g_main_loop_quit(main_loop);
  t.join();
}

TEST(Instance, DbusServer) {
  auto  main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);
  DbusServer& dbus_server1 = DbusServer::GetInstance();
  DbusServer* dbus_server2 = DbusServer::GetInstance().UnitTestInstance();
  dbus_server1.WaitUntilValid();
  EXPECT_TRUE(dbus_server1.IsMainInstance());
  EXPECT_FALSE(dbus_server2->IsMainInstance());
  g_main_loop_unref(main_loop);
  g_main_loop_quit(main_loop);
  t.join();

}

TEST(DbusClient, Raise) {
  std::this_thread::sleep_for(200ms);    // WTF
  std::string server_cmd(TEST_ROOT);
  server_cmd += "/test_server.py 1";
  FILE* p = popen(server_cmd.c_str(), "r");
  std::this_thread::sleep_for(100ms);    // Need some time to start server
  DbusLocalClient dbus_local_client;
  std::this_thread::sleep_for(200ms);
  auto result = dbus_local_client.SendRaise();
  EXPECT_EQ(std::string(""), result.second);
  EXPECT_TRUE(result.first);
  EXPECT_EQ(pclose(p), 0);
}

TEST(DbusClient, Quit) {
  std::string server_cmd(TEST_ROOT);
  server_cmd += "/test_server.py 1";
  FILE* p = popen(server_cmd.c_str(), "r");
  std::this_thread::sleep_for(100ms);    // Need some time to start server
  DbusLocalClient dbus_local_client;
  auto result = dbus_local_client.SendQuit();
  EXPECT_TRUE(result.first);
  EXPECT_EQ(std::string(""), result.second);
  EXPECT_EQ(pclose(p), 0);
}

TEST(DbusClient, Open) {
  auto logfile = std::string(CMAKE_BINARY_DIR) + "/unittests.log";
  wxLog::SetActiveTarget(new OcpnLog(logfile.c_str()));
  wxLog::SetLogLevel(wxLOG_Debug);
  wxLogMessage("Starting DbusClient::Open");
  auto server_cmd = std::string(TEST_ROOT) + "/test_server.py 1";
  FILE* p = popen(server_cmd.c_str(), "r");
  std::this_thread::sleep_for(100ms);    // Need some time to start server
  DbusLocalClient dbus_local_client;
  auto result = dbus_local_client.SendOpen("/foo/bar");
  EXPECT_TRUE(result.first);
  EXPECT_EQ(std::string("/foo/bar"), result.second);
  EXPECT_EQ(pclose(p), 0);
}

TEST(DbusClient, GetEndpoint) {
  std::string server_cmd(TEST_ROOT);
  server_cmd += "/test_server.py 1";
  FILE* p = popen(server_cmd.c_str(), "r");
  std::this_thread::sleep_for(100ms);    // Need some time to start server
  DbusLocalClient dbus_local_client;
  auto result = dbus_local_client.GetRestEndpoint();
  EXPECT_EQ(pclose(p), 0);
  EXPECT_TRUE(result.first);
  EXPECT_EQ(std::string("0.0.0.0/1025"), result.second);
}
