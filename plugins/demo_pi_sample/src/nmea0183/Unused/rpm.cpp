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

IMPLEMENT_DYNAMIC( RPM, RESPONSE )

RPM::RPM()
{
   Mnemonic = "RPM";
   Empty();
}

RPM::~RPM()
{
   Mnemonic.Empty();
   Empty();
}

void RPM::Empty( void )
{
   ASSERT_VALID( this );

   Source.Empty();
   SourceNumber             = 0;
   RevolutionsPerMinute     = 0.0;
   PropellerPitchPercentage = 0.0;
   IsDataValid              = Unknown;
}

BOOL RPM::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** RPM - Revolutions
   **
   **        1 2 3   4   5 6
   **        | | |   |   | |
   ** $--RPM,a,x,x.x,x.x,A*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Sourse, S = Shaft, E = Engine
   **  2) Engine or shaft number
   **  3) Speed, Revolutions per minute
   **  4) Propeller pitch, % of maximum, "-" means astern
   **  5) Status, A means data is valid
   **  6) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 6 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   Source                   = sentence.Field( 1 );
   SourceNumber             = sentence.Integer( 2 );
   RevolutionsPerMinute     = sentence.Double( 3 );
   PropellerPitchPercentage = sentence.Double( 4 );
   IsDataValid              = sentence.Boolean( 5 );

   return( TRUE );
}

BOOL RPM::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += Source;
   sentence += SourceNumber;
   sentence += RevolutionsPerMinute;
   sentence += PropellerPitchPercentage;
   sentence += IsDataValid;
   
   sentence.Finish();

   return( TRUE );
}

const RPM& RPM::operator = ( const RPM& source )
{
   ASSERT_VALID( this );

   Source                   = source.Source;
   SourceNumber             = source.SourceNumber;
   RevolutionsPerMinute     = source.RevolutionsPerMinute;
   PropellerPitchPercentage = source.PropellerPitchPercentage;
   IsDataValid              = source.IsDataValid;

   return( *this );
}
