/** Api handling decoded, binary messages. */

#ifndef _APP_MSG_H
#define _APP_MSG_H

#include <memory>
#include <iomanip>

#include <wx/event.h>

#include "comm_driver.h"

enum class AppMsgType {gnss_fix, ais_data, data_prio_needed, undef /*, ...*/};

double PosPartsToDegrees(float degrees, float minutes, float percent_of_minute);

struct Position{
  enum class Type {NE, NW, SE, SW, Undef};
  Type type;
  double lat;
  double lon;

  std::string to_string() const {
    std::stringstream buf;
    buf << std::setw(2) << lat << " " << lon << TypeToStr(type);
    return buf.str();
  }

  Position(double _lat, double _lon, Type t = Type::Undef)
    : type(t), lat(_lat), lon(_lon) {}
  Position() : type(Type::Undef), lat(0), lon(0) {};

  std::string TypeToStr(const Type t) const {
    switch (t) {
      case Type::NE:  return "NE"; break;
      case Type::NW:  return "NW"; break;
      case Type::SE:  return "SE"; break;
      case Type::SW:  return "SW"; break;
      default: return "??"; break;
    }
  }
};


class AppMsg {
public:
  const AppMsgType type;
  const std::string name;  // Must be unique, probably using TypeToString().
  NavAddr source;
  unsigned short prio;     // Initially 0, modified using set_priority

  virtual std::string key() const { return std::string("@!appmsg-") + name; }

  std::string TypeToString(const AppMsgType t) const {
    switch (t) {
        case AppMsgType::gnss_fix: return "gnss-fix"; break;
        case AppMsgType::ais_data: return "ais-data"; break;
        case AppMsgType::data_prio_needed: return "data-prio-needed"; break;
        case AppMsgType::undef: return "??"; break;
        default: return "????";
    }
  }
    
  AppMsg(AppMsgType t) 
    : type(t), name(TypeToString(t)), source(NavAddr()), prio(0) {};

protected:
  AppMsg(AppMsgType tp, const std::string& nm, NavAddr src)
    : type(tp), name(nm), source(src), prio(0) {};

  AppMsg& operator=(const AppMsg&) = default;
};


/**
 * Issued when there are multiple sources providing 'what' with priority == 0.
 * Should result in GUI actions eventually calling set_priority()
 */
class DataPrioNeeded: public AppMsg {
public:
  AppMsgType what;
  std::vector<NavAddr> sources;
};


class GnssFix: public AppMsg {
public:
  enum class Quality {none, gnss, differential };
  const Position pos;
  const time_t time;
  Quality quality;
  int satellites_used;
  GnssFix(Position p, time_t t, Quality q = Quality::none, int s_used = -1) 
    : AppMsg(AppMsgType::gnss_fix, "gnss-fix", NavAddr()),
    pos(p), time(t), quality(q), satellites_used(s_used) {};

  std::string to_string() const {
    std::stringstream buf;
    buf << time;
    return pos.to_string() + " " + buf.str();
  }
};


class AisData: public AppMsg {
public:
  time_t time;
  Position pos;
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
  void notify(std::shared_ptr<const AppMsg> msg);
  
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
