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

IMPLEMENT_DYNAMIC( BEC, RESPONSE )

BEC::BEC()
{
   Mnemonic = "BEC";
   Empty();
}

BEC::~BEC()
{
   Mnemonic.Empty();
   Empty();
}

void BEC::Empty( void )
{
   ASSERT_VALID( this );

   UTCTime.Empty();
   BearingTrue           = 0.0;
   BearingMagnetic       = 0.0;
   DistanceNauticalMiles = 0.0;
   To.Empty();
}

BOOL BEC::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** BEC - Bearing & Distance to Waypoint - Dead Reckoning
   **                                                         12
   **        1         2       3 4        5 6   7 8   9 10  11|    13
   **        |         |       | |        | |   | |   | |   | |    |
   ** $--BEC,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x.x,T,x.x,M,x.x,N,c--c*hh<CR><LF>
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 13 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   UTCTime               = sentence.Field( 1 );
   Position.Parse( 2, 3, 4, 5, sentence );
   BearingTrue           = sentence.Double( 6 );
   BearingMagnetic       = sentence.Double( 8 );
   DistanceNauticalMiles = sentence.Double( 10 );
   To                    = sentence.Field( 12 );

   return( TRUE );
}

BOOL BEC::Write( SENTENCE& sentence )
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
   sentence += DistanceNauticalMiles;
   sentence += "N";
   sentence += To;

   sentence.Finish();

   return( TRUE );
}

const BEC& BEC::operator = ( const BEC& source )
{
   ASSERT_VALID( this );

   UTCTime               = source.UTCTime;
   Position              = source.Position;
   BearingTrue           = source.BearingTrue;
   BearingMagnetic       = source.BearingMagnetic;
   DistanceNauticalMiles = source.DistanceNauticalMiles;
   To                    = source.To;

   return( *this );
}
