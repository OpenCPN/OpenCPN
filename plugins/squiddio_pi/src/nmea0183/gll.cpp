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

//IMPLEMENT_DYNAMIC( GLL, RESPONSE )

GLL::GLL()
{
   Mnemonic = _T("GLL");
   Empty();
}

GLL::~GLL()
{
   Mnemonic.Empty();
   Empty();
}

void GLL::Empty( void )
{
//   ASSERT_VALID( this );

   Position.Empty();
   UTCTime.Empty();
   IsDataValid = Unknown0183;
}

bool GLL::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** GLL - Geographic Position - Latitude/Longitude
   ** Latitude, N/S, Longitude, E/W, UTC, Status
   **
   **        +-------------------------------- 1) Latitude
   **        |       +------------------------ 2) N or S (North or South)
   **        |       | +---------------------- 3) Longitude
   **        |       | |        +------------- 4) E or W (East or West)
   **        |       | |        | +----------- 5) Universal Time Coordinated (UTC)
   **        |       | |        | |         +- 6) Status A - Data Valid, V - Data Invalid
   **        |       | |        | |         | +7) Checksum
   ** $--GLL,llll.ll,a,yyyyy.yy,a,hhmmss.ss,A*hh<CR><LF>
   */

   /*
   ** First we check the checksum...
   */

      int target_field_count = 6;

      NMEA0183_BOOLEAN check = sentence.IsChecksumBad( 7 );

      if ( check == NTrue )
      {

  /*
            ** This may be an NMEA Version 2.3 sentence, with "Mode" field
  */
            wxString checksum_in_sentence = sentence.Field( 7 );
            if(checksum_in_sentence.StartsWith(_T("*")))       // Field is a valid erroneous checksum
            {
                  SetErrorMessage( _T("Invalid Checksum") );
                  return( FALSE );
            }

            else
            {
                  target_field_count = 7;
                  check = sentence.IsChecksumBad( 8 );
                  if( check == NTrue)
                  {
                        SetErrorMessage( _T("Invalid Checksum") );
                        return( FALSE );
                  }
            }
      }


      if ( sentence.GetNumberOfDataFields() == target_field_count )
      {
            Position.Parse( 1, 2, 3, 4, sentence );
            UTCTime     = sentence.Field( 5 );
            IsDataValid = sentence.Boolean( 6 );

            return( TRUE );
      }

      //    May be old style GLL sentence
      if ( sentence.GetNumberOfDataFields() == 4 )
      {
            Position.Parse( 1, 2, 3, 4, sentence );
            IsDataValid = NTrue;

            return( TRUE );
      }

      //    A real error...
      SetErrorMessage( _T("Invalid FieldCount") );
      return( FALSE );
}


const wxString& GLL::PlainEnglish( void )
{
//   ASSERT_VALID( this );

   static wxString return_string;

   return_string.Empty();
/*
   char temp_string[ 128 ];

   sprintf( temp_string, "At %d, you were at Latitude %ld %s, Longitude %ld %s.",
            (const char *) UTCTime,
            Position.Latitude.Latitude,
            ( Position.Latitude.Northing == North ) ? "North" : "South",
            Position.Longitude.Longitude,
            ( Position.Longitude.Easting == East ) ? "East" : "West" );

   return_string = temp_string;
*/
   return( return_string );
}


bool GLL::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += Position;
   sentence += UTCTime;
   sentence += IsDataValid;

   sentence.Finish();

   return( TRUE );
}

const GLL& GLL::operator = ( const GLL& source )
{
//   ASSERT_VALID( this );

   Position    = source.Position;
   UTCTime     = source.UTCTime;
   IsDataValid = source.IsDataValid;

   return( *this );
}
