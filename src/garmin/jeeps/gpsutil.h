/********************************************************************
** @source JEEPS utility functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301,  USA.
********************************************************************/
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
