/******************************************************************************
 *
 * Project:  N2KParser.cpp
 * Purpose:  Common OpenCPN core and plugin N2K parser library
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

/*  Some elements of this combined work are covered by the following license. */

/*
Copyright (c) 2015-2022 Timo Lappalainen, Kave Oy, www.kave.fi

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "N2KParser.h"
#include "N2kMsg.h"
#include "N2kMessages.h"


uint32_t millis() {
  return 42;
}

tN2kMsg MakeN2kMsg(std::vector<unsigned char> &v) {

  tN2kMsg Msg;
  Msg.Clear();;

  unsigned char *Buf = v.data();

  int i=2;
  Msg.Priority=Buf[i++];
  Msg.PGN=GetBuf3ByteUInt(i,Buf);
  Msg.Destination=Buf[i++];
  if ( Buf[0]==/*MsgTypeN2kData*/0x93 ) {
    Msg.Source=Buf[i++];
    Msg.MsgTime=GetBuf4ByteUInt(i,Buf);
  } else {
    Msg.Source=255 /*DefaultSource*/;
    Msg.MsgTime=millis();
  }
  Msg.DataLen=Buf[i++];

  if ( Msg.DataLen>tN2kMsg::MaxDataLen ) {
    Msg.Clear();
  }

  for (int j=0; i<static_cast<int>(v.size())-1; i++, j++) Msg.Data[j]=Buf[i];

  return Msg;
}

bool ParseN2kPGN128275(std::vector<unsigned char> &v, uint16_t &DaysSince1970,
                       double &SecondsSinceMidnight, uint32_t &Log, uint32_t &TripLog) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN128275(msg, DaysSince1970, SecondsSinceMidnight, Log, TripLog);
}


bool ParseN2kPGN129029(std::vector<unsigned char> &v, unsigned char &SID, uint16_t &DaysSince1970, double &SecondsSinceMidnight,
                     double &Latitude, double &Longitude, double &Altitude,
                     tN2kGNSStype &GNSStype, tN2kGNSSmethod &GNSSmethod,
                     uint8_t &nSatellites, double &HDOP, double &PDOP, double &GeoidalSeparation,
                     uint8_t &nReferenceStations, tN2kGNSStype &ReferenceStationType, uint16_t &ReferenceSationID,
                     double &AgeOfCorrection
                     ) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN129029(msg, SID, DaysSince1970, SecondsSinceMidnight,
                     Latitude, Longitude, Altitude,
                     GNSStype, GNSSmethod,
                     nSatellites, HDOP, PDOP, GeoidalSeparation,
                     nReferenceStations, ReferenceStationType, ReferenceSationID,
                     AgeOfCorrection
                     );
}


bool ParseN2kPGN129025(std::vector<unsigned char> &v, double &Latitude, double &Longitude) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN129025(msg, Latitude, Longitude);
}


bool ParseN2kPGN129026(std::vector<unsigned char> &v, unsigned char &SID,
                       tN2kHeadingReference &ref, double &COG, double &SOG) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN129026(msg, SID, ref, COG, SOG);

}

// Rudder
bool ParseN2kPGN127245(std::vector<unsigned char> &v, double &RudderPosition, unsigned char &Instance,
                       tN2kRudderDirectionOrder &RudderDirectionOrder, double &AngleOrder) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN127245(msg, RudderPosition, Instance,
                           RudderDirectionOrder, AngleOrder);
}

bool ParseN2kPGN127250(std::vector<unsigned char> &v, unsigned char &SID,
                       double &Heading, double &Deviation, double &Variation, tN2kHeadingReference &ref) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN127250(msg, SID, Heading, Deviation, Variation, ref);

}

bool ParseN2kPGN127257(std::vector<unsigned char> &v, unsigned char &SID,
                       double &Yaw, double &Pitch, double &Roll) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN127257(msg, SID, Yaw, Pitch, Roll);

}

bool ParseN2kPGN128259(std::vector<unsigned char> &v, unsigned char &SID,
                       double &WaterReferenced, double &GroundReferenced,
                       tN2kSpeedWaterReferenceType &SWRT) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN128259(msg, SID, WaterReferenced, GroundReferenced, SWRT);

}

bool ParseN2kPGN129540(std::vector<unsigned char> &v, unsigned char &SID,
                       tN2kRangeResidualMode &Mode, uint8_t &nSats) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN129540(msg, SID, Mode, nSats);

}

bool ParseN2kPGN129540(std::vector<unsigned char> &v, uint8_t SVIndex, tSatelliteInfo& SatelliteInfo) {

  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN129540(msg, SVIndex, SatelliteInfo);
}


bool ParseN2kPGN129038(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM, uint8_t &Seconds,
                        double &COG, double &SOG, double &Heading, double &ROT, tN2kAISNavStatus &NavStatus)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN129038(msg, MessageID, Repeat, UserID,
                        Latitude, Longitude, Accuracy, RAIM, Seconds,
                        COG, SOG, Heading, ROT, NavStatus);
}

bool ParseN2kPGN129039(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM, uint8_t &Seconds, double &COG,
                        double &SOG, tN2kAISTransceiverInformation &AISTransceiverInformation, double &Heading,
                        tN2kAISUnit &Unit, bool &Display, bool &DSC, bool &Band, bool &Msg22, tN2kAISMode &Mode, bool &State)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN129039(msg, MessageID, Repeat, UserID,
                        Latitude, Longitude, Accuracy, RAIM, Seconds, COG,
                        SOG, AISTransceiverInformation, Heading,
                        Unit, Display, DSC, Band, Msg22, Mode, State);
}



bool ParseN2kPGN129794(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        uint32_t &IMOnumber, char *Callsign, char *Name, uint8_t &VesselType, double &Length,
                        double &Beam, double &PosRefStbd, double &PosRefBow, uint16_t &ETAdate, double &ETAtime,
                        double &Draught, char *Destination, tN2kAISVersion &AISversion, tN2kGNSStype &GNSStype,
                        tN2kAISDTE &DTE, tN2kAISTranceiverInfo &AISinfo)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN129794(msg, MessageID, Repeat, UserID,
                        IMOnumber, Callsign, Name, VesselType, Length,
                        Beam, PosRefStbd, PosRefBow, ETAdate, ETAtime,
                        Draught, Destination, AISversion, GNSStype,
                        DTE, AISinfo);
}

bool ParseN2kPGN129809(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID, char *Name)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN129809(msg, MessageID, Repeat, UserID, Name);
}


bool ParseN2kPGN129810(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                      uint8_t &VesselType, char *Vendor, char *Callsign, double &Length, double &Beam,
                      double &PosRefStbd, double &PosRefBow, uint32_t &MothershipID)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN129810(msg, MessageID, Repeat, UserID,
                      VesselType, Vendor, Callsign, Length, Beam,
                      PosRefStbd, PosRefBow, MothershipID);
}

bool ParseN2kPGN129041(std::vector<unsigned char> &v, tN2kAISAtoNReportData &N2kData)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN129041(msg, N2kData);
}

// Water depth
bool ParseN2kPGN128267(std::vector<unsigned char> &v, unsigned char &SID,
                       double &DepthBelowTransducer, double &Offset, double &Range)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN128267(msg, SID,
                       DepthBelowTransducer, Offset, Range);
}

// Wind Speed
bool ParseN2kPGN130306(std::vector<unsigned char> &v, unsigned char &SID,
                       double &WindSpeed, double &WindAngle, tN2kWindReference &WindReference)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN130306(msg, SID,
                       WindSpeed, WindAngle, WindReference);
}

// Outside Environmental parameters
bool ParseN2kPGN130310(std::vector<unsigned char> &v, unsigned char &SID, double &WaterTemperature,
                     double &OutsideAmbientAirTemperature, double &AtmosphericPressure)
{
    tN2kMsg msg = MakeN2kMsg(v);

    return ParseN2kPGN130310(msg, SID, WaterTemperature,
                     OutsideAmbientAirTemperature, AtmosphericPressure);
}


// AIS Base Station position/time report
bool ParseN2kPGN129793(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Longitude, double &Latitude, unsigned int &SecondsSinceMidnight,
                        unsigned int &DaysSinceEpoch)
{
  tN2kMsg msg = MakeN2kMsg(v);

  return ParseN2kPGN129793(msg, MessageID, Repeat, UserID,
                        Longitude, Latitude,
                        SecondsSinceMidnight, DaysSinceEpoch);
}
