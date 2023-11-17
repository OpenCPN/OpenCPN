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

#ifndef OCPN_SOUND_H__
#define OCPN_SOUND_H__

#include <functional>
#include <memory>
#include <string>

typedef std::function<void(void* userPtr)>  AudioDoneCallback;

/**
 * Sound  class supports playing a sound using synchronous or asynchronous
 * mode. Also supports sound device enumeration and various status
 * checks.
 *
 * Instances should normally be obtained using SoundFactory();
 */

class OcpnSound
{
    friend OcpnSound* SoundFactory(const char* system_command);

    public:

        virtual ~OcpnSound();

        /** Return number of available devices. */
        virtual int DeviceCount() const { return 1; }

        /** Return free-format info on device or "" if not available. */
        virtual std::string GetDeviceInfo(int deviceIndex) { return ""; }

        /** Return true if given device is an output device. */
        virtual bool IsOutputDevice(int deviceIndex) const { return true; }

        /**
         * Callback invoked as cb(userData) when audio stream is done and
         * drained. Should be called before Load() to be effective.
         *
         * Setting a non-null callback forces use of asynchronous mode.
         * Using a default, 0 argument restores to synchronous mode.
         *
         * Deleting a pointer owning the OcpnSound object in the callback is
         * not supported.
         *
         * NOTE: Callback might be invoked in an interrupt or thread context,
         * avoid anything which might block (I/O, memory allocation etc.).
         */
        virtual void SetFinishedCallback(AudioDoneCallback cb = 0,
                                         void* userData = 0);

        /**
         * Initiate the class, loading data from given path and using the
         * given device as output. DeviceIx == -1 implies default device.
         */
        virtual bool Load(const char* path, int deviceIx = -1) = 0;

         /**
         * De-Initiate the class, closing any open sound file,
         * preparing for a new Load() using the same OcpnSound instance
         */
        virtual void UnLoad() = 0;

        /**
         * Plays the file loaded by Load(). If a callback is defined using
         * SetFinishedCallback() the playback is asynchronous, otherwise the
         * call blocks until the playback has completed.
         *
         * If the sound has not been successfully Load()'ed or another sound
         * is currently played the request is dropped and logged returning
         * false.
         *
         * Otherwise in synchronous mode returns success/failure from
         * backend. Asynchronous mode returns true.
         */
        virtual bool Play(void) = 0;

        /**
         * Stop possible playback and unload buffers, allowed even if
         * not playing. Returns true if stopping is supported, else false.
         */
        virtual bool Stop() = 0;

        /** Reflects loading errors. */
        virtual bool IsOk() const { return m_OK; }

    protected:

        /** Default ctor. Load() must be called before actual usage. */
        OcpnSound();

        bool m_OK;
        int m_deviceIx;
        std::string m_soundfile;
        AudioDoneCallback m_onFinished;
        void* m_callbackData;

};

#endif // OCPN_SOUND_H__
