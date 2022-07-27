#include <sstream>

#include "observable_msg.h"

/**
 * Container  which carries a shared_ptr through wxWidgets event handling
 * which only allows simple  objects which can be represented by a void*.
 */
class NavPointerMsg {
public:
  /** Create  an instance containing p. */
  NavPointerMsg(std::shared_ptr<NavMsg> p) : ptr(p){};

  /** Retrieve the pointer contained in an instance and delete instance. */
  static std::shared_ptr<NavMsg> get_pointer(wxCommandEvent ev) {
    auto npm = static_cast<NavPointerMsg*>(ev.GetClientData());
    auto tmp = std::move(npm->ptr);
    delete npm;
    return tmp;
  };

private:
  NavPointerMsg() = delete;
  std::shared_ptr<NavMsg> operator=(std::shared_ptr<NavMsg>&) = delete;
  std::shared_ptr<NavMsg> ptr;
};

std::shared_ptr<NavMsg> get_message_ptr(wxCommandEvent ev) {
  return NavPointerMsg::get_pointer(ev);
}

/* ObservableMsg implementation */

void ObservableMsg::notify(NavMsg* msg_ptr) {
  auto raw_shared_ptr = new std::shared_ptr<NavMsg>(msg_ptr);
  auto boxed_msg = new NavPointerMsg(*raw_shared_ptr);
  ObservedVar::notify("", boxed_msg);
}

void ObservableMsg::notify(const Nmea2000Msg& msg) {
  notify(new Nmea2000Msg(msg.id, msg.payload));
}

void ObservableMsg::notify(const Nmea0183Msg& msg) {
  notify(new Nmea0183Msg(msg.id, msg.payload));
}

void ObservableMsg::notify(const SignalK_Msg& msg) {
  notify(new SignalK_Msg(0));
}
