#include "observable_appmsg.h"
#include "comm_pointer_msg.h"
#include "comm_app_msg.h"


std::shared_ptr<AppMsg> get_appmsg_ptr(wxCommandEvent ev) {
  return PointerMsg<AppMsg>::get_pointer(ev);
}

/* ObservableMsg implementation */

void ObservableAppMsg::notify(const AppMsg& msg) {
  auto raw_ptr = new AppMsg(msg);
  auto shared_ptr = new std::shared_ptr<AppMsg>(raw_ptr);
  auto boxed_msg = new PointerMsg<AppMsg>(*shared_ptr);
  ObservedVar::notify("", boxed_msg);
}
