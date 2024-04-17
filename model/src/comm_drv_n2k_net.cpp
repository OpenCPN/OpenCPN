/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_n2k_net.h -- network nmea2K driver
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2023 by David Register, Alec Leamas                     *
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

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <ws2tcpip.h>
#include <windows.h>
#endif

#ifdef __MSVC__
#include "winsock2.h"
#include <wx/msw/winundef.h>
#include <ws2tcpip.h>
#endif

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/tokenzr.h>
#include <wx/datetime.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "model/sys_events.h"

#ifndef __WXMSW__
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include <vector>
#include <wx/socket.h>
#include <wx/log.h>
#include <wx/memory.h>
#include <wx/chartype.h>
#include <wx/wx.h>
#include <wx/sckaddr.h>


#include "model/comm_drv_n2k_net.h"
#include "model/comm_navmsg_bus.h"
#include "model/idents.h"
#include "model/comm_drv_registry.h"

#define N_DOG_TIMEOUT 8

static const int kNotFound = -1;

class MrqContainer{
public:
  struct ip_mreq m_mrq;
  void SetMrqAddr(unsigned int addr) {
    m_mrq.imr_multiaddr.s_addr = addr;
    m_mrq.imr_interface.s_addr = INADDR_ANY;
  }
};

// circular_buffer implementation

circular_buffer::circular_buffer(size_t size)
      : buf_(std::unique_ptr<unsigned char[]>(new unsigned char[size])), max_size_(size) {}

//void circular_buffer::reset()
//{}

//size_t circular_buffer::capacity() const
//{}

//size_t circular_buffer::size() const
//{}

bool circular_buffer::empty() const {
    // if head and tail are equal, we are empty
    return (!full_ && (head_ == tail_));
}

bool circular_buffer::full() const {
    // If tail is ahead the head by 1, we are full
    return full_;
}

void circular_buffer::put(unsigned char item) {
    std::lock_guard<std::mutex> lock(mutex_);
    buf_[head_] = item;
    if (full_) tail_ = (tail_ + 1) % max_size_;

    head_ = (head_ + 1) % max_size_;

    full_ = head_ == tail_;
}

unsigned char circular_buffer::get() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (empty()) return 0;

    // Read data and advance the tail (we now have a free space)
    auto val = buf_[tail_];
    full_ = false;
    tail_ = (tail_ + 1) % max_size_;

    return val;
}

/// CAN v2.0 29 bit header as used by NMEA 2000
CanHeader::CanHeader()
    : priority('\0'), source('\0'), destination('\0'), pgn(-1) {};



wxDEFINE_EVENT(wxEVT_COMMDRIVER_N2K_NET, CommDriverN2KNetEvent);

class CommDriverN2KNetEvent;
wxDECLARE_EVENT(wxEVT_COMMDRIVER_N2K_NET, CommDriverN2KNetEvent);

class CommDriverN2KNetEvent : public wxEvent {
public:
  CommDriverN2KNetEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
      : wxEvent(id, commandType){};
  ~CommDriverN2KNetEvent(){};

  // accessors
  void SetPayload(std::shared_ptr<std::vector<unsigned char>> data) {
    m_payload = data;
  }
  std::shared_ptr<std::vector<unsigned char>> GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const {
    CommDriverN2KNetEvent* newevent = new CommDriverN2KNetEvent(*this);
    newevent->m_payload = this->m_payload;
    return newevent;
  };

private:
  std::shared_ptr<std::vector<unsigned char>> m_payload;
};

static uint64_t PayloadToName(const std::vector<unsigned char> payload) {
  uint64_t name;
  memcpy(&name, reinterpret_cast<const void*>(payload.data()), sizeof(name));
  return name;
}

//========================================================================
/*    commdriverN2KNet implementation
 * */

#define TIMER_SOCKET_N2KNET 7339

BEGIN_EVENT_TABLE(CommDriverN2KNet, wxEvtHandler)
EVT_TIMER(TIMER_SOCKET_N2KNET, CommDriverN2KNet::OnTimerSocket)
EVT_SOCKET(DS_SOCKET_ID, CommDriverN2KNet::OnSocketEvent)
EVT_SOCKET(DS_SERVERSOCKET_ID, CommDriverN2KNet::OnServerSocketEvent)
EVT_TIMER(TIMER_SOCKET_N2KNET + 1, CommDriverN2KNet::OnSocketReadWatchdogTimer)
END_EVENT_TABLE()

// CommDriverN0183Net::CommDriverN0183Net() : CommDriverN0183() {}

CommDriverN2KNet::CommDriverN2KNet(const ConnectionParams* params,
                                       DriverListener& listener)
    : CommDriverN2K(((ConnectionParams*)params)->GetStrippedDSPort()),
      m_params(*params),
      m_listener(listener),
      m_net_port(wxString::Format("%i", params->NetworkPort)),
      m_net_protocol(params->NetProtocol),
      m_sock(NULL),
      m_tsock(NULL),
      m_socket_server(NULL),
      m_is_multicast(false),
      m_txenter(0),
      m_portstring(params->GetDSPort()),
      m_io_select(params->IOSelect),
      m_connection_type(params->Type),
      m_bok(false)

{
  m_addr.Hostname(params->NetworkAddress);
  m_addr.Service(params->NetworkPort);

  m_socket_timer.SetOwner(this, TIMER_SOCKET_N2KNET);
  m_socketread_watchdog_timer.SetOwner(this, TIMER_SOCKET_N2KNET + 1);
  this->attributes["netAddress"] = params->NetworkAddress.ToStdString();
  char port_char[10];
  sprintf(port_char, "%d",params->NetworkPort);
  this->attributes["netPort"] = std::string(port_char);

    // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_N2K_NET, &CommDriverN2KNet::handle_N2K_MSG, this);

  m_mrq_container = new MrqContainer;
  m_ib = 0;
  m_bInMsg = false;
  m_bGotESC = false;
  m_bGotSOT = false;
  rx_buffer = new unsigned char[RX_BUFFER_SIZE_NET + 1];
  m_circle = new  circular_buffer(RX_BUFFER_SIZE_NET);

  fast_messages = new FastMessageMap();
  m_order = 0;    // initialize the fast message order bits, for TX
  m_n2k_format = N2KFormat_YD_RAW;

          // Establish the power events response
  resume_listener.Init(SystemEvents::GetInstance().evt_resume,
                       [&](ObservedEvt&) { HandleResume(); });

  Open();
}

CommDriverN2KNet::~CommDriverN2KNet() {
  delete m_mrq_container;
  delete[] rx_buffer;
  delete m_circle;

  Close();
}

typedef struct {
  std::string Model_ID;
  char RT_flag;
} product_info;

std::unordered_map<uint8_t, product_info> prod_info_map;

bool CommDriverN2KNet::HandleMgntMsg(uint64_t pgn, std::vector<unsigned char> &payload){
  // Process a few N2K network management messages
  auto name = PayloadToName(payload);
  auto msg = std::make_shared<const Nmea2000Msg>(pgn, payload, GetAddress(name));

  bool b_handled = false;
  switch(pgn){
    case 126996: {  // Product information
      uint8_t src_addr = payload.at(7);
      if(src_addr == 75) return false;    // skip simulator mgnt messages
      product_info pr_info;
      pr_info.Model_ID = std::string((char *) &payload.data()[17], 32);
      pr_info.RT_flag = m_TX_flag;

      prod_info_map[src_addr] = pr_info;
      b_handled = true;
      break;
    }
    case 59904: {  // ISO request
      uint8_t src_addr = payload.at(7);
      b_handled = true;
      break;
    }

    default:
      break;
  }
  return b_handled;
}

void CommDriverN2KNet::handle_N2K_MSG(CommDriverN2KNetEvent& event) {
  auto p = event.GetPayload();
  std::vector<unsigned char>* payload = p.get();

  // extract PGN
  uint64_t pgn = 0;
  unsigned char* c = (unsigned char*)&pgn;
  *c++ = payload->at(3);
  *c++ = payload->at(4);
  *c++ = payload->at(5);
  // memcpy(&v, &data[3], 1);
  //printf("          %ld\n", pgn);

  auto name = PayloadToName(*payload);
  auto msg = std::make_shared<const Nmea2000Msg>(pgn, *payload, GetAddress(name));
  auto msg_all = std::make_shared<const Nmea2000Msg>(1, *payload, GetAddress(name));

  m_listener.Notify(std::move(msg));
  m_listener.Notify(std::move(msg_all));
}

void CommDriverN2KNet::Activate() {
  CommDriverRegistry::GetInstance().Activate(shared_from_this());
  // TODO: Read input data.
}

void CommDriverN2KNet::Open(void) {
#ifdef __UNIX__
#if wxCHECK_VERSION(3, 0, 0)
  in_addr_t addr =
      ((struct sockaddr_in*)GetAddr().GetAddressData())->sin_addr.s_addr;
#else
  in_addr_t addr =
      ((struct sockaddr_in*)GetAddr().GetAddress()->m_addr)->sin_addr.s_addr;
#endif
#else
  unsigned int addr = inet_addr(GetAddr().IPAddress().mb_str());
#endif
  // Create the socket
  switch (m_net_protocol) {
    case TCP: {
      OpenNetworkTCP(addr);
      break;
    }
    case UDP: {
      OpenNetworkUDP(addr);
      break;
    }
    default:
      break;
  }
  SetOk(true);
}

void CommDriverN2KNet::OpenNetworkUDP(unsigned int addr) {
  if (GetPortType() != DS_TYPE_OUTPUT) {
    //  We need a local (bindable) address to create the Datagram receive socket
    // Set up the receive socket
    wxIPV4address conn_addr;
    conn_addr.Service(GetNetPort());
    conn_addr.AnyAddress();
    SetSock(
        new wxDatagramSocket(conn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR));

    // Test if address is IPv4 multicast
    if ((ntohl(addr) & 0xf0000000) == 0xe0000000) {
      SetMulticast(true);
      m_mrq_container->SetMrqAddr(addr);
      GetSock()->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &m_mrq_container->m_mrq,
                           sizeof(m_mrq_container->m_mrq));
    }

    GetSock()->SetEventHandler(*this, DS_SOCKET_ID);

    GetSock()->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG |
                         wxSOCKET_LOST_FLAG);
    GetSock()->Notify(TRUE);
    GetSock()->SetTimeout(1);  // Short timeout
  }

  // Set up another socket for transmit
  if (GetPortType() != DS_TYPE_INPUT) {
    wxIPV4address tconn_addr;
    tconn_addr.Service(0);  // use ephemeral out port
    tconn_addr.AnyAddress();
    SetTSock(
        new wxDatagramSocket(tconn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR));
    // Here would be the place to disable multicast loopback
    // but for consistency with broadcast behaviour, we will
    // instead rely on setting priority levels to ignore
    // sentences read back that have just been transmitted
    if ((!GetMulticast()) && (GetAddr().IPAddress().EndsWith(_T("255")))) {
      int broadcastEnable = 1;
      bool bam = GetTSock()->SetOption(
          SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    }
  }

  // In case the connection is lost before acquired....
  SetConnectTime(wxDateTime::Now());
}

void CommDriverN2KNet::OpenNetworkTCP(unsigned int addr) {
  int isServer = ((addr == INADDR_ANY) ? 1 : 0);
  wxLogMessage(wxString::Format(_T("Opening TCP Server %d"), isServer));

  if (isServer) {
    SetSockServer(new wxSocketServer(GetAddr(), wxSOCKET_REUSEADDR));
  } else {
    SetSock(new wxSocketClient());
  }

  if (isServer) {
    GetSockServer()->SetEventHandler(*this, DS_SERVERSOCKET_ID);
    GetSockServer()->SetNotify(wxSOCKET_CONNECTION_FLAG);
    GetSockServer()->Notify(TRUE);
    GetSockServer()->SetTimeout(1);  // Short timeout
  } else {
    GetSock()->SetEventHandler(*this, DS_SOCKET_ID);
    int notify_flags = (wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
    if (GetPortType() != DS_TYPE_INPUT) notify_flags |= wxSOCKET_OUTPUT_FLAG;
    if (GetPortType() != DS_TYPE_OUTPUT) notify_flags |= wxSOCKET_INPUT_FLAG;
    GetSock()->SetNotify(notify_flags);
    GetSock()->Notify(TRUE);
    GetSock()->SetTimeout(1);  // Short timeout

    SetBrxConnectEvent(false);
    GetSocketTimer()->Start(100, wxTIMER_ONE_SHOT);  // schedule a connection
  }

  // In case the connection is lost before acquired....
  SetConnectTime(wxDateTime::Now());
}


void CommDriverN2KNet::OnSocketReadWatchdogTimer(wxTimerEvent& event) {
  m_dog_value--;

  if (m_dog_value <= 0) {  // No receive in n seconds
    if (GetParams().NoDataReconnect) {
      // Reconnect on NO DATA is true, so try to reconnect now.
      if (GetProtocol() == TCP) {
        wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(GetSock());
        if (tcp_socket)
          tcp_socket->Close();

        int n_reconnect_delay = wxMax(N_DOG_TIMEOUT - 2, 2);
        wxLogMessage(wxString::Format(" Reconnection scheduled in %d seconds.",
                                      n_reconnect_delay));
        GetSocketTimer()->Start(n_reconnect_delay * 1000, wxTIMER_ONE_SHOT);

        //  Stop DATA watchdog, will be restarted on successful connection.
        GetSocketThreadWatchdogTimer()->Stop();
      }
    }
  }
}

void CommDriverN2KNet::OnTimerSocket() {
  //  Attempt a connection
  wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(GetSock());
  if (tcp_socket) {
    if (tcp_socket->IsDisconnected()) {
      wxLogDebug(" Attempting reconnection...");
      SetBrxConnectEvent(false);
      //  Stop DATA watchdog, may be restarted on successful connection.
      GetSocketThreadWatchdogTimer()->Stop();
      tcp_socket->Connect(GetAddr(), FALSE);

      // schedule another connection attempt, in case this one fails
      int n_reconnect_delay =  N_DOG_TIMEOUT;
      GetSocketTimer()->Start(n_reconnect_delay * 1000,
                              wxTIMER_ONE_SHOT);
    }
  }
}


void CommDriverN2KNet::HandleResume() {

  //  Attempt a stop and restart of connection
  wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(GetSock());
  if (tcp_socket) {
    GetSocketThreadWatchdogTimer()->Stop();

    tcp_socket->Close();

    // schedule reconnect attempt
    int n_reconnect_delay =  wxMax(N_DOG_TIMEOUT-2, 2);
    wxLogMessage(wxString::Format(" Reconnection scheduled in %d seconds.", n_reconnect_delay));

    GetSocketTimer()->Start(n_reconnect_delay * 1000,
                              wxTIMER_ONE_SHOT);
  }
}

bool CommDriverN2KNet::SendMessage(std::shared_ptr<const NavMsg> msg,
                                     std::shared_ptr<const NavAddr> addr) {
  auto msg_n2k = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
  auto dest_addr_n2k = std::static_pointer_cast<const NavAddr2000>(addr);
  return SendN2KNetwork(msg_n2k, dest_addr_n2k);
}

std::vector<unsigned char> CommDriverN2KNet::PushCompleteMsg(const CanHeader header,
                                                   int position,
                                                   const can_frame frame) {
  std::vector<unsigned char> data;
  data.push_back(0x93);
  data.push_back(0x13);
  data.push_back(header.priority);
  data.push_back(header.pgn & 0xFF);
  data.push_back((header.pgn >> 8) & 0xFF);
  data.push_back((header.pgn >> 16) & 0xFF);
  data.push_back(header.destination);
  data.push_back(header.source);
  data.push_back(0xFF);  // FIXME (dave) generate the time fields
  data.push_back(0xFF);
  data.push_back(0xFF);
  data.push_back(0xFF);
  data.push_back(CAN_MAX_DLEN);  // nominally 8
  for (size_t n = 0; n < CAN_MAX_DLEN; n++) data.push_back(frame.data[n]);
  data.push_back(0x55);  // CRC dummy, not checked
  return data;
}

std::vector<unsigned char> CommDriverN2KNet::PushFastMsgFragment(const CanHeader& header,
                                                       int position) {
  std::vector<unsigned char> data;
  data.push_back(0x93);
  data.push_back(fast_messages->entries[position].expected_length + 11);
  data.push_back(header.priority);
  data.push_back(header.pgn & 0xFF);
  data.push_back((header.pgn >> 8) & 0xFF);
  data.push_back((header.pgn >> 16) & 0xFF);
  data.push_back(header.destination);
  data.push_back(header.source);
  data.push_back(0xFF);  // FIXME (dave) Could generate the time fields
  data.push_back(0xFF);
  data.push_back(0xFF);
  data.push_back(0xFF);
  data.push_back(fast_messages->entries[position].expected_length);
  for (size_t n = 0; n < fast_messages->entries[position].expected_length; n++)
    data.push_back(fast_messages->entries[position].data[n]);
  data.push_back(0x55);  // CRC dummy
  fast_messages->Remove(position);
  return data;
}

/**
 * Handle a frame. A complete message or last part of a multipart fast
 * message is sent to m_listener, basically making it available to upper
 * layers. Otherwise, the fast message fragment is stored waiting for
 * next fragment.
 */
void CommDriverN2KNet::HandleCanFrameInput(can_frame frame) {
  int position = -1;
  bool ready = true;

  CanHeader header(frame);
  if (header.IsFastMessage()) {
    position = fast_messages->FindMatchingEntry(header, frame.data[0]);
    if (position == kNotFound) {
      // Not an existing fast message:
      // If valid, create new entry and insert first frame
      // First, sanity check the arriving frame.
      // If it is not the first frame of a FastMessage, then discard it
      // n.b. This should be considered a network error, or possibly a gateway
      //  error.  Maybe as simple as a dropped starting frame....
      if ((frame.data[0] & 0x1F) == 0) {
        position = fast_messages->AddNewEntry();
        ready = fast_messages->InsertEntry(header, frame.data, position);
      }
      else
        ready = false;
    } else {
      // An existing fast message entry is present, append the frame
      ready = fast_messages->AppendEntry(header, frame.data, position);
    }
  }
  if (ready) {
    std::vector<unsigned char> vec;
    if (position >= 0) {
      // Re-assembled fast message
      vec = PushFastMsgFragment(header, position);
    } else {
      // Single frame message
      vec = PushCompleteMsg(header, position, frame);
    }

    // Intercept network management messages not used by OCPN navigation core.
    if (HandleMgntMsg( header.pgn, vec))
      return;


    // Message is ready
    CommDriverN2KNetEvent Nevent(wxEVT_COMMDRIVER_N2K_NET, 0);
    auto payload = std::make_shared<std::vector<uint8_t> >(vec);
    Nevent.SetPayload(payload);
    AddPendingEvent(Nevent);

  }
}

bool isASCII(std::vector<unsigned char> packet) {
  for (unsigned char c : packet) {
    if (!isascii(c)) return false;
  }
  return true;
}

N2K_Format CommDriverN2KNet::DetectFormat(std::vector<unsigned char> packet) {

  // A simplistic attempt at identifying which of the various available
  //    on-wire (or air) formats being emitted by a configured
  //    Actisense N2k<->ethernet device.

  if (isASCII(packet)) {
    if (std::find(packet.begin(), packet.end(), ':') != packet.end())
      return N2KFormat_Actisense_RAW_ASCII;
    else
      return N2KFormat_Actisense_N2K_ASCII;
  }
  else {
    if (packet[2] == 0x95)
      return N2KFormat_Actisense_RAW;
    else if (packet[2] == 0xd0)
      return N2KFormat_Actisense_N2K;
  }
  return N2KFormat_Undefined;
}

bool CommDriverN2KNet::ProcessActisense_N2K(std::vector<unsigned char> packet) {

  //1002 d0 1500ff0401f80900684c1b00a074eb14f89052d288 1003

  std::vector<unsigned char> data;
  bool bInMsg = false;
  bool bGotESC = false;
  bool bGotSOT = false;

  while (!m_circle->empty()) {
    uint8_t next_byte = m_circle->get();

    if (bInMsg) {
      if (bGotESC) {
        if (ESCAPE == next_byte) {
          data.push_back(next_byte);
          bGotESC = false;
        }
      }

      if (bGotESC && (ENDOFTEXT == next_byte)) {
        // Process packet
        // first 3 bytes are: 1 byte for message type, 2 bytes for rest of message length
        unsigned int msg_length = (uint32_t)data[1] + ((uint32_t)data[2]<<8);

        // As a sanity check, verify message length
        if (msg_length == data.size()-1) {
          uint8_t destination = data[3];
          uint8_t source = data[4];

          uint8_t dprp = data[7];
          uint8_t priority = (dprp >> 2) & 7;  // priority bits are 3,4,5th bit
          uint8_t rAndDP = dprp & 3;  // data page + reserved is first 2 bits

          // PGN
          uint8_t pduFormat = data[6];  // PF (PDU Format)
          uint32_t pgn = (rAndDP << 16) + (pduFormat << 8);
          if (pduFormat >=
              240)  // message is broadcast, PS contains group extension
            pgn += data[5];  // +PS (PDU Specific)

          // Create the OCPN payload
          std::vector<uint8_t> o_payload;
          o_payload.push_back(0x93);
          o_payload.push_back(0x13);
          o_payload.push_back(priority);  // priority;
          o_payload.push_back(pgn & 0xFF);
          o_payload.push_back((pgn >> 8) & 0xFF);
          o_payload.push_back((pgn >> 16) & 0xFF);
          o_payload.push_back(destination);  // destination;
          o_payload.push_back(source);       // source);
          o_payload.push_back(0xFF);  // FIXME (dave) generate the time fields
          o_payload.push_back(0xFF);
          o_payload.push_back(0xFF);
          o_payload.push_back(0xFF);
          o_payload.push_back(data.size());

          // Data starts at offset 13
          for (size_t n = 13; n < data.size() - 1; n++)
            o_payload.push_back(data[n]);

          o_payload.push_back(0x55);  // CRC dummy, not checked

          // Message is ready
          CommDriverN2KNetEvent Nevent(wxEVT_COMMDRIVER_N2K_NET, 0);
          auto n2k_payload = std::make_shared<std::vector<uint8_t>>(o_payload);
          Nevent.SetPayload(n2k_payload);
          AddPendingEvent(Nevent);
        }

        // reset for next packet
        bInMsg = false;
        bGotESC = false;
        data.clear();

      } else {
        bGotESC = (next_byte == ESCAPE);

        if (!bGotESC) {
          data.push_back(next_byte);
        }
      }
    }

    else {
      if (STARTOFTEXT == next_byte) {
        bGotSOT = false;
        if (bGotESC) {
          bGotSOT = true;
        }
      } else {
        bGotESC = (next_byte == ESCAPE);
        if (bGotSOT) {
          bGotSOT = false;
          bInMsg = true;

          data.push_back(next_byte);
        }
      }
    }
  }  // while

    return true;
}

bool CommDriverN2KNet::ProcessActisense_RAW(std::vector<unsigned char> packet) {
    //1002 95 0e15870402f8094b  fc e6 20 00 00 ff ff 6f 1003

    can_frame frame;

    std::vector<unsigned char> data;
    bool bInMsg = false;
    bool bGotESC = false;
    bool bGotSOT = false;

    while (!m_circle->empty()) {
      uint8_t next_byte = m_circle->get();

      if (bInMsg) {
        if (bGotESC) {
          if (ESCAPE == next_byte) {
            data.push_back(next_byte);
            bGotESC = false;
          }
        }

        if (bGotESC && (ENDOFTEXT == next_byte)) {
          // Process packet
          // Create a can_frame, to assemble fast packets.

          // As a sanity check, verify message length
          if (data.size() >= 8) {
            size_t dLen = data[1];

            if (dLen+3 == data.size()) {

              // can_id
              memcpy(&frame.can_id, &data.data()[4], 4);

              // data
              memcpy(&frame.data, &data.data()[8], 8);

              HandleCanFrameInput(frame);

              // reset for next packet
              bInMsg = false;
              bGotESC = false;
              data.clear();
            }
          }
        } else {
          bGotESC = (next_byte == ESCAPE);

          if (!bGotESC) {
            data.push_back(next_byte);
          }
        }
      }

      else {
        if (STARTOFTEXT == next_byte) {
          bGotSOT = false;
          if (bGotESC) {
            bGotSOT = true;
          }
        } else {
          bGotESC = (next_byte == ESCAPE);
          if (bGotSOT) {
            bGotSOT = false;
            bInMsg = true;

            data.push_back(next_byte);
          }
        }
      }
    }  // while

    return true;
}

bool CommDriverN2KNet::ProcessActisense_NGT(std::vector<unsigned char> packet) {
  return true;
}

bool CommDriverN2KNet::ProcessActisense_ASCII_RAW(std::vector<unsigned char> packet) {
  can_frame frame;

  while (!m_circle->empty()) {
    char b = m_circle->get();
    if ((b != 0x0a) && (b != 0x0d)) {
      m_sentence += b;
    }
    if (b == 0x0a) {  // end of sentence

      // Extract a can_frame from ASCII stream
      //printf("%s\n", m_sentence.c_str());

      wxString ss(m_sentence.c_str());
      m_sentence.clear();
      wxStringTokenizer tkz(ss, " ");

      // Discard first token
      wxString token = tkz.GetNextToken();  // time stamp

      token = tkz.GetNextToken();           // R/T
      // Record the R/T flag, for use in device detect logic
      m_TX_flag = token[0];

      // can_id;
      token = tkz.GetNextToken();
      long canID;
      token.ToLong(&canID, 16);
      frame.can_id = canID;

      // 8 data bytes, if present, 0 otherwise
      unsigned char bytes[8];
      memset(bytes, 0, 8);
      for (unsigned int i = 0; i < 8; i++) {
        if (tkz.HasMoreTokens()) {
          token = tkz.GetNextToken();
          long tui;
          token.ToLong(&tui, 16);
          bytes[i] = (uint8_t)tui;
        }
      }
      memcpy(&frame.data, bytes, 8);
      HandleCanFrameInput(frame);
    }
  }
  return true;
}

bool CommDriverN2KNet::ProcessActisense_ASCII_N2K(std::vector<unsigned char> packet) {
  // A001001.732 04FF6 1FA03 C8FBA80329026400
  std::string sentence;

  while (!m_circle->empty()) {
    char b = m_circle->get();
    if ((b != 0x0a) && (b != 0x0d)) {
      sentence += b;
    }
    if (b == 0x0a) {  // end of sentence

      // Extract items
      //printf("%s", sentence.c_str());

      wxString ss(sentence.c_str());
      wxStringTokenizer tkz(ss, " ");
      sentence.clear();  // for next while loop

      // skip timestamp
      wxString time_header = tkz.GetNextToken();

      wxString sprio_addr = tkz.GetNextToken();
      long prio_addr;
      sprio_addr.ToLong(&prio_addr, 16);
      uint8_t priority = (uint8_t)prio_addr & 0X0F;
      uint8_t destination = (uint8_t)(prio_addr >> 4) & 0X0FF;
      uint8_t source = (uint8_t)(prio_addr >> 12) & 0X0FF;


      // PGN
      wxString sPGN = tkz.GetNextToken();
      unsigned long PGN;
      sPGN.ToULong(&PGN, 16);
      //printf("  PGN: %ld\n", PGN);

      // data field
      wxString sdata = tkz.GetNextToken();
      std::vector<uint8_t> data;
      for (size_t i = 0; i < sdata.Length(); i += 2) {
        long dv;
        wxString stui = sdata.Mid(i, 2);
        stui.ToLong(&dv, 16);
        data.push_back((uint8_t)dv);
      }

      // Create the OCPN payload
      std::vector<uint8_t> o_payload;
      o_payload.push_back(0x93);
      o_payload.push_back(0x13);
      o_payload.push_back(priority);      //priority;
      o_payload.push_back(PGN & 0xFF);
      o_payload.push_back((PGN >> 8) & 0xFF);
      o_payload.push_back((PGN >> 16) & 0xFF);
      o_payload.push_back(destination);   //destination;
      o_payload.push_back(source);        // header.source);
      o_payload.push_back(0xFF);  // FIXME (dave) generate the time fields
      o_payload.push_back(0xFF);
      o_payload.push_back(0xFF);
      o_payload.push_back(0xFF);
      o_payload.push_back(data.size());
      for (size_t n = 0; n < data.size(); n++) o_payload.push_back(data[n]);
      o_payload.push_back(0x55);          // CRC dummy, not checked

      if (HandleMgntMsg(PGN, o_payload))
        return false;

      // Message is ready
      CommDriverN2KNetEvent Nevent(wxEVT_COMMDRIVER_N2K_NET, 0);
      auto n2k_payload = std::make_shared<std::vector<uint8_t>>(o_payload);
      Nevent.SetPayload(n2k_payload);
      AddPendingEvent(Nevent);
    }
  }
  return true;
}

void CommDriverN2KNet::OnSocketEvent(wxSocketEvent& event) {
#define RD_BUF_SIZE \
  4096
  //can_frame frame;

  switch (event.GetSocketEvent()) {
    case wxSOCKET_INPUT: {
      // TODO determine if the follwing SetFlags needs to be done at every
      // socket event or only once when socket is created, it it needs to be
      // done at all!
      // m_sock->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK);      // was
      // (wxSOCKET_NOWAIT);

      // We use wxSOCKET_BLOCK to avoid Yield() reentrancy problems
      // if a long ProgressDialog is active, as in S57 SENC creation.

      //    Disable input event notifications to preclude re-entrancy on
      //    non-blocking socket
      //           m_sock->SetNotify(wxSOCKET_LOST_FLAG);

      std::vector<unsigned char> data(RD_BUF_SIZE + 1);
      int newdata = 0;
      uint8_t next_byte = 0;

      event.GetSocket()->Read(&data.front(), RD_BUF_SIZE);
      if (!event.GetSocket()->Error()) {
        size_t count = event.GetSocket()->LastCount();
        if (count) {
          if (1 /*FIXME !g_benableUDPNullHeader*/) {
            data[count] = 0;
            newdata = count;
          } else {
            // XXX FIXME: is it reliable?
          }
        }
      }

      bool done = false;
      if (newdata > 0) {
        for (int i = 0; i < newdata; i++) {
          m_circle->put(data[i]);
          //printf("%c", data.at(i));
        }
      }

      m_n2k_format = DetectFormat(data);

      switch (m_n2k_format) {
        case N2KFormat_Actisense_RAW_ASCII:
          ProcessActisense_ASCII_RAW(data);
          break;
        case N2KFormat_YD_RAW:    // RX Byte compatible with Actisense ASCII RAW
          ProcessActisense_ASCII_RAW(data);
          break;
        case N2KFormat_Actisense_N2K_ASCII:
          ProcessActisense_ASCII_N2K(data);
          break;
        case N2KFormat_Actisense_N2K:
          ProcessActisense_N2K(data);
          break;
        case N2KFormat_Actisense_RAW:
          ProcessActisense_RAW(data);
          break;
        case N2KFormat_Actisense_NGT:
          ProcessActisense_NGT(data);
          break;
        case N2KFormat_Undefined:
        default:
          break;
      }
         //      Check for any pending output message
    }   // case

      m_dog_value = N_DOG_TIMEOUT;  // feed the dog
      break;
#if 1

    case wxSOCKET_LOST: {
      if (GetProtocol() == TCP || GetProtocol() == GPSD) {
        if (GetBrxConnectEvent())
          wxLogMessage(wxString::Format(
              _T("NetworkDataStream connection lost: %s"), GetPort().c_str()));
        if (GetSockServer()) {
          GetSock()->Destroy();
          SetSock(NULL);
          break;
        }
        wxDateTime now = wxDateTime::Now();
        wxTimeSpan since_connect(
            0, 0, 10);  // ten secs assumed, if connect time is uninitialized
        if (GetConnectTime().IsValid()) since_connect = now - GetConnectTime();

        int retry_time = 5000;  // default

        //  If the socket has never connected, and it is a short interval since
        //  the connect request then stretch the time a bit.  This happens on
        //  Windows if there is no dafault IP on any interface

        if (!GetBrxConnectEvent() && (since_connect.GetSeconds() < 5))
          retry_time = 10000;  // 10 secs

        GetSocketThreadWatchdogTimer()->Stop();
        GetSocketTimer()->Start(
            retry_time, wxTIMER_ONE_SHOT);  // Schedule a re-connect attempt
      }
      break;
    }

    case wxSOCKET_CONNECTION: {
      if (GetProtocol() == GPSD) {
        //      Sign up for watcher mode, Cooked NMEA
        //      Note that SIRF devices will be converted by gpsd into
        //      pseudo-NMEA
        char cmd[] = "?WATCH={\"class\":\"WATCH\", \"nmea\":true}";
        GetSock()->Write(cmd, strlen(cmd));
      } else if (GetProtocol() == TCP) {
        wxLogMessage(wxString::Format(
            _T("TCP NetworkDataStream connection established: %s"),
            GetPort().c_str()));
        m_dog_value = N_DOG_TIMEOUT;  // feed the dog
        if (GetPortType() != DS_TYPE_OUTPUT) {
          ///start the DATA watchdog only if NODATA Reconnect is desired
          if (GetParams().NoDataReconnect)
            GetSocketThreadWatchdogTimer()->Start(1000);
        }
        if (GetPortType() != DS_TYPE_INPUT && GetSock()->IsOk())
          (void)SetOutputSocketOptions(GetSock());
        GetSocketTimer()->Stop();
        SetBrxConnectEvent(true);
      }

      SetConnectTime(wxDateTime::Now());
      break;
    }
#endif
    default:
      break;
  }
}

void CommDriverN2KNet::OnServerSocketEvent(wxSocketEvent& event) {
  switch (event.GetSocketEvent()) {
    case wxSOCKET_CONNECTION: {
      SetSock(GetSockServer()->Accept(false));

      if (GetSock()) {
        GetSock()->SetTimeout(2);
        //        GetSock()->SetFlags(wxSOCKET_BLOCK);
        GetSock()->SetEventHandler(*this, DS_SOCKET_ID);
        int notify_flags = (wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
        if (GetPortType() != DS_TYPE_INPUT) {
          notify_flags |= wxSOCKET_OUTPUT_FLAG;
          (void)SetOutputSocketOptions(GetSock());
        }
        if (GetPortType() != DS_TYPE_OUTPUT)
          notify_flags |= wxSOCKET_INPUT_FLAG;
        GetSock()->SetNotify(notify_flags);
        GetSock()->Notify(true);
      }

      break;
    }

    default:
      break;
  }
}

std::vector<std::vector<unsigned char>> CommDriverN2KNet::GetTxVector(const std::shared_ptr<const Nmea2000Msg> &msg,
                                                                      std::shared_ptr<const NavAddr2000> dest_addr) {
  std::vector<std::vector<unsigned char>> tx_vector;

  // Branch based on detected network data format currently in use
  switch(m_n2k_format) {
    case N2KFormat_YD_RAW:
      break;
    case N2KFormat_Actisense_RAW_ASCII: {
      //  00:34:02.718 R 15FD0800 FF 00 01 CA 6F FF FF FF
      if (!IsFastMessagePGN(msg->PGN.pgn) && msg->payload.size() < 8) {
        // Single packet
      } else {
        std::vector<unsigned char> header_vec;
        std::vector<unsigned char> out_vec;

        // No Need to create a timestamp or frame R/T indicator
#if 0
        // time header
        wxDateTime now = wxDateTime::Now();
        wxString stime = now.Format("%H:%M:%S");
        stime += ".000 ";
        std::string sstime = stime.ToStdString();
        for (unsigned char s : sstime) header_vec.push_back(s);

        // Tx indicator
        header_vec.push_back('T');
        header_vec.push_back(' ');
#endif

        // Craft the canID
        // No need to specify the source address
        // The TX frame will adopt the gateway's claimed N2K address.
        unsigned long can_id =
            BuildCanID(msg->priority, 0, dest_addr->address, msg->PGN.pgn);
        wxString scan_id;
        scan_id.Printf("%08X", can_id);
        std::string sscan_id = scan_id.ToStdString();
        for (unsigned char s : sscan_id) header_vec.push_back(s);
        header_vec.push_back(' ');

        // format the required number of short packets, in a loop
        int payload_size = msg->payload.size();
        unsigned char temp[8];  // {0,0,0,0,0,0,0,0};
        int cur = 0;
        int nframes =
            (payload_size > 6 ? (payload_size - 6 - 1) / 7 + 1 + 1 : 1);
        bool result = true;
        for (int i = 0; i < nframes && result; i++) {
          temp[0] = i | m_order;          //frame counter
          if (i == 0) {
            temp[1] = msg->payload.size();  // total bytes in fast packet
            // send the first 6 bytes
            for (int j = 2; j < 8; j++) {
              temp[j] = msg->payload.data()[cur];
              cur++;
            }
          } else {
            int j = 1;
            // send the next 7 data bytes
            for (; j < 8 && cur < payload_size; j++) {
              temp[j] = msg->payload.data()[cur];
              cur++;
            }
            for (; j < 8; j++) {
              temp[j] = 0xff;
            }
          }

          out_vec.clear();

          // constant header
          for (unsigned char s : header_vec) out_vec.push_back(s);

          // data, per packet
          std::string ssdata;
          for (unsigned int k = 0; k < 8; k++) {
            char tb[4];
            snprintf(tb, 4, "%02X ", temp[k]);
            ssdata += tb;
          }
          for (unsigned char s : ssdata) out_vec.push_back(s);
          out_vec.pop_back();   // drop the last space character

          out_vec.push_back(0x0d);  // terminate the string
          out_vec.push_back(0x0a);

          //for (char s :out_vec)
            //printf( "%c", s);

          tx_vector.push_back(out_vec);
        }  // for loop
      }
    }
      break;
    case N2KFormat_Actisense_N2K_ASCII: {
      // Source: Actisense own documentation `NMEA 2000 ASCII Output format.docx`
      //
      // Ahhmmss.ddd <SS><DD><P> <PPPPP> b0b1b2b3b4b5b6b7.....bn<CR><LF>
      // A = message is N2K or J1939 message
      // 173321.107 - time 17:33:21.107
      // <SS> - source address
      // <DD> - destination address
      // <P> - priority
      // <PPPPP> - PGN number
      // b0b1b2b3b4b5b6b7.....bn - data payload in hex. NB: ISO TP payload could be up to 1786 bytes
      //
      // Example: `A173321.107 23FF7 1F513 012F3070002F30709F\n`
      //                      1     2     3                  4

      std::vector<unsigned char> ovec;

      // Create the time field
      wxDateTime now = wxDateTime::Now();
      wxString stime = now.Format("%H%M%S");
      stime += ".000 ";
      std::string sstime = stime.ToStdString();
      ovec.push_back('A');
      for (unsigned char s : sstime) ovec.push_back(s);

      // src/dest/prio field
      wxString sdp;
      sdp.Printf( "%02X%02X%1X ",
               1,       // source
               (unsigned char)dest_addr->address,
               (unsigned char)msg->priority);
      std::string ssdp = sdp.ToStdString();
      for (unsigned char s : ssdp) ovec.push_back(s);

      //  PGN field
      wxString spgn;
      spgn.Printf("%05X ", (int)msg->PGN.pgn);
      std::string sspgn = spgn.ToStdString();
      for (unsigned char s : sspgn) ovec.push_back(s);

      // Data payload
      std::string sspl;
      char tv[3];
      for (unsigned char d : msg->payload){
        snprintf(tv, 3, "%02X", d);
        sspl += tv;
      }
      for (unsigned char s : sspl) ovec.push_back(s);

      // term
      ovec.push_back(0x0d);
      ovec.push_back(0x0a);

      //form the result
      tx_vector.push_back(ovec);

      break;
    }
    case N2KFormat_Actisense_N2K:
      break;
    case N2KFormat_Actisense_RAW:
      break;
    case N2KFormat_Actisense_NGT:
      break;
    default:
      break;
  }

  m_order += 16;    // update the fast message order bits

  return tx_vector;
}


std::vector<unsigned char> MakeSimpleOutMsg( int data_format, int pgn, std::vector<unsigned char> &payload) {
    std::vector<unsigned char> out_vec;

    switch (data_format) {
      case N2KFormat_YD_RAW:
      case N2KFormat_Actisense_RAW_ASCII: {

        // Craft the canID
        unsigned  can_id = BuildCanID(6, 0xff, 0xff, pgn);
        wxString scan_id;
        scan_id.Printf("%08X", can_id);
        std::string sscan_id = scan_id.ToStdString();
        for (unsigned char s : sscan_id) out_vec.push_back(s);
        out_vec.push_back(' ');

        // Data payload
        std::string sspl;
        char tv[4];
        for (unsigned char d : payload) {
          snprintf(tv, 4, "%02X ", d);
          sspl += tv;
        }
        for (unsigned char s : sspl) out_vec.push_back(s);

        // terminate
        out_vec.push_back(0x0d);
        out_vec.push_back(0x0a);
        break;
     }
     case N2KFormat_Actisense_N2K_ASCII:{

        // Create the time field
        wxDateTime now = wxDateTime::Now();
        wxString stime = now.Format("%H%M%S");
        stime += ".000 ";
        std::string sstime = stime.ToStdString();
        out_vec.push_back('A');
        for (unsigned char s : sstime) out_vec.push_back(s);

        // src/dest/prio field
        wxString sdp;
        sdp.Printf( "%02X%02X%1X ",
                   1,       // source
                   (unsigned char)0xFF,
                   0x6);
        std::string ssdp = sdp.ToStdString();
        for (unsigned char s : ssdp) out_vec.push_back(s);

        //  PGN field
        wxString spgn;
        spgn.Printf("%05X ", pgn);
        std::string sspgn = spgn.ToStdString();
        for (unsigned char s : sspgn) out_vec.push_back(s);

        // Data payload
        std::string sspl;
        char tv[3];
        for (unsigned char d : payload){
          snprintf(tv, 3, "%02X", d);
          sspl += tv;
        }
        for (unsigned char s : sspl) out_vec.push_back(s);

        // terminator
        out_vec.push_back(0x0d);
        out_vec.push_back(0x0a);
        break;
     }
    default:
      break;
    }
    return out_vec;
}


bool CommDriverN2KNet::PrepareForTX() {

    // We need to determine several items before TX operations can commence.
    // 1.  Is the gateway configured at my ip present, and if so, which of
    //     the two supported gateways is it?  (YDEN-type, or Actisense-type.
    // 2.  If Actisense type, we need to infer the N2K source address it has
    //     claimed, so that we can use that address for our TX operations.

    //  BASIC ASSUMPTION:  There is (or has been) enough network traffic to
    //  allow occurate determination of data format currently in use

  bool b_found = false;

  // Step 1.
  // If the detected data format is N2KFormat_Actisense_N2K_ASCII,
  // then we are clearly connected to an actisense device.
  // Nothing else need be done.

  if (m_n2k_format == N2KFormat_Actisense_N2K_ASCII)
    return true;

  // Step 2

  //  Assume that the gateway is YDEN type, RAW mode. Verify if true.
  //  Logic:  Actisense gateway will not respond to TX_FORMAT_YDEN,
  //  so if we get sensible response, the gw must be YDEN type.

  prod_info_map.clear();

  // Send a broadcast request for PGN 126996, Product Information
  std::vector<unsigned char> payload;
  payload.push_back(0x14);
  payload.push_back(0xF0);
  payload.push_back(0x01);

  std::vector<std::vector<unsigned char>> out_data;
  std::vector<unsigned char> msg_vec = MakeSimpleOutMsg( N2KFormat_YD_RAW, 59904, payload);
  out_data.push_back(msg_vec);
  SendSentenceNetwork(out_data);

  // Wait some time, and study results
  wxMilliSleep(200);
  wxYield();

  // Check the results of the PGN 126996 capture
  for (const auto& [key, value] : prod_info_map){
    auto prod_info = value;
    if (prod_info.Model_ID.find("YDEN") != std::string::npos) {
      // Found a YDEN device
      // If this configured port is actually connector to YDEN,
      // then the device will have marked the received TCP packet
      // with "T" indicator.  Check it.
      if (prod_info.RT_flag == 'T')
        b_found = true;
      break;
    }
  }

  if( b_found)
    return true;

  //  No acceptable TX device found
  return false;
}




bool CommDriverN2KNet::SendN2KNetwork(std::shared_ptr<const Nmea2000Msg> &msg,
                                      std::shared_ptr<const NavAddr2000> addr) {
  PrepareForTX();

  std::vector<std::vector<unsigned char>> out_data = GetTxVector(msg, addr);
  SendSentenceNetwork(out_data);
  return true;
};


bool CommDriverN2KNet::SendSentenceNetwork(std::vector<std::vector<unsigned char>> payload) {
  if (m_txenter)
    return false;  // do not allow recursion, could happen with non-blocking
                   // sockets
  m_txenter++;

  bool ret = true;
  wxDatagramSocket* udp_socket;
  switch (GetProtocol()) {
    case TCP:
      for (std::vector<unsigned char> &v : payload ) {
        if (GetSock() && GetSock()->IsOk()) {
          printf("---%s", v.data());
          GetSock()->Write(v.data(), v.size());
          if (GetSock()->Error()) {
            if (GetSockServer()) {
              GetSock()->Destroy();
              SetSock(NULL);
            } else {
              wxSocketClient* tcp_socket =
                  dynamic_cast<wxSocketClient*>(GetSock());
              if (tcp_socket) tcp_socket->Close();
              if (!GetSocketTimer()->IsRunning())
                GetSocketTimer()->Start(
                    5000, wxTIMER_ONE_SHOT);  // schedule a reconnect
              GetSocketThreadWatchdogTimer()->Stop();
            }
            ret = false;
          }
          wxMilliSleep(200);
        } else
          ret = false;
      }
      break;
    case UDP:
#if 0
      udp_socket = dynamic_cast<wxDatagramSocket*>(GetTSock());
      if (udp_socket && udp_socket->IsOk()) {
        udp_socket->SendTo(GetAddr(), payload.mb_str(), payload.size());
        if (udp_socket->Error()) ret = false;
      } else
        ret = false;
#endif
      break;

    case GPSD:
    default:
      ret = false;
      break;
  }
  m_txenter--;
  return ret;
}

void CommDriverN2KNet::Close() {
  wxLogMessage(wxString::Format(_T("Closing NMEA NetworkDataStream %s"),
                                GetNetPort().c_str()));
  //    Kill off the TCP Socket if alive
  if (m_sock) {
    if (m_is_multicast)
      m_sock->SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, &m_mrq_container->m_mrq,
                        sizeof(m_mrq_container->m_mrq));
    m_sock->Notify(FALSE);
    m_sock->Destroy();
  }

  if (m_tsock) {
    m_tsock->Notify(FALSE);
    m_tsock->Destroy();
  }

  if (m_socket_server) {
    m_socket_server->Notify(FALSE);
    m_socket_server->Destroy();
  }

  m_socket_timer.Stop();
  m_socketread_watchdog_timer.Stop();
}

bool CommDriverN2KNet::SetOutputSocketOptions(wxSocketBase* tsock) {
  int ret;

  // Disable nagle algorithm on outgoing connection
  // Doing this here rather than after the accept() is
  // pointless  on platforms where TCP_NODELAY is
  // not inherited.  However, none of OpenCPN's currently
  // supported platforms fall into that category.

  int nagleDisable = 1;
  ret = tsock->SetOption(IPPROTO_TCP, TCP_NODELAY, &nagleDisable,
                         sizeof(nagleDisable));

  //  Drastically reduce the size of the socket output buffer
  //  so that when client goes away without properly closing, the stream will
  //  quickly fill the output buffer, and thus fail the write() call
  //  within a few seconds.
  unsigned long outbuf_size = 1024;  // Smallest allowable value on Linux
  return (tsock->SetOption(SOL_SOCKET, SO_SNDBUF, &outbuf_size,
                           sizeof(outbuf_size)) &&
          ret);
}
