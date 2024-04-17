/**************************************************************************
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

/** \file comm_drv_n0183_serial.cpp  Implement comm_drv_n0183_serial.h */

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <mutex>  // std::mutex
#include <queue>  // std::queue
#include <thread>
#include <vector>

#include <wx/event.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "config.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_navmsg_bus.h"
#include "model/comm_drv_registry.h"
#include "model/logger.h"
#include "model/sys_events.h"
#include "model/wait_continue.h"
#include "observable.h"

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

#define OUT_QUEUE_LENGTH 20
#define MAX_OUT_QUEUE_MESSAGE_LENGTH 100

wxDEFINE_EVENT(wxEVT_COMMDRIVER_N0183_SERIAL, CommDriverN0183SerialEvent);

CommDriverN0183SerialEvent::CommDriverN0183SerialEvent(wxEventType commandType,
                                                       int id = 0)
    : wxEvent(id, commandType){};

CommDriverN0183SerialEvent::~CommDriverN0183SerialEvent(){};

void CommDriverN0183SerialEvent::SetPayload(
    std::shared_ptr<std::vector<unsigned char>> data) {
  m_payload = data;
}
std::shared_ptr<std::vector<unsigned char>>
CommDriverN0183SerialEvent::GetPayload() {
  return m_payload;
}

// required for sending with wxPostEvent()
wxEvent* CommDriverN0183SerialEvent::Clone() const {
  CommDriverN0183SerialEvent* newevent = new CommDriverN0183SerialEvent(*this);
  newevent->m_payload = this->m_payload;
  return newevent;
};

#ifndef __ANDROID__
class CommDriverN0183SerialThread {
public:
  CommDriverN0183SerialThread(CommDriverN0183Serial* Launcher,
                              const wxString& PortName,
                              const wxString& strBaudRate);

  ~CommDriverN0183SerialThread(void);
  void* Entry();
  bool SetOutMsg(const wxString& msg);
  void OnExit(void);

private:
  serial::Serial m_serial;
  void ThreadMessage(const wxString& msg);
  bool OpenComPortPhysical(const wxString& com_name, int baud_rate);
  void CloseComPortPhysical();
  ssize_t WriteComPortPhysical(const std::string& msg);

  CommDriverN0183Serial* m_parent_driver;
  wxString m_port_name;
  wxString m_full_port_name;

  int m_baud;
  size_t m_send_retries;

  n0183_atomic_queue<char*> out_que;
  WaitContinue device_waiter;
  ObsListener resume_listener;
  ObsListener new_device_listener;
};
#endif

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

CommDriverN0183Serial::CommDriverN0183Serial(const ConnectionParams* params,
                                             DriverListener& listener)
    : CommDriverN0183(NavAddr::Bus::N0183,
                      ((ConnectionParams*)params)->GetStrippedDSPort()),
      m_Thread_run_flag(-1),
      m_ok(false),
      m_portstring(params->GetDSPort()),
      m_secondary_thread(NULL),
      m_params(*params),
      m_listener(listener),
      m_out_queue(std::unique_ptr<CommOutQueue>(new MeasuredCommOutQueue(12))) {
  m_baudrate = wxString::Format("%i", params->Baudrate);
  SetSecThreadInActive();
  m_garmin_handler = NULL;
  this->attributes["commPort"] = params->Port.ToStdString();

  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_N0183_SERIAL, &CommDriverN0183Serial::handle_N0183_MSG,
       this);

  Open();
}

CommDriverN0183Serial::~CommDriverN0183Serial() { Close(); }

bool CommDriverN0183Serial::Open() {
  wxString comx;
  comx = m_params.GetDSPort().AfterFirst(':');  // strip "Serial:"
  if (comx.IsEmpty()) return false;

  wxString port_uc = m_params.GetDSPort().Upper();

  if ((wxNOT_FOUND != port_uc.Find("USB")) &&
      (wxNOT_FOUND != port_uc.Find("GARMIN"))) {
    m_garmin_handler = new GarminProtocolHandler(comx, this, true);
  } else if (m_params.Garmin) {
    m_garmin_handler = new GarminProtocolHandler(comx, this, false);
  } else {
    // strip off any description provided by Windows
    comx = comx.BeforeFirst(' ');

#ifndef __ANDROID__
    //    Kick off the  RX thread
    SetSecondaryThread(new CommDriverN0183SerialThread(this, comx, m_baudrate));
    SetThreadRunFlag(1);
    std::thread t(&CommDriverN0183SerialThread::Entry, GetSecondaryThread());
    t.detach();
#else
    androidStartUSBSerial(comx, m_baudrate, this);
#endif
  }

  return true;
}

void CommDriverN0183Serial::Close() {
  wxLogMessage(
      wxString::Format("Closing NMEA Driver %s", m_portstring.c_str()));

  // FIXME (dave)
  // If port is opened, and then closed immediately,
  // the secondary thread may not stop quickly enough.
  // It can then crash trying to send an event to its "parent".

  Unbind(wxEVT_COMMDRIVER_N0183_SERIAL,
         &CommDriverN0183Serial::handle_N0183_MSG, this);

#ifndef __ANDROID__
  //    Kill off the Secondary RX Thread if alive
  if (m_secondary_thread) {
    if (m_sec_thread_active)  // Try to be sure thread object is still alive
    {
      wxLogMessage("Stopping Secondary Thread");

      m_Thread_run_flag = 0;

      int tsec = 10;
      while ((m_Thread_run_flag >= 0) && (tsec--)) wxSleep(1);

      wxString msg;
      if (m_Thread_run_flag < 0)
        msg.Printf("Stopped in %d sec.", 10 - tsec);
      else
        msg.Printf("Not Stopped after 10 sec.");
      wxLogMessage(msg);
    }

    delete m_secondary_thread;
    m_secondary_thread = NULL;
    m_sec_thread_active = false;
  }

  //  Kill off the Garmin handler, if alive
  if (m_garmin_handler) {
    m_garmin_handler->Close();
    delete m_garmin_handler;
    m_garmin_handler = NULL;
  }

#else
  wxString comx;
  comx = m_params.GetDSPort().AfterFirst(':');  // strip "Serial:"
  androidStopUSBSerial(comx);
#endif
}

bool CommDriverN0183Serial::IsGarminThreadActive() {
  if (m_garmin_handler) {
    // TODO expand for serial
#ifdef __WXMSW__
    if (m_garmin_handler->m_usb_handle != INVALID_HANDLE_VALUE)
      return true;
    else
      return false;
#endif
  }

  return false;
}

void CommDriverN0183Serial::StopGarminUSBIOThread(bool b_pause) {
  if (m_garmin_handler) {
    m_garmin_handler->StopIOThread(b_pause);
  }
}

void CommDriverN0183Serial::Activate() {
  CommDriverRegistry::GetInstance().Activate(shared_from_this());
  // TODO: Read input data.
}

bool CommDriverN0183Serial::SendMessage(std::shared_ptr<const NavMsg> msg,
                                        std::shared_ptr<const NavAddr> addr) {
  auto msg_0183 = std::dynamic_pointer_cast<const Nmea0183Msg>(msg);
  wxString sentence(msg_0183->payload.c_str());

#ifdef __ANDROID__
  wxString payload = sentence;
  if (!sentence.EndsWith("\r\n")) payload += "\r\n";

  wxString port = m_params.GetStrippedDSPort();  // GetPort().AfterFirst(':');
  androidWriteSerial(port, payload);
  return true;
#else
  if (GetSecondaryThread()) {
    if (IsSecThreadActive()) {
      int retry = 10;
      while (retry) {
        if (GetSecondaryThread()->SetOutMsg(sentence))
          return true;
        else
          retry--;
      }
      return false;  // could not send after several tries....
    } else
      return false;
  }
  return true;
#endif
}

void CommDriverN0183Serial::handle_N0183_MSG(
    CommDriverN0183SerialEvent& event) {
  // Is this an output-only port?
  // Commonly used for "Send to GPS" function
  if (m_params.IOSelect == DS_TYPE_OUTPUT) return;

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
  m_parent_driver = Launcher;  // This thread's immediate "parent"

  m_port_name = PortName;
  m_full_port_name = "Serial:" + PortName;

  m_baud = 4800;  // default
  long lbaud;
  if (strBaudRate.ToLong(&lbaud)) m_baud = (int)lbaud;
  resume_listener.Init(SystemEvents::GetInstance().evt_resume,
                       [&](ObservedEvt&) { device_waiter.Continue(); });
  new_device_listener.Init(SystemEvents::GetInstance().evt_dev_change,
                           [&](ObservedEvt&) { device_waiter.Continue(); });
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
  } catch (std::exception&) {
    //      std::cerr << "Unhandled Exception while opening serial port: " <<
    //      e.what() << std::endl;
  }
  return m_serial.isOpen();
}

void CommDriverN0183SerialThread::CloseComPortPhysical() {
  try {
    m_serial.close();
  } catch (std::exception&) {
    //      std::cerr << "Unhandled Exception while closing serial port: " <<
    //      e.what() << std::endl;
  }
}

bool CommDriverN0183SerialThread::SetOutMsg(const wxString& msg) {
  if (out_que.size() < OUT_QUEUE_LENGTH) {
    wxCharBuffer buf = msg.ToUTF8();
    if (buf.data()) {
      char* qmsg = (char*)malloc(strlen(buf.data()) + 1);
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

ssize_t CommDriverN0183SerialThread::WriteComPortPhysical(const std::string& msg) {
  if (!m_serial.isOpen())  return -1;

  auto ptr = reinterpret_cast<const uint8_t*>(msg.c_str());
  size_t written = 0;
  int tries = 0;
  while (written < msg.size()) {
    int chunk_size;
    try {
      chunk_size = m_serial.write(ptr, msg.size() - written);
    } catch (std::exception& e) {
      MESSAGE_LOG << "Exception while writing to serial port: " << e.what();
      return -1;
    }
    if (chunk_size < 0) return chunk_size;
    written += chunk_size;
    ptr += chunk_size;
    if (tries++ > 20) {
      MESSAGE_LOG << "Error writing data (output stalled?)";
      return -1;
    }
    if (written < msg.size()) std::this_thread::sleep_for(10ms);
    // FIXME (leamas) really?
  }
  return written;
}

void* CommDriverN0183SerialThread::Entry() {
  bool not_done = true;
  m_parent_driver->SetSecThreadActive();  // I am alive
  wxString msg;
  circular_buffer<uint8_t> circle(DS_RX_BUFFER_SIZE);
  std::vector<uint8_t> tmp_vec;

  //    Request the com port from the comm manager
  if (!OpenComPortPhysical(m_port_name, m_baud)) {
    wxString msg("NMEA input device open failed: ");
    msg.Append(m_port_name);
    ThreadMessage(msg);
    // goto thread_exit; // This means we will not be trying to connect = The
    // device must be connected when the thread is created. Does not seem to be
    // needed/what we want as the reconnection logic is able to pick it up
    // whenever it actually appears (Of course given it appears with the
    // expected device name).
  }

  //    The main loop
  m_send_retries = 0;

  while ((not_done) && (m_parent_driver->m_Thread_run_flag > 0)) {
    if (m_parent_driver->m_Thread_run_flag == 0) goto thread_exit;

    uint8_t next_byte = 0;
    unsigned int newdata = 0;
    uint8_t rdata[2000];

    if (m_serial.isOpen()) {
      try {
        newdata = m_serial.read(rdata, 200);
      } catch (std::exception&) {
        //        std::cerr << "Serial read exception: " << e.what() <<
        //        std::endl;
        if (10 < m_send_retries++) {
          // We timed out waiting for the next character 10 times, let's close
          // the port so that the reconnection logic kicks in and tries to fix
          // our connection.
          CloseComPortPhysical();
          m_send_retries = 0;
        }
      }
    } else {
      // Reconnection logic. Let's try to reopen the port while waiting longer
      // every time (until we simply keep trying every 2.5 seconds)
      // std::cerr << "Serial port seems closed." << std::endl;
      device_waiter.Wait(250 * m_send_retries);
      CloseComPortPhysical();
      if (OpenComPortPhysical(m_port_name, m_baud))
        m_send_retries = 0;
      else if (m_send_retries < 10)
        m_send_retries++;
    }

    if (newdata > 0) {
      for (unsigned int i = 0; i < newdata; i++) circle.put(rdata[i]);
    }

    // Process the queue until empty
    while (!circle.empty()) {
      if (m_parent_driver->m_Thread_run_flag == 0) goto thread_exit;

      uint8_t take_byte = circle.get();
      while ((take_byte != 0x0a) && !circle.empty()) {
        tmp_vec.push_back(take_byte);
        take_byte = circle.get();
      }

      if (circle.empty() && take_byte != 0x0a) {
        tmp_vec.push_back(take_byte);
        break;
      }

      if (take_byte == 0x0a) {
        tmp_vec.push_back(take_byte);

        //    Copy the message into a vector for transmittal upstream
        auto buffer = std::make_shared<std::vector<unsigned char>>();
        std::vector<unsigned char>* vec = buffer.get();

        for (size_t i = 0; i < tmp_vec.size(); i++)
          vec->push_back(tmp_vec.at(i));

        //    Message is ready to parse and send out
        //    Messages may be coming in as <blah blah><lf><cr>.
        //    One example device is KVH1000 heading sensor.
        //    If that happens, the first character of a new captured message
        //    will the <cr>, and we need to discard it. This is out of spec,
        //    but we should handle it anyway
        if (vec->at(0) == '\r') vec->erase(vec->begin());

        CommDriverN0183SerialEvent Nevent(wxEVT_COMMDRIVER_N0183_SERIAL, 0);
        Nevent.SetPayload(buffer);
        m_parent_driver->AddPendingEvent(Nevent);
        tmp_vec.clear();
      }
    }  // while

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

      if (-1 == WriteComPortPhysical(msg) && 10 < m_send_retries++) {
        // We failed to write the port 10 times, let's close the port so that
        // the reconnection logic kicks in and tries to fix our connection.
        m_send_retries = 0;
        CloseComPortPhysical();
      }

      b_qdata = !out_que.empty();
    }  // while b_qdata
  }    // while not done.

thread_exit:
  CloseComPortPhysical();
  m_parent_driver->SetSecThreadInActive();  // I am dead
  m_parent_driver->m_Thread_run_flag = -1;

  return 0;
}
#endif  //__ANDROID__
