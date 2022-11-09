/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_n2k.h -- Nmea2000 serial driver.
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

// TODO (leamas): Re-implement parts which more or less emulates C++
// memory handling, free slots, etc. Remove malloc/free and raw pointers.

#if !defined(__linux__) || defined(__ANDROID__)
#error "This file can only be compiled on Linux"
#endif

#include <algorithm>
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef __ANDROID__
#include "serial/serial.h"
#endif

#include <wx/datetime.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "atomic_queue.h"
#include "comm_drv_n2k_socketcan.h"
#include "comm_drv_registry.h"
#include "comm_navmsg_bus.h"

#define NOT_FOUND -1
#define CONST_MAX_MESSAGES 100
#define CONST_TIME_EXCEEDED 250

typedef struct can_frame CanFrame;

// CAN v2.0 29 bit header as used by NMEA 2000
typedef struct CanHeader {
  unsigned char priority;
  unsigned char source;
  unsigned char destination;
  int pgn;
} CanHeader;

// Buffer used to re-assemble sequences of multi frame Fast Packet messages

// Decodes a 29 bit CAN header from an int
void DecodeCanHeader(const int canId, CanHeader* header) {
  unsigned char buf[4];
  buf[0] = canId & 0xFF;
  buf[1] = (canId >> 8) & 0xFF;
  buf[2] = (canId >> 16) & 0xFF;
  buf[3] = (canId >> 24) & 0xFF;

  header->source = buf[0];
  header->destination = buf[2] < 240 ? buf[1] : 255;
  header->pgn =
      (buf[3] & 0x01) << 16 | (buf[2] << 8) | (buf[2] < 240 ? 0 : buf[1]);
  header->priority = (buf[3] & 0x1c) >> 2;
}

unsigned long long GetTimeInMicroseconds() {
#if (defined(__APPLE__) && defined(__MACH__)) || defined(__LINUX__)
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  return (currentTime.tv_sec * 1e6) + currentTime.tv_usec;
#endif
}

/** Track fast message fragments forming a complete message. */
class FastMessageMap {

public:
  class Entry {
  public:
    unsigned char is_free;            // indicate whether this entry is free
    unsigned long long time_arrived;  // time of last message in microseconds.
    CanHeader header;  // the header of the message. Used to "map" the incoming
                       // fast message fragments
    unsigned int sid;  // message sequence identifier, used to check if a
                       // received message is the next message in the sequence
    unsigned int expected_length;  // total data length from first frame
    unsigned int cursor;  // cursor into the current position in the below data
    unsigned char* data;  // pointer to allocated data memory. Note: must
                          // be freed when IsFree is set to true.
    Entry(): is_free(true), data(0) {}
  };

  FastMessageMap() : dropped_frames(0) { Initialize(); }

  Entry operator[](int i) const { return fastMessages[i]; }
  Entry& operator[](int i) { return fastMessages[i]; }

  int FindMatchingEntry(const CanHeader header, const unsigned char sid);
  int FindFreeEntry(void);
  int GarbageCollector(void);
  void InsertEntry(const CanHeader header, const unsigned char* data,
                   int position, bool& ready);
  int AppendEntry(const CanHeader header, const unsigned char* data,
                  int position, bool& ready);
  void Initialize(void);

  int dropped_frames;
  wxDateTime dropped_frame_time;

private:
  std::vector<Entry> fastMessages;
};

class CommDriverN2KSocketCanImpl;    // fwd

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

private:
  void Entry();

  void ThreadMessage(const std::string& msg, int level = wxLOG_Message);

  bool IsFastMessage(const CanHeader header);

  int InitSocket(const std::string port_name);
  void SocketMessage(const std::string& msg, const std::string& device);
  void HandleInput(const CanHeader& header, CanFrame can_socket_frame);

  void PushCompleteMsg(std::vector<unsigned char>& data, const CanHeader header,
                       int position, const CanFrame socket_frame);
  void PushFastMsgFragment(std::vector<unsigned char>& data,
                           const CanHeader& header, int position);

  CommDriverN2KSocketCanImpl* m_parent_driver;
  wxString m_port_name;
  std::atomic<int> m_run_flag;
  atomic_queue<char*> out_que;

  FastMessageMap fast_messages;
};

/** Local driver implementation, not visible outside this file.*/
class CommDriverN2KSocketCanImpl : public CommDriverN2KSocketCAN  {
friend class Worker;

public:
  CommDriverN2KSocketCanImpl(const ConnectionParams* p, DriverListener& l)
      : CommDriverN2KSocketCAN(p, l), m_worker(this, p->socketCAN_port) {
    Open();
  }

  ~CommDriverN2KSocketCanImpl() { Close(); }

  bool Open();
  void Close();

private:
  Worker m_worker;
};

// Static CommDriverN2KSocketCAN factory implementation.

std::shared_ptr<CommDriverN2KSocketCAN> CommDriverN2KSocketCAN::Create(
    const ConnectionParams* params, DriverListener& listener) {
  return std::shared_ptr<CommDriverN2KSocketCAN>(
      new CommDriverN2KSocketCanImpl(params, listener));
}


// CommDriverN2KSocketCanImpl implementation

bool CommDriverN2KSocketCanImpl::Open() { return m_worker.StartThread(); }

void CommDriverN2KSocketCanImpl::Close() {
  wxLogMessage("Closing N2K socketCAN: %s", m_params.socketCAN_port.c_str());
  m_worker.StopThread();

  // We cannot use shared_from_this() since we might be in the destructor.
  auto& registry = CommDriverRegistry::GetInstance();
  auto me = FindDriver(registry.GetDrivers(), iface, bus);
  registry.Deactivate(me);
}


// CommDriverN2KSocketCAN implementation

CommDriverN2KSocketCAN::CommDriverN2KSocketCAN(const ConnectionParams* params,
                                               DriverListener& listener)
    : CommDriverN2K(((ConnectionParams*)params)->GetStrippedDSPort()),
      m_params(*params),
      m_listener(listener),
      m_ok(false),
      m_portstring(params->GetDSPort()),
      m_baudrate(wxString::Format("%i", params->Baudrate)) {}

CommDriverN2KSocketCAN::~CommDriverN2KSocketCAN() {}

void CommDriverN2KSocketCAN::Activate() {
  CommDriverRegistry::GetInstance().Activate(shared_from_this());
  // TODO: Read input data.
}


// Worker implementation

Worker::Worker(CommDriverN2KSocketCAN* parent, const wxString& port_name) {
  m_parent_driver = dynamic_cast<CommDriverN2KSocketCanImpl*>(parent);
  assert(m_parent_driver != 0);

  m_run_flag = -1;
  m_port_name = port_name.Clone();
}

void Worker::PushCompleteMsg(std::vector<unsigned char>& data,
                             const CanHeader header, int position,
                             const CanFrame socket_frame) {
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
  for (size_t n = 0; n < CAN_MAX_DLEN; n++)
    data.push_back(socket_frame.data[n]);
  data.push_back(0x55);  // CRC dummy, not checked
}

void Worker::PushFastMsgFragment(std::vector<unsigned char>& data,
                                 const CanHeader& header, int position) {
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
  free(fast_messages[position].data);
  fast_messages[position].is_free = true;
  fast_messages[position].data = NULL;
}

void Worker::ThreadMessage(const std::string& msg, int level) {
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
  int sock;

  sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (sock < 0) {
    SocketMessage("SocketCAN socket create failed: ", port_name);
    return -1;
  }

  struct ifreq can_request;
  strcpy(can_request.ifr_name, port_name.c_str());

  // Get the index of the interface
  if (ioctl(sock, SIOCGIFINDEX, &can_request) < 0) {
    SocketMessage("SocketCAN ioctl (SIOCGIFINDEX) failed: ", port_name);
    return -1;
  }

  // Check if the interface is UP
  struct sockaddr_can can_address;
  can_address.can_family = AF_CAN;
  can_address.can_ifindex = can_request.ifr_ifindex;
  if (ioctl(sock, SIOCGIFFLAGS, &can_request) < 0) {
    SocketMessage("SocketCAN socket IOCTL (SIOCGIFFLAGS) failed: ", port_name);
    return -1;
  }
  if (can_request.ifr_flags & IFF_UP) {
    ThreadMessage("socketCan interface is UP");
  } else {
    ThreadMessage("socketCan interface is NOT UP");
    return -1;
  }

  int r = bind(sock, (struct sockaddr*)&can_address, sizeof(can_address));
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
void Worker::HandleInput(const CanHeader& header, CanFrame can_socket_frame) {
  int position = -1;
  bool ready = false;

  if (IsFastMessage(header) == true) {
    position = fast_messages.FindMatchingEntry(header,
                                               can_socket_frame.data[0]);
    if (position == NOT_FOUND) {
      // Not an existing fast message, find a free slot
      position = fast_messages.FindFreeEntry();
      if (position == NOT_FOUND) {
        // No free slots, exit. FIXME (dave) return;
      } else {
        // Insert the first frame of the fast message
        fast_messages.InsertEntry(header, can_socket_frame.data, position,
                                  ready);
      }
    } else {
      // An existing fast message is present, append the frame
      fast_messages.AppendEntry(header, can_socket_frame.data, position,
                                ready);
    }
  } else {
    ready = true;  // This is a single frame message, parse it
  }
  if (ready) {
    std::vector<unsigned char> vec;

    if (position >= 0) {
      // Re-assembled fast message
      PushFastMsgFragment(vec, header, position);
    } else {
      // Single frame message
      PushCompleteMsg(vec, header, position, can_socket_frame);
    }
    auto name = static_cast<uint64_t>(header.pgn);
    auto src_addr = m_parent_driver->GetAddress(name);
    auto buffer = std::make_shared<std::vector<unsigned char>>(vec);
    auto msg =
        std::make_unique<const Nmea2000Msg>(header.pgn, *buffer, src_addr);
    m_parent_driver->m_listener.Notify(std::move(msg));
  }
}

/** Worker thread main function. */
void Worker::Entry() {
  CanHeader header;
  int recvbytes;
  int can_socket;
  CanFrame can_socket_frame;

  can_socket = InitSocket(m_port_name.ToStdString());
  if (can_socket < 0) {
    std::string msg("SocketCAN socket create failed: ");
    ThreadMessage(msg + m_port_name.ToStdString());
    return;
  }

  // The main loop
  while (m_run_flag > 0) {
    recvbytes = read(can_socket, &can_socket_frame, sizeof(CanFrame));
    if (recvbytes == -1) {
      if (errno == EAGAIN) {
        wxLogMessage("can socket %s: EAGAIN (retrying)", m_port_name.c_str());
        sleep(1);
        continue;
      } else {
        wxLogWarning("can socket %s: fatal error %s", m_port_name.c_str(),
                     strerror(errno));
        return;
      }
    }
    if (recvbytes != 16) {
      wxLogWarning("can socket %s: bad frame size: %d (ignored)",
                   m_port_name.c_str(), recvbytes);
      sleep(1);
      continue;
    }
    DecodeCanHeader(can_socket_frame.can_id, &header);
    HandleInput(header, can_socket_frame);
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
 

// Checks whether a frame is a single frame message or multiframe Fast Packet
// message
bool Worker::IsFastMessage(const CanHeader header) {
  static const std::vector<unsigned> haystack = {
      // All known multiframe fast messages
      65240u,  126208u, 126464u, 126996u, 126998u, 127233u, 127237u, 127489u,
      127496u, 127506u, 128275u, 129029u, 129038u, 129039u, 129040u, 129041u,
      129284u, 129285u, 129540u, 129793u, 129794u, 129795u, 129797u, 129798u,
      129801u, 129802u, 129808u, 129809u, 129810u, 130065u, 130074u, 130323u,
      130577u, 130820u, 130822u, 130824u};

  unsigned needle = static_cast<unsigned>(header.pgn);
  auto found = std::find_if(haystack.begin(), haystack.end(),
                            [needle](unsigned i) { return i == needle; });
  return found != haystack.end();
}

//  FastMessage implementation

int FastMessageMap::FindMatchingEntry(const CanHeader header,
                                      const unsigned char sid) {
  for (int i = 0; i < fastMessages.size(); i++) {
    if (((sid & 0xE0) == (fastMessages[i].sid & 0xE0)) &&
        (fastMessages[i].is_free == false) &&
        (fastMessages[i].header.pgn == header.pgn) &&
        (fastMessages[i].header.source == header.source) &&
        (fastMessages[i].header.destination == header.destination)) {
      return i;
    }
  }
  return NOT_FOUND;
}

int FastMessageMap::FindFreeEntry(void) {
  fastMessages.push_back(Entry());
  return fastMessages.size() - 1; 
}

int FastMessageMap::GarbageCollector(void) {
  int staleEntries;
  staleEntries = 0;
  for (int i = 0; i < CONST_MAX_MESSAGES; i++) {
    if ((fastMessages[i].is_free == false) &&
        (GetTimeInMicroseconds() - fastMessages[i].time_arrived >
         CONST_TIME_EXCEEDED)) {
      staleEntries++;
      free(fastMessages[i].data);
      fastMessages[i].is_free = true;
    }
  }
  return staleEntries;
}

void FastMessageMap::InsertEntry(const CanHeader header,
                                 const unsigned char* data, int position,
                                 bool& ready) {
  // first message of fast packet
  // data[0] Sequence Identifier (sid)
  // data[1] Length of data bytes
  // data[2..7] 6 data bytes

  // Ensure that this is indeed the first frame of a fast message
  if ((data[0] & 0x1F) == 0) {
    int totalDataLength;  // will also include padding as we memcpy all of the
                          // frame, because I'm lazy
    totalDataLength = (unsigned int)data[1];
    totalDataLength += 7 - ((totalDataLength - 6) % 7);

    fastMessages[position].sid = (unsigned int)data[0];
    fastMessages[position].expected_length = (unsigned int)data[1];
    fastMessages[position].header = header;
    fastMessages[position].time_arrived = GetTimeInMicroseconds();

    fastMessages[position].is_free = false;
    fastMessages[position].data = (unsigned char*)malloc(totalDataLength);
    memcpy(&fastMessages[position].data[0], &data[2], 6);
    // First frame of a multi-frame Fast Message contains six data bytes.
    // Position the cursor ready for next message
    fastMessages[position].cursor = 6;

    // Fusion, using fast messages to sends frames less than eight bytes
    if (fastMessages[position].expected_length <= 6) {
      ready = true;
    }
  }
  // No further processing is performed if this is not a start frame.
  // A start frame may have been dropped and we received a subsequent frame
}

int FastMessageMap::AppendEntry(const CanHeader header,
                                const unsigned char* data, int position,
                                bool& ready) {
  // Check that this is the next message in the sequence
  ready = false;
  if ((fastMessages[position].sid + 1) == data[0]) {
    memcpy(&fastMessages[position].data[fastMessages[position].cursor],
           &data[1], 7);
    fastMessages[position].sid = data[0];
    // Subsequent messages contains seven data bytes (last message may be padded
    // with 0xFF)
    fastMessages[position].cursor += 7;
    // Is this the last message ?
    if (fastMessages[position].cursor >=
        fastMessages[position].expected_length) {
      // Mark as ready for further processing
      ready = true;
    }
    return true;
  } else if ((data[0] & 0x1F) == 0) {
    // We've found a matching entry, however this is a start frame, therefore
    // we've missed an end frame, and now we have a start frame with the same id
    // (top 3 bits). The id has obviously rolled over. Should really double
    // check that (data[0] & 0xE0) Clear the entry as we don't want to leak
    // memory, prior to inserting a start frame
    free(fastMessages[position].data);
    fastMessages[position].is_free = true;
    fastMessages[position].data = NULL;
    // And now insert it
    InsertEntry(header, data, position, ready);
    // FIXME (dave) Should update the dropped frame stats
    return true;
  } else {
    // This is not the next frame in the sequence and not a start frame
    // We've dropped an intermedite frame, so free the slot and do no further
    // processing
    free(fastMessages[position].data);
    fastMessages[position].is_free = true;
    fastMessages[position].data = NULL;
    // Dropped Frame Statistics
    if (dropped_frames == 0) {
      dropped_frame_time = wxDateTime::Now();
      dropped_frames += 1;
    } else {
      dropped_frames += 1;
    }
    // FIXME (dave)
    //     if ((dropped_frames > CONST_DROPPEDFRAME_THRESHOLD) &&
    //     (wxDateTime::Now() < (dropped_frame_time +
    //     wxTimeSpan::Seconds(CONST_DROPPEDFRAME_PERIOD) ) ) ) {
    //       wxLogError(_T("TwoCan Device, Dropped Frames rate exceeded"));
    //       wxLogError(wxString::Format(_T("Frame: Source: %d Destination: %d
    //       Priority: %d PGN: %d"),header.source, header.destination,
    //       header.priority, header.pgn)); dropped_frames = 0;
    //     }
    return false;
  }
}

void FastMessageMap::Initialize(void) {
  for (int i = 0; i < CONST_MAX_MESSAGES; i++) {
    fastMessages.push_back(Entry());
  }
}
