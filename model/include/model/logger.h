
/**************************************************************************
 *   Copyright (C) 2013 David S. Register                                  *
 *   Copyright (C) 2022 - 2024 Alec Leamas                                 *
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
 ***************************************************************************/

/**
 * \file
 * Enhanced logging interface on top of wx/log.h.
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

#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>

#include <wx/log.h>

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

/** Filter logging every nth message */
class CountedLogFilter {
public:
  CountedLogFilter(unsigned n, wxLogLevel level = wxLOG_Message)
      : m_count(n), m_level(level), m_not_logged(0) {}

  /** Log a repeated message after suppressing n ones. */
  void Log(const std::string& message);

private:
  const unsigned m_count;
  const wxLogLevel m_level;
  unsigned m_not_logged;
};

/** Filter logging repeated message with specified interval. */
class TimedLogFilter {
public:
  TimedLogFilter(const std::chrono::seconds& interval,
                 wxLogLevel level = wxLOG_Message)
      : m_interval(interval), m_level(level), m_not_logged(0) {}

  /** Log a repeated message after interval seconds. */
  void Log(const std::string& message);

private:
  const std::chrono::seconds m_interval;
  const wxLogLevel m_level;
  std::chrono::time_point<std::chrono::steady_clock> m_last_logged;
  unsigned m_not_logged;
};

#endif  // LOGGER_H
