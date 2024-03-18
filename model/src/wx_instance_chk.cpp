/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
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

#include <wx/filename.h>
#include <wx/string.h>

#include "model/base_platform.h"
#include "model/logger.h"
#include "model/wx_instance_chk.h"

#ifdef _MSC_VER
#include <process.h>
#else
#include <signal.h>
#endif

static const char* const  kName  = "_OpenCPN_SILock";

static void KillProcess(int pid) {
#ifdef _MSC_VER
  if (GetCurrentProcessId() != pid) {
    const auto proc = OpenProcess(PROCESS_TERMINATE, false, pid);
    TerminateProcess(proc, 1);
    CloseHandle(proc);
  }
#else
  if (pid != getpid()) kill(static_cast<pid_t>(pid), SIGKILL);
#endif
}

//
// Since required global variables does not exist from the beginning we
// use lazy  init, postponed until object is actually used. At this point
// required  globals should be in place
WxInstanceCheck::WxInstanceCheck()
       : m_checker(new wxSingleInstanceChecker), is_inited(false) { }

void WxInstanceCheck::Init() {
  assert(g_BasePlatform && "NULL g_BasePlatform");
  wxString dir = g_BasePlatform ->GetPrivateDataDir();
  if (!m_checker->Create(kName, dir)) {
    WARNING_LOG << "Cannot create instance locker (!)";
  }
  is_inited = true;
}

bool  WxInstanceCheck::IsMainInstance() {
  if (!is_inited) Init();
  return !m_checker->IsAnotherRunning();
}

void WxInstanceCheck::CleanUp() {
  if (!is_inited) Init();
  wxFileName lockfile(g_BasePlatform ->GetPrivateDataDir(), kName);
  if (!wxFileExists(lockfile.GetFullPath())) return;

  // Best effort try to read pid from lock file and kill it.
  int pid = -1;
  std::ifstream f(lockfile.GetFullPath().ToStdString());
  if (f.good()) {
    std::stringstream ss;
    ss << f.rdbuf();
    try {
      pid = std::stoi(ss.str());
    } catch (...) {}
  }
  wxRemoveFile(lockfile.GetFullPath());
  if (pid != -1) KillProcess(pid);
}

void WxInstanceCheck::OnExit() {
  if (!is_inited) Init();
  delete m_checker;
  m_checker = 0;
}
