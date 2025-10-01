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

#ifndef ANDROID_SOUND_H_
#define ANDROID_SOUND_H_

#include <mutex>
#include <condition_variable>

#include "o_sound/sound.h"

namespace o_sound_private {

/**
 * Sound backend supports synchronous mode on Android devices.
 */
class AndroidSound : public o_sound::Sound {
public:
  AndroidSound() : m_is_playing(false) {};
  ~AndroidSound() override;

  bool Load(const char*, int deviceIndex = -1) override;
  void UnLoad() override {};
  bool Reset() { return true; };
  bool Play() override;
  bool Stop() override;
  void SetFinishedCallback(AudioDoneCallback cb, void* userData) override;
  void OnSoundDone() override;

protected:
  bool CanPlay();
  std::string m_soundfile;
  bool m_is_playing;

private:
  std::mutex mtx;
  std::condition_variable done;
};

}  //namespace o_sound_private

#endif  // ANDROID_SOUND_H_
