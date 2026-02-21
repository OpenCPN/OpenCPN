/**************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 * \file
 *
 * Global color handling by name.
 */

#ifndef _COLOR_HANDLER_H
#define _COLOR_HANDLER_H

#include <wx/colour.h>
#include "color_types.h"

extern wxColorHashMap* pcurrent_user_color_hash; /**< Global instance */

wxColour GetGlobalColor(wxString colorName);
/**
 * Enumeration of color types used in dialogs.
 * This enum defines different color roles used in the application's dialogs.
 */
enum DialogColor {
  DLG_BACKGROUND,            /**< Background color of the dialog */
  DLG_SELECTED_BACKGROUND,   /**< Background color for selected items */
  DLG_UNSELECTED_BACKGROUND, /**< Background color for unselected items */
  DLG_ACCENT,                /**< Accent color */
  DLG_SELECTED_ACCENT,       /**< Accent color for selected items */
  DLG_UNSELECTED_ACCENT,     /**< Accent color for unselected items */
  DLG_TEXT,                  /**< Text color */
  DLG_HIGHLIGHT              /**< Highlight color */
};

/**
 * Retrieves a dialog color based on its role in the application's dialogs.
 */
wxColour GetDialogColor(DialogColor color);

#endif  // _COLOR_HANDLER_H
