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

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <ostream>
#include <sstream>
#include <string>

#include <wx/log.h>

/**
 * Logging interface.
 *
 * The OcpnLog acts as the active wxLog target: it formats and prints
 * log messages, overriding the default setup.
 *
 * The Logger acts as a frontend to the wxLog logging providing C-style
 * macros used like
 *
 *    LOG_INFO("Broken: %s", why);
 *
 * Here is also C++ ostream log macros used like
 *
 *    INFO_LOG << "Broken: " << why;
 *
 * These macros are very fast when logging is disabled for actual level,
 * faster then the original wxLogMessage() and friends. They do not respect
 * wxLog's component levels and trace masks, logging anything with a
 * level <= wxLog::GetLogLevel().
 */

/**
 * Customized logger class appending to a file providing:
 *   - Millisecond timestamps
 *   - Consistent tagging WARNING, ERROR, MESSAGE etc.
 *   - Filename:line info.
 */
class OcpnLog : public wxLog {
public:
  static const wxLogLevel LOG_BADLEVEL;

  /** Create logger appending to given filename. */
  OcpnLog(const char* path);

  virtual ~OcpnLog();

  void Flush() override;

  void DoLogRecord(wxLogLevel level, const wxString& msg,
                   const wxLogRecordInfo& info) override;

  static wxLogLevel str2level(const char* string);
  static std::string level2str(wxLogLevel level);

protected:
  std::ofstream log;
};

/** Transient logger class, instantiated/used by the *LOG* macros. */
class Logger {
public:
  Logger();
  ~Logger();

  /** DoLogRecord public wrapper. */
  void logRecord(wxLogLevel level, const char* msg, const wxLogRecordInfo info);

  std::ostream& get(wxLogLevel level, const char* path, int line);

  static void logMessage(wxLogLevel level, const char* path, int line,
                         const char* fmt, ...);

protected:
  std::stringstream os;
  wxLogRecordInfo info;
  wxLogLevel level;
};

#define DO_LOG_MESSAGE(level, fmt, ...)                                  \
  {                                                                      \
    if (level <= wxLog::GetLogLevel()) {                                 \
      Logger::logMessage(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    }                                                                    \
  }

#define _LOG(level)                 \
  if (level > wxLog::GetLogLevel()) \
    ;                               \
  else                              \
    Logger().get(level, __FILE__, __LINE__)

#define TRACE_LOG _LOG(wxLOG_Trace)
#define DEBUG_LOG _LOG(wxLOG_Debug)
#define INFO_LOG _LOG(wxLOG_Info)
#define MESSAGE_LOG _LOG(wxLOG_Message)
#define WARNING_LOG _LOG(wxLOG_Warning)
#define ERROR_LOG _LOG(wxLOG_Error)

#define LOG_TRACE(fmt, ...) DO_LOG_MESSAGE(wxLOG_Trace, fmt, ##__VA_ARGS__);
#define LOG_DEBUG(fmt, ...) DO_LOG_MESSAGE(wxLOG_Debug, fmt, ##__VA_ARGS__);
#define LOG_INFO(fmt, ...) DO_LOG_MESSAGE(wxLOG_Info, fmt, ##__VA_ARGS__);
#define LOG_MESSAGE(fmt, ...) DO_LOG_MESSAGE(wxLOG_Message, fmt, ##__VA_ARGS__);
#define LOG_WARNING(fmt, ...) DO_LOG_MESSAGE(wxLOG_Warning, fmt, ##__VA_ARGS__);
#define LOG_ERROR(fmt, ...) DO_LOG_MESSAGE(wxLOG_Error, fmt, ##__VA_ARGS__);

#endif  // LOGGER_H
