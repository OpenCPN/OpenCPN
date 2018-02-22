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

IMPLEMENT_DYNAMIC( TTM, RESPONSE )

TTM::TTM()
{
   Mnemonic = "TTM";
   Empty();
}

TTM::~TTM()
{
   Mnemonic.Empty();
   Empty();
}

void TTM::Empty( void )
{
   ASSERT_VALID( this );

   TargetNumber                            = 0;
   TargetDistance                          = 0.0;
   BearingFromOwnShip                      = 0.0;
   BearingUnits.Empty();
   TargetSpeed                             = 0.0;
   TargetCourse                            = 0.0;
   TargetCourseUnits.Empty();
   DistanceOfClosestPointOfApproach        = 0.0;
   NumberOfMinutesToClosestPointOfApproach = 0.0;
   Increasing.Empty();
   TargetName.Empty();
   TargetStatus                            = TargetUnknown;
}

BOOL TTM::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** TTM - Tracked Target Message
   **
   **                                         11     13
   **        1  2   3   4 5   6   7 8   9   10|    12| 14
   **        |  |   |   | |   |   | |   |   | |    | | |
   ** $--TTM,xx,x.x,x.x,a,x.x,x.x,a,x.x,x.x,a,c--c,a,a*hh<CR><LF>
   **
   **  1) Target Number
   **  2) Target Distance
   **  3) Bearing from own ship
   **  4) Bearing Units
   **  5) Target speed
   **  6) Target Course
   **  7) Course Units
   **  8) Distance of closest-point-of-approach
   **  9) Time until closest-point-of-approach "-" means increasing
   ** 10) "-" means increasing
   ** 11) Target name
   ** 12) Target Status
   ** 13) Reference Target
   ** 14) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 14 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   TargetNumber                            = sentence.Integer( 1 );
   TargetDistance                          = sentence.Double( 2 );
   BearingFromOwnShip                      = sentence.Double( 3 );
   BearingUnits                            = sentence.Field( 4 );
   TargetSpeed                             = sentence.Double( 5 );
   TargetCourse                            = sentence.Double( 6 );
   TargetCourseUnits                       = sentence.Field( 7 );
   DistanceOfClosestPointOfApproach        = sentence.Double( 8 );
   NumberOfMinutesToClosestPointOfApproach = sentence.Double( 9 );
   Increasing                              = sentence.Field( 10 );
   TargetName                              = sentence.Field( 11 );
   
   CString field_data = sentence.Field( 12 );

   if ( field_data == "L" )
   {
      TargetStatus = TargetLost;
   }
   else if ( field_data == "Q" )
   {
      TargetStatus = TargetQuery;
   }
   else if ( field_data == "T" )
   {
      TargetStatus = TargetTracking;
   }
   else
   {
      TargetStatus = TargetUnknown;
   }

   ReferenceTarget = sentence.Field( 13 );

   return( TRUE );
}

BOOL TTM::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += TargetNumber;
   sentence += TargetDistance;
   sentence += BearingFromOwnShip;
   sentence += BearingUnits;
   sentence += TargetSpeed;
   sentence += TargetCourse;
   sentence += TargetCourseUnits;
   sentence += DistanceOfClosestPointOfApproach;
   sentence += NumberOfMinutesToClosestPointOfApproach;
   sentence += Increasing;
   sentence += TargetName;
   
   switch( TargetStatus )
   {
      case TargetLost:

         sentence += "L";
         break;

      case TargetQuery:

         sentence += "Q";
         break;

      case TargetTracking:

         sentence += "T";
         break;

      default:

         sentence += "";
         break;
   }

   sentence += ReferenceTarget;

   sentence.Finish();

   return( TRUE );
}

const TTM& TTM::operator = ( const TTM& source )
{
   ASSERT_VALID( this );

   TargetNumber                            = source.TargetNumber;
   TargetDistance                          = source.TargetDistance;
   BearingFromOwnShip                      = source.BearingFromOwnShip;
   BearingUnits                            = source.BearingUnits;
   TargetSpeed                             = source.TargetSpeed;
   TargetCourse                            = source.TargetCourse;
   TargetCourseUnits                       = source.TargetCourseUnits;
   DistanceOfClosestPointOfApproach        = source.DistanceOfClosestPointOfApproach;
   NumberOfMinutesToClosestPointOfApproach = source.NumberOfMinutesToClosestPointOfApproach;
   Increasing                              = source.Increasing;
   TargetName                              = source.TargetName;
   TargetStatus                            = source.TargetStatus;

   return( *this );
}
