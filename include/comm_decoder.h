/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 *
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include <memory>
#include <string>

#include "comm_appmsg.h"
#include "nmea0183.h"

#ifndef _COMM_DECODER_H
#define _COMM_DECODER_H

typedef struct {
  int gps_watchdog;
  int var_watchdog;
  int hdx_watchdog;
  int hdt_watchdog;
  int sat_watchdog;

} Watchdogs;

class CommDecoder {
public:
  CommDecoder(){};
  ~CommDecoder(){};

  // NMEA decoding, by sentence.
  // Each method updates the global variable set
  bool DecodeRMC(std::string s, Watchdogs& dogs);
  bool DecodeHDM(std::string s, Watchdogs& dogs);
  bool DecodeHDT(std::string s, Watchdogs& dogs);
  bool DecodeHDG(std::string s, Watchdogs& dogs);
  bool DecodeVTG(std::string s, Watchdogs& dogs);
  bool DecodeGSV(std::string s, Watchdogs& dogs);
  bool DecodeGGA(std::string s, Watchdogs& dogs);
  bool DecodeGLL(std::string s, Watchdogs& dogs);

  bool ParsePosition(const LATLONG& Position, double& lat, double& lon);

  NMEA0183 m_NMEA0183;  // Used to parse messages from NMEA threads
};

#endif  // _COMM_DECODER_H
