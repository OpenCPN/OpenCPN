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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
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


RMC::RMC()
{
    Mnemonic = _T("RMC");
   Empty();
}

RMC::~RMC()
{
   Mnemonic.Empty();
   Empty();
}

void RMC::Empty( void )
{

   UTCTime.Empty();
   IsDataValid                = Unknown0183;
   SpeedOverGroundKnots       = 0.0;
   Position.Empty();
   TrackMadeGoodDegreesTrue   = 0.0;
   Date.Empty();
   MagneticVariation          = 0.0;
   MagneticVariationDirection = EW_Unknown;
}

bool RMC::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** RMC - Recommended Minimum Navigation Information
   **
   **  Version 2.0 Format
   **                                                            12
   **        1         2 3       4 5        6 7   8   9    10  11|
   **        |         | |       | |        | |   |   |    |   | |
   ** $--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a*hh<CR><LF>
   **
   ** Field Number:
   **  1) UTC Time
   **  2) Status, V = Navigation receiver warning
   **  3) Latitude
   **  4) N or S
   **  5) Longitude
   **  6) E or W
   **  7) Speed over ground, knots
   **  8) Track made good, degrees true
   **  9) Date, ddmmyy
   ** 10) Magnetic Variation, degrees
   ** 11) E or W

   ** Version 2.0
   ** 12) Checksum

   ** Version 2.3
   ** 12) The value can be A=autonomous, D=differential, E=Estimated, N=not valid, S=Simulator, optional, may be NULL
   ** 13) Checksum
   */

   /*
   ** First we check the checksum...
   */

   int nFields = sentence.GetNumberOfDataFields( );

   NMEA0183_BOOLEAN check = sentence.IsChecksumBad( nFields + 1 );

   if ( check == NTrue )
   {
   /*
   ** This may be an NMEA Version 3+ sentence, with added fields
   */
        wxString checksum_in_sentence = sentence.Field( nFields + 1 );
       if(checksum_in_sentence.StartsWith(_T("*")))       // Field is a valid erroneous checksum
       {
         SetErrorMessage( _T("Invalid Checksum") );
         return( FALSE );
       }
   }

   // If sentence is at least Version 2.3, check the extra mode indicator field
   bool mode_valid = true;
   if(nFields >= 12){
       wxString mode_string = sentence.Field( 12 );
       if(!mode_string.StartsWith(_T("*"))) {
           if((mode_string == _T("N")) || (mode_string == _T("S")))     // Not valid, or simulator mode
               mode_valid = false;
       }
   }


   UTCTime                    = sentence.Field( 1 );

   IsDataValid                = sentence.Boolean( 2 );
   if( !mode_valid )
       IsDataValid = NFalse;

   Position.Parse( 3, 4, 5, 6, sentence );
   SpeedOverGroundKnots       = sentence.Double( 7 );
   TrackMadeGoodDegreesTrue   = sentence.Double( 8 );
   Date                       = sentence.Field( 9 );
   MagneticVariation          = sentence.Double( 10 );
   MagneticVariationDirection = sentence.EastOrWest( 11 );

   return( TRUE );
}

bool RMC::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += UTCTime;
   sentence += IsDataValid;
   sentence += Position;
   sentence += SpeedOverGroundKnots;
   sentence += TrackMadeGoodDegreesTrue;
   sentence += Date;

   if(MagneticVariation > 360.)
         sentence += _T(",,");
   else
   {
         sentence += MagneticVariation;
         sentence += MagneticVariationDirection;
   }
   sentence += FAAModeIndicator;
   sentence.Finish();

   return( TRUE );
}

const RMC& RMC::operator = ( const RMC& source )
{
//   ASSERT_VALID( this );

   UTCTime                    = source.UTCTime;
   IsDataValid                = source.IsDataValid;
   Position                   = source.Position;
   SpeedOverGroundKnots       = source.SpeedOverGroundKnots;
   TrackMadeGoodDegreesTrue   = source.TrackMadeGoodDegreesTrue;
   Date                       = source.Date;
   MagneticVariation          = source.MagneticVariation;
   MagneticVariationDirection = source.MagneticVariationDirection;
   FAAModeIndicator           = source.FAAModeIndicator;

  return( *this );
}
