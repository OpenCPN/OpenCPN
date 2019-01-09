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

//IMPLEMENT_DYNAMIC( VHW, RESPONSE )

VHW::VHW()
{
   Mnemonic = _T("VHW");
   Empty();
}

VHW::~VHW()
{
   Mnemonic.Empty();
   Empty();
}

void VHW::Empty( void )
{
//   ASSERT_VALID( this );

   DegreesTrue       = 0.0;
   DegreesMagnetic   = 0.0;
   Knots             = 0.0;
   KilometersPerHour = 0.0;
}

bool VHW::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** VHW - Water speed and heading
   **
   **        1   2 3   4 5   6 7   8 9
   **        |   | |   | |   | |   | |
   ** $--VHW,x.x,T,x.x,M,x.x,N,x.x,K*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Degress True
   **  2) T = True
   **  3) Degrees Magnetic
   **  4) M = Magnetic
   **  5) Knots (speed of vessel relative to the water)
   **  6) N = Knots
   **  7) Kilometers (speed of vessel relative to the water)
   **  8) K = Kilometers
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

   DegreesTrue       = sentence.Double( 1 );
   DegreesMagnetic   = sentence.Double( 3 );
   Knots             = sentence.Double( 5 );
   KilometersPerHour = sentence.Double( 7 );

   return( TRUE );
}

bool VHW::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += DegreesTrue;
   sentence += _T("T");
   sentence += DegreesMagnetic;
   sentence += _T("M");
   sentence += Knots;
   sentence += _T("N");
   sentence += KilometersPerHour;
   sentence += _T("K");

   sentence.Finish();

   return( TRUE );
}

const VHW& VHW::operator = ( const VHW& source )
{
//   ASSERT_VALID( this );

   DegreesTrue       = source.DegreesTrue;
   DegreesMagnetic   = source.DegreesMagnetic;
   Knots             = source.Knots;
   KilometersPerHour = source.KilometersPerHour;

   return( *this );
}
