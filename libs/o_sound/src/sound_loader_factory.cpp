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
 *   GNU General Public License for more details.                          nd
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement sound_loader_factory.h -- sound loader factory
 */

#include "snd_config.h"
#include "sound_file_loader.h"

#ifdef HAVE_SNDFILE
#include "sndfile_sound_loader.h"
#endif

namespace o_sound_private {

#ifdef HAVE_SNDFILE

AbstractSoundLoader* SoundLoaderFactory() { return new o_sound_private::SndfileSoundLoader(); }

#else

AbstractSoundLoader* SoundLoaderFactory() { return new SoundFileLoader(); }

#endif

}
