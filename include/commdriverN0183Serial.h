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

#ifndef _COMMDRIVERN0183SERIAL_H__
#define _COMMDRIVERN0183SERIAL_H__

#include <wx/thread.h>
//#include <wx/string.h>
//#include <wx/event.h>
//#include <wx/arrstr.h>

#include <mutex>  // std::mutex
#include <queue>  // std::queue

#include "commdriverN0183.h"
#include "ConnectionParams.h"

#ifndef __OCPN__ANDROID__
#include "serial/serial.h"
#endif

#define MAX_OUT_QUEUE_MESSAGE_LENGTH 100

class commDriverN0183Serial;    // fwd

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

class commDriverN0183SerialEvent;   //fwd


class commDriverN0183SerialThread : public wxThread {
public:
  commDriverN0183SerialThread(commDriverN0183Serial *Launcher,
                             const wxString &PortName,
                             const wxString &strBaudRate);

  ~commDriverN0183SerialThread(void);
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

  commDriverN0183Serial *m_pParentDriver;
  wxString m_PortName;
  wxString m_FullPortName;

  unsigned char *put_ptr;
  unsigned char *tak_ptr;

  unsigned char *rx_buffer;

  int m_baud;
  int m_n_timeout;

  n0183_atomic_queue<char *> out_que;

#ifdef __WXMSW__
  HANDLE m_hSerialComm;
  bool m_nl_found;
#endif
};






class commDriverN0183Serial : public commDriverN0183 {

public:
  commDriverN0183Serial();
  commDriverN0183Serial(const ConnectionParams *params);

  virtual ~commDriverN0183Serial();

  bool Open();

  int m_Thread_run_flag;

  //    Secondary thread life toggle
  //    Used to inform launching object (this) to determine if the thread can
  //    be safely called or polled, e.g. wxThread->Destroy();
  void SetSecThreadActive(void) { m_bsec_thread_active = true; }
  void SetSecThreadInActive(void) { m_bsec_thread_active = false; }
  bool IsSecThreadActive() const { return m_bsec_thread_active; }

  void SetSecondaryThread(commDriverN0183SerialThread *secondary_Thread) {
    m_pSecondary_Thread = secondary_Thread;
  }
  commDriverN0183SerialThread *GetSecondaryThread() {
    return m_pSecondary_Thread;
  }
  void SetThreadRunFlag(int run) { m_Thread_run_flag = run; }

  wxEvtHandler m_EventHandler;

private:
  bool m_bok;
  std::string m_portstring;
  std::string m_BaudRate;
  int m_handshake;

  commDriverN0183SerialThread *m_pSecondary_Thread;
  bool m_bsec_thread_active;

  ConnectionParams m_params;

  void handle_N0183_MSG( commDriverN0183SerialEvent &event );

};


#endif    //guard
