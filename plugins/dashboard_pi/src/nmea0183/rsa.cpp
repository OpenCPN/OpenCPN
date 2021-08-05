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


#include "nmea0183.h"

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

//IMPLEMENT_DYNAMIC( RSA, RESPONSE )

RSA::RSA()
{
   Mnemonic = _T("RSA");
   Empty();
}

RSA::~RSA()
{
   Mnemonic.Empty();
   Empty();
}

void RSA::Empty( void )
{
//   ASSERT_VALID( this );

   Starboard            = 0.0;
   IsStarboardDataValid = Unknown0183;
   Port                 = 0.0;
   IsPortDataValid      = Unknown0183;
}

bool RSA::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** RSA - Rudder Sensor Angle
   **
   **        1   2 3   4 5
   **        |   | |   | |
   ** $--RSA,x.x,A,x.x,A*hh<CR><LF>
   **
   ** Field Number:
   **  1) Starboard (or single) rudder sensor, "-" means Turn To Port
   **  2) Status, A means data is valid
   **  3) Port rudder sensor
   **  4) Status, A means data is valid
   **  5) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 5 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   Starboard            = sentence.Double(  1 );
   IsStarboardDataValid = sentence.Boolean( 2 );
   Port                 = sentence.Double(  3 );
   IsPortDataValid      = sentence.Boolean( 4 );

   return( TRUE );
}

bool RSA::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += Starboard;
   sentence += IsStarboardDataValid;
   sentence += Port;
   sentence += IsPortDataValid;

   sentence.Finish();

   return( TRUE );
}

const RSA& RSA::operator = ( const RSA& source )
{
//   ASSERT_VALID( this );

   Starboard            = source.Starboard;
   IsStarboardDataValid = source.IsStarboardDataValid;
   Port                 = source.Port;
   IsPortDataValid      = source.IsPortDataValid;

   return( *this );
}
