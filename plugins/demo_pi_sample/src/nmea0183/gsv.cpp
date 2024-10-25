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


GSV::GSV()
{
   Mnemonic = _T("GSV");
   Empty();
}

GSV::~GSV()
{
   Mnemonic.Empty();
   Empty();
}

void GSV::Empty( void )
{
   MessageNumber = 0;
   SatsInView = 0;
}

bool GSV::Parse( const SENTENCE& sentence )
{
/*
  $GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75

Where:
      GSV          Satellites in view
      2            Number of sentences for full data
      1            sentence 1 of 2
      08           Number of satellites in view

      01           Satellite PRN number
      40           Elevation, degrees
      083          Azimuth, degrees
      46           SNR - higher is better
           for up to 4 satellites per sentence
      *75          the checksum data, always begins with *
*/


   /*
   ** GSV - GPS satellite Status
   **
   **        1 2 3 4 5 6 7         n
   **        | | | | | | |         |
   ** $--GSV,x,x,x,x,x,x,x.........*hh<CR><LF>
   **
   ** Field Number:
   **  1) Number of sentences for full data
   **  2) sentence number
   **  3) Number of satellites in view
   **  4) Satellite PRN number
   **  5) Elevation, degrees
   **  6) Azimuth, degrees
   **  7) SNR - higher is better
   **  Fields 4-7 may repeat up to 4 times per sentence
   **  n) Checksum
   */

   /*
   ** Ignore the checksum...
   */

   MessageNumber = sentence.Integer( 2 );
   SatsInView = sentence.Integer( 3 );

   return( TRUE );
}

bool GSV::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += 1;
   sentence += 1;
   sentence += SatsInView;

   sentence.Finish();

   return( TRUE );
}

const GSV& GSV::operator = ( const GSV& source )
{
      SatsInView = source.SatsInView;

   return( *this );
}
