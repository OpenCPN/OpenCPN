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
 * Implement ocpn_wx_sound.h -- sound backend on top of wxwidget's sound
 */


#include <thread>

#include <wx/file.h>
#include <wx/log.h>
#include <wx/sound.h>

#include "ocpn_wx_sound.h"

using namespace o_sound_private;

std::string OcpnWxSound::GetDeviceInfo(int deviceIndex) {
  return "OcpnWxSound";
}

bool OcpnWxSound::Load(const char* path, int deviceIndex) {
  m_ok = m_sound.Create(path);
  m_is_playing = false;
  if (m_ok) {
    m_path = path;
  }
  return m_ok;
}

bool OcpnWxSound::Stop() {
  m_sound.Stop();
  m_ok = false;
  m_is_playing = false;
  return false;
}

void OcpnWxSound::Worker() {
  wxLogDebug("wxSound::worker()");
  m_is_playing = true;
  m_sound.Play(wxSOUND_SYNC);
  if (m_on_finished) {
    m_on_finished(m_callback_data);
    m_on_finished = nullptr;
  }
  m_is_playing = false;
}

bool OcpnWxSound::Play() {
  wxLogDebug("wxSound::Play()");
  if (!m_ok || m_is_playing) {
    wxLogWarning("OcpnWxSound: cannot play: not loaded or busy playing.");
    return false;
  }
  if (m_on_finished) {
    std::thread t([this]() { Worker(); });
    t.detach();
    return true;
  }
  return m_sound.Play(wxSOUND_SYNC);
}
