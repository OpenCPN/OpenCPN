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

#include <atomic>

#include <wx/thread.h>

#include "config.h"
#include "comm_drv_n2k.h"
#include "conn_params.h"

#ifndef __ANDROID__
#include "serial/serial.h"
#endif

#define OUT_QUEUE_LENGTH                20
#define MAX_OUT_QUEUE_MESSAGE_LENGTH    200

#define ESCAPE 0x10
#define STARTOFTEXT 0x02
#define ENDOFTEXT 0x03

#define MsgTypeN2kData 0x93
#define MsgTypeN2kRequest 0x94

class CommDriverN2KSerialThread;  // fwd
class CommDriverN2KSerialEvent;

class CommDriverN2KSerial : public CommDriverN2K, public wxEvtHandler {
public:
  CommDriverN2KSerial();
  CommDriverN2KSerial(const ConnectionParams* params, DriverListener& listener);

  virtual ~CommDriverN2KSerial();

  /** Register driver and possibly do other post-ctor steps. */
  void Activate() override;

  void SetListener(DriverListener& l) override{};

  bool Open();
  void Close();

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                    std::shared_ptr<const NavAddr> addr) override;

  int SetTXPGN(int pgn) override;

  //    Secondary thread life toggle
  //    Used to inform launching object (this) to determine if the thread can
  //    be safely called or polled, e.g. wxThread->Destroy();
  void SetSecThreadActive(void) { m_bsec_thread_active = true; }
  void SetSecThreadInActive(void) { m_bsec_thread_active = false; }
  bool IsSecThreadActive() const { return m_bsec_thread_active; }

  void SetSecondaryThread(CommDriverN2KSerialThread* secondary_Thread) {
    m_pSecondary_Thread = secondary_Thread;
  }
  CommDriverN2KSerialThread* GetSecondaryThread() {
    return m_pSecondary_Thread;
  }
  void SetThreadRunFlag(int run) { m_Thread_run_flag = run; }

  void handle_N2K_SERIAL_RAW(CommDriverN2KSerialEvent& event);

  std::atomic_int m_Thread_run_flag;

private:
  void ProcessManagementPacket(std::vector<unsigned char> *payload);
  int SendMgmtMsg( unsigned char *string, size_t string_size,
                   unsigned char cmd_code,
                   int timeout_msec, bool *response_flag);

  bool m_bok;
  std::string m_portstring;
  std::string m_BaudRate;
  int m_handshake;

  CommDriverN2KSerialThread* m_pSecondary_Thread;
  bool m_bsec_thread_active;

  ConnectionParams m_params;
  DriverListener& m_listener;

  bool m_bmg47_resp;
  bool m_bmg01_resp;
  bool m_bmg4B_resp;
  bool m_bmg41_resp;
  bool m_bmg42_resp;

  std::string m_device_common_name;
  uint64_t NAME;
  int m_manufacturers_code;
  bool m_got_mfg_code;

};

#endif  // guard
