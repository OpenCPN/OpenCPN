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

/** \file wait_continue.h Basic synchronization primitive */

#ifndef WAIT_COND__
#define WAIT_COND__

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

class WaitContinue {
public:
  /** Default constructor, neither copyable nor assignable. */
  WaitContinue() : is_waiting(false) {}

  WaitContinue& operator=(const WaitContinue) = delete;
  WaitContinue(const WaitContinue&) = delete;

  /** Release any threads blocked by Wait(). */
  void Continue() {
    {
      std::unique_lock lock(m_mutex);
      is_waiting = false;
    }
    cv.notify_all();
  }

  /** Blocking wait for next Continue() with optional timeout. */
  bool Wait(std::chrono::milliseconds timeout = 0s) {
    std::unique_lock lock(m_mutex);
    is_waiting = true;
    auto t = std::chrono::high_resolution_clock::now() + timeout;
    auto result = cv.wait_until(lock, t, [&] { return !is_waiting; });
    return true;
  }

  /** Blocking wait for next Continue() or timeout (milliseconds)*/
  void Wait(unsigned timeout_ms) {
    Wait(std::chrono::milliseconds(timeout_ms));
  }

private:
  bool is_waiting;
  std::mutex m_mutex;
  std::condition_variable cv;
};

#endif  //  WAIT_COND__
