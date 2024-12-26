/***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 by Alec Leamas                                     *
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

/**
 *  \file
 *   NMEA0183 serial driver
 */

#ifndef _COMMDRIVERN0183SERIAL_H
#define _COMMDRIVERN0183SERIAL_H

#include <atomic>
#include <string>

#include <wx/event.h>

#include "model/comm_drv_n0183.h"
#include "model/comm_out_queue.h"
#include "model/conn_params.h"
#include "model/garmin_protocol_mgr.h"
#include "model/serial_io.h"

class CommDriverN0183Serial : public CommDriverN0183, public wxEvtHandler {
public:
  CommDriverN0183Serial(const ConnectionParams* params, DriverListener& l);

  virtual ~CommDriverN0183Serial();

  bool Open();
  void Close();

  bool IsSecThreadActive() { return m_serial_io->IsRunning(); }

  bool IsGarminThreadActive();
  void StopGarminUSBIOThread(bool bPause);

  ConnectionParams GetParams() const { return m_params; }

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;

private:
  /**
   * Send a message to all listeners after applying filtering. Ends up in a
   * Notify() and can thus be used as a callback in IO threads.
   */
  void SendMessage(const std::vector<unsigned char>& msg);

  std::string m_portstring;
  unsigned m_baudrate;

  std::unique_ptr<SerialIo> m_serial_io;
  GarminProtocolHandler* m_garmin_handler;

  ConnectionParams m_params;
  DriverListener& m_listener;
};

#endif  // guard
