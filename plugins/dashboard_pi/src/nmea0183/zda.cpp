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

//IMPLEMENT_DYNAMIC( ZDA, RESPONSE )

ZDA::ZDA()
{
   Mnemonic = _T("ZDA");
   Empty();
}

ZDA::~ZDA()
{
   Mnemonic.Empty();
   Empty();
}

void ZDA::Empty( void )
{
//   ASSERT_VALID( this );

   UTCTime.Empty();
   Day                   = 0;
   Month                 = 0;
   Year                  = 0;
   LocalHourDeviation    = 0;
   LocalMinutesDeviation = 0;
}

bool ZDA::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** ZDA - Time & Date
   ** UTC, day, month, year and local time zone
   **
   ** $--ZDA,hhmmss.ss,xx,xx,xxxx,xx,xx*hh<CR><LF>
   **        |         |  |  |    |  |
   **        |         |  |  |    |  +- Local zone minutes description, same sign as local hours
   **        |         |  |  |    +- Local zone description, 00 to +- 13 hours
   **        |         |  |  +- Year
   **        |         |  +- Month, 01 to 12
   **        |         +- Day, 01 to 31
   **        +- Universal Time Coordinated (UTC)
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 7 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   UTCTime               = sentence.Field( 1 );
   Day                   = sentence.Integer( 2 );
   Month                 = sentence.Integer( 3 );
   Year                  = sentence.Integer( 4 );
   LocalHourDeviation    = sentence.Integer( 5 );
   LocalMinutesDeviation = sentence.Integer( 6 );

   return( TRUE );
}

bool ZDA::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += UTCTime;
   sentence += Day;
   sentence += Month;
   sentence += Year;
   sentence += LocalHourDeviation;
   sentence += LocalMinutesDeviation;

   sentence.Finish();

   return( TRUE );
}

const ZDA& ZDA::operator = ( const ZDA& source )
{
//   ASSERT_VALID( this );

   UTCTime               = source.UTCTime;
   Day                   = source.Day;
   Month                 = source.Month;
   Year                  = source.Year;
   LocalHourDeviation    = source.LocalHourDeviation;
   LocalMinutesDeviation = source.LocalMinutesDeviation;

   return( *this );
}
