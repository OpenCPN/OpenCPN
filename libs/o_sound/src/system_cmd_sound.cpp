/***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 * Implement system_cmd_sound.h -- Sound backend based on CLI tools.
 */


#include <cstdlib>
#include <thread>

#include <wx/file.h>
#include <wx/log.h>

#include "system_cmd_sound.h"

#include "ocpn_wx_sound.h"

namespace  o_sound {

#ifdef _WIN32
#include <windows.h>

static int do_play(const char* cmd, const char* path) {
  wchar_t sound_path[80];
  MultiByteToWideChar(0, 0, path, -1, sound_path, 80);
  LPCWSTR wide_path = sound_path;

  PlaySound(wide_path, NULL, SND_FILENAME);

  return 0;
}

#else                  /* _WIN32, i. e. POSIX */
#include <sys/wait.h>  // for WEXITSTATUS & friends

static int do_play(const char* cmd, const char* path) {
  char buff[1024];
  snprintf(buff, sizeof(buff), cmd, path);
  wxLogDebug("Sound command: %s", buff);

  int status = system(buff);
  if (status == -1) {
    wxLogWarning("Cannot fork process running %s", buff);
    return -1;
  }
  if (WIFEXITED(status)) {
    status = WEXITSTATUS(status);
    if (status != 0) {
      wxLogWarning("Exit code %d from command %s", status, buff);
    }
  } else {
    wxLogWarning("Strange return code %d (0x%x) running %s", status, status,
                 buff);
  }
  return status;
}
#endif

bool SystemCmdSound::Load(const char* path, int deviceIndex) {
  m_path = path;
  if (deviceIndex != -1) {
    wxLogMessage("Selecting device is not supported by SystemCmdSound");
  }
  m_ok = wxFileExists(m_path);
  return m_ok;
}

bool SystemCmdSound::Stop() { return false; }

bool SystemCmdSound::CanPlay() {
  if (m_is_playing) wxLogWarning("SystemCmdSound: cannot play: already playing");
  return m_ok && !m_is_playing;
}

void SystemCmdSound::Worker() {
  wxLogDebug("SystemCmdSound::worker()");
  m_is_playing = true;
  do_play(m_cmd.c_str(), m_path.c_str());
  m_on_finished(m_callback_data);
  m_on_finished = nullptr;
  m_is_playing = false;
}

bool SystemCmdSound::Play() {
  wxLogDebug("SystemCmdSound::Play()");
  if (m_is_playing) {
    wxLogWarning("SystemCmdSound: cannot play: already playing");
    return false;
  }
  if (m_on_finished) {
    std::thread t([this]() { Worker(); });
    t.detach();
    return true;
  }
  int r = do_play(m_cmd.c_str(), m_path.c_str());
  return r == 0;
}
}  // namespace o_sound
