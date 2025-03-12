/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#include "ocpn_frame.h"
#include "printout_base.h"

std::unique_ptr<wxPrintData> BasePrintout::s_print_data = nullptr;

BasePrintout::BasePrintout(const wxChar* title)
    : m_page_numbers(false), m_pages(1) {}

void BasePrintout::SetOrientation(wxPrintOrientation orientation) {
  if (!s_print_data) {
    s_print_data = std::unique_ptr<wxPrintData>(new wxPrintData());
    s_print_data->SetOrientation(orientation);
  }
}

void BasePrintout::Print(wxWindow* parent, bool prompt) {
  wxPrintDialogData print_data(*s_print_data);
  print_data.EnablePageNumbers(m_page_numbers);

  wxPrinter printer(&print_data);
  if (!printer.Print(parent, this, prompt)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      OCPNMessageBox(
          NULL,
          _("There was a problem printing.\nPerhaps your current printer is "
            "not set correctly?"),
          _T("OpenCPN"), wxOK);
    }
  } else {
    s_print_data = std::make_unique<wxPrintData>(
        printer.GetPrintDialogData().GetPrintData());
  }
}

bool BasePrintout::HasPage(int page) { return page > 0 && page <= m_pages; }

bool BasePrintout::OnBeginDocument(int startPage, int endPage) {
  if (!wxPrintout::OnBeginDocument(startPage, endPage)) return false;
  return true;
}

void BasePrintout::GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                               int* selPageTo) {
  *minPage = 1;
  *maxPage = m_pages;
  *selPageFrom = 1;
  *selPageTo = m_pages;
}
