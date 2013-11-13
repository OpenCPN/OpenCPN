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

#ifndef __GRIBUIDIALOG_H__
#define __GRIBUIDIALOG_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include <wx/fileconf.h>
#include <wx/glcanvas.h>

#include "GribUIDialogBase.h"
#include "GribSettingsDialog.h"
#include "GribReader.h"
#include "GribRecordSet.h"
#include "IsoLine.h"

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

class GRIBFile;
class GRIBRecord;
class GribRecordTree;
class GRIBOverlayFactory;
class GribRecordSet;

class wxFileConfig;
class grib_pi;
class wxGraphicsContext;

WX_DECLARE_OBJARRAY( GribRecordSet, ArrayOfGribRecordSets );

class GribTimelineRecordSet : public GribRecordSet
{
public:
    GribTimelineRecordSet(GribRecordSet &GRS1, GribRecordSet &GRS2, double interp_const);
    ~GribTimelineRecordSet();

    void ClearCachedData();

    /* cache isobars here to speed up rendering */
    wxArrayPtrVoid *m_IsobarArray[Idx_COUNT];
};

//----------------------------------------------------------------------------------------------------------
//    GRIB Selector/Control Dialog Specification
//----------------------------------------------------------------------------------------------------------
class GRIBUIDialog: public GRIBUIDialogBase {
public:

    GRIBUIDialog(wxWindow *parent, grib_pi *ppi);
    ~GRIBUIDialog();

    void OpenFile( bool newestFile = false );
    
    void SetCursorLatLon( double lat, double lon );
    void SetFactoryOptions( bool set_val = false );

    wxDateTime TimelineTime();
    GribTimelineRecordSet* GetTimeLineRecordSet(wxDateTime time);
    void TimelineChanged();
    void CreateActiveFileFromName( wxString filename );
    void PopulateComboDataList();
    void ComputeBestForecastForNow();
    void SetViewPort( PlugIn_ViewPort *vp ) { m_vp = new PlugIn_ViewPort(*vp); }

    GribOverlaySettings m_OverlaySettings;

    wxTimer m_tPlayStop;

    GRIBFile        *m_bGRIBActiveFile;
    bool             m_InterpolateMode;

private:
    void OnClose( wxCloseEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnSize( wxSizeEvent& event );
    void OnSettings( wxCommandEvent& event );
    void OnPlayStop( wxCommandEvent& event );
    void OnPlayStopTimer( wxTimerEvent & );

    void AddTrackingControl( wxControl *ctrl1,  wxControl *ctrl2,  wxControl *ctrl3, bool show );
    void PopulateTrackingControls( void );
    void UpdateTrackingControls( void );

    void OnPrev( wxCommandEvent& event );
    void OnRecordForecast( wxCommandEvent& event ) { m_InterpolateMode = false; TimelineChanged(); }
    void OnNext( wxCommandEvent& event );
    void OnNow( wxCommandEvent& event ) { ComputeBestForecastForNow(); }
    void OnOpenFile( wxCommandEvent& event );
    void OnRequest(  wxCommandEvent& event );

    void OnTimeline( wxScrollEvent& event );
    void OnCBAny( wxCommandEvent& event );

    wxDateTime MinTime();
    wxString GetNewestFileInDirectory();
    void SetGribTimelineRecordSet(GribTimelineRecordSet *pTimelineSet);
    int GetTimePosition(wxDateTime time);

    //    Data
    wxWindow *pParent;
    grib_pi *pPlugIn;

    PlugIn_ViewPort  *m_vp;
    int m_lastdatatype;

    double m_cursor_lat, m_cursor_lon;

    GribTimelineRecordSet *m_pTimelineSet;
    int m_TimeLineHours;

    wxString         m_file_name;   /* selected file */
    wxString         m_grib_dir;
    wxBitmap         *m_bPlay;
};

//----------------------------------------------------------------------------------------------------------
//    GRIBFile Object Specification
//----------------------------------------------------------------------------------------------------------
class GRIBFile {
public:

    GRIBFile( const wxString file_name, bool CumRec, bool WaveRec );
    ~GRIBFile();

    bool IsOK( void )
    {
        return m_bOK;
    }
    wxString GetFileName( void )
    {
        return m_FileName;
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
    wxString m_FileName;
    GribReader *m_pGribReader;
    time_t m_pRefDateTime;

    //    An array of GribRecordSets found in this GRIB file
    ArrayOfGribRecordSets m_GribRecordSetArray;

    int m_nGribRecords;
};

//----------------------------------------------------------------------------------------------------------
//    Request setting Specification
//----------------------------------------------------------------------------------------------------------
class GribRequestSetting : public GribRequestSettingBase
{
public:
      GribRequestSetting( wxWindow *parent, wxString config, int latmax, int latmin, int lonmin,
          int lonmax, wxString fromadd, wxString toadd, wxString login, wxString code)
          : GribRequestSettingBase(parent)
      {m_RequestConfigBase = config; m_LatmaxBase = latmax;  m_LatminBase = latmin;  m_LonminBase = lonmin;  m_LonmaxBase = lonmax; 
          m_pSenderAddress->ChangeValue(fromadd); m_MailToAddresses = toadd; m_pLogin->ChangeValue(login); m_pCode->ChangeValue(code);}

      ~GribRequestSetting() {}

      void InitRequestConfig();

      wxString m_RequestConfigBase;
      wxString m_MailToAddresses;
      wxString m_MailFromAddress;
      int m_LatmaxBase;
      int m_LatminBase;
      int m_LonminBase;
      int m_LonmaxBase;
    
private:
      void ApplyRequestConfig( int sel1, int sel2 );
      wxString WriteMail();
      bool EstimateFileSize();

      void OnTopChange(wxCommandEvent &event);
      void OnAnyChange( wxCommandEvent& event );
      void OnSendMaiL( wxCommandEvent& event );
      void OnSaveMail( wxCommandEvent& event ) { this->EndModal(wxID_APPLY); }

      int  m_MailError_Nb;

};

#endif

