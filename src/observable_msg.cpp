#include "observable_msg.h"
#include "comm_pointer_msg.h"


std::shared_ptr<NavMsg> get_navmsg_ptr(wxCommandEvent ev) {
  return PointerMsg<NavMsg>::get_pointer(ev);
}

/* ObservableMsg implementation */

void ObservableMsg::notify(std::shared_ptr<const NavMsg> msg) {
  auto boxed_msg = new PointerMsg<const NavMsg>(msg);
  ObservedVar::notify("", boxed_msg);
}
