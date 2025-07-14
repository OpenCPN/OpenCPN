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
 * Implement periodic_timer.h
 */

#include "model/periodic_timer.h"

PeriodicTimer::PeriodicTimer(std::chrono::milliseconds interval)
    : m_interval(interval),
      m_run_sts(1),
      m_thread(std::thread([&] { Worker(); })) {}

PeriodicTimer::~PeriodicTimer() {
  if (m_run_sts == 1) {
    Stop();
  } else if (m_run_sts == 0) {
    // ongoing stop() operation, wait until completed
    std::unique_lock lock(m_mutex);
    m_cond_var.wait_for(lock, m_interval, [&] { return m_run_sts < 0; });
  }
  if (m_thread.joinable()) m_thread.join();
}

void PeriodicTimer::Stop() {
  m_run_sts = 0;
  m_cond_var.notify_all();
  std::unique_lock lock(m_mutex);
  m_cond_var.wait_for(lock, m_interval, [&] { return m_run_sts < 0; });
  lock.unlock();
}

void PeriodicTimer::Worker() {
  while (m_run_sts > 0) {
    std::unique_lock lock(m_mutex);
    m_cond_var.wait_for(lock, m_interval, [&] { return m_run_sts <= 0; });
    lock.unlock();
    if (m_run_sts > 0) Notify();
  }
  m_run_sts = -1;
  m_cond_var.notify_all();
}
