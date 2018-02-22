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

IMPLEMENT_DYNAMIC( TRF, RESPONSE )

TRF::TRF()
{
   Mnemonic = "TRF";
   Empty();
}

TRF::~TRF()
{
   Mnemonic.Empty();
   Empty();
}

void TRF::Empty( void )
{
   ASSERT_VALID( this );

   UTCTime.Empty();
   Date.Empty();
   Position.Empty();
   ElevationAngle              = 0.0;
   NumberOfIterations          = 0.0;
   NumberOfDopplerIntervals    = 0.0;
   UpdateDistanceNauticalMiles = 0.0;
   SatelliteID                 = 0;
   IsDataValid                 = Unknown;
}

BOOL TRF::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** TRF - TRANSIT Fix Data
   **                                                                    13
   **        1         2      3       4 5        6 7   8   9   10  11  12|
   **        |         |      |       | |        | |   |   |   |   |   | |
   ** $--TRF,hhmmss.ss,xxxxxx,llll.ll,a,yyyyy.yy,a,x.x,x.x,x.x,x.x,xxx,A*hh<CR><LF>
   **
   ** Field Number: 
   **  1) UTC Time
   **  2) Date, ddmmyy
   **  3) Latitude
   **  4) N or S
   **  5) Longitude
   **  6) E or W
   **  7) Elevation Angle
   **  8) Number of iterations
   **  9) Number of Doppler intervals
   ** 10) Update distance, nautical miles
   ** 11) Satellite ID
   ** 12) Data Validity
   ** 13) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 13 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   UTCTime                     = sentence.Field( 1 );
   Date                        = sentence.Field( 2 );
   Position.Parse( 3, 4, 5, 6, sentence );
   ElevationAngle              = sentence.Double( 7 );
   NumberOfIterations          = sentence.Double( 8 );
   NumberOfDopplerIntervals    = sentence.Double( 9 );
   UpdateDistanceNauticalMiles = sentence.Double( 10 );
   SatelliteID                 = sentence.Integer( 11 );
   IsDataValid                 = sentence.Boolean( 12 );

   return( TRUE );
}

BOOL TRF::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += UTCTime;
   sentence += Date;
   sentence += Position;
   sentence += ElevationAngle;
   sentence += NumberOfIterations;
   sentence += NumberOfDopplerIntervals;
   sentence += UpdateDistanceNauticalMiles;
   sentence += SatelliteID;
   sentence += IsDataValid;

   sentence.Finish();

   return( TRUE );
}

const TRF& TRF::operator = ( const TRF& source )
{
   ASSERT_VALID( this );

   UTCTime                     = source.UTCTime;
   Date                        = source.Date;
   Position                    = source.Position;
   ElevationAngle              = source.ElevationAngle;
   NumberOfIterations          = source.NumberOfIterations;
   NumberOfDopplerIntervals    = source.NumberOfDopplerIntervals;
   UpdateDistanceNauticalMiles = source.UpdateDistanceNauticalMiles;
   SatelliteID                 = source.SatelliteID;
   IsDataValid                 = source.IsDataValid;

  return( *this );
}
