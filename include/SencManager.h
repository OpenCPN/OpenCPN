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

#ifndef __SENCMGR_H__
#define __SENCMGR_H__

#include <vector>

// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Fwd Defns
//----------------------------------------------------------------------------

class s57chart;
class SENCBuildThread;

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

extern const wxEventType wxEVT_OCPN_BUILDSENCTHREAD;

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

  SENCBuildThread *m_thread;

  SENCThreadStatus m_status;
  EVENTSENCResult m_SENCResult;
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

//----------------------------------------------------------------------------
// s57 Chart Thread based SENC creator
//----------------------------------------------------------------------------
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
  int GetJobCount();

  int m_max_jobs;

  std::vector<SENCJobTicket *> ticket_list;
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
  s57chart *m_chart;
  SENCThreadManager *m_manager;
  SENCJobTicket *m_ticket;
};

#endif
