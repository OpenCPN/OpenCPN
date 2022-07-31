/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
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

#include "comm_util.h"
#include "commdriverN2KSerial.h"
#include "commdriverN0183Serial.h"

AbstractCommDriver *MakeCommDriver(const ConnectionParams *params) {
  wxLogMessage(
      wxString::Format(_T("MakeCommDriver: %s"), params->GetDSPort().c_str()));

  switch (params->Type) {
    case SERIAL:
      switch (params->Protocol) {
         case PROTO_NMEA2000:
           return new commDriverN2KSerial(params);
        default:
          return new commDriverN0183Serial(params);
      }
#if 0
    case NETWORK:
      switch (params->NetProtocol) {
        case SIGNALK:
          return new SignalKDataStream(input_consumer, params);
        default:
          return new NetworkDataStream(input_consumer, params);
      }
    case INTERNAL_GPS:
      return new InternalGPSDataStream(input_consumer, params);
    case INTERNAL_BT:
      return new InternalBTDataStream(input_consumer, params);

#endif
    default:
      return NULL;
  }

}



