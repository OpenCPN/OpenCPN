/*
NMEA2000StdTypes.h

Copyright (c) 2019-2022 Timo Lappalainen, Kave Oy, www.kave.fi

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

NMEA2000 standard type definitions.

This file contains (and must only contain) type definitions of data types
defined in the NMEA2000 standard.  These all have names of the form "DDnnn".
The language of the definitions should follow exactly that of the standard
document from which the information is drawn.  No abbreviated names. No
mangled capitalisation.

The entries here are mostly enums describing bit field types. Where an enum
member has multiple meanings, each meaning is aliased (see tN2kDD0002 for an
example).

This file is included by N2kTypes.h where the standard code names are aliased
into corresponding long names (once again, exactly as described by NMEA).
*/

#ifndef _NMEA2000StdTypes_H_
#define _NMEA2000StdTypes_H_

#include <stdint.h>

// DD002 - Generic Status Pair
//
enum tN2kDD002 {
                  N2kDD002_No=0,
                  N2kDD002_Yes=1,
                  N2kDD002_Error=2,
                  N2kDD002_Unavailable=3,
                  N2kDD002_Off=N2kDD002_No,
                  N2kDD002_Disabled=N2kDD002_No,
                  N2kDD002_Reset=N2kDD002_No,
                  N2kDD002_0=N2kDD002_No,
                  N2kDD002_On=N2kDD002_Yes,
                  N2kDD002_Enabled=N2kDD002_Yes,
                  N2kDD002_Set=N2kDD002_Yes,
                  N2kDD002_1=N2kDD002_Yes,
                  N2kDD002_Unknown=N2kDD002_Unavailable
                };

enum tN2kDD025 {
                  N2kDD025_Autonomous=0,
                  N2kDD025_Differential=1,
                  N2kDD025_Estimated=2,
                  N2kDD025_Simulator=3,
                  N2kDD025_Manual=4,
                  N2kDD025_Error=0xe,
                  N2kDD025_Unavailable=0xf
                };

enum tN2kDD072 {
                  N2kDD072_RangeResidualsWereUsedToCalculateData=0,
                  N2kDD072_RangeResidualsWereCalculatedAfterPosition=1,
                  N2kDD072_Error=2,
                  N2kDD072_Unavailable=3
                };

enum tN2kDD124 {
                  N2kDD124_NotTracked=0,
                  N2kDD124_TrackedButNotUsedInSolution=1,
                  N2kDD124_UsedInSolutionWithoutDifferentialCorrections=2,
                  N2kDD124_DifferentialCorrectionsAvailable=3,
                  N2kDD124_TrackedWithDifferentialCorrections=4,
                  N2kDD124_UsedWithDifferentialCorrections=5,
                  N2kDD124_Error=14,
                  N2kDD124_Unavailable=15,
                };

#if 0 //DSR
union tN2kDD206 {
          uint16_t Status;
          struct {
              uint16_t CheckEngine:1;
              uint16_t OverTemperature:1;
              uint16_t LowOilPressure:1;
              uint16_t LowOilLevel:1;
              uint16_t LowFuelPressure:1;
              uint16_t LowSystemVoltage:1;
              uint16_t LowCoolantLevel:1;
              uint16_t WaterFlow:1;
              uint16_t WaterInFuel:1;
              uint16_t ChargeIndicator:1;
              uint16_t PreheatIndicator:1;
              uint16_t HighBoostPressure:1;
              uint16_t RevLimitExceeded:1;
              uint16_t EGRSystem:1;
              uint16_t ThrottlePositionSensor:1;
              uint16_t EngineEmergencyStopMode:1;
          } Bits;
          tN2kDD206(uint16_t _Status=0): Status(_Status) {};
};

union tN2kDD223 {
          uint16_t Status;
          struct {
              uint16_t WarningLevel1:1;
              uint16_t WarningLevel2:1;
              uint16_t LowOiPowerReduction:1;
              uint16_t MaintenanceNeeded:1;
              uint16_t EngineCommError:1;
              uint16_t SubOrSecondaryThrottle:1;
              uint16_t NeutralStartProtect:1;
              uint16_t EngineShuttingDown:1;
              uint16_t Manufacturer1:1;
              uint16_t Manufacturer2:1;
              uint16_t Manufacturer3:1;
              uint16_t Manufacturer4:1;
              uint16_t Manufacturer5:1;
              uint16_t Manufacturer6:1;
              uint16_t Manufacturer7:1;
              uint16_t Manufacturer8:1;
          } Bits;
          tN2kDD223(uint16_t _Status=0): Status(_Status) {};
          uint16_t operator= (uint16_t val) { Status=val & 0x00ff; return Status;}
};
#endif

enum tN2kDD305 {
          N2kAISAtoN_not_specified=0,
          N2kAISAtoN_reference_point=1,
          N2kAISAtoN_RACON=2,
          N2kAISAtoN_fixed_structure=3,
          N2kAISAtoN_emergency_wreck_marking_buoy=4,
          N2kAISAtoN_light_without_sectors=5,
          N2kAISAtoN_light_with_sectors=6,
          N2kAISAtoN_leading_light_front=7,
          N2kAISAtoN_leading_light_rear=8,
          N2kAISAtoN_beacon_cardinal_N=9,
          N2kAISAtoN_beacon_cardinal_E=10,
          N2kAISAtoN_beacon_cardinal_S=11,
          N2kAISAtoN_beacon_cardinal_W=12,
          N2kAISAtoN_beacon_port_hand=13,
          N2kAISAtoN_beacon_starboard_hand=14,
          N2kAISAtoN_beacon_preferred_ch_port_hand=15,
          N2kAISAtoN_beacon_preferred_ch_starboard_hand=16,
          N2kAISAtoN_beacon_isolated_danger=17,
          N2kAISAtoN_beacon_safe_water=18,
          N2kAISAtoN_beacon_special_mark=19,
          N2kAISAtoN_cardinal_mark_N=20,
          N2kAISAtoN_cardinal_mark_E=21,
          N2kAISAtoN_cardinal_mark_S=22,
          N2kAISAtoN_cardinal_mark_W=23,
          N2kAISAtoN_port_hand_mark=24,
          N2kAISAtoN_starboard_hand_mark=25,
          N2kAISAtoN_preferred_channel_port_hand=26,
          N2kAISAtoN_preferred_channel_starboard_hand=27,
          N2kAISAtoN_isolated_danger=28,
          N2kAISAtoN_safe_water=29,
          N2kAISAtoN_special_mark=30,
          N2kAISAtoN_light_vessel_lanby_rigs=31,
};

// Thruster Motor Events
//
union tN2kDD471 {
	unsigned char Events;
	struct {
		unsigned char MotorOverTemperatureCutout:1;
		unsigned char MotorOverCurrentCutout:1;
		unsigned char LowOilLevelWarning:1;
		unsigned char OilOverTemperatureWarning:1;
		unsigned char ControllerUnderVoltageCutout:1;
		unsigned char ManufacturerDefined:1;
		unsigned char Reserved:1;
		unsigned char DataNotAvailable:1;
	} Event;
	tN2kDD471(): Events(0) {};
	void SetEvents(unsigned char _Events) { Events = (_Events & 0xbf); }
};

// Thruster Direction Control
//
enum tN2kDD473 {
	N2kDD473_OFF=0,
	N2kDD473_ThrusterReady=1,
	N2kDD473_ThrusterToPORT=2,
	N2kDD473_ThrusterToSTARBOARD=3
};

// Thruster Retraction
//
enum tN2kDD474 {
	N2kDD474_OFF=0,
	N2kDD474_Extend=1,
	N2kDD474_Retract=2
};

// Thruster Control Events
//
union tN2kDD475 {
	unsigned char Events;
	struct {
		unsigned char AnotherDeviceControllingThruster:1;
		unsigned char BoatSpeedTooFast:1;
	} Event;
	tN2kDD475(): Events(0) {};
	void SetEvents(unsigned char _Events) { Events = (_Events & 0x03); }
};

// DD477 - Windlass Monitoring Events
//
union tN2kDD477 {
          unsigned char Events;
          struct {
            unsigned char ControllerUnderVoltageCutout:1;
            unsigned char ControllerOverCurrentCutout:1;
            unsigned char ControllerOverTemperatureCutout:1;
          } Event;
          tN2kDD477(): Events(0) {};
          void SetEvents(unsigned char _Events) { Events = (_Events & 0x07); }
        };

// DD478 - Windlass Control Events
//
union tN2kDD478 {
          unsigned char Events;
          struct {
            unsigned char AnotherDeviceControllingWindlass:1;
          } Event;
          tN2kDD478(): Events(0) {};
          void SetEvents(unsigned char _Events) { Events = (_Events & 0x01); }
        };


// DD480 - Windlass Motion States
//
enum tN2kDD480 {
                N2kDD480_WindlassStopped=0,
                N2kDD480_DeploymentOccurring=1,
                N2kDD480_RetrievalOccurring=2,
                N2kDD480_Unavailable=3
              };

//  DD481 - Rode Type States
//
enum tN2kDD481 {
                N2kDD481_ChainPresentlyDetected=0,
                N2kDD481_RopePresentlyDetected=1,
                N2kDD481_Error=2,
                N2kDD481_Unavailable=3
              };

// DD482 - Anchor Docking States
//
enum tN2kDD482 {
                N2kDD482_NotDocked=0,
                N2kDD482_FullyDocked=1,
                N2kDD482_Error=2,
                N2kDD482_DataNotAvailable=3
              };

// DD483 - Windlass Operating Events
//
union tN2kDD483 {
          unsigned char Events;
          struct {
            unsigned char SystemError:1;
            unsigned char SensorError:1;
            unsigned char NoWindlassMotionDetected:1;
            unsigned char RetrievalDockingDistanceReached:1;
            unsigned char EndOfRodeReached:1;
          } Event;
          tN2kDD483(): Events(0) {};
          void SetEvents(unsigned char _Events) { Events = (_Events & 0x1F); }
        };

// DD484 - Windlass Direction Control
//
enum tN2kDD484 {
                            N2kDD484_Off=0,             // Status only / cannot command
                            N2kDD484_Down=1,
                            N2kDD484_Up=2,
                            N2kDD484_Reserved=3
                          };

// DD487 - Motor Power Type
//
enum tN2kDD487 {
	N2kDD487_12VDC=0,
	N2kDD487_24VDC=1,
	N2kDD487_48VDC=2,
	N2kDD487_24VAC=3,
	N2kDD487_Hydraulic=4,
};

// DD488 - Speed Type
//
enum tN2kDD488 {
                            N2kDD488_SingleSpeed=0,
                            N2kDD488_DualSpeed=1,
                            N2kDD488_ProportionalSpeed=2,
                            N2kDD488_DataNotAvailable=3
                          };



#endif

