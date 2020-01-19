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
    
    pugi::xml_document *doc = new pugi::xml_document;
    bool ret = doc->load_file( path.mb_str() );
    if (ret)
        ret = LoadFromXml( doc, headerOnly );
    else
        charts.Clear();

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
 
bool ChartCatalog::LoadFromXml( pugi::xml_document * doc, bool headerOnly )
{
    pugi::xml_node root = doc->first_child();
    
    wxString rootName = wxString::FromUTF8( root.name() );
    charts.Clear();
    if( rootName.StartsWith( _T("RncProductCatalog") ) )
    {
        if( !ParseNoaaHeader(root.first_child()) )
        {
            return false;
        }
        if (headerOnly)
            return true;
        
        for (pugi::xml_node element = root.first_child(); element; element = element.next_sibling()){
            if( !strcmp(element.name(), "chart") ){
                charts.Add(new RasterChart(element));
            }
        }
    }
    else if( rootName.StartsWith( _T("EncProductCatalog") ) )
    {
        if( !ParseNoaaHeader(root.first_child()) )
        {
            return false;
        }
        if (headerOnly)
            return true;
        
        for (pugi::xml_node element = root.first_child(); element; element = element.next_sibling()){
            if( !strcmp(element.name(), "cell") ){
                charts.Add(new EncCell(element));
            }
        }
    }
    // "IENCBuoyProductCatalog" and "IENCSouthwestPassProductCatalog" added by .Paul.
    else if( rootName.StartsWith(_T("IENCU37ProductCatalog")) ||
             rootName.StartsWith(_T("IENCBuoyProductCatalog")) ||
             rootName.StartsWith(_T("IENCSouthwestPassProductCatalog")) )
    {
        if( !ParseNoaaHeader(root.first_child()) )
        {
            return false;
        }
        if( headerOnly )
            return true;
        
        for (pugi::xml_node element = root.first_child(); element; element = element.next_sibling()){
            if( !strcmp(element.name(), "Cell") ){
                charts.Add(new IEncCell(element));
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool ChartCatalog::ParseNoaaHeader( const pugi::xml_node &xmldata )
{
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "title") ){
            title = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "date_created")) {
            date_created.ParseDate( wxString::FromUTF8(element.first_child().value()) );
            wxASSERT(date_created.IsValid());
        }
        else if( !strcmp(element.name(), "time_created")) {
            time_created.ParseTime( wxString::FromUTF8(element.first_child().value()) );
            wxASSERT(time_created.IsValid());
        }
        else if( !strcmp(element.name(), "date_valid")) {
            date_valid.ParseDate( wxString::FromUTF8(element.first_child().value()) );
            wxASSERT(time_created.IsValid());
        }
        else if( !strcmp(element.name(), "time_valid")) {
            time_valid.ParseTime( wxString::FromUTF8(element.first_child().value()) );
            wxASSERT(time_created.IsValid());
        }
        else if( !strcmp(element.name(), "dt_valid")) {
            wxStringTokenizer tk( wxString::FromUTF8(element.first_child().value()), _T("TZ") );
            dt_valid.ParseDate(tk.GetNextToken());
            dt_valid.ParseTime(tk.GetNextToken());
            dt_valid.MakeFromTimezone(wxDateTime::UTC);
            wxASSERT(dt_valid.IsValid());
        }
        else if( !strcmp(element.name(), "ref_spec")) {
            ref_spec = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "ref_spec_vers")) {
            ref_spec_vers = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "s62AgencyCode")) {
            s62AgencyCode = wxString::FromUTF8(element.first_child().value());
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

Chart::Chart( pugi::xml_node &xmldata )
{
    coast_guard_districts = new wxArrayString();
    states = new wxArrayString();
    regions = new wxArrayString();
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
    
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "title")) {
            title = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "lname")) {
            title = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "coast_guard_districts")) {
            for (pugi::xml_node subElement = element.first_child(); subElement; subElement = subElement.next_sibling()){
                coast_guard_districts->Add( wxString::FromUTF8(subElement.first_child().value()) );
            }
        }
        else if( !strcmp(element.name(), "states")) {
            for (pugi::xml_node subElement = element.first_child(); subElement; subElement = subElement.next_sibling()){
                states->Add( wxString::FromUTF8(subElement.first_child().value()) );
            }
        }
        else if( !strcmp(element.name(), "regions")) {
            for (pugi::xml_node subElement = element.first_child(); subElement; subElement = subElement.next_sibling()){
                regions->Add( wxString::FromUTF8(subElement.first_child().value()) );
            }
        }
        else if( !strcmp(element.name(), "zipfile_location")) {
            zipfile_location = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "zipfile_datetime")) {
            if( zipfile_datetime.ParseFormat(wxString::FromUTF8(element.first_child().value()), _T("%Y%m%d_%H%M%S")) )
                    zipfile_datetime.MakeFromTimezone(wxDateTime::UTC);
        }
        else if( !strcmp(element.name(), "zipfile_datetime_iso8601")) {
            wxStringTokenizer tk(wxString::FromUTF8(element.first_child().value()), _T("TZ"));
            zipfile_datetime_iso8601.ParseDate(tk.GetNextToken());
            zipfile_datetime_iso8601.ParseTime(tk.GetNextToken());
            zipfile_datetime_iso8601.MakeFromTimezone(wxDateTime::UTC);
        }
        else if( !strcmp(element.name(), "zipfile_size")) {
            zipfile_size = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "cov")) {
            for (pugi::xml_node subElement = element.first_child(); subElement; subElement = subElement.next_sibling()){
                coverage.Add(new Panel(subElement));
            }
        }
        else if( !strcmp(element.name(), "target_filename")) {
            target_filename = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "reference_file")) {
            reference_file = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "manual_download_url")) {
            manual_download_url = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "nm")) {
            // NOT USED
            // nm = new NoticeToMariners(element);
        }
        else if( !strcmp(element.name(), "lnm")) {
            // NOT USED
            // lnm = new NoticeToMariners(element);
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

RasterChart::RasterChart( pugi::xml_node &xmldata ) : Chart( xmldata )
{
    number = wxEmptyString;
    source_edition = -1;
    raster_edition = -1;
    ntm_edition = -1;
    source_date = wxEmptyString;
    ntm_date = wxEmptyString;
    source_edition_last_correction = wxEmptyString;
    raster_edition_last_correction = wxEmptyString;
    ntm_edition_last_correction = wxEmptyString;
    
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "number")) {
            number = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "source_edition")) {
            source_edition = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "raster_edition")) {
            raster_edition = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "ntm_edition")) {
            ntm_edition = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "source_date")) {
            source_date = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "ntm_date")) {
            ntm_date = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "source_edition_last_correction")) {
            source_edition_last_correction = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "raster_edition_last_correction")) {
            raster_edition_last_correction = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "ntm_edition_last_correction")) {
            ntm_edition_last_correction = wxString::FromUTF8(element.first_child().value());
        }
    }
}

EncCell::EncCell( pugi::xml_node &xmldata ) : Chart( xmldata )
{
    number = wxEmptyString;  //  Use number (not name) for zip file name and cell name  .Paul.
    src_chart = wxEmptyString;
    cscale = -1;
    status = wxEmptyString;
    edtn = -1;
    updn = -1;
    uadt = wxInvalidDateTime;
    isdt = wxInvalidDateTime;
    
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "name")) {
            number = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "src_chart")) {
            src_chart = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "cscale")) {
            cscale = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "status")) {
            status = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "edtn")) {
            edtn = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "updn")) {
            updn = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "uadt")) {
            uadt.ParseDateTime(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "isdt")) {
            isdt.ParseDateTime(wxString::FromUTF8(element.first_child().value()));
        }
    }
}

IEncCell::IEncCell( pugi::xml_node &xmldata ) : Chart( xmldata )
{
    //  Use number (not name) for zip file name and cell name  .Paul.
    number = wxEmptyString;
    location = NULL;
    river_name = wxEmptyString;
    river_miles = NULL;
    area = NULL;
    edition = wxEmptyString;
    shp_file = NULL;
    s57_file = NULL;
    kml_file = NULL;
    
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "name")) {
            //  Use number (not name) for zip file name and cell name  .Paul.
            number = wxString::FromUTF8(element.first_child().value());
            zipfile_location = wxString::Format(_T("%s.zip"), number.c_str());
        }
        else if( !strcmp(element.name(), "location")) {
            location = new Location(element);
        }
        else if( !strcmp(element.name(), "river_name")) {
            river_name = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "river_miles")) {
            river_miles = new RiverMiles(element);
        }
        else if( !strcmp(element.name(), "river_miles")) {
            river_miles = new RiverMiles(element);
        }
        else if( !strcmp(element.name(), "area")) {
            area = new Area(element);
        }
        else if( !strcmp(element.name(), "shp_file")) {
            shp_file = new ChartFile(element);
        }
        else if( !strcmp(element.name(), "s57_file")) {
            s57_file = new ChartFile(element);
        }
        else if( !strcmp(element.name(), "kml_file")) {
            kml_file = new ChartFile(element);
        }
        else if( !strcmp(element.name(), "edition")) {
            edition = wxString::FromUTF8(element.first_child().value());
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
// Revised by .Paul. to support IENC catalogs that do not identify rivers or river miles.
{
    if( river_name != wxEmptyString )
    {
        // This formatting of river_name.c_str() ... river_miles->end works for "IENCU37ProductCatalog"
        // where river_name is specified.
        return wxString::Format(_("%s (%s to %s), river miles %3.1f - %3.1f"), river_name.c_str(), location->from.c_str(), location->to.c_str(), river_miles->begin, river_miles->end);
    }
    else
    {
        // Simply use the Cell_name for "IENCBuoyProductCatalog" or "IENCSouthwestPassProductCatalog"
        // where river_name is not specified.
        // Cell_name is in number.c_str()     Cell_name was in name.c_str()
        return wxString::Format(_("%s"), number.c_str());// .Paul.
    }
}

wxString IEncCell::GetDownloadLocation()
{
    return s57_file->location;
}

wxDateTime IEncCell::GetUpdateDatetime()
{
    return s57_file->date_posted;
}

ChartFile::ChartFile( pugi::xml_node &xmldata )
{
    file_size = -1;
    location = wxEmptyString;
    date_posted = wxInvalidDateTime;
    time_posted = wxInvalidDateTime;

    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "location")) {
            location = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "date_posted")) {
            date_posted.ParseDate(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "time_posted")) {
            if(strlen(element.first_child().value()))
                time_posted.ParseTime(wxString::FromUTF8(element.first_child().value()));
            else
                time_posted.ParseTime(_T("00:00:00"));
        }
        else if( !strcmp(element.name(), "file_size")) {
            if(strlen(element.first_child().value()))
                file_size = wxAtoi(wxString::FromUTF8(element.first_child().value()));
            else
                file_size = -1;
        }
    }
}

Area::Area( pugi::xml_node &xmldata )
{
    north = 0.0;
    south = 0.0;
    east = 0.0;
    west = 0.0;
    
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "north")) {
                north = wxAtof(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "south")) {
                south = wxAtof(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "east")) {
                east = wxAtof(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "west")) {
                west = wxAtof(wxString::FromUTF8(element.first_child().value()));
        }
    }
}

RiverMiles::RiverMiles( pugi::xml_node &xmldata )
{
    begin = -1;
    end = -1;
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "begin")) {
                begin = wxAtof(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "end")) {
                end = wxAtof(wxString::FromUTF8(element.first_child().value()));
        }
    }
}

Location::Location( pugi::xml_node &xmldata )
{
    from = wxEmptyString;
    to = wxEmptyString;
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "from")) {
            from = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "to")) {
            to = wxString::FromUTF8(element.first_child().value());
        }
    }
}


NoticeToMariners::NoticeToMariners( pugi::xml_node &xmldata )
{
    agency = wxEmptyString;
    doc = wxEmptyString;
    date = wxInvalidDateTime;
    
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "nm_agency")) {
            agency = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "lnm_agency")) {
            agency = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "doc")) {
            doc = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "date")) {
            date.ParseDate(wxString::FromUTF8(element.first_child().value()));
        }
    }
}

Panel::Panel( pugi::xml_node &xmldata )
{
    panel_no = -1;
    
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "panel_no")) {
                panel_no = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
        else if( !strcmp(element.name(), "vertex")) {
            // NOT USED
            //vertexes.Add(new Vertex(element));
        }
    }
}

Panel::~Panel()
{
}

RncPanel::RncPanel( pugi::xml_node &xmldata ) : Panel( xmldata )
{
    panel_title = wxEmptyString;
    file_name = wxEmptyString;
    scale = 0;
    
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "panel_title")) {
            panel_title = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "file_name")) {
            file_name = wxString::FromUTF8(element.first_child().value());
        }
        else if( !strcmp(element.name(), "scale")) {
            scale = wxAtoi(wxString::FromUTF8(element.first_child().value()));
        }
    }
}

EncPanel::EncPanel( pugi::xml_node &xmldata ) : Panel( xmldata )
{
    type = wxEmptyString;
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "type")) {
            type = wxString::FromUTF8(element.first_child().value());
        }
    }
}

Vertex::Vertex( pugi::xml_node &xmldata )
{
    //Init properties
    lat = 999.0;
    lon = 999.0;
    for (pugi::xml_node element = xmldata.first_child(); element; element = element.next_sibling()){
        if( !strcmp(element.name(), "lat")) {
            wxString::FromUTF8(element.first_child().value()).ToDouble(&lat);
        }
        else if( !strcmp(element.name(), "lon")) {
            wxString::FromUTF8(element.first_child().value()).ToDouble(&lon);
        }
    }
}
