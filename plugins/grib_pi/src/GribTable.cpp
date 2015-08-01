/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB table
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/glcanvas.h>
#include <wx/graphics.h>
#include <wx/progdlg.h>
#include "GribTable.h"

extern double m_cursor_lat, m_cursor_lon;
//----------------------------------------------------------------------------------------------------------
//          GRIB Table Implementation
//----------------------------------------------------------------------------------------------------------

GRIBTable::GRIBTable(GRIBUICtrlBar &parent)
    : GRIBTableBase(&parent), m_pGDialog(&parent){}

void GRIBTable::InitGribTable( int zone, ArrayOfGribRecordSets *rsa )
{
    //init fonts and colours
    wxFont dayfont = wxFont( 11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
    wxFont labelfont = wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
    wxFont timefont = wxFont( 9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
    wxFont datafont = wxFont( 9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );

    wxColour colour;
    GetGlobalColor(_T("DILG1"), &colour);
   
    //populate "cursor position" display 
    wxString l;
    l.Append(toSDMM_PlugIn(1, m_cursor_lat)).Append(_T("   "))
        .Append(toSDMM_PlugIn(2, m_cursor_lon));
    m_pCursorPosition->SetLabel(l);
    m_pCursorPosition->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    m_pPositionText->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    //init row attr
    wxGridCellAttr *daysrow = new wxGridCellAttr();
    daysrow->SetFont(dayfont);
    daysrow->SetBackgroundColour(colour);
    daysrow->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);

    wxGridCellAttr *timerow = new wxGridCellAttr();
    timerow->SetFont(timefont);
    timerow->SetBackgroundColour(colour);
    timerow->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);

    wxGridCellAttr *singledatarow = new wxGridCellAttr();
    singledatarow->SetFont(datafont);
    singledatarow->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);

    wxGridCellAttr *doubledatarow = new wxGridCellAttr();
    doubledatarow->SetFont(datafont);
    doubledatarow->SetAlignment(wxALIGN_CENTRE, -1);

    //init labels
    m_pGribTable->SetLabelFont(labelfont);                  
    m_pGribTable->SetLabelBackgroundColour(colour);

    //create as columns as necessary
    m_pGribTable->AppendCols(rsa->GetCount());
    m_pGribTable->SetSelectionBackground(colour);
    m_pGribTable->SetSelectionForeground(m_pGribTable->GetDefaultCellTextColour());

    //init days row
    m_pGribTable->SetRowLabelValue(0, wxEmptyString);
    m_pGribTable->SetRowAttr(0, daysrow);

    //init time row
    m_pGribTable->SetRowLabelValue(1, wxEmptyString);
    m_pGribTable->SetRowAttr(1, timerow);

    //populate grib
    wxDateTime day(rsa->Item(0).m_Reference_Time);
    wxDateTime time;
    int ncols = -1,nrows,dcol = 0;

    for(unsigned i = 0; i < rsa->GetCount(); i++ ) {
        time = rsa->Item(i).m_Reference_Time;
   
        //populate 'time' row
        m_pGribTable->SetCellValue(1, i, GetTimeRowsStrings( rsa->Item(i).m_Reference_Time, zone , 0) );
        
        nrows = 2;

        m_pTimeset = m_pGDialog->GetTimeLineRecordSet(time);
        GribRecord **RecordArray = m_pTimeset->m_GribRecordPtrArray;

        //create and polulate wind data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VX) != wxNOT_FOUND &&
            m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VY) != wxNOT_FOUND) {
                AddDataRow( nrows, i, _("Wind"), doubledatarow );
                wxString wc(GetWind(RecordArray));
                //wind is a special case: if current unit is not bf ==> double speed display (current unit + bf)
                if(m_pGDialog->m_OverlaySettings.Settings[GribOverlaySettings::WIND].m_Units != GribOverlaySettings::BFS)
                    if(!wc.IsEmpty()) wc.Append(_T("\n")).Append(GetWindBf(RecordArray));
                //
                m_pGribTable->SetCellValue(nrows, i, wc);
                m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
                nrows++;
        }
        //create and polulate Pressure data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRESSURE) != wxNOT_FOUND) {
            AddDataRow( nrows, i, _("Pressure"), singledatarow );
            m_pGribTable->SetCellValue(nrows, i, GetPressure(RecordArray));
            nrows++;
        }
        //create and polulate Wind gusts data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_GUST) != wxNOT_FOUND) {
            AddDataRow( nrows, i, _("Wind Gust"), singledatarow );
            m_pGribTable->SetCellValue(nrows, i, GetWindGust(RecordArray));
            m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
            nrows++;
        }
        //create and polulate Waves data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_HTSIGW) != wxNOT_FOUND ||
            m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVDIR) != wxNOT_FOUND) {
                AddDataRow( nrows, i, _("Waves"), doubledatarow );
                m_pGribTable->SetCellValue(nrows, i, GetWaves(RecordArray));
                m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
                nrows++;
        }
        //create and polulate total rainfall data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRECIP_TOT) != wxNOT_FOUND) {
            AddDataRow( nrows, i, _("Rainfall"), singledatarow );
            m_pGribTable->SetCellValue(nrows, i, GetRainfall(RecordArray));
            m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
            nrows++;
        }
        //create and polulate total cloud control
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CLOUD_TOT) != wxNOT_FOUND) {
            AddDataRow( nrows, i, _("Cloud Cover"), singledatarow );
            m_pGribTable->SetCellValue(nrows, i, GetCloudCover(RecordArray));
            m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
            nrows++;
        }
        //create and polulate the Air Temperature data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_AIR_TEMP) != wxNOT_FOUND) {
            AddDataRow( nrows, i, _("Air\nTemperature"), singledatarow );
            m_pGribTable->SetCellValue(nrows, i, GetAirTemp(RecordArray));
            m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
            nrows++;
        }
        //create and polulate the Sea Surface Temperature data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEA_TEMP) != wxNOT_FOUND) {
            AddDataRow( nrows, i, _("Sea\nTemperature"), singledatarow );
            m_pGribTable->SetCellValue(nrows, i, GetSeaTemp(RecordArray));
            m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
            nrows++;
        }
        //create and polulate the Convective Available Potential Energy (CAPE) data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CAPE) != wxNOT_FOUND) {
            AddDataRow( nrows, i, _("CAPE"), singledatarow );
            m_pGribTable->SetCellValue(nrows, i, GetCAPE(RecordArray));
            m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
            nrows++;
        }
        //create and polulate the current data row
        if(m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEACURRENT_VX) != wxNOT_FOUND &&
            m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEACURRENT_VY) != wxNOT_FOUND) {
                AddDataRow( nrows, i, _("Current"), doubledatarow );
                m_pGribTable->SetCellValue(nrows, i, GetCurrent(RecordArray));
                m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
                nrows++;
        }

        m_pGribTable->AutoSizeColumn(i, false);
        ncols++;

        //write 'days' row
        if(time.GetDateOnly() != day.GetDateOnly() || i == rsa->GetCount()- 1){
            if( i == 0 ) continue;                                      //not the first item

            if(i == rsa->GetCount() - 1 && ncols != 1) ncols++;         ////if end of time range don't forgett the last col

            m_pGribTable->SetCellSize(0, dcol, 1, ncols);
            m_pGribTable->SetCellValue(0, dcol, GetTimeRowsStrings(day, zone, 1));

            day = rsa->Item(i).m_Reference_Time;
            dcol = i;

            if( ncols == 1){                                            //if only one item per day
                m_pGribTable->AutoSizeColumn(i-1, false);
                if(i == rsa->GetCount() - 1 ) {                         //if end of time range
                    m_pGribTable->SetCellValue(0, i, GetTimeRowsStrings(day, zone, 1));
                    m_pGribTable->AutoSizeColumn(i, false);
                }
            }  
            ncols = 0;
        }
    }
    AutoSizeDataRows();
    m_pGribTable->SetGridCursor( m_pGribTable->GetNumberRows(), 0);     //put cursor outside the grid

    this->Fit();
    this->Refresh();
}

void GRIBTable::OnClose( wxCloseEvent& event )
{
    CloseDialog();
    this->EndModal(wxID_OK);
}

void GRIBTable::OnOKButton( wxCommandEvent& event )
{
    CloseDialog();
    this->EndModal(wxID_OK);
}

void GRIBTable::OnClick( wxGridEvent& event )
{
    if(m_pGribTable->IsSelection()) m_pGribTable->ClearSelection();
}

void GRIBTable::OnRangeClick( wxGridRangeSelectEvent& event )
{
    if(m_pGribTable->IsSelection()) m_pGribTable->ClearSelection();
}

void GRIBTable::CloseDialog()
{
    int x,y,w,h;
    this->GetPosition(&x, &y);
    this->GetSize(&w, &h);

    wxFileConfig *pConf = GetOCPNConfigObject();

    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/GRIB" ) );

        pConf->Write( _T ( "GribDataTablePosition_x" ), x);
        pConf->Write( _T ( "GribDataTablePosition_y" ), y);
        pConf->Write( _T ( "GribDataTableWidth" ), w);
        pConf->Write( _T ( "GribDataTableHeight" ), h);
    }
}

void GRIBTable::AddDataRow( int num_rows, int num_cols, wxString label, wxGridCellAttr *row_attr )
{
    if(m_pGribTable->GetNumberRows() == num_rows) {
        m_pGribTable->AppendRows(1);
        m_pGribTable->SetRowLabelValue(num_rows, label);
        m_pGribTable->SetRowAttr(num_rows, row_attr);
    }
    m_pDataCellsColour = m_pGribTable->GetCellBackgroundColour(num_rows, num_cols);  //set default colour
}

void GRIBTable::AutoSizeDataRows()
{
    for( int i = 0; i < m_pGribTable->GetNumberRows(); i++ ) {
        m_pGribTable->AutoSizeRow(i, false);
        int h = m_pGribTable->GetRowHeight(i);
        h += 10;
        m_pGribTable->SetRowSize(i, h);
    }
}

wxString GRIBTable::GetWind(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    int altitude = 0;
    double vkn, ang;
    if(GribRecord::getInterpolatedValues(vkn, ang,
                                         recordarray[Idx_WIND_VX + altitude],
                                         recordarray[Idx_WIND_VY + altitude],
                                         m_cursor_lon, m_cursor_lat)) {

       skn.Printf( _T("%03d\u00B0"), (int) ( ang ) );

       skn.Append(_T("\n\n"));

       vkn = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND, vkn);
       skn.Append(wxString::Format( _T("%3d ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WIND) , (int) wxRound( vkn )));
       m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::WIND, vkn);
    }
    return skn;
}

wxString GRIBTable::GetWindBf(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    int altitude = 0;
    double vkn, ang;
    if(GribRecord::getInterpolatedValues(vkn, ang,
                                         recordarray[Idx_WIND_VX + altitude],
                                         recordarray[Idx_WIND_VY + altitude],
                                         m_cursor_lon, m_cursor_lat)) {
       vkn = m_pGDialog->m_OverlaySettings.GetmstobfFactor(vkn)* vkn;
       skn.Printf( wxString::Format( _T("%2d bf"), (int)wxRound( vkn )) );
    }
    return skn;
}

wxString GRIBTable::GetPressure(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    if( recordarray[Idx_PRESSURE] ) {
        double press = recordarray[Idx_PRESSURE]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( press != GRIB_NOTDEF ) {
            press = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::PRESSURE, press);
            int p = (m_pGDialog->m_OverlaySettings.Settings[GribOverlaySettings::PRESSURE].m_Units == 2) ? 2 : 0;  // if PRESSURE & inHG = two decimals
            skn.Printf( wxString::Format(_T("%2.*f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRESSURE), p, ( press )) );
        }
    }
    return skn;
}

wxString GRIBTable::GetWindGust(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    if( recordarray[Idx_WIND_GUST] ) {
        double vkn = recordarray[Idx_WIND_GUST]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

            if( vkn != GRIB_NOTDEF ) {
                vkn = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::WIND_GUST, vkn);
                skn.Printf( wxString::Format(_T("%2d ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WIND_GUST), (int) wxRound( vkn )) );
                m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::WIND_GUST, vkn);
            }
    }
    return skn;
}

wxString GRIBTable::GetWaves(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    if( recordarray[Idx_HTSIGW] ) {
        double height = recordarray[Idx_HTSIGW]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
        if( height != GRIB_NOTDEF ) {
            height = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::WAVE, height);
            skn.Printf( wxString::Format( _T("%4.1f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::WAVE), height ));
            m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::WAVE, height);

            if( recordarray[Idx_WVDIR] ) {
                double direction = recordarray[Idx_WVDIR]->
                    getInterpolatedValue(m_cursor_lon, m_cursor_lat, true, true );
                if( direction != GRIB_NOTDEF ){
                    skn.Prepend(wxString::Format( _T("%03d\u00B0\n\n"), (int)direction ));
                   
                    if( recordarray[Idx_WVPER] ) {
                        double period = recordarray[Idx_WVPER]->
                            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );
                        if( period != GRIB_NOTDEF ) {
                            skn.Append( wxString::Format( _T("\n%01ds") , (int) (period + 0.5)) );
                        }
                    }
                }
            }
        }
    }
    return skn;
}

wxString GRIBTable::GetRainfall(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    if( recordarray[Idx_PRECIP_TOT] ) {
        double precip = recordarray[Idx_PRECIP_TOT]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( precip != GRIB_NOTDEF ) {
            precip = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::PRECIPITATION, precip);
            skn.Printf( _T("%6.2f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::PRECIPITATION), precip );
            m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::PRECIPITATION, precip);
        }
    }
    return skn;
}

wxString GRIBTable::GetCloudCover(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    if( recordarray[Idx_CLOUD_TOT] ) {
        double cloud = recordarray[Idx_CLOUD_TOT]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( cloud != GRIB_NOTDEF ) {
            cloud = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::CLOUD, cloud);
            skn.Printf( _T("%5.1f "), cloud );
            skn.Append(m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CLOUD));
            m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::CLOUD, cloud);
        }
    }
    return skn;
}

wxString GRIBTable::GetAirTemp(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    if( recordarray[Idx_AIR_TEMP] ) {
        double temp = recordarray[Idx_AIR_TEMP]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( temp != GRIB_NOTDEF ) {
            temp = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::AIR_TEMPERATURE, temp);
            skn.Printf( _T("%5.1f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::AIR_TEMPERATURE), temp );
            m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::AIR_TEMPERATURE, temp);
        }
    }
    return skn;
}

wxString GRIBTable::GetSeaTemp(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    if( recordarray[Idx_SEA_TEMP] ) {
        double temp = recordarray[Idx_SEA_TEMP]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( temp != GRIB_NOTDEF ) {
            temp = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::SEA_TEMPERATURE, temp);
            skn.Printf( _T("%5.1f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::SEA_TEMPERATURE), temp );
            m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::SEA_TEMPERATURE, temp);
        }
    }
    return skn;
}

wxString GRIBTable::GetCAPE(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    if( recordarray[Idx_CAPE] ) {
        double cape = recordarray[Idx_CAPE]->
            getInterpolatedValue(m_cursor_lon, m_cursor_lat, true );

        if( cape != GRIB_NOTDEF ) {
            cape = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::CAPE, cape);
            skn.Printf( wxString::Format( _T("%5.0f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CAPE), cape ) );
            m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::CAPE, cape);
        }
    }
    return skn;
}


wxString GRIBTable::GetCurrent(GribRecord **recordarray)
{
    wxString skn(wxEmptyString);
    double vkn, ang;
    if(GribRecord::getInterpolatedValues(vkn, ang,
                                         recordarray[Idx_SEACURRENT_VX],
                                         recordarray[Idx_SEACURRENT_VY],
                                         m_cursor_lon, m_cursor_lat)) {
        vkn = m_pGDialog->m_OverlaySettings.CalibrateValue(GribOverlaySettings::CURRENT, vkn);

        skn.Printf( _T("%03d\u00B0"), (int) ( ang ) );
        
        skn.Append(_T("\n\n"));
        
        skn.Append( wxString::Format( _T("%4.1f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(GribOverlaySettings::CURRENT), vkn ) );
        m_pDataCellsColour = m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(GribOverlaySettings::CURRENT, vkn);
    }
    return skn;
}

wxString GRIBTable::GetTimeRowsStrings( wxDateTime date_time, int time_zone, int type )
{
    wxDateTime t( date_time );
    switch( time_zone ) {
        case 0:
			if( (wxDateTime::Now() == (wxDateTime::Now().ToGMT())) && t.IsDST() )  //bug in wxWingets 3.0 for UTC meridien ?
				t.Add( wxTimeSpan( 1, 0, 0, 0 ) );
            switch( type ){
            case 0:
                return t.Format( _T(" %H:%M  "), wxDateTime::Local ) + _T("LOC");
            case 1:
                return t.Format( _T(" %a-%d-%b-%Y  "), wxDateTime::Local);
            }
        case 1:
            switch( type ){
            case 0:
                return t.Format( _T(" %H:%M  "), wxDateTime::UTC ) + _T("UTC");
            case 1:
                return t.Format( _T(" %a-%d-%b-%Y  "), wxDateTime::UTC );
            }
        default:
            return wxEmptyString;
    }
}
