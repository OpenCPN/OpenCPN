/**************************************************************************
 *   Copyright (C) 2026 by David S. Register                               *
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
 ***************************************************************************/

/**
 * \file
 *
 * Define threaded chart database classes
 */

#ifndef __CHARTDBTHREAD_H__
#define __CHARTDBTHREAD_H__

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

// #include <wx/xml/xml.h>

// #include "chartbase.h"
// #include "chartbase.h"
// #include "chartdbs.h"
// #include "chartimg.h"
#include "chartclassdescriptor.h"

class ChartTableEntryPoolThread;

#if 1
class ChartTableEntryJobTicket {
public:
  ChartTableEntryJobTicket() {
    pthread = nullptr;
    b_thread_safe = true;
    m_provider_type = 0;
  }
  ~ChartTableEntryJobTicket() {}

  bool DoJob();

  wxString m_ChartPath;
  wxString m_ChartPathUTF8;
  ChartClassDescriptor chart_desc;

  ChartTableEntryPoolThread* pthread;
  std::shared_ptr<struct ChartTableEntry> m_chart_table_entry;

  bool b_isaborted;
  bool b_thread_safe;
  int m_provider_type;
  wxString provider_class_name;
};
#endif

#if 0
class ChartTableEntryPoolThread : public wxThread {
public:
  ChartTableEntryPoolThread(ChartTableEntryJobTicket *ticket, wxEvtHandler *message_target);
  void *Entry();

  wxEvtHandler *m_pMessageTarget;
  ChartTableEntryJobTicket *m_ticket;
};
#endif

#if 1
class ChartTableEntryPoolThread : public wxThread {
public:
  ChartTableEntryPoolThread(std::shared_ptr<ChartTableEntryJobTicket> ticket,
                            wxEvtHandler* message_target);
  void* Entry();

private:
  wxEvtHandler* m_pMessageTarget = nullptr;
  std::shared_ptr<ChartTableEntryJobTicket> m_ticket;
};

#endif

#if 0
class  OCPN_ChartTableEntryThreadEvent : public wxEvent {
public:
  OCPN_ChartTableEntryThreadEvent(wxEventType commandType = wxEVT_NULL, int id = 0);
  ~OCPN_ChartTableEntryThreadEvent();

  // accessors
  void SetTicket(ChartTableEntryJobTicket *ticket) { m_ticket = ticket; }
  ChartTableEntryJobTicket *GetTicket(void) { return m_ticket; }

  // required for sending with wxPostEvent()
  wxEvent *Clone() const;

  int type;
  int nstat;
  int nstat_max;

private:
  ChartTableEntryJobTicket *m_ticket;
};
#endif

#if 1
// using TicketPtr = std::shared_ptr<ChartTableEntryJobTicket>;

class OCPN_ChartTableEntryThreadEvent : public wxEvent {
public:
  OCPN_ChartTableEntryThreadEvent(wxEventType type) : wxEvent(0, type) {}

  wxEvent* Clone() const override {
    return new OCPN_ChartTableEntryThreadEvent(*this);
  }

  void SetTicket(std::shared_ptr<ChartTableEntryJobTicket> t) {
    m_ticket = std::move(t);
  }
  std::shared_ptr<ChartTableEntryJobTicket> GetTicket() const {
    return m_ticket;
  }

private:
  std::shared_ptr<ChartTableEntryJobTicket> m_ticket;
};
#endif

// Declare the event type
wxDECLARE_EVENT(wxEVT_OCPN_CHARTTABLEENTRYTHREAD,
                OCPN_ChartTableEntryThreadEvent);

class JobQueueCTE {
public:
  JobQueueCTE() {}
  void Push(std::shared_ptr<ChartTableEntryJobTicket> job) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(job);
    m_cv.notify_one();
  }

  // Blocks until job available or shutdown requested
  bool Pop(std::shared_ptr<ChartTableEntryJobTicket>& job) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [&] { return !m_queue.empty() || m_shutdown; });

    if (m_shutdown && m_queue.empty()) return false;

    job = m_queue.front();
    m_queue.pop();
    return true;
  }

  void Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shutdown = true;
    m_cv.notify_all();
    n_workers = 0;
  }

  void AddWorker() { n_workers++; }
  int GetWorkerCount() { return n_workers; }

private:
  std::queue<std::shared_ptr<ChartTableEntryJobTicket>> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_cv;
  bool m_shutdown = false;
  int n_workers = 0;
};

class PoolWorkerThread : public wxThread {
public:
  PoolWorkerThread(JobQueueCTE& queue, wxEvtHandler* target)
      : wxThread(wxTHREAD_DETACHED), m_queue(queue), m_target(target) {}

protected:
  ExitCode Entry() override {
    std::shared_ptr<ChartTableEntryJobTicket> job;

    while (m_queue.Pop(job)) {
      if (!job->DoJob()) {
        job->b_isaborted = true;
        printf("job aborted\n");
      }

      printf("..Queue event\n");
      auto* evt =
          new OCPN_ChartTableEntryThreadEvent(wxEVT_OCPN_CHARTTABLEENTRYTHREAD);
      evt->SetTicket(job);
      wxQueueEvent(m_target, evt);
    }

    printf("thread exit\n");
    return 0;
  }

private:
  JobQueueCTE& m_queue;
  wxEvtHandler* m_target;
};

#endif
