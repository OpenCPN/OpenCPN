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
 *   along with this program; if not, see <https://www.gnu.org/licenses/   *
 **************************************************************************/

/**
 * \file
 *
 * Implement std_instance_chgk.h -- native instance check implementation
 */

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#elif defined(__APPLE__)
#include <errno.h>
#include <libproc.h>
#endif

#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <thread>

#ifdef HAVE_UNISTD_H
#include <signal.h>
#include <unistd.h>
#endif

#include <wx/string.h>

#include "model/base_platform.h"
#include "model/ocpn_utils.h"

#include "model/std_instance_chk.h"
#include "std_filesystem.h"

static const char* const kName = "_OpenCPN_SILock";

static int GetLockfilePid(const std::string& path) {
  std::ifstream f(path.c_str());
  std::stringstream ss;
  ss << f.rdbuf();
  int pid = -1;
  try {
    ss >> pid;
  } catch (...) {
    pid = -1;
  }
  return pid;
}

static bool DoesProcessExist(int pid) {
#ifdef _WIN32
  HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
  DWORD ret = WaitForSingleObject(process, 0);
  CloseHandle(process);
  return ret == WAIT_TIMEOUT;
#elif defined(__APPLE__)
  char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
  bool is_active;
  int rv = proc_pidpath(pid, pathbuf, sizeof(pathbuf));
  return true;
#else
  int rv = kill(pid, 0);
  return rv == 0 || errno != ESRCH;
#endif
}

static int GetPid() {
#ifdef _WIN32
  return _getpid();
#else
  return getpid();
#endif
}

StdInstanceCheck::StdInstanceCheck()
    : m_is_main_instance(false), is_inited(false), is_my_lock(false) {}

void StdInstanceCheck::Init() {
  m_path = g_BasePlatform->GetPrivateDataDir().ToStdString();
  m_path /= kName;
  for (int i = 0; i < 5; i += 1) {
    std::FILE* f = std::fopen(m_path.generic_u8string().c_str(), "wx");
    if (f) {
      std::stringstream ss;
      ss << getpid() << "\n";
      std::fputs(ss.str().c_str(), f);
      fclose(f);
      m_is_main_instance = true;
      is_my_lock = true;
      break;
    }
    if (i >= 2) {
      wxLogDebug("Removing unusable lock file");
      fs::remove(m_path);  // unusable
      continue;
    }
    int pid = -1;
    try {
      std::ifstream stream(m_path);
      stream >> pid;
    } catch (...) {
      // Handle race conditions: other process creates lock
      using namespace std::chrono;
      std::this_thread::sleep_for(50ms);
      continue;
    }
    if (pid == GetPid()) {
      m_is_main_instance = true;
      break;
    }
    if (pid != -1 && !DoesProcessExist(pid)) {
      wxLogDebug("Removing orphaned lock for pid: %d", pid);
      fs::remove(m_path);
    }
    if (pid != -1 && fs::exists(m_path)) break;
  };
  is_inited = true;
}

bool StdInstanceCheck::IsMainInstance() {
  if (!is_inited) Init();
  return m_is_main_instance;
}

void StdInstanceCheck::CleanUp() {
  if (!is_inited && g_BasePlatform) Init();
  if (fs::exists(m_path)) fs::remove(m_path);
}

StdInstanceCheck::~StdInstanceCheck() {
  if (fs::exists(m_path) && is_my_lock) fs::remove(m_path);
}
