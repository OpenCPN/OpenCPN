/***************************************************************************
 *   Copyright (C) 2025 by NoCodeHummel                                    *
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
 ***************************************************************************
 */
#ifndef DIALOG_ALERT_H
#define DIALOG_ALERT_H

#include <string>
#include <wx/timer.h>

#include "dialog_base.h"
#include "dialog_footer.h"

/**
 * Alert confirmation listener interface.
 *
 * @interface IAlertConfirmation dialog_alert.h "dialog_alert.h"
 */
class IAlertConfirmation {
public:
  /**
   * Destroy the IAlertConfirmation object.
   */
  virtual ~IAlertConfirmation() = default;

  /**
   * Handle confirmation response.
   * @param result User response.
   */
  virtual void OnConfirm(const bool result) = 0;
};

/**
 * A modal message dialog with confirmation button and cancel button.
 * Can be used with a listener to handle the response.
 * Alternatively the static GetConfirmation function can be used without
 * listener.
 */
class AlertDialog : public BaseDialog {
public:
  /**
   * Alert dialog with close button.
   * @param parent Parent window.
   * @param title Dialog title.
   */
  AlertDialog(wxWindow* parent, const std::string& title);

  /**
   * Alert dialog with labelled confirmation button and cancel.
   * @param parent Parent window.
   * @param title Dialog title.
   * @param action Action button label.
   */
  AlertDialog(wxWindow* parent, const std::string& title,
              const std::string& action);

  ~AlertDialog();

  /**
   * Listen for response.
   * @param listener Confirmation listener.
   */
  void SetListener(IAlertConfirmation* listener);

  /**
   * Activate timer.
   * @param seconds Timer in seconds.
   */
  void SetTimer(int seconds);

  /**
   * Set alert message.
   * @param msg Alert message.
   */
  void SetMessage(const std::string& msg);

  /**
   * Set default button (for enter).
   * @param id Button ID.
   */
  void SetDefaultButton(int id);

  /**
   * Overwrite cancel button label.
   * @param label Label for cancel button.
   */
  void SetCancelLabel(const std::string& label);

  /**
   * Show dialog and return response.
   * @return OK/Cancel response.
   */
  int ShowModal() override;

  /**
   * Helper that returns the dialog response.
   * @return OK/Cancel response.
   */
  static int GetConfirmation(wxWindow* parent, const std::string& title,
                             const std::string& action, const std::string& msg);

private:
  std::string m_action;
  wxTimer m_timer;
  IAlertConfirmation* m_listener;
  void OnClick(wxCommandEvent& event);
  void OnTimer(wxTimerEvent& evt);
};

#endif  // DIALOG_ALERT_H
