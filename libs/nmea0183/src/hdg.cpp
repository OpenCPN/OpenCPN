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

HDG::HDG()
{
   Mnemonic = _T("HDG");
   Empty();
}

HDG::~HDG()
{
   Mnemonic.Empty();
   Empty();
}

void HDG::Empty( void )
{
   MagneticSensorHeadingDegrees = 0.0;
   MagneticDeviationDegrees     = 0.0;
   MagneticDeviationDirection   = EW_Unknown;
   MagneticVariationDegrees     = 0.0;
   MagneticVariationDirection   = EW_Unknown;
}

bool HDG::Parse( const SENTENCE& sentence )
{
   /*
   ** HDG - Heading - Deviation & Variation
   **
   **        1   2   3 4   5 6
   **        |   |   | |   | |
   ** $--HDG,x.x,x.x,a,x.x,a*hh<CR><LF>
   **
   ** Field Number:
   **  1) Magnetic Sensor heading in degrees
   **  2) Magnetic Deviation, degrees
   **  3) Magnetic Deviation direction, E = Easterly, W = Westerly
   **  4) Magnetic Variation degrees
   **  5) Magnetic Variation direction, E = Easterly, W = Westerly
   **  6) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 6 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   MagneticSensorHeadingDegrees = sentence.Double( 1 );
   MagneticDeviationDegrees     = sentence.Double( 2 );
   MagneticDeviationDirection   = sentence.EastOrWest( 3 );
   MagneticVariationDegrees     = sentence.Double( 4 );
   MagneticVariationDirection   = sentence.EastOrWest( 5 );

   return( TRUE );
}

bool HDG::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += MagneticSensorHeadingDegrees;
   sentence += MagneticDeviationDegrees;
   sentence += MagneticDeviationDirection;
   sentence += MagneticVariationDegrees;
   sentence += MagneticVariationDirection;

   sentence.Finish();

   return( TRUE );
}

const HDG& HDG::operator = ( const HDG& source )
{
   MagneticSensorHeadingDegrees = source.MagneticSensorHeadingDegrees;
   MagneticDeviationDegrees     = source.MagneticDeviationDegrees;
   MagneticDeviationDirection   = source.MagneticDeviationDirection;
   MagneticVariationDegrees     = source.MagneticVariationDegrees;
   MagneticVariationDirection   = source.MagneticVariationDirection;

   return( *this );
}
