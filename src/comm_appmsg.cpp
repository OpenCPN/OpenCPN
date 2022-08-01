#include <sstream>
#include <iomanip>

#include "comm_appmsg.h"

double PosPartsToDegrees(float degrees, float minutes, float percent_of_minute) {
  return degrees + minutes / 60 + percent_of_minute / 6000;
}

std::string TimeToString(const time_t t) {
  char buff[30];
#ifdef _MSC_VER
  errno_t e = ctime_s(buf, sizeof(buf), cur_time);
  assert(e == 0 && "Huh? ctime_s returned an error");
  return std::string(buffer);
#else
  const char* r = ctime_r(&t, buff);
  assert(r != NULL && "ctime_r failed...");
  return std::string(buff);
#endif
}

std::string DegreesToString(double degrees) {
  using namespace std;
  std::stringstream buf;
  buf << setw(2) <<  static_cast<int>(trunc(degrees)) << "\u00B0"
      << static_cast<int>(trunc(degrees * 100)) % 100 << ","
      << setw(2) << (static_cast<int>(trunc(degrees * 10000))% 10000) % 100;
  return buf.str();
}


std::string AppMsg::TypeToString(const AppMsg::Type t) const {
  switch (t) {
    case AppMsg::Type::GnssFix: return "gnss-fix"; break;
    case AppMsg::Type::AisData: return "ais-data"; break;
    case AppMsg::Type::DataPrioNeeded: return "data-prio-needed"; break;
    case AppMsg::Type::Undef: return "??"; break;
    default: return "????";
  }
}
