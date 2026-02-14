/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Wrapper for creating a N0183DlgCtx based on global vars
 * Author:   Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2023 by Alec Leamas
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
#ifndef _N0183_DLG_CTX_FACTORY_H__
#define _N0183_DLG_CTX_FACTORY_H__

#include <string>

#include <wx/string.h>

#include "gui_lib.h"
#include "SendToGpsDlg.h"
#include "model/comm_n0183_output.h"

static bool ConfirmOverwrite() {
  int r =
      OCPNMessageBox(NULL, _("Overwrite Garmin device route number 1?"),
                     _("OpenCPN Message"), wxOK | wxCANCEL | wxICON_QUESTION);
  return r == wxID_OK;
}

static N0183DlgCtx GetDialogCtx(SendToGpsDlg* dialog) {
  N0183DlgCtx dlg_ctx;
  dlg_ctx.set_value = [dialog](int v) {
    if (!dialog || !dialog->GetProgressGauge()) return;
    dialog->GetProgressGauge()->SetValue(v);
    dialog->GetProgressGauge()->Refresh();
    dialog->GetProgressGauge()->Update();
  };
  dlg_ctx.set_range = [dialog](int r) {
    if (!dialog || !dialog->GetProgressGauge()) return;
    dialog->GetProgressGauge()->SetRange(r);
  };
  dlg_ctx.pulse = [dialog](void) {
    if (!dialog || !dialog->GetProgressGauge()) return;
    dialog->GetProgressGauge()->Pulse();
  };
  dlg_ctx.set_message = [dialog](const std::string& s) {
    dialog->SetMessage(wxString(s));
  };
  dlg_ctx.confirm_overwrite = []() { return ConfirmOverwrite(); };
  return dlg_ctx;
}

#endif  //  _N0183_DLG_CTX_FACTORY_H__
