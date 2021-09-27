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

//IMPLEMENT_DYNAMIC( MTA, RESPONSE )

MTA::MTA()
{
   Mnemonic = _T("MTA");
   Empty();
}

MTA::~MTA()
{
   Mnemonic.Empty();
   Empty();
}

void MTA::Empty( void )
{
//   ASSERT_VALID( this );

   Temperature = 0.0;
   UnitOfMeasurement.Empty();
}

bool MTA::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** MTA - Air Temperature
   **
   **        1   2 3
   **        |   | |
   ** $--MTA,x.x,C*hh<CR><LF>
   **
   ** Field Number:
   **  1) Degrees
   **  2) Unit of Measurement, Celcius
   **  3) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 3 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   Temperature       = sentence.Double( 1 );
   UnitOfMeasurement = sentence.Field( 2 );

   return( TRUE );
}

bool MTA::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += Temperature;
   sentence += UnitOfMeasurement;

   sentence.Finish();

   return( TRUE );
}

const MTA& MTA::operator = ( const MTA& source )
{
//   ASSERT_VALID( this );

   Temperature       = source.Temperature;
   UnitOfMeasurement = source.UnitOfMeasurement;

   return( *this );
}
