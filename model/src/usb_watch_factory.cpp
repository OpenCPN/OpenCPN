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

/** \file usb_watch_factory.cpp UsbWatchDaemon factory */

#if defined(__linux__) && !defined(__ANDROID__)
#include "model/linux_usb_watch.h"
#include "model/sys_events.h"

UsbWatchDaemon& UsbWatchDaemon::GetInstance() {
  static LinuxUsbWatchDaemon instance(SystemEvents::GetInstance());
  return instance;
}

#elif defined(_WIN32)
#include "model/win_usb_watch.h"
#include "model/sys_events.h"

UsbWatchDaemon& UsbWatchDaemon::GetInstance() {
  static WinUsbWatchDaemon instance(SystemEvents::GetInstance());
  return instance;
}


#else
#include "model/usb_watch_daemon.h"

UsbWatchDaemon& UsbWatchDaemon::GetInstance() {
  static DummyWatchDaemon instance;
  return instance;
}
#endif
