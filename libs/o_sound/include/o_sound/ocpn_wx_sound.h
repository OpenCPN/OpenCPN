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

#ifndef WX_SOUND_H_
#define WX_SOUND_H_

#include <wx/sound.h>

#include "sound.h"

namespace o_sound_private {

/**
 * Sound backend based on wxWidget's wxSound class. On Linux, this seems
 * to be broken -- at a minimum it requires the old OSS API which on
 * pulseaudio platforms means to have the osspd service running. Even so,
 * there seems to be many quirks here.
 *
 * However, the backend might,  possibly work on macOS and/or Windows.
 * Supports synchronous and asynchronous mode.
 */
class OcpnWxSound : public o_sound::Sound {
public:
  OcpnWxSound() : m_is_playing(false) {};
  ~OcpnWxSound() override { OcpnWxSound::Stop(); };

  bool Load(const char* path, int deviceIndex) override;
  bool Load(const char* path) override { return Load(path, -1); }
  void UnLoad() override {};
  bool Play() override;
  bool Stop() override;
  std::string GetDeviceInfo(int deviceIndex) override;

private:
  void Worker();
  std::string m_path;
  bool m_is_playing;
  wxSound m_sound;
};

}  // namespace o_sound_private
#endif  // WX_SOUND_H_
