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
 * Implement periodic_timer.h -- pure C++17 periodic timer
 */

#include "model/periodic_timer.h"

PeriodicTimer::PeriodicTimer(std::chrono::milliseconds interval)
    : m_interval(interval) {}

PeriodicTimer::~PeriodicTimer() { Stop(); }

void PeriodicTimer::Start() {
  std::lock_guard thread_lock(m_thread_mutex);
  if (m_thread.joinable()) return;
  {
    std::lock_guard lock(m_mutex);
    m_stop_requested = false;
  }
  m_thread = std::thread([this] { Worker(); });
}

void PeriodicTimer::Stop() {
  std::lock_guard thread_lock(m_thread_mutex);
  {
    std::lock_guard lock(m_mutex);
    m_stop_requested = true;
  }
  m_cond_var.notify_all();
  // Do not hold the worker's mutex while joining: it needs it to exit.
  if (m_thread.joinable()) m_thread.join();
}

void PeriodicTimer::Worker() {
  std::unique_lock lock(m_mutex);
  while (!m_cond_var.wait_for(lock, m_interval,
                              [this] { return m_stop_requested; })) {
    lock.unlock();
    Notify();
    lock.lock();
  }
}
