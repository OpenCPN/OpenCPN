/* API handling raw partially decoded messages. */

#include <memory>
#include <vector>

#include <wx/event.h>
#include <wx/jsonreader.h>

#include "datastream.h"

#include "commdriverBase.h"

#ifndef _TRANSPORT_H
#define _TRANSPORT_H

/** The transport layer, a singleton. */
class Transport : public DriverListener {
public:
  void send_message(const NavMsg& message, const NavAddr& address);

  /**
   * Return a listening object which generates wxEventType events sent to
   * wxEvtHandler when a message with given key is received. The events
   * contains a shared_ptr<NavMsg>, use get_message_ptr(event) to retrieve it.
   */
  ObservedVarListener get_listener(wxEventType et, wxEvtHandler* eh,
                                   const std::string& key);

  /* DriverListener implementation: */
  void notify(const NavMsg& message);
  void notify(const AbstractCommDriver& driver);

  /* Singleton implementation. */
  static Transport* getInstance();
  Transport& operator=(Transport&) = delete;
  Transport(const Transport&) = delete;

private:
  Transport() = default;
};

#endif  // TRANSPORT_H
