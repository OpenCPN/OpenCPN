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

IMPLEMENT_DYNAMIC( ROT, RESPONSE )

ROT::ROT()
{
   Mnemonic = "ROT";
   Empty();
}

ROT::~ROT()
{
   Mnemonic.Empty();
   Empty();
}

void ROT::Empty( void )
{
   ASSERT_VALID( this );

   RateOfTurn  = 0.0;
   IsDataValid = Unknown;
}

BOOL ROT::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** ROT - Rate Of Turn
   **
   **        1   2 3
   **        |   | |
   ** $--ROT,x.x,A*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Rate Of Turn, degrees per minute, "-" means bow turns to port
   **  2) Status, A means data is valid
   **  3) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 3 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   RateOfTurn  = sentence.Double( 1 );
   IsDataValid = sentence.Boolean( 2 );

   return( TRUE );
}

BOOL ROT::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += RateOfTurn;
   sentence += IsDataValid;
   
   sentence.Finish();

   return( TRUE );
}

const ROT& ROT::operator = ( const ROT& source )
{
   ASSERT_VALID( this );

   RateOfTurn  = source.RateOfTurn;
   IsDataValid = source.IsDataValid;

   return( *this );
}
