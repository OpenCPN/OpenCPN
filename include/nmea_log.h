#ifndef _ABSTRACT_NMEA_LOG__
#define _ABSTRACT_NMEA_LOG__

#include <wx/string.h>

class NmeaLog {

  /** Add an formatted string to log output. */
  virtual void Add(const wxString& s) = 0;

  /** Return true if log is visible i. e., if it's any point using Add(). */
  virtual bool Active() const = 0;
};

#endif    // _ABSTRACT_NMEA_LOG__
