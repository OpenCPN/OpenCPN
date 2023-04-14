/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef SNDFILE_SOUND_LOADER_H
#define SNDFILE_SOUND_LOADER_H

#include <sndfile.h>

#include "SoundFileLoader.h"

/**
 * The original sound file loader supports WAV files only, the same format
 * as supported by the wxSound widget.
 */
class SndfileSoundLoader: public AbstractSoundLoader
{

    public:

        SndfileSoundLoader() {};

        virtual ~SndfileSoundLoader();

        virtual bool Load(const char* path);
        virtual void UnLoad();

        virtual bool Reset();

        virtual size_t Get(void* samples, size_t length);

        unsigned GetBytesPerSample() const { return 2 * m_sfinfo.channels; };

        unsigned GetChannelCount() const;

        unsigned GetSamplingRate() const;

    protected:
        SNDFILE* m_sndfile;
        SF_INFO m_sfinfo;
};


#endif // SNDFILE_SOUND_LOADER_H
