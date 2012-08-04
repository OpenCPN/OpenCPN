/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GPX utility classes
 * Author:   David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                                      *
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
*/

#include <wx/wx.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include "gpxdocument.h"

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif


#include <wx/listimpl.cpp>

WX_DEFINE_LIST(ListOfGpxLinks);
WX_DEFINE_LIST(ListOfGpxWpts);
WX_DEFINE_LIST(ListOfGpxRoutes);
WX_DEFINE_LIST(ListOfGpxTrksegs);
WX_DEFINE_LIST(ListOfGpxTracks);

GpxDocument::GpxDocument(const wxString &filename)
{
      LoadFile(filename);
      //FIXME: we should probably validate if the file is GPX DTD compliant and die if not... BUT we would need a dependency to some validating parser.
}

bool GpxDocument::LoadFile(const wxString &filename)
{
      SeedRandom();
      wxRegEx re; //We try to fix popularily broken GPX files. Unencoded '&' is an illegal character in XML, but seems higly popular amongst users (and perhaps even vendors not aware of what XML is...)
      //The same as above is true for '<' but it would be harder to solve - it's illegal just inside a value, not when it starts a tag
      int re_compile_flags = wxRE_ICASE;
#ifdef wxHAS_REGEX_ADVANCED
      re_compile_flags |= wxRE_ADVANCED;
#endif
      bool b = re.Compile(wxT("&(?!amp;|lt;|gt;|apos;|quot;|#[0-9]{1,};|#x[0-f]{1,};)"), re_compile_flags); //Should find all the non-XML entites to be encoded as text
      wxFFile file(filename);
      wxString s;
      if(file.IsOpened()) {
            file.ReadAll(&s, wxConvUTF8);

            //Fallback for not-well formed (non-UTF8) GPX files
            //the "garbage" characters are lost, but the important part of the information should survive...
            if (s == wxEmptyString)
            {
                  file.Seek(0);
                  file.ReadAll(&s, wxConvISO8859_1);
                  wxLogMessage(wxString::Format(wxT("File %s seems not to be well-formed UTF-8 XML, used fallback ASCII format conversion - some text information might have not been imported."), filename.c_str()));
            }

            file.Close();
      }
      if(b)
      {
            //CDATA handling makes this task way too complex for regular expressions to handle,
            // so we do nothing and just let the possible damage happen...
            if (!s.Contains(wxT("![CDATA[")))
            {
                  int cnt = re.ReplaceAll(&s, wxT("&amp;"));
                  if (cnt > 0)
                        wxLogMessage(wxString::Format(wxT("File %s seems broken, %i occurences of '&' were replaced with '&amp;' to try to fix it."), filename.c_str(), cnt));
            }
      }
      wxFFile gpxfile;
      wxString gpxfilename = wxFileName::CreateTempFileName(wxT("gpx"), &gpxfile);
      gpxfile.Write(s);
      gpxfile.Close();
      bool res = TiXmlDocument::LoadFile((const char*)gpxfilename.mb_str());

	  if( ! res ) {
		  wxString msg = _T("Failed to load ");
		  msg << filename;
		  msg << _T(": ");
		  msg << wxString( TiXmlDocument::ErrorDesc(), wxConvUTF8 );
 		  wxLogMessage( msg );
	  }
      ::wxRemoveFile(gpxfilename);
      return res;
}

bool GpxDocument::SaveFile(const wxString &filename)
{
      return TiXmlDocument::SaveFile((const char*)filename.mb_str());
}

GpxDocument::GpxDocument()
{
      PopulateEmptyDocument(_T("OpenCPN"));
      AddCustomNamespace(_T("xmlns:opencpn"), _T("http://www.opencpn.org"));
      SeedRandom();
}

void GpxDocument::SeedRandom()
{
      /* Fill with random. Miliseconds hopefully good enough for our usage, reading /dev/random would be much better on linux and system guid function on Windows as well */
      wxDateTime x = wxDateTime::UNow();
      long seed = x.GetMillisecond();
      seed *= x.GetTicks();
      srand(seed);
}

GpxDocument::~GpxDocument()
{
}

// RFC4122 version 4 compliant random UUIDs generator.
wxString GpxDocument::GetUUID(void)
{
      wxString str;
      struct {
      int time_low;
      int time_mid;
      int time_hi_and_version;
      int clock_seq_hi_and_rsv;
      int clock_seq_low;
      int node_hi;
      int node_low;
      } uuid;

      uuid.time_low = GetRandomNumber(0, 2147483647);//FIXME: the max should be set to something like MAXINT32, but it doesn't compile un gcc...
      uuid.time_mid = GetRandomNumber(0, 65535);
      uuid.time_hi_and_version = GetRandomNumber(0, 65535);
      uuid.clock_seq_hi_and_rsv = GetRandomNumber(0, 255);
      uuid.clock_seq_low = GetRandomNumber(0, 255);
      uuid.node_hi = GetRandomNumber(0, 65535);
      uuid.node_low = GetRandomNumber(0, 2147483647);

      /* Set the two most significant bits (bits 6 and 7) of the
      * clock_seq_hi_and_rsv to zero and one, respectively. */
      uuid.clock_seq_hi_and_rsv = (uuid.clock_seq_hi_and_rsv & 0x3F) | 0x80;

      /* Set the four most significant bits (bits 12 through 15) of the
      * time_hi_and_version field to 4 */
      uuid.time_hi_and_version = (uuid.time_hi_and_version & 0x0fff) | 0x4000;

      str.Printf(_T("%08x-%04x-%04x-%02x%02x-%04x%08x"),
      uuid.time_low,
      uuid.time_mid,
      uuid.time_hi_and_version,
      uuid.clock_seq_hi_and_rsv,
      uuid.clock_seq_low,
      uuid.node_hi,
      uuid.node_low);

      return str;
}

int GpxDocument::GetRandomNumber(int range_min, int range_max)
{
      long u = (long)wxRound(((double)rand() / ((double)(RAND_MAX) + 1) * (range_max - range_min)) + range_min);
      return (int)u;
}

void GpxDocument::AddCustomNamespace(const wxString &name, const wxString &url)
{
    RootElement()->SetAttribute(name.ToUTF8(), url.ToUTF8());
}

void GpxDocument::PopulateEmptyDocument(const wxString &creator)
{
      TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", "" );
      GpxRootElement * gpx_root = new GpxRootElement(creator);

      LinkEndChild(decl);
      LinkEndChild(gpx_root);
}

GpxRootElement::GpxRootElement(const wxString &creator, GpxMetadataElement *metadata, ListOfGpxWpts *waypoints, ListOfGpxRoutes *routes, ListOfGpxTracks *tracks, GpxExtensionsElement *extensions) : TiXmlElement("gpx")
{
      my_extensions = NULL;
      my_metadata = NULL;
      first_waypoint = NULL;
      last_waypoint = NULL;
      first_route = NULL;
      last_route = NULL;
      first_track = NULL;
      last_track = NULL;

      SetAttribute ( "version", "1.1" );
      SetAttribute ( "creator", creator.ToUTF8() );
      SetAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
      SetAttribute( "xmlns", "http://www.topografix.com/GPX/1/1" );
      SetAttribute( "xmlns:gpxx", "http://www.garmin.com/xmlschemas/GpxExtensions/v3" );
      SetAttribute( "xsi:schemaLocation", "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd" );
      SetMetadata(metadata);
      if (waypoints) {
            wxListOfGpxWptsNode *waypoint = waypoints->GetFirst();
            while (waypoint)
            {
                  AddWaypoint(waypoint->GetData());
                  waypoint = waypoint->GetNext();
            }
      }
      if (routes) {
            wxListOfGpxRoutesNode *route = routes->GetFirst();
            while (route)
            {
                  AddRoute(route->GetData());
                  route = route->GetNext();
            }
      }
      if (tracks) {
            wxListOfGpxTracksNode *track = tracks->GetFirst();
            while (track)
            {
                  AddTrack(track->GetData());
                  track = track->GetNext();
            }
      }
      SetExtensions(extensions);
}

void GpxRootElement::AddWaypoint(GpxWptElement *waypoint)
{
      bool b_was_cloned = true;

      if (last_waypoint)
             last_waypoint = (GpxWptElement *)InsertAfterChild(last_waypoint, *waypoint );
      else if (my_metadata)
            last_waypoint = (GpxWptElement *)InsertAfterChild(my_metadata, *waypoint);
      else if (first_route)
            last_waypoint = (GpxWptElement *)InsertBeforeChild(first_route, *waypoint);
      else if (first_track)
            last_waypoint = (GpxWptElement *)InsertBeforeChild(first_track, *waypoint);
      else if (my_extensions)
            last_waypoint = (GpxWptElement *)InsertBeforeChild(my_extensions, *waypoint);
      else
      {
            last_waypoint = (GpxWptElement *)LinkEndChild(waypoint);
            b_was_cloned = false;
      }

      if (!first_waypoint)
            first_waypoint = last_waypoint;

      if(b_was_cloned)
      {
            waypoint->Clear();
            delete waypoint;
      }
}

void GpxRootElement::AddRoute(GpxRteElement *route)
{
      bool b_was_cloned = true;

      if (last_route)
            last_route = (GpxRteElement *)InsertAfterChild(last_route, *route);
      else if (last_waypoint)
            last_route = (GpxRteElement *)InsertAfterChild(last_waypoint, *route);
      else if (my_metadata)
            last_route = (GpxRteElement *)InsertAfterChild(my_metadata, *route);
      else if (first_track)
            last_route = (GpxRteElement *)InsertBeforeChild(first_track, *route);
      else if (my_extensions)
            last_route = (GpxRteElement *)InsertBeforeChild(my_extensions, *route);
      else
      {
            last_route = (GpxRteElement *)LinkEndChild(route);
            b_was_cloned = false;
      }

      if (!first_route)
            first_route = last_route;

      if(b_was_cloned)
      {
            route->Clear();
            delete route;
      }

}

void GpxRootElement::AddTrack(GpxTrkElement *track)
{
      bool b_was_cloned = true;

      if (last_track)
            last_track = (GpxTrkElement *)InsertAfterChild(last_track, *track);
      else if (last_route)
            last_track = (GpxTrkElement *)InsertAfterChild(last_route, *track);
      else if (last_waypoint)
            last_track = (GpxTrkElement *)InsertAfterChild(last_waypoint, *track);
      else if (my_metadata)
            last_track = (GpxTrkElement *)InsertAfterChild(my_metadata, *track);
      else if (my_extensions)
            last_track = (GpxTrkElement *)InsertBeforeChild(my_extensions, *track);
      else
      {
            last_track = (GpxTrkElement *)LinkEndChild(track);
            b_was_cloned = false;
      }

      if (!first_track)
            first_track = last_track;

      if(b_was_cloned)
      {
            track->Clear();
            delete track;
      }

}

void GpxRootElement::SetMetadata(GpxMetadataElement *metadata)
{
      bool b_was_cloned = true;

      if (!metadata)
            RemoveMetadata();
      else
      {
            if(my_metadata)
                  my_metadata = (GpxMetadataElement *)ReplaceChild(my_metadata, *metadata);
            else if (first_waypoint)
                  my_metadata = (GpxMetadataElement *)InsertBeforeChild(first_waypoint, *metadata);
            else if (first_route)
                  my_metadata = (GpxMetadataElement *)InsertBeforeChild(first_route, *metadata);
            else if (first_track)
                  my_metadata = (GpxMetadataElement *)InsertBeforeChild(first_track, *metadata);
            else if (my_extensions)
                  my_metadata = (GpxMetadataElement *)InsertBeforeChild(my_extensions, *metadata);
            else
            {
                  b_was_cloned = false;
                  my_metadata = (GpxMetadataElement *)LinkEndChild(metadata);
            }

            if(b_was_cloned)
            {
                  metadata->Clear();
                  delete metadata;
            }

      }
}

void GpxRootElement::RemoveMetadata()
{
      if(my_metadata)
            RemoveChild(my_metadata);
      delete my_metadata;
      my_metadata = NULL;
}

void GpxRootElement::SetExtensions(GpxExtensionsElement *extensions)
{
      if (!extensions)
            RemoveExtensions();
      else
      {
            if(!my_extensions)
                  my_extensions = (GpxExtensionsElement *)LinkEndChild(extensions);
            else
            {
                  my_extensions = (GpxExtensionsElement *)ReplaceChild(my_extensions, *extensions);
                  extensions->Clear();
                  delete extensions;
            }
      }
}

void GpxRootElement::RemoveExtensions()
{
      if(my_extensions)
            RemoveChild(my_extensions);
      delete my_extensions;
      my_extensions = NULL;
}

GpxExtensionsElement::GpxExtensionsElement() : TiXmlElement("extensions")
{
}

GpxxExtensionsElement::GpxxExtensionsElement(const wxString &element_name) : TiXmlElement(element_name.mb_str())
{
      if ( element_name.EndsWith (_T("RouteExtension")) )
      {
            GpxSimpleElement * g = new GpxSimpleElement(wxString(_T("gpxx:IsAutoNamed")), _T("false")); //FIXME: the namespace should be taken from element_name...
            LinkEndChild(g);
      }
}

GpxLinkElement::GpxLinkElement(const wxString &uri, const wxString &description, const wxString &mime_type) : TiXmlElement("link")
{
      SetAttribute("href", uri.ToUTF8()); //TODO: some checks?
      if(!description.IsEmpty()) {
            GpxSimpleElement * g = new GpxSimpleElement(wxString(_T("text")), description);
            LinkEndChild(g);
      }
      if(!mime_type.IsEmpty())
            LinkEndChild(new GpxSimpleElement(wxString(_T("type")), mime_type));
}

GpxWptElement::GpxWptElement(char *waypoint_type, double lat, double lon, double ele, wxDateTime * time,
            double magvar, double geoidheight, const wxString &name, const wxString &cmt,
            const wxString &desc, const wxString &src, ListOfGpxLinks *links, const wxString &sym, const wxString &type,
            GpxFixType fixtype, int sat, double hdop, double vdop, double pdop,
            double ageofgpsdata, int dgpsid, GpxExtensionsElement *extensions) : TiXmlElement(waypoint_type)
{
      SetAttribute("lat", wxString::Format(_T("%.9f"), lat).ToUTF8());
      SetAttribute("lon", wxString::Format(_T("%.9f"), lon).ToUTF8());

      if (ele != 0)
            SetProperty(wxString(_T("ele")), wxString::Format(_T("%f"), ele));

      if (time)
            if (time->IsValid())
                  SetProperty(wxString(_T("time")), time->FormatISODate().Append(_T("T")).Append(time->FormatISOTime()).Append(_T("Z")));
      if (magvar != 0)
            SetProperty(wxString(_T("magvar")), wxString::Format(_T("%f"), magvar));
      if (geoidheight != -1)
            SetProperty(wxString(_T("geoidheight")), wxString::Format(_T("%f"), geoidheight));
      if (!name.IsEmpty())
            SetProperty(wxString(_T("name")), name);
      if (!cmt.IsEmpty())
            SetProperty(wxString(_T("cmt")), cmt);
      if (!desc.IsEmpty())
            SetProperty(wxString(_T("desc")), desc);
      if (!src.IsEmpty())
            SetProperty(wxString(_T("src")), src);
      if (links) {
            wxListOfGpxLinksNode *link = links->GetFirst();
            while (link)
            {
                  LinkEndChild(link->GetData());
                  link = link->GetNext();
            }
      }
      if (!sym.IsEmpty() /*&& (sym != _T("empty"))*/) //"empty" is a valid symbol for us, we need to preserve it, otherwise it would be non existent and replaced by a circle on next load...
            SetProperty(wxString(_T("sym")), sym);
      if (!type.IsEmpty())
            SetProperty(wxString(_T("type")), type);
      if (fixtype != fix_undefined)
            SetProperty(wxString(_T("fix")), FixTypeToStr(fixtype));
      if (sat != -1)
            SetProperty(wxString(_T("sat")), wxString::Format(_T("%u"), sat));
      if (hdop != -1)
            SetProperty(wxString(_T("hdop")), wxString::Format(_T("%f"), hdop));
      if (vdop != -1)
            SetProperty(wxString(_T("vdop")), wxString::Format(_T("%f"), vdop));
      if (pdop != -1)
            SetProperty(wxString(_T("pdop")), wxString::Format(_T("%f"), pdop));
      if (ageofgpsdata != -1)
            SetProperty(wxString(_T("ageofgpsdata")), wxString::Format(_T("%f"), ageofgpsdata));
      if (dgpsid != -1)
            SetProperty(wxString(_T("dgpsid")), wxString::Format(_T("%u"), dgpsid));
      if (extensions)
            LinkEndChild(extensions);
}

wxString GpxWptElement::FixTypeToStr(GpxFixType fixtype)
{
      switch(fixtype)
      {
            case fix_none:
                  return wxString(_T("none"));
            case fix_2d:
                  return wxString(_T("2d"));
            case fix_3d:
                  return wxString(_T("3d"));
            case fix_dgps:
                  return wxString(_T("dgps"));
            case fix_pps:
                  return wxString(_T("pps"));
            default:
                  return wxString(_T(""));
      }
}

void GpxWptElement::SetSimpleExtension(const wxString &name, const wxString &value)
{
      //FIXME: if the extensions don't exist, we should create them
      TiXmlElement * exts = FirstChildElement("extensions");
      if (exts) {
            TiXmlElement * ext = exts->FirstChildElement(name.ToUTF8());
            if (ext)
                  exts->ReplaceChild(ext, GpxSimpleElement(name, value));
            else
                  exts->LinkEndChild(new GpxSimpleElement(name, value));
      }
}

void GpxRteElement::SetSimpleExtension(const wxString &name, const wxString &value)
{
      //FIXME: if the extensions don't exist, we should create them
      TiXmlElement * exts = FirstChildElement("extensions");
      if (exts) {
            TiXmlElement * ext = exts->FirstChildElement(name.ToUTF8());
            if (ext)
                  exts->ReplaceChild(ext, GpxSimpleElement(name, value));
            else
                  exts->LinkEndChild(new GpxSimpleElement(name, value));
      }
}

void GpxTrkElement::SetSimpleExtension(const wxString &name, const wxString &value)
{
      //FIXME: if the extensions don't exist, we should create them
      TiXmlElement * exts = FirstChildElement("extensions");
      if (exts) {
            TiXmlElement * ext = exts->FirstChildElement(name.ToUTF8());
            if (ext)
                  exts->ReplaceChild(ext, GpxSimpleElement(name, value));
            else
                  exts->LinkEndChild(new GpxSimpleElement(name, value));
      }
}

void GpxWptElement::SetProperty(const wxString &name, const wxString &value)
{
      //FIXME: doesn't care about order so it can be absolutely wrong, have to redo this code if it has to be used by something else than the constructor
      //then it can be made public
      GpxSimpleElement *element = new GpxSimpleElement(name, value);
      TiXmlElement *curelement = FirstChildElement();
      bool found = false;
      while(curelement)
      {
            if((const char *)curelement->Value() == (const char *)name.ToUTF8())
            {
                  ReplaceChild(curelement, *element);
                  element->Clear();
                  delete element;
                  found = true;
                  break;
            }
            curelement = curelement->NextSiblingElement();
      }
      if (!found)
            LinkEndChild(element);
}

GpxRteElement::GpxRteElement(const wxString &name, const wxString &cmt, const wxString &desc,
              const wxString &src, ListOfGpxLinks *links, int number,
              const wxString &type, GpxExtensionsElement *extensions, ListOfGpxWpts *waypoints) : TiXmlElement("rte")
{
      if (!name.IsEmpty())
            SetProperty(wxString(_T("name")), name);
      if (!cmt.IsEmpty())
            SetProperty(wxString(_T("cmt")), cmt);
      if (!desc.IsEmpty())
            SetProperty(wxString(_T("desc")), desc);
      if (!src.IsEmpty())
            SetProperty(wxString(_T("src")), src);
      if (links)
      {
            wxListOfGpxLinksNode *link = links->GetFirst();
            while (link)
            {
                  LinkEndChild(link->GetData());
                  link = link->GetNext();
            }
      }
      if (number != -1)
            SetProperty(wxString(_T("number")), wxString::Format(_T("%u"), number));
      if (!type.IsEmpty())
            SetProperty(wxString(_T("type")), type);
      if (extensions)
            LinkEndChild(extensions);
      if (waypoints) {
            wxListOfGpxWptsNode *wpt = waypoints->GetFirst();
            while (wpt)
            {
                  //TODO: Here we should check whether the waypoint is a *rtept*
                  AppendRtePoint(wpt->GetData());
                  wpt = wpt->GetNext();
            }
      }
}

void GpxRteElement::SetProperty(const wxString &name, const wxString &value)
{
      //FIXME: doesn't care about order so it can be absolutely wrong, have to redo this code if it has to be used by something else than the constructor
      //then it can be made public
      //FIXME: can be reused for route and track
      GpxSimpleElement *element = new GpxSimpleElement(name, value);
      TiXmlElement *curelement = FirstChildElement();
      bool found = false;
      while(curelement)
      {
            if((const char *)curelement->Value() == (const char *)name.ToUTF8())
            {
                  ReplaceChild(curelement, *element);
                  element->Clear();
                  delete element;
                  found = true;
                  break;
            }
            curelement = curelement->NextSiblingElement();
      }
      if (!found)
            LinkEndChild(element);
}

void GpxRteElement::AppendRtePoint(GpxWptElement *rtept)
{
      //FIXME: doesn't care about order so it can be absolutely wrong, have to redo this code if it has to be used by something else than the constructor
      //FIXME: can be reused for route and track segment
      LinkEndChild(rtept);
}

void GpxTrksegElement::AppendTrkPoint(GpxWptElement *trkpt)
{
      //FIXME: can be reused for route and track segment
      LinkEndChild(trkpt);
}

GpxTrkElement::GpxTrkElement(const wxString &name, const wxString &cmt, const wxString &desc,
              const wxString &src, ListOfGpxLinks *links, int number,
              const wxString &type, GpxExtensionsElement *extensions, ListOfGpxTrksegs *segments) : TiXmlElement("trk")
{
      if (!name.IsEmpty())
            SetProperty(wxString(_T("name")), name);
      if (!cmt.IsEmpty())
            SetProperty(wxString(_T("cmt")), cmt);
      if (!desc.IsEmpty())
            SetProperty(wxString(_T("desc")), desc);
      if (!src.IsEmpty())
            SetProperty(wxString(_T("src")), src);
      if (links)
      {
            wxListOfGpxLinksNode *link = links->GetFirst();
            while (link)
            {
                  LinkEndChild(link->GetData());
                  link = link->GetNext();
            }
      }
      if (number != -1)
            SetProperty(wxString(_T("number")), wxString::Format(_T("%u"), number));
      if (!type.IsEmpty())
            SetProperty(wxString(_T("type")), type);
      if (extensions)
            LinkEndChild(extensions);
      if (segments)
      {
            wxListOfGpxTrksegsNode *seg = segments->GetFirst();
            while (seg)
            {
                  AppendTrkSegment(seg->GetData());
                  seg = seg->GetNext();
            }
      }
}

void GpxTrkElement::AppendTrkSegment(GpxTrksegElement *trkseg)
{
      //FIXME: can be reused for route and track segment
      LinkEndChild(trkseg);
}

void GpxTrkElement::SetProperty(const wxString &name, const wxString &value)
{
      //FIXME: doesn't care about order so it can be absolutely wrong, have to redo this code if it has to be used by something else than the constructor
      //then it can be made public
      //FIXME: can be reused for route and track
      GpxSimpleElement *element = new GpxSimpleElement(name, value);
      TiXmlElement *curelement = FirstChildElement();
      bool found = false;
      while(curelement)
      {
            if((const char *)curelement->Value() == (const char *)name.ToUTF8())
            {
                  ReplaceChild(curelement, *element);
                  element->Clear();
                  delete element;
                  break;
            }
            curelement = curelement->NextSiblingElement();
      }
      if (!found)
            LinkEndChild(element);
}

GpxTrksegElement::GpxTrksegElement(ListOfGpxWpts *waypoints, GpxExtensionsElement *extensions) : TiXmlElement("trkseg")
{
      if (waypoints) {
            wxListOfGpxWptsNode *wpt = waypoints->GetFirst();
            while (wpt)
            {
                  //TODO: Here we should check whether the waypoint is a *trkpt*
                  LinkEndChild(wpt->GetData());
                  wpt = wpt->GetNext();
            }
      }
      if (extensions)
            LinkEndChild(extensions);
}

GpxSimpleElement::GpxSimpleElement(const wxString &element_name, const wxString &element_value) : TiXmlElement(element_name.ToUTF8())
{
      TiXmlText * value = new TiXmlText(element_value.ToUTF8());
      LinkEndChild(value);
}
