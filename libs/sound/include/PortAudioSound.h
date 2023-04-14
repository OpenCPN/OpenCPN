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

#ifndef PORTAUDIO_SOUND_H
#define PORTAUDIO_SOUND_H

#include <atomic>

#include <portaudio.h>

#include "OCPN_Sound.h"
#include "SoundLoaderFactory.h"

/**
 * Backend based on the portaudio library and API. Supports asynchronous
 * and synchronous mode. Device enumeration is also supported.
 *
 * PortAudioSound uses a SoundLoader to load files. If libsndfile is
 * available the SndfileSoundLoader is used and a wide range of formats
 * is supported. Otherwise, using the standard loader only .wav files
 * could be used.
 */
class PortAudioSound: public OcpnSound
{
    public:
        PortAudioSound();
        ~PortAudioSound();

        int DeviceCount() const override;
        std::string GetDeviceInfo(int deviceIndex) override;
        bool IsOutputDevice(int deviceIndex) const override;
        bool Load(const char* path, int deviceIndex = -1) override;
        void UnLoad() override;
        bool Play() override;
        bool Stop() override;
        void SetFinishedCallback(AudioDoneCallback cb, void* userData = 0);

        /**
         * Invoked from PortAudio when new data is requested.
         * Possibly called from interrupt context, don't block!
         * @see: PortAudio.
         */
        int SoundCallback(
            void* outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags
        );

        /** Invoked when playback done and drained. */
        void DoneCallback(void);


    protected:
        bool SetDeviceIndex(int deviceIndex = -1);
        void lock();
        void unlock();

        PaStream* m_stream;
        std::unique_ptr<AbstractSoundLoader> m_soundLoader;
        bool m_isPaInitialized;
        bool m_isAsynch;
        std::atomic_flag m_lock;
};


#endif // PORTAUDIO_SOUND_H
