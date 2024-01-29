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

/** \file linux_usb_watch.h Linux specific hardware events DBus interface */

#ifndef __linux__
#error "This file can only be compiled on linux. "
#endif

#include "model/usb_watch_daemon.h"

#include <gio/gio.h>

/**
 * Listen to DBus system bus signals reflecting for example suspend/resume,
 * new USB devicesbeing plugged in, etc; update EventVars in SysEvents
 * accordingly
 */
class LinuxUsbWatchDaemon : public UsbWatchDaemon {
public:
  LinuxUsbWatchDaemon(SystemEvents& se) : UsbWatchDaemon(se) {}
  ~LinuxUsbWatchDaemon();

  void Start();
  void Stop();

private:
  void DoStart();

  GDBusConnection* m_conn;
  GMainLoop* m_main_loop;
  GMainContext* m_worker_context;
  std::thread m_thread;
};
