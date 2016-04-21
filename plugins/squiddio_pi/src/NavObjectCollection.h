/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose   Squiddio plugin
 * This work is based on the NavObjectCollection.h, file which is:
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
 ***************************************************************************
 */

#ifndef __NAVOBJECTCOLLECTION_H__
#define __NAVOBJECTCOLLECTION_H__

#include "pugixml.hpp"
#include "Poi.h"

//class Track;

//      Bitfield definition controlling the GPX nodes output for point objects
#define         OUT_TYPE        1 << 1          //  Output point type
#define         OUT_TIME        1 << 2          //  Output time as ISO string
#define         OUT_NAME        1 << 3          //  Output point name if defined
#define         OUT_NAME_FORCE  1 << 4          //  Output point name, even if empty
#define         OUT_DESC        1 << 5          //  Output description if defined
#define         OUT_DESC_FORCE  1 << 6          //  Output description, even if empty
#define         OUT_SYM_FORCE   1 << 7          //  Output symbol name, using "empty" if undefined
#define         OUT_GUID        1 << 8          //  Output GUID if defined
#define         OUT_VIZ         1 << 9          //  Output point viz, if non-zero(true)
#define         OUT_VIZ_NAME    1 << 10         //  Output point name viz, if non-zero(true)
#define         OUT_SHARED      1 << 11         //  Output point shared state, if non-zero(true)
#define         OUT_AUTO_NAME   1 << 12         //  Output point auto_name state, if non-zero(true)
#define         OUT_HYPERLINKS  1 << 13         //  Output point Hyperlinks, if present
#define         OUT_ACTION_ADD  1 << 14         //  opencpn:action node support
#define         OUT_ACTION_DEL  1 << 15
#define         OUT_ACTION_UPD  1 << 16

#define  OPT_TRACKPT    OUT_TIME
#define  OPT_WPT        (OUT_TYPE) +\
                        (OUT_TIME) +\
                        (OUT_NAME) +\
                        (OUT_DESC) +\
                        (OUT_SYM_FORCE) +\
                        (OUT_GUID) +\
                        (OUT_VIZ) +\
                        (OUT_VIZ_NAME) +\
                        (OUT_SHARED) +\
                        (OUT_AUTO_NAME) +\
                        (OUT_HYPERLINKS)
#define OPT_ROUTEPT     OPT_WPT                        

//      Bitfield definitions controlling the GPX nodes output for Route.Track objects
#define         RT_OUT_ACTION_ADD         1 << 1          //  opencpn:action node support
#define         RT_OUT_ACTION_DEL         1 << 2
#define         RT_OUT_ACTION_UPD         1 << 3


class NavObjectCollection1 : public pugi::xml_document
{
public:
    NavObjectCollection1();
    ~NavObjectCollection1();

    bool CreateNavObjGPXPoints(void);
    bool AddGPXPointsList( PoiList *pRoutePoints );
    bool AddGPXWaypoint(Poi *pWP );
    bool CreateAllGPXObjects();
    bool LoadAllGPXObjects(void);
    int LoadAllGPXObjectsAsLayer(int layer_id, bool b_layerviz);
    bool SaveFile( const wxString filename );

    void SetRootGPXNode(void);
    Poi *WaypointExists( const wxString& name, double lat, double lon);
    
    pugi::xml_node      m_gpx_root;
};

#endif
