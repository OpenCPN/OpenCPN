/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register, Hakan Svensson
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Samuel R. Blackburn.                            *
 *   2025 by David S Register, Hakan Svensson                              *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
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


THS::THS()
{
   Mnemonic = _T("THS");
   Empty();
}

THS::~THS()
{
   Mnemonic.Empty();
   Empty();
}

void THS::Empty( void )
{
  TrueHeading = 0.0;
  ModeInd.Empty();
}

bool THS::Parse( const SENTENCE& sentence )
{

   /*
   ** THS - True heading and status
   **
   **        1   2 3
   **        |   | |
   ** $--THS,x.x,A*hh<CR><LF>
   **
   ** Field Number:
   **  1) Heading Degrees, TRUE
   **  2) Mode indicator A = Autonomous. Mode: E/M/S/V, are not valid for navigation
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

   TrueHeading = sentence.Double(1);
   ModeInd = sentence.Field( 2 );

   return( TRUE );
}

bool THS::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += TrueHeading;
   sentence += ModeInd;

   sentence.Finish();

   return( TRUE );
}

const THS& THS::operator = ( const THS& source )
{
  TrueHeading = source.TrueHeading;
  ModeInd = source.ModeInd;

   return( *this );
}
