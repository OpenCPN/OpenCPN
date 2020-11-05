/******************************************************************************
 * $Id:   $
 *
 * Project:  OpenCPN
 * Purpose:  RadarView Object
 * Author:   Johan van der Sman
 *
 ***************************************************************************
 *   Copyright (C) 2015 Johan van der Sman                                 *
 *   johan.sman@gmail.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PUversion 2 of the License, or     *
 *   (at your option) any later version.  RPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */
#include <wx/filedlg.h>
#include "wx/wx.h"
#include <wx/debug.h>
#include <wx/fileconf.h>
#include <wx/socket.h>
#include <math.h>
#include "aisradar_pi.h"
#include "Canvas.h"
// #include "kalman/FusionEKF.h"
#include <map>
#include <list>
#include <algorithm>
#include <thread>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <fstream>


#define  min(a,b)  ( (a>b)? b : a )
#define  max(a,b)  ( (a>b)? a : b )

#if wxUSE_IPV6
    typedef wxIPV6address IPaddress;
#else
    typedef wxIPV4address IPaddress;
#endif

using namespace std;

extern double           gLat, gLon;
extern double           gCog, gSog;
extern double g_n_ownship_length_meters;
extern double g_n_ownship_beam_meters;


static const double    RangeData[9] = {
    0.25, 0.5, 1, 2, 4, 8, 12, 16, 32
};

enum    Ids { cbRangeId = 10001,
                cbNorthUpId,
                cbBearingLineId,
                btShowAisList,
                tmRefreshId,
                plCanvasId,

                connectOptionLinkId,        // option button
                tmTTSId,                    // TTS timer
                soundPlayId,                // open button
                // id for socket
                SOCKET_ID,
                SERVER_ID
};

static const int RESTART  = -1;
static const int BASE_STATION = 3;
//zhh0
static unsigned int GetClientResultNo = 0;//获得张梁结果计数
//播报时序计数
static unsigned int YawAlarmLeftNo = 0; 
static unsigned int YawAlarmRightNo = 0;
static unsigned int BoundaryAlarmLeftNo = 0;
static unsigned int BoundaryAlarmRightNo = 0;
static unsigned int TurnAlarm8minNo =0;
static unsigned int TurnAlarm5minNo =0;
static unsigned int TurnAlarm2minNo =0;
wxString AidDecisionTemp;

const char * YawAlarmBroadcastContent;
const char * BoundaryAlarmBroadcastContent;
const char * TurnAlarmBroadcastContent;
const char * AidDecisionBroadcastContent;

const char *YawAlarmFileName;
const char *YawAlarmText;
const char *BoundaryAlarmFileName;
const char *BoundaryAlarmText;
const char *TurnAlarmFileName;
const char *TurnAlarmText;
const char *AidDecisionMakingFileName;
const char *AidDecisionMakingText;
//zhh0





double a1 = 6;    //AIS距离标准差
double a2 = 0.4;   //AIS方位标准差
double a3 = 0.161;    //AIS航速标准差
double a4 = 0.637;    //AIS航向标准差
double r1 = 20;     //Radar距离标准差
double r2 = 0.8;      //Radar方位标准差
double r3 = 0.148;    //Radar航速标准差
double r4 = 0.743;     //Radar航向标准差

//计算融合的权重系数
double a11 = (r1*r1) / (a1*a1 + r1*r1);
double a12 = (a1*a1) / (a1*a1 + r1*r1);
double a21 = (r2*r2) / (a2*a2 + r2*r2);
double a22 = (a2*a2) / (a2*a2 + r2*r2);
double a31 = (r3*r3) / (a3*a3 + r3*r3);
double a32 = (a3*a3) / (a3*a3 + r3*r3);
double a41 = (r4*r4) / (a4*a4 + r4*r4);
double a42 = (a4*a4) / (a4*a4 + r4*r4);



#include <sys/wait.h> // for WEXITSTATUS & friends
//zhh
void CilentResultSignalZero(void){
      YawAlarmLeftNo = 0; 
      YawAlarmRightNo = 0;
      BoundaryAlarmLeftNo = 0;
      BoundaryAlarmRightNo = 0;
      TurnAlarm8minNo =0;
      TurnAlarm5minNo =0;
      TurnAlarm2minNo =0;
      AidDecisionTemp = wxT("");
      YawAlarmBroadcastContent = "0";
      BoundaryAlarmBroadcastContent = "0";
      TurnAlarmBroadcastContent = "0";
      AidDecisionBroadcastContent = "0";

}

// static int do_play_wav(const char* cmd)
// {
//     char buff_wav[1024];
//     snprintf(buff_wav, sizeof( buff_wav ), "play  '%s'.wav", cmd);
//     int status = system(buff_wav);
//     return status ;
// }


//zhh获取shell命令结果(当前账号用户名)
char* username(void)
{  
    FILE *fp , *fp1;
    fp = popen("whoami", "r");
    if(!fp){

        cout << "popen failed" << endl;
        return 0;
    }
    int username_number = -1;
    char ch = 0;
    while( !feof(fp)){
       ch=fgetc(fp);//从文件中读取一个字符 
       if(ch != '\0'){
            username_number ++;
        }  
    }

    pclose(fp);
    fp = NULL;

    fp1 = popen("whoami", "r");
    if(!fp1){
        cout << "popen failed" << endl;
        return 0;
    }
    char *username_buffer = new char[username_number]; 
    fgets(username_buffer,username_number,fp1);
    return username_buffer;

    pclose(fp1);
    fp1 = NULL;

}


//zhh

//zhh_tts
static int do_play_wav_tts(const char* filename ,const char* text )
{
    char change_directory_buff[1024];
    char create_buff[1024];
    char play_buff[1024];
    
    const char * Username_buffer;
    Username_buffer = username();

    snprintf(create_buff, sizeof( create_buff ), "/home/%s/Desktop/voiceplay/tts_offline_sample '%s.wav' '%s'", Username_buffer, filename, text);

    int create_status = system(create_buff);
    if (create_status == -1) {
        wxLogWarning("Cannot fork process running %s", create_buff);
        return -1;
    }
    if (WIFEXITED(create_status)) {
        create_status = WEXITSTATUS(create_status);
        if (create_status != 0) {
            wxLogWarning("Exit code %d from command %s",
                create_status, create_buff);
        }
    } else {
        wxLogWarning("Strange return code %d (0x%x) running %s",
                     create_status, create_status, create_buff);
    }
    //sleep(2);

    snprintf(play_buff, sizeof( play_buff ), "play /home/%s/ship-collision-prevention/build/%s.wav", Username_buffer,  filename);

    int play_status = system(play_buff);
    if (play_status == -1) {
        wxLogWarning("Cannot fork process running %s", play_buff);
        return -1;
    }
    if (WIFEXITED(play_status)) {
        play_status = WEXITSTATUS(play_status);
        if (play_status != 0) {
            wxLogWarning("Exit code %d from command %s",
                play_status, play_status);
        }
    } else {
        wxLogWarning("Strange return code %d (0x%x) running %s",
                     play_status, play_status, play_buff);
    }
    sleep(2);

    return create_status||play_status;

    //return create_status;
}
//zhh_tts

void executeCMD(const char *cmd, char *result)   
{   
    char buf_ps[1024];   
    char ps[1024]={0};   
    FILE *ptr;   
    strcpy(ps, cmd);   
    if((ptr=popen(ps, "r"))!=NULL)   
    {   
        while(fgets(buf_ps, 1024, ptr)!=NULL)   
        {   
           strcat(result, buf_ps);   
           if(strlen(result)>1024)   
               break;   
        }   
        pclose(ptr);   
        ptr = NULL;   
    }   
    else  
    {   
        printf("popen %s error\n", ps);   
    }   
}
vector<wxString> split(const wxString& str, const wxString& delim) {
	vector<wxString> res;
	if("" == str) return res;
	//先将要切割的字符串从string类型转换为char*类型
	char * strs = new char[str.length() + 1] ; //不要忘了
	strcpy(strs, str.c_str()); 
 
	char * d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());
 
	char *p = strtok(strs, d);
	while(p) {
		wxString s = p; //分割得到的字符串转换为string类型
		res.push_back(s); //存入结果数组
		p = strtok(NULL, d);
	}
 
	return res;
}

//---------------------------------------------------------------------------------------
//          Radar Dialog Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS ( RadarFrame, wxFrame )

BEGIN_EVENT_TABLE ( RadarFrame, wxFrame )

    EVT_CLOSE    ( RadarFrame::OnClose )
    EVT_MOVE     ( RadarFrame::OnMove )
    EVT_SIZE     ( RadarFrame::OnSize )
//    EVT_PAINT    ( RadarFrame::paintEvent)
    EVT_COMBOBOX ( cbRangeId, RadarFrame::OnRange)
    EVT_CHECKBOX ( cbNorthUpId, RadarFrame::OnNorthUp )
    EVT_CHECKBOX ( cbBearingLineId, RadarFrame::OnBearingLine )
    EVT_TIMER    ( tmRefreshId, RadarFrame::OnTimer )
    // EVT_TIMER    ( tmTTSId, RadarFrame::TTSPlaySoundTimer )
    // EVT_BUTTON   ( soundPlayId, RadarFrame::TTSPlaySound )
     EVT_BUTTON   ( connectOptionLinkId, RadarFrame::ReadDataFromFile )
    // EVT_SOCKET   ( SOCKET_ID,     RadarFrame::OnSocketEvent) 
    EVT_SOCKET(SERVER_ID,  RadarFrame::OnServerEvent)
    EVT_SOCKET(SOCKET_ID,  RadarFrame::OnSocketEvent)
END_EVENT_TABLE()

RadarFrame::RadarFrame() 
: pParent(0), 
    pPlugIn(0), 
    m_Timer(0), 
    m_pCanvas(0), 
    m_pNorthUp(0), 
    m_pRange(0),
    m_pBearingLine(0), 
    m_BgColour(),
    m_Ebl(0.0),  
    m_Range(0), 
    m_pViewState(0)
{
    Init();
}

RadarFrame::~RadarFrame( ) {
}


void RadarFrame::Init() {
    GetGlobalColor(_T("DILG1"), &m_BgColour);
    SetBackgroundColour(m_BgColour);
}


bool RadarFrame::Create ( wxWindow *parent, aisradar_pi *ppi, wxWindowID id,
                              const wxString& caption, 
                              const wxPoint& pos, const wxSize& size )
{
    pParent = parent;
    pPlugIn = ppi;
    //long wstyle = wxDEFAULT_FRAME_STYLE;
    long wstyle = wxDEFAULT_FRAME_STYLE|wxSTAY_ON_TOP;
    m_pViewState = new ViewState(pos, size);
    if ( !wxFrame::Create ( parent, id, caption, pos, m_pViewState->GetWindowSize(), wstyle ) ) {
        return false;
    }

    // Add panel to contents of frame
    wxPanel *panel = new wxPanel(this, plCanvasId );
    panel->SetAutoLayout(true);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(vbox);

    // Add Canvas panel to draw upon
    // Use the stored size provided by the pi
    // Create a square box based on the smallest side
    wxBoxSizer *canvas = new wxBoxSizer(wxHORIZONTAL);
    m_pCanvas = new Canvas(panel, this, wxID_ANY, pos, m_pViewState->GetCanvasSize());
    m_pCanvas->SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    wxBoxSizer *cbox = new wxBoxSizer(wxVERTICAL);
    cbox->FitInside(m_pCanvas);
    canvas->Add(m_pCanvas, 1, wxEXPAND);
    vbox->Add(canvas, 1, wxALL | wxEXPAND, 5);


//zhh1 huizhi biaoge
    ShipInfo = new wxGrid( panel, wxID_ANY, wxPoint(0,0), wxDefaultSize, 0 );

     // Grid
	ShipInfo->CreateGrid( DEFAULT_SHIPINFO_GRID_ROWS_NUMBER, 3);
	ShipInfo->EnableEditing( true );
	ShipInfo->EnableGridLines( true );
	//ShipInfo->SetGridLineColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	ShipInfo->EnableDragGridSize( false );
	ShipInfo->SetMargins( 0, 0 );

	// Columns
	ShipInfo->EnableDragColMove( false );
	ShipInfo->EnableDragColSize( true );
	//ShipInfo->SetColLabelSize( 100 );
	ShipInfo->SetColLabelAlignment( wxALIGN_CENTER, wxALIGN_CENTER );
    ShipInfo->SetColSize(0,150);
    ShipInfo->SetColSize(1,150);
    ShipInfo->SetColSize(2,150);
   // ShipInfo->SetColSize(3,130);
    ShipInfo->SetColLabelValue(0,wxT("船舶编号"));
    ShipInfo->SetColLabelValue(1,wxT("真航向"));
    ShipInfo->SetColLabelValue(2,wxT("航速"));
    //ShipInfo->SetColLabelValue(3,wxT("船舶类型"));
    
	// Rows
	ShipInfo->EnableDragRowSize( true );
	ShipInfo->SetRowLabelSize( 150);
	ShipInfo->SetRowLabelAlignment( wxALIGN_CENTER, wxALIGN_CENTER );
    
  
   
   
	 // Label Appearance

	 // Cell Defaults
	 ShipInfo->SetDefaultCellAlignment( wxALIGN_CENTER, wxALIGN_CENTER);
     
     


	  vbox->Add( ShipInfo, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );
      //vbox->MyFit(ShipInfo); 
      
     OwnShipDesion = new wxGrid( panel, wxID_ANY, wxPoint(0,0), wxDefaultSize, 0 );
     OwnShipDesion->CreateGrid(4,1);

     OwnShipDesion->SetColLabelValue(0,wxT("预警与辅助决策"));
    
     OwnShipDesion->SetColSize(0,450);

     OwnShipDesion->EnableDragRowSize( true );
	 OwnShipDesion->SetRowLabelSize( 150);
	 OwnShipDesion->SetRowLabelAlignment( wxALIGN_CENTER, wxALIGN_CENTER );
     OwnShipDesion->SetRowLabelValue(0,wxT("偏航报警"));
     OwnShipDesion->SetRowLabelValue(1,wxT("边界报警"));
     OwnShipDesion->SetRowLabelValue(2,wxT("转向点提示"));
     OwnShipDesion->SetRowLabelValue(3,wxT("辅助决策"));

     OwnShipDesion->SetDefaultCellAlignment( wxALIGN_CENTER, wxALIGN_CENTER);
     

     vbox->Add( OwnShipDesion, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );
     
//zhh1 huizhi biaoge
  
    

    //加一个语音播报窗口
    wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( panel, wxID_ANY, wxT("Instruction:") ), wxHORIZONTAL );
    sbSizer1->SetMinSize( wxSize( -1,150 ) ); 
    
    wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *m_warningInstructionLabel = new wxStaticText(panel, wxID_ANY, wxT("Advices:"), wxDefaultPosition, wxSize(-1,-1), 0);
    m_warningInstructionLabel->Wrap(-1);
    sbSizer1->Add( m_warningInstructionLabel, 0, wxALIGN_CENTER, 5 );
    m_textCtrl1 = new wxTextCtrl( panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 300,50 ), wxTE_MULTILINE | wxTE_READONLY);
    sbSizer1->Add( m_textCtrl1, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );
    // delete wxLog::SetActiveTarget(new wxLogTextCtrl(m_textCtrl1));

    wxBoxSizer *m_buttonBox;
    m_buttonBox = new wxBoxSizer( wxVERTICAL );
    
    m_ConnectOptionButton = new wxButton( panel, connectOptionLinkId, wxT("Addroute"), wxPoint( -1,-1 ), wxDefaultSize, 0 );
    m_buttonBox->Add( m_ConnectOptionButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );
    m_soundButton = new wxButton( panel, soundPlayId, wxT("Open"), wxPoint( -1,-1 ), wxDefaultSize, 0 );
    m_buttonBox->Add( m_soundButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5  );
    m_soundButton->Disable();
    sbSizer1->Add( m_buttonBox, 0, wxALIGN_CENTER, 5 );
    
    vbox->Add(sbSizer1, 0, wxEXPAND | wxALL, 5);
    
    // Add timer
    m_Timer = new wxTimer(this, tmRefreshId);
    m_Timer->Start(2000);

    m_Timer_TTS = new wxTimer(this, tmTTSId);
    m_Timer_TTS->Start(6000);

    vbox->MyFit(this);

    // // Create the socket
    // m_sock = new wxSocketClient();

    // // Setup the event handler and subscribe to most events
    // m_sock->SetEventHandler(*this, SOCKET_ID);
    // m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
    //                     wxSOCKET_INPUT_FLAG |
    //                     wxSOCKET_LOST_FLAG);
    // m_sock->Notify(true);
    // m_busy = false;

    // Create the address - defaults to localhost:0 initially
    IPaddress addr;
    addr.Service(20002);

    wxLogMessage("Creating server at %s:%u", addr.IPAddress(), addr.Service());

    // Create the socket
    m_server = new wxSocketServer(addr);

    // We use IsOk() here to see if the server is really listening
    if (! m_server->IsOk())
    {
        wxLogMessage("Could not listen at the specified port !");
        return true;
    }

    IPaddress addrReal;
    if ( !m_server->GetLocal(addrReal) )
    {
        wxLogMessage("ERROR: couldn't get the address we bound to");
    }
    else
    {
        wxLogMessage("Server listening at %s:%u",
                    addrReal.IPAddress(), addrReal.Service());
    }

    // Setup the event handler and subscribe to connection events
    m_server->SetEventHandler(*this, SERVER_ID);
    m_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
    m_server->Notify(true);

    m_busy = false;
    m_numClients = 0;
#if wxUSE_STATUSBAR
    // Status bar
    CreateStatusBar(2);
#endif // wxUSE_STATUSBAR

    UpdateStatusBar();

    return true;
}


void RadarFrame::SetColourScheme(PI_ColorScheme cs) {
      GetGlobalColor(_T("DILG1"), &m_BgColour);
      SetBackgroundColour(m_BgColour);
      this->Refresh();
}


void RadarFrame::OnClose ( wxCloseEvent& event ) {
    // Stop timer if still running
    m_Timer->Stop();
    m_Timer_TTS->Stop();
    delete m_Timer;
    delete m_Timer_TTS;
    // delete m_sock;
    
    // Save window size
    pPlugIn->SetRadarFrameX(m_pViewState->GetPos().x);
    pPlugIn->SetRadarFrameY(m_pViewState->GetPos().y);
    pPlugIn->SetRadarFrameSizeX(m_pViewState->GetSize().GetWidth());
    pPlugIn->SetRadarFrameSizeY(m_pViewState->GetSize().GetHeight());

    // Cleanup
    RequestRefresh(pParent);
    Destroy();
    pPlugIn->OnRadarFrameClose();
}


void RadarFrame::OnRange ( wxCommandEvent& event ) {
    pPlugIn->SetRadarRange(m_pRange->GetSelection());
    this->Refresh();
}



void RadarFrame::OnNorthUp ( wxCommandEvent& event ) {
    pPlugIn->SetRadarNorthUp(m_pNorthUp->GetValue());
    if (m_pNorthUp->GetValue()) {
        m_Ebl += pPlugIn->GetCog();
    } else {
        m_Ebl -= pPlugIn->GetCog();
    }
    this->Refresh();
}


void RadarFrame::OnTimer( wxTimerEvent& event ) {
    this->Refresh();
}


void RadarFrame::OnBearingLine( wxCommandEvent& event ) {
    this->Refresh();
}


void RadarFrame::OnLeftMouse(const wxPoint &curpos) {
    // if (m_pBearingLine->GetValue()) {
    //     int width      = max(m_pCanvas->GetSize().GetWidth(), (MIN_RADIUS)*2 );
    //     int height     = max(m_pCanvas->GetSize().GetHeight(),(MIN_RADIUS)*2 );
    //     wxPoint center(width/2, height/2);
    //     int dx = curpos.x - center.x;
    //     int dy = center.y - curpos.y;    // top of screen y=0
    //     double tmpradius = sqrt((double)(dx*dx)+(double)(dy*dy));
    //     double angle= dy/tmpradius;
    //     m_Ebl = asin(angle)*(double)((double)180./(double)3.141592653589);
    //     if ( dx >= 0 ) {
    //         m_Ebl = 90 - m_Ebl;
    //     } else {
    //         m_Ebl = 360 - (90 - m_Ebl);
    //     }
    //     this->Refresh();
    // }
}


void RadarFrame::OnMove ( wxMoveEvent& event ) {
    wxPoint p = event.GetPosition();

    // Save window position
    m_pViewState->SetPos(wxPoint(p.x, p.y));
    event.Skip();
}


void RadarFrame::OnSize ( wxSizeEvent& event ) {
    event.Skip();
    if( m_pCanvas )
    {
        wxClientDC dc(m_pCanvas);
        m_pViewState->SetCanvasSize(GetSize());
        m_pViewState->SetWindowSize(GetSize());
        render(dc);
    }
}


void RadarFrame::paintEvent(wxPaintEvent & event) {
    wxAutoBufferedPaintDC   dc(m_pCanvas);
    render(dc);
    event.Skip();
}
#if 0 // Client
/**
 * @name: 根据IP地址和端口号建立socket连接，通过按键m_ConnectOptionButton事件触发
 * @msg:  Open session - 打开连接  Close session - 关闭连接
 * @param {type} 
 * @return: 
 */
void RadarFrame::SetConnectOption( wxCommandEvent& event ){
    if (!m_sock->IsConnected() && m_ConnectOptionButton->GetLabel() == "Open session" ) {
        OpenConnection(wxSockAddress::IPV4);
        // Wait until the request completes or until we decide to give up
        bool waitmore = true;
        while ( !m_sock->WaitOnConnect(-1, 3000) && waitmore )
        {
            
        }
        if (m_sock->IsConnected()) {
            m_ConnectOptionButton->SetLabel("Close session");
            m_soundButton->Enable();
            UpdateStatusBar();
        }
        
    }
    else if ( m_sock->IsConnected() && m_ConnectOptionButton->GetLabel() == "Close session" ) {
        m_sock->Close();
        m_ConnectOptionButton->SetLabel("Open session");
        UpdateStatusBar();
        m_soundButton->SetLabel("Open");
        m_soundButton->Disable();
    }
    else{
        SetStatusText("State : 0", 1);
    }
}

/**
 * @name: 建立连接
 * @msg: 
 * @param {type} 
 * @return: 
 */
void RadarFrame::OpenConnection(wxSockAddress::Family family)
{
    wxUnusedVar(family); // unused in !wxUSE_IPV6 case

    wxIPaddress * addr;
    wxIPV4address addr4;
    addr = &addr4;
#if wxUSE_IPV6
    wxIPV6address addr6;
    if ( family == wxSockAddress::IPV6 )
        addr = &addr6;
    else
#endif
    // Ask user for server address
    wxString hostname = wxGetTextFromUser(
        _("Enter the address of the wxSocket demo server:"),
        _("Connect ..."),
        _("localhost"));
    if ( hostname.empty() )
        return;

    addr->Hostname(hostname);
    addr->Service(3000);

    // we connect asynchronously and will get a wxSOCKET_CONNECTION event when
    // the connection is really established
    //
    // if you want to make sure that connection is established right here you
    // could call WaitOnConnect(timeout) instead
    // wxLogMessage("Trying to connect to %s:%d", hostname, addr->Service());

    m_sock->Connect(*addr, false);
}

void RadarFrame::OnSocketEvent(wxSocketEvent& event)
{
    switch ( event.GetSocketEvent() )
    {
        case wxSOCKET_INPUT:
            wxLogMessage("Input available on the socket");
            break;

        case wxSOCKET_LOST:
            wxLogMessage("Socket connection was unexpectedly lost.");
            UpdateStatusBar();
            break;

        case wxSOCKET_CONNECTION:
            wxLogMessage("... socket is now connected.");
            UpdateStatusBar();
            break;

        default:
            wxLogMessage("Unknown socket event!!!");
            break;
    }
}

void RadarFrame::OnTest(){
    // Disable socket menu entries (exception: Close Session)
    m_busy = true;
    UpdateStatusBar();

    // Tell the server which test we are running
    unsigned char c = 0xCE;
    m_sock->Write(&c, 1);

    // Here we use ReadMsg and WriteMsg to send messages with
    // a header with size information. Also, the reception is
    // event triggered, so we test input events as well.
    //
    // We need to set no flags here (ReadMsg and WriteMsg are
    // not affected by flags)

    m_sock->SetFlags(wxSOCKET_WAITALL);

    wxString s = "data";
    {
        // TODO:本部分发送数据内容（数据备选模式：wxString,json,xml）
        // 如果没有接入AIS或者其他设备，发送某种状态码
    }

    const wxScopedCharBuffer msg1(s.utf8_str());
    size_t len  = wxStrlen(msg1) + 1;
    wxCharBuffer msg2(wxStrlen(msg1));
    wxCharBuffer msgState[2] = {"1","2"};
    m_sock->WriteMsg(msg1, len);

    // Wait until data available (will also return if the connection is lost)
    m_sock->WaitForRead(2);

    if (m_sock->IsData())
    {
        m_sock->ReadMsg(msg2.data(), len);

        if (strcmp(msgState[0],msg2)){
            // 默认无返回数据，或者返回数据为空
        }
        else{
            // 存在返回船舶控制指令，采用espeak(TTS)文本语音指令播放指令语音内容
            do_play(msg2);
        }
    }
    else
        SetStatusText("Timeout ! Test 2 failed.", 1);

    m_busy = false;
    // UpdateStatusBar();
}

void RadarFrame::TTSPlaySound( wxCommandEvent &event) {
    if(m_soundButton->GetLabel() == "Open"){
        m_soundButton->SetLabel("Close");
    }
    else if (m_soundButton->GetLabel() == "Close"){
        m_soundButton->SetLabel("Open");
    }
}

void RadarFrame::Connect(){
    // m_Timer_TTS->Stop();   // timer can only be started from the main thread
    OnTest();
    // m_Timer_TTS->Start(RESTART);
}

void RadarFrame::TTSPlaySoundTimer( wxTimerEvent& event ) {
    //std::cout << "try send xml data" << std::endl;
    
    if(m_soundButton->GetLabel() == "Close"){
        // std::cout << "play sound function is open" << std::endl;
        // function 
        // 如果socket连接已经创建，发送融合后船舶位置数据，并且接收避碰算法处理结果，包含状态码和内容
        if (m_sock->IsConnected() ) {
           thread connectThread(&RadarFrame::Connect, this); // 线程实现
           connectThread.detach();
        }
        
        // TTS(Text to Speech)是通过命令行线程实现，需要提前配置espeak-ng以及其中文发音库.
        // const char *input = "espeak-ng  'TTS(Text to Speech)是通过命令行线程实现，需要提前配置espeak-ng以及其中文发音库.' -s 10 -p 15 -v zh";
        // char *result;
        // std::thread t(executeCMD, input, result);
        // t.detach();
    }
    else{
        // std::cout << "play sound function is close" << std::endl;
        // not connect with the alg model
        
    }
}

void RadarFrame::UpdateStatusBar()
{
#if wxUSE_STATUSBAR
    wxString s;

    if (!m_sock->IsConnected())
    {
        s = "Not connected";
        m_ConnectOptionButton->SetLabel("Open session");
        m_soundButton->SetLabel("Open");
        m_soundButton->Disable();
    }
    else
    {
#if wxUSE_IPV6
        wxIPV6address addr;
#else
        wxIPV4address addr;
#endif

        m_sock->GetPeer(addr);
        s.Printf("%s : %d", addr.Hostname(), addr.Service());
    }

    SetStatusText(s, 1);
#endif // wxUSE_STATUSBAR
}
#else
void RadarFrame::OnServerEvent(wxSocketEvent& event)
{
    wxString s = _("OnServerEvent: ");
    wxSocketBase *sock;

    switch(event.GetSocketEvent())
    {
        case wxSOCKET_CONNECTION : s.Append(_("wxSOCKET_CONNECTION\n")); break;
        default                  : s.Append(_("Unexpected event !\n")); break;
    }

    m_textCtrl1->AppendText(s);

    // Accept new connection if there is one in the pending
    // connections queue, else exit. We use Accept(false) for
    // non-blocking accept (although if we got here, there
    // should ALWAYS be a pending connection).

    sock = m_server->Accept(false);

    if (sock)
    {
        IPaddress addr;
        if ( !sock->GetPeer(addr) )
        {
        wxLogMessage("New connection from unknown client accepted.");
        }
        else
        {
        wxLogMessage("New client connection from %s:%u accepted",
                    addr.IPAddress(), addr.Service());
        }
    }
    else
    {
        wxLogMessage("Error: couldn't accept a new connection");
        return;
    }

    sock->SetEventHandler(*this, SOCKET_ID);
    sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    sock->Notify(true);

    m_numClients++;
    UpdateStatusBar();
}

void RadarFrame::OnSocketEvent(wxSocketEvent& event)
{
    wxString s = _("OnSocketEvent: ");
    wxSocketBase *sock = event.GetSocket();

    // First, print a message
    switch(event.GetSocketEvent())
    {
        case wxSOCKET_INPUT : s.Append(_("wxSOCKET_INPUT\n")); break;
        case wxSOCKET_LOST  : s.Append(_("wxSOCKET_LOST\n")); break;
        default             : s.Append(_("Unexpected event !\n")); break;
    }

    m_textCtrl1->AppendText(s);

    // Now we process the event
    switch(event.GetSocketEvent())
    {
        case wxSOCKET_INPUT:
        {
            // We disable input events, so that the test doesn't trigger
            // wxSocketEvent again.
            sock->SetNotify(wxSOCKET_LOST_FLAG);

            // Which test are we going to run?
            unsigned char c;
            sock->Read(&c, 1);

            switch (c)
            {
                case 0xBE: SendData2Client(sock); break;
                case 0xCE: GetClientResult(sock); break;
                //case 0xDE: Test3(sock); break;
                default:
                wxLogMessage("Unknown test id received from client");
            }

            // Enable input events again.
            sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
            break;
        }
        case wxSOCKET_LOST:
        {
            m_numClients--;

            // Destroy() should be used instead of delete wherever possible,
            // due to the fact that wxSocket uses 'delayed events' (see the
            // documentation for wxPostEvent) and we don't want an event to
            // arrive to the event handler (the frame, here) after the socket
            // has been deleted. Also, we might be doing some other thing with
            // the socket at the same time; for example, we might be in the
            // middle of a test or something. Destroy() takes care of all
            // this for us.

            wxLogMessage("Deleting socket.");
            sock->Destroy();
            break;
        }
        default: ;
    }

    UpdateStatusBar();
}

// convenience functions

void RadarFrame::UpdateStatusBar()
{
#if wxUSE_STATUSBAR
    wxString s;
    s.Printf(_("%d clients connected"), m_numClients);
    SetStatusText(s, 1);
#endif // wxUSE_STATUSBAR
}

void RadarFrame::SendData2Client(wxSocketBase *sock)
{
    TestLogger logtest("SendData2Client");

    // Receive data from socket and send it back. We will first
    // get a byte with the buffer size, so we can specify the
    // exact size and use the wxSOCKET_WAITALL flag. Also, we
    // disabled input events so we won't have unwanted reentrance.
    // This way we can avoid the infamous wxSOCKET_BLOCK flag.

    sock->SetFlags(wxSOCKET_WAITALL);

    

    // Read the data
    // sock->Read(buf.data(), len);
    wxLogMessage("Got the data, sending it back");

    wxString data;
    data += wxString::Format(wxT("$!NDOS:OwnShip,%f,"), gLon)
            +wxString::Format(wxT("%f,"),gLat)
            +wxString::Format(wxT("%f,"),((gSog)*1852/3600))
            +wxString::Format(wxT("%f,"), gCog)
            +wxString::Format(wxT("%f,"), g_n_ownship_length_meters)
            +wxString::Format(wxT("%f\r\n"), g_n_ownship_beam_meters);
    
    ArrayOfPlugIn_AIS_Targets *current_targets = pPlugIn->GetAisTargets();
    
    for (auto it = current_targets->begin(); it != current_targets->end(); ++it )
    {
        data += wxString::Format(wxT("$!NDAR:%i,"),(*it)->MMSI)
                +wxString::Format(wxT("%f,"),(*it)->Lon)
                +wxString::Format(wxT("%f,"),(*it)->Lat)
                +wxString::Format(wxT("%f,"),(((*it)->SOG)*1852/3600))
                +wxString::Format(wxT("%f\r\n"),(*it)->COG);
            
        //data = "$!NDAR:12,12,12,12,12\r\n";
    }
    for (int k = 0; k< 500; k++) 
        data += "$!NDAR:12,12,12,12,12\r\n";
    // Read the size
    
    // sock->Read(&len, 1);
    
    unsigned int bufflen = data.size();
    wxCharBuffer buf(bufflen);
    buf = data.ToUTF8();
    sock->Write(&bufflen, sizeof(bufflen));
    sock->Write(buf, bufflen);
}

//zhh2
void RadarFrame::OwnShipDecisionBroadcast(void){
     
//version1 全部播报，如果数据变化较多，播报会来不及出现语音混杂情况
    // if(YawAlarmBroadcastContent != "0"){
    //     do_play_wav(YawAlarmBroadcastContent);
    // }
    // if(BoundaryAlarmBroadcastContent != "0"){
    //     do_play_wav(BoundaryAlarmBroadcastContent);
    // }
    // if(TurnAlarmBroadcastContent != "0"){
    //     do_play_wav(TurnAlarmBroadcastContent);
    // }
    // if(AidDecisionBroadcastContent != "0"){
    //     do_play_wav(AidDecisionBroadcastContent);
    // }
 //version2 辅助决策暂时不播，其他三项优先级-转向点>边界>偏航   
    // if(TurnAlarmBroadcastContent != "无需播报"){
    //     do_play_wav(TurnAlarmBroadcastContent);
    // }
    // else {
    //     if(BoundaryAlarmBroadcastContent != "无需播报"){
    //         do_play_wav(BoundaryAlarmBroadcastContent);
    //     }
    //     else{
    //         if(YawAlarmBroadcastContent != "无需播报"){
    //             do_play_wav(YawAlarmBroadcastContent);
    //         }
    //     }
    // }

    if(YawAlarmBroadcastContent != "无需播报")
    {
        if (YawAlarmBroadcastContent == "YawAlarmRight")
        {
            YawAlarmFileName = "YawAlarmRight";
            YawAlarmText = "您已向右侧偏航。。。";
            do_play_wav_tts(YawAlarmFileName, YawAlarmText);
        }
        else if (YawAlarmBroadcastContent == "YawAlarmLeft")
        {
            YawAlarmFileName = "YawAlarmLeft";
            YawAlarmText = "您已向左侧偏航。。。";
            do_play_wav_tts(YawAlarmFileName, YawAlarmText);
        }
         
    }
    //sleep(8);

    if(BoundaryAlarmBroadcastContent != "无需播报")
    {
        if (BoundaryAlarmBroadcastContent == "BoundaryAlarmRight")
        {
            BoundaryAlarmFileName = "BoundaryAlarmRight";
            BoundaryAlarmText = "船舶靠近航道右侧。。。";
            do_play_wav_tts(BoundaryAlarmFileName, BoundaryAlarmText);
        }
        else if (BoundaryAlarmBroadcastContent == "BoundaryAlarmLeft")
        {
            BoundaryAlarmFileName = "BoundaryAlarmLeft";
            BoundaryAlarmText = "船舶靠近航道左侧。。。";
            do_play_wav_tts(BoundaryAlarmFileName, BoundaryAlarmText);
        }
    }
    //sleep(8);

    if(TurnAlarmBroadcastContent != "无需播报")
    {
        if(TurnAlarmBroadcastContent == "TurnAlarm8min")
        {
            TurnAlarmFileName = "TurnAlarm8min";
            TurnAlarmText = "8分钟后转向。。。";
            do_play_wav_tts(TurnAlarmFileName, TurnAlarmText);
        }
        else if(TurnAlarmBroadcastContent == "TurnAlarm5min")
        {
            TurnAlarmFileName = "TurnAlarm5min";
            TurnAlarmText = "5分钟后转向。。。";
            do_play_wav_tts(TurnAlarmFileName, TurnAlarmText);
        }
        else if(TurnAlarmBroadcastContent == "TurnAlarm2min")
        {
            TurnAlarmFileName = "TurnAlarm2min";
            TurnAlarmText = "两分钟后转向。。。";
            do_play_wav_tts(TurnAlarmFileName, TurnAlarmText);
        }
    }
    //sleep(8);

    if(AidDecisionBroadcastContent != "无需播报")
    {
        if(AidDecisionBroadcastContent == "AidDecisionMakingBranchingRiver")
        {
            AidDecisionMakingFileName = "AidDecisionMakingBranchingRiver";
            AidDecisionMakingText = "船舶处于汊河口，注意航行。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingDanger")
        {
            AidDecisionMakingFileName = "AidDecisionMakingDanger";
            AidDecisionMakingText = "危险，请注意避让。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingKeepMinus")
        {
            AidDecisionMakingFileName = "AidDecisionMakingKeepMinus";
            AidDecisionMakingText = "附近有危险船舶，请保持方向并减速行驶。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingKeepAdd")
        {
            AidDecisionMakingFileName = "AidDecisionMakingKeepAdd";
            AidDecisionMakingText = "附近有危险船舶，请保持方向并加速行驶。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingLeftMinus")
        {
            AidDecisionMakingFileName = "AidDecisionMakingLeftMinus";
            AidDecisionMakingText = "附近有危险船舶，建议向左转向,并减速行驶。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingLeftAdd")
        {
            AidDecisionMakingFileName = "AidDecisionMakingLeftAdd";
            AidDecisionMakingText = "附近有危险船舶，建议向左转向,并加速行驶。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingLeftKeep")
        {
            AidDecisionMakingFileName = "AidDecisionMakingLeftKeep";
            AidDecisionMakingText = "附近有危险船舶，建议向左转向,并保速行驶。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingRightMinus")
        {
            AidDecisionMakingFileName = "AidDecisionMakingRightMinus";
            AidDecisionMakingText = "附近有危险船舶，建议向右转向,并减速行驶。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingRightAdd")
        {
            AidDecisionMakingFileName = "AidDecisionMakingRightAdd";
            AidDecisionMakingText = "附近有危险船舶，建议向右转向,并加速行驶。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        else if(AidDecisionBroadcastContent == "AidDecisionMakingRightKeep")
        {
            AidDecisionMakingFileName = "AidDecisionMakingRightKeep";
            AidDecisionMakingText = "附近有危险船舶，建议向右转向,并保速行驶。。。";
            do_play_wav_tts(AidDecisionMakingFileName, AidDecisionMakingText);
        }
        //sleep(8);
        
        
    }




}


//zhh3
void RadarFrame::GetClientResult(wxSocketBase *sock)
{
    TestLogger logtest("GetClientResult");
   

    // Read the message
    unsigned int len;
    sock->Read(&len, 4);
    // unsigned char len;
    // sock->Read(&len, 1);
    wxCharBuffer buf(len);
    sock->Read(buf.data(), len);
    
    if (buf.data()){
        wxString sock_buffer;
        ShipInfo->ClearGrid();
        OwnShipDesion->ClearGrid();
        sock_buffer = wxString::FromUTF8(buf.data());
        {
            m_textCtrl1->Clear();
            
            // 处理张梁算法结果
            // "2-10-2-R-M-L
            std::vector<wxString> res = split(sock_buffer, wxT("-"));
            int i = 1; 
            int j;
            // TODO:改成表格显示
            if (res[0] == "out"){
                OwnShipDesion->SetCellValue(0, 0, wxT("请驶入推荐航道"));
                CilentResultSignalZero();
            }
            else if(res[0] == "not"){
                OwnShipDesion->SetCellValue(0, 0, wxT("请顺航道行驶"));
                CilentResultSignalZero();
            }

            else {}
            int k = atoi(res[i].c_str());
            // 调整shipinfo表格行数   
            if( ShipInfo->GetNumberRows() > DEFAULT_SHIPINFO_GRID_ROWS_NUMBER) { 
                if(k > ShipInfo->GetNumberRows()){

                    ShipInfo->InsertRows(k, k - ShipInfo->GetNumberRows(),true);
                }
                else if (k < ShipInfo->GetNumberRows()){
                    if(k > DEFAULT_SHIPINFO_GRID_ROWS_NUMBER){
                        ShipInfo->DeleteRows(k, ShipInfo->GetNumberRows() - k,true);
                    }
                    else if(k < DEFAULT_SHIPINFO_GRID_ROWS_NUMBER){
                        ShipInfo->DeleteRows(DEFAULT_SHIPINFO_GRID_ROWS_NUMBER,k - DEFAULT_SHIPINFO_GRID_ROWS_NUMBER,true );
                    }
                    
                }
            }
            else{
                if(k > ShipInfo->GetNumberRows()){

                    ShipInfo->InsertRows(k, k - ShipInfo->GetNumberRows(),true);
                }

            }
            
            //在shipinfo grid上显示mmsi，cog，sog
            if (k == 0){

                ShipInfo->SetCellValue(0, 0 , wxT("无危险船舶"));
                i++;
            }
            else if(k > 0 ){

                for (; i <= k; i++){

                j = (i-1) * 3 ;
                ShipInfo->SetCellValue(i-1, 0,	res[j+2]);
                ShipInfo->SetCellValue(i-1, 1,	res[j+3]);
                ShipInfo->SetCellValue(i-1, 2,	res[j+4]);
                }
                i = j+5;

            }
                
            
 /*偏航报警时序思想
在此cpp头部定义两个计数全局变量YawAlarmLeftNo和YawAlarmRightNo
和张梁商定，如若重复，那只播报第一次以及每接收到25次数据再播报
故可以在相应环境下，将计数变量++或者置0
边界报警同理
 */           
            YawAlarm = wxT("");
            BoundaryAlarm = wxT("");
            TurnAlarm = wxT("");
            AidDecisionMaking = wxT("");
                
            //偏航报警
            if(res[0] == "in"){
            
                if(res[i]=="R"){
                    YawAlarm.append(wxT("右侧偏航"));
                    if (YawAlarmRightNo % CLIENT_RESULT_PLAY_INTERVAL == 0){
                        //do_play_wav("YawAlarmRight");
                        YawAlarmBroadcastContent = "YawAlarmRight";
                    }
                    else{
                        YawAlarmBroadcastContent = "无需播报";
                    }
                    YawAlarmLeftNo = 0;
                    YawAlarmRightNo ++;
                    
                }
                else if(res[i]=="L"){
                    YawAlarm.append(wxT("左侧偏航"));
                    if (YawAlarmLeftNo % CLIENT_RESULT_PLAY_INTERVAL == 0){
                        //do_play_wav("YawAlarmLeft");
                        YawAlarmBroadcastContent = "YawAlarmLeft";
                    }
                    else{
                        YawAlarmBroadcastContent = "无需播报";
                    }
                    YawAlarmRightNo = 0;
                    YawAlarmLeftNo ++;
                    
                }
                else if(res[i]=="K"){
                    YawAlarm.append(wxT("正常航行"));
                    YawAlarmBroadcastContent = "无需播报";
                    YawAlarmLeftNo = 0;
                    YawAlarmRightNo = 0;
                }

                else{
                    YawAlarmBroadcastContent = "无需播报";    
                    YawAlarmLeftNo = 0;
                    YawAlarmRightNo = 0;
                }
                i++;

                if(res[i]=="0")
                    YawAlarm.append(wxT(""));

                else {
                    YawAlarm.append(res[i]);
                    YawAlarm.append(wxT("m"));
                }
                OwnShipDesion->SetCellValue(0, 0, YawAlarm);
                i++;


/*转向点报警时序思想
分别在8 5 2min进行报警
但只在相应区间报警一次
*/

                //边界报警

                if(res[i]=="R"){
                    BoundaryAlarm.append(wxT("距离右侧航道边界"));
                    if (BoundaryAlarmRightNo % CLIENT_RESULT_PLAY_INTERVAL == 0){
                        //do_play_wav("BoundaryAlarmRight");
                        BoundaryAlarmBroadcastContent = "BoundaryAlarmRight";
                    }
                    else{
                        BoundaryAlarmBroadcastContent = "无需播报";
                    }
                    BoundaryAlarmLeftNo = 0;
                    BoundaryAlarmRightNo ++;
                    
                }
                else if(res[i]=="L"){
                    
                    BoundaryAlarm.append(wxT("距离左侧航道边界"));
                    if (BoundaryAlarmLeftNo % CLIENT_RESULT_PLAY_INTERVAL == 0){
                        //do_play_wav("BoundaryAlarmLeft");
                        BoundaryAlarmBroadcastContent = "BoundaryAlarmLeft";
                    }
                    else{
                        BoundaryAlarmBroadcastContent = "无需播报";
                    }
                    BoundaryAlarmLeftNo ++;
                    BoundaryAlarmRightNo = 0;
                    
                }
                else if(res[i]=="K"){
                    BoundaryAlarm.append(wxT("正常航行"));
                    BoundaryAlarmBroadcastContent = "无需播报";
                    BoundaryAlarmLeftNo = 0;
                    BoundaryAlarmRightNo = 0;
                }
                else{ 
                    BoundaryAlarmBroadcastContent = "无需播报";    
                    BoundaryAlarmLeftNo = 0;
                    BoundaryAlarmRightNo = 0;
                }
                i++;

                if(res[i]=="0")
                    BoundaryAlarm.append(wxT(""));

                else{
                    BoundaryAlarm.append(res[i]);
                    BoundaryAlarm.append(wxT("m"));
                }
                OwnShipDesion->SetCellValue(1, 0, BoundaryAlarm);
                i++;
                
                //转向报警
                char TurnAlarmBuff[1024];
                if(res[i]=="N"){
                    TurnAlarm.append(wxT("正常航行"));
                    TurnAlarmBroadcastContent = "无需播报";
                }
                else{
                    TurnAlarm.append(res[i]);
                    TurnAlarm.append(wxT("秒后到达下一转向点"));

                    if ( (atoi(res[i].c_str())>=475) && (atoi(res[i].c_str())<=495) )
                    {
                        if(TurnAlarm8minNo == 0){
                            //do_play_wav("TurnAlarm8min");
                            TurnAlarmBroadcastContent = "TurnAlarm8min";
                            TurnAlarm8minNo = 1;
                            TurnAlarm5minNo = 0;
                            TurnAlarm2minNo = 0;
                        }
                        else{
                            TurnAlarmBroadcastContent = "无需播报";
                        }
                    }

                    else if ( (atoi(res[i].c_str())>=285) && (atoi(res[i].c_str())<=315) )
                    {
                        if(TurnAlarm5minNo == 0){
                            //do_play_wav("TurnAlarm5min");
                            TurnAlarmBroadcastContent = "TurnAlarm5min";
                            TurnAlarm8minNo = 0;
                            TurnAlarm5minNo = 1;
                            TurnAlarm2minNo = 0;
                        }
                        else{
                            TurnAlarmBroadcastContent = "无需播报";
                        }
                    }
                    else if ( (atoi(res[i].c_str())>=105) && (atoi(res[i].c_str())<=135) )
                    {
                        if(TurnAlarm5minNo == 0){
                            //do_play_wav("TurnAlarm2min");
                            TurnAlarmBroadcastContent = "TurnAlarm2min";
                            TurnAlarm8minNo = 0;
                            TurnAlarm5minNo = 0;
                            TurnAlarm2minNo = 1;
                        }
                        else{
                            TurnAlarmBroadcastContent = "无需播报";
                        }
                    }
                    else  
                    {
                        TurnAlarm8minNo = 0;
                        TurnAlarm5minNo = 0;
                        TurnAlarm2minNo = 0;
                        TurnAlarmBroadcastContent = "无需播报";

                    }
                    
                } 
                OwnShipDesion->SetCellValue(2, 0, TurnAlarm);
                i++;
                
            //辅助决策
                if(res[i]=="R"){
                    AidDecisionMaking.append(wxT("右转向"));
                    
                }
                else if(res[i]=="L"){
                    AidDecisionMaking.append(wxT("左转向"));
                    
                }
                else if(res[i]=="K")
                    AidDecisionMaking.append(wxT("保向"));
                
                else if(res[i]=="0")
                    AidDecisionMaking.append(wxT("船舶处于汊河口，注意航行"));

                else if(res[i]=="D"){
                    AidDecisionMaking.append(wxT("危险，请注意避让"));
                    
                }
                else{      
                }
                i++;

                if(res[i]=="M"){
                    AidDecisionMaking.append(wxT("减速"));
                    
                }
                else if(res[i]=="A"){
                    AidDecisionMaking.append(wxT("加速"));
                    
                }
                else if(res[i]=="K")
                    AidDecisionMaking.append(wxT("保速"));
                
                
                else{      
                }
                OwnShipDesion->SetCellValue(3, 0, AidDecisionMaking);
                //std::thread t1(OwnShipDecisionBroadcast);
                //thread OwnShipDecisionBroadcastThread(&RadarFrame::OwnShipDecisionBroadcast, this); // 线程实现
                //OwnShipDecisionBroadcastThread.detach();

                if(AidDecisionMaking != AidDecisionTemp){
                    if(AidDecisionMaking == wxT("船舶处于汊河口，注意航行")){
                    //do_play_wav("AidDecisionMakingBranchingRiver");
                    AidDecisionBroadcastContent = "AidDecisionMakingBranchingRiver";
                    }
                
                    else if(AidDecisionMaking == wxT("危险，请注意避让")){
                    //do_play_wav("AidDecisionMakingDanger");
                    AidDecisionBroadcastContent = "AidDecisionMakingDanger";
                    }

                    else if(AidDecisionMaking == wxT("保向加速")){
                    //do_play_wav("AidDecisionMakingKeepAdd");
                    AidDecisionBroadcastContent = "AidDecisionMakingKeepAdd";
                    }

                    else if(AidDecisionMaking == wxT("保向减速")){
                    //do_play_wav("AidDecisionMakingKeepMinus");
                    AidDecisionBroadcastContent = "AidDecisionMakingKeepMinus";
                    }

                    else if(AidDecisionMaking == wxT("左转向加速")){
                    //do_play_wav("AidDecisionMakingLeftAdd");
                    AidDecisionBroadcastContent = "AidDecisionMakingLeftAdd";              
                    }
                    
                    else if(AidDecisionMaking == wxT("左转向保速")){
                    //do_play_wav("AidDecisionMakingLeftKeep");
                    AidDecisionBroadcastContent = "AidDecisionMakingLeftKeep";
                    }

                    else if(AidDecisionMaking == wxT("左转向减速")){
                    //do_play_wav("AidDecisionMakingLeftMinus");
                    AidDecisionBroadcastContent = "AidDecisionMakingLeftMinus";
                    }

                    else if(AidDecisionMaking == wxT("右转向加速")){
                    //do_play_wav("AidDecisionMakingRightAdd");
                    AidDecisionBroadcastContent = "AidDecisionMakingRightAdd";              
                    }
                    
                    else if(AidDecisionMaking == wxT("右转向保速")){
                    //do_play_wav("AidDecisionMakingRightKeep");
                    AidDecisionBroadcastContent = "AidDecisionMakingRightKeep";
                    }

                    else if(AidDecisionMaking == wxT("右转向减速")){
                    //do_play_wav("AidDecisionMakingRightMinus");
                    AidDecisionBroadcastContent = "AidDecisionMakingRightMinus";
                    }

                    else{          
                        AidDecisionBroadcastContent = "无需播报";     
                    }
                
                }
                else{
                    AidDecisionBroadcastContent = "无需播报";
                }
            }
            AidDecisionTemp = AidDecisionMaking;
            //t1.join(); 
            thread OwnShipDecisionBroadcastThread(&RadarFrame::OwnShipDecisionBroadcast, this); // 线程实现
            OwnShipDecisionBroadcastThread.detach();
        }
        wxString s = "Get Message and Prase Right";
        unsigned int bufflen = s.size();
        wxCharBuffer buff(bufflen);
        buff = s.ToUTF8();
        // Write it back
        sock->Write(&bufflen, 4);
        sock->Write(buff, bufflen);
        return ;
    }

    
    
    wxString s = "Something wrong!";
    unsigned int bufflen = s.size();
    wxCharBuffer buff(bufflen);
    buff = s.ToUTF8();
    // Write it back
    sock->Write(&bufflen, sizeof(bufflen));
    sock->Write(buff, bufflen);
    
}


void RadarFrame::Test3(wxSocketBase *sock)
{
    
}
#endif

void RadarFrame::render(wxDC& dc)     {
    m_Timer->Start(RESTART);
    
    // Calculate the size based on paint area size, if smaller then the minimum
    // then the default minimum size applies
    int width  = max(m_pCanvas->GetSize().GetWidth(), (MIN_RADIUS)*2 );
    int height = max(m_pCanvas->GetSize().GetHeight(),(MIN_RADIUS)*2 );
    wxSize       size(width, height);
    wxPoint      center(width/2, height/2);
    int radius = max((min(width,height)/2),MIN_RADIUS);
    
    //    m_pCanvas->SetBackgroundColour (m_BgColour);
    renderRange(dc, center, size, radius);
    ArrayOfPlugIn_AIS_Targets *AisTargets = pPlugIn->GetAisTargets();
	if ( AisTargets ) {
        renderBoats(dc, center, size, radius, AisTargets);
    }
}


void RadarFrame::TrimAisField(wxString *fld) {
    assert(fld);
    while (fld->Right(1)=='@' || fld->Right(1)==' ') {
        fld->RemoveLast();
    }
}


void RadarFrame::renderBoats(wxDC& dc, wxPoint &center, wxSize &size, int radius, ArrayOfPlugIn_AIS_Targets *AisTargets ) {
    // Determine orientation
    double offset=pPlugIn->GetCog();
    // if (m_pNorthUp->GetValue()) {
    //     offset=0;
    // }

    // Get display settings
    bool   m_ShowMoored=pPlugIn->ShowMoored();
    double m_MooredSpeed=pPlugIn->GetMooredSpeed();
    bool   m_ShowCogArrows=pPlugIn->ShowCogArrows();
    int    m_CogArrowMinutes=pPlugIn->GetCogArrowMinutes();

    // Show other boats and base stations
    Target    dt;
//    ArrayOfPlugIn_AIS_Targets *AisTargets = pPlugIn->GetAisTargets();
    PlugIn_AIS_Target *t;
    ArrayOfPlugIn_AIS_Targets::iterator it;
    wxString  Name;
#if 0
    // Set generic details for all targets
    dt.SetCanvas(center,radius, m_BgColour);
    dt.SetNavDetails(RangeData[m_pRange->GetSelection()], offset, m_ShowCogArrows, m_CogArrowMinutes);

    //TODO: 融合模块_1(统计加权融合模型) 数据map队列输入 map<<mmsi, class>, list<PlugIn_AIS_Target>>
    static std::map<std::pair<int, int>, std::list<PlugIn_AIS_Target> > Ais_Target;
    std::map<std::pair<int, int>, PlugIn_AIS_Target > Now_Ais_Target;
    
    for( it = (*AisTargets).begin(); it != (*AisTargets).end(); ++it ) {
        t = *it;
        // Only display well defined targets
        if (t->Range_NM>0.0 && t->Brg>0.0) {
            //t->MMSI, Name, t->Range_NM, t->Brg, t->COG, t->SOG, t->Class, t->alarm_state, t->ROTAIS
            std::pair<int, int> the_key(t->MMSI, t->Class);
            if (t->MID!=5 && t->MMSI){
                // 过滤ais静态数据
                Now_Ais_Target[the_key] = *t;
                if (Ais_Target.find(the_key)!=Ais_Target.end()){
                    // 去除utc时间相同的目标
                    if( t->Utc_hour!=Ais_Target[the_key].back().Utc_hour
                    ||t->Utc_min!=Ais_Target[the_key].back().Utc_min
                    ||t->Utc_sec!=Ais_Target[the_key].back().Utc_sec){
                        Ais_Target[the_key].push_back(*t);
                    }
                    if (Ais_Target[the_key].size()>10){
                        Ais_Target[the_key].pop_front();
                    }
                }
                else{
                    std::list<PlugIn_AIS_Target> newlist;
                    newlist.push_back(*t);
                    Ais_Target[the_key] = newlist;
                }
            }
        }
    }
    
    unsigned N = Ais_Target.size();
    int size_target[N];
    for (decltype(N) i = 0; i<N; ++i){
        size_target[i] = 0;
    }
    // if () //队列长度判断 距离本船距离 大于这一距离的船舶不需要考虑
    for (auto ais_it = Ais_Target.begin(); ais_it!=Ais_Target.end(); ++ais_it){
        
        if (size_target[distance(Ais_Target.begin(), ais_it)] == 1){
            std::cout << distance(Ais_Target.begin(), ais_it) << "当前目标已经融合\t";
            continue; //当前目标已经融合
        }
        
        auto ais_it_c = ais_it;
        while(++ais_it_c!=Ais_Target.end()){
            if (ais_it->first.second == ais_it_c->first.second || size_target[distance(Ais_Target.begin(), ais_it_c)]==1){
                continue; // 同类目标 或者 已经融合目标
            }
            else{
                //计算Radar_a和AIS_b的关联度
                //TODO:重复检测，减枝
                int m = 0;  //关联点数
                auto i = ais_it->second.begin();
                auto j = ais_it_c->second.begin();
                for(; i !=ais_it->second.end() && j != ais_it_c->second.end(); ++i, ++j)
                {
                    //距离的关联度函数
                    volatile double dt = fabs(i->Range_NM - j->Range_NM);
                    volatile double ad = exp(-0.1*(dt*dt)/(r1*r1));
                    //方位的关联度函数
                    volatile double ht = fabs(i->Brg - j->Brg);
                    volatile double ah = exp(-0.1*(ht*ht)/(r2*r2));
                    //速度的关联度函数
                    volatile double st = fabs(i->SOG - j->SOG);
                    volatile double as = exp(-0.5*(st*st)/(r3*r3));
                    //航向的关联度函数
                    volatile double ct = fabs(i->COG - j->COG);
                    volatile double ac = exp(-0.5*(ct*ct)/(r4*r4));

                    //该点的综合关联度,加权求和的方法
                    volatile double a = 0.35*ad + 0.35*ah + 0.15*as + 0.15*ac;
                    if(a > 0.6)
                    {
                        m++;
                    }
                }
                if( m > 8 ){

                    #if 1  // kalman
                    
                    #else  // 加权融合
                    // 航迹最后一个点 融合
                    std::cout << "Erase fusion point:" << std::endl;
                    size_target[std::distance(Ais_Target.begin(), ais_it)] = 1;
                    size_target[std::distance(Ais_Target.begin(), ais_it_c)] = 1;
                    std::cout << "Type:" << Now_Ais_Target[ais_it->first].Class << "\tName:" << Now_Ais_Target[ais_it->first].ShipName << Now_Ais_Target[ais_it->first].MMSI << std::endl;
                    std::cout << "Type:" << Now_Ais_Target[ais_it_c->first].Class << "\tName:" << Now_Ais_Target[ais_it_c->first].ShipName << Now_Ais_Target[ais_it_c->first].MMSI << std::endl;
                    // strcat(Now_Ais_Target[ais_it_c->first].ShipName, Now_Ais_Target[ais_it->first].ShipName);
                    
                    char buf[43];
                    strcpy(buf, Now_Ais_Target[ais_it_c->first].ShipName);
                    strcat(buf, Now_Ais_Target[ais_it->first].ShipName);
                    
                    
                    Now_Ais_Target.erase(ais_it->first);
                    //Now_Ais_Target.erase(ais_it_c->first);
                    
                    Now_Ais_Target[ais_it_c->first].Range_NM = a11*ais_it->second.back().Range_NM + a12*ais_it_c->second.back().Range_NM;
                    Now_Ais_Target[ais_it_c->first].Brg = a21*ais_it->second.back().Brg + a22*ais_it_c->second.back().Brg;
                    Now_Ais_Target[ais_it_c->first].SOG = a31*ais_it->second.back().SOG + a32*ais_it_c->second.back().SOG;
                    Now_Ais_Target[ais_it_c->first].COG = a41*ais_it->second.back().COG + a42*ais_it_c->second.back().COG;

                    #endif
                                       
                }
                
            }
        }
    }

    //TODO: 融合模块_2(分布式卡尔曼融合模型) by zhh
    {
        
    }
    
    for(auto it_ = (Now_Ais_Target).begin(); it_ != (Now_Ais_Target).end(); ++it_ ) {
        t        = &(it_->second);
        // Only display well defined targets
        if (t->Range_NM>0.0 && t->Brg>0.0) {
            if (m_ShowMoored 
                || t->Class == BASE_STATION
                ||(!m_ShowMoored && t->SOG > m_MooredSpeed)
            ) {
                Name     = wxString::From8BitData(t->ShipName);
                TrimAisField(&Name);
                dt.SetState(t->MMSI, Name, t->Range_NM, t->Brg, t->COG, t->SOG, 
                    t->Class, t->alarm_state, t->ROTAIS
                );
                dt.Render(dc);
            }
        }
    }
#else
    for( it = (*AisTargets).begin(); it != (*AisTargets).end(); ++it ) {
        t        = *it;
        // Only display well defined targets
        if (t->Range_NM>0.0 && t->Brg>0.0) {
            if (m_ShowMoored 
                || t->Class == BASE_STATION
                ||(!m_ShowMoored && t->SOG > m_MooredSpeed)
            ) {
                Name     = wxString::From8BitData(t->ShipName);
                TrimAisField(&Name);
                dt.SetState(t->MMSI, Name, t->Range_NM, t->Brg, t->COG, t->SOG, 
                    t->Class, t->alarm_state, t->ROTAIS
                );
                dt.Render(dc);
            }
        }
    }
#endif
}


void RadarFrame::renderRange(wxDC& dc, wxPoint &center, wxSize &size, int radius) {
    // Draw the circles
    // dc.SetBackground(wxBrush(m_BgColour));
    // dc.Clear();
    // dc.SetBrush(wxBrush(wxColour(0,0,0),wxTRANSPARENT));
    // dc.SetPen( wxPen( wxColor(128,128,128), 1, wxSOLID ) );
	// dc.DrawCircle( center, radius);
    // dc.SetPen( wxPen( wxColor(128,128,128), 1, wxDOT ) );
    // dc.DrawCircle( center, radius*0.75 );
    // dc.DrawCircle( center, radius*0.50 );
    // dc.DrawCircle( center, radius*0.25 );
    // dc.SetPen( wxPen( wxColor(128,128,128), 2, wxSOLID ) );
    // dc.DrawCircle( center, 10 );

    // // Draw the crosshairs
    // dc.SetPen( wxPen( wxColor(128,128,128), 1, wxDOT ) );
    // dc.DrawLine( size.GetWidth()/2,0, size.GetWidth()/2, size.GetHeight());
    // dc.DrawLine( 0,size.GetHeight()/2, size.GetWidth(), size.GetHeight()/2);

    // // Draw the range description
    // wxFont fnt = dc.GetFont();
    // fnt.SetPointSize(14);
    // int fh=fnt.GetPointSize();
    // dc.SetFont(fnt);
    // float Range=RangeData[m_pRange->GetSelection()];
    // dc.DrawText(wxString::Format(wxT("%s %2.2f"), _("Range"),Range  ), 0, 0); 
 //   dc.DrawText(wxString::Format(wxT("%s %2.2f"), _("Ring "), Range/4), 0, fh+TEXT_MARGIN); 

    // Draw the orientation info
	// wxString dir;
	// if (m_pNorthUp->GetValue()) {
	// 	dir=_("North Up");
    //     // Draw north, east, south and west indicators
    //     dc.SetTextForeground(wxColour(128,128,128));
    //     dc.DrawText(_("N"), size.GetWidth()/2 + 5, 0);
    //     dc.DrawText(_("S"), size.GetWidth()/2 + 5, size.GetHeight()-dc.GetCharHeight());
    //     dc.DrawText(_("W"), 5, size.GetHeight()/2 - dc.GetCharHeight());
    //     dc.DrawText(_("E"), size.GetWidth() - 7 - dc.GetCharWidth(), size.GetHeight()/2 - dc.GetCharHeight());
    // } else {
    //     dir=_("Course Up"); 
    //     // Display our own course at to top
    //     double offset=pPlugIn->GetCog();
    //     dc.SetTextForeground(wxColour(128,128,128));
    //     int cpos=0;
    //     dc.DrawText(wxString::Format(_T("%3.0f\u00B0"),offset), size.GetWidth()/2 - dc.GetCharWidth()*2, cpos);
    // }
    // dc.SetTextForeground(wxColour(0,0,0));
    // dc.DrawText(dir,  size.GetWidth()-dc.GetCharWidth()*dir.Len()-fh-TEXT_MARGIN, 0); 
    // if (m_pBearingLine->GetValue()) {
    //     // Display and estimated bearing line
    //     int x = center.x;
    //     int y = center.y;
    //     double angle = m_Ebl *(double)((double)3.141592653589/(double)180.);
    //     x += sin(angle) * (radius + 20);
    //     y -= cos(angle) * (radius + 20);
    //     dc.DrawLine(center.x, center.y, x, y);
    //     int tx = center.x + sin(angle) * (radius - 20) - dc.GetCharWidth() * 1.5;
    //     int ty = center.y - cos(angle) * (radius - 20);
    //     double offset=0.;
    //     if ( !m_pNorthUp->GetValue() ) {
    //         offset = pPlugIn->GetCog();
    //     }
    //     dc.SetTextForeground(wxColour(128,128,128));
    //     dc.DrawText(wxString::Format(_T("%3.1d\u00B0"),(int)(m_Ebl+offset)%360),tx,ty);
    // }
}
void RadarFrame::ReadDataFromFile(wxCommandEvent& event){
    bool yellowline, greenline, megentaline;
    ifstream data;
    wxArrayString allpaths;
    string path;
    wxString wildcard = wxT("TXT files (*.txt)|*.txt");
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog(this, "Test for file pick", "/home/nlq/ship-boundary/", defaultFilename,
        wildcard,wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        dialog.GetPaths(allpaths);
        int filterIndex = dialog.GetFilterIndex();
    }
    for(int i = 0; i < allpaths.size(); i++ )
   {
    path = allpaths[i];
    data.open(path);
    string s;
    
    PlugIn_Route *newRouteLine = new PlugIn_Route();
    
    while (getline(data, s)) //getline(data,s)是逐行读取data中的文件信息
    {
        vector<wxString> oneLineRouteData = split(s, "\t"); // number lat_1 lat_2 lon_1 lon_2
        int number_route = wxAtoi(oneLineRouteData[0]);
        double lat_1, lat_2, lon_1, lon_2;
        if (number_route > 0 
        && oneLineRouteData[1].ToDouble(&lat_1) 
        && oneLineRouteData[2].ToDouble(&lat_2) 
        && oneLineRouteData[3].ToDouble(&lon_1) 
        && oneLineRouteData[4].ToDouble(&lon_2) ){
            double newLat = lat_1 + lat_2/60.0;
            double newLon = lon_1 + lon_2/60.0;
            PlugIn_Waypoint *newWayPoint = new PlugIn_Waypoint(newLat, newLon, "empty", "Line");

            newRouteLine->pWaypointList->Append(newWayPoint);
        }
    }
    data.close();

    string::size_type iPos = path.find_last_of('/') + 1;
    string filename = path.substr(iPos, path.length() - iPos);
    string name = filename.substr(0, filename.rfind("."));
    newRouteLine->m_NameString = name; 
    
    yellowline = ((name =="1-1")||(name =="1-2" )||(name == "1-5")||(name == "1-6")||(name == "2-3")||(name == "2-4")||
    (name == "2-6")||(name == "2-7")||(name == "4-1")||(name == "4-2")||(name == "4-5")||(name == "4-7")||(name == "4-8")||
    (name == "4-9")||(name == "4-10")||(name == "5-1")||(name == "5-2")||(name == "7-1")||(name == "7-2")||(name == "9-1")||
    (name == "9-2")||(name == "9-3")||(name == "9-4")||(name == "10-1")||(name == "11-1")||(name == "11-3"));

    greenline = ((name =="1-3")||(name =="1-7" )||(name == "2-1")||(name == "2-2")||(name == "3-1")||(name == "3-2")||
    (name == "3-3")||(name == "3-4")||(name == "3-5")||(name == "3-7")||(name == "4-3")||(name == "5-3")||
    (name == "5-4")||(name == "5-5")||(name == "6-1")||(name == "6-3")||(name == "7-4")||(name == "8-1")||(name == "8-3")||
    (name == "9-6")||(name == "10-2")||(name == "10-3")||(name == "11-4")||(name == "11-6"));

    megentaline = ((name =="隔离1-1")||(name =="隔离1-2" )||(name == "隔离2-1")||(name == "隔离2-2")||(name == "隔离3-1")||
    (name == "隔离3-2")||(name == "隔离4-1")||(name == "隔离4-2")||(name == "隔离4-3")||(name == "隔离4-4")||(name == "隔离4-6")||
    (name == "隔离4-7")||(name == "隔离4-8")||(name == "隔离4-9")||(name == "隔离5-1")||(name == "隔离5-2")||(name == "隔离6-1")||(name == "隔离6-2"));

    if(yellowline)
    {
        AddPlugInRoute2(newRouteLine,"DarkCyan",3,wxPENSTYLE_LONG_DASH);
    }
    else if(greenline)
    {
        AddPlugInRoute2(newRouteLine,"Red",3,wxPENSTYLE_SOLID);
    }
    else if(megentaline)
    {

        AddPlugInRoute2(newRouteLine,"DarkRed",1,wxPENSTYLE_SOLID);
    }
    else
    {
        AddPlugInRoute2(newRouteLine,"Black",1,wxPENSTYLE_SOLID);
    }
    
    
    
    delete newRouteLine;
    }
    // delete newRouteLine;
}