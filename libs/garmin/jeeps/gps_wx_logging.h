/** Remap internal logging functions to wxWidgets logging functions. */

#ifndef wx_log_h_
#define wx_log_h_

#include <sstream>
#include <iomanip>

#include <wx/log.h>

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


void static do_GPS_Diagnose(int c) {
    static std::stringstream ss;
    if (ss.str().length() > 80 || c == 10) {
        wxLogDebug(ss.str().c_str());
        ss.clear();
    }
    ss << " " << std::setfill('0') << std::setw(2) << c;
}



#if defined(__GNUC__) || defined (__clang__)

#define GPS_Error(fmt, ...)     _wxLogError(fmt, ## __VA_ARGS__)
#define GPS_Warning(fmt, ...)   _wxLogWarning(fmt, ## __VA_ARGS__)
#define GPS_User(fmt, ...)      _wxLogMessage(fmt, ## __VA_ARGS__)
#define GPS_Diag(fmt, ...)      _wxLogDebug(fmt, ## __VA_ARGS__)

#else

#define GPS_Error(fmt, ...)     _wxLogError(fmt, __VA_ARGS__)
#define GPS_Warning(fmt, ...)   _wxLogWarning(fmt, __VA_ARGS__)
#define GPS_User(fmt, ...)      _wxLogMessage(fmt, __VA_ARGS__)
#define GPS_Diag(fmt, ...)      _wxLogDebug(fmt, __VA_ARGS__)

#endif   // defined(__GNUC__) || defined (__clang__)
#define GPS_Diagnose(c)         _do_GPS_Diagnose((int) c);


#endif  // wx_log_h_
