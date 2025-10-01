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
 * Sound file loader on top of libsndfile
 */

#ifndef SNDFILE_SOUND_LOADER_H
#define SNDFILE_SOUND_LOADER_H

#include <sndfile.h>

#include "sound_file_loader.h"

namespace o_sound_private {

/**
 * The original sound file loader supports WAV files only, the same format
 * as supported by the wxSound widget. libsndfile supports a wide range of
 * formats; the exact list is platform dependent.
 */
class SndfileSoundLoader : public AbstractSoundLoader {
public:
  SndfileSoundLoader() :m_sndfile(nullptr), m_sfinfo() {};

  ~SndfileSoundLoader() override;

  bool Load(const char* path) override;
  void UnLoad() override;

  bool Reset() override;

  size_t Get(void* samples, size_t length) override;

  [[nodiscard]] unsigned GetBytesPerSample() const override { return 2 * m_sfinfo.channels; }

 [[nodiscard]]  unsigned GetChannelCount() const override;

  [[nodiscard]] unsigned GetSamplingRate() const override;

protected:
  SNDFILE* m_sndfile;
  SF_INFO m_sfinfo;
};

}  // namespace o_sound_private

#endif  // SNDFILE_SOUND_LOADER_H
