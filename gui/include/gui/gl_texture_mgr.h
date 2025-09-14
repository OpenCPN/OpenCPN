/******************************************************************************
 *
 * Project:  OpenCPN
 * Authors:  David Register
 *           Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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
 ***************************************************************************
 */

#ifndef __GLTEXTUREMANAGER_H__
#define __GLTEXTUREMANAGER_H__

#include <list>

#include <wx/event.h>
#include <wx/string.h>
#include <wx/thread.h>
#include <wx/timer.h>

const wxEventType wxEVT_OCPN_COMPRESSIONTHREAD = wxNewEventType();

class JobTicket;
class wxGenericProgressDialog;

class ProgressInfoItem;
using ProgressInfoList = std::list<ProgressInfoItem *>;

class ProgressInfoItem {
public:
  ProgressInfoItem() {};
  ~ProgressInfoItem() {};

  wxString file_path;
  wxString msgx;
};

class CompressionPoolThread : public wxThread {
public:
  CompressionPoolThread(JobTicket *ticket, wxEvtHandler *message_target);
  void *Entry();

  wxEvtHandler *m_pMessageTarget;
  JobTicket *m_ticket;
};

class OCPN_CompressionThreadEvent : public wxEvent {
public:
  OCPN_CompressionThreadEvent(wxEventType commandType = wxEVT_NULL, int id = 0);
  ~OCPN_CompressionThreadEvent();

  // accessors
  void SetTicket(JobTicket *ticket) { m_ticket = ticket; }
  JobTicket *GetTicket(void) { return m_ticket; }

  // required for sending with wxPostEvent()
  wxEvent *Clone() const;

  int type;
  int nstat;
  int nstat_max;

private:
  JobTicket *m_ticket;
};

class CompressionPoolThread;
class JobTicket {
public:
  JobTicket();
  ~JobTicket() { free(level0_bits); }
  bool DoJob();
  bool DoJob(const wxRect &rect);

  glTexFactory *pFact;
  wxRect m_rect;
  int level_min_request;
  int ident;
  bool b_throttle;

  CompressionPoolThread *pthread;
  unsigned char *level0_bits;
  unsigned char *comp_bits_array[10];
  wxString m_ChartPath;
  bool b_abort;
  bool b_isaborted;
  bool bpost_zip_compress;
  bool binplace;
  unsigned char *compcomp_bits_array[10];
  int compcomp_size_array[10];
  bool b_inCompressAll;
};

//      This is a hashmap with Chart full path as key, and glTexFactory as value
WX_DECLARE_STRING_HASH_MAP(glTexFactory *, ChartPathHashTexfactType);

//      glTextureManager Definition
class glTextureManager : public wxEvtHandler {
public:
  glTextureManager();
  ~glTextureManager();

  void OnEvtThread(OCPN_CompressionThreadEvent &event);
  void OnTimer(wxTimerEvent &event);
  bool ScheduleJob(glTexFactory *client, const wxRect &rect, int level_min,
                   bool b_throttle_thread, bool b_nolimit, bool b_postZip,
                   bool b_inplace);

  int GetRunningJobCount() { return running_list.size(); }
  int GetJobCount() { return GetRunningJobCount() + todo_list.size(); }
  bool AsJob(wxString const &chart_path) const;
  void PurgeJobList(wxString chart_path = wxEmptyString);
  void ClearJobList();
  void ClearAllRasterTextures(void);
  bool PurgeChartTextures(ChartBase *pc, bool b_purge_factory = false);
  bool TextureCrunch(double factor);
  bool FactoryCrunch(double factor);
  void BuildCompressedCache();

  //    This is a hash table
  //    key is Chart full path
  //    Value is glTexFactory*
  ChartPathHashTexfactType m_chart_texfactory_hash;

private:
  bool DoJob(JobTicket *pticket);
  bool DoThreadJob(JobTicket *pticket);
  bool StartTopJob();

  std::list<JobTicket *> running_list;
  std::list<JobTicket *> todo_list;
  int m_max_jobs;

  int m_prevMemUsed;

  wxTimer m_timer;
  size_t m_ticks;
  wxGenericProgressDialog *m_progDialog;
  wxString m_progMsg;
  unsigned int m_jcnt;
  ProgressInfoList progList;
  bool m_skip;
  bool m_skipout;
  bool m_bcompact;
};

class glTextureDescriptor;
void GetFullMap(glTextureDescriptor *ptd, const wxRect &rect,
                wxString chart_path, int level);
int TextureDim(int level);
int TextureTileSize(int level, bool compressed);

#endif
