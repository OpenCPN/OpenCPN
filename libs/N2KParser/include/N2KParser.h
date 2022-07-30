/******************************************************************************
 *
 * Project:  N2KParser.h
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

#ifndef _N2KPARSER_H__
#define _N2KPARSER_H__

#include <vector>
#include <stdint.h>

#include "N2kTypes.h"
#include "N2kMessages.h"

// Raw data definition
typedef std::vector<unsigned char> n2k_rawData;

// The Raw data vector arrives in the following byte format.
//<length (1)><priority (1)><PGN (3)><destination (1)><source (1)><time (4)><len (1)><data (len)><CRC (1)>

// Utility Functions
unsigned long GetPGN( n2k_rawData *v);
bool VerifyCRC( n2k_rawData *v);
int GetSource(n2k_rawData *v);
int GetDestination(n2k_rawData *v);
//time_t GetTime(n2k_rawData *v);


//-----------------------------------------------------------------------------
//  Basic navigation information parsing
//-----------------------------------------------------------------------------

//*****************************************************************************
// GNSS Position Data
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and GPS at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - DaysSince1970         Days since 1.1.1970. You can find sample how to convert e.g. NMEA0183 date info to this on my NMEA0183 library on
//                          NMEA0183Messages.cpp on function NMEA0183ParseRMC_nc
//  - Latitude              Latitude in degrees
//  - Longitude             Longitude in degrees
//  - Altitude              Altitude in meters
//  - GNSStype              GNSS type. See definition of tN2kGNSStype
//  - GNSSmethod            GNSS method type. See definition of tN2kGNSSmethod
//  - nSatellites           Number of satellites used for data
//  - HDOP                  Horizontal Dilution Of Precision in meters.
//  - PDOP                  Probable dilution of precision in meters.
//  - GeoidalSeparation     Geoidal separation in meters
bool ParseN2kPGN129029(std::vector<unsigned char> &v, unsigned char &SID, uint16_t &DaysSince1970, double &SecondsSinceMidnight,
                     double &Latitude, double &Longitude, double &Altitude,
                     tN2kGNSStype &GNSStype, tN2kGNSSmethod &GNSSmethod,
                     unsigned char &nSatellites, double &HDOP, double &PDOP, double &GeoidalSeparation,
                     unsigned char &nReferenceStations, tN2kGNSStype &ReferenceStationType, uint16_t &ReferenceSationID,
                     double &AgeOfCorrection
                     );

//*****************************************************************************
// GNSS DOP data
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and GPS at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - DesiredMode           Desired DOP mode.
//  - ActualMode            Actual DOP mode.
//  - HDOP                  Horizontal Dilution Of Precision in meters.
//  - PDOP                  Probable dilution of precision in meters.
//  - TDOP                  Time dilution of precision
bool ParseN2kPgn129539(std::vector<unsigned char> &v, unsigned char& SID, tN2kGNSSDOPmode& DesiredMode, tN2kGNSSDOPmode& ActualMode,
                       double& HDOP, double& VDOP, double& TDOP);


//*****************************************************************************
// struct tSatelliteInfo {
//   unsigned char PRN;
//   double Elevation;
//   double Azimuth;
//   double SNR;
//   double RangeResiduals;
//   tN2kPRNUsageStatus UsageStatus;
// };

//*****************************************************************************
// GNSS Satellites in View
// Parse GNSS Satellites in View
// Use first function to get basic information and specially NumberOfSVs. Then use second function to get
// information for specific satellite.
// It is possible to use just second function, but it returns also false for wrong message.
//
// Request common information for GNSS Satellites in View
// Output:
//  - SID                   Sequence ID. If your device is e.g. boat speed and GPS at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Mode                  Range residual mode.
//  - NumberOfSVs           Number of satellite infos in message.
//
// Return:
//   true  - if function succeeds.
//   false - when called with wrong message.
bool ParseN2kPGN129540(std::vector<unsigned char> &v, unsigned char& SID, tN2kRangeResidualMode &Mode, uint8_t& NumberOfSVs);

//*****************************************************************************
// Request specific satellite info from message.
// Input:
//  - SVIndex               Index of saelliten info requested.
//
// Output:
//  - SatelliteInfo         Requested satellite info.
//
// Return:
//   true  - if function succeeds.
//   false - when called with wrong message or SVIndex in second function is out of range.
bool ParseN2kPGN129540(const tN2kMsg& N2kMsg, uint8_t SVIndex, tSatelliteInfo& SatelliteInfo);

//*****************************************************************************
// Lat/lon rapid
// Input:
//  - Latitude               Latitude in degrees
//  - Longitude              Longitude in degrees
bool ParseN2kPGN129025(std::vector<unsigned char> &v, double &Latitude, double &Longitude);


//*****************************************************************************
// COG SOG rapid
// Input:
//  - COG                   Cource Over Ground in radians
//  - SOG                   Speed Over Ground in m/s
bool ParseN2kPGN129026(std::vector<unsigned char> &v, unsigned char &SID, tN2kHeadingReference &ref, double &COG, double &SOG);

//*****************************************************************************
// Vessel Heading
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Heading               Heading in radians
//  - Deviation             Magnetic deviation in radians. Use N2kDoubleNA for undefined value.
//  - Variation             Magnetic variation in radians. Use N2kDoubleNA for undefined value.
//  - ref                   Heading reference. See definition of tN2kHeadingReference.
bool ParseN2kPGN127250(std::vector<unsigned char> &v, unsigned char &SID, double &Heading, double &Deviation, double &Variation, tN2kHeadingReference &ref);

//*****************************************************************************
// Magnetic Variation
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Source                How was the variation value generated
//  - DaysSince1970         Days since January 1, 1970
//  - Variation             Magnetic variation/declination in radians
bool ParseN2kPGN127258(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kMagneticVariation &Source, uint16_t &DaysSince1970, double &Variation);

//*****************************************************************************
// Direction Data
// Input:
//  - DataMode
//  - COGReference            True or Magnetic
//  - SID                     Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                            to indicate that they are measured at same time
//  - COG                     Course Over Ground in radians
//  - SOG                     Speed Over Ground in m/s
//  - Heading                 In radians
//  - SpeedThroughWater       In m/s
//  - Set                     In radians
//  - Drift                   In m/s
bool ParseN2kPGN130577(std::vector<unsigned char> &v,tN2kDataMode &DataMode, tN2kHeadingReference &CogReference,unsigned char &SID,double &COG,
      double &SOG,double &Heading,double &SpeedThroughWater,double &Set,double &Drift);

//*****************************************************************************
// Boat speed
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and wind at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - WaterReferenced        Speed over water in m/s
//  - GroundReferenced      Ground referenced speed in m/s
//  - SWRT                  Type of transducer. See definition for tN2kSpeedWaterReferenceType
bool ParseN2kPGN128259(std::vector<unsigned char> &v, unsigned char &SID, double &WaterReferenced, double &GroundReferenced, tN2kSpeedWaterReferenceType &SWRT);



//-----------------------------------------------------------------------------
// Extended Navigation information parsing
//-----------------------------------------------------------------------------


//*****************************************************************************
// Rate of Turn
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Rate of turn          Change in heading in radians per second
bool ParseN2kPGN127251(std::vector<unsigned char> &v, unsigned char &SID, double &RateOfTurn);

//*****************************************************************************
// Attitude
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Yaw                   Heading in radians.
//  - Pitch                 Pitch in radians. Positive, when your bow rises.
//  - Roll                  Roll in radians. Positive, when tilted right.
bool ParseN2kPGN127257(std::vector<unsigned char> &v, unsigned char &SID, double &Yaw, double &Pitch, double &Roll);

//*****************************************************************************
// Leeway
// Input:
//  - SID            Sequence ID field
//  - Leeway         Nautical Leeway Angle, which is defined as the angle between the vessel’s heading (direction to which the
//                   vessel’s bow points) and its course (direction of its motion (track) through the water)
bool ParseN2kPGN128000(std::vector<unsigned char> &v, unsigned char &SID, double &Leeway);


//*****************************************************************************
// Water depth
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and depth at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - DepthBelowTransducer  Depth below transducer in meters
//  - Offset                Distance in meters between transducer and surface (positive) or transducer and keel (negative)
//  - Range                 Measuring range
bool ParseN2kPGN128267(std::vector<unsigned char> &v, unsigned char &SID, double &DepthBelowTransducer, double &Offset, double &Range);

//*****************************************************************************
// Rudder
// Input:
// - RudderPosition         Current rudder postion in radians.
// - Instance               Rudder instance.
// - RudderDirectionOrder   See tN2kRudderDirectionOrder. Direction, where rudder should be turned.
// - AngleOrder             In radians angle where rudder should be turned.
bool ParseN2kPGN127245(std::vector<unsigned char> &v, double &RudderPosition, unsigned char &Instance,
                     tN2kRudderDirectionOrder &RudderDirectionOrder, double &AngleOrder);


//*****************************************************************************
// Distance log
// Input:
//  - DaysSince1970         Timestamp
//  - SecondsSinceMidnight  Timestamp
//  - Log                   Total meters travelled
//  - Trip Log              Meters travelled since last reset
bool ParseN2kPGN128275(std::vector<unsigned char> &v, uint16_t &DaysSince1970, double &SecondsSinceMidnight, uint32_t &Log, uint32_t &TripLog);


//*****************************************************************************
// Heading/Track control
// Input:
// - RudderLimitExceeded         Yes/No
// - OffHeadingLimitExceeded     Yes/No
// - OffTrackLimitExceeded       Yes/No
// - Override                    Yes/No
// - SteeringMode                Steering mode
// - TurnMode                    Turn mode
// - HeadingReference            True or Magnetic
// - CommandedRudderDirection    Port or Starboard
// - CommandedRudderAngle        In radians
// - HeadingToSteerCourse        In radians
// - Track                       In radians
// - RudderLimit,                In radians
// - OffHeadingLimit,            In radians
// - RadiusOfTurnOrder,          In metres
// - RateOfTurnOrder,            In radians/s
// - OffTrackLimit,              In metres
// - VesselHeading               In radians
bool ParseN2kPGN127237(const tN2kMsg &N2kMsg,
      tN2kOnOff &RudderLimitExceeded,
      tN2kOnOff &OffHeadingLimitExceeded,
      tN2kOnOff &OffTrackLimitExceeded,
      tN2kOnOff &Override,
      tN2kSteeringMode &SteeringMode,
      tN2kTurnMode &TurnMode,
      tN2kHeadingReference &HeadingReference,
      tN2kRudderDirectionOrder &CommandedRudderDirection,
      double &CommandedRudderAngle,
      double &HeadingToSteerCourse,
      double &Track,
      double &RudderLimit,
      double &OffHeadingLimit,
      double &RadiusOfTurnOrder,
      double &RateOfTurnOrder,
      double &OffTrackLimit,
      double &VesselHeading);

//*****************************************************************************
// Cross Track Error
bool ParseN2kPGN129283(std::vector<unsigned char> &v, unsigned char& SID, tN2kXTEMode& XTEMode, bool& NavigationTerminated, double& XTE);


//*****************************************************************************
// Navigation info
bool ParseN2kPGN129284(std::vector<unsigned char> &v, unsigned char& SID, double& DistanceToWaypoint, tN2kHeadingReference& BearingReference,
                      bool& PerpendicularCrossed, bool& ArrivalCircleEntered, tN2kDistanceCalculationType& CalculationType,
                      double& ETATime, int16_t& ETADate, double& BearingOriginToDestinationWaypoint, double& BearingPositionToDestinationWaypoint,
                      uint8_t& OriginWaypointNumber, uint8_t& DestinationWaypointNumber,
                      double& DestinationLatitude, double& DestinationLongitude, double& WaypointClosingVelocity);

//*****************************************************************************
// Man Overboard Notification
// Input:
//  - SID                     Sequence ID.
//  - MobEmitterId            Identifier for each MOB emitter, unique to the vessel
//  - MOBStatus               MOB Status
//  - ActivationTime          Time of day (UTC) when MOB was activated
//  - PositionSource          Position Source
//
//  - PositionDate            Date of MOB position
//  - PositionTime            Time of day of MOB position (UTC)
//  - Latitude                Latitude in degrees
//  - Longitude               Longitude in degrees
//  - COGReference            True or Magnetic
//  - COG                     Course Over Ground in radians
//  - SOG                     Speed Over Ground in m/s
//  - MMSI                    MMSI
//  - MOBEmitterBatteryStatus Battery status
// Output:
//  - N2kMsg                  NMEA2000 message ready to be send.

bool ParseN2kPGN127233(std::vector<unsigned char> &v,
      unsigned char &SID,
      uint32_t &MobEmitterId,
      tN2kMOBStatus &MOBStatus,
      double &ActivationTime,
      tN2kMOBPositionSource &PositionSource,
      uint16_t &PositionDate,
      double &PositionTime,
      double &Latitude,
      double &Longitude,
      tN2kHeadingReference &COGReference,
      double &COG,
      double &SOG,
      uint32_t &MMSI,
      tN2kMOBEmitterBatteryStatus &MOBEmitterBatteryStatus);


//-----------------------------------------------------------------------------
//  AIS information parsing
//-----------------------------------------------------------------------------

//*****************************************************************************
// AIS position reports for Class A
// Input:
//  - N2kMsg                NMEA2000 message to decode
bool ParseN2kPGN129038(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID, double &Latitude, double &Longitude,
                        bool &Accuracy, bool &RAIM, uint8_t &Seconds, double &COG, double &SOG, double &Heading, double &ROT, tN2kAISNavStatus &NavStatus);

//*****************************************************************************
// AIS position reports for Class B
// Input:
//  - N2kMsg                NMEA2000 message to decode
bool ParseN2kPGN129039(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM, uint8_t &Seconds, double &COG,
                        double &SOG, tN2kAISTransceiverInformation &AISTransceiverInformation, double &Heading,
                        tN2kAISUnit &Unit, bool &Display, bool &DSC, bool &Band, bool &Msg22, tN2kAISMode &Mode, bool &State);


//*****************************************************************************
// AIS static data class A
// Input:
//  - MessageID             Message type
//  - Repeat                Repeat indicator
//  - UserID                MMSI
bool ParseN2kPGN129794(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        uint32_t &IMOnumber, char *Callsign, char *Name, uint8_t &VesselType, double &Length,
                        double &Beam, double &PosRefStbd, double &PosRefBow, uint16_t &ETAdate, double &ETAtime,
                        double &Draught, char *Destination, tN2kAISVersion &AISversion, tN2kGNSStype &GNSStype,
                        tN2kAISDTE &DTE, tN2kAISTranceiverInfo &AISinfo);

//*****************************************************************************
// AIS static data class B part A
// Input:
//  - MessageID             Message type
//  - Repeat                Repeat indicator
//  - UserID                MMSI
//  - Name                  Vessel name
bool ParseN2kPGN129809(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID, char *Name);


//*****************************************************************************
// AIS static data class B part B
// Input:
//  - MessageID             Message type
//  - Repeat                Repeat indicator
//  - UserID                MMSI
//  - Name                  Vessel name
bool ParseN2kPGN129810(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                      uint8_t &VesselType, char *Vendor, char *Callsign, double &Length, double &Beam,
                      double &PosRefStbd, double &PosRefBow, uint32_t &MothershipID);

//*****************************************************************************
// AIS Aids to Navigation (AtoN) Report
// Input:
// - MessageID
// - Repeat                       Used by the repeater to indicate how many times a message has been repeated
// - UserID                       MMSI Number
// - Longitude                    Longitude Position
// - Latitude                     Latitude Position
// - Accuracy                     Position accuracy (1 = <10m, 0 = >10m)
// - RAIM                         RAIM (Receiver autonomous integrity monitoring) flag of electronic position fixing device
// - Seconds                      UTC second when the report was generated by the EPFS
// - Length                       Length of AtoN
// - Beam                         Beam of AtoN
// - PositionReferenceStarboard
// - PositionReferenceTrueNorth
// - AtoNType                     Type of AtoN
// - OffPositionIndicator         For floating AtoN, only: 0 = on position; 1 = off position.
// - VirtualAtoNFlag              0 = default = real AtoN at indicated position; 1 = virtual AtoN, does not physically exist
// - AssignedModeFlag             0 = autonomous mode, 1 = assigned mode
// - GNSSTYpe                     Type of GPS
// - AtoNStatus                   Reserved for indication of AtoN status (0b00000000 default)
// - AISTransceiverInformation    see tN2kAISTransceiverInformation
// - AtoNName
//
bool ParseN2kPGN129041(std::vector<unsigned char> &v, tN2kAISAtoNReportData &N2kData);




//-----------------------------------------------------------------------------
//  Engine information parsing
//-----------------------------------------------------------------------------

//*****************************************************************************
// Engine parameters rapid
// Input:
//  - EngineInstance        Engine instance.
//  - EngineSpeed           RPM (Revolutions Per Minute)
//  - EngineBoostPressure   in Pascal
//  - EngineTiltTrim        in %
bool ParseN2kPGN127488(std::vector<unsigned char> &v, unsigned char &EngineInstance, double &EngineSpeed,
                     double &EngineBoostPressure, int8_t &EngineTiltTrim);

#if 0
//*****************************************************************************
// Engine parameters dynamic
// Input:
//  - EngineInstance        Engine instance.
//  - EngineOilPress        in Pascal
//  - EngineOilTemp         in Kelvin
//  - EngineCoolantTemp     in Kelvin
//  - AltenatorVoltage      in Voltage
//  - FuelRate              in litres/hour
//  - EngineHours           in seconds
//  - EngineCoolantPress    in Pascal
//  - EngineFuelPress       in Pascal
//  - EngineLoad            in %
//  - EngineTorque          in %
bool ParseN2kPGN127489(std::vector<unsigned char> &v, unsigned char &EngineInstance, double &EngineOilPress,
                      double &EngineOilTemp, double &EngineCoolantTemp, double &AltenatorVoltage,
                      double &FuelRate, double &EngineHours, double &EngineCoolantPress, double &EngineFuelPress,
                      int8_t &EngineLoad, int8_t &EngineTorque,
                      tN2kEngineDiscreteStatus1 &Status1, tN2kEngineDiscreteStatus2 &Status2);
#endif

//*****************************************************************************
// Transmission parameters, dynamic
// Input:
//  - EngineInstance        Engine instance.
//  - TransmissionGear      Selected transmission. See tN2kTransmissionGear
//  - OilPressure           in Pascal
//  - OilTemperature        in K
//  - EngineTiltTrim        in %

bool ParseN2kPGN127493(std::vector<unsigned char> &v, unsigned char &EngineInstance, tN2kTransmissionGear &TransmissionGear,
                     double &OilPressure, double &OilTemperature, unsigned char &DiscreteStatus1);

//*****************************************************************************
// Trip Parameters, Engine
// Input:
//  - EngineInstance           Engine instance.
//  - TripFuelUsed             in litres
//  - FuelRateAverage          in litres/hour
//  - FuelRateEconomy          in litres/hour
//  - InstantaneousFuelEconomy in litres/hour
bool ParseN2kPGN127497(std::vector<unsigned char> &v, unsigned char &EngineInstance, double &TripFuelUsed,
                     double &FuelRateAverage,
                     double &FuelRateEconomy, double &InstantaneousFuelEconomy);



//-----------------------------------------------------------------------------
//  Environmental information parsing
//-----------------------------------------------------------------------------

//*****************************************************************************
// Wind Speed
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and wind at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - WindSpeed             Measured wind speed in m/s
//  - WindAngle             Measured wind angle in radians. If you have value in degrees, use function DegToRad(myval) in call.
//  - WindReference         Wind reference, see definition of tN2kWindReference
bool ParseN2kPGN130306(std::vector<unsigned char> &v, unsigned char &SID, double &WindSpeed, double &WindAngle, tN2kWindReference &WindReference);

//*****************************************************************************
// Outside Environmental parameters
// Input:
//  - SID                   Sequence ID.
//  - WaterTemperature      Water temperature in K. Use function CToKelvin, if you want to use °C.
//  - OutsideAmbientAirTemperature      Outside ambient temperature in K. Use function CToKelvin, if you want to use °C.
//  - AtmosphericPressure   Atmospheric pressure in Pascals. Use function mBarToPascal, if you like to use mBar
bool ParseN2kPGN130310(std::vector<unsigned char> &v, unsigned char &SID, double &WaterTemperature,
                     double &OutsideAmbientAirTemperature, double &AtmosphericPressure);

//*****************************************************************************
// Environmental parameters
// Note that in PGN 130311 TempInstance is as TempSource in PGN 130312. I do not know why this
// renaming is confusing.
// Pressure has to be in pascal. Use function mBarToPascal, if you like to use mBar
// Input:
//  - SID                   Sequence ID.
//  - TempSource            see tN2kTempSource
//  - Temperature           Temperature in K. Use function CToKelvin, if you want to use °C.
//  - HumiditySource        see tN2kHumiditySource.
//  - Humidity              Humidity in %
//  - AtmosphericPressure   Atmospheric pressure in Pascals. Use function mBarToPascal, if you like to use mBar
bool ParseN2kPGN130311(std::vector<unsigned char> &v, unsigned char &SID, tN2kTempSource &TempSource, double &Temperature,
                     tN2kHumiditySource &HumiditySource, double &Humidity, double &AtmosphericPressure);

//*****************************************************************************
// Temperature
// Temperatures should be in Kelvins
// Input:
//  - SID                   Sequence ID.
//  - TempInstance          This should be unic at least on one device. May be best to have it unic over all devices sending this PGN.
//  - TempSource            see tN2kTempSource
//  - ActualTemperature     Temperature in K. Use function CToKelvin, if you want to use °C.
//  - SetTemperature        Set temperature in K. Use function CToKelvin, if you want to use °C. This is meaningfull for temperatures,
//                          which can be controlled like cabin, freezer, refridgeration temperature. God can use value for this for
//                          outside and sea temperature values.
bool ParseN2kPGN130312(std::vector<unsigned char> &v, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature);

//*****************************************************************************
// Humidity
// Humidity should be a percent
// Input:
//  - SID                   Sequence ID.
//  - HumidityInstance      This should be unic at least on one device. May be best to have it unic over all devices sending this PGN.
//  - HumiditySource        see tN2kHumiditySource
//  - Humidity              Humidity in percent
bool ParseN2kPGN130313(std::vector<unsigned char> &v, unsigned char &SID, unsigned char &HumidityInstance,
                       tN2kHumiditySource &HumiditySource, double &ActualHumidity, double &SetHumidity);

//*****************************************************************************
// Pressure
// Pressures should be in Pascals
// Input:
//  - SID                   Sequence ID.
//  - PressureInstance      This should be unic at least on one device. May be best to have it unic over all devices sending this PGN.
//  - PressureSource        see tN2kPressureSource
//  - Pressure              Pressure in Pascals. Use function mBarToPascal, if you like to use mBar
bool ParseN2kPGN130314(std::vector<unsigned char> &v, unsigned char &SID, unsigned char &PressureInstance,
                       tN2kPressureSource &PressureSource, double &Pressure);

//*****************************************************************************
// Temperature
// Temperatures should be in Kelvins
// Input:
//  - SID                   Sequence ID.
//  - TempInstance          This should be unic at least on one device. May be best to have it unic over all devices sending this PGN.
//  - TempSource            see tN2kTempSource
//  - ActualTemperature     Temperature in K. Use function CToKelvin, if you want to use °C.
//  - SetTemperature        Set temperature in K. Use function CToKelvin, if you want to use °C. This is meaningfull for temperatures,
//                          which can be controlled like cabin, freezer, refridgeration temperature. God can use value for this for
//                          outside and sea temperature values.
bool ParseN2kPGN130316(std::vector<unsigned char> &v, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature);

#if 0
//*****************************************************************************
// Meteorlogical Station Data
// Input:
//  - SystemDate            Days since 1970-01-01
//  - SystemTime            seconds since midnight
//  - Latitude              The latitude of the current waypoint
//  - Longitude             The longitude of the current waypoint
//  - WindSpeed             Measured wind speed in m/s
//  - WindDirection         Measured wind direction in radians. If you have value in degrees, use function DegToRad(myval) in call.
//  - WindReference         Wind reference, see definition of tN2kWindReference
//  - WindGusts             Measured wind gusts speed in m/s
//  - AtmosphericPressure   Atmospheric pressure in Pascals. Use function mBarToPascal, if you like to use mBar
//  - OutsideAmbientAirTemperature      Outside ambient temperature in K. Use function CToKelvin, if you want to use °C.
//  - StationID             Identifier of the transmitting weather station. (15 bytes max)
//  - StationName           Friendly name of the transmitting weather station. (50 bytes max)
//
//
//*****************************************************************************
struct tN2kMeteorlogicalStationData {
  tN2kAISMode Mode;
  uint16_t SystemDate;
  double SystemTime;
  double Latitude;
  double Longitude;
  double WindSpeed;
  double WindDirection;
  tN2kWindReference WindReference;
  double WindGusts;
  double AtmosphericPressure;
  double OutsideAmbientAirTemperature;
  char StationID[15 + 1];
  char StationName[50 + 1];

  tN2kMeteorlogicalStationData():
    Mode(N2kaismode_Autonomous),
    SystemDate(N2kUInt16NA),
    SystemTime(N2kDoubleNA),
    Latitude(N2kDoubleNA),
    Longitude(N2kDoubleNA),
    WindSpeed(N2kDoubleNA),
    WindDirection(N2kDoubleNA),
    WindReference(N2kWind_Unavailable),
    WindGusts(N2kDoubleNA),
    AtmosphericPressure(N2kDoubleNA),
    OutsideAmbientAirTemperature(N2kDoubleNA) {
      StationID[0] = 0;
      StationName[0] = 0;
    }

  void SetStationID(const char *id) {
    strncpy(StationID, id, sizeof(StationID));
    StationID[sizeof(StationID) - 1] = 0;
  }

  void SetStationName(const char *name) {
    strncpy(StationName, name, sizeof(StationName));
    StationName[sizeof(StationName) - 1] = 0;
  }
};

bool ParseN2kPGN130323(std::vector<unsigned char> &v, tN2kMeteorlogicalStationData &N2kData);
#endif

//-----------------------------------------------------------------------------
//  Ship/System utility information parsing
//-----------------------------------------------------------------------------



//*****************************************************************************
// System date/time
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - SystemDate            Days since 1970-01-01
//  - SystemTime            seconds since midnight
//  - TimeSource            see tN2kTimeSource
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
bool ParseN2kPGN126992(std::vector<unsigned char> &v, unsigned char &SID, uint16_t &SystemDate,
                     double &SystemTime, tN2kTimeSource &TimeSource);

//*****************************************************************************
// Date,Time & Local offset  ( see also PGN 126992 )
// Input:
//  - DaysSince1970         Days since 1.1.1970. You can find sample how to convert e.g. NMEA0183 date info to this on my NMEA0183 library on
//                          NMEA0183Messages.cpp on function NMEA0183ParseRMC_nc
//  - Time                  Seconds since midnight
//  - Local offset          Local offset in minutes
bool ParseN2kPGN129033(std::vector<unsigned char> &v, uint16_t &DaysSince1970, double &SecondsSinceMidnight, int16_t &LocalOffset);




//*****************************************************************************
// Fluid level
// Input:
//  - Instance              Tank instance. Different devices handles this a bit differently. So it is best to have instance unique over
//                          all devices on the bus.
//  - FluidType             Defines type of fluid. See definition of tN2kFluidType
//  - Level                 Tank level in % of full tank.
//  - Capacity              Tank Capacity in litres
bool ParseN2kPGN127505(std::vector<unsigned char> &v, unsigned char &Instance, tN2kFluidType &FluidType, double &Level, double &Capacity);

//*****************************************************************************
// DC Detailed Status
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - DCInstance            DC instance.
//  - DCType                Defines type of DC source. See definition of tN2kDCType
//  - StateOfCharge         % of charge
//  - StateOfHealth         % of heath
//  - TimeRemaining         Time remaining in seconds
//  - RippleVoltage         DC output voltage ripple in V
//  - Capacity              Battery capacity in coulombs
bool ParseN2kPGN127506(std::vector<unsigned char> &v, unsigned char &SID, unsigned char &DCInstance, tN2kDCType &DCType,
                     unsigned char &StateOfCharge, unsigned char &StateOfHealth, double &TimeRemaining, double &RippleVoltage, double &Capacity);

//*****************************************************************************
// Charger Status
// Input:
//  - Instance                     ChargerInstance.
//  - BatteryInstance              BatteryInstance.
//  - Operating State              see. tN2kChargeState
//  - Charger Mode                 see. tN2kChargerMode
//  - Charger Enable/Disable       boolean
//  - Equalization Pending         boolean
//  - Equalization Time Remaining  double seconds
//
bool ParseN2kPGN127507(std::vector<unsigned char> &v, unsigned char &Instance, unsigned char &BatteryInstance,
                     tN2kChargeState &ChargeState, tN2kChargerMode &ChargerMode,
                     tN2kOnOff &Enabled, tN2kOnOff &EqualizationPending, double &EqualizationTimeRemaining);

//*****************************************************************************
// Battery Status
// Input:
//  - BatteryInstance       BatteryInstance.
//  - BatteryVoltage        Battery voltage in V
//  - BatteryCurrent        Current in A
//  - BatteryTemperature    Battery temperature in K. Use function CToKelvin, if you want to use °C.
//  - SID                   Sequence ID.
bool ParseN2kPGN127508(std::vector<unsigned char> &v, unsigned char &BatteryInstance, double &BatteryVoltage, double &BatteryCurrent,
                     double &BatteryTemperature, unsigned char &SID);


//*****************************************************************************
// Battery Configuration Status
// Note this has not yet confirmed to be right. Specially Peukert Exponent can have in
// this configuration values from 1 to 1.504. And I expect on code that I have to send
// value PeukertExponent-1 to the bus.
// Input:
//  - BatteryInstance       BatteryInstance.
//  - BatType               Type of battery. See definition of tN2kBatType
//  - SupportsEqual         Supports equalization. See definition of tN2kBatEqSupport
//  - BatNominalVoltage     Battery nominal voltage. See definition of tN2kBatNomVolt
//  - BatChemistry          Battery See definition of tN2kBatChem
//  - BatCapacity           Battery capacity in Coulombs. Use AhToCoulombs, if you have your value in Ah.
//  - BatTemperatureCoeff   Battery temperature coefficient in %
//  - PeukertExponent       Peukert Exponent
//  - ChargeEfficiencyFactor Charge efficiency factor
bool ParseN2kPGN127513(std::vector<unsigned char> &v, unsigned char &BatInstance, tN2kBatType &BatType, tN2kBatEqSupport &SupportsEqual,
                     tN2kBatNomVolt &BatNominalVoltage, tN2kBatChem &BatChemistry, double &BatCapacity, int8_t &BatTemperatureCoefficient,
                    double &PeukertExponent, int8_t &ChargeEfficiencyFactor);


//*****************************************************************************
// PGN128776 - Anchor Windlass Control Status
//
// Eg: SetN2kPGN128776(N2kMsg, SID, WindlassIdentifier, WindlassDirectionControl, SpeedControl);
// -- SID - message sequence id - use to tie together windlass status msgs
// -- WindlassIdentifier - instance of the windlass being reported
// -- WindlassDirectionControl - see tN2kWindlassDirectionControl
// -- SpeedControl - Single speed 0=off, 1-100=on; Dual speed: 0=off, 1-49=slow, 50-100=fast; Proporional speed: 0=off, 1-100=speed
// -- SpeedControlType (optional) - see tN2kSpeedType
// -- AnchorDockingControl (optional) - see tN2kGenericStatusPair
// -- PowerEnable (optional) - see tN2kDGenericStatusPair
// -- MechanicalLock (optional) - see tN2kDGenericStatusPair
// -- DeckAndAnchorWash (optional) - see tN2kDGenericStatusPair
// -- AnchorLight (optional) - see tN2kDGenericStatusPair
// -- CommandTimeout (optional) - range 0.25 ... 1.26 seconds
// -- WindlassControlEvents (optional) - see tN2kWindlassControlEvents


bool ParseN2kPGN128776(
  std::vector<unsigned char> &v,
  unsigned char &SID,
  unsigned char &WindlassIdentifier,
  tN2kWindlassDirectionControl &WindlassDirectionControl,
  unsigned char &SpeedControl,
  tN2kSpeedType &SpeedControlType,
  tN2kGenericStatusPair &AnchorDockingControl,
  tN2kGenericStatusPair &PowerEnable,
  tN2kGenericStatusPair &MechanicalLock,
  tN2kGenericStatusPair &DeckAndAnchorWash,
  tN2kGenericStatusPair &AnchorLight,
  double &CommandTimeout,
  tN2kWindlassControlEvents &WindlassControlEvents
);

//*****************************************************************************
// PGN128777 - Anchor Windlass Operating Status
//
// Eg: SetN2kPGN128777(N2kMsg, SID, WindlassIdentifier, RodeCounterValue, WindlassLineSpeed);
// -- SID - message sequence id - use to tie together windlass status msgs
// -- WindlassIdentifier - instance of the windlass being reported
// -- RodeCounterValue - amount of rode deployed in metres
// -- WindlassLineSpeed (optional) - deployment speed in metres per second
// -- WindlassMotionStatus (optional) - see tN2kWindlassMotionStates
// -- RodeTypeStatus (optional) - see tN2kRodeTypeStates
// -- AnchorDockingStatus (optional) - see tN2kAnchorDockingStates
// -- WindlassOperatingEvents (optional) -- see tN2kWindlassOperatingEvents

bool ParseN2kPGN128777(
  std::vector<unsigned char> &v,
  unsigned char &SID,
  unsigned char &WindlassIdentifier,
  double &RodeCounterValue,
  double &WindlassLineSpeed,
  tN2kWindlassMotionStates &WindlassMotionStatus,
  tN2kRodeTypeStates &RodeTypeStatus,
  tN2kAnchorDockingStates &AnchorDockingStatus,
  tN2kWindlassOperatingEvents &WindlassOperatingEvents
);

//*****************************************************************************
// PGN128778 - Anchor Windlass Monitoring Status
//
// Eg: SetN2kPGN128778(N2kMsg, SID, WindlassIdentifier, TotalMotorTime);
// -- SID - message sequence id - use to tie together windlass status msgs
// -- WindlassIdentifier - instance of the windlass being reported
// -- TotalMotorTime - total run time of the motor in seconds
// -- ControllerVoltage (optional) - voltage in Volts
// -- MotorCurrent (optional) - current in Amperes
// -- WindlassMonitoringEvents (optional) - see tN2kWindlassMonitoringEvents

bool ParseN2kPGN128778(
  std::vector<unsigned char> &v,
  unsigned char &SID,
  unsigned char &WindlassIdentifier,
  double &TotalMotorTime,
  double &ControllerVoltage,
  double &MotorCurrent,
  tN2kWindlassMonitoringEvents &WindlassMonitoringEvents
);



//*****************************************************************************
// Small Craft Status (Trim Tab Position)
// Trim tab position is a percentage 0 to 100% where 0 is fully retracted and 100 is fully extended
// Input:
//  - PortTrimTab           Port trim tab position
//  - StbdTrimTab           Starboard trim tab position

bool ParseN2kPGN130576(std::vector<unsigned char> &v, int8_t &PortTrimTab, int8_t &StbdTrimTab);

#endif  //guard


