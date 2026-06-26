/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 * Purpose:  S57 Chart Object
 */

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "senc_manager.h"

#include "model/config_vars.h"

#include "s57chart.h"
#include "o_senc.h"
#include "chartbase.h"
#include "chcanv.h"
#include "s57class_registrar.h"
#include "top_frame.h"

SENCThreadManager *g_SencThreadManager;

namespace {
constexpr int kThreadWaitSeconds = 5;
}

//----------------------------------------------------------------------------------
//      SENCJobTicket Implementation
//----------------------------------------------------------------------------------
SENCJobTicket::SENCJobTicket() {
  m_chart = nullptr;
  m_SENCResult = SENC_BUILD_INACTIVE;
  m_status = THREAD_INACTIVE;
  m_completion_posted = false;
}

const wxEventType wxEVT_OCPN_BUILDSENCTHREAD = wxNewEventType();

//----------------------------------------------------------------------------------
//      OCPN_BUILDSENC_ThreadEvent Implementation
//----------------------------------------------------------------------------------
OCPN_BUILDSENC_ThreadEvent::OCPN_BUILDSENC_ThreadEvent(wxEventType commandType,
                                                       int id)
    : wxEvent(id, commandType) {
  stat = 0;
  m_ticket = nullptr;
}

OCPN_BUILDSENC_ThreadEvent::~OCPN_BUILDSENC_ThreadEvent() {}

wxEvent *OCPN_BUILDSENC_ThreadEvent::Clone() const {
  OCPN_BUILDSENC_ThreadEvent *newevent = new OCPN_BUILDSENC_ThreadEvent(*this);
  newevent->stat = this->stat;
  newevent->type = this->type;
  newevent->m_ticket = this->m_ticket;

  return newevent;
}

//----------------------------------------------------------------------------------
//      SENCThreadManager Implementation
//----------------------------------------------------------------------------------
SENCThreadManager::SENCThreadManager() : m_shutting_down(false) {
  int nCPU = wxMax(1, wxThread::GetCPUCount());
  if (g_nCPUCount > 0) nCPU = g_nCPUCount;

  if (nCPU < 1) nCPU = 1;

  m_max_jobs = wxMax(nCPU - 1, 1);

  wxLogDebug("SENC: nCPU: %d    m_max_jobs :%d\n", nCPU, m_max_jobs);

  Connect(
      wxEVT_OCPN_BUILDSENCTHREAD,
      (wxObjectEventFunction)(wxEventFunction)&SENCThreadManager::OnEvtThread);
}

SENCThreadManager::~SENCThreadManager() { ClearJobList(); }

bool SENCThreadManager::IsShuttingDown() const { return m_shutting_down; }

int SENCThreadManager::GetRunningJobCount() {
  wxCriticalSectionLocker lock(m_list_mutex);
  int nRunning = 0;
  for (SENCJobTicket *ticket : ticket_list) {
    if (ticket->m_status == THREAD_STARTED) nRunning++;
  }
  return nRunning;
}

void SENCThreadManager::UpdateAlertString() {
  int nRunning = 0;
  size_t jobCount = 0;
  {
    wxCriticalSectionLocker lock(m_list_mutex);
    jobCount = ticket_list.size();
    for (SENCJobTicket *ticket : ticket_list) {
      if (ticket->m_status == THREAD_STARTED) nRunning++;
    }
  }

  if (auto tfPtr = top_frame::Get()) {
    if (nRunning) {
      wxString count;
      count.Printf("  %ld", static_cast<long>(jobCount));
      tfPtr->SetAlertString(_("Preparing vector chart ") + count);
    } else {
      tfPtr->SetAlertString("");
    }
  }
}

void SENCThreadManager::ClearJobList() {
  m_shutting_down = true;

  wxDateTime end = wxDateTime::Now();
  end.Add(wxTimeSpan::Seconds(kThreadWaitSeconds));

  while (wxDateTime::Now() < end && GetRunningJobCount() > 0) {
    wxSleep(1);
    wxYield();
  }

  std::vector<SENCJobTicket *> tickets_to_delete;
  {
    wxCriticalSectionLocker lock(m_list_mutex);
    auto collect = [&](std::vector<SENCJobTicket *> &list) {
      for (SENCJobTicket *ticket : list) tickets_to_delete.push_back(ticket);
      list.clear();
    };
    collect(ticket_list);
    collect(completing_list);
  }

  for (SENCJobTicket *ticket : tickets_to_delete) delete ticket;
}

SENCThreadStatus SENCThreadManager::ScheduleJob(SENCJobTicket *ticket) {
  if (m_shutting_down) return THREAD_INACTIVE;

  {
    wxCriticalSectionLocker lock(m_list_mutex);

    for (SENCJobTicket *queued : ticket_list) {
      if (queued->m_FullPath000 == ticket->m_FullPath000) return THREAD_PENDING;
    }

    ticket->m_status = THREAD_PENDING;
    ticket->m_completion_posted = false;
    ticket_list.push_back(ticket);
  }

  StartTopJob();
  return THREAD_PENDING;
}

void SENCThreadManager::StartTopJob() {
  if (m_shutting_down) return;

  bool started_job = false;
  do {
    started_job = false;

    {
      wxCriticalSectionLocker lock(m_list_mutex);

      int nRunning = 0;
      for (SENCJobTicket *ticket : ticket_list) {
        if (ticket->m_status == THREAD_STARTED) nRunning++;
      }

      if (nRunning >= m_max_jobs) break;

      SENCJobTicket *startCandidate = nullptr;
      for (SENCJobTicket *ticket : ticket_list) {
        if (ticket->m_status == THREAD_PENDING) {
          startCandidate = ticket;
          break;
        }
      }

      if (!startCandidate) break;

      SENCBuildThread *thread = new SENCBuildThread(startCandidate, this);
      startCandidate->m_status = THREAD_STARTED;
      thread->SetPriority(20);
      if (thread->Run() != wxTHREAD_NO_ERROR) {
        startCandidate->m_status = THREAD_PENDING;
        delete thread;
        break;
      }

      started_job = true;
    }
  } while (started_job);

  UpdateAlertString();
}

void SENCThreadManager::FinishJob(SENCJobTicket *ticket) {
  {
    wxCriticalSectionLocker lock(m_list_mutex);

    for (size_t i = 0; i < ticket_list.size(); i++) {
      if (ticket_list[i] == ticket) {
        ticket->m_status = THREAD_FINISHED;
        completing_list.push_back(ticket);
        ticket_list.erase(ticket_list.begin() + i);
        break;
      }
    }
  }

  UpdateAlertString();
}

int SENCThreadManager::GetJobCount() {
  wxCriticalSectionLocker lock(m_list_mutex);
  return static_cast<int>(ticket_list.size());
}

bool SENCThreadManager::IsChartInTicketlist(s57chart *chart) {
  wxCriticalSectionLocker lock(m_list_mutex);
  for (SENCJobTicket *ticket : ticket_list) {
    if (ticket->m_chart == chart) return true;
  }
  for (SENCJobTicket *ticket : completing_list) {
    if (ticket->m_chart == chart) return true;
  }
  return false;
}

bool SENCThreadManager::SetChartPointer(s57chart *chart, void *new_ptr) {
  wxCriticalSectionLocker lock(m_list_mutex);
  auto update = [&](std::vector<SENCJobTicket *> &list) {
    for (SENCJobTicket *ticket : list) {
      if (ticket->m_chart == chart) {
        ticket->m_chart = static_cast<s57chart *>(new_ptr);
        return true;
      }
    }
    return false;
  };

  if (update(ticket_list)) return true;
  return update(completing_list);
}

void SENCThreadManager::InvalidateChartPointer(s57chart *chart) {
  SetChartPointer(chart, nullptr);
}

void SENCThreadManager::ReleaseCompletedTicket(SENCJobTicket *ticket) {
  wxCriticalSectionLocker lock(m_list_mutex);
  for (size_t i = 0; i < completing_list.size(); i++) {
    if (completing_list[i] == ticket) {
      completing_list.erase(completing_list.begin() + i);
      break;
    }
  }
}

void SENCThreadManager::NotifyFrame(OCPN_BUILDSENC_ThreadEvent &event) {
  if (!top_frame::Get()) return;

  OCPN_BUILDSENC_ThreadEvent frame_event(wxEVT_OCPN_BUILDSENCTHREAD, 0);
  frame_event.stat = event.stat;
  frame_event.type = event.type;
  frame_event.m_ticket = event.m_ticket;
  top_frame::Get()->GetEventHandler()->AddPendingEvent(frame_event);
}

void SENCThreadManager::OnEvtThread(OCPN_BUILDSENC_ThreadEvent &event) {
  switch (event.type) {
    case SENC_BUILD_DONE_NOERROR:
    case SENC_BUILD_DONE_ERROR:
      FinishJob(event.m_ticket);
      StartTopJob();
      NotifyFrame(event);
      break;
    default:
      break;
  }
}

//----------------------------------------------------------------------------------
//      SENCBuildThread Implementation
//----------------------------------------------------------------------------------

SENCBuildThread::SENCBuildThread(SENCJobTicket *ticket,
                                 SENCThreadManager *manager)
    : wxThread(wxTHREAD_DETACHED), m_manager(manager), m_ticket(ticket) {
  m_FullPath000 = ticket->m_FullPath000;
  m_SENCFileName = ticket->m_SENCFileName;
  Create();
}

void *SENCBuildThread::Entry() {
  EVENTSENCResult result = SENC_BUILD_DONE_ERROR;
  int stat = -1;

  if (!m_manager || m_manager->IsShuttingDown()) return nullptr;

  try {
    Osenc senc;

    senc.setRegistrar(g_poRegistrar);
    senc.setRefLocn(m_ticket->ref_lat, m_ticket->ref_lon);
    senc.SetLODMeters(m_ticket->m_LOD_meters);
    senc.setNoErrDialog(true);

    m_ticket->m_SENCResult = SENC_BUILD_STARTED;

    stat = senc.createSenc200(m_FullPath000, m_SENCFileName, false);
    result = (stat == ERROR_INGESTING000) ? SENC_BUILD_DONE_ERROR
                                          : SENC_BUILD_DONE_NOERROR;
  } catch (const std::exception &e) {
    wxLogMessage("SENC build thread exception: %s", e.what());
    result = SENC_BUILD_DONE_ERROR;
    stat = -1;
  }

  // Post exactly one completion event to the manager.
  if (!m_manager || m_manager->IsShuttingDown() ||
      m_ticket->m_completion_posted)
    return nullptr;

  m_ticket->m_completion_posted = true;
  m_ticket->m_SENCResult = result;

  OCPN_BUILDSENC_ThreadEvent event(wxEVT_OCPN_BUILDSENCTHREAD, 0);
  event.stat = stat;
  event.type = result;
  event.m_ticket = m_ticket;
  m_manager->QueueEvent(event.Clone());

  return nullptr;
}
