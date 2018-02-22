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

IMPLEMENT_DYNAMIC( OLN, RESPONSE )

OLN::OLN()
{
   Mnemonic = "OLN";
   Empty();
}

OLN::~OLN()
{
   Mnemonic.Empty();
   Empty();
}

void OLN::Empty( void )
{
   ASSERT_VALID( this );

   Pair1.Empty();
   Pair2.Empty();
   Pair3.Empty();
}

BOOL OLN::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** OLN - Omega Lane Numbers
   **
   **        1          2          3          4
   **        |--------+ |--------+ |--------+ |
   ** $--OLN,aa,xxx,xxx,aa,xxx,xxx,aa,xxx,xxx*hh<CR><LF>
   **
   **  1) Omega Pair 1
   **  2) Omega Pair 1
   **  3) Omega Pair 1
   **  4) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 10 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   Pair1.Parse( 1, sentence );
   Pair2.Parse( 4, sentence );
   Pair3.Parse( 7, sentence );

   return( TRUE );
}

BOOL OLN::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   Pair1.Write( sentence );
   Pair2.Write( sentence );
   Pair3.Write( sentence );

   sentence.Finish();

   return( TRUE );
}

const OLN& OLN::operator = ( const OLN& source )
{
   ASSERT_VALID( this );

   Pair1 = source.Pair1;
   Pair2 = source.Pair2;
   Pair3 = source.Pair3;

   return( *this );
}
