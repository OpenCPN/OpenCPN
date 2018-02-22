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


#if ! defined( TTM_CLASS_HEADER )
#define TTM_CLASS_HEADER

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

typedef enum
{
   TargetUnknown = 0,
   TargetLost,
   TargetQuery,
   TargetTracking
}
TARGET_STATUS;

class TTM : public RESPONSE
{

   public:

      TTM();
     ~TTM();

      /*
      ** Data
      */

      int           TargetNumber;
      double        TargetDistance;
      double        BearingFromOwnShip;
      wxString       BearingUnits;
      double        TargetSpeed;
      double        TargetCourse;
      wxString       TargetCourseUnits;
      double        DistanceOfClosestPointOfApproach;
      double        NumberOfMinutesToClosestPointOfApproach;
      wxString       Increasing;
      wxString       TargetName;
      TARGET_STATUS TargetStatus;
      wxString       ReferenceTarget;

      /*
      ** Methods
      */

      virtual void Empty( void );
      virtual BOOL Parse( const SENTENCE& sentence );
      virtual BOOL Write( SENTENCE& sentence );

      /*
      ** Operators
      */

      virtual const TTM& operator = ( const TTM& source );
};

#endif // TTM_CLASS_HEADER
