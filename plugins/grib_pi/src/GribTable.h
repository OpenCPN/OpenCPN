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

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/grid.h>

#include "GribUIDialogBase.h"
#include "grib_pi.h"
#include "ocpn_plugin.h"

class GRIBUICtrlBar;

//----------------------------------------------------------------------------------------------------------
//    GRIB table dialog Specification
//----------------------------------------------------------------------------------------------------------
class GRIBTable: public GRIBTableBase 
{
public:

    GRIBTable( GRIBUICtrlBar &parent);
    
    ~GRIBTable(){}

    void InitGribTable( int zone, ArrayOfGribRecordSets *rsa);
    void CloseDialog();

private:

    void AddDataRow( int num_rows, int num_cols, wxString label, wxGridCellAttr *row_attr );
    void AutoSizeDataRows();

    wxString GetWind(GribRecord **recordarray);
    wxString GetWindBf(GribRecord **recordarray);
    wxString GetPressure(GribRecord **recordarray);
    wxString GetWindGust(GribRecord **recordarray);
    wxString GetWaves(GribRecord **recordarray);
    wxString GetRainfall(GribRecord **recordarray);
    wxString GetCloudCover(GribRecord **recordarray);
    wxString GetAirTemp(GribRecord **recordarray);
    wxString GetSeaTemp(GribRecord **recordarray);
    wxString GetCAPE(GribRecord **recordarray);
    wxString GetCurrent(GribRecord **recordarray);
    wxString GetTimeRowsStrings( wxDateTime date_time, int time_zone, int type );

    void OnClose( wxCloseEvent& event );
    void OnOKButton( wxCommandEvent& event );
    void OnClick( wxGridEvent& event );
    void OnRangeClick( wxGridRangeSelectEvent& event );

    GRIBUICtrlBar *m_pGDialog;
    GribTimelineRecordSet * m_pTimeset;
    wxColour m_pDataCellsColour;
};

#endif //__GRIBTABLE_H__
