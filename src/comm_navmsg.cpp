#include <algorithm>
#include <string>
#include <iomanip>

#include "comm_driver.h"

std::string NavAddr::BusToString(NavAddr::Bus b) {
  switch (b) {
    case NavAddr::Bus::N0183: return "nmea0183"; break;
    case NavAddr::Bus::N2000: return "nmea2000"; break;
    case NavAddr::Bus::Signalk: return "SignalK"; break;
    case NavAddr::Bus::Onenet: return "Onenet"; break;
    case NavAddr::Bus::TestBus: return "TestBus"; break;
    case NavAddr::Bus::Undef: return "??"; break;
  }
  return "????";
}

static std::string CharToString(unsigned char c) {
  using namespace std;
  stringstream ss;
  ss << setfill('0') << hex << setw(2) << (c & 0x00ff);
  return ss.str();
}

std::string Nmea2000Msg::to_string() const {
  std::string s;
  std::for_each(payload.begin(), payload.end(),  
                [&s](unsigned char c) { s.append(CharToString(c)); });
  return NavMsg::to_string() + " " + id.to_string() + " " + s;
}


