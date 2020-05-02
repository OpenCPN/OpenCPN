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

//IMPLEMENT_DYNAMIC( VTG, RESPONSE )

VTG::VTG()
{
   Mnemonic = _T("VTG");
   Empty();
}

VTG::~VTG()
{
   Mnemonic.Empty();
   Empty();
}

void VTG::Empty( void )
{
//   ASSERT_VALID( this );

   TrackDegreesTrue       = 0.0;
   TrackDegreesMagnetic   = 0.0;
   SpeedKnots             = 0.0;
   SpeedKilometersPerHour = 0.0;
}

bool VTG::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** VTG - Track made good and Ground speed
   **
   **        1   2 3   4 5	 6 7   8 9
   **        |   | |   | |	 | |   | |
   ** $--VTG,x.x,T,x.x,M,x.x,N,x.x,K*hh<CR><LF>
   **
   ** Field Number:
   **  1) Track Degrees
   **  2) T = True
   **  3) Track Degrees
   **  4) M = Magnetic
   **  5) Speed Knots
   **  6) N = Knots
   **  7) Speed Kilometers Per Hour
   **  8) K = Kilometers Per Hour
   **  9) Checksum
   */

   /*
   ** First we check the checksum...
   */

      int target_field_count = 8;

      NMEA0183_BOOLEAN check = sentence.IsChecksumBad( 9 );

      if ( check == NTrue )
      {

  /*
      ** This may be an NMEA Version 2.3 sentence, with "Mode" field
  */
            wxString checksum_in_sentence = sentence.Field( 9 );
            if(checksum_in_sentence.StartsWith(_T("*")))       // Field is a valid erroneous checksum
            {
                  SetErrorMessage( _T("Invalid Checksum") );
                  return( FALSE );
            }

           else
           {
                  target_field_count = 9;
                  check = sentence.IsChecksumBad( 10 );
                  if( check == NTrue)
                  {
                        SetErrorMessage( _T("Invalid Checksum") );
                        return( FALSE );
                  }
            }
      }



   if ( sentence.GetNumberOfDataFields() != target_field_count )
   {
         SetErrorMessage( _T("Invalid FieldCount") );
         return( FALSE );
   }


   TrackDegreesTrue       = sentence.Double( 1 );
   TrackDegreesMagnetic   = sentence.Double( 3 );
   SpeedKnots             = sentence.Double( 5 );
   SpeedKilometersPerHour = sentence.Double( 7 );

   return( TRUE );
}

bool VTG::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += TrackDegreesTrue;
   sentence += _T("T");
   sentence += TrackDegreesMagnetic;
   sentence += _T("M");
   sentence += SpeedKnots;
   sentence += _T("N");
   sentence += SpeedKilometersPerHour;
   sentence += _T("K");

   sentence.Finish();

   return( TRUE );
}

const VTG& VTG::operator = ( const VTG& source )
{
//   ASSERT_VALID( this );

   TrackDegreesTrue       = source.TrackDegreesTrue;
   TrackDegreesMagnetic   = source.TrackDegreesMagnetic;
   SpeedKnots             = source.SpeedKnots;
   SpeedKilometersPerHour = source.SpeedKilometersPerHour;

   return( *this );
}
