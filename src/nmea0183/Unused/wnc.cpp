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

IMPLEMENT_DYNAMIC( WNC, RESPONSE )

WNC::WNC()
{
   Mnemonic = "WNC";
   Empty();
}

WNC::~WNC()
{
   Mnemonic.Empty();
   Empty();
}

void WNC::Empty( void )
{
   ASSERT_VALID( this );

   MilesDistance      = 0.0;
   KilometersDistance = 0.0;
   To.Empty();
   From.Empty();
}

BOOL WNC::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** WNC - Distance - Waypoint to Waypoint
   **
   **        1   2 3   4 5    6    7
   **        |   | |   | |    |    |
   ** $--WNC,x.x,N,x.x,K,c--c,c--c*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Distance, Nautical Miles
   **  2) N = Nautical Miles
   **  3) Distance, Kilometers
   **  4) K = Kilometers
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

   MilesDistance      = sentence.Double( 1 );
   KilometersDistance = sentence.Double( 3 );
   To                 = sentence.Field( 5 );
   From               = sentence.Field( 6 );

   return( TRUE );
}

BOOL WNC::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += MilesDistance;
   sentence += "N";
   sentence += KilometersDistance;
   sentence += "K";
   sentence += To;
   sentence += From;

   sentence.Finish();

   return( TRUE );
}

const WNC& WNC::operator = ( const WNC& source )
{
   ASSERT_VALID( this );

   MilesDistance      = source.MilesDistance;
   KilometersDistance = source.KilometersDistance;
   To                 = source.To;
   From               = source.From;

   return( *this );
}
