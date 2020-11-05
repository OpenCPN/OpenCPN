/******************************************************************************
 * $Id:  $
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin 
 * Author:   Johan van der Sman
 *
 ***************************************************************************
 *   Copyright (C) 2015 Johan van der Sman    sss                             *
 *   johan.sman@gmail.com                                                  *
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

#ifndef _RADAR_H_
#define _RADAR_H_

#include "wx/socket.h"
#include "wx/wxprec.h"
#include "wx/dcbuffer.h"
#include <wx/textctrl.h>
#include <wx/grid.h>
#include "Target.h"

#include <fstream>       //提供文件头文件


#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define ID_OK                       10001
#define MIN_RADIUS                  150
#define TEXT_MARGIN                 5
#define SPACER_MARGIN               5
#define DEFAULT_GRID_ROWS_NUMBER 5 //zhh
#ifdef WIN32
    #define   MyFit(a)    Fit(a)
#else
    #define   MyFit(a)    FitInside(a)
#endif

class aisradar_pi;
// class FusionEKF;

class ViewState {
public:
    ViewState (const wxPoint& p, const wxSize& s) : Pos(p), Size(s) {};
    wxPoint  GetPos() { return Pos; }
    wxSize    GetWindowSize() { 
#ifdef WIN32
        return wxDefaultSize;
#else
        return Size; 
#endif
}
    wxSize   GetCanvasSize() {
#ifdef WIN32
        return Size;
#else
        return wxDefaultSize;
#endif
    }
    wxSize   GetSize() { return Size; }
    void     SetPos(const wxPoint& p) { Pos=p; }
    void     SetWindowSize(const wxSize& s) { 
#ifndef WIN32
        Size=s; 
#endif
    }
    void     SetCanvasSize(const wxSize& s) { 
#ifdef WIN32
        Size=s; 
#endif
    }

private:
    wxPoint    Pos;
    wxSize    Size;
};

//----------------------------------------------------------------------------------------------------------
//    RADAR View Specification
//----------------------------------------------------------------------------------------------------------
class RadarFrame: public wxFrame
{
    DECLARE_CLASS( RadarFrame )
    DECLARE_EVENT_TABLE()

public:
    RadarFrame( );
    ~RadarFrame( );
    void Init();
    void Config(int min_radius);
    bool Create(  wxWindow *parent, aisradar_pi *ppi, wxWindowID id = wxID_ANY,
    const wxString& caption = _("Radar Display"), 
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize );
    void SetColourScheme(PI_ColorScheme cs);
    void OnLeftMouse( const wxPoint &curpos );
    void paintEvent( wxPaintEvent& event );
    void OwnShipDecisionBroadcast(void);

//zhh0
    wxString YawAlarm;
    wxString BoundaryAlarm;
    wxString TurnAlarm;
    wxString AidDecisionMaking; 
//zhh0

private:
    void OnClose(wxCloseEvent& event);
    void OnRange( wxCommandEvent& event );
	void OnNorthUp( wxCommandEvent& eMyLabelent );
    void OnBearingLine( wxCommandEvent& event );
    void OnTimer( wxTimerEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnSize( wxSizeEvent& event );
    void render( wxDC& dc );
    void renderRange( wxDC& dc, wxPoint &center, wxSize &size, int radius );
    void renderBoats(wxDC& dc, wxPoint &center, wxSize &size, int radius, ArrayOfPlugIn_AIS_Targets *AisTargets );
    void TrimAisField(wxString *fld);
    

    // void SetConnectOption( wxCommandEvent& event );
    // void TTSPlaySound( wxCommandEvent& event );
    // void TTSPlaySoundTimer( wxTimerEvent& event );
    // void SpeechBroadcast(ArrayOfPlugIn_AIS_Targets *AisTargets);
    // void ShipContorl(); // 航运学院

    // //    Data
    // // convenience functions
    // void OnOpenConnection(wxCommandEvent& event);
    // void OnTest();
    // void Connect();
    // void OnSocketEvent(wxSocketEvent& event);
    // void UpdateStatusBar();
    // void OpenConnection(wxSockAddress::Family family);


    wxWindow               *pParent;
    aisradar_pi            *pPlugIn;
    wxTimer                *m_Timer;
    wxTimer                *m_Timer_TTS;

    wxPanel                *m_pCanvas;
    wxCheckBox             *m_pNorthUp;
    wxComboBox             *m_pRange;
    wxCheckBox             *m_pBearingLine;
    wxColor                 m_BgColour;
    double                  m_Ebl;
    int                     m_Range;
    ViewState              *m_pViewState;

    wxTextCtrl             *m_VHFTextCtrl;
    wxGrid                 *m_VHFGrid;
    wxTextCtrl             *m_TPTextCtrl;
    wxGrid                 *m_TPGrid;

  

    wxButton               *m_pShowList;
    wxTextCtrl             *m_textCtrl1;
    wxButton               *m_soundButton;
    wxButton               *m_ConnectOptionButton;

    // TCP socket
    // wxSocketClient         *m_sock;
    wxSocketServer         *m_server;
    int                     m_numClients;
    bool                    m_busy;

    void OnServerEvent(wxSocketEvent& event);
    void OnSocketEvent(wxSocketEvent& event);
    void UpdateStatusBar();

    
     

    void SendData2Client(wxSocketBase *sock);
    
    void GetClientResult(wxSocketBase *sock);//zhh
    void Test3(wxSocketBase *sock);

    void ReadDataFromFile(wxCommandEvent& event);//nlq 
    // kalman
    //FusionEKF               fusionEKF;
    

};


class TestLogger
{
public:
    TestLogger(const wxString& name) : m_name(name)
    {
        wxLogMessage("=== %s begins ===", m_name);
    }

    ~TestLogger()
    {
        wxLogMessage("=== %s ends ===", m_name);
    }

private:
    const wxString m_name;
};
#endif
