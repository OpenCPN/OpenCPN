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

IMPLEMENT_DYNAMIC( OMEGA_PAIR, CObject )

OMEGA_PAIR::OMEGA_PAIR()
{
   Empty();
}

OMEGA_PAIR::~OMEGA_PAIR()
{
   Empty();
}

void OMEGA_PAIR::Empty( void )
{
   ASSERT_VALID( this );

   Name.Empty();
   LaneNumber      = 0;
   CentilaneNumber = 0;
}

void OMEGA_PAIR::Parse( int first_field_number, const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   Name            = sentence.Field( first_field_number );
   LaneNumber      = sentence.Integer( first_field_number + 1 );
   CentilaneNumber = sentence.Integer( first_field_number + 2 );
}

void OMEGA_PAIR::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   sentence += Name;
   sentence += LaneNumber;
   sentence += CentilaneNumber;
}

const OMEGA_PAIR& OMEGA_PAIR::operator = ( const OMEGA_PAIR& source )
{
   ASSERT_VALID( this );

   Name            = source.Name;
   LaneNumber      = source.LaneNumber;
   CentilaneNumber = source.CentilaneNumber;

   return( *this );
}
