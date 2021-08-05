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

//IMPLEMENT_DYNAMIC( MWV, RESPONSE )

MWV::MWV()
{
   Mnemonic = _T("MWV");
   Empty();
}

MWV::~MWV()
{
   Mnemonic.Empty();
   Empty();
}

void MWV::Empty( void )
{
//   ASSERT_VALID( this );

   WindAngle   = 0.0;
   Reference.Empty();
   WindSpeed   = 0.0;
   WindSpeedUnits.Empty();
   IsDataValid = Unknown0183;
}

bool MWV::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** MWV - Wind Speed and Angle
   **
   **        1   2 3   4 5
   **        |   | |   | |
   ** $--MWV,x.x,a,x.x,a*hh<CR><LF>
   **
   ** Field Number:
   **  1) Wind Angle, 0 to 360 degrees
   **  2) Reference, R = Relative, T = True
   **  3) Wind Speed
   **  4) Wind Speed Units, K/M/N
   **  5) Status, A = Data Valid
   **  6) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 6 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   WindAngle      = sentence.Double( 1 );
   Reference      = sentence.Field( 2 );
   WindSpeed      = sentence.Double( 3 );
   WindSpeedUnits = sentence.Field( 4 );
   IsDataValid    = sentence.Boolean( 5 );

   return( TRUE );
}

bool MWV::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += WindAngle;
   sentence += Reference;
   sentence += WindSpeed;
   sentence += WindSpeedUnits;
   sentence += IsDataValid;

   sentence.Finish();

   return( TRUE );
}

const MWV& MWV::operator = ( const MWV& source )
{
//   ASSERT_VALID( this );

   WindAngle      = source.WindAngle;
   Reference      = source.Reference;
   WindSpeed      = source.WindSpeed;
   WindSpeedUnits = source.WindSpeedUnits;
   IsDataValid    = source.IsDataValid;

   return( *this );
}
