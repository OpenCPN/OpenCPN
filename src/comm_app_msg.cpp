#include "comm_app_msg.h"

double PosPartsToDegrees(float degrees, float minutes, float percent_of_minute) {
  return degrees + minutes / 60 + percent_of_minute / 6000;
}
