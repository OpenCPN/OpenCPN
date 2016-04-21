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

//IMPLEMENT_DYNAMIC( MDA, RESPONSE )
// TODO: Read rest of MDA sentece.

MDA::MDA()
{
   Mnemonic = _T("MDA");
   Empty();
}

MDA::~MDA()
{
   Mnemonic.Empty();
   Empty();
}

void MDA::Empty( void )
{
//   ASSERT_VALID( this );

   Pressure = 0.0;
   UnitOfMeasurement.Empty();
}

bool MDA::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

/*Wind speed, meters/second
**Wind speed, knots
**Wind direction,
**degrees Magnetic
**Wind direction, degrees True
**$
**--
**MDA,x.x,I,x.x,B,x.x,C,x.x,C,x.x,x.x,x.x,C,x.x,T,x.x,M,x.x,N,x.x,M*hh<CR><LF>
**    |   |  |  |          Dew point, degrees C
**    |   |  |  |          Absolute humidity, percent
**    |   |  |  |          Relative humidity, percent
**    |   |  |  |        Water temperature, degrees C
**    |   |  |  |          Air temperature, degrees C
**    |   |  |----Barometric pressure, bars
**    |----- Barometric pressure, inches of mercur
*/


   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( sentence.GetNumberOfDataFields() +1) == TRUE || FALSE ) //diferent vendors have different length of data message and not 24 field as in standard.
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

Pressure       = sentence.Double( 3 );
UnitOfMeasurement = sentence.Field( 4 );

if(UnitOfMeasurement==wxT("B"))
{
   Pressure       = sentence.Double( 3 ); //from bar to Hecto pascal

}


   return( TRUE );
}

bool MDA::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += Pressure;
   sentence += UnitOfMeasurement;

   sentence.Finish();

   return( TRUE );
}

const MDA& MDA::operator = ( const MDA& source )
{
//   ASSERT_VALID( this );

   Pressure       = source.Pressure;
   UnitOfMeasurement = source.UnitOfMeasurement;

   return( *this );
}
