/******************************************************************************
 * $Id: chartcatalog.cpp,v 1.0 2011/02/26 01:54:37 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Chart downloader Plugin
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Pavel Kalian   *
 *   $EMAIL$   *
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
 */

#include "chartcatalog.h"
#include <wx/tokenzr.h>

#include <wx/arrimpl.cpp>
    //WX_DEFINE_OBJARRAY(wxArrayOfNoticeToMariners);
    WX_DEFINE_OBJARRAY(wxArrayOfVertexes);
    WX_DEFINE_OBJARRAY(wxArrayOfPanels);
    WX_DEFINE_OBJARRAY(wxArrayOfCharts);


// Chart Catalog implementation
bool ChartCatalog::LoadFromFile( wxString path, bool headerOnly )
{
    dt_valid = wxInvalidDateTime;      // Invalidate all dates
    date_created = dt_valid;            // so dates of one catalog
    time_created = dt_valid;            // don't propagate into another
    date_valid = dt_valid;
    title = _("Catalog is not valid.");      // Invalidate the title in case we read a bad file
    if( !wxFileExists(path) )
        return false;
    TiXmlDocument * doc = new TiXmlDocument();
    bool ret = doc->LoadFile( path.mb_str(), TIXML_ENCODING_UTF8 );
    if (ret)
        ret = LoadFromXml( doc, headerOnly );
    else
        charts.Clear();
    doc->Clear();
    wxDELETE(doc);

    return ret;
}

ChartCatalog::ChartCatalog()
{
}

ChartCatalog::~ChartCatalog()
{
}

wxDateTime ChartCatalog::GetReleaseDate()
{
    if( !dt_valid.IsValid() )
    {
        // date-time was invalid so we will create it from time_created and date_created
        // If they are not valid then we return an invalid date for debugging purposes
        if ( date_created.IsValid() && time_created.IsValid() )
        {
            dt_valid.ParseDate(date_created.FormatDate());
            dt_valid.ParseTime(time_created.FormatTime());
            dt_valid.MakeFromTimezone(wxDateTime::UTC);
        }
    }
    wxASSERT(dt_valid.IsValid());
    return dt_valid;
}
 
bool ChartCatalog::LoadFromXml( TiXmlDocument * doc, bool headerOnly )
{
    TiXmlElement * root = doc->RootElement();
    wxString rootName = wxString::FromUTF8( root->Value() );
    charts.Clear();

    if( rootName.StartsWith( _T("RncProductCatalog") ) )
    {
        if( !ParseNoaaHeader(root->FirstChildElement()) )
        {
            return false;
        }
        if (headerOnly)
            return true;
        TiXmlNode *child;
        for ( child = root->FirstChildElement()->NextSibling(); child != 0; child = child->NextSibling() )
        {
            if( _T("chart") == wxString::FromUTF8( child->Value() ) )
                charts.Add(new RasterChart(child));
        }
    }
    else if( rootName.StartsWith(_T("EncProductCatalog")) )
    {
        if( !ParseNoaaHeader(root->FirstChildElement()) )
            return false;
            
        if( headerOnly )
            return true;
        TiXmlNode *child;
        for( child = root->FirstChildElement()->NextSibling(); child != 0; child = child->NextSibling() )
        {
            if( _T("cell") == wxString::FromUTF8( child->Value() ) )
                charts.Add( new EncCell(child) );
        }
    }
    else if( rootName.StartsWith(_T("IENCU37ProductCatalog")) )
    {
        if( !ParseNoaaHeader(root->FirstChildElement()) )
        {
            return false;
        }
        if( headerOnly )
            return true;
        TiXmlNode *child;
        for( child = root->FirstChildElement()->NextSibling(); child != 0; child = child->NextSibling())
        {
            if( _T("Cell") == wxString::FromUTF8( child->Value() ) )
                charts.Add(new IEncCell(child));
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool ChartCatalog::ParseNoaaHeader( TiXmlElement * xmldata )
{
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling())
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("title") )
        {
            if( !child->NoChildren() )
                title = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("date_created") )
        {
            if( !child->NoChildren() )
            {
                date_created.ParseDate( wxString::FromUTF8(child->FirstChild()->Value()) );
                wxASSERT(date_created.IsValid());
            }
        }
        else if( s == _T("time_created") )
        {
            if( !child->NoChildren() )
            {
                time_created.ParseTime( wxString::FromUTF8(child->FirstChild()->Value()) );
                wxASSERT(time_created.IsValid());
            }
        }
        else if( s == _T("date_valid") )
        {
            if( !child->NoChildren() )
            {
                date_valid.ParseDate( wxString::FromUTF8(child->FirstChild()->Value()) );
                wxASSERT(date_valid.IsValid());
            }
        }
        else if( s == _T("time_valid") )
        {
            if( !child->NoChildren() )
            {
                time_valid.ParseTime( wxString::FromUTF8(child->FirstChild()->Value()) );
                wxASSERT(time_valid.IsValid());
            }
        }
        else if( s == _T("dt_valid") )
        {
            if( !child->NoChildren() )
            {
                wxStringTokenizer tk( wxString::FromUTF8(child->FirstChild()->Value()), _T("TZ") );
                dt_valid.ParseDate(tk.GetNextToken());
                dt_valid.ParseTime(tk.GetNextToken());
                dt_valid.MakeFromTimezone(wxDateTime::UTC);
                wxASSERT(dt_valid.IsValid());
            }
        }
        else if( s == _T("ref_spec") )
        {
            if( !child->NoChildren() )
                ref_spec = wxString::FromUTF8( child->FirstChild()->Value() );
        }
        else if( s == _T("ref_spec_vers") )
        {
            if( !child->NoChildren() )
                ref_spec_vers = wxString::FromUTF8( child->FirstChild()->Value() );
        }
        else if( s == _T("s62AgencyCode") )
        {
            if( !child->NoChildren() )
                s62AgencyCode = wxString::FromUTF8(child->FirstChild()->Value());
        }
    }
    return true;
}

Chart::~Chart()
{
    coast_guard_districts->Clear();
    wxDELETE(coast_guard_districts);
    states->Clear();
    wxDELETE(states);
    regions->Clear();
    wxDELETE(regions);
    wxDELETE(nm);
    wxDELETE(lnm);
}

Chart::Chart( TiXmlNode * xmldata )
{
    coast_guard_districts = new wxArrayString();
    states = new wxArrayString();
    regions = new wxArrayString();
    TiXmlNode *child;
    target_filename = wxEmptyString;
    reference_file = wxEmptyString;
    manual_download_url = wxEmptyString;
    title = wxEmptyString;
    zipfile_location = wxEmptyString;
    zipfile_size = -1;
    zipfile_datetime = wxInvalidDateTime;
    zipfile_datetime_iso8601 = wxInvalidDateTime;
    nm = NULL;
    lnm = NULL;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("title") || s == _T("lname") )
        {
            if( !child->NoChildren() )
                title = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("coast_guard_districts") )
        {
            TiXmlNode *mychild;
            for( mychild = child->FirstChild(); mychild != 0; mychild = mychild->NextSibling() )
            {
                if( !mychild->NoChildren() )
                    coast_guard_districts->Add( wxString::FromUTF8(mychild->FirstChild()->Value()) );
            }
        }
        else if( s == _T("states") )
        {
            TiXmlNode *mychild;
            for ( mychild = child->FirstChild(); mychild != 0; mychild = mychild->NextSibling())
            {
                if( !mychild->NoChildren() )
                    states->Add( wxString::FromUTF8(mychild->FirstChild()->Value()) );
            }
        }
        else if( s == _T("regions") )
        {
            TiXmlNode *mychild;
            for( mychild = child->FirstChild(); mychild != 0; mychild = mychild->NextSibling() )
            {
                if( !mychild->NoChildren() )
                    regions->Add( wxString::FromUTF8(mychild->FirstChild()->Value()) );
            }
        }
        else if( s == _T("zipfile_location") )
        {
            if( !child->NoChildren() )
                zipfile_location = wxString::FromUTF8( child->FirstChild()->Value() );
        }
        else if( s == _T("zipfile_datetime") )
        {
            if( !child->NoChildren() )
                if( zipfile_datetime.ParseFormat(wxString::FromUTF8(child->FirstChild()->Value()), _T("%Y%m%d_%H%M%S")) )
                    zipfile_datetime.MakeFromTimezone(wxDateTime::UTC);
        }
        else if( s == _T("zipfile_datetime_iso8601") )
        {
            if( !child->NoChildren() )
            {
                wxStringTokenizer tk(wxString::FromUTF8(child->FirstChild()->Value()), _T("TZ"));
                zipfile_datetime_iso8601.ParseDate(tk.GetNextToken());
                zipfile_datetime_iso8601.ParseTime(tk.GetNextToken());
                zipfile_datetime_iso8601.MakeFromTimezone(wxDateTime::UTC);
            }
        }
        else if( s == _T("zipfile_size") )
        {
            if( !child->NoChildren() )
                zipfile_size = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("nm") )
        {
            // NOT USED
            // nm = new NoticeToMariners(child);
        }
        else if( s == _T("lnm") )
        {
            // NOT USED
            // lnm = new NoticeToMariners(child);
        }
        else if( s == _T("cov") )
        {
            TiXmlNode *mychild;
            for( mychild = child->FirstChild(); mychild != 0; mychild = mychild->NextSibling() )
            {
                coverage.Add(new Panel(mychild));
            }
        }
        else if( s == _T("target_filename") )
        {
            if( !child->NoChildren() )
                target_filename = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("reference_file") )
        {
            if( !child->NoChildren() )
                reference_file = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("manual_download_url") )
        {
            if( !child->NoChildren() )
                manual_download_url = wxString::FromUTF8(child->FirstChild()->Value());
        }
    }
}

wxString Chart::GetChartFilename( bool to_check )
{
    if( to_check && reference_file != wxEmptyString )
        return reference_file;
    if( target_filename != wxEmptyString )
        return target_filename;
    wxString file;
    wxStringTokenizer tk(zipfile_location, _T("/"));
    do
    {
        file = tk.GetNextToken();
    } while(tk.HasMoreTokens());
    return file;
}

RasterChart::RasterChart( TiXmlNode * xmldata ) : Chart( xmldata )
{
    TiXmlNode *child;
    number = wxEmptyString;
    source_edition = -1;
    raster_edition = -1;
    ntm_edition = -1;
    source_date = wxEmptyString;
    ntm_date = wxEmptyString;
    source_edition_last_correction = wxEmptyString;
    raster_edition_last_correction = wxEmptyString;
    ntm_edition_last_correction = wxEmptyString;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("number") )
        {
            if( !child->NoChildren() )
                number = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("source_edition") )
        {
            if( !child->NoChildren() )
                source_edition = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("raster_edition") )
        {
            if( !child->NoChildren() )
                raster_edition = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("ntm_edition") )
        {
            if( !child->NoChildren() )
                ntm_edition = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("source_date") )
        {
            if( !child->NoChildren() )
                source_date = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("ntm_date") )
        {
            if( !child->NoChildren() )
                ntm_date = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("source_edition_last_correction") )
        {
            if( !child->NoChildren() )
                source_edition_last_correction = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("raster_edition_last_correction") )
        {
            if( !child->NoChildren() )
                raster_edition_last_correction = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("ntm_edition_last_correction") )
        {
            if( !child->NoChildren() )
                ntm_edition_last_correction = wxString::FromUTF8(child->FirstChild()->Value());
        }
    }
}

EncCell::EncCell( TiXmlNode * xmldata ) : Chart( xmldata )
{
    TiXmlNode *child;
    name = wxEmptyString;
    src_chart = wxEmptyString;
    cscale = -1;
    status = wxEmptyString;
    edtn = -1;
    updn = -1;
    uadt = wxInvalidDateTime;
    isdt = wxInvalidDateTime;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("name") )
        {
            if( !child->NoChildren() )
            {
                name = wxString::FromUTF8(child->FirstChild()->Value());
                number = name;
            }
        }
        else if( s == _T("src_chart") )
        {
            if( !child->NoChildren() )
                src_chart = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("cscale") )
        {
            if( !child->NoChildren() )
                cscale = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("status") )
        {
            if( !child->NoChildren() )
                status = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("edtn") )
        {
            if( !child->NoChildren() )
                edtn = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("updn") )
        {
            if( !child->NoChildren() )
                updn = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("uadt") )
        {
            if( !child->NoChildren() )
                uadt.ParseDateTime(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("isdt") )
        {
            if( !child->NoChildren() )
                isdt.ParseDateTime(wxString::FromUTF8(child->FirstChild()->Value()));
        }
    }
}

IEncCell::IEncCell( TiXmlNode * xmldata ) : Chart( xmldata )
{
    name = wxEmptyString;
    location = NULL;
    river_name = wxEmptyString;
    river_miles = NULL;
    area = NULL;
    edition = wxEmptyString;
    shp_file = NULL;
    s57_file = NULL;
    kml_file = NULL;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("name") )
        {
            if( !child->NoChildren() )
            {
                name = wxString::FromUTF8(child->FirstChild()->Value());
                zipfile_location = wxString::Format(_T("%s.zip"), name.c_str());
            }
        }
        else if( s == _T("location") )
        {
            location = new Location(child);
        }
        else if( s == _T("river_name") )
        {
            river_name = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("river_miles") )
        {
            river_miles = new RiverMiles(child);
        }
        else if( s == _T("area") )
        {
            area = new Area(child);
        }
        else if( s == _T("edition") )
        {
            if( !child->NoChildren() )
                edition = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("shp_file") )
        {
            shp_file = new ChartFile(child);
        }
        else if( s == _T("s57_file") )
        {
            s57_file = new ChartFile(child);
        }
        else if( s == _T("kml_file") )
        {
            kml_file = new ChartFile(child);
        }
    }
}

IEncCell::~IEncCell()
{
    wxDELETE(location);
    wxDELETE(river_miles);
    wxDELETE(area);
    wxDELETE(shp_file);
    wxDELETE(s57_file);
    wxDELETE(kml_file);
}

wxString IEncCell::GetChartTitle()
{
    return wxString::Format(_("%s (%s to %s), river miles %3.1f - %3.1f"), river_name.c_str(), location->from.c_str(), location->to.c_str(), river_miles->begin, river_miles->end);
}

wxString IEncCell::GetDownloadLocation()
{
    return s57_file->location;
}

wxDateTime IEncCell::GetUpdateDatetime()
{
    return s57_file->date_posted;
}

ChartFile::ChartFile( TiXmlNode * xmldata )
{
    file_size = -1;
    location = wxEmptyString;
    date_posted = wxInvalidDateTime;
    time_posted = wxInvalidDateTime;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("location") )
        {
            if( !child->NoChildren() )
                location = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("date_posted") )
        {
            if( !child->NoChildren() )
                date_posted.ParseDate(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("time_posted") )
        {
            if( !child->NoChildren() )
                time_posted.ParseTime(wxString::FromUTF8(child->FirstChild()->Value()));
            else
                time_posted.ParseTime(_T("00:00:00"));
        }
        else if( s == _T("file_size") )
        {
            if( !child->NoChildren() )
                file_size = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
            else
                file_size = -1;
        }
    }
}

Area::Area( TiXmlNode * xmldata )
{
    north = 0.0;
    south = 0.0;
    east = 0.0;
    west = 0.0;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("north") )
        {
            if( !child->NoChildren() )
                north = wxAtof(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("south") )
        {
            if( !child->NoChildren() )
                south = wxAtof(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("east") )
        {
            if( !child->NoChildren() )
                east = wxAtof(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("west") )
        {
            if( !child->NoChildren() )
                west = wxAtof(wxString::FromUTF8(child->FirstChild()->Value()));
        }
    }
}

RiverMiles::RiverMiles( TiXmlNode * xmldata )
{
    begin = -1;
    end = -1;
    TiXmlNode *child;
    for ( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("begin") )
        {
            if( !child->NoChildren() )
                begin = wxAtof(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("end") )
        {
            if( !child->NoChildren() )
                end = wxAtof(wxString::FromUTF8(child->FirstChild()->Value()));
        }
    }
}

Location::Location( TiXmlNode * xmldata )
{
    from = wxEmptyString;
    to = wxEmptyString;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("from") )
        {
            if( !child->NoChildren() )
                from = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("to") )
        {
            if( !child->NoChildren() )
                to = wxString::FromUTF8(child->FirstChild()->Value());
        }
    }
}


NoticeToMariners::NoticeToMariners( TiXmlNode * xmldata )
{
    agency = wxEmptyString;
    doc = wxEmptyString;
    date = wxInvalidDateTime;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("nm_agency") || s == _T("lnm_agency") )
        {
            if( !child->NoChildren() )
                agency = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("doc") )
        {
            if( !child->NoChildren() )
                doc = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("date") )
        {
            if( !child->NoChildren() )
                date.ParseDate(wxString::FromUTF8(child->FirstChild()->Value()));
        }
    }
}

Panel::Panel( TiXmlNode * xmldata )
{
    panel_no = -1;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("panel_no") )
        {
            if( !child->NoChildren() )
                panel_no = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
        else if( s == _T("vertex") )
        {
            // NOT USED
            //vertexes.Add(new Vertex(child));
        }
    }
}

Panel::~Panel()
{
}

RncPanel::RncPanel( TiXmlNode * xmldata ) : Panel( xmldata )
{
    panel_title = wxEmptyString;
    file_name = wxEmptyString;
    scale = 0;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("panel_title") )
        {
            if( !child->NoChildren() )
                panel_title = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("file_name") )
        {
            if( !child->NoChildren() )
                file_name = wxString::FromUTF8(child->FirstChild()->Value());
        }
        else if( s == _T("scale") )
        {
            if( !child->NoChildren() )
                scale = wxAtoi(wxString::FromUTF8(child->FirstChild()->Value()));
        }
    }
}

EncPanel::EncPanel( TiXmlNode * xmldata ) : Panel( xmldata )
{
    type = wxEmptyString;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("type") )
        {
            if( !child->NoChildren() )
                type = wxString::FromUTF8(child->FirstChild()->Value());
        }
    }
}

Vertex::Vertex( TiXmlNode * xmldata )
{
    //Init properties
    lat = 999.0;
    lon = 999.0;
    TiXmlNode *child;
    for( child = xmldata->FirstChild(); child != 0; child = child->NextSibling() )
    {
        wxString s = wxString::FromUTF8(child->Value());
        if( s == _T("lat") )
        {
            if( !child->NoChildren() )
                wxString::FromUTF8(child->FirstChild()->Value()).ToDouble(&lat);
        }
        else if( s == _T("long") )
        {
            if( !child->NoChildren() )
                wxString::FromUTF8(child->FirstChild()->Value()).ToDouble(&lon);
        }
    }
}
