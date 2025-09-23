/**************************************************************************
 *   Copyright (C) 2022 Alec Leamas                                        *
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

#ifndef LOAD_ERRORS_DLG_H_
#define LOAD_ERRORS_DLG_H_

#include <wx/event.h>
#include <wx/window.h>

#include "observable_evtvar.h"

/**
 * Run a dialog if there are load errors encountered in plugin_loader
 */
class LoadErrorsDlgCtrl : public wxEvtHandler {
public:
  LoadErrorsDlgCtrl(wxWindow* parent);

private:
  ObservableListener load_complete_listener;
  wxWindow* m_parent;
};

#endif  //  LOAD_ERRORS_DLG_H_
