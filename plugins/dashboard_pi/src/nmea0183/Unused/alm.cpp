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

IMPLEMENT_DYNAMIC( ALM, RESPONSE )

ALM::ALM()
{
   Mnemonic = "ALM";
   Empty();
}

ALM::~ALM()
{
   Mnemonic.Empty();
   Empty();
}

void ALM::Empty( void )
{
   ASSERT_VALID( this );

   NumberOfMessages         = 0;
   MessageNumber            = 0;
   PRNNumber                = 0;
   WeekNumber               = 0;
   SVHealth                 = 0;
   Eccentricity             = 0;
   AlmanacReferenceTime     = 0;
   InclinationAngle         = 0;
   RateOfRightAscension     = 0;
   RootOfSemiMajorAxis      = 0;
   ArgumentOfPerigee        = 0;
   LongitudeOfAscensionNode = 0;
   MeanAnomaly              = 0;
   F0ClockParameter         = 0;
   F1ClockParameter         = 0;
}

BOOL ALM::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   wxString field_data;

   /*
   ** ALM - GPS Almanac Data
   **
   **        1   2   3  4   5  6    7  8    9    10     11     12     13     14  15   16
   **        |   |   |  |   |  |    |  |    |    |      |      |      |      |   |    |
   ** $--ALM,x.x,x.x,xx,x.x,hh,hhhh,hh,hhhh,hhhh,hhhhhh,hhhhhh,hhhhhh,hhhhhh,hhh,hhh,*hh<CR><LF>
   **
   **  1) Total number of messages
   **  2) Message Number
   **  3) Satellite PRN number (01 to 32)
   **  4) GPS Week Number
   **  5) SV health, bits 17-24 of each almanac page
   **  6) Eccentricity
   **  7) Almanac Reference Time
   **  8) Inclination Angle
   **  9) Rate of Right Ascension
   ** 10) Root of semi-major axis
   ** 11) Argument of perigee
   ** 12) Longitude of ascension node
   ** 13) Mean anomaly
   ** 14) F0 Clock Parameter
   ** 15) F1 Clock Parameter
   ** 16) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 16 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   NumberOfMessages         = (WORD) sentence.Integer( 1 );
   MessageNumber            = (WORD) sentence.Integer( 2 );
   PRNNumber                = (WORD) sentence.Integer( 3 );
   WeekNumber               = (WORD) sentence.Integer( 4 );
   SVHealth                 = (WORD) ::HexValue( sentence.Field( 5 ) );
   Eccentricity             = (WORD) ::HexValue( sentence.Field( 6 ) );
   AlmanacReferenceTime     = (WORD) ::HexValue( sentence.Field( 7 ) );
   InclinationAngle         = (WORD) ::HexValue( sentence.Field( 8 ) );
   RateOfRightAscension     = (WORD) ::HexValue( sentence.Field( 9 ) );
   RootOfSemiMajorAxis      = ::HexValue( sentence.Field( 10 ) );
   ArgumentOfPerigee        = ::HexValue( sentence.Field( 11 ) );
   LongitudeOfAscensionNode = ::HexValue( sentence.Field( 12 ) );
   MeanAnomaly              = ::HexValue( sentence.Field( 13 ) );
   F0ClockParameter         = (WORD) ::HexValue( sentence.Field( 14 ) );
   F1ClockParameter         = (WORD) ::HexValue( sentence.Field( 15 ) );

   return( TRUE );
}

BOOL ALM::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += NumberOfMessages;
   sentence += MessageNumber;
   sentence += PRNNumber;
   sentence += WeekNumber;
   sentence += Hex( SVHealth );
   sentence += Hex( Eccentricity );
   sentence += Hex( AlmanacReferenceTime );
   sentence += Hex( InclinationAngle );
   sentence += Hex( RateOfRightAscension );
   sentence += Hex( RootOfSemiMajorAxis );
   sentence += Hex( ArgumentOfPerigee );
   sentence += Hex( LongitudeOfAscensionNode );
   sentence += Hex( MeanAnomaly );
   sentence += Hex( F0ClockParameter );
   sentence += Hex( F1ClockParameter );

   sentence.Finish();

   return( TRUE );
}

const ALM& ALM::operator = ( const ALM& source )
{
   ASSERT_VALID( this );

   NumberOfMessages         = source.NumberOfMessages;
   MessageNumber            = source.MessageNumber;
   PRNNumber                = source.PRNNumber;
   WeekNumber               = source.WeekNumber;
   SVHealth                 = source.SVHealth;
   Eccentricity             = source.Eccentricity;
   AlmanacReferenceTime     = source.AlmanacReferenceTime;
   InclinationAngle         = source.InclinationAngle;
   RateOfRightAscension     = source.RateOfRightAscension;
   RootOfSemiMajorAxis      = source.RootOfSemiMajorAxis;
   ArgumentOfPerigee        = source.ArgumentOfPerigee;
   LongitudeOfAscensionNode = source.LongitudeOfAscensionNode;
   MeanAnomaly              = source.MeanAnomaly;
   F0ClockParameter         = source.F0ClockParameter;
   F1ClockParameter         = source.F1ClockParameter;

   return( *this );
}
