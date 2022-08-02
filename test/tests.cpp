#include <gtest/gtest.h>

#include "wx/event.h"
#include "wx/app.h"

#include "comm_appmsg_bus.h"
#include "comm_drv_registry.h"

wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
wxDEFINE_EVENT(EVT_BAR, wxCommandEvent);

std::string s_result;
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
      auto msg = std::make_shared<Nmea2000Msg>(id, payload);
      NavMsgBus::getInstance()->notify(msg);
    }
  };

  class Sink: public wxEvtHandler {
  public:
    Sink() {
      auto t = NavMsgBus::getInstance();
      Nmea2000Msg n2k_msg(static_cast<uint64_t>(1234));
      listener = t->get_listener(EVT_FOO, this, n2k_msg.key());

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

class SillyDriver: public AbstractCommDriver {
public:
  SillyDriver() : AbstractCommDriver(NavAddr::Bus::TestBus, "silly") {}
  SillyDriver(const string& s) : AbstractCommDriver(NavAddr::Bus::TestBus, s) {}

  virtual void send_message(const NavMsg& msg, const NavAddr& addr) {}

  virtual void set_listener(std::shared_ptr<const DriverListener> listener) {}
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

TEST(Drivers, registry1) {
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
