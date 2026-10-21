/***************************************************************************
 *   Copyright (C) 2026 Alec Leamas                                        *
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
 ***************************************************************************/

/**
 * \file
 *
 * Dialog and support code for editing a connection
 */

#ifndef COnn_EDIT_H
#define COnn_EDIT_H

#include <memory>

#include <wx/panel.h>
#include <wx/window.h>

#include "model/conn_params.h"

/** Callback invoked when user clicks OK button in dialog */
using ConnEditCallback =
    std::function<void(std::unique_ptr<ConnectionParams>, bool editing)>;

/** Dialog editing existing or new connection parameters. */
class AbstractConnectionEditDlg : public wxPanel {
public:
  /**
   * Set parameters to use and reset dialog.
   * @param params Either an existing ConnectionParams instance when
   *     editing or nullptr if creating a new connection.
   */
  virtual void SetParams(std::unique_ptr<ConnectionParams> params) = 0;

  /** Return edited parameters */
  virtual ConnectionParams GetEditedParams() = 0;

  static AbstractConnectionEditDlg* Create(wxWindow* parent,
                                           std::function<void()> on_cancel,
                                           ConnEditCallback on_ok);

protected:
  AbstractConnectionEditDlg(wxWindow* parent) : wxPanel(parent) {}
};

#endif  // COnn_EDIT_H
