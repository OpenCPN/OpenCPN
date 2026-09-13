/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2026 Alec Leamas                                        *
 *
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
 *  \file
 *
 *  Implement welcome.h -- welcome to OpenCPN stuff
 */

#include "welcome.h"

#include <wx/msgdlg.h>
#include <wx/string.h>

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

static const char *const kMessage = _(R"(
OpenCPN is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.

See the GNU General Public License for more details.

OpenCPN must only be used in conjunction with approved
paper charts and traditional methods of navigation.

DO NOT rely upon OpenCPN for safety of life or property.

Please click "Agree" and proceed, or "Cancel" to quit.)");

class WelcomeDlg : public wxMessageDialog {
public:
  WelcomeDlg(wxWindow *parent)
      : wxMessageDialog(parent, kMessage, _("Welcome to OpenCPN"),
                        wxOK | wxCANCEL) {
    SetOKCancelLabels(_("Agree"), _("Cancel"));
  }
};

#ifdef __ANDROID__
bool ShowNavWarning(wxWindow *parent) {
  wxString vs = wxString::Format(" .. Version %s", VERSION_FULL);
  androidShowDisclaimer(_("OpenCPN for Android") + vs, kNavWarning);
  return true;
}
#else
bool ShowNavWarning(wxWindow *parent) {
  WelcomeDlg dlg(parent);
  int agreed = dlg.ShowModal();
  return agreed == wxID_OK;
}
#endif
