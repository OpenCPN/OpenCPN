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

#include <chrono>
#include <memory>
#include <mutex>  // std::mutex
#include <queue>  // std::queue
#include <thread>
#include <vector>

#include "rapidjson/document.h"
#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXSocketTLSOptions.h"
#include "observable.h"

#include "model/comm_drv_signalk_net.h"
#include "model/comm_navmsg_bus.h"
#include "model/geodesic.h"
#include "model/logger.h"
#include "model/sys_events.h"
#include "wxServDisc.h"

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
WebSocketThread::WebSocketThread(const std::string& iface,
                                 wxIPV4address address, wxEvtHandler* consumer,
                                 const std::string& token)
    : m_address(address), m_consumer(consumer), m_iface(iface), m_token(token) {
  m_resume_listener.Init(SystemEvents::GetInstance().evt_resume,
                         [&](ObservedEvt& ev) {
                           m_ws.stop();
                           m_ws.start();
                           wxLogDebug("WebSocketThread: restarted");
                         });
}

void* WebSocketThread::Entry() {
  using namespace std::chrono_literals;
  bool not_done = true;

  m_ws_sk_consumer = m_consumer;

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

  m_ws.setUrl(wssAddress.str());
  ix::SocketTLSOptions opt;
  opt.disable_hostname_validation = true;
  opt.caFile = "NONE";
  m_ws.setTLSOptions(opt);
  m_ws.setPingInterval(30);

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
      m_ws.getUrl() == wsAddress.str() ? m_ws.setUrl(wssAddress.str())
                                       : m_ws.setUrl(wsAddress.str());
    }
  };

  m_ws.setOnMessageCallback(message_callback);

  {
    std::lock_guard lock(m_stats_mutex);
    m_driver_stats.driver_bus = NavAddr::Bus::Signalk;
    m_driver_stats.driver_iface = m_iface;
    m_driver_stats.available = false;
  }

  m_ws.start();

  while (KeepGoing()) {
    std::this_thread::sleep_for(100ms);
  }

  m_ws_sk_consumer = nullptr;
  m_ws.stop();
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
  if (m_ws_sk_consumer) {
    m_ws_sk_consumer->QueueEvent(new CommDriverSignalKNetEvent(message));
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
  Bind(SignalkEvtType, &CommDriverSignalKNet::HandleSkSentence, this);

  m_addr.Hostname(params->NetworkAddress);
  m_addr.Service(params->NetworkPort);
  m_token = params->AuthToken;
  m_socketread_watchdog_timer.SetOwner(this, kTimerSocket);
  m_ws_thread = NULL;

  // Dummy Driver Stats, may be polled before worker thread is active
  m_driver_stats.driver_bus = NavAddr::Bus::Signalk;
  m_driver_stats.driver_iface = m_params.GetStrippedDSPort();
  m_driver_stats.available = false;

  Open();
}

CommDriverSignalKNet::~CommDriverSignalKNet() { Close(); }

DriverStats CommDriverSignalKNet::GetDriverStats() const {
  if (m_ws_thread)
    return m_ws_thread->GetStats();
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
    std::string service_ident =
        std::string("_signalk-ws._tcp.local.");  // Works for node.js server
    OpenWebSocket();
  }
}
void CommDriverSignalKNet::Close() { CloseWebSocket(); }

bool CommDriverSignalKNet::DiscoverSKServer(std::string serviceIdent,
                                            wxString& ip, int& port, int tSec) {
  auto servscan = std::make_unique<wxServDisc>(
      nullptr, wxString(serviceIdent.c_str()), QTYPE_PTR);
  for (int i = 0; i < 10; i++) {
    if (servscan->getResultCount()) {
      auto result = servscan->getResults().at(0);
      auto namescan =
          std::make_unique<wxServDisc>(nullptr, result.name, QTYPE_SRV);
      for (int j = 0; j < 10; j++) {
        if (namescan->getResultCount()) {
          auto namescan_result = namescan->getResults().at(0);
          port = namescan_result.port;
          auto addrscan = std::make_unique<wxServDisc>(
              nullptr, namescan_result.name, QTYPE_A);
          for (int k = 0; k < 10; k++) {
            if (addrscan->getResultCount()) {
              auto addrscan_result = addrscan->getResults().at(0);
              ip = addrscan_result.ip;
              return true;
              break;
            } else {
              wxYield();
              wxMilliSleep(1000 * tSec / 10);
            }
          }
          return false;
        } else {
          wxYield();
          wxMilliSleep(1000 * tSec / 10);
        }
      }
      return false;
    } else {
      wxYield();
      wxMilliSleep(1000 * tSec / 10);
    }
  }
  return false;
}

void CommDriverSignalKNet::OpenWebSocket() {
  // printf("OpenWebSocket\n");
  wxLogMessage(wxString::Format("Opening Signal K WebSocket client: %s",
                                m_params.GetDSPort().c_str()));

  // Start a thread to run the client without blocking

  m_ws_thread = new WebSocketThread(m_params.GetStrippedDSPort(), GetAddr(),
                                    this, m_token);
  if (m_ws_thread->Create() != wxTHREAD_NO_ERROR) {
    wxLogError("Can't create WebSocketThread!");

    return;
  }

  ResetWatchdog();
  GetSocketThreadWatchdogTimer()->Start(1000,
                                        wxTIMER_ONE_SHOT);  // Start the dog

  m_ws_thread->Run();
}

void CommDriverSignalKNet::CloseWebSocket() {
  using namespace std::chrono;
  if (m_ws_thread) {
    if (m_ws_thread->ThreadCtrl::IsRunning()) {
      wxLogMessage("Stopping Secondary SignalK Thread");
      m_stats_timer.Stop();
      milliseconds stop_delay;
      m_ws_thread->RequestStop();
      bool stop_ok = m_ws_thread->WaitUntilStopped(10s, stop_delay);
      if (stop_ok)
        MESSAGE_LOG << "Stopped in" << stop_delay.count() << " msec.";
      else
        WARNING_LOG << "Not stopped after 10 sec.";
    }

    wxMilliSleep(100);
  }
}

void CommDriverSignalKNet::HandleSkSentence(CommDriverSignalKNetEvent& event) {
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
