/***************************************************************************
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
 * S57 Chart Object
 */

#ifndef SENCMGR_H_
#define SENCMGR_H_

#include <vector>

#include <wx/event.h>
#include <wx/string.h>
#include <wx/thread.h>

#include "s57chart.h"

class SENCThreadManager;                       // forward
extern SENCThreadManager *g_SencThreadManager; /**< Global instance */

extern const wxEventType wxEVT_OCPN_BUILDSENCTHREAD; /**< Global instance */

class SENCBuildThread;  // forward

typedef enum {
  THREAD_INACTIVE = 0,
  THREAD_PENDING,
  THREAD_STARTED,
  THREAD_FINISHED
} SENCThreadStatus;

typedef enum {
  SENC_BUILD_INACTIVE = 0,
  SENC_BUILD_PENDING,
  SENC_BUILD_STARTED,
  SENC_BUILD_DONE_NOERROR,
  SENC_BUILD_DONE_ERROR,
} EVENTSENCResult;

//----------------------------------------------------------------------------
// s57 Chart Thread based SENC job ticket
//----------------------------------------------------------------------------
class SENCJobTicket {
public:
  SENCJobTicket();
  ~SENCJobTicket() {}

  s57chart *m_chart;
  wxString m_FullPath000;
  wxString m_SENCFileName;
  double ref_lat, ref_lon;
  double m_LOD_meters;

  SENCThreadStatus m_status;
  EVENTSENCResult m_SENCResult;
  bool m_completion_posted;
};

//----------------------------------------------------------------------------
// s57 Chart Thread based SENC creator status message
//----------------------------------------------------------------------------
class OCPN_BUILDSENC_ThreadEvent : public wxEvent {
public:
  OCPN_BUILDSENC_ThreadEvent(wxEventType commandType = wxEVT_NULL, int id = 0);
  ~OCPN_BUILDSENC_ThreadEvent();

  // required for sending with wxPostEvent()
  wxEvent *Clone() const;

  int stat;
  EVENTSENCResult type;
  SENCJobTicket *m_ticket;

private:
};

/**
 * Manager for S57 chart SENC creation threads.
 * Manages the creation of SENC (Simplified Electronic Navigational Chart) files
 * from S57 charts using background threads. Handles scheduling and executing
 * SENC build jobs.
 *
 * Worker threads post exactly one completion event to this manager via
 * QueueEvent(). OnEvtThread() performs queue bookkeeping and forwards a single
 * copy to the main frame for UI work. The frame deletes the ticket.
 */
class SENCThreadManager : public wxEvtHandler {
public:
  SENCThreadManager();
  ~SENCThreadManager();

  void OnEvtThread(OCPN_BUILDSENC_ThreadEvent &event);

  SENCThreadStatus ScheduleJob(SENCJobTicket *ticket);
  void FinishJob(SENCJobTicket *ticket);
  void StartTopJob();
  bool IsChartInTicketlist(s57chart *chart);
  bool SetChartPointer(s57chart *chart, void *new_ptr);
  void InvalidateChartPointer(s57chart *chart);
  void ReleaseCompletedTicket(SENCJobTicket *ticket);
  void ClearJobList();
  int GetJobCount();
  int GetRunningJobCount();
  bool IsShuttingDown() const;

  int m_max_jobs;

  std::vector<SENCJobTicket *> ticket_list;

private:
  void UpdateAlertString();
  void NotifyFrame(OCPN_BUILDSENC_ThreadEvent &event);

  std::vector<SENCJobTicket *> completing_list;
  wxCriticalSection m_list_mutex;
  bool m_shutting_down;
};

//----------------------------------------------------------------------------
// s57 Chart Thread based SENC creator
//----------------------------------------------------------------------------
class SENCBuildThread : public wxThread {
public:
  SENCBuildThread(SENCJobTicket *ticket, SENCThreadManager *manager);
  void *Entry();

  wxString m_FullPath000;
  wxString m_SENCFileName;
  SENCThreadManager *m_manager;
  SENCJobTicket *m_ticket;
};

#endif  // SENCMGR_H_
