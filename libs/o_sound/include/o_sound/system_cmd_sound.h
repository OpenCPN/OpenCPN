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
 * Sound backend based on CLI tools lika aplay(1), etc.
 */

#ifndef SYSTEM_CMD_SOUND_H_
#define SYSTEM_CMD_SOUND_H_

#include "sound.h"
#include "snd_config.h"

namespace o_sound {

/**
 * Sound backend based on running an external CLI tool using system(3).
 * Supports synchronous and asynchronous mode. The command line tools
 * also typically supports a wide range of audio formats; the exact
 * list is platform dependent.
 */
class SystemCmdSound : public Sound {
public:
  explicit SystemCmdSound(const char* cmd = OCPN_SOUND_CMD)
       : m_is_playing(false), m_cmd(cmd) {};
  ~SystemCmdSound() override  = default;

  bool Load(const char* path, int deviceIndex) override;
  bool Load(const char* path) override  { return  Load(path, -1); }
  void UnLoad() override {};
  bool Play() override;
  bool Stop() override;
  /**
   * Set system command string in case program wants to change from
   * default string.
   */
  void SetCmd(const char* cmd) { m_cmd = cmd; };

private:
  void Worker();
  bool CanPlay();
  bool m_is_playing;
  std::string m_cmd;
  std::string m_path;
};

constexpr unsigned maxPlayTime = 200;  // maximum stall time is 200mS

}  // namespace o_sound

#endif  // SYSTEM_CMD_SOUND_H_
