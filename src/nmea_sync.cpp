/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Convert gpx-string to a base64 string and send is as nmea sentences
 * Author:   David Register / Dirk Smits
 *
 ***************************************************************************
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
 ***************************************************************************
 */


#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/zstream.h>
#include "nmea_sync.h"
#include <NavObjectCollection.h>
#include "pugixml.hpp"
#include "navutil.h"

#include <string>
#include <sstream>
#include <algorithm>
#include <string>
#include <wx/tokenzr.h>
#include <OCPN_DataStreamEvent.h>
#include <multiplexer.h>
#include <iostream>
#include <routemanagerdialog.h>

extern Multiplexer        *g_pMUX;
extern RouteManagerDialog *pRouteManagerDialog;
extern MyFrame            *gFrame;
RxMessages                *g_pNmeaSync;


/* Routes, waypoints and tracks are send as nmea sentences. In princip we do take a gpx file as string, remove the header part (we add it again after receiving, but no need to send static data). The gpx string is transferred to base64 alla ais and spread over as much sentences as needed.
There are two kinds of nmea sentences used:
  a) MessageNo 0, $OPCPN, MessID, MessNo, NumberOfLines, SizeOfMessage(bytes), UserName, UserComputerName *FF
  b) Message 1+,  $OPCPN, MessID, MessNo, datapayload *FF
*/

SendGpxMessage::SendGpxMessage()
{}
SendGpxMessage::~SendGpxMessage()
{}

void SendGpxMessage::SendRoute( RouteList *pRoutes )
{
    pgpx.AddGPXRoutesList( pRoutes ); // add routes to xml doc
    std::stringstream ss8;
    for (pugi::xml_node child: pgpx.child("gpx").children()){
        child.print(ss8, "", pugi::format_raw);} //extract only the 'data' from the xml

    SendMessage(ss8.str());  
}

void SendGpxMessage::SendTrack( TrackList *pTracks)
{
    pgpx.AddGPXTracksList( pTracks ); // add tracks to xml doc
    std::stringstream ss8;
    for (pugi::xml_node child: pgpx.child("gpx").children()){
        child.print(ss8, "", pugi::format_raw);} //extract only the 'data' from the xml

    SendMessage(ss8.str());  
}

void SendGpxMessage::SendPoint( RoutePointList *pWpoints )
{
    pgpx.AddGPXPointsList( pWpoints ); // add points to xml doc
    std::stringstream ss8;
    for (pugi::xml_node child: pgpx.child("gpx").children()){
        child.print(ss8, "", pugi::format_raw);} //extract only the 'data' from the xml

    SendMessage(ss8.str());  
}

void SendGpxMessage::SendMessage( std::string Message8b)
{
    char* buffer;
    size_t buf_size; 
 
    SeedRandom();
    int MessId;
    if ( !g_pNmeaSync )
        g_pNmeaSync = new RxMessages(gFrame);
    // Generate a new random ID that is not in use
    do {
        MessId = GetRandomNumber(0, 4095);
    }
    while ( ( g_pNmeaSync->RxMessMap.find(-MessId) != g_pNmeaSync->RxMessMap.end() ) &&
            ( g_pNmeaSync->RxMessMap.find(MessId) != g_pNmeaSync->RxMessMap.end() ) );
        
    // Add MessId as negative to recognize a self send messages later
    g_pNmeaSync->RxMessMap[-MessId] = nullptr;
    
    //todo also check for usen rx id's

    std::string Message6b = EncodeStr(Message8b, true);
    int i = 1;
    size_t Message6bIndex = 0;
    
    while ( Message6bIndex <  Message6b.length() )
    {
        std::string Sentencedata = _GPX_NMEA_ID  ;
        Sentencedata.append(",");
        Sentencedata.append( EncodeInt( MessId ) ).append(",");        
        Sentencedata.append(EncodeInt(i)).append(",");
        int maxDataLenght = ((int)(82 - 3 - Sentencedata.length())/4)*4;
        Sentencedata.append( Message6b.substr(Message6bIndex, maxDataLenght ));
        AddComputeChecksum(Sentencedata);
        OCPN_DataStreamEvent event( wxEVT_OCPN_DATASTREAM, 0 );;
        event.SetNMEAString( Sentencedata);
        g_pMUX->AddPendingEvent( event );
        Message6bIndex += maxDataLenght;
        i++;
    }
    //MessageNo 0, $OPCPN, MessID, MessNo, NumberOfLines, SizeOfMessage(bytes), UserName, UserComputerName *FF
    std::string Sentencedata0  =  _GPX_NMEA_ID ;
    Sentencedata0.append(",");
    Sentencedata0.append( EncodeInt( MessId ) ).append(",");
    Sentencedata0.append(EncodeInt(0)).append(",");
    Sentencedata0.append(EncodeInt(i)).append(",");
    Sentencedata0.append(EncodeInt(Message8b.length())).append(",");
    Sentencedata0.append(EncodeStr( wxGetUserName().ToStdString() )).append(",");
    Sentencedata0.append(EncodeStr( wxGetHostName().ToStdString() ));
    AddComputeChecksum(Sentencedata0);
    OCPN_DataStreamEvent event( wxEVT_OCPN_DATASTREAM, 0 );;
    event.SetNMEAString( Sentencedata0 );
    g_pMUX->AddPendingEvent( event );
}

std::string SendGpxMessage::EncodeInt(const int x)
{   // only for integers < 16515073 
    std::string ret;
    long xx = x;
    if (xx & 0x0fc0000 ) ret.push_back( ascii2ais((xx >> 18) & 0x0000003f) );
    if (xx & 0x0fff000 ) ret.push_back( ascii2ais((xx >> 12) & 0x0000003f) );
    if (xx & 0x0ffffc0 ) ret.push_back( ascii2ais((xx >> 6 ) & 0x0000003f) ); 
    ret.push_back(ascii2ais( xx & 0x0000003f ) );
    return ret;
}

std::string SendGpxMessage::EncodeStr(std::string byte8str, bool ZipIt) { 
    std::string ret="";
    char* buffer;
    size_t buf_size;
    if (ZipIt){        
        const char *cstr = byte8str.c_str();
        wxMemoryOutputStream out;
        wxZlibOutputStream zlib(out, wxZ_BEST_COMPRESSION, wxZLIB_NO_HEADER);
        zlib.Write(cstr, strlen(cstr));
        zlib.Close();

        buf_size = out.GetLength();
        buffer = new char[buf_size];
        out.CopyTo(buffer, buf_size);
    }
    else{
        buffer = &byte8str[0];
        buf_size=byte8str.length()+1;        
    }
    std::vector<char> data(buffer, buffer + buf_size);

    for (size_t i=0; i < data.size(); i+=3){
        unsigned long int x = 0;
        for (size_t j = i; j < i+3; j++){
            x = x << 8;
            if ( j < data.size() ) x = x | (( data[j] ) & 0x000000ff);
        }
        if (( x & 0x00ffffff ) || i < data.size()) ret.push_back( ascii2ais((x >> 18) & 0x0000003f) );
        if (( x & 0x0003ffff ) || i < data.size()) ret.push_back( ascii2ais((x >> 12) & 0x0000003f) );
        if (( x & 0x00000fff ) || i +1 < data.size()) ret.push_back( ascii2ais((x >> 6) & 0x0000003f) ); 
        if (( x & 0x0000003f ) || i +2 < data.size()) ret.push_back(ascii2ais( x & 0x0000003f ) ); 
    }
    return ret;
}

void SendGpxMessage::AddComputeChecksum( std::string &str  ) 
{
    unsigned char checksum_value = 0;

    int string_length = str.length();
    int index = 1; // Skip over the $ at the begining of the sentence

    while( index < string_length    &&
        str.at( index ) != '*' &&
        str.at( index ) != 0x0D &&
        str.at( index ) != 0x0A )
    {
        checksum_value ^= (char)str.at( index );
            index++;
    }
    wxString cs = wxString::Format(_("*%02X"), checksum_value);
    str.append(cs.mb_str() );
}
int SendGpxMessage::GetRandomNumber(int range_min, int range_max)
{
    long u = (long)wxRound(((double)rand() / ((double)(RAND_MAX) + 1) * (range_max - range_min)) + range_min);
    return (int)u;
}

void SendGpxMessage::SeedRandom()
{
    /* Fill with random. Miliseconds hopefully good enough for our usage, reading /dev/random would be much better on linux and system guid function on Windows as well */
    wxDateTime x = wxDateTime::UNow();
    long seed = x.GetMillisecond();
    seed *= x.GetTicks();
    srand(seed);
}

///////////////////////////////////////////////////////////////////////////////
// Receive Messages
///////////////////////////////////////////////////////////////////////////////

RxMessages::RxMessages(wxFrame *parent)
{
    counter=0;
    parentt = parent;
    MyTimer.Start(1000*60); //set to once every 5 minutes
    //  Create/connect a dynamic event handler slot for wxEVT_OCPN_DATASTREAM(s)
    Connect(wxEVT_OCPN_DATASTREAM, (wxObjectEventFunction)(wxEventFunction)&RxMessages::OnEvtSYNC);

    MyTimer.Connect( wxEVT_TIMER, wxTimerEventHandler(RxMessages::OnTimer), NULL, this);
}
RxMessages::~RxMessages()
{
    for(std::map<int, RxMessage*>::iterator ci=RxMessMap.begin(); ci!=RxMessMap.end(); ++ci)
        delete ci->second;
    RxMessMap.clear();
}

void RxMessages::MessageReceived(std::string message, RxMessage* RxM)
{
    NavObjectCollection1 *pgpx = new NavObjectCollection1;
    pgpx->SetRootGPXNode();
    
        // Read XML string into temporary document to copy in gpx-xml
    pugi::xml_document tmpDoc;
    if (tmpDoc.load(message.c_str()))
        for (pugi::xml_node_iterator it = tmpDoc.begin(); it != tmpDoc.end(); ++it){
            pgpx->m_gpx_root.append_copy(*it);
        }
 
    RxAcceptDlg* RxADlg = new RxAcceptDlg(parentt, wxID_ANY, _T("Message Received"), wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP );
    
    RxADlg->m_SenderText->SetLabel( RxADlg->m_SenderText->GetLabel().Append(
        RxM->SenderUserName ) );
    RxADlg->m_SenderComputerText->SetLabel( RxADlg->m_SenderComputerText->GetLabel().Append(
        RxM->SenderComputerName ) ); 
    RxADlg->CenterOnParent();

    switch( RxADlg->ShowModal() ){
        case wxID_SAVE:
        {   wxString SugName = wxDateTime::Now().FormatISOCombined(' ').Append("From ").Append(RxM->SenderComputerName).Append(".gpx");
            wxFileName fn = exportFileName(parentt, SugName );
            if (fn.IsOk())
                pgpx->SaveFile(fn.GetFullPath());
            break;}
        case wxID_OK:
        {   int wpt_dups;
            pgpx->LoadAllGPXObjects( !pgpx->IsOpenCPN(), wpt_dups );
            if(wpt_dups > 0) {
                OCPNMessageBox(parentt, wxString::Format(_T("%d ")+_("duplicate waypoints detected during import and ignored."), wpt_dups), _("OpenCPN Info"), wxICON_INFORMATION|wxOK, 10);
            }
            if ( pRouteManagerDialog )
                if ( pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateLists();
            break;}         
        default:
            
            break;
        break;
    }
    RxM->AllLinesReceived = true;
    delete RxADlg;
}

//     Handle events from muliplexer DataStream
void RxMessages::OnEvtSYNC( OCPN_DataStreamEvent& event )
{
    event.Skip();
    int messageID, SentenceNr, NrOfLines, SizeOfMessage ;
    wxString message = event.ProcessNMEA4Tags();
    std::string UserName, UserComputerName, mess;

    if( !message.IsEmpty() )
    {
        if( message.Left( wxString( _GPX_NMEA_ID ).length() ).IsSameAs( wxString( _GPX_NMEA_ID ) ) )
        {
            wxStringTokenizer tkz( message, _T(",*") );
            wxString token = tkz.GetNextToken(); //Sentence (OPCPN)
            token = tkz.GetNextToken(); //1) messageID
            messageID=DecodeInt( std::string(token.mb_str()) );
            if (RxMessMap.find(-messageID) != RxMessMap.end() )
                return; // this is a self send messageID
            else{ // check if a record is made already and make on if needed
                if (RxMessMap.find(messageID) == RxMessMap.end() ){                
                    RxMessMap.insert(std::make_pair(messageID, new RxMessage(this) ) );
                }
            }
            std::map<int, RxMessage*>::const_iterator it = RxMessMap.find(messageID);
            token = tkz.GetNextToken(); // 2)sentence nr.
            SentenceNr=DecodeInt( std::string(token.mb_str()) );
            if (SentenceNr !=0){
                token = tkz.GetNextToken(); // 3 message payload
                it->second->AddSentence( SentenceNr, std::string(token.mb_str()) );
            }
            else{ //Sentence Nr is 0
                token = tkz.GetNextToken(); // 3 NrOfLines.
                NrOfLines=DecodeInt( std::string(token.mb_str()) );
                token = tkz.GetNextToken(); // 4 SizeOfMessage.(bytes)
                SizeOfMessage=DecodeInt( std::string(token.mb_str()) );
                token = tkz.GetNextToken();
                UserName = DecodeStr( std::string(token.mb_str() )); // 4 UserName (Sender)
                token = tkz.GetNextToken();
                UserComputerName = DecodeStr( std::string(token.mb_str() ) ); // 4 UserComputerName (Sender)
                it->second->AddSentence0( NrOfLines, SizeOfMessage, UserName, UserComputerName);
            }
            if ( it->second->AllLinesReceived ){
                RxMessMap.erase(it);
            }
        }
    }
}

std::string RxMessages::DecodeStr(std::string input6, bool IsZipped)
{    
    std::vector<char> out;
    std::string Ret;
    for (size_t i=0; i < input6.length(); i+=4){
        unsigned long int x = 0;
        for (size_t j = i; j < i+4; j++){
            x = x << 6;
            if ( j < input6.length() ) x = x | (ais2ascii( input6[j] ) & 0x0000003f);
        }
        if (( x & 0x00ffffff ) || i < input6.length()) out.push_back((char)(x >> 16) & 0x000000ff);
        if (( x & 0x0000ffff ) || i+1 < input6.length())  out.push_back((char)(x >> 8) & 0x000000ff); 
        if (( x & 0x000000ff ) || i+2 < input6.length()) out.push_back((char) x & 0x000000ff);
        }
    out.push_back(0); // Make sure the array is properly null terminated
    
    if (IsZipped){
        wxMemoryInputStream in(&out[0], out.size());
        wxZlibInputStream zlib(in, wxZLIB_NO_HEADER);
        char data[256];
        size_t n;
        while ((n = zlib.Read(data, sizeof(data)).LastRead()) > 0){
            Ret.append(data, n);
        }
    }
    else 
        Ret = std::string(out.begin(), out.end());
    return Ret;
}           

int RxMessages::DecodeInt(const std::string str)
{
    long xx = 0;
    int ret = 0;
    for (size_t i = 0; i< str.length();   i++){
        xx = xx << 6;
        xx = (xx | ( ais2ascii(str[i]) &0x003f));
    }
    ret = xx;
    return ret;
} 

void RxMessages::OnTimer(wxTimerEvent & event)
{ // delete Rx objects if there is 5 minutes or more no activity
    for (auto it = RxMessMap.begin(); it != RxMessMap.end(); ++it){
        if( it->second != NULL ){
            if ( (it->second->TicksLastUpdate + 300) < wxDateTime::Now().GetTicks() ){
                delete it->second;
                wxLogMessage( _("incomplete message deleted") );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// Single Rx message
//////////////////////////////////////////////////////////////////////////////
RxMessage::RxMessage( RxMessages* p)
{
    TotalLines= -1;
    TotalSize = -1;
    SenderUserName = "";
    SenderComputerName = "";
    AllLinesReceived = false;
    parent = p;
}
RxMessage::~RxMessage()
{
    SentencesMap.empty();
}

//MessageNo 0, $OPCPN, MessID, MessNo, NumberOfLines, SizeOfMessage(bytes), UserName, UserComputerName *FF
void RxMessage::AddSentence0(int NrOfLines, int Size, std::string UserName, std::string UserComputerName )
{
    TotalLines= NrOfLines;
    TotalSize = Size;
    SenderUserName = UserName;
    SenderComputerName = UserComputerName;
    if ( IsComplete() ) DecodeFinish();
}
 
void RxMessage::AddSentence(int sNr, std::string s)
{
    SentencesMap.insert( make_pair(sNr, s) );
    if ( IsComplete() ) DecodeFinish();
}

bool RxMessage::IsComplete()
{
    TicksLastUpdate = wxDateTime::Now().GetTicks();
    if ( TotalLines )
        if ((int)SentencesMap.size() == TotalLines-1){
            std::map<int, std::string>::iterator it;
            for ( int i=1; i < TotalLines; i++){
                if ( SentencesMap.find(i) == SentencesMap.end() )
                    return false;
            }
            return true;
        }
    return false;
}

void RxMessage::DecodeFinish()
{
    if (AllLinesReceived) return; //A hack to prevent multiple messages if there is a double connection somehow
    std::string message;
    std::map<int, std::string>::iterator it;
    for ( int i=1; i < TotalLines; i++){
        it = SentencesMap.find(i);
        if ( it != SentencesMap.end() ){
            message.append( it->second );
        } else{
            return;
        }
    }
    AllLinesReceived = true;
    message = parent->DecodeStr( message, true);
    parent->MessageReceived(message, this);
}

///////////////////////////////////////////////////////////////////////////////
/// Class RxAcceptDlg
///////////////////////////////////////////////////////////////////////////////
RxAcceptDlg::RxAcceptDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText = new wxStaticText( this, wxID_ANY, _T("A message containing route(s) / track(s) / waypoint(s) was received"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText->Wrap( -1 );
	fgSizer1->Add( m_staticText, 0, wxALL, 5 );
    m_SenderText = new wxStaticText( this, wxID_ANY, _T("Send by: "), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_SenderText, 0, wxALL, 5 );
    m_SenderComputerText = new wxStaticText( this, wxID_ANY, _T("Send from: "), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_SenderComputerText, 0, wxALL, 5 );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );

	m_RejectBtn = new wxButton( this, wxID_ANY, wxT("Reject"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_RejectBtn, 0, wxALL, 5 );

	m_SaveBtn = new wxButton( this, wxID_ANY, wxT("Save to File"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_SaveBtn, 0, wxALL, 5 );

	m_AcceptBtn = new wxButton( this, wxID_ANY, wxT("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_AcceptBtn, 0, wxALL, 5 );

	fgSizer1->Add( bSizer1, 1, wxALIGN_BOTTOM|wxALIGN_RIGHT|wxEXPAND|wxRIGHT, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();
	fgSizer1->Fit( this );
	this->Centre( wxBOTH );

	// Connect Events
	m_RejectBtn->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( RxAcceptDlg::m_RejectBtnOnLeftDown ), NULL, this );
	m_SaveBtn->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( RxAcceptDlg::m_SaveBtnOnLeftDown ), NULL, this );
	m_AcceptBtn->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( RxAcceptDlg::m_AcceptBtnOnLeftDown ), NULL, this );
}

RxAcceptDlg::~RxAcceptDlg()
{
	// Disconnect Events
	m_RejectBtn->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( RxAcceptDlg::m_RejectBtnOnLeftDown ), NULL, this );
	m_SaveBtn->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( RxAcceptDlg::m_SaveBtnOnLeftDown ), NULL, this );
	m_AcceptBtn->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( RxAcceptDlg::m_AcceptBtnOnLeftDown ), NULL, this );
}
