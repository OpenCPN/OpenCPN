#include "commTransport.h"

#include "observable_appmsg.h"

using namespace std;

ObservedVarListener AppMsgBus::get_listener(wxEventType et, wxEvtHandler* eh,
                                            const string& key) {
  ObservableAppMsg oam(key);
  return oam.get_listener(eh, et);
}

void AppMsgBus::notify(const AppMsg& message) {
  ObservableAppMsg om(string("appmsg-") + message.name);
  om.notify(message);
}

AppMsgBus* AppMsgBus::getInstance() {
  static AppMsgBus* instance = 0;

  if (!instance) instance = new AppMsgBus();
  return instance;
}
