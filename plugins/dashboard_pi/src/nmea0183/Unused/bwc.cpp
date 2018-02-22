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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 *   S Blackburn's original source license:                                *
 *         "You can use it any way you like."                              *
 *   More recent (2010) license statement:                                 *
 *         "It is BSD license, do with it what you will"                   *
 */


#include "nmea0183.h"
#pragma hdrstop

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

IMPLEMENT_DYNAMIC( BWC, RESPONSE )

BWC::BWC()
{
   Mnemonic = "BWC";
   Empty();
}

BWC::~BWC()
{
   Mnemonic.Empty();
   Empty();
}

void BWC::Empty( void )
{
   ASSERT_VALID( this );

   UTCTime.Empty();
   BearingTrue     = 0.0;
   BearingMagnetic = 0.0;
   NauticalMiles   = 0.0;
   To.Empty();
}

BOOL BWC::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** BWC - Bearing and Distance to Waypoint
   ** Latitude, N/S, Longitude, E/W, UTC, Status
   **                                                       11
   **        1         2       3 4        5 6   7 8   9 10  | 12   13
   **        |         |       | |        | |   | |   | |   | |    |
   ** $--BWC,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x.x,T,x.x,M,x.x,N,c--c*hh<CR><LF>
   **
   **  1) UTCTime
   **  2) Waypoint Latitude
   **  3) N = North, S = South
   **  4) Waypoint Longitude
   **  5) E = East, W = West
   **  6) Bearing, True
   **  7) T = True
   **  8) Bearing, Magnetic
   **  9) M = Magnetic
   ** 10) Nautical Miles
   ** 11) N = Nautical Miles
   ** 12) Waypoint ID
   ** 13) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 13 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   UTCTime         = sentence.Field( 1 );
   Position.Parse( 2, 3, 4, 5, sentence );
   BearingTrue     = sentence.Double( 6 );
   BearingMagnetic = sentence.Double( 8 );
   NauticalMiles   = sentence.Double( 10 );
   To              = sentence.Field( 12 );

   return( TRUE );
}

BOOL BWC::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += UTCTime;
   sentence += Position;
   sentence += BearingTrue;
   sentence += "T";
   sentence += BearingMagnetic;
   sentence += "M";
   sentence += NauticalMiles;
   sentence += "N";
   sentence += To;

   sentence.Finish();

   return( TRUE );
}

const BWC& BWC::operator = ( const BWC& source )
{
   ASSERT_VALID( this );

   UTCTime         = source.UTCTime;
   Position        = source.Position;
   BearingTrue     = source.BearingTrue;
   BearingMagnetic = source.BearingMagnetic;
   NauticalMiles   = source.NauticalMiles;
   To              = source.To;

   return( *this );
}
