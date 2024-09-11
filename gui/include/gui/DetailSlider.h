/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __DETAILSLIDE_H
#define __DETAILSLIDE_H

#include <wx/dialog.h>
#include <wx/frame.h>
#include "chartdb.h"

// #include "chcanv.h"

class PopUpDSlide : public wxFrame {
public:
  PopUpDSlide(wxWindow* parent, wxWindowID id, ChartTypeEnum ChartType,
              ChartFamilyEnum ChartF, const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize, long style = 0,
              const wxString& title = _T(""));

  ~PopUpDSlide(void);

  void Init(void);
  bool Create(wxWindow* parent, wxWindowID id, ChartTypeEnum ChartType,
              ChartFamilyEnum ChartF, const wxPoint& pos, const wxSize& size,
              long style, const wxString& title);

  void OnCancelClick(wxCommandEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnMove(wxMoveEvent& event);
  void OnChangeValue(wxScrollEvent& event);
  void OnClose(wxCloseEvent& event);

  wxSlider* m_p_DetailSlider;
  wxWindow* m_pparent;

private:
  ChartTypeEnum ChartType;
  ChartFamilyEnum ChartFam;

  DECLARE_EVENT_TABLE()
};

#endif  //__DETAILSLIDE_H__
