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

/** \file usb_watch_daemon.h Generic hardware events interface */

#ifndef USB_WATCH_DAEMON__H
#define USB_WATCH_DAEMON__H

#include <thread>

#include "model/sys_events.h"

/**
 * Listen to hardware events and notifies SystemEvents when new devices are
 * plugged in. This is platform dependent, first implementation is  Linux.
 *
 * No filtering of events is done. Listening drivers are supposed to check
 * the actual device when receiving the notification; there is no guarantee
 * regarding what device which is triggering the event.
 */
class UsbWatchDaemon {
public:
  static UsbWatchDaemon& GetInstance();

  virtual ~UsbWatchDaemon() = default;

  virtual void Start() = 0;
  virtual void Stop() = 0;

  SystemEvents& m_sys_events;

protected:
  UsbWatchDaemon(SystemEvents& sys_events) : m_sys_events(sys_events) {}
};

#if !defined(__linux__) || defined(__ANDROID__)
/** Empty placeholder, does not do anything meaningful. */
class DummyWatchDaemon : public UsbWatchDaemon {
public:
  DummyWatchDaemon() : UsbWatchDaemon(SystemEvents::GetInstance()) {}
  void Start() {}
  void Stop() {}
};
#endif

#endif  //  USB_WATCH_DAEMON__H
