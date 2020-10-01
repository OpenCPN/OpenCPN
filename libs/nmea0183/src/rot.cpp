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

ROT::ROT()
{
   Mnemonic = _T("ROT");
   Empty();
}

ROT::~ROT()
{
   Mnemonic.Empty();
   Empty();
}

void ROT::Empty( void )
{
   RotSensorDegreesMinute = 0.0;
}

bool ROT::Parse( const SENTENCE& sentence )
{
   /*
   **ROT - Rate Of Turn

   **     1   2 3 
   **     |   | | 
   **  $--ROT,x.x,A*hh<CR><LF>

   **  Field Number:  
   **  1) Rate Of Turn, degrees per minute, "-" means bow turns to port 
   **  2) Status, A means data is valid 
   **  3) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 3 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   RotSensorDegreesMinute       = sentence.Double( 1 );
   

   return( TRUE );
}

bool ROT::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += RotSensorDegreesMinute;
   

   sentence.Finish();

   return( TRUE );
}

const ROT& ROT::operator = ( const ROT& source )
{
   RotSensorDegreesMinute = source.RotSensorDegreesMinute;
   

   return( *this );
}
