/**************************************************************************
 *   Copyright (C) 2024 Alec Leamas                                        *
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

/** \file win_usb_watch.h Windows specific hardware events interface */

#ifndef _WIN32
#error "This file can only be compiled on windows. "
#endif

#include "model/usb_watch_daemon.h"

#include <windows.h>

/**
 * Listen to OS signals reflecting for example suspend/resume,
 * new USB devicesbeing plugged in, etc; update EventVars in SysEvents
 * accordingly
 */
class WinUsbWatchDaemon : public UsbWatchDaemon {
public:
  WinUsbWatchDaemon(SystemEvents& se) : UsbWatchDaemon(se), m_frame(0) {}
  virtual ~WinUsbWatchDaemon() = default;

  void Start();
  void Stop();

private:
  wxFrame* m_frame;
};
