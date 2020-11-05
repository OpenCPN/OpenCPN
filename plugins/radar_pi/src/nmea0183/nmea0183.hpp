/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Samuel R. Blackburn, David S Register           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *   S Blackburn's original source license:                                *
 *         "You can use it any way you like."                              *
 *   More recent (2010) license statement:                                 *
 *         "It is BSD license, do with it what you will"                   *
 */

#if !defined(NMEA_0183_CLASS_HEADER)
#define NMEA_0183_CLASS_HEADER

#include "pi_common.h"

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

/*
** General Purpose Classes
*/

#include "Response.hpp"
#include "Sentence.hpp"

/*
** Response Classes
*/

#include "hdg.hpp"
#include "hdm.hpp"
#include "hdt.hpp"

PLUGIN_BEGIN_NAMESPACE

WX_DECLARE_LIST(RESPONSE, MRL);

class NMEA0183 {
 private:
  SENTENCE sentence;

  void initialize(void);

 protected:
  MRL response_table;

  void set_container_pointers(void);
  void sort_response_table(void);

 public:
  NMEA0183();
  virtual ~NMEA0183();

  wxArrayString GetRecognizedArray(void);

  /*
  ** NMEA 0183 Sentences we understand
  */

  HDM Hdm;
  HDG Hdg;
  HDT Hdt;

  wxString ErrorMessage;            // Filled when Parse returns FALSE
  wxString LastSentenceIDParsed;    // ID of the lst sentence successfully parsed
  wxString LastSentenceIDReceived;  // ID of the last sentence received, may not have parsed successfully

  wxString TalkerID;
  wxString ExpandedTalkerID;

  //      MANUFACTURER_LIST Manufacturers;

  virtual bool IsGood(void) const;
  virtual bool Parse(void);
  virtual bool PreParse(void);

  NMEA0183& operator<<(wxString& source);
  NMEA0183& operator>>(wxString& destination);
};

PLUGIN_END_NAMESPACE

#endif  // NMEA_0183_CLASS_HEADER
