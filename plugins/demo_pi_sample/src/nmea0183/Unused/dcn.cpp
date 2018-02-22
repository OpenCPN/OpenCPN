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

IMPLEMENT_DYNAMIC( DCN, RESPONSE )

DCN::DCN()
{
   Mnemonic = "DCN";
   Empty();
}

DCN::~DCN()
{
   Mnemonic.Empty();
   Empty();
}

void DCN::Empty( void )
{
   ASSERT_VALID( this );

   DeccaChainID                     = 0;
   Red.Empty();
   Green.Empty();
   Purple.Empty();
   RedLineNavigationUse             = Unknown;
   GreenLineNavigationUse           = Unknown;
   PurpleLineNavigationUse          = Unknown;
   PositionUncertaintyNauticalMiles = 0.0;
   Basis                            = BasisUnknown;
}

BOOL DCN::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** DCN - Decca Position
   **                                      11  13      16
   **        1  2  3   4 5  6   7 8  9   10| 12| 14  15| 17
   **        |  |  |   | |  |   | |  |   | | | | |   | | |
   ** $--DCN,xx,cc,x.x,A,cc,x.x,A,cc,x.x,A,A,A,A,x.x,N,x*hh<CR><LF>
   **
   **  1) Decca chain identifier
   **  2) Red Zone Identifier
   **  3) Red Line Of Position
   **  4) Red Master Line Status
   **  5) Green Zone Identifier
   **  6) Green Line Of Position
   **  7) Green Master Line Status
   **  8) Purple Zone Identifier
   **  9) Purple Line Of Position
   ** 10) Purple Master Line Status
   ** 11) Red Line Navigation Use
   ** 12) Green Line Navigation Use
   ** 13) Purple Line Navigation Use
   ** 14) Position Uncertainity
   ** 15) N = Nautical Miles
   ** 16) Fix Data Basis
   **     1 = Normal Pattern
   **     2 = Lane Identification Pattern
   **     3 = Lane Identification Transmissions
   ** 17) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 17 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   DeccaChainID                     = sentence.Integer( 1 );
   Red.Parse( 2, sentence );
   Green.Parse( 5, sentence );
   Purple.Parse( 8, sentence );
   RedLineNavigationUse             = sentence.Boolean( 11 );
   GreenLineNavigationUse           = sentence.Boolean( 12 );
   PurpleLineNavigationUse          = sentence.Boolean( 13 );
   PositionUncertaintyNauticalMiles = sentence.Double( 14 );

   int temp_integer = sentence.Integer( 16 );

   switch( temp_integer )
   {
      case 1:

         Basis = NormalPatternBasis;
         break;

      case 2:

         Basis = LaneIdentificationPatternBasis;
         break;

      case 3:

         Basis = LaneIdentificationTransmissionsBasis;
         break;

      default:

         Basis = BasisUnknown;
         break;
   }

   return( TRUE );
}

BOOL DCN::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += DeccaChainID;
   Red.Write( sentence );
   Green.Write( sentence );
   Purple.Write( sentence );
   sentence += RedLineNavigationUse;
   sentence += GreenLineNavigationUse;
   sentence += PurpleLineNavigationUse;
   sentence += PositionUncertaintyNauticalMiles;
   sentence += "N";

   switch( Basis )
   {
      case NormalPatternBasis:

         sentence += 1;
         break;

      case LaneIdentificationPatternBasis:

         sentence += 2;
         break;

      case LaneIdentificationTransmissionsBasis:

         sentence += 3;
         break;

      default:

         sentence += "";
         break;
   }

   sentence.Finish();

   return( TRUE );
}

const DCN& DCN::operator = ( const DCN& source )
{
   ASSERT_VALID( this );

   DeccaChainID                     = source.DeccaChainID;
   Red                              = source.Red;
   Green                            = source.Green;
   Purple                           = source.Purple;
   RedLineNavigationUse             = source.RedLineNavigationUse;
   GreenLineNavigationUse           = source.GreenLineNavigationUse;
   PurpleLineNavigationUse          = source.PurpleLineNavigationUse;
   PositionUncertaintyNauticalMiles = source.PositionUncertaintyNauticalMiles;
   Basis                            = source.Basis;
   
   return( *this );
}
