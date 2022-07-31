#include "observable_appmsg.h"
#include "comm_pointer_msg.h"
#include "comm_app_msg.h"


std::shared_ptr<const AppMsg> get_appmsg_ptr(wxCommandEvent ev) {
  return PointerMsg<const AppMsg>::get_pointer(ev);
}

/* ObservableMsg implementation */

void ObservableAppMsg::notify(std::shared_ptr<const AppMsg> msg) {
  auto boxed_msg = new PointerMsg<const AppMsg>(msg);
  ObservedVar::notify("", boxed_msg);
};
