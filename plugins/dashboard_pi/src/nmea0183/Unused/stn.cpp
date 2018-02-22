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

IMPLEMENT_DYNAMIC( STN, RESPONSE )

STN::STN()
{
   Mnemonic = "STN";
   Empty();
}

STN::~STN()
{
   Mnemonic.Empty();
   Empty();
}

void STN::Empty( void )
{
   ASSERT_VALID( this );

   TalkerIDNumber = 0;
}

BOOL STN::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** STN - Multiple Data ID
   **
   **        1   2
   **        |   |
   ** $--STN,x.x,*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Talker ID Number
   **  2) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 2 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   TalkerIDNumber   = sentence.Integer( 1 );

   return( TRUE );
}

BOOL STN::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += TalkerIDNumber;

   sentence.Finish();

   return( TRUE );
}

const STN& STN::operator = ( const STN& source )
{
   ASSERT_VALID( this );

   TalkerIDNumber = source.TalkerIDNumber;

   return( *this );
}
