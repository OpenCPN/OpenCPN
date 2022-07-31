#include "observable_appmsg.h"
#include "comm_pointer_msg.h"
#include "comm_app_msg.h"


std::shared_ptr<AppMsg> get_appmsg_ptr(wxCommandEvent ev) {
  return PointerMsg<AppMsg>::get_pointer(ev);
}

/* ObservableMsg implementation */

void ObservableAppMsg::notify(std::shared_ptr<AppMsg> msg) {
  auto boxed_msg = new PointerMsg<AppMsg>(msg);
  ObservedVar::notify("", boxed_msg);
};
