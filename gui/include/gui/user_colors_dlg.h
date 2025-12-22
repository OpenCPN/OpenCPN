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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

#include <wx/frame.h>
#include <wx/window.h>

#ifndef UsEr_COlOrS_DlG_H
#define UsEr_COlOrS_DlG_H

/**
 * Run the Data Monitor dialog for user defined colors.
 *
 * If the dialog does not exist it is created, otherwise it is made visible
 * by a Raise() call.
 *
 * The dialog is created as a child of given parent and normally not
 * that managed. It exists until user exits it at which point it is
 * destroyed.
 *
 * @return pointer do dialog if created, else nullptr.
 */
extern wxFrame* UserColorsDlg(wxWindow* parent);

#endif  // UsEr_COlOrS_DlG_H
