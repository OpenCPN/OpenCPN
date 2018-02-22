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

IMPLEMENT_DYNAMIC( RMA, RESPONSE )

RMA::RMA()
{
   Mnemonic = "RMA";
   Empty();
}

RMA::~RMA()
{
   Mnemonic.Empty();
   Empty();
}

void RMA::Empty( void )
{
   ASSERT_VALID( this );

   IsDataValid                = Unknown;
   TimeDifferenceA            = 0.0;
   TimeDifferenceB            = 0.0;
   SpeedOverGroundKnots       = 0.0;
   TrackMadeGoodDegreesTrue   = 0.0;
   MagneticVariation          = 0.0;
   MagneticVariationDirection = EW_Unknown;
}

BOOL RMA::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** RMA - Recommended Minimum Navigation Information
   **                                                    12
   **        1 2       3 4        5 6   7   8   9   10  11|
   **        | |       | |        | |   |   |   |   |   | |
   ** $--RMA,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,x.x,x.x,x.x,a*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Blink Warning
   **  2) Latitude
   **  3) N or S
   **  4) Longitude
   **  5) E or W
   **  6) Time Difference A, uS
   **  7) Time Difference B, uS
   **  8) Speed Over Ground, Knots
   **  9) Track Made Good, degrees true
   ** 10) Magnetic Variation, degrees
   ** 11) E or W
   ** 12) Checksum
   */

   /*
   ** First we check the checksum...
   */

   NMEA0183_BOOLEAN check = sentence.IsChecksumBad( 12 );

   if ( check == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   }
   
   if ( check == Unknown )
   {
      SetErrorMessage( "Missing Checksum" );
      return( FALSE );
   } 

   IsDataValid                = sentence.Boolean( 1 );
   Position.Parse( 2, 3, 4, 5, sentence );
   TimeDifferenceA            = sentence.Double( 6 );
   TimeDifferenceB            = sentence.Double( 7 );
   SpeedOverGroundKnots       = sentence.Double( 8 );
   TrackMadeGoodDegreesTrue   = sentence.Double( 9 );
   MagneticVariation          = sentence.Double( 10 );
   MagneticVariationDirection = sentence.EastOrWest( 11 );

   return( TRUE );
}

BOOL RMA::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += IsDataValid;
   sentence += Position;
   sentence += TimeDifferenceA;
   sentence += TimeDifferenceB;
   sentence += SpeedOverGroundKnots;
   sentence += TrackMadeGoodDegreesTrue;
   sentence += MagneticVariation;
   sentence += MagneticVariationDirection;

   sentence.Finish();

   return( TRUE );
}

const RMA& RMA::operator = ( const RMA& source )
{
   ASSERT_VALID( this );

   IsDataValid                = source.IsDataValid;
   Position                   = source.Position;
   TimeDifferenceA            = source.TimeDifferenceA;
   TimeDifferenceB            = source.TimeDifferenceB;
   SpeedOverGroundKnots       = source.SpeedOverGroundKnots;
   TrackMadeGoodDegreesTrue   = source.TrackMadeGoodDegreesTrue;
   MagneticVariation          = source.MagneticVariation;
   MagneticVariationDirection = source.MagneticVariationDirection;

   return( *this );
}
