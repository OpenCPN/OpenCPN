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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * New NMEA Debugger successor main window.
 */

#ifndef DATA_MONITOR_DLG_
#define DATA_MONITOR_DLG_

#include <fstream>

#include <wx/frame.h>

#include "model/data_monitor_src.h"
#include "tty_scroll.h"
#include "std_filesystem.h"

/**
 * Internal helper class
 * \internal
 */
class DataLogger {
public:
  enum class Format { kVdr, kDefault, kCsv };

  DataLogger(wxWindow* parent, const fs::path& path);

  explicit DataLogger(wxWindow* parent);

  void SetLogging(bool logging);

  void SetLogfile(const fs::path& path);

  fs::path GetLogfile() const { return m_path; }

  void Add(const Logline& ll);

  void SetFormat(Format format);

  std::string GetFileDlgTypes() const;

  fs::path GetDefaultLogfile();

  /** Notified with new path on filename change. */
  EventVar OnNewLogfile;

private:
  wxWindow* m_parent;
  fs::path m_path;
  std::ofstream m_stream;
  bool m_is_logging;
  Format m_format;
  const NavmsgTimePoint m_log_start;
  std::string m_header;
};

/** Overall logging handler, outputs to screen and log file. */
class DataMonitor : public wxFrame, public NmeaLog {
public:
  explicit DataMonitor(wxWindow* parent);

  /** Add an input line to log output. */
  void Add(const Logline& ll) override;

  bool IsVisible() const override;

  void OnHide();

private:
  static void OnFilterListChange();
  void OnFilterUpdate(const std::string& name) const;
  void OnFilterApply(const std::string& name);

  DataMonitorSrc m_monitor_src;
  wxWindow* m_quick_filter;
  DataLogger m_logger;
  ObsListener m_filter_list_lstnr;
  ObsListener m_filter_update_lstnr;
  ObsListener m_filter_apply_lstnr;
  std::string m_current_filter;
};

#endif  //  DATA_MONITOR_DLG_
