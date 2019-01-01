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


LATITUDE::LATITUDE()
{
   Empty();
}

LATITUDE::~LATITUDE()
{
   Empty();
}

void LATITUDE::Empty( void )
{

   Latitude = 0.0;
   Northing = NS_Unknown;
}

bool LATITUDE::IsDataValid( void )
{
   if ( Northing != North && Northing != South )
   {
      return( FALSE );
   }

   return( TRUE );
}

void LATITUDE::Parse( int position_field_number, int north_or_south_field_number, const SENTENCE& sentence )
{
   wxString n_or_s = sentence.Field( north_or_south_field_number );
   Set( sentence.Double( position_field_number ), n_or_s );
}

void LATITUDE::Set( double position, const wxString& north_or_south )
{
//   assert( north_or_south != NULL );

   Latitude = position;
   wxString ts = north_or_south;

   if ( !ts.IsEmpty( ) && ts.Trim(false)[ 0 ] == _T('N') )
   {
      Northing = North;
   }
   else if ( !ts.IsEmpty( ) && ts.Trim(false)[ 0 ] == _T('S') )
   {
      Northing = South;
   }
   else
   {
      Northing = NS_Unknown;
   }
}

void LATITUDE::Write( SENTENCE& sentence )
{

   wxString temp_string;
    int neg = 0;
    int d;
    int m;

    if (Latitude < 0.0) {
            Latitude = -Latitude;
            neg = 1;
            }
    d = (int) Latitude;
    m = (int) ((Latitude - (double) d) * 60000.0);

    if (neg)
            d = -d;

    temp_string.Printf(_T("%d%02d.%03d"), d, m / 1000, m % 1000);

   sentence += temp_string;

   if ( Northing == North )
   {
       sentence += _T("N");
   }
   else if ( Northing == South )
   {
       sentence += _T("S");
   }
   else
   {
      /*
      ** Add Nothing
      */
   }
}

const LATITUDE& LATITUDE::operator = ( const LATITUDE& source )
{

   Latitude = source.Latitude;
   Northing = source.Northing;

   return( *this );
}
