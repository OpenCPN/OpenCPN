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

#include "NavObjectCollection.h"
#include "RoutePoint.h"
#include "routeman.h"
#include "navutil.h"
#include "Select.h"

#include "tinyxml.h"

extern WayPointman *pWayPointMan;
extern RouteList *pRouteList;
extern Select *pSelect;

NavObjectCollection::NavObjectCollection()
	: GpxDocument()
{
    m_pXMLrootnode = (GpxRootElement *) RootElement();
}

NavObjectCollection::~NavObjectCollection()
{
//     delete m_pXMLrootnode;            // done in base class
}

bool NavObjectCollection::CreateNavObjGPXPoints( void )
{

    //    Iterate over the Routepoint list, creating Nodes for
    //    Routepoints that are not in any Route
    //    as indicated by m_bIsolatedMark == false

    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

    RoutePoint *pr;

    while( node ) {
        pr = node->GetData();

        if( ( pr->m_bIsolatedMark ) && !( pr->m_bIsInLayer ) && !(pr->m_btemp) )
                {
            m_pXMLrootnode->AddWaypoint( CreateGPXWpt( pr, GPX_WPT_WAYPOINT ) );
        }
        node = node->GetNext();
    }

    return true;
}

bool NavObjectCollection::CreateNavObjGPXRoutes( void )
{
    // Routes
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();

        if( !pRoute->m_bIsTrack && !( pRoute->m_bIsInLayer ) && (!pRoute->m_btemp) )
            m_pXMLrootnode->AddRoute( CreateGPXRte( pRoute ) );
        node1 = node1->GetNext();
    }

    return true;
}

bool NavObjectCollection::CreateNavObjGPXTracks( void )
{
    // Tracks
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        RoutePointList *pRoutePointList = pRoute->pRoutePointList;             //->GetCount(); do if

        if( pRoutePointList->GetCount() ) {
            if( pRoute->m_bIsTrack && !( pRoute->m_bIsInLayer ) && (!pRoute->m_btemp) ) 
                m_pXMLrootnode->AddTrack( CreateGPXTrk( pRoute ) );
        }
        node1 = node1->GetNext();
    }

    return true;
}

bool NavObjectCollection::LoadAllGPXObjects()
{
    //FIXME: unite with MyConfig::ImportGPX
    TiXmlNode *root = RootElement();

    wxString RootName = wxString::FromUTF8( root->Value() );
    if( RootName == _T ( "gpx" ) ) {
        TiXmlNode *child;
        for( child = root->FirstChild(); child != 0; child = child->NextSibling() ) {
            wxString ChildName = wxString::FromUTF8( child->Value() );
            if( ChildName == _T ( "trk" ) ) ::GPXLoadTrack( (GpxTrkElement *) child );
            else
                if( ChildName == _T ( "rte" ) ) {
                    int m_NextRouteNum = 0; //FIXME: we do not need it for GPX
                    ::GPXLoadRoute( (GpxRteElement *) child, m_NextRouteNum );
                } else
                    if( ChildName == _T ( "wpt" ) ) {
                        int m_NextWPNum = 0; //FIXME: we do not need it for GPX
                        RoutePoint *pWp = ::LoadGPXWaypoint( (GpxWptElement *) child,
                                _T("circle") );
                        RoutePoint *pExisting = WaypointExists( pWp->GetName(), pWp->m_lat,
                                pWp->m_lon );
                        if( !pExisting ) {
                            if( NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->Append( pWp );
                            pWp->m_bIsolatedMark = true;      // This is an isolated mark
                            pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                            pWp->m_ConfigWPNum = m_NextWPNum;
                            m_NextWPNum++;
                        }
                    }
        }
    }

    return true;
}

