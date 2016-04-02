/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose   Squiddio plugin
 * Note:     This work is based on the original RoutePointMan.h file which is:
 * Copyright (C) 2010 by David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Mauro Calvi                                     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/


#ifndef __ROUTEMANSQ_H__
#define __ROUTEMANSQ_H__


#include <wx/imaglist.h>
#include "Poi.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

//----------------------------------------------------------------------------
//    forward class declarations
//----------------------------------------------------------------------------
class Poi;
class PoiList;

//----------------------------------------------------------------------------
//   WayPointman
//----------------------------------------------------------------------------

class PoiMan
{
public:
      PoiMan();
      ~PoiMan();
      //wxString CreateGUID(Poi *pRP);
      bool AddRoutePoint(Poi *prp);
      bool RemoveRoutePoint(Poi *prp);
      PoiList *GetWaypointList(void) { return m_pWayPointList; }
private:
      PoiList    		*m_pWayPointList;
      int         		m_nGUID;
};

#endif
