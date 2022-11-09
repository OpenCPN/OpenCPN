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

#include <memory>
#include <string>

#include "comm_drv_n2k.h"
#include "conn_params.h"

class CommDriverN2KSocketCAN : public CommDriverN2K, public wxEvtHandler {

public:
  static std::shared_ptr<CommDriverN2KSocketCAN> Create(
      const ConnectionParams* params, DriverListener& listener);

  virtual ~CommDriverN2KSocketCAN();

  /** Register driver and possibly do other post-ctor steps. */
  void Activate() override;

  void SetListener(std::shared_ptr<DriverListener> l) override{};

  virtual bool Open() = 0;
  virtual void Close() = 0;

protected:
  CommDriverN2KSocketCAN(const ConnectionParams* params,
                         DriverListener& listener);
  ConnectionParams m_params;
  DriverListener& m_listener;

private:
  bool m_ok;
  std::string m_portstring;
  std::string m_baudrate;
};

#endif  // guard
