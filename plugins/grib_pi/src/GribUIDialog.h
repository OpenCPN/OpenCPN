/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Freinds
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

#ifndef __GRIBUIDIALOG_H__
#define __GRIBUIDIALOG_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include <wx/fileconf.h>
#include <wx/glcanvas.h>

#include "GribUIDialogBase.h"
#include "GribReader.h"
#include "GribRecord.h"
#include "IsoLine.h"

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

enum OVERLAP {
    _IN, _ON, _OUT
};

enum {
  GENERIC_GRAPHIC_INDEX, CURRENT_GRAPHIC_INDEX, SEATEMP_GRAPHIC_INDEX,
  CRAIN_GRAPHIC_INDEX, QUICKSCAT_GRAPHIC_INDEX
};

class GRIBFile;
class GRIBRecord;
class GribRecordTree;
class GRIBOverlayFactory;
class GribRecordSet;

class wxFileConfig;
class grib_pi;
class wxGraphicsContext;

WX_DECLARE_OBJARRAY( GribRecordSet, ArrayOfGribRecordSets );
WX_DECLARE_OBJARRAY( GribRecord *, ArrayOfGribRecordPtrs );

    // These are indexes into the array
enum { Idx_WIND_VX, Idx_WIND_VY, Idx_PRESS, Idx_HTSIGW, Idx_WVDIR,
       Idx_WINDSCAT_VY, Idx_WINDSCAT_VX, Idx_SEATEMP, Idx_SEACURRENT_VX,
       Idx_SEACURRENT_VY, Idx_COUNT };

class GribRecordSet {
public:
    GribRecordSet();
    GribRecordSet(GribRecordSet &GRS1, GribRecordSet &GRS2, double interp_const);
    ~GribRecordSet();

    time_t m_Reference_Time;
    GribRecord *m_GribRecordPtrArray[Idx_COUNT];

    /* cache isobars here to speed up rendering */
    wxArrayPtrVoid m_IsobarArray;
};

//----------------------------------------------------------------------------------------------------------
//    GRIB Selector/Control Dialog Specification
//----------------------------------------------------------------------------------------------------------
class GRIBUIDialog: public GRIBUIDialogBase {
DECLARE_CLASS( GRIBUIDialog )DECLARE_EVENT_TABLE()
public:

    GRIBUIDialog(wxWindow *parent, grib_pi *ppi);

    ~GRIBUIDialog();
    void Init();

    void SelectTreeControlGRS( GRIBFile *pgribfile );
    void PopulateTreeControlGRS( GRIBFile *pgribfile, int file_index );
    void SetGribRecordSet( GribRecordSet *pGribRecordSet );
    void SelectGribRecordSet( GribRecordSet *pGribRecordSet );

    void SetCursorLatLon( double lat, double lon );

private:
    void OnClose( wxCloseEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnSize( wxSizeEvent& event );
    void OnFileDirChange( wxFileDirPickerEvent &event );
    void UpdateTrackingControls( void );
    void PopulateTreeControl( void );
    void SetFactoryOptions();
    void TimelineChanged();

    void OnTimeline( wxCommandEvent& event );
    void OnCBAny( wxCommandEvent& event );

    //    Data
    wxWindow *pParent;
    grib_pi *pPlugIn;

    wxFont *m_dFont;

    wxTreeItemId m_RecordTree_root_id;

    int m_n_files;

    GribRecordSet *m_pCurrentGribRecordSet;

    int m_sequence_active;

    double m_cursor_lat, m_cursor_lon;

    ArrayOfGribRecordSets *m_timelineset, *m_timelinebase;
};

//----------------------------------------------------------------------------------------------------------
//    GRIBFile Object Specification
//----------------------------------------------------------------------------------------------------------
class GRIBFile {
public:

    GRIBFile( const wxString file_name );
    ~GRIBFile();

    bool IsOK( void )
    {
        return m_bOK;
    }
    wxString GetLastErrorMessage( void )
    {
        return m_last_error_message;
    }
    ArrayOfGribRecordSets *GetRecordSetArrayPtr( void )
    {
        return &m_GribRecordSetArray;
    }

private:

    bool m_bOK;
    wxString m_last_error_message;
    GribReader *m_pGribReader;

    //    An array of GribRecordSets found in this GRIB file
    ArrayOfGribRecordSets m_GribRecordSetArray;

    int m_nGribRecords;

};

#endif

