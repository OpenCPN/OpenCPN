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

#ifndef __TTYWINDOW_H__
#define __TTYWINDOW_H__

#include <wx/frame.h>
#include <wx/bitmap.h>

class wxButton;
class wxTextCtrl;
class TTYScroll;
class WindowDestroyListener;

class TTYWindow : public wxFrame {
  DECLARE_DYNAMIC_CLASS(TTYWindow)
  DECLARE_EVENT_TABLE()

public:
  TTYWindow();
  TTYWindow(wxWindow *parent, int n_lines,
            WindowDestroyListener *listener = NULL);
  virtual ~TTYWindow();

  void Add(const wxString &line);
  void OnCloseWindow(wxCloseEvent &event);
  void Close();
  void OnPauseClick(wxCommandEvent &event);
  void OnCopyClick(wxCommandEvent &event);

protected:
  void CreateLegendBitmap();
  WindowDestroyListener *m_window_destroy_listener;
  TTYScroll *m_tty_scroll;
  wxButton *m_btn_pause;
  wxButton *m_btn_copy;
  bool m_is_paused;
  wxBitmap m_bm_legend;
  wxTextCtrl *m_filter;
};

#endif
