#include "observable_msg.h"
#include "comm_pointer_msg.h"


std::shared_ptr<const NavMsg> get_navmsg_ptr(wxCommandEvent ev) {
  return PointerMsg<NavMsg>::get_pointer(ev);
}

/* ObservableMsg implementation */

void ObservableMsg::notify(std::shared_ptr<const NavMsg> msg) {
  auto voidptr = std::dynamic_pointer_cast<const void>(msg);
  ObservedVar::notify(voidptr, "", 0, 0);
}
