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
 * Implement ocpn_sound.h -- abstract sound interface
 */

#include "sound.h"

using namespace o_sound;

Sound::Sound()
  : m_ok(false),
    m_device_ix(-1),
    m_on_finished(nullptr),
    m_callback_data(nullptr) {}

Sound::~Sound()  = default;

void Sound::SetFinishedCallback(AudioDoneCallback cb, void* user_data) {
  m_on_finished = std::move(cb);
  m_callback_data = user_data;
}
