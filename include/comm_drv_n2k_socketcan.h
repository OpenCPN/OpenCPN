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

#ifndef _COMMDRIVERN2KSOCKETCAN_H
#define _COMMDRIVERN2KSOCKETCAN_H

#ifdef _MSC_VER
#error "This file can not be compiled on Windows."
#endif

#include <memory>
#include <string>
#include <thread>

// SocketCAN
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>

#ifndef __ANDROID__
#include "serial/serial.h"
#endif

#include "comm_drv_n2k.h"
#include "conn_params.h"

#define MAX_OUT_QUEUE_MESSAGE_LENGTH 100

#define ESCAPE 0x10
#define STARTOFTEXT 0x02
#define ENDOFTEXT 0x03

#define MsgTypeN2kData 0x93
#define MsgTypeN2kRequest 0x94

// CAN v2.0 29 bit header as used by NMEA 2000
typedef struct CanHeader {
  unsigned char priority;
  unsigned char source;
  unsigned char destination;
  int pgn;
} CanHeader;

class CommDriverN2KSocketCANThread;  // fwd
class CommDriverN2KSocketCANEvent;

class CommDriverN2KSocketCAN : public CommDriverN2K, public wxEvtHandler {

friend class CommDriverN2KSocketCANThread;

public:
  static std::shared_ptr<CommDriverN2KSocketCAN> Create() {
    return std::shared_ptr<CommDriverN2KSocketCAN>(
        new CommDriverN2KSocketCAN());
  }

  static std::shared_ptr<CommDriverN2KSocketCAN> Create(
      const ConnectionParams* params, DriverListener& listener) {
    return std::shared_ptr<CommDriverN2KSocketCAN>(
        new CommDriverN2KSocketCAN(params, listener));
  }

  virtual ~CommDriverN2KSocketCAN();

  /** Register driver and possibly do other post-ctor steps. */
  void Activate() override;

  void SetListener(std::shared_ptr<DriverListener> l) override{};

  bool Open();
  void Close();

  //    Secondary thread life toggle
  //    Used to inform launching object (this) to determine if the thread can
  //    be safely called or polled, e.g. wxThread->Destroy();
  void SetSecThreadActive(void) { m_sec_thread_active = true; }
  void SetSecThreadInActive(void) { m_sec_thread_active = false; }
  bool IsSecThreadActive() const { return m_sec_thread_active; }

  void SetSecondaryThread(CommDriverN2KSocketCANThread* thread) {
    m_secondary_thread = thread;
  }
  CommDriverN2KSocketCANThread* GetSecondaryThread() {
    return m_secondary_thread;
  }
  void SetThreadRunFlag(int run) { m_thread_run_flag = run; }

  void handle_N2K_SocketCAN_RAW(CommDriverN2KSocketCANEvent& event);

  int m_thread_run_flag;

private:
  CommDriverN2KSocketCAN();
  CommDriverN2KSocketCAN(const ConnectionParams* params,
                         DriverListener& listener);


  bool m_ok;
  std::string m_portstring;
  std::string m_baudrate;
  int m_handshake;

  CommDriverN2KSocketCANThread* m_secondary_thread;
  bool m_sec_thread_active;

  ConnectionParams m_params;
  DriverListener& m_listener;
};

#endif  // guard
