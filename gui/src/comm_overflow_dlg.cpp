/***************************************************************************
 *   Copyright (C) 2024 Alec Leamas                                        *
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

/** \file  comm_oveflow_dlg.cpp Implement comm_oveflow_dlg.cpp.h. */

#include <cassert>

#include <wx/window.h>

#include "comm_overflow_dlg.h"
#include "gui_lib.h"

#include "model/comm_drv_registry.h"

static const char* const kMessage =
    _(R"---(Communication overflow detected, the system is not able to process
all input. This is not fatal, system will continue to work but
will have to discard some input data.

It is possible to control the data discarded using filtering,
see the manual.

Please review the logfile for more info on discarded messages.
)---");

static const char* const kCaption = _("Communication overflow");

CommOverflowDlg::CommOverflowDlg(wxWindow* parent) : m_parent(parent) {
  assert(parent && "Null parent window");
  auto action = [&](ObservedEvt& evt) {
    ShowDialog(evt.GetString().ToStdString());
  };
  m_listener.Init(CommDriverRegistry::GetInstance().evt_comm_overrun, action);
}

void CommOverflowDlg::ShowDialog(const std::string& msg) {
  OCPNMessageBox(m_parent, kMessage, kCaption, wxICON_INFORMATION);
}
