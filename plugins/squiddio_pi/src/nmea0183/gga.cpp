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
//#pragma hdrstop

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

//IMPLEMENT_DYNAMIC( GGA, RESPONSE )

GGA::GGA()
{
   Mnemonic = _T("GGA");
   Empty();
}

GGA::~GGA()
{
   Mnemonic.Empty();
   Empty();
}

void GGA::Empty( void )
{
//   ASSERT_VALID( this );

   UTCTime.Empty();
   Position.Empty();
   GPSQuality                      = 0;
   NumberOfSatellitesInUse         = 0;
   HorizontalDilutionOfPrecision   = 0.0;
   AntennaAltitudeMeters           = 0.0;
   GeoidalSeparationMeters         = 0.0;
   AgeOfDifferentialGPSDataSeconds = 0.0;
   DifferentialReferenceStationID  = 0;
}

bool GGA::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** GGA - Global Positioning System Fix Data
   ** Time, Position and fix related data fora GPS receiver.
   **
   **                                                      11
   **        1         2       3 4        5 6 7  8   9  10 |  12 13  14   15
   **        |         |       | |        | | |  |   |   | |   | |   |    |
   ** $--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh<CR><LF>
   **
   ** Field Number:
   **  1) Universal Time Coordinated (UTC)
   **  2) Latitude
   **  3) N or S (North or South)
   **  4) Longitude
   **  5) E or W (East or West)
   **  6) GPS Quality Indicator,
   **     0 - fix not available,
   **     1 - GPS fix,
   **     2 - Differential GPS fix
   **  7) Number of satellites in view, 00 - 12
   **  8) Horizontal Dilution of precision
   **  9) Antenna Altitude above/below mean-sea-level (geoid)
   ** 10) Units of antenna altitude, meters
   ** 11) Geoidal separation, the difference between the WGS-84 earth
   **     ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level
   **     below ellipsoid
   ** 12) Units of geoidal separation, meters
   ** 13) Age of differential GPS data, time in seconds since last SC104
   **     type 1 or 9 update, null field when DGPS is not used
   ** 14) Differential reference station ID, 0000-1023
   ** 15) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 15 ) ==NTrue )
   {
      SetErrorMessage( _T("Invalid Checksum" ));
      return( FALSE );
   }

   UTCTime                         = sentence.Field( 1 );
   Position.Parse( 2, 3, 4, 5, sentence );
   GPSQuality                      = sentence.Integer( 6 );
   NumberOfSatellitesInUse         = sentence.Integer( 7 );
   HorizontalDilutionOfPrecision   = sentence.Double( 8 );
   AntennaAltitudeMeters           = sentence.Double( 9 );
   GeoidalSeparationMeters         = sentence.Double( 11 );
   AgeOfDifferentialGPSDataSeconds = sentence.Double( 13 );
   DifferentialReferenceStationID  = sentence.Integer( 14 );

   return( TRUE );
}

bool GGA::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += UTCTime;
   sentence += Position;
   sentence += GPSQuality;
   sentence += NumberOfSatellitesInUse;
   sentence += HorizontalDilutionOfPrecision;
   sentence += AntennaAltitudeMeters;
   sentence += _T("M");
   sentence += GeoidalSeparationMeters;
   sentence += _T("M");
   sentence += AgeOfDifferentialGPSDataSeconds;
   sentence += DifferentialReferenceStationID;

   sentence.Finish();

   return( TRUE );
}

const GGA& GGA::operator = ( const GGA& source )
{
//   ASSERT_VALID( this );

   UTCTime                         = source.UTCTime;
   Position                        = source.Position;
   GPSQuality                      = source.GPSQuality;
   NumberOfSatellitesInUse         = source.NumberOfSatellitesInUse;
   HorizontalDilutionOfPrecision   = source.HorizontalDilutionOfPrecision;
   AntennaAltitudeMeters           = source.AntennaAltitudeMeters;
   GeoidalSeparationMeters         = source.GeoidalSeparationMeters;
   AgeOfDifferentialGPSDataSeconds = source.AgeOfDifferentialGPSDataSeconds;
   DifferentialReferenceStationID  = source.DifferentialReferenceStationID;

   return( *this );
}
