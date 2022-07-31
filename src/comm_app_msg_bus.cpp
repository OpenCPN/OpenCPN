#include "comm_app_msg.h"

#include "observable_appmsg.h"

using namespace std;

ObservedVarListener AppMsgBus::get_listener(wxEventType et, wxEvtHandler* eh,
                                            const string& key) {
  ObservableAppMsg oam(key);
  return oam.get_listener(eh, et);
}

void AppMsgBus::notify(std::shared_ptr<const AppMsg> msg) {
  ObservableAppMsg om(msg->key());
  om.notify(msg);
}
AppMsgBus* AppMsgBus::getInstance() {
  static AppMsgBus* instance = 0;

  if (!instance) instance = new AppMsgBus();
  return instance;
}
