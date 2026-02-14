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
 **************************************************************************/
#ifndef PRINT_DIALOG_H
#define PRINT_DIALOG_H

#include <wx/print.h>

/**
 * Handle the print process and dialog.
 */
class PrintDialog {
public:
  PrintDialog(wxPrintOrientation orientation) = delete;
  PrintDialog& operator=(const PrintDialog&) = delete;

  /**
   * Get instance to handle the print process,
   * @return Singleton instance to handle print process.
   */
  static PrintDialog& GetInstance();

  /**
   * Initialize the printer with default setup.
   * @param orientation Default page orientation.
   */
  void Initialize(wxPrintOrientation orientation);

  /**
   * Print page numbers.
   * @param enable Enable or disable page numbers.
   */
  void EnablePageNumbers(bool enable);

  /**
   * Start print process and opens the print dialog.
   * @param parent Parent window.
   * @param output Printer output.
   */
  void Print(wxWindow* parent, wxPrintout* output);

private:
  PrintDialog();

  bool m_initialized;
  wxPrintDialogData m_print_data;
};

#endif  // PRINT_DIALOG_H
