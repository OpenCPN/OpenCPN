/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
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
 * Safe start without plugins dialog.
 */

#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>

#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/sizer.h>

#include "model/cmdline.h"
#include "model/ocpn_utils.h"
#include "model/safe_mode.h"

#include "dialog_alert.h"
#include "gui_lib.h"

namespace safe_mode {

static const char* LAST_RUN_ERROR_MSG =
    _("<p>The last opencpn run seems to have failed. Do you want to run\n"
      "in safe mode without plugins and other possibly problematic\n"
      "features?\n</p><br/></br><p>You may consider visiting the <a "
      "href=\"https://github.com/OpenCPN/OpenCPN/wiki/"
      "OpenCPN-5.10-known-issues\">list of known issues</a>.</p>");

/**
 * Check if the last start failed, possibly invoke user dialog and set
 * safe mode state.
 */
void check_last_start() {
  std::string path = check_file_path();
  if (!ocpn::exists(path)) {
    std::ofstream dest(path, std::ios::binary);
    dest << "Internal opencpn use" << std::endl;
    dest.close();
    return;
  }

  std::string title = _("Safe Restart").ToStdString();
  std::string action = _("Safe mode start").ToStdString();
  AlertDialog dlg(0, title, action);
  dlg.SetCancelLabel(_("Normal start").ToStdString());
  dlg.SetDefaultButton(wxID_CANCEL);
  dlg.SetInitialSize();
  dlg.SetTimer(15);

  std::stringstream html;
  html << "<html><body>";
  html << LAST_RUN_ERROR_MSG;
  html << "</body></html>";
  dlg.AddHtmlContent(html);

  int reply = dlg.ShowModal();
  safe_mode = reply == wxID_OK;
}

}  // namespace safe_mode
