/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Low-level driver for socketcan devices (linux only).
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

class CommDriverN2KSocketCanImpl;

class CommDriverN2KSocketCAN : public CommDriverN2K, public wxEvtHandler {

public:
  static std::shared_ptr<CommDriverN2KSocketCAN> Create();
  static std::shared_ptr<CommDriverN2KSocketCAN> Create(
      const ConnectionParams* params, DriverListener& listener);

  virtual ~CommDriverN2KSocketCAN();

  /** Register driver and possibly do other post-ctor steps. */
  void Activate() override;

  void SetListener(std::shared_ptr<DriverListener> l) override{};

  virtual bool Open() = 0;
  virtual void Close() = 0;

  //    Secondary thread life toggle
  //    Used to inform launching object (this) to determine if the thread can
  //    be safely called or polled, e.g. wxThread->Destroy();
  void SetSecThreadActive(void) { m_sec_thread_active = true; }
  void SetSecThreadInActive(void) { m_sec_thread_active = false; }
  bool IsSecThreadActive() const { return m_sec_thread_active; }

protected:
  CommDriverN2KSocketCAN(const ConnectionParams* params,
                         DriverListener& listener);

  int m_thread_run_flag;
  bool m_ok;
  std::string m_portstring;
  std::string m_baudrate;

  bool m_sec_thread_active;

  ConnectionParams m_params;
  DriverListener& m_listener;
};

#endif  // guard
