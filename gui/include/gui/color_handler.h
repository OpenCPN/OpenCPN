#include <wx/string.h>
#include <wx/colour.h>

#ifndef _COLOR_HANDLER_H
#define _COLOR_HANDLER_H

wxColour GetGlobalColor(wxString colorName);
enum DialogColor {
  DLG_BACKGROUND,
  DLG_SELECTED_BACKGROUND,
  DLG_UNSELECTED_BACKGROUND,
  DLG_ACCENT,
  DLG_SELECTED_ACCENT,
  DLG_UNSELECTED_ACCENT,
  DLG_TEXT,
  DLG_HIGHLIGHT
};
wxColour GetDialogColor(DialogColor color);


#endif   // _COLOR_HANDLER_H
