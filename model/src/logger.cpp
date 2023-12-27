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
#include <map>
#include <sstream>
#include <string>

#include <wx/datetime.h>
#include <wx/filename.h>

#include "model/logger.h"

const wxLogLevel OcpnLog::LOG_BADLEVEL = wxLOG_Max + 1;

const static std::map<wxLogLevel, const char*> name_by_level = {
    {wxLOG_FatalError, "FATALERR"}, {wxLOG_Error, "ERROR"},
    {wxLOG_Warning, "WARNING"},     {wxLOG_Message, "MESSAGE"},
    {wxLOG_Status, "STATUS"},       {wxLOG_Info, "INFO"},
    {wxLOG_Debug, "DEBUG"},         {wxLOG_Trace, "TRACE"},
    {wxLOG_Progress, "PROGRESS"}};

static std::map<std::string, wxLogLevel> level_by_name;

static std::string basename(const std::string path) {
  size_t pos = path.rfind(wxFileName::GetPathSeparator(), path.length());
  return pos == std::string::npos ? path : path.substr(pos + 1);
}

static void init_level_by_name() {
  for (auto it = name_by_level.begin(); it != name_by_level.end(); it++) {
    level_by_name[std::string(it->second)] = it->first;
  }
}

static std::string timeStamp() {
  wxDateTime now = wxDateTime::UNow();
  std::stringstream stamp;
  stamp << std::setfill('0') << std::setw(2) << now.GetHour() << ":"
        << std::setw(2) << now.GetMinute() << ":" << std::setw(2)
        << now.GetSecond() << "." << std::setw(3) << now.GetMillisecond();
  return stamp.str();
}

std::string OcpnLog::level2str(wxLogLevel level) {
  auto search = name_by_level.find(level);
  return search == name_by_level.end() ? "Unknown level" : search->second;
}

wxLogLevel OcpnLog::str2level(const char* string) {
  if (level_by_name.size() == 0) {
    init_level_by_name();
  }
  std::string key(string);
  std::transform(key.begin(), key.end(), key.begin(), ::toupper);
  auto search = level_by_name.find(key);
  return search == level_by_name.end() ? LOG_BADLEVEL : search->second;
}

OcpnLog::OcpnLog(const char* path) {
  log.open(path, std::fstream::out | std::fstream::app);
}

OcpnLog::~OcpnLog() { log.close(); }

void OcpnLog::Flush() {
  wxLog::Flush();
  log.flush();
}

void OcpnLog::DoLogRecord(wxLogLevel level, const wxString& msg,
                          const wxLogRecordInfo& info) {
  std::ostringstream oss;
  oss << timeStamp() << " " << std::setw(7) << level2str(level) << " "
      << basename(info.filename) << ":" << info.line << " " << msg << std::endl;
  log << oss.str();
}

Logger::Logger() : info("", 0, "", ""), level(wxLOG_Info){};

Logger::~Logger() {
  wxString msg(os.str());
  wxLog* log = wxLog::GetActiveTarget();
  auto ocpnLog = dynamic_cast<OcpnLog*>(log);
  if (ocpnLog) {
    ocpnLog->LogRecord(level, msg, info);
  }
}

std::ostream& Logger::get(wxLogLevel l, const char* path, int line) {
  info.filename = path;
  info.line = line;
  level = l;
  return os;
}

void Logger::logRecord(wxLogLevel level, const char* msg,
                       const wxLogRecordInfo info) {
  wxLog* log = wxLog::GetActiveTarget();
  auto ocpnLog = dynamic_cast<OcpnLog*>(log);
  if (ocpnLog) {
    ocpnLog->LogRecord(level, wxString(msg), info);
  }
}

void Logger::logMessage(wxLogLevel level, const char* path, int line,
                        const char* fmt, ...) {
  wxLogRecordInfo info(__FILE__, __LINE__, "", "");
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  auto log = dynamic_cast<OcpnLog*>(wxLog::GetActiveTarget());
  if (log) {
    log->LogRecord(level, buf, info);
  }
}
