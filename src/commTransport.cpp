#include "commTransport.h"

#include "observable_msg.h"

using namespace std;

ObservedVarListener Transport::get_listener(wxEventType et, wxEvtHandler* eh,
                                            const string& key) {
  ObservableMsg om(key);
  return om.get_listener(eh, et);
}

void Transport::notify(std::shared_ptr<const NavMsg> msg) {
  ObservableMsg om(msg->key());
  om.notify(msg);
}

Transport* Transport::getInstance() {
  static Transport* instance = 0;

  if (!instance) instance = new Transport();
  return instance;
}

/** Handle changes in driver list. */
void Transport::notify(AbstractCommDriver const&) {}
