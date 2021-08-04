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

VLW::VLW()
{
   Mnemonic = _T("VLW");
   Empty();
}

VLW::~VLW()
{
   Mnemonic.Empty();
   Empty();
}

void VLW::Empty( void )
{
   TotalMileage = 0.0;
   TripMileage  = 0.0;
   }

bool VLW::Parse( const SENTENCE& sentence )
{
   /*
   VLW - Distance Traveled through Water 

        1   2 3   4 5 
        |   | |   | | 
 $--VLW,x.x,N,x.x,N*hh<CR><LF> 

 Field Number:  
  1) Total cumulative distance 
  2) N = Nautical Miles 
  3) Distance since Reset 
  4) N = Nautical Miles 
  5) Checksum 

   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 5 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   TotalMileage = sentence.Double( 1 );
   TripMileage  = sentence.Double( 3 );
   
   return( TRUE );
}

bool VLW::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += TotalMileage;
   sentence += _T("N");
   sentence += TripMileage;
   sentence += _T("N");
   sentence.Finish();

   return( TRUE );
}

const VLW& VLW::operator = ( const VLW& source )
{
   TotalMileage = source.TotalMileage;
   TripMileage  = source.TripMileage;
      return( *this );
}
