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

#ifndef PRINTOUT_CHART_H
#define PRINTOUT_CHART_H

#include "printout_base.h"

class ChartPrintout : public BasePrintout {
public:
  ChartPrintout() : BasePrintout(_("Chart Print").ToStdString()) {};

  bool OnPrintPage(int page);

  /**
   * In OperGL mode, make the bitmap capture of the screen before the print
   * method starts as to be sure the "Abort..." dialog does not appear on the
   * image.
   */
  void GenerateGLbmp();

private:
  wxBitmap m_gl_bmp;

  void DrawPage(wxDC *dc, int page);
};

#endif  // PRINTOUT_CHART_H
