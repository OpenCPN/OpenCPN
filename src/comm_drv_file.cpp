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
#include "ocpn_utils.h"

using namespace std;

FileCommDriver::FileCommDriver(const string& opath, const string& ipath)
    : AbstractCommDriver(NavAddr::Bus::TestBus, opath),
      output_path(opath),
      input_path(ipath) {
}

FileCommDriver::FileCommDriver(const string& opath, const string& ipath,
                               std::shared_ptr<DriverListener> l)
    : FileCommDriver(opath, ipath) {
  listener = l;
}



void FileCommDriver::send_message(const NavMsg& msg, const NavAddr& addr) {
  cerr << "Opening output file\n" << flush;
  ofstream f;
  f.open(output_path, ios::app);
  if (!f.is_open()) {
    wxLogWarning("Cannot open file %s for writing", output_path.c_str());
    return;
  }
  f << msg.to_string();
  f.close();
}

static vector<unsigned char> HexToChar(string hex) {
  if (hex.size() % 2 == 1) hex = string("0") + hex;
  vector<unsigned char> chars;
  for (int i = 0; i <hex.size(); i += 2) {
     istringstream ss(hex.substr(i, 2));
     unsigned ival;
     ss >> std::hex >> ival;
     chars.push_back(static_cast<unsigned char>(ival));
  }
  return chars;
}

static unique_ptr<const NavMsg> LineToMessage(const string& line) {
  auto words = ocpn::split(line.c_str(), " ");
  NavAddr::Bus bus = NavAddr::StringToBus(words[0]);
  switch (bus) {
    case NavAddr::Bus::N2000:
      if (true) {    // Create a separate scope.
        N2kId id(N2kId::StringToId(words[2]));
        vector<unsigned char> payload(HexToChar(words[3]));
        return make_unique<Nmea2000Msg>(id, payload);
      }
      break;
    case NavAddr::Bus::N0183:
      if (true) {    // Create a separate scope.
        const string id(words[2]);
        return make_unique<Nmea0183Msg>(id, words[3]);
      }
      break;
    default:
      assert(false && "Not implemented message parsing");
      return make_unique<NullNavMsg>();
      break;
  }
  return make_unique<NullNavMsg>();   // for the compiler.
}

void FileCommDriver::Activate() {
  CommDriverRegistry::getInstance()->Activate(shared_from_this());
  if (!listener) {
    wxLogWarning("No listener defined, ignoring input");
    cerr << "No listener defined, ignoring input\n";
  }
  if (input_path != "") {
    ifstream f(input_path);
    string line;
    while (getline(f, line)) {
      auto msg = LineToMessage(line);
      if (msg->bus != NavAddr::Bus::Undef && listener)
        listener->notify(move(msg));
    }
  }
}

