#include <stdio.h>

#include <gtest/gtest.h>

#include <wx/app.h>

#include "config.h"

#include "base_platform.h"
#include "comm_ais.h"
#include "comm_appmsg_bus.h"
#include "comm_bridge.h"
#include "comm_drv_file.h"
#include "comm_drv_registry.h"
#include "conn_params.h"
#include "observable_navmsg.h"
#include "observable_confvar.h"
#include "ocpn_types.h"
#include "routeman.h"
#include "ais_defs.h"
#include "ais_decoder.h"
#include "select.h"

#ifdef __linux__
#include "comm_drv_n2k_socketcan.h"
#endif

class AISTargetAlertDialog;
class Multiplexer;

extern bool g_bAIS_ACK_Timeout;
extern bool g_bAIS_CPA_Alert_Suppress_Moored;
extern bool g_bCPAMax;
extern bool g_bCPAWarn;
extern bool g_bHideMoored;
extern bool g_bTCPA_Max;
extern double g_AckTimeout_Mins;
extern double g_CPAMax_NM;
extern double g_CPAWarn_NM;
extern double g_ShowMoored_Kts;
extern double g_TCPA_Max;
extern bool g_bShowMag;
extern bool g_bShowTrue;
extern bool bGPSValid;
extern bool g_bInlandEcdis;
extern bool g_bRemoveLost;
extern bool g_bMarkLost;
extern bool g_bShowScaled;
extern bool g_bAllowShowScaled;
extern bool g_bAISRolloverShowCOG;
extern bool g_bAISRolloverShowCPA;
extern bool g_bAISShowTracks;
extern bool g_bAISRolloverShowClass;

extern Multiplexer* g_pMUX;
extern std::vector<Track*> g_TrackList;
extern int g_WplAction;
extern AISTargetAlertDialog* g_pais_alert_dialog_active;
extern wxString AISTargetNameFileName;
extern double g_AISShowTracks_Mins;
extern bool g_bAIS_CPA_Alert;
extern Route *pAISMOBRoute;
extern double g_RemoveLost_Mins;
extern double g_MarkLost_Mins;
extern float g_selection_radius_mm;
extern float g_selection_radius_touch_mm;
extern wxString g_GPS_Ident;
extern bool g_bGarminHostUpload;
extern int g_nCOMPortCheck;
extern bool g_benableUDPNullHeader;

extern BasePlatform* g_BasePlatform;
extern bool g_bportable;
extern wxString g_winPluginDir;
extern wxConfigBase* pBaseConfig;
extern void* g_pi_manager;
extern wxString g_compatOS;
extern wxString g_compatOsVersion;

extern double gCog;
extern double gHdm;
extern double gHdt;
extern double gLat;
extern double gLon;
extern double gSog;
extern double gVar;
extern double g_UserVar;
extern int gps_watchdog_timeout_ticks;
extern bool g_bHDT_Rx;
extern int g_nNMEADebug;
extern bool g_bSatValid;
extern bool g_bVAR_Rx;
extern int g_NMEAAPBPrecision;
extern int g_SatsInView;
extern int g_priSats;
extern int sat_watchdog_timeout_ticks;

extern Select* pSelectAIS;
extern Select* pSelect;
extern AisDecoder* g_pAIS;

int int0 = -1;
int int1 = -1;
int int2 = -1;

class N2kTest: public testing::Test {
public:
   N2kTest() : testing::Test(), app(0) {}

protected:
  wxAppConsole* app;

  virtual void SetUp()  override {
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

  virtual void TearDown()  override {
    app->OnExit();
  }
};

#ifdef __linux__    // linux-only socketcan driver setup
class N2kTestDriverRegistry : public wxAppConsole {
public:
  N2kTestDriverRegistry() : wxAppConsole() {};

  bool OnInit() {
    wxAppConsole::OnInit();

    auto& registry = CommDriverRegistry::getInstance();
    int start_size = registry.GetDrivers().size();

    auto& msgbus = NavMsgBus::GetInstance();
    ConnectionParams params;
    params.socketCAN_port = "vcan0";
    params.Type = SOCKETCAN;
    auto driver = CommDriverN2KSocketCAN::Create(&params, msgbus);
    driver->Activate();
    ProcessPendingEvents();
    int0 = registry.GetDrivers().size() - start_size;
    driver->Close();
    int1 = registry.GetDrivers().size() - start_size;
    registry.CloseAllDrivers();
    int2 = registry.GetDrivers().size();
    return true;
  }
};

#endif

#ifdef __linux__    // Based on linux-only socketcan driver
class DriverRegistry: public N2kTest  {
public:
  DriverRegistry(): N2kTest() { app = new N2kTestDriverRegistry(); }
};

TEST_F(DriverRegistry, RegisterDriver) {
  EXPECT_EQ(int0, 1);   // Driver activated and registered
  EXPECT_EQ(int1, 0);   // Driver closed.
  EXPECT_EQ(int2, 0);   // All drivers closed.
}

TEST(CanEnvironment, vcan0) {
  char line[256];
  FILE* f = popen("ip address show vcan0", "r");
  char* r =  fgets(line, sizeof(line), f);
  EXPECT_TRUE(r != 0)  << "The vcan0 device is not available (?) \n";
  int i = pclose(f);
  EXPECT_TRUE(i == 0)  << "Error running the ip(8) command\n";
}
#endif
