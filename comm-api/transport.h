/* API handling raw partially decoded messages. */

#include <memory>
#include <vector>

#include <wx/event.h>
#include <wx/jsonreader.h>

#include "datastream.h"

#include "drivers.h"

#ifndef _TRANSPORT_H
#define _TRANSPORT_H


/** Generate a unique string used for listen()/notify(). */
class msg_key {
public:
  virtual std::string to_string() = 0;
};

class nmea2000_key: public msg_key {
public:
   nmea2000_key(uint64_t name);
   std::string to_string();
};

class nmea0183_key: public msg_key {
public:
   nmea0183_key(const std::string&);
   std::string to_string();
};
   
class signalK_key: public msg_key {
public:
   /**
    * Generating an unique string from json is non-trivial.
    * Escape hatch: return a constant,
    * meaning listening to signalK will return all sK messages.
    */
   signalK_key(const std::string&);
   std::string to_string();
};


/** The transport layer, a singleton. */
class Transport: public DriverListener {
public:

  void send_message(const nav_msg& message, const nav_addr_t& address);
  
  /**
   * Send given eventType message to handler when receiving a mmea0183 message
   * Message contains the payload as a wxString.
   */
  void listen(wxEventType, wxEvtHandler, const nmea0183_key&);
  
  /**
   * Send given eventType message to handler when receiving a n2k message.
   * Message contains the payload as a raw_msg_ptr;
   */
  void listen(wxEventType, wxEvtHandler, const nmea2000_key&);
  
  /**
   * Send given eventType message to handler when receiving a signalK message.
   * Message contains the parsed message tree root as a wxJSONValue*
   * NOTE: Current implementation will return all signalK messages, effectively 
   * disregarding the key.
   */
  void listen(wxEventType, wxEvtHandler, const signalK_key&);


  /*DriverListener implementation: */
  void notify(const nav_msg& message);
  void notify(const AbstractDriver& driver);

  /* Singleton implementation. */
  static Transport* getInstance();
};

#endif  // TRANSPORT_H
