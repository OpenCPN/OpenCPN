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

#ifndef _COMMDRIVERN2KSERIAL_H__
#define _COMMDRIVERN2KSERIAL_H__

#include <wx/thread.h>
//#include <wx/string.h>
//#include <wx/event.h>
//#include <wx/arrstr.h>

#include <mutex>  // std::mutex
#include <queue>  // std::queue

#include "commdriverN2K.h"
#include "ConnectionParams.h"

#ifndef __OCPN__ANDROID__
#include "serial/serial.h"
#endif

#define MAX_OUT_QUEUE_MESSAGE_LENGTH 100

#define ESCAPE 0x10
#define STARTOFTEXT 0x02
#define ENDOFTEXT 0x03

#define MsgTypeN2kData 0x93
#define MsgTypeN2kRequest 0x94

class commDriverN2KSerial;    // fwd

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

  const T &front() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queque.front();
  }

  void push(const T &value) {
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

class commDriverN2KSerialEvent;     //fwd

class commDriverN2KSerialThread : public wxThread {
public:
  commDriverN2KSerialThread(commDriverN2KSerial *Launcher,
                             const wxString &PortName,
                             const wxString &strBaudRate);

  ~commDriverN2KSerialThread(void);
  void *Entry();
  bool SetOutMsg(const wxString &msg);
  void OnExit(void);

private:
#ifndef __OCPN__ANDROID__
  serial::Serial m_serial;
#endif
  void ThreadMessage(const wxString &msg);
  bool OpenComPortPhysical(const wxString &com_name, int baud_rate);
  void CloseComPortPhysical();
  size_t WriteComPortPhysical(char *msg);
  size_t WriteComPortPhysical(const wxString &string);

  commDriverN2KSerial *m_pParentDriver;
  wxString m_PortName;
  wxString m_FullPortName;

  unsigned char *put_ptr;
  unsigned char *tak_ptr;

   unsigned char *rx_buffer;

  int m_baud;
  int m_n_timeout;

  n2k_atomic_queue<char *> out_que;

#ifdef __WXMSW__
  HANDLE m_hSerialComm;
  bool m_nl_found;
#endif
};






class commDriverN2KSerial : public commDriverN2K, public wxEvtHandler {

public:
  commDriverN2KSerial();
  commDriverN2KSerial(const ConnectionParams *params);

  virtual ~commDriverN2KSerial();

  void set_listener(DriverListener* listener);

  bool Open();

  int m_Thread_run_flag;

  //    Secondary thread life toggle
  //    Used to inform launching object (this) to determine if the thread can
  //    be safely called or polled, e.g. wxThread->Destroy();
  void SetSecThreadActive(void) { m_bsec_thread_active = true; }
  void SetSecThreadInActive(void) { m_bsec_thread_active = false; }
  bool IsSecThreadActive() const { return m_bsec_thread_active; }

  void SetSecondaryThread(commDriverN2KSerialThread *secondary_Thread) {
    m_pSecondary_Thread = secondary_Thread;
  }
  commDriverN2KSerialThread *GetSecondaryThread() {
    return m_pSecondary_Thread;
  }
  void SetThreadRunFlag(int run) { m_Thread_run_flag = run; }

  void handle_N2K_SERIAL_RAW( commDriverN2KSerialEvent &event );

private:
  bool m_bok;
  std::string m_portstring;
  std::string m_BaudRate;
  int m_handshake;

  commDriverN2KSerialThread *m_pSecondary_Thread;
  bool m_bsec_thread_active;

  ConnectionParams m_params;

};


#endif    //guard
