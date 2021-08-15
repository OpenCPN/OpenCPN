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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *                                                                         *
 *   S Blackburn's original source license:                                *
 *         "You can use it any way you like."                              *
 *   More recent (2010) license statement:                                 *
 *         "It is BSD license, do with it what you will"                   *
 */


#include "nmea0183.h"

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

extern int              g_NMEAAPBPrecision;


APB::APB()
{
   Mnemonic = _T("APB");
   Empty();
}

APB::~APB()
{
   Mnemonic.Empty();
   Empty();
}

void APB::Empty( void )
{
//   ASSERT_VALID( this );

   CrossTrackErrorMagnitude = 0.0;
   DirectionToSteer         = LR_Unknown;
   CrossTrackUnits.Empty();
   To.Empty();
   IsArrivalCircleEntered   = Unknown0183;
   IsPerpendicular          = Unknown0183;
}

bool APB::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   wxString field_data;

   /*
   ** APB - Autopilot Sentence "B"
   **                                         13    15
   **        1 2 3   4 5 6 7 8   9 10   11  12|   14|
   **        | | |   | | | | |   | |    |   | |   | |
   ** $--APB,A,A,x.x,a,N,A,A,x.x,a,c--c,x.x,a,x.x,a*hh<CR><LF>
   **
   **  1) Status
   **     V = LORAN-C Blink or SNR warning
   **     V = general warning flag or other navigation systems when a reliable
   **         fix is not available
   **  2) Status
   **     V = Loran-C Cycle Lock warning flag
   **     A = OK or not used
   **  3) Cross Track Error Magnitude
   **  4) Direction to steer, L or R
   **  5) Cross Track Units, N = Nautical Miles
   **  6) Status
   **     A = Arrival Circle Entered
   **  7) Status
   **     A = Perpendicular passed at waypoint
   **  8) Bearing origin to destination
   **  9) M = Magnetic, T = True
   ** 10) Destination Waypoint ID
   ** 11) Bearing, present position to Destination
   ** 12) M = Magnetic, T = True
   ** 13) Heading to steer to destination waypoint
   ** 14) M = Magnetic, T = True
   ** 15) Checksum
   */

   /*
   ** First we check the checksum...
   */

   NMEA0183_BOOLEAN check = sentence.IsChecksumBad( 15 );

   if ( check == NTrue )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   /*
   ** Line has already been checked for checksum validity
   */

   IsLoranBlinkOK                           = sentence.Boolean( 1 );
   IsLoranCCycleLockOK                      = sentence.Boolean( 2 );
   CrossTrackErrorMagnitude                 = sentence.Double( 3 );
   DirectionToSteer                         = sentence.LeftOrRight( 4 );
   CrossTrackUnits                          = sentence.Field( 5 );
   IsArrivalCircleEntered                   = sentence.Boolean( 6 );
   IsPerpendicular                          = sentence.Boolean( 7 );
   BearingOriginToDestination               = sentence.Double( 8 );
   BearingOriginToDestinationUnits          = sentence.Field( 9 );
   To                                       = sentence.Field( 10 );
   BearingPresentPositionToDestination      = sentence.Double( 11 );
   BearingPresentPositionToDestinationUnits = sentence.Field( 12 );
   HeadingToSteer                           = sentence.Double( 13 );
   HeadingToSteerUnits                      = sentence.Field( 14 );

   return( TRUE );
}

bool APB::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += IsLoranBlinkOK;
   sentence += IsLoranCCycleLockOK;
   sentence.Add( CrossTrackErrorMagnitude, g_NMEAAPBPrecision);

   if(DirectionToSteer == Left)
       sentence += _T("L");
   else
       sentence += _T("R");

   sentence += CrossTrackUnits;
   sentence += IsArrivalCircleEntered;
   sentence += IsPerpendicular;
   sentence.Add( BearingOriginToDestination, g_NMEAAPBPrecision);
   sentence += BearingOriginToDestinationUnits;
   sentence += To;
   sentence.Add( BearingPresentPositionToDestination, g_NMEAAPBPrecision );
   sentence += BearingPresentPositionToDestinationUnits;
   sentence.Add( HeadingToSteer, g_NMEAAPBPrecision );
   sentence += HeadingToSteerUnits;

   sentence.Finish();

   return( TRUE );
}

const APB& APB::operator = ( const APB& source )
{
//   ASSERT_VALID( this );

   IsLoranBlinkOK                           = source.IsLoranBlinkOK;
   IsLoranCCycleLockOK                      = source.IsLoranCCycleLockOK;
   CrossTrackErrorMagnitude                 = source.CrossTrackErrorMagnitude;
   DirectionToSteer                         = source.DirectionToSteer;
   CrossTrackUnits                          = source.CrossTrackUnits;
   IsArrivalCircleEntered                   = source.IsArrivalCircleEntered;
   IsPerpendicular                          = source.IsPerpendicular;
   BearingOriginToDestination               = source.BearingOriginToDestination;
   BearingOriginToDestinationUnits          = source.BearingOriginToDestinationUnits;
   To                                       = source.To;
   BearingPresentPositionToDestination      = source.BearingPresentPositionToDestination;
   BearingPresentPositionToDestinationUnits = source.BearingPresentPositionToDestinationUnits;
   HeadingToSteer                           = source.HeadingToSteer;
   HeadingToSteerUnits                      = source.HeadingToSteerUnits;

   return( *this );
}
