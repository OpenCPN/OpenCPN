/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _NAVICORADARINFO_H_
#define _NAVICORADARINFO_H_

#include <wx/tokenzr.h>
#include "socketutil.h"

PLUGIN_BEGIN_NAMESPACE

class NavicoRadarInfo {
 public:
  wxString serialNr;                 // Serial # for this radar
  NetworkAddress spoke_data_addr;    // Where the radar will send data spokes
  NetworkAddress report_addr;        // Where the radar will send reports
  NetworkAddress send_command_addr;  // Where displays will send commands to the radar

  wxString to_string() const {
    if (spoke_data_addr.IsNull() && serialNr.IsNull()) {
      return wxT("");
    }
    return wxString::Format(wxT("%s/%s/%s/%s"), serialNr, spoke_data_addr.to_string(), report_addr.to_string(),
                            send_command_addr.to_string());
  }

  NavicoRadarInfo() {}

  NavicoRadarInfo(wxString &str) {
    wxStringTokenizer tokenizer(str, "/");

    if (tokenizer.HasMoreTokens()) {
      serialNr = tokenizer.GetNextToken();
    }
    if (tokenizer.HasMoreTokens()) {
      spoke_data_addr = NetworkAddress(tokenizer.GetNextToken());
    }
    if (tokenizer.HasMoreTokens()) {
      report_addr = NetworkAddress(tokenizer.GetNextToken());
    }
    if (tokenizer.HasMoreTokens()) {
      send_command_addr = NetworkAddress(tokenizer.GetNextToken());
    }
  }
  bool operator == (NavicoRadarInfo inf) {
    if (serialNr == inf.serialNr && report_addr == inf.report_addr && spoke_data_addr == inf.spoke_data_addr
      && send_command_addr == inf.send_command_addr) {
      return true;
    }
    else {
      return false;
    }
  };
};

PLUGIN_END_NAMESPACE

#endif /* _NAVICORADARINFO_H_ */
