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
 * GUI library related utils.
 */
#include <wx/window.h>

/**
 * Organizes constant variables and methods.
 */
namespace GUI {

/**
 * Default spacing in pixels.
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

void PropagateResize(wxWindow* window);
}  // namespace GUI
