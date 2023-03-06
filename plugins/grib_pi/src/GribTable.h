/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Friends - gribtable
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
 */
#ifndef __GRIBTABLE_H__
#define __GRIBTABLE_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "pi_gl.h"

#include <wx/grid.h>

#include "GribUIDialogBase.h"
#include "grib_pi.h"
#include "ocpn_plugin.h"
#include "CustomGrid.h"

class GRIBUICtrlBar;

enum NumericalRows { R_WIND, R_WAVES, R_CURRENT };
//----------------------------------------------------------------------------------------------------------
//    GRIB table dialog Specification
//----------------------------------------------------------------------------------------------------------
class GRIBTable : public GRIBTableBase {
public:
  GRIBTable(GRIBUICtrlBar &parent);

  ~GRIBTable() { delete m_pGribTable; }

  void InitGribTable(int zone, ArrayOfGribRecordSets *rsa);
  void InitGribTable(int zone, ArrayOfGribRecordSets *rsa, int NowIndex);
  void SetTableSizePosition(int vpWidth, int vpHeight);
  wxBitmap GetScaledBitmap(wxBitmap bmp, wxString svgfile, double scfactor) {
    return m_pGDialog->GetScaledBitmap(bmp, svgfile, scfactor);
  }
  void CloseDialog();

private:
  void AddDataRow(int num_rows, int num_cols, wxString label,
                  wxGridCellAttr *row_attr);
  void AutoSizeDataRows();
  int GetVisibleRow(int col);
  void OnScrollToNowTimer(wxTimerEvent &event);

  wxString GetWind(GribRecord **recordarray, int datatype, double &wdir);
  wxString GetWindGust(GribRecord **recordarray, int datatype);
  wxString GetPressure(GribRecord **recordarray);
  wxString GetWaves(GribRecord **recordarray, int datatype, double &wdir);
  wxString GetRainfall(GribRecord **recordarray);
  wxString GetCloudCover(GribRecord **recordarray);
  wxString GetAirTemp(GribRecord **recordarray);
  wxString GetSeaTemp(GribRecord **recordarray);
  wxString GetCAPE(GribRecord **recordarray);
  wxString GetCompRefl(GribRecord **recordarray);
  wxString GetCurrent(GribRecord **recordarray, int datatype, double &wdir);
  wxString GetTimeRowsStrings(wxDateTime date_time, int time_zone, int type);

  void OnClose(wxCloseEvent &event);
  void OnOKButton(wxCommandEvent &event);

  GRIBUICtrlBar *m_pGDialog;
  wxColour m_pDataCellsColour;
  wxTimer m_tScrollToNowTimer;
};

#endif  //__GRIBTABLE_H__
