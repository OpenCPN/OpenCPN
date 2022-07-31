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

#ifndef _COMMDRIVERN2KSERIAL_H
#define _COMMDRIVERN2KSERIAL_H

#include <wx/thread.h>

#include "comm_drv_n2k.h"
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

class commDriverN2KSerialThread;    // fwd
class commDriverN2KSerialEvent;

class commDriverN2KSerial : public commDriverN2K, public wxEvtHandler {
public:
  commDriverN2KSerial();
  commDriverN2KSerial(const ConnectionParams *params);

  virtual ~commDriverN2KSerial();

  void set_listener(DriverListener& listener);

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
