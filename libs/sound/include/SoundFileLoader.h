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

#ifndef SOUND_FILE_LOADER_H
#define SOUND_FILE_LOADER_H

#include <functional>
#include <memory>
#include <string>


/**
 * A sound loader loads data from a sound file into memory, making it
 * available to be played using a backend.
 */
class AbstractSoundLoader {

     public:

        virtual ~AbstractSoundLoader() {};

        /** Load data from sound file at given path. */
        virtual bool Load(const char* path) = 0;

        /** Unload and close any open sound file. */
        virtual void UnLoad() = 0;

        /** Reset internal pointers to file start, like seek(0, SEEK_SET) */
        virtual bool Reset() = 0;

        /**
         * Fetch length bytes from loaded sample data. Returns nr of bytes
         * transferred, possibly less than requested at end of data.
         */
        virtual size_t Get(void* samples, size_t length) = 0;

        /** Get bytes/sample, typically 1 or 2. */
        virtual unsigned GetBytesPerSample() const = 0;

        /** Get nr of channels, typically 1 (mono) or 2 (stereo). */
        virtual unsigned GetChannelCount() const = 0;

        /** Get sampling rate . */
        virtual unsigned GetSamplingRate() const = 0;
};

/**
 * Standard loader, supports wav files only.
 */
class SoundFileLoader: public AbstractSoundLoader
{

    public:

        SoundFileLoader() {};
        virtual ~SoundFileLoader();

        virtual bool Load(const char* path) override;
        virtual void UnLoad() override;
        virtual bool Reset() override;
        virtual size_t Get(void* samples, size_t length) override;
        unsigned GetBytesPerSample() const override;
        unsigned GetChannelCount() const override;
        unsigned GetSamplingRate() const override;

    protected:

        union SoundSamples {
                const uint8_t* bytes;   // Unsigned 8-bit samples
                const uint16_t* words;  // Signed 16-bit samples
                SoundSamples() { bytes = 0; }
                ~SoundSamples() { if (bytes) delete [] bytes;  }
        };

        /** Sound data as loaded from .wav file: */
        struct SoundData
        {
            /* .wav header information: */
            unsigned m_channels;       // num of channels (mono:1, stereo:2)
            unsigned m_samplingRate;
            unsigned m_bitsPerSample;  // 8: m_data contains bytes, 16: words
            unsigned m_samples;        // length in samples:

            /* wave data: */
            size_t m_dataBytes;
            union SoundSamples m_data; // The sound samples.
        };

        int m_next;
        std::unique_ptr<SoundData> m_osdata;
        bool LoadWAV(const uint8_t* data, size_t len);
};


#endif //  SOUND_FILE_LOADER_H
