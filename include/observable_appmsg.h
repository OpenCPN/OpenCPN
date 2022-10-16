/* API handling decoded application messages. */

#ifndef _OBSERVABLE_APPMSG_H
#define _OBSERVABLE_APPMSG_H

#include <memory>
#include <string>

#include <wx/event.h>

#include "comm_appmsg.h"


class ObservableAppMsg : public Observable {
public:
  ObservableAppMsg(const std::string key) : Observable(key) {};

  /* Send message to all listeners. */
  void Notify(std::shared_ptr<const AppMsg> msg) {
    Observable::Notify(std::dynamic_pointer_cast<const void>(msg));
  }
};


#endif  // OBSERVABLE_MSG_H
