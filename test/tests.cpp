#include "config.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <thread>

#include <wx/event.h>
#include <wx/app.h>
#include <wx/jsonval.h>

#include <gtest/gtest.h>

#include "ais_decoder.h"
#include "ais_defs.h"
#include "base_platform.h"
#include "comm_ais.h"
#include "comm_appmsg_bus.h"
#include "comm_bridge.h"
#include "comm_drv_file.h"
#include "comm_drv_registry.h"
#include "comm_navmsg_bus.h"
#include "config_vars.h"
#include "observable_confvar.h"
#include "ocpn_types.h"
#include "ocpn_plugin.h"
#include "own_ship.h"
#include "routeman.h"
#include "select.h"

class AISTargetAlertDialog;
class Multiplexer;

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

Multiplexer* g_pMUX;
std::vector<Track*> g_TrackList;
int g_WplAction;
AISTargetAlertDialog* g_pais_alert_dialog_active;
wxString AISTargetNameFileName;
double g_AISShowTracks_Mins;
bool g_bAIS_CPA_Alert;
Route *pAISMOBRoute;
double g_RemoveLost_Mins;
double g_MarkLost_Mins;
float g_selection_radius_mm;
float g_selection_radius_touch_mm;
int g_nCOMPortCheck = 32;
bool g_benableUDPNullHeader;

BasePlatform* g_BasePlatform = 0;
bool g_bportable = false;
wxString g_winPluginDir;
void* g_pi_manager = reinterpret_cast<void*>(1L);
wxString g_compatOS = PKG_TARGET;
wxString g_compatOsVersion = PKG_TARGET_VERSION;

Select* pSelect;
double g_n_arrival_circle_radius;
double g_PlanSpeed;
bool g_bTrackDaily;
int g_trackFilterMax;
wxString g_default_routepoint_icon;
double g_TrackDeltaDistance;
float g_fWaypointRangeRingsStep;
float g_ChartScaleFactorExp;
wxString g_default_wp_icon;
bool g_btouch;
int g_iWaypointRangeRingsNumber;
int g_iWaypointRangeRingsStepUnits;
wxColour g_colourWaypointRangeRingsColour;
bool g_bUseWptScaMin;
int g_iWpt_ScaMin;
bool g_bShowWptName;
int g_LayerIdx;
bool g_bOverruleScaMin;
int g_nTrackPrecision;
bool g_bIsNewLayer;
RouteList *pRouteList;
WayPointman* pWayPointMan;
int g_route_line_width;
int g_track_line_width;
RoutePoint* pAnchorWatchPoint1(nullptr);
RoutePoint* pAnchorWatchPoint2(nullptr);
bool g_bAllowShipToActive;
wxRect g_blink_rect;
bool g_bMagneticAPB;

Routeman* g_pRouteMan;



namespace safe_mode {
bool get_mode() { return false; }
}  // namespace safe_mode

wxString g_catalog_custom_url;
wxString g_catalog_channel;
wxLog* g_logger;
AisDecoder* g_pAIS;
Select* pSelectAIS;

/* comm_bridge context. */


// navutil_base context

int g_iDistanceFormat = 0;
int g_iSDMMFormat = 0;
int g_iSpeedFormat = 0;

wxDEFINE_EVENT(EVT_FOO, ObservedEvt);
wxDEFINE_EVENT(EVT_BAR, ObservedEvt);

std::string s_result;
std::string s_result2;
std::string s_result3;

int int_result0;

NavAddr::Bus s_bus;
AppMsg::Type s_apptype;

auto shared_navaddr_none = std::make_shared<NavAddr>();

wxLogStderr defaultLog;

class MsgCliApp : public wxAppConsole {
public:
  class Sink : public wxEvtHandler {
  private:
    ObservableListener listener;

  public:
    Sink() {
      Observable observable("1234");
      listener.Listen(observable, this, EVT_BAR);
      Bind(EVT_BAR, [&](ObservedEvt ev) {
        auto msg = ev.GetSharedPtr();
        auto n2000_msg = std::static_pointer_cast<const Nmea2000Msg>(msg);
        std::string s(n2000_msg->payload.begin(), n2000_msg->payload.end());
        s_result = s;
        s_bus = n2000_msg->bus;
      });
    }
  };

  class Source {
  public:
    Source() {
      std::string s("payload data");
      auto payload = std::vector<unsigned char>(s.begin(), s.end());
      auto id = static_cast<uint64_t>(1234);
      auto n2k_msg =
          std::make_shared<const Nmea2000Msg>(id, payload, shared_navaddr_none);
      Observable observable("1234");
      observable.Notify(n2k_msg);
    }
  };

  MsgCliApp() : wxAppConsole() {
    Sink sink;
    Source source;
    ProcessPendingEvents();
  }
};

class TransportCliApp : public wxAppConsole {
public:
  class Source {
  public:
    Source() {
      std::string s("payload data");
      auto payload = std::vector<unsigned char>(s.begin(), s.end());
      auto id = static_cast<uint64_t>(1234);
      auto msg =
          std::make_unique<Nmea2000Msg>(id, payload, shared_navaddr_none);
      NavMsgBus::GetInstance().Notify(std::move(msg));
    }
  };

  class Sink : public wxEvtHandler {
  public:
    Sink() {
      auto& t = NavMsgBus::GetInstance();
      Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234));
      listener.Listen(n2k_msg, this, EVT_FOO);

      Bind(EVT_FOO, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto n2k_msg = std::static_pointer_cast<const Nmea2000Msg>(ptr);
        std::string s(n2k_msg->payload.begin(), n2k_msg->payload.end());
        s_result = s;
        s_bus = n2k_msg->bus;
      });
    }
    ObservableListener listener;
  };

  TransportCliApp() : wxAppConsole() {
    Sink sink;
    Source source;
    ProcessPendingEvents();
  }
};

class All0183App : public wxAppConsole {
public:
  class Source {
  public:
    Source() {
      std::string payload("payload data");
      std::string id("GPGGA");
      auto msg1 =
          std::make_shared<Nmea0183Msg>(id, payload, shared_navaddr_none);
      auto msg_all = std::make_shared<const Nmea0183Msg>(*msg1, "ALL");
      NavMsgBus::GetInstance().Notify(std::move(msg_all));
    }
  };

  class Sink : public wxEvtHandler {
  public:
    Sink() {
      auto& t = NavMsgBus::GetInstance();
      listener.Listen(Nmea0183Msg::MessageKey("ALL"), this, EVT_FOO);

      Bind(EVT_FOO, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
        s_result = msg->payload;
        s_bus = msg->bus;
      });
    }
    ObservableListener listener;
  };

  All0183App() : wxAppConsole() {
    Sink sink;
    Source source;
    ProcessPendingEvents();
  }
};

class ListenerCliApp : public wxAppConsole {
public:
  class Source {
  public:
    Source() {
      std::string s("payload data");
      auto payload = std::vector<unsigned char>(s.begin(), s.end());
      auto id = static_cast<uint64_t>(1234);
      auto msg =
          std::make_unique<Nmea2000Msg>(id, payload, shared_navaddr_none);
      NavMsgBus::GetInstance().Notify(std::move(msg));
    }
  };

  class Sink : public wxEvtHandler {
  public:
    Sink() {
      ObservableListener listener;
      Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234));
      listener.Listen(n2k_msg, this, EVT_FOO);
      listeners.push_back(std::move(listener));
      Bind(EVT_FOO, [&](ObservedEvt ev) {
        auto n2k_msg = UnpackEvtPointer<Nmea2000Msg>(ev);
        std::string s(n2k_msg->payload.begin(), n2k_msg->payload.end());
        s_result = s;
        s_bus = n2k_msg->bus;
      });
    }
    std::vector<ObservableListener> listeners;
  };

  ListenerCliApp() : wxAppConsole() {
    Sink sink;
    Source source;
    ProcessPendingEvents();
  }

};

class AppmsgCliApp : public wxAppConsole {
public:
  class Source {
  public:
    Source() {
      Position pos(65.2211, 21.4433, Position::Type::NE);
      auto fix = std::make_shared<GnssFix>(pos, 1659345030);
      AppMsgBus::GetInstance().Notify(std::move(fix));
    }
  };

  class Sink : public wxEvtHandler {
  public:
    Sink() {
      listener.Listen(AppMsg(AppMsg::Type::GnssFix), this, EVT_FOO);
      Bind(EVT_FOO, [&](ObservedEvt ev) {
        auto msg = UnpackEvtPointer<const AppMsg>(ev);
        auto fix = std::static_pointer_cast<const GnssFix>(msg);
        if (fix == 0) {
          std::cerr << "Cannot cast pointer\n" << std::flush;
        } else {
          s_result = fix->pos.to_string();
          s_apptype = fix->type;
        }
      });
    }

    ObservableListener listener;
  };

  AppmsgCliApp() : wxAppConsole() {
    Sink sink;
    Source source;
    ProcessPendingEvents();
  };
};

using namespace std;

#ifdef _MSC_VER
const static string kSEP("\\");
#else
const static string kSEP("/");
#endif

class GuernseyApp : public wxAppConsole {
public:
  GuernseyApp(vector<string>& log) : wxAppConsole() {
    auto& msgbus = NavMsgBus::GetInstance();
    string path("..");
    path += kSEP + ".." + kSEP + "test" + kSEP + "testdata" + kSEP +
            "Guernesey-1659560590623.input.txt";
    auto driver = make_shared<FileCommDriver>("test-output.txt", path, msgbus);
    listener.Listen(Nmea0183Msg("GPGLL"), this, EVT_FOO);
    Bind(EVT_FOO, [&log](ObservedEvt ev) {
      auto ptr = ev.GetSharedPtr();
      auto n0183_msg = static_pointer_cast<const Nmea0183Msg>(ptr);
      log.push_back(n0183_msg->to_string());
    });
    driver->Activate();
    ProcessPendingEvents();
  }

  ObservableListener listener;
};

class PriorityApp : public wxAppConsole {
public:
  PriorityApp(string inputfile) : wxAppConsole() {
    auto& msgbus = NavMsgBus::GetInstance();
    string path("..");
    path += kSEP + ".." + kSEP + "test" + kSEP + "testdata" + kSEP + inputfile;
    auto driver = make_shared<FileCommDriver>(inputfile + ".log", path, msgbus);
    CommBridge comm_bridge;
    comm_bridge.Initialize();
    driver->Activate();
    ProcessPendingEvents();
  }
};

class PriorityApp2 : public wxAppConsole {
public:
  PriorityApp2(const char* msg1, const char* msg2) : wxAppConsole() {
    auto& msgbus = NavMsgBus::GetInstance();
    CommBridge comm_bridge;
    comm_bridge.Initialize();

    auto addr1 = std::make_shared<NavAddr>(NavAddr0183("interface1"));
    auto m1 = std::make_shared<const Nmea0183Msg>(
        Nmea0183Msg("GPGGA", msg1, addr1));
    auto addr2 = std::make_shared<NavAddr>(NavAddr0183("interface2"));
    auto m2 = std::make_shared<const Nmea0183Msg>(
        Nmea0183Msg("GPGGA", msg2, addr2));
    msgbus.Notify(m1);
    msgbus.Notify(m2);
    ProcessPendingEvents();

  }
};

class AisApp : public wxAppConsole {
public:
  AisApp(const char* type, const char* msg) : wxAppConsole() {
    SetAppName("opencpn_unittests");
    g_BasePlatform = new BasePlatform();
    pSelectAIS = new Select();
    pSelect = new Select();
    g_pAIS = new AisDecoder(AisDecoderCallbacks());
    auto& msgbus = NavMsgBus::GetInstance();
    CommBridge comm_bridge;
    comm_bridge.Initialize();

    auto addr1 = std::make_shared<NavAddr>(NavAddr0183("interface1"));
    auto m = std::make_shared<const Nmea0183Msg>(
        Nmea0183Msg(type, msg, addr1));
    msgbus.Notify(m);
    ProcessPendingEvents();
  }
};

class ObsTorture : public wxAppConsole {
public:

  class ObsListener : public wxEvtHandler {
  public:
    ObsListener(): wxEvtHandler() {
      wxDEFINE_EVENT(EVT_OBS_NOTIFY, ObservedEvt);
      m_listener.Listen("key1", this, EVT_OBS_NOTIFY);
      Bind(EVT_OBS_NOTIFY, [&](ObservedEvt& o) { OnNotify(o); });
    }

  private:
    void OnNotify(ObservedEvt& o) {
      auto s = UnpackEvtPointer<std::string>(o);
      EXPECT_TRUE(*s == "arg1");
      int_result0++;
    }

    ObservableListener m_listener;
  };

  ObsTorture() {
    ObsListener l1;
    Observable o("key1");
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i += 1) {
        threads.push_back(std::thread([&]{
           auto p = std::make_shared<const std::string>("arg1");
           o.Notify(p); }));
    }
    for (auto& t : threads) t.join();
    ProcessPendingEvents();
  }
};

class SillyDriver : public AbstractCommDriver {
public:
  SillyDriver() : AbstractCommDriver(NavAddr::Bus::TestBus, "silly") {}
  SillyDriver(const string& s) : AbstractCommDriver(NavAddr::Bus::TestBus, s) {}
  virtual ~SillyDriver() = default;

  virtual bool SendMessage(std::shared_ptr<const NavMsg> msg,
                           std::shared_ptr<const NavAddr> addr) {return false;}

  virtual void SetListener(DriverListener& listener) {}

  virtual void Activate(){};
};

class SillyListener : public DriverListener {
public:
  /** Handle a received message. */
  virtual void Notify(std::shared_ptr<const NavMsg> message) {
    s_result2 = NavAddr::BusToString(message->bus);

    auto base_ptr = message.get();
    auto n2k_msg = dynamic_cast<const Nmea2000Msg*>(base_ptr);
    s_result3 = n2k_msg->PGN.to_string();

    stringstream ss;
    std::for_each(n2k_msg->payload.begin(), n2k_msg->payload.end(),
                  [&ss](unsigned char c) { ss << static_cast<char>(c); });
    s_result = ss.str();
  }

  /** Handle driver status change. */
  virtual void Notify(const AbstractCommDriver& driver) {}
};

TEST(Messaging, ObservableMsg) {
  wxLog::SetActiveTarget(&defaultLog);
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  MsgCliApp app;
  EXPECT_EQ(s_result, string("payload data"));
  EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
};

TEST(Messaging, NavMsg) {
  wxLog::SetActiveTarget(&defaultLog);
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  TransportCliApp app;
  EXPECT_EQ(s_result, string("payload data"));
  EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
};

TEST(Messaging, All0183) {
  wxLog::SetActiveTarget(&defaultLog);
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  All0183App app;
  EXPECT_EQ(s_result, string("payload data"));
  EXPECT_EQ(NavAddr::Bus::N0183, s_bus);
};

#ifndef _MSC_VER
// FIXME (leamas) Fails on string representation of UTF degrees 0x00B0 on Win
TEST(Messaging, AppMsg) {
  wxLog::SetActiveTarget(&defaultLog);
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  AppmsgCliApp app;
  EXPECT_EQ(s_result, string("65°22,11N 21°44,33E"));
  EXPECT_EQ(s_apptype, AppMsg::Type::GnssFix);
};

#endif



//static void p1() { ObsListener l1; l1.Start(); }

TEST(Observable, torture) {
  int_result0  = 0;
  ObsTorture ot;
  EXPECT_EQ(int_result0, 10);
}

TEST(Drivers, Registry) {
  wxLog::SetActiveTarget(&defaultLog);
  auto driver = std::make_shared<SillyDriver>();
  auto& registry = CommDriverRegistry::GetInstance();
  registry.Activate(std::static_pointer_cast<AbstractCommDriver>(driver));
  auto drivers = registry.GetDrivers();
  EXPECT_EQ(registry.GetDrivers().size(), 1);
  EXPECT_EQ(registry.GetDrivers()[0]->iface, string("silly"));
  EXPECT_EQ(registry.GetDrivers()[0]->bus, NavAddr::Bus::TestBus);

  /* Add it again, should be ignored. */
  registry.Activate(std::static_pointer_cast<AbstractCommDriver>(driver));
  EXPECT_EQ(registry.GetDrivers().size(), 1);

  /* Add another one, should be accepted */
  auto driver2 = std::make_shared<SillyDriver>("orvar");
  registry.Activate(
      std::static_pointer_cast<AbstractCommDriver>(driver2));
  EXPECT_EQ(registry.GetDrivers().size(), 2);

  /* Remove one, leaving one in place. */
  registry.Deactivate(
      std::static_pointer_cast<AbstractCommDriver>(driver2));
  EXPECT_EQ(registry.GetDrivers().size(), 1);

  /* Remove it again, should be ignored. */
  registry.Deactivate(
      std::static_pointer_cast<AbstractCommDriver>(driver2));
  EXPECT_EQ(registry.GetDrivers().size(), 1);
}

TEST(Navmsg2000, to_string) {
  wxLog::SetActiveTarget(&defaultLog);
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  auto msg = std::make_shared<Nmea2000Msg>(id, payload, shared_navaddr_none);
  EXPECT_EQ(string("nmea2000 n2000-1234 1234 7061796c6f61642064617461"),
            msg->to_string());
}

TEST(FileDriver, Registration) {
  wxLog::SetActiveTarget(&defaultLog);
  auto driver = std::make_shared<FileCommDriver>("test-output.txt");
  auto& registry = CommDriverRegistry::GetInstance();
  int start_size = registry.GetDrivers().size();
  driver->Activate();
  auto drivers = registry.GetDrivers();
  EXPECT_EQ(registry.GetDrivers().size(), start_size + 1);
}

TEST(FileDriver, output) {
  wxLog::SetActiveTarget(&defaultLog);
  auto driver = std::make_shared<FileCommDriver>("test-output.txt");
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  Nmea2000Msg msg(id, payload, shared_navaddr_none);
  remove("test-output.txt");

  driver->SendMessage(std::make_shared<const Nmea2000Msg>(msg),
                      std::make_shared<const NavAddr>());
  std::ifstream f("test-output.txt");
  stringstream ss;
  ss << f.rdbuf();
  EXPECT_EQ(ss.str(),
            string("nmea2000 n2000-1234 1234 7061796c6f61642064617461"));
}

#if 0
// FIXME (comm_drv_file, see FIXME there)
TEST(FileDriver, input) {
  wxLog::SetActiveTarget(&defaultLog);
  auto driver = std::make_shared<FileCommDriver>("test-output.txt");
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  Nmea2000Msg msg(id, payload, shared_navaddr_none);
  remove("test-output.txt");
  driver->SendMessage(std::make_shared<Nmea2000Msg>(msg),
                      std::make_shared<NavAddr>());

  SillyListener listener;
  auto indriver = std::make_shared<FileCommDriver>("/tmp/foo.txt",
                                                   "test-output.txt", listener);
  indriver->Activate();
  EXPECT_EQ(s_result2, string("nmea2000"));
  EXPECT_EQ(s_result3, string("1234"));
  EXPECT_EQ(s_result, string("payload data"));
}
#endif

TEST(Listeners, vector) {
  wxLog::SetActiveTarget(&defaultLog);
  s_result = "";
  ListenerCliApp app;
  EXPECT_EQ(s_result, string("payload data"));
  EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
};

TEST(Guernsey, play_log) {
  wxLog::SetActiveTarget(&defaultLog);
  vector<string> log;
  GuernseyApp app(log);
  EXPECT_EQ(log.size(), 14522);
}

TEST(FindDriver, lookup) {
  wxLog::SetActiveTarget(&defaultLog);
  std::vector<DriverPtr> drivers;
  std::vector<std::string> ifaces{"foo", "bar", "foobar"};
  for (const auto& iface : ifaces) {
    drivers.push_back(std::make_shared<SillyDriver>(SillyDriver(iface)));
  }
  auto found = FindDriver(drivers, "bar");
  EXPECT_EQ(found->iface, string("bar"));
  found = FindDriver(drivers, "baz");
  EXPECT_FALSE(found);
  auto file_drv = std::dynamic_pointer_cast<const FileCommDriver>(found);
  EXPECT_EQ(file_drv.get(), nullptr);
}

TEST(Registry, persistence) {
  wxLog::SetActiveTarget(&defaultLog);
  int start_size = 0;
  if (true) {  // a scope
    auto driver = std::make_shared<SillyDriver>();
    auto& registry = CommDriverRegistry::GetInstance();
    start_size = registry.GetDrivers().size();
    registry.Activate(
        std::static_pointer_cast<AbstractCommDriver>(driver));
  }
  auto& registry = CommDriverRegistry::GetInstance();
  auto drivers = registry.GetDrivers();
  EXPECT_EQ(registry.GetDrivers().size(), start_size + 1);
  EXPECT_EQ(registry.GetDrivers()[start_size]->iface, string("silly"));
  EXPECT_EQ(registry.GetDrivers()[start_size]->bus, NavAddr::Bus::TestBus);
}

TEST(Position, ParseGGA) {
  wxLog::SetActiveTarget(&defaultLog);
  Position p = Position::ParseGGA("5800.602,N,01145.789,E");
  EXPECT_NEAR(p.lat, 58.010033, 0.0001);
  EXPECT_NEAR(p.lon, 11.763150, 0.0001);
}

TEST(Priority, Framework) {
  wxLog::SetActiveTarget(&defaultLog);
  PriorityApp app("stupan.se-10112-tcp.log.input");
  EXPECT_NEAR(gLat, 57.6460, 0.001);
  EXPECT_NEAR(gLon, 11.7130, 0.001);
}

TEST(Priority, DifferentSource) {
  wxLog::SetActiveTarget(&defaultLog);
  const char* const GPGGA_1 =
    "$GPGGA,092212,5759.097,N,01144.345,E,1,06,1.9,3.5,M,39.4,M,,*4C";
  const char* const GPGGA_2 =
    "$GPGGA,092212,5755.043,N,01344.585,E,1,06,1.9,3.5,M,39.4,M,,*4C";
  PriorityApp2 app(GPGGA_1, GPGGA_2);
  Position p = Position::ParseGGA("5759.097,N,01144.345,E");
  EXPECT_NEAR(gLat, p.lat, 0.0001);
  EXPECT_NEAR(gLon, p.lon, 0.0001);
}

TEST(AIS, Decoding) {
  const char* AISVDO_1 = "!AIVDO,1,1,,,B3uBrjP0;h=Koh`Bp1tEowrUsP06,0*31";
  GenericPosDatEx gpd;
  AisError status = DecodeSingleVDO(AISVDO_1, &gpd);
  EXPECT_EQ(status, AIS_NoError);
}

TEST(AIS, AISVDO) {
  wxLog::SetActiveTarget(&defaultLog);
  const char* AISVDO_1 = "!AIVDO,1,1,,,B3uBrjP0;h=Koh`Bp1tEowrUsP06,0*31";
  int MMSI = 123456;
  AisApp app("AIVDO", AISVDO_1);

  EXPECT_NEAR(gLat, 57.985758, 0.0001);
  EXPECT_NEAR(gLon, 11.740108, 0.0001);
}

TEST(AIS, AISVDM) {
  const char* AISVDM_1 = "!AIVDM,1,1,,A,1535SB002qOg@MVLTi@b;H8V08;?,0*47";
  int MMSI = 338781000;

  AisApp app("AIVDM", AISVDM_1);
  auto found = g_pAIS->GetTargetList().find(MMSI);
  EXPECT_NE(found, g_pAIS->GetTargetList().end());
  if (found != g_pAIS->GetTargetList().end()) {
    EXPECT_NEAR(found->second->Lat, 49.93760, 0.0001);
    EXPECT_NEAR(found->second->Lon, -3.65751, 0.0001);
  }
}

TEST(PluginApi, SignalK) {
  const char* const kJsonMsg = R"""(
  {
      "foo": 1,
      "bar": "bar value",
      "list" : [1, 2, 3]
  }
  )""";

  SignalkMsg signalk_msg("ownship_ctx", "global_ctx", kJsonMsg);

  const wxEventTypeTag<ObservedEvt> EvtTest(wxNewEventType());
  ObservedEvt ev(EvtTest);
  ev.SetSharedPtr(std::make_shared<SignalkMsg>(signalk_msg));

  auto payload = GetSignalkPayload(ev);
  const auto msg = *std::static_pointer_cast<const wxJSONValue>(payload);
  EXPECT_EQ(0, msg.ItemAt("ErrorCount").AsInt());
  EXPECT_EQ(0, msg.ItemAt("WarningCount").AsInt());
  EXPECT_EQ(wxString("ownship_ctx"), msg.ItemAt("ContextSelf").AsString());
  EXPECT_EQ(wxString("global_ctx"), msg.ItemAt("Context").AsString());
  EXPECT_EQ(1, msg.ItemAt("Data").ItemAt("foo").AsInt());
  EXPECT_EQ(wxString("bar value"), msg.ItemAt("Data").ItemAt("bar").AsString());
  EXPECT_EQ(1, msg.ItemAt("Data").ItemAt("list").ItemAt(0).AsInt());
}
