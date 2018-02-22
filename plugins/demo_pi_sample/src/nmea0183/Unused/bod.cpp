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

IMPLEMENT_DYNAMIC( BOD, RESPONSE )

BOD::BOD()
{
   Mnemonic = "BOD";
   Empty();
}

BOD::~BOD()
{
   Mnemonic.Empty();
   Empty();
}

void BOD::Empty( void )
{
   ASSERT_VALID( this );

   BearingTrue     = 0.0;
   BearingMagnetic = 0.0;
   To.Empty();
   From.Empty();
}

BOOL BOD::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** BOD - Bearing - Waypoint to Waypoint
   **
   **        1   2 3   4 5    6    7
   **        |   | |   | |    |    |
   ** $--BOD,x.x,T,x.x,M,c--c,c--c*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Bearing Degrees, TRUE
   **  2) T = True
   **  3) Bearing Degrees, Magnetic
   **  4) M = Magnetic
   **  5) TO Waypoint
   **  6) FROM Waypoint
   **  7) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 7 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   BearingTrue     = sentence.Double( 1 );
   BearingMagnetic = sentence.Double( 3 );
   To              = sentence.Field( 5 );
   From            = sentence.Field( 6 );

   return( TRUE );
}

BOOL BOD::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += BearingTrue;
   sentence += "T";
   sentence += BearingMagnetic;
   sentence += "M";
   sentence += To;
   sentence += From;

   sentence.Finish();

   return( TRUE );
}

const BOD& BOD::operator = ( const BOD& source )
{
   ASSERT_VALID( this );

   BearingTrue     = source.BearingTrue;
   BearingMagnetic = source.BearingMagnetic;
   To              = source.To;
   From            = source.From;

   return( *this );
}
