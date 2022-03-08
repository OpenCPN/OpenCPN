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


GSV::GSV()
{
   Mnemonic = _T("GSV");
   Empty();
}

GSV::~GSV()
{
   Mnemonic.Empty();
   Empty();
}

void GSV::Empty( void )
{
   NumberOfMessages = 0;
   MessageNumber = 0;
   SatsInView = 0;
   for (int idx = 0; idx < 4; idx++)
   {
         SatInfo[idx].SatNumber = 0;
         SatInfo[idx].ElevationDegrees = 0;
         SatInfo[idx].AzimuthDegreesTrue = 0;
         SatInfo[idx].SignalToNoiseRatio = 0;
   }
}

bool GSV::Parse( const SENTENCE& sentence )
{
/*
  $GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75

Where:
      GSV          Satellites in view
      2            Number of sentences for full data
      1            sentence 1 of 2
      08           Number of satellites in view

      01           Satellite PRN number
      40           Elevation, degrees
      083          Azimuth, degrees
      46           SNR - higher is better
           for up to 4 satellites per sentence
      *75          the checksum data, always begins with *
*/


   /*
   ** GSV - GPS satellite Status
   **
   **        1 2 3 4 5 6 7         n
   **        | | | | | | |         |
   ** $--GSV,x,x,x,x,x,x,x.........*hh<CR><LF>
   **
   ** Field Number:
   **  1) Number of sentences for full data
   **  2) sentence number
   **  3) Number of satellites in view
   **  4) Satellite PRN number
   **  5) Elevation, degrees
   **  6) Azimuth, degrees
   **  7) SNR - higher is better
   **  Fields 4-7 may repeat up to 4 times per sentence
   **  n) Checksum
   */

   /*
   ** First we check the checksum...
   */
    int nNumberOfDataFields = sentence.GetNumberOfDataFields();
    int satInfoCnt = (nNumberOfDataFields-3) / 4;
    if (satInfoCnt < 1)
    {
        SetErrorMessage(_T("Invalid Field count"));
        return(FALSE);
    }

    if (sentence.IsChecksumBad(nNumberOfDataFields + 1) == NTrue)
    {
        SetErrorMessage( _T("Invalid Checksum" ));
        return( FALSE );
    }

    NumberOfMessages = sentence.Integer( 1 );
    MessageNumber = sentence.Integer( 2 );
    SatsInView = sentence.Integer( 3 );

    for (int idx = 0; idx < satInfoCnt; idx++) {
      SatInfo[idx].SatNumber = sentence.Integer(idx * 4 + 4);
      SatInfo[idx].ElevationDegrees = sentence.Integer(idx * 4 + 5);
      SatInfo[idx].AzimuthDegreesTrue = sentence.Integer(idx * 4 + 6);
      SatInfo[idx].SignalToNoiseRatio = sentence.Integer(idx * 4 + 7);
    }
    return( TRUE );
}

bool GSV::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += NumberOfMessages;
   sentence += MessageNumber;
   sentence += SatsInView;

   for (int idx = 0; idx < 4; idx++)
   {
         sentence += SatInfo[idx].SatNumber;
         sentence += SatInfo[idx].ElevationDegrees;
         sentence += SatInfo[idx].AzimuthDegreesTrue;
         sentence += SatInfo[idx].SignalToNoiseRatio;
   }

   sentence.Finish();

   return( TRUE );
}

const GSV& GSV::operator = ( const GSV& source )
{
   NumberOfMessages = source.NumberOfMessages;
   MessageNumber = source.MessageNumber;
   SatsInView = source.SatsInView;

   for (int idx = 0; idx < 4; idx++)
   {
         SatInfo[idx].SatNumber = source.SatInfo[idx].SatNumber;
         SatInfo[idx].ElevationDegrees = source.SatInfo[idx].ElevationDegrees;
         SatInfo[idx].AzimuthDegreesTrue = source.SatInfo[idx].AzimuthDegreesTrue;
         SatInfo[idx].SignalToNoiseRatio = source.SatInfo[idx].SignalToNoiseRatio;
   }

   return( *this );
}
