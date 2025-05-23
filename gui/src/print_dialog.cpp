
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

#include "gui_lib.h"
#include "print_dialog.h"

PrintDialog::PrintDialog() {
  wxPrintData data;
  m_initialized = false;
}

PrintDialog& PrintDialog::GetInstance() {
  static PrintDialog instance;
  return instance;
}

void PrintDialog::Initialize(wxPrintOrientation orientation) {
  if (!m_initialized) {
    wxPrintData data;
    data.SetOrientation(orientation);
    m_print_data = wxPrintDialogData(data);
    m_initialized = true;
  }
}

void PrintDialog::EnablePageNumbers(bool enable) {
  m_print_data.EnablePageNumbers(enable);
}

void PrintDialog::Print(wxWindow* parent, wxPrintout* output) {
  assert(m_initialized);
  wxPrinter printer(&m_print_data);
  if (!printer.Print(parent, output, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      OCPNMessageBox(
          NULL,
          _("There was a problem printing.\nPerhaps your current printer is "
            "not set correctly?"),
          _("OpenCPN"), wxOK);
    }

  } else {
    wxPrintData data = printer.GetPrintDialogData().GetPrintData();
    m_print_data.SetPrintData(data);
  }
}
