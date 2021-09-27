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

//IMPLEMENT_DYNAMIC( DBT, RESPONSE )

DBT::DBT()
{
   Mnemonic = _T("DBT");
   Empty();
}

DBT::~DBT()
{
   Mnemonic.Empty();
   Empty();
}

void DBT::Empty( void )
{
//   ASSERT_VALID( this );

   DepthFeet    = 0.0;
   DepthMeters  = 0.0;
   DepthFathoms = 0.0;
}

bool DBT::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** DBT - Depth below transducer
   **
   **        1   2 3   4 5   6 7
   **        |   | |   | |   | |
   ** $--DBT,x.x,f,x.x,M,x.x,F*hh<CR><LF>
   **
   ** Field Number:
   **  1) Depth, feet
   **  2) f = feet
   **  3) Depth, meters
   **  4) M = meters
   **  5) Depth, Fathoms
   **  6) F = Fathoms
   **  7) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 7 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   DepthFeet    = sentence.Double( 1 );
   DepthMeters  = sentence.Double( 3 );
   DepthFathoms = sentence.Double( 5 );

   return( TRUE );
}

bool DBT::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += DepthFeet;
   sentence += _T("f");
   sentence += DepthMeters;
   sentence += _T("M");
   sentence += DepthFathoms;
   sentence += _T("F");

   sentence.Finish();

   return( TRUE );
}

const DBT& DBT::operator = ( const DBT& source )
{
   //ASSERT_VALID( this );

   DepthFeet    = source.DepthFeet;
   DepthMeters  = source.DepthMeters;
   DepthFathoms = source.DepthFathoms;

   return( *this );
}
