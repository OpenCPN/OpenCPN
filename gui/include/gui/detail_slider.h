/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Chart display details slider
 */

#ifndef __DETAILSLIDE_H
#define __DETAILSLIDE_H

#include <wx/event.h>
#include <wx/frame.h>
#include <wx/slider.h>
#include <wx/string.h>

#include "chartdb.h"

class PopUpDSlide;                      // forward
extern PopUpDSlide* pPopupDetailSlider; /**< Global instance */

/**
 * A popup frame containing a detail slider for chart display.
 */
class PopUpDSlide : public wxFrame {
public:
  PopUpDSlide(wxWindow* parent, wxWindowID id, ChartTypeEnum ChartType,
              ChartFamilyEnum ChartF, const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize, long style = 0,
              const wxString& title = "");

  ~PopUpDSlide();

  void Init();
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
