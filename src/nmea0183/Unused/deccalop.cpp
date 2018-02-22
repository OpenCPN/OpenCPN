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

IMPLEMENT_DYNAMIC( LINE_OF_POSITION, CObject )

LINE_OF_POSITION::LINE_OF_POSITION()
{
   Empty();
}

LINE_OF_POSITION::~LINE_OF_POSITION()
{
   Empty();
}

void LINE_OF_POSITION::Empty( void )
{
   ASSERT_VALID( this );

   ZoneID.Empty();
   LineOfPosition = 0.0;
   MasterLine     = Unknown;
}

void LINE_OF_POSITION::Parse( int first_field_number, const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   ZoneID         = sentence.Field(   first_field_number );
   LineOfPosition = sentence.Double(  first_field_number + 1 );
   MasterLine     = sentence.Boolean( first_field_number + 2 );
}

void LINE_OF_POSITION::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   sentence += ZoneID;
   sentence += LineOfPosition;
   sentence += MasterLine;
}

const LINE_OF_POSITION& LINE_OF_POSITION::operator = ( const LINE_OF_POSITION& source )
{
   ASSERT_VALID( this );

   ZoneID         = source.ZoneID;
   LineOfPosition = source.LineOfPosition;
   MasterLine     = source.MasterLine;

   return( *this );
}
