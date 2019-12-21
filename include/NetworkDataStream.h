/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Object
 * Author:   David Register
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
 *
 *
 *
 *
 */


#ifndef __NETWORKDATASTREAM_H__
#define __NETWORKDATASTREAM_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled header


#include <wx/datetime.h>


#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
//#include <gtk/gtk.h>
#define GSocket GlibGSocket
#include "wx/socket.h"
#undef GSocket
#else
#include "wx/socket.h"
#endif

#ifndef __WXMSW__
#include <sys/socket.h>                 // needed for (some) Mac builds
#include <netinet/in.h>
#endif

#ifdef __WXMSW__
#include <windows.h>
#include <dbt.h>
#include <initguid.h>
#endif
#include <string>
#include "ConnectionParams.h"
#include "dsPortType.h"
#include "datastream.h"

class NetworkDataStream : public DataStream {
public:
    NetworkDataStream(wxEvtHandler *input_consumer,
                      const ConnectionParams *params)
            : DataStream(input_consumer, params),
              m_net_port(wxString::Format(wxT("%i"), params->NetworkPort)),
              m_net_protocol(params->NetProtocol),
              m_sock(NULL),
              m_tsock(NULL),
              m_socket_server(NULL),
              m_is_multicast(false),
              m_txenter(0)
    {
        m_addr.Hostname(params->NetworkAddress);
        m_addr.Service(params->NetworkPort);

        m_socket_timer.SetOwner(this, TIMER_SOCKET);
        m_socketread_watchdog_timer.SetOwner(this, TIMER_SOCKET + 1);

        Open();
    }
    ~NetworkDataStream() {
        Close();
    }

    virtual bool SendSentence( const wxString &sentence ) {
        wxString payload = sentence;
        if( !sentence.EndsWith(_T("\r\n")) )
            payload += _T("\r\n");
        return SendSentenceNetwork(payload);
    }
    virtual void Close();
private:
    wxString            m_net_port;
    NetworkProtocol     m_net_protocol;
    wxIPV4address       m_addr;
    wxSocketBase        *m_sock;
    wxSocketBase        *m_tsock;
    wxSocketServer      *m_socket_server;
    bool                m_is_multicast;
    struct ip_mreq      m_mrq;
    int                 m_txenter;
    int                 m_dog_value;
    std::string         m_sock_buffer;
    wxTimer             m_socket_timer;
    wxTimer             m_socketread_watchdog_timer;
    bool                m_brx_connect_event;


    void Open();
    void OpenNetworkGPSD();
    void OpenNetworkTCP(unsigned int addr);
    void OpenNetworkUDP(unsigned int addr);
    bool SendSentenceNetwork(const wxString &payload);

    void OnTimerSocket(wxTimerEvent& event);
    void OnSocketEvent(wxSocketEvent& event);
    //  TCP Server support
    void OnServerSocketEvent(wxSocketEvent& event);             // The listener
    // void OnActiveServerEvent(wxSocketEvent& event);             // The open connection
    void OnSocketReadWatchdogTimer(wxTimerEvent& event);
    wxIPV4address GetAddr() const { return m_addr; }
    bool SetOutputSocketOptions(wxSocketBase* tsock);


    wxString  GetNetPort() const { return m_net_port; }

    NetworkProtocol GetProtocol() { return m_net_protocol; }

    void SetSock(wxSocketBase* sock) { m_sock = sock; }

    wxSocketBase* GetSock() const { return m_sock; }

    void SetTSock(wxSocketBase* sock) { m_tsock = sock; }

    wxSocketBase* GetTSock() const { return m_tsock; }

    void SetSockServer(wxSocketServer* sock) { m_socket_server = sock; }

    wxSocketServer* GetSockServer() const { return m_socket_server; }

    void SetMulticast(bool multicast) { m_is_multicast = multicast; }

    bool GetMulticast() const { return m_is_multicast; }

    void SetMrqAddr(unsigned int addr) {
        m_mrq.imr_multiaddr.s_addr = addr;
        m_mrq.imr_interface.s_addr = INADDR_ANY;
    }
    
    struct ip_mreq& GetMrq() { return m_mrq; }

    wxTimer* GetSocketTimer() { return &m_socket_timer; }

    wxTimer* GetSocketThreadWatchdogTimer() { return &m_socketread_watchdog_timer; }

    void SetBrxConnectEvent(bool event) {m_brx_connect_event = event;}

    bool GetBrxConnectEvent() { return m_brx_connect_event; }

    DECLARE_EVENT_TABLE()
};


#endif // __NETWORKDATASTREAM_H__