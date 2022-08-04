#include "comm_navmsg_bus.h"

#include "observable_msg.h"

using namespace std;

ObservedVarListener NavMsgBus::get_listener(wxEventType et, wxEvtHandler* eh,
                                            const string& key) {
  ObservableMsg om(key);
  return om.get_listener(eh, et);
}

void NavMsgBus::notify(std::unique_ptr<const NavMsg> msg) {
  std::shared_ptr<const NavMsg> shared_msg = std::move(msg);
  ObservableMsg om(shared_msg->key());
  om.notify(shared_msg);
}

NavMsgBus& NavMsgBus::getInstance() {
  static NavMsgBus instance;
  return instance;
}

/** Handle changes in driver list. */
void NavMsgBus::notify(AbstractCommDriver const&) {}
