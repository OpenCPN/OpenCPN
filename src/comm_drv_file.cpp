/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_file.h -- driver reading/writing to/from files
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

#include <wx/log.h>

#include "comm_driver.h"
#include "comm_drv_registry.h"
#include "comm_drv_file.h"
#include "ocpn_utils.h"

class VoidDriverListener : public DriverListener {
  virtual void Notify(std::shared_ptr<const NavMsg> message) {}
  virtual void Notify(const AbstractCommDriver& driver) {}
};

static VoidDriverListener kVoidDriverListener;

using namespace std;

FileCommDriver::FileCommDriver(const string& opath, const string& ipath,
                               DriverListener& l)
    : AbstractCommDriver(NavAddr::Bus::TestBus, opath),
      output_path(opath),
      input_path(ipath),
      listener(l) {}

FileCommDriver::FileCommDriver(const string& opath)
    : FileCommDriver(opath, "", kVoidDriverListener) {}

std::shared_ptr<NavAddr> FileCommDriver::GetAddress() {
    return std::make_shared<NavAddr>(NavAddrTest(output_path));
}

bool FileCommDriver::SendMessage(std::shared_ptr<const NavMsg> msg,
                                 std::shared_ptr<const NavAddr> addr) {
  ofstream f;
  f.open(output_path, ios::app);
  if (!f.is_open()) {
    wxLogWarning("Cannot open file %s for writing", output_path.c_str());
    return false;
  }
  f << msg->to_string();
  f.close();
  return true;
}

static vector<unsigned char> HexToChar(string hex) {
  if (hex.size() % 2 == 1) hex = string("0") + hex;
  vector<unsigned char> chars;
  for (size_t i = 0; i < hex.size(); i += 2) {
    istringstream ss(hex.substr(i, 2));
    unsigned ival;
    ss >> std::hex >> ival;
    chars.push_back(static_cast<unsigned char>(ival));
  }
  return chars;
}

static shared_ptr<const NavMsg> LineToMessage(const string& line,
                                              std::shared_ptr<NavAddr> src) {
  auto words = ocpn::split(line.c_str(), " ");
  NavAddr::Bus bus = NavAddr::StringToBus(words[0]);
  switch (bus) {
    case NavAddr::Bus::N2000:
      if (true) {  // Create a separate scope.
        N2kName name(N2kName::Parse(words[2]));
        vector<unsigned char> payload(HexToChar(words[3]));
        return make_shared<Nmea2000Msg>(name, payload, src);
      }
      break;
    case NavAddr::Bus::N0183:
      if (true) {  // Create a separate scope.
        const string id(words[2]);
        return make_shared<Nmea0183Msg>(id, words[3], src);
      }
      break;
    default:
      std::cerr << "Cannot parse line: \"" << line << "\"\n" << flush;
      return make_shared<NullNavMsg>();
      break;
  }
  return make_shared<NullNavMsg>();  // for the compiler.
}

void FileCommDriver::Activate() {
  CommDriverRegistry::getInstance().Activate(shared_from_this());
  if (input_path != "") {
    ifstream f(input_path);
    string line;
    while (getline(f, line)) {
      auto msg = LineToMessage(line, GetAddress());
      if (msg->bus != NavAddr::Bus::Undef) listener.Notify(msg);
    }
  }
}
