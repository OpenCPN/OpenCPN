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

#include <cstdio>
#include <fstream>
#include <sstream>

#ifdef HAVE_UNISTD_H
#include <signal.h>
#include <unistd.h>
#endif

#include <wx/string.h>

#include "base_platform.h"
#include "ocpn_utils.h"

#include "std_instance_chk.h"


extern BasePlatform* g_BasePlatform;

static const char* const  kName  = "_OpenCPN_SILock";

static int GetLockfilePid(const std::string& path){ 
  std::ifstream f(path.c_str());
  std::stringstream ss;
  ss << f.rdbuf();
  int pid = -1;
  try { ss >> pid; } catch (...) { pid = -1; }
  return  pid; 
}
 
StdInstanceCheck::StdInstanceCheck() : m_is_main_instance(false) {
  wxString dir = g_BasePlatform ->GetPrivateDataDir();
  m_path = (dir + "/" + kName).ToStdString();
  std::stringstream ss;
  ss << m_path <<  "." << getpid();
  std::ofstream f(ss.str());
  f << getpid() << "\n";
  if (!ocpn::exists(m_path.c_str())) {
    std::rename(ss.str().c_str(), m_path.c_str());
    m_is_main_instance = true;
  } else {
    std::remove(ss.str().c_str());
  }
}

bool StdInstanceCheck::IsMainInstance() { return m_is_main_instance; }

void StdInstanceCheck::CleanUp() {
  if (!ocpn::exists(m_path)) {
    return;
  }
  int pid = GetLockfilePid(m_path);
  if (pid == -1) {
    return;
  }
  // Try to kill zombie process and remove lock file
  for (int  i = 0; kill(pid, 0) == 0 && i < 3; i++) {
    kill(pid, SIGTERM);
    sleep(1);
  }      
  if (kill(pid, 0) == 0) kill(pid, SIGKILL);
  std::remove(m_path.c_str());
}

StdInstanceCheck::~StdInstanceCheck() {
  if (!ocpn::exists(m_path)) return;
  int  pid = GetLockfilePid(m_path);
  if (pid == getpid()) std::remove(m_path.c_str());
}
