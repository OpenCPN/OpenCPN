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

IMPLEMENT_DYNAMIC( FREQUENCY_AND_MODE, CObject )

FREQUENCY_AND_MODE::FREQUENCY_AND_MODE()
{
   Empty();
}

FREQUENCY_AND_MODE::~FREQUENCY_AND_MODE()
{
   Empty();
}

void FREQUENCY_AND_MODE::Empty( void )
{
   ASSERT_VALID( this );

   Frequency = 0.0;
   Mode      = CommunicationsModeUnknown;
}

void FREQUENCY_AND_MODE::Parse( int first_field_number, const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   Frequency = sentence.Double( first_field_number );
   Mode      = sentence.CommunicationsMode( first_field_number + 1 );
}

void FREQUENCY_AND_MODE::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   sentence += Frequency;
   sentence += Mode;
}

const FREQUENCY_AND_MODE& FREQUENCY_AND_MODE::operator = ( const FREQUENCY_AND_MODE& source )
{
   ASSERT_VALID( this );

   Frequency = source.Frequency;
   Mode      = source.Mode;

   return( *this );
}
