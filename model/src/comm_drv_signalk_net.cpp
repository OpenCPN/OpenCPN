/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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
#include "model/sys_events.h"
#include "wxServDisc.h"

#include "observable.h"

#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXUserAgent.h"
#include "ixwebsocket/IXSocketTLSOptions.h"

const int kTimerSocket = 9006;

class CommDriverSignalKNetEvent;  // fwd

class CommDriverSignalKNetThread : public wxThread {
public:
  CommDriverSignalKNetThread(CommDriverSignalKNet* Launcher,
                             const wxString& PortName,
                             const wxString& strBaudRate);

  ~CommDriverSignalKNetThread(void);
  void* Entry();
  bool SetOutMsg(const wxString& msg);
  void OnExit(void);

private:
  void ThreadMessage(const wxString& msg);
  bool OpenComPortPhysical(const wxString& com_name, int baud_rate);
  void CloseComPortPhysical();
  size_t WriteComPortPhysical(std::vector<unsigned char> msg);
  size_t WriteComPortPhysical(unsigned char* msg, size_t length);
  void SetGatewayOperationMode(void);

  CommDriverSignalKNet* m_pParentDriver;
  wxString m_PortName;
  wxString m_FullPortName;

  unsigned char* put_ptr;
  unsigned char* tak_ptr;

  unsigned char* rx_buffer;

  int m_baud;
  int m_n_timeout;

  //  n2k_atomic_queue<char*> out_que;
};

class CommDriverSignalKNetEvent;
wxDECLARE_EVENT(wxEVT_COMMDRIVER_SIGNALK_NET, CommDriverSignalKNetEvent);

class CommDriverSignalKNetEvent : public wxEvent {
public:
  CommDriverSignalKNetEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
      : wxEvent(id, commandType) {};
  ~CommDriverSignalKNetEvent() {};

  // accessors
  void SetPayload(std::shared_ptr<std::string> data) { m_payload = data; }
  std::shared_ptr<std::string> GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const {
    CommDriverSignalKNetEvent* newevent = new CommDriverSignalKNetEvent(*this);
    newevent->m_payload = this->m_payload;
    return newevent;
  };

private:
  std::shared_ptr<std::string> m_payload;
};

//      WebSocket implementation

class WebSocketThread : public wxThread {
public:
  WebSocketThread(CommDriverSignalKNet* parent, wxIPV4address address,
                  wxEvtHandler* consumer, const std::string& token);
  virtual void* Entry();

private:
  void HandleMessage(const std::string& message);
  wxEvtHandler* s_wsSKConsumer;
  wxIPV4address m_address;
  wxEvtHandler* m_consumer;
  CommDriverSignalKNet* m_parentStream;
  std::string m_token;
  ix::WebSocket ws;
  ObsListener resume_listener;
};

WebSocketThread::WebSocketThread(CommDriverSignalKNet* parent,
                                 wxIPV4address address, wxEvtHandler* consumer,
                                 const std::string& token)
    : m_address(address),
      m_consumer(consumer),
      m_parentStream(parent),
      m_token(token) {
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

  m_parentStream->SetThreadRunning(true);

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
    } else if (msg->type == ix::WebSocketMessageType::Close) {
      wxLogDebug("websocket: Connection disconnected");
    } else if (msg->type == ix::WebSocketMessageType::Error) {
      wxLogDebug("websocket: error: %s", msg->errorInfo.reason.c_str());
      ws.getUrl() == wsAddress.str() ? ws.setUrl(wssAddress.str())
                                     : ws.setUrl(wsAddress.str());
    }
  };

  ws.setOnMessageCallback(message_callback);
  ws.start();

  while (m_parentStream->m_Thread_run_flag > 0) {
    std::this_thread::sleep_for(100ms);
  }

  ws.stop();
  m_parentStream->SetThreadRunning(false);
  m_parentStream->m_Thread_run_flag = -1;

  return 0;
}

void WebSocketThread::HandleMessage(const std::string& message) {
  if (s_wsSKConsumer) {
    CommDriverSignalKNetEvent signalKEvent(wxEVT_COMMDRIVER_SIGNALK_NET, 0);
    auto buffer = std::make_shared<std::string>(message);

    signalKEvent.SetPayload(buffer);
    s_wsSKConsumer->AddPendingEvent(signalKEvent);
  }
}

//========================================================================
/*    CommDriverSignalKNet implementation
 * */

wxDEFINE_EVENT(wxEVT_COMMDRIVER_SIGNALK_NET, CommDriverSignalKNetEvent);

CommDriverSignalKNet::CommDriverSignalKNet(const ConnectionParams* params,
                                           DriverListener& listener)
    : CommDriverSignalK(((ConnectionParams*)params)->GetStrippedDSPort()),
      m_Thread_run_flag(-1),
      m_params(*params),
      m_listener(listener) {
  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_SIGNALK_NET, &CommDriverSignalKNet::handle_SK_sentence,
       this);

  m_addr.Hostname(params->NetworkAddress);
  m_addr.Service(params->NetworkPort);
  m_token = params->AuthToken;
  m_socketread_watchdog_timer.SetOwner(this, kTimerSocket);
  m_wsThread = NULL;
  m_threadActive = false;

  Open();
}

CommDriverSignalKNet::~CommDriverSignalKNet() { Close(); }

void CommDriverSignalKNet::Activate() {
  CommDriverRegistry::GetInstance().Activate(shared_from_this());
}

void CommDriverSignalKNet::Open(void) {
  wxString discoveredIP;
#if 0
  int discoveredPort;
#endif

  // if (m_useWebSocket)
  {
    std::string serviceIdent =
        std::string("_signalk-ws._tcp.local.");  // Works for node.js server
#if 0
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
#endif
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
  wxLogMessage(wxString::Format(_T("Opening Signal K WebSocket client: %s"),
                                m_params.GetDSPort().c_str()));

  // Start a thread to run the client without blocking

  m_wsThread = new WebSocketThread(this, GetAddr(), this, m_token);
  if (m_wsThread->Create() != wxTHREAD_NO_ERROR) {
    wxLogError(wxT("Can't create WebSocketThread!"));

    return;
  }

  ResetWatchdog();
  GetSocketThreadWatchdogTimer()->Start(1000,
                                        wxTIMER_ONE_SHOT);  // Start the dog
  SetThreadRunFlag(1);

  m_wsThread->Run();
}

void CommDriverSignalKNet::CloseWebSocket() {
  if (m_wsThread) {
    if (IsThreadRunning()) {
      wxLogMessage(_T("Stopping Secondary SignalK Thread"));

      m_Thread_run_flag = 0;
      int tsec = 10;
      while (IsThreadRunning() && tsec) {
        wxSleep(1);
        tsec--;
      }

      wxString msg;
      if (m_Thread_run_flag <= 0)
        msg.Printf(_T("Stopped in %d sec."), 10 - tsec);
      else
        msg.Printf(_T("Not Stopped after 10 sec."));
      wxLogMessage(msg);
    }

    wxMilliSleep(100);

#if 0
      m_thread_run_flag = 0;
       printf("sending delete\n");
      m_wsThread->Delete();
      wxMilliSleep(100);

      int nDeadman = 0;
      while (IsThreadRunning() && (++nDeadman < 200)) {  // spin for max 2 secs.
        wxMilliSleep(10);
      }
       printf("Closed in %d\n", nDeadman);
      wxMilliSleep(100);
#endif
  }
}

void CommDriverSignalKNet::handle_SK_sentence(
    CommDriverSignalKNetEvent& event) {
  rapidjson::Document root;

  // LOG_DEBUG("%s\n", msg.c_str());

  std::string* msg = event.GetPayload().get();
  std::string msgTerminated = *msg;
  msgTerminated.append("\r\n");

  root.Parse(*msg);
  if (root.HasParseError()) {
    wxLogMessage(wxString::Format(
        _T("SignalKDataStream ERROR: the JSON document is not well-formed:%d"),
        root.GetParseError()));
    return;
  }

  // Decode just enough of string to extract some identifiers
  // such as the sK version, "self" context, and target context
  if (root.HasMember("version")) {
    wxString msg = _T("Connected to Signal K server version: ");
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
  auto navmsg = std::make_shared<const SignalkMsg>(
      m_self, m_context, msgTerminated, comm_interface);
  m_listener.Notify(std::move(navmsg));
}

void CommDriverSignalKNet::initIXNetSystem() { ix::initNetSystem(); };

void CommDriverSignalKNet::uninitIXNetSystem() { ix::uninitNetSystem(); };

#if 0
void CommDriverSignalKNet::handleUpdate(wxJSONValue &update) {
  wxString sfixtime = "";

  if (update.HasMember("timestamp")) {
    sfixtime = update["timestamp"].AsString();
  }
  if (update.HasMember("values") && update["values"].IsArray()) {
    for (int j = 0; j < update["values"].Size(); ++j) {
      wxJSONValue &item = update["values"][j];
      updateItem(item, sfixtime);
    }
  }
}

void CommDriverSignalKNet::updateItem(wxJSONValue &item,
                                     wxString &sfixtime) {
  if (item.HasMember("path") && item.HasMember("value")) {
    const wxString &update_path = item["path"].AsString();
    wxJSONValue &value = item["value"];

    if (update_path == _T("navigation.position") && !value.IsNull()) {
      updateNavigationPosition(value, sfixtime);
    } else if (update_path == _T("navigation.speedOverGround") &&
               m_bGPSValid_SK && !value.IsNull()) {
      updateNavigationSpeedOverGround(value, sfixtime);
    } else if (update_path == _T("navigation.courseOverGroundTrue") &&
               m_bGPSValid_SK && !value.IsNull()) {
      updateNavigationCourseOverGround(value, sfixtime);
    } else if (update_path == _T("navigation.courseOverGroundMagnetic")) {
    }
    else if (update_path ==
             _T("navigation.gnss.satellites"))  // From GGA sats in use
    {
      /*if (g_priSats >= 2)*/ updateGnssSatellites(value, sfixtime);
    } else if (update_path ==
               _T("navigation.gnss.satellitesInView"))  // From GSV sats in view
    {
      /*if (g_priSats >= 3)*/ updateGnssSatellites(value, sfixtime);
    } else if (update_path == _T("navigation.headingTrue")) {
      if(!value.IsNull())
        updateHeadingTrue(value, sfixtime);
    } else if (update_path == _T("navigation.headingMagnetic")) {
      if(!value.IsNull())
        updateHeadingMagnetic(value, sfixtime);
    } else if (update_path == _T("navigation.magneticVariation")) {
      if(!value.IsNull())
        updateMagneticVariance(value, sfixtime);
    } else {
      // wxLogMessage(wxString::Format(_T("** Signal K unhandled update: %s"),
      // update_path));
#if 0
            wxString dbg;
            wxJSONWriter writer;
            writer.Write(item, dbg);
            wxString msg( _T("update: ") );
            msg.append(dbg);
            wxLogMessage(msg);
#endif
    }
  }
}

void CommDriverSignalKNet::updateNavigationPosition(
    wxJSONValue &value, const wxString &sfixtime) {
  if ((value.HasMember("latitude" && value["latitude"].IsDouble())) &&
      (value.HasMember("longitude") && value["longitude"].IsDouble())) {
    // wxLogMessage(_T(" ***** Position Update"));
    m_lat = value["latitude"].AsDouble();
    m_lon = value["longitude"].AsDouble();
    m_bGPSValid_SK = true;
  } else {
    m_bGPSValid_SK = false;
  }
}


void CommDriverSignalKNet::updateNavigationSpeedOverGround(
    wxJSONValue &value, const wxString &sfixtime){
  double sog_ms = value.AsDouble();
  double sog_knot = sog_ms * ms_to_knot_factor;
  // wxLogMessage(wxString::Format(_T(" ***** SOG: %f, %f"), sog_ms, sog_knot));
  m_sog = sog_knot;
}

void CommDriverSignalKNet::updateNavigationCourseOverGround(
    wxJSONValue &value, const wxString &sfixtime) {
  double cog_rad = value.AsDouble();
  double cog_deg = GEODESIC_RAD2DEG(cog_rad);
  // wxLogMessage(wxString::Format(_T(" ***** COG: %f, %f"), cog_rad, cog_deg));
  m_cog = cog_deg;
}

void CommDriverSignalKNet::updateGnssSatellites(wxJSONValue &value,
                                               const wxString &sfixtime) {
#if 0
  if (value.IsInt()) {
    if (value.AsInt() > 0) {
      m_frame->setSatelitesInView(value.AsInt());
      g_priSats = 2;
    }
  } else if ((value.HasMember("count") && value["count"].IsInt())) {
    m_frame->setSatelitesInView(value["count"].AsInt());
    g_priSats = 3;
  }
#endif
}

void CommDriverSignalKNet::updateHeadingTrue(wxJSONValue &value,
                                            const wxString &sfixtime) {
  m_hdt = GEODESIC_RAD2DEG(value.AsDouble());
}

void CommDriverSignalKNet::updateHeadingMagnetic(
    wxJSONValue &value, const wxString &sfixtime) {
  m_hdm = GEODESIC_RAD2DEG(value.AsDouble());
}

void CommDriverSignalKNet::updateMagneticVariance(
    wxJSONValue &value, const wxString &sfixtime) {
  m_var = GEODESIC_RAD2DEG(value.AsDouble());
}

#endif
////////////

//   std::vector<unsigned char>* payload = p.get();
//
//   // Extract the NMEA0183 sentence
//   std::string full_sentence = std::string(payload->begin(), payload->end());
