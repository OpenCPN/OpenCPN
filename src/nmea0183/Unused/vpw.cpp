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

IMPLEMENT_DYNAMIC( VPW, RESPONSE )

VPW::VPW()
{
   Mnemonic = "VPW";
   Empty();
}

VPW::~VPW()
{
   Mnemonic.Empty();
   Empty();
}

void VPW::Empty( void )
{
   ASSERT_VALID( this );

   Knots           = 0.0;
   MetersPerSecond = 0.0;
}

BOOL VPW::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** VPW - Speed - Measured Parallel to Wind
   **
   **        1   2 3   4 5
   **        |   | |   | |
   ** $--VPW,x.x,N,x.x,M*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Speed, "-" means downwind
   **  2) N = Knots
   **  3) Speed, "-" means downwind
   **  4) M = Meters per second
   **  5) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 5 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   Knots           = sentence.Double( 1 );
   MetersPerSecond = sentence.Double( 3 );

   return( TRUE );
}

BOOL VPW::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += Knots;
   sentence += "N";
   sentence += MetersPerSecond;
   sentence += "M";

   sentence.Finish();

   return( TRUE );
}

const VPW& VPW::operator = ( const VPW& source )
{
   ASSERT_VALID( this );

   Knots           = source.Knots;
   MetersPerSecond = source.MetersPerSecond;

   return( *this );
}
