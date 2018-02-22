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

IMPLEMENT_DYNAMIC( XDR, RESPONSE )

XDR::XDR()
{
   Mnemonic = "XDR";
   Empty();
}

XDR::~XDR()
{
   Mnemonic.Empty();
   Empty();
}

void XDR::Empty( void )
{
   ASSERT_VALID( this );
   delete_all_entries();
}

BOOL XDR::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** XDR - Cross Track Error - Dead Reckoning
   **
   **        1 2   3 4			    n
   **        | |   | |            |
   ** $--XDR,a,x.x,a,c--c, ..... *hh<CR><LF>
   **
   ** Field Number: 
   **  1) Transducer Type
   **  2) Measurement Data
   **  3) Units of measurement
   **  4) Name of transducer
   **  x) More of the same
   **  n) Checksum
   */

   delete_all_entries();

   int field_number = 1;

   TRANSDUCER_DATA *transducer_data_p = (TRANSDUCER_DATA *) NULL;

   while( sentence.Field( field_number + 1 ) != "" )
   {
      transducer_data_p = new TRANSDUCER_DATA;

      transducer_data_p->Parse( field_number, sentence );
      Transducers.Add( transducer_data_p );
      
      field_number += 4;
   }

   return( TRUE );
}

BOOL XDR::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   int index = 0;
   int number_of_entries = Transducers.GetSize();

   while( index < number_of_entries )
   {
      ( (TRANSDUCER_DATA *) Transducers[ index ] )->Write( sentence );
      index++;
   }

   sentence.Finish();

   return( TRUE );
}

void XDR::delete_all_entries( void )
{
   ASSERT_VALID( this );

   int index = 0;
   int number_of_entries = Transducers.GetSize();

   while( index < number_of_entries )
   {
      delete Transducers[ index ];
      index++;
   }

   Transducers.RemoveAll();
}

IMPLEMENT_DYNAMIC( TRANSDUCER_DATA, CObject );

TRANSDUCER_DATA::TRANSDUCER_DATA()
{
   Empty();
}

TRANSDUCER_DATA::~TRANSDUCER_DATA()
{
   Empty();
}

void TRANSDUCER_DATA::Empty( void )
{
   ASSERT_VALID( this );

   TransducerType  = TransducerUnknown;
   MeasurementData = 0.0;
   MeasurementUnits.Empty();
   TransducerName.Empty();
}

void TRANSDUCER_DATA::Parse( int first_field_number, const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   TransducerType   = sentence.TransducerType( first_field_number );
   MeasurementData  = sentence.Double( first_field_number + 1 );
   MeasurementUnits = sentence.Field(  first_field_number + 2 );
   TransducerName   = sentence.Field(  first_field_number + 3 );
}

void TRANSDUCER_DATA::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   sentence += TransducerType;
   sentence += MeasurementData;
   sentence += MeasurementUnits;
   sentence += TransducerName;
}

const TRANSDUCER_DATA& TRANSDUCER_DATA::operator = ( const TRANSDUCER_DATA& source )
{
   ASSERT_VALID( this );

   TransducerType   = source.TransducerType;
   MeasurementData  = source.MeasurementData;
   MeasurementUnits = source.MeasurementUnits;
   TransducerName   = source.TransducerName;

   return( *this );
}
