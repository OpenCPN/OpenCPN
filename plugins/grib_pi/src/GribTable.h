/***************************************************************************
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
 ***************************************************************************/
/**
 * \file
 * GRIB Data Table View and Export Interface.
 *
 * Provides a tabular visualization of GRIB weather data with:
 * - Time-based columns showing forecast progression
 * - Parameter-based rows (wind, waves, pressure, etc.)
 * - Unit conversion and formatting
 * - Real-time cursor position data updates
 * - Time zone handling (UTC/local)
 *
 * Features:
 * - Automatic column sizing and layout
 * - Custom grid rendering for weather data
 * - Parameter highlighting and emphasis
 * - Vector data representation (direction + magnitude)
 * - Export capabilities for data analysis
 *
 * The table view provides an alternative to graphical overlay visualization,
 * allowing precise numerical inspection of forecast data at specific locations
 * and times.
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

/**
 * Dialog showing GRIB data in a table format.
 *
 * Displays meteorological data from GRIB files in a grid layout, with columns
 * representing different time points and rows showing various parameters like
 * wind, pressure, waves etc. The data shown corresponds to the current cursor
 * position on the chart.
 */
class GRIBTable : public GRIBTableBase {
public:
  GRIBTable(GRIBUICtrlBar &parent);

  ~GRIBTable() { delete m_pGribTable; }

  /**
   * Initialize the GRIB data table.
   *
   * @param rsa Array of GRIB record sets containing the weather data.
   * @param NowIndex Index of the current time point to highlight.
   */
  void InitGribTable(ArrayOfGribRecordSets *rsa);
  void InitGribTable(ArrayOfGribRecordSets *rsa, int NowIndex);
  /**
   * Set the table size and position relative to viewport.
   *
   * @param vpWidth Viewport width
   * @param vpHeight Viewport height
   */
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
  /**
   * Get a combined wave property (height, period, or direction) from the GRIB
   * record array.
   *
   * @param recordarray Array of pointers to GRIB records for the current time
   * step.
   * @param datatype    The GRIB data type (e.g., significant wave height,
   * period, or direction).
   * @param wdir        Output: direction (degrees) if applicable, otherwise set
   * to GRIB_NOTDEF.
   * @return            String representation of the requested property, or
   * empty if not available or if direction is requested.
   */
  wxString GetCombinedWaves(GribRecord **recordarray, int datatype,
                            double &wdir);
  /**
   * Get a wind wave property (height, period, or direction) from the GRIB
   * record array.
   *
   * Depending on the datatype, returns the wind wave height or period as a
   * formatted string. If the datatype is direction (e.g., Idx_WINDWAVE_DIR),
   * sets wdir to the direction in degrees and returns an empty string.
   *
   * @param recordarray Array of pointers to GRIB records for the current time
   * step.
   * @param datatype    The GRIB data type (e.g., Idx_WINDWAVE_HT,
   * Idx_WINDWAVE_PER, Idx_WINDWAVE_DIR).
   * @param wdir        Output: wind wave direction (degrees) if applicable,
   * otherwise set to GRIB_NOTDEF.
   * @return            String representation of the requested property (height
   * or period), or empty if not available or if direction is requested.
   */
  wxString GetWindWaves(GribRecord **recordarray, int datatype, double &wdir);
  /**
   * Get a swell wave property (height, period, or direction) from the GRIB
   * record array.
   *
   * Depending on the datatype, returns the swell wave height or period as a
   * formatted string. If the datatype is direction (e.g., Idx_SWELL_DIR), sets
   * wdir to the direction in degrees and returns an empty string.
   *
   * @param recordarray Array of pointers to GRIB records for the current time
   * step.
   * @param datatype    The GRIB data type (e.g., Idx_SWELL_HT, Idx_SWELL_PER,
   * Idx_SWELL_DIR).
   * @param wdir        Output: swell wave direction (degrees) if applicable,
   * otherwise set to GRIB_NOTDEF.
   * @return            String representation of the requested property (height
   * or period), or empty if not available or if direction is requested.
   */
  wxString GetSwellWaves(GribRecord **recordarray, int datatype, double &wdir);
  wxString GetRainfall(GribRecord **recordarray);
  wxString GetCloudCover(GribRecord **recordarray);
  wxString GetAirTemp(GribRecord **recordarray);
  wxString GetSeaTemp(GribRecord **recordarray);
  wxString GetCAPE(GribRecord **recordarray);
  wxString GetCompRefl(GribRecord **recordarray);
  wxString GetCurrent(GribRecord **recordarray, int datatype, double &wdir);

  void OnClose(wxCloseEvent &event);
  void OnOKButton(wxCommandEvent &event);

  GRIBUICtrlBar *m_pGDialog;
  wxColour m_pDataCellsColour;
  wxTimer m_tScrollToNowTimer;
};

#endif  //__GRIBTABLE_H__
