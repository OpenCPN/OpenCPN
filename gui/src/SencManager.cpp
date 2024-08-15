/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Object
 * Author:   David Register
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "s57chart.h"
#include "Osenc.h"
#include "chcanv.h"
#include "ocpn_frame.h"

extern MyFrame *gFrame;
extern int g_nCPUCount;
extern S57ClassRegistrar *g_poRegistrar;

//----------------------------------------------------------------------------------
//      SENCJobTicket Implementation
//----------------------------------------------------------------------------------
SENCJobTicket::SENCJobTicket() {
  m_SENCResult = SENC_BUILD_INACTIVE;
  m_status = THREAD_INACTIVE;
}

const wxEventType wxEVT_OCPN_BUILDSENCTHREAD = wxNewEventType();

//----------------------------------------------------------------------------------
//      OCPN_BUILDSENC_ThreadEvent Implementation
//----------------------------------------------------------------------------------
OCPN_BUILDSENC_ThreadEvent::OCPN_BUILDSENC_ThreadEvent(wxEventType commandType,
                                                       int id)
    : wxEvent(id, commandType) {
  stat = 0;
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
SENCThreadManager::SENCThreadManager() {
  // ideally we would use the cpu count -1, and only launch jobs
  // when the idle load average is sufficient (greater than 1)
  int nCPU = wxMax(1, wxThread::GetCPUCount());
  if (g_nCPUCount > 0) nCPU = g_nCPUCount;

  // obviously there's at least one CPU!
  if (nCPU < 1) nCPU = 1;

  m_max_jobs = wxMax(nCPU - 1, 1);
  // m_max_jobs = 1;

  wxLogDebug("SENC: nCPU: %d    m_max_jobs :%d\n", nCPU, m_max_jobs);

  //  Create/connect a dynamic event handler slot for messages from the worker
  //  threads
  Connect(
      wxEVT_OCPN_BUILDSENCTHREAD,
      (wxObjectEventFunction)(wxEventFunction)&SENCThreadManager::OnEvtThread);

  //     m_timer.Connect(wxEVT_TIMER, wxTimerEventHandler(
  //     glTextureManager::OnTimer ), NULL, this); m_timer.Start(500);
}

SENCThreadManager::~SENCThreadManager() {
  //    ClearJobList();
}

SENCThreadStatus SENCThreadManager::ScheduleJob(SENCJobTicket *ticket) {
  //  Do not add a job if there is already a job pending for this chart, by name
  for (size_t i = 0; i < ticket_list.size(); i++) {
    if (ticket_list[i]->m_FullPath000 == ticket->m_FullPath000)
      return THREAD_PENDING;
  }

  ticket->m_status = THREAD_PENDING;
  ticket_list.push_back(ticket);

  // printf("Scheduling job:  %s\n", (const
  // char*)ticket->m_FullPath000.mb_str()); printf("Job count:  %d\n",
  // ticket_list.size());
  StartTopJob();
  return THREAD_PENDING;
}

void SENCThreadManager::StartTopJob() {
  SENCJobTicket *startCandidate;
  // Get the running job count
  int nRunning = 0;
  for (size_t i = 0; i < ticket_list.size(); i++) {
    if (ticket_list[i]->m_status == THREAD_STARTED) nRunning++;
  }

  // OK to start one?
  if (nRunning < m_max_jobs) {
    // Find the first eligible
    startCandidate = NULL;
    for (size_t i = 0; i < ticket_list.size(); i++) {
      if (ticket_list[i]->m_status == THREAD_PENDING) {
        startCandidate = ticket_list[i];
        break;
      }
    }

    // Found one?
    if (startCandidate) {
      // printf("Starting job:  %s\n", (const
      // char*)startCandidate->m_FullPath000.mb_str());

      SENCBuildThread *thread = new SENCBuildThread(startCandidate, this);
      startCandidate->m_thread = thread;
      startCandidate->m_status = THREAD_STARTED;
      thread->SetPriority(20);
      thread->Run();
      nRunning++;
    }
  }

  if (nRunning) {
    wxString count;
    count.Printf(_T("  %ld"), ticket_list.size());
    if (gFrame->GetPrimaryCanvas())
      gFrame->GetPrimaryCanvas()->SetAlertString(_("Preparing vector chart  ") +
                                                 count);
  }
}

void SENCThreadManager::FinishJob(SENCJobTicket *ticket) {
  // printf("Finishing job:  %s\n", (const
  // char*)ticket->m_FullPath000.mb_str());

  // Find and remove the ticket from the list
  for (size_t i = 0; i < ticket_list.size(); i++) {
    if (ticket_list[i] == ticket) {
      // printf("   Removing job:  %s\n", (const
      // char*)ticket->m_FullPath000.mb_str());

      ticket_list.erase(ticket_list.begin() + i);
      // printf("Job count:  %d\n", ticket_list.size());

      break;
    }
  }

#if 1
  int nRunning = 0;
  for (size_t i = 0; i < ticket_list.size(); i++) {
    if (ticket_list[i]->m_status == THREAD_STARTED) nRunning++;
  }

  if (nRunning) {
    wxString count;
    count.Printf(_T("  %ld"), ticket_list.size());
    if (gFrame->GetPrimaryCanvas())
      gFrame->GetPrimaryCanvas()->SetAlertString(_("Preparing vector chart  ") +
                                                 count);
  } else {
    if (gFrame->GetPrimaryCanvas())
      gFrame->GetPrimaryCanvas()->SetAlertString(_T(""));
  }
#endif
}

int SENCThreadManager::GetJobCount() { return ticket_list.size(); }

bool SENCThreadManager::IsChartInTicketlist(s57chart *chart) {
  for (size_t i = 0; i < ticket_list.size(); i++) {
    if (ticket_list[i]->m_chart == chart) return true;
  }
  return false;
}

bool SENCThreadManager::SetChartPointer(s57chart *chart, void *new_ptr) {
  // Find the ticket
  for (size_t i = 0; i < ticket_list.size(); i++) {
    if (ticket_list[i]->m_chart == chart) {
      ticket_list[i]->m_chart = (s57chart *)new_ptr;
      return true;
    }
  }
  return false;
}

#define NBAR_LENGTH 40

void SENCThreadManager::OnEvtThread(OCPN_BUILDSENC_ThreadEvent &event) {
  OCPN_BUILDSENC_ThreadEvent Sevent(wxEVT_OCPN_BUILDSENCTHREAD, 0);

  switch (event.type) {
    case SENC_BUILD_STARTED:
      // printf("SENC build started\n");
      Sevent.type = SENC_BUILD_STARTED;
      Sevent.m_ticket = event.m_ticket;

      break;
    case SENC_BUILD_DONE_NOERROR:
      // printf("SENC build done no error\n");
      Sevent.type = SENC_BUILD_DONE_NOERROR;
      Sevent.m_ticket = event.m_ticket;
      FinishJob(event.m_ticket);
      StartTopJob();

      break;
    case SENC_BUILD_DONE_ERROR:
      // printf("SENC build done ERROR\n");
      Sevent.type = SENC_BUILD_DONE_ERROR;
      Sevent.m_ticket = event.m_ticket;
      FinishJob(event.m_ticket);
      StartTopJob();

      break;
    default:
      break;
  }
  if (gFrame) gFrame->GetEventHandler()->AddPendingEvent(Sevent);
}

//----------------------------------------------------------------------------------
//      SENCBuildThread Implementation
//----------------------------------------------------------------------------------

SENCBuildThread::SENCBuildThread(SENCJobTicket *ticket,
                                 SENCThreadManager *manager) {
  m_FullPath000 = ticket->m_FullPath000;
  m_SENCFileName = ticket->m_SENCFileName;
  m_manager = manager;
  m_ticket = ticket;

  Create();
}

void *SENCBuildThread::Entry() {
  // #ifdef __MSVC__
  //   _set_se_translator(my_translate);

  //  On Windows, if anything in this thread produces a SEH exception (like
  //  access violation) we handle the exception locally, and simply alow the
  //  thread to exit smoothly with no results. Upstream will notice that nothing
  //  got done, and maybe try again later.

  try
  // #endif
  {
    // Start the SENC build
    Osenc senc;

    senc.setRegistrar(g_poRegistrar);
    senc.setRefLocn(m_ticket->ref_lat, m_ticket->ref_lon);
    senc.SetLODMeters(m_ticket->m_LOD_meters);
    senc.setNoErrDialog(true);

    m_ticket->m_SENCResult = SENC_BUILD_STARTED;
    OCPN_BUILDSENC_ThreadEvent Sevent(wxEVT_OCPN_BUILDSENCTHREAD, 0);
    Sevent.stat = 0;
    Sevent.type = SENC_BUILD_STARTED;
    Sevent.m_ticket = m_ticket;
    if (m_manager) m_manager->QueueEvent(Sevent.Clone());

    int ret = senc.createSenc200(m_FullPath000, m_SENCFileName, false);

    OCPN_BUILDSENC_ThreadEvent Nevent(wxEVT_OCPN_BUILDSENCTHREAD, 0);
    Nevent.stat = ret;
    Nevent.m_ticket = m_ticket;
    if (ret == ERROR_INGESTING000)
      Nevent.type = SENC_BUILD_DONE_ERROR;
    else
      Nevent.type = SENC_BUILD_DONE_NOERROR;

    m_ticket->m_SENCResult = Sevent.type;
    if (m_manager) m_manager->QueueEvent(Nevent.Clone());

    // if(ret == ERROR_INGESTING000)
    //  return BUILD_SENC_NOK_PERMANENT;
    // else
    //  return ret;

    return 0;
  }  // try

  // #ifdef __MSVC__
  catch (const std::exception &e /*SE_Exception e*/) {
    const char *msg = e.what();
    if (m_manager) {
      //             OCPN_CompressionThreadEvent
      //             Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
      //             m_ticket->b_isaborted = true;
      //             Nevent.SetTicket(m_ticket);
      //             Nevent.type = 0;
      //             m_manager->QueueEvent(Nevent.Clone());
    }

    return 0;
  }
  // #endif
}
