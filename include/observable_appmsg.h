/* API handling decoded application messages. */

#ifndef _OBSERVABLE_APPMSG_H
#define _OBSERVABLE_APPMSG_H

#include <memory>
#include <string>

#include <wx/event.h>
#include "comm_appmsg.h"


/** Return the pointer available in wxCommandEvent.GetClientData() */
std::shared_ptr<const AppMsg> get_appmsg_ptr(wxCommandEvent ev);

class ObservableAppMsg : public ObservedVar {
public:
  ObservableAppMsg(const std::string key) : ObservedVar(key) {};

  /* Send message to all listeners. */
  void notify(std::shared_ptr<const AppMsg> msg);

};


#endif  // OBSERVABLE_MSG_H
