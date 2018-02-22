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

IMPLEMENT_DYNAMIC( WCV, RESPONSE )

WCV::WCV()
{
   Mnemonic = "WCV";
   Empty();
}

WCV::~WCV()
{
   Mnemonic.Empty();
   Empty();
}

void WCV::Empty( void )
{
   ASSERT_VALID( this );

   Velocity = 0.0;
   To.Empty();
}

BOOL WCV::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** WCV - Waypoint Closure Velocity
   **
   **        1   2 3    4
   **        |   | |    |
   ** $--WCV,x.x,N,c--c*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Velocity
   **  2) N = knots
   **  3) Waypoint ID
   **  4) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 4 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   Velocity = sentence.Double( 1 );
   To       = sentence.Field( 3 );

   return( TRUE );
}

BOOL WCV::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += Velocity;
   sentence += "N";
   sentence += To;

   sentence.Finish();
   return( TRUE );
}

const WCV& WCV::operator = ( const WCV& source )
{
   ASSERT_VALID( this );

   Velocity = source.Velocity;
   To       = source.To;

   return( *this );
}
