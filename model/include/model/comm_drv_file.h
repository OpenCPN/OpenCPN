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

#ifndef _COMM_DRV_FILE_H
#define _COMM_DRV_FILE_H

#include <memory>
#include <sstream>
#include <vector>
#include <string>

#include "model/comm_driver.h"


/** Read and write data to/from files test driver  */
class FileCommDriver : public AbstractCommDriver {
public:
  /** An instance which can write to file and play data from another. */
  FileCommDriver(const std::string& opath, const std::string& ipath,
                 DriverListener& l);

  /** A write-only instance writing to file. */
  FileCommDriver(const std::string& opath);

  virtual ~FileCommDriver() = default;

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;

  void Activate() override;

  virtual std::shared_ptr<NavAddr> GetAddress();

private:
  std::string output_path;
  std::string input_path;
  DriverListener& listener;
};

#endif  // COMM_DRV_FILE_H
