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

VWR::VWR()
{
   Mnemonic = _T("VWR");
   Empty();
}

VWR::~VWR()
{
   Mnemonic.Empty();
   Empty();
}

void VWR::Empty( void )
{
//   ASSERT_VALID( this );

    WindDirectionMagnitude = 0.0;
    DirectionOfWind = LR_Unknown;
    WindSpeedKnots = 0.0;
    WindSpeedms = 0.0;
    WindSpeedKmh = 0.0;
}

bool VWR::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
    ** MWV - Wind Speed and Angle
    **
    **        1   2 3 4   5 6   7   8
    **        |   | | |   | |   |   |
    ** $--VWR,x.x,L,x.x,N,x.x,M,x.x,K,*hh<CR><LF>
    **
    ** 1) Wind direction magnitude in degrees
    ** 2) Wind direction Left/Right of bow
    ** 3) Speed
    ** 4) N = Knots
    ** 5) Speed
    ** 6) M = Meters Per Second
    ** 7) Speed
    ** 8) K = Kilometers Per Hour
    ** 9) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 9 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   WindDirectionMagnitude = sentence.Double( 1 );
   DirectionOfWind = sentence.LeftOrRight( 2 );
   WindSpeedKnots = sentence.Double( 3 );
   WindSpeedms = sentence.Double( 5 );
   WindSpeedKmh = sentence.Double( 7 );

   return( TRUE );
}

bool VWR::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += WindDirectionMagnitude;
   sentence += DirectionOfWind;
   sentence += WindSpeedKnots;
   sentence += WindSpeedms;
   sentence += WindSpeedms;
   sentence += WindSpeedKmh;

   return( TRUE );
}

const VWR& VWR::operator = ( const VWR& source )
{
//   ASSERT_VALID( this );

   WindDirectionMagnitude   = source.WindDirectionMagnitude;
   DirectionOfWind          = source.DirectionOfWind;
   WindSpeedKnots           = source.WindSpeedKnots;
   WindSpeedms              = source.WindSpeedms;
   WindSpeedKmh             = source.WindSpeedKmh;

   return( *this );
}
