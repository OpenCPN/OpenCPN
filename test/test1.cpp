#include <gtest/gtest.h>

#include "wx/event.h"
#include "wx/app.h"

#include "comm_appmsg_bus.h"

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


TEST(Messaging, ObservableMsg) {
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  MsgCliApp app;
  EXPECT_STREQ(s_result.c_str(), "payload data");
  EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
};

TEST(Messaging, NavMsg) {
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  TransportCliApp app;
  EXPECT_STREQ(s_result.c_str(), "payload data");
  EXPECT_EQ(NavAddr::Bus::N2000, s_bus);
};

TEST(Messaging, ApppMsg) {
  s_result = "";
  s_bus = NavAddr::Bus::Undef;
  AppmsgCliApp app;
  EXPECT_STREQ(s_result.c_str(), "65°22,11N 21°44,33W");
  EXPECT_EQ(s_apptype, AppMsg::Type::GnssFix);
};
