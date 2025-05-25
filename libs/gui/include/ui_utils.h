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
#ifndef UI_UTILS_H
#define UI_UTILS_H

/**
 * \file
 * GUI library utils and events.
 */
#include <set>
#include <wx/window.h>
#include <wx/scrolwin.h>
#include <wx/event.h>

// Declare custom events
wxDECLARE_EVENT(EVT_LAYOUT_RESIZE, wxCommandEvent);

/**
 * Organizes constant variables and methods.
 */
namespace GUI {

// Screen breakpoints.
enum class Breakpoint : int {
  kExtraSmall = 1,    // extra small
  kSmall = 480,       // small
  kMedium = 767,      // medium
  kLarge = 1024,      // large
  kExtraLarge = 1440  // extra large
};

/**
 * UI guideline default spacing in pixels.
 * Use GetSpacing() for DIP.
 */
static const int kSpacing = 6;

/**
 * Check if a key exists in a set.
 * @param set Set of keys.
 * @param key An enum class key (to be cast to int).
 */
template <typename T>
bool HasKey(const std::set<int>& set, T key) {
  return set.find(static_cast<int>(key)) != set.end();
}

/**
 * Multiply default spacing with a factor,
 * and calculate device independent pixels.
 * @param ctx Window context.
 * @param int Scaling factor.
 * @return Scaling in DIP.
 */
int GetSpacing(wxWindow* ctx, int factor);

/**
 * Trigger window layout event.
 * @param ctx Window context.
 */
void LayoutResizeEvent(wxWindow* ctx);

/**
 * Get screen size breakpoint.
 * @param rect Display geometry.
 * @return Screen size breakpoint.
 */
Breakpoint GetScreenSize(wxRect* rect);
}  // namespace GUI

#endif  // UI_UTILS_H
