/** Api handling decoded, binary messages. */

#ifndef APP_MSG_H
#define APP_MSG_H

#include <memory>

#include <wx/event.h>

#include "transport.h"

enum class AppMsgType {gps_fix, ais_data, data_prio_needed /*, ...*/};

typedef struct {
  double lat;
  double lon;
  bool N;   // N/S
  bool E;   // E/W
  // All sorts of ctors and other usable functions
} position_t;

typedef struct {
  enum class Quality {none, gps, differential };
  time_t time;
  position_t pos;
  Quality quality;
  int satellites_used;
} gps_fix_t;

typedef struct {
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
} ais_data_t;


/**
 * Issued when there are multiple sources providing 'what' with priority == 0.
 * Should result in GUI actions eventually calling set_priority()
 */
typedef struct {
  AppMsgType what;
  std::vector<nav_addr_t> sources;
} data_prio_needed_t;

static std::string type_to_string(AppMsgType);

typedef struct {
  const AppMsgType type;
  const std::string name;  // Must be unique, probably using type_to_string().
  nav_addr_t source;
  unsigned short prio;     // Initially 0, modified using set_priority().
  union {
    gps_fix_t gps_fix;
    ais_data_t ais_data;
    data_prio_needed_t data_prio_needed;
    // ...
  };
} ocpn_msg_t;

typedef const std::shared_ptr<ocpn_msg_t> ocpn_msg_ptr;


/** Application layer messaging, a singleton. */
class AppMessages {
public:

  /** Send message to everyone listening to it. */
  void notify(const ocpn_msg_t& message);
  
  /**
   * Send given event message to handler when receiving NavMsgType.
   * Message contains a nav_msg_ptr.
   */
  void listen(wxEventType, wxEvtHandler, AppMsgType);

  /**
   * Set the priority for a given data source providing data.
   * Higher priorities are preferred.
   */
  void set_priority(AppMsgType data, const nav_addr_t& src, unsigned prio);

  static AppMessages* getInstance();
};

#endif  // APP_MSG_H
