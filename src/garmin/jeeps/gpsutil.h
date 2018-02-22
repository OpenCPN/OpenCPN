#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gpsutil_h
#define gpsutil_h


#include "garmin_gps.h"

int32  GPS_Util_Little(void);

US     GPS_Util_Get_Short(const UC *s);
void   GPS_Util_Put_Short(UC *s, const US v);
int32  GPS_Util_Get_Int(const UC *s);
void   GPS_Util_Put_Int(UC *s, const int32 v);
double GPS_Util_Get_Double(const UC *s);
void   GPS_Util_Put_Double(UC *s, const double v);
float  GPS_Util_Get_Float(const UC *s);
void   GPS_Util_Put_Float(UC *s, const float v);
void   GPS_Util_Canon(int32 state);
int32  GPS_Util_Block(int32 fd, int32 state);
void   GPS_Util_Put_Uint(UC *s, const uint32 v);
uint32 GPS_Util_Get_Uint(const UC *s);

void   GPS_Warning(char *s);
void   GPS_Error(char *fmt, ...);
void   GPS_Serial_Error(const char *hdr, ...);
void   GPS_Fatal(char *s);
void   GPS_Enable_Error(void);
void   GPS_Enable_Warning(void);
void   GPS_Disable_Error(void);
void   GPS_Disable_Warning(void);
void   GPS_User(const char *fmt, ...);
void   GPS_Disable_User(void);
void   GPS_Enable_User(void);
void   GPS_Diagnose(int32 c);
void   GPS_Diag(const char *fmt, ...);

void   GPS_Enable_Diagnose(void);
void   GPS_Disable_Diagnose(void);
char * GetDeviceLastError(void);


#endif

#ifdef __cplusplus
}
#endif
