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


#if ! defined( LATLONG_CLASS_HEADER )
#define LATLONG_CLASS_HEADER

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

class SENTENCE;

class LATITUDE
{

   public:

      LATITUDE();
      virtual ~LATITUDE();

      /*
      ** Data
      */

      double Latitude;

      NORTHSOUTH Northing;

      /*
      ** Methods
      */

      virtual void Empty( void );
      virtual bool IsDataValid( void );
      virtual void Parse( int PositionFieldNumber, int NorthingFieldNumber, const SENTENCE& LineToParse );
      virtual void Set( double Position, const wxString& Northing );
      virtual void Write( SENTENCE& sentence );

      /*
      ** Operators
      */

      virtual const LATITUDE& operator = ( const LATITUDE& source );
};

class LONGITUDE
{

   public:

      LONGITUDE();
      virtual ~LONGITUDE();

      /*
      ** Data
      */

      double Longitude;

      EASTWEST Easting;

      /*
      ** Methods
      */

      virtual void Empty( void );
      virtual bool IsDataValid( void );
      virtual void Parse( int PositionFieldNumber, int EastingFieldNumber, const SENTENCE& LineToParse );
      virtual void Set( double Position, const wxString& Easting );
      virtual void Write( SENTENCE& sentence );

      /*
      ** Operators
      */

      virtual const LONGITUDE& operator = ( const LONGITUDE& source );
};

class LATLONG
{

   public:

      LATLONG();
      virtual ~LATLONG();

      /*
      ** Data
      */

      LATITUDE  Latitude;
      LONGITUDE Longitude;

      /*
      ** Methods
      */

      virtual void Empty( void );
      virtual bool Parse( int LatitudePostionFieldNumber, int NorthingFieldNumber, int LongitudePositionFieldNumber, int EastingFieldNumber, const SENTENCE& LineToParse );
      virtual void Write( SENTENCE& sentence );

      /*
      ** Operators
      */

      virtual const LATLONG& operator = ( const LATLONG& source );
};

#endif // LATLONG_CLASS_HEADER
