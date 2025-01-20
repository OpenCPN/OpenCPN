/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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
 * \file
 * Pure C++17 periodic timer
 */

#ifndef _PERIODIC_TIMER_H
#define _PERIODIC_TIMER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

/**
 * Continuously run Notify with given interval. Runs from being constructed
 * until Stop() or going out of scope.
 *
 * To use, derive a subclass and override the Notify() method.
 *
 * Similar to wxTimer when used in continuous mode. However, it does not
 * depend on wxEventHandler and also has better destructor semantics.
 */
class PeriodicTimer {
public:
  PeriodicTimer(std::chrono::milliseconds interval);

  virtual ~PeriodicTimer();

  void Stop();

protected:
  virtual void Notify() = 0;

private:
  const std::chrono::milliseconds m_interval;
  std::mutex m_mutex;
  std::condition_variable m_cond_var;
  std::atomic<int> m_run_sts;  // 1 == running, 0 == request stop, -1 == stopped
  std::thread m_thread;

  void Worker();
};

#endif  // PERIODIC_TIMER_H
