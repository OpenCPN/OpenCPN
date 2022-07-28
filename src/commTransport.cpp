#include "commTransport.h"

#include "observable_msg.h"

using namespace std;

ObservedVarListener Transport::get_listener(wxEventType et, wxEvtHandler* eh,
                                            const string& key) {
  ObservableMsg om(key);
  return om.get_listener(eh, et);
}

void Transport::notify(const NavMsg& message) {
  ObservableMsg om(message.key());
  switch (message.bus) {
    case NavBus::n2k:
      om.notify(*dynamic_cast<const Nmea2000Msg*>(&message));
      break;
    case NavBus::nmea0183:
      om.notify(*dynamic_cast<const Nmea0183Msg*>(&message));
      break;
    case NavBus::signalK:
      om.notify(*dynamic_cast<const SignalK_Msg*>(&message));
      break;
    default:
      assert(false);
      break;
  }
}

Transport* Transport::getInstance() {
  static Transport* instance = 0;

  if (!instance) instance = new Transport();
  return instance;
}

/** Handle changes in driver list. */
void Transport::notify(AbstractCommDriver const&) {}
