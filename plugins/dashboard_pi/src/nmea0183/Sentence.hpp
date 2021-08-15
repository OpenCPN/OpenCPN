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


#if ! defined( SENTENCE_CLASS_HEADER )
#define SENTENCE_CLASS_HEADER

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

class LATLONG;

class SENTENCE
{
//   DECLARE_DYNAMIC( SENTENCE )

   public:

      SENTENCE();
      virtual ~SENTENCE();

      /*
      ** Data
      */

      wxString Sentence;

      /*
      ** Methods
      */

      virtual NMEA0183_BOOLEAN Boolean( int field_number ) const;
      virtual unsigned char ComputeChecksum( void ) const;
      virtual COMMUNICATIONS_MODE CommunicationsMode( int field_number ) const;
      virtual double Double( int field_number ) const;
      virtual EASTWEST EastOrWest( int field_number ) const;
      virtual const wxString& Field( int field_number ) const;
      virtual void Finish( void );
      virtual int GetNumberOfDataFields( void ) const;
      virtual int Integer( int field_number ) const;
      virtual NMEA0183_BOOLEAN IsChecksumBad( int checksum_field_number ) const;
      virtual LEFTRIGHT LeftOrRight( int field_number ) const;
      virtual NORTHSOUTH NorthOrSouth( int field_number ) const;
      virtual REFERENCE Reference( int field_number ) const;
      virtual TRANSDUCER_TYPE TransducerType( int field_number ) const;

      /*
      ** Operators
      */

      operator wxString() const;
      virtual const SENTENCE& operator  = ( const SENTENCE& source );
      virtual const SENTENCE& operator  = ( const wxString& source );
      virtual const SENTENCE& operator += ( const wxString& source );
      virtual const SENTENCE& operator += ( double value );
      virtual const SENTENCE& operator += ( NORTHSOUTH northing );
      virtual const SENTENCE& operator += ( COMMUNICATIONS_MODE mode );
      virtual const SENTENCE& operator += ( int value );
      virtual const SENTENCE& operator += ( EASTWEST easting );
      virtual const SENTENCE& operator += ( TRANSDUCER_TYPE transducer );
      virtual const SENTENCE& operator += ( NMEA0183_BOOLEAN boolean );
      virtual const SENTENCE& operator += ( LATLONG& source );
};

#endif // SENTENCE_CLASS_HEADER
