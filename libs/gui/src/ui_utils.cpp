/***************************************************************************
 *   Copyright (C) 2025 by NoCodeHummel                                    *
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

#include <wx/frame.h>
#include <wx/window.h>

#include "ui_utils.h"

int GUI::GetSpacing(wxWindow* window, int factor) {
#if wxCHECK_VERSION(3, 2, 0)
  return window->FromDIP(kSpacing * factor);
#else
  return kSpacing * factor;
#endif
}

void GUI::PropagateLayout(wxWindow* window) {
  wxWindow* parent = window->GetParent();
  parent->Layout();
  if (parent->IsTopLevel()) return;
  PropagateLayout(parent);
}
