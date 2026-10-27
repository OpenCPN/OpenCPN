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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Pure C++17 periodic timer
 */

#ifndef _PERIODIC_TIMER_H
#define _PERIODIC_TIMER_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

/**
 * Continuously run Notify with given interval after Start(), until Stop().
 *
 * To use, derive a subclass and override Notify(). Call Start() only after the
 * subclass and everything Notify() uses have been initialized. Call Stop()
 * before destroying those resources, including from the subclass destructor.
 *
 * Similar to wxTimer when used in continuous mode. However, it does not
 * depend on wxEventHandler. Start() and Stop() must not be called from
 * Notify(), or while holding a lock needed by Notify().
 */
class PeriodicTimer {
public:
  PeriodicTimer(std::chrono::milliseconds interval);

  virtual ~PeriodicTimer();

  /** Start the worker, or do nothing if already running. Can restart after
   * Stop. */
  void Start();

  /** Stop and join the worker, waiting for any running Notify(). Idempotent. */
  void Stop();

protected:
  virtual void Notify() = 0;

private:
  const std::chrono::milliseconds m_interval;
  // Serialize Start/Stop and access to m_thread, including concurrent Stop
  // calls.
  std::mutex m_thread_mutex;
  std::mutex m_mutex;
  std::condition_variable m_cond_var;
  bool m_stop_requested = true;  // Guarded by m_mutex.
  std::thread m_thread;

  void Worker();
};

#endif  // PERIODIC_TIMER_H
