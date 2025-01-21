/**************************************************************************
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

/**
 *  \file
 *  Implement gui.h.
 */
#include <wx/dialog.h>
#include <wx/frame.h>

#include "model/gui.h"

wxWindow* GetTopWindow() {
  auto top_window = wxWindow::FindWindowByName(kTopLevelWindowName);
  assert(top_window && "Cannot find MainWindow a k a gFrame");
  return top_window;
}

// Propagate layout resize to all parent windows
void PropagateResize(wxWindow* window) {
  wxWindow* parent = window->GetParent();
  while (parent) {
    parent->Layout();
    parent = parent->GetParent();
    if (wxTopLevelWindow* topLevel = wxDynamicCast(parent, wxTopLevelWindow)) {
      break;
    }
  }
}