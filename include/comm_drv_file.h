/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Test driver which reads and writes data to/from files.
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#include <memory>
#include <sstream>
#include <vector>
#include <string>

#include <wx/event.h>
#include <wx/jsonreader.h>

#include "comm_driver.h"

#ifndef _COMM_DRV_FILE_H
#define _COMM_DRV_FILE_H

/** Read and write data to/from files test driver  */
class FileCommDriver : public AbstractCommDriver {
public:
  /** Ctor, the argument are paths to files for input/output.  */
  FileCommDriver(const std::string& opath, const std::string& ipath);

  void send_message(const NavMsg& msg, const NavAddr& addr);

  void Activate();

  void set_listener(std::shared_ptr<const DriverListener> l) { listener = l; }

private:
  std::shared_ptr<const DriverListener> listener;
  std::string output_path;
  std::string input_path;
};

#endif  // COMM_DRV_FILE_H
