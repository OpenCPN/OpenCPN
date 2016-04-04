/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Read and write KML Format (http://en.wikipedia.org/wiki/Keyhole_Markup_Language)
 * Author:   Jesper Weissglas
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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
 *
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>

#include <wx/file.h>
#include <wx/datetime.h>
#include <wx/clipbrd.h>

#include "ocpn_types.h"
#include "navutil.h"
#include "tinyxml.h"
#include "kml.h"

extern MyFrame *gFrame;
extern double gLat;
extern double gLon;

int Kml::seqCounter = 0;
bool Kml::insertQtVlmExtendedData = false;

int Kml::ParseCoordinates( TiXmlNode* node, dPointList& points ) {
    TiXmlElement* e = node->FirstChildElement( "coordinates" );
    if( ! e ) {
        wxString msg( _T("KML Parser found no <coordinates> for the element: ") );
        msg << wxString( node->ToElement()->Value(), wxConvUTF8 );
        wxLogMessage( msg );
        return 0;
    }

    // Parse "long,lat,z" format.

    dPoint point;

    std::stringstream ss( e->GetText() );
    std::string txtCoord;

	while(1) {
        if( ! std::getline( ss, txtCoord, ',' ) ) break;;
        if( txtCoord.length() == 0 ) break;

        point.x = atof( txtCoord.c_str() );
        std::getline( ss, txtCoord, ',' );
        point.y = atof( txtCoord.c_str() );
        std::getline( ss, txtCoord, ' ' );
        point.z = atof( txtCoord.c_str() );

        points.push_back( point );
    }
    return points.size();
}

KmlPastebufferType Kml::ParseTrack( TiXmlNode* node, wxString& name ) {
    parsedTrack = new Track();
    parsedTrack->m_TrackNameString = name;

    if( 0 == strncmp( node->ToElement()->Value(), "LineString", 10 ) ) {
        dPointList coordinates;
        if( ParseCoordinates( node, coordinates ) > 2 ) {
            TrackPoint* trackpoint = NULL;
            TrackPoint* prevPoint = NULL;

            for( unsigned int i=0; i<coordinates.size(); i++ ) {
                trackpoint = new TrackPoint(coordinates[i].y, coordinates[i].x);
                parsedTrack->AddPoint( trackpoint );
            }
        }
        return KML_PASTE_TRACK;
    }

    if( 0 == strncmp( node->ToElement()->Value(), "gx:Track", 8 ) ) {
        TrackPoint* trackpoint = NULL;
        TiXmlElement* point = node->FirstChildElement( "gx:coord" );
        int pointCounter = 0;

        for( ; point; point=point->NextSiblingElement( "gx:coord" ) ) {
            double lat, lon;
            std::stringstream ss( point->GetText() );
            std::string txtCoord;
            std::getline( ss, txtCoord, ' ' );
            lon = atof( txtCoord.c_str() );
            std::getline( ss, txtCoord, ' ' );
            lat = atof( txtCoord.c_str() );

            parsedTrack->AddPoint( new TrackPoint(lat, lon) );
            pointCounter++;
        }

        TiXmlElement* when = node->FirstChildElement( "when" );

        wxDateTime whenTime;

        int i = 0;
        for( ; when; when=when->NextSiblingElement( "when" ) ) {
            trackpoint = parsedTrack->GetPoint(i);
            if( ! trackpoint ) continue;
            whenTime.ParseFormat( wxString( when->GetText(), wxConvUTF8 ), _T("%Y-%m-%dT%H:%M:%SZ") );
            trackpoint->SetCreateTime(whenTime);
            i++;
        }

        return KML_PASTE_TRACK;
    }
    return KML_PASTE_INVALID;
}

KmlPastebufferType Kml::ParseOnePlacemarkPoint( TiXmlNode* node, wxString& name ) {
    double newLat = 0., newLon = 0.;
    dPointList coordinates;

    if( ParseCoordinates( node->ToElement(), coordinates ) ) {
        newLat = coordinates[0].y;
        newLon = coordinates[0].x;
    }

    if( newLat == 0.0 && newLon == 0.0 ) {
        wxString msg( _T("KML Parser failed to convert <Point> coordinates.") );
        wxLogMessage( msg );
        return KML_PASTE_INVALID;
    }
    wxString pointName = wxEmptyString;
	TiXmlElement* e = node->Parent()->FirstChild( "name" )->ToElement();
    if( e ) pointName = wxString( e->GetText(), wxConvUTF8 );

    wxString pointDescr = wxEmptyString;
    e = node->Parent()->FirstChildElement( "description" );

    // If the <description> is an XML element we must convert it to text,
    // otherwise it gets lost.
    if( e ) {
        TiXmlNode* n = e->FirstChild();
        if( n ) switch( n->Type() ){
            case TiXmlNode::TINYXML_TEXT:
                pointDescr = wxString( e->GetText(), wxConvUTF8 );
                break;
            case TiXmlNode::TINYXML_ELEMENT:
                TiXmlPrinter printer;
                printer.SetIndent( "\t" );
                n->Accept( &printer );
                pointDescr = wxString( printer.CStr(), wxConvUTF8 );
                break;
        }
    }

    // Extended data will override description.
    TiXmlNode* n = node->Parent()->FirstChild( "ExtendedData" );
    if( n ) {
        TiXmlPrinter printer;
        printer.SetIndent( "\t" );
        n->Accept( &printer );
        pointDescr = wxString( printer.CStr(), wxConvUTF8 );
    }

    // XXX leak ?
    parsedRoutePoint = new RoutePoint();
    parsedRoutePoint->m_lat = newLat;
    parsedRoutePoint->m_lon = newLon;
    parsedRoutePoint->m_bIsolatedMark = true;
    parsedRoutePoint->m_bPtIsSelected = false;
    parsedRoutePoint->m_MarkDescription = pointDescr;
    parsedRoutePoint->SetName( pointName );

    return KML_PASTE_WAYPOINT;
}

KmlPastebufferType Kml::ParsePasteBuffer() {
    if( !wxTheClipboard->IsOpened() )
        if( ! wxTheClipboard->Open() ) return KML_PASTE_INVALID;

    wxTextDataObject data;
    wxTheClipboard->GetData( data );
    kmlText = data.GetText();
    wxTheClipboard->Close();

    if( kmlText.Find( _T("<kml") ) == wxNOT_FOUND ) return KML_PASTE_INVALID;

    TiXmlDocument doc;
    if( ! doc.Parse( kmlText.mb_str( wxConvUTF8 ), 0, TIXML_ENCODING_UTF8 ) ) {
		wxLogError( wxString( doc.ErrorDesc(), wxConvUTF8 ) );
		return KML_PASTE_INVALID;
	}
    if( 0 != strncmp( doc.RootElement()->Value(), "kml", 3 ) ) return KML_PASTE_INVALID;

    TiXmlHandle docHandle( doc.RootElement() );

	// We may or may not have a <document> depending on what the user copied.
    TiXmlElement* placemark = docHandle.FirstChild( "Document" ).FirstChild( "Placemark" ).ToElement();
    if( ! placemark ) {
        placemark = docHandle.FirstChild( "Placemark" ).ToElement();
	}
    if( ! placemark ) {
        wxString msg( _T("KML Parser found no <Placemark> tag in the KML.") );
        wxLogMessage( msg );
        return KML_PASTE_INVALID;
    }

    int pointCounter = 0;
    wxString name;
    for( ; placemark; placemark=placemark->NextSiblingElement() ) {
        TiXmlElement* e = placemark->FirstChildElement( "name" );
        if( e ) name = wxString( e->GetText(),wxConvUTF8 );
        pointCounter++;
    }

    if( pointCounter == 1 ) {

        // Is it a single waypoint?
        TiXmlNode* element = docHandle.FirstChild( "Document" ).FirstChild( "Placemark" ).FirstChild( "Point" ).ToNode();
        if( ! element ) element = docHandle.FirstChild( "Placemark" ).FirstChild( "Point" ).ToNode();
        if( element ) return ParseOnePlacemarkPoint( element, name );

        // Is it a dumb <LineString> track?
        element = docHandle.FirstChild( "Document" ).FirstChild( "Placemark" ).FirstChild( "LineString" ).ToNode();
        if( ! element ) element = docHandle.FirstChild( "Placemark" ).FirstChild( "LineString" ).ToNode();
        if( element ) return ParseTrack( element, name );

        // Is it a smart extended <gx:track> track?
        element = docHandle.FirstChild( "Document" ).FirstChild( "Placemark" ).FirstChild( "gx:Track" ).ToNode();
        if( ! element ) element = docHandle.FirstChild( "Placemark" ).FirstChild( "gx:Track" ).ToNode();
        if( element ) return ParseTrack( element, name );

        wxString msg( _T("KML Parser found a single <Placemark> in the KML, but no useable data in it.") );
        wxLogMessage( msg );
        return KML_PASTE_INVALID;
    }

    // Here we go with a full route.

    parsedRoute = new Route();
    bool foundPoints = false;
    bool foundTrack = false;
    TiXmlElement* element = docHandle.FirstChild( "Document" ).FirstChild( "name" ).ToElement();
    if( element )
        parsedRoute->m_RouteNameString = wxString( element->GetText(), wxConvUTF8 );

    RoutePoint* rp = NULL;
    placemark = docHandle.FirstChild( "Document" ).FirstChild( "Placemark" ).ToElement();
    for( ; placemark; placemark=placemark->NextSiblingElement() ) {

        TiXmlNode* n = placemark->FirstChild( "Point" );
        if( n ) {
            if( ParseOnePlacemarkPoint( n->ToElement(), name ) == KML_PASTE_WAYPOINT ) {
                parsedRoute->AddPoint( new RoutePoint( parsedRoutePoint ) );
                delete parsedRoutePoint;
                parsedRoutePoint = 0;
                foundPoints = true;
            }
        }

        n = placemark->FirstChild( "LineString" );
        if( n ) {
            ParseTrack( n->ToElement(), name );
            foundTrack = true;
        }
        n = placemark->FirstChild( "gx:Track" );
        if( n ) {
            ParseTrack( n->ToElement(), name );
            foundTrack = true;
        }
    }

    if( foundPoints && parsedRoute->GetnPoints() < 2 ) {
        wxString msg( _T("KML Parser did not find enough <Point>s to make a route.") );
        wxLogMessage( msg );
        foundPoints = false;
    }

    if( foundPoints && ! foundTrack ) return KML_PASTE_ROUTE;
    if( foundPoints && foundTrack ) return KML_PASTE_ROUTE_TRACK;
    if( ! foundPoints && foundTrack ) return KML_PASTE_TRACK;
    return KML_PASTE_INVALID;
}

TiXmlElement* Kml::StandardHead( TiXmlDocument& xmlDoc, wxString name ) {
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
    xmlDoc.LinkEndChild( decl );

    TiXmlElement* kml = new TiXmlElement( "kml" );
    kml->SetAttribute( "xmlns:atom", "http://www.w3.org/2005/Atom" );
    kml->SetAttribute( "xmlns", "http://www.opengis.net/kml/2.2" );
    kml->SetAttribute( "xmlns:gx", "http://www.google.com/kml/ext/2.2" );
    kml->SetAttribute( "xmlns:kml", "http://www.opengis.net/kml/2.2" );

    if( insertQtVlmExtendedData )
        kml->SetAttribute( "xmlns:vlm", "http://virtual-loup-de-mer.org" );

    xmlDoc.LinkEndChild( kml );

    TiXmlElement* document = new TiXmlElement( "Document" );
    kml->LinkEndChild( document );
    TiXmlElement* docName = new TiXmlElement( "name" );
    document->LinkEndChild( docName );
    TiXmlText* docNameVal = new TiXmlText( name.mb_str( wxConvUTF8 ) );
    docName->LinkEndChild( docNameVal );
    return document;
}

std::string Kml::PointPlacemark(  TiXmlElement* document, RoutePoint* routepoint ) {
    TiXmlElement* pmPoint = new TiXmlElement( "Placemark" );
    document->LinkEndChild( pmPoint );
    TiXmlElement* pmPointName = new TiXmlElement( "name" );
    pmPoint->LinkEndChild( pmPointName );
    TiXmlText* pmPointNameVal = new TiXmlText( routepoint->GetName().mb_str( wxConvUTF8 ) );
    pmPointName->LinkEndChild( pmPointNameVal );

    TiXmlElement* pointDescr = new TiXmlElement( "description" );
    pmPoint->LinkEndChild( pointDescr );

    bool descrIsPlainText = true;
    wxCharBuffer descrString = routepoint->m_MarkDescription.mb_str( wxConvUTF8 );

    if( insertQtVlmExtendedData ) {
        // Does the RoutePoint description parse as XML with an <ExtendedData> root tag?
        TiXmlDocument descrDoc;
        TiXmlElement* extendedData;
        if( descrDoc.Parse( descrString, 0, TIXML_ENCODING_UTF8 ) ) {
            if( 0 == strncmp( descrDoc.RootElement()->Value(), "ExtendedData", 12 ) ) {
                descrIsPlainText = false;
                extendedData = descrDoc.RootElement();
                TiXmlHandle docHandle( &descrDoc );
                TiXmlElement* seq = docHandle.FirstChild( "ExtendedData" ).FirstChild( "vlm:sequence" ).ToElement();
                if( ! seq ) {
                    seq = new TiXmlElement( "vlm:sequence" );
                    TiXmlText* snVal = new TiXmlText(
                            wxString::Format( _T("%04d"), seqCounter ).mb_str( wxConvUTF8 ) );
                    seq->LinkEndChild( snVal );
                    descrDoc.RootElement()->LinkEndChild( seq );
                }
                pmPoint->LinkEndChild( descrDoc.RootElement()->Clone() );
            }
        }
        if( descrIsPlainText ) {
            // We want Sequence names but there was some non-parsing stuff in the description.
            // Push that into a sub-tag of an XML formatted description.
            extendedData = new TiXmlElement( "ExtendedData" );
            pmPoint->LinkEndChild( extendedData );
            TiXmlElement* seq = new TiXmlElement( "vlm:sequence" );
            extendedData->LinkEndChild( seq );
            TiXmlText* snVal = new TiXmlText(
                    wxString::Format( _T("%04d"), seqCounter ).mb_str( wxConvUTF8 ) );
            seq->LinkEndChild( snVal );

            if( routepoint->m_MarkDescription.Length() ) {
                TiXmlElement* data = new TiXmlElement( "Data" );
                data->SetAttribute( "name", "Description" );
                extendedData->LinkEndChild( data );

                TiXmlElement* value = new TiXmlElement( "value" );
                data->LinkEndChild( value );
                TiXmlText* txtVal = new TiXmlText( descrString );
                value->LinkEndChild( txtVal );
            }
        }
        if( extendedData && seqCounter == 0 ) {
            const wxCharBuffer ownshipPos = wxString::Format( _T("%f %f"), gLon, gLat ).mb_str( wxConvUTF8 );
            TiXmlHandle h( extendedData );
            TiXmlElement* route = h.FirstChild( "vlm:route" ).ToElement();
            TiXmlElement* ownship = h.FirstChild( "vlm:route" ).FirstChild( "ownship" ).ToElement();
            if( route ) {
                if( ownship ) {
                    TiXmlText* owns = ownship->FirstChild()->ToText();
                    if( owns ) {
                        owns->SetValue( ownshipPos );
                    } else {
                        owns = new TiXmlText( ownshipPos );
                        ownship->LinkEndChild( owns );
                    }
                } else {
                    ownship = new TiXmlElement( "ownship" );
                    route->LinkEndChild( ownship );
                    TiXmlText* owns = new TiXmlText( ownshipPos );
                    ownship->LinkEndChild( owns );
                }
            } else {
                route = new TiXmlElement( "vlm:route" );
                extendedData->LinkEndChild( route );
                ownship = new TiXmlElement( "ownship" );
                route->LinkEndChild( ownship );
                TiXmlText* owns = new TiXmlText( ownshipPos );
                ownship->LinkEndChild( owns );
            }
        }
    }

    else {
        // Add description as dumb text.
        TiXmlText* pointDescrVal = new TiXmlText( descrString );
        pointDescr->LinkEndChild( pointDescrVal );
    }

    TiXmlElement* point = new TiXmlElement( "Point" );
    pmPoint->LinkEndChild( point );

    TiXmlElement* pointCoord = new TiXmlElement( "coordinates" );
    point->LinkEndChild( pointCoord );

    std::stringstream pointCoordStr;
    pointCoordStr << routepoint->m_lon << "," << routepoint->m_lat << ",0. ";

    TiXmlText* pointText = new TiXmlText( pointCoordStr.str() );
    pointCoord->LinkEndChild( pointText );

    return pointCoordStr.str();
}

wxString Kml::MakeKmlFromRoute( Route* route, bool insertSeq ) {
    insertQtVlmExtendedData = insertSeq;
    seqCounter = 0;
    TiXmlDocument xmlDoc;
    wxString name = _("OpenCPN Route");
    if( route->m_RouteNameString.Length() ) name = route->m_RouteNameString;
    TiXmlElement* document = StandardHead( xmlDoc, name );

    std::stringstream lineStringCoords;

    RoutePointList *pointList = route->pRoutePointList;
    wxRoutePointListNode *pointnode = pointList->GetFirst();
    RoutePoint *routepoint;

    while( pointnode ) {
        routepoint = pointnode->GetData();

        lineStringCoords << PointPlacemark( document, routepoint );
        seqCounter++;
        pointnode = pointnode->GetNext();
    }

    TiXmlElement* pmPath = new TiXmlElement( "Placemark" );
    document->LinkEndChild( pmPath );

    TiXmlElement* pmName = new TiXmlElement( "name" );
    pmPath->LinkEndChild( pmName );
    TiXmlText* pmNameVal = new TiXmlText( "Path" );
    pmName->LinkEndChild( pmNameVal );

    TiXmlElement* linestring = new TiXmlElement( "LineString" );
    pmPath->LinkEndChild( linestring );

    TiXmlElement* coordinates = new TiXmlElement( "coordinates" );
    linestring->LinkEndChild( coordinates );

    TiXmlText* text = new TiXmlText( lineStringCoords.str() );
    coordinates->LinkEndChild( text );

    TiXmlPrinter printer;
    printer.SetIndent( "  " );
    xmlDoc.Accept( &printer );

    return wxString( printer.CStr(), wxConvUTF8 );
}


wxString Kml::MakeKmlFromTrack( Track* track ) {
    TiXmlDocument xmlDoc;
    wxString name = _("OpenCPN Track");
    if( track->m_TrackNameString.Length() ) name = track->m_TrackNameString;
    TiXmlElement* document = StandardHead( xmlDoc, name );

    TiXmlElement* pmTrack = new TiXmlElement( "Placemark" );
    document->LinkEndChild( pmTrack );

    TiXmlElement* pmName = new TiXmlElement( "name" );
    pmTrack->LinkEndChild( pmName );
    TiXmlText* pmNameVal = new TiXmlText( track->m_TrackNameString.mb_str( wxConvUTF8 ) );
    pmName->LinkEndChild( pmNameVal );

    TiXmlElement* gxTrack = new TiXmlElement( "gx:Track" );
    pmTrack->LinkEndChild( gxTrack );

    std::stringstream lineStringCoords;

    for(int i=0; i<track->GetnPoints(); i++) {
        TrackPoint *trackpoint = track->GetPoint(i);

        TiXmlElement* when = new TiXmlElement( "when" );
        gxTrack->LinkEndChild( when );

        wxDateTime whenTime( trackpoint->GetCreateTime() );
        TiXmlText* whenVal = new TiXmlText( whenTime.Format( _T("%Y-%m-%dT%H:%M:%SZ") ).mb_str( wxConvUTF8 ) );
        when->LinkEndChild( whenVal );
    }

    for(int i=0; i<track->GetnPoints(); i++) {
        TrackPoint *trackpoint = track->GetPoint(i);

        TiXmlElement* coord = new TiXmlElement( "gx:coord" );
        gxTrack->LinkEndChild( coord );
        wxString coordStr = wxString::Format( _T("%f %f 0.0"), trackpoint->m_lon, trackpoint->m_lat );
        TiXmlText* coordVal = new TiXmlText( coordStr.mb_str( wxConvUTF8 ) );
        coord->LinkEndChild( coordVal );
    }


    TiXmlPrinter printer;
    printer.SetIndent( "  " );
    xmlDoc.Accept( &printer );

    return wxString( printer.CStr(), wxConvUTF8 );
}

wxString Kml::MakeKmlFromWaypoint( RoutePoint* routepoint ) {
    TiXmlDocument xmlDoc;
    wxString name = _("OpenCPN Waypoint");
    if( routepoint->GetName().Length() ) name = routepoint->GetName();
    TiXmlElement* document = StandardHead( xmlDoc, name );

    insertQtVlmExtendedData = false;
    PointPlacemark( document, routepoint );

    TiXmlPrinter printer;
    printer.SetIndent( "  " );
    xmlDoc.Accept( &printer );

    return wxString( printer.CStr(), wxConvUTF8 );
}

void Kml::CopyRouteToClipboard( Route* route ) {
    KmlFormatDialog* formatDlg = new KmlFormatDialog( gFrame );
    int format = formatDlg->ShowModal();

    if( format != wxID_CANCEL ) {
        format = formatDlg->GetSelectedFormat();
        bool extradata = (format == KML_COPY_EXTRADATA);

        ::wxBeginBusyCursor();
        if( wxTheClipboard->Open() ) {
            wxTextDataObject* data = new wxTextDataObject;
            data->SetText( MakeKmlFromRoute( route, extradata ) );
            wxTheClipboard->SetData( data );
        }
        ::wxEndBusyCursor();
    }
        delete formatDlg;
}

void Kml::CopyTrackToClipboard( Track* track ) {
    ::wxBeginBusyCursor();
    if( wxTheClipboard->Open() ) {
        wxTextDataObject* data = new wxTextDataObject;
        data->SetText( MakeKmlFromTrack( track ) );
        wxTheClipboard->SetData( data );
    }
    ::wxEndBusyCursor();
}

void Kml::CopyWaypointToClipboard( RoutePoint* rp ) {
    if( wxTheClipboard->Open() ) {
        wxTextDataObject* data = new wxTextDataObject;
        data->SetText( MakeKmlFromWaypoint( rp ) );
        wxTheClipboard->SetData( data );
    }
}

Kml::Kml() {
    parsedRoute = NULL;
    parsedTrack = NULL;
    parsedRoutePoint = NULL;
}

Kml::~Kml() {
    if( parsedTrack ) {
        for( int i=1; i<=parsedTrack->GetnPoints(); i++ ) {
            if( parsedTrack->GetPoint(i) ) delete parsedTrack->GetPoint(i);
        }
        delete parsedTrack;
    }
    if( parsedRoute ) {
        for( int i=1; i<=parsedRoute->GetnPoints(); i++ ) {
            if( parsedRoute->GetPoint(i) ) delete parsedRoute->GetPoint(i);
        }
        delete parsedRoute;
    }
    delete parsedRoutePoint;
}

//----------------------------------------------------------------------------------

KmlFormatDialog::KmlFormatDialog( wxWindow* parent )
       : wxDialog( parent, wxID_ANY, _("Choose Format for Copy"), wxDefaultPosition, wxSize(250, 230) )
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    topSizer->Add( sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    choices.push_back( new wxRadioButton( this, KML_COPY_STANDARD, _("KML Standard (Google Earth and others)"),
            wxDefaultPosition, wxDefaultSize, wxRB_GROUP ) );

    choices.push_back( new wxRadioButton( this, KML_COPY_EXTRADATA, _("KML with extended waypoint data (QtVlm)"),
            wxDefaultPosition) );

    wxStdDialogButtonSizer* buttonSizer = CreateStdDialogButtonSizer( wxOK | wxCANCEL );

    sizer->Add( choices[0], 0, wxEXPAND | wxALL, 5 );
    sizer->Add( choices[1], 0, wxEXPAND | wxALL, 5 );
    sizer->Add( buttonSizer, 0, wxEXPAND | wxTOP, 5 );

    topSizer->SetSizeHints(this);
    SetSizer( topSizer );
}

int KmlFormatDialog::GetSelectedFormat() {
    for( unsigned int i=0; i<choices.size(); i++ ) {
        if( choices[i]->GetValue() ) return choices[i]->GetId();
    }
    return 0;
}

