#include "observable_appmsg.h"
#include "comm_appmsg.h"

/* ObservableMsg implementation */

void ObservableAppMsg::notify(std::shared_ptr<const AppMsg> msg) {
  ObservedVar::notify(std::dynamic_pointer_cast<const void>(msg));
};
