/***************************************************************************
 *   Copyright (C) 2026  Alec Leamas                                       *
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

#ifndef GuI_heLP_TooLS
#define GuI_heLP_TooLS

#include <string>

#include <wx/button.h>
#include <wx/frame.h>
#include <wx/window.h>

namespace libgui {

/**
 *  \ingroup UI-tools
 *
 * Button labeled "Toggle Help" displaying/hiding a help info frame.
 * Using wxWidgets stock item wxID_HELP to render a platform specific
 * button.
 *
 * Designed to be used in a modal dialog started using ShowModal();
 *
 * Example: filter_sentence_dlg.cpp
 *  .
 */
class ModalHelpBtn : public wxButton {
public:
  /**
   * Create a new instance.
   * @param parent Containing window.
   * @param header  Info frame header
   * @param info Info frame contents
   */
  ModalHelpBtn(wxWindow* parent, const std::string& header,
               const std::string& info);

private:
  wxFrame* m_info_frame;
};

}  // namespace libgui

/** @} */

#endif  //  GuI_heLP_TooLS
