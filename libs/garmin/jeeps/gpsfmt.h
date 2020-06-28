#ifndef gpsfmt_h
#define gpsfmt_h


#include "gps.h"
#include <cstdio>
#include <ctime>

  void   GPS_Fmt_Print_Time(time_t Time, FILE* outf);
  void   GPS_Fmt_Print_Position(double lat, double lon, FILE* outf);
  void   GPS_Fmt_Print_Pvt(GPS_PPvt_Data pvt, FILE* outf);
  void   GPS_Fmt_Print_Almanac(GPS_PAlmanac* alm, int32 n, FILE* outf);
  void   GPS_Fmt_Print_Track(GPS_PTrack* trk, int32 n, FILE* outf);
  int32  GPS_Fmt_Print_Waypoint(GPS_PWay* way, int32 n, FILE* outf);
  int32  GPS_Fmt_Print_Proximity(GPS_PWay* way, int32 n, FILE* outf);
  int32  GPS_Fmt_Print_Route(GPS_PWay* way, int32 n, FILE* outf);

#endif
