/*
N2kMessages.h

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


This is collection of functions for handling NMEA2000 bus messages.
Library contains functions to e.g. create message named with PGN like SetN2kPGN129025 and
inline alias for them like SetN2kLatLonRapid.

Each Setxxx funtion sets related message PGN and its default priority. So if you want to change
priority on function, you have to do it after Setxxx call.

If you do not send anything to NMEA2000 bus, you do not need this library. Funtions for
BUS handling PGN:s like 60928 "ISO Address Claim" has been defined in bus device library
NMEA2000.h
*/

#ifndef _N2kMessages_H_
#define _N2kMessages_H_

#include "N2kMsg.h"
#include "N2kTypes.h"
#include <string.h>
#include <stdint.h>

inline double RadToDeg(double v) { return N2kIsNA(v)?v:v*180.0/3.1415926535897932384626433832795L; }
inline double DegToRad(double v) { return N2kIsNA(v)?v:v/180.0*3.1415926535897932384626433832795L; }
inline double CToKelvin(double v) { return N2kIsNA(v)?v:v+273.15L; }
inline double KelvinToC(double v) { return N2kIsNA(v)?v:v-273.15L; }
inline double FToKelvin(double v) { return N2kIsNA(v)?v:(v-32)*5.0/9.0+273.15; }
inline double KelvinToF(double v) { return N2kIsNA(v)?v:(v-273.15)*9.0/5.0+32; }
inline double mBarToPascal(double v) { return N2kIsNA(v)?v:v*100L; }
inline double PascalTomBar(double v) { return N2kIsNA(v)?v:v/100L; }
inline double hPAToPascal(double v) { return N2kIsNA(v)?v:v*100L; }
inline double PascalTohPA(double v) { return N2kIsNA(v)?v:v/100L; }
inline double AhToCoulomb(double v) { return N2kIsNA(v)?v:v*3600L; }
inline double CoulombToAh(double v) { return N2kIsNA(v)?v:v/3600L; }
inline double hToSeconds(double v) { return N2kIsNA(v)?v:v*3600L; }
inline double SecondsToh(double v) { return N2kIsNA(v)?v:v/3600L; }
inline double msToKnots(double v) { return N2kIsNA(v)?v:v*1.9438444924406047516198704103672L; } // 3600L/1852.0L
inline double KnotsToms(double v) { return N2kIsNA(v)?v:v*0.51444444444444444444444444444444L; } // 1852L/3600.0L

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
void SetN2kPGN126992(tN2kMsg &N2kMsg, unsigned char SID, uint16_t SystemDate,
                     double SystemTime, tN2kTimeSource TimeSource=N2ktimes_GPS);

inline void SetN2kSystemTime(tN2kMsg &N2kMsg, unsigned char SID, uint16_t SystemDate,
                     double SystemTime, tN2kTimeSource TimeSource=N2ktimes_GPS) {
  SetN2kPGN126992(N2kMsg,SID,SystemDate,SystemTime,TimeSource);
}

bool ParseN2kPGN126992(const tN2kMsg &N2kMsg, unsigned char &SID, uint16_t &SystemDate,
                     double &SystemTime, tN2kTimeSource &TimeSource);
inline bool ParseN2kSystemTime(const tN2kMsg &N2kMsg, unsigned char &SID, uint16_t &SystemDate,
                     double &SystemTime, tN2kTimeSource &TimeSource) {
  return ParseN2kPGN126992(N2kMsg,SID,SystemDate,SystemTime,TimeSource);
}

//*****************************************************************************
// AIS Safety Related Broadcast Message
// Input:
//  - MessageID                        Message type
//  - Repeat                           Repeat indicator
//  - SourceID                         MMSI
//  - tN2kAISTransceiverInformation
//  - SafetyRelatedText
// Output:
//  - N2kMsg                           NMEA2000 message ready to be send.
void SetN2kPGN129802(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t SourceID,
      tN2kAISTransceiverInformation AISTransceiverInformation, char * SafetyRelatedText);

inline void SetN2kAISSafetyRelatedBroadcastMsg(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t SourceID,
      tN2kAISTransceiverInformation AISTransceiverInformation, char * SafetyRelatedText) {
   return SetN2kPGN129802(N2kMsg, MessageID, Repeat, SourceID, AISTransceiverInformation, SafetyRelatedText);
}

bool ParseN2kPGN129802(tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &SourceID,
      tN2kAISTransceiverInformation &AISTransceiverInformation, char * SafetyRelatedText, size_t &SafetyRelatedTextMaxSize);

inline bool ParseN2kAISSafetyRelatedBroadcastMsg(tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &SourceID,
      tN2kAISTransceiverInformation &AISTransceiverInformation, char * SafetyRelatedText, size_t &SafetyRelatedTextMaxSize) {
   return ParseN2kPGN129802(N2kMsg, MessageID, Repeat, SourceID, AISTransceiverInformation, SafetyRelatedText, SafetyRelatedTextMaxSize);
}

//*****************************************************************************
// Man Overboard Notification
// Input:
//  - SID                     Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                            to indicate that they are measured at same time.
//  - MobEmitterId            Identifier for each MOB emitter, unique to the vessel
//  - MOBStatus               MOB Status
//  - ActivationTime          Time of day (UTC) when MOB was activated
//  - PositionSource          Position Source
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
void SetN2kPGN127233(tN2kMsg &N2kMsg,
      unsigned char SID,
      uint32_t MobEmitterId,
      tN2kMOBStatus MOBStatus,
      double ActivationTime,
      tN2kMOBPositionSource PositionSource,
      uint16_t PositionDate,
      double PositionTime,
      double Latitude,
      double Longitude,
      tN2kHeadingReference COGReference,
      double COG,
      double SOG,
      uint32_t MMSI,
      tN2kMOBEmitterBatteryStatus MOBEmitterBatteryStatus);

inline void SetN2kMOBNotification(tN2kMsg &N2kMsg,
      unsigned char SID,
      uint32_t MobEmitterId,
      tN2kMOBStatus MOBStatus,
      double ActivationTime,
      tN2kMOBPositionSource PositionSource,
      uint16_t PositionDate,
      double PositionTime,
      double Latitude,
      double Longitude,
      tN2kHeadingReference COGReference,
      double COG,
      double SOG,
      uint32_t MMSI,
      tN2kMOBEmitterBatteryStatus MOBEmitterBatteryStatus) {
  SetN2kPGN127233(N2kMsg,SID,MobEmitterId,MOBStatus,ActivationTime,PositionSource,PositionDate,PositionTime,Latitude,Longitude,COGReference,COG,SOG,MMSI,MOBEmitterBatteryStatus);
}

bool ParseN2kPGN127233(const tN2kMsg &N2kMsg,
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

inline bool ParseN2kMOBNotification(const tN2kMsg &N2kMsg,
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
      tN2kMOBEmitterBatteryStatus &MOBEmitterBatteryStatus) {
  return ParseN2kPGN127233(N2kMsg,SID,MobEmitterId,MOBStatus,ActivationTime,PositionSource,PositionDate,PositionTime,Latitude,Longitude,COGReference,COG,SOG,MMSI,MOBEmitterBatteryStatus);
}

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
// Output:
//  - N2kMsg                     NMEA2000 message ready to be send.
void SetN2kPGN127237(tN2kMsg &N2kMsg,
      tN2kOnOff RudderLimitExceeded,
      tN2kOnOff OffHeadingLimitExceeded,
      tN2kOnOff OffTrackLimitExceeded,
      tN2kOnOff Override,
      tN2kSteeringMode SteeringMode,
      tN2kTurnMode TurnMode,
      tN2kHeadingReference HeadingReference,
      tN2kRudderDirectionOrder CommandedRudderDirection,
      double CommandedRudderAngle,
      double HeadingToSteerCourse,
      double Track,
      double RudderLimit,
      double OffHeadingLimit,
      double RadiusOfTurnOrder,
      double RateOfTurnOrder,
      double OffTrackLimit,
      double VesselHeading);

inline void SetN2kHeadingTrackControl(tN2kMsg &N2kMsg,
      tN2kOnOff RudderLimitExceeded,
      tN2kOnOff OffHeadingLimitExceeded,
      tN2kOnOff OffTrackLimitExceeded,
      tN2kOnOff Override,
      tN2kSteeringMode SteeringMode,
      tN2kTurnMode TurnMode,
      tN2kHeadingReference HeadingReference,
      tN2kRudderDirectionOrder CommandedRudderDirection,
      double CommandedRudderAngle,
      double HeadingToSteerCourse,
      double Track,
      double RudderLimit,
      double OffHeadingLimit,
      double RadiusOfTurnOrder,
      double RateOfTurnOrder,
      double OffTrackLimit,
      double VesselHeading) {
  SetN2kPGN127237(N2kMsg, RudderLimitExceeded,OffHeadingLimitExceeded,OffTrackLimitExceeded,Override,SteeringMode,TurnMode,
         HeadingReference,CommandedRudderDirection,CommandedRudderAngle,HeadingToSteerCourse,Track,RudderLimit,OffHeadingLimit,
         RadiusOfTurnOrder,RateOfTurnOrder,OffTrackLimit,VesselHeading);
}

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

inline bool ParseN2kHeadingTrackControl(const tN2kMsg &N2kMsg,
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
      double &VesselHeading)
{
  return ParseN2kPGN127237(N2kMsg,RudderLimitExceeded,OffHeadingLimitExceeded,OffTrackLimitExceeded,Override,SteeringMode,
         TurnMode,HeadingReference,CommandedRudderDirection, CommandedRudderAngle,HeadingToSteerCourse,Track,RudderLimit,
         OffHeadingLimit,RadiusOfTurnOrder,RateOfTurnOrder,OffTrackLimit,VesselHeading);
}

//*****************************************************************************
// Rudder
// Input:
// - RudderPosition         Current rudder postion in radians.
// - Instance               Rudder instance.
// - RudderDirectionOrder   See tN2kRudderDirectionOrder. Direction, where rudder should be turned.
// - AngleOrder             In radians angle where rudder should be turned.
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127245(tN2kMsg &N2kMsg, double RudderPosition, unsigned char Instance=0,
                     tN2kRudderDirectionOrder RudderDirectionOrder=N2kRDO_NoDirectionOrder, double AngleOrder=N2kDoubleNA);

inline void SetN2kRudder(tN2kMsg &N2kMsg, double RudderPosition, unsigned char Instance=0,
                     tN2kRudderDirectionOrder RudderDirectionOrder=N2kRDO_NoDirectionOrder, double AngleOrder=N2kDoubleNA) {
  SetN2kPGN127245(N2kMsg,RudderPosition,Instance,RudderDirectionOrder,AngleOrder);
}

bool ParseN2kPGN127245(const tN2kMsg &N2kMsg, double &RudderPosition, unsigned char &Instance,
                     tN2kRudderDirectionOrder &RudderDirectionOrder, double &AngleOrder);

inline bool ParseN2kRudder(const tN2kMsg &N2kMsg, double &RudderPosition, unsigned char &Instance,
                     tN2kRudderDirectionOrder &RudderDirectionOrder, double &AngleOrder) {
  return ParseN2kPGN127245(N2kMsg,RudderPosition,Instance,RudderDirectionOrder,AngleOrder);
}

inline bool ParseN2kRudder(const tN2kMsg &N2kMsg, double &RudderPosition) {
  tN2kRudderDirectionOrder RudderDirectionOrder;
  double AngleOrder;
  unsigned char Instance;
  return ParseN2kPGN127245(N2kMsg,RudderPosition,Instance,RudderDirectionOrder,AngleOrder);
}

//*****************************************************************************
// Vessel Heading
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Heading               Heading in radians
//  - Deviation             Magnetic deviation in radians. Use N2kDoubleNA for undefined value.
//  - Variation             Magnetic variation in radians. Use N2kDoubleNA for undefined value.
//  - ref                   Heading reference. See definition of tN2kHeadingReference.
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127250(tN2kMsg &N2kMsg, unsigned char SID, double Heading, double Deviation, double Variation, tN2kHeadingReference ref);

inline void SetN2kTrueHeading(tN2kMsg &N2kMsg, unsigned char SID, double Heading) {
  SetN2kPGN127250(N2kMsg,SID,Heading,N2kDoubleNA,N2kDoubleNA,N2khr_true);
}
inline void SetN2kMagneticHeading(tN2kMsg &N2kMsg, unsigned char SID, double Heading, double Deviation=N2kDoubleNA, double Variation=N2kDoubleNA) {
  SetN2kPGN127250(N2kMsg,SID,Heading,Deviation,Variation,N2khr_magnetic);
}

bool ParseN2kPGN127250(const tN2kMsg &N2kMsg, unsigned char &SID, double &Heading, double &Deviation, double &Variation, tN2kHeadingReference &ref);
inline bool ParseN2kHeading(const tN2kMsg &N2kMsg, unsigned char &SID, double &Heading, double &Deviation, double &Variation, tN2kHeadingReference &ref) {
  return ParseN2kPGN127250(N2kMsg,SID,Heading,Deviation,Variation,ref);
}

//*****************************************************************************
// Rate of Turn
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Rate of turn          Change in heading in radians per second
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127251(tN2kMsg &N2kMsg, unsigned char SID, double RateOfTurn);

inline void SetN2kRateOfTurn(tN2kMsg &N2kMsg, unsigned char SID, double RateOfTurn) {
  SetN2kPGN127251(N2kMsg,SID,RateOfTurn);
}

bool ParseN2kPGN127251(const tN2kMsg &N2kMsg, unsigned char &SID, double &RateOfTurn);
inline bool ParseN2kRateOfTurn(const tN2kMsg &N2kMsg, unsigned char &SID, double &RateOfTurn) {
  return ParseN2kPGN127251(N2kMsg,SID,RateOfTurn);
}

//*****************************************************************************
// Attitude
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Yaw                   Heading in radians.
//  - Pitch                 Pitch in radians. Positive, when your bow rises.
//  - Roll                  Roll in radians. Positive, when tilted right.
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127257(tN2kMsg &N2kMsg, unsigned char SID, double Yaw, double Pitch, double Roll);

inline void SetN2kAttitude(tN2kMsg &N2kMsg, unsigned char SID, double Yaw, double Pitch, double Roll) {
  SetN2kPGN127257(N2kMsg,SID, Yaw, Pitch, Roll);
}

bool ParseN2kPGN127257(const tN2kMsg &N2kMsg, unsigned char &SID, double &Yaw, double &Pitch, double &Roll);
inline bool ParseN2kAttitude(const tN2kMsg &N2kMsg, unsigned char &SID, double &Yaw, double &Pitch, double &Roll) {
  return ParseN2kPGN127257(N2kMsg,SID, Yaw, Pitch, Roll);
}

//*****************************************************************************
// Magnetic Variation
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Source                How was the variation value generated
//  - DaysSince1970         Days since January 1, 1970
//  - Variation             Magnetic variation/declination in radians
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127258(tN2kMsg &N2kMsg, unsigned char SID, tN2kMagneticVariation Source, uint16_t DaysSince1970, double Variation);

inline void SetN2kMagneticVariation(tN2kMsg &N2kMsg, unsigned char SID, tN2kMagneticVariation Source, uint16_t DaysSince1970, double Variation) {
  SetN2kPGN127258(N2kMsg, SID, Source, DaysSince1970, Variation);
}

bool ParseN2kPGN127258(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kMagneticVariation &Source, uint16_t &DaysSince1970, double &Variation);

inline bool ParseN2kMagneticVariation(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kMagneticVariation &Source, uint16_t &DaysSince1970, double &Variation) {
  return ParseN2kPGN127258(N2kMsg, SID, Source, DaysSince1970, Variation);
}

//*****************************************************************************
// Engine parameters rapid
// Input:
//  - EngineInstance        Engine instance.
//  - EngineSpeed           RPM (Revolutions Per Minute)
//  - EngineBoostPressure   in Pascal
//  - EngineTiltTrim        in %
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127488(tN2kMsg &N2kMsg, unsigned char EngineInstance, double EngineSpeed,
                     double EngineBoostPressure=N2kDoubleNA, int8_t EngineTiltTrim=N2kInt8NA);

inline void SetN2kEngineParamRapid(tN2kMsg &N2kMsg, unsigned char EngineInstance, double EngineSpeed,
                      double EngineBoostPressure=N2kDoubleNA, int8_t EngineTiltTrim=N2kInt8NA) {
  SetN2kPGN127488(N2kMsg,EngineInstance,EngineSpeed,EngineBoostPressure,EngineTiltTrim);
}

bool ParseN2kPGN127488(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineSpeed,
                     double &EngineBoostPressure, int8_t &EngineTiltTrim);
inline bool ParseN2kEngineParamRapid(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineSpeed,
                     double &EngineBoostPressure, int8_t &EngineTiltTrim) {
  return ParseN2kPGN127488(N2kMsg,EngineInstance,EngineSpeed,EngineBoostPressure,EngineTiltTrim);
}

#if 0  //DSR
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
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127489(tN2kMsg &N2kMsg, unsigned char EngineInstance, double EngineOilPress, double EngineOilTemp, double EngineCoolantTemp, double AltenatorVoltage,
                       double FuelRate, double EngineHours, double EngineCoolantPress=N2kDoubleNA, double EngineFuelPress=N2kDoubleNA,
                       int8_t EngineLoad=N2kInt8NA, int8_t EngineTorque=N2kInt8NA,
                       tN2kEngineDiscreteStatus1 Status1=0, tN2kEngineDiscreteStatus2 Status2=0);

inline void SetN2kEngineDynamicParam(tN2kMsg &N2kMsg, unsigned char EngineInstance, double EngineOilPress, double EngineOilTemp, double EngineCoolantTemp, double AltenatorVoltage,
                       double FuelRate, double EngineHours, double EngineCoolantPress=N2kDoubleNA, double EngineFuelPress=N2kDoubleNA,
                       int8_t EngineLoad=N2kInt8NA, int8_t EngineTorque=N2kInt8NA,
                       tN2kEngineDiscreteStatus1 Status1=0, tN2kEngineDiscreteStatus2 Status2=0) {
  SetN2kPGN127489(N2kMsg,EngineInstance, EngineOilPress, EngineOilTemp, EngineCoolantTemp, AltenatorVoltage,
                       FuelRate, EngineHours, EngineCoolantPress, EngineFuelPress, EngineLoad, EngineTorque,
                       Status1,Status2);
}

inline void SetN2kPGN127489(tN2kMsg &N2kMsg, unsigned char EngineInstance, double EngineOilPress, double EngineOilTemp, double EngineCoolantTemp, double AltenatorVoltage,
                       double FuelRate, double EngineHours, double EngineCoolantPress=N2kDoubleNA, double EngineFuelPress=N2kDoubleNA,
                       int8_t EngineLoad=N2kInt8NA, int8_t EngineTorque=N2kInt8NA,
                       bool flagCheckEngine=false,       bool flagOverTemp=false,         bool flagLowOilPress=false,         bool flagLowOilLevel=false,
                       bool flagLowFuelPress=false,      bool flagLowSystemVoltage=false, bool flagLowCoolantLevel=false,     bool flagWaterFlow=false,
                       bool flagWaterInFuel=false,       bool flagChargeIndicator=false,  bool flagPreheatIndicator=false,    bool flagHighBoostPress=false,
                       bool flagRevLimitExceeded=false,  bool flagEgrSystem=false,        bool flagTPS=false,                 bool flagEmergencyStopMode=false,
                       bool flagWarning1=false,          bool flagWarning2=false,         bool flagPowerReduction=false,      bool flagMaintenanceNeeded=false,
                       bool flagEngineCommError=false,   bool flagSubThrottle=false,      bool flagNeutralStartProtect=false, bool flagEngineShuttingDown=false) {
  tN2kEngineDiscreteStatus1 Status1;
  tN2kEngineDiscreteStatus2 Status2;
  Status1.Bits.CheckEngine=flagCheckEngine;
  Status1.Bits.OverTemperature=flagOverTemp;
  Status1.Bits.LowOilPressure=flagLowOilPress;
  Status1.Bits.LowOilLevel=flagLowOilLevel;
  Status1.Bits.LowFuelPressure=flagLowFuelPress;
  Status1.Bits.LowSystemVoltage=flagLowSystemVoltage;
  Status1.Bits.LowCoolantLevel=flagLowCoolantLevel;
  Status1.Bits.WaterFlow=flagWaterFlow;
  Status1.Bits.WaterInFuel=flagWaterInFuel;
  Status1.Bits.ChargeIndicator=flagChargeIndicator;
  Status1.Bits.PreheatIndicator=flagPreheatIndicator;
  Status1.Bits.HighBoostPressure=flagHighBoostPress;
  Status1.Bits.RevLimitExceeded=flagRevLimitExceeded;
  Status1.Bits.EGRSystem=flagEgrSystem;
  Status1.Bits.ThrottlePositionSensor=flagTPS;
  Status1.Bits.EngineEmergencyStopMode=flagEmergencyStopMode;
  Status2.Bits.WarningLevel1=flagWarning1;
  Status2.Bits.WarningLevel2=flagWarning2;
  Status2.Bits.LowOiPowerReduction=flagPowerReduction;
  Status2.Bits.MaintenanceNeeded=flagMaintenanceNeeded;
  Status2.Bits.EngineCommError=flagEngineCommError;
  Status2.Bits.SubOrSecondaryThrottle=flagSubThrottle;
  Status2.Bits.NeutralStartProtect=flagNeutralStartProtect;
  Status2.Bits.EngineShuttingDown=flagEngineShuttingDown;

  SetN2kPGN127489(N2kMsg,EngineInstance, EngineOilPress, EngineOilTemp, EngineCoolantTemp, AltenatorVoltage,
                       FuelRate, EngineHours, EngineCoolantPress, EngineFuelPress, EngineLoad, EngineTorque,
                       Status1,Status2);
}
inline void SetN2kEngineDynamicParam(tN2kMsg &N2kMsg, unsigned char EngineInstance, double EngineOilPress, double EngineOilTemp, double EngineCoolantTemp, double AltenatorVoltage,
                       double FuelRate, double EngineHours, double EngineCoolantPress=N2kDoubleNA, double EngineFuelPress=N2kDoubleNA,
                       int8_t EngineLoad=N2kInt8NA, int8_t EngineTorque=N2kInt8NA,
                       bool flagCheckEngine=false,       bool flagOverTemp=false,         bool flagLowOilPress=false,         bool flagLowOilLevel=false,
                       bool flagLowFuelPress=false,      bool flagLowSystemVoltage=false, bool flagLowCoolantLevel=false,     bool flagWaterFlow=false,
                       bool flagWaterInFuel=false,       bool flagChargeIndicator=false,  bool flagPreheatIndicator=false,    bool flagHighBoostPress=false,
                       bool flagRevLimitExceeded=false,  bool flagEgrSystem=false,        bool flagTPS=false,                 bool flagEmergencyStopMode=false,
                       bool flagWarning1=false,          bool flagWarning2=false,         bool flagPowerReduction=false,      bool flagMaintenanceNeeded=false,
                       bool flagEngineCommError=false,   bool flagSubThrottle=false,      bool flagNeutralStartProtect=false, bool flagEngineShuttingDown=false) {
  SetN2kPGN127489(N2kMsg,EngineInstance, EngineOilPress, EngineOilTemp, EngineCoolantTemp, AltenatorVoltage,
                       FuelRate, EngineHours, EngineCoolantPress, EngineFuelPress, EngineLoad, EngineTorque,
                       flagCheckEngine, flagOverTemp, flagLowOilPress, flagLowOilLevel,
                       flagLowFuelPress, flagLowSystemVoltage, flagLowCoolantLevel, flagWaterFlow,
                       flagWaterInFuel, flagChargeIndicator, flagPreheatIndicator, flagHighBoostPress,
                       flagRevLimitExceeded, flagEgrSystem, flagTPS, flagEmergencyStopMode,
                       flagWarning1, flagWarning2, flagPowerReduction, flagMaintenanceNeeded,
                       flagEngineCommError, flagSubThrottle, flagNeutralStartProtect, flagEngineShuttingDown);
}

bool ParseN2kPGN127489(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineOilPress,
                      double &EngineOilTemp, double &EngineCoolantTemp, double &AltenatorVoltage,
                      double &FuelRate, double &EngineHours, double &EngineCoolantPress, double &EngineFuelPress,
                      int8_t &EngineLoad, int8_t &EngineTorque,
                      tN2kEngineDiscreteStatus1 &Status1, tN2kEngineDiscreteStatus2 &Status2);

inline bool ParseN2kPGN127489(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineOilPress,
                      double &EngineOilTemp, double &EngineCoolantTemp, double &AltenatorVoltage,
                      double &FuelRate, double &EngineHours, double &EngineCoolantPress, double &EngineFuelPress,
                      int8_t &EngineLoad, int8_t &EngineTorque) {
  tN2kEngineDiscreteStatus1 Status1;
  tN2kEngineDiscreteStatus2 Status2;
  return ParseN2kPGN127489(N2kMsg, EngineInstance, EngineOilPress,
                    EngineOilTemp, EngineCoolantTemp, AltenatorVoltage,
                    FuelRate, EngineHours,EngineCoolantPress, EngineFuelPress,
                    EngineLoad, EngineTorque,Status1,Status2);
}

inline bool ParseN2kEngineDynamicParam(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineOilPress,
                      double &EngineOilTemp, double &EngineCoolantTemp, double &AltenatorVoltage,
                      double &FuelRate, double &EngineHours, double &EngineCoolantPress, double &EngineFuelPress,
                      int8_t &EngineLoad, int8_t &EngineTorque) {
    return ParseN2kPGN127489(N2kMsg, EngineInstance, EngineOilPress,
                      EngineOilTemp, EngineCoolantTemp, AltenatorVoltage,
                      FuelRate, EngineHours,EngineCoolantPress, EngineFuelPress,
                      EngineLoad, EngineTorque);
}
inline bool ParseN2kEngineDynamicParam(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineOilPress,
                      double &EngineOilTemp, double &EngineCoolantTemp, double &AltenatorVoltage,
                      double &FuelRate, double &EngineHours, double &EngineCoolantPress, double &EngineFuelPress,
                      int8_t &EngineLoad, int8_t &EngineTorque,
                      tN2kEngineDiscreteStatus1 &Status1, tN2kEngineDiscreteStatus2 &Status2) {
    return ParseN2kPGN127489(N2kMsg, EngineInstance, EngineOilPress,
                      EngineOilTemp, EngineCoolantTemp, AltenatorVoltage,
                      FuelRate, EngineHours,EngineCoolantPress, EngineFuelPress,
                      EngineLoad, EngineTorque,
                      Status1, Status2);
}
inline bool ParseN2kEngineDynamicParam(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineOilPress,
                      double &EngineOilTemp, double &EngineCoolantTemp, double &AltenatorVoltage,
                      double &FuelRate, double &EngineHours) {
    double EngineCoolantPress, EngineFuelPress;
    int8_t EngineLoad, EngineTorque;
    tN2kEngineDiscreteStatus1 Status1;
    tN2kEngineDiscreteStatus2 Status2;
    return ParseN2kPGN127489(N2kMsg, EngineInstance, EngineOilPress,
                      EngineOilTemp, EngineCoolantTemp, AltenatorVoltage,
                      FuelRate, EngineHours,EngineCoolantPress, EngineFuelPress,
                      EngineLoad, EngineTorque,Status1,Status2);
}
#endif
//*****************************************************************************
// Transmission parameters, dynamic
// Input:
//  - EngineInstance        Engine instance.
//  - TransmissionGear      Selected transmission. See tN2kTransmissionGear
//  - OilPressure           in Pascal
//  - OilTemperature        in K
//  - EngineTiltTrim        in %
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127493(tN2kMsg &N2kMsg, unsigned char EngineInstance, tN2kTransmissionGear TransmissionGear,
                     double OilPressure, double OilTemperature, unsigned char DiscreteStatus1=0);

inline void SetN2kTransmissionParameters(tN2kMsg &N2kMsg, unsigned char EngineInstance, tN2kTransmissionGear TransmissionGear,
                     double OilPressure, double OilTemperature, unsigned char DiscreteStatus1=0) {
  SetN2kPGN127493(N2kMsg, EngineInstance, TransmissionGear, OilPressure, OilTemperature, DiscreteStatus1);
}

inline void SetN2kTransmissionParameters(tN2kMsg &N2kMsg, unsigned char EngineInstance, tN2kTransmissionGear TransmissionGear,
                     double OilPressure, double OilTemperature,
                     bool flagCheck,       bool flagOverTemp,         bool flagLowOilPressure=false,         bool flagLowOilLevel=false,
                     bool flagSailDrive=false) {
  unsigned char DiscreteStatus1=0;

  if (flagCheck) DiscreteStatus1          |= BIT(0);
  if (flagOverTemp) DiscreteStatus1       |= BIT(1);
  if (flagLowOilPressure) DiscreteStatus1 |= BIT(2);
  if (flagLowOilLevel) DiscreteStatus1    |= BIT(3);
  if (flagSailDrive) DiscreteStatus1      |= BIT(4);
  SetN2kPGN127493(N2kMsg, EngineInstance, TransmissionGear, OilPressure, OilTemperature,DiscreteStatus1);
}

bool ParseN2kPGN127493(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, tN2kTransmissionGear &TransmissionGear,
                     double &OilPressure, double &OilTemperature, unsigned char &DiscreteStatus1);
inline bool ParseN2kTransmissionParameters(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, tN2kTransmissionGear &TransmissionGear,
                     double &OilPressure, double &OilTemperature, unsigned char &DiscreteStatus1) {
  return ParseN2kPGN127493(N2kMsg, EngineInstance, TransmissionGear, OilPressure, OilTemperature, DiscreteStatus1);
}

inline bool ParseN2kTransmissionParameters(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, tN2kTransmissionGear &TransmissionGear,
                     double &OilPressure, double &OilTemperature,
                     bool &flagCheck,       bool &flagOverTemp,         bool &flagLowOilPressure,         bool &flagLowOilLevel,
                     bool &flagSailDrive) {
  unsigned char DiscreteStatus1;
  bool ret=ParseN2kPGN127493(N2kMsg, EngineInstance, TransmissionGear, OilPressure, OilTemperature, DiscreteStatus1);
  if (ret) {
    flagCheck          = ((DiscreteStatus1 & BIT(0))!=0);
    flagOverTemp       = ((DiscreteStatus1 & BIT(1))!=0);
    flagLowOilPressure = ((DiscreteStatus1 & BIT(2))!=0);
    flagLowOilLevel    = ((DiscreteStatus1 & BIT(3))!=0);
    flagSailDrive      = ((DiscreteStatus1 & BIT(4))!=0);
  }
  return ret;
}

//*****************************************************************************
// Trip Parameters, Engine
// Input:
//  - EngineInstance           Engine instance.
//  - TripFuelUsed             in litres
//  - FuelRateAverage          in litres/hour
//  - FuelRateEconomy          in litres/hour
//  - InstantaneousFuelEconomy in litres/hour
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127497(tN2kMsg &N2kMsg, unsigned char EngineInstance, double TripFuelUsed,
                     double FuelRateAverage,
                     double FuelRateEconomy=N2kDoubleNA, double InstantaneousFuelEconomy=N2kDoubleNA);

inline void SetN2kEngineTripParameters(tN2kMsg &N2kMsg, unsigned char EngineInstance, double TripFuelUsed,
                     double FuelRateAverage,
                     double FuelRateEconomy=N2kDoubleNA, double InstantaneousFuelEconomy=N2kDoubleNA) {
  SetN2kPGN127497(N2kMsg,EngineInstance,TripFuelUsed,FuelRateAverage,FuelRateEconomy,InstantaneousFuelEconomy);
}

bool ParseN2kPGN127497(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &TripFuelUsed,
                     double &FuelRateAverage,
                     double &FuelRateEconomy, double &InstantaneousFuelEconomy);
inline bool ParseN2kEngineTripParameters(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &TripFuelUsed,
                     double &FuelRateAverage,
                     double &FuelRateEconomy, double &InstantaneousFuelEconomy) {
  return ParseN2kPGN127497(N2kMsg,EngineInstance,TripFuelUsed,FuelRateAverage,FuelRateEconomy, InstantaneousFuelEconomy);
}

typedef uint64_t tN2kBinaryStatus;

//*****************************************************************************
// Returns single status of full binary bank status returned by ParseN2kPGN127501 or ParseN2kBinaryStatus.
//   Status		- Full bank status read by ParseN2kPGN127501 or ParseN2kBinaryStatus
//   ItemIndex	- Status item index 1-28
tN2kOnOff N2kGetStatusOnBinaryStatus(tN2kBinaryStatus BankStatus, uint8_t ItemIndex=1);

//*****************************************************************************
// Reset all single binary status values to not available
inline void N2kResetBinaryStatus(tN2kBinaryStatus &BankStatus) { BankStatus=0xffffffffffffffffULL; }

//*****************************************************************************
// Set single status to full binary bank status.
void N2kSetStatusBinaryOnStatus(tN2kBinaryStatus &BankStatus, tN2kOnOff ItemStatus, uint8_t ItemIndex=1);

//*****************************************************************************
// Binary status report
//  BankStatus        - Full bank status. Read single status by using N2kGetBinaryStatus
void SetN2kPGN127501(tN2kMsg &N2kMsg, unsigned char DeviceBankInstance, tN2kBinaryStatus BankStatus);

inline void SetN2kBinaryStatus(tN2kMsg &N2kMsg, unsigned char DeviceBankInstance, tN2kBinaryStatus BankStatus) {
	SetN2kPGN127501(N2kMsg,DeviceBankInstance,BankStatus);
}

//*****************************************************************************
// Binary status report
void SetN2kPGN127501(tN2kMsg &N2kMsg, unsigned char DeviceBankInstance
                      ,tN2kOnOff Status1
                      ,tN2kOnOff Status2=N2kOnOff_Unavailable
                      ,tN2kOnOff Status3=N2kOnOff_Unavailable
                      ,tN2kOnOff Status4=N2kOnOff_Unavailable
                    );

inline void SetN2kBinaryStatus(tN2kMsg &N2kMsg, unsigned char DeviceBankInstance
                      ,tN2kOnOff Status1
                      ,tN2kOnOff Status2=N2kOnOff_Unavailable
                      ,tN2kOnOff Status3=N2kOnOff_Unavailable
                      ,tN2kOnOff Status4=N2kOnOff_Unavailable
					) {
  SetN2kPGN127501(N2kMsg, DeviceBankInstance,Status1,Status2,Status3,Status4);
}

// Parse four first status of binary status report.
bool ParseN2kPGN127501(const tN2kMsg &N2kMsg, unsigned char &DeviceBankInstance
                      ,tN2kOnOff &Status1
                      ,tN2kOnOff &Status2
                      ,tN2kOnOff &Status3
                      ,tN2kOnOff &Status4
                    );
inline bool ParseN2kBinaryStatus(const tN2kMsg &N2kMsg, unsigned char &DeviceBankInstance
                      ,tN2kOnOff &Status1
                      ,tN2kOnOff &Status2
                      ,tN2kOnOff &Status3
                      ,tN2kOnOff &Status4
                    ) {
 return ParseN2kPGN127501(N2kMsg,DeviceBankInstance,Status1,Status2,Status3,Status4);
}

// Parse bank status of binary status report. Use N2kGetBinaryStatus to read specific status
bool ParseN2kPGN127501(const tN2kMsg &N2kMsg, unsigned char &DeviceBankInstance, tN2kBinaryStatus &BankStatus);

inline bool ParseN2kBinaryStatus(const tN2kMsg &N2kMsg, unsigned char &DeviceBankInstance, tN2kBinaryStatus &BankStatus) {
 return ParseN2kPGN127501(N2kMsg,DeviceBankInstance,BankStatus);
}

//*****************************************************************************
// Fluid level
// Input:
//  - Instance              Tank instance. Different devices handles this a bit differently. So it is best to have instance unique over
//                          all devices on the bus.
//  - FluidType             Defines type of fluid. See definition of tN2kFluidType
//  - Level                 Tank level in % of full tank.
//  - Capacity              Tank Capacity in litres
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN127505(tN2kMsg &N2kMsg, unsigned char Instance, tN2kFluidType FluidType, double Level, double Capacity);

inline void SetN2kFluidLevel(tN2kMsg &N2kMsg, unsigned char Instance, tN2kFluidType FluidType, double Level, double Capacity) {
  SetN2kPGN127505(N2kMsg, Instance, FluidType, Level, Capacity);
}

// Parse fluid level message
// Output:
//  - Instance              Tank instance.
//  - FluidType             Defines type of fluid. See definition of tN2kFluidType
//  - Level                 Tank level in % of full tank.
//  - Capacity              Tank Capacity in litres
bool ParseN2kPGN127505(const tN2kMsg &N2kMsg, unsigned char &Instance, tN2kFluidType &FluidType, double &Level, double &Capacity);

inline bool ParseN2kFluidLevel(const tN2kMsg &N2kMsg, unsigned char &Instance, tN2kFluidType &FluidType, double &Level, double &Capacity) {
  return ParseN2kPGN127505(N2kMsg, Instance, FluidType, Level, Capacity);
}

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
void SetN2kPGN127506(tN2kMsg &N2kMsg, unsigned char SID, unsigned char DCInstance, tN2kDCType DCType,
                     unsigned char StateOfCharge, unsigned char StateOfHealth, double TimeRemaining, double RippleVoltage=N2kDoubleNA, double Capacity=N2kDoubleNA);

inline void SetN2kDCStatus(tN2kMsg &N2kMsg, unsigned char SID, unsigned char DCInstance, tN2kDCType DCType,
                     unsigned char StateOfCharge, unsigned char StateOfHealth, double TimeRemaining, double RippleVoltage=N2kDoubleNA, double Capacity=N2kDoubleNA) {
  SetN2kPGN127506(N2kMsg,SID,DCInstance,DCType,StateOfCharge,StateOfHealth,TimeRemaining,RippleVoltage,Capacity);
}

bool ParseN2kPGN127506(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &DCInstance, tN2kDCType &DCType,
                     unsigned char &StateOfCharge, unsigned char &StateOfHealth, double &TimeRemaining, double &RippleVoltage, double &Capacity);

inline bool ParseN2kDCStatus(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &DCInstance, tN2kDCType &DCType,
                     unsigned char &StateOfCharge, unsigned char &StateOfHealth, double &TimeRemaining, double &RippleVoltage, double &Capacity) {
  return ParseN2kPGN127506(N2kMsg,SID,DCInstance,DCType,StateOfCharge,StateOfHealth,TimeRemaining,RippleVoltage, Capacity);
}

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
void SetN2kPGN127507(tN2kMsg &N2kMsg, unsigned char Instance, unsigned char BatteryInstance,
                     tN2kChargeState ChargeState, tN2kChargerMode ChargerMode=N2kCM_Standalone,
                     tN2kOnOff Enabled=N2kOnOff_On, tN2kOnOff EqualizationPending=N2kOnOff_Unavailable, double EqualizationTimeRemaining=N2kDoubleNA);

inline void SetN2kChargerStatus(tN2kMsg &N2kMsg, unsigned char Instance, unsigned char BatteryInstance,
                     tN2kChargeState ChargeState, tN2kChargerMode ChargerMode=N2kCM_Standalone,
                     tN2kOnOff Enabled=N2kOnOff_On, tN2kOnOff EqualizationPending=N2kOnOff_Unavailable, double EqualizationTimeRemaining=N2kDoubleNA) {
 SetN2kPGN127507(N2kMsg, Instance,BatteryInstance,ChargeState,ChargerMode,Enabled,EqualizationPending,EqualizationTimeRemaining);
}

bool ParseN2kPGN127507(tN2kMsg &N2kMsg, unsigned char &Instance, unsigned char &BatteryInstance,
                     tN2kChargeState &ChargeState, tN2kChargerMode &ChargerMode,
                     tN2kOnOff &Enabled, tN2kOnOff &EqualizationPending, double &EqualizationTimeRemaining);
inline bool ParseN2kChargerStatus(tN2kMsg &N2kMsg, unsigned char &Instance, unsigned char &BatteryInstance,
                     tN2kChargeState &ChargeState, tN2kChargerMode &ChargerMode,
                     tN2kOnOff &Enabled, tN2kOnOff &EqualizationPending, double &EqualizationTimeRemaining) {
 return ParseN2kPGN127507(N2kMsg, Instance,BatteryInstance,ChargeState,ChargerMode,Enabled,EqualizationPending,EqualizationTimeRemaining);
}

//*****************************************************************************
// Battery Status
// Input:
//  - BatteryInstance       BatteryInstance.
//  - BatteryVoltage        Battery voltage in V
//  - BatteryCurrent        Current in A
//  - BatteryTemperature    Battery temperature in K. Use function CToKelvin, if you want to use °C.
//  - SID                   Sequence ID.
void SetN2kPGN127508(tN2kMsg &N2kMsg, unsigned char BatteryInstance, double BatteryVoltage, double BatteryCurrent=N2kDoubleNA,
                     double BatteryTemperature=N2kDoubleNA, unsigned char SID=1);

inline void SetN2kDCBatStatus(tN2kMsg &N2kMsg, unsigned char BatteryInstance, double BatteryVoltage, double BatteryCurrent=N2kDoubleNA,
                     double BatteryTemperature=N2kDoubleNA, unsigned char SID=1) {
  SetN2kPGN127508(N2kMsg,BatteryInstance,BatteryVoltage,BatteryCurrent,BatteryTemperature,SID);
}

bool ParseN2kPGN127508(const tN2kMsg &N2kMsg, unsigned char &BatteryInstance, double &BatteryVoltage, double &BatteryCurrent,
                     double &BatteryTemperature, unsigned char &SID);
inline bool ParseN2kDCBatStatus(const tN2kMsg &N2kMsg, unsigned char &BatteryInstance, double &BatteryVoltage, double &BatteryCurrent,
                     double &BatteryTemperature, unsigned char &SID) {
  return ParseN2kPGN127508(N2kMsg, BatteryInstance, BatteryVoltage, BatteryCurrent, BatteryTemperature, SID);
}


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
void SetN2kPGN127513(tN2kMsg &N2kMsg, unsigned char BatInstance, tN2kBatType BatType, tN2kBatEqSupport SupportsEqual,
                     tN2kBatNomVolt BatNominalVoltage, tN2kBatChem BatChemistry, double BatCapacity, int8_t BatTemperatureCoefficient,
				double PeukertExponent, int8_t ChargeEfficiencyFactor);

inline void SetN2kBatConf(tN2kMsg &N2kMsg, unsigned char BatInstance, tN2kBatType BatType, tN2kBatEqSupport SupportsEqual,
                     tN2kBatNomVolt BatNominalVoltage, tN2kBatChem BatChemistry, double BatCapacity, int8_t BatTemperatureCoefficient,
				double PeukertExponent, int8_t ChargeEfficiencyFactor) {
   SetN2kPGN127513(N2kMsg,BatInstance,BatType,SupportsEqual,BatNominalVoltage,BatChemistry,BatCapacity,BatTemperatureCoefficient,
				PeukertExponent,ChargeEfficiencyFactor);
}

bool ParseN2kPGN127513(const tN2kMsg &N2kMsg, unsigned char &BatInstance, tN2kBatType &BatType, tN2kBatEqSupport &SupportsEqual,
                     tN2kBatNomVolt &BatNominalVoltage, tN2kBatChem &BatChemistry, double &BatCapacity, int8_t &BatTemperatureCoefficient,
				double &PeukertExponent, int8_t &ChargeEfficiencyFactor);


inline bool ParseN2kBatConf(const tN2kMsg &N2kMsg, unsigned char &BatInstance, tN2kBatType &BatType, tN2kBatEqSupport &SupportsEqual,
                     tN2kBatNomVolt &BatNominalVoltage, tN2kBatChem &BatChemistry, double &BatCapacity, int8_t &BatTemperatureCoefficient,
				double &PeukertExponent, int8_t &ChargeEfficiencyFactor) {
	return ParseN2kPGN127513(N2kMsg,BatInstance,BatType,SupportsEqual,BatNominalVoltage,BatChemistry,BatCapacity,BatTemperatureCoefficient,
				PeukertExponent,ChargeEfficiencyFactor);
}

//*****************************************************************************
// Leeway
// Input:
//  - SID            Sequence ID field
//  - Leeway         Nautical Leeway Angle, which is defined as the angle between the vessel’s heading (direction to which the
//                   vessel’s bow points) and its course (direction of its motion (track) through the water)
// Output:
//  - N2kMsg         NMEA2000 message ready to be send.
void SetN2kPGN128000(tN2kMsg &N2kMsg, unsigned char SID, double Leeway);

inline void SetN2kLeeway(tN2kMsg &N2kMsg, unsigned char SID, double Leeway) {
  SetN2kPGN128000(N2kMsg,SID,Leeway);
}

bool ParseN2kPGN128000(const tN2kMsg &N2kMsg, unsigned char &SID, double &Leeway);

inline bool ParseN2kLeeway(const tN2kMsg &N2kMsg, unsigned char &SID, double &Leeway) {
  return ParseN2kPGN128000(N2kMsg, SID, Leeway);
}

//*****************************************************************************
// Boat speed
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and wind at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - WaterReferenced        Speed over water in m/s
//  - GroundReferenced      Ground referenced speed in m/s
//  - SWRT                  Type of transducer. See definition for tN2kSpeedWaterReferenceType
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN128259(tN2kMsg &N2kMsg, unsigned char SID, double WaterReferenced, double GroundReferenced=N2kDoubleNA, tN2kSpeedWaterReferenceType SWRT=N2kSWRT_Paddle_wheel);

inline void SetN2kBoatSpeed(tN2kMsg &N2kMsg, unsigned char SID, double WaterReferenced, double GroundReferenced=N2kDoubleNA, tN2kSpeedWaterReferenceType SWRT=N2kSWRT_Paddle_wheel) {
  SetN2kPGN128259(N2kMsg,SID,WaterReferenced,GroundReferenced,SWRT);
}

bool ParseN2kPGN128259(const tN2kMsg &N2kMsg, unsigned char &SID, double &WaterReferenced, double &GroundReferenced, tN2kSpeedWaterReferenceType &SWRT);

inline bool ParseN2kBoatSpeed(const tN2kMsg &N2kMsg, unsigned char &SID, double &WaterReferenced, double &GroundReferenced, tN2kSpeedWaterReferenceType &SWRT) {
  return ParseN2kPGN128259(N2kMsg, SID, WaterReferenced, GroundReferenced, SWRT);
}

//*****************************************************************************
// Water depth
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and depth at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - DepthBelowTransducer  Depth below transducer in meters
//  - Offset                Distance in meters between transducer and surface (positive) or transducer and keel (negative)
//  - Range                 Measuring range
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN128267(tN2kMsg &N2kMsg, unsigned char SID, double DepthBelowTransducer, double Offset, double Range=N2kDoubleNA);

inline void SetN2kWaterDepth(tN2kMsg &N2kMsg, unsigned char SID, double DepthBelowTransducer, double Offset, double Range=N2kDoubleNA) {
  SetN2kPGN128267(N2kMsg,SID,DepthBelowTransducer,Offset,Range);
}

bool ParseN2kPGN128267(const tN2kMsg &N2kMsg, unsigned char &SID, double &DepthBelowTransducer, double &Offset, double &Range);

inline bool ParseN2kWaterDepth(const tN2kMsg &N2kMsg, unsigned char &SID, double &DepthBelowTransducer, double &Offset) {
  double Range;
  return ParseN2kPGN128267(N2kMsg, SID, DepthBelowTransducer, Offset, Range);
}

inline bool ParseN2kWaterDepth(const tN2kMsg &N2kMsg, unsigned char &SID, double &DepthBelowTransducer, double &Offset, double &Range) {
  return ParseN2kPGN128267(N2kMsg, SID, DepthBelowTransducer, Offset, Range);
}

//*****************************************************************************
// Distance log
// Input:
//  - DaysSince1970         Timestamp
//  - SecondsSinceMidnight  Timestamp
//  - Log                   Total meters travelled
//  - Trip Log              Meters travelled since last reset
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN128275(tN2kMsg &N2kMsg, uint16_t DaysSince1970, double SecondsSinceMidnight, uint32_t Log, uint32_t TripLog);

inline void SetN2kDistanceLog(tN2kMsg &N2kMsg, uint16_t DaysSince1970, double SecondsSinceMidnight, uint32_t Log, uint32_t TripLog) {
  SetN2kPGN128275(N2kMsg,DaysSince1970,SecondsSinceMidnight,Log,TripLog);
}

bool ParseN2kPGN128275(const tN2kMsg &N2kMsg, uint16_t &DaysSince1970, double &SecondsSinceMidnight, uint32_t &Log, uint32_t &TripLog);

inline bool ParseN2kDistanceLog(const tN2kMsg &N2kMsg, uint16_t &DaysSince1970, double &SecondsSinceMidnight, uint32_t &Log, uint32_t &TripLog) {
  return ParseN2kPGN128275(N2kMsg,DaysSince1970,SecondsSinceMidnight,Log,TripLog);
}

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

void SetN2kPGN128776(
  tN2kMsg &N2kMsg,
  unsigned char SID,
  unsigned char WindlassIdentifier,
  tN2kWindlassDirectionControl WindlassDirectionControl,
  unsigned char SpeedControl,
  tN2kSpeedType SpeedControlType = N2kDD488_DataNotAvailable,
  tN2kGenericStatusPair AnchorDockingControl = N2kDD002_Unavailable,
  tN2kGenericStatusPair PowerEnable = N2kDD002_Unavailable,
  tN2kGenericStatusPair MechanicalLock = N2kDD002_Unavailable,
  tN2kGenericStatusPair DeckAndAnchorWash = N2kDD002_Unavailable,
  tN2kGenericStatusPair AnchorLight = N2kDD002_Unavailable,
  double CommandTimeout = 0.4,
  const tN2kWindlassControlEvents &WindlassControlEvents = tN2kWindlassControlEvents()
);

bool ParseN2kPGN128776(
  const tN2kMsg &N2kMsg,
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

void SetN2kPGN128777(
  tN2kMsg &N2kMsg,
  unsigned char SID,
  unsigned char WindlassIdentifier,
  double RodeCounterValue,
  double WindlassLineSpeed = N2kDoubleNA,
  tN2kWindlassMotionStates WindlassMotionStatus = N2kDD480_Unavailable,
  tN2kRodeTypeStates RodeTypeStatus = N2kDD481_Unavailable,
  tN2kDD482 AnchorDockingStatus = N2kDD482_DataNotAvailable,
  const tN2kWindlassOperatingEvents &WindlassOperatingEvents = tN2kWindlassOperatingEvents()
);

bool ParseN2kPGN128777(
  const tN2kMsg &N2kMsg,
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

void SetN2kPGN128778(
  tN2kMsg &N2kMsg,
  unsigned char SID,
  unsigned char WindlassIdentifier,
  double TotalMotorTime,
  double ControllerVoltage = N2kDoubleNA,
  double MotorCurrent = N2kDoubleNA,
  const tN2kWindlassMonitoringEvents &WindlassMonitoringEvents = tN2kWindlassMonitoringEvents()
);

bool ParseN2kPGN128778(
  const tN2kMsg &N2kMsg,
  unsigned char &SID,
  unsigned char &WindlassIdentifier,
  double &TotalMotorTime,
  double &ControllerVoltage,
  double &MotorCurrent,
  tN2kWindlassMonitoringEvents &WindlassMonitoringEvents
);

//*****************************************************************************
// Lat/lon rapid
// Input:
//  - Latitude               Latitude in degrees
//  - Longitude              Longitude in degrees
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129025(tN2kMsg &N2kMsg, double Latitude, double Longitude);

inline void SetN2kLatLonRapid(tN2kMsg &N2kMsg, double Latitude, double Longitude) {
  SetN2kPGN129025(N2kMsg,Latitude,Longitude);
}

bool ParseN2kPGN129025(const tN2kMsg &N2kMsg, double &Latitude, double &Longitude);
inline bool ParseN2kPositionRapid(const tN2kMsg &N2kMsg, double &Latitude, double &Longitude) {
	return ParseN2kPGN129025(N2kMsg, Latitude, Longitude);
}
//*****************************************************************************
// COG SOG rapid
// Input:
//  - COG                   Cource Over Ground in radians
//  - SOG                   Speed Over Ground in m/s
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129026(tN2kMsg &N2kMsg, unsigned char SID, tN2kHeadingReference ref, double COG, double SOG);

inline void SetN2kCOGSOGRapid(tN2kMsg &N2kMsg, unsigned char SID, tN2kHeadingReference ref, double COG, double SOG) {
  SetN2kPGN129026(N2kMsg,SID,ref,COG,SOG);
}

bool ParseN2kPGN129026(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kHeadingReference &ref, double &COG, double &SOG);
inline bool ParseN2kCOGSOGRapid(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kHeadingReference &ref, double &COG, double &SOG) {
  return ParseN2kPGN129026(N2kMsg,SID,ref,COG,SOG);
}

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
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129029(tN2kMsg &N2kMsg, unsigned char SID, uint16_t DaysSince1970, double SecondsSinceMidnight,
                     double Latitude, double Longitude, double Altitude,
                     tN2kGNSStype GNSStype, tN2kGNSSmethod GNSSmethod,
                     unsigned char nSatellites, double HDOP, double PDOP=0, double GeoidalSeparation=0,
                     unsigned char nReferenceStations=0, tN2kGNSStype ReferenceStationType=N2kGNSSt_GPS, uint16_t ReferenceSationID=0,
                     double AgeOfCorrection=0
                     );

inline void SetN2kGNSS(tN2kMsg &N2kMsg, unsigned char SID, uint16_t DaysSince1970, double SecondsSinceMidnight,
                     double Latitude, double Longitude, double Altitude,
                     tN2kGNSStype GNSStype, tN2kGNSSmethod GNSSmethod,
                     unsigned char nSatellites, double HDOP, double PDOP=0, double GeoidalSeparation=0,
                     unsigned char nReferenceStations=0, tN2kGNSStype ReferenceStationType=N2kGNSSt_GPS, uint16_t ReferenceSationID=0,
                     double AgeOfCorrection=0
                     ) {
  SetN2kPGN129029(N2kMsg,SID,DaysSince1970,SecondsSinceMidnight,
                  Latitude,Longitude,Altitude,
                  GNSStype,GNSSmethod,
                  nSatellites,HDOP,PDOP,GeoidalSeparation,
                  nReferenceStations,ReferenceStationType,ReferenceSationID,
                  AgeOfCorrection);
}

bool ParseN2kPGN129029(const tN2kMsg &N2kMsg, unsigned char &SID, uint16_t &DaysSince1970, double &SecondsSinceMidnight,
                     double &Latitude, double &Longitude, double &Altitude,
                     tN2kGNSStype &GNSStype, tN2kGNSSmethod &GNSSmethod,
                     unsigned char &nSatellites, double &HDOP, double &PDOP, double &GeoidalSeparation,
                     unsigned char &nReferenceStations, tN2kGNSStype &ReferenceStationType, uint16_t &ReferenceSationID,
                     double &AgeOfCorrection
                     );
inline bool ParseN2kGNSS(const tN2kMsg &N2kMsg, unsigned char &SID, uint16_t &DaysSince1970, double &SecondsSinceMidnight,
                     double &Latitude, double &Longitude, double &Altitude,
                     tN2kGNSStype &GNSStype, tN2kGNSSmethod &GNSSmethod,
                     unsigned char &nSatellites, double &HDOP, double &PDOP, double &GeoidalSeparation,
                     unsigned char &nReferenceStations, tN2kGNSStype &ReferenceStationType, uint16_t &ReferenceSationID,
                     double &AgeOfCorrection
                     ) {
  return ParseN2kPGN129029(N2kMsg, SID, DaysSince1970, SecondsSinceMidnight,
                     Latitude, Longitude, Altitude,
                     GNSStype, GNSSmethod,
                     nSatellites, HDOP, PDOP, GeoidalSeparation,
                     nReferenceStations, ReferenceStationType, ReferenceSationID,
                     AgeOfCorrection
                     );
}

//*****************************************************************************
// Date,Time & Local offset  ( see also PGN 126992 )
// Input:
//  - DaysSince1970         Days since 1.1.1970. You can find sample how to convert e.g. NMEA0183 date info to this on my NMEA0183 library on
//                          NMEA0183Messages.cpp on function NMEA0183ParseRMC_nc
//  - Time                  Seconds since midnight
//  - Local offset          Local offset in minutes
void SetN2kPGN129033(tN2kMsg &N2kMsg, uint16_t DaysSince1970, double SecondsSinceMidnight, int16_t LocalOffset);

inline void SetN2kLocalOffset(tN2kMsg &N2kMsg, uint16_t DaysSince1970, double SecondsSinceMidnight, int16_t LocalOffset) {
  SetN2kPGN129033(N2kMsg,DaysSince1970,SecondsSinceMidnight,LocalOffset);
}

bool ParseN2kPGN129033(const tN2kMsg &N2kMsg, uint16_t &DaysSince1970, double &SecondsSinceMidnight, int16_t &LocalOffset);

inline bool ParseN2kLocalOffset(const tN2kMsg &N2kMsg, uint16_t &DaysSince1970, double &SecondsSinceMidnight, int16_t &LocalOffset) {
  return ParseN2kPGN129033(N2kMsg,DaysSince1970,SecondsSinceMidnight,LocalOffset);
}

//*****************************************************************************
// AIS position reports for Class A
// Input:
//  - N2kMsg                NMEA2000 message to decode
void SetN2kPGN129038(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID, double Latitude, double Longitude,
                        bool Accuracy, bool RAIM, uint8_t Seconds, double COG, double SOG, double Heading, double ROT, tN2kAISNavStatus NavStatus);

inline void SetN2kAISClassAPosition(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID, double Latitude, double Longitude,
                        bool Accuracy, bool RAIM, uint8_t Seconds, double COG, double SOG, double Heading, double ROT, tN2kAISNavStatus NavStatus) {
  SetN2kPGN129038(N2kMsg, MessageID, Repeat, UserID, Latitude, Longitude, Accuracy, RAIM, Seconds, COG, SOG, Heading, ROT, NavStatus);
}

bool ParseN2kPGN129038(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID, double &Latitude, double &Longitude,
                        bool &Accuracy, bool &RAIM, uint8_t &Seconds, double &COG, double &SOG, double &Heading, double &ROT, tN2kAISNavStatus &NavStatus, tN2kAISTransceiverInformation &AISTransceiverInformation);

inline bool ParseN2kAISClassAPosition(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID, double &Latitude, double &Longitude,
                        bool &Accuracy, bool &RAIM, uint8_t &Seconds, double &COG, double &SOG, double &Heading, double &ROT, tN2kAISNavStatus & NavStatus, tN2kAISTransceiverInformation &AISTransceiverInformation) {
  return ParseN2kPGN129038(N2kMsg, MessageID, Repeat, UserID, Latitude, Longitude, Accuracy, RAIM, Seconds, COG, SOG, Heading, ROT, NavStatus, AISTransceiverInformation);
}

//*****************************************************************************
// AIS position reports for Class B
// Input:
//  - N2kMsg                NMEA2000 message to decode
void SetN2kPGN129039(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                        double Latitude, double Longitude, bool Accuracy, bool RAIM,
                        uint8_t Seconds, double COG, double SOG, tN2kAISTransceiverInformation AISTransceiverInformation,
                        double Heading, tN2kAISUnit Unit, bool Display, bool DSC, bool Band, bool Msg22, tN2kAISMode Mode,
                        bool State);

// Latest inline version
inline void SetN2kAISClassBPosition(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                        double Latitude, double Longitude, bool Accuracy, bool RAIM,
                        uint8_t Seconds, double COG, double SOG, tN2kAISTransceiverInformation AISTransceiverInformation,
                        double Heading, tN2kAISUnit Unit, bool Display, bool DSC, bool Band, bool Msg22, tN2kAISMode Mode,
                        bool State) {
  SetN2kPGN129039(N2kMsg, MessageID, Repeat, UserID, Latitude, Longitude, Accuracy, RAIM, Seconds,
                    COG, SOG, AISTransceiverInformation, Heading, Unit, Display, DSC, Band, Msg22, Mode, State);
}

// Previous inline version for backwards compatibility, using N2kaischannel_A_VDL_reception(0) as default value
inline void SetN2kAISClassBPosition(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                        double Latitude, double Longitude, bool Accuracy, bool RAIM,
                        uint8_t Seconds, double COG, double SOG, double Heading, tN2kAISUnit Unit,
                        bool Display, bool DSC, bool Band, bool Msg22, tN2kAISMode Mode, bool State) {
  SetN2kPGN129039(N2kMsg, MessageID, Repeat, UserID, Latitude, Longitude, Accuracy, RAIM, Seconds,
                    COG, SOG, N2kaischannel_A_VDL_reception, Heading, Unit, Display, DSC, Band, Msg22, Mode, State);
}

bool ParseN2kPGN129039(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM, uint8_t &Seconds, double &COG,
                        double &SOG, tN2kAISTransceiverInformation &AISTransceiverInformation, double &Heading,
                        tN2kAISUnit &Unit, bool &Display, bool &DSC, bool &Band, bool &Msg22, tN2kAISMode &Mode, bool &State);

// Latest inline version
inline bool ParseN2kAISClassBPosition(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM,
                        uint8_t &Seconds, double &COG, double &SOG, tN2kAISTransceiverInformation &AISTransceiverInformation,
                        double &Heading, tN2kAISUnit &Unit, bool &Display, bool &DSC, bool &Band, bool &Msg22, tN2kAISMode &Mode,
                        bool &State) {
  return ParseN2kPGN129039(N2kMsg, MessageID, Repeat, UserID, Latitude, Longitude, Accuracy, RAIM, Seconds, COG, SOG, AISTransceiverInformation, Heading, Unit, Display, DSC, Band, Msg22, Mode, State);
}

// Previous inline version for backwards compatibility, using temporary value to parse unused paramter
inline bool ParseN2kAISClassBPosition(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM,
                        uint8_t &Seconds, double &COG, double &SOG, double &Heading, tN2kAISUnit &Unit,
                        bool &Display, bool &DSC, bool &Band, bool &Msg22, tN2kAISMode &Mode, bool &State) {
  tN2kAISTransceiverInformation AISTransceiverInformation; // for backwards compatibility
  return ParseN2kPGN129039(N2kMsg, MessageID, Repeat, UserID, Latitude, Longitude, Accuracy,
                            RAIM, Seconds, COG, SOG, AISTransceiverInformation, Heading, Unit, Display, DSC, Band, Msg22, Mode, State);
}

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
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
struct tN2kAISAtoNReportData {
  uint8_t MessageID;
  tN2kAISRepeat Repeat;
  uint32_t UserID;
  double Longitude;
  double Latitude;
  bool Accuracy;
  bool RAIM;
  uint8_t Seconds;
  double Length;
  double Beam;
  double PositionReferenceStarboard ;
  double PositionReferenceTrueNorth;
  tN2kAISAtoNType AtoNType;
  bool OffPositionIndicator;
  bool VirtualAtoNFlag;
  bool AssignedModeFlag;
  tN2kGNSStype GNSSType;
  uint8_t AtoNStatus;
  tN2kAISTransceiverInformation AISTransceiverInformation;
  char AtoNName[34 + 1];

  tN2kAISAtoNReportData():
    MessageID(N2kUInt8NA),
    Repeat(N2kaisr_Initial),
    UserID(N2kUInt32NA),
    Longitude(N2kDoubleNA),
    Latitude(N2kDoubleNA),
    Accuracy(false),
    RAIM(false),
    Seconds(N2kUInt8NA),
    Length(N2kDoubleNA),
    Beam(N2kDoubleNA),
    PositionReferenceStarboard(N2kDoubleNA),
    PositionReferenceTrueNorth(N2kDoubleNA),
    AtoNType(N2kAISAtoN_not_specified),
    OffPositionIndicator(false),
    VirtualAtoNFlag(false),
    AssignedModeFlag(false),
    GNSSType(N2kGNSSt_GPS),
    AtoNStatus(N2kUInt8NA),
    AISTransceiverInformation(N2kaischannel_A_VDL_reception) {
      AtoNName[0]=0;
   }

  void SetAtoNName(const char *name) {
    strncpy(AtoNName, name, sizeof(AtoNName));
    AtoNName[sizeof(AtoNName) - 1] = 0;
  }
};

void SetN2kPGN129041(tN2kMsg &N2kMsg, const tN2kAISAtoNReportData &N2kData);
inline void SetN2kAISAtoNReport(tN2kMsg &N2kMsg, const tN2kAISAtoNReportData &N2kData) {
  SetN2kPGN129041(N2kMsg, N2kData);
}

bool ParseN2kPGN129041(const tN2kMsg &N2kMsg, tN2kAISAtoNReportData &N2kData);
inline bool ParseN2kAISAtoNReport(const tN2kMsg &N2kMsg, tN2kAISAtoNReportData &N2kData) {
  return ParseN2kPGN129041(N2kMsg, N2kData);
}

//*****************************************************************************
// Cross Track Error
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129283(tN2kMsg &N2kMsg, unsigned char SID, tN2kXTEMode XTEMode, bool NavigationTerminated, double XTE);

inline void SetN2kXTE(tN2kMsg &N2kMsg, unsigned char SID, tN2kXTEMode XTEMode, bool NavigationTerminated, double XTE) {
  SetN2kPGN129283(N2kMsg, SID, XTEMode, NavigationTerminated, XTE);
}

bool ParseN2kPGN129283(const tN2kMsg &N2kMsg, unsigned char& SID, tN2kXTEMode& XTEMode, bool& NavigationTerminated, double& XTE);

inline bool ParseN2kXTE(const tN2kMsg &N2kMsg, unsigned char& SID, tN2kXTEMode& XTEMode, bool& NavigationTerminated, double& XTE) {
   return ParseN2kPGN129283(N2kMsg, SID, XTEMode, NavigationTerminated, XTE);
}

//*****************************************************************************
// Navigation info
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129284(tN2kMsg &N2kMsg, unsigned char SID, double DistanceToWaypoint, tN2kHeadingReference BearingReference,
                      bool PerpendicularCrossed, bool ArrivalCircleEntered, tN2kDistanceCalculationType CalculationType,
                      double ETATime, int16_t ETADate, double BearingOriginToDestinationWaypoint, double BearingPositionToDestinationWaypoint,
                      uint8_t OriginWaypointNumber, uint8_t DestinationWaypointNumber,
                      double DestinationLatitude, double DestinationLongitude, double WaypointClosingVelocity);

inline void SetN2kNavigationInfo(tN2kMsg &N2kMsg, unsigned char SID, double DistanceToWaypoint, tN2kHeadingReference BearingReference,
                      bool PerpendicularCrossed, bool ArrivalCircleEntered, tN2kDistanceCalculationType CalculationType,
                      double ETATime, int16_t ETADate, double BearingOriginToDestinationWaypoint, double BearingPositionToDestinationWaypoint,
                      uint8_t OriginWaypointNumber, uint8_t DestinationWaypointNumber,
                      double DestinationLatitude, double DestinationLongitude, double WaypointClosingVelocity) {
  SetN2kPGN129284(N2kMsg, SID, DistanceToWaypoint, BearingReference,
                      PerpendicularCrossed, ArrivalCircleEntered, CalculationType,
                      ETATime, ETADate, BearingOriginToDestinationWaypoint, BearingPositionToDestinationWaypoint,
                      OriginWaypointNumber, DestinationWaypointNumber,
                      DestinationLatitude, DestinationLongitude, WaypointClosingVelocity);
}

bool ParseN2kPGN129284(const tN2kMsg &N2kMsg, unsigned char& SID, double& DistanceToWaypoint, tN2kHeadingReference& BearingReference,
                      bool& PerpendicularCrossed, bool& ArrivalCircleEntered, tN2kDistanceCalculationType& CalculationType,
                      double& ETATime, int16_t& ETADate, double& BearingOriginToDestinationWaypoint, double& BearingPositionToDestinationWaypoint,
                      uint8_t& OriginWaypointNumber, uint8_t& DestinationWaypointNumber,
                      double& DestinationLatitude, double& DestinationLongitude, double& WaypointClosingVelocity);

inline bool ParseN2kNavigationInfo(const tN2kMsg &N2kMsg, unsigned char& SID, double& DistanceToWaypoint, tN2kHeadingReference& BearingReference,
                      bool& PerpendicularCrossed, bool& ArrivalCircleEntered, tN2kDistanceCalculationType& CalculationType,
                      double& ETATime, int16_t& ETADate, double& BearingOriginToDestinationWaypoint, double& BearingPositionToDestinationWaypoint,
                      uint8_t& OriginWaypointNumber, uint8_t& DestinationWaypointNumber,
                      double& DestinationLatitude, double& DestinationLongitude, double& WaypointClosingVelocity) {
   return ParseN2kPGN129284(N2kMsg, SID, DistanceToWaypoint, BearingReference, PerpendicularCrossed, ArrivalCircleEntered, CalculationType,
                            ETATime, ETADate, BearingOriginToDestinationWaypoint, BearingPositionToDestinationWaypoint,
                            OriginWaypointNumber, DestinationWaypointNumber, DestinationLatitude, DestinationLongitude, WaypointClosingVelocity);
}

//*****************************************************************************
// Route/WP information
// Input:
//  - Start                 The ID of the first waypoint
//  - Database              Database ID
//  - Route                 Route ID
//  - NavDirection          Navigation direction in route
//  - SupplementaryData     Supplementary Route/WP data available
//  - RouteName             The name of the current route
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129285(tN2kMsg &N2kMsg, uint16_t Start, uint16_t Database, uint16_t Route,
      tN2kNavigationDirection NavDirection, uint8_t SupplementaryData, char* RouteName);

inline void SetN2kRouteWPInfo(tN2kMsg &N2kMsg, uint16_t Start, uint16_t Database, uint16_t Route,
      tN2kNavigationDirection NavDirection, uint8_t SupplementaryData, char* RouteName)
{
   SetN2kPGN129285(N2kMsg, Start, Database, Route, NavDirection, SupplementaryData, RouteName);
}

// for backwards compatibility
inline void SetN2kPGN129285(tN2kMsg &N2kMsg, uint16_t Start, uint16_t Database, uint16_t Route,
                        bool NavDirection, bool SupplementaryData, char* RouteName)
{
   tN2kNavigationDirection NavDirection1 = NavDirection?N2kdir_reverse:N2kdir_forward;
	SetN2kPGN129285(N2kMsg, Start, Database, Route, NavDirection1, (uint8_t)SupplementaryData, RouteName);
}

// Route/WP appended information
// Input:
//  - ID                    The ID of the current waypoint
//  - Name                  The name of the current waypoint
//  - Latitude              The latitude of the current waypoint
//  - Longitude             The longitude of the current waypoint
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
// Return:
//  - true if there was enough space in the message
bool AppendN2kPGN129285(tN2kMsg &N2kMsg, uint16_t WPID, char* WPName, double Latitude, double Longitude);

inline bool AppendN2kRouteWPInfo(tN2kMsg &N2kMsg, uint16_t WPID, char* WPName, double Latitude, double Longitude)
{
   return AppendN2kPGN129285(N2kMsg, WPID, WPName, Latitude, Longitude);
}

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
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129539(tN2kMsg& N2kMsg, unsigned char SID, tN2kGNSSDOPmode DesiredMode, tN2kGNSSDOPmode ActualMode,
                     double HDOP, double VDOP, double TDOP);

inline void SetN2kGNSSDOPData(tN2kMsg& N2kMsg, unsigned char SID, tN2kGNSSDOPmode DesiredMode, tN2kGNSSDOPmode ActualMode,
                              double HDOP, double VDOP, double TDOP)
{
    SetN2kPGN129539(N2kMsg, SID, DesiredMode, ActualMode, HDOP, VDOP, TDOP);
}

bool ParseN2kPgn129539(const tN2kMsg& N2kMsg, unsigned char& SID, tN2kGNSSDOPmode& DesiredMode, tN2kGNSSDOPmode& ActualMode,
                       double& HDOP, double& VDOP, double& TDOP);

inline bool ParseN2kGNSSDOPData(const tN2kMsg& N2kMsg, unsigned char& SID, tN2kGNSSDOPmode& DesiredMode, tN2kGNSSDOPmode& ActualMode,
                         double& HDOP, double& VDOP, double& TDOP)
{
    return ParseN2kPgn129539(N2kMsg, SID, DesiredMode, ActualMode, HDOP, VDOP, TDOP);
}


//*****************************************************************************
struct tSatelliteInfo {
  unsigned char PRN;
  double Elevation;
  double Azimuth;
  double SNR;
  double RangeResiduals;
  tN2kPRNUsageStatus UsageStatus;
};

//*****************************************************************************
// GNSS Satellites in View
// Use first function for basic initialization and second for adding satellites.
//
// Initialize GNSS Satellites in View message
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and GPS at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - Mode                  Range residual mode.
//  - SatelliteInfo         Satellite info see tSatelliteInfo above.
void SetN2kPGN129540(tN2kMsg& N2kMsg, unsigned char SID=0xff, tN2kRangeResidualMode Mode=N2kDD072_Unavailable);

// Append new satellite info for GNSS Satellites in View message
// Input:
//  - SatelliteInfo         Requested satellite info.
//
// Return:
//   true  - if function succeeds.
//   false - if no more satellites can be added or try to use wrong or uninitialized message.
bool AppendN2kPGN129540(tN2kMsg& N2kMsg, const tSatelliteInfo& SatelliteInfo);

inline void SetN2kGNSSSatellitesInView(tN2kMsg& N2kMsg, unsigned char SID=0xff, tN2kRangeResidualMode Mode=N2kDD072_Unavailable) {
  SetN2kPGN129540(N2kMsg,SID,Mode);
}

inline bool AppendSatelliteInfo(tN2kMsg& N2kMsg, const tSatelliteInfo& SatelliteInfo) {
  return AppendN2kPGN129540(N2kMsg,SatelliteInfo);
}

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
bool ParseN2kPGN129540(const tN2kMsg& N2kMsg, unsigned char& SID, tN2kRangeResidualMode &Mode, uint8_t& NumberOfSVs);

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

inline bool ParseN2kPGNSatellitesInView(const tN2kMsg& N2kMsg, unsigned char& SID, tN2kRangeResidualMode &Mode, uint8_t& NumberOfSVs){
  return ParseN2kPGN129540(N2kMsg,SID,Mode,NumberOfSVs);
}
inline bool ParseN2kPGNSatellitesInView(const tN2kMsg& N2kMsg, uint8_t SVIndex, tSatelliteInfo& SatelliteInfo){
  return ParseN2kPGN129540(N2kMsg,SVIndex,SatelliteInfo);
}


//*****************************************************************************
// AIS static data class A
// Input:
//  - MessageID             Message type
//  - Repeat                Repeat indicator
//  - UserID                MMSI
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129794(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                        uint32_t IMOnumber, char *Callsign, char *Name, uint8_t VesselType, double Length,
                        double Beam, double PosRefStbd, double PosRefBow, uint16_t ETAdate, double ETAtime,
                        double Draught, char *Destination, tN2kAISVersion AISversion, tN2kGNSStype GNSStype,
                        tN2kAISDTE DTE, tN2kAISTranceiverInfo AISinfo);
inline void SetN2kAISClassAStatic(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                        uint32_t IMOnumber, char *Callsign, char *Name, uint8_t VesselType, double Length,
                        double Beam, double PosRefStbd, double PosRefBow, uint16_t ETAdate, double ETAtime,
                        double Draught, char *Destination, tN2kAISVersion AISversion, tN2kGNSStype GNSStype,
                        tN2kAISDTE DTE, tN2kAISTranceiverInfo AISinfo) {
  SetN2kPGN129794(N2kMsg, MessageID, Repeat, UserID, IMOnumber, Callsign, Name, VesselType, Length,
                  Beam, PosRefStbd, PosRefBow, ETAdate, ETAtime, Draught, Destination, AISversion, GNSStype, DTE, AISinfo);
}

bool ParseN2kPGN129794(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        uint32_t &IMOnumber, char *Callsign, char *Name, uint8_t &VesselType, double &Length,
                        double &Beam, double &PosRefStbd, double &PosRefBow, uint16_t &ETAdate, double &ETAtime,
                        double &Draught, char *Destination, tN2kAISVersion &AISversion, tN2kGNSStype &GNSStype,
                        tN2kAISDTE &DTE, tN2kAISTranceiverInfo &AISinfo);

inline bool ParseN2kAISClassAStatic(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        uint32_t & IMOnumber, char *Callsign, char *Name, uint8_t &VesselType, double &Length,
                        double &Beam, double &PosRefStbd, double &PosRefBow, uint16_t &ETAdate, double &ETAtime,
                        double &Draught, char *Destination, tN2kAISVersion &AISversion, tN2kGNSStype &GNSStype,
                        tN2kAISDTE &DTE, tN2kAISTranceiverInfo &AISinfo) {
  return ParseN2kPGN129794(N2kMsg, MessageID, Repeat, UserID, IMOnumber, Callsign, Name, VesselType, Length,
                          Beam, PosRefStbd, PosRefBow, ETAdate, ETAtime, Draught, Destination, AISversion,
                          GNSStype, DTE, AISinfo);
}

//*****************************************************************************
// AIS static data class B part A
// Input:
//  - MessageID             Message type
//  - Repeat                Repeat indicator
//  - UserID                MMSI
//  - Name                  Vessel name
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129809(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID, char *Name);

inline void SetN2kAISClassBStaticPartA(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID, char *Name) {
  SetN2kPGN129809(N2kMsg, MessageID, Repeat, UserID, Name);
}

bool ParseN2kPGN129809(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID, char *Name);

inline bool ParseN2kAISClassBStaticPartA(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID, char *Name) {
  return ParseN2kPGN129809(N2kMsg, MessageID, Repeat, UserID, Name);
}

//*****************************************************************************
// AIS static data class B part B
// Input:
//  - MessageID             Message type
//  - Repeat                Repeat indicator
//  - UserID                MMSI
//  - Name                  Vessel name
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN129810(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                      uint8_t VesselType, char *Vendor, char *Callsign, double Length, double Beam,
                      double PosRefStbd, double PosRefBow, uint32_t MothershipID);

inline void SetN2kAISClassBStaticPartB(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                      uint8_t VesselType, char *Vendor, char *Callsign, double Length, double Beam,
                      double PosRefStbd, double PosRefBow, uint32_t MothershipID) {
  SetN2kPGN129810(N2kMsg, MessageID, Repeat, UserID, VesselType, Vendor, Callsign, Length, Beam,
                  PosRefStbd, PosRefBow, MothershipID);
}

bool ParseN2kPGN129810(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                      uint8_t &VesselType, char *Vendor, char *Callsign, double &Length, double &Beam,
                      double &PosRefStbd, double &PosRefBow, uint32_t &MothershipID);

inline bool ParseN2kAISClassBStaticPartB(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                      uint8_t &VesselType, char *Vendor, char *Callsign, double &Length, double &Beam,
                      double &PosRefStbd, double &PosRefBow, uint32_t &MothershipID) {
  return ParseN2kPGN129810(N2kMsg, MessageID, Repeat, UserID, VesselType, Vendor, Callsign,
                                Length, Beam, PosRefStbd, PosRefBow, MothershipID);
}

//*****************************************************************************
// Waypoint list
// Input:
//  - Start                 The ID of the first waypoint
//    NumItems
//  - NumWaypoints          Number of valid WPs in the WP-List
//  - Database              Database ID
//  - ID                    The ID of the current waypoint
//  - Name                  The name of the current waypoint
//  - Latitude              The latitude of the current waypoint
//  - Longitude             The longitude of the current waypoint
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130074(tN2kMsg &N2kMsg, uint16_t Start, uint16_t NumWaypoints, uint16_t Database);

inline void SetN2kWaypointList(tN2kMsg &N2kMsg, uint16_t Start, uint16_t NumWaypoints, uint16_t Database) {
  SetN2kPGN130074(N2kMsg,Start,NumWaypoints,Database);
}

bool AppendN2kPGN130074(tN2kMsg &N2kMsg, uint16_t ID, char* Name, double Latitude, double Longitude);

inline bool AppendN2kWaypointList(tN2kMsg &N2kMsg, uint16_t ID, char* Name, double Latitude, double Longitude) {
  return AppendN2kPGN130074(N2kMsg,ID,Name,Latitude,Longitude);
}

//*****************************************************************************
// Wind Speed
// Input:
//  - SID                   Sequence ID. If your device is e.g. boat speed and wind at same time, you can set same SID for different messages
//                          to indicate that they are measured at same time.
//  - WindSpeed             Measured wind speed in m/s
//  - WindAngle             Measured wind angle in radians. If you have value in degrees, use function DegToRad(myval) in call.
//  - WindReference         Wind reference, see definition of tN2kWindReference
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130306(tN2kMsg &N2kMsg, unsigned char SID, double WindSpeed, double WindAngle, tN2kWindReference WindReference);

inline void SetN2kWindSpeed(tN2kMsg &N2kMsg, unsigned char SID, double WindSpeed, double WindAngle, tN2kWindReference WindReference) {
  SetN2kPGN130306(N2kMsg,SID,WindSpeed,WindAngle,WindReference);
}

bool ParseN2kPGN130306(const tN2kMsg &N2kMsg, unsigned char &SID, double &WindSpeed, double &WindAngle, tN2kWindReference &WindReference);

inline bool ParseN2kWindSpeed(const tN2kMsg &N2kMsg, unsigned char &SID, double &WindSpeed, double &WindAngle, tN2kWindReference &WindReference) {
  return ParseN2kPGN130306(N2kMsg,SID,WindSpeed,WindAngle,WindReference);
}

//*****************************************************************************
// Outside Environmental parameters
// Input:
//  - SID                   Sequence ID.
//  - WaterTemperature      Water temperature in K. Use function CToKelvin, if you want to use °C.
//  - OutsideAmbientAirTemperature      Outside ambient temperature in K. Use function CToKelvin, if you want to use °C.
//  - AtmosphericPressure   Atmospheric pressure in Pascals. Use function mBarToPascal, if you like to use mBar
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130310(tN2kMsg &N2kMsg, unsigned char SID, double WaterTemperature,
                     double OutsideAmbientAirTemperature=N2kDoubleNA, double AtmosphericPressure=N2kDoubleNA);

inline void SetN2kOutsideEnvironmentalParameters(tN2kMsg &N2kMsg, unsigned char SID, double WaterTemperature,
                     double OutsideAmbientAirTemperature=N2kDoubleNA, double AtmosphericPressure=N2kDoubleNA) {
  SetN2kPGN130310(N2kMsg,SID,WaterTemperature,OutsideAmbientAirTemperature,AtmosphericPressure);
}

bool ParseN2kPGN130310(const tN2kMsg &N2kMsg, unsigned char &SID, double &WaterTemperature,
                     double &OutsideAmbientAirTemperature, double &AtmosphericPressure);
inline bool ParseN2kOutsideEnvironmentalParameters(const tN2kMsg &N2kMsg, unsigned char &SID, double &WaterTemperature,
                     double &OutsideAmbientAirTemperature, double &AtmosphericPressure) {
  return ParseN2kPGN130310(N2kMsg, SID,WaterTemperature,OutsideAmbientAirTemperature,AtmosphericPressure);
}

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
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130311(tN2kMsg &N2kMsg, unsigned char SID, tN2kTempSource TempSource, double Temperature,
                     tN2kHumiditySource HumiditySource=N2khs_Undef, double Humidity=N2kDoubleNA, double AtmosphericPressure=N2kDoubleNA);

inline void SetN2kEnvironmentalParameters(tN2kMsg &N2kMsg, unsigned char SID, tN2kTempSource TempSource, double Temperature,
                     tN2kHumiditySource HumiditySource=N2khs_Undef, double Humidity=N2kDoubleNA, double AtmosphericPressure=N2kDoubleNA) {
  SetN2kPGN130311(N2kMsg,SID,TempSource,Temperature,HumiditySource,Humidity,AtmosphericPressure);
}

bool ParseN2kPGN130311(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kTempSource &TempSource, double &Temperature,
                     tN2kHumiditySource &HumiditySource, double &Humidity, double &AtmosphericPressure);
inline bool ParseN2kEnvironmentalParameters(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kTempSource &TempSource, double &Temperature,
                     tN2kHumiditySource &HumiditySource, double &Humidity, double &AtmosphericPressure) {
  return ParseN2kPGN130311(N2kMsg,SID,TempSource,Temperature,HumiditySource,Humidity,AtmosphericPressure);
}

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
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130312(tN2kMsg &N2kMsg, unsigned char SID, unsigned char TempInstance, tN2kTempSource TempSource,
                     double ActualTemperature, double SetTemperature=N2kDoubleNA);

inline void SetN2kTemperature(tN2kMsg &N2kMsg, unsigned char SID, unsigned char TempInstance, tN2kTempSource TempSource,
                     double ActualTemperature, double SetTemperature=N2kDoubleNA) {
  SetN2kPGN130312(N2kMsg,SID,TempInstance,TempSource,ActualTemperature,SetTemperature);
}

bool ParseN2kPGN130312(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature);
inline bool ParseN2kTemperature(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature) {
  return ParseN2kPGN130312(N2kMsg, SID, TempInstance, TempSource, ActualTemperature, SetTemperature);
}

//*****************************************************************************
// Humidity
// Humidity should be a percent
// Input:
//  - SID                   Sequence ID.
//  - HumidityInstance      This should be unic at least on one device. May be best to have it unic over all devices sending this PGN.
//  - HumiditySource        see tN2kHumiditySource
//  - Humidity              Humidity in percent
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130313(tN2kMsg &N2kMsg, unsigned char SID, unsigned char HumidityInstance,
                     tN2kHumiditySource HumiditySource, double ActualHumidity, double SetHumidity=N2kDoubleNA);
inline void SetN2kHumidity(tN2kMsg &N2kMsg, unsigned char SID, unsigned char HumidityInstance,
                     tN2kHumiditySource HumiditySource, double ActualHumidity, double SetHumidity=N2kDoubleNA) {
  SetN2kPGN130313(N2kMsg, SID, HumidityInstance, HumiditySource, ActualHumidity,SetHumidity);
}

bool ParseN2kPGN130313(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &HumidityInstance,
                       tN2kHumiditySource &HumiditySource, double &ActualHumidity, double &SetHumidity);

inline bool ParseN2kHumidity(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &HumidityInstance,
                       tN2kHumiditySource &HumiditySource, double &ActualHumidity, double &SetHumidity) {
  return ParseN2kPGN130313(N2kMsg, SID, HumidityInstance, HumiditySource, ActualHumidity, SetHumidity);
}

inline bool ParseN2kPGN130313(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &HumidityInstance,
                       tN2kHumiditySource &HumiditySource, double &ActualHumidity) {
  double SetHumidity;
  return ParseN2kPGN130313(N2kMsg, SID, HumidityInstance, HumiditySource, ActualHumidity, SetHumidity);
}

inline bool ParseN2kHumidity(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &HumidityInstance,
                       tN2kHumiditySource &HumiditySource, double &ActualHumidity) {
  return ParseN2kPGN130313(N2kMsg, SID, HumidityInstance, HumiditySource, ActualHumidity);
}

//*****************************************************************************
// Pressure
// Pressures should be in Pascals
// Input:
//  - SID                   Sequence ID.
//  - PressureInstance      This should be unic at least on one device. May be best to have it unic over all devices sending this PGN.
//  - PressureSource        see tN2kPressureSource
//  - Pressure              Pressure in Pascals. Use function mBarToPascal, if you like to use mBar
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130314(tN2kMsg &N2kMsg, unsigned char SID, unsigned char PressureInstance,
                     tN2kPressureSource PressureSource, double Pressure);
inline void SetN2kPressure(tN2kMsg &N2kMsg, unsigned char SID, unsigned char PressureInstance,
                           tN2kPressureSource PressureSource, double Pressure) {
  SetN2kPGN130314(N2kMsg, SID, PressureInstance, PressureSource, Pressure);
}
bool ParseN2kPGN130314(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &PressureInstance,
                       tN2kPressureSource &PressureSource, double &Pressure);
inline bool ParseN2kPressure(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &PressureInstance,
                       tN2kPressureSource &PressureSource, double &Pressure) {
  return ParseN2kPGN130314(N2kMsg, SID, PressureInstance, PressureSource, Pressure);
}

//*****************************************************************************
// Set pressure
// Pressures should be in Pascals
// Input:
//  - SID                       Sequence ID.
//  - PressureInstance          This should be unic at least on one device. May be best to have it unic over all devices sending this PGN.
//  - PressureSource            see tN2kPressureSource
//  - Set pressure              Set pressure in Pascals. Use function mBarToPascal, if you like to use mBar
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130315(tN2kMsg &N2kMsg, unsigned char SID, unsigned char PressureInstance,
                     tN2kPressureSource PressureSource, double SetPressure);
inline void SetN2kSetPressure(tN2kMsg &N2kMsg, unsigned char SID, unsigned char PressureInstance,
                           tN2kPressureSource PressureSource, double SetPressure) {
  SetN2kPGN130315(N2kMsg, SID, PressureInstance, PressureSource, SetPressure);
}

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
// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130316(tN2kMsg &N2kMsg, unsigned char SID, unsigned char TempInstance, tN2kTempSource TempSource,
                     double ActualTemperature, double SetTemperature=N2kDoubleNA);

inline void SetN2kTemperatureExt(tN2kMsg &N2kMsg, unsigned char SID, unsigned char TempInstance, tN2kTempSource TempSource,
                     double ActualTemperature, double SetTemperature=N2kDoubleNA) {
  SetN2kPGN130316(N2kMsg,SID,TempInstance,TempSource,ActualTemperature,SetTemperature);
}

bool ParseN2kPGN130316(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature);
inline bool ParseN2kTemperatureExt(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature) {
  return ParseN2kPGN130316(N2kMsg, SID, TempInstance, TempSource, ActualTemperature, SetTemperature);
}

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
// Output:
//  - N2kMsg                NMEA2000 message ready to be sent.
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

void SetN2kPGN130323(tN2kMsg &N2kMsg, const tN2kMeteorlogicalStationData &N2kData);
inline void SetN2kMeteorlogicalStationData(tN2kMsg &N2kMsg, const tN2kMeteorlogicalStationData &N2kData) { SetN2kPGN130323(N2kMsg, N2kData); }

bool ParseN2kPGN130323(const tN2kMsg &N2kMsg, tN2kMeteorlogicalStationData &N2kData);
inline bool ParseN2kMeteorlogicalStationData(const tN2kMsg &N2kMsg, tN2kMeteorlogicalStationData &N2kData) {
  return ParseN2kPGN130323(N2kMsg, N2kData);
}

//*****************************************************************************
// Small Craft Status (Trim Tab Position)
// Trim tab position is a percentage 0 to 100% where 0 is fully retracted and 100 is fully extended
// Input:
//  - PortTrimTab           Port trim tab position
//  - StbdTrimTab           Starboard trim tab position

// Output:
//  - N2kMsg                NMEA2000 message ready to be send.
void SetN2kPGN130576(tN2kMsg &N2kMsg, int8_t PortTrimTab, int8_t StbdTrimTab);

inline void SetN2kTrimTab(tN2kMsg &N2kMsg, int8_t PortTrimTab, int8_t StbdTrimTab){

  SetN2kPGN130576(N2kMsg,PortTrimTab, StbdTrimTab);
}

bool ParseN2kPGN130576(const tN2kMsg &N2kMsg, int8_t &PortTrimTab, int8_t &StbdTrimTab);
inline bool ParseN2kTrimTab(const tN2kMsg &N2kMsg, int8_t &PortTrimTab, int8_t &StbdTrimTab) {
  return ParseN2kPGN130576(N2kMsg, PortTrimTab, StbdTrimTab);
}

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
// Output:
//  - N2kMsg                  NMEA2000 message ready to be send.
void SetN2kPGN130577(tN2kMsg &N2kMsg, tN2kDataMode DataMode, tN2kHeadingReference CogReference, unsigned char SID, double COG, double SOG,
      double Heading, double SpeedThroughWater, double Set, double Drift);

inline void SetN2kDirectionData(tN2kMsg &N2kMsg, tN2kDataMode DataMode, tN2kHeadingReference CogReference, unsigned char SID, double COG, double SOG,
      double Heading, double SpeedThroughWater, double Set, double Drift){

   SetN2kPGN130577(N2kMsg,DataMode,CogReference,SID,COG,SOG,Heading,SpeedThroughWater,Set,Drift);
}

bool ParseN2kPGN130577(const tN2kMsg &N2kMsg,tN2kDataMode &DataMode, tN2kHeadingReference &CogReference,unsigned char &SID,double &COG,
      double &SOG,double &Heading,double &SpeedThroughWater,double &Set,double &Drift);

inline bool ParseN2kDirectionData(const tN2kMsg &N2kMsg,tN2kDataMode &DataMode, tN2kHeadingReference &CogReference,unsigned char &SID,double &COG,
      double &SOG,double &Heading,double &SpeedThroughWater,double &Set,double &Drift) {

   return ParseN2kPGN130577(N2kMsg,DataMode,CogReference,SID,COG,SOG,Heading,SpeedThroughWater,Set,Drift);
}


bool ParseN2kPGN129793(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Longitude, double &Latitude, unsigned int &SecondsSinceMidnight,
                        unsigned int &DaysSinceEpoch);

#endif
