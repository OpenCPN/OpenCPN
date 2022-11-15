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
 ***************************************************************************
 */

#ifndef __AISTARGETLISTDIALOG_H__
#define __AISTARGETLISTDIALOG_H__

#include <memory>

#include <wx/panel.h>
#include <wx/checkbox.h>

#define ID_AIS_TARGET_LIST 10003
#define ID_RCLK_UNDOCK 7035

class OCPNListCtrl;
class AisDecoder;
class AisTargetData;
class wxWindow;
class wxAuiManager;
class wxButton;
class wxStaticText;
class wxSpinCtrl;
class wxTextCtrl;
class wxListEvent;
class wxAuiManager;
class wxAuiManagerEvent;

WX_DEFINE_SORTED_ARRAY_INT(int, ArrayOfMMSI);

class AISTargetListDialog : public wxPanel {
  DECLARE_CLASS(AISTargetListDialog)

public:
  AISTargetListDialog(wxWindow *parent, wxAuiManager *auimgr,
                      AisDecoder *pdecoder);
  ~AISTargetListDialog();

  void Shutdown(void);
  void OnClose(wxCloseEvent &event);
  void Disconnect_decoder();

  void RecalculateSize(void);
  void SetColorScheme();
  void UpdateAISTargetList();  // Rebuild AIS target list
  void UpdateNVAISTargetList();
  void CopyMMSItoClipBoard(int);
  void CenterToTarget(bool);
  std::shared_ptr<AisTargetData> GetpTarget(unsigned int list_item);

  OCPNListCtrl *m_pListCtrlAISTargets;
  AisDecoder *m_pdecoder;

  ArrayOfMMSI *m_pMMSI_array;

private:
  void CreateControls(void);

  void OnPaneClose(wxAuiManagerEvent &event);
  void UpdateButtons();
  void OnTargetSelected(wxListEvent &event);
  void DoTargetQuery(int mmsi);
  void OnTargetDefaultAction(wxListEvent &event);
  void OnTargetQuery(wxCommandEvent &event);
  void OnTargetListColumnClicked(wxListEvent &event);
  void OnTargetScrollTo(wxCommandEvent &event);
  void OnTargetScrollToClose(wxCommandEvent &event);
  void OnTargetCreateWpt(wxCommandEvent &event);
  void OnShowAllTracks(wxCommandEvent &event);
  void OnHideAllTracks(wxCommandEvent &event);
  void OnToggleTrack(wxCommandEvent &event);
  void OnCopyMMSI(wxCommandEvent &event);
  void OnLimitRange(wxCommandEvent &event);
  void OnCloseButton(wxCommandEvent &event);
  void OnAutosortCB(wxCommandEvent &event);
  void OnRightClickContext(wxCommandEvent &event);
  void OnContextUndock(wxCommandEvent &event);

  wxWindow *m_pparent;
  wxAuiManager *m_pAuiManager;
  wxButton *m_pButtonInfo;
  wxButton *m_pButtonJumpTo;
  wxButton *m_pButtonJumpTo_Close;
  wxButton *m_pButtonCreateWpt;
  wxButton *m_pButtonHideAllTracks;
  wxButton *m_pButtonShowAllTracks;
  wxButton *m_pButtonToggleTrack;
  wxButton *m_pButtonCopyMMSI;
  wxStaticText *m_pStaticTextRange;
  wxSpinCtrl *m_pSpinCtrlRange;
  wxStaticText *m_pStaticTextCount;
  wxTextCtrl *m_pTextTargetCount;
  wxButton *m_pButtonOK;
  wxCheckBox *m_pCBAutosort;

  bool m_bautosort_force;

  DECLARE_EVENT_TABLE()
};

#endif
