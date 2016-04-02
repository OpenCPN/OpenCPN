/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose   Squiddio plugin
 * This work is based on the NavObjectCollection.cpp, file which is:
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

#include "NavObjectCollection.h"
#include "Poi.h"
#include "PoiMan.h"

extern PoiMan *pPoiMan;

NavObjectCollection1::NavObjectCollection1()
: pugi::xml_document()
{
}

NavObjectCollection1::~NavObjectCollection1()
{
}

Poi * GPXLoadWaypoint1( pugi::xml_node &wpt_node,
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

    wxString SymString = def_symbol_name;       // default icon
    wxString NameString;
    wxString DescString;
    wxString TypeString;
    wxString GuidString = GUID;         // default
    wxString TimeString;
    wxDateTime dt;
    Poi *pWP = NULL;
    
    HyperlinkList *linklist = NULL;

    double rlat = wpt_node.attribute( "lat" ).as_double();
    double rlon = wpt_node.attribute( "lon" ).as_double();;

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
            }// for 
        } //extensions
    }   // for

    // Create waypoint

    if( b_layer ) {
        if( GuidString.IsEmpty() )
            GuidString = _("LayGUID");
    }

    pWP = new Poi( rlat, rlon, SymString, NameString, GuidString, false ); // do not add to global WP list yet...
    pWP->m_MarkDescription = DescString;
    pWP->m_bIsolatedMark = bshared;      // This is an isolated mark
    

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
        delete pWP->m_HyperlinkList;                    // created in Poi ctor
        pWP->m_HyperlinkList = linklist;
    }

    return ( pWP );
}

bool GPXCreateWpt( pugi::xml_node node, Poi *pr, unsigned int flags )
{
    wxString s;
    pugi::xml_node child;
    pugi::xml_attribute attr;
    
    s.Printf(_("%.9f"), pr->m_lat);
    node.append_attribute("lat") = s.mb_str();
    s.Printf(_("%.9f"), pr->m_lon);
    node.append_attribute("lon") = s.mb_str();
 
    if(flags & OUT_TIME) {
            child = node.append_child("time");
            if( pr->m_timestring.Len() )
                child.append_child(pugi::node_pcdata).set_value(pr->m_timestring.mb_str());
            else {
                wxString t = pr->GetCreateTime().FormatISODate().Append(_("T")).Append(pr->GetCreateTime().FormatISOTime()).Append(_("Z"));
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
        if (!pr->m_IconName.IsEmpty()) {
            child.append_child(pugi::node_pcdata).set_value(pr->m_IconName.mb_str());
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
            || (flags & OUT_AUTO_NAME) ) {
        
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
    }
    
    return true;
}
                       
bool NavObjectCollection1::CreateNavObjGPXPoints( void )
{
    
    //    Iterate over the Routepoint list, creating Nodes for
    //    Routepoints that are not in any Route
    //    as indicated by m_bIsolatedMark == false
    
    wxPoiListNode *node = pPoiMan->GetWaypointList()->GetFirst();
    
    Poi *pr;
    
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

bool NavObjectCollection1::CreateAllGPXObjects()
{
    SetRootGPXNode();
    CreateNavObjGPXPoints();
    return true;
}

bool NavObjectCollection1::AddGPXWaypoint(Poi *pWP )
{
    SetRootGPXNode();
    GPXCreateWpt(m_gpx_root.append_child("wpt"), pWP, OPT_WPT);
    return true;
}

bool NavObjectCollection1::AddGPXPointsList( PoiList *pRoutePoints )
{
    SetRootGPXNode();
    
    wxPoiListNode* pRoutePointNode = pRoutePoints->GetFirst();
    while (pRoutePointNode) {
        Poi* pRP = pRoutePointNode->GetData();
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
        

bool NavObjectCollection1::SaveFile( const wxString filename )
{
    save_file(filename.fn_str(), "  ");
    return true;
}

bool NavObjectCollection1::LoadAllGPXObjects()
{
    pugi::xml_node objects = this->child("gpx");
    
    for (pugi::xml_node object = objects.first_child(); object; object = object.next_sibling())
    {
        if( !strcmp(object.name(), "wpt") ) {
            Poi *pWp = ::GPXLoadWaypoint1( object, _("circle"), _(""), false, false, false, 0 );
            pWp->m_bIsolatedMark = true;      // This is an isolated mark
            
            if(pWp) {
                Poi *pExisting = WaypointExists( pWp->GetName(), pWp->m_lat, pWp->m_lon );
                if( !pExisting ) {
                    if( NULL != pPoiMan )
                        pPoiMan->AddRoutePoint( pWp );
//                     pSelectSq->AddSelectableRoutePoint( pWp->m_lat, pWp->m_lon, pWp );
                }
                else
                    delete pWp;
            }
        }
    }
    
    return true;
}

int NavObjectCollection1::LoadAllGPXObjectsAsLayer(int layer_id, bool b_layerviz)
{
    if(!pPoiMan)
        return 0;

    int n_obj = 0;
    pugi::xml_node objects = this->child("gpx");
    
    for (pugi::xml_node object = objects.first_child(); object; object = object.next_sibling())
    {
        if( !strcmp(object.name(), "wpt") ) {
            Poi *pWp = ::GPXLoadWaypoint1( object, _("circle"), _(""), true, true, b_layerviz, layer_id );
            pWp->m_bIsolatedMark = true;      // This is an isolated mark
            
            if(pWp) {
                pPoiMan->AddRoutePoint( pWp );
                n_obj++;
            }
            else
                delete pWp;
        }
    }
    
    return n_obj;
}

//-------------------------------------------------------------------------
//
//          Static GPX Support Routines (from navutil.cpp)
//
//-------------------------------------------------------------------------
Poi *NavObjectCollection1::WaypointExists( const wxString& name, double lat, double lon )
{
    Poi *pret = NULL;
//    if( g_bIsNewLayer ) return NULL;
    wxPoiListNode *node = pPoiMan->GetWaypointList()->GetFirst();
    bool Exists = false;
    while( node ) {
        Poi *pr = node->GetData();

//        if( pr->m_bIsInLayer ) return NULL;

        if( name == pr->GetName() ) {
            if( fabs( lat - pr->m_lat ) < 1.e-6 && fabs( lon - pr->m_lon ) < 1.e-6 ) {
                Exists = true;
                pret = pr;
                break;
            }
        }
        node = node->GetNext();
    }

    return pret;
}


