/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCP Track printout
 * Author:   Pavel Saviankou, Sean D'Epagnier
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

#include <iostream>

#ifndef __TRACKPRINTOUT_H__
#define __TRACKPRINTOUT_H__

#include <wx/print.h>
#include <wx/datetime.h>
#include <wx/cmdline.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "ocpn_types.h"
#include "navutil.h"
#include "TrackPropDlg.h"
#include "printtable.h"
#include "ocpn_frame.h"

class MyTrackPrintout : public MyPrintout {
public:
  MyTrackPrintout(std::vector<bool> _toPrintOut, Track* track,
                  OCPNTrackListCtrl* lcPoints,
                  const wxString& title = _T( "My Track printout" ));
  virtual bool OnPrintPage(int page);
  void DrawPage(wxDC* dc);
  virtual void OnPreparePrinting();
  virtual bool HasPage(int num) { return num > 0 && num <= numberOfPages; };

  virtual void GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                           int* selPageTo);

protected:
  wxDC* myDC;
  PrintTable table;
  Track* myTrack;
  std::vector<bool> toPrintOut;  // list of fields of bool, if certain element
                                 // should be print out.
  static const int pN = 5;       // number of fields sofar
  int pageToPrint;
  int numberOfPages;
  int marginX;
  int marginY;
  int textOffsetX;
  int textOffsetY;
};

// track elements selection dialog
///@begin control identifiers
#define ID_TRACKPRINTSELECTION 9000
#define SYMBOL_TRACKPRINT_SELECTION_STYLE \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
#define SYMBOL_TRACKPRINT_SELECTION_TITLE _("Print Track Selection")
#define SYMBOL_TRACKPRINT_SELECTION_IDNAME ID_TRACKPRINTSELECTION
#define SYMBOL_TRACKPRINT_SELECTION_SIZE wxSize(750, 300)
#define SYMBOL_TRACKPRINT_SELECTION_POSITION wxDefaultPosition

#define ID_TRACKPRINT_SELECTION_OK 9001
#define ID_TRACKPRINT_SELECTION_CANCEL 9002

class TrackPrintSelection : public wxDialog {
  DECLARE_DYNAMIC_CLASS(TrackPrintSelection)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  TrackPrintSelection();
  TrackPrintSelection(
      wxWindow* parent, Track* track, OCPNTrackListCtrl* lcPoints,
      wxWindowID id = SYMBOL_TRACKPRINT_SELECTION_IDNAME,
      const wxString& caption = SYMBOL_TRACKPRINT_SELECTION_TITLE,
      const wxPoint& pos = SYMBOL_TRACKPRINT_SELECTION_POSITION,
      const wxSize& size = SYMBOL_TRACKPRINT_SELECTION_SIZE,
      long style = SYMBOL_TRACKPRINT_SELECTION_STYLE);
  ~TrackPrintSelection();

  // Creation
  bool Create(wxWindow* parent,
              wxWindowID id = SYMBOL_TRACKPRINT_SELECTION_IDNAME,
              const wxString& caption = SYMBOL_TRACKPRINT_SELECTION_TITLE,
              const wxPoint& pos = SYMBOL_TRACKPRINT_SELECTION_POSITION,
              const wxSize& size = SYMBOL_TRACKPRINT_SELECTION_SIZE,
              long style = SYMBOL_TRACKPRINT_SELECTION_STYLE);

  void CreateControls();
  void SetColorScheme(ColorScheme cs);
  void SetDialogTitle(const wxString& title);
  void OnTrackpropCancelClick(wxCommandEvent& event);
  void OnTrackpropOkClick(wxCommandEvent& event);

  // Should we show tooltips?
  static bool ShowToolTips();

  wxButton* m_CancelButton;
  wxButton* m_OKButton;

  wxCheckBox* m_checkBoxPosition;
  wxCheckBox* m_checkBoxCourse;
  wxCheckBox* m_checkBoxDistance;
  wxCheckBox* m_checkBoxTime;
  wxCheckBox* m_checkBoxSpeed;

  Track* track;
  OCPNTrackListCtrl* m_lcPoints;
};

#endif
