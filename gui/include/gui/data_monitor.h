/***************************************************************************
 *   Copyright (C) 2025  Alec Leamas                                       *
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
 **************************************************************************/

/**
 * \file
 * New NMEA Debugger successor main window.
 */

#ifndef DATA_MONITOR_DLG__
#define DATA_MONITOR_DLG__

#include <iostream>  // debug junk
#include <functional>
#include <fstream>

#include <wx/frame.h>

#include "data_monitor_src.h"
#include "std_filesystem.h"

class DataLogger {
public:
  enum class Format { kCandump, kDefault, kCsv };

  DataLogger(wxWindow* parent, fs::path path);

  DataLogger(wxWindow* parent);

  void SetLogging(bool logging);

  void SetLogfile(fs::path path);

  void Add(std::string msg);

  void Add(const Logline& ll);

  void SetFormat(Format format);

  fs::path GetLogfile() { return m_path; }

private:
  wxWindow* m_parent;
  fs::path m_path;
  std::ofstream m_stream;
  bool m_is_logging;
  Format m_format;

  fs::path DefaultLogfile();
};

/** Overall logging handler, outputs to screen and log file. */
class DataMonitor : public wxFrame, public NmeaLog {
public:
  DataMonitor(wxWindow* parent);

  /** Add an input line to log output. */
  void Add(std::string msg);

  void Add(const Logline& ll) override;

  bool IsActive() const override;

private:
  DataMonitorSrc m_monitor_src;
  wxWindow* m_quick_filter;
  DataLogger m_logger;
};

#endif  //  DATA_MONITOR_DLG__
