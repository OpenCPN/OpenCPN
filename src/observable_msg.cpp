#include <sstream>

#include "observable_msg.h"
#include "comm_pointer_msg.h"


std::shared_ptr<NavMsg> get_navmsg_ptr(wxCommandEvent ev) {
  return PointerMsg<NavMsg>::get_pointer(ev);
}

/* ObservableMsg implementation */

void ObservableMsg::notify(NavMsg* msg_ptr) {
  auto raw_shared_ptr = new std::shared_ptr<NavMsg>(msg_ptr);
  auto boxed_msg = new PointerMsg<NavMsg>(*raw_shared_ptr);
  ObservedVar::notify("", boxed_msg);
}

void ObservableMsg::notify(const Nmea2000Msg& msg) {
  notify(new Nmea2000Msg(msg.id, msg.payload));
}

void ObservableMsg::notify(const Nmea0183Msg& msg) {
  notify(new Nmea0183Msg(msg.id, msg.payload));
}

void ObservableMsg::notify(const SignalkMsg& msg) {
  notify(new SignalkMsg(0));
}
