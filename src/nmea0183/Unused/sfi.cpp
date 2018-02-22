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

IMPLEMENT_DYNAMIC( SFI, RESPONSE )

SFI::SFI()
{
   Mnemonic = "SFI";
   Empty();
}

SFI::~SFI()
{
   Mnemonic.Empty();
   Empty();
}

void SFI::Empty( void )
{
   ASSERT_VALID( this );

   TotalMessages = 0.0;
   MessageNumber = 0.0;

   int index = 0;

   while( index < 6 )
   {
      Frequencies[ index ].Empty();
      index++;
   }

}

BOOL SFI::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** SFI - Scanning Frequency Information
   **
   **        1   2   3      4                     x
   **        |   |   |      |                     |
   ** $--SFI,x.x,x.x,xxxxxx,c .......... xxxxxx,c*hh<CR><LF>
   **
   **  1) Total Number Of Messages
   **  2) Message Number
   **  3) Frequency 1
   **  4) Mode 1
   **  x) Checksum
   */

   /*
   ** First we check the checksum...
   */

   int number_of_data_fields = sentence.GetNumberOfDataFields();

   if ( sentence.IsChecksumBad( number_of_data_fields + 1 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   TotalMessages = sentence.Double( 1 );
   MessageNumber = sentence.Double( 2 );

   /*
   ** Clear out any old data
   */

   int index = 0;

   while( index < 6 )
   {
      Frequencies[ index ].Empty();
      index++;
   }

   int number_of_frequencies = ( number_of_data_fields - 2 ) / 2;
   int frequency_number      = 0;

   /*
   ** index is the number of data fields before the frequency/mode +
   ** the frequency number times the number of fields in a FREQUENC_AND_MODE
   */

   while( frequency_number < number_of_frequencies )
   {
      index = 2 + ( frequency_number * 2 );

      Frequencies[ frequency_number ].Parse( index, sentence );

      frequency_number++;
   }

   return( TRUE );
}

BOOL SFI::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += TotalMessages;
   sentence += MessageNumber;

   int index = 0;

   while( index < 6 )
   {
      Frequencies[ index ].Write( sentence );
      index++;
   }

   sentence.Finish();

   return( TRUE );
}

const SFI& SFI::operator = ( const SFI& source )
{
   ASSERT_VALID( this );

   TotalMessages = source.TotalMessages;
   MessageNumber = source.MessageNumber;

   int index = 0;

   while( index < 6 )
   {
      Frequencies[ index ] = source.Frequencies[ index ];
      index++;
   }

   return( *this );
}
