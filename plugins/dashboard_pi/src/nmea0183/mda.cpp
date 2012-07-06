/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register, Jean-Eudes Onfray
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
#include <iostream>
#include "nmea0183.h"

//IMPLEMENT_DYNAMIC( MDA, RESPONSE )

MDA::MDA()
{
   Mnemonic = _T("MDA");
   Empty();
}

MDA::~MDA()
{
   Mnemonic.Empty();
   Empty();
}

void MDA::Empty( void )
{
//   ASSERT_VALID( this );

   BarometricPressInch      = 0.0;
   BarometricPressBar       = 0.0;
   AirTemp                  = 0.0;
   WindAngleTrue   = 0.0;
   WindAngleMagnetic   = 0.0;
   WindSpeedKnots   = 0.0;
   WindSpeedms   = 0.0;
}

bool MDA::Parse( const SENTENCE& sentence )
{
//   ASSERT_VALID( this );
    /*Syntax MDA NMEA 0183 standard Meteorological Composite.
    $WIMDA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,
    <12>,<13>,<14>,<15>,<16>,<17>,<18>,<19>,<20>*hh
    <CR><LF>
    Fields
    <1> Barometric pressure, inches of mercury, to the nearest 0.01 inch
    <2> I = inches of mercury
    <3> Barometric pressure, bars, to the nearest .001 bar
    <4> B = bars
    <5> Air temperature, degrees C, to the nearest 0.1 degree C
    <6> C = degrees C
    <7> Water temperature, degrees C (this field left blank by WeatherStation)
    <8> C = degrees C (this field left blank by WeatherStation)
    <9> Relative humidity, percent, to the nearest 0.1 percent (this field left
    blank by WeatherStation)
    <10> Absolute humidity, percent (this field left blank by WeatherStation)
    <11> Dew point, degrees C, to the nearest 0.1 degree C (this field left blank
    by WeatherStation)
    <12> C = degrees C
    <13> Wind direction, degrees True, to the nearest 0.1 degree
    <14> T = true
    <15> Wind direction, degrees Magnetic, to the nearest 0.1 degree
    <16> M = magnetic
    <17> Wind speed, knots, to the nearest 0.1 knot
    <18> N = knots
    <19> Wind speed, meters per second, to the nearest 0.1 m/s
    <20> M = meters per second
    $WIMDA,29.8932,I,1.0123,B,27.9,C,,,,,,,203.0,T,203.0,M,0.9,N,0.5,M*20*/
   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 9 ) == TRUE )
   {
      SetErrorMessage( _T("Invalid Checksum") );
      return( FALSE );
   } 

   BarometricPressInch      = sentence.Double( 1 );
   BarometricPressBar       = sentence.Double( 3 );
   AirTemp                  = sentence.Double( 5 );
   WindAngleTrue            = sentence.Double( 13 );
   WindAngleMagnetic        = sentence.Double( 15 );
   WindSpeedKnots           = sentence.Double( 17 );
   WindSpeedms              = sentence.Double( 19 );
   return( TRUE );
}

bool MDA::Write( SENTENCE& sentence )
{
//   ASSERT_VALID( this );

   /*
   ** Let the parent do its thing
   */
   
   RESPONSE::Write( sentence );

   sentence += BarometricPressInch;
   sentence += _T("I");
   sentence += BarometricPressBar;
   sentence += _T("B");
   sentence += AirTemp;
   sentence += _T("C");
   sentence += WindAngleTrue;
   sentence += _T("T");
   sentence += WindAngleMagnetic;
   sentence += _T("M");
   sentence += WindSpeedKnots;
   sentence += _T("N");
   sentence += WindSpeedms;
   sentence += _T("M");

   sentence.Finish();

   return( TRUE );
}

const MDA& MDA::operator = ( const MDA& source )
{
//   ASSERT_VALID( this );
 
   BarometricPressInch= source.BarometricPressInch;
   BarometricPressBar = source.BarometricPressBar;
   AirTemp            = source.AirTemp;
   WindAngleTrue      = source.WindAngleTrue;
   WindAngleMagnetic  = source.WindAngleMagnetic;
   WindSpeedKnots     = source.WindSpeedKnots;
   WindSpeedms        = source.WindSpeedms;

   return( *this );
}
