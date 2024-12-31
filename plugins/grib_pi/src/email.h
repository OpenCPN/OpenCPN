/////////////////////////////////////////////////////////////////////////////
// Name:        email.h
// Purpose:     wxEmail: portable email client class
// Author:      Julian Smart
// Modified by:
// Created:     2001-08-21
// RCS-ID:      $Id: email.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
/**
 * \file
 * Email Request System for GRIB Data.
 *
 * Provides portable email functionality for requesting GRIB weather data from
 * email-based services. Features:
 * - Cross-platform email sending capabilities
 * - Support for multiple sending methods:
 *   - Direct sendmail on Unix systems
 *   - xdg-email for desktop integration
 *   - System-specific methods for other platforms
 * - Profile-based configuration
 * - Message template handling
 *
 * The system allows mariners to request GRIB files through email services
 * when direct downloads are not available or preferred.
 */
#ifndef _WX_EMAIL_H_
#define _WX_EMAIL_H_

#include "msg.h"

/*
 * wxEmail
 * Miscellaneous email functions
 */

class WXDLLIMPEXP_NETUTILS wxEmail {
public:
  //// Ctor/dtor
  wxEmail() {};

  //// Operations

  // Send a message.
  // Specify profile, or leave it to wxWidgets to find the current user name
  // two sending methods for Unix plateforms : sendmail or xdg-email shell
  // script
  static bool Send(
      wxMailMessage& message, int sendMethod,
      const wxString& profileName = wxEmptyString,
      const wxString& sendMail2 = wxT("/usr/sbin/sendmail -t"),  // sendmail
      const wxString& sendMail1 =
          wxT("/usr/bin/xdg-email"),  // xdg in bin folder
      const wxString& sendMail0 =
          wxT("/usr/sbin/xdg-email"));  // xdg in sbin folder

protected:
};

#endif  //_WX_EMAIL_H_
