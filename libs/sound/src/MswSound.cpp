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
#include <thread>

#include <windows.h>

#include <wx/string.h>
#include <wx/log.h>

#include "MswSound.h"


bool MswSound::Load(const char* path, int deviceIndex)
{
    m_path = wxString(path).ToStdWstring();
    m_isPlaying = false;
    m_OK = true;
    return m_OK;
}


bool MswSound::Stop(void)
{
    m_isPlaying = false;
    return PlaySound(NULL, NULL, 0);
}


void MswSound::worker(void)
{
    wxLogDebug("mswSound::worker()");
    m_isPlaying = true;
    PlaySound(m_path.c_str(), NULL, SND_FILENAME);
    if  (m_onFinished) {
        m_onFinished(m_callbackData);
        m_onFinished = 0;
    }
    m_isPlaying = false;
}


bool MswSound::Play()
{
    wxLogDebug("mswSound::Play()");
    if( !m_OK || m_isPlaying) {
        wxLogWarning("MswSound: cannot play: not loaded or busy playing.");
        return false;
    }
    if  (m_onFinished) {
        std::thread t([this]() { worker(); });
        t.detach();
        return true;
    }
    return PlaySound(m_path.c_str(), NULL, SND_FILENAME);
}
