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

#if ! defined( NMEA_0183_HEADER )

#define NMEA_0183_HEADER

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

//    Include wxWindows stuff
//#include "wx/wxprec.h"

//#ifndef  WX_PRECOMP
//  #include "wx/wx.h"
//#endif //precompiled headers
#include "wx/string.h"
#include "wx/list.h"
#include "wx/arrstr.h"
#include <wx/math.h>

/*
** Turn off the warning about precompiled headers, it is rather annoying
*/

#ifdef __MSVC__
#pragma warning( disable : 4699 )
#endif

#define CARRIAGE_RETURN 0x0D
#define LINE_FEED       0x0A


typedef enum _NMEA0183_BOOLEAN
{
   Unknown0183 = 0,
   NTrue,
   NFalse
} NMEA0183_BOOLEAN;

typedef enum _leftright
{
   LR_Unknown = 0,
   Left,
   Right
} LEFTRIGHT;

typedef enum _eastwest
{
   EW_Unknown = 0,
   East,
   West
} EASTWEST;

typedef enum _northsouth
{
   NS_Unknown = 0,
   North,
   South
} NORTHSOUTH;

typedef enum _reference
{
   ReferenceUnknown = 0,
   BottomTrackingLog,
   ManuallyEntered,
   WaterReferenced,
   RadarTrackingOfFixedTarget,
   PositioningSystemGroundReference
} REFERENCE;

typedef enum _communicationsmode
{
   CommunicationsModeUnknown         = 0,
   F3E_G3E_SimplexTelephone          = 'd',
   F3E_G3E_DuplexTelephone           = 'e',
   J3E_Telephone                     = 'm',
   H3E_Telephone                     = 'o',
   F1B_J2B_FEC_NBDP_TelexTeleprinter = 'q',
   F1B_J2B_ARQ_NBDP_TelexTeleprinter = 's',
   F1B_J2B_ReceiveOnlyTeleprinterDSC = 'w',
   A1A_MorseTapeRecorder             = 'x',
   A1A_MorseKeyHeadset               = '{',
   F1C_F2C_F3C_FaxMachine            = '|'
} COMMUNICATIONS_MODE;

typedef enum _transducertype
{
   TransducerUnknown   = 0,
   AngularDisplacementTransducer = 'A',
   TemperatureTransducer         = 'C',
   LinearDisplacementTransducer  = 'D',
   FrequencyTransducer           = 'F',
   HumidityTransducer            = 'H',
   ForceTransducer               = 'N',
   PressureTransducer            = 'P',
   FlowRateTransducer            = 'R',
   TachometerTransducer          = 'T',
   VolumeTransducer              = 'V'
} TRANSDUCER_TYPE;

typedef enum
{
      RouteUnknown = 0,
      CompleteRoute,
      WorkingRoute
} ROUTE_TYPE;

/*
** Misc Function Prototypes
*/

int HexValue( const wxString& hex_string );

wxString& expand_talker_id( const wxString & );
wxString& Hex( int value );
wxString& talker_id( const wxString& sentence );

#include "nmea0183.hpp"

#endif // NMEA0183_HEADER
