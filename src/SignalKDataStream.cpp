/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
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
 **************************************************************************/
// Originally by balp on 2018-07-28.

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include "zeroconf.hpp"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "wx/tokenzr.h"
#include <wx/datetime.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef __WXMSW__
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include <vector>
#include <sstream>
#include <wx/socket.h>
#include <wx/log.h>
#include <wx/memory.h>
#include <wx/chartype.h>
#include <wx/wx.h>
#include <wx/sckaddr.h>
#include "easywsclient.hpp"
#include "wxServDisc.h"
#include "ocpn_frame.h"


#if defined(__WXMSW__) && !defined(__MINGW32__)
#include <Ws2tcpip.h>  // for ip_mreq
#endif

#include "dychart.h"

#include "datastream.h"
#include "SignalKDataStream.h"
#include "NetworkDataStream.h"
#include "OCPN_SignalKEvent.h"
#include "OCPN_DataStreamEvent.h"

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

extern bool g_benableUDPNullHeader;

static wxEvtHandler *s_wsConsumer;
extern MyFrame *gFrame;

#include <wx/sckstrm.h>
#include "wx/jsonreader.h"
#include "wx/jsonwriter.h"

// Handle nessages from WebSocket thread, forward to stipulated consumer, and
// manage watchdog
class OCPN_WebSocketMessageHandler : public wxEvtHandler {
public:
  OCPN_WebSocketMessageHandler(SignalKDataStream *parent,
                               wxEvtHandler *upstream_consumer);
  ~OCPN_WebSocketMessageHandler();

  void OnWebSocketMessage(OCPN_SignalKEvent &event);

  SignalKDataStream *m_parent;
  wxEvtHandler *m_upstream_consumer;
};

OCPN_WebSocketMessageHandler::OCPN_WebSocketMessageHandler(
    SignalKDataStream *parent, wxEvtHandler *upstream_consumer) {
  m_upstream_consumer = upstream_consumer;
  m_parent = parent;
  Bind(EVT_OCPN_SIGNALKSTREAM,
       &OCPN_WebSocketMessageHandler::OnWebSocketMessage, this);
}

void OCPN_WebSocketMessageHandler::OnWebSocketMessage(
    OCPN_SignalKEvent &event) {
  if (m_upstream_consumer) {
    OCPN_SignalKEvent signalKEvent(0, EVT_OCPN_SIGNALKSTREAM,
                                   event.GetString());
    m_upstream_consumer->AddPendingEvent(signalKEvent);
  }

  m_parent->ResetWatchdog();  // feed the dog
}

OCPN_WebSocketMessageHandler::~OCPN_WebSocketMessageHandler() {
  Unbind(EVT_OCPN_SIGNALKSTREAM,
         &OCPN_WebSocketMessageHandler::OnWebSocketMessage, this);
}

BEGIN_EVENT_TABLE(SignalKDataStream, wxEvtHandler)
EVT_TIMER(TIMER_SOCKET + 2, SignalKDataStream::OnTimerSocket)
EVT_SOCKET(SIGNALK_SOCKET_ID, SignalKDataStream::OnSocketEvent)
EVT_TIMER(TIMER_SOCKET + 3, SignalKDataStream::OnSocketReadWatchdogTimer)
END_EVENT_TABLE()

SignalKDataStream::SignalKDataStream(wxEvtHandler *input_consumer,
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
  m_useWebSocket = true;
  m_wsThread = NULL;
  m_threadActive = false;
  m_eventHandler = new OCPN_WebSocketMessageHandler(this, GetConsumer());

  Open();
}

SignalKDataStream::~SignalKDataStream() {
  if (m_useWebSocket) {
    delete m_eventHandler;
    m_eventHandler = NULL;
    s_wsConsumer = NULL;
  } else {
    if (GetSock()->IsOk()) {
      char unsub[] =
          "{\"context\":\"*\",\"unsubscribe\":[{\"path\":\"*\"}]}\r\n";
      GetSock()->Write(unsub, strlen(unsub));
    }
  }

  Close();
}

void SignalKDataStream::Open(void) {
  wxString discoveredIP;
  int discoveredPort;

  if (m_useWebSocket) {
    std::string serviceIdent =
        std::string("_signalk-ws._tcp.local.");  // Works for node.js server
    if (m_params->AutoSKDiscover) {
      if (DiscoverSKServer(serviceIdent, discoveredIP, discoveredPort,
                           1)) {  // 1 second scan
        wxLogDebug(wxString::Format(
            _T("SK server autodiscovery finds WebSocket service: %s:%d"),
            discoveredIP.c_str(), discoveredPort));
        m_addr.Hostname(discoveredIP);
        m_addr.Service(discoveredPort);

        // Update the connection params, by pointer to item in global params
        // array
        ConnectionParams *params = (ConnectionParams *)m_params;  // non-const
        params->NetworkAddress = discoveredIP;
        params->NetworkPort = discoveredPort;
      } else
        wxLogDebug(_T("SK server autodiscovery finds no WebSocket server."));
    }

    OpenWebSocket();
  } else {
    std::string serviceIdent =
        std::string("_signalk-ws._tcp.local");  // Works for node.js server
    if (m_params->AutoSKDiscover) {
      if (DiscoverSKServer(serviceIdent, discoveredIP, discoveredPort,
                           1)) {  // 1 second scan
        wxLogMessage(wxString::Format(
            _T("SK server autodiscovery finds REST service: %s:%d"),
            discoveredIP.c_str(), discoveredPort));
        m_addr.Hostname(discoveredIP);
        m_addr.Service(discoveredPort);
      } else
        wxLogMessage(_T("SK server autodiscovery finds no REST server."));
    }

    OpenTCPSocket();
  }
}

bool SignalKDataStream::DiscoverSKServer(std::string serviceIdent, wxString &ip,
                                         int &port, int tSec) {
#if 0
    std::vector<Zeroconf::mdns_responce> result;
    bool st = Zeroconf::Resolve(serviceIdent.c_str(), tSec, &result);

    for(size_t i = 0 ; i < result.size() ; i++){
      sockaddr_storage sas = result[i].peer;                 // Address of the responded machine
      sockaddr_in *sai = (sockaddr_in*)&sas;
      ip = wxString(inet_ntoa(sai->sin_addr));

      std::vector<uint8_t> data = result[i].data;

      std::vector<Zeroconf::Detail::mdns_record> records = result[i].records;
      for(size_t j = 0 ; j < records.size() ; j++){

        uint16_t type = records[j].type;
        if(type == 33){                                 // SRV
            size_t pos = records[j].pos;
            //size_t len = records[j].len;
            //std::string name = records[j].name;

            // TODO This is pretty ugly, but I cannot find a definition of SRV record
            unsigned char portHi = data[pos + 16];
            unsigned char portLo = data[pos + 17];
            port = (portHi * 256) + portLo;
            return true;
        }
      }
    }
    return false;
#else
  wxServDisc *servscan =
      new wxServDisc(gFrame, wxString(serviceIdent.c_str()), QTYPE_PTR);

  for (int i = 0; i < 10; i++) {
    if (servscan->getResultCount()) {
      auto result = servscan->getResults().at(0);
      delete servscan;
      // wxSleep(5);

      // return false;
      wxServDisc *namescan = new wxServDisc(0, result.name, QTYPE_SRV);
      for (int j = 0; j < 10; j++) {
        if (namescan->getResultCount()) {
          auto namescanResult = namescan->getResults().at(0);
          port = namescanResult.port;
          delete namescan;

          wxServDisc *addrscan =
              new wxServDisc(0, namescanResult.name, QTYPE_A);
          for (int k = 0; k < 10; k++) {
            if (addrscan->getResultCount()) {
              auto addrscanResult = addrscan->getResults().at(0);
              ip = addrscanResult.ip;
              delete addrscan;
              return true;
              break;
            } else {
              wxYield();
              wxMilliSleep(1000);
            }
          }
          delete addrscan;
          return false;
        } else {
          wxYield();
          wxMilliSleep(1000);
        }
      }
      delete namescan;
      return false;
    } else {
      wxYield();
      wxMilliSleep(1000);
    }
  }

  delete servscan;
  return false;

  // wxString a = servscan.getResults().at(0).ip;
  // wxServDisc namescan(0, servscan->getResults().at(0).name, QTYPE_SRV);
  // wxServDisc addrscan(0, namescan.getResults().at(0).name, QTYPE_A);
  // wxString addr = addrscan.getResults().at(0).ip;
#endif
}

void SignalKDataStream::OpenTCPSocket() {
  wxLogMessage(wxString::Format(_T("Opening Signal K TCPSocket client: %s"),
                                m_params->GetDSPort().c_str()));

  SetSock(new wxSocketClient());
  GetSock()->SetEventHandler(*this, SIGNALK_SOCKET_ID);
  GetSock()->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG |
                       wxSOCKET_LOST_FLAG);
  GetSock()->Notify(TRUE);
  GetSock()->SetTimeout(1);  // Short timeout

  SetConnectTime(wxDateTime::Now());

  wxSocketClient *tcp_socket = static_cast<wxSocketClient *>(GetSock());
  tcp_socket->Connect(GetAddr(), FALSE);
  SetBrxConnectEvent(false);
}

int sdogval;
void SignalKDataStream::OnSocketReadWatchdogTimer(wxTimerEvent &event) {
  m_dog_value--;
  sdogval++;

  if (m_dog_value <= 0) {  // No receive in n seconds, assume connection lost
    if (m_useWebSocket) {
      wxLogMessage(wxString::Format(
          _T("    WebSocket SignalKDataStream watchdog timeout: %s"),
          GetPort().c_str()));

      // printf("DOGTIME  %d\n", sdogval);
      CloseWebSocket();
      OpenWebSocket();
      SetWatchdog(N_DOG_TIMEOUT_RECONNECT);
    } else {
      wxLogMessage(
          wxString::Format(_T("    TCP SignalKDataStream watchdog timeout: %s"),
                           GetPort().c_str()));

      if (GetProtocol() == TCP) {
        wxSocketClient *tcp_socket = dynamic_cast<wxSocketClient *>(GetSock());
        if (tcp_socket) {
          tcp_socket->Close();
        }
        GetSocketTimer()->Start(5000,
                                wxTIMER_ONE_SHOT);  // schedule a reconnect
        GetSocketThreadWatchdogTimer()->Stop();
      }
    }
  }

  GetSocketThreadWatchdogTimer()->Start(1000,
                                        wxTIMER_ONE_SHOT);  // re-Start the dog
}

void SignalKDataStream::OnTimerSocket(wxTimerEvent &event) {
  //  Attempt a connection
  wxSocketClient *tcp_socket = dynamic_cast<wxSocketClient *>(GetSock());
  if (tcp_socket) {
    if (tcp_socket->IsDisconnected()) {
      SetBrxConnectEvent(false);
      tcp_socket->Connect(GetAddr(), FALSE);
      GetSocketTimer()->Start(5000,
                              wxTIMER_ONE_SHOT);  // schedule another attempt
    }
  }
}

void SignalKDataStream::OnSocketEvent(wxSocketEvent &event) {
#define RD_BUF_SIZE 4096

  switch (event.GetSocketEvent()) {
    case wxSOCKET_CONNECTION: {
      wxLogMessage(
          wxString::Format(_T("SignalKDataStream connection established: %s"),
                           GetPort().c_str()));
      ResetWatchdog();  // feed the dog
      if (GetSock()->IsOk()) (void)SetOutputSocketOptions(GetSock());
      GetSocketTimer()->Stop();
      SetBrxConnectEvent(true);
      SetConnectTime(wxDateTime::Now());

      char sub2[] =
          "{\"context\":\"vessels.self\",\"subscribe\":[{\"path\":\"navigation."
          "*\"}]}\r\n";
      GetSock()->Write(sub2, strlen(sub2));

      break;
    }

    case wxSOCKET_LOST: {
      if (GetBrxConnectEvent())
        wxLogMessage(wxString::Format(
            _T("SignalKDataStream connection lost: %s"), GetPort().c_str()));
      wxDateTime now = wxDateTime::Now();
      wxTimeSpan since_connect = now - GetConnectTime();

      int retry_time = 5000;  // default

      //  If the socket has never connected, and it is a short interval since
      //  the connect request then stretch the time a bit.  This happens on
      //  Windows if there is no dafault IP on any interface

      if (!GetBrxConnectEvent() && (since_connect.GetSeconds() < 5))
        retry_time = 10000;  // 10 secs

      GetSocketThreadWatchdogTimer()->Stop();
      GetSocketTimer()->Start(
          retry_time, wxTIMER_ONE_SHOT);  // Schedule a re-connect attempt

      break;
    }

    case wxSOCKET_INPUT: {
#define RD_BUF_SIZE 4096  // Allows handling of high volume data streams.
      wxJSONReader jsonReader;
      wxJSONValue root;

      std::vector<char> data(RD_BUF_SIZE + 1);
      event.GetSocket()->Read(&data.front(), RD_BUF_SIZE);
      if (!event.GetSocket()->Error()) {
        size_t count = event.GetSocket()->LastCount();
        m_sock_buffer.append(&data.front(), count);
      }

      bool done = false;

      while (!done) {
        int sk_tail = 2;
        size_t sk_end = m_sock_buffer.find_first_of(
            "\r\n");  // detect the end of an SK string by finding the EOL

        if (sk_end ==
            wxString::npos)  // No termination characters: continue reading
          break;

        size_t bufl = m_sock_buffer.size();
        if (sk_end <= m_sock_buffer.size() - sk_tail) {
          std::string sk_line = m_sock_buffer.substr(0, sk_end + sk_tail);

          //  If, due to some logic error, the {nmea_end} parameter is larger
          //  than the length of the socket buffer, then std::string::substr()
          //  will throw an exception. We don't want that, so test for it. If
          //  found, the simple solution is to clear the socket buffer, and
          //  carry on This has been seen on high volume TCP feeds, Windows
          //  only. Hard to catch.....
          if (sk_end > m_sock_buffer.size())
            m_sock_buffer.clear();
          else
            m_sock_buffer = m_sock_buffer.substr(sk_end + sk_tail);

          size_t sk_start = 0;
          if (sk_start != wxString::npos) {
            sk_line = sk_line.substr(sk_start);
            if (sk_line.size()) {
              int errors = jsonReader.Parse(sk_line, &root);
              if (errors > 0) {
                wxLogMessage(
                    wxString::Format(_T("SignalKDataStream ERROR: the JSON ")
                                     _T("document is not well-formed:%d: %s"),
                                     errors, GetPort().c_str()));

              } else {
                if (GetConsumer()) {
#if 0
                                    wxString dbg;
                                    wxJSONWriter writer;
                                    writer.Write(root, dbg);

                                    wxString msg( _T("SignalK TCP Socket Event sent to consumer:\n") );
                                    msg.append(dbg);
                                    wxLogMessage(msg);
#endif
                  OCPN_SignalKEvent signalKEvent(0, EVT_OCPN_SIGNALKSTREAM,
                                                 sk_line);
                  GetConsumer()->AddPendingEvent(signalKEvent);
                }
              }
            }
          }
        } else
          done = true;
      }

      // Prevent non-nmea junk from consuming to much memory by limiting
      // carry-over buffer size.
      if (m_sock_buffer.size() > RD_BUF_SIZE)
        m_sock_buffer =
            m_sock_buffer.substr(m_sock_buffer.size() - RD_BUF_SIZE);

      ResetWatchdog();  // feed the dog

      break;
    }
    default:
      break;
  }
}

bool SignalKDataStream::SetOutputSocketOptions(wxSocketBase *sock) {
  int ret;
  int nagleDisable = 1;
  ret = sock->SetOption(IPPROTO_TCP, TCP_NODELAY, &nagleDisable,
                        sizeof(nagleDisable));
  unsigned long outbuf_size = 1024;
  return (sock->SetOption(SOL_SOCKET, SO_SNDBUF, &outbuf_size,
                          sizeof(outbuf_size)) &&
          ret);
}

void SignalKDataStream::Close() {
  if (m_useWebSocket) {
    wxLogMessage(_T("Closing Signal K WebSocket DataStream "));
    CloseWebSocket();
  } else {
    wxLogMessage(wxString::Format(_T("Closing Signal K DataStream %s"),
                                  GetPort().c_str()));
    //    Kill off the TCP Socket if alive
    if (m_sock) {
      m_sock->Notify(FALSE);
      m_sock->Destroy();
    }
  }

  m_socket_timer.Stop();
  m_socketread_watchdog_timer.Stop();

  DataStream::Close();
}

//      WebSocket implementation

class WebSocketThread : public wxThread {
public:
  WebSocketThread(SignalKDataStream *parent, wxIPV4address address,
                  wxEvtHandler *consumer);
  virtual void *Entry();

private:
  static void HandleMessage(const std::string &message);

  wxIPV4address m_address;
  wxEvtHandler *m_consumer;
  SignalKDataStream *m_parentStream;
};

WebSocketThread::WebSocketThread(SignalKDataStream *parent,
                                 wxIPV4address address,
                                 wxEvtHandler *consumer) {
  m_address = address;
  m_consumer = consumer;
  m_parentStream = parent;
}

void *WebSocketThread::Entry() {
  using easywsclient::WebSocket;

  m_parentStream->SetThreadRunning(true);

  s_wsConsumer = m_consumer;

  wxString host = m_address.IPAddress();
  int port = m_address.Service();

  // Craft the address string
  std::stringstream wsAddress;
  wsAddress << "ws://" << host.mb_str() << ":" << port
            << "/signalk/v1/stream?subscribe=all&sendCachedValues=false";

  WebSocket::pointer ws = WebSocket::from_url(wsAddress.str());
  if (ws == NULL) {
    // printf("No Connect\n");
    m_parentStream->SetThreadRunning(false);
    return 0;
  }
  while (true) {
    if (TestDestroy()) {
      // printf("receiving delete\n");
      ws->close();
    }

    if (ws->getReadyState() == WebSocket::CLOSED) {
      // printf("closed\n");
      break;
    }
    ws->poll(10);
    if (ws->getReadyState() == WebSocket::OPEN) {
      ws->dispatch(HandleMessage);
    }
  }

  // printf("ws delete\n");
  delete ws;

  m_parentStream->SetThreadRunning(false);

  return 0;
}

void WebSocketThread::HandleMessage(const std::string &message) {
  if (s_wsConsumer) {
    OCPN_SignalKEvent signalKEvent(0, EVT_OCPN_SIGNALKSTREAM, message);
    s_wsConsumer->AddPendingEvent(signalKEvent);
  }
}

void SignalKDataStream::OpenWebSocket() {
  // printf("OpenWebSocket\n");
  wxLogMessage(wxString::Format(_T("Opening Signal K WebSocket client: %s"),
                                m_params->GetDSPort().c_str()));

  // Start a thread to run the client without blocking

  m_wsThread = new WebSocketThread(this, GetAddr(), m_eventHandler);
  if (m_wsThread->Create() != wxTHREAD_NO_ERROR) {
    wxLogError(wxT("Can't create WebSocketThread!"));

    return;
  }

  ResetWatchdog();
  GetSocketThreadWatchdogTimer()->Start(1000,
                                        wxTIMER_ONE_SHOT);  // Start the dog

  m_wsThread->Run();
}

void SignalKDataStream::CloseWebSocket() {
  if (m_wsThread) {
    if (IsThreadRunning()) {
      // printf("sending delete\n");
      m_wsThread->Delete();
      wxMilliSleep(100);

      int nDeadman = 0;
      while (IsThreadRunning() && (++nDeadman < 200)) {  // spin for max 2 secs.
        wxMilliSleep(10);
      }
      // printf("Closed in %d\n", nDeadman);
      wxMilliSleep(100);
    }
  }
}
