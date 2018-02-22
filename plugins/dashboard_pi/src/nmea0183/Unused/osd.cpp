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

IMPLEMENT_DYNAMIC( OSD, RESPONSE )

OSD::OSD()
{
   Mnemonic = "OSD";
   Empty();
}

OSD::~OSD()
{
   Mnemonic.Empty();
   Empty();
}

void OSD::Empty( void )
{
   ASSERT_VALID( this );

   HeadingDegreesTrue       = 0.0;
   IsHeadingValid           = Unknown;
   VesselCourseDegreesTrue  = 0.0;
   VesselCourseReference    = ReferenceUnknown;
   VesselSpeed              = 0.0;
   VesselSpeedReference     = ReferenceUnknown;
   VesselSetDegreesTrue     = 0.0;
   VesselDriftSpeed         = 0.0;
   VesselDriftSpeedUnits.Empty();
}

BOOL OSD::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** OSD - Own Ship Data
   **
   **        1   2 3   4 5   6 7   8   9 10
   **        |   | |   | |   | |   |   | |
   ** $--OSD,x.x,A,x.x,a,x.x,a,x.x,x.x,a*hh<CR><LF>
   **
   **  1) Heading, degrees true
   **  2) Status, A = Data Valid
   **  3) Vessel Course, degrees True
   **  4) Course Reference
   **  5) Vessel Speed
   **  6) Speed Reference
   **  7) Vessel Set, degrees True
   **  8) Vessel drift (speed)
   **  9) Speed Units
   ** 10) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 10 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   HeadingDegreesTrue       = sentence.Double( 1 );
   IsHeadingValid           = sentence.Boolean( 2 );
   VesselCourseDegreesTrue  = sentence.Double( 3 );
   VesselCourseReference    = sentence.Reference( 4 );
   VesselSpeed              = sentence.Double( 5 );
   VesselSpeedReference     = sentence.Reference( 6 );
   VesselSetDegreesTrue     = sentence.Double( 7 );
   VesselDriftSpeed         = sentence.Double( 8 );
   VesselDriftSpeedUnits    = sentence.Field( 9 );

   return( TRUE );
}

BOOL OSD::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += HeadingDegreesTrue;
   sentence += IsHeadingValid;
   sentence += VesselCourseDegreesTrue;
   sentence += VesselCourseReference;
   sentence += VesselSpeed;
   sentence += VesselSpeedReference;
   sentence += VesselSetDegreesTrue;
   sentence += VesselDriftSpeed;
   sentence += VesselDriftSpeedUnits;

   sentence.Finish();

   return( TRUE );
}

const OSD& OSD::operator = ( const OSD& source )
{
   ASSERT_VALID( this );

   HeadingDegreesTrue       = source.HeadingDegreesTrue;
   IsHeadingValid           = source.IsHeadingValid;
   VesselCourseDegreesTrue  = source.VesselCourseDegreesTrue;
   VesselCourseReference    = source.VesselCourseReference;
   VesselSpeed              = source.VesselSpeed;
   VesselSpeedReference     = source.VesselSpeedReference;
   VesselSetDegreesTrue     = source.VesselSetDegreesTrue;
   VesselDriftSpeed         = source.VesselDriftSpeed;
   VesselDriftSpeedUnits    = source.VesselDriftSpeedUnits;

   return( *this );
}
