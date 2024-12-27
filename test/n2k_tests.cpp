#ifndef __linux__
#error "This file can only be compiled on Linux."
#endif

#include <stdio.h>

#include <gtest/gtest.h>

#include <wx/app.h>

#include "config.h"

#include "model/ais_state_vars.h"
#include "model/cli_platform.h"
#include "model/cmdline.h"
#include "model/comm_ais.h"
#include "model/comm_appmsg_bus.h"
#include "model/comm_bridge.h"
#include "model/comm_drv_file.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/config_vars.h"
#include "model/conn_params.h"
#include "observable_confvar.h"
#include "model/ocpn_types.h"
#include "model/own_ship.h"
#include "model/routeman.h"
#include "model/ais_defs.h"
#include "model/ais_decoder.h"
#include "model/select.h"

#include "model/comm_drv_n2k_socketcan.h"

#ifdef _MSC_VER
const static std::string kSEP("\\");
#else
const static std::string kSEP("/");
#endif
static const std::vector<std::pair<double, double>> expected_targets = {
    {56.7018, 8.2187},  {56.7047, 8.22219}, {56.7031, 8.22308},
    {56.6974, 8.2182},  {56.608, 8.13714},  {57.0929, 7.91285},
    {56.6969, 8.21844}, {56.702, 8.21929},  {56.7012, 8.22062},
    {56.6977, 8.2199},  {56.6982, 8.21946}, {56.7029, 8.22314},
    {56.701, 8.21929},  {56.7018, 8.2187},  {56.7047, 8.22219},
    {56.7031, 8.22308}, {56.6974, 8.2182},  {56.608, 8.13714},
    {57.0929, 7.91285}, {56.6969, 8.21844}, {56.702, 8.21929},
    {56.7012, 8.22062}, {56.6977, 8.2199},  {56.6982, 8.21946},
    {56.7029, 8.22314}, {56.701, 8.21929},  {56.7018, 8.2187},
    {56.7047, 8.22219}, {56.7031, 8.22308}, {56.6974, 8.2182},
    {56.608, 8.13714},  {57.0929, 7.91285}, {56.6969, 8.21844},
    {56.702, 8.21929},  {56.7012, 8.22062}, {56.6977, 8.2199},
    {56.6982, 8.21946}, {56.7029, 8.22314}, {56.701, 8.21929},
    {56.7018, 8.2187},  {56.7047, 8.22219}, {56.7031, 8.22308},
    {56.6974, 8.2182},  {56.608, 8.13714},  {57.0929, 7.91285},
    {56.6969, 8.21844}, {56.702, 8.21929},  {56.7012, 8.22062},
    {56.6977, 8.2199},  {56.6982, 8.21946}, {56.7029, 8.22314},
    {56.701, 8.21929}};

class AISTargetAlertDialog;
class Multiplexer;

extern Multiplexer* g_pMUX;
extern std::vector<Track*> g_TrackList;
extern wxString AISTargetNameFileName;

extern BasePlatform* g_BasePlatform;
extern void* g_pi_manager;

extern bool g_bHDT_Rx;
extern int g_NMEAAPBPrecision;

extern Select* pSelectAIS;
extern Select* pSelect;
extern AisDecoder* g_pAIS;

int int0 = -1;
int int1 = -1;
int int2 = -1;

using namespace std;

static void CheckAisTargets() {
  auto found_targets = pSelectAIS->GetSelectList();
  for (auto t : expected_targets) {
    bool found = false;
    for (auto it = found_targets->begin(); it != found_targets->end(); it++) {
      auto found_target = *it;
      if (std::abs(found_target->m_slat - t.first) < 0.0001 &&
          std::abs(found_target->m_slon - t.second) < 0.0001) {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found) << "Cannot find lat: " << t.first
                       << ", lon: " << t.second << "in target list";
  }
}

FILE* RunRecordedBuffer() {
  string path("..");
  path += kSEP + ".." + kSEP + "test" + kSEP + "testdata" + kSEP +
          "candump-2022-07-30_102821-head.log";
  string cmd("canplayer -I ");
  cmd += path + " vcan0=can0";
  FILE* f = popen(cmd.c_str(), "r");
  EXPECT_TRUE(f != 0);
  return f;
}

class N2kTest : public testing::Test {
public:
  N2kTest() : testing::Test(), app(0) {}

protected:
  wxAppConsole* app;

  virtual void SetUp() override {
    g_BasePlatform = new BasePlatform();
    pSelectAIS = new Select();
    pSelect = new Select();
    g_pAIS = new AisDecoder(AisDecoderCallbacks());

    char argv0[] = "n2k_test.exe";
    char* argv[1] = {argv0};
    int argc = 1;
    wxApp::SetInstance(app);
    wxEntryStart(argc, argv);

    app->OnInit();
  }

  virtual void TearDown() override { app->OnExit(); }
};

#ifdef __linux__  // linux-only socketcan driver setup

class N2kTestDriverRegistry : public wxAppConsole {
public:
  N2kTestDriverRegistry() : wxAppConsole() {};

  bool OnInit() {
    wxAppConsole::OnInit();

    auto& registry = CommDriverRegistry::GetInstance();
    int start_size = registry.GetDrivers().size();

    auto& msgbus = NavMsgBus::GetInstance();
    ConnectionParams params;
    params.socketCAN_port = "vcan0";
    params.Type = SOCKETCAN;
    auto driver = CommDriverN2KSocketCAN::Create(&params, msgbus);
    auto raw_driver = driver.get();
    CommDriverRegistry::GetInstance().Activate(std::move(driver));
    ProcessPendingEvents();
    int0 = registry.GetDrivers().size() - start_size;
    raw_driver->Close();
    int1 = registry.GetDrivers().size() - start_size;
    registry.CloseAllDrivers();
    int2 = registry.GetDrivers().size();
    return true;
  }
};

class N2kTestData : public wxAppConsole {
public:
  N2kTestData() : wxAppConsole() {}

  bool OnInit() {
    string path("..");
    path += kSEP + ".." + kSEP + "test" + kSEP + "testdata" + kSEP +
            "candump-2022-07-30_102821-head.log";
    string cmd("canplayer -I ");
    cmd += path + " vcan0=can0";
    FILE* f = popen(cmd.c_str(), "r");
    N2kTestData n2k_test_data;

    wxAppConsole::OnInit();
    auto& registry = CommDriverRegistry::GetInstance();
    auto& msgbus = NavMsgBus::GetInstance();
    ConnectionParams params;
    params.socketCAN_port = "vcan0";
    params.Type = SOCKETCAN;
    auto driver = CommDriverN2KSocketCAN::Create(&params, msgbus);
    CommBridge comm_bridge;
    comm_bridge.Initialize();
    CommDriverRegistry::GetInstance().Activate(std::move(driver));
    ProcessPendingEvents();

    int i = pclose(f);
    EXPECT_TRUE(i == 0) << "Error running the canplayer command\n";
    return true;
  }
};

class N2kRunLog : public wxAppConsole {
public:
  N2kRunLog() : wxAppConsole() {}

  bool OnInit() {
    wxAppConsole::OnInit();

    // Observable::Clear();
    g_BasePlatform = new BasePlatform();
    delete pSelectAIS;
    pSelectAIS = new Select();
    delete pSelect;
    pSelect = new Select();
    delete g_pAIS;
    g_pAIS = new AisDecoder(AisDecoderCallbacks());
    auto& msgbus = NavMsgBus::GetInstance();
    params.socketCAN_port = "vcan0";
    params.Type = SOCKETCAN;
    driver = CommDriverN2KSocketCAN::Create(&params, msgbus);
    comm_bridge.Initialize();
    CommDriverRegistry::GetInstance().Activate(std::move(driver));
    return true;
  }

  int OnRun() {
    FILE* f = RunRecordedBuffer();
    int i = pclose(f);
    EXPECT_TRUE(i == 0) << "Error running the canplayer command\n";
    ProcessPendingEvents();
    driver->Close();
    return 0;
  }

private:
  std::unique_ptr<CommDriverN2KSocketCAN> driver;
  CommBridge comm_bridge;
  ConnectionParams params;
};

#endif

class LogProcessing : public N2kTest {
public:
  LogProcessing() : N2kTest() { app = new N2kRunLog(); }
};

#ifndef OCPN_DISTRO_BUILD
TEST(DriverRegistry, RegisterDriver) {
  N2kTestDriverRegistry app;
  app.OnInit();
  EXPECT_EQ(int0, 1);  // Driver activated and registered
  EXPECT_EQ(int1, 0);  // Driver closed.
  EXPECT_EQ(int2, 0);  // All drivers closed.
}
#endif

#ifdef ENABLE_VCAN_TESTS
TEST(CanEnvironment, vcan0) {
  char line[256];
  FILE* f = popen("ip address show vcan0", "r");
  char* r = fgets(line, sizeof(line), f);
  EXPECT_TRUE(r != 0) << "The vcan0 device is not available (?) \n";
  int i = pclose(f);
  EXPECT_TRUE(i == 0) << "Error running the ip(8) command\n";
}

TEST(CanEnvironment, canplayer) {
  gLat = 0;
  gLon = 0;
  string path("..");
  path += kSEP + ".." + kSEP + "test" + kSEP + "testdata" + kSEP +
          "candump-2022-07-30_102821-head.log";
  string cmd("canplayer -I ");
  cmd += path + " vcan0=can0";
  FILE* f = popen(cmd.c_str(), "r");
  int i = pclose(f);
  EXPECT_TRUE(i == 0) << "Error running the canplayer command\n";
}

TEST(DriverProcessing, base) {
  N2kTestDriverRegistry app;
  app.OnInit();
  EXPECT_EQ(int0, 1);
  EXPECT_EQ(int1, 0);
  EXPECT_EQ(int2, 0);
  std::cerr << "DriverProcessing, gSog: " << gSog << "\n";
  std::cerr << "LogProcessing, gCog: " << gCog << "\n";
  gCog = 0;
  gSog = 0;
}

TEST(LogProcessing, base) {
  N2kRunLog n2k_run_log;
  n2k_run_log.OnInit();
  n2k_run_log.OnRun();
  EXPECT_NEAR(gLat, 56.7064, 0.0001);
  EXPECT_NEAR(gLon, 8.22156, 0.0001);
  CheckAisTargets();
}
#endif  // ENABLE_VCAN_TESTS
