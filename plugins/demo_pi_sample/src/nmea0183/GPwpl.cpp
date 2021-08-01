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

//IMPLEMENT_DYNAMIC( GPWPL, RESPONSE )

GPWPL::GPWPL()
{
   Mnemonic = _T("GPwpl");
   Empty();
}

GPWPL::~GPWPL()
{
   Mnemonic.Empty();
   Empty();
}

void GPWPL::Empty( void )
{

   Position.Empty();
   To.Empty();
}

bool GPWPL::Parse( const SENTENCE& sentence )
{

   /*
   ** WPL - Waypoint Location
   **
   **        +-------------------------------- 1) Latitude
   **        |       +------------------------ 2) N or S (North or South)
   **        |       | +---------------------- 3) Longitude
   **        |       | |        +------------- 4) E or W (East or West)
   **        |       | |        | +----------- 5) Waypoint name
   **        |       | |        | |    +-------6) Checksum
   **        |       | |        | |    |
   ** $--WPL,llll.ll,a,yyyyy.yy,a,c--c*hh<CR><LF>
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 6 ) == NTrue )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   Position.Parse( 1, 2, 3, 4, sentence );
   To = sentence.Field( 5 );

   return( TRUE );
}

bool GPWPL::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += Position;
   sentence += To;
   sentence += _T("");            // color = black
   sentence += _T("@q");             // comment, minimum
   sentence += _T("A");
   sentence += _T("");
   sentence += _T("");
   sentence += _T("");

   // No checksum required
   wxString temp_string;
   temp_string.Printf(_T("%c%c"), CARRIAGE_RETURN, LINE_FEED );
   sentence += temp_string;

   return( TRUE );
}

const GPWPL& GPWPL::operator = ( const GPWPL& source )
{

   Position = source.Position;
   To       = source.To;

   return( *this );
}
