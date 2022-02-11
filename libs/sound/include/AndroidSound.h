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

#ifndef ANDROID_SOUND_H__
#define ANDROID_SOUND_H__

#include <mutex>
#include <condition_variable>

#include "OCPN_Sound.h"
#include "androidUTIL.h"

/**
 * Sound backend supports synchronous mode on Android devices.
 */

class AndroidSound: public OcpnSound
{
    public:
        ~AndroidSound();

        bool Load(const char*, int deviceIndex = -1) override;
        bool Reset() /*override*/ { return true; };
        bool Play() override;
        bool Stop() override;
        void SetFinishedCallback(AudioDoneCallback cb, void* userData);
        void OnSoundDone();

    protected:
        bool canPlay();
        std::string m_soundfile;
        bool m_isPlaying;

   private:
        std::mutex mtx;
        std::condition_variable done;
};


#endif // ANDROID_SOUND_H__
