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
 * Implement android_sound.h --  Android sound backend.
 */

#ifndef OCPN_ANDROID_SOUND
#define OCPN_ANDROID_SOUND

#include <sstream>
#include <thread>

#include <wx/log.h>

#include "o_sound/android_sound.h"

#include "android_jvm.h"

using namespace o_sound_private;

static bool androidPlaySound(const wxString soundfile, AndroidSound* sound) {
  std::ostringstream oss;
  oss << sound;
  wxString wxSound(oss.str());
  wxString result =
      callActivityMethod_s2s("playSound", soundfile, wxSound.Mid(2));
  return true;
}

AndroidSound::~AndroidSound() {}

void AndroidSound::SetFinishedCallback(AudioDoneCallback cb, void* userData) {
  m_on_finished = cb;
  m_callback_data = userData;
}

bool AndroidSound::Load(const char* path, int deviceIndex) {
  m_soundfile = path;
  m_ok = true;
  if (deviceIndex != -1) {
    wxLogWarning("Android backend does not support audio device setup.");
  }
  return true;
}

bool AndroidSound::CanPlay() {
  if (m_is_playing) wxLogWarning("SystemCmdSound: cannot play: already playing");
  return m_ok && !m_is_playing;
}

bool AndroidSound::Stop() { return false; }

void AndroidSound::OnSoundDone() {
  std::unique_lock<std::mutex> lock(mtx);
  if (m_on_finished) m_on_finished(m_callback_data);
  m_on_finished = 0;
  m_is_playing = false;
  done.notify_one();
}

bool AndroidSound::Play() {
  std::unique_lock<std::mutex> lock(mtx);
  wxLogDebug("AndroidSound::Play()");
  if (m_is_playing) {
    wxLogWarning("AndroidSound: cannot play: already playing");
    return false;
  }
  m_is_playing = true;
  bool ok = androidPlaySound(m_soundfile, this);
  if (!m_on_finished) {
    wxLogDebug("AndroidSound: waiting for completion");
    done.wait(lock);
    return ok;
  }
  return true;
}

#endif  //  OCPN_ANDROID_SOUND
