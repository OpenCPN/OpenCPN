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

IMPLEMENT_DYNAMIC( AAM, RESPONSE )

AAM::AAM()
{
   Mnemonic = "AAM";
   Empty();
}

AAM::~AAM()
{
   Mnemonic.Empty();
   Empty();
}

void AAM::Empty( void )
{
   ASSERT_VALID( this );

   IsArrivalCircleEntered = Unknown;
   IsPerpendicular        = Unknown;
   CircleRadius           = 0.0;
   WaypointID.Empty();
}

BOOL AAM::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** AAM - Waypoint Arrival Alarm
   **
   **        1 2 3   4 5    6
   **        | | |   | |    |
   ** $--AAM,A,A,x.x,N,c--c*hh<CR><LF>
   **
   ** 1) Status, A = Arrival circle entered
   ** 2) Status, A = perpendicular passed at waypoint
   ** 3) Arrival circle radius
   ** 4) Units of radius, nautical miles
   ** 5) Waypoint ID
   ** 6) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 6 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   /*
   ** Line has already been checked for checksum validity
   */

   IsArrivalCircleEntered = sentence.Boolean( 1 );
   IsPerpendicular        = sentence.Boolean( 2 );
   CircleRadius           = sentence.Double( 3 );
   WaypointID             = sentence.Field( 5 );

   return( TRUE );
}

BOOL AAM::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += IsArrivalCircleEntered;
   sentence += IsPerpendicular;
   sentence += CircleRadius;
   sentence += "N";
   sentence += WaypointID;

   sentence.Finish();

   return( TRUE );
}

const AAM& AAM::operator = ( const AAM& source )
{
   ASSERT_VALID( this );

   IsArrivalCircleEntered = source.IsArrivalCircleEntered;
   IsPerpendicular        = source.IsPerpendicular;
   CircleRadius           = source.CircleRadius;
   WaypointID             = source.WaypointID;

   return( *this );
}
