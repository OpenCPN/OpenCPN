#include "config.h"

#include <chrono>

#include <stdio.h>
#include <stdlib.h>

#include <wx/app.h>
#include <wx/fileconf.h>
#include <wx/event.h>
#include <wx/evtloop.h>

#include <gtest/gtest.h>

#include <gio/gio.h>

#include "model/base_platform.h"
#include "model/dbus_client.h"
#include "model/logger.h"
#include "model/ocpn_utils.h"

using namespace std::literals::chrono_literals;

static bool bool_result0;
static std::string s_result;
static std::string s_pid;

static std::string StartDbusSession() {
  if (!s_pid.empty()) {
    int num_pid = std::stoi(s_pid);
    int r = kill(num_pid, 0);
    if (r == 0) return s_pid;  // the daemon is already running
  }
  FILE* pipe = popen("dbus-launch --sh-syntax", "r");
  assert(pipe != NULL);

  char* line = NULL;
  size_t len = 0;
  std::stringstream ss;
  int r = getline(&line, &len, pipe);
  assert(r != -1);
  ss << line;
  r = getline(&line, &len, pipe);
  assert(r != -1);
  r = getline(&line, &len, pipe);
  pclose(pipe);
  auto pid_words = ocpn::split(line, "=");
  ocpn::replace(pid_words[1], ";", "");

  auto equals_pos = ss.str().find("=");
  auto var = ss.str().substr(0, equals_pos);
  auto value = ss.str().substr(equals_pos + 1);
  while (ocpn::replace(value, "'", ""));
  while (ocpn::replace(value, "\"", ""));
  while (ocpn::replace(value, ";", ""));
  value = ocpn::rtrim(value);

  assert(var == "DBUS_SESSION_BUS_ADDRESS");
  std::cout << "Setting DBUS_SESSION_BUS_ADDRESS to: \"" << value << "\"\n";
  unsetenv(var.c_str());
  setenv(var.c_str(), value.c_str(), true);
  std::cout << "Starting dbus server: " << pid_words[1];
  return pid_words[1];
}

void CloseDbusSession(const std::string& pid) {
  std::string s("kill ");
  s += pid;
  FILE* pipe = popen(s.c_str(), "r");
  char* line = NULL;
  size_t len = 0;
  getline(&line, &len, pipe);
  pclose(pipe);
  unsetenv("DBUS_SESSION_BUS_ADDRESS");
}

class DbusRaise : public wxAppConsole {
public:
  class ObsListener : public wxEvtHandler {
  public:
    ObsListener(const KeyProvider& kp) : wxEvtHandler() {
      wxDEFINE_EVENT(EVT_OBS_RAISE, ObservedEvt);
      m_listener.Listen(kp.GetKey(), this, EVT_OBS_RAISE);
      Bind(EVT_OBS_RAISE, [&](ObservedEvt& o) { bool_result0 = true; });
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
    std::this_thread::sleep_for(50ms);  // Need some time to settle input
    char buff[1024];
    char* line;
    do {
      line = fgets(buff, sizeof(buff), f);
    } while (line);
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
    ObsListener(DbusServer& dh) : wxEvtHandler() {
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
    do {
      line = fgets(buff, sizeof(buff), f);
    } while (line);
    int r = pclose(f);
    EXPECT_EQ(r, 0);
    std::this_thread::sleep_for(25ms);  // Need some time to settle input
    EXPECT_TRUE(HasPendingEvents());
    ProcessPendingEvents();
    EXPECT_TRUE(bool_result0);
  };
};

TEST(DbusServer, Ping) {
  s_pid = StartDbusSession();
  auto main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);
  DbusServer& dbus_server = DbusServer::GetInstance();

  constexpr const char* const kDbusSendCmd =
      "dbus-send --type=method_call --print-reply --dest=org.opencpn.OpenCPN"
      " /org/opencpn/OpenCPN opencpn.desktop.Ping";
  FILE* f = popen(kDbusSendCmd, "r");
  char buff[1024];
  std::this_thread::sleep_for(100ms);
  char* line = fgets(buff, sizeof(buff), f);  // initial line, throw.
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
  DbusServer::Disconnect();
}

TEST(DbusServer, Raise) {
  std::this_thread::sleep_for(200ms);
  s_pid = StartDbusSession();
  std::this_thread::sleep_for(200ms);
  auto main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);
  auto logfile = std::string(CMAKE_BINARY_DIR) + "/unittests.log";
  wxLog::SetActiveTarget(new OcpnLog(logfile.c_str()));
  wxLog::SetLogLevel(wxLOG_Debug);
  wxLogMessage("Starting DbusServer::Raise");
  wxLog::FlushActive();

  bool_result0 = false;
  std::this_thread::sleep_for(200ms);
  DbusRaise dbus_raise;
  std::this_thread::sleep_for(100ms);
  EXPECT_TRUE(bool_result0);
  g_main_loop_unref(main_loop);
  g_main_loop_quit(main_loop);
  t.join();
  DbusServer::Disconnect();
};

TEST(DbusServer, Quit) {
  auto main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);
  bool_result0 = false;
  DbusQuit dbus_quit;
  std::this_thread::sleep_for(100ms);
  EXPECT_TRUE(bool_result0);
  g_main_loop_unref(main_loop);
  g_main_loop_quit(main_loop);
  t.join();
  DbusServer::Disconnect();
}

TEST(DbusServer, Open) {
  s_result = "";
  bool_result0 = false;
  auto main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);

  DbusServer& dbus_server = DbusServer::GetInstance();
  dbus_server.open_file_cb = [](const std::string& s) {
    s_result = s;
    bool_result0 = true;
    return true;
  };

  constexpr const char* const kDbusSendCmd =
      "dbus-send --type=method_call --print-reply --dest=org.opencpn.OpenCPN"
      " /org/opencpn/OpenCPN opencpn.desktop.Open string:/foo/bar.gpx";
  FILE* f = popen(kDbusSendCmd, "r");
  std::this_thread::sleep_for(100ms);
  char buff[1024];
  char* line = fgets(buff, sizeof(buff), f);  // initial line, throw.
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
  DbusServer::Disconnect();
}

TEST(DbusServer, GetRestEndpoint) {
  s_result = "";
  bool_result0 = false;
  auto main_loop = g_main_loop_new(0, false);
  std::thread t(g_main_loop_run, main_loop);

  DbusServer& dbus_server = DbusServer::GetInstance();

  dbus_server.get_rest_api_endpoint_cb = []() { return "2.2.2.2/3333"; };

  constexpr const char* const kDbusSendCmd =
      "dbus-send --type=method_call --print-reply --dest=org.opencpn.OpenCPN"
      " /org/opencpn/OpenCPN opencpn.desktop.GetRestEndpoint";
  FILE* f = popen(kDbusSendCmd, "r");
  char buff[1024];
  char* line = fgets(buff, sizeof(buff), f);  // initial line, throw.
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
  DbusServer::Disconnect();
}

#if 0
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
  DbusServer::Disconnect();
  delete dbus_server2;
}


TEST(DbusClient, Raise) {
  //DbusServer::Disconnect();
  std::string server_cmd(TEST_ROOT);
  server_cmd += "/test_server.py 1";
  FILE* p = popen(server_cmd.c_str(), "r");
  std::this_thread::sleep_for(150ms);    // Need some time to start server
  DbusLocalClient dbus_local_client;
  auto result = dbus_local_client.SendRaise();
  EXPECT_EQ(std::string(""), result.second);
  EXPECT_TRUE(result.first);
  EXPECT_EQ(pclose(p), 0);
}

TEST(DbusClient, Quit) {
  DbusServer::Disconnect();
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
  DbusServer::Disconnect();
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
  DbusServer::Disconnect();
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

#endif  // 0

TEST(DbusServer, Exit) { CloseDbusSession(s_pid); }
