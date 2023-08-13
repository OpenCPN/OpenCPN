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

#include "base_platform.h"
#include "logger.h"
#include "wx_instance_chk.h"

#ifdef _MSC_VER
#include <process.h>
#else
#include <signal.h>
#endif

extern BasePlatform* g_BasePlatform;

static const char* const  kName  = "_OpenCPN_SILock";

static void KillProcess(int pid) {
#ifdef _MSC_VER
  const auto proc = OpenProcess(PROCESS_TERMINATE, false, pid);
  TerminateProcess(proc, 1);
  CloseHandle(proc);
#else
  kill(static_cast<pid_t>(pid), SIGKILL);
#endif
}

WxInstanceCheck::WxInstanceCheck() {
  wxString dir = g_BasePlatform ->GetPrivateDataDir();
  if (!m_checker.Create(kName, dir)) {
    WARNING_LOG << "Cannot create instance locker (!)";
  }
}
void WxInstanceCheck::CleanUp() {
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
