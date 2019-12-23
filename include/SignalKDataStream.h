//
// Created by balp on 2018-07-28.
//

#ifndef OPENCPN_SIGNALKDATASTREAM_H
#define OPENCPN_SIGNALKDATASTREAM_H


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

#define SIGNALK_SOCKET_ID             5011

class SignalKDataStream : public DataStream {
public:
    SignalKDataStream(wxEvtHandler *input_consumer,
                      const ConnectionParams *params)
            : DataStream(input_consumer, params),
              m_params(params),
              m_sock(0),
              m_brx_connect_event(false)

    {
        m_addr.Hostname(params->NetworkAddress);
        m_addr.Service(params->NetworkPort);
        m_socket_timer.SetOwner(this, TIMER_SOCKET + 2);
        m_socketread_watchdog_timer.SetOwner(this, TIMER_SOCKET + 3);

        Open();

    }

    void Close();

    static bool DiscoverSKServer( wxString &ip, int &port, int tSec);

    virtual ~SignalKDataStream(); 

private:
    const ConnectionParams *m_params;
    wxSocketBase        *m_sock;
    void SetSock(wxSocketBase* sock) { m_sock = sock; }
    wxSocketBase* GetSock() const { return m_sock; }

    wxIPV4address       m_addr;
    wxIPV4address GetAddr() const { return m_addr; }

    bool                m_brx_connect_event;
    void SetBrxConnectEvent(bool event) { m_brx_connect_event = event;}
    bool GetBrxConnectEvent() { return m_brx_connect_event; }

    int                 m_dog_value;
    wxTimer             m_socket_timer;
    wxTimer* GetSocketTimer() { return &m_socket_timer; }

    wxTimer             m_socketread_watchdog_timer;
    wxTimer* GetSocketThreadWatchdogTimer() { return &m_socketread_watchdog_timer; }

    NetworkProtocol GetProtocol() { return m_params->NetProtocol; }
    std::string         m_sock_buffer;

    void Open();


    void OnTimerSocket(wxTimerEvent& event);
    void OnSocketEvent(wxSocketEvent& event);
    void OnSocketReadWatchdogTimer(wxTimerEvent& event);

    bool SetOutputSocketOptions(wxSocketBase* sock);

DECLARE_EVENT_TABLE()
};


#endif //OPENCPN_SIGNALKDATASTREAM_H
