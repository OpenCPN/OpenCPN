/* API handling decoded application messages. */

#ifndef _OBSERVABLE_APPMSG_H
#define _OBSERVABLE_APPMSG_H

#include <memory>
#include <string>

#include <wx/event.h>

#include "comm_appmsg.h"


class ObservableAppMsg : public ObservedVar {
public:
  ObservableAppMsg(const std::string key) : ObservedVar(key) {};

  /* Send message to all listeners. */
  void notify(std::shared_ptr<const AppMsg> msg) {
    ObservedVar::notify(std::dynamic_pointer_cast<const void>(msg));
  }
};


#endif  // OBSERVABLE_MSG_H
