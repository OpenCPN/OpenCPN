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

#include <wx/wx.h>
#include <wx/display.h>

#include "ui_utils.h"

// Define custom events
wxDEFINE_EVENT(EVT_LAYOUT_RESIZE, wxCommandEvent);

int GUI::GetSpacing(wxWindow* ctx, int factor) {
#if wxCHECK_VERSION(3, 2, 0)
  return ctx->FromDIP(kSpacing * factor);
#else
  return kSpacing * factor;
#endif
}

void GUI::LayoutResizeEvent(wxWindow* ctx) {
  wxCommandEvent event(EVT_LAYOUT_RESIZE, ctx->GetId());
  wxPostEvent(ctx, event);
}

GUI::Breakpoint GUI::GetScreenSize(wxRect* rect) {
  if (rect->GetWidth() <= static_cast<int>(Breakpoint::kSmall) ||
      rect->GetHeight() <= static_cast<int>(Breakpoint::kSmall)) {
    return Breakpoint::kExtraSmall;
  } else if (rect->GetWidth() < static_cast<int>(Breakpoint::kMedium)) {
    return Breakpoint::kSmall;
  } else if (rect->GetWidth() < static_cast<int>(Breakpoint::kLarge)) {
    return Breakpoint::kMedium;
  } else if (rect->GetWidth() < static_cast<int>(Breakpoint::kExtraLarge)) {
    return Breakpoint::kLarge;
  } else {
    return Breakpoint::kExtraLarge;
  }
}
