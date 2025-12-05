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

// OS-specific path length limits
#ifdef _WIN32
  #ifndef MAX_PATH
    #define MAX_PATH 260
  #endif
  #define MAX_SAFE_PATH MAX_PATH
#else
  #include <limits.h>
  #ifdef PATH_MAX
    #define MAX_SAFE_PATH PATH_MAX
  #else
    #define MAX_SAFE_PATH 4096  // POSIX minimum
  #endif
#endif


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

/**
 * @brief Validate file path to prevent command injection
 * @param path File path to validate
 * @return true if path is safe, false otherwise
 */
static bool is_safe_path(const char* path) {
  if (!path || path[0] == '\0') {
    return false;
  }
  
  // Block dangerous shell metacharacters
  const char* dangerous_chars = ";|&$`<>(){}[]!*?~^";
  for (size_t i = 0; i < strlen(dangerous_chars); i++) {
    if (strchr(path, dangerous_chars[i]) != NULL) {
      wxLogWarning("Security: Path contains dangerous character: %c", dangerous_chars[i]);
      return false;
    }
  }
  
  // Prevent path traversal
  if (strstr(path, "..") != NULL) {
    wxLogWarning("Security: Path contains '..' (path traversal attempt)");
    return false;
  }
  
  // Block newline injection
  if (strchr(path, '\n') != NULL || strchr(path, '\r') != NULL) {
    wxLogWarning("Security: Path contains newline character");
    return false;
  }
  
  // Path length limit
  if (strlen(path) >= MAX_SAFE_PATH) {
    wxLogWarning("Security: Path too long (max %d characters)", MAX_SAFE_PATH);
    return false;
  }
  
  return true;
}

static int do_play(const char* cmd, const char* path) {
  // Security: Validate input before executing
  if (!is_safe_path(path)) {
    wxLogWarning("Security: Invalid or unsafe or unsafe file path rejected: %s", path);
    return -1;
  }

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
