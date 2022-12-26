/*
N2kMessages.cpp

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
#include "N2kMessages.h"
#include <string.h>

//*****************************************************************************
// System time
void SetN2kPGN126992(tN2kMsg &N2kMsg, unsigned char SID, uint16_t SystemDate,
                     double SystemTime, tN2kTimeSource TimeSource) {
    N2kMsg.SetPGN(126992L);
    N2kMsg.Priority=3;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte((TimeSource & 0x0f) | 0xf0);
    N2kMsg.Add2ByteUInt(SystemDate);
    N2kMsg.Add4ByteUDouble(SystemTime,0.0001);
}

//*****************************************************************************
bool ParseN2kPGN126992(const tN2kMsg &N2kMsg, unsigned char &SID, uint16_t &SystemDate,
                     double &SystemTime, tN2kTimeSource &TimeSource) {
  if (N2kMsg.PGN!=126992L) return false;

  int Index=0;

  SID=N2kMsg.GetByte(Index);
  TimeSource=(tN2kTimeSource)(N2kMsg.GetByte(Index) & 0x0f);
  SystemDate=N2kMsg.Get2ByteUInt(Index);
  SystemTime=N2kMsg.Get4ByteUDouble(0.0001,Index);

  return true;
}

//*****************************************************************************
// AIS Safety Related Broadcast Message
void SetN2kPGN129802(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t SourceID,
      tN2kAISTransceiverInformation AISTransceiverInformation, char * SafetyRelatedText)
{
   N2kMsg.SetPGN(129802L);
   N2kMsg.Priority=5;
   N2kMsg.AddByte((Repeat & 0x03)<<6 | (MessageID & 0x3f));
   N2kMsg.Add4ByteUInt(0xc0000000 | (SourceID & 0x3fffffff));
   N2kMsg.AddByte(0xe0 | (0x1f & AISTransceiverInformation));
   N2kMsg.AddVarStr(SafetyRelatedText);
}

bool ParseN2kPGN129802(tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &SourceID,
      tN2kAISTransceiverInformation &AISTransceiverInformation, char * SafetyRelatedText, size_t &SafetyRelatedTextMaxSize)
{
   if (N2kMsg.PGN!=129802L) return false;

   int Index=0;
   unsigned char vb;

   vb=N2kMsg.GetByte(Index);
   MessageID=(vb & 0x3f);
   Repeat=(tN2kAISRepeat)(vb>>6 & 0x03);
   SourceID = N2kMsg.Get4ByteUInt(Index) & 0x3fffffff;
   AISTransceiverInformation = (tN2kAISTransceiverInformation)(N2kMsg.GetByte(Index) & 0x1f);
   N2kMsg.GetVarStr(SafetyRelatedTextMaxSize, SafetyRelatedText, Index);

   return true;
}


//*****************************************************************************
// Man Overboard Notification
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
      tN2kMOBEmitterBatteryStatus MOBEmitterBatteryStatus)
{
  N2kMsg.SetPGN(127233L);
  N2kMsg.Priority=3;
  N2kMsg.AddByte(SID);
  N2kMsg.Add4ByteUInt(MobEmitterId);
  N2kMsg.AddByte((MOBStatus & 0x07) | 0xf8);
  N2kMsg.Add4ByteUDouble(ActivationTime,0.0001);
  N2kMsg.AddByte((PositionSource & 0x07) | 0xf8);
  N2kMsg.Add2ByteUInt(PositionDate);
  N2kMsg.Add4ByteUDouble(PositionTime,0.0001);
  N2kMsg.Add4ByteDouble(Latitude,1e-7);
  N2kMsg.Add4ByteDouble(Longitude,1e-7);
  N2kMsg.AddByte((COGReference & 0x03) | 0xfc);
  N2kMsg.Add2ByteUDouble(COG,0.0001);
  N2kMsg.Add2ByteUDouble(SOG,0.01);
  N2kMsg.Add4ByteUInt(MMSI);
  N2kMsg.AddByte((MOBEmitterBatteryStatus & 0x07) | 0xf8);
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
      tN2kMOBEmitterBatteryStatus &MOBEmitterBatteryStatus)
{
  if (N2kMsg.PGN!=127233L) return false;
  int Index = 0;
  SID = N2kMsg.GetByte(Index);
  MobEmitterId = N2kMsg.Get4ByteUInt(Index);
  MOBStatus = (tN2kMOBStatus)(N2kMsg.GetByte(Index) & 0x07);
  ActivationTime = N2kMsg.Get4ByteUDouble(0.0001,Index);
  PositionSource = (tN2kMOBPositionSource)(N2kMsg.GetByte(Index) & 0x07);
  PositionDate = N2kMsg.Get2ByteUInt(Index);
  PositionTime = N2kMsg.Get4ByteUDouble(0.0001,Index);
  Latitude = N2kMsg.Get4ByteDouble(1e-7,Index);
  Longitude = N2kMsg.Get4ByteDouble(1e-7,Index);
  COGReference = (tN2kHeadingReference)(N2kMsg.GetByte(Index) & 0x03);
  COG = N2kMsg.Get2ByteUDouble(0.0001,Index);
  SOG = N2kMsg.Get2ByteUDouble(0.01,Index);
  MMSI = N2kMsg.Get4ByteUInt(Index);
  MOBEmitterBatteryStatus = (tN2kMOBEmitterBatteryStatus)(N2kMsg.GetByte(Index) & 0x07);
  return true;
}


//*****************************************************************************
// Heading/Track control
// Angles should be in radians
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
      double VesselHeading) {

  N2kMsg.SetPGN(127237L);
  N2kMsg.Priority=2;
  N2kMsg.AddByte(
        (0x03 & RudderLimitExceeded)           |
        (0x03 & OffHeadingLimitExceeded)  << 2 |
        (0x03 & OffTrackLimitExceeded)    << 4 |
        (0x03 & Override)                 << 6
  );
  N2kMsg.AddByte(
        (0x07 & SteeringMode)                  |
        (0x07 & TurnMode)                 << 3 |
        (0x03 & HeadingReference)         << 6
  );
  N2kMsg.AddByte(
        0x1f                                   |
        (0x07 & CommandedRudderDirection) << 5
  );
  N2kMsg.Add2ByteDouble(CommandedRudderAngle,0.0001);
  N2kMsg.Add2ByteUDouble(HeadingToSteerCourse,0.0001);
  N2kMsg.Add2ByteUDouble(Track,0.0001);
  N2kMsg.Add2ByteUDouble(RudderLimit,0.0001);
  N2kMsg.Add2ByteUDouble(OffHeadingLimit,0.0001);
  N2kMsg.Add2ByteDouble(RadiusOfTurnOrder,1);
  N2kMsg.Add2ByteDouble(RateOfTurnOrder,3.125e-5);
  N2kMsg.Add2ByteDouble(OffTrackLimit,1);
  N2kMsg.Add2ByteUDouble(VesselHeading,0.0001);
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
      double &VesselHeading) {

  if (N2kMsg.PGN!=127237L) return false;
  int Index = 0;
  unsigned char v;
  v = N2kMsg.GetByte(Index);
  RudderLimitExceeded =      (tN2kOnOff)(v & 0x03);
  OffHeadingLimitExceeded =  (tN2kOnOff)((v >> 2) & 0x03);
  OffTrackLimitExceeded =    (tN2kOnOff)((v >> 4) & 0x03);
  Override =                 (tN2kOnOff)((v >> 6) & 0x03);
  v = N2kMsg.GetByte(Index);
  SteeringMode =             (tN2kSteeringMode)(v & 0x07);
  TurnMode =                 (tN2kTurnMode)((v >> 3) & 0x07);
  HeadingReference =         (tN2kHeadingReference)((v >> 6) & 0x03);
  CommandedRudderDirection = (tN2kRudderDirectionOrder)((N2kMsg.GetByte(Index) >> 5) & 0x07);
  CommandedRudderAngle = N2kMsg.Get2ByteDouble(0.0001,Index);
  HeadingToSteerCourse = N2kMsg.Get2ByteUDouble(0.0001,Index);
  Track = N2kMsg.Get2ByteUDouble(0.0001,Index);
  RudderLimit = N2kMsg.Get2ByteUDouble(0.0001,Index);
  OffHeadingLimit = N2kMsg.Get2ByteUDouble(0.0001,Index);
  RadiusOfTurnOrder = N2kMsg.Get2ByteDouble(1,Index);
  RateOfTurnOrder = N2kMsg.Get2ByteDouble(3.125e-5,Index);
  OffTrackLimit = N2kMsg.Get2ByteDouble(1,Index);
  VesselHeading = N2kMsg.Get2ByteUDouble(0.0001,Index);
  return true;
}

//*****************************************************************************
// Rudder
// Angles should be in radians
void SetN2kPGN127245(tN2kMsg &N2kMsg, double RudderPosition, unsigned char Instance,
                     tN2kRudderDirectionOrder RudderDirectionOrder, double AngleOrder) {
    N2kMsg.SetPGN(127245L);
    N2kMsg.Priority=2;
    N2kMsg.AddByte(Instance);
    N2kMsg.AddByte(0xf8 | (RudderDirectionOrder&0x07));
    N2kMsg.Add2ByteDouble(AngleOrder,0.0001);
    N2kMsg.Add2ByteDouble(RudderPosition,0.0001);
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN127245(const tN2kMsg &N2kMsg, double &RudderPosition, unsigned char &Instance,
                     tN2kRudderDirectionOrder &RudderDirectionOrder, double &AngleOrder) {
  if (N2kMsg.PGN!=127245L) return false;

  int Index=0;
  Instance=N2kMsg.GetByte(Index);
  RudderDirectionOrder=(tN2kRudderDirectionOrder)(N2kMsg.GetByte(Index)&0x7);
  AngleOrder=N2kMsg.Get2ByteDouble(0.0001,Index);
  RudderPosition=N2kMsg.Get2ByteDouble(0.0001,Index);
  return true;
}

//*****************************************************************************
// Vessel Heading
// Angles should be in radians
void SetN2kPGN127250(tN2kMsg &N2kMsg, unsigned char SID, double Heading, double Deviation, double Variation, tN2kHeadingReference ref) {
    N2kMsg.SetPGN(127250L);
    N2kMsg.Priority=2;
    N2kMsg.AddByte(SID);
    N2kMsg.Add2ByteUDouble(Heading,0.0001);
    N2kMsg.Add2ByteDouble(Deviation,0.0001);
    N2kMsg.Add2ByteDouble(Variation,0.0001);
    N2kMsg.AddByte(0xfc | ref);
}

bool ParseN2kPGN127250(const tN2kMsg &N2kMsg, unsigned char &SID, double &Heading, double &Deviation, double &Variation, tN2kHeadingReference &ref) {
  if (N2kMsg.PGN!=127250L) return false;

  int Index=0;

  SID=N2kMsg.GetByte(Index);
  Heading=N2kMsg.Get2ByteUDouble(0.0001,Index);
  Deviation=N2kMsg.Get2ByteDouble(0.0001,Index);
  Variation=N2kMsg.Get2ByteDouble(0.0001,Index);
  ref=(tN2kHeadingReference)(N2kMsg.GetByte(Index)&0x03);

  return true;
}

//*****************************************************************************
// Rate of turn
// Angles should be in radians
void SetN2kPGN127251(tN2kMsg &N2kMsg, unsigned char SID, double RateOfTurn) {
    N2kMsg.SetPGN(127251L);
    N2kMsg.Priority=2;
    N2kMsg.AddByte(SID);
    N2kMsg.Add4ByteDouble(RateOfTurn,3.125E-08); //1e-6/32.0
    N2kMsg.AddByte(0xff);
    N2kMsg.Add2ByteUInt(0xffff);
}

bool ParseN2kPGN127251(const tN2kMsg &N2kMsg, unsigned char &SID, double &RateOfTurn) {
  if (N2kMsg.PGN!=127251L) return false;

  int Index=0;

  SID=N2kMsg.GetByte(Index);
  RateOfTurn=N2kMsg.Get4ByteDouble(3.125E-08,Index); //1e-6/32.0

  return true;
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
void SetN2kPGN127257(tN2kMsg &N2kMsg, unsigned char SID, double Yaw, double Pitch, double Roll) {
    N2kMsg.SetPGN(127257L);
    N2kMsg.Priority=3;
    N2kMsg.AddByte(SID);
    N2kMsg.Add2ByteDouble(Yaw,0.0001);
    N2kMsg.Add2ByteDouble(Pitch,0.0001);
    N2kMsg.Add2ByteDouble(Roll,0.0001);
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN127257(const tN2kMsg &N2kMsg, unsigned char &SID, double &Yaw, double &Pitch, double &Roll){
  if (N2kMsg.PGN!=127257L) return false;

  int Index=0;
  SID=N2kMsg.GetByte(Index);
  Yaw=N2kMsg.Get2ByteDouble(0.0001,Index);
  Pitch=N2kMsg.Get2ByteDouble(0.0001,Index);
  Roll=N2kMsg.Get2ByteDouble(0.0001,Index);

  return true;
}

//*****************************************************************************
// Magnetic variation
void SetN2kPGN127258(tN2kMsg &N2kMsg, unsigned char SID, tN2kMagneticVariation Source, uint16_t DaysSince1970, double Variation) {
  N2kMsg.SetPGN(127258L);
  N2kMsg.Priority=6;
  N2kMsg.AddByte(SID);
  N2kMsg.AddByte(Source & 0x0f);
  N2kMsg.Add2ByteUInt(DaysSince1970);
  N2kMsg.Add2ByteDouble(Variation, 0.0001);
  N2kMsg.Add2ByteUInt(0xffff);
}

bool ParseN2kPGN127258(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kMagneticVariation &Source, uint16_t &DaysSince1970, double &Variation) {
  if (N2kMsg.PGN!=127258L) return false;

  int Index=0;
  SID=N2kMsg.GetByte(Index);
  Source=(tN2kMagneticVariation) (N2kMsg.GetByte(Index) & 0x0f);
  DaysSince1970=N2kMsg.Get2ByteUInt(Index);
  Variation=N2kMsg.Get2ByteDouble(0.0001, Index);

  return true;
}

//*****************************************************************************
// Engine rapid param
void SetN2kPGN127488(tN2kMsg &N2kMsg, unsigned char EngineInstance, double EngineSpeed,
                     double EngineBoostPressure, int8_t EngineTiltTrim) {
    N2kMsg.SetPGN(127488L);
    N2kMsg.Priority=2;
    N2kMsg.AddByte(EngineInstance);
    N2kMsg.Add2ByteUDouble(EngineSpeed,0.25);
    N2kMsg.Add2ByteUDouble(EngineBoostPressure, 100);
    N2kMsg.AddByte(EngineTiltTrim);
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN127488(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineSpeed,
                     double &EngineBoostPressure, int8_t &EngineTiltTrim) {
  if (N2kMsg.PGN!=127488L) return false;

  int Index=0;

  EngineInstance=N2kMsg.GetByte(Index);
  EngineSpeed=N2kMsg.Get2ByteUDouble(0.25,Index);
  EngineBoostPressure=N2kMsg.Get2ByteUDouble(100,Index);
  EngineTiltTrim=N2kMsg.GetByte(Index);

  return true;
}

#if 0 //DSR
//*****************************************************************************
// Engine parameters dynamic
void SetN2kPGN127489(tN2kMsg &N2kMsg, unsigned char EngineInstance, double EngineOilPress, double EngineOilTemp, double EngineCoolantTemp, double AltenatorVoltage,
                       double FuelRate, double EngineHours, double EngineCoolantPress, double EngineFuelPress, int8_t EngineLoad, int8_t EngineTorque,
                       tN2kEngineDiscreteStatus1 Status1, tN2kEngineDiscreteStatus2 Status2) {
    N2kMsg.SetPGN(127489L);
    N2kMsg.Priority=2;

  N2kMsg.AddByte(EngineInstance);
  N2kMsg.Add2ByteUDouble(EngineOilPress, 100);
  N2kMsg.Add2ByteUDouble(EngineOilTemp, 0.1);
  N2kMsg.Add2ByteUDouble(EngineCoolantTemp, 0.01);
  N2kMsg.Add2ByteDouble(AltenatorVoltage, 0.01);
  N2kMsg.Add2ByteDouble(FuelRate, 0.1);
  N2kMsg.Add4ByteUDouble(EngineHours, 1);
  N2kMsg.Add2ByteUDouble(EngineCoolantPress, 100);
  N2kMsg.Add2ByteUDouble(EngineFuelPress, 1000);
  N2kMsg.AddByte(0xff);  // reserved

  N2kMsg.Add2ByteUInt(Status1.Status); // Discrete Status 1
  N2kMsg.Add2ByteUInt(Status2.Status);  // Discrete Status 2

  N2kMsg.AddByte(EngineLoad);
  N2kMsg.AddByte(EngineTorque);
}
bool ParseN2kPGN127489(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineOilPress,
                      double &EngineOilTemp, double &EngineCoolantTemp, double &AltenatorVoltage,
                      double &FuelRate, double &EngineHours, double &EngineCoolantPress, double &EngineFuelPress,
                      int8_t &EngineLoad, int8_t &EngineTorque,
                      tN2kEngineDiscreteStatus1 &Status1, tN2kEngineDiscreteStatus2 &Status2) {
  if (N2kMsg.PGN != 127489L) return false;

  int Index = 0;

  EngineInstance = N2kMsg.GetByte(Index);
  EngineOilPress = N2kMsg.Get2ByteUDouble(100, Index);
  EngineOilTemp  = N2kMsg.Get2ByteUDouble(0.1, Index);
  EngineCoolantTemp = N2kMsg.Get2ByteUDouble(0.01, Index);
  AltenatorVoltage = N2kMsg.Get2ByteDouble(0.01, Index);
  FuelRate =  N2kMsg.Get2ByteDouble(0.1, Index);
  EngineHours = N2kMsg.Get4ByteUDouble(1, Index);
  EngineCoolantPress=N2kMsg.Get2ByteUDouble(100, Index);
  EngineFuelPress=N2kMsg.Get2ByteUDouble(1000, Index);
  N2kMsg.GetByte(Index);  // reserved
  Status1=N2kMsg.Get2ByteUInt(Index);  // Discrete Status 1
  Status2=N2kMsg.Get2ByteUInt(Index);  // Discrete Status 2
  EngineLoad=N2kMsg.GetByte(Index);
  EngineTorque=N2kMsg.GetByte(Index);

  return true;
}

#endif

//*****************************************************************************
// Transmission parameters, dynamic
void SetN2kPGN127493(tN2kMsg &N2kMsg, unsigned char EngineInstance, tN2kTransmissionGear TransmissionGear,
                     double OilPressure, double OilTemperature, unsigned char DiscreteStatus1) {
  N2kMsg.SetPGN(127493L);
  N2kMsg.Priority=2;
  N2kMsg.AddByte(EngineInstance);
  N2kMsg.AddByte((TransmissionGear & 0x03) | 0xfc );
  N2kMsg.Add2ByteUDouble(OilPressure, 100);
  N2kMsg.Add2ByteUDouble(OilTemperature, 0.1);
  N2kMsg.AddByte(DiscreteStatus1);
  N2kMsg.AddByte(0xff);  // Reserved
}

bool ParseN2kPGN127493(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, tN2kTransmissionGear &TransmissionGear,
                     double &OilPressure, double &OilTemperature, unsigned char &DiscreteStatus1) {
  if (N2kMsg.PGN!=127493L) return false;

  int Index=0;

  EngineInstance=N2kMsg.GetByte(Index);
  TransmissionGear=(tN2kTransmissionGear)(N2kMsg.GetByte(Index) & 0x03);
  OilPressure=N2kMsg.Get2ByteUDouble(100,Index);
  OilTemperature=N2kMsg.Get2ByteUDouble(0.1,Index);
  DiscreteStatus1=N2kMsg.GetByte(Index);

  return true;
}

//*****************************************************************************
// Trip Parameters, Engine
void SetN2kPGN127497(tN2kMsg &N2kMsg, unsigned char EngineInstance, double TripFuelUsed,
                     double FuelRateAverage,
                     double FuelRateEconomy, double InstantaneousFuelEconomy) {
  N2kMsg.SetPGN(127497L);
  N2kMsg.Priority=2;
  N2kMsg.AddByte(EngineInstance);
  N2kMsg.Add2ByteUDouble(TripFuelUsed,1);
  N2kMsg.Add2ByteDouble(FuelRateAverage, 0.1);
  N2kMsg.Add2ByteDouble(FuelRateEconomy, 0.1);
  N2kMsg.Add2ByteDouble(InstantaneousFuelEconomy, 0.1);
}

bool ParseN2kPGN127497(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &TripFuelUsed,
                     double &FuelRateAverage,
                     double &FuelRateEconomy, double &InstantaneousFuelEconomy) {
  if (N2kMsg.PGN!=127497L) return false;

  int Index=0;

  EngineInstance=N2kMsg.GetByte(Index);
  TripFuelUsed=N2kMsg.Get2ByteUDouble(1,Index);
  FuelRateAverage=N2kMsg.Get2ByteDouble(0.1,Index);
  FuelRateEconomy=N2kMsg.Get2ByteDouble(0.1,Index);
  InstantaneousFuelEconomy=N2kMsg.Get2ByteDouble(0.1,Index);

  return true;
}

//*****************************************************************************
// Binary status

//*****************************************************************************
tN2kOnOff N2kGetStatusOnBinaryStatus(tN2kBinaryStatus BankStatus, uint8_t ItemIndex) {
	ItemIndex--;
	if (ItemIndex>27) return N2kOnOff_Unavailable;

	return (tN2kOnOff)((BankStatus >> (2*ItemIndex)) & 0x03);
}

//*****************************************************************************
void N2kSetStatusBinaryOnStatus(tN2kBinaryStatus &BankStatus, tN2kOnOff ItemStatus, uint8_t ItemIndex) {
	ItemIndex--;
	if (ItemIndex>27) return;

  tN2kBinaryStatus Mask = ~(3 << (2*ItemIndex));

	BankStatus = (BankStatus & Mask) | (ItemStatus << (2*ItemIndex));
}

//*****************************************************************************
void SetN2kPGN127501(tN2kMsg &N2kMsg, unsigned char DeviceBankInstance, tN2kBinaryStatus BankStatus) {
    N2kMsg.SetPGN(127501L);
    N2kMsg.Priority=3;
	BankStatus = (BankStatus << 8) | DeviceBankInstance;
	N2kMsg.AddUInt64(BankStatus);
}

//*****************************************************************************
// Binary status report
void SetN2kPGN127501(tN2kMsg &N2kMsg, unsigned char DeviceBankInstance
                      ,tN2kOnOff Status1
                      ,tN2kOnOff Status2
                      ,tN2kOnOff Status3
                      ,tN2kOnOff Status4
                    ) {
  tN2kBinaryStatus BankStatus;

    N2kResetBinaryStatus(BankStatus);
	BankStatus = (BankStatus << 2) | Status4;
	BankStatus = (BankStatus << 2) | Status3;
	BankStatus = (BankStatus << 2) | Status2;
	BankStatus = (BankStatus << 2) | Status1;
	SetN2kPGN127501(N2kMsg,DeviceBankInstance,BankStatus);
}

//*****************************************************************************
bool ParseN2kPGN127501(const tN2kMsg &N2kMsg, unsigned char &DeviceBankInstance
                      ,tN2kOnOff &Status1
                      ,tN2kOnOff &Status2
                      ,tN2kOnOff &Status3
                      ,tN2kOnOff &Status4
                    ) {
  if (N2kMsg.PGN!=127501L) return false;

  int Index=0;
  DeviceBankInstance=N2kMsg.GetByte(Index);
  unsigned char b=N2kMsg.GetByte(Index);
  Status1=(tN2kOnOff)(b & 0x03);
  b>>=2; Status2=(tN2kOnOff)(b & 0x03);
  b>>=2; Status3=(tN2kOnOff)(b & 0x03);
  b>>=2; Status4=(tN2kOnOff)(b & 0x03);

  return true;
}

//*****************************************************************************
bool ParseN2kPGN127501(const tN2kMsg &N2kMsg, unsigned char &DeviceBankInstance, tN2kBinaryStatus &BankStatus) {
  if (N2kMsg.PGN!=127501L) return false;

  int Index=0;
  BankStatus=N2kMsg.GetUInt64(Index);
  DeviceBankInstance = BankStatus & 0xff;
  BankStatus>>=8;

  return true;
}

//*****************************************************************************
// Fluid level
void SetN2kPGN127505(tN2kMsg &N2kMsg, unsigned char Instance, tN2kFluidType FluidType, double Level, double Capacity) {
    N2kMsg.SetPGN(127505L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte((Instance&0x0f) | ((FluidType&0x0f)<<4));
    N2kMsg.Add2ByteDouble(Level,0.004);
    N2kMsg.Add4ByteUDouble(Capacity,0.1);
    N2kMsg.AddByte(0xff); // Reserved
}

//*****************************************************************************
bool ParseN2kPGN127505(const tN2kMsg &N2kMsg, unsigned char &Instance, tN2kFluidType &FluidType, double &Level, double &Capacity) {
  if (N2kMsg.PGN!=127505L) return false;

  int Index=0;
  unsigned char IFt=N2kMsg.GetByte(Index);

  Instance=IFt&0x0f;
  FluidType=(tN2kFluidType)((IFt>>4)&0x0f);
  Level=N2kMsg.Get2ByteDouble(0.004,Index);
  Capacity=N2kMsg.Get4ByteUDouble(0.1,Index);

  return true;
}

//*****************************************************************************
// DC Detailed Status
//
void SetN2kPGN127506(tN2kMsg &N2kMsg, unsigned char SID, unsigned char DCInstance, tN2kDCType DCType,
                     uint8_t StateOfCharge, uint8_t StateOfHealth, double TimeRemaining, double RippleVoltage, double Capacity) {
    N2kMsg.SetPGN(127506L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte(DCInstance);
    N2kMsg.AddByte((unsigned char)DCType);
    N2kMsg.AddByte(StateOfCharge);
    N2kMsg.AddByte(StateOfHealth);
    N2kMsg.Add2ByteUDouble(TimeRemaining,60);
    N2kMsg.Add2ByteUDouble(RippleVoltage,0.001);
    N2kMsg.Add2ByteUDouble(Capacity,3600);
}

//*****************************************************************************
bool ParseN2kPGN127506(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &DCInstance, tN2kDCType &DCType,
                     uint8_t &StateOfCharge, uint8_t &StateOfHealth, double &TimeRemaining, double &RippleVoltage, double &Capacity){
  if (N2kMsg.PGN!=127506L) return false;
  int Index=0;
  SID=N2kMsg.GetByte(Index);
  DCInstance=N2kMsg.GetByte(Index);
  DCType=(tN2kDCType)(N2kMsg.GetByte(Index));
  StateOfCharge=N2kMsg.GetByte(Index);
  StateOfHealth=N2kMsg.GetByte(Index);
  TimeRemaining=N2kMsg.Get2ByteUDouble(60,Index);
  RippleVoltage=N2kMsg.Get2ByteUDouble(0.001,Index);
  Capacity=N2kMsg.Get2ByteUDouble(3600,Index);

  return true;
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
                     tN2kChargeState ChargeState, tN2kChargerMode ChargerMode,
                     tN2kOnOff Enabled, tN2kOnOff EqualizationPending, double EqualizationTimeRemaining) {
    N2kMsg.SetPGN(127507UL);
    N2kMsg.Priority=6;
    N2kMsg.AddByte(Instance);
    N2kMsg.AddByte(BatteryInstance);
    N2kMsg.AddByte((ChargerMode & 0x0f)<<4 | (ChargeState & 0x0f));
    N2kMsg.AddByte(0x0f<<4 | (EqualizationPending & 0x03) << 2 | (Enabled & 0x03));
    N2kMsg.Add2ByteUDouble(EqualizationTimeRemaining,1);
}

bool ParseN2kPGN127507(tN2kMsg &N2kMsg, unsigned char &Instance, unsigned char &BatteryInstance,
                     tN2kChargeState &ChargeState, tN2kChargerMode &ChargerMode,
                     tN2kOnOff &Enabled, tN2kOnOff &EqualizationPending, double &EqualizationTimeRemaining) {
  if (N2kMsg.PGN!=127507UL) return false;
  int Index=0;
  Instance=N2kMsg.GetByte(Index);
  BatteryInstance=N2kMsg.GetByte(Index);
  unsigned char v;
  v=N2kMsg.GetByte(Index);
  ChargeState=(tN2kChargeState)(v&0x0f);
  ChargerMode=(tN2kChargerMode)((v>>4)&0x0f);
  v=N2kMsg.GetByte(Index);
  Enabled=(tN2kOnOff)(v&0x03);
  EqualizationPending=(tN2kOnOff)((v>>2)&0x03);
  EqualizationTimeRemaining=N2kMsg.Get2ByteUDouble(60,Index);

  return true;
}

//*****************************************************************************
// Battery Status
// Temperatures should be in Kelvins
void SetN2kPGN127508(tN2kMsg &N2kMsg, unsigned char BatteryInstance, double BatteryVoltage, double BatteryCurrent,
                     double BatteryTemperature, unsigned char SID) {
    N2kMsg.SetPGN(127508L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte(BatteryInstance);
    N2kMsg.Add2ByteDouble(BatteryVoltage,0.01);
    N2kMsg.Add2ByteDouble(BatteryCurrent,0.1);
    N2kMsg.Add2ByteUDouble(BatteryTemperature,0.01);
    N2kMsg.AddByte(SID);
}

//*****************************************************************************
bool ParseN2kPGN127508(const tN2kMsg &N2kMsg, unsigned char &BatteryInstance, double &BatteryVoltage, double &BatteryCurrent,
                     double &BatteryTemperature, unsigned char &SID) {
  if (N2kMsg.PGN!=127508L) return false;
  int Index=0;
  BatteryInstance=N2kMsg.GetByte(Index);
  BatteryVoltage=N2kMsg.Get2ByteDouble(0.01,Index);
  BatteryCurrent=N2kMsg.Get2ByteDouble(0.1,Index);
  BatteryTemperature=N2kMsg.Get2ByteUDouble(0.01,Index);
  SID=N2kMsg.GetByte(Index);

  return true;
}

//*****************************************************************************
// Battery Configuration Status
void SetN2kPGN127513(tN2kMsg &N2kMsg, unsigned char BatInstance, tN2kBatType BatType, tN2kBatEqSupport SupportsEqual,
                     tN2kBatNomVolt BatNominalVoltage, tN2kBatChem BatChemistry, double BatCapacity, int8_t BatTemperatureCoefficient,
				double PeukertExponent, int8_t ChargeEfficiencyFactor) {
    N2kMsg.SetPGN(127513L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte(BatInstance);
    N2kMsg.AddByte(0xc0 | ((SupportsEqual & 0x03) << 4) | (BatType & 0x0f)); // BatType (4 bit), SupportsEqual (2 bit), Reserved (2 bit)
    N2kMsg.AddByte( ((BatChemistry & 0x0f) << 4) | (BatNominalVoltage & 0x0f) ); // BatNominalVoltage (4 bit), BatChemistry (4 bit)
    N2kMsg.Add2ByteUDouble(BatCapacity,3600);
    N2kMsg.AddByte((int8_t)BatTemperatureCoefficient);
    PeukertExponent-=1; // Is this right or not I am not yet sure!
    if (PeukertExponent<0 || PeukertExponent>0.504) { N2kMsg.AddByte(0xff); } else { N2kMsg.Add1ByteUDouble(PeukertExponent,0.002,-1); }
    N2kMsg.AddByte((int8_t)ChargeEfficiencyFactor);
}

//*****************************************************************************
bool ParseN2kPGN127513(const tN2kMsg &N2kMsg, unsigned char &BatInstance, tN2kBatType &BatType, tN2kBatEqSupport &SupportsEqual,
                     tN2kBatNomVolt &BatNominalVoltage, tN2kBatChem &BatChemistry, double &BatCapacity, int8_t &BatTemperatureCoefficient,
				double &PeukertExponent, int8_t &ChargeEfficiencyFactor) {
  if (N2kMsg.PGN!=127513L) return false;
  int Index=0;
  unsigned char v;
  BatInstance = N2kMsg.GetByte(Index);
  v = N2kMsg.GetByte(Index); BatType=(tN2kBatType)(v & 0x0f); SupportsEqual=(tN2kBatEqSupport)((v>>4) & 0x03);
  v = N2kMsg.GetByte(Index); BatNominalVoltage=(tN2kBatNomVolt)(v & 0x0f);  BatChemistry=(tN2kBatChem)((v>>4) & 0x0f);
  BatCapacity=N2kMsg.Get2ByteDouble(3600,Index);
  BatTemperatureCoefficient=N2kMsg.GetByte(Index);
  PeukertExponent=N2kMsg.Get1ByteUDouble(0.002,Index); PeukertExponent+=1;
  ChargeEfficiencyFactor=N2kMsg.GetByte(Index);

  return true;
}

//*****************************************************************************
// Leeway
void SetN2kPGN128000(tN2kMsg &N2kMsg, unsigned char SID, double Leeway) {
    N2kMsg.SetPGN(128000L);
    N2kMsg.Priority=4;
    N2kMsg.AddByte(SID);
    N2kMsg.Add2ByteDouble(Leeway,0.0001);
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN128000(const tN2kMsg &N2kMsg, unsigned char &SID, double &Leeway) {
  if (N2kMsg.PGN!=128000L) return false;

  int Index=0;

  SID=N2kMsg.GetByte(Index);
  Leeway=N2kMsg.Get2ByteDouble(0.0001,Index);

  return true;
}

//*****************************************************************************
// Boat speed
void SetN2kPGN128259(tN2kMsg &N2kMsg, unsigned char SID, double WaterReferenced, double GroundReferenced, tN2kSpeedWaterReferenceType SWRT) {
    N2kMsg.SetPGN(128259L);
    N2kMsg.Priority=2;
    N2kMsg.AddByte(SID);
    N2kMsg.Add2ByteUDouble(WaterReferenced,0.01);
    N2kMsg.Add2ByteUDouble(GroundReferenced,0.01);
    N2kMsg.AddByte(SWRT);
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN128259(const tN2kMsg &N2kMsg, unsigned char &SID, double &WaterReferenced, double &GroundReferenced, tN2kSpeedWaterReferenceType &SWRT) {
  if (N2kMsg.PGN!=128259L) return false;

  int Index=0;

  SID=N2kMsg.GetByte(Index);
  WaterReferenced=N2kMsg.Get2ByteUDouble(0.01,Index);
  GroundReferenced=N2kMsg.Get2ByteUDouble(0.01,Index);
  SWRT=(tN2kSpeedWaterReferenceType)(N2kMsg.GetByte(Index)&0x0F);

  return true;
}

//*****************************************************************************
// Water depth
void SetN2kPGN128267(tN2kMsg &N2kMsg, unsigned char SID, double DepthBelowTransducer, double Offset, double Range) {
    N2kMsg.SetPGN(128267L);
    N2kMsg.Priority=3;
    N2kMsg.AddByte(SID);
    N2kMsg.Add4ByteUDouble(DepthBelowTransducer,0.01);
    N2kMsg.Add2ByteDouble(Offset,0.001);
    N2kMsg.Add1ByteUDouble(Range,10);
}

bool ParseN2kPGN128267(const tN2kMsg &N2kMsg, unsigned char &SID, double &DepthBelowTransducer, double &Offset, double &Range) {
  if (N2kMsg.PGN!=128267L) return false;

  int Index=0;
  SID=N2kMsg.GetByte(Index);
  DepthBelowTransducer=N2kMsg.Get4ByteUDouble(0.01,Index);
  Offset=N2kMsg.Get2ByteDouble(0.001,Index);
  Range=N2kMsg.Get1ByteUDouble(10,Index);

  return true;
}

//*****************************************************************************
// Distance log
void SetN2kPGN128275(tN2kMsg &N2kMsg, uint16_t DaysSince1970, double SecondsSinceMidnight, uint32_t Log, uint32_t TripLog) {
    N2kMsg.SetPGN(128275L);
    N2kMsg.Priority=6;
    N2kMsg.Add2ByteUInt(DaysSince1970);
    N2kMsg.Add4ByteUDouble(SecondsSinceMidnight,0.0001);
    N2kMsg.Add4ByteUInt(Log);
    N2kMsg.Add4ByteUInt(TripLog);
}

bool ParseN2kPGN128275(const tN2kMsg &N2kMsg, uint16_t &DaysSince1970, double &SecondsSinceMidnight, uint32_t &Log, uint32_t &TripLog) {
    if (N2kMsg.PGN!=128275L) return false;

    int Index=0;

    DaysSince1970=N2kMsg.Get2ByteUInt(Index);
    SecondsSinceMidnight=N2kMsg.Get4ByteUDouble(0.0001,Index);
    Log=N2kMsg.Get4ByteUDouble(1,Index);
    TripLog=N2kMsg.Get4ByteUDouble(1,Index);

    return true;
}


//*****************************************************************************
// PGN128776 - Windlass Control Status
//

void SetN2kPGN128776(
  tN2kMsg &N2kMsg,
  unsigned char SID,
  unsigned char WindlassIdentifier,
  tN2kWindlassDirectionControl WindlassDirectionControl,
  unsigned char SpeedControl,
  tN2kSpeedType SpeedControlType,
  tN2kGenericStatusPair AnchorDockingControl,
  tN2kGenericStatusPair PowerEnable,
  tN2kGenericStatusPair MechanicalLock,
  tN2kGenericStatusPair DeckAndAnchorWash,
  tN2kGenericStatusPair AnchorLight,
  double CommandTimeout,
  const tN2kWindlassControlEvents &WindlassControlEvents
){
  N2kMsg.SetPGN(128776L);
  N2kMsg.Priority=2;
  N2kMsg.AddByte(SID);
  N2kMsg.AddByte(WindlassIdentifier);
  N2kMsg.AddByte((unsigned char) ((0x03 << 6) | ((SpeedControlType & 0x03) << 4) | ((AnchorDockingControl & 0x03) << 2) | (WindlassDirectionControl & 0x03)));
  N2kMsg.AddByte(SpeedControl);
  N2kMsg.AddByte((unsigned char) (((AnchorLight & 0x03) << 6) | ((DeckAndAnchorWash & 0x03) << 4) | ((MechanicalLock & 0x03) << 2) | (PowerEnable & 0x03)));
  N2kMsg.Add1ByteUDouble(CommandTimeout, 0.005);
  N2kMsg.AddByte(WindlassControlEvents.Events);
}

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
) {
  if (N2kMsg.PGN!=128776L) return false;
  int Index = 0;
  unsigned char field;
  SID = N2kMsg.GetByte(Index);
  WindlassIdentifier = N2kMsg.GetByte(Index);
  field = N2kMsg.GetByte(Index);
  WindlassDirectionControl = (tN2kWindlassDirectionControl) (field & 0x03);
  AnchorDockingControl = (tN2kGenericStatusPair) ((field >> 2) & 0x03);
  SpeedControlType = (tN2kSpeedType) ((field >> 4) & 0x03);
  SpeedControl = N2kMsg.GetByte(Index);
  field = N2kMsg.GetByte(Index);
  PowerEnable = (tN2kGenericStatusPair) (field & 0x03);
  MechanicalLock = (tN2kGenericStatusPair) ((field >> 2) & 0x03);
  DeckAndAnchorWash = (tN2kGenericStatusPair) ((field >> 4) & 0x03);
  AnchorLight = (tN2kGenericStatusPair) ((field >> 6) & 0x03);
  CommandTimeout = N2kMsg.Get1ByteUDouble(0.005, Index);
  WindlassControlEvents.SetEvents(N2kMsg.GetByte(Index));
  return true;
}

//*****************************************************************************
// PGN128777 Windlass Operating Status
//

void SetN2kPGN128777(
  tN2kMsg &N2kMsg,
  unsigned char SID,
  unsigned char WindlassIdentifier,
  double RodeCounterValue,
  double WindlassLineSpeed,
  tN2kWindlassMotionStates WindlassMotionStatus,
  tN2kRodeTypeStates RodeTypeStatus,
  tN2kAnchorDockingStates AnchorDockingStatus,
  const tN2kWindlassOperatingEvents &WindlassOperatingEvents
){
  N2kMsg.SetPGN(128777L);
  N2kMsg.Priority=2;
  N2kMsg.AddByte(SID);
  N2kMsg.AddByte(WindlassIdentifier);
  N2kMsg.AddByte((unsigned char) (0xF0 | ((RodeTypeStatus & 0x03) << 2) | (WindlassMotionStatus & 0x03)));
  N2kMsg.Add2ByteUDouble(RodeCounterValue, 0.1);
  N2kMsg.Add2ByteUDouble(WindlassLineSpeed, 0.01);
  N2kMsg.AddByte((WindlassOperatingEvents.Events << 2) | (AnchorDockingStatus & 0x03));
}

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
){
  if (N2kMsg.PGN!=128777L) return false;
  int Index = 0;
  unsigned char field;
  SID = N2kMsg.GetByte(Index);
  WindlassIdentifier = N2kMsg.GetByte(Index);
  field = N2kMsg.GetByte(Index);
  WindlassMotionStatus = (tN2kWindlassMotionStates) (field & 0x03);
  RodeTypeStatus = (tN2kRodeTypeStates) ((field >> 2) & 0x03);
  RodeCounterValue = N2kMsg.Get2ByteUDouble(0.1, Index);
  WindlassLineSpeed = N2kMsg.Get2ByteUDouble(0.01, Index);
  field = N2kMsg.GetByte(Index);
  AnchorDockingStatus = (tN2kAnchorDockingStates) (field & 0x03);
  WindlassOperatingEvents.SetEvents(field >> 2);
  return true;
}

//*****************************************************************************
// PGN128778 - Windlass Monitoring Status
//

void SetN2kPGN128778(
  tN2kMsg &N2kMsg,
  unsigned char SID,
  unsigned char WindlassIdentifier,
  double TotalMotorTime,
  double ControllerVoltage,
  double MotorCurrent,
  const tN2kWindlassMonitoringEvents &WindlassMonitoringEvents
){
  N2kMsg.SetPGN(128778L);
  N2kMsg.Priority=2;
  N2kMsg.AddByte(SID);
  N2kMsg.AddByte(WindlassIdentifier);
  N2kMsg.AddByte(WindlassMonitoringEvents.Events);
  N2kMsg.Add1ByteUDouble(ControllerVoltage, 0.2);
  N2kMsg.Add1ByteUDouble(MotorCurrent, 1.0);
  N2kMsg.Add2ByteUDouble(TotalMotorTime, 60.0);
  N2kMsg.AddByte(0xFF);
}

bool ParseN2kPGN128778(
  const tN2kMsg &N2kMsg,
  unsigned char &SID,
  unsigned char &WindlassIdentifier,
  double &TotalMotorTime,
  double &ControllerVoltage,
  double &MotorCurrent,
  tN2kWindlassMonitoringEvents &WindlassMonitoringEvents
) {
  if (N2kMsg.PGN!=128778L) return false;
  int Index = 0;
  SID = N2kMsg.GetByte(Index);
  WindlassIdentifier = N2kMsg.GetByte(Index);
  WindlassMonitoringEvents.SetEvents(N2kMsg.GetByte(Index));
  ControllerVoltage = N2kMsg.Get1ByteUDouble(0.2, Index);
  MotorCurrent = N2kMsg.Get1ByteUDouble(1.0, Index);
  TotalMotorTime = N2kMsg.Get2ByteUDouble(60.0, Index);
  return true;
}

//*****************************************************************************
// Lat long rapid
void SetN2kPGN129025(tN2kMsg &N2kMsg, double Latitude, double Longitude) {
    N2kMsg.SetPGN(129025L);
    N2kMsg.Priority=2;
    N2kMsg.Add4ByteDouble(Latitude,1e-7);
    N2kMsg.Add4ByteDouble(Longitude,1e-7);
}

bool ParseN2kPGN129025(const tN2kMsg &N2kMsg, double &Latitude, double &Longitude) {
	if (N2kMsg.PGN!=129025L) return false;

	int Index = 0;
	Latitude=N2kMsg.Get4ByteDouble(1e-7, Index);
	Longitude=N2kMsg.Get4ByteDouble(1e-7, Index);
	return true;
}
//*****************************************************************************
// COG SOG rapid
// COG should be in radians
// SOG should be in m/s
void SetN2kPGN129026(tN2kMsg &N2kMsg, unsigned char SID, tN2kHeadingReference ref, double COG, double SOG) {
    N2kMsg.SetPGN(129026L);
    N2kMsg.Priority=2;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte( (((unsigned char)(ref)) & 0x03) | 0xfc );
    N2kMsg.Add2ByteUDouble(COG,0.0001); //0.0057295779513082332);
    N2kMsg.Add2ByteUDouble(SOG,0.01);
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN129026(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kHeadingReference &ref, double &COG, double &SOG) {
  if (N2kMsg.PGN!=129026L) return false;
  int Index=0;
  unsigned char b;

  SID=N2kMsg.GetByte(Index);
  b=N2kMsg.GetByte(Index); ref=(tN2kHeadingReference)( b & 0x03 );
  COG=N2kMsg.Get2ByteUDouble(0.0001,Index);
  SOG=N2kMsg.Get2ByteUDouble(0.01,Index);

  return true;
}

//*****************************************************************************
// GNSS Position Data
void SetN2kPGN129029(tN2kMsg &N2kMsg, unsigned char SID, uint16_t DaysSince1970, double SecondsSinceMidnight,
                     double Latitude, double Longitude, double Altitude,
                     tN2kGNSStype GNSStype, tN2kGNSSmethod GNSSmethod,
                     unsigned char nSatellites, double HDOP, double PDOP, double GeoidalSeparation,
                     unsigned char nReferenceStations, tN2kGNSStype ReferenceStationType, uint16_t ReferenceSationID,
                     double AgeOfCorrection
                     ) {


    N2kMsg.SetPGN(129029L);
    N2kMsg.Priority=3;
    N2kMsg.AddByte(SID);
    N2kMsg.Add2ByteUInt(DaysSince1970);
    N2kMsg.Add4ByteUDouble(SecondsSinceMidnight,0.0001);
    N2kMsg.Add8ByteDouble(Latitude,1e-16);
    N2kMsg.Add8ByteDouble(Longitude,1e-16);
    N2kMsg.Add8ByteDouble(Altitude,1e-6);
    N2kMsg.AddByte( (((unsigned char) GNSStype) & 0x0f) | (((unsigned char) GNSSmethod) & 0x0f)<<4 );
    N2kMsg.AddByte(1 | 0xfc);  // Integrity 2 bit, reserved 6 bits
    N2kMsg.AddByte(nSatellites);
    N2kMsg.Add2ByteDouble(HDOP,0.01);
    N2kMsg.Add2ByteDouble(PDOP,0.01);
    N2kMsg.Add4ByteDouble(GeoidalSeparation,0.01);
    if (nReferenceStations!=0xff && nReferenceStations>0) {
      N2kMsg.AddByte(1); // Note that we have values for only one reference station, so pass only one values.
      N2kMsg.Add2ByteInt( (((int)ReferenceStationType) & 0x0f) | ReferenceSationID<<4 );
      N2kMsg.Add2ByteUDouble(AgeOfCorrection,0.01);
    } else N2kMsg.AddByte(nReferenceStations);
}

bool ParseN2kPGN129029(const tN2kMsg &N2kMsg, unsigned char &SID, uint16_t &DaysSince1970, double &SecondsSinceMidnight,
                     double &Latitude, double &Longitude, double &Altitude,
                     tN2kGNSStype &GNSStype, tN2kGNSSmethod &GNSSmethod,
                     uint8_t &nSatellites, double &HDOP, double &PDOP, double &GeoidalSeparation,
                     uint8_t &nReferenceStations, tN2kGNSStype &ReferenceStationType, uint16_t &ReferenceSationID,
                     double &AgeOfCorrection
                     ) {
  if (N2kMsg.PGN!=129029L) return false;
  int Index=0;
  unsigned char vb;
  int16_t vi;

  SID=N2kMsg.GetByte(Index);
  DaysSince1970=N2kMsg.Get2ByteUInt(Index);
  SecondsSinceMidnight=N2kMsg.Get4ByteUDouble(0.0001,Index);
  Latitude=N2kMsg.Get8ByteDouble(1e-16,Index);
  Longitude=N2kMsg.Get8ByteDouble(1e-16,Index);
  Altitude=N2kMsg.Get8ByteDouble(1e-6,Index);
  vb=N2kMsg.GetByte(Index); GNSStype=(tN2kGNSStype)(vb & 0x0f); GNSSmethod=(tN2kGNSSmethod)((vb>>4) & 0x0f);
  vb=N2kMsg.GetByte(Index);  // Integrity 2 bit, reserved 6 bits
  nSatellites=N2kMsg.GetByte(Index);
  HDOP=N2kMsg.Get2ByteDouble(0.01,Index);
  PDOP=N2kMsg.Get2ByteDouble(0.01,Index);
  GeoidalSeparation=N2kMsg.Get4ByteDouble(0.01,Index);
  nReferenceStations=N2kMsg.GetByte(Index);
  if (nReferenceStations!=N2kUInt8NA && nReferenceStations>0) {
    // Note that we return real number of stations, but we only have variabes for one.
    vi=N2kMsg.Get2ByteUInt(Index); ReferenceStationType=(tN2kGNSStype)(vi & 0x0f); ReferenceSationID=(vi>>4);
    AgeOfCorrection=N2kMsg.Get2ByteUDouble(0.01,Index);
  }

  return true;
}

//*****************************************************************************
// Date,Time & Local offset
void SetN2kPGN129033(tN2kMsg &N2kMsg, uint16_t DaysSince1970, double SecondsSinceMidnight, int16_t LocalOffset) {
    N2kMsg.SetPGN(129033L);
    N2kMsg.Priority=3;
    N2kMsg.Add2ByteUInt(DaysSince1970);
    N2kMsg.Add4ByteUDouble(SecondsSinceMidnight,0.0001);
    N2kMsg.Add2ByteInt(LocalOffset);
}

bool ParseN2kPGN129033(const tN2kMsg &N2kMsg, uint16_t &DaysSince1970, double &SecondsSinceMidnight, int16_t &LocalOffset) {
    if ( N2kMsg.PGN != 129033L ) return false;

    int Index = 0;

    DaysSince1970 = N2kMsg.Get2ByteUInt(Index);
    SecondsSinceMidnight = N2kMsg.Get4ByteUDouble(0.0001, Index);
    LocalOffset = N2kMsg.Get2ByteInt(Index);
    return true;
}

//*****************************************************************************
// GNSS DOP data
void SetN2kPGN129539(tN2kMsg& N2kMsg, unsigned char SID, tN2kGNSSDOPmode DesiredMode, tN2kGNSSDOPmode ActualMode,
                     double HDOP, double VDOP, double TDOP)
{
    N2kMsg.SetPGN(129539L);
    N2kMsg.Priority = 6;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte(((DesiredMode & 0x07) << 5) | ((ActualMode & 0x07) << 2));
    N2kMsg.Add2ByteDouble(HDOP, 0.01);
    N2kMsg.Add2ByteDouble(VDOP, 0.01);
    N2kMsg.Add2ByteDouble(TDOP, 0.01);
}

bool ParseN2kPgn129539(const tN2kMsg& N2kMsg, unsigned char& SID, tN2kGNSSDOPmode& DesiredMode, tN2kGNSSDOPmode& ActualMode,
                       double& HDOP, double& VDOP, double& TDOP)
{
    if(N2kMsg.PGN != 129539L)
        return false;

    unsigned char modes;
    int Index = 0;

    SID = N2kMsg.GetByte(Index);
    modes = N2kMsg.GetByte(Index);
    DesiredMode = (tN2kGNSSDOPmode)((modes >> 5) & 0x07);
    ActualMode = (tN2kGNSSDOPmode)(modes & 0x07);
    HDOP = N2kMsg.Get2ByteDouble(0.01, Index);
    VDOP = N2kMsg.Get2ByteDouble(0.01, Index);
    TDOP = N2kMsg.Get2ByteDouble(0.01, Index);
    return true;
}

//*****************************************************************************
// GNSS Satellites in View

#define MaxSatelliteInfoCount 18 // Fast packet can hold this max stellites. Maybe later more with TP message.

void SetN2kPGN129540(tN2kMsg& N2kMsg, unsigned char SID, tN2kRangeResidualMode Mode) {
    N2kMsg.SetPGN(129540L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte(0xfc || Mode);
    N2kMsg.AddByte(0); // Init satellite count to 0
}

bool AppendN2kPGN129540(tN2kMsg& N2kMsg, const tSatelliteInfo& SatelliteInfo) {
  if(N2kMsg.PGN != 129540L) return false;

  int Index = 2;
  uint8_t NumberOfSVs=N2kMsg.GetByte(Index);

  if ( NumberOfSVs>=MaxSatelliteInfoCount ) return false;

  NumberOfSVs++;
  Index=2;
  N2kMsg.SetByte(NumberOfSVs, Index);  // increment the number satellites
  // add the new satellite info
  N2kMsg.AddByte(SatelliteInfo.PRN);
  N2kMsg.Add2ByteDouble(SatelliteInfo.Elevation,1e-4L);
  N2kMsg.Add2ByteUDouble(SatelliteInfo.Azimuth,1e-4L);
  N2kMsg.Add2ByteDouble(SatelliteInfo.SNR,1e-2L);
  N2kMsg.Add4ByteDouble(SatelliteInfo.RangeResiduals,1e-5L);
  N2kMsg.AddByte(0xf0 | SatelliteInfo.UsageStatus);

  return true;
}

bool ParseN2kPGN129540(const tN2kMsg& N2kMsg, unsigned char& SID, tN2kRangeResidualMode &Mode, uint8_t& NumberOfSVs) {
  if(N2kMsg.PGN != 129540L) return false;

  int Index = 0;

  SID = N2kMsg.GetByte(Index);
  Mode = (tN2kRangeResidualMode)(N2kMsg.GetByte(Index) & 0x03);
  NumberOfSVs = N2kMsg.GetByte(Index);

  return true;
}

bool ParseN2kPGN129540(const tN2kMsg& N2kMsg, uint8_t SVIndex, tSatelliteInfo& SatelliteInfo) {
  if(N2kMsg.PGN != 129540L) return false;

  int Index = 2;
  uint8_t NumberOfSVs=N2kMsg.GetByte(Index);
  bool ret=( NumberOfSVs<MaxSatelliteInfoCount && SVIndex<NumberOfSVs );

  if ( ret ) {
    Index=3+SVIndex*12;
    SatelliteInfo.PRN=N2kMsg.GetByte(Index);
    SatelliteInfo.Elevation=N2kMsg.Get2ByteDouble(1e-4L,Index);
    SatelliteInfo.Azimuth=N2kMsg.Get2ByteUDouble(1e-4L,Index);
    SatelliteInfo.SNR=N2kMsg.Get2ByteDouble(1e-2L,Index);
    SatelliteInfo.RangeResiduals=N2kMsg.Get4ByteDouble(1e-5L,Index);;
    SatelliteInfo.UsageStatus=(tN2kPRNUsageStatus)(N2kMsg.GetByte(Index) & 0x0f);
  } else {
    SatelliteInfo.PRN=0xff;
    SatelliteInfo.Elevation=N2kDoubleNA;
    SatelliteInfo.Azimuth=N2kDoubleNA;
    SatelliteInfo.SNR=N2kDoubleNA;
    SatelliteInfo.RangeResiduals=N2kDoubleNA;
    SatelliteInfo.UsageStatus=N2kDD124_Unavailable;
  }

  return ret;
}

//*****************************************************************************
// AIS position report (class A 129038)
// Latitude and Longitude in degrees (1e7)
// COG and Heading in radians (1e4)
void SetN2kPGN129038(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                        double Latitude, double Longitude, bool Accuracy, bool RAIM, uint8_t Seconds,
                        double COG, double SOG, double Heading, double ROT, tN2kAISNavStatus NavStatus)
{
    N2kMsg.SetPGN(129038L);
    N2kMsg.Priority=4;
    N2kMsg.AddByte((Repeat & 0x03)<<6 | (MessageID & 0x3f));
    N2kMsg.Add4ByteUInt(UserID);
    N2kMsg.Add4ByteDouble(Longitude, 1e-07);
    N2kMsg.Add4ByteDouble(Latitude, 1e-07);
    N2kMsg.AddByte((Seconds & 0x3f)<<2 | (RAIM & 0x01)<<1 | (Accuracy & 0x01));
    N2kMsg.Add2ByteUDouble(COG, 1e-04);
    N2kMsg.Add2ByteUDouble(SOG, 0.01);
    N2kMsg.AddByte(0xff); // Communication State (19 bits)
    N2kMsg.AddByte(0xff);
    N2kMsg.AddByte(0xff); // AIS transceiver information (5 bits)
    N2kMsg.Add2ByteUDouble(Heading, 1e-04);
    N2kMsg.Add2ByteDouble(ROT, 3.125E-05); // 1e-3/32.0
    N2kMsg.AddByte(0xF0 | (NavStatus & 0x0f));
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN129038(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM, uint8_t &Seconds,
                        double &COG, double &SOG, double &Heading, double &ROT, tN2kAISNavStatus &NavStatus,
                        tN2kAISTransceiverInformation &AISTransceiverInformation
                      )
{
    if (N2kMsg.PGN!=129038L) return false;

    int Index=0;
    unsigned char vb;

    vb=N2kMsg.GetByte(Index); MessageID=(vb & 0x3f); Repeat=(tN2kAISRepeat)(vb>>6 & 0x03);
    UserID=N2kMsg.Get4ByteUInt(Index);
    Longitude=N2kMsg.Get4ByteDouble(1e-07, Index);
    Latitude=N2kMsg.Get4ByteDouble(1e-07, Index);
    vb=N2kMsg.GetByte(Index); Accuracy=(vb & 0x01); RAIM=(vb>>1 & 0x01); Seconds=(vb>>2 & 0x3f);
    COG=N2kMsg.Get2ByteUDouble(1e-04, Index);
    SOG=N2kMsg.Get2ByteUDouble(0.01, Index);
    vb=N2kMsg.GetByte(Index); // Communication State (19 bits)
    vb=N2kMsg.GetByte(Index);
    vb=N2kMsg.GetByte(Index); // AIS transceiver information (5 bits)
    AISTransceiverInformation = (tN2kAISTransceiverInformation)((vb >> 3) & 0x1f);
    Heading=N2kMsg.Get2ByteUDouble(1e-04, Index);
    ROT=N2kMsg.Get2ByteDouble(3.125E-05, Index); // 1e-3/32.0
    vb=N2kMsg.GetByte(Index); NavStatus=(tN2kAISNavStatus)(vb & 0x0f);
    vb=N2kMsg.GetByte(Index); // Reserved

    return true;
}

//*****************************************************************************
// AIS position report (class B 129039)
void SetN2kPGN129039(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                        double Latitude, double Longitude, bool Accuracy, bool RAIM,
                        uint8_t Seconds, double COG, double SOG, tN2kAISTransceiverInformation AISTransceiverInformation,
                        double Heading, tN2kAISUnit Unit, bool Display, bool DSC, bool Band, bool Msg22, tN2kAISMode Mode,
                        bool State)
{
    N2kMsg.SetPGN(129039L);
    N2kMsg.Priority=4;
    N2kMsg.AddByte((Repeat & 0x03)<<6 | (MessageID & 0x3f));
    N2kMsg.Add4ByteUInt(UserID);
    N2kMsg.Add4ByteDouble(Longitude, 1e-07);
    N2kMsg.Add4ByteDouble(Latitude, 1e-07);
    N2kMsg.AddByte((Seconds & 0x3f)<<2 | (RAIM & 0x01)<<1 | (Accuracy & 0x01));
    N2kMsg.Add2ByteUDouble(COG, 1e-04);
    N2kMsg.Add2ByteUDouble(SOG, 0.01);
    N2kMsg.AddByte(0xff); // Communication State (19 bits)
    N2kMsg.AddByte(0xff);
    N2kMsg.AddByte(((0x1f & AISTransceiverInformation) << 3) | 0x7);
    N2kMsg.Add2ByteUDouble(Heading, 1e-04);
    N2kMsg.AddByte(0xff); // Regional application
    N2kMsg.AddByte((Mode & 0x01)<<7 | (Msg22 & 0x01)<<6 | (Band & 0x01)<<5 |
                    (DSC & 0x01)<<4 | (Display & 0x01)<<3 | (Unit & 0x01)<<2);
    N2kMsg.AddByte(0xfe | (State & 0x01));
}


bool ParseN2kPGN129039(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM,
                        uint8_t &Seconds, double &COG, double &SOG, tN2kAISTransceiverInformation &AISTransceiverInformation,
                        double &Heading, tN2kAISUnit &Unit, bool &Display, bool &DSC, bool &Band, bool &Msg22,
                        tN2kAISMode &Mode, bool &State)
{
    if (N2kMsg.PGN!=129039L) return false;

    int Index=0;
    unsigned char vb;

    vb=N2kMsg.GetByte(Index); MessageID=(vb & 0x3f); Repeat=(tN2kAISRepeat)(vb>>6 & 0x03);
    UserID=N2kMsg.Get4ByteUInt(Index);
    Longitude=N2kMsg.Get4ByteDouble(1e-07, Index);
    Latitude=N2kMsg.Get4ByteDouble(1e-07, Index);
    vb=N2kMsg.GetByte(Index); Accuracy=(vb & 0x01); RAIM=(vb>>1 & 0x01); Seconds=(vb>>2 & 0x3f);
    COG=N2kMsg.Get2ByteUDouble(1e-04, Index);
    SOG=N2kMsg.Get2ByteUDouble(0.01, Index);
    vb=N2kMsg.GetByte(Index); // Communication State (19 bits)
    vb=N2kMsg.GetByte(Index);
    vb=N2kMsg.GetByte(Index); // AIS transceiver information (5 bits)
    AISTransceiverInformation = (tN2kAISTransceiverInformation)((vb >> 3) & 0x1f);
    Heading=N2kMsg.Get2ByteUDouble(1e-04, Index);
    vb=N2kMsg.GetByte(Index); // Regional application
    vb=N2kMsg.GetByte(Index);
    Unit=(tN2kAISUnit)(vb>>2 & 0x01); Display=(vb>>3 & 0x01); DSC=(vb>>4 & 0x01);
    Band=(vb>>5 & 0x01); Msg22=(vb>>6 & 0x01); Mode=(tN2kAISMode)(vb>>7 & 0x01);
    vb=N2kMsg.GetByte(Index); State=(vb & 0x01);

    return true;
}

//*****************************************************************************
// AIS Aids to Navigation (AtoN) Report (PGN 129041)
void SetN2kPGN129041(tN2kMsg &N2kMsg, const tN2kAISAtoNReportData &N2kData) {
    N2kMsg.SetPGN(129041L);
    N2kMsg.Priority=4;
    N2kMsg.AddByte((N2kData.Repeat & 0x03) << 6 | (N2kData.MessageID & 0x3f));
    N2kMsg.Add4ByteUInt(N2kData.UserID);
    N2kMsg.Add4ByteDouble(N2kData.Longitude, 1e-07);
    N2kMsg.Add4ByteDouble(N2kData.Latitude, 1e-07);
    N2kMsg.AddByte((N2kData.Seconds & 0x3f)<<2 | (N2kData.RAIM & 0x01)<<1 | (N2kData.Accuracy & 0x01));
    N2kMsg.Add2ByteUDouble(N2kData.Length, 0.1);
    N2kMsg.Add2ByteUDouble(N2kData.Beam, 0.1);
    N2kMsg.Add2ByteUDouble(N2kData.PositionReferenceStarboard, 0.1);
    N2kMsg.Add2ByteUDouble(N2kData.PositionReferenceTrueNorth, 0.1);
    N2kMsg.AddByte((N2kData.AssignedModeFlag & 0x01) << 7
                    | (N2kData.VirtualAtoNFlag & 0x01) << 6
                    | (N2kData.OffPositionIndicator & 0x01) << 5
                    | (N2kData.AtoNType & 0x1f));
    N2kMsg.AddByte((N2kData.GNSSType & 0x0F) << 1 | 0xe0);
    N2kMsg.AddByte(N2kData.AtoNStatus);
    N2kMsg.AddByte((N2kData.AISTransceiverInformation & 0x1f) | 0xe0);
    N2kMsg.AddVarStr((char*)N2kData.AtoNName);
}


bool ParseN2kPGN129041(const tN2kMsg &N2kMsg, tN2kAISAtoNReportData &N2kData) {
    if (N2kMsg.PGN!=129041L) return false;

    int Index=0;
    unsigned char vb;
    vb=N2kMsg.GetByte(Index); N2kData.MessageID=(vb & 0x3f); N2kData.Repeat=(tN2kAISRepeat)(vb>>6 & 0x03);
    N2kData.UserID=N2kMsg.Get4ByteUInt(Index);
    N2kData.Longitude=N2kMsg.Get4ByteDouble(1e-07, Index);
    N2kData.Latitude=N2kMsg.Get4ByteDouble(1e-07, Index);
    vb=N2kMsg.GetByte(Index); N2kData.Accuracy=(vb & 0x01); N2kData.RAIM=(vb>>1 & 0x01); N2kData.Seconds=(vb>>2 & 0x3f);
    N2kData.Length=N2kMsg.Get2ByteDouble(0.1, Index);
    N2kData.Beam=N2kMsg.Get2ByteDouble(0.1, Index);
    N2kData.PositionReferenceStarboard=N2kMsg.Get2ByteDouble(0.1, Index);
    N2kData.PositionReferenceTrueNorth=N2kMsg.Get2ByteDouble(0.1, Index);
    vb=N2kMsg.GetByte(Index);
    N2kData.AtoNType=(tN2kAISAtoNType)(vb & 0x1f);
    N2kData.OffPositionIndicator=(vb >> 5  & 0x01);
    N2kData.VirtualAtoNFlag=(vb >> 6  & 0x01);
    N2kData.AssignedModeFlag=(vb >> 7 &  0x01);
    N2kData.GNSSType = (tN2kGNSStype)((N2kMsg.GetByte(Index) >> 1) & 0x0f);
    N2kData.AtoNStatus=N2kMsg.GetByte(Index);
    N2kData.AISTransceiverInformation = (tN2kAISTransceiverInformation)(N2kMsg.GetByte(Index) & 0x1f);
    size_t AtoNNameSize = sizeof(N2kData.AtoNName);
    N2kMsg.GetVarStr(AtoNNameSize, (char*)N2kData.AtoNName, Index);

    return true;
}

//*****************************************************************************
// Cross Track Error
void SetN2kPGN129283(tN2kMsg &N2kMsg, unsigned char SID, tN2kXTEMode XTEMode, bool NavigationTerminated, double XTE) {
    N2kMsg.SetPGN(129283L);
    N2kMsg.Priority=3;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte((char)XTEMode | (NavigationTerminated?0x40:0));
    N2kMsg.Add4ByteDouble(XTE,0.01);
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN129283(const tN2kMsg &N2kMsg, unsigned char& SID, tN2kXTEMode& XTEMode, bool& NavigationTerminated, double& XTE) {
    if(N2kMsg.PGN != 129283L)
        return false;

    int Index = 0;
    unsigned char c;
    SID = N2kMsg.GetByte(Index);
    c = N2kMsg.GetByte(Index);
    XTEMode = (tN2kXTEMode)(c & 0x0F);
    NavigationTerminated = c & 0x40;
    XTE = N2kMsg.Get4ByteDouble(0.01, Index);
    return true;
}

//*****************************************************************************
// Navigation info
void SetN2kPGN129284(tN2kMsg &N2kMsg, unsigned char SID, double DistanceToWaypoint, tN2kHeadingReference BearingReference,
                      bool PerpendicularCrossed, bool ArrivalCircleEntered, tN2kDistanceCalculationType CalculationType,
                      double ETATime, int16_t ETADate, double BearingOriginToDestinationWaypoint, double BearingPositionToDestinationWaypoint,
                      uint8_t OriginWaypointNumber, uint8_t DestinationWaypointNumber,
                      double DestinationLatitude, double DestinationLongitude, double WaypointClosingVelocity) {
    N2kMsg.SetPGN(129284L);
    N2kMsg.Priority=3;
    N2kMsg.AddByte(SID);
    N2kMsg.Add4ByteUDouble(DistanceToWaypoint,0.01);
    N2kMsg.AddByte((char)BearingReference | (PerpendicularCrossed?0x04:0) | (ArrivalCircleEntered?0x10:0)  | (CalculationType==N2kdct_RhumbLine?0x40:0));
    N2kMsg.Add4ByteUDouble(ETATime,0.0001);
    N2kMsg.Add2ByteUInt(ETADate);
    N2kMsg.Add2ByteUDouble(BearingOriginToDestinationWaypoint,0.0001);
    N2kMsg.Add2ByteUDouble(BearingPositionToDestinationWaypoint,0.0001);
    N2kMsg.Add4ByteUInt(OriginWaypointNumber);
    N2kMsg.Add4ByteUInt(DestinationWaypointNumber);
    N2kMsg.Add4ByteDouble(DestinationLatitude,1e-07);
    N2kMsg.Add4ByteDouble(DestinationLongitude,1e-07);
    N2kMsg.Add2ByteDouble(WaypointClosingVelocity,0.01);
}

bool ParseN2kPGN129284(const tN2kMsg &N2kMsg, unsigned char& SID, double& DistanceToWaypoint, tN2kHeadingReference& BearingReference,
                      bool& PerpendicularCrossed, bool& ArrivalCircleEntered, tN2kDistanceCalculationType& CalculationType,
                      double& ETATime, int16_t& ETADate, double& BearingOriginToDestinationWaypoint, double& BearingPositionToDestinationWaypoint,
                      uint8_t& OriginWaypointNumber, uint8_t& DestinationWaypointNumber,
                      double& DestinationLatitude, double& DestinationLongitude, double& WaypointClosingVelocity) {

    if(N2kMsg.PGN != 129284L)
      return false;

    int Index=0;
    unsigned char c;
    SID = N2kMsg.GetByte(Index);
    DistanceToWaypoint = N2kMsg.Get4ByteUDouble(0.01, Index);
    c = N2kMsg.GetByte(Index);
    BearingReference     = c & 0x01 ? N2khr_magnetic : N2khr_true;
    PerpendicularCrossed = c & 0x04;
    ArrivalCircleEntered = c & 0x10;
    CalculationType      = c & 0x40 ? N2kdct_RhumbLine : N2kdct_GreatCircle;
    ETATime = N2kMsg.Get4ByteUDouble(0.0001, Index);
    ETADate = N2kMsg.Get2ByteUInt(Index);
    BearingOriginToDestinationWaypoint = N2kMsg.Get2ByteUDouble(0.0001, Index);
    BearingPositionToDestinationWaypoint = N2kMsg.Get2ByteUDouble(0.0001, Index);
    OriginWaypointNumber = N2kMsg.Get4ByteUInt(Index);
    DestinationWaypointNumber = N2kMsg.Get4ByteUInt(Index);
    DestinationLatitude = N2kMsg.Get4ByteDouble(1e-07, Index);
    DestinationLongitude = N2kMsg.Get4ByteDouble(1e-07, Index);
    WaypointClosingVelocity = N2kMsg.Get2ByteDouble(0.01, Index);
    return true;
}

//*****************************************************************************
// Waypoint list
void SetN2kPGN129285(tN2kMsg &N2kMsg, uint16_t Start, uint16_t Database, uint16_t Route,
         tN2kNavigationDirection NavDirection, uint8_t SupplementaryData, char* RouteName) {
    unsigned int i;
    N2kMsg.SetPGN(129285L);
    N2kMsg.Priority=6;
    N2kMsg.Add2ByteUInt(Start);
    N2kMsg.Add2ByteUInt(0); // number of items initially 0
    N2kMsg.Add2ByteUInt(Database);
    N2kMsg.Add2ByteUInt(Route);
    N2kMsg.AddByte(0xC0 | (SupplementaryData & 0x03)<<4 | (NavDirection & 0x0F));
    if (strlen(RouteName) == 0) {
      N2kMsg.AddByte(0x03);N2kMsg.AddByte(0x01);N2kMsg.AddByte(0x00);
    } else {
      N2kMsg.AddByte(strlen(RouteName)+2);N2kMsg.AddByte(0x01);
      for (i=0; i<strlen(RouteName); i++)
        N2kMsg.AddByte(RouteName[i]);
    }
    N2kMsg.AddByte(0xff); // reserved
}

bool AppendN2kPGN129285(tN2kMsg &N2kMsg, uint16_t ID, char* Name, double Latitude, double Longitude) {
   if (N2kMsg.PGN!=129285L) return false;

    unsigned int i;
    int NumItemsIdx, len;
    uint16_t NumItems;

    if (strlen(Name) > 0) {
        len = 12 + strlen(Name);
    } else {
        len = 13;
    }

    if (N2kMsg.DataLen + len < N2kMsg.MaxDataLen) {
        NumItemsIdx = 2;
        NumItems = N2kMsg.Get2ByteUInt(NumItemsIdx);      // get and increment the number of items
        NumItemsIdx = 2;
        N2kMsg.Set2ByteUInt(++NumItems, NumItemsIdx);     // increment the number of items
        N2kMsg.Add2ByteUInt(ID);                          // add the new item
        if (strlen(Name) == 0) {
          N2kMsg.AddByte(0x03);N2kMsg.AddByte(0x01);N2kMsg.AddByte(0x00);
        } else {
          N2kMsg.AddByte(strlen(Name)+2);N2kMsg.AddByte(0x01);
          for (i=0; i<strlen(Name); i++)
            N2kMsg.AddByte(Name[i]);
        }
        N2kMsg.Add4ByteDouble(Latitude,1e-07);
        N2kMsg.Add4ByteDouble(Longitude,1e-07);
        return true;
    } else
        return false;
}

bool ParseN2kPGN129793(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        double &Longitude, double &Latitude, unsigned int &SecondsSinceMidnight,
                        unsigned int &DaysSinceEpoch)
{
    if (N2kMsg.PGN!=129793L) return false;

    int Index=0;
    unsigned char vb;

    vb=N2kMsg.GetByte(Index); MessageID=(vb & 0x3f); Repeat=(tN2kAISRepeat)(vb>>6 & 0x03);
    UserID=N2kMsg.Get4ByteUInt(Index);
    Longitude=N2kMsg.Get4ByteDouble(1e-07, Index);
    Latitude=N2kMsg.Get4ByteDouble(1e-07, Index);

    vb=N2kMsg.GetByte(Index);   //PositionAccuracy/RAIN/Reserved
    SecondsSinceMidnight=N2kMsg.Get4ByteUInt(Index);
    vb = N2kMsg.GetByte(Index);
    vb = N2kMsg.GetByte(Index);
    vb = N2kMsg.GetByte(Index);
    DaysSinceEpoch = N2kMsg.Get2ByteUInt(Index);

    return true;
}

//*****************************************************************************
// AIS static data A
void SetN2kPGN129794(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                        uint32_t IMOnumber, char *Callsign, char *Name, uint8_t VesselType, double Length,
                        double Beam, double PosRefStbd, double PosRefBow, uint16_t ETAdate, double ETAtime,
                        double Draught, char *Destination, tN2kAISVersion AISversion, tN2kGNSStype GNSStype,
                        tN2kAISDTE DTE, tN2kAISTranceiverInfo AISinfo)
{
    N2kMsg.SetPGN(129794L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte((Repeat & 0x03)<<6 | (MessageID & 0x3f));
    N2kMsg.Add4ByteUInt(UserID);
    N2kMsg.Add4ByteUInt(IMOnumber);
    N2kMsg.AddStr(Callsign, 7);
    N2kMsg.AddStr(Name, 20);
    N2kMsg.AddByte(VesselType);
    N2kMsg.Add2ByteDouble(Length, 0.1);
    N2kMsg.Add2ByteDouble(Beam, 0.1);
    N2kMsg.Add2ByteDouble(PosRefStbd, 0.1);
    N2kMsg.Add2ByteDouble(PosRefBow, 0.1);
    N2kMsg.Add2ByteUInt(ETAdate);
    N2kMsg.Add4ByteUDouble(ETAtime, 0.0001);
    N2kMsg.Add2ByteDouble(Draught, 0.01);
    N2kMsg.AddStr(Destination, 20);
    N2kMsg.AddByte((DTE & 0x01)<<6 | (GNSStype & 0x0f)<<2 | (AISversion & 0x03));
    N2kMsg.AddByte(0xe0 | (AISinfo & 0x1f));
    N2kMsg.AddByte(0xff);
}

bool ParseN2kPGN129794(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                        uint32_t &IMOnumber, char *Callsign, char *Name, uint8_t &VesselType, double &Length,
                        double &Beam, double &PosRefStbd, double &PosRefBow, uint16_t &ETAdate, double &ETAtime,
                        double &Draught, char *Destination, tN2kAISVersion &AISversion, tN2kGNSStype &GNSStype,
                        tN2kAISDTE &DTE, tN2kAISTranceiverInfo &AISinfo)
{
    if (N2kMsg.PGN!=129794L) return false;

    int Index=0;
    unsigned char vb;

    vb=N2kMsg.GetByte(Index); MessageID=(vb & 0x3f); Repeat=(tN2kAISRepeat)(vb>>6 & 0x03);
    UserID=N2kMsg.Get4ByteUInt(Index);
    IMOnumber=N2kMsg.Get4ByteUInt(Index);
    N2kMsg.GetStr(Callsign, 7, Index);
    N2kMsg.GetStr(Name, 20, Index);
    VesselType=N2kMsg.GetByte(Index);
    Length=N2kMsg.Get2ByteDouble(0.1, Index);
    Beam=N2kMsg.Get2ByteDouble(0.1, Index);
    PosRefStbd=N2kMsg.Get2ByteDouble(0.1, Index);
    PosRefBow=N2kMsg.Get2ByteDouble(0.1, Index);
    ETAdate=N2kMsg.Get2ByteUInt(Index);
    ETAtime=N2kMsg.Get4ByteUDouble(0.0001, Index);
    Draught=N2kMsg.Get2ByteDouble(0.01, Index);
    N2kMsg.GetStr(Destination, 20, Index);
    vb=N2kMsg.GetByte(Index); AISversion=(tN2kAISVersion)(vb & 0x03); GNSStype=(tN2kGNSStype)(vb>>2 & 0x0f); DTE=(tN2kAISDTE)(vb>>6 & 0x01);
    vb=N2kMsg.GetByte(Index); AISinfo=(tN2kAISTranceiverInfo)(vb & 0x1f);

    return true;
}

//*****************************************************************************
// AIS static data class B part A
void SetN2kPGN129809(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID, char *Name)
{
    N2kMsg.SetPGN(129809L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte((Repeat & 0x03)<<6 | (MessageID & 0x3f));
    N2kMsg.Add4ByteUInt(UserID);
    N2kMsg.AddStr(Name, 20);
}

bool ParseN2kPGN129809(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID, char *Name)
{
    if (N2kMsg.PGN!=129809L) return false;

    int Index=0;
    unsigned char vb;

    vb=N2kMsg.GetByte(Index); MessageID=(vb & 0x3f); Repeat=(tN2kAISRepeat)(vb>>6 & 0x03);
    UserID=N2kMsg.Get4ByteUInt(Index);
    N2kMsg.GetStr(Name, 20, Index);

    return true;
}

//*****************************************************************************
// AIS static data class B part B
void SetN2kPGN129810(tN2kMsg &N2kMsg, uint8_t MessageID, tN2kAISRepeat Repeat, uint32_t UserID,
                      uint8_t VesselType, char *Vendor, char *Callsign, double Length, double Beam,
                      double PosRefStbd, double PosRefBow, uint32_t MothershipID)
{
    N2kMsg.SetPGN(129810L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte((Repeat & 0x03)<<6 | (MessageID & 0x3f));
    N2kMsg.Add4ByteUInt(UserID);
    N2kMsg.AddByte(VesselType);
    N2kMsg.AddStr(Vendor, 7);
    N2kMsg.AddStr(Callsign, 7);
    N2kMsg.Add2ByteUDouble(Length, 0.1);
    N2kMsg.Add2ByteUDouble(Beam, 0.1);
    N2kMsg.Add2ByteUDouble(PosRefStbd, 0.1);
    N2kMsg.Add2ByteUDouble(PosRefBow, 0.1);
    N2kMsg.Add4ByteUInt(MothershipID);
    N2kMsg.AddByte(0xff);  // Reserved
}

bool ParseN2kPGN129810(const tN2kMsg &N2kMsg, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
                      uint8_t &VesselType, char *Vendor, char *Callsign, double &Length, double &Beam,
                      double &PosRefStbd, double &PosRefBow, uint32_t &MothershipID)
{
    if (N2kMsg.PGN!=129810L) return false;

    int Index=0;
    unsigned char vb;

    vb=N2kMsg.GetByte(Index); MessageID=(vb & 0x3f); Repeat=(tN2kAISRepeat)(vb>>6 & 0x03);
    UserID=N2kMsg.Get4ByteUInt(Index);
    VesselType=N2kMsg.GetByte(Index);
    N2kMsg.GetStr(Vendor, 7, Index);
    N2kMsg.GetStr(Callsign, 7, Index);
    Length = N2kMsg.Get2ByteUDouble(0.1, Index);
    Beam = N2kMsg.Get2ByteUDouble(0.1, Index);
    PosRefStbd = N2kMsg.Get2ByteUDouble(0.1, Index);
    PosRefBow = N2kMsg.Get2ByteUDouble(0.1, Index);
    MothershipID = N2kMsg.Get4ByteUInt(Index);

    return true;
}

//*****************************************************************************
// Waypoint list
void SetN2kPGN130074(tN2kMsg &N2kMsg, uint16_t Start, uint16_t NumWaypoints, uint16_t Database) {
    N2kMsg.SetPGN(130074L);
    N2kMsg.Priority=7;
    N2kMsg.Add2ByteUInt(Start);
    N2kMsg.Add2ByteUInt(0); // set number of items to 0 initially
    N2kMsg.Add2ByteUInt(NumWaypoints);
    N2kMsg.Add2ByteUInt(Database);
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
}

bool AppendN2kPGN130074(tN2kMsg &N2kMsg, uint16_t ID, char* Name, double Latitude, double Longitude) {
    if (N2kMsg.PGN!=130074L) return false;

    unsigned int i;
    int NumItemsIdx, len;
    uint16_t NumItems;

    if (strlen(Name) > 0)
        len = 12 + strlen(Name);
    else
        len = 13;

    if (N2kMsg.DataLen + len < N2kMsg.MaxDataLen) {
        NumItemsIdx = 2;
        NumItems = N2kMsg.Get2ByteUInt(NumItemsIdx);      // get and increment the number of items
        NumItemsIdx = 2;
        N2kMsg.Set2ByteUInt(++NumItems, NumItemsIdx);     // increment the number of items

        N2kMsg.Add2ByteUInt(ID);
        if (strlen(Name) == 0) {
          N2kMsg.AddByte(0x03);N2kMsg.AddByte(0x01);N2kMsg.AddByte(0x00);
        } else {
          N2kMsg.AddByte(strlen(Name)+2);N2kMsg.AddByte(0x01);
          for (i=0; i<strlen(Name); i++)
            N2kMsg.AddByte(Name[i]);
        }
        N2kMsg.Add4ByteDouble(Latitude,1e-07);
        N2kMsg.Add4ByteDouble(Longitude,1e-07);
        return true;
    } else
        return false;
}

//*****************************************************************************
// Wind Speed
void SetN2kPGN130306(tN2kMsg &N2kMsg, unsigned char SID, double WindSpeed, double WindAngle, tN2kWindReference WindReference) {
    N2kMsg.SetPGN(130306L);
    N2kMsg.Priority=2;
    N2kMsg.AddByte(SID);
    N2kMsg.Add2ByteUDouble(WindSpeed,0.01);
    N2kMsg.Add2ByteUDouble(WindAngle,0.0001);
    N2kMsg.AddByte((unsigned char)WindReference);
    N2kMsg.AddByte(0xff); // Reserved
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN130306(const tN2kMsg &N2kMsg, unsigned char &SID, double &WindSpeed, double &WindAngle, tN2kWindReference &WindReference) {
  if (N2kMsg.PGN!=130306L) return false;
  int Index=0;
  SID=N2kMsg.GetByte(Index);
  WindSpeed=N2kMsg.Get2ByteUDouble(0.01,Index);
  WindAngle=N2kMsg.Get2ByteUDouble(0.0001,Index);
  WindReference=(tN2kWindReference)(N2kMsg.GetByte(Index)&0x07);

  return true;
}

//*****************************************************************************
// Outside Environmental parameters
void SetN2kPGN130310(tN2kMsg &N2kMsg, unsigned char SID, double WaterTemperature,
                     double OutsideAmbientAirTemperature, double AtmosphericPressure) {
    N2kMsg.SetPGN(130310L);
    N2kMsg.Priority=5;
    N2kMsg.AddByte(SID);
    N2kMsg.Add2ByteUDouble(WaterTemperature,0.01);
    N2kMsg.Add2ByteUDouble(OutsideAmbientAirTemperature,0.01);
    N2kMsg.Add2ByteUDouble(AtmosphericPressure,100);
    N2kMsg.AddByte(0xff);  // reserved
}

bool ParseN2kPGN130310(const tN2kMsg &N2kMsg, unsigned char &SID, double &WaterTemperature,
                     double &OutsideAmbientAirTemperature, double &AtmosphericPressure) {
  if (N2kMsg.PGN!=130310L) return false;
  int Index=0;
  SID=N2kMsg.GetByte(Index);
  WaterTemperature=N2kMsg.Get2ByteUDouble(0.01,Index);
  OutsideAmbientAirTemperature=N2kMsg.Get2ByteUDouble(0.01,Index);
  AtmosphericPressure=N2kMsg.Get2ByteUDouble(100,Index);

  return true;
}


//*****************************************************************************
// Environmental parameters
void SetN2kPGN130311(tN2kMsg &N2kMsg, unsigned char SID, tN2kTempSource TempSource, double Temperature,
                     tN2kHumiditySource HumiditySource, double Humidity, double AtmosphericPressure) {
    N2kMsg.SetPGN(130311L);
    N2kMsg.Priority=5;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte(((HumiditySource) & 0x03)<<6 | (TempSource & 0x3f));
    N2kMsg.Add2ByteUDouble(Temperature,0.01);
    N2kMsg.Add2ByteDouble(Humidity,0.004);
    N2kMsg.Add2ByteUDouble(AtmosphericPressure,100);
}

bool ParseN2kPGN130311(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kTempSource &TempSource, double &Temperature,
                     tN2kHumiditySource &HumiditySource, double &Humidity, double &AtmosphericPressure) {
    if (N2kMsg.PGN!=130311L) return false;
    unsigned char vb;
    int Index=0;
    SID=N2kMsg.GetByte(Index);
    vb=N2kMsg.GetByte(Index); TempSource=(tN2kTempSource)(vb & 0x3f); HumiditySource=(tN2kHumiditySource)(vb>>6 & 0x03);
    Temperature=N2kMsg.Get2ByteUDouble(0.01,Index);
    Humidity=N2kMsg.Get2ByteDouble(0.004,Index);
    AtmosphericPressure=N2kMsg.Get2ByteUDouble(100,Index);

    return true;
}

//*****************************************************************************
// Temperature
// Temperatures should be in Kelvins
void SetN2kPGN130312(tN2kMsg &N2kMsg, unsigned char SID, unsigned char TempInstance, tN2kTempSource TempSource,
                     double ActualTemperature, double SetTemperature) {
    N2kMsg.SetPGN(130312L);
    N2kMsg.Priority=5;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte((unsigned char)TempInstance);
    N2kMsg.AddByte((unsigned char)TempSource);
    N2kMsg.Add2ByteUDouble(ActualTemperature,0.01);
    N2kMsg.Add2ByteUDouble(SetTemperature,0.01);
    N2kMsg.AddByte(0xff); // Reserved
}

bool ParseN2kPGN130312(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature) {
  if (N2kMsg.PGN!=130312L) return false;
  int Index=0;
  SID=N2kMsg.GetByte(Index);
  TempInstance=N2kMsg.GetByte(Index);
  TempSource=(tN2kTempSource)(N2kMsg.GetByte(Index));
  ActualTemperature=N2kMsg.Get2ByteUDouble(0.01,Index);
  SetTemperature=N2kMsg.Get2ByteUDouble(0.01,Index);

  return true;
}

//*****************************************************************************
// Humidity
// Humidity should be in percent
void SetN2kPGN130313(tN2kMsg &N2kMsg, unsigned char SID, unsigned char HumidityInstance,
                     tN2kHumiditySource HumiditySource, double ActualHumidity, double SetHumidity) {
  N2kMsg.SetPGN(130313L);
  N2kMsg.Priority = 5;
  N2kMsg.AddByte(SID);
  N2kMsg.AddByte((unsigned char) HumidityInstance);
  N2kMsg.AddByte((unsigned char) HumiditySource);
  N2kMsg.Add2ByteDouble(ActualHumidity, 0.004);
  N2kMsg.Add2ByteDouble(SetHumidity, 0.004);
  N2kMsg.AddByte(0xff); // reserved
}

bool ParseN2kPGN130313(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &HumidityInstance,
                       tN2kHumiditySource &HumiditySource, double &ActualHumidity, double &SetHumidity) {
  if (N2kMsg.PGN != 130313L) return false;
  int Index = 0;
  SID=N2kMsg.GetByte(Index);
  HumidityInstance=N2kMsg.GetByte(Index);
  HumiditySource=(tN2kHumiditySource)N2kMsg.GetByte(Index);
  ActualHumidity=N2kMsg.Get2ByteDouble(0.004, Index);
  SetHumidity=N2kMsg.Get2ByteDouble(0.004, Index);
  return true;
}

//*****************************************************************************
// Actual Pressure
// Pressure should be in Pascals
void SetN2kPGN130314(tN2kMsg &N2kMsg, unsigned char SID, unsigned char PressureInstance,
                     tN2kPressureSource PressureSource, double ActualPressure) {
  N2kMsg.SetPGN(130314L);
  N2kMsg.Priority = 5;
  N2kMsg.AddByte(SID);
  N2kMsg.AddByte((unsigned char) PressureInstance);
  N2kMsg.AddByte((unsigned char) PressureSource);
  N2kMsg.Add4ByteDouble(ActualPressure,0.1);
  N2kMsg.AddByte(0xff); // reserved
}

bool ParseN2kPGN130314(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &PressureInstance,
                       tN2kPressureSource &PressureSource, double &ActualPressure) {
  if (N2kMsg.PGN != 130314L) return false;
  int Index = 0;
  SID=N2kMsg.GetByte(Index);
  PressureInstance=N2kMsg.GetByte(Index);
  PressureSource=(tN2kPressureSource)N2kMsg.GetByte(Index);
  ActualPressure=N2kMsg.Get4ByteDouble(0.1, Index);
  return true;
}

//*****************************************************************************
// Set Pressure
// Pressure should be in Pascals
void SetN2kPGN130315(tN2kMsg &N2kMsg, unsigned char SID, unsigned char PressureInstance,
                     tN2kPressureSource PressureSource, double SetPressure) {
  N2kMsg.SetPGN(130315L);
  N2kMsg.Priority = 5;
  N2kMsg.AddByte(SID);
  N2kMsg.AddByte((unsigned char) PressureInstance);
  N2kMsg.AddByte((unsigned char) PressureSource);
  N2kMsg.Add4ByteDouble(SetPressure,0.1);
  N2kMsg.AddByte(0xff); // reserved
}

//*****************************************************************************
// Temperature extended range
// Temperatures should be in Kelvins
void SetN2kPGN130316(tN2kMsg &N2kMsg, unsigned char SID, unsigned char TempInstance, tN2kTempSource TempSource,
                     double ActualTemperature, double SetTemperature) {
    N2kMsg.SetPGN(130316L);
    N2kMsg.Priority=5;
    N2kMsg.AddByte(SID);
    N2kMsg.AddByte((unsigned char)TempInstance);
    N2kMsg.AddByte((unsigned char)TempSource);
    N2kMsg.Add3ByteDouble(ActualTemperature,0.001);
    N2kMsg.Add2ByteDouble(SetTemperature,0.1);
}

bool ParseN2kPGN130316(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature) {
  if (N2kMsg.PGN!=130316L) return false;
  int Index=0;
  SID=N2kMsg.GetByte(Index);
  TempInstance=N2kMsg.GetByte(Index);
  TempSource=(tN2kTempSource)(N2kMsg.GetByte(Index));
  ActualTemperature=N2kMsg.Get3ByteDouble(0.001,Index);
  SetTemperature=N2kMsg.Get2ByteDouble(0.1,Index);

  return true;
}

//*****************************************************************************
// Meteorological Station Data
void SetN2kPGN130323(tN2kMsg &N2kMsg, const tN2kMeteorlogicalStationData &N2kData) {
    N2kMsg.SetPGN(130323L);
    N2kMsg.Priority=6;
    N2kMsg.AddByte( (((unsigned char) N2kData.Mode) & 0x0f) | 0xf0 );
    N2kMsg.Add2ByteUInt(N2kData.SystemDate);
    N2kMsg.Add4ByteUDouble(N2kData.SystemTime,0.0001);
    N2kMsg.Add4ByteDouble(N2kData.Latitude,1e-7);
    N2kMsg.Add4ByteDouble(N2kData.Longitude,1e-7);
    N2kMsg.Add2ByteUDouble(N2kData.WindSpeed,0.01);
    N2kMsg.Add2ByteUDouble(N2kData.WindDirection,0.0001);
    N2kMsg.AddByte( (((unsigned char)N2kData.WindReference) & 0x07) | 0xf8 );
    N2kMsg.Add2ByteUDouble(N2kData.WindGusts,0.01);
    N2kMsg.Add2ByteUDouble(N2kData.AtmosphericPressure,100);
    N2kMsg.Add2ByteUDouble(N2kData.OutsideAmbientAirTemperature,0.01);
    N2kMsg.AddVarStr((char*)N2kData.StationID);
    N2kMsg.AddVarStr((char*)N2kData.StationName);
}


bool ParseN2kPGN130323(const tN2kMsg &N2kMsg, tN2kMeteorlogicalStationData &N2kData) {
    if (N2kMsg.PGN!=130323L) return false;
    int Index=0;
    unsigned char vb;

    vb = N2kMsg.GetByte(Index);
    N2kData.Mode=(tN2kAISMode)(vb & 0x0f);
    N2kData.SystemDate = N2kMsg.Get2ByteUInt(Index);
    N2kData.SystemTime = N2kMsg.Get4ByteUDouble(0.0001,Index);
    N2kData.Latitude = N2kMsg.Get4ByteDouble(1e-7,Index);
    N2kData.Longitude = N2kMsg.Get4ByteDouble(1e-7,Index);
    N2kData.WindSpeed = N2kMsg.Get2ByteUDouble(0.01,Index);
    N2kData.WindDirection = N2kMsg.Get2ByteUDouble(0.0001,Index);
    vb = N2kMsg.GetByte(Index);
    N2kData.WindReference = (tN2kWindReference)(vb & 0x07);
    N2kData.WindGusts = N2kMsg.Get2ByteUDouble(0.01,Index);
    N2kData.AtmosphericPressure = N2kMsg.Get2ByteUDouble(100,Index);
    N2kData.OutsideAmbientAirTemperature=N2kMsg.Get2ByteUDouble(0.01,Index);
    size_t StationIDSize = sizeof(N2kData.StationID);
    size_t StationNameSize = sizeof(N2kData.StationName);
    N2kMsg.GetVarStr(StationIDSize, (char*)N2kData.StationID, Index);
    N2kMsg.GetVarStr(StationNameSize, (char*)N2kData.StationName, Index);

    return true;
}

//*****************************************************************************
// Trim Tab Position
// Trim tab position is a percentage 0 to 100% where 0 is fully retracted and 100 is fully extended
void SetN2kPGN130576(tN2kMsg &N2kMsg, int8_t PortTrimTab, int8_t StbdTrimTab) {
    N2kMsg.SetPGN(130576L);
    N2kMsg.Priority=2;
    N2kMsg.AddByte(PortTrimTab);
    N2kMsg.AddByte(StbdTrimTab);
    N2kMsg.Add2ByteUInt(0xFFFF); // Reserved.
    N2kMsg.Add4ByteUInt(0xFFFFFFFF); // Reserved.
}

bool ParseN2kPGN130576(const tN2kMsg &N2kMsg, int8_t &PortTrimTab, int8_t &StbdTrimTab) {

  if (N2kMsg.PGN!=130576L) return false;
  int Index=0;
  PortTrimTab=N2kMsg.GetByte(Index);
  StbdTrimTab=N2kMsg.GetByte(Index);

  return true;
}

//*****************************************************************************
// Direction Data
void SetN2kPGN130577(tN2kMsg &N2kMsg,
      tN2kDataMode DataMode,
      tN2kHeadingReference CogReference,
      unsigned char SID,
      double COG,
      double SOG,
      double Heading,
      double SpeedThroughWater,
      double Set,
      double Drift
      )
{
  N2kMsg.SetPGN(130577L);
  N2kMsg.Priority=2;
  N2kMsg.AddByte(
        (0x0F & DataMode)          |
        (0x03 & CogReference) << 4 |
        0xc0
        );
  N2kMsg.AddByte(SID);
  N2kMsg.Add2ByteUDouble(COG,0.0001);
  N2kMsg.Add2ByteUDouble(SOG,0.01);
  N2kMsg.Add2ByteUDouble(Heading,0.0001);
  N2kMsg.Add2ByteUDouble(SpeedThroughWater,0.01);
  N2kMsg.Add2ByteUDouble(Set,0.0001);
  N2kMsg.Add2ByteUDouble(Drift,0.01);
}

bool ParseN2kPGN130577(const tN2kMsg &N2kMsg,
      tN2kDataMode &DataMode,
      tN2kHeadingReference &CogReference,
      unsigned char &SID,
      double &COG,
      double &SOG,
      double &Heading,
      double &SpeedThroughWater,
      double &Set,
      double &Drift)
{
  if (N2kMsg.PGN!=130577L) return false;
  int Index=0;
  unsigned char v;
  v = N2kMsg.GetByte(Index);
  DataMode = (tN2kDataMode)(v & 0x0F);
  CogReference = (tN2kHeadingReference)((v >> 4) & 0x03);
  SID = N2kMsg.GetByte(Index);
  COG = N2kMsg.Get2ByteUDouble(0.0001,Index);
  SOG = N2kMsg.Get2ByteUDouble(0.01, Index);
  Heading = N2kMsg.Get2ByteUDouble(0.0001, Index);
  SpeedThroughWater=  N2kMsg.Get2ByteUDouble(0.01, Index);
  Set = N2kMsg.Get2ByteUDouble(0.0001, Index);
  Drift = N2kMsg.Get2ByteUDouble(0.01, Index);
  return true;
}

