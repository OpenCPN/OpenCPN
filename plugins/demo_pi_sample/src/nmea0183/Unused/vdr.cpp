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

IMPLEMENT_DYNAMIC( VDR, RESPONSE )

VDR::VDR()
{
   Mnemonic = "VDR";
   Empty();
}

VDR::~VDR()
{
   Mnemonic.Empty();
   Empty();
}

void VDR::Empty( void )
{
   ASSERT_VALID( this );

   DegreesTrue     = 0.0;
   DegreesMagnetic = 0.0;
   Knots           = 0.0;
}

BOOL VDR::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** VDR - Set and Drift
   **
   **        1   2 3   4 5   6 7
   **        |   | |   | |   | |
   ** $--VDR,x.x,T,x.x,M,x.x,N*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Degress True
   **  2) T = True
   **  3) Degrees Magnetic
   **  4) M = Magnetic
   **  5) Knots (speed of current)
   **  6) N = Knots
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

   DegreesTrue     = sentence.Double( 1 );
   DegreesMagnetic = sentence.Double( 3 );
   Knots           = sentence.Double( 5 );

   return( TRUE );
}

BOOL VDR::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += DegreesTrue;
   sentence += "T";
   sentence += DegreesMagnetic;
   sentence += "M";
   sentence += Knots;
   sentence += "N";

   sentence.Finish();

   return( TRUE );
}

const VDR& VDR::operator = ( const VDR& source )
{
   ASSERT_VALID( this );

   DegreesTrue     = source.DegreesTrue;
   DegreesMagnetic = source.DegreesMagnetic;
   Knots           = source.Knots;

   return( *this );
}
