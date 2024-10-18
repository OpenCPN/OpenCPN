/***************************************************************************
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

/** \file  thread_ctrl.cpp Implement thread_ctrl.h. */

#include "model/thread_ctrl.h"

void ThreadCtrl::RequestStop() {
  std::lock_guard lock(m_mutex);
  m_keep_going = 0;
}

void ThreadCtrl::WaitUntilStopped() {
  std::unique_lock lock(m_mutex);
  m_cv.wait(lock, [&] { return m_keep_going < 0; });
}

bool ThreadCtrl::WaitUntilStopped(std::chrono::duration<int> timeout) {
  std::unique_lock lock(m_mutex);
  m_cv.wait_for(lock, timeout, [&] { return m_keep_going < 0; });
  return m_keep_going < 0;
}

bool ThreadCtrl::WaitUntilStopped(std::chrono::duration<int> timeout,
                                  std::chrono::milliseconds& elapsed) {
  using namespace std::chrono;

  auto start = steady_clock::now();
  std::unique_lock lock(m_mutex);
  m_cv.wait_for(lock, timeout, [&] { return m_keep_going < 0; });
  auto end = steady_clock::now();
  elapsed = duration_cast<milliseconds>(end - start);
  return m_keep_going < 0;
}

bool ThreadCtrl::KeepGoing() const {
  std::lock_guard lock(m_mutex);
  return m_keep_going > 0;
}

void ThreadCtrl::SignalExit() {
  std::unique_lock lock(m_mutex);
  m_keep_going = -1;
  lock.unlock();
  m_cv.notify_one();
}
