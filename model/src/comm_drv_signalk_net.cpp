/***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement comm_drv_signalk_net.h -- IP netork SignalK driver
 */

#include <vector>
#include <mutex>  // std::mutex
#include <queue>  // std::queue
#include <chrono>
#include <thread>

#include "rapidjson/document.h"

#include "model/comm_drv_signalk_net.h"
#include "model/comm_navmsg_bus.h"
#include "model/comm_drv_registry.h"
#include "model/geodesic.h"
#include "model/logger.h"
#include "model/sys_events.h"
#include "wxServDisc.h"

#include "observable.h"

#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXUserAgent.h"
#include "ixwebsocket/IXSocketTLSOptions.h"

using namespace std::literals::chrono_literals;

static const int kTimerSocket = 9006;

class CommDriverSignalKNetEvent;  // fwd

// i. e. wxDEFINE_EVENT(), avoiding the evil macro.
static const wxEventTypeTag<CommDriverSignalKNetEvent> SignalkEvtType(
    wxNewEventType());

class CommDriverSignalKNetEvent : public wxEvent {
public:
  CommDriverSignalKNetEvent(const std::string& payload)
      : wxEvent(0, SignalkEvtType), m_payload(payload) {};

  ~CommDriverSignalKNetEvent() = default;

  std::string GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const { return new CommDriverSignalKNetEvent(m_payload); };

private:
  const std::string m_payload;
};

//      WebSocket implementation

class WebSocketThread : public wxThread, public ThreadCtrl {
public:
  WebSocketThread(const std::string& iface, wxIPV4address address,
                  wxEvtHandler* consumer, const std::string& token);
  virtual void* Entry();

  DriverStats GetStats() const;

private:
  void HandleMessage(const std::string& message);
  wxEvtHandler* s_wsSKConsumer;
  wxIPV4address m_address;
  wxEvtHandler* m_consumer;
  const std::string m_iface;
  std::string m_token;
  ix::WebSocket ws;
  ObsListener resume_listener;
  DriverStats m_driver_stats;
  mutable std::mutex m_stats_mutex;
};

WebSocketThread::WebSocketThread(const std::string& iface,
                                 wxIPV4address address, wxEvtHandler* consumer,
                                 const std::string& token)
    : m_address(address), m_consumer(consumer), m_iface(iface), m_token(token) {
  resume_listener.Init(SystemEvents::GetInstance().evt_resume,
                       [&](ObservedEvt& ev) {
                         ws.stop();
                         ws.start();
                         wxLogDebug("WebSocketThread: restarted");
                       });
}

void* WebSocketThread::Entry() {
  using namespace std::chrono_literals;
  bool not_done = true;

  s_wsSKConsumer = m_consumer;

  wxString host = m_address.IPAddress();
  int port = m_address.Service();

  // Craft the address string
  std::stringstream wsAddress;
  wsAddress << "ws://" << host << ":" << port
            << "/signalk/v1/stream?subscribe=all&sendCachedValues=false";
  std::stringstream wssAddress;
  wssAddress << "wss://" << host << ":" << port
             << "/signalk/v1/stream?subscribe=all&sendCachedValues=false";

  if (!m_token.empty()) {
    wsAddress << "&token=" << m_token;
    wssAddress << "&token=" << m_token;
  }

  ws.setUrl(wssAddress.str());
  ix::SocketTLSOptions opt;
  opt.disable_hostname_validation = true;
  opt.caFile = "NONE";
  ws.setTLSOptions(opt);
  ws.setPingInterval(30);

  auto message_callback = [&](const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
      HandleMessage(msg->str);
    } else if (msg->type == ix::WebSocketMessageType::Open) {
      wxLogDebug("websocket: Connection established");
      std::lock_guard lock(m_stats_mutex);
      m_driver_stats.available = true;
    } else if (msg->type == ix::WebSocketMessageType::Close) {
      wxLogDebug("websocket: Connection disconnected");
      std::lock_guard lock(m_stats_mutex);
      m_driver_stats.available = false;
    } else if (msg->type == ix::WebSocketMessageType::Error) {
      std::lock_guard lock(m_stats_mutex);
      m_driver_stats.error_count++;
      wxLogDebug("websocket: error: %s", msg->errorInfo.reason.c_str());
      ws.getUrl() == wsAddress.str() ? ws.setUrl(wssAddress.str())
                                     : ws.setUrl(wsAddress.str());
    }
  };

  ws.setOnMessageCallback(message_callback);

  {
    std::lock_guard lock(m_stats_mutex);
    m_driver_stats.driver_bus = NavAddr::Bus::Signalk;
    m_driver_stats.driver_iface = m_iface;
    m_driver_stats.available = false;
  }

  ws.start();

  while (KeepGoing()) {
    std::this_thread::sleep_for(100ms);
  }

  s_wsSKConsumer = nullptr;
  ws.stop();
  SignalExit();
  {
    std::lock_guard lock(m_stats_mutex);
    m_driver_stats.available = false;
  }

  return 0;
}

DriverStats WebSocketThread::GetStats() const {
  std::lock_guard lock(m_stats_mutex);
  return m_driver_stats;
}

void WebSocketThread::HandleMessage(const std::string& message) {
  if (s_wsSKConsumer) {
    s_wsSKConsumer->QueueEvent(new CommDriverSignalKNetEvent(message));
    m_driver_stats.rx_count++;
  }
}

//========================================================================
/*    CommDriverSignalKNet implementation
 * */

CommDriverSignalKNet::CommDriverSignalKNet(const ConnectionParams* params,
                                           DriverListener& listener)
    : CommDriverSignalK(params->GetStrippedDSPort()),
      m_params(*params),
      m_listener(listener),
      m_stats_timer(*this, 2s) {
  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(SignalkEvtType, &CommDriverSignalKNet::handle_SK_sentence, this);

  m_addr.Hostname(params->NetworkAddress);
  m_addr.Service(params->NetworkPort);
  m_token = params->AuthToken;
  m_socketread_watchdog_timer.SetOwner(this, kTimerSocket);
  m_wsThread = NULL;

  // Dummy Driver Stats, may be polled before worker thread is active
  m_driver_stats.driver_bus = NavAddr::Bus::Signalk;
  m_driver_stats.driver_iface = m_params.GetStrippedDSPort();
  m_driver_stats.available = false;

  Open();
}

CommDriverSignalKNet::~CommDriverSignalKNet() { Close(); }

DriverStats CommDriverSignalKNet::GetDriverStats() const {
  if (m_wsThread)
    return m_wsThread->GetStats();
  else
    return m_driver_stats;
}

void CommDriverSignalKNet::Open() {
  wxString discoveredIP;
#if 0
  int discoveredPort;
#endif

  // if (m_useWebSocket)
  {
    std::string serviceIdent =
        std::string("_signalk-ws._tcp.local.");  // Works for node.js server
    OpenWebSocket();
  }
}
void CommDriverSignalKNet::Close() { CloseWebSocket(); }

bool CommDriverSignalKNet::DiscoverSKServer(std::string serviceIdent,
                                            wxString& ip, int& port, int tSec) {
  wxServDisc* servscan =
      new wxServDisc(0, wxString(serviceIdent.c_str()), QTYPE_PTR);

  for (int i = 0; i < 10; i++) {
    if (servscan->getResultCount()) {
      auto result = servscan->getResults().at(0);
      delete servscan;

      wxServDisc* namescan = new wxServDisc(0, result.name, QTYPE_SRV);
      for (int j = 0; j < 10; j++) {
        if (namescan->getResultCount()) {
          auto namescanResult = namescan->getResults().at(0);
          port = namescanResult.port;
          delete namescan;

          wxServDisc* addrscan =
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
              wxMilliSleep(1000 * tSec / 10);
            }
          }
          delete addrscan;
          return false;
        } else {
          wxYield();
          wxMilliSleep(1000 * tSec / 10);
        }
      }
      delete namescan;
      return false;
    } else {
      wxYield();
      wxMilliSleep(1000 * tSec / 10);
    }
  }

  delete servscan;
  return false;
}

void CommDriverSignalKNet::OpenWebSocket() {
  // printf("OpenWebSocket\n");
  wxLogMessage(wxString::Format("Opening Signal K WebSocket client: %s",
                                m_params.GetDSPort().c_str()));

  // Start a thread to run the client without blocking

  m_wsThread = new WebSocketThread(m_params.GetStrippedDSPort(), GetAddr(),
                                   this, m_token);
  if (m_wsThread->Create() != wxTHREAD_NO_ERROR) {
    wxLogError("Can't create WebSocketThread!");

    return;
  }

  ResetWatchdog();
  GetSocketThreadWatchdogTimer()->Start(1000,
                                        wxTIMER_ONE_SHOT);  // Start the dog

  m_wsThread->Run();
}

void CommDriverSignalKNet::CloseWebSocket() {
  using namespace std::chrono;
  if (m_wsThread) {
    if (m_wsThread->ThreadCtrl::IsRunning()) {
      wxLogMessage("Stopping Secondary SignalK Thread");
      m_stats_timer.Stop();
      milliseconds stop_delay;
      m_wsThread->RequestStop();
      bool stop_ok = m_wsThread->WaitUntilStopped(10s, stop_delay);
      if (stop_ok)
        MESSAGE_LOG << "Stopped in" << stop_delay.count() << " msec.";
      else
        WARNING_LOG << "Not stopped after 10 sec.";
    }

    wxMilliSleep(100);
  }
}

void CommDriverSignalKNet::handle_SK_sentence(
    CommDriverSignalKNetEvent& event) {
  rapidjson::Document root;

  std::string msg = event.GetPayload();
  root.Parse(msg);
  if (root.HasParseError()) {
    wxLogMessage(
        "SignalKDataStream ERROR: the JSON document is not well-formed: %d",
        root.GetParseError());
    return;
  }

  if (!root.IsObject()) {
    wxLogMessage(wxString::Format(
        "SignalKDataStream ERROR: Message is not a JSON Object: %s",
        msg.c_str()));
    return;
  }

  // Decode just enough of string to extract some identifiers
  // such as the sK version, "self" context, and target context
  if (root.HasMember("version")) {
    wxString msg = "Connected to Signal K server version: ";
    msg << (root["version"].GetString());
    wxLogMessage(msg);
  }

  if (root.HasMember("self")) {
    if (strncmp(root["self"].GetString(), "vessels.", 8) == 0)
      m_self = (root["self"].GetString());  // for java server, and OpenPlotter
                                            // node.js server 1.20
    else
      m_self = std::string("vessels.")
                   .append(root["self"].GetString());  // for Node.js server
  }

  if (root.HasMember("context") && root["context"].IsString()) {
    m_context = root["context"].GetString();
  }

  // Notify all listeners
  auto pos = iface.find(":");
  std::string comm_interface = "";
  if (pos != std::string::npos) comm_interface = iface.substr(pos + 1);
  auto navmsg = std::make_shared<const SignalkMsg>(m_self, m_context, msg,
                                                   comm_interface);
  m_listener.Notify(std::move(navmsg));
}

void CommDriverSignalKNet::initIXNetSystem() { ix::initNetSystem(); };

void CommDriverSignalKNet::uninitIXNetSystem() { ix::uninitNetSystem(); };