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
#include <windows.h>

#include <wx/file.h>
#include <wx/log.h>

#ifdef _WIN32
#define WIFEXITED(stat_val) ( (stat_val) != -1 )
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val))
#endif

#include "SystemCmdSound.h"

extern int quitflag;

static int do_play(const char* cmd, const char* path)
{
    char buff[1024];
    snprintf(buff, sizeof(buff), cmd, path);

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Start the child process. 
    int status = CreateProcessA(NULL,   // No module name (use command line)
        buff,            // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW,// No window flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi);           // Pointer to PROCESS_INFORMATION structure
    
    if (!status) {
        wxLogWarning("Cannot fork process running %s", buff);
        return -1;
    }

    int waitStatus = WaitForSingleObject(pi.hProcess, maxPlayTime);
    while (!quitflag && waitStatus == WAIT_TIMEOUT) {
        waitStatus = WaitForSingleObject(pi.hProcess, maxPlayTime);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (WIFEXITED(status)) {
        status = WEXITSTATUS(status);
        if (status != 1) {
            wxLogWarning("Exit code %d from command %s",
                         status, buff);
        }
    } else {
        wxLogWarning("Strange return code %d (0x%x) running %s", 
                     status, status, buff);
    }
    return status;
}    


bool SystemCmdSound::Load(const char* path, int deviceIndex)
{
    m_path = path;
    if (deviceIndex != -1) {
        wxLogWarning("Selecting device is not supported by SystemCmdSound");
    }
    return true;
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
    wxLogMessage("SystemCmdSound::worker()");
    m_isPlaying = true;
    do_play(m_cmd.c_str(), m_path.c_str());
    m_onFinished(m_callbackData);
    m_onFinished = 0;
    m_isPlaying = false;
}


bool SystemCmdSound::Play()
{
    wxLogInfo("SystemCmdSound::Play()");
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
