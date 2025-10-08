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
 * Windows sound backend
 */

#ifndef MSW_SOUND_H_
#define MSW_SOUND_H_

#include "sound.h"

namespace o_sound_private {

/**
 * Sound backend on the windows PlaySound() API.
 */
class MswSound : public o_sound::Sound {
public:
  MswSound() : m_is_playing(false) {};
  ~MswSound() override {MswSound::Stop(); };

  bool Load(const char* path, int deviceIndex = -1) override;
  bool Play() override;
  bool Stop() override;
  void UnLoad() override;

private:
  void Worker();
  std::wstring m_path;
  bool m_is_playing;
};

}  // namespace o_sound_private
#endif  // MSW_SOUND_H_
