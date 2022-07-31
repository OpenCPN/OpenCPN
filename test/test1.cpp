#include <gtest/gtest.h>

#include "wx/event.h"
#include "wx/app.h"

#include "comm_navmsg_bus.h"

wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
wxDEFINE_EVENT(EVT_BAR, wxCommandEvent);

std::string s_result;

class MsgSink:  public wxEvtHandler {
private:
  ObservedVarListener listener;
public:
  MsgSink() {
    ObservableMsg observable("1234");
    listener = observable.get_listener(this, EVT_BAR);
    Bind(EVT_BAR, [&](wxCommandEvent ev) {
      auto msg = get_navmsg_ptr(ev);
      auto n2000_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
      std::string s(n2000_msg->payload.begin(), n2000_msg->payload.end());
      s_result = s;
    });
  }
};


class MsgSource {
public:
  MsgSource(wxEvtHandler& sink) {
    std::string s("payload data");
    auto payload = std::vector<unsigned char>(s.begin(), s.end());
    auto id = static_cast<uint64_t>(1234);
    auto n2k_msg = std::make_shared<const Nmea2000Msg>(id, payload);
    ObservableMsg observable("1234");
    observable.notify(n2k_msg);
  }
};

class CliApp1 : public wxAppConsole {
public:
  CliApp1() : wxAppConsole() {
    CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");
    SetAppName("opencpn");
    MsgSink sink;
    MsgSource source(sink);
    ProcessPendingEvents();
  }
};


TEST(HelloTest, ObservableMsg) {
  CliApp1 app;
  EXPECT_STREQ(s_result.c_str(), "payload data");
}
 
