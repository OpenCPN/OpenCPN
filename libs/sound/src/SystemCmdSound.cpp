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
#include <stdlib.h>
#include <thread>

#include <wx/file.h>
#include <wx/log.h>

#include "SystemCmdSound.h"

#ifdef _WIN32
#include <windows.h>

static int do_play(const char* cmd, const char* path)
{
    wchar_t sound_path[80];
    MultiByteToWideChar( 0, 0, path, -1, sound_path, 80 );
    LPCWSTR wide_path = sound_path;

    PlaySound( wide_path, NULL, SND_FILENAME );

    return 0;
}

#else /* _WIN32, i. e. POSIX */
#include <sys/wait.h> // for WEXITSTATUS & friends

static int do_play(const char* cmd, const char* path)
{
    char buff[1024];
    snprintf(buff, sizeof( buff ), cmd, path);
    wxLogDebug("Sound command: %s", buff);

    int status = system(buff);
    if (status == -1) {
        wxLogWarning("Cannot fork process running %s", buff);
        return -1;
    }
    if (WIFEXITED(status)) {
        status = WEXITSTATUS(status);
        if (status != 0) {
            wxLogWarning("Exit code %d from command %s",
                status, buff);
        }
    } else {
        wxLogWarning("Strange return code %d (0x%x) running %s",
                     status, status, buff);
    }
    return status;
}
#endif

bool SystemCmdSound::Load(const char* path, int deviceIndex)
{
    m_path = path;
    if (deviceIndex != -1) {
        wxLogMessage("Selecting device is not supported by SystemCmdSound");
    }
    m_OK = wxFileExists(m_path);
    return m_OK;
}


bool SystemCmdSound::Stop(void)  { return false; }


bool SystemCmdSound::canPlay(void)
{
    if (m_isPlaying)
        wxLogWarning("SystemCmdSound: cannot play: already playing");
    return m_OK && !m_isPlaying;
}


void SystemCmdSound::worker(void)
{
    wxLogDebug("SystemCmdSound::worker()");
    m_isPlaying = true;
    do_play(m_cmd.c_str(), m_path.c_str());
    m_onFinished(m_callbackData);
    m_onFinished = 0;
    m_isPlaying = false;
}


bool SystemCmdSound::Play()
{
    wxLogDebug("SystemCmdSound::Play()");
    if (m_isPlaying) {
        wxLogWarning("SystemCmdSound: cannot play: already playing");
        return false;
    }
    if  (m_onFinished) {
        std::thread t([this]() { worker(); });
        t.detach();
        return true;
    }
    int r = do_play(m_cmd.c_str(), m_path.c_str());
    return r == 0;
}
