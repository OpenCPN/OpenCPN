#ifndef gpsmem_h
#define gpsmem_h


#include "gps.h"
  GPS_PPvt_Data     GPS_Pvt_New();
  void              GPS_Pvt_Del(GPS_PPvt_Data* thys);
  GPS_PAlmanac      GPS_Almanac_New();
  void              GPS_Almanac_Del(GPS_PAlmanac* thys);
  GPS_PTrack        GPS_Track_New();
  void              GPS_Track_Del(GPS_PTrack* thys);
  GPS_PWay          GPS_Way_New();
  void              GPS_Way_Del(GPS_PWay* thys);
  GPS_PLap          GPS_Lap_New();
  void              GPS_Lap_Del(GPS_PLap* thys);
  GPS_PCourse       GPS_Course_New();
  void              GPS_Course_Del(GPS_PCourse* thys);
  GPS_PCourse_Lap   GPS_Course_Lap_New();
  void              GPS_Course_Lap_Del(GPS_PCourse_Lap* thys);
  GPS_PCourse_Point GPS_Course_Point_New();
  void              GPS_Course_Point_Del(GPS_PCourse_Point* thys);

#endif
