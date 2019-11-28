/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Samuel R. Blackburn, David S Register, Dirk Smits*
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

//IMPLEMENT_DYNAMIC( MOB, RESPONSE )

MOB::MOB()
{
   Mnemonic = _T("MOB");
   Empty();
}

MOB::~MOB()
{
   Mnemonic.Empty();
   Empty();
}

void MOB::Empty( void )
{
//   ASSERT_VALID( this );

   BatteryStatus.Empty();;
   MMSIofVessel.Empty();;
   SpeedOverGroundKnots       = -1.0;
   CourseMadeGoodDegreesTrue  = -1.0;
   Position.Empty();
   UTCTimeOfPosition.Empty();
   DateOfPosition.Empty();
   MobPositionSource.Empty();
   UTCTimeOfActivation.Empty();
   MobStatus;
   MobEmitterID.Empty();
}

bool MOB::Parse( const SENTENCE& sentence )
{

   /*
   $--MOB,hhhhh,a,hhmmss.ss,x,xxxxxx,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxxxxxxx,x*hh<CR><LF>
            |   |    |      |   |        |        |    |    |     |  |   |      |     *----Battery status 14
            |   |    |      |   |        |        |    |    |     |  |   |      *----------MMSI of vessel 13
            |   |    |      |   |        |        |    |    |     |  |   *---------------- Speed over ground 12
            |   |    |      |   |        |        |    |    |     |  *-------------------- Course over ground, degrees true 11
            |   |    |      |   |        |        |    |    *-----*----------------------- Longitude, E/W 9, 10
            |   |    |      |   |        |        *----*---------------------------------- Latitude, N/S 7, 8
            |   |    |      |   |        *------------------------------------------------ UTC of position 6
            |   |    |      |   *--------------------------------------------------------- Date of position (ddmmyy)5
            |   |    |      *------------------------------------------------------------- MOB position source 4
            |   |    *-------------------------------------------------------------------- UTC of MOB activation 3
            |   *------------------------------------------------------------------------- MOB status 2
            *----------------------------------------------------------------------------- MOB emitter ID 1

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
   MobEmitterID = sentence.Field( 1 );
   MobStatus = sentence.Field( 2 );
   UTCTimeOfActivation = sentence.Field( 3 );
   MobPositionSource = sentence.Field( 4 );
   DateOfPosition = sentence.Field( 5 );
   UTCTimeOfPosition = sentence.Field( 6 );
   Position.Parse( 7, 8, 9, 10, sentence );
   CourseMadeGoodDegreesTrue = sentence.Double( 11 );
   SpeedOverGroundKnots = sentence.Double( 12 );
   MMSIofVessel = sentence.Field( 13 );
   BatteryStatus = sentence.Field( 14 );

   return( TRUE );
}

bool MOB::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += MobEmitterID;
   sentence += MobStatus;
   sentence += UTCTimeOfActivation;
   sentence += MobPositionSource;
   sentence += DateOfPosition;
   sentence += UTCTimeOfPosition;
   sentence += Position;
   sentence += CourseMadeGoodDegreesTrue;
   sentence += SpeedOverGroundKnots;
   sentence += MMSIofVessel;
   sentence += BatteryStatus;

   sentence.Finish();

   return( TRUE );
}

const MOB& MOB::operator = ( const MOB& source )
{

   MobEmitterID = source.MobEmitterID;
   MobStatus = source.MobStatus;
   UTCTimeOfActivation = source.UTCTimeOfActivation;
   MobPositionSource = source.MobPositionSource;
   DateOfPosition = source.DateOfPosition;
   UTCTimeOfPosition = source.UTCTimeOfPosition;
   Position = source.Position;
   CourseMadeGoodDegreesTrue = source.CourseMadeGoodDegreesTrue;
   SpeedOverGroundKnots = source.SpeedOverGroundKnots;
   MMSIofVessel = source.MMSIofVessel;
   BatteryStatus = source.BatteryStatus;

   return( *this );
}
