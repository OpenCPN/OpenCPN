//////////////////////////////////////////////////////////////////////////////
// File:        crashprint.h
// Purpose:     wxCrashPrint class
// Maintainer:  Wyo
// Created:     2004-09-28
// RCS-ID:      $Id: crashprint.h,v 1.5 2004-11-22 18:27:11 wyo Exp $
// Copyright:   (c) 2004 wxCode
// Licence:     wxWindows
//////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CRASHPRINT_H_
#define _WX_CRASHPRINT_H_

// #ifdef __GNUG__
//     #pragma implementation "crashprint.h"
// #endif

//----------------------------------------------------------------------------
// information
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------
//! wxWidgets headers

//============================================================================
// declarations
//============================================================================
const int maxBtCount = 100;

/**
 * Handles crash reporting in wxWidgets applications.
 */
class wxCrashPrint {
public:
  //! constructor
  wxCrashPrint(int flags = 0, const wxString &fname = wxEmptyString);

  //! destructor
  ~wxCrashPrint() {};

  // format and print report
  void Report();

private:
  //! variables
  int m_flags; ///< Flags controlling the crash report behavior.
  wxString m_fname; ///< Filename to save the crash report to.

  void *m_btBuffer[maxBtCount]; ///< Buffer for storing backtrace information.
  char **m_btStrings; ///< Strings containing backtrace information.
};

#endif  // _WX_CRASHPRINT_H_
