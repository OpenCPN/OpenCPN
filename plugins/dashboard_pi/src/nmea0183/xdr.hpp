/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register, Thomas Rauch
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

#if ! defined( XDR_CLASS_HEADER )
#define XDR_CLASS_HEADER

/*
** Author: Thomas Rauch
**
**
**
** You can use it any way you like.
*/

#if ! defined( TRANSDUCER_INFO_HEADER )
#define TRANSDUCER_INFO_HEADER

typedef struct
{
      wxString  TransducerType;
      double    MeasurementData;
      wxString  UnitOfMeasurement;
      wxString TransducerName;
 } TRANSDUCER_INFO;
#endif

#define MaxTransducerCnt 10 //max number of transducer quadrupels per NMEA line

class XDR : public RESPONSE
{
//   DECLARE_DYNAMIC( XDR )

   public:

      XDR();
     ~XDR();

      /*
      ** Data
      */

      int TransducerCnt;
      TRANSDUCER_INFO TransducerInfo[MaxTransducerCnt];


      /*
      ** Methods
      */

      virtual void Empty( void );
      virtual bool Parse( const SENTENCE& sentence );
      virtual bool Write( SENTENCE& sentence );

      /*
      ** Operators
      */

      virtual const XDR& operator = ( const XDR& source );
};

#endif // XDR_CLASS_HEADER
