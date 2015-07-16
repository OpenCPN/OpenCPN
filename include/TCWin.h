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

#ifndef __TCWIN_H__
#define __TCWIN_H__

#include <wx/dialog.h>
#include <wx/datetime.h>
#include <wx/timer.h>
#include <wx/list.h>

class IDX_entry;
class ChartCanvas;
class RolloverWin;
class wxTimerEvent;
class wxCommandEvent;
class wxCloseEvent;
class wxTextCtrl;
class wxButton;
class wxListBox;

WX_DECLARE_LIST(wxPoint, SplineList);           // for spline curve points

class TCWin: public wxDialog
{
public:
      TCWin(ChartCanvas *parent, int x, int y, void *pvIDX);
      ~TCWin();

      void OnSize(wxSizeEvent& event);
      void OnPaint(wxPaintEvent& event);
      void MouseEvent(wxMouseEvent& event);
	  void OnTCWinPopupTimerEvent(wxTimerEvent& event);
      void OKEvent(wxCommandEvent& event);
      void NXEvent(wxCommandEvent& event);
      void PREvent(wxCommandEvent& event);
      void OnCloseWindow(wxCloseEvent& event);

      void RePosition(void);
      
      void RecalculateSize();

private:
    wxTextCtrl  *m_ptextctrl;
    wxTimer	  m_TCWinPopupTimer;
    RolloverWin *m_pTCRolloverWin;
    int           curs_x;
    int           curs_y;
    int          m_plot_type;
    wxSize        m_tc_size;
    wxPoint       m_position;
    int           m_x;
    int           m_y;
    bool          m_created;
    int           m_tsx;
    int           m_tsy;
    
      IDX_entry   *pIDX;
      wxButton    *OK_button;
      wxButton    *NX_button;
      wxButton    *PR_button;

      int         im;
      int         ib;
      int         it;
      int         val_off;
      wxRect    m_graph_rect;


      float       tcv[26];
      wxListBox  *m_tList ;
      bool        btc_valid;
      ChartCanvas    *pParent;
      int         m_corr_mins;
      wxString    m_stz;
      int         m_t_graphday_00_at_station;
      wxDateTime  m_graphday;
      int         m_plot_y_offset;

      SplineList  m_sList;

      wxFont *pSFont;
      wxFont *pSMFont;
      wxFont *pMFont;
      wxFont *pLFont;

      wxPen *pblack_1;
      wxPen *pblack_2;
      wxPen *pblack_3;
      wxPen *pred_2;
      wxBrush *pltgray;
      wxBrush *pltgray2;

      int         m_button_height;

DECLARE_EVENT_TABLE()
};


#endif
