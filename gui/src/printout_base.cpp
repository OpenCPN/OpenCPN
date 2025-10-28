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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement printout_base.h -- print support abstract base class
 */

#include "gl_headers.h"  // Must come before anything using GL stuff

#include "ocpn_frame.h"
#include "printout_base.h"

BasePrintout::BasePrintout(const std::string& title)
    : wxPrintout(title), m_pages(1), m_margin_x(100), m_margin_y(100) {}

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

bool BasePrintout::OnPrintPage(int page) {
  wxDC* dc = GetDC();
  if (dc && page <= m_pages) {
    DrawPage(dc, page);
    return true;
  } else
    return false;
}
