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

IMPLEMENT_DYNAMIC( RSD, RESPONSE )

RSD::RSD()
{
   Mnemonic = "RSD";
   Empty();
}

RSD::~RSD()
{
   Mnemonic.Empty();
   Empty();
}

void RSD::Empty( void )
{
   ASSERT_VALID( this );

   Data1.Empty();
   Data2.Empty();
   CursorRangeFromOwnShip                = 0.0;
   CursorBearingDegreesClockwiseFromZero = 0.0;
   RangeScale                            = 0.0;
   RangeUnits.Empty();
   DisplayRotation                       = RotationUnknown;
}

BOOL RSD::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** RSD - RADAR System Data
   **                                                        14
   **        1   2   3   4   5   6   7   8   9   10  11 12 13|
   **        |   |   |   |   |   |   |   |   |   |   |   | | |
   ** $--RSD,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,a,a*hh<CR><LF>
   **
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

   Data1.Parse( 1, sentence );
   Data2.Parse( 5, sentence );
   CursorRangeFromOwnShip                = sentence.Double( 9 );
   CursorBearingDegreesClockwiseFromZero = sentence.Double( 10 );
   RangeScale                            = sentence.Double( 11 );
   RangeUnits                            = sentence.Field( 12 );

   int temp_integer = sentence.Integer( 13 );

   switch( temp_integer )
   {
      case 'C':

         DisplayRotation = CourseUpRotation;
         break;

      case 'H':

         DisplayRotation = HeadUpRotation;
         break;

      case 'N':

         DisplayRotation = NorthUpRotation;
         break;

      default:

         DisplayRotation = RotationUnknown;
         break;
   }

   return( TRUE );
}

BOOL RSD::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   Data1.Write( sentence );
   Data2.Write( sentence );
   sentence += CursorRangeFromOwnShip;
   sentence += CursorBearingDegreesClockwiseFromZero;
   sentence += RangeScale;
   sentence += RangeUnits;

   switch( DisplayRotation )
   {
      case CourseUpRotation:

         sentence += "C";
         break;

      case HeadUpRotation:

         sentence += "H";
         break;

      case NorthUpRotation:

         sentence += "N";
         break;

      default:

         sentence += "";
         break;
   }

   sentence.Finish();

   return( TRUE );
}

const RSD& RSD::operator = ( const RSD& source )
{
   ASSERT_VALID( this );

   Data1                                 = source.Data1;
   Data2                                 = source.Data2;
   CursorRangeFromOwnShip                = source.CursorRangeFromOwnShip;
   CursorBearingDegreesClockwiseFromZero = source.CursorBearingDegreesClockwiseFromZero;
   RangeScale                            = source.RangeScale;
   RangeUnits                            = source.RangeUnits;
   DisplayRotation                       = source.DisplayRotation;

   return( *this );
}
