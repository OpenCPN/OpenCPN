#include "config.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>


#include <wx/app.h>
#include <wx/event.h>
#include <wx/evtloop.h>
#include <wx/fileconf.h>
#include <wx/jsonval.h>
#include <wx/timer.h>

#include <gtest/gtest.h>

#include "model/ais_decoder.h"
#include "model/ais_state_vars.h"
#include "model/ais_defs.h"
#include "model/cli_platform.h"
#include "model/comm_ais.h"
#include "model/comm_appmsg_bus.h"
#include "model/comm_bridge.h"
#include "model/comm_drv_file.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/config_vars.h"
#include "model/ipc_api.h"
#include "model/logger.h"
#include "model/multiplexer.h"
#include "observable_confvar.h"
#include "ocpn_plugin.h"
#include "model/ocpn_types.h"
#include "model/ocpn_utils.h"
#include "model/own_ship.h"
#include "model/S57ClassRegistrar.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/std_instance_chk.h"
#include "wx_instance_chk.h"

// Macos up to 10.13
#if defined(__clang_major__) && (__clang_major__ < 15)
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

// Ubuntu Bionic:
#elif !defined(__clang_major__) && defined(__GNUC__) && (__GNUC__ < 8)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#else
#include <filesystem>
#include <utility>
namespace fs = std::filesystem;
#endif

using namespace std::literals::chrono_literals;

void* g_pi_manager = reinterpret_cast<void*>(1L);

wxLog* g_logger;

wxDEFINE_EVENT(EVT_FOO, ObservedEvt);
wxDEFINE_EVENT(EVT_BAR, ObservedEvt);

std::string s_result;
std::string s_result2;
std::string s_result3;

int int_result0;
bool bool_result0;

NavAddr::Bus s_bus;
AppMsg::Type s_apptype;

auto shared_navaddr_none = std::make_shared<NavAddr>();

wxLogStderr defaultLog;

#ifdef _MSC_VER
int setenv(const char* name, const char* value, bool overwrite) {
  if (!overwrite) {
     if (getenv(name)) return 1;
  }
  return _putenv_s(name, value);
}
#endif

static void ConfigSetup() {
  const auto config_orig = fs::path(TESTDATA) / "opencpn.conf";
  const auto config_path = fs::path(CMAKE_BINARY_DIR) / "opencpn.conf";
  std::remove(config_path.string().c_str());
  fs::copy(config_orig, config_path);
  InitBaseConfig(new wxFileConfig("", "", config_path.string()));
  g_BasePlatform = new BasePlatform();
  pSelectAIS = new Select();
  pSelect = new Select();
  g_pAIS = new AisDecoder(AisDecoderCallbacks());
}

class SillyDriver : public AbstractCommDriver {
public:
  SillyDriver() : AbstractCommDriver(NavAddr::Bus::TestBus, "silly") {}
  SillyDriver(const std::string& s) : AbstractCommDriver(NavAddr::Bus::TestBus, s) {}
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

    std::stringstream ss;
    std::for_each(n2k_msg->payload.begin(), n2k_msg->payload.end(),
                  [&ss](unsigned char c) { ss << static_cast<char>(c); });
    s_result = ss.str();
  }

  /** Handle driver status change. */
  virtual void Notify(const AbstractCommDriver& driver) {}
};


class BasicTest  : public wxAppConsole  {
public:
  BasicTest() {
    const auto config_orig = fs::path(TESTDATA) / "opencpn.conf";
    const auto config_path = fs::path(CMAKE_BINARY_DIR) / "opencpn.conf";
    auto logfile = fs::path(CMAKE_BINARY_DIR) / "unittests.log";
    wxLog::SetActiveTarget(new OcpnLog(logfile.string().c_str()));
    wxLog::SetLogLevel(wxLOG_Debug);
    wxLog::FlushActive();
    std::remove(config_path.string().c_str());
    fs::copy(config_orig, config_path);
    InitBaseConfig(new wxFileConfig("", "", config_path.string()));
    g_BasePlatform = new BasePlatform();
    pSelectAIS = new Select();
    pSelect = new Select();
    g_pAIS = new AisDecoder(AisDecoderCallbacks());

    Work();
  }
  virtual void Work() { std::this_thread::sleep_for(20ms); }
};


class MsgCliApp : public BasicTest {
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

  MsgCliApp() : BasicTest() {}

  void Work() {
    s_result = "";
    s_bus = NavAddr::Bus::Undef;
    Sink sink;
    Source source;
    ProcessPendingEvents();
    EXPECT_EQ(s_result, std::string("payload data"));
    EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
  }
};

class TransportCliApp : public BasicTest {
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

  TransportCliApp() : BasicTest() {}

  void Work() {
    s_result = "";
    s_bus = NavAddr::Bus::Undef;
    Sink sink;
    Source source;
    ProcessPendingEvents();
    EXPECT_EQ(s_result, std::string("payload data"));
    EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
  }
};

class All0183App : public BasicTest {
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

  All0183App() : BasicTest() { }

  void Work() {
    s_bus = NavAddr::Bus::Undef;
    s_result = "";
    Sink sink;
    Source source;
    ProcessPendingEvents();
    EXPECT_EQ(s_result, std::string("payload data"));
    EXPECT_EQ(NavAddr::Bus::N0183, s_bus);
  }
};

class ListenerCliApp : public BasicTest {
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

  ListenerCliApp() : BasicTest() { }

  void Work() {
    s_bus = NavAddr::Bus::Undef;
    s_result = "";
    Sink sink;
    Source source;
    ProcessPendingEvents();
    EXPECT_EQ(s_result, std::string("payload data"));
    EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
  }

};

class AppmsgCliApp : public BasicTest {
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

  AppmsgCliApp() : BasicTest() { }
   void Work() {
    s_apptype =  AppMsg::Type::Undef;
    s_bus = NavAddr::Bus::Undef;
    Sink sink;
    Source source;
    ProcessPendingEvents();
    EXPECT_EQ(s_result, std::string("65°22,11N 21°44,33E"));
    EXPECT_EQ(s_apptype, AppMsg::Type::GnssFix);
  };
};

using namespace std;

#ifdef _MSC_VER
const static std::string kSEP("\\");
#else
const static std::string kSEP("/");
#endif

class GuernseyApp : public BasicTest {
public:
  GuernseyApp() : BasicTest() { }

  void Work() {
    vector<string> log;
    string path(TESTDATA);
    path += kSEP + "Guernesey-1659560590623.input.txt";
    auto& msgbus = NavMsgBus::GetInstance();
    auto driver = make_shared<FileCommDriver>("test-output.txt", path, msgbus);
    listener.Listen(Nmea0183Msg("GPGLL"), this, EVT_FOO);
    Bind(EVT_FOO, [&log](ObservedEvt ev) {
      auto ptr = ev.GetSharedPtr();
      auto n0183_msg = static_pointer_cast<const Nmea0183Msg>(ptr);
      log.push_back(n0183_msg->to_string());
    });
    driver->Activate();
    ProcessPendingEvents();
    EXPECT_EQ(log.size(), 14522);
  }

  ObservableListener listener;
};


class FindDriverApp : public BasicTest {
public:
  FindDriverApp() : BasicTest() { }

  void Work() override {
    std::vector<DriverPtr> drivers;
    std::vector<std::string> ifaces{"foo", "bar", "foobar"};
    for (const auto& iface : ifaces) {
      drivers.push_back(std::make_shared<SillyDriver>(SillyDriver(iface)));
    }
    auto found = FindDriver(drivers, "bar");
    EXPECT_EQ(found->iface, std::string("bar"));
    found = FindDriver(drivers, "baz");
    EXPECT_FALSE(found);
    auto file_drv = std::dynamic_pointer_cast<const FileCommDriver>(found);
    EXPECT_EQ(file_drv.get(), nullptr);
  }
};

class PositionApp : public BasicTest {
public:
  void Work() override {
    Position p = Position::ParseGGA("5800.602,N,01145.789,E");
    EXPECT_NEAR(p.lat, 58.010033, 0.0001);
    EXPECT_NEAR(p.lon, 11.763150, 0.0001);
  }
};

class PriorityApp : public wxAppConsole {
public:
  PriorityApp(string inputfile) : wxAppConsole() {
    ConfigSetup();
    gLat = 0.0;
    gLon = 0.0;
    auto& msgbus = NavMsgBus::GetInstance();
    std::string path(TESTDATA);
    path += kSEP + inputfile;
    auto driver = make_shared<FileCommDriver>(inputfile + ".log", path, msgbus);
    CommBridge comm_bridge;
    comm_bridge.Initialize();
    driver->Activate();
    ProcessPendingEvents();
    EXPECT_NEAR(gLat, 57.6460, 0.001);
    EXPECT_NEAR(gLon, 11.7130, 0.001);
  }
};

class RegistryPersistApp : public BasicTest {
public:
  RegistryPersistApp() : BasicTest() { }

  void Work() {
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
    EXPECT_EQ(registry.GetDrivers()[start_size]->iface, std::string("silly"));
    EXPECT_EQ(registry.GetDrivers()[start_size]->bus, NavAddr::Bus::TestBus);
  }
};

class PriorityApp2 : public BasicTest {
public:
  PriorityApp2() : BasicTest() { }
  void Work() override {
    const char* const GPGGA_1 =
      "$GPGGA,092212,5759.097,N,01144.345,E,1,06,1.9,3.5,M,39.4,M,,*4C";
    const char* const GPGGA_2 =
      "$GPGGA,092212,5755.043,N,01344.585,E,1,06,1.9,3.5,M,39.4,M,,*4C";
    auto& msgbus = NavMsgBus::GetInstance();
    CommBridge comm_bridge;
    comm_bridge.Initialize();

    auto addr1 = std::make_shared<NavAddr>(NavAddr0183("interface1"));
    auto m1 = std::make_shared<const Nmea0183Msg>(
        Nmea0183Msg("GPGGA", GPGGA_1, addr1));
    auto addr2 = std::make_shared<NavAddr>(NavAddr0183("interface2"));
    auto m2 = std::make_shared<const Nmea0183Msg>(
        Nmea0183Msg("GPGGA", GPGGA_2, addr2));
    msgbus.Notify(m1);
    msgbus.Notify(m2);
    ProcessPendingEvents();
    Position p = Position::ParseGGA("5759.097,N,01144.345,E");
    EXPECT_NEAR(gLat, p.lat, 0.0001);
    EXPECT_NEAR(gLon, p.lon, 0.0001);
  }
};

class AisVdoApp : public BasicTest {
public:
  AisVdoApp() : BasicTest() {
    gLat = 0.0;
    gLon = 0.0;
    const char* AISVDO_1 = "!AIVDO,1,1,,,B3uBrjP0;h=Koh`Bp1tEowrUsP06,0*31";
    int MMSI = 123456;
    auto& msgbus = NavMsgBus::GetInstance();
    CommBridge comm_bridge;
    comm_bridge.Initialize();

    auto addr1 = std::make_shared<NavAddr>(NavAddr0183("interface1"));
    auto m = std::make_shared<const Nmea0183Msg>(
        Nmea0183Msg("AIVDO", AISVDO_1, addr1));
    msgbus.Notify(m);
    ProcessPendingEvents();
    EXPECT_NEAR(gLat, 57.985758, 0.0001);
    EXPECT_NEAR(gLon, 11.740108, 0.0001);
  }
};

// GetSignalkPayload() introduced in 1.19
#if API_VERSION_MINOR > 18
class SignalKApp : public BasicTest {
public:
  SignalKApp() : BasicTest() {}
  void Work() override {
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
    EXPECT_EQ(wxString("bar value"),
              msg.ItemAt("Data").ItemAt("bar").AsString());
    EXPECT_EQ(1, msg.ItemAt("Data").ItemAt("list").ItemAt(0).AsInt());
  }
};
#endif

class AisDecodeApp : public BasicTest {
public:
  AisDecodeApp() : BasicTest() { }
  void Work() override {
    const char* AISVDO_1 = "!AIVDO,1,1,,,B3uBrjP0;h=Koh`Bp1tEowrUsP06,0*31";
    GenericPosDatEx gpd;
    AisError status = DecodeSingleVDO(AISVDO_1, &gpd);
    EXPECT_EQ(status, AIS_NoError);
  }
};

class AisVdmApp : public BasicTest {
public:
  AisVdmApp() : BasicTest() {
    const char* AISVDM_1 = "!AIVDM,1,1,,A,1535SB002qOg@MVLTi@b;H8V08;?,0*47";
    int MMSI = 338781000;
    auto& msgbus = NavMsgBus::GetInstance();
    CommBridge comm_bridge;
    comm_bridge.Initialize();

    auto addr1 = std::make_shared<NavAddr>(NavAddr0183("interface1"));
    auto m = std::make_shared<const Nmea0183Msg>(
        Nmea0183Msg("AIVDM", AISVDM_1, addr1));
    msgbus.Notify(m);
    ProcessPendingEvents();

    auto found = g_pAIS->GetTargetList().find(MMSI);
    EXPECT_NE(found, g_pAIS->GetTargetList().end());
    if (found != g_pAIS->GetTargetList().end()) {
      EXPECT_NEAR(found->second->Lat, 49.93760, 0.0001);
      EXPECT_NEAR(found->second->Lon, -3.65751, 0.0001);
    }
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

#ifdef HAVE_UNISTD_H
class WxInstanceChk : public BasicTest {
public:
  WxInstanceChk() : BasicTest()  { }

  void Work() override {
    auto cmd = std::string(CMAKE_BINARY_DIR) +  "/test/wx-instance";
    auto stream = popen(cmd.c_str(), "w");
    std::this_thread::sleep_for(200ms);    // Random time to catch up w IO
    WxInstanceCheck check1;
    EXPECT_TRUE(check1.IsMainInstance());
    fputs("foobar\n", stream);
    pclose(stream);
    std::this_thread::sleep_for(200ms);
    WxInstanceCheck check2;
    EXPECT_TRUE(check2.IsMainInstance());
  }
};

class StdInstanceTest : public BasicTest {
public:
  StdInstanceTest(): BasicTest() {}

  void Work() override {
    auto cmd = std::string(CMAKE_BINARY_DIR) +  "/test/std-instance";
    auto stream = popen(cmd.c_str(), "w");
    std::this_thread::sleep_for(200ms);
    StdInstanceCheck check1;
    EXPECT_TRUE(check1.IsMainInstance());
    fputs("foobar\n", stream);
    pclose(stream);
    std::this_thread::sleep_for(200ms);
    StdInstanceCheck check2;
    EXPECT_FALSE(check2.IsMainInstance());
  }
};
#endif


static void UpdateBool0() {
    bool_result0 = true;
};

#ifdef __unix__
class StdInstanceCheck2 : public wxAppConsole {
public:
  StdInstanceCheck2() {
    SetAppName("opencpn_unittests");
    auto cmd = std::string(CMAKE_BINARY_DIR) +  "/test/std-instance";
    auto stream = popen(cmd.c_str(), "w");
    std::this_thread::sleep_for(200ms);
    StdInstanceCheck check1;
    EXPECT_FALSE(check1.IsMainInstance());
    fputs("foobar\n", stream);
    pclose(stream);
    std::this_thread::sleep_for(200ms);
    StdInstanceCheck check2;
    EXPECT_TRUE(check2.IsMainInstance());
  }
};

#endif

#ifdef __unix__
class IpcClientTest : public BasicTest {
public:
   IpcClientTest() : BasicTest() {
    std::string server_cmd(CMAKE_BINARY_DIR);
    server_cmd += "/test/cli-server";
    stream = popen(server_cmd.c_str(), "r");
    std::this_thread::sleep_for(25ms);    // Need some time to start server
    char buff[1024];
    char* line = fgets(buff, sizeof(buff), stream);   // initial line, throw.
    EXPECT_TRUE(line);
  }
  static std::string GetSocketPath() {
    wxFileName path("~/.opencpn", "opencpn-ipc");
    path.Normalize(wxPATH_NORM_TILDE);
    auto dirpath = path.GetPath();
    if (!wxFileName::DirExists(dirpath)) wxFileName::Mkdir(dirpath);
    return path.GetFullPath().ToStdString();
  }
protected:
  FILE* stream;
};

class CliRaise : public IpcClientTest {
public:
  CliRaise() : IpcClientTest() {
    IpcClient client(IpcClientTest::GetSocketPath());
    auto result = client.SendRaise();
    char buff[1024];
    char* line = fgets(buff, sizeof(buff), stream);
    EXPECT_TRUE(line);
    EXPECT_EQ(ocpn::trim(std::string(line)), "raise");

    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second.size(), 0);
    EXPECT_EQ(pclose(stream), 0);
  }
};

class IpcGetEndpoint : public IpcClientTest {
public:
  IpcGetEndpoint() {
    IpcClient client(IpcClientTest::GetSocketPath());
    auto result = client.GetRestEndpoint();

    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second, "0.0.0.0/api");
    EXPECT_EQ(pclose(stream), 0);
  }
};

class IpcOpen : public IpcClientTest {
public:
  IpcOpen() {
    IpcClient client(IpcClientTest::GetSocketPath());
    auto result = client.SendOpen("/foo/bar");

    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second, "/foo/bar");
    EXPECT_EQ(pclose(stream), 0);
  }
};
#endif  // __unix__

TEST(MsgFixture, ObservableMsg) { MsgCliApp app; };

TEST(Messaging, NavMsg) { TransportCliApp app; };

TEST(Messaging, All0183) { All0183App app; };

#ifndef _MSC_VER
// FIXME (leamas) Fails on string representation of UTF degrees 0x00B0 on Win
TEST(Messaging, AppMsg) { AppmsgCliApp app; };
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

TEST(Listeners, vector) { ListenerCliApp app; };

TEST(Guernsey, play_log) { GuernseyApp app; }

TEST(FindDriver, lookup) { FindDriverApp app; }

TEST(Registry, persistence) { RegistryPersistApp app; }

TEST(Position, ParseGGA) { PositionApp app; };

TEST(Priority, Framework) { PriorityApp app("stupan.se-10112-tcp.log.input"); }

TEST(Priority, DifferentSource) { PriorityApp2 app; }

TEST(AIS, Decoding) { AisDecodeApp app; }

TEST(AIS, AISVDO) { AisVdoApp app; }

TEST(AIS, AISVDM) { AisVdmApp app; }

#if API_VERSION_MINOR > 18
TEST(PluginApi, SignalK) { SignalKApp app; }
#endif

#ifdef HAVE_UNISTD_H
TEST(Instance, StdInstanceChk) { StdInstanceTest check; }
#endif

TEST(Instance, WxInstanceChk) { WxInstanceCheck check; }

#if !defined(FLATPAK) && defined(__unix__)
TEST(IpcClient, IpcGetEndpoint) { IpcGetEndpoint run_test; }

TEST(IpcClient, Raise) { CliRaise run_test; }

TEST(IpcClient, Open) { IpcOpen run_test; }
#endif
