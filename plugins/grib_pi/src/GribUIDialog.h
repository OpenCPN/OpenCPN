/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Friends
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

#ifndef __GRIBUICTRLBAR_H__
#define __GRIBUICTRLBAR_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include <wx/fileconf.h>
#include <wx/glcanvas.h>

#include "GribUIDialogBase.h"
#include "CursorData.h"
#include "GribSettingsDialog.h"
#include "GribRequestDialog.h"
#include "GribReader.h"
#include "GribRecordSet.h"
#include "IsoLine.h"
#include "GrabberWin.h"

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

class GRIBUICtrlBar;
class GRIBUICData;
class GRIBFile;
class GRIBRecord;
class GribRecordTree;
class GRIBOverlayFactory;
class GribRecordSet;
class GribRequestSetting;
class GribGrabberWin;
class GribSpacerWin;

class wxFileConfig;
class grib_pi;
class wxGraphicsContext;

WX_DECLARE_OBJARRAY( GribRecordSet, ArrayOfGribRecordSets );

enum ZoneSelection { AUTO_SELECTION, SAVED_SELECTION, START_SELECTION, DRAW_SELECTION, COMPLETE_SELECTION };

class GribTimelineRecordSet : public GribRecordSet
{
public:
    GribTimelineRecordSet();
//    GribTimelineRecordSet(GribRecordSet &GRS1, GribRecordSet &GRS2, double interp_const);
    ~GribTimelineRecordSet();

    void ClearCachedData();

    /* cache isobars here to speed up rendering */
    wxArrayPtrVoid *m_IsobarArray[Idx_COUNT];
};

//----------------------------------------------------------------------------------------------------------
//    GRIB CtrlBar Specification
//----------------------------------------------------------------------------------------------------------
class GRIBUICtrlBar: public GRIBUICtrlBarBase {
public:

    GRIBUICtrlBar(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ,grib_pi *ppi );
    ~GRIBUICtrlBar();

    void OpenFile( bool newestFile = false );

    void ContextMenuItemCallback(int id);
    void SetFactoryOptions();

    wxDateTime TimelineTime();
    GribTimelineRecordSet* GetTimeLineRecordSet(wxDateTime time);
    void StopPlayBack();
    void TimelineChanged();
    void CreateActiveFileFromNames( const wxArrayString &filenames );
    void PopulateComboDataList();
    void ComputeBestForecastForNow();
    void SetViewPort( PlugIn_ViewPort *vp );
    void SetDataBackGroundColor();
    void SetTimeLineMax( bool SetValue );
	void SetCursorLatLon( double lat, double lon );
    void UpdateTrackingControl();
	void SetDialogsStyleSizePosition( bool force_recompute = false );
    void SetRequestBitmap( int type );
    void OnMouseEvent( wxMouseEvent& event );
    GRIBUICData *GetCDataDialog() { return m_gGRIBUICData; }
    bool InDataPlot (int id) { return id > wxID_ANY && id < (int)GribOverlaySettings::GEO_ALTITUDE; }
    void SetScaledBitmap( double factor );
    wxBitmap GetScaledBitmap(wxBitmap bitmap, const wxString svgFileName, double scale_factor);
    void OpenFileFromJSON( wxString json);
        
    wxWindow *pParent;
    GribOverlaySettings m_OverlaySettings;

	GribTimelineRecordSet *m_pTimelineSet;

    wxTimer         m_tPlayStop;
    grib_pi         *pPlugIn;
    GribRequestSetting  *pReq_Dialog;
    GRIBFile        *m_bGRIBActiveFile;
	bool            m_bDataPlot[GribOverlaySettings::GEO_ALTITUDE];  //only for no altitude parameters
	bool            m_CDataIsShown;
    int             m_ZoneSelAllowed;
    int             m_old_DialogStyle;
	double			m_ScaledFactor;
private:
    void OnClose( wxCloseEvent& event );
    void OnSize( wxSizeEvent& event );
    void OnPaint( wxPaintEvent& event );
    void OnSettings( wxCommandEvent& event );
    void OnPlayStop( wxCommandEvent& event );
    void OnPlayStopTimer( wxTimerEvent & event);
	void OnMove( wxMoveEvent& event );
    void OnMenuEvent( wxMenuEvent& event );
    void MenuAppend( wxMenu *menu, int id, wxString label, wxItemKind kind, wxBitmap bitmap = wxNullBitmap, wxMenu *submenu = NULL );
    void OnZoomToCenterClick( wxCommandEvent& event );
    void OnPrev( wxCommandEvent& event );
    void OnRecordForecast( wxCommandEvent& event ) { StopPlayBack(); m_InterpolateMode = false; m_pNowMode = false; TimelineChanged(); }
    void OnNext( wxCommandEvent& event );
    void OnNow( wxCommandEvent& event ) { StopPlayBack(); ComputeBestForecastForNow(); }
    void OnAltitude( wxCommandEvent& event );
    void OnOpenFile( wxCommandEvent& event );
    void OnRequest(  wxCommandEvent& event );
    void OnCompositeDialog( wxCommandEvent& event );
    
    void OnTimeline( wxScrollEvent& event );
	void OnShowCursorData( wxCommandEvent& event );

    wxDateTime MinTime();
    wxArrayString GetFilesInDirectory();
    void SetGribTimelineRecordSet(GribTimelineRecordSet *pTimelineSet);
    int GetNearestIndex(wxDateTime time, int model);
    int GetNearestValue(wxDateTime time, int model);
    bool GetGribZoneLimits(GribTimelineRecordSet *timelineSet, double *latmin, double *latmax, double *lonmin, double *lonmax);
    wxDateTime GetNow();
    void RestaureSelectionString();
    void SaveSelectionString()  { m_SelectionIsSaved = true; m_Selection_index = m_cRecordForecast->GetSelection();
            m_Selection_label = m_cRecordForecast->GetString( m_Selection_index); }

    //    Data
	CursorData        *m_gCursorData;
    GribGrabberWin    *m_gGrabber;
    GRIBUICData       *m_gGRIBUICData;

    PlugIn_ViewPort   *m_vp;
    int m_lastdatatype;

    int m_TimeLineHours;
    int m_FileIntervalIndex;
    bool m_InterpolateMode;
    bool m_pNowMode;
    bool m_HasAltitude;

    bool             m_SelectionIsSaved;
    int              m_Selection_index;
    wxString         m_Selection_label;
    wxArrayString    m_file_names;   /* selected files */
    wxString         m_grib_dir;
	wxSize           m_DialogsOffset;
};

//----------------------------------------------------------------------------------------------------------
//    GRIBFile Object Specification
//----------------------------------------------------------------------------------------------------------
class GRIBFile {
public:

    GRIBFile( const wxArrayString & file_names, bool CumRec, bool WaveRec, bool newestFile = false );
    ~GRIBFile();

    bool IsOK( void )
    {
        return m_bOK;
    }
    wxArrayString &GetFileNames( void )
    {
        return m_FileNames;
    }
    wxString GetLastMessage( void )
    {
        return m_last_message;
    }
    ArrayOfGribRecordSets *GetRecordSetArrayPtr( void )
    {
        return &m_GribRecordSetArray;
    }
    time_t GetRefDateTime( void )
    {
        return m_pRefDateTime;
    }

    WX_DEFINE_ARRAY_INT(int, GribIdxArray);
    GribIdxArray m_GribIdxArray;

private:

    bool m_bOK;
    wxString m_last_message;
    wxArrayString m_FileNames;
    GribReader *m_pGribReader;
    time_t m_pRefDateTime;

    //    An array of GribRecordSets found in this GRIB file
    ArrayOfGribRecordSets m_GribRecordSetArray;

    int m_nGribRecords;
};

//----------------------------------------------------------------------------------------------------------
//    GRIB CursorData Dialog Specification
//----------------------------------------------------------------------------------------------------------
class GRIBUICData: public GRIBUICDataBase
{
public:

    GRIBUICData( GRIBUICtrlBar &parent );
    ~GRIBUICData() {}

    GribGrabberWin      *m_gGrabber;
    GRIBUICtrlBar       &m_gpparent;
    CursorData          *m_gCursorData;
private:
    void OnMove( wxMoveEvent& event );
};

#endif
