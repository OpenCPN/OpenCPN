/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GPX utility classes
 * Author:   David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                                  *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the                            *
 *   Free Software Foundation, Inc.,                                          *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.                *
 ***************************************************************************
*/
#ifndef _GPXDOCUMENT_H_
#define _GPXDOCUMENT_H_

#include <wx/wx.h>
#include <wx/regex.h>
#include <tinyxml.h>
//#include <tinystr.h>
#include <stdio.h>

const wxString GpxxColorNames[] = { _("Black"), _("DarkRed"), _("DarkGreen"), _("DarkYellow"), _("DarkBlue"), _("DarkMagenta"), _("DarkCyan"), _("LightGray"), _("DarkGray"), _("Red"), _("Green"), _("Yellow"), _("Blue"), _("Magenta"), _("Cyan"), _("White") };//The last color defined by Garmin is transparent - we ignore it
const wxColour GpxxColors[] = { wxColour(0x00, 0x00, 0x00), wxColour(0x60, 0x00, 0x00), wxColour(0x00, 0x60, 0x00), wxColour(0x80, 0x80, 0x00), wxColour(0x00, 0x00, 0x60), wxColour(0x60, 0x00, 0x60), wxColour(0x00, 0x80, 0x80), wxColour(0xC0, 0xC0, 0xC0), wxColour(0x60, 0x60, 0x60), wxColour(0xFF, 0x00, 0x00), wxColour(0x00, 0xFF, 0x00), wxColour(0xF0, 0xF0, 0x00), wxColour(0x00, 0x00, 0xFF), wxColour(0xFE, 0x00, 0xFE), wxColour(0x00, 0xFF, 0xFF), wxColour(0xFF, 0xFF, 0xFF) };
const int StyleValues[] = { -1, wxSOLID, wxDOT, wxLONG_DASH, wxSHORT_DASH, wxDOT_DASH };
const int WidthValues[] = { -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };


// Forward declarations
class GpxWptElement;
class GpxRteElement;
class GpxTrkElement;
class GpxRootElement;
class GpxDocument;

//Class Declarations
class GpxDocument : public TiXmlDocument
{
public:
      GpxDocument();
      GpxDocument(const wxString &filename);
      bool LoadFile(const wxString &filename);
      bool SaveFile(const wxString &filename);
      void AddCustomNamespace(const wxString &name, const wxString &url);

      virtual ~GpxDocument();
      // RFC4122 version 4 compliant random UUIDs generator.
      static wxString GetUUID(void);
private:
      static int GetRandomNumber(int min, int max);
      void PopulateEmptyDocument(const wxString &creator);
      void SeedRandom();
};

//Utility class for the simple types
class GpxSimpleElement : public TiXmlElement
{
public:
      GpxSimpleElement(const wxString &element_name, const wxString &element_value);
};

class GpxEmailElement : public TiXmlElement
{
public:
      GpxEmailElement(const wxString &email);
};

class GpxLinkElement : public TiXmlElement
{
public:
      GpxLinkElement(const wxString &uri, const wxString &description, const wxString &mime_type);
};

//    Declare a list of links
WX_DECLARE_LIST(GpxLinkElement, ListOfGpxLinks);

class GpxPersonElement : public TiXmlElement
{
public:
      GpxPersonElement(const wxString &name, GpxEmailElement *email, GpxLinkElement *link);
};

class GpxCopyrightElement : public TiXmlElement
{
public:
      GpxCopyrightElement(const wxString &author, int year, const wxString &licenseUri);
};

class GpxExtensionsElement : public TiXmlElement
{
public:
      GpxExtensionsElement();
};

class GpxBoundsElement : public TiXmlElement
{
public:
      GpxBoundsElement(double minlat, double minlon, double maxlat, double maxlon);
};

class GpxMetadataElement : public TiXmlElement
{
public:
      GpxMetadataElement(const wxString &name, const wxString &desc, GpxPersonElement *author, GpxCopyrightElement *copyright, GpxLinkElement *link, wxDateTime *time, const wxString &keywords, GpxBoundsElement *bounds, GpxExtensionsElement *extensions);
};

class GpxxExtensionsElement : public TiXmlElement
{
public:
      GpxxExtensionsElement(const wxString &element_name);
};

typedef enum {
      fix_undefined,
      fix_none,
      fix_2d,
      fix_3d,
      fix_dgps,
      fix_pps
} GpxFixType;

// Waypoint types
#define GPX_WPT_WAYPOINT (char *)"wpt"
#define GPX_WPT_ROUTEPOINT (char *)"rtept"
#define GPX_WPT_TRACKPOINT (char *)"trkpt"

#define GPX_EMPTY_STRING _T("")

class GpxWptElement : public TiXmlElement
{
public:
      GpxWptElement(char *waypoint_type, double lat, double lon, double ele = 0, wxDateTime * time = NULL,
          double magvar = 0, double geoidheight = -1, const wxString &name = GPX_EMPTY_STRING, const wxString &cmt = GPX_EMPTY_STRING,
          const wxString &desc = GPX_EMPTY_STRING, const wxString &src = GPX_EMPTY_STRING, ListOfGpxLinks *links = NULL, const wxString &sym = GPX_EMPTY_STRING,
          const wxString &type = GPX_EMPTY_STRING, GpxFixType fixtype = fix_undefined, int sat = -1, double hdop = -1, double vdop = -1, double pdop = -1,
          double ageofgpsdata = -1, int dgpsid = -1, GpxExtensionsElement *extensions = NULL);
      void SetSimpleExtension(const wxString &name, const wxString &value);
private:
      void SetProperty(const wxString &name, const wxString &value);
      wxString FixTypeToStr(GpxFixType fixtype);
};

//    Declare a list of waypoints
WX_DECLARE_LIST(GpxWptElement, ListOfGpxWpts);

class GpxRteElement : public TiXmlElement
{
public:
      GpxRteElement(const wxString &name = GPX_EMPTY_STRING, const wxString &cmt = GPX_EMPTY_STRING, const wxString &desc = GPX_EMPTY_STRING,
          const wxString &src = GPX_EMPTY_STRING, ListOfGpxLinks *links = NULL, int number = -1,
          const wxString &type = GPX_EMPTY_STRING, GpxExtensionsElement *extensions = NULL, ListOfGpxWpts *waypoints = NULL);
      void AppendRtePoint(GpxWptElement *rtept);
      void SetSimpleExtension(const wxString &name, const wxString &value);
private:
      void SetProperty(const wxString &name, const wxString &value);
};

//    Declare a list of routes
WX_DECLARE_LIST(GpxRteElement, ListOfGpxRoutes);

class GpxTrksegElement : public TiXmlElement
{
public:
      GpxTrksegElement(ListOfGpxWpts *waypoints = NULL, GpxExtensionsElement *extensions = NULL);
      void AppendTrkPoint(GpxWptElement *trkpt);
};

//    Declare an array of track segments
WX_DECLARE_LIST(GpxTrksegElement, ListOfGpxTrksegs);

class GpxTrkElement : public TiXmlElement
{
public:
      GpxTrkElement(const wxString &name = GPX_EMPTY_STRING, const wxString &cmt = GPX_EMPTY_STRING, const wxString &desc = GPX_EMPTY_STRING,
          const wxString &src = GPX_EMPTY_STRING, ListOfGpxLinks *links = NULL, int number = -1,
          const wxString &type = GPX_EMPTY_STRING, GpxExtensionsElement *extensions = NULL, ListOfGpxTrksegs *segments = NULL);
      void AppendTrkSegment(GpxTrksegElement *trkseg);
      void SetSimpleExtension(const wxString &name, const wxString &value);
private:
      void SetProperty(const wxString &name, const wxString &value);
};

//    Declare an array of tracks
WX_DECLARE_LIST(GpxTrkElement , ListOfGpxTracks);

class GpxRootElement : public TiXmlElement
{
public:
      GpxRootElement(const wxString &creator, GpxMetadataElement *metadata = NULL, ListOfGpxWpts *waypoints = NULL, ListOfGpxRoutes *routes = NULL, ListOfGpxTracks *tracks = NULL, GpxExtensionsElement *extensions = NULL);
      void AddWaypoint(GpxWptElement *waypoint);
      void AddRoute(GpxRteElement *route);
      void AddTrack(GpxTrkElement *track);
      void SetMetadata(GpxMetadataElement *metadata);
      void RemoveMetadata();
      void SetExtensions(GpxExtensionsElement *extensions);
      void RemoveExtensions();
private:
      GpxWptElement *first_waypoint;
      GpxWptElement *last_waypoint;
      GpxRteElement *first_route;
      GpxRteElement *last_route;
      GpxTrkElement *first_track;
      GpxTrkElement *last_track;
      GpxMetadataElement *my_metadata;
      GpxExtensionsElement *my_extensions;
};

#endif          // _GPXDOCUMENT_H_
