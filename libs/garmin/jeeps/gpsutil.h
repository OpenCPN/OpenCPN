#ifndef gpsutil_h
#define gpsutil_h


#include "gps.h"

  int32  GPS_Util_Little();

  US     GPS_Util_Get_Short(const UC* s);
  void   GPS_Util_Put_Short(UC* s, US v);
  int32  GPS_Util_Get_Int(const UC* s);
  void   GPS_Util_Put_Int(UC* s, int32 v);
  double GPS_Util_Get_Double(const UC* s);
  void   GPS_Util_Put_Double(UC* s, double v);
  float  GPS_Util_Get_Float(const UC* s);
  void   GPS_Util_Put_Float(UC* s, float v);
  void   GPS_Util_Canon(int32 state);
  int32  GPS_Util_Block(int32 fd, int32 state);
  void   GPS_Util_Put_Uint(UC* s, uint32 v);
  uint32 GPS_Util_Get_Uint(const UC* s);

  void   GPS_Warning(const char* s);
  void   GPS_Error(const char* fmt, ...);
  void   GPS_Serial_Error(const char* hdr, ...);
  void   GPS_Fatal(const char* s);
  void   GPS_Enable_Error();
  void   GPS_Enable_Warning();
  void   GPS_Disable_Error();
  void   GPS_Disable_Warning();
  void   GPS_User(const char* fmt, ...);
  void   GPS_Disable_User();
  void   GPS_Enable_User();
  void   GPS_Diagnose(int32 c);
  void   GPS_Diag(const char* fmt, ...);

  void   GPS_Enable_Diagnose();
  void   GPS_Disable_Diagnose();


#endif
