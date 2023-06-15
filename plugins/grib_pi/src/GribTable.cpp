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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "pi_gl.h"

#include <wx/glcanvas.h>
#include <wx/graphics.h>
#include <wx/progdlg.h>
#include <wx/grid.h>
#include "GribTable.h"
#include "CustomGrid.h"
#include <wx/tokenzr.h>

extern double m_cursor_lat, m_cursor_lon;

//----------------------------------------------------------------------------------------------------------
//          GRIB Table Implementation
//----------------------------------------------------------------------------------------------------------

GRIBTable::GRIBTable(GRIBUICtrlBar &parent)
    : GRIBTableBase(&parent), m_pGDialog(&parent) {}

void GRIBTable::InitGribTable(int zone, ArrayOfGribRecordSets *rsa,
                              int NowIndex) {
  m_pGribTable->m_gParent = this;
  m_pIndex = NowIndex;

  // init row attr
  wxGridCellAttr *datarow = new wxGridCellAttr();
  datarow->SetFont(GetOCPNGUIScaledFont_PlugIn(_T("Dialog")));
  datarow->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);

  // populate "cursor position" display
  wxString l;
  l.Append(toSDMM_PlugIn(1, m_cursor_lat))
      .Append(_T("   "))
      .Append(toSDMM_PlugIn(2, m_cursor_lon));
  m_pCursorPosition->SetLabel(l);
  m_pCursorPosition->SetFont(
      GetOCPNGUIScaledFont_PlugIn(_T("Dialog")).MakeBold());
  m_pPositionText->SetFont(
      GetOCPNGUIScaledFont_PlugIn(_T("Dialog")).MakeBold());

  // create as columns as necessary
  m_pGribTable->AppendCols(rsa->GetCount());

  // populate grid
  wxDateTime time, day, ptime;
  int nrows, wcols = 0;
  for (unsigned i = 0; i < rsa->GetCount(); i++) {
    // populate time labels
    time = rsa->Item(i).m_Reference_Time;
    m_pGribTable->SetColLabelValue(
        i, GetTimeRowsStrings(time, zone, 1)
               .Append(_T("\n"))
               .Append(
                   GetTimeRowsStrings(rsa->Item(i).m_Reference_Time, zone, 0)));
    nrows = -1;
    GribTimelineRecordSet *pTimeset = m_pGDialog->GetTimeLineRecordSet(time);
    if (pTimeset == 0) continue;

    GribRecord **RecordArray = pTimeset->m_GribRecordPtrArray;

    /*create and polulate wind data row
         wind is a special case:
         1) if current unit is not bf ==> double speed display (current unit +
       bf) 2) create two lines for direction and speed and a third for gust if
       exists 3) these two or three lines will be part of the same block*/
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VX) !=
            wxNOT_FOUND &&
        m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_VY) !=
            wxNOT_FOUND) {
      nrows++;
      AddDataRow(nrows, i, _("Wind,Dir"), datarow);
      double wdir;
      GetWind(RecordArray, 1, wdir);  // this is a numerical row
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
      m_pGribTable->SetNumericalRow(nrows, i, R_WIND, wdir);
      nrows++;
      AddDataRow(nrows, i, _("Wind,Speed"), datarow);
      if (m_pGDialog->m_OverlaySettings.Settings[GribOverlaySettings::WIND]
              .m_Units != GribOverlaySettings::BFS)
        m_pGribTable->SetCellValue(nrows, i, GetWind(RecordArray, 2, wdir));
      else
        m_pGribTable->SetCellValue(nrows, i, GetWind(RecordArray, 3, wdir));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
      if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WIND_GUST) !=
          wxNOT_FOUND) {
        nrows++;
        AddDataRow(nrows, i, _("Wind,Gust"), datarow);
        if (m_pGDialog->m_OverlaySettings.Settings[GribOverlaySettings::WIND]
                .m_Units != GribOverlaySettings::BFS)
          m_pGribTable->SetCellValue(nrows, i, GetWindGust(RecordArray, 1));
        else
          m_pGribTable->SetCellValue(nrows, i, GetWindGust(RecordArray, 2));
        m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
      }
    }  // wind

    // create and polulate Pressure data rown
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRESSURE) !=
        wxNOT_FOUND) {
      nrows++;
      AddDataRow(nrows, i, _("Pressure"), datarow);
      m_pGribTable->SetCellValue(nrows, i, GetPressure(RecordArray));
    }  // pressure

    /*create and polulate Waves data rows
        waves is another special case:
         1) if significant height data exists then create a line for direction
       and height then a third for periode if data exists
         2) these two or three lines will be part of the same block*/
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_HTSIGW) !=
        wxNOT_FOUND) {
      double wdir;
      if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVDIR) !=
          wxNOT_FOUND) {
        nrows++;
        AddDataRow(nrows, i, _("Waves,Dir"), datarow);
        GetWaves(RecordArray, Idx_WVDIR, wdir);  // this is a numerical row
        m_pGribTable->SetNumericalRow(nrows, i, R_WAVES, wdir);
      }
      nrows++;
      AddDataRow(nrows, i, _("Waves,Hsig"), datarow);
      m_pGribTable->SetCellValue(nrows, i,
                                 GetWaves(RecordArray, Idx_HTSIGW, wdir));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
      if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_WVPER) !=
          wxNOT_FOUND) {
        nrows++;
        AddDataRow(nrows, i, _("Waves,Per"), datarow);
        m_pGribTable->SetCellValue(nrows, i,
                                   GetWaves(RecordArray, Idx_WVPER, wdir));
      }
    }  // waves

    // create and polulate total rainfall data row
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_PRECIP_TOT) !=
        wxNOT_FOUND) {
      nrows++;
      AddDataRow(nrows, i, _("Rainfall"), datarow);
      m_pGribTable->SetCellValue(nrows, i, GetRainfall(RecordArray));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
    }  // rainfall

    // create and populate total cloud control
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CLOUD_TOT) !=
        wxNOT_FOUND) {
      nrows++;
      AddDataRow(nrows, i, _("Cloud Cover"), datarow);
      m_pGribTable->SetCellValue(nrows, i, GetCloudCover(RecordArray));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
    }  // cloud

    // create and polulate the Air Temperature data row
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_AIR_TEMP) !=
        wxNOT_FOUND) {
      nrows++;
      AddDataRow(nrows, i, _("Air Temp."), datarow);
      m_pGribTable->SetCellValue(nrows, i, GetAirTemp(RecordArray));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
    }  // air temp

    // create and polulate the Sea Surface Temperature data row
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_SEA_TEMP) !=
        wxNOT_FOUND) {
      nrows++;
      AddDataRow(nrows, i, _("Sea Temp."), datarow);
      m_pGribTable->SetCellValue(nrows, i, GetSeaTemp(RecordArray));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
    }  // sea temp

    // create and polulate the Convective Available Potential Energy (CAPE) data
    // row
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_CAPE) !=
        wxNOT_FOUND) {
      nrows++;
      AddDataRow(nrows, i, _("CAPE"), datarow);
      m_pGribTable->SetCellValue(nrows, i, GetCAPE(RecordArray));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
    }  // cape

    // create and polulate the Composite Reflectivity data row
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(Idx_COMP_REFL) !=
        wxNOT_FOUND) {
      nrows++;
      AddDataRow(nrows, i, _("C. Reflect."), datarow);
      m_pGribTable->SetCellValue(nrows, i, GetCompRefl(RecordArray));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
    }  // composite Reflectivity

    /*create and populate the current data rows
        1)create two lines for direstion and speed
        2) these two or three lines will be part of the same block*/
    if (m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(
            Idx_SEACURRENT_VX) != wxNOT_FOUND &&
        m_pGDialog->m_bGRIBActiveFile->m_GribIdxArray.Index(
            Idx_SEACURRENT_VY) != wxNOT_FOUND) {
      double wdir;
      nrows++;
      AddDataRow(nrows, i, _("Current,Dir"), datarow);
      GetCurrent(RecordArray, 1, wdir);  // this is a numerical row
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
      m_pGribTable->SetNumericalRow(nrows, i, R_CURRENT, wdir);

      nrows++;
      AddDataRow(nrows, i, _("Current,Speed"), datarow);
      m_pGribTable->SetCellValue(nrows, i, GetCurrent(RecordArray, 2, wdir));
      m_pGribTable->SetCellBackgroundColour(nrows, i, m_pDataCellsColour);
    }  // current // populate grid
    delete pTimeset;
    m_pGribTable->AutoSizeColumn(i, false);
    wcols = wxMax(m_pGribTable->GetColSize(i), wcols);
  }  // populate grid
  // put cursor outside the grid
  m_pGribTable->SetGridCursor(m_pGribTable->GetNumberRows() - 1, 0);
  // set col size
  m_pGribTable->SetDefaultColSize(wcols, true);
  // set row size
  AutoSizeDataRows();
  // set label size
  m_pGribTable->SetColLabelSize(wxGRID_AUTOSIZE);
  // set scroll steps
  m_pGribTable->SetScrollLineX(wcols);

  datarow->DecRef();  // Give up pointer contrl to Grid

  m_tScrollToNowTimer.Connect(
      wxEVT_TIMER, wxTimerEventHandler(GRIBTable::OnScrollToNowTimer), NULL,
      this);
}

void GRIBTable::SetTableSizePosition(int vpWidth, int vpHeight) {
  // Get size & position previously saved
  int x = -1, y = -1, w = -1, h = -1;
  wxFileConfig *pConf = GetOCPNConfigObject();
  if (pConf) {
    pConf->SetPath(_T("/Settings/GRIB"));
    pConf->Read(_T("GribDataTablePosition_x"), &x);
    pConf->Read(_T("GribDataTablePosition_y"), &y);
    pConf->Read(_T("GribDataTableWidth"), &w);
    pConf->Read(_T("GribDataTableHeight"), &h);
  }
  wxPoint final_pos = GetOCPNCanvasWindow()->ClientToScreen(wxPoint(x, y));
  // set a default size & position if saved values are outside of limits
  bool refit = false;
  wxSize scw = wxSize(vpWidth, (vpHeight - GetChartbarHeight()));
  wxPoint pcw = GetOCPNCanvasWindow()->ClientToScreen(
      GetOCPNCanvasWindow()->GetPosition());
  if (!(wxRect(pcw, scw).Contains(wxRect(final_pos, wxSize(w, h)))))
    refit = true;
  if (w < (m_pGribTable->GetRowLabelSize() + (m_pGribTable->GetColSize(0))) ||
      h < (m_pGribTable->GetColLabelSize() + (m_pGribTable->GetRowSize(0))))
    refit = true;

#ifdef __ANDROID__
  refit = true;
#endif
  if (refit) {
    w = (scw.GetWidth() / 10) * 9;  // 10% less than canvas
    h = (scw.GetHeight() / 10) * 9;
    x = (scw.GetWidth() / 20);   // centered horizontally
    y = (scw.GetHeight() / 50);  // a bit out-centered toward the top
#ifdef __ANDROID__
    // Position directly below GRIB control dialog
    y = m_pGDialog->GetSize().GetHeight() * 11 / 10;
    h = scw.GetHeight() - m_pGDialog->GetSize().GetHeight();
#endif
    final_pos = GetOCPNCanvasWindow()->ClientToScreen(wxPoint(x, y));
  }  //
  // in case client size too large for the grib
  int w1 = m_pGribTable->GetRowLabelSize() +
           (m_pGribTable->GetColSize(0) * m_pGribTable->GetNumberCols());
  w = wxMin(w, w1);
  int h1 = m_pGribTable->GetColLabelSize() +
           (m_pGribTable->GetRowSize(0) * (m_pGribTable->GetNumberRows() + 4));
  h = wxMin(h, h1);

  this->SetClientSize(w, h);
  this->Move(final_pos);

  // scroll to the end of time steps to prepare the next scroll to 'now'
  m_pGribTable->MakeCellVisible(0, m_pGribTable->GetNumberCols() - 1);
  // trigger scroll to 'now'(must be postpone after the dialog is shown)
  m_tScrollToNowTimer.Start(200, wxTIMER_ONE_SHOT);
}

void GRIBTable::OnScrollToNowTimer(wxTimerEvent &event) {
  m_pGribTable->MakeCellVisible(0, m_pIndex);
}

void GRIBTable::OnClose(wxCloseEvent &event) {
  CloseDialog();
  this->EndModal(wxID_OK);
}

void GRIBTable::OnOKButton(wxCommandEvent &event) {
  CloseDialog();
  this->EndModal(wxID_OK);
}

void GRIBTable::CloseDialog() {
  wxSize s = this->GetClientSize();
  wxPoint p = GetOCPNCanvasWindow()->ScreenToClient(this->GetPosition());

  wxFileConfig *pConf = GetOCPNConfigObject();
  if (pConf) {
    pConf->SetPath(_T ( "/Settings/GRIB" ));

    pConf->Write(_T ( "GribDataTablePosition_x" ), p.x);
    pConf->Write(_T ( "GribDataTablePosition_y" ), p.y);
    pConf->Write(_T ( "GribDataTableWidth" ), s.GetWidth());
    pConf->Write(_T ( "GribDataTableHeight" ), s.GetHeight());
  }  //
}

void GRIBTable::AddDataRow(int num_rows, int num_cols, wxString label,
                           wxGridCellAttr *row_attr) {
  if (m_pGribTable->GetNumberRows() == num_rows) {
    m_pGribTable->AppendRows(1);
    m_pGribTable->SetRowLabelValue(num_rows, label);
    row_attr->IncRef();
    m_pGribTable->SetRowAttr(num_rows, row_attr);
  }
  m_pDataCellsColour = m_pGribTable->GetCellBackgroundColour(
      num_rows, num_cols);  // set default colour
}

void GRIBTable::AutoSizeDataRows() {
  int hrows = 0;
  for (int i = 0; i < m_pGribTable->GetNumberRows(); i++) {
    m_pGribTable->AutoSizeRow(i, false);
    hrows = wxMax(m_pGribTable->GetRowHeight(i) + 3, hrows);
  }
  m_pGribTable->SetDefaultRowSize(hrows, true);
  // set scroll steps
  m_pGribTable->SetScrollLineY(hrows);
}

wxString GRIBTable::GetWind(GribRecord **recordarray, int datatype,
                            double &wdir) {
  wxString skn(wxEmptyString);
  int altitude = 0;
  double vkn, ang;
  wdir = GRIB_NOTDEF;
  if (GribRecord::getInterpolatedValues(
          vkn, ang, recordarray[Idx_WIND_VX + altitude],
          recordarray[Idx_WIND_VY + altitude], m_cursor_lon, m_cursor_lat)) {
    if (datatype == 1) {
      wdir = ang;
      return skn;
    }
    double cvkn = m_pGDialog->m_OverlaySettings.CalibrateValue(
        GribOverlaySettings::WIND, vkn);
    m_pDataCellsColour =
        m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
            GribOverlaySettings::WIND, cvkn);
    skn.Printf(wxString::Format(
        _T("%2d bf"),
        (int)wxRound(m_pGDialog->m_OverlaySettings.GetmstobfFactor(vkn) *
                     vkn)));
    if (datatype == 2) {  // wind speed unit other than bf
      skn.Prepend(wxString::Format(
                      _T("%2d ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                                       GribOverlaySettings::WIND),
                      (int)wxRound(cvkn)) +
                  _T(" - "));
    }
  }
  return skn;
}

wxString GRIBTable::GetWindGust(GribRecord **recordarray, int datatype) {
  wxString skn(wxEmptyString);
  if (recordarray[Idx_WIND_GUST]) {
    double vkn = recordarray[Idx_WIND_GUST]->getInterpolatedValue(
        m_cursor_lon, m_cursor_lat, true);
    if (vkn != GRIB_NOTDEF) {
      double cvkn = m_pGDialog->m_OverlaySettings.CalibrateValue(
          GribOverlaySettings::WIND_GUST, vkn);
      m_pDataCellsColour =
          m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
              GribOverlaySettings::WIND_GUST, cvkn);

      skn.Printf(wxString::Format(
          _T("%2d bf"),
          (int)wxRound(m_pGDialog->m_OverlaySettings.GetmstobfFactor(vkn) *
                       vkn)));  // wind gust unit bf
      if (datatype == 1) {      // wind gust unit other than bf
        skn.Prepend(
            wxString::Format(
                _T("%2d ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                                 GribOverlaySettings::WIND_GUST),
                (int)wxRound(cvkn)) +
            _T(" - " ));
      }
    }
  }
  return skn;
}

wxString GRIBTable::GetPressure(GribRecord **recordarray) {
  wxString skn(wxEmptyString);
  if (recordarray[Idx_PRESSURE]) {
    double press = recordarray[Idx_PRESSURE]->getInterpolatedValue(
        m_cursor_lon, m_cursor_lat, true);

    if (press != GRIB_NOTDEF) {
      press = m_pGDialog->m_OverlaySettings.CalibrateValue(
          GribOverlaySettings::PRESSURE, press);
      int p =
          (m_pGDialog->m_OverlaySettings.Settings[GribOverlaySettings::PRESSURE]
               .m_Units == 2)
              ? 2
              : 0;  // if PRESSURE & inHG = two decimals
      skn.Printf(wxString::Format(
          _T("%2.*f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                             GribOverlaySettings::PRESSURE),
          p, (press)));
    }
  }
  return skn;
}

wxString GRIBTable::GetWaves(GribRecord **recordarray, int datatype,
                             double &wdir) {
  wxString skn(wxEmptyString);
  wdir = GRIB_NOTDEF;
  switch (datatype) {
    case Idx_HTSIGW:
      if (recordarray[Idx_HTSIGW]) {
        double height = recordarray[Idx_HTSIGW]->getInterpolatedValue(
            m_cursor_lon, m_cursor_lat, true);
        if (height != GRIB_NOTDEF) {
          double cheight = m_pGDialog->m_OverlaySettings.CalibrateValue(
              GribOverlaySettings::WAVE, height);
          skn.Printf(wxString::Format(
              _T("%4.1f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                                 GribOverlaySettings::WAVE),
              cheight));
          m_pDataCellsColour =
              m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
                  GribOverlaySettings::WAVE, cheight);
        }
      }
      break;
    case Idx_WVDIR:
      if (recordarray[Idx_WVDIR]) {
        double direction = recordarray[Idx_WVDIR]->getInterpolatedValue(
            m_cursor_lon, m_cursor_lat, true, true);
        wdir = direction;
        return skn;
      }
      break;
    case Idx_WVPER:
      if (recordarray[Idx_WVPER]) {
        double period = recordarray[Idx_WVPER]->getInterpolatedValue(
            m_cursor_lon, m_cursor_lat, true);
        if (period != GRIB_NOTDEF)
          skn.Printf(wxString::Format(_T("%01ds"), (int)(period + 0.5)));
      }
  }
  return skn;
}

wxString GRIBTable::GetRainfall(GribRecord **recordarray) {
  wxString skn(wxEmptyString);
  if (recordarray[Idx_PRECIP_TOT]) {
    double precip = recordarray[Idx_PRECIP_TOT]->getInterpolatedValue(
        m_cursor_lon, m_cursor_lat, true);

    if (precip != GRIB_NOTDEF) {
      precip = m_pGDialog->m_OverlaySettings.CalibrateValue(
          GribOverlaySettings::PRECIPITATION, precip);
      skn.Printf(_T("%6.2f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                                    GribOverlaySettings::PRECIPITATION),
                 precip);
      m_pDataCellsColour =
          m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
              GribOverlaySettings::PRECIPITATION, precip);
    }
  }
  return skn;
}

wxString GRIBTable::GetCloudCover(GribRecord **recordarray) {
  wxString skn(wxEmptyString);
  if (recordarray[Idx_CLOUD_TOT]) {
    double cloud = recordarray[Idx_CLOUD_TOT]->getInterpolatedValue(
        m_cursor_lon, m_cursor_lat, true);

    if (cloud != GRIB_NOTDEF) {
      cloud = m_pGDialog->m_OverlaySettings.CalibrateValue(
          GribOverlaySettings::CLOUD, cloud);
      skn.Printf(_T("%5.1f "), cloud);
      skn.Append(m_pGDialog->m_OverlaySettings.GetUnitSymbol(
          GribOverlaySettings::CLOUD));
      m_pDataCellsColour =
          m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
              GribOverlaySettings::CLOUD, cloud);
    }
  }
  return skn;
}

wxString GRIBTable::GetAirTemp(GribRecord **recordarray) {
  wxString skn(wxEmptyString);
  if (recordarray[Idx_AIR_TEMP]) {
    double temp = recordarray[Idx_AIR_TEMP]->getInterpolatedValue(
        m_cursor_lon, m_cursor_lat, true);

    if (temp != GRIB_NOTDEF) {
      temp = m_pGDialog->m_OverlaySettings.CalibrateValue(
          GribOverlaySettings::AIR_TEMPERATURE, temp);
      skn.Printf(_T("%5.1f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                                    GribOverlaySettings::AIR_TEMPERATURE),
                 temp);
      m_pDataCellsColour =
          m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
              GribOverlaySettings::AIR_TEMPERATURE, temp);
    }
  }
  return skn;
}

wxString GRIBTable::GetSeaTemp(GribRecord **recordarray) {
  wxString skn(wxEmptyString);
  if (recordarray[Idx_SEA_TEMP]) {
    double temp = recordarray[Idx_SEA_TEMP]->getInterpolatedValue(
        m_cursor_lon, m_cursor_lat, true);

    if (temp != GRIB_NOTDEF) {
      temp = m_pGDialog->m_OverlaySettings.CalibrateValue(
          GribOverlaySettings::SEA_TEMPERATURE, temp);
      skn.Printf(_T("%5.1f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                                    GribOverlaySettings::SEA_TEMPERATURE),
                 temp);
      m_pDataCellsColour =
          m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
              GribOverlaySettings::SEA_TEMPERATURE, temp);
    }
  }
  return skn;
}

wxString GRIBTable::GetCAPE(GribRecord **recordarray) {
  wxString skn(wxEmptyString);
  if (recordarray[Idx_CAPE]) {
    double cape = recordarray[Idx_CAPE]->getInterpolatedValue(
        m_cursor_lon, m_cursor_lat, true);

    if (cape != GRIB_NOTDEF) {
      cape = m_pGDialog->m_OverlaySettings.CalibrateValue(
          GribOverlaySettings::CAPE, cape);
      skn.Printf(wxString::Format(
          _T("%5.0f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                             GribOverlaySettings::CAPE),
          cape));
      m_pDataCellsColour =
          m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
              GribOverlaySettings::CAPE, cape);
    }
  }
  return skn;
}

wxString GRIBTable::GetCompRefl(GribRecord **recordarray) {
  wxString skn(wxEmptyString);
  if (recordarray[Idx_COMP_REFL]) {
    double refl = recordarray[Idx_COMP_REFL]->getInterpolatedValue(
        m_cursor_lon, m_cursor_lat, true);

    if (refl != GRIB_NOTDEF) {
      refl = m_pGDialog->m_OverlaySettings.CalibrateValue(
          GribOverlaySettings::COMP_REFL, refl);
      skn.Printf(wxString::Format(
          _T("%5.0f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                             GribOverlaySettings::COMP_REFL),
          refl));
      m_pDataCellsColour =
          m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
              GribOverlaySettings::COMP_REFL, refl);
    }
  }
  return skn;
}

wxString GRIBTable::GetCurrent(GribRecord **recordarray, int datatype,
                               double &wdir) {
  wxString skn(wxEmptyString);
  double vkn, ang;
  wdir = GRIB_NOTDEF;
  if (GribRecord::getInterpolatedValues(
          vkn, ang, recordarray[Idx_SEACURRENT_VX],
          recordarray[Idx_SEACURRENT_VY], m_cursor_lon, m_cursor_lat)) {
    if (datatype == 1) {
      wdir = ang;
      return skn;
    }
    vkn = m_pGDialog->m_OverlaySettings.CalibrateValue(
        GribOverlaySettings::CURRENT, vkn);
    skn.Printf(wxString::Format(
        _T("%4.1f ") + m_pGDialog->m_OverlaySettings.GetUnitSymbol(
                           GribOverlaySettings::CURRENT),
        vkn));
    m_pDataCellsColour =
        m_pGDialog->pPlugIn->m_pGRIBOverlayFactory->GetGraphicColor(
            GribOverlaySettings::CURRENT, vkn);
  }
  return skn;
}

wxString GRIBTable::GetTimeRowsStrings(wxDateTime date_time, int time_zone,
                                       int type) {
  wxDateTime t(date_time);
  switch (time_zone) {
    case 0:
      if ((wxDateTime::Now() == (wxDateTime::Now().ToGMT())) &&
          t.IsDST())  // bug in wxWingets 3.0 for UTC meridien ?
        t.Add(wxTimeSpan(1, 0, 0, 0));
      switch (type) {
        case 0:
          return t.Format(_T(" %H:%M  "), wxDateTime::Local) + _T("LOC");
        case 1:
          if (GetLocaleCanonicalName() == _T("en_US"))
            return t.Format(_T(" %a-%m/%d/%y  "), wxDateTime::Local);
          else
            return t.Format(_T(" %a-%d/%m/%y  "), wxDateTime::Local);
      }
    case 1:
      switch (type) {
        case 0:
          return t.Format(_T(" %H:%M  "), wxDateTime::UTC) + _T("UTC");
        case 1:
          if (GetLocaleCanonicalName() == _T("en_US"))
            return t.Format(_T(" %a-%m/%d/%y  "), wxDateTime::UTC);
          else
            return t.Format(_T(" %a-%d/%m/%y  "), wxDateTime::UTC);
      }
    default:
      return wxEmptyString;
  }
}
