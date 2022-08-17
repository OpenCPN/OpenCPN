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

#include <vector>
#include <mutex>  // std::mutex
#include <queue>  // std::queue

#include "comm_drv_n2k_socketcan.h"
#include "comm_navmsg_bus.h"
#include "comm_drv_registry.h"

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


// Decodes a 29 bit CAN header from an int
void DecodeCanHeader(const int canId, CanHeader *header) {
	unsigned char buf[4];
	buf[0] = canId & 0xFF;
	buf[1] = (canId >> 8) & 0xFF;
	buf[2] = (canId >> 16) & 0xFF;
	buf[3] = (canId >> 24) & 0xFF;

	header->source = buf[0];
	header->destination = buf[2] < 240 ? buf[1] : 255;
	header->pgn = (buf[3] & 0x01) << 16 | (buf[2] << 8) | (buf[2] < 240 ? 0 : buf[1]);
	header->priority = (buf[3] & 0x1c) >> 2;
}

class CommDriverN2KSocketCANEvent;  // fwd

class CommDriverN2KSocketCANThread : public wxThread {
public:
  CommDriverN2KSocketCANThread(CommDriverN2KSocketCAN* Launcher,
                            const wxString& PortName,
                            const wxString& strBaudRate);

  ~CommDriverN2KSocketCANThread(void);
  void* Entry();
  bool SetOutMsg(const wxString& msg);
  void OnExit(void);

private:
#ifndef __OCPN__ANDROID__
  serial::Serial m_serial;
#endif
  void ThreadMessage(const wxString& msg);
  bool OpenComPortPhysical(const wxString& com_name, int baud_rate);
  void CloseComPortPhysical();
  size_t WriteComPortPhysical(char* msg);
  size_t WriteComPortPhysical(const wxString& string);

  CommDriverN2KSocketCAN* m_pParentDriver;
  wxString m_PortName;
  wxString m_FullPortName;

  unsigned char* put_ptr;
  unsigned char* tak_ptr;

  unsigned char* rx_buffer;

  int m_baud;
  int m_n_timeout;

  n2k_atomic_queue<char*> out_que;

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
    CommDriverN2KSocketCANEvent* newevent = new CommDriverN2KSocketCANEvent(*this);
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
  Bind(wxEVT_COMMDRIVER_N2K_SOCKETCAN, &CommDriverN2KSocketCAN::handle_N2K_SocketCAN_RAW,
       this);

  Open();
}

CommDriverN2KSocketCAN::~CommDriverN2KSocketCAN() {}

bool CommDriverN2KSocketCAN::Open() {
  wxString comx;
  comx = m_params.GetDSPort().AfterFirst(':');  // strip "Serial:"

  comx =
      comx.BeforeFirst(' ');  // strip off any description provided by Windows

  //    Kick off the  RX thread
  SetSecondaryThread(new CommDriverN2KSocketCANThread(this, comx, "220"));
  SetThreadRunFlag(1);
  GetSecondaryThread()->Run();

  return true;
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
  uint64_t pgn = 0;
  unsigned char* c = (unsigned char*)&pgn;
  *c++ = payload->at(3);
  *c++ = payload->at(4);
  *c++ = payload->at(5);
  // memcpy(&v, &data[3], 1);

  auto name = PayloadToName(*payload);
  auto msg = std::make_unique<const Nmea2000Msg>(pgn, *payload,
                                                 GetAddress(name));
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

#define DS_RX_BUFFER_SIZE 4096

CommDriverN2KSocketCANThread::CommDriverN2KSocketCANThread(
    CommDriverN2KSocketCAN* Launcher, const wxString& PortName,
    const wxString& strBaudRate) {
  m_pParentDriver = Launcher;  // This thread's immediate "parent"

  m_PortName = PortName;
  m_FullPortName = _T("Serial:") + PortName;

  rx_buffer = new unsigned char[DS_RX_BUFFER_SIZE + 1];

  put_ptr = rx_buffer;  // local circular queue
  tak_ptr = rx_buffer;

  m_baud = 4800;  // default
  long lbaud;
  if (strBaudRate.ToLong(&lbaud)) m_baud = (int)lbaud;

  Create();
}

CommDriverN2KSocketCANThread::~CommDriverN2KSocketCANThread(void) {
  delete[] rx_buffer;
}

void CommDriverN2KSocketCANThread::OnExit(void) {}

bool CommDriverN2KSocketCANThread::OpenComPortPhysical(const wxString& com_name,
                                                    int baud_rate) {
  try {
    m_serial.setPort(com_name.ToStdString());
    m_serial.setBaudrate(baud_rate);
    m_serial.open();
    m_serial.setTimeout(250, 250, 0, 250, 0);
  } catch (std::exception& e) {
    // std::cerr << "Unhandled Exception while opening serial port: " <<
    // e.what() << std::endl;
  }
  return m_serial.isOpen();
}

void CommDriverN2KSocketCANThread::CloseComPortPhysical() {
  try {
    m_serial.close();
  } catch (std::exception& e) {
    // std::cerr << "Unhandled Exception while closing serial port: " <<
    // e.what() << std::endl;
  }
}

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
    wxString msg(_T("SocketCAN socket create failed: "));
    msg.Append(m_PortName);
    ThreadMessage(msg);
    return 0;
  }

  // (eg. Native Interface CAN0, Serial Interface SLCAN0, Virtual Interface VCAN0)
  std::string port_name("vcan0");
  strcpy(can_request.ifr_name, port_name.c_str());

  // Get the index of the interface
  if (ioctl(can_socket, SIOCGIFINDEX, &can_request) < 0) {
    wxString msg(_T("SocketCAN socket IOCTL (SIOCGIFINDEX) failed: "));
    msg.Append(m_PortName);
    ThreadMessage(msg);
    return 0;
  }

  can_address.can_family = AF_CAN;
  can_address.can_ifindex = can_request.ifr_ifindex;

  // Check if the interface is UP
  if (ioctl(can_socket, SIOCGIFFLAGS, &can_request) < 0) {
    wxString msg(_T("SocketCAN socket IOCTL (SIOCGIFFLAGS) failed: "));
    msg.Append(m_PortName);
    ThreadMessage(msg);
    return 0;
  }

  if ((can_request.ifr_flags & IFF_UP)) {
    wxString msg(_T("socketCan interface is UP"));
    ThreadMessage(msg);
  }
  else {
    wxString msg(_T("socketCan Socket interface is DOWN"));
    return 0;
  }

  // Set to non blocking
  fcntl(can_socket, F_SETFL, O_NONBLOCK);

  // set the timeout values
  socketTimeout.tv_sec = 0;
  socketTimeout.tv_usec = 100;

  // set the socket timeout
  setsockopt (can_socket, SOL_SOCKET, SO_RCVTIMEO, &socketTimeout, sizeof(socketTimeout));

  // and then bind
  if (bind(can_socket, (struct sockaddr *)&can_address, sizeof(can_address)) < 0) {
    wxString msg(_T("SocketCAN socket bind() failed: "));
    msg.Append(m_PortName);
    ThreadMessage(msg);
    return 0;
  }


  m_pParentDriver->SetSecThreadActive();  // I am alive

    // The main loop
  while ((not_done) && (m_pParentDriver->m_Thread_run_flag > 0)) {

    if (TestDestroy()) {
      not_done = false;
    }

    // Read from the socket

    // Set the file descriptor
    FD_ZERO(&readFD);
    FD_SET(can_socket, &readFD);

    // Set the timeout
    //FIXME Too long, maybe 5 secs?
    socketTimeout.tv_sec = 500;
    socketTimeout.tv_usec = 0;

    if (select((can_socket + 1), &readFD, NULL, NULL, &socketTimeout) != -1)	{
      if (FD_ISSET(can_socket, &readFD)) {
        recvbytes = read(can_socket, &canSocketFrame, sizeof(struct can_frame));
        if (recvbytes) {

          DecodeCanHeader(canSocketFrame.can_id,&header);

          auto buffer = std::make_shared<std::vector<unsigned char>>();
          std::vector<unsigned char>* vec = buffer.get();

          //unsigned char buffer[40];

          // Populate the buffer

          vec->push_back(0x12); // Won't append Actisense CRC
          vec->push_back(header.priority);
          vec->push_back(header.pgn & 0xFF);
          vec->push_back((header.pgn >> 8) & 0xFF);
          vec->push_back((header.pgn >> 16) & 0xFF);
          vec->push_back(header.destination);
          vec->push_back(header.source);
          vec->push_back(0xFF); // BUG BUG Could generate the time fields
          vec->push_back(0xFF);
          vec->push_back(0xFF);
          vec->push_back(0xFF);
          vec->push_back(sizeof(struct can_frame)); //.dlc has been deprecated
          for (size_t n = 0; n < sizeof(struct can_frame); n++)
            vec->push_back(canSocketFrame.data[n]);
          //memcpy(&buffer[12], canSocketFrame.data, sizeof(struct can_frame));
          // BUG BUG Could calculate a CRC to keep Dave/Alec happy


          // BUG BUG just filter on known 8 byte PGN's of interest to OCPN
          // Position (rapid update), COG/SOG (rapid update), Heading,

          if ((header.pgn == 129025) || (header.pgn == 129026) || (header.pgn == 127250)) {
            printf("PGN:  %d\n", header.pgn);

            CommDriverN2KSocketCANEvent frameReceivedEvent(wxEVT_COMMDRIVER_N2K_SOCKETCAN, 0);
            frameReceivedEvent.SetPayload(buffer);
            m_pParentDriver->AddPendingEvent(frameReceivedEvent);

          } // supported pgn's
        } // recv bytes
			} //FDISSET
		} // select
	} // while


#if 0
  //    The main loop
  static size_t retries = 0;

  bool bInMsg = false;
  bool bGotESC = false;
  bool bGotSOT = false;

  while ((not_done) && (m_pParentDriver->m_Thread_run_flag > 0)) {
    if (TestDestroy()) not_done = false;  // smooth exit

    uint8_t next_byte = 0;
    size_t newdata = -1;
    if (m_serial.isOpen()) {
      try {
        newdata = m_serial.read(&next_byte, 1);
      } catch (std::exception& e) {
        // std::cerr << "Serial read exception: " << e.what() << std::endl;
        if (10 < retries++) {
          // We timed out waiting for the next character 10 times, let's close
          // the port so that the reconnection logic kicks in and tries to fix
          // our connection.
          CloseComPortPhysical();
          retries = 0;
        }
      }
    } else {
      // Reconnection logic. Let's try to reopen the port while waiting longer
      // every time (until we simply keep trying every 2.5 seconds)
      // std::cerr << "Serial port seems closed." << std::endl;
      wxMilliSleep(250 * retries);
      CloseComPortPhysical();
      if (OpenComPortPhysical(m_PortName, m_baud))
        retries = 0;
      else if (retries < 10)
        retries++;
    }

    if (newdata == 1) {
      if (bInMsg) {
        if (bGotESC) {
          if (ESCAPE == next_byte) {
            *put_ptr++ = next_byte;
            if ((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE) put_ptr = rx_buffer;
            bGotESC = false;
          }
        }

        if (bGotESC && (ENDOFTEXT == next_byte)) {
          // Process packet
          //    Copy the message into a std::vector

          auto buffer = std::make_shared<std::vector<unsigned char>>();
          std::vector<unsigned char>* vec = buffer.get();

          unsigned char* tptr;
          tptr = tak_ptr;

          while ((tptr != put_ptr)) {
            vec->push_back(*tptr++);
            if ((tptr - rx_buffer) > DS_RX_BUFFER_SIZE) tptr = rx_buffer;
          }

          tak_ptr = tptr;
          bInMsg = false;
          bGotESC = false;

          // Message is finished
          // Send the captured raw data vector pointer to the thread's "parent"
          //  thereby releasing the thread for further data capture
          CommDriverN2KSocketCANEvent Nevent(wxEVT_COMMDRIVER_N2K_SOCKETCOMM, 0);
          Nevent.SetPayload(buffer);
          m_pParentDriver->AddPendingEvent(Nevent);

        } else {
          bGotESC = (next_byte == ESCAPE);

          if (!bGotESC) {
            *put_ptr++ = next_byte;
            if ((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE) put_ptr = rx_buffer;
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

            *put_ptr++ = next_byte;
            if ((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE) put_ptr = rx_buffer;
          }
        }
      }
    }  // if newdata > 0

    //      Check for any pending output message
#if 0
    bool b_qdata = !out_que.empty();

    while (b_qdata) {
      //  Take a copy of message
      char *qmsg = out_que.front();
      out_que.pop();
      // m_outCritical.Leave();
      char msg[MAX_OUT_QUEUE_MESSAGE_LENGTH];
      strncpy(msg, qmsg, MAX_OUT_QUEUE_MESSAGE_LENGTH - 1);
      free(qmsg);

      if (static_cast<size_t>(-1) == WriteComPortPhysical(msg) &&
          10 < retries++) {
        // We failed to write the port 10 times, let's close the port so that
        // the reconnection logic kicks in and tries to fix our connection.
        retries = 0;
        CloseComPortPhysical();
      }

      b_qdata = !out_que.empty();
    }  // while b_qdata

#endif
  }  // while ((not_done)

#endif

  // thread_exit:
  CloseComPortPhysical();
  m_pParentDriver->SetSecThreadInActive();  // I am dead
  m_pParentDriver->m_Thread_run_flag = -1;

  return 0;
}

#endif
#endif
