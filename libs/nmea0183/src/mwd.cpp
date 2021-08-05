/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register, Jean-Eudes Onfray
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

//IMPLEMENT_DYNAMIC( MWD, RESPONSE )

MWD::MWD()
{
   Mnemonic = _T("MWD");
   Empty();
}

MWD::~MWD()
{
   Mnemonic.Empty();
   Empty();
}

void MWD::Empty( void )
{
//   ASSERT_VALID( this );

   WindAngleTrue   = 0.0;
   WindAngleMagnetic   = 0.0;
   WindSpeedKnots   = 0.0;
   WindSpeedms   = 0.0;
}

bool MWD::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** MWD - Wind Speed and Angle
   **
   **        1   2 3   4 5   6 7   8 9
   **        |   | |   | |   | |   | |
   ** $--MWD,x.x,T,x.x,M,x.x,N,x.x,M*hh<CR><LF>
   **
   ** Field Number:
   **  1) Wind direction, 0.0 to 359.9 degrees True, to the nearest 0.1 degree
   **  2) T = True
   **  3) Wind direction, 0.0 to 359.9 degrees Magnetic, to the nearest 0.1 degree
   **  4) M = Magnetic
   **  5> Wind speed, knots, to the nearest 0.1 knot.
   **  6> N = Knots
   **  7> Wind speed, meters/second, to the nearest 0.1 m/s.
   **  8> M = Meters/second
   **  9) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 9 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   WindAngleTrue      = sentence.Double( 1 );
   WindAngleMagnetic  = sentence.Double( 3 );
   WindSpeedKnots     = sentence.Double( 5 );
   WindSpeedms        = sentence.Double( 7 );

   return( TRUE );
}

bool MWD::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += WindAngleTrue;
   sentence += _T("T");
   sentence += WindAngleMagnetic;
   sentence += _T("M");
   sentence += WindSpeedKnots;
   sentence += _T("N");
   sentence += WindSpeedms;
   sentence += _T("M");

   sentence.Finish();

   return( TRUE );
}

const MWD& MWD::operator = ( const MWD& source )
{
//   ASSERT_VALID( this );

   WindAngleTrue      = source.WindAngleTrue;
   WindAngleMagnetic  = source.WindAngleMagnetic;
   WindSpeedKnots     = source.WindSpeedKnots;
   WindSpeedms        = source.WindSpeedms;

   return( *this );
}
