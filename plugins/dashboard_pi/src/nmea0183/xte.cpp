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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *                                                                         *
 *   S Blackburn's original source license:                                *
 *         "You can use it any way you like."                              *
 *   More recent (2010) license statement:                                 *
 *         "It is BSD license, do with it what you will"                   *
 */


#include "nmea0183.h"

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

extern int              g_NMEAXTEPrecision;


XTE::XTE()
{
   Mnemonic = _T("XTE");
   Empty();
}

XTE::~XTE()
{
   Mnemonic.Empty();
   Empty();
}

void XTE::Empty( void )
{
//   ASSERT_VALID( this );

   CrossTrackErrorDistance = 0.0;
   DirectionToSteer         = LR_Unknown;
}

bool XTE::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   wxString field_data;

   /*
   ** XTE - Autopilot Sentence
   **
   **        1 2 3   4 5 6
   **        | | |   | | |
   ** $--XTE,A,A,x.x,a,N*hh<CR><LF>
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

   NMEA0183_BOOLEAN check = sentence.IsChecksumBad( 15 );

   if ( check == NTrue )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   }

   /*
   ** Line has already been checked for checksum validity
   */

   IsLoranBlinkOK                           = sentence.Boolean( 1 );
   IsLoranCCycleLockOK                      = sentence.Boolean( 2 );
   CrossTrackErrorDistance                  = sentence.Double( 3 );
   DirectionToSteer                         = sentence.LeftOrRight( 4 );
   CrossTrackUnits                          = sentence.Field( 5 );

   return( TRUE );
}

bool XTE::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += IsLoranBlinkOK;
   sentence += IsLoranCCycleLockOK;
   sentence += CrossTrackErrorDistance;

   if(DirectionToSteer == Left)
       sentence += _T("L");
   else
       sentence += _T("R");

   sentence += CrossTrackUnits;

   sentence.Finish();

   return( TRUE );
}

const XTE& XTE::operator = ( const XTE& source )
{
//   ASSERT_VALID( this );

   IsLoranBlinkOK                           = source.IsLoranBlinkOK;
   IsLoranCCycleLockOK                      = source.IsLoranCCycleLockOK;
   CrossTrackErrorDistance                  = source.CrossTrackErrorDistance;
   DirectionToSteer                         = source.DirectionToSteer;
   CrossTrackUnits                          = source.CrossTrackUnits;

   return( *this );
}
