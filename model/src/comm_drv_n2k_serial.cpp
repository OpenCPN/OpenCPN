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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <vector>
#include <mutex>  // std::mutex
#include <queue>  // std::queue

#include <wx/log.h>

#include "model/comm_drv_n2k_serial.h"
#include "model/comm_navmsg_bus.h"
#include "model/comm_drv_registry.h"
#include "model/logger.h"

#include <N2kMsg.h>
std::vector<unsigned char> BufferToActisenseFormat( tN2kMsg &msg);

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


class CommDriverN2KSerialEvent;  // fwd

class CommDriverN2KSerialThread : public wxThread {
public:
  CommDriverN2KSerialThread(CommDriverN2KSerial* Launcher,
                            const wxString& PortName,
                            const wxString& strBaudRate);

  ~CommDriverN2KSerialThread(void);
  void* Entry();
  bool SetOutMsg(const std::vector<unsigned char> &load);
  void OnExit(void);

private:
#ifndef __ANDROID__
  serial::Serial m_serial;
#endif
  void ThreadMessage(const wxString& msg);
  bool OpenComPortPhysical(const wxString& com_name, int baud_rate);
  void CloseComPortPhysical();
  size_t WriteComPortPhysical(std::vector<unsigned char> msg);
  size_t WriteComPortPhysical(unsigned char *msg, size_t length);
  void SetGatewayOperationMode(void);

  CommDriverN2KSerial* m_pParentDriver;
  wxString m_PortName;
  wxString m_FullPortName;

  unsigned char* put_ptr;
  unsigned char* tak_ptr;

  unsigned char* rx_buffer;

  int m_baud;
  int m_n_timeout;

  n2k_atomic_queue<std::vector<unsigned char>> out_que;

#ifdef __WXMSW__
  HANDLE m_hSerialComm;
  bool m_nl_found;
#endif
};

class CommDriverN2KSerialEvent;
wxDECLARE_EVENT(wxEVT_COMMDRIVER_N2K_SERIAL, CommDriverN2KSerialEvent);

class CommDriverN2KSerialEvent : public wxEvent {
public:
  CommDriverN2KSerialEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
      : wxEvent(id, commandType){};
  ~CommDriverN2KSerialEvent(){};

  // accessors
  void SetPayload(std::shared_ptr<std::vector<unsigned char>> data) {
    m_payload = data;
  }
  std::shared_ptr<std::vector<unsigned char>> GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const {
    CommDriverN2KSerialEvent* newevent = new CommDriverN2KSerialEvent(*this);
    newevent->m_payload = this->m_payload;
    return newevent;
  };

private:
  std::shared_ptr<std::vector<unsigned char>> m_payload;
};

//========================================================================
/*    commdriverN2KSerial implementation
 * */

wxDEFINE_EVENT(wxEVT_COMMDRIVER_N2K_SERIAL, CommDriverN2KSerialEvent);

CommDriverN2KSerial::CommDriverN2KSerial(const ConnectionParams* params,
                                         DriverListener& listener)
    : CommDriverN2K(((ConnectionParams*)params)->GetStrippedDSPort()),
      m_Thread_run_flag(-1),
      m_bok(false),
      m_portstring(params->GetDSPort()),
      m_pSecondary_Thread(NULL),
      m_params(*params),
      m_listener(listener) {
  m_BaudRate = wxString::Format("%i", params->Baudrate), SetSecThreadInActive();
  m_manufacturers_code = 0;
  m_got_mfg_code = false;
  this->attributes["canAddress"] = std::string("-1");

  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_N2K_SERIAL, &CommDriverN2KSerial::handle_N2K_SERIAL_RAW,
       this);

  Open();

  wxMilliSleep(100);
  GetMfgCode();

#if 0
  // Testing TX of Heartbeat
  wxSleep(1);

  tN2kMsg N2kMsg;   // automatically sets destination 255
  //SetHeartbeat(N2kMsg,2000,0);
      //SetN2kPGN126993(N2kMsg, 2000, 0);
        N2kMsg.SetPGN(126993L);
        //N2kMsg.Priority=7;
        N2kMsg.Source = 2;
        N2kMsg.Destination = 133;
        N2kMsg.Add2ByteUInt((uint16_t)(2000));    // Rate, msec

        N2kMsg.AddByte(0);    //Status
        N2kMsg.AddByte(0xff); // Reserved
        N2kMsg.Add4ByteUInt(0xffffffff); // Reserved

  const std::vector<unsigned char> mv = BufferToActisenseFormat(N2kMsg);

  size_t len = mv.size();

  wxString comx = m_params.GetDSPort().AfterFirst(':');
  std::string interface = comx.ToStdString();

  N2kName source_name(1234);
  auto source_address = std::make_shared<NavAddr2000>(interface, source_name);
  auto dest_address = std::make_shared<NavAddr2000>(interface, N2kMsg.Destination);

  auto message_to_send = std::make_shared<Nmea2000Msg>(126993L,
                                  mv, source_address, 3);

    for(size_t i=0; i< mv.size(); i++){
      printf("%02X ", mv.at(i));
    }
    printf("\n\n");

  SetTXPGN(126993);
  wxSleep(1);

  SendMessage(message_to_send, dest_address);

  int yyp = 4;
#endif


}

CommDriverN2KSerial::~CommDriverN2KSerial() {
  Close();
}

bool CommDriverN2KSerial::Open() {
  wxString comx;
  comx = m_params.GetDSPort().AfterFirst(':');  // strip "Serial:"

  comx =
      comx.BeforeFirst(' ');  // strip off any description provided by Windows

#ifndef ANDROID
  //    Kick off the  RX thread
  SetSecondaryThread(new CommDriverN2KSerialThread(this, comx, m_BaudRate));
  SetThreadRunFlag(1);
  GetSecondaryThread()->Run();
#endif

  return true;
}

void CommDriverN2KSerial::Close() {
  wxLogMessage(
      wxString::Format(_T("Closing N2K Driver %s"), m_portstring.c_str()));

  //    Kill off the Secondary RX Thread if alive
  if (m_pSecondary_Thread) {
    if (m_bsec_thread_active)  // Try to be sure thread object is still alive
    {
      wxLogMessage(_T("Stopping Secondary Thread"));

      m_Thread_run_flag = 0;
      int tsec = 10;
      while ((m_Thread_run_flag >= 0) && (tsec--)) wxSleep(1);

      wxString msg;
      if (m_Thread_run_flag < 0)
        msg.Printf(_T("Stopped in %d sec."), 10 - tsec);
      else
        msg.Printf(_T("Not Stopped after 10 sec."));
      wxLogMessage(msg);
    }

    m_pSecondary_Thread = NULL;
    m_bsec_thread_active = false;
  }
}

void CommDriverN2KSerial::Activate() {
  CommDriverRegistry::GetInstance().Activate(shared_from_this());
  // TODO: Read input data.
}

bool CommDriverN2KSerial::SendMessage(std::shared_ptr<const NavMsg> msg,
                                        std::shared_ptr<const NavAddr> addr) {
#ifndef ANDROID

  auto msg_n2k = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
  std::vector<uint8_t> load = msg_n2k->payload;

  uint64_t _pgn = msg_n2k->PGN.pgn;
  auto destination_address = std::static_pointer_cast<const NavAddr2000>(addr);

  tN2kMsg N2kMsg;   // automatically sets destination 255
  N2kMsg.SetPGN(_pgn);
  N2kMsg.Priority = msg_n2k->priority;
  if (destination_address)
    N2kMsg.Destination = destination_address->address;

  for (size_t i=0 ; i < load.size(); i++)
    N2kMsg.AddByte(load.at(i));    //data

  const std::vector<uint8_t> mv = BufferToActisenseFormat(N2kMsg);

//    printf("mv\n");
//    for(size_t i=0; i< mv.size(); i++){
//        printf("%02X ", mv.at(i));
//      }
//    printf("\n\n");

    if( GetSecondaryThread() ) {
        if( IsSecThreadActive() )
        {
            int retry = 10;
            while( retry ) {
                if( GetSecondaryThread()->SetOutMsg(mv))
                    return true;
                else
                    retry--;
            }
            return false;   // could not send after several tries....
        }
        else
            return false;
    }
#endif
    return true;
}

void CommDriverN2KSerial::ProcessManagementPacket(std::vector<unsigned char> *payload) {

   if (payload->at(2) != 0xF2) {   // hearbeat
     //printf("    pl ");
     //for (unsigned int i = 0; i < payload->size(); i++)
     // printf("%02X ", payload->at(i));
     //printf("\n");
   }

  switch (payload->at(2)){
    case 0x47:
      m_bmg47_resp = true;
      break;
    case 0x01:
      m_bmg01_resp = true;
      break;
    case 0x4B:
      m_bmg4B_resp = true;
      break;
    case 0x041: {
      m_bmg41_resp = true;
      if (payload->at(3) == 0x02) {     // ASCII device_common_name
        std::string device_common_name;
        for (unsigned int i = 0; i < 32; i++) {
          device_common_name += payload->at(i + 14);
        }
        device_common_name += '\0';
        m_device_common_name = device_common_name;
      }
      break;
    }
    case 0x042: {
      m_bmg42_resp = true;
      unsigned char name[8];
      for (unsigned int i = 0; i < 8; i++)
        name[i] = payload->at(i + 15);

      memcpy( (void *)&NAME, name, 8);
      // Extract the manufacturers code
      int *f1 = (int *)&NAME;
      int f1d = *f1;
      m_manufacturers_code = f1d >> 21;
      break;
    }

    default:
      break;

  }
}


static uint64_t PayloadToName(const std::vector<unsigned char> payload) {
  uint64_t name;
  memcpy(&name, reinterpret_cast<const void*>(payload.data()), sizeof(name));
  return name;
}


void CommDriverN2KSerial::handle_N2K_SERIAL_RAW(
    CommDriverN2KSerialEvent& event) {
  auto p = event.GetPayload();

  std::vector<unsigned char>* payload = p.get();

  if (payload->at(0) == 0xA0) {
    ProcessManagementPacket(payload);
    return;
  }

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

#if 0  // Debug output
  size_t packetLength = (size_t)payload->at(1);
  size_t vector_length = payload->size();


  //if(pgn > 120000)
  {
    printf("Payload Length: %ld\n", vector_length);

    printf("PGN: %ld\n", pgn);

    for(size_t i=0; i< vector_length ; i++){
      printf("%02X ", payload->at(i));
    }
    printf("\n\n");
  }
#endif
}

int CommDriverN2KSerial::GetMfgCode(){
  unsigned char request_name[] = { 0x42};
  int ni = SendMgmtMsg( request_name, sizeof(request_name), 0x41, 2000, &m_bmg42_resp);
  if (ni)
    return ni;     // Not responding, return error so upper retries.
  m_got_mfg_code = true;
  return 0;
}

int CommDriverN2KSerial::SendMgmtMsg( unsigned char *string, size_t string_size,
                                      unsigned char cmd_code,
                                      int timeout_msec, bool *response_flag) {
#ifndef ANDROID
    // Make a message
  int byteSum = 0;
  uint8_t CheckSum;
  std::vector <unsigned char> msg;

  msg.push_back(ESCAPE);
  msg.push_back(STARTOFTEXT);
  msg.push_back(0xA1);
  byteSum += 0xA1;
  msg.push_back(string_size);      // payload length
  byteSum += string_size;

  for (unsigned int i=0; i < string_size; i++){
    if (string[i] == ESCAPE)
      msg.push_back(string[i]);
    msg.push_back(string[i]);
    byteSum += string[i];
  }

  // checksum
  byteSum %= 256;
  CheckSum = (uint8_t)((byteSum == 0) ? 0 : (256 - byteSum));
  msg.push_back(CheckSum);

  msg.push_back(ESCAPE);
  msg.push_back(ENDOFTEXT);

  // send it out

  if (response_flag)
    *response_flag = false;  // prime the response detector

  // Send the msg
  bool bsent = false;
  bool not_done = true;
  int ntry_outer = 10;
  while ( not_done ){
    if( GetSecondaryThread() && IsSecThreadActive() ) {
      int retry = 10;
      while( retry ) {
        if( GetSecondaryThread()->SetOutMsg( msg )){
          bsent = true;
          not_done = false;
          break;
        }
        else
          retry--;
      }
    }
    else {
      wxMilliSleep(100);
      if (ntry_outer-- <= 0)
        not_done = false;
    }
  }

  if (!bsent)
    return 1;

  bool bOK = false;
  if (timeout_msec) {
    int timeout = timeout_msec;
    while (timeout > 0) {
      wxYieldIfNeeded();
      wxMilliSleep(100);
      if (response_flag){
        if (*response_flag){
          bOK = true;
          break;
        }
      }
      timeout -= 100;
    }
  }
  else
    bOK = true;

  if(!bOK){
    //printf( "***Err-1\n");
    return 1;
  }
  //else
    //printf("***OK-1  %d\n", timeout);
#endif
  return 0;
}

/* Copied from canboat Project
 * https://github.com/canboat/canboat
 *
 * The following startup command reverse engineered from Actisense NMEAreader.
 * It instructs the NGT1 to clear its PGN message TX list, thus it starts
 * sending all PGNs.
 */
static unsigned char NGT_STARTUP_SEQ[] = {
    0x11, /* msg byte 1, meaning ? */
    0x02, /* msg byte 2, meaning ? */
    0x00  /* msg byte 3, meaning ? */
};

int CommDriverN2KSerial::SetTXPGN(int pgn) {

  // Try to detect Actisense NGT-xx, has Mfg_code == 273
//   if (m_got_mfg_code) {
//     if (m_manufacturers_code != 273)
//       return 0;  // Not Actisense, no error
//   }

  SendMgmtMsg( NGT_STARTUP_SEQ, sizeof(NGT_STARTUP_SEQ), 0x11, 0, NULL);


#if 0
  //  Enable PGN message
  unsigned char request_enable[] = { 0x47,
                      0x00, 0x00, 0x00,           //pgn
                      0x00, 0x01,
                      0xFF, 0xFF, 0xFF, 0xFF};

  int PGN = 0;
  unsigned char* c = (unsigned char*)&pgn;
  request_enable[1] = c[0];
  request_enable[2] = c[1];
  request_enable[3] = c[2];

  int aa = SendMgmtMsg( request_enable, sizeof(request_enable), 0x47, 2000, &m_bmg47_resp);
//   if (aa)
//     return -1;

  //  Commit message
  unsigned char request_commit[] = { 0x01 };
  int bb = SendMgmtMsg( request_commit, sizeof(request_commit), 0x01, 2000, &m_bmg01_resp);
//   if (bb)
//     return -2;


  // Activate message
  unsigned char request_activate[] = { 0x4B };
  int cc = SendMgmtMsg( request_activate, sizeof(request_activate), 0x4B, 2000, &m_bmg4B_resp);
//   if (cc)
//     return -3;
#endif


  return 0;
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

CommDriverN2KSerialThread::CommDriverN2KSerialThread(
    CommDriverN2KSerial* Launcher, const wxString& PortName,
    const wxString& strBaudRate) {
  m_pParentDriver = Launcher;  // This thread's immediate "parent"

  m_PortName = PortName;
  m_FullPortName = _T("Serial:") + PortName;

  rx_buffer = new unsigned char[DS_RX_BUFFER_SIZE + 1];

  put_ptr = rx_buffer;  // local circular queue
  tak_ptr = rx_buffer;

  m_baud = 9600;  // default
  long lbaud;
  if (strBaudRate.ToLong(&lbaud)) m_baud = (int)lbaud;

  Create();
}

CommDriverN2KSerialThread::~CommDriverN2KSerialThread(void) {
  delete[] rx_buffer;
}

void CommDriverN2KSerialThread::OnExit(void) {}

bool CommDriverN2KSerialThread::OpenComPortPhysical(const wxString& com_name,
                                                    int baud_rate) {
  try {
    m_serial.setPort(com_name.ToStdString());
    m_serial.setBaudrate(baud_rate);
    m_serial.open();
    m_serial.setTimeout(250, 250, 0, 250, 0);
  } catch (std::exception&) {
    // std::cerr << "Unhandled Exception while opening serial port: " <<
    // e.what() << std::endl;
  }
  return m_serial.isOpen();
}

void CommDriverN2KSerialThread::CloseComPortPhysical() {
  try {
    m_serial.close();
  } catch (std::exception&) {
    // std::cerr << "Unhandled Exception while closing serial port: " <<
    // e.what() << std::endl;
  }
}

void CommDriverN2KSerialThread::SetGatewayOperationMode(void) {

  // For YDNU-02 device
  // From Device User Manual
  // Set the mode to "N2K"
  unsigned char config_string[] = { 0x10, 0x02, 0xA1, 0x03, 0x11,
                                    0x02, 0x00, 0x49, 0x10, 0x03};
	//std::vector<byte>writeBuffer {DLE,STX,NGT_TX_CMD,0x03,0x11,0x02,0x00,0x49, DLE,ETX};

  WriteComPortPhysical(config_string, 10);

}


void CommDriverN2KSerialThread::ThreadMessage(const wxString& msg) {
  //    Signal the main program thread
  //   OCPN_ThreadMessageEvent event(wxEVT_OCPN_THREADMSG, 0);
  //   event.SetSString(std::string(msg.mb_str()));
  //   if (gFrame) gFrame->GetEventHandler()->AddPendingEvent(event);
}

size_t CommDriverN2KSerialThread::WriteComPortPhysical(std::vector<unsigned char> msg) {
  if (m_serial.isOpen()) {
    ssize_t status = 0;
#if 0
    printf("X                ");
    for (size_t i = 0; i < msg.size(); i++) printf("%02X ", msg[i]);
    printf("\n");
#endif
    try {
      status = m_serial.write((uint8_t*)msg.data(), msg.size());
    } catch (std::exception& e) {
      WARNING_LOG << "Unhandled Exception while writing to serial port: "
          << e.what();
      return -1;
    }
    return status;
  } else {
    return -1;
  }
}

size_t CommDriverN2KSerialThread::WriteComPortPhysical(unsigned char *msg, size_t length) {
  if (m_serial.isOpen()) {
    ssize_t status;
    try {
      status = m_serial.write((uint8_t*)msg, length);
    } catch (std::exception&) {
//       std::cerr << "Unhandled Exception while writing to serial port: " <<
//       e.what() << std::endl;
      return -1;
    }
    return status;
  } else {
    return -1;
  }
}

bool CommDriverN2KSerialThread::SetOutMsg(const std::vector<unsigned char> &msg)
{
  if(out_que.size() < OUT_QUEUE_LENGTH){
    out_que.push(msg);
    return true;
  }
  return false;
}

#ifndef __WXMSW__
void* CommDriverN2KSerialThread::Entry() {
  bool not_done = true;
  bool nl_found = false;
  wxString msg;
  uint8_t rdata[2000];
  circular_buffer<uint8_t> circle(DS_RX_BUFFER_SIZE);
  int ib = 0;

  //    Request the com port from the comm manager
  if (!OpenComPortPhysical(m_PortName, m_baud)) {
    wxString msg(_T("NMEA input device open failed: "));
    msg.Append(m_PortName);
    ThreadMessage(msg);
    // goto thread_exit; // This means we will not be trying to connect = The
    // device must be connected when the thread is created. Does not seem to be
    // needed/what we want as the reconnection logic is able to pick it up
    // whenever it actually appears (Of course given it appears with the
    // expected device name).
  }
  else {
    wxMilliSleep(100);
    SetGatewayOperationMode();
  }


  m_pParentDriver->SetSecThreadActive();  // I am alive

  //    The main loop
  static size_t retries = 0;

  bool bInMsg = false;
  bool bGotESC = false;
  bool bGotSOT = false;

  while ((not_done) && (m_pParentDriver->m_Thread_run_flag > 0)) {
    if (TestDestroy()) not_done = false;  // smooth exit

    uint8_t next_byte = 0;
    int newdata = 0;
    if (m_serial.isOpen()) {
      try {
        newdata = m_serial.read(rdata, 1000);
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
      if (OpenComPortPhysical(m_PortName, m_baud)){
        SetGatewayOperationMode();
        retries = 0;
      }
      else if (retries < 10)
        retries++;
    }

    if (newdata > 0) {
      for (int i = 0; i < newdata; i++) {
        circle.put(rdata[i]);
      }
    }

    while (!circle.empty()) {
      if (ib >= DS_RX_BUFFER_SIZE)
        ib = 0;
      uint8_t next_byte = circle.get();

      if (bInMsg) {
        if (bGotESC) {
          if (ESCAPE == next_byte) {
            rx_buffer[ib++] = next_byte;
            bGotESC = false;
          }
        }

        if (bGotESC && (ENDOFTEXT == next_byte)) {
          // Process packet
          //    Copy the message into a std::vector

          auto buffer = std::make_shared<std::vector<unsigned char>>(rx_buffer, rx_buffer + ib);
          std::vector<unsigned char>* vec = buffer.get();

          ib = 0;
          //tak_ptr = tptr;
          bInMsg = false;
          bGotESC = false;

//           printf("raw ");
//              for (unsigned int i = 0; i < vec->size(); i++)
//                printf("%02X ", vec->at(i));
//              printf("\n");

          // Message is finished
          // Send the captured raw data vector pointer to the thread's "parent"
          //  thereby releasing the thread for further data capture
          CommDriverN2KSerialEvent Nevent(wxEVT_COMMDRIVER_N2K_SERIAL, 0);
          Nevent.SetPayload(buffer);
          m_pParentDriver->AddPendingEvent(Nevent);


        } else {
          bGotESC = (next_byte == ESCAPE);

          if (!bGotESC) {
            rx_buffer[ib++] = next_byte;
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

            rx_buffer[ib++] = next_byte;
          }
        }
      }
    }  // if newdata > 0

    //      Check for any pending output message
#if 1
    bool b_qdata = !out_que.empty();

    while (b_qdata) {
      //  Take a copy of message
      std::vector<unsigned char> qmsg = out_que.front();
      out_que.pop();

      if (static_cast<size_t>(-1) == WriteComPortPhysical(qmsg) &&
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

  // thread_exit:
  CloseComPortPhysical();
  m_pParentDriver->SetSecThreadInActive();  // I am dead
  m_pParentDriver->m_Thread_run_flag = -1;

  return 0;
}

#else
void* CommDriverN2KSerialThread::Entry() {
  bool not_done = true;
  bool nl_found = false;
  wxString msg;
  circular_buffer<uint8_t> circle(DS_RX_BUFFER_SIZE);

  //    Request the com port from the comm manager
  if (!OpenComPortPhysical(m_PortName, m_baud)) {
    wxString msg(_T("NMEA input device open failed: "));
    msg.Append(m_PortName);
    ThreadMessage(msg);
    // goto thread_exit; // This means we will not be trying to connect = The
    // device must be connected when the thread is created. Does not seem to be
    // needed/what we want as the reconnection logic is able to pick it up
    // whenever it actually appears (Of course given it appears with the
    // expected device name).
  }
  else {
    SetGatewayOperationMode();
  }

  m_pParentDriver->SetSecThreadActive();  // I am alive

  //    The main loop
  static size_t retries = 0;

  bool bInMsg = false;
  bool bGotESC = false;
  bool bGotSOT = false;

  while ((not_done) && (m_pParentDriver->m_Thread_run_flag > 0)) {
    if (TestDestroy()) not_done = false;  // smooth exit

    uint8_t next_byte = 0;
    int newdata = -1;
    uint8_t rdata[2000];

    if (m_serial.isOpen()) {
      try {
        newdata = m_serial.read(rdata, 200);
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
      if (OpenComPortPhysical(m_PortName, m_baud)){
        SetGatewayOperationMode();
        retries = 0;
      }
      else if (retries < 10)
        retries++;
    }

    if (newdata > 0) {
      for (int i = 0; i < newdata; i++) {
        circle.put(rdata[i]);
      }
    }

    while (!circle.empty()) {
      uint8_t next_byte = circle.get();

      if (1) {
        if (bInMsg) {
          if (bGotESC) {
            if (ESCAPE == next_byte) {
              *put_ptr++ = next_byte;
              if ((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE)
                put_ptr = rx_buffer;
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
            // Send the captured raw data vector pointer to the thread's
            // "parent"
            //  thereby releasing the thread for further data capture
            CommDriverN2KSerialEvent Nevent(wxEVT_COMMDRIVER_N2K_SERIAL, 0);
            Nevent.SetPayload(buffer);
            m_pParentDriver->AddPendingEvent(Nevent);

          } else {
            bGotESC = (next_byte == ESCAPE);

            if (!bGotESC) {
              *put_ptr++ = next_byte;
              if ((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE)
                put_ptr = rx_buffer;
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
              if ((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE)
                put_ptr = rx_buffer;
            }
          }
        }
      }  // if newdata > 0
    }    // while

    //      Check for any pending output message
    bool b_qdata = !out_que.empty();

    while (b_qdata) {
      //  Take a copy of message
      std::vector<unsigned char> qmsg = out_que.front();
      out_que.pop();

      if (static_cast<size_t>(-1) == WriteComPortPhysical(qmsg) &&
          10 < retries++) {
        // We failed to write the port 10 times, let's close the port so that
        // the reconnection logic kicks in and tries to fix our connection.
        retries = 0;
        CloseComPortPhysical();
      }

      b_qdata = !out_que.empty();
    }  // while b_qdata
  }  // while ((not_done)

  // thread_exit:
  CloseComPortPhysical();
  m_pParentDriver->SetSecThreadInActive();  // I am dead
  m_pParentDriver->m_Thread_run_flag = -1;

  return 0;
}

#endif  //  wxmsw Entry()

#endif  // Android


//*****************************************************************************
// Actisense Format:
// <10><02><93><length (1)><priority (1)><PGN (3)><destination (1)><source (1)><time (4)><len (1)><data (len)><CRC (1)><10><03>
#define MaxActisenseMsgBuf 400
#define MsgTypeN2kTX 0x94

void AddByteEscapedToBuf(unsigned char byteToAdd, uint8_t &idx, unsigned char *buf, int &byteSum);

std::vector<unsigned char> BufferToActisenseFormat( tN2kMsg &msg){
  unsigned long _PGN=msg.PGN;
  uint8_t msgIdx=0;
  int byteSum = 0;
  uint8_t CheckSum;
  unsigned char ActisenseMsgBuf[MaxActisenseMsgBuf];


  ActisenseMsgBuf[msgIdx++]=ESCAPE;
  ActisenseMsgBuf[msgIdx++]=STARTOFTEXT;
  AddByteEscapedToBuf(MsgTypeN2kTX,msgIdx,ActisenseMsgBuf,byteSum);
  AddByteEscapedToBuf(msg.DataLen+6,msgIdx,ActisenseMsgBuf,byteSum); //length does not include escaped chars

  AddByteEscapedToBuf(msg.Priority,msgIdx,ActisenseMsgBuf,byteSum);
  AddByteEscapedToBuf(_PGN & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _PGN>>=8;
  AddByteEscapedToBuf(_PGN & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _PGN>>=8;
  AddByteEscapedToBuf(_PGN & 0xff,msgIdx,ActisenseMsgBuf,byteSum);
  AddByteEscapedToBuf(msg.Destination,msgIdx,ActisenseMsgBuf,byteSum);

#if 0
  // For TX through Actisense compatible gateway, we skip "source" byte and msg time fields
  // Source
  AddByteEscapedToBuf(msg.Source,msgIdx,ActisenseMsgBuf,byteSum);
  // Time
  int _MsgTime = 0;
  AddByteEscapedToBuf(_MsgTime & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _MsgTime>>=8;
  AddByteEscapedToBuf(_MsgTime & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _MsgTime>>=8;
  AddByteEscapedToBuf(_MsgTime & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _MsgTime>>=8;
  AddByteEscapedToBuf(_MsgTime & 0xff,msgIdx,ActisenseMsgBuf,byteSum);

#endif

  AddByteEscapedToBuf(msg.DataLen,msgIdx,ActisenseMsgBuf,byteSum);

  for (int i = 0; i < msg.DataLen; i++)
    AddByteEscapedToBuf(msg.Data[i],msgIdx,ActisenseMsgBuf,byteSum);
  byteSum %= 256;

  CheckSum = (uint8_t)((byteSum == 0) ? 0 : (256 - byteSum));
  ActisenseMsgBuf[msgIdx++]=CheckSum;
  if (CheckSum==ESCAPE) ActisenseMsgBuf[msgIdx++]=CheckSum;

  ActisenseMsgBuf[msgIdx++] = ESCAPE;
  ActisenseMsgBuf[msgIdx++] = ENDOFTEXT;

  std::vector<unsigned char> rv;
  for (unsigned int i=0 ; i < msgIdx; i++)
    rv.push_back(ActisenseMsgBuf[i]);

  return rv;
}
