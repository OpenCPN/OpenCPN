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
 * Implement msw_sound.h -- Windows sound backend
 */

#include <thread>

#include <windows.h>

#include <wx/string.h>
#include <wx/log.h>

#include "msw_sound.h"

using namespace o_sound_private;

bool MswSound::Load(const char* path, int deviceIndex) {
  m_path = wxString(path).ToStdWstring();
  m_is_playing = false;
  m_ok = true;
  return m_ok;
}

bool MswSound::Stop() {
  m_is_playing = false;
  return PlaySound(NULL, NULL, 0);
}

void MswSound::Worker() {
  wxLogDebug("mswSound::worker()");
  m_is_playing = true;
  PlaySound(m_path.c_str(), NULL, SND_FILENAME);
  if (m_on_finished) {
    m_on_finished(m_callback_data);
    m_on_finished = 0;
  }
  m_is_playing = false;
}

bool MswSound::Play() {
  wxLogDebug("mswSound::Play()");
  if (!m_ok || m_is_playing) {
    wxLogWarning("MswSound: cannot play: not loaded or busy playing.");
    return false;
  }
  if (m_on_finished) {
    std::thread t([this]() { Worker(); });
    t.detach();
    return true;
  }
  return PlaySound(m_path.c_str(), NULL, SND_FILENAME);
}

void MswSound::UnLoad() {
  PlaySound(NULL, NULL, 0);
  m_ok = false;
}
