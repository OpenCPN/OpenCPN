/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
#include "Route.h"
#include "RoutePoint.h"
#include "Track.h"

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
#define         OUT_EXTENSION   1 << 17
#define         OUT_ARRIVAL_RADIUS 1 << 18
#define         OUT_WAYPOINT_RANGE_RINGS 1 << 19

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
                        (OUT_HYPERLINKS) +\
                        (OUT_ARRIVAL_RADIUS) +\
                        (OUT_WAYPOINT_RANGE_RINGS)
#define OPT_ROUTEPT     OPT_WPT                        

//      Bitfield definitions controlling the GPX nodes output for Route.Track objects
#define         RT_OUT_ACTION_ADD         1 << 1          //  opencpn:action node support
#define         RT_OUT_ACTION_DEL         1 << 2
#define         RT_OUT_ACTION_UPD         1 << 3
#define         RT_OUT_NO_RTPTS           1 << 4


class NavObjectCollection1 : public pugi::xml_document
{
public:
    NavObjectCollection1();
    ~NavObjectCollection1();
    
    bool CreateNavObjGPXPoints(void);
    bool CreateNavObjGPXRoutes(void);
    bool CreateNavObjGPXTracks(void);
 
    void AddGPXRoutesList( RouteList *pRoutes );
    void AddGPXTracksList( TrackList *pTracks );
    bool AddGPXPointsList( RoutePointList *pRoutePoints );
    bool AddGPXRoute(Route *pRoute);
    bool AddGPXTrack(Track *pTrk);
    bool AddGPXWaypoint(RoutePoint *pWP );
    
    bool CreateAllGPXObjects();
    bool LoadAllGPXObjects( bool b_full_viz = false);
    int LoadAllGPXObjectsAsLayer(int layer_id, bool b_layerviz);
    
    bool SaveFile( const wxString filename );

    void SetRootGPXNode(void);
    bool IsOpenCPN();
    
    pugi::xml_node      m_gpx_root;
};


class NavObjectChanges : public NavObjectCollection1
{
public:
    NavObjectChanges();
    NavObjectChanges( wxString file_name );
    ~NavObjectChanges();
    
    void AddRoute( Route *pr, const char *action );           // support "changes" file set
    void AddTrack( Track *pr, const char *action );
    void AddWP( RoutePoint *pr, const char *action );
    void AddTrackPoint( TrackPoint *pWP, const char *action, const wxString& parent_GUID );
    
    bool ApplyChanges(void);
    
    wxString    m_filename;
    FILE *      m_changes_file;
    
};


#endif
