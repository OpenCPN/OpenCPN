/* API handling raw partially decoded messages. */

#include <memory>

#include <wx/event.h>
#include "commTransport.h"

#ifndef _OBSERVABLE_MSG_H
#define _OBSERVABLE_MSG_H

/** Return the message pointer available in wxCommandEvent.GetClientData() */
std::shared_ptr<NavMsg> get_navmsg_ptr(wxCommandEvent ev);

class ObservableMsg : public ObservedVar {
public:
  ObservableMsg(const std::string key) : ObservedVar(key){};

  /* Send message to all listeners. */
  void notify(std::shared_ptr<const NavMsg> msg);

protected:
  void notify(NavMsg* msg);
};

#endif  // OBSERVABLE_MSG_H
