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

#include <chrono>
#include <memory>
#include <string>

#include "model/comm_drv_n2k.h"
#include "model/comm_drv_stats.h"
#include "model/conn_params.h"

class CommDriverN2KSocketCAN : public CommDriverN2K,
                               public DriverStatsProvider {
public:
  static std::unique_ptr<CommDriverN2KSocketCAN> Create(
      const ConnectionParams* params, DriverListener& listener);

  virtual ~CommDriverN2KSocketCAN();

  void SetListener(DriverListener& l) override { m_listener = l; }

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) = 0;  // override;

  virtual bool Open() = 0;
  virtual void Close() = 0;

  void UpdateAttrCanAddress();
  DriverStats GetDriverStats() const override { return m_driver_stats; }
  void SetDriverStats(DriverStats _stats) { m_driver_stats = _stats; }

protected:
  CommDriverN2KSocketCAN(const ConnectionParams* params,
                         DriverListener& listener);
  ConnectionParams m_params;
  DriverListener& m_listener;
  StatsTimer m_stats_timer;

private:
  bool m_ok;
  std::string m_portstring;
  std::string m_baudrate;
  DriverStats m_driver_stats;
};

#endif  // guard
