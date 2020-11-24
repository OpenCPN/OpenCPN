/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  About Dialog
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *

 */


#ifndef _NMEASYNC_H_
#define _NMEASYNC_H_

#define _GPX_NMEA_ID "$OPCPN"

#include "navutil.h"
#include "pugixml.hpp"
#include "NavObjectCollection.h"
#include "OCPN_DataStreamEvent.h"
#include <string>
#include <iostream>
#include <cstring>
#include <wx/timer.h>
#include <map>

//class SendGpxMessages;
class RxMessage;

class SendGpxMessage
{
public:
    SendGpxMessage();
    ~SendGpxMessage();
    void SendRoute( RouteList *pRoutes );
    void SendTrack( TrackList *pTracks);
    void SendPoint( RoutePointList *pWpoints );
    void SendMessage(std::string Message8b);     
protected:
    unsigned char ascii2ais (unsigned char a){ return (a > 39) ? a + 57: a + 48;}
    std::string EncodeStr(std::string byte8str, bool ZipIt=false);
    std::string EncodeInt(const int x);
    void AddComputeChecksum( std::string &str ) ;
    NavObjectCollection1 pgpx;
};

class RxMessages  : public wxEvtHandler
{
public:
    RxMessages(wxFrame *parent);
    ~RxMessages();
    void MessageReceived(std::string message, RxMessage* RxM);
    void OnEvtSYNC( OCPN_DataStreamEvent& event );
    int DecodeInt(const std::string str);
    std::string DecodeStr(std::string byte6str,bool IsZipped=false);
    void SaveSentece( int ID, int LineNr, std::string str);
    bool IsOwnMessage( wxString message);
    std::map<int, RxMessage*> RxMessMap;
private:
    unsigned char ais2ascii (unsigned char a){ return (a > 96) ? a - 57: a - 48;}
    void OnTimer(wxTimerEvent & event);
    
    int counter;
    wxFrame *parentt;
    wxTimer MyTimer;
};

class RxMessage
{
public:
    RxMessage( RxMessages* p);
    ~RxMessage();
    void AddSentence(int sNr, int NrOfLines, int Size, std::string UserName, std::string UserComputerName );
    void AddSentence(int sNr, std::string s);
    bool IsComplete();
    void DecodeFinish();
    int TotalLines;
    int TotalSize;
    std::string SenderUserName;
    std::string SenderComputerName;
    time_t TicksLastUpdate;
private:
    std::map<int, std::string> SentencesMap;
    RxMessages* parent;
};

///////////////////////////////////////////////////////////////////////////////
/// Class RxAcceptDlg
///////////////////////////////////////////////////////////////////////////////
class RxAcceptDlg : public wxDialog
{
	private:

	protected:
		//wxStaticText* m_staticText;
		wxButton* m_RejectBtn;
		wxButton* m_SaveBtn;
		wxButton* m_AcceptBtn;

		// Virtual event handlers, override them in your derived class
		virtual void m_RejectBtnOnLeftDown( wxMouseEvent& event ) { event.Skip(); EndModal(wxID_CANCEL);}
		virtual void m_SaveBtnOnLeftDown( wxMouseEvent& event ) { event.Skip(); EndModal(wxID_SAVE); }
		virtual void m_AcceptBtnOnLeftDown( wxMouseEvent& event ) { event.Skip(); EndModal(wxID_OK);}


	public:
        wxStaticText* m_staticText;
        wxStaticText* m_SenderText;
        wxStaticText* m_SenderComputerText;
		RxAcceptDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
		~RxAcceptDlg();

};
#endif    // _NMEASYNC_H_


