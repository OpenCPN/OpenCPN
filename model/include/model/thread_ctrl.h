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

/**
 *  \file
 *  ThreadCtrl mixin class definition
 */

#ifndef THREAD_CTRL_H__
#define THREAD_CTRL_H__

#include <condition_variable>
#include <mutex>

/** Thread mixin providing a "stop thread"/"wait until stopped" interface. */
class ThreadCtrl {
public:
  ThreadCtrl() : m_keep_going(1) {}

  /**
   * Destroy the Thread Ctrl object.
   */
  virtual ~ThreadCtrl() = default;

  /** Return true if thread is running. */
  bool IsRunning() const { return KeepGoing(); }

  /** Request that thread stops operation. */
  virtual void RequestStop();

  /** Block until thread invokes SignalExit(). */
  void WaitUntilStopped();

  /**
   *  Block  until thread invokes SignalExit() or timeout
   *  @return false if the timeout triggered, else true.
   */
  bool WaitUntilStopped(std::chrono::duration<int> timeout);

  /**
   *  Block  until thread invokes SignalExit() or timeout
   *  @param timeout Maximum time to wait for thread  to exit.
   *  @param elapsed On exit, the time spent in method.
   *  @return false if the timeout triggered, else true.
   */
  bool WaitUntilStopped(std::chrono::duration<int> timeout,
                        std::chrono::milliseconds& elapsed);

protected:
  /** If true continue thread operation, else exit and invoke SignalExit() */
  bool KeepGoing() const;

  /** Signal that thread has exited. */
  void SignalExit();

private:
  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
  int m_keep_going;
};

#endif  //  THREAD_CTRL_H__
