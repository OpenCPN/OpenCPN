/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_file.h
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

#include <iostream>
#include <fstream>
#include <string>

#include "wx/log.h"

#include "comm_driver.h"
#include "comm_drv_registry.h"
#include "comm_drv_file.h"

FileCommDriver::FileCommDriver(const std::string& opath,
                               const std::string& ipath)
    : AbstractCommDriver(NavAddr::Bus::TestBus, opath),
      output_path(opath),
      input_path(ipath) {
  std::cerr << "CTOR!!\n" << std::flush;
  std::cerr << "CTOR:done\n" << std::flush;
}

void FileCommDriver::send_message(const NavMsg& msg, const NavAddr& addr) {
  std::cerr << "Opening output file\n" << std::flush;
  std::ofstream f;
  f.open(output_path, std::ios::app);
  if (!f.is_open()) {
    wxLogWarning("Cannot open file %s for writing", output_path.c_str());
    return;
  }
  f << msg.to_string();
  f.close();
}

void FileCommDriver::Activate() {
  CommDriverRegistry::getInstance()->Activate(shared_from_this());
  // TODO: Read input data.
}
