/***************************************************************************
 *   Copyright (C) 2025  Alec Leamas                                       *
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

#include <functional>

#include <wx/button.h>
#include <wx/window.h>

/**
 *
 * Two state button showing either an edit
 * \image{inline}  html ./edit-button-2.png "Edit"
 * or done icon
 * \image{inline} html ./edit-button-1.png "Done"
 *
 * Example: filter_dlg.cpp
 *  .
 */
class EditButton : public wxButton {
public:
  /**
   * Create a new instance.
   * @param parent Containing window.
   * @param id Window id, possibly wxID_ANY
   * @param on_click Callback invoked when user clicks on button.
   */
  EditButton(wxWindow* parent, int id, std::function<void()> on_click);

  /**
   * Set icon to either pen or checkmark.
   * @param is_editing If true set icon to checkmark, else set it ri pen.
   */
  void SetIcon(bool is_editing);

private:
  std::function<void()> m_on_click;
};
