/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Application print support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifndef _OCPN_PRINT_H__
#define _OCPN_PRINT_H__

#include <wx/dc.h>
#include <wx/bitmap.h>
#include <wx/print.h>
#include <wx/string.h>

class MyPrintout : public wxPrintout {
public:
  MyPrintout(const wxChar *title = _T("My printout")) : wxPrintout(title) {}
  virtual bool OnPrintPage(int page);
  virtual bool HasPage(int page);
  virtual bool OnBeginDocument(int startPage, int endPage);
  virtual void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom,
                           int *selPageTo);

  void DrawPageOne(wxDC *dc);

  void GenerateGLbmp(void);

private:
  wxBitmap m_GLbmp;
};

#endif  //  _OCPN_PRINT_H__
