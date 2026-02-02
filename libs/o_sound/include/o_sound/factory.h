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
 * Sound factory class
 */

#ifndef SOUND_FACTORY_H
#define SOUND_FACTORY_H

#include "sound.h"

namespace o_sound {

/**
 * Creates a Sound instance based on configuration done by cmake
 * enshrined in config.h.
 *
 * @return Sound* instance owned by caller.
 */

Sound* Factory(const char* system_command = nullptr);

}  // namespace o_sound

#endif  // SOUND_FACTORY_H
