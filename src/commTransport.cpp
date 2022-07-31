#include "commTransport.h"

#include "observable_msg.h"

using namespace std;

ObservedVarListener NavMsgBus::get_listener(wxEventType et, wxEvtHandler* eh,
                                            const string& key) {
  ObservableMsg om(key);
  return om.get_listener(eh, et);
}

void NavMsgBus::notify(std::shared_ptr<const NavMsg> msg) {
  ObservableMsg om(msg->key());
  om.notify(msg);
}

NavMsgBus* NavMsgBus::getInstance() {
  static NavMsgBus* instance = 0;

  if (!instance) instance = new NavMsgBus();
  return instance;
}

/** Handle changes in driver list. */
void NavMsgBus::notify(AbstractCommDriver const&) {}
