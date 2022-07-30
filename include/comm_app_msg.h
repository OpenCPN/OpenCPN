/** Api handling decoded, binary messages. */

#ifndef APP_MSG_H
#define APP_MSG_H

#include <memory>

#include <wx/event.h>

#include "commdriverBase.h"

enum class AppMsgType {gnss_fix, ais_data, data_prio_needed /*, ...*/};

typedef struct {
  double lat;
  double lon;
  bool N;   // N/S
  bool E;   // E/W
  // All sorts of ctors and other usable functions
} position_t;

/**
 * Issued when there are multiple sources providing 'what' with priority == 0.
 * Should result in GUI actions eventually calling set_priority()
 */
static std::string type_to_string(AppMsgType);


class AppMsg {
public:
  const AppMsgType type;
  const std::string name;  // Must be unique, probably using type_to_string().
  NavAddr source;
  unsigned short prio;     // Initially 0, modified using set_priority
protected:
  AppMsg(AppMsgType tp, const std::string& nm, NavAddr src)
    : type(tp), name(nm), source(src), prio(0) {};
};

class DataPrioNeeded: public AppMsg {
public:
  AppMsgType what;
  std::vector<NavAddr> sources;
};

class GnssFix: public AppMsg {
  enum class Quality {none, gnss, differential };
  time_t time;
  position_t pos;
  Quality quality;
  int satellites_used;
};


class AisData: public AppMsg {
public:
  time_t time;
  position_t pos;
  float sog;             // Speed over ground, knots.
  float cog;             // Course over ground, 0..360 degrees.
  float heading;         // Magnetic sensor, 0..360 degrees.
  float rate_of_turn;    // Degrees per minute, "-" means bow turns to port.
  uint8_t type;          // https://api.vtexplorer.com/docs/ref-aistypes.html
  std::string name;
  std::string callsign;
  std::string dest;      // Destination port
  int length;
  int beam;
  int draft;
  uint8_t status;        // https://api.vtexplorer.com/docs/ref-navstat.html
};


/** Application layer messaging, a singleton. */
class AppMsgBus {
public:

  /** Send message to everyone listening to it. */
  void notify(const AppMsg& message);
  
  /**
   * Return a listening object which generates wxEventType events sent to
   * wxEvtHandler when a message with given key is received. The events
   * contains a shared_ptr<NavMsg>, use get_navmsg_ptr(event) to retrieve it.
   */
  ObservedVarListener get_listener(wxEventType et, wxEvtHandler* eh,
                                   const std::string& key);

  /**
   * Set the priority for a given data source providing data.
   * Higher priorities are preferred.
   */
  void set_priority(AppMsgType data, const NavAddr& src, unsigned prio);

  static AppMsgBus* getInstance();
  void notify(const AbstractCommDriver&) {}
};

#endif  // APP_MSG_H
