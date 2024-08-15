/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_socketcan.h -- socketcan driver.
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

#if !defined(__linux__) || defined(__ANDROID__)
#error "This file can only be compiled on Linux"
#endif

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <future>

#include "config.h"

#include <net/if.h>
#include <serial/serial.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>
#include <wx/thread.h>

#include "model/comm_can_util.h"
#include "model/comm_drv_n2k_socketcan.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/config_vars.h"

#define DEFAULT_N2K_SOURCE_ADDRESS 72

wxDEFINE_EVENT(EVT_N2K_59904, ObservedEvt);

static const int kNotFound = -1;

/// Read timeout in worker main loop (seconds)
static const int kSocketTimeoutSeconds = 2;

typedef struct can_frame CanFrame;

class CommDriverN2KSocketCanImpl;  // fwd

/**
 * Manages reading the N2K data stream provided by some N2K gateways from the
 * declared serial port.
 *
 * Commonly used raw format is actually inherited from an old paketizing format:
 * <10><02><application data><CRC (1)><10><03>
 *
 * Actisense application data, from NGT-1 to PC
 * <data code=93><length (1)><priority (1)><PGN (3)><destination(1)><source
 * (1)><time (4)><len (1)><data (len)>
 *
 * As applied to a real application data element, after extraction from packet
 * format: 93 13 02 01 F8 01 FF 01 76 C2 52 00 08 08 70 EB 14 E8 8E 52 D2 BB 10
 *
 * length (1):      0x13
 * priority (1);    0x02
 * PGN (3):         0x01 0xF8 0x01
 * destination(1):  0xFF
 * source (1):      0x01
 * time (4):        0x76 0xC2 0x52 0x00
 * len (1):         0x08
 * data (len):      08 70 EB 14 E8 8E 52 D2
 * packet CRC:      0xBB
 */
class Worker {
public:
  Worker(CommDriverN2KSocketCAN* parent, const wxString& PortName);

  bool StartThread();
  void StopThread();
  int GetSocket() { return m_socket; }

private:
  void Entry();

  void ThreadMessage(const std::string& msg, wxLogLevel l = wxLOG_Message);

  int InitSocket(const std::string port_name);
  void SocketMessage(const std::string& msg, const std::string& device);
  void HandleInput(CanFrame frame);
  void ProcessRxMessages(std::shared_ptr<const Nmea2000Msg> n2k_msg);

  std::vector<unsigned char> PushCompleteMsg(const CanHeader header,
                                             int position,
                                             const CanFrame frame);
  std::vector<unsigned char> PushFastMsgFragment(const CanHeader& header,
                                                 int position);

  CommDriverN2KSocketCanImpl* const m_parent_driver;
  const wxString m_port_name;
  std::atomic<int> m_run_flag;
  FastMessageMap fast_messages;
  int m_socket;
};

/** Local driver implementation, not visible outside this file.*/
class CommDriverN2KSocketCanImpl : public CommDriverN2KSocketCAN {
  friend class Worker;

public:
  CommDriverN2KSocketCanImpl(const ConnectionParams* p, DriverListener& l)
      : CommDriverN2KSocketCAN(p, l),
        m_worker(this, p->socketCAN_port),
        m_source_address(-1),
        m_last_TX_sequence(0) {
    SetN2K_Name();
    Open();
  }

  ~CommDriverN2KSocketCanImpl() { Close(); }

  bool Open();
  void Close();
  void SetN2K_Name();

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr);

  int DoAddressClaim();
  bool SendAddressClaim(int proposed_source_address);
  bool SendProductInfo();

  Worker& GetWorker() { return m_worker; }
  void UpdateAttrCanAddress();

private:
  N2kName node_name;
  Worker m_worker;
  int m_source_address;
  int m_last_TX_sequence;
  std::future<int> m_AddressClaimFuture;
  wxMutex m_TX_mutex;
  int m_unique_number;

  ObservableListener listener_N2K_59904;
  bool HandleN2K_59904(std::shared_ptr<const Nmea2000Msg> n2k_msg);
};

// Static CommDriverN2KSocketCAN factory implementation.

std::shared_ptr<CommDriverN2KSocketCAN> CommDriverN2KSocketCAN::Create(
    const ConnectionParams* params, DriverListener& listener) {
  return std::shared_ptr<CommDriverN2KSocketCAN>(
      new CommDriverN2KSocketCanImpl(params, listener));
}

// CommDriverN2KSocketCanImpl implementation

void CommDriverN2KSocketCanImpl::SetN2K_Name() {
  // We choose some "benign" values for OCPN socketCan interface
  node_name.value.Name = 0;

  m_unique_number = 1;
  // Build a simple 16 bit hash of g_hostname, to use as unique "serial number"
  int hash = 0;
  std::string str(g_hostname.mb_str());
  int len = str.size();
  const char* ch = str.data();
  for (int i = 0; i < len; i++)
    hash = hash + ((hash) << 5) + *(ch + i) + ((*(ch + i)) << 7);
  m_unique_number = ((hash) ^ (hash >> 16)) & 0xffff;

  node_name.SetManufacturerCode(2046);
  node_name.SetUniqueNumber(m_unique_number);
  node_name.SetDeviceFunction(130);  // Display
  node_name.SetDeviceClass(120);     // Display
  node_name.SetIndustryGroup(4);     // Marine
  node_name.SetSystemInstance(0);
}

void CommDriverN2KSocketCanImpl::UpdateAttrCanAddress() {
  this->attributes["canAddress"] = std::to_string(m_source_address);
}

bool CommDriverN2KSocketCanImpl::Open() {
  // Start the RX worker thread
  bool bws = m_worker.StartThread();
  return bws;
}

void CommDriverN2KSocketCanImpl::Close() {
  wxLogMessage("Closing N2K socketCAN: %s", m_params.socketCAN_port.c_str());
  m_worker.StopThread();

  // We cannot use shared_from_this() since we might be in the destructor.
  auto& registry = CommDriverRegistry::GetInstance();
  auto me = FindDriver(registry.GetDrivers(), iface, bus);
  registry.Deactivate(me);
}

bool CommDriverN2KSocketCanImpl::SendAddressClaim(int proposed_source_address) {
  wxMutexLocker lock(m_TX_mutex);

  int socket = GetWorker().GetSocket();

  if (socket < 0) return false;

  CanFrame frame;
  memset(&frame, 0, sizeof(frame));

  uint64_t _pgn = 60928;
  unsigned long canId = BuildCanID(6, proposed_source_address, 255, _pgn);
  frame.can_id = canId | CAN_EFF_FLAG;

  // Load the data
  uint32_t b32_0 = node_name.value.UnicNumberAndManCode;
  memcpy(&frame.data, &b32_0, 4);

  unsigned char b81 = node_name.value.DeviceInstance;
  memcpy(&frame.data[4], &b81, 1);

  b81 = node_name.value.DeviceFunction;
  memcpy(&frame.data[5], &b81, 1);

  b81 = (node_name.value.DeviceClass);
  memcpy(&frame.data[6], &b81, 1);

  b81 = node_name.value.IndustryGroupAndSystemInstance;
  memcpy(&frame.data[7], &b81, 1);

  frame.can_dlc = 8;  // data length

  int sentbytes = write(socket, &frame, sizeof(frame));

  return (sentbytes == 16);
}

void AddStr(std::vector<uint8_t>& vec, std::string str, size_t max_len) {
  size_t i;
  for (i = 0; i < str.size(); i++) {
    vec.push_back(str[i]);
    ;
  }
  for (; i < max_len; i++) {
    vec.push_back(0);
  }
}

bool CommDriverN2KSocketCanImpl::SendProductInfo() {
  // Create the payload
  std::vector<uint8_t> payload;

  payload.push_back(2100 & 0xFF);  // N2KVersion
  payload.push_back(2100 >> 8);
  payload.push_back(0xEC);  // Product Code, 1772
  payload.push_back(0x06);

  std::string ModelID("OpenCPN");  // Model ID
  AddStr(payload, ModelID, 32);

  std::string ModelSWCode(PACKAGE_VERSION);  // SwCode
  AddStr(payload, ModelSWCode, 32);

  std::string ModelVersion(PACKAGE_VERSION);  // Model Version
  AddStr(payload, ModelVersion, 32);

  std::string ModelSerialCode(
      std::to_string(m_unique_number));  // Model Serial Code
  AddStr(payload, ModelSerialCode, 32);

  payload.push_back(0);  // CertificationLevel
  payload.push_back(0);  // LoadEquivalency

  auto dest_addr = std::make_shared<const NavAddr2000>(iface, 255);
  uint64_t _PGN;
  _PGN = 126996;

  auto msg = std::make_shared<const Nmea2000Msg>(_PGN, payload, dest_addr);
  SendMessage(msg, dest_addr);

  return true;
}

bool CommDriverN2KSocketCanImpl::SendMessage(
    std::shared_ptr<const NavMsg> msg, std::shared_ptr<const NavAddr> addr) {
  wxMutexLocker lock(m_TX_mutex);

  // Verify claimed address is useable
  if (m_source_address < 0) return false;

  if (m_source_address > 253)  // Could not claim...
    return false;

  int socket = GetWorker().GetSocket();

  if (socket < 0) return false;

  CanFrame frame;
  memset(&frame, 0, sizeof(frame));

  auto msg_n2k = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
  std::vector<uint8_t> load = msg_n2k->payload;

  uint64_t _pgn = msg_n2k->PGN.pgn;
  auto destination_address = std::static_pointer_cast<const NavAddr2000>(addr);

  unsigned long canId = BuildCanID(msg_n2k->priority, m_source_address,
                                   destination_address->address, _pgn);

  frame.can_id = canId | CAN_EFF_FLAG;

  if (load.size() <= 8) {
    frame.can_dlc = load.size();
    if (load.size() > 0) memcpy(&frame.data, load.data(), load.size());

    int sentbytes = write(socket, &frame, sizeof(frame));
  } else {  // Fast Packet
    int sequence = (m_last_TX_sequence + 0x20) & 0xE0;
    m_last_TX_sequence = sequence;
    unsigned char* data_ptr = load.data();
    int n_remaining = load.size();

    // First packet
    frame.can_dlc = 8;
    frame.data[0] = sequence;
    frame.data[1] = load.size();
    int data_len_0 = wxMin(load.size(), 6);
    memcpy(&frame.data[2], load.data(), data_len_0);

    int sentbytes0 = write(socket, &frame, sizeof(frame));

    data_ptr += data_len_0;
    n_remaining -= data_len_0;
    sequence++;

    // The rest of the bytes
    while (n_remaining > 0) {
      wxMilliSleep(10);
      frame.data[0] = sequence;
      int data_len_n = wxMin(n_remaining, 7);
      memcpy(&frame.data[1], data_ptr, data_len_n);

      int sentbytesn = write(socket, &frame, sizeof(frame));

      data_ptr += data_len_n;
      n_remaining -= data_len_n;
      sequence++;
    }
  }

  return true;
}

// CommDriverN2KSocketCAN implementation

CommDriverN2KSocketCAN::CommDriverN2KSocketCAN(const ConnectionParams* params,
                                               DriverListener& listener)
    : CommDriverN2K(((ConnectionParams*)params)->GetStrippedDSPort()),
      m_params(*params),
      m_listener(listener),
      m_ok(false),
      m_portstring(params->GetDSPort()),
      m_baudrate(wxString::Format("%i", params->Baudrate)) {
  this->attributes["canPort"] = params->socketCAN_port.ToStdString();
  this->attributes["canAddress"] = std::to_string(DEFAULT_N2K_SOURCE_ADDRESS);
  this->attributes["userComment"] = params->UserComment.ToStdString();
  this->attributes["ioDirection"] = std::string("IN/OUT");
}

CommDriverN2KSocketCAN::~CommDriverN2KSocketCAN() {}

void CommDriverN2KSocketCAN::Activate() {
  CommDriverRegistry::GetInstance().Activate(shared_from_this());
}

// Worker implementation

Worker::Worker(CommDriverN2KSocketCAN* parent, const wxString& port_name)
    : m_parent_driver(dynamic_cast<CommDriverN2KSocketCanImpl*>(parent)),
      m_port_name(port_name.Clone()),
      m_run_flag(-1),
      m_socket(-1) {
  assert(m_parent_driver != 0);
}

std::vector<unsigned char> Worker::PushCompleteMsg(const CanHeader header,
                                                   int position,
                                                   const CanFrame frame) {
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

std::vector<unsigned char> Worker::PushFastMsgFragment(const CanHeader& header,
                                                       int position) {
  std::vector<unsigned char> data;
  data.push_back(0x93);
  data.push_back(fast_messages[position].expected_length + 11);
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
  data.push_back(fast_messages[position].expected_length);
  for (size_t n = 0; n < fast_messages[position].expected_length; n++)
    data.push_back(fast_messages[position].data[n]);
  data.push_back(0x55);  // CRC dummy
  fast_messages.Remove(position);
  return data;
}

void Worker::ThreadMessage(const std::string& msg, wxLogLevel level) {
  wxLogGeneric(level, wxString(msg.c_str()));
  auto s = std::string("CommDriverN2KSocketCAN: ") + msg;
  CommDriverRegistry::GetInstance().evt_driver_msg.Notify(level, s);
}

void Worker::SocketMessage(const std::string& msg, const std::string& device) {
  std::stringstream ss;
  ss << msg << device << ": " << strerror(errno);
  ThreadMessage(ss.str());
}

/**
 * Initiate can socket
 * @param port_name Name of device, for example "can0" (native) , "slcan0"
 *                  (serial) or "vcan0" (virtual)
 * @return positive socket number or -1 on errors.
 */
int Worker::InitSocket(const std::string port_name) {
  int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (sock < 0) {
    SocketMessage("SocketCAN socket create failed: ", port_name);
    return -1;
  }

  // Get the interface index
  struct ifreq if_request;
  strcpy(if_request.ifr_name, port_name.c_str());
  if (ioctl(sock, SIOCGIFINDEX, &if_request) < 0) {
    SocketMessage("SocketCAN ioctl (SIOCGIFINDEX) failed: ", port_name);
    return -1;
  }

  // Check if interface is UP
  struct sockaddr_can can_address;
  can_address.can_family = AF_CAN;
  can_address.can_ifindex = if_request.ifr_ifindex;
  if (ioctl(sock, SIOCGIFFLAGS, &if_request) < 0) {
    SocketMessage("SocketCAN socket IOCTL (SIOCGIFFLAGS) failed: ", port_name);
    return -1;
  }
  if (if_request.ifr_flags & IFF_UP) {
    ThreadMessage("socketCan interface is UP");
  } else {
    ThreadMessage("socketCan interface is NOT UP");
    return -1;
  }

  // Set timeout and bind
  struct timeval tv;
  tv.tv_sec = kSocketTimeoutSeconds;
  tv.tv_usec = 0;
  int r =
      setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
  if (r < 0) {
    SocketMessage("SocketCAN setsockopt SO_RCVTIMEO failed on device: ",
                  port_name);
    return -1;
  }
  r = bind(sock, (struct sockaddr*)&can_address, sizeof(can_address));
  if (r < 0) {
    SocketMessage("SocketCAN socket bind() failed: ", port_name);
    return -1;
  }
  return sock;
}

/**
 * Handle a frame. A complete message or last part of a multipart fast
 * message is sent to m_listener, basically making it available to upper
 * layers. Otherwise, the fast message fragment is stored waiting for
 * next fragment.
 */
void Worker::HandleInput(CanFrame frame) {
  int position = -1;
  bool ready = true;

  CanHeader header(frame);
  if (header.IsFastMessage()) {
    position = fast_messages.FindMatchingEntry(header, frame.data[0]);
    if (position == kNotFound) {
      // Not an existing fast message: create new entry and insert first frame
      position = fast_messages.AddNewEntry();
      ready = fast_messages.InsertEntry(header, frame.data, position);
    } else {
      // An existing fast message entry is present, append the frame
      ready = fast_messages.AppendEntry(header, frame.data, position);
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
    // auto name = N2kName(static_cast<uint64_t>(header.pgn));
    auto src_addr = m_parent_driver->GetAddress(m_parent_driver->node_name);
    auto msg = std::make_shared<const Nmea2000Msg>(header.pgn, vec, src_addr);
    auto msg_all = std::make_shared<const Nmea2000Msg>(1, vec, src_addr);

    ProcessRxMessages(msg);
    m_parent_driver->m_listener.Notify(std::move(msg));
    m_parent_driver->m_listener.Notify(std::move(msg_all));
  }
}

/** Worker thread handles some RX messages. */
void Worker::ProcessRxMessages(std::shared_ptr<const Nmea2000Msg> n2k_msg) {
  if (n2k_msg->PGN.pgn == 59904) {
    unsigned long RequestedPGN = 0;
    RequestedPGN = n2k_msg->payload.at(15) << 16;
    RequestedPGN += n2k_msg->payload.at(14) << 8;
    RequestedPGN += n2k_msg->payload.at(13);

    switch (RequestedPGN) {
      case 60928:
        m_parent_driver->SendAddressClaim(m_parent_driver->m_source_address);
        break;
      case 126996:
        m_parent_driver->SendProductInfo();
        break;
      default:
        break;
    }
  }

  else if (n2k_msg->PGN.pgn == 60928) {
    // Watch for conflicting source address
    if (n2k_msg->payload.at(7) == m_parent_driver->m_source_address) {
      // My name
      uint64_t my_name = m_parent_driver->node_name.GetName();

      // His name
      uint64_t his_name = 0;
      unsigned char* p = (unsigned char*)&his_name;
      for (unsigned int i = 0; i < 8; i++) *p++ = n2k_msg->payload.at(13 + i);

      // Compare literally the NAME values
      if (his_name < my_name) {
        //  I lose, so select a new address
        m_parent_driver->m_source_address++;
        if (m_parent_driver->m_source_address > 253)
          // Could not claim an address
          m_parent_driver->m_source_address = 254;
        m_parent_driver->UpdateAttrCanAddress();
      }

      // Claim the existing or modified address
      m_parent_driver->SendAddressClaim(m_parent_driver->m_source_address);
    }
  }
}

/** Worker thread main function. */
void Worker::Entry() {
  int recvbytes;
  int socket;
  CanFrame frame;

  socket = InitSocket(m_port_name.ToStdString());
  if (socket < 0) {
    std::string msg("SocketCAN socket create failed: ");
    ThreadMessage(msg + m_port_name.ToStdString());
    m_run_flag = -1;
    return;
  }
  m_socket = socket;

  //  Claim our default address
  if (m_parent_driver->SendAddressClaim(DEFAULT_N2K_SOURCE_ADDRESS)) {
    m_parent_driver->m_source_address = DEFAULT_N2K_SOURCE_ADDRESS;
    m_parent_driver->UpdateAttrCanAddress();
  }

  // The main loop
  while (m_run_flag > 0) {
    recvbytes = read(socket, &frame, sizeof(frame));
    if (recvbytes == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) continue;  // timeout

      wxLogWarning("can socket %s: fatal error %s", m_port_name.c_str(),
                   strerror(errno));
      break;
    }
    if (recvbytes != 16) {
      wxLogWarning("can socket %s: bad frame size: %d (ignored)",
                   m_port_name.c_str(), recvbytes);
      sleep(1);
      continue;
    }
    HandleInput(frame);
  }
  m_run_flag = -1;
  return;
}

bool Worker::StartThread() {
  m_run_flag = 1;
  std::thread t(&Worker::Entry, this);
  t.detach();
  return true;
}

void Worker::StopThread() {
  if (m_run_flag < 0) {
    wxLogMessage("Attempt to stop already dead thread (ignored).");
    return;
  }
  wxLogMessage("Stopping Worker Thread");

  m_run_flag = 0;
  int tsec = 10;
  while ((m_run_flag >= 0) && (tsec--)) wxSleep(1);

  if (m_run_flag < 0)
    wxLogMessage("StopThread: Stopped in %d sec.", 10 - tsec);
  else
    wxLogWarning("StopThread: Not Stopped after 10 sec.");
}
