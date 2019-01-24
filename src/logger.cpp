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

#include <iomanip>
#include <sstream>
#include <string>

#include <wx/time.h>
#include <wx/datetime.h>
#include <wx/filename.h>

#include "logger.h"

const wxLogLevel OcpnLog::LOG_BADLEVEL = wxLOG_Max + 1;


static std::string basename(const std::string path)
{
    size_t pos = path.rfind(wxFileName::GetPathSeparator(), path.length());
    return pos == std::string::npos ? path : path.substr(pos + 1);
}


static std::string timeStamp()
{
    wxDateTime now = wxDateTime::UNow();
    std::stringstream stamp;
    stamp << std::setfill('0')
          << std::setw(2) << now.GetHour() << ":"
          << std::setw(2) << now.GetMinute() << ":"
          << std::setw(2) << now.GetSecond() << "."
          << std::setw(3) << now.GetMillisecond();
    return stamp.str();
}


static std::string level2str(wxLogLevel level)
{
    switch (level) {
        case wxLOG_FatalError:
            return "FATALERR";
        case wxLOG_Error:
            return "ERROR";
        case wxLOG_Warning:
            return "WARNING ";
        case wxLOG_Message:
            return "MESSAGE";
        case wxLOG_Status:
            return "STATUS";
        case wxLOG_Info:
            return "INFO";
        case wxLOG_Debug:
            return "DEBUG";
        case wxLOG_Trace:
            return "TRACE";
        case wxLOG_Progress:
            return "PROGRESS";
        default:
            break;
    }
    return "Unknown level";
}


wxLogLevel OcpnLog::str2level(const char* string)
{
    std::string level(string);
    for (auto& c: level)
        c = tolower(c);
    if (level == "fatalerror")
        return wxLOG_FatalError;
    if (level == "error")
        return wxLOG_Error;
    if (level == "warning")
        return wxLOG_Warning;
    if (level == "message")
        return wxLOG_Message;
    if (level == "info")
        return wxLOG_Info;
    if (level == "debug")
        return wxLOG_Debug;
    if (level == "trace")
        return wxLOG_Trace;
    if (level == "progress")
        return wxLOG_Progress;
    return LOG_BADLEVEL;
}



OcpnLog::OcpnLog(const char* path)
{
    log.open(path, std::fstream::out | std::fstream::app);
}


OcpnLog::~OcpnLog()
{
    log.close();
}


void OcpnLog::DoLogRecord(wxLogLevel level,
		          const wxString& msg,
		          const wxLogRecordInfo& info)
{
    log << timeStamp() << " "
        << std::setw(7) << level2str(level) << " "
        << basename(info.filename) << ":" << info.line << " "
        << msg << std::endl;
}
