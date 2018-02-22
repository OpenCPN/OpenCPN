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

IMPLEMENT_DYNAMIC( LORAN_TIME_DIFFERENCE, CObject )

LORAN_TIME_DIFFERENCE::LORAN_TIME_DIFFERENCE()
{
   Empty();
}

LORAN_TIME_DIFFERENCE::~LORAN_TIME_DIFFERENCE()
{
   Empty();
}

void LORAN_TIME_DIFFERENCE::Empty( void )
{
   ASSERT_VALID( this );
   Microseconds = 0.0;
   SignalStatus = LoranUnknown;
}

void LORAN_TIME_DIFFERENCE::Parse( int first_field_number, const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   Microseconds  = sentence.Double( first_field_number );

   wxString field_data = sentence.Field( first_field_number + 1 );

   if ( field_data == "B" )
   {
      SignalStatus = LoranBlinkWarning;   
   }
   else if ( field_data == "C" )
   {
      SignalStatus = LoranCycleWarning;
   }
   else if ( field_data == "C" )
   {
      SignalStatus = LoranSNRWarning;
   }
   else if ( field_data == "A" )
   {
      SignalStatus = LoranValid;
   }
   else
   {
      SignalStatus = LoranUnknown;
   }
}

void LORAN_TIME_DIFFERENCE::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   sentence += Microseconds;

   switch( SignalStatus )
   {
      case LoranValid:
      
         sentence += "A";
         break; 

      case LoranBlinkWarning:
      
         sentence += "B";
         break; 

      case LoranCycleWarning:
      
         sentence += "C";
         break; 

      case LoranSNRWarning:
      
         sentence += "S";
         break; 

      default:

         sentence += "";
   }
}

const LORAN_TIME_DIFFERENCE& LORAN_TIME_DIFFERENCE::operator = ( const LORAN_TIME_DIFFERENCE& source )
{
   ASSERT_VALID( this );

   Microseconds = source.Microseconds;
   SignalStatus = source.SignalStatus;

   return( *this );
}
