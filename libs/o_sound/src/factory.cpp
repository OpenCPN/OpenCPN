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
 * Implement sound_factory.h -- Sound factory.
 */

#include "snd_config.h"
#include "factory.h"
#include "ocpn_wx_sound.h"

#if defined(__ANDROID__)
#include "android_sound.h"
#endif

#if defined(HAVE_PORTAUDIO)
#include "port_audio_sound.h"
#endif

#if defined(HAVE_SYSTEM_CMD_SOUND)
#include "system_cmd_sound.h"
#endif

#if defined(_WIN32)
#include "msw_sound.h"
#endif

namespace o_sound {

using namespace  o_sound_private;

Sound *g_anchorwatch_sound = Factory();

#if defined(__ANDROID__)
Sound* Factory(const char* not_used) { return new AndroidSound(); }

#elif defined(HAVE_PORTAUDIO)
Sound* Factory(const char* not_used) { return new PortAudioSound(); }

#elif defined(HAVE_SYSTEM_CMD_SOUND)
Sound* Factory(const char* sound_cmd) {
  return new SystemCmdSound(sound_cmd ? sound_cmd : OCPN_SOUND_CMD);
}

#elif defined(_WIN32)
Sound* Factory(const char* not_used) { return new MswSound(); }

#else
Sound* Factory(const char* not_used) { return new OcpnWxSound(); }

#endif

}  // namespace o_sound
