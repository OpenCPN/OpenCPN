#include <wx/log.h>

#include "color_handler.h"
#include "s52plib.h"
#include "model/ocpn_types.h"

extern s52plib *ps52plib;
extern wxColorHashMap* pcurrent_user_color_hash;


wxColour GetGlobalColor(wxString colorName) {
  wxColour ret_color;

  //    Use the S52 Presentation library if present
  if (ps52plib) ret_color = ps52plib->getwxColour(colorName);
  if (!ret_color.Ok() && pcurrent_user_color_hash)
    ret_color = (*pcurrent_user_color_hash)[colorName];

  //    Default
  if (!ret_color.Ok()) {
    ret_color.Set(128, 128, 128);  // Simple Grey
    wxLogMessage(_T("Warning: Color not found ") + colorName);
    // Avoid duplicate warnings:
    if (pcurrent_user_color_hash)
      (*pcurrent_user_color_hash)[colorName] = ret_color;
  }
  return ret_color;
}

wxColour GetDialogColor(DialogColor color) {
  wxColour col = *wxRED;

  bool bUseSysColors = false;
  bool bIsDarkMode = false;
#ifdef __WXOSX__
  if (wxPlatformInfo::Get().CheckOSVersion(10, 14)) bUseSysColors = true;
#endif
#ifdef __WXGTK__
  bUseSysColors = true;
#endif

  if (bUseSysColors) {
    wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
    if (bg.Red() < 128) {
      bIsDarkMode = true;
    }
  }

  switch (color) {
    case DLG_BACKGROUND:
      if (bUseSysColors && bIsDarkMode) {
        col = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
      } else {
        col = GetGlobalColor("DILG0");
      }
      break;
    case DLG_SELECTED_BACKGROUND:
      if (bUseSysColors && bIsDarkMode) {
        col = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
      } else {
        col = GetGlobalColor("DILG1");
      }
      break;
    case DLG_UNSELECTED_BACKGROUND:
      if (bUseSysColors && bIsDarkMode) {
        col = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
      } else {
        col = GetGlobalColor("DILG0");
      }
      break;
    case DLG_ACCENT:
    case DLG_SELECTED_ACCENT:
      if (bUseSysColors && bIsDarkMode) {
        col = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
      } else {
        col = GetGlobalColor("DILG3");
      }
      break;
    case DLG_UNSELECTED_ACCENT:
      if (bUseSysColors && bIsDarkMode) {
        col = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
      } else {
        col = GetGlobalColor("DILG1");
      }
      break;
    case DLG_TEXT:
      if (bUseSysColors && bIsDarkMode) {
        col = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
      } else {
        col = GetGlobalColor("DILG3");
      }
      break;
    case DLG_HIGHLIGHT:
      if (bUseSysColors && bIsDarkMode) {
        col = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
      } else {
        col = GetGlobalColor("UIBCK");
      }
      break;
  }
  return col;
}



