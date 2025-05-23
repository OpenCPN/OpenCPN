#include <wx/string.h>
#include <wx/colour.h>

#ifndef _COLOR_HANDLER_H
#define _COLOR_HANDLER_H

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
