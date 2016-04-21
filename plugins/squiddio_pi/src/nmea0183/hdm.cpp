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


HDM::HDM()
{
   Mnemonic = _T("HDM");
   Empty();
}

HDM::~HDM()
{
   Mnemonic.Empty();
   Empty();
}

void HDM::Empty( void )
{
   DegreesMagnetic = 0.0;
}

bool HDM::Parse( const SENTENCE& sentence )
{

   /*
   ** HDM - Heading - Magnetic
   **
   **        1   2 3
   **        |   | |
   ** $--HDM,x.x,M*hh<CR><LF>
   **
   ** Field Number:
   **  1) Heading Degrees, Magnetic
   **  2) M = Magnetic
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

   DegreesMagnetic = sentence.Double( 1 );

   return( TRUE );
}

bool HDM::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += DegreesMagnetic;
   sentence += _T("M");

   sentence.Finish();

   return( TRUE );
}

const HDM& HDM::operator = ( const HDM& source )
{
      DegreesMagnetic = source.DegreesMagnetic;

   return( *this );
}
