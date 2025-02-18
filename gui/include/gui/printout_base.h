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
#ifndef PRINTOUT_BASE_H
#define PRINTOUT_BASE_H

#include <memory>
#include <wx/dc.h>
#include <wx/print.h>
#include <wx/string.h>

/**
 * Application print support.
 */
class BasePrintout : public wxPrintout {
public:
  BasePrintout(const wxChar *title = _T("OpenCPN print"));

  /**
   * Set page orientation.
   * @param orientation Page orientation.
   */
  void SetOrientation(wxPrintOrientation orientation);

  /**
   * Print page numbers.
   * @param flag Page numbers flag.
   */
  void EnablePageNumbers(bool flag) { m_page_numbers = flag; };

  /**
   * Start printing.
   * @param parent Parent window.
   * @param prompt Dialog flag.
   */
  void Print(wxWindow *parent, bool prompt);

  // Methods required by wxPrintout.
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom,
                   int *selPageTo);

  // Virtual to be implemented by derived class.
  virtual bool OnPrintPage(int page) = 0;

protected:
  int m_pages;
  bool m_page_numbers;

private:
  // Remember print settings.
  static std::unique_ptr<wxPrintData> s_print_data;
};

#endif  // PRINTOUT_BASE_H
