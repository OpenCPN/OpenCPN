/*
    Garmin Jeeps Interface Wrapper.

    Copyright (C) 2010 David S Register

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA

 */

#ifndef garmin_wrapper_utils_h
#define garmin_wrapper_utils_h

#include <sstream>
#include <iomanip>

#include <string.h>

#include <wx/log.h>

#include "../jeeps/gps.h"

int Garmin_Serial_GPS_PVT_On(const char *port_name);
int Garmin_Serial_GPS_PVT_Off(const char *port_name);
int GPS_Serial_Command_Pvt_Get(GPS_PPvt_Data *pvt);

void static inline _wxLogFatal(const char* fmt...) {
    va_list ap;
    va_start(ap, fmt);
    wxVLogFatalError(fmt, ap);
    va_end(ap);
}

void static inline _wxLogError(const char* fmt...) {
    va_list ap;
    va_start(ap, fmt);
    wxVLogError(fmt, ap);
    va_end(ap);
}


void static inline _wxLogWarning(const char* fmt...) {
    va_list ap;
    va_start(ap, fmt);
    wxVLogWarning(fmt, ap);
    va_end(ap);
}


void static inline _wxLogMessage(const char* fmt...) {
    va_list ap;
    va_start(ap, fmt);
    wxVLogMessage(fmt, ap);
    va_end(ap);
}


void static inline _wxLogDebug(const char* fmt...) {
    va_list ap;
    va_start(ap, fmt);
    wxVLogDebug(fmt, ap);
    va_end(ap);
}

#if defined(__GNUC__) || defined (__clang__)

#define fatal(fmt, ...)       _wxLogFatal(fmt, ## __VA_ARGS__)
#define error(fmt, ...)       _wxLogError(fmt, ## __VA_ARGS__)
#define warning(fmt, ...)     _wxLogWarning(fmt, ## __VA_ARGS__)
#define message(fmt, ...)     _wxLogMessage(fmt, ## __VA_ARGS__)
#define debug(fmt, ...)       _wxLogDebug(fmt, ## __VA_ARGS__)
#define GPS_User(fmt, ...)    _wxLogMessage(fmt, ## __VA_ARGS__)
#define GPS_Diag(fmt, ...)    _wxLogDebug(fmt, ## __VA_ARGS__)
#define GPS_Error(fmt, ...)   _wxLogError(fmt, ## __VA_ARGS__)

#else

#define fatal(fmt, ...)       _wxLogFatal(fmt, __VA_ARGS__)
#define error(fmt, ...)       _wxLogError(fmt, __VA_ARGS__)
#define warning(fmt, ...)     _wxLogWarning(fmt, __VA_ARGS__)
#define message(fmt, ...)     _wxLogMessage(fmt, __VA_ARGS__)
#define debug(fmt, ...)       _wxLogDebug(fmt, __VA_ARGS__)
#define GPS_User(fmt, ...)    _wxLogMessage(fmt, __VA_ARGS__)
#define GPS_Diag(fmt, ...)    _wxLogDebug(fmt, __VA_ARGS__)
#define GPS_Error(fmt, ...)   _wxLogError(fmt,  __VA_ARGS__)

#endif

static inline void gbser__db(int l, const char* msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  if  (l == 1)
    wxVLogError(msg, ap);
  else if (l == 2)
    wxVLogWarning(msg, ap);
  else if (l == 3)
    wxVLogMessage(msg, ap);
  else
    wxVLogDebug(msg, ap);
  va_end(ap);
}


#ifdef __cplusplus
extern "C" {
#endif


/*    Some function stubs sprinkled throughout jeeps  */
int case_ignore_strcmp(const char *s1, const char *s2);
int case_ignore_strncmp(const char *s1, const char *s2, int n);
void xfree(void *mem);

extern gpsdevh* g_gps_devh;

const char* GetDeviceLastError(void);

#ifdef __cplusplus
}
#endif

#endif      /* garmin_wrapper_utils_h */
