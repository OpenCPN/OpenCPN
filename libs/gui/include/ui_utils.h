/***************************************************************************
 *   Copyright (C) 2025 NoCodeHummel                                       *
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
 * \file
 * GUI library utils and events.
 */
#include <wx/window.h>
#include <wx/scrolwin.h>
#include <wx/event.h>

// Declare custom events
wxDECLARE_EVENT(EVT_LAYOUT_RESIZE, wxCommandEvent);

/**
 * Organizes constant variables and methods.
 */
namespace GUI {

/**
 * UI guideline default spacing in pixels.
 * Use GetSpacing() for DIP.
 */
static const int kSpacing = 6;

/**
 * Multiply default spacing with a factor,
 * and calculate device independent pixels.
 * @param window Owner class.
 * @param int Scaling factor.
 * @return Scaling in DIP.
 */
int GetSpacing(wxWindow* window, int factor);

/**
 * Trigger window layout event.
 * @param ctx Window context.
 */
void LayoutResizeEvent(wxWindow* ctx);
}  // namespace GUI
