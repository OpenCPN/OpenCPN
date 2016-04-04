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
#include "routeman.h"
#include "navutil.h"
#include "Select.h"


extern WayPointman *pWayPointMan;
extern Routeman    *g_pRouteMan;
extern MyConfig    *pConfig;

extern RouteList *pRouteList;
extern TrackList *pTrackList;
extern Select *pSelect;
//extern bool g_bIsNewLayer;
//extern bool g_bLayerViz;

extern int g_iWaypointRangeRingsNumber;
extern float g_fWaypointRangeRingsStep;
extern int g_iWaypointRangeRingsStepUnits;

NavObjectCollection1::NavObjectCollection1()
: pugi::xml_document()
{
}

NavObjectCollection1::~NavObjectCollection1()
{
}



RoutePoint * GPXLoadWaypoint1( pugi::xml_node &wpt_node, 
                               wxString def_symbol_name,
                               wxString GUID,
                               bool b_fullviz,
                               bool b_layer,
                               bool b_layerviz,
                               int layer_id
                             )
{
    bool bviz = false;
    bool bviz_name = false;
    bool bauto_name = false;
    bool bshared = false;
    bool b_propvizname = false;
    bool b_propviz = false;
    bool bviz_waypointrangerings = false;

    wxString SymString = def_symbol_name;       // default icon
    wxString NameString;
    wxString DescString;
    wxString TypeString;
    wxString GuidString = GUID;         // default
    wxString TimeString;
    wxDateTime dt;
    RoutePoint *pWP = NULL;
    
    HyperlinkList *linklist = NULL;

    double rlat = wpt_node.attribute( "lat" ).as_double();
    double rlon = wpt_node.attribute( "lon" ).as_double();
    double ArrivalRadius = 0;
    int     l_iWaypointRangeRingsNumber = -1;
    float   l_fWaypointRangeRingsStep = -1;
    int     l_pWaypointRangeRingsStepUnits = -1;
    bool    l_bWaypointRangeRingsVisible = false;
    wxColour    l_wxcWaypointRangeRingsColour;
    l_wxcWaypointRangeRingsColour.Set( _T( "#FFFFFF" ) );

    for( pugi::xml_node child = wpt_node.first_child(); child != 0; child = child.next_sibling() ) {
        const char *pcn = child.name();
        
        if( !strcmp( pcn, "sym" ) ) {
            SymString = wxString::FromUTF8( child.first_child().value() );
        }
        else
        if( !strcmp( pcn, "time") ) 
            TimeString = wxString::FromUTF8( child.first_child().value() );

        else
        if( !strcmp( pcn, "name") ) {
                NameString = wxString::FromUTF8( child.first_child().value() );
        } 
        
        else
        if( !strcmp( pcn, "desc") ) {
                DescString = wxString::FromUTF8( child.first_child().value() );
        }
        
        else
        if( !strcmp( pcn, "type") ) {
                TypeString = wxString::FromUTF8( child.first_child().value() );
        }
        
        else              // Read hyperlink
        if( !strcmp( pcn, "link") ) {
            wxString HrefString;
            wxString HrefTextString;
            wxString HrefTypeString;
            if( linklist == NULL )
                linklist = new HyperlinkList;
            HrefString = wxString::FromUTF8( child.first_attribute().value() );

            for( pugi::xml_node child1 = child.first_child(); child1; child1 = child1.next_sibling() ) {
                wxString LinkString = wxString::FromUTF8( child1.name() );

                if( LinkString == _T ( "text" ) )
                    HrefTextString = wxString::FromUTF8( child1.first_child().value() );
                if( LinkString == _T ( "type" ) ) 
                    HrefTypeString = wxString::FromUTF8( child1.first_child().value() );
            }
          
            Hyperlink *link = new Hyperlink;
            link->Link = HrefString;
            link->DescrText = HrefTextString;
            link->LType = HrefTypeString;
            linklist->Append( link );
        }

    //    OpenCPN Extensions....
        else
        if( !strcmp( pcn, "extensions") ) {
            for( pugi::xml_node ext_child = child.first_child(); ext_child; ext_child = ext_child.next_sibling() ) {
                wxString ext_name = wxString::FromUTF8( ext_child.name() );
                if( ext_name == _T ( "opencpn:guid" ) ) {
                  GuidString = wxString::FromUTF8( ext_child.first_child().value() );
                }
                else
                if( ext_name == _T ( "opencpn:viz" ) ) {
                    b_propviz = true;
                    wxString s = wxString::FromUTF8( ext_child.first_child().value() );
                    long v = 0;
                    if( s.ToLong( &v ) )
                        bviz = ( v != 0 );
                }
                else
                if( ext_name == _T ( "opencpn:viz_name" ) ) {
                    b_propvizname = true;
                    wxString s = wxString::FromUTF8( ext_child.first_child().value() );
                    long v = 0;
                    if( s.ToLong( &v ) )
                        bviz_name = ( v != 0 );
                }
                else
                if( ext_name == _T ( "opencpn:auto_name" ) ) {
                    wxString s = wxString::FromUTF8( ext_child.first_child().value() );
                    long v = 0;
                    if( s.ToLong( &v ) )
                        bauto_name = ( v != 0 );
                }
                else
                if( ext_name  == _T ( "opencpn:shared" ) ) {
                    wxString s = wxString::FromUTF8( ext_child.first_child().value() );
                    long v = 0;
                    if( s.ToLong( &v ) )
                        bshared = ( v != 0 );
                }
                if( ext_name == _T ( "opencpn:arrival_radius" ) ) {
                    wxString::FromUTF8(ext_child.first_child().value()).ToDouble(&ArrivalRadius ) ;
                }
                if ( ext_name == _T("opencpn:waypoint_range_rings") ) {
                    for ( pugi::xml_attribute attr = ext_child.first_attribute(); attr; attr = attr.next_attribute() ) {
                        if ( wxString::FromUTF8(attr.name()) == _T("number") )
                            l_iWaypointRangeRingsNumber = attr.as_int();
                        else if ( wxString::FromUTF8(attr.name()) == _T("step") )
                            l_fWaypointRangeRingsStep = attr.as_float();
                        else if ( wxString::FromUTF8(attr.name()) == _T("units") )
                            l_pWaypointRangeRingsStepUnits = attr.as_int();
                        else if ( wxString::FromUTF8(attr.name()) == _T("visible") )
                            l_bWaypointRangeRingsVisible =  attr.as_bool();
                        else if ( wxString::FromUTF8(attr.name()) == _T("colour") )
                            l_wxcWaypointRangeRingsColour.Set( wxString::FromUTF8( attr.as_string() ) );
                    }
                }
             }// for 
        } //extensions
    }   // for

    // Create waypoint

    if( b_layer ) {
        if( GuidString.IsEmpty() )
            GuidString = _T("LayGUID");
    }

    pWP = new RoutePoint( rlat, rlon, SymString, NameString, GuidString, false ); // do not add to global WP list yet...
    pWP->m_MarkDescription = DescString;
    pWP->m_bIsolatedMark = bshared;      // This is an isolated mark
    pWP->SetWaypointArrivalRadius( ArrivalRadius );
    pWP->SetWaypointRangeRingsNumber( l_iWaypointRangeRingsNumber );
    pWP->SetWaypointRangeRingsStep( l_fWaypointRangeRingsStep );
    pWP->SetWaypointRangeRingsStepUnits( l_pWaypointRangeRingsStepUnits );
    pWP->SetShowWaypointRangeRings( l_bWaypointRangeRingsVisible );
    pWP->SetWaypointRangeRingsColour( l_wxcWaypointRangeRingsColour );

    if( b_propvizname )
        pWP->m_bShowName = bviz_name;
    else
        if( b_fullviz )
            pWP->m_bShowName = true;
        else
            pWP->m_bShowName = false;

    if( b_propviz )
        pWP->m_bIsVisible = bviz;
    else
        if( b_fullviz )
            pWP->m_bIsVisible = true;

    if(b_layer) {
        pWP->m_bIsInLayer = true;
        pWP->m_LayerID = layer_id;
        pWP->m_bIsVisible = b_layerviz;
        pWP->SetListed( false );
    }
   
    pWP->m_bKeepXRoute = bshared;
    pWP->m_bDynamicName = bauto_name;

    if(TimeString.Len()) {
        pWP->m_timestring = TimeString;
        pWP->SetCreateTime(wxInvalidDateTime);          // cause deferred timestamp parsing
    }
        

    if( linklist ) {
        delete pWP->m_HyperlinkList;                    // created in RoutePoint ctor
        pWP->m_HyperlinkList = linklist;
    }

    return ( pWP );
}

TrackPoint * GPXLoadTrackPoint1( pugi::xml_node &wpt_node )
{

    wxString TimeString;
    TrackPoint *pWP = NULL;
    
    double rlat = wpt_node.attribute( "lat" ).as_double();
    double rlon = wpt_node.attribute( "lon" ).as_double();

    for( pugi::xml_node child = wpt_node.first_child(); child != 0; child = child.next_sibling() ) {
        const char *pcn = child.name();
        
        if( !strcmp( pcn, "time") ) 
            TimeString = wxString::FromUTF8( child.first_child().value() );


    //    OpenCPN Extensions....
        else
        if( !strcmp( pcn, "extensions") ) {
            for( pugi::xml_node ext_child = child.first_child(); ext_child; ext_child = ext_child.next_sibling() ) {
                wxString ext_name = wxString::FromUTF8( ext_child.name() );
                if( ext_name == _T ( "opencpn:action" ) ) {
                }
             }// for 
        } //extensions
    }   // for

    // Create waypoint

    pWP = new TrackPoint( rlat, rlon );

    if(TimeString.Len()) {
        pWP->m_timestring = TimeString;
        pWP->SetCreateTime(wxInvalidDateTime);          // cause deferred timestamp parsing
    }

    return ( pWP );
}

Track *GPXLoadTrack1( pugi::xml_node &trk_node, bool b_fullviz,
                      bool b_layer,
                      bool b_layerviz,
                      int layer_id )
{
    wxString TrackName;
    wxString DescString;
    unsigned short int GPXSeg;            
    bool b_propviz = false;
    bool b_viz = true;
    Track *pTentTrack = NULL;
    HyperlinkList *linklist = NULL;
    
    wxString Name = wxString::FromUTF8( trk_node.name() );
    if( Name == _T ( "trk" ) ) {
        pTentTrack = new Track();
        GPXSeg = 0;   
        
        TrackPoint *pWp = NULL;
        
        for( pugi::xml_node tschild = trk_node.first_child(); tschild; tschild = tschild.next_sibling() ) {
            wxString ChildName = wxString::FromUTF8( tschild.name() );
            if( ChildName == _T ( "trkseg" ) ) {
                GPXSeg += 1;                
                
                //    Official GPX spec calls for trkseg to have children trkpt
                for( pugi::xml_node tpchild = tschild.first_child(); tpchild; tpchild = tpchild.next_sibling() ) {
                    wxString tpChildName = wxString::FromUTF8( tpchild.name() );
                    if( tpChildName == _T("trkpt") ) {
                        pWp = ::GPXLoadTrackPoint1(tpchild);
                        pTentTrack->AddPoint( pWp );          // defer BBox calculation
                        pWp->m_GPXTrkSegNo = GPXSeg;
                    }
                }
            }
            else
            if( ChildName == _T ( "name" ) )
                TrackName = wxString::FromUTF8( tschild.first_child().value() );
            else
            if( ChildName == _T ( "desc" ) )
                DescString = wxString::FromUTF8( tschild.first_child().value() );
            else
            
            if( ChildName == _T ( "link") ) {
                wxString HrefString;
                wxString HrefTextString;
                wxString HrefTypeString;
                if( linklist == NULL )
                    linklist = new HyperlinkList;
                HrefString = wxString::FromUTF8( tschild.first_attribute().value() );

                for( pugi::xml_node child1 = tschild.first_child(); child1; child1 = child1.next_sibling() ) {
                    wxString LinkString = wxString::FromUTF8( child1.name() );

                    if( LinkString == _T ( "text" ) )
                        HrefTextString = wxString::FromUTF8( child1.first_child().value() );
                    if( LinkString == _T ( "type" ) ) 
                        HrefTypeString = wxString::FromUTF8( child1.first_child().value() );
                }
              
                Hyperlink *link = new Hyperlink;
                link->Link = HrefString;
                link->DescrText = HrefTextString;
                link->LType = HrefTypeString;
                linklist->Append( link );
            }
            
            else
            if( ChildName == _T ( "extensions" ) ) {
                        for( pugi::xml_node ext_child = tschild.first_child(); ext_child; ext_child = ext_child.next_sibling() ) {
                            wxString ext_name = wxString::FromUTF8( ext_child.name() );
                            if( ext_name == _T ( "opencpn:start" ) ) {
                                pTentTrack->m_TrackStartString = wxString::FromUTF8(ext_child.first_child().value());
                            }
                            else
                            if( ext_name == _T ( "opencpn:end" ) ) {
                                pTentTrack->m_TrackEndString = wxString::FromUTF8(ext_child.first_child().value());
                            }
                            
                            else
                            if( ext_name == _T ( "opencpn:viz" ) ) {
                                wxString viz = wxString::FromUTF8(ext_child.first_child().value());
                                b_propviz = true;
                                b_viz = ( viz == _T("1") );
                            }
                            else
                            if( ext_name == _T ( "opencpn:style" ) ) {
                                for (pugi::xml_attribute attr = ext_child.first_attribute(); attr; attr = attr.next_attribute())
                                {
                                    if( !strcmp( attr.name(), "style" ) )
                                        pTentTrack->m_style = (wxPenStyle)attr.as_int();
                                    else if( !strcmp( attr.name(), "width" ) )
                                        pTentTrack->m_width = attr.as_int();
                                }
                            }
 
                            else
                            if( ext_name == _T ( "opencpn:guid" ) ) {
                                    pTentTrack->m_GUID =  wxString::FromUTF8(ext_child.first_child().value());
                            }
                            
                            else
                            if( ext_name.EndsWith( _T ( "TrackExtension" ) ) ) //Parse GPXX color
                              {
                                for( pugi::xml_node gpxx_child = ext_child.first_child(); gpxx_child; gpxx_child = gpxx_child.next_sibling() ) {
                                    wxString gpxx_name = wxString::FromUTF8( gpxx_child.name() );
                                    if( gpxx_name.EndsWith( _T ( "DisplayColor" ) ) )
                                        pTentTrack->m_Colour = wxString::FromUTF8(gpxx_child.first_child().value() );
                                }
                            }
                    } //extensions
                }
        }
        
        pTentTrack->m_TrackNameString = TrackName;
        pTentTrack->m_TrackDescription = DescString;

        if( b_propviz )
                pTentTrack->SetVisible( b_viz );
        else {
                if( b_fullviz )
                    pTentTrack->SetVisible();
        }

        if( b_layer ) {
            pTentTrack->SetVisible( b_layerviz );
            pTentTrack->m_bIsInLayer = true;
            pTentTrack->m_LayerID = layer_id;
            pTentTrack->SetListed( false );
        }
                
        pTentTrack->SetCurrentTrackSeg( GPXSeg );
                
    }
    
    if( linklist ) {
        delete pTentTrack->m_HyperlinkList;                    // created in TrackPoint ctor
        pTentTrack->m_HyperlinkList = linklist;
    }

    return pTentTrack;
}


Route *GPXLoadRoute1( pugi::xml_node &wpt_node, bool b_fullviz,
                      bool b_layer, bool b_layerviz,
                      int layer_id, bool b_change )
{
    wxString RouteName;
    wxString DescString;
    bool b_propviz = false;
    bool b_viz = true;
    Route *pTentRoute = NULL;
    
    wxString Name = wxString::FromUTF8( wpt_node.name() );
    if( Name == _T ( "rte" ) ) {
        pTentRoute = new Route();
        
        RoutePoint *pWp = NULL;
		bool route_existing = false;
        
        for( pugi::xml_node tschild = wpt_node.first_child(); tschild; tschild = tschild.next_sibling() ) {
            wxString ChildName = wxString::FromUTF8( tschild.name() );

			//load extentions first to determine if the route still exists
			if( ChildName == _T ( "extensions" ) ) {
                for( pugi::xml_node ext_child = tschild.first_child(); ext_child; ext_child = ext_child.next_sibling() ) {
                    wxString ext_name = wxString::FromUTF8( ext_child.name() );

                    if( ext_name == _T ( "opencpn:start" ) ) {
                        pTentRoute->m_RouteStartString = wxString::FromUTF8(ext_child.first_child().value());
                    }
                    else
                    if( ext_name == _T ( "opencpn:end" ) ) {
                        pTentRoute->m_RouteEndString = wxString::FromUTF8(ext_child.first_child().value());
                    }
                        
                    else
                    if( ext_name == _T ( "opencpn:viz" ) ) {
                                wxString viz = wxString::FromUTF8(ext_child.first_child().value());
                                b_propviz = true;
                                b_viz = ( viz == _T("1") );
                    }
                    
                    else
                    if( ext_name == _T ( "opencpn:style" ) ) {
                        for (pugi::xml_attribute attr = ext_child.first_attribute(); attr; attr = attr.next_attribute())
                        {
                            if( !strcmp( attr.name(), "style" ) )
                                pTentRoute->m_style = (wxPenStyle)attr.as_int();
                            else
                            if( !strcmp( attr.name(), "width" ) )
                                pTentRoute->m_width = attr.as_int();
                        }
                     }
                     
                     else
                     if( ext_name == _T ( "opencpn:guid" ) ) {
                        //if ( !g_bIsNewLayer ) ) 
                        pTentRoute->m_GUID =  wxString::FromUTF8(ext_child.first_child().value());
                     }
                     
                     else
                     if( ext_name == _T ( "opencpn:planned_speed" ) ) {
                        pTentRoute->m_PlannedSpeed = atof( ext_child.first_child().value() );
                     }
                     
                     else
                     if( ext_name == _T ( "opencpn:planned_departure" ) ) {
                        ParseGPXDateTime( pTentRoute->m_PlannedDeparture, wxString::FromUTF8(ext_child.first_child().value()) );
                     }
                     
                     else
                     if( ext_name == _T ( "opencpn:time_display" ) ) {
                        pTentRoute->m_TimeDisplayFormat, wxString::FromUTF8(ext_child.first_child().value());
                     }
                     else
                     if( ext_name.EndsWith( _T ( "RouteExtension" ) ) ) //Parse GPXX color
                     {
                        for( pugi::xml_node gpxx_child = ext_child.first_child(); gpxx_child; gpxx_child = gpxx_child.next_sibling() ) {
                            wxString gpxx_name = wxString::FromUTF8( gpxx_child.name() );
                            if( gpxx_name.EndsWith( _T ( "DisplayColor" ) ) )
                                 pTentRoute->m_Colour = wxString::FromUTF8(gpxx_child.first_child().value() );
                        }
                     }
                }
				if( !b_change) {
					if( RouteExists(pTentRoute->m_GUID) ) { //we are loading a different route with the same guid so let's generate a new guid
						pTentRoute->m_GUID = pWayPointMan->CreateGUID(NULL);
						route_existing = true;
					}
				}
			}//extension
			else
            if( ChildName == _T ( "rtept" ) ) {
                RoutePoint *tpWp = ::GPXLoadWaypoint1(  tschild, _T("square"), _T(""), b_fullviz, b_layer, b_layerviz, layer_id);
                RoutePoint *erp = ::WaypointExists( tpWp->m_GUID );
				// 1) if b_change is true, that means we are after crash - load the route and points as found in source file
				// 2) if route_existing, we are loading a different route with the same guid. In this case load points as found in
				//source file, changing the guid, but keep existing "isolated point" as found in the DB
				// 3) in all other cases keep existing points if found and load new points if not found
				bool new_wpt = true;
				if( b_change )
					pWp = tpWp;
                else {
					if(  erp != NULL && (!route_existing || ( route_existing && tpWp->m_bKeepXRoute) ) ) {
						pWp = erp;
						new_wpt = false;
					}
					else {
						if( route_existing )
							tpWp->m_GUID = pWayPointMan->CreateGUID( NULL );
						pWp = tpWp;
					}
				}

                pTentRoute->AddPoint( pWp, false, true );          // defer BBox calculation
                pWp->m_bIsInRoute = true;                      // Hack

                if( new_wpt )
					pWayPointMan->AddRoutePoint( pWp );
				else
					delete tpWp;
            }
            else
            if( ChildName == _T ( "name" ) ) {
                RouteName = wxString::FromUTF8( tschild.first_child().value() );
            }
            else
            if( ChildName == _T ( "desc" ) ) {
                DescString = wxString::FromUTF8( tschild.first_child().value() );
            }

            else
            //TODO: This is wrong, left here just to save data of the 3.3 beta series users.
            if( ChildName.EndsWith( _T ( "RouteExtension" ) ) ) //Parse GPXX color
            {
                for( pugi::xml_node gpxx_child = tschild.first_child(); gpxx_child; gpxx_child = gpxx_child.next_sibling() ) {
                    wxString gpxx_name = wxString::FromUTF8( gpxx_child.name() );
                    if( gpxx_name.EndsWith( _T ( "DisplayColor" ) ) )
                         pTentRoute->m_Colour = wxString::FromUTF8(gpxx_child.first_child().value() );
                }
            }

       }
                    
        pTentRoute->m_RouteNameString = RouteName;
        pTentRoute->m_RouteDescription = DescString;

        if( b_propviz )
                  pTentRoute->SetVisible( b_viz );
        else {
            if( b_fullviz )
                pTentRoute->SetVisible();
        }
 
        if( b_layer ){
            pTentRoute->SetVisible( b_layerviz );
            pTentRoute->m_bIsInLayer = true;
            pTentRoute->m_LayerID = layer_id;
            pTentRoute->SetListed( false );
        }            
 
    }

    return pTentRoute;
}


bool GPXCreateWpt( pugi::xml_node node, RoutePoint *pr, unsigned int flags )
{
    wxString s;
    pugi::xml_node child;
    pugi::xml_attribute attr;
    
    s.Printf(_T("%.9f"), pr->m_lat);
    node.append_attribute("lat") = s.mb_str();
    s.Printf(_T("%.9f"), pr->m_lon);
    node.append_attribute("lon") = s.mb_str();
 
    if(flags & OUT_TIME) {
            child = node.append_child("time");
            if( pr->m_timestring.Len() )
                child.append_child(pugi::node_pcdata).set_value(pr->m_timestring.mb_str());
            else {
                wxString t = pr->GetCreateTime().FormatISODate().Append(_T("T")).Append(pr->GetCreateTime().FormatISOTime()).Append(_T("Z"));
                child.append_child(pugi::node_pcdata).set_value(t.mb_str());
            }
    }
        
    if ( (!pr->GetName().IsEmpty() && (flags & OUT_NAME)) || (flags & OUT_NAME_FORCE) ) {
        wxCharBuffer buffer=pr->GetName().ToUTF8();
        if(buffer.data()) {
            child = node.append_child("name");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }       

    if ( (!pr->GetDescription().IsEmpty() && (flags & OUT_DESC)) || (flags & OUT_DESC_FORCE) ) {
        wxCharBuffer buffer=pr->GetDescription().ToUTF8();
        if(buffer.data()) {
            child = node.append_child("desc");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }       

    // Hyperlinks
    if(flags & OUT_HYPERLINKS ){
        HyperlinkList *linklist = pr->m_HyperlinkList;
        if( linklist && linklist->GetCount() ) {
            wxHyperlinkListNode *linknode = linklist->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();
                
                pugi::xml_node child_link = node.append_child("link");;
                wxCharBuffer buffer=link->Link.ToUTF8();
                if(buffer.data())
                    child_link.append_attribute("href") = buffer.data();
                                
                buffer=link->DescrText.ToUTF8();
                if(buffer.data()) {
                    child = child_link.append_child("text");
                    child.append_child(pugi::node_pcdata).set_value(buffer.data());
                }
                
                buffer=link->LType.ToUTF8();
                if(buffer.data() && strlen(buffer.data()) > 0) {
                    child = child_link.append_child("type");
                    child.append_child(pugi::node_pcdata).set_value(buffer.data());
                }
                
                linknode = linknode->GetNext();
            }
        }
    }
    
    if (flags & OUT_SYM_FORCE) {
        child = node.append_child("sym");
        if (!pr->GetIconName().IsEmpty()) {
            child.append_child(pugi::node_pcdata).set_value(pr->GetIconName().mb_str());
        }
        else {
            child.append_child("empty");
        }
    }       
    
    if(flags & OUT_TYPE) {
        child = node.append_child("type");
        child.append_child(pugi::node_pcdata).set_value("WPT");
    }
    
    if( (flags & OUT_GUID) || (flags & OUT_VIZ) || (flags & OUT_VIZ_NAME) || (flags & OUT_SHARED)
            || (flags & OUT_AUTO_NAME)  || (flags & OUT_EXTENSION) ) {
        
        pugi::xml_node child_ext = node.append_child("extensions");
        
        if (!pr->m_GUID.IsEmpty() && (flags & OUT_GUID) ) {
            child = child_ext.append_child("opencpn:guid");
            child.append_child(pugi::node_pcdata).set_value(pr->m_GUID.mb_str());
        }
         
         if((flags & OUT_VIZ) && !pr->m_bIsVisible) {
             child = child_ext.append_child("opencpn:viz");
             child.append_child(pugi::node_pcdata).set_value("0");
         }
            
         if((flags & OUT_VIZ_NAME) && pr->m_bShowName) {
             child = child_ext.append_child("opencpn:viz_name");
             child.append_child(pugi::node_pcdata).set_value("1");
         }
         
         if((flags & OUT_AUTO_NAME) && pr->m_bDynamicName) {
             child = child_ext.append_child("opencpn:auto_name");
             child.append_child(pugi::node_pcdata).set_value("1");
         }
         if((flags & OUT_SHARED) && pr->m_bKeepXRoute) {
             child = child_ext.append_child("opencpn:shared");
             child.append_child(pugi::node_pcdata).set_value("1");
         }
        if(flags & OUT_ARRIVAL_RADIUS) {
            child = child_ext.append_child("opencpn:arrival_radius");
            s.Printf(_T("%.3f"), pr->GetWaypointArrivalRadius());
            child.append_child(pugi::node_pcdata).set_value( s.mbc_str() );
        }
        if(flags & OUT_WAYPOINT_RANGE_RINGS) {
            child = child_ext.append_child("opencpn:waypoint_range_rings");
            pugi::xml_attribute viz = child.append_attribute( "visible" );
            viz.set_value( pr->m_bShowWaypointRangeRings );
            pugi::xml_attribute number = child.append_attribute( "number" );
            number.set_value( pr->m_iWaypointRangeRingsNumber );
            pugi::xml_attribute step = child.append_attribute( "step" );
            step.set_value( pr->m_fWaypointRangeRingsStep );
            pugi::xml_attribute units = child.append_attribute( "units" );
            units.set_value( pr->m_iWaypointRangeRingsStepUnits );
            pugi::xml_attribute colour = child.append_attribute( "colour" );
            colour.set_value( pr->m_wxcWaypointRangeRingsColour.GetAsString( wxC2S_HTML_SYNTAX ).utf8_str() ) ;
        }
    }
    
    return true;
}

bool GPXCreateTrkpt( pugi::xml_node node, TrackPoint *pt, unsigned int flags )
{
    wxString s;
    pugi::xml_node child;
    pugi::xml_attribute attr;
    
    s.Printf(_T("%.9f"), pt->m_lat);
    node.append_attribute("lat") = s.mb_str();
    s.Printf(_T("%.9f"), pt->m_lon);
    node.append_attribute("lon") = s.mb_str();
 
    if(flags & OUT_TIME) {
        child = node.append_child("time");
        if( pt->m_timestring.Len() )
            child.append_child(pugi::node_pcdata).set_value(pt->m_timestring.mb_str());
        else {
            wxString t = pt->GetCreateTime().FormatISODate().Append(_T("T")).Append(pt->GetCreateTime().FormatISOTime()).Append(_T("Z"));
            child.append_child(pugi::node_pcdata).set_value(t.mb_str());
        }
    }
    
    return true;
}

bool GPXCreateTrk( pugi::xml_node node, Track *pTrack, unsigned int flags )
{
    pugi::xml_node child;

    if( pTrack->m_TrackNameString.Len() ) {
        wxCharBuffer buffer=pTrack->m_TrackNameString.ToUTF8();
        if(buffer.data()) {
            child = node.append_child("name");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }
    
    if( pTrack->m_TrackDescription.Len() ) {
        wxCharBuffer buffer=pTrack->m_TrackDescription.ToUTF8();
        if(buffer.data()) {
            child = node.append_child("desc");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }
    
    // Hyperlinks
    HyperlinkList *linklist = pTrack->m_HyperlinkList;
    if( linklist && linklist->GetCount() ) {
        wxHyperlinkListNode *linknode = linklist->GetFirst();
        while( linknode ) {
            Hyperlink *link = linknode->GetData();
        
            pugi::xml_node child_link = node.append_child("link");
            wxCharBuffer buffer = link->Link.ToUTF8();
            if(buffer.data())
                child_link.append_attribute("href") = buffer.data();
                        
            buffer=link->DescrText.ToUTF8();
            if(buffer.data()) {
                child = child_link.append_child("text");
                child.append_child(pugi::node_pcdata).set_value(buffer.data());
            }
        
            buffer=link->LType.ToUTF8();
            if(buffer.data()  && strlen(buffer.data()) > 0) {
                child = child_link.append_child("type");
                child.append_child(pugi::node_pcdata).set_value(buffer.data());
            }
        
            linknode = linknode->GetNext();
        }
    }
    
    pugi::xml_node child_ext = node.append_child("extensions");
    
    child = child_ext.append_child("opencpn:guid");
    child.append_child(pugi::node_pcdata).set_value(pTrack->m_GUID.mb_str());
    
    child = child_ext.append_child("opencpn:viz");
    child.append_child(pugi::node_pcdata).set_value(pTrack->IsVisible() == true ? "1" : "0");

    if( pTrack->m_TrackStartString.Len() ) {
        wxCharBuffer buffer=pTrack->m_TrackStartString.ToUTF8();
        if(buffer.data()) {
            child = child_ext.append_child("opencpn:start");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }
    
    if( pTrack->m_TrackEndString.Len() ) {
        wxCharBuffer buffer=pTrack->m_TrackEndString.ToUTF8();
        if(buffer.data()) {
            child = child_ext.append_child("opencpn:end");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }
    
    if( pTrack->m_width != WIDTH_UNDEFINED || pTrack->m_style != wxPENSTYLE_INVALID ) {
        child = child_ext.append_child("opencpn:style");
        
        if( pTrack->m_width != WIDTH_UNDEFINED )
            child.append_attribute("width") = pTrack->m_width;
        if( pTrack->m_style != wxPENSTYLE_INVALID )
            child.append_attribute("style") = pTrack->m_style;
    }
    
    if( pTrack->m_Colour != wxEmptyString ) {
        pugi::xml_node gpxx_ext = child_ext.append_child("gpxx:TrackExtension");
        child = gpxx_ext.append_child("gpxx:DisplayColor");
        child.append_child(pugi::node_pcdata).set_value(pTrack->m_Colour.mb_str());
    }
        
    
    if(flags & RT_OUT_NO_RTPTS)
        return true;
    
    int node2 = 0;
    TrackPoint *prp;
        
    unsigned short int GPXTrkSegNo1 = 1;
        
    do {
        unsigned short int GPXTrkSegNo2 = GPXTrkSegNo1;
        
        pugi::xml_node seg = node.append_child("trkseg");
        
        while( node2 < pTrack->GetnPoints() ) {
            prp = pTrack->GetPoint(node2);
            GPXTrkSegNo1 = prp->m_GPXTrkSegNo;
            if(GPXTrkSegNo1 != GPXTrkSegNo2)
                break;
            
            GPXCreateTrkpt(seg.append_child("trkpt"), prp, OPT_TRACKPT);
            
            node2++;
        }
    } while( node2 < pTrack->GetnPoints() );
        
        
    return true;
}
                   
bool GPXCreateRoute( pugi::xml_node node, Route *pRoute )
{
    pugi::xml_node child;
    
    if( pRoute->m_RouteNameString.Len() ) {
        wxCharBuffer buffer=pRoute->m_RouteNameString.ToUTF8();
        if(buffer.data()) {
            child = node.append_child("name");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }
    
    if( pRoute->m_RouteDescription.Len() ) {
        wxCharBuffer buffer=pRoute->m_RouteDescription.ToUTF8();
        if(buffer.data()) {
            child = node.append_child("desc");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }
    
    pugi::xml_node child_ext = node.append_child("extensions");
    
    child = child_ext.append_child("opencpn:guid");
    child.append_child(pugi::node_pcdata).set_value(pRoute->m_GUID.mb_str());
    
    child = child_ext.append_child("opencpn:viz");
    child.append_child(pugi::node_pcdata).set_value(pRoute->IsVisible() == true ? "1" : "0");
    
    if( pRoute->m_RouteStartString.Len() ) {
        wxCharBuffer buffer=pRoute->m_RouteStartString.ToUTF8();
        if(buffer.data()) {
            child = child_ext.append_child("opencpn:start");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }
    
    if( pRoute->m_RouteEndString.Len() ) {
        wxCharBuffer buffer=pRoute->m_RouteEndString.ToUTF8();
        if(buffer.data()) {
            child = child_ext.append_child("opencpn:end");
            child.append_child(pugi::node_pcdata).set_value(buffer.data());
        }
    }
    
    if( pRoute->m_PlannedSpeed != ROUTE_DEFAULT_SPEED ) {
        child = child_ext.append_child("opencpn:planned_speed");
        wxString s;
        s.Printf(_T("%.2f"), pRoute->m_PlannedSpeed);
        child.append_child(pugi::node_pcdata).set_value(s.mb_str());
    }
    
    if( pRoute->m_PlannedDeparture.IsValid() ) {
        child = child_ext.append_child("opencpn:planned_departure");
        wxString t = pRoute->m_PlannedDeparture.FormatISODate().Append(_T("T")).Append(pRoute->m_PlannedDeparture.FormatISOTime()).Append(_T("Z"));
        child.append_child(pugi::node_pcdata).set_value(t.mb_str());
    }

    if( pRoute->m_TimeDisplayFormat != RTE_TIME_DISP_UTC ) {
        child = child_ext.append_child("opencpn:time_display");
        child.append_child(pugi::node_pcdata).set_value(pRoute->m_TimeDisplayFormat.mb_str());
    }                        
    
    if( pRoute->m_width != WIDTH_UNDEFINED || pRoute->m_style != wxPENSTYLE_INVALID ) {
        child = child_ext.append_child("opencpn:style");
        
        if( pRoute->m_width != WIDTH_UNDEFINED )
            child.append_attribute("width") = pRoute->m_width;
        if( pRoute->m_style != wxPENSTYLE_INVALID )
            child.append_attribute("style") = pRoute->m_style;
    }
    
    if( pRoute->m_Colour != wxEmptyString ) {
        pugi::xml_node gpxx_ext = child_ext.append_child("gpxx:RouteExtension");
        child = gpxx_ext.append_child("gpxx:DisplayColor");
        child.append_child(pugi::node_pcdata).set_value(pRoute->m_Colour.mb_str());
    }
                                 
    RoutePointList *pRoutePointList = pRoute->pRoutePointList;
    wxRoutePointListNode *node2 = pRoutePointList->GetFirst();
    RoutePoint *prp;
    
    unsigned short int GPXTrkSegNo1 = 1;
    
    while( node2  ) {
        prp = node2->GetData();
            
        GPXCreateWpt(node.append_child("rtept"), prp, OPT_ROUTEPT);
            
        node2 = node2->GetNext();
    }
    
    return true;
}
                       

void InsertRouteA( Route *pTentRoute )
{
    if( !pTentRoute )
        return;
    
    bool bAddroute = true;
    //    If the route has only 1 point, don't load it.
    if( pTentRoute->GetnPoints() < 2 )
        bAddroute = false;
    
    //    TODO  All this trouble for a tentative route.......Should make some Route methods????
    if( bAddroute ) {
            
        pRouteList->Append( pTentRoute );
        pTentRoute->RebuildGUIDList();                  // ensure the GUID list is intact
        
                 
                //    Do the (deferred) calculation of BBox
                    pTentRoute->FinalizeForRendering();

                    
                    //    Add the selectable points and segments
                    
                    int ip = 0;
                    float prev_rlat = 0., prev_rlon = 0.;
                    RoutePoint *prev_pConfPoint = NULL;
                    
                    wxRoutePointListNode *node = pTentRoute->pRoutePointList->GetFirst();
                    while( node ) {
                        RoutePoint *prp = node->GetData();
                        
                        if( ip )
                            pSelect->AddSelectableRouteSegment( prev_rlat, prev_rlon, prp->m_lat,
                                                                prp->m_lon, prev_pConfPoint, prp, pTentRoute );
                        pSelect->AddSelectableRoutePoint(prp->m_lat, prp->m_lon, prp);
                        prev_rlat = prp->m_lat;
                        prev_rlon = prp->m_lon;
                        prev_pConfPoint = prp;
                        
                        ip++;
                        
                        node = node->GetNext();
                    }
    }
    else {
        
        // walk the route, deleting points used only by this route
        wxRoutePointListNode *pnode = ( pTentRoute->pRoutePointList )->GetFirst();
        while( pnode ) {
            RoutePoint *prp = pnode->GetData();
            
            // check all other routes to see if this point appears in any other route
            Route *pcontainer_route = g_pRouteMan->FindRouteContainingWaypoint( prp );
            
            if( pcontainer_route == NULL ) {
                prp->m_bIsInRoute = false; // Take this point out of this (and only) track/route
                if( !prp->m_bKeepXRoute ) {
                    pConfig->m_bSkipChangeSetUpdate = true;
                    pConfig->DeleteWayPoint( prp );
                    pConfig->m_bSkipChangeSetUpdate = false;
                    delete prp;
                }
            }
            
            pnode = pnode->GetNext();
        }
        
        delete pTentRoute;
    }
}
                       
void InsertTrack( Track *pTentTrack )
{
    if(!pTentTrack)
        return;
    
    bool bAddtrack = true;
    //    If the track has only 1 point, don't load it.
    //    This usually occurs if some points were discarded as being co-incident.
    if( pTentTrack->GetnPoints() < 2 )
        bAddtrack = false;
    
    //    TODO  All this trouble for a tentative track.......Should make some Track methods????
    if( bAddtrack ) {
        pTrackList->Append( pTentTrack );
        
               
        //    Do the (deferred) calculation of Track BBox
//        pTentTrack->FinalizeForRendering();
            
        //    Add the selectable points and segments
                
        float prev_rlat = 0., prev_rlon = 0.;
        TrackPoint *prev_pConfPoint = NULL;
                
        for(int i=0; i<pTentTrack->GetnPoints(); i++) {
            TrackPoint *prp = pTentTrack->GetPoint(i);
            
            if( i ) pSelect->AddSelectableTrackSegment( prev_rlat, prev_rlon, prp->m_lat,
                                                         prp->m_lon, prev_pConfPoint, prp, pTentTrack );
                    
            prev_rlat = prp->m_lat;
            prev_rlon = prp->m_lon;
            prev_pConfPoint = prp;
        }
    } else
        delete pTentTrack;
}                       
                       
void UpdateRouteA( Route *pTentRoute )
 {
    if( !pTentRoute )
        return;
    if( pTentRoute->GetnPoints() < 2 )
        return;

    //first delete the route to be modified if exists
    Route *pExisting = ::RouteExists( pTentRoute->m_GUID );
    if( pExisting ) {
        pConfig->m_bSkipChangeSetUpdate = true;
        g_pRouteMan->DeleteRoute( pExisting );
        pConfig->m_bSkipChangeSetUpdate = false;
    }

    //create a new route
    Route *pChangeRoute = new Route();
    pRouteList->Append( pChangeRoute );

    //update new route keeping the same gui
    pChangeRoute->m_GUID = pTentRoute->m_GUID;
    pChangeRoute->m_RouteNameString = pTentRoute->m_RouteNameString;
    pChangeRoute->m_RouteStartString = pTentRoute->m_RouteStartString;
    pChangeRoute->m_RouteEndString = pTentRoute->m_RouteEndString;
    pChangeRoute->SetVisible( pTentRoute->IsVisible() );

    //Add points and segments to new route
    int ip = 0;
    float prev_rlat = 0., prev_rlon = 0.;
    RoutePoint *prev_pConfPoint = NULL;

    wxRoutePointListNode *node = pTentRoute->pRoutePointList->GetFirst();
    while( node ) {
		RoutePoint *prp = node->GetData();

		//if some wpts have been not deleted, that meens they should be used in other routes
		//or are isolated way points so need to be updated
		RoutePoint *ex_rp = ::WaypointExists( prp->m_GUID );
		if( ex_rp ) {
			pSelect->DeleteSelectableRoutePoint(ex_rp);
			ex_rp->m_lat = prp->m_lat;
			ex_rp->m_lon = prp->m_lon;
			ex_rp->SetIconName( prp->GetIconName() );
			ex_rp->m_MarkDescription = prp->m_MarkDescription;
			ex_rp->SetName( prp->GetName() );
			pChangeRoute->AddPoint( ex_rp );
			pSelect->AddSelectableRoutePoint(prp->m_lat, prp->m_lon, ex_rp);

		} else {
			pChangeRoute->AddPoint( prp );
			pSelect->AddSelectableRoutePoint(prp->m_lat, prp->m_lon, prp);
		}

        if( ip )
			pSelect->AddSelectableRouteSegment( prev_rlat, prev_rlon, prp->m_lat,
					prp->m_lon, prev_pConfPoint, prp, pChangeRoute );
        prev_rlat = prp->m_lat;
        prev_rlon = prp->m_lon;
        prev_pConfPoint = prp;

        ip++;

        node = node->GetNext();
	}
	//    Do the (deferred) calculation of BBox
    pChangeRoute->FinalizeForRendering();
}

                       
bool NavObjectCollection1::CreateNavObjGPXPoints( void )
{
    
    //    Iterate over the Routepoint list, creating Nodes for
    //    Routepoints that are not in any Route
    //    as indicated by m_bIsolatedMark == false

    if(!pWayPointMan)
        return false;

    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
    
    RoutePoint *pr;
    
    while( node ) {
        pr = node->GetData();
        
        if( ( pr->m_bIsolatedMark ) && !( pr->m_bIsInLayer ) && !(pr->m_btemp) )
        {
            GPXCreateWpt(m_gpx_root.append_child("wpt"), pr, OPT_WPT);
        }
        node = node->GetNext();
    }
    
    return true;
}

bool NavObjectCollection1::CreateNavObjGPXRoutes( void )
{
    // Routes
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        
        if( !pRoute->m_bIsInLayer && !pRoute->m_btemp )
            GPXCreateRoute(m_gpx_root.append_child("rte"), pRoute);
        node1 = node1->GetNext();
    }
    
    return true;
}

bool NavObjectCollection1::CreateNavObjGPXTracks( void )
{
    // Tracks
    wxTrackListNode *node1 = pTrackList->GetFirst();
    while( node1 ) {
        Track *pTrack = node1->GetData();
        
        if( pTrack->GetnPoints() ) {
            if( !pTrack->m_bIsInLayer && !pTrack->m_btemp ) 
                GPXCreateTrk(m_gpx_root.append_child("trk"), pTrack, 0);
        }
        node1 = node1->GetNext();
    }
    
    return true;
}


bool NavObjectCollection1::CreateAllGPXObjects()
{
    SetRootGPXNode();
    
    CreateNavObjGPXPoints();
    CreateNavObjGPXRoutes();
    CreateNavObjGPXTracks();
    
    return true;
}

bool NavObjectCollection1::AddGPXRoute(Route *pRoute)
{
    SetRootGPXNode();
    GPXCreateRoute(m_gpx_root.append_child("rte"), pRoute);
    return true;
}

bool NavObjectCollection1::AddGPXTrack(Track *pTrk)
{
    SetRootGPXNode();
    GPXCreateTrk(m_gpx_root.append_child("trk"), pTrk, 0 );
    return true;
}

bool NavObjectCollection1::AddGPXWaypoint(RoutePoint *pWP )
{
    SetRootGPXNode();
    GPXCreateWpt(m_gpx_root.append_child("wpt"), pWP, OPT_WPT);
    return true;
}

void NavObjectCollection1::AddGPXRoutesList( RouteList *pRoutes )
{
    SetRootGPXNode();
    
    wxRouteListNode* pRoute = pRoutes->GetFirst();
    while (pRoute) {
        Route* pRData = pRoute->GetData();
        AddGPXRoute( pRData);
        pRoute = pRoute->GetNext();
    }
}

void NavObjectCollection1::AddGPXTracksList( TrackList *pTracks )
{
    SetRootGPXNode();
    
    wxTrackListNode* pTrack = pTracks->GetFirst();
    while (pTrack) {
        Track* pRData = pTrack->GetData();
        AddGPXTrack( pRData);
        pTrack = pTrack->GetNext();
    }
}

bool NavObjectCollection1::AddGPXPointsList( RoutePointList *pRoutePoints )
{
    SetRootGPXNode();
    
    wxRoutePointListNode* pRoutePointNode = pRoutePoints->GetFirst();
    while (pRoutePointNode) {
        RoutePoint* pRP = pRoutePointNode->GetData();
        AddGPXWaypoint( pRP);
        pRoutePointNode = pRoutePointNode->GetNext();
    }
    
    return true;
}



void NavObjectCollection1::SetRootGPXNode(void)
{
    if(!strlen(m_gpx_root.name())) {
        m_gpx_root = append_child("gpx");
        m_gpx_root.append_attribute ( "version" ) = "1.1";
        m_gpx_root.append_attribute ( "creator" ) = "OpenCPN";
        m_gpx_root.append_attribute( "xmlns:xsi" ) = "http://www.w3.org/2001/XMLSchema-instance";
        m_gpx_root.append_attribute( "xmlns" ) = "http://www.topografix.com/GPX/1/1";
        m_gpx_root.append_attribute( "xmlns:gpxx" ) =  "http://www.garmin.com/xmlschemas/GpxExtensions/v3";
        m_gpx_root.append_attribute( "xsi:schemaLocation" ) = "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd";
        m_gpx_root.append_attribute( "xmlns:opencpn" ) = "http://www.opencpn.org";
    }
}
        
bool NavObjectCollection1::IsOpenCPN()
{
    for (pugi::xml_attribute attr = root().first_child().first_attribute(); attr; attr = attr.next_attribute())
    if( !strcmp(attr.name(), "creator") && !strcmp(attr.value(), "OpenCPN") )
        return true;
    return false;
}
            
bool NavObjectCollection1::SaveFile( const wxString filename )
{
    save_file(filename.fn_str(), "  ");
    return true;
}

bool NavObjectCollection1::LoadAllGPXObjects( bool b_full_viz )
{
    pugi::xml_node objects = this->child("gpx");
    
    for (pugi::xml_node object = objects.first_child(); object; object = object.next_sibling())
    {
        if( !strcmp(object.name(), "wpt") ) {
            RoutePoint *pWp = ::GPXLoadWaypoint1( object, _T("circle"), _T(""), b_full_viz, false, false, 0 );
            
            if(pWp) {
                pWp->m_bIsolatedMark = true;      // This is an isolated mark
                RoutePoint *pExisting = WaypointExists( pWp->GetName(), pWp->m_lat, pWp->m_lon );
                if( !pExisting ) {
                    if( NULL != pWayPointMan )
                        pWayPointMan->AddRoutePoint( pWp );
                     pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                }
                else
                    delete pWp;
            }
        }
        else
            if( !strcmp(object.name(), "trk") ) {
                Track *pTrack = GPXLoadTrack1( object, b_full_viz, false, false, 0);
                InsertTrack( pTrack );
            }
            else
                if( !strcmp(object.name(), "rte") ) {
                    Route *pRoute = GPXLoadRoute1( object, b_full_viz, false, false, 0, false );
                    InsertRouteA( pRoute );
                }
                
                
    }
    
    return true;
}

int NavObjectCollection1::LoadAllGPXObjectsAsLayer(int layer_id, bool b_layerviz)
{
    if(!pWayPointMan)
        return 0;
    
    int n_obj = 0;
    pugi::xml_node objects = this->child("gpx");
    
    for (pugi::xml_node object = objects.first_child(); object; object = object.next_sibling())
    {
        if( !strcmp(object.name(), "wpt") ) {
            RoutePoint *pWp = ::GPXLoadWaypoint1( object, _T("circle"), _T(""), true, true, b_layerviz, layer_id );
            pWp->m_bIsolatedMark = true;      // This is an isolated mark
            
            if(pWp) {
                pWayPointMan->AddRoutePoint( pWp );
                pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                n_obj++;
            }
            else
                delete pWp;
        }
        else{
            if( !strcmp(object.name(), "trk") ) {
                Track *pTrack = GPXLoadTrack1( object, false, true, b_layerviz, layer_id);
                n_obj++;
                InsertTrack( pTrack );
            }
            else
                if( !strcmp(object.name(), "rte") ) {
                    Route *pRoute = GPXLoadRoute1( object, true, true, b_layerviz, layer_id, false );
                    n_obj++;
                    InsertRouteA( pRoute );
                }
        }   
    }
    
    return n_obj;
}







NavObjectChanges::NavObjectChanges()
: NavObjectCollection1()
{
    m_changes_file = 0;
}



NavObjectChanges::NavObjectChanges(wxString file_name)
    : NavObjectCollection1()
{
    m_filename = file_name;
    
    m_changes_file = fopen(m_filename.mb_str(), "a");
    
    
}

NavObjectChanges::~NavObjectChanges()
{
    if(m_changes_file)
        fclose(m_changes_file);

    if( ::wxFileExists( m_filename ) )
        ::wxRemoveFile( m_filename );
        
}

void NavObjectChanges::AddRoute( Route *pr, const char *action )
{
    SetRootGPXNode();
    
    pugi::xml_node object = m_gpx_root.append_child("rte");
    GPXCreateRoute(object, pr );
    
    pugi::xml_node xchild = object.child("extensions");
    //FIXME  What if extensions do not exist?
    pugi::xml_node child = xchild.append_child("opencpn:action");
    child.append_child(pugi::node_pcdata).set_value(action);

    pugi::xml_writer_file writer(m_changes_file);
    object.print(writer, " ");
    fflush(m_changes_file);
}

void NavObjectChanges::AddTrack( Track *pr, const char *action )
{
    SetRootGPXNode();
    
    pugi::xml_node object = m_gpx_root.append_child("trk");
    GPXCreateTrk(object, pr, RT_OUT_NO_RTPTS );         // emit a void track, no waypoints
    
    pugi::xml_node xchild = object.child("extensions");
    pugi::xml_node child = xchild.append_child("opencpn:action");
    child.append_child(pugi::node_pcdata).set_value(action);

    pugi::xml_writer_file writer(m_changes_file);
    object.print(writer, " ");
    fflush(m_changes_file);
}

void NavObjectChanges::AddWP( RoutePoint *pWP, const char *action )
{
    SetRootGPXNode();
    
    pugi::xml_node object = m_gpx_root.append_child("wpt");
    GPXCreateWpt(object, pWP, OPT_WPT);

    pugi::xml_node xchild = object.child("extensions");
    pugi::xml_node child = xchild.append_child("opencpn:action");
    child.append_child(pugi::node_pcdata).set_value(action);

    pugi::xml_writer_file writer(m_changes_file);
    object.print(writer, " ");
    fflush(m_changes_file);
}

void NavObjectChanges::AddTrackPoint( TrackPoint *pWP, const char *action, const wxString& parent_GUID )
{
    SetRootGPXNode();
    
    pugi::xml_node object = m_gpx_root.append_child("tkpt");
    GPXCreateTrkpt(object, pWP, OPT_TRACKPT);

    pugi::xml_node xchild = object.child("extensions");
    
    pugi::xml_node child = xchild.append_child("opencpn:action");
    child.append_child(pugi::node_pcdata).set_value(action);
    
    pugi::xml_node gchild = xchild.append_child("opencpn:track_GUID");
    gchild.append_child(pugi::node_pcdata).set_value(parent_GUID.mb_str());

    pugi::xml_writer_file writer(m_changes_file);
    object.print(writer, " ");
    fflush(m_changes_file);
}


bool NavObjectChanges::ApplyChanges(void)
{
    //Let's reconstruct the unsaved changes
    
    pugi::xml_node object = this->first_child();
    
    while(strlen(object.name()))
    {
        if( !strcmp(object.name(), "wpt") ) {
            RoutePoint *pWp = ::GPXLoadWaypoint1( object, _T("circle"), _T(""), false, false, false, 0 );
            
            if(pWp && pWayPointMan) {
                pWp->m_bIsolatedMark = true;
                RoutePoint *pExisting = WaypointExists( pWp->m_GUID );
                
                pugi::xml_node xchild = object.child("extensions");
                pugi::xml_node child = xchild.child("opencpn:action");
                
                if(!strcmp(child.first_child().value(), "add") ){
                    if( !pExisting ) 
                        pWayPointMan->AddRoutePoint( pWp );
                    pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                }                    
                
                else if(!strcmp(child.first_child().value(), "update") ){
                    if( pExisting )
                        pWayPointMan->RemoveRoutePoint( pExisting );
                    pWayPointMan->AddRoutePoint( pWp );
                    pSelect->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                }
                
                else if(!strcmp(child.first_child().value(), "delete") ){
                    if( pExisting )
                        pWayPointMan->DestroyWaypoint( pExisting, false );
                }
                 
                else
                    delete pWp;
            }
        }
        else
            if( !strcmp(object.name(), "trk") ) {
                Track * pTrack = GPXLoadTrack1( object, false, false, false, 0);

                if(pTrack && g_pRouteMan) {
                    pugi::xml_node xchild = object.child("extensions");
                    pugi::xml_node child = xchild.child("opencpn:action");
                    
                    Track *pExisting = TrackExists( pTrack->m_GUID );
                    if(!strcmp(child.first_child().value(), "update") ){
                         if( pExisting ) {
                             pExisting->m_TrackNameString = pTrack->m_TrackNameString;
                             pExisting->m_TrackStartString = pTrack->m_TrackStartString;
                             pExisting->m_TrackEndString = pTrack->m_TrackEndString;
                        }
                    }
            
                    else if(!strcmp(child.first_child().value(), "delete") ){
                        if( pExisting )
                            g_pRouteMan->DeleteTrack( pExisting );
                    }
            
                    else if(!strcmp(child.first_child().value(), "add") ){
                        if( !pExisting )
                            ::InsertTrack( pTrack );
                    }
                
                    else
                        delete pTrack;
                }
            }
            
            else
                if( !strcmp(object.name(), "rte") ) {
                    Route *pRoute = GPXLoadRoute1( object, true, false, false, 0, true );
                    
                    if(pRoute && g_pRouteMan) {
                        pugi::xml_node xchild = object.child("extensions");
                        pugi::xml_node child = xchild.child("opencpn:action");

                        if(!strcmp(child.first_child().value(), "add") ){
                            ::UpdateRouteA( pRoute );
                        }                    
                    
                        else if(!strcmp(child.first_child().value(), "update") ){
                            ::UpdateRouteA( pRoute );
                        }
                    
                        else if(!strcmp(child.first_child().value(), "delete") ){
                            Route *pExisting = RouteExists( pRoute->m_GUID );
                            if(pExisting){
                                pConfig->m_bSkipChangeSetUpdate = true;
                                g_pRouteMan->DeleteRoute( pExisting );
                                pConfig->m_bSkipChangeSetUpdate = false;
                            }
                        }
                    
                        else
                            delete pRoute;
                    }
                }
            else
                if( !strcmp(object.name(), "tkpt") ) {
                    TrackPoint *pWp = ::GPXLoadTrackPoint1( object );
                    
                    if(pWp && pWayPointMan) {
//                        RoutePoint *pExisting = WaypointExists( pWp->GetName(), pWp->m_lat, pWp->m_lon );
                        
                        pugi::xml_node xchild = object.child("extensions");
                        pugi::xml_node child = xchild.child("opencpn:action");

                        pugi::xml_node guid_child = xchild.child("opencpn:track_GUID");
                        wxString track_GUID(guid_child.first_child().value(), wxConvUTF8);

                        Track *pExistingTrack = (Track *)RouteExists( track_GUID );
                        
                        if(!strcmp(child.first_child().value(), "add") ){
                            if( pExistingTrack ) {
                                pExistingTrack->AddPoint( pWp );
                                pWp->m_GPXTrkSegNo = pExistingTrack->GetCurrentTrackSeg() + 1;
                            }                                
                        }                    
                        
                        else
                            delete pWp;
                    }
        }
    
        object = object.next_sibling();
                
    }

    return true;
}
