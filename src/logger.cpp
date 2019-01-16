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

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <streambuf>

#include <ctype.h>

#include <wx/datetime.h>

#include "OCPNPlatform.h"
#include "logger.h"



/** Use log path transformation to support both new and old style logging. */
#define LOG_TRANSITIONAL_PATH 1


extern OCPNPlatform*    g_Platform;

static LogLevel level;


static std::string timeStamp()
{
    wxDateTime now = wxDateTime::UNow();
    std::stringstream stamp;
    stamp << std::setfill('0') 
          << std::setw(2) << now.GetHour() << ":" 
          << std::setw(2) << now.GetMinute() << ":" 
          << std::setw(2) << now.GetSecond() << "." 
          << std::setw(3) << now.GetMillisecond();
    return  std::string(stamp.str());
}


static std::string basename(const std::string path) 
{
    unsigned int pos = path.rfind('/', path.length());
    if (pos == std::string::npos) {
        pos = path.rfind('\\', path.length());
    }
    return pos == std::string::npos ? path : path.substr(pos + 1);
}


LogBackend::LogBackend() : isBuffering(true) {};


LogBackend::~LogBackend()
{
    fclose(f);
}


LogBackend& LogBackend::getInstance()
{
    static LogBackend instance;
    return instance;
}


bool LogBackend::setLogfile(const char* path)
{
    f = fopen(path, "a");
    if (f == NULL) {
        return false;
    }
    time_t ticks =  wxDateTime::GetTimeNow();
    wxDateTime now = wxDateTime(ticks);
    std::string stamp(
            (now.FormatISODate() +  " " + now.FormatISOTime()).mb_str());
    fprintf(f, "------- OpenCPN restarted at %s -------\n", stamp.c_str()); 
    fprintf(f, buffer.str().c_str());
    isBuffering = false;
    return true;
}

 
void LogBackend::write(const char* text)
{
    if (isBuffering) {
        buffer << text; 
    } else {
        fprintf(f, text);
    }
}


Logger::Logger() {}


Logger::~Logger()
{
    LogBackend::getInstance().write(os.str().c_str());
    LogBackend::getInstance().write("\n");
}


static std::string level2str(LogLevel l)
{
    switch (l) {
        case LogLevel::Error:
            return std::string("Error");
        case LogLevel::Warning:
            return std::string("Warning");
        case LogLevel::Notice:
            return std::string("Notice");
        case LogLevel::Info:
            return std::string("Info");
        case LogLevel::Debug:
            return std::string("Debug");
        case LogLevel::Trace:
            return std::string("Trace");
        default:
            break;
    };
    return std::string("unknown level: ")
        + std::to_string(static_cast<int>(l));
}


LogLevel Logger::string2level(const char* string)
{
    std::string level(string);
    for (auto& c: level)
        c = tolower(c);
    if (level == "error")
        return LogLevel::Error;
    if (level == "warning")
        return LogLevel::Warning;
    if (level == "notice")
        return LogLevel::Notice;
    if (level == "info")
        return LogLevel::Info;
    if (level == "debug")
        return LogLevel::Debug;
    if (level == "trace")
        return LogLevel::Trace;
    return LogLevel::BadLevel;
}


void Logger::setLevel(LogLevel l)
{
    level = l;
}


LogLevel Logger::getLevel()
{
   return level;
}


std::ostream& Logger::get(LogLevel level)
{
    os << timeStamp() << " " << level2str(level) << ": ";
    return os;
}


std::ostream& Logger::get(LogLevel l, const char* path, int line)
{
    std::string filename(basename(path));
    os << timeStamp() << " " << level2str(l) << ": ";
    os << filename << ":" << line << " ";
    return os;
}


void Logger::write(LogLevel level, const char* path, int line,
                   const char* fmt, ...)
{
    char buf[1024];
    std::string filename(basename(path));
    snprintf(buf, sizeof(buf), "%s %s %s:%d ",
             timeStamp().c_str(), level2str(level).c_str(), filename.c_str(), line);
    LogBackend::getInstance().write(buf);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    LogBackend::getInstance().write(buf);
    LogBackend::getInstance().write("\n");
}

void Logger::write(LogLevel level, const char* file, int line,
                   wxString fmt, ...)
{
    const char* wxfmt = fmt.mb_str();
    va_list ap;
    va_start(ap, fmt);
    write(level, file, line, wxfmt, ap);
    va_end(ap);
}

