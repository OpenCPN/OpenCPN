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
#include <algorithm>
#include <mutex>  // std::mutex
#include <queue>  // std::queue
#include <thread>
#include <vector>

#include <sys/time.h>

#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "comm_drv_n2k_socketcan.h"
#include "comm_navmsg_bus.h"
#include "comm_drv_registry.h"

#define DS_RX_BUFFER_SIZE 4096

template <typename T>
class n2k_atomic_queue {
public:
  size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queque.size();
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queque.empty();
  }

  const T& front() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queque.front();
  }

  void push(const T& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queque.push(value);
  }

  void pop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queque.pop();
  }

private:
  std::queue<T> m_queque;
  mutable std::mutex m_mutex;
};

template <class T>
class circular_buffer {
public:
  explicit circular_buffer(size_t size)
      : buf_(std::unique_ptr<T[]>(new T[size])), max_size_(size) {}

  void reset();
  size_t capacity() const;
  size_t size() const;

  bool empty() const {
    // if head and tail are equal, we are empty
    return (!full_ && (head_ == tail_));
  }

  bool full() const {
    // If tail is ahead the head by 1, we are full
    return full_;
  }

  void put(T item) {
    std::lock_guard<std::mutex> lock(mutex_);
    buf_[head_] = item;
    if (full_) tail_ = (tail_ + 1) % max_size_;

    head_ = (head_ + 1) % max_size_;

    full_ = head_ == tail_;
  }

  T get() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (empty()) return T();

    // Read data and advance the tail (we now have a free space)
    auto val = buf_[tail_];
    full_ = false;
    tail_ = (tail_ + 1) % max_size_;

    return val;
  }

private:
  std::mutex mutex_;
  std::unique_ptr<T[]> buf_;
  size_t head_ = 0;
  size_t tail_ = 0;
  const size_t max_size_;
  bool full_ = 0;
};

#define NOT_FOUND -1
#define CONST_MAX_MESSAGES 100
#define CONST_TIME_EXCEEDED 250

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

class CommDriverN2KSocketCANEvent;  // fwd

// Buffer used to re-assemble sequences of multi frame Fast Packet messages
typedef struct FastMessageEntry {
  unsigned char isFree;            // indicate whether this entry is free
  unsigned long long timeArrived;  // time of last message in microseconds.
  CanHeader header;  // the header of the message. Used to "map" the incoming
                     // fast message fragments
  unsigned int sid;  // message sequence identifier, used to check if a received
                     // message is the next message in the sequence
  unsigned int expectedLength;  // total data length obtained from first frame
  unsigned int cursor;  // cursor into the current position in the below data
  unsigned char* data;  // pointer to memory allocated for the data. Note: must
                        // be freed when IsFree is set to TRUE.
} FastMessageEntry;

unsigned long long GetTimeInMicroseconds() {
#if (defined(__APPLE__) && defined(__MACH__)) || defined(__LINUX__)
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  return (currentTime.tv_sec * 1e6) + currentTime.tv_usec;
#endif
}

class CommDriverN2KSocketCANThread {
public:
  CommDriverN2KSocketCANThread(CommDriverN2KSocketCAN* Launcher,
                               const wxString& PortName);

  void* Entry();
  bool SetOutMsg(const wxString& msg);
  void OnExit(void);

private:
  void ThreadMessage(const wxString& msg);

  bool IsFastMessage(const CanHeader header);
  int MapFindMatchingEntry(const CanHeader header, const unsigned char sid);
  int MapFindFreeEntry(void);
  int MapGarbageCollector(void);
  void MapInsertEntry(const CanHeader header, const unsigned char* data,
                      const int position, bool& bReady);
  int MapAppendEntry(const CanHeader header, const unsigned char* data,
                     const int position, bool& bReady);
  void MapInitialize(void);

  CommDriverN2KSocketCAN* m_pParentDriver;
  wxString m_PortName;

  unsigned char* put_ptr;
  unsigned char* tak_ptr;

  unsigned char rx_buffer[DS_RX_BUFFER_SIZE + 1];

  int m_n_timeout;

  n2k_atomic_queue<char*> out_que;

  // The Fast Packet buffer - used to reassemble Fast packet messages
  FastMessageEntry fastMessages[CONST_MAX_MESSAGES];
  int droppedFrames;
  wxDateTime droppedFrameTime;

  // CAN connection variables
  struct sockaddr_can can_address;
  // Interface Request
  struct ifreq can_request;
  // Socket Descriptor
  int can_socket;
  int flags;
  // Socket Timeouts
  struct timeval socketTimeout;
  fd_set readFD;
};

class CommDriverN2KSocketCANEvent;
wxDECLARE_EVENT(wxEVT_COMMDRIVER_N2K_SOCKETCAN, CommDriverN2KSocketCANEvent);

class CommDriverN2KSocketCANEvent : public wxEvent {
public:
  CommDriverN2KSocketCANEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
      : wxEvent(id, commandType){};
  ~CommDriverN2KSocketCANEvent(){};

  // accessors
  void SetPayload(std::shared_ptr<std::vector<unsigned char>> data) {
    m_payload = data;
  }
  std::shared_ptr<std::vector<unsigned char>> GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const {
    CommDriverN2KSocketCANEvent* newevent =
        new CommDriverN2KSocketCANEvent(*this);
    newevent->m_payload = this->m_payload;
    return newevent;
  };

private:
  std::shared_ptr<std::vector<unsigned char>> m_payload;
};

//========================================================================
/*    CommDriverN2KSocketCAN implementation
 * */

wxDEFINE_EVENT(wxEVT_COMMDRIVER_N2K_SOCKETCAN, CommDriverN2KSocketCANEvent);

CommDriverN2KSocketCAN::CommDriverN2KSocketCAN(const ConnectionParams* params,
                                               DriverListener& listener)
    : CommDriverN2K(((ConnectionParams*)params)->GetStrippedDSPort()),
      m_Thread_run_flag(-1),
      m_bok(false),
      m_portstring(params->GetDSPort()),
      m_pSecondary_Thread(NULL),
      m_params(*params),
      m_listener(listener) {
  m_BaudRate = wxString::Format("%i", params->Baudrate), SetSecThreadInActive();

  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_N2K_SOCKETCAN,
       &CommDriverN2KSocketCAN::handle_N2K_SocketCAN_RAW, this);

  Open();
}

CommDriverN2KSocketCAN::~CommDriverN2KSocketCAN() {
  if (m_pSecondary_Thread) Close();
}

bool CommDriverN2KSocketCAN::Open() {
  //    Kick off the  RX thread
  SetSecondaryThread(
      new CommDriverN2KSocketCANThread(this, m_params.socketCAN_port));
  SetThreadRunFlag(1);
  std::thread t(&CommDriverN2KSocketCANThread::Entry, GetSecondaryThread());
  t.detach();

  return true;
}

void CommDriverN2KSocketCAN::Close() {
  wxLogMessage("Closing N2K socketCAN: %s", m_params.socketCAN_port.c_str());

  //    Kill off the Secondary RX Thread if alive
  if (m_pSecondary_Thread) {
    if (m_bsec_thread_active)  // Try to be sure thread object is still alive
    {
      wxLogMessage("Stopping Secondary Thread");

      m_Thread_run_flag = 0;
      int tsec = 10;
      while ((m_Thread_run_flag >= 0) && (tsec--)) wxSleep(1);

      if (m_Thread_run_flag < 0)
        wxLogMessage("Stopped in %d sec.", 10 - tsec);
      else
        wxLogMessage("Not Stopped after 10 sec.");
    }
    delete m_pSecondary_Thread;
    m_pSecondary_Thread = NULL;
    m_bsec_thread_active = false;
  }
  CommDriverRegistry::getInstance().Deactivate(shared_from_this());
}

void CommDriverN2KSocketCAN::Activate() {
  CommDriverRegistry::getInstance().Activate(shared_from_this());
  // TODO: Read input data.
}

static uint64_t PayloadToName(const std::vector<unsigned char> payload) {
  uint64_t name;
  memcpy(&name, reinterpret_cast<const void*>(payload.data()), sizeof(name));
  return name;
}

void CommDriverN2KSocketCAN::handle_N2K_SocketCAN_RAW(
    CommDriverN2KSocketCANEvent& event) {
  auto p = event.GetPayload();

  std::vector<unsigned char>* payload = p.get();

  // extract PGN
  uint32_t pgn = 0;
  unsigned char* c = (unsigned char*)&pgn;

  *c++ = payload->at(3);
  *c++ = payload->at(4);
  *c++ = payload->at(5);

  auto name = PayloadToName(*payload);
  auto msg =
      std::make_unique<const Nmea2000Msg>(pgn, *payload, GetAddress(name));
  m_listener.Notify(std::move(msg));

#if 0  // Debug output
  size_t packetLength = (size_t)data->at(1);
  size_t vector_length = data->size();

  printf("Payload Length: %ld\n", vector_length);

  // extract PGN
  uint32_t v = 0;
  unsigned char *t = (unsigned char *)&v;
  *t++ = data->at(3);
  *t++ = data->at(4);
  *t++ = data->at(5);
  //memcpy(&v, &data[3], 1);

  printf("PGN: %d\n", v);

  for(size_t i=0; i< vector_length ; i++){
    printf("%02X ", data->at(i));
  }
  printf("\n\n");
#endif
}

#ifndef __ANDROID__

/**
 * This thread manages reading the N2K data stream provided by some N2K gateways
 * from the declared serial port.
 *
 */

// Commonly used raw format is actually inherited from an old paketizing format:
// <10><02><application data><CRC (1)><10><03>

// Actisense application data, from NGT-1 to PC
// <data code=93><length (1)><priority (1)><PGN (3)><destination(1)><source
// (1)><time (4)><len (1)><data (len)>

// As applied to a real application data element, after extraction from packet
// format: 93 13 02 01 F8 01 FF 01 76 C2 52 00 08 08 70 EB 14 E8 8E 52 D2 BB 10

// length (1):      0x13
// priority (1);    0x02
// PGN (3):         0x01 0xF8 0x01
// destination(1):  0xFF
// source (1):      0x01
// time (4):        0x76 0xC2 0x52 0x00
// len (1):         0x08
// data (len):      08 70 EB 14 E8 8E 52 D2
// packet CRC:      0xBB

CommDriverN2KSocketCANThread::CommDriverN2KSocketCANThread(
    CommDriverN2KSocketCAN* Launcher, const wxString& PortName) {
  m_pParentDriver = Launcher;  // This thread's immediate "parent"

  m_PortName = PortName;

  put_ptr = rx_buffer;  // local circular queue
  tak_ptr = rx_buffer;

  MapInitialize();
}

void CommDriverN2KSocketCANThread::OnExit(void) {}

void CommDriverN2KSocketCANThread::ThreadMessage(const wxString& msg) {
  //    Signal the main program thread
  //   OCPN_ThreadMessageEvent event(wxEVT_OCPN_THREADMSG, 0);
  //   event.SetSString(std::string(msg.mb_str()));
  //   if (gFrame) gFrame->GetEventHandler()->AddPendingEvent(event);
}

#ifndef __WXMSW__
void* CommDriverN2KSocketCANThread::Entry() {
  bool not_done = true;
  bool nl_found = false;
  wxString msg;

  CanHeader header;
  int recvbytes;
  struct can_frame canSocketFrame;

  // Create and open the CAN socket

  can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (can_socket < 0) {
    wxString msg("SocketCAN socket create failed: ");
    ThreadMessage(msg + m_PortName);
    return 0;
  }

  // eg. Native Interface "can0", Serial Interface "slcan0", Virtual Interface
  // "vcan0"
  std::string port_name(m_PortName);
  strcpy(can_request.ifr_name, port_name.c_str());

  // Get the index of the interface
  if (ioctl(can_socket, SIOCGIFINDEX, &can_request) < 0) {
    wxString msg("SocketCAN socket IOCTL (SIOCGIFINDEX) failed: ");
    ThreadMessage(msg + m_PortName);
    return 0;
  }

  can_address.can_family = AF_CAN;
  can_address.can_ifindex = can_request.ifr_ifindex;

  // Check if the interface is UP
  if (ioctl(can_socket, SIOCGIFFLAGS, &can_request) < 0) {
    wxString msg("SocketCAN socket IOCTL (SIOCGIFFLAGS) failed: ");
    ThreadMessage(msg + m_PortName);
    return 0;
  }

  if ((can_request.ifr_flags & IFF_UP)) {
    ThreadMessage(wxString("socketCan interface is UP"));
  } else {
    return 0;
  }

  // Set to non blocking
  fcntl(can_socket, F_SETFL, O_NONBLOCK);

  // set the timeout values
  socketTimeout.tv_sec = 0;
  socketTimeout.tv_usec = 100;

  // set the socket timeout
  setsockopt(can_socket, SOL_SOCKET, SO_RCVTIMEO, &socketTimeout,
             sizeof(socketTimeout));

  // and then bind
  int r = bind(can_socket, (struct sockaddr*)&can_address, sizeof(can_address));
  if (r < 0) {
    wxString("SocketCAN socket bind() failed: ");
    ThreadMessage(wxString("SocketCAN socket bind() failed: ") + m_PortName);
    return 0;
  }

  m_pParentDriver->SetSecThreadActive();  // I am alive

  // The main loop
  while ((not_done) && (m_pParentDriver->m_Thread_run_flag > 0)) {
    // Read from the socket

    // Set the file descriptor
    FD_ZERO(&readFD);
    FD_SET(can_socket, &readFD);

    // Set the timeout
    socketTimeout.tv_sec = 5;
    socketTimeout.tv_usec = 0;

    if (select((can_socket + 1), &readFD, NULL, NULL, &socketTimeout) != -1) {
      if (FD_ISSET(can_socket, &readFD)) {
        recvbytes = read(can_socket, &canSocketFrame, sizeof(struct can_frame));
        if (recvbytes) {
          int position = -1;
          bool bReady = false;

          DecodeCanHeader(canSocketFrame.can_id, &header);

          if (IsFastMessage(header) == TRUE) {
            position = MapFindMatchingEntry(header, canSocketFrame.data[0]);
            // No existing fast message
            if (position == NOT_FOUND) {
              // Find a free slot
              position = MapFindFreeEntry();
              // No free slots, exit
              if (position == NOT_FOUND) {
                // FIXME (dave) return;
              }
              // Insert the first frame of the fast message
              else {
                MapInsertEntry(header, canSocketFrame.data, position, bReady);
              }
            }
            // An existing fast message is present, append the frame
            else {
              MapAppendEntry(header, canSocketFrame.data, position, bReady);
            }
          }

          // This is a single frame message, parse it
          else {
            bReady = true;
          }

          if (bReady) {
            auto buffer = std::make_shared<std::vector<unsigned char>>();
            std::vector<unsigned char>* vec = buffer.get();

            // Populate the vector

            if (position >= 0) {  // re-assembled fast message
              // printf("PGN:  %d\n", header.pgn);

              vec->push_back(0x93);
              vec->push_back(fastMessages[position].expectedLength + 11);
              vec->push_back(header.priority);
              vec->push_back(header.pgn & 0xFF);
              vec->push_back((header.pgn >> 8) & 0xFF);
              vec->push_back((header.pgn >> 16) & 0xFF);
              vec->push_back(header.destination);
              vec->push_back(header.source);
              vec->push_back(
                  0xFF);  // FIXME (dave) Could generate the time fields
              vec->push_back(0xFF);
              vec->push_back(0xFF);
              vec->push_back(0xFF);
              vec->push_back(fastMessages[position].expectedLength);
              for (size_t n = 0; n < fastMessages[position].expectedLength; n++)
                vec->push_back(fastMessages[position].data[n]);
              vec->push_back(0x55);  // CRC dummy

              // Clear the message assembly buffer
              free(fastMessages[position].data);
              fastMessages[position].isFree = TRUE;
              fastMessages[position].data = NULL;
            } else {  // single frame message
              vec->push_back(0x93);
              vec->push_back(0x13);
              vec->push_back(header.priority);
              vec->push_back(header.pgn & 0xFF);
              vec->push_back((header.pgn >> 8) & 0xFF);
              vec->push_back((header.pgn >> 16) & 0xFF);
              vec->push_back(header.destination);
              vec->push_back(header.source);
              vec->push_back(0xFF);  // Fixme (dave) generate the time fields
              vec->push_back(0xFF);
              vec->push_back(0xFF);
              vec->push_back(0xFF);
              vec->push_back(CAN_MAX_DLEN);  // nominally 8
              for (size_t n = 0; n < CAN_MAX_DLEN; n++)
                vec->push_back(canSocketFrame.data[n]);
              vec->push_back(0x55);  // CRC dummy, not checked
            }

            CommDriverN2KSocketCANEvent frameReceivedEvent(
                wxEVT_COMMDRIVER_N2K_SOCKETCAN, 0);
            frameReceivedEvent.SetPayload(buffer);
            m_pParentDriver->AddPendingEvent(frameReceivedEvent);

          }  // bReady

        }  // recv bytes
      }    // FDISSET
    }      // select
  }        // while

  // thread_exit:
  m_pParentDriver->SetSecThreadInActive();  // I am dead
  m_pParentDriver->m_Thread_run_flag = -1;

  return 0;
}

// Checks whether a frame is a single frame message or multiframe Fast Packet
// message
bool CommDriverN2KSocketCANThread::IsFastMessage(const CanHeader header) {
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

// Determine whether an entry with a matching header & sequence ID exists.
// If not, then assume this is the first frame of a multi-frame Fast Message
int CommDriverN2KSocketCANThread::MapFindMatchingEntry(
    const CanHeader header, const unsigned char sid) {
  for (int i = 0; i < CONST_MAX_MESSAGES; i++) {
    if (((sid & 0xE0) == (fastMessages[i].sid & 0xE0)) &&
        (fastMessages[i].isFree == FALSE) &&
        (fastMessages[i].header.pgn == header.pgn) &&
        (fastMessages[i].header.source == header.source) &&
        (fastMessages[i].header.destination == header.destination)) {
      return i;
    }
  }
  return NOT_FOUND;
}

// Find first free entry in fastMessages
int CommDriverN2KSocketCANThread::MapFindFreeEntry(void) {
  for (int i = 0; i < CONST_MAX_MESSAGES; i++) {
    if (fastMessages[i].isFree == TRUE) {
      return i;
    }
  }
  // Could also run the Garbage Collection routine in a separate thread, would
  // require locking etc. But this will look for stale entries in case there are
  // no free entries If there are no free entries, then indicative that we are
  // receiving more Fast messages than I anticipated.
  int staleEntries;
  staleEntries = MapGarbageCollector();
  if (staleEntries == 0) {
    return NOT_FOUND;
    // FIXME (dave) Log this so as to increase the number of FastMessages that
    // may be received
    //  wxLogError(_T("socketCan Device, No free entries in Fast Message Map"));
  } else {
    return MapFindFreeEntry();
  }
}

int CommDriverN2KSocketCANThread::MapGarbageCollector(void) {
  int staleEntries;
  staleEntries = 0;
  for (int i = 0; i < CONST_MAX_MESSAGES; i++) {
    if ((fastMessages[i].isFree == FALSE) &&
        (GetTimeInMicroseconds() - fastMessages[i].timeArrived >
         CONST_TIME_EXCEEDED)) {
      staleEntries++;
      free(fastMessages[i].data);
      fastMessages[i].isFree = TRUE;
    }
  }
  return staleEntries;
}

// Insert the first message of a sequence of fast messages
void CommDriverN2KSocketCANThread::MapInsertEntry(const CanHeader header,
                                                  const unsigned char* data,
                                                  const int position,
                                                  bool& bReady) {
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
    fastMessages[position].expectedLength = (unsigned int)data[1];
    fastMessages[position].header = header;
    fastMessages[position].timeArrived = GetTimeInMicroseconds();
    ;
    fastMessages[position].isFree = FALSE;
    fastMessages[position].data = (unsigned char*)malloc(totalDataLength);
    memcpy(&fastMessages[position].data[0], &data[2], 6);
    // First frame of a multi-frame Fast Message contains six data bytes.
    // Position the cursor ready for next message
    fastMessages[position].cursor = 6;

    // Fusion, using fast messages to sends frames less than eight bytes
    if (fastMessages[position].expectedLength <= 6) {
      bReady = true;
    }
  }
  // No further processing is performed if this is not a start frame.
  // A start frame may have been dropped and we received a subsequent frame
}

// Append subsequent messages of a sequence of fast messages
// Subsequent messages of fast packet
// data[0] Sequence Identifier (sid)
// data[1..7] 7 data bytes
int CommDriverN2KSocketCANThread::MapAppendEntry(const CanHeader header,
                                                 const unsigned char* data,
                                                 const int position,
                                                 bool& bReady) {
  // Check that this is the next message in the sequence
  bReady = false;
  if ((fastMessages[position].sid + 1) == data[0]) {
    memcpy(&fastMessages[position].data[fastMessages[position].cursor],
           &data[1], 7);
    fastMessages[position].sid = data[0];
    // Subsequent messages contains seven data bytes (last message may be padded
    // with 0xFF)
    fastMessages[position].cursor += 7;
    // Is this the last message ?
    if (fastMessages[position].cursor >=
        fastMessages[position].expectedLength) {
      // Mark as ready for further processing
      bReady = true;
    }
    return TRUE;
  } else if ((data[0] & 0x1F) == 0) {
    // We've found a matching entry, however this is a start frame, therefore
    // we've missed an end frame, and now we have a start frame with the same id
    // (top 3 bits). The id has obviously rolled over. Should really double
    // check that (data[0] & 0xE0) Clear the entry as we don't want to leak
    // memory, prior to inserting a start frame
    free(fastMessages[position].data);
    fastMessages[position].isFree = TRUE;
    fastMessages[position].data = NULL;
    // And now insert it
    MapInsertEntry(header, data, position, bReady);
    // FIXME (dave) Should update the dropped frame stats
    return TRUE;
  } else {
    // This is not the next frame in the sequence and not a start frame
    // We've dropped an intermedite frame, so free the slot and do no further
    // processing
    free(fastMessages[position].data);
    fastMessages[position].isFree = TRUE;
    fastMessages[position].data = NULL;
    // Dropped Frame Statistics
    if (droppedFrames == 0) {
      droppedFrameTime = wxDateTime::Now();
      droppedFrames += 1;
    } else {
      droppedFrames += 1;
    }
    // FIXME (dave)
    //     if ((droppedFrames > CONST_DROPPEDFRAME_THRESHOLD) &&
    //     (wxDateTime::Now() < (droppedFrameTime +
    //     wxTimeSpan::Seconds(CONST_DROPPEDFRAME_PERIOD) ) ) ) {
    //       wxLogError(_T("TwoCan Device, Dropped Frames rate exceeded"));
    //       wxLogError(wxString::Format(_T("Frame: Source: %d Destination: %d
    //       Priority: %d PGN: %d"),header.source, header.destination,
    //       header.priority, header.pgn)); droppedFrames = 0;
    //     }
    return FALSE;
  }
}

// Initialize each entry in the Fast Message Map
void CommDriverN2KSocketCANThread::MapInitialize(void) {
  for (int i = 0; i < CONST_MAX_MESSAGES; i++) {
    fastMessages[i].isFree = TRUE;
    fastMessages[i].data = NULL;
  }
}

#endif
#endif
