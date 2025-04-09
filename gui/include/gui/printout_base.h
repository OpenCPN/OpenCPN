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

#include <string>
#include <wx/dc.h>
#include <wx/print.h>

/**
 * Application print support.
 */
class BasePrintout : public wxPrintout {
public:
  BasePrintout(const std::string &title = _("OpenCPN print").ToStdString());

  // Methods required by wxPrintout.
  bool HasPage(int page) override;
  bool OnBeginDocument(int startPage, int endPage) override;
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom,
                   int *selPageTo) override;
  bool OnPrintPage(int page) override;

protected:
  int m_pages;
  int m_margin_x;
  int m_margin_y;

  /**
   * Called by the print framework to draw the page.
   * @param dc Device context to draw on.
   * @param page Page number to draw.
   */
  virtual void DrawPage(wxDC *dc, int page) = 0;
};

#endif  // PRINTOUT_BASE_H
