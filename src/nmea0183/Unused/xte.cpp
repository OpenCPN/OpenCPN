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

IMPLEMENT_DYNAMIC( XTE, RESPONSE )

XTE::XTE()
{
   Mnemonic = "XTE";
   Empty();
}

XTE::~XTE()
{
   Mnemonic.Empty();
   Empty();
}

void XTE::Empty( void )
{
   ASSERT_VALID( this );

   CrossTrackErrorMagnitude = 0.0;
   DirectionToSteer         = LR_Unknown;
   CrossTrackUnits.Empty();
}

BOOL XTE::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   CString field_data;

   /*
   ** XTE - Cross-Track Error, Measured
   **
   **        1 2 3   4 5  6
   **        | | |   | |  |
   ** $--XTE,A,A,x.x,a,N,*hh<CR><LF>
   **
   **  1) Status
   **     V = LORAN-C Blink or SNR warning
   **     V = general warning flag or other navigation systems when a reliable
   **         fix is not available
   **  2) Status
   **     V = Loran-C Cycle Lock warning flag
   **     A = OK or not used
   **  3) Cross Track Error Magnitude
   **  4) Direction to steer, L or R
   **  5) Cross Track Units, N = Nautical Miles
   **  6) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 6 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   /*
   ** Line has already been checked for checksum validity
   */

   IsLoranBlinkOK           = sentence.Boolean( 1 );
   IsLoranCCycleLockOK      = sentence.Boolean( 2 );
   CrossTrackErrorMagnitude = sentence.Double( 3 );
   DirectionToSteer         = sentence.LeftOrRight( 4 );
   CrossTrackUnits          = sentence.Field( 5 );

   return( TRUE );
}

BOOL XTE::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += IsLoranBlinkOK;
   sentence += IsLoranCCycleLockOK;
   sentence += CrossTrackErrorMagnitude;
   sentence += DirectionToSteer;
   sentence += CrossTrackUnits;

   sentence.Finish();

   return( TRUE );
}

const XTE& XTE::operator = ( const XTE& source )
{
   ASSERT_VALID( this );

   IsLoranBlinkOK           = source.IsLoranBlinkOK;
   IsLoranCCycleLockOK      = source.IsLoranCCycleLockOK;
   CrossTrackErrorMagnitude = source.CrossTrackErrorMagnitude;
   DirectionToSteer         = source.DirectionToSteer;
   CrossTrackUnits          = source.CrossTrackUnits;

   return( *this );
}
