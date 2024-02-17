/***************************************************************************
 *
 * Project:  OpenCPN
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

#ifndef __AISTARGETQUERYDIALOG_H__
#define __AISTARGETQUERYDIALOG_H__

#include <wx/frame.h>
#include <wx/sizer.h>

#include "model/ocpn_types.h"
#include "color_types.h"

#define AIS_TARGET_QUERY_STYLE wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT

class wxHtmlWindow;
class AisTargetData;

class AISTargetQueryDialog : public wxFrame {
  DECLARE_CLASS(AISTargetQueryDialog)
  DECLARE_EVENT_TABLE()
public:
  /// Constructors

  AISTargetQueryDialog();
  AISTargetQueryDialog(wxWindow* parent, wxWindowID id = wxID_ANY,
                       const wxString& caption = _("Object Query"),
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       long style = AIS_TARGET_QUERY_STYLE);

  ~AISTargetQueryDialog();
  /// Initialise our variables
  void Init();

  /// Creation
  bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
              const wxString& caption = _("Object Query"),
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = AIS_TARGET_QUERY_STYLE);

  void OnClose(wxCloseEvent& event);
  void OnIdOKClick(wxCommandEvent& event);
  void OnIdWptCreateClick(wxCommandEvent& event);
  void OnIdTrkCreateClick(wxCommandEvent& event);
  void OnMove(wxMoveEvent& event);
  void OnSize(wxSizeEvent& event);
  void AdjustBestSize(AisTargetData* td);
  void CreateControls();
  void RenderHTMLQuery(AisTargetData* td);
  void OnKey(wxKeyEvent& ke);

  void SetText(const wxString& text_string);
  void SetColorScheme(ColorScheme cs);

  void RecalculateSize(void);
  void SetAutoCentre(bool bval) { m_bautoCentre = bval; }
  void SetAutoSize(bool bval) { m_bautosize = bval; }

  void UpdateText(void);
  void SetMMSI(int mmsi);
  int GetMMSI(void) { return m_MMSI; }

  //    Data
  int m_MMSI;
  wxHtmlWindow* m_pQueryTextCtl;
  ColorScheme m_colorscheme;
  wxBoxSizer* m_pboxSizer;
  int m_nl;
  wxButton* m_okButton;
  wxButton* m_createWptBtn;
  wxButton* m_createTrkBtn;
  int m_adjustedFontSize;
  int m_control_font_size;
  wxFont* m_basefont;
  wxWindow* m_parent;
  bool m_bautoCentre;
  bool m_bautosize;
};

#endif
