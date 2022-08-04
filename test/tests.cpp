#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "wx/event.h"
#include "wx/app.h"

#include "comm_appmsg_bus.h"
#include "comm_drv_registry.h"
#include "comm_drv_file.h"

wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
wxDEFINE_EVENT(EVT_BAR, wxCommandEvent);

std::string s_result;
std::string s_result2;
std::string s_result3;

NavAddr::Bus s_bus;
AppMsg::Type s_apptype;

class MsgCliApp : public wxAppConsole {
public:
  class Sink:  public wxEvtHandler {
  private:
    ObservedVarListener listener;
  public:
    Sink() {
      ObservableMsg observable("1234");
      listener = observable.get_listener(this, EVT_BAR);
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
      auto n2k_msg = std::make_shared<const Nmea2000Msg>(id, payload);
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
      auto msg = std::make_unique<Nmea2000Msg>(id, payload);
      NavMsgBus::getInstance().notify(std::move(msg));
    }
  };

  class Sink: public wxEvtHandler {
  public:
    Sink() {
      auto& t = NavMsgBus::getInstance();
      Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234));
      listener = t.get_listener(EVT_FOO, this, n2k_msg.key());

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
      auto msg = std::make_unique<Nmea2000Msg>(id, payload);
      NavMsgBus::getInstance().notify(std::move(msg));
    }
  };

  class Sink: public wxEvtHandler {
  public:
    Sink() {
      auto& t = NavMsgBus::getInstance();
      Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234));
      listeners.push_back(t.get_listener(EVT_FOO, this, n2k_msg.key()));
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
      Position pos(65.2211, 21.4433, Position::Type::NW);
      using namespace std::chrono;
      auto fix = std::make_shared<GnssFix>(pos, 1659345030);
      AppMsgBus::getInstance()->notify(std::move(fix));
    }
  };

  class Sink: public wxEvtHandler {
  public:
    Sink() {
      auto a = AppMsgBus::getInstance();
      AppMsg msg(AppMsg::Type::GnssFix);
      listener = a->get_listener(EVT_FOO, this, msg.key());

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


class GuernseyApp: public wxAppConsole {
public:
  GuernseyApp(vector<string>* log) : wxAppConsole() {
    auto& msgbus = NavMsgBus::getInstance();
    auto driver =
        make_shared<FileCommDriver>("/tmp/output.txt",
                                    "Guernesey-1659560590623.input.txt",
                                    msgbus);
    Nmea0183Msg msg("GPGLL");
    auto listener = msgbus.get_listener(EVT_FOO, this, msg.key());
    Bind(EVT_FOO, [log](wxCommandEvent ev) { 
      auto message = get_navmsg_ptr(ev);
      auto n0183_msg = dynamic_pointer_cast<const Nmea0183Msg>(message);
      log->push_back(n0183_msg->to_string());
    });
    driver->Activate();
    ProcessPendingEvents();
  }
};



class SillyDriver: public AbstractCommDriver {
public:
  SillyDriver() : AbstractCommDriver(NavAddr::Bus::TestBus, "silly") {}
  SillyDriver(const string& s) : AbstractCommDriver(NavAddr::Bus::TestBus, s) {}

  virtual void SendMessage(const NavMsg& msg, const NavAddr& addr) {}

  virtual void SetListener(DriverListener& listener) {}

  virtual void Activate() {};
};


class SillyListener: public DriverListener {
public:
  /** Handle a received message. */
  virtual void notify(std::unique_ptr<const NavMsg> message)  {
    s_result2 = NavAddr::BusToString(message->bus);

    auto base_ptr = message.get();
    auto n2k_msg = dynamic_cast<const Nmea2000Msg*>(base_ptr);
    s_result3 = n2k_msg->id.to_string();

    stringstream ss;
    std::for_each(n2k_msg->payload.begin(), n2k_msg->payload.end(),
                  [&ss](unsigned char c) {ss << static_cast<char>(c); });
    s_result = ss.str();
  }

  /** Handle driver status change. */
  virtual void notify(const AbstractCommDriver& driver) {}
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

TEST(Messaging, AppMsg) {
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  AppmsgCliApp app;
  EXPECT_EQ(s_result, string("65°22,11N 21°44,33W"));
  EXPECT_EQ(s_apptype, AppMsg::Type::GnssFix);
};

TEST(Drivers, Registry) {
  auto driver = std::make_shared<const SillyDriver>();
  auto registry = CommDriverRegistry::getInstance();
  registry->Activate(std::static_pointer_cast<const AbstractCommDriver>(driver));
  auto drivers = registry->get_drivers();
  EXPECT_EQ(registry->get_drivers().size(), 1);
  EXPECT_EQ(registry->get_drivers()[0]->iface, string("silly"));
  EXPECT_EQ(registry->get_drivers()[0]->bus, NavAddr::Bus::TestBus);

  /* Add it again, should be ignored. */
  registry->Activate(std::static_pointer_cast<const AbstractCommDriver>(driver));
  EXPECT_EQ(registry->get_drivers().size(), 1);

  /* Add another one, should be accepted */
  auto driver2 = std::make_shared<const SillyDriver>("orvar");
  registry->Activate(std::static_pointer_cast<const AbstractCommDriver>(driver2));
  EXPECT_EQ(registry->get_drivers().size(), 2);

  /* Remove one, leaving one in place. */
  registry->Deactivate(std::static_pointer_cast<const AbstractCommDriver>(driver2));
  EXPECT_EQ(registry->get_drivers().size(), 1);

  /* Remove it again, should be ignored. */
  registry->Deactivate(std::static_pointer_cast<const AbstractCommDriver>(driver2));
  EXPECT_EQ(registry->get_drivers().size(), 1);
}

TEST(Navmsg2000, to_string) {
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  auto msg = std::make_shared<Nmea2000Msg>(id, payload);
  EXPECT_EQ(string("nmea2000 n2000-1234 1234 7061796c6f61642064617461"),
            msg->to_string());
}

TEST(FileDriver, Registration) {
  auto driver = std::make_shared<FileCommDriver>("/tmp/output.txt");
  driver->Activate();
  auto registry = CommDriverRegistry::getInstance();
  auto drivers = registry->get_drivers();
  EXPECT_EQ(registry->get_drivers().size(), 1);
}

TEST(FileDriver, output) {
  auto driver = std::make_shared<FileCommDriver>("/tmp/output.txt");
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  Nmea2000Msg msg(id, payload);
  remove("/tmp/output.txt");
  driver->SendMessage(msg, NavAddr());
  std::ifstream f("/tmp/output.txt");
  stringstream ss;
  ss << f.rdbuf();
  EXPECT_EQ(ss.str(),
            string("nmea2000 n2000-1234 1234 7061796c6f61642064617461"));
}


TEST(FileDriver, input) {

  auto driver = std::make_shared<FileCommDriver>("/tmp/output.txt");
  std::string s("payload data");
  auto payload = std::vector<unsigned char>(s.begin(), s.end());
  auto id = static_cast<uint64_t>(1234);
  Nmea2000Msg msg(id, payload);
  remove("/tmp/output.txt");
  driver->SendMessage(msg, NavAddr());

  SillyListener listener;
  auto indriver = std::make_shared<FileCommDriver>("/tmp/foo.txt",
                                                   "/tmp/output.txt", 
                                                   listener);
  //indriver->SetListener(listener);
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
  GuernseyApp app(&log);
  EXPECT_EQ(log.size(), 14522);
}
