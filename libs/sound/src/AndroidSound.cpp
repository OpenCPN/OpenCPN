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

#ifndef OCPN_ANDROID_SOUND
#define OCPN_ANDROID_SOUND

#include "OCPN_Sound.h"

#ifdef __OCPN__ANDROID__

#include "AndroidSound.h"


AndroidSound::~AndroidSound()
{
    Stop();
}

void AndroidSound::SetFinishedCallback(AudioDoneCallback cb, void* userData)
{
    m_onFinished = cb;
    m_callbackData = userData;
}


bool AndroidSound::Load(const char* path, int deviceIndex)
{
    m_soundfile = path;
    m_OK = true;
    if (deviceIndex != -1) {
        wxLogWarning("Android backend does not support audio device setup.");
    }
    return true;
}


bool AndroidSound::Stop(void)
{
    //m_OK = false;
    return true;
}


bool AndroidSound::Play(void)
{
    return androidPlaySound(m_soundfile, m_onFinished, m_callbackData);
}


#endif                  // __OCPN__ANDROID__

#endif  //  OCPN_ANDROID_SOUND
