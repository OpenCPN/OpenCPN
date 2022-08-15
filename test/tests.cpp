
#include "config.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "wx/event.h"
#include "wx/app.h"

#include <gtest/gtest.h>

#include "BasePlatform.h"
#include "comm_appmsg_bus.h"
#include "comm_drv_file.h"
#include "comm_drv_registry.h"
#include "observable_msg.h"

BasePlatform* g_BasePlatform = 0;
bool g_bportable = false;
wxString g_winPluginDir;
wxConfigBase* pBaseConfig = 0;
void* g_pi_manager = reinterpret_cast<void*>(1L);
wxString g_compatOS = PKG_TARGET;
wxString g_compatOsVersion = PKG_TARGET_VERSION;

namespace safe_mode { bool get_mode() { return false; } }

wxString g_catalog_custom_url;
wxString g_catalog_channel;
wxLog* g_logger;

wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
wxDEFINE_EVENT(EVT_BAR, wxCommandEvent);

std::string s_result;
std::string s_result2;
std::string s_result3;

NavAddr::Bus s_bus;
AppMsg::Type s_apptype;

auto shared_navaddr_none = std::make_shared<NavAddr>(NavAddrNone());

class MsgCliApp : public wxAppConsole {
public:
  class Sink:  public wxEvtHandler {
  private:
    ObservedVarListener listener;
  public:
    Sink() {
      ObservableMsg observable("1234");
      listener = observable.GetListener(this, EVT_BAR);
      Bind(EVT_BAR, [&](wxCommandEvent ev) {
        auto msg = get_navmsg_ptr(ev);
        auto n2000_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
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
      auto n2k_msg = std::make_shared<const Nmea2000Msg>(id, payload,
                                                         shared_navaddr_none);
      ObservableMsg observable("1234");
      observable.notify(n2k_msg);
    }
  };

  MsgCliApp() : wxAppConsole() {
    Sink sink;
    Source source;
    ProcessPendingEvents();
  }
};


class TransportCliApp: public wxAppConsole {
public:
  class Source {
  public:
    Source() {
      std::string s("payload data");
      auto payload = std::vector<unsigned char>(s.begin(), s.end());
      auto id = static_cast<uint64_t>(1234);
      auto msg = std::make_unique<Nmea2000Msg>(id, payload,
                                               shared_navaddr_none);
      NavMsgBus::GetInstance().Notify(std::move(msg));
    }
  };

  class Sink: public wxEvtHandler {
  public:
    Sink() {
      auto& t = NavMsgBus::GetInstance();
      Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234));
      listener = t.GetListener(EVT_FOO, this, n2k_msg);

      Bind(EVT_FOO, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n2k_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(message);
        std::string s(n2k_msg->payload.begin(), n2k_msg->payload.end());
        s_result = s;
        s_bus = n2k_msg->bus;
      });
    }
    ObservedVarListener listener;
  };

  TransportCliApp() : wxAppConsole() {
    Sink sink;
    Source source;
    ProcessPendingEvents();
  }
};


class ListenerCliApp: public wxAppConsole {
public:
  class Source {
  public:
    Source() {
      std::string s("payload data");
      auto payload = std::vector<unsigned char>(s.begin(), s.end());
      auto id = static_cast<uint64_t>(1234);
      auto msg = std::make_unique<Nmea2000Msg>(id, payload,
                                               shared_navaddr_none);
      NavMsgBus::GetInstance().Notify(std::move(msg));
    }
  };

  class Sink: public wxEvtHandler {
  public:
    Sink() {
      auto& t = NavMsgBus::GetInstance();
      Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234));
      listeners.push_back(t.GetListener(EVT_FOO, this, n2k_msg));
      Bind(EVT_FOO, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n2k_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(message);
        std::string s(n2k_msg->payload.begin(), n2k_msg->payload.end());
        s_result = s;
        s_bus = message->bus;
      });
    }
    std::vector<ObservedVarListener> listeners;
  };

  ListenerCliApp() : wxAppConsole() {
    Sink sink;
    Source source;
    ProcessPendingEvents();
  }
};


class AppmsgCliApp: public wxAppConsole {
public:

  class Source {
  public:
    Source() {
      Position pos(65.2211, 21.4433, Position::Type::NE);
      auto fix = std::make_shared<GnssFix>(pos, 1659345030);
      AppMsgBus::GetInstance().Notify(std::move(fix));
    }
  };

  class Sink: public wxEvtHandler {
  public:
    Sink() {
      auto& a = AppMsgBus::GetInstance();
      listener = a.GetListener(EVT_FOO, this, AppMsg::Type::GnssFix);

      Bind(EVT_FOO, [&](wxCommandEvent ev) {
        auto message = get_appmsg_ptr(ev);
        std::cout << message->TypeToString(message->type) << "\n";
        auto fix = std::dynamic_pointer_cast<const GnssFix>(message);
        if (fix == 0) {
          std::cerr << "Cannot cast pointer\n" << std::flush;
        } else {
          s_result = fix->pos.to_string();
          s_apptype = fix->type;
        }
      });
    }
    ObservedVarListener listener;
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

class GuernseyApp: public wxAppConsole {
public:
  GuernseyApp(vector<string>& log) : wxAppConsole() {
    auto& msgbus = NavMsgBus::GetInstance();
    string path("..");
    path += kSEP + ".." +  kSEP + "test" + kSEP + "testdata" + kSEP
        +  "Guernesey-1659560590623.input.txt";
    auto driver =
        make_shared<FileCommDriver>("test-output.txt", path, msgbus);
    auto listener = msgbus.GetListener(EVT_FOO, this, Nmea0183Msg("GPGLL"));
    Bind(EVT_FOO, [&log](wxCommandEvent ev) {
      auto message = get_navmsg_ptr(ev);
      auto n0183_msg = dynamic_pointer_cast<const Nmea0183Msg>(message);
      log.push_back(n0183_msg->to_string());
    });
    driver->Activate();
    ProcessPendingEvents();
  }
};


class SillyDriver: public AbstractCommDriver {
public:

  SillyDriver() : AbstractCommDriver(NavAddr::Bus::TestBus, "silly") {}
  SillyDriver(const string& s)
      : AbstractCommDriver(NavAddr::Bus::TestBus, s) {}

  virtual void SendMessage(const NavMsg& msg, const NavAddr& addr) {}

  virtual void SetListener(DriverListener& listener) {}

  virtual void Activate() {};
};



class SillyListener: public DriverListener {
public:
  /** Handle a received message. */
  virtual void Notify(std::unique_ptr<const NavMsg> message)  {
    s_result2 = NavAddr::BusToString(message->bus);

    auto base_ptr = message.get();
    auto n2k_msg = dynamic_cast<const Nmea2000Msg*>(base_ptr);
    s_result3 = n2k_msg->name.to_string();

    stringstream ss;
    std::for_each(n2k_msg->payload.begin(), n2k_msg->payload.end(),
                  [&ss](unsigned char c) {ss << static_cast<char>(c); });
    s_result = ss.str();
  }

  /** Handle driver status change. */
  virtual void Notify(const AbstractCommDriver& driver) {}
};


TEST(Messaging, ObservableMsg) {
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  MsgCliApp app;
  EXPECT_EQ(s_result, string("payload data"));
  EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
};


TEST(Messaging, NavMsg) {
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  TransportCliApp app;
  EXPECT_EQ(s_result, string("payload data"));
  EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
};

#ifndef _MSC_VER
// FIXME (leamas) Fails on string representation of UTF degrees 0x00B0 on Win
TEST(Messaging, AppMsg) {
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  AppmsgCliApp app;
  EXPECT_EQ(s_result, string("65°22,11N 21°44,33E"));
  EXPECT_EQ(s_apptype, AppMsg::Type::GnssFix);
};

#endif


TEST(Drivers, Registry) {
  auto driver = std::make_shared<const SillyDriver>();
  auto& registry = CommDriverRegistry::getInstance();
  registry.Activate(std::static_pointer_cast<const AbstractCommDriver>(driver));
  auto drivers = registry.GetDrivers();
  EXPECT_EQ(registry.GetDrivers().size(), 1);
  EXPECT_EQ(registry.GetDrivers()[0]->iface, string("silly"));
  EXPECT_EQ(registry.GetDrivers()[0]->bus, NavAddr::Bus::TestBus);

  /* Add it again, should be ignored. */
  registry.Activate(std::static_pointer_cast<const AbstractCommDriver>(driver));
  EXPECT_EQ(registry.GetDrivers().size(), 1);

  /* Add another one, should be accepted */
  auto driver2 = std::make_shared<const SillyDriver>("orvar");
  registry.Activate(std::static_pointer_cast<const AbstractCommDriver>(driver2));
  EXPECT_EQ(registry.GetDrivers().size(), 2);

  /* Remove one, leaving one in place. */
  registry.Deactivate(std::static_pointer_cast<const AbstractCommDriver>(driver2));
  EXPECT_EQ(registry.GetDrivers().size(), 1);

  /* Remove it again, should be ignored. */
  registry.Deactivate(std::static_pointer_cast<const AbstractCommDriver>(driver2));
  EXPECT_EQ(registry.GetDrivers().size(), 1);
}


TEST(Navmsg2000, to_string) {
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  auto msg = std::make_shared<Nmea2000Msg>(id, payload, shared_navaddr_none);
  EXPECT_EQ(string("nmea2000 n2000-1234 1234 7061796c6f61642064617461"),
            msg->to_string());
}


TEST(FileDriver, Registration) {
  auto driver = std::make_shared<FileCommDriver>("test-output.txt");
  driver->Activate();
  auto& registry = CommDriverRegistry::getInstance();
  auto drivers = registry.GetDrivers();
  EXPECT_EQ(registry.GetDrivers().size(), 1);
}


TEST(FileDriver, output) {
  auto driver = std::make_shared<FileCommDriver>("test-output.txt");
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  Nmea2000Msg msg(id, payload, shared_navaddr_none);
  remove("test-output.txt");
  driver->SendMessage(msg, NavAddr());
  std::ifstream f("test-output.txt");
  stringstream ss;
  ss << f.rdbuf();
  EXPECT_EQ(ss.str(),
            string("nmea2000 n2000-1234 1234 7061796c6f61642064617461"));
}


TEST(FileDriver, input) {
  auto driver = std::make_shared<FileCommDriver>("test-output.txt");
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  Nmea2000Msg msg(id, payload, shared_navaddr_none);
  remove("test-output.txt");
  driver->SendMessage(msg, NavAddr());

  SillyListener listener;
  auto indriver = std::make_shared<FileCommDriver>("/tmp/foo.txt",
                                                   "test-output.txt",
                                                   listener);
  indriver->Activate();
  EXPECT_EQ(s_result2, string("nmea2000"));
  EXPECT_EQ(s_result3, string("1234"));
  EXPECT_EQ(s_result, string("payload data"));
}


TEST(Listeners, vector) {
  s_result = "";
  ListenerCliApp app;
  EXPECT_EQ(s_result, string("payload data"));
  EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
};


TEST(Guernsey, play_log) {
  vector<string> log;
  GuernseyApp app(log);
  EXPECT_EQ(log.size(), 14522);
}


TEST(FindDriver, lookup) {
   std::vector<DriverPtr> drivers;
   std::vector<std::string> ifaces {"foo", "bar", "foobar"};
   for (const auto& iface : ifaces) {
     drivers.push_back(std::make_shared<SillyDriver>(SillyDriver(iface)));
   }
   auto found = FindDriver(drivers, "bar");
   EXPECT_EQ(found->iface, string("bar"));
   found = FindDriver(drivers, "baz");
   EXPECT_FALSE(found);
}

TEST(Registry, persistence) {
    if (true) {  // a scope
      auto driver = std::make_shared<const SillyDriver>();
      auto& registry = CommDriverRegistry::getInstance();
      registry.Activate(
        std::static_pointer_cast<const AbstractCommDriver>(driver));
    }
    auto& registry = CommDriverRegistry::getInstance();
    auto drivers = registry.GetDrivers();
    EXPECT_EQ(registry.GetDrivers().size(), 1);
    EXPECT_EQ(registry.GetDrivers()[0]->iface, string("silly"));
    EXPECT_EQ(registry.GetDrivers()[0]->bus, NavAddr::Bus::TestBus);
}
