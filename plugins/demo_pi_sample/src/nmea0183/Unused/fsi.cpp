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

IMPLEMENT_DYNAMIC( FSI, RESPONSE )

FSI::FSI()
{
   Mnemonic = "FSI";
   Empty();
}

FSI::~FSI()
{
   Mnemonic.Empty();
   Empty();
}

void FSI::Empty( void )
{
   ASSERT_VALID( this );

   TransmittingFrequency = 0.0;
   ReceivingFrequency    = 0.0;
   Mode                  = CommunicationsModeUnknown;
   PowerLevel            = 0;
}

BOOL FSI::Parse( const SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** FSI - Frequency Set Information
   **
   **        1      2      3 4 5
   **        |      |      | | |
   ** $--FSI,xxxxxx,xxxxxx,c,x*hh<CR><LF>
   **
   ** Field Number: 
   **  1) Transmitting Frequency
   **  2) Receiving Frequency
   **  3) Communications Mode
   **  4) Power Level
   **  5) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 5 ) == True )
   {
      SetErrorMessage( "Invalid Checksum" );
      return( FALSE );
   } 

   TransmittingFrequency = sentence.Double( 1 );
   ReceivingFrequency    = sentence.Double( 2 );
   Mode                  = sentence.CommunicationsMode( 3 );
   PowerLevel            = (short) sentence.Integer( 4 );

   return( TRUE );
}

BOOL FSI::Write( SENTENCE& sentence )
{
   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += TransmittingFrequency;
   sentence += ReceivingFrequency;
   sentence += Mode;
   sentence += PowerLevel;

   sentence.Finish();

   return( TRUE );
}

const FSI& FSI::operator = ( const FSI& source )
{
   ASSERT_VALID( this );

   TransmittingFrequency = source.TransmittingFrequency;
   ReceivingFrequency    = source.ReceivingFrequency;
   Mode                  = source.Mode;
   PowerLevel            = source.PowerLevel;

   return( *this );
}
