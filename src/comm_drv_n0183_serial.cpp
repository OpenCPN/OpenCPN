/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_n0183_serial.h -- serial Nmea 0183 driver.
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

#include <mutex>  // std::mutex
#include <queue>  // std::queue
#include <vector>

#include <wx/event.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/thread.h>
#include <wx/utils.h>

#include "comm_drv_n0183_serial.h"
#include "comm_navmsg_bus.h"
#include "comm_drv_registry.h"

#ifndef __ANDROID__
#include "serial/serial.h"
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

typedef enum DS_ENUM_BUFFER_STATE {
  DS_RX_BUFFER_EMPTY,
  DS_RX_BUFFER_FULL
} _DS_ENUM_BUFFER_STATE;

class CommDriverN0183Serial;  // fwd

#define MAX_OUT_QUEUE_MESSAGE_LENGTH 100

template <typename T>
class n0183_atomic_queue {
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

#define OUT_QUEUE_LENGTH                20
#define MAX_OUT_QUEUE_MESSAGE_LENGTH    100

class CommDriverN0183SerialEvent;  // fwd

class CommDriverN0183SerialThread : public wxThread {
public:
  CommDriverN0183SerialThread(CommDriverN0183Serial* Launcher,
                              const wxString& PortName,
                              const wxString& strBaudRate);

  ~CommDriverN0183SerialThread(void);
  void* Entry();
  bool SetOutMsg(const wxString& msg);
  void OnExit(void);

private:
#ifndef __ANDROID__
  serial::Serial m_serial;
#endif
  void ThreadMessage(const wxString& msg);
  bool OpenComPortPhysical(const wxString& com_name, int baud_rate);
  void CloseComPortPhysical();
  size_t WriteComPortPhysical(char* msg);
  size_t WriteComPortPhysical(const wxString& string);

  CommDriverN0183Serial* m_pParentDriver;
  wxString m_PortName;
  wxString m_FullPortName;

  int m_baud;

  n0183_atomic_queue<char*> out_que;

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

class CommDriverN0183SerialEvent;
wxDECLARE_EVENT(wxEVT_COMMDRIVER_N0183_SERIAL, CommDriverN0183SerialEvent);

class CommDriverN0183SerialEvent : public wxEvent {
public:
  CommDriverN0183SerialEvent(
      wxEventType commandType = wxEVT_COMMDRIVER_N0183_SERIAL, int id = 0)
      : wxEvent(id, commandType){};
  ~CommDriverN0183SerialEvent(){};

  // accessors
  void SetPayload(std::shared_ptr<std::vector<unsigned char>> data) {
    m_payload = data;
  }
  std::shared_ptr<std::vector<unsigned char>> GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const {
    CommDriverN0183SerialEvent* newevent =
        new CommDriverN0183SerialEvent(*this);
    newevent->m_payload = this->m_payload;
    return newevent;
  };

private:
  std::shared_ptr<std::vector<unsigned char>> m_payload;
};

//========================================================================
/*    commdriverN0183Serial implementation
 * */
wxDEFINE_EVENT(wxEVT_COMMDRIVER_N0183_SERIAL, CommDriverN0183SerialEvent);

CommDriverN0183Serial::CommDriverN0183Serial(const ConnectionParams* params,
                                             DriverListener& listener)
    : CommDriverN0183(NavAddr::Bus::N0183,
                      ((ConnectionParams*)params)->GetStrippedDSPort()),
      m_Thread_run_flag(-1),
      m_bok(false),
      m_portstring(params->GetDSPort()),
      m_pSecondary_Thread(NULL),
      m_params(*params),
      m_listener(listener) {
  m_BaudRate = wxString::Format("%i", params->Baudrate), SetSecThreadInActive();

  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_N0183_SERIAL, &CommDriverN0183Serial::handle_N0183_MSG,
       this);

  Open();
}

CommDriverN0183Serial::~CommDriverN0183Serial() { Close(); }

bool CommDriverN0183Serial::Open() {
  wxString comx;
  comx = m_params.GetDSPort().AfterFirst(':');  // strip "Serial:"

  comx =
      comx.BeforeFirst(' ');  // strip off any description provided by Windows

  //    Kick off the  RX thread
  SetSecondaryThread(new CommDriverN0183SerialThread(this, comx, m_BaudRate));
  SetThreadRunFlag(1);
  GetSecondaryThread()->Run();

  return true;
}

void CommDriverN0183Serial::Close() {
  wxLogMessage(
      wxString::Format(_T("Closing NMEA Driver %s"), m_portstring.c_str()));

  // FIXME (dave)
  // If port is opened, and then closed immediately,
  // the secondary thread may not stop quickly enough.
  // It can then crash trying to send an event to its "parent".

  Unbind(wxEVT_COMMDRIVER_N0183_SERIAL, &CommDriverN0183Serial::handle_N0183_MSG,
       this);

  //    Kill off the Secondary RX Thread if alive
  if (m_pSecondary_Thread) {
    m_pSecondary_Thread->Delete();

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

  //  FIXME Kill off the Garmin handler, if alive
  //  if (m_GarminHandler) {
  //   m_GarminHandler->Close();
  //   delete m_GarminHandler;
  //  }
}

void CommDriverN0183Serial::Activate() {
  CommDriverRegistry::getInstance().Activate(shared_from_this());
  // TODO: Read input data.
}

bool CommDriverN0183Serial::SendMessage(std::shared_ptr<const NavMsg> msg,
                                        std::shared_ptr<const NavAddr> addr) {

  auto msg_0183 = std::dynamic_pointer_cast<const Nmea0183Msg>(msg);
  wxString sentence(msg_0183->payload.c_str());

#ifdef __ANDROID__
    wxString payload = sentence;
    if( !sentence.EndsWith(_T("\r\n")) )
        payload += _T("\r\n");


    wxString port = m_params.GetStrippedDSPort(); //GetPort().AfterFirst(':');
    androidWriteSerial( port, payload );
    return;
#endif
    if( GetSecondaryThread() ) {
        if( IsSecThreadActive() )
        {
            int retry = 10;
            while( retry ) {
                if( GetSecondaryThread()->SetOutMsg( sentence ))
                    return true;
                else
                    retry--;
            }
            return false;   // could not send after several tries....
        }
        else
            return false;
    }
    return true;
}



void CommDriverN0183Serial::handle_N0183_MSG(
    CommDriverN0183SerialEvent& event) {
  auto p = event.GetPayload();
  std::vector<unsigned char>* payload = p.get();

  // Extract the NMEA0183 sentence
  std::string full_sentence = std::string(payload->begin(), payload->end());

  if ((full_sentence[0] == '$') || (full_sentence[0] == '!')) {  // Sanity check
    std::string identifier;
    // We notify based on full message, including the Talker ID
    identifier = full_sentence.substr(1, 5);

    // notify message listener and also "ALL" N0183 messages, to support plugin
    // API using original talker id
    auto msg = std::make_shared<const Nmea0183Msg>(identifier, full_sentence,
                                                   GetAddress());
    auto msg_all = std::make_shared<const Nmea0183Msg>(*msg, "ALL");

    if (m_params.SentencePassesFilter(full_sentence, FILTER_INPUT))
      m_listener.Notify(std::move(msg));

    m_listener.Notify(std::move(msg_all));
  }
}

#ifndef __ANDROID__

#define DS_RX_BUFFER_SIZE 4096

CommDriverN0183SerialThread::CommDriverN0183SerialThread(
    CommDriverN0183Serial* Launcher, const wxString& PortName,
    const wxString& strBaudRate) {
  m_pParentDriver = Launcher;  // This thread's immediate "parent"

  m_PortName = PortName;
  m_FullPortName = _T("Serial:") + PortName;

  m_baud = 4800;  // default
  long lbaud;
  if (strBaudRate.ToLong(&lbaud)) m_baud = (int)lbaud;

  Create();
}

CommDriverN0183SerialThread::~CommDriverN0183SerialThread(void) {}

void CommDriverN0183SerialThread::OnExit(void) {}

bool CommDriverN0183SerialThread::OpenComPortPhysical(const wxString& com_name,
                                                      int baud_rate) {
  try {
    m_serial.setPort(com_name.ToStdString());
    m_serial.setBaudrate(baud_rate);
    m_serial.open();
    m_serial.setTimeout(250, 250, 0, 250, 0);
  } catch (std::exception& e) {
    //      std::cerr << "Unhandled Exception while opening serial port: " <<
    //      e.what() << std::endl;
  }
  return m_serial.isOpen();
}

void CommDriverN0183SerialThread::CloseComPortPhysical() {
  try {
    m_serial.close();
  } catch (std::exception& e) {
    //      std::cerr << "Unhandled Exception while closing serial port: " <<
    //      e.what() << std::endl;
  }
}

bool CommDriverN0183SerialThread::SetOutMsg(const wxString &msg)
{
  if(out_que.size() < OUT_QUEUE_LENGTH){
    wxCharBuffer buf = msg.ToUTF8();
    if(buf.data()){
      char *qmsg = (char *)malloc(strlen(buf.data()) +1);
      strcpy(qmsg, buf.data());
      out_que.push(qmsg);
      return true;
    }
  }

    return false;
}

void CommDriverN0183SerialThread::ThreadMessage(const wxString& msg) {
  //    Signal the main program thread
  //   OCPN_ThreadMessageEvent event(wxEVT_OCPN_THREADMSG, 0);
  //   event.SetSString(std::string(msg.mb_str()));
  //   if (gFrame) gFrame->GetEventHandler()->AddPendingEvent(event);
}

size_t CommDriverN0183SerialThread::WriteComPortPhysical(char* msg) {
  if (m_serial.isOpen()) {
    ssize_t status;
    try {
      status = m_serial.write((uint8_t*)msg, strlen(msg));
    } catch (std::exception& e) {
      //       std::cerr << "Unhandled Exception while writing to serial port: "
      //       << e.what() << std::endl;
      return -1;
    }
    return status;
  } else {
    return -1;
  }
}

void* CommDriverN0183SerialThread::Entry() {
  bool not_done = true;
  m_pParentDriver->SetSecThreadActive();  // I am alive
  int nl_found = 0;
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


  //    The main loop
  static size_t retries = 0;

  while ((not_done) && (m_pParentDriver->m_Thread_run_flag > 0)) {
    if (TestDestroy()) goto thread_exit;

    uint8_t next_byte = 0;
    unsigned int newdata = 0;
    uint8_t rdata[2000];

    if (m_serial.isOpen()) {
      try {
        newdata = m_serial.read(rdata, 200);
      } catch (std::exception& e) {
        //        std::cerr << "Serial read exception: " << e.what() <<
        //        std::endl;
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

    if (newdata > 0) {
      nl_found = 0;
      for (unsigned int i = 0; i < newdata; i++) {
        circle.put(rdata[i]);
        if (0x0a == rdata[i]) nl_found++;
      }

      //    Found a NL char, thus end of message?
      if (nl_found) {
        bool done = false;
        while (!done) {
          if (circle.empty()) {
            done = true;
            break;
          }

          //    Copy the message into a vector for tranmittal upstream
          auto buffer = std::make_shared<std::vector<unsigned char>>();
          std::vector<unsigned char>* vec = buffer.get();

          uint8_t take_byte = circle.get();
          while ((take_byte != 0x0a) && !circle.empty()) {
            vec->push_back(take_byte);
            take_byte = circle.get();
          }

          if (take_byte == 0x0a) {
            vec->push_back(take_byte);

            if (TestDestroy()) goto thread_exit;

            //    Message is ready to parse and send out
            //    Messages may be coming in as <blah blah><lf><cr>.
            //    One example device is KVH1000 heading sensor.
            //    If that happens, the first character of a new captured message
            //    will the <cr>, and we need to discard it. This is out of spec,
            //    but we should handle it anyway
            if (vec->at(0) == '\r') vec->erase(vec->begin());

            CommDriverN0183SerialEvent Nevent(wxEVT_COMMDRIVER_N0183_SERIAL, 0);
            Nevent.SetPayload(buffer);
            m_pParentDriver->AddPendingEvent(Nevent);

          } else {
            done = true;
          }
        }
      }  // if nl
    }    // if newdata > 0

    //      Check for any pending output message

    bool b_qdata = !out_que.empty();

    while (b_qdata) {
      //  Take a copy of message
      char* qmsg = out_que.front();
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
  }

thread_exit:
  CloseComPortPhysical();
  m_pParentDriver->SetSecThreadInActive();  // I am dead
  m_pParentDriver->m_Thread_run_flag = -1;

  return 0;
}

#endif  // Android
