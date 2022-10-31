/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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

#ifndef _CONNECT_DIALOG_H
#define _CONNECT_DIALOG_H

#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/timer.h>


#include "conn_params.h"
#include "comm_util.h"
class options;
class ConnectionParamsPanel;

//----------------------------------------------------------------------------
// ConnectionsDialog
//----------------------------------------------------------------------------
class ConnectionsDialog : public wxEvtHandler {
public:
  ConnectionsDialog();
  ConnectionsDialog(wxScrolledWindow *container, options *parent);

  ~ConnectionsDialog();

  void Init(void);
  void RecalculateSize(void);
  void SetInitialSettings(void);
  void ApplySettings();
  ConnectionParams *CreateConnectionParamsFromSelectedItem();
  ConnectionParams *UpdateConnectionParamsFromSelectedItem(
      ConnectionParams *pConnectionParams);
  void SetSelectedConnectionPanel(ConnectionParamsPanel *panel);
  void OnSelectDatasource(wxListEvent &event);
  void OnAddDatasourceClick(wxCommandEvent &event);
  void OnRemoveDatasourceClick(wxCommandEvent &event);

  void OnTypeSerialSelected(wxCommandEvent &event);
  void OnTypeNetSelected(wxCommandEvent &event);
  void OnTypeGPSSelected(wxCommandEvent &event);
  void OnTypeBTSelected(wxCommandEvent &event);
  void OnTypeCANSelected(wxCommandEvent &event);

  void OnNetProtocolSelected(wxCommandEvent &event);
  void OnBaudrateChoice(wxCommandEvent &event) { OnConnValChange(event); }
  void OnProtocolChoice(wxCommandEvent &event) { OnConnValChange(event); }
  void OnCrcCheck(wxCommandEvent &event) { OnConnValChange(event); }
  void OnRbAcceptInput(wxCommandEvent &event);
  void OnRbIgnoreInput(wxCommandEvent &event);
  void OnBtnIStcs(wxCommandEvent &event);
  void OnCbInput(wxCommandEvent &event);
  void OnCbOutput(wxCommandEvent &event);
  void OnRbOutput(wxCommandEvent &event);
  void OnBtnOStcs(wxCommandEvent &event);
  void OnConnValChange(wxCommandEvent &event);
  void OnValChange(wxCommandEvent &event);
  void OnUploadFormatChange(wxCommandEvent &event);
  void OnShowGpsWindowCheckboxClick(wxCommandEvent& event);
  void EnableConnection(ConnectionParams *conn, bool value);
  void OnDiscoverButton(wxCommandEvent &event);
  void UpdateDiscoverStatus(wxString stat);
  void OnPriorityDialog(wxCommandEvent &event);

  void SetConnectionParams(ConnectionParams *cp);
  void SetDefaultConnectionParams(void);
  void SetDSFormRWStates();
  void SetDSFormOptionVizStates();
  void FillSourceList();
  void UpdateSourceList(bool bResort);
  bool SortSourceList(void);

  void ClearNMEAForm(void);
  void SetNMEAFormToSerial(void);
  void SetNMEAFormToNet(void);
  void SetNMEAFormToGPS(void);
  void SetNMEAFormToBT(void);
  void SetNMEAFormToCAN(void);

  void ShowNMEACommon(bool visible);
  void ShowNMEASerial(bool visible);
  void ShowNMEANet(bool visible);
  void ShowNMEAGPS(bool visible);
  void ShowNMEABT(bool visible);
  void ShowNMEACAN(bool visible);

  void OnScanBTClick(wxCommandEvent &event);
  void onBTScanTimer(wxTimerEvent &event);
  void StopBTScan(void);

//private:
  wxScrolledWindow *m_container;
  options *m_parent;

  wxGridSizer *gSizerNetProps, *gSizerSerProps;
  wxTextCtrl *m_tNetAddress, *m_tNetPort, *m_tFilterSec, *m_tcInputStc;
  wxTextCtrl *m_tcOutputStc, *m_TalkerIdText;
  wxCheckBox *m_cbCheckCRC, *m_cbGarminHost, *m_cbGarminUploadHost,
      *m_cbCheckSKDiscover;
  wxCheckBox *m_cbFurunoGP3X, *m_cbNMEADebug, *m_cbFilterSogCog, *m_cbInput;
  wxCheckBox *m_cbOutput, *m_cbAPBMagnetic, *m_cbPriorityDialog;
  wxComboBox *m_comboPort;
  wxStdDialogButtonSizer *m_sdbSizerDlgButtons;
  wxButton  *m_ButtonSKDiscover;
  wxStaticText *m_StaticTextSKServerStatus;

  wxButton *m_buttonAdd, *m_buttonRemove, *m_buttonScanBT, *m_btnInputStcList;
  wxButton *m_btnOutputStcList, *m_sdbSizerDlgButtonsOK;
  wxButton *m_sdbSizerDlgButtonsApply, *m_sdbSizerDlgButtonsCancel;
  wxStaticBoxSizer *sbSizerConnectionProps, *sbSizerInFilter;
  wxStaticBoxSizer *sbSizerOutFilter;
  wxRadioButton *m_rbTypeSerial, *m_rbTypeNet, *m_rbTypeInternalGPS;
  wxRadioButton *m_rbTypeInternalBT, *m_rbNetProtoTCP, *m_rbNetProtoUDP;
  wxRadioButton *m_rbNetProtoGPSD, *m_rbIAccept, *m_rbIIgnore, *m_rbOAccept;
  wxRadioButton *m_rbNetProtoSignalK;
  wxRadioButton *m_rbOIgnore, *m_rbTypeCAN;
  wxStaticText *m_stBTPairs, *m_stNetProto, *m_stNetAddr, *m_stNetPort;
  wxStaticText *m_stSerPort, *m_stSerBaudrate, *m_stSerProtocol;
  wxStaticText *m_stPriority, *m_stFilterSec, *m_stPrecision;
  wxStaticText *m_stTalkerIdText;
  wxStaticText *m_stNetComment, *m_stSerialComment, *m_stCANSource;
  wxTextCtrl *m_tNetComment, *m_tSerialComment;
  wxStaticBox *m_sbConnEdit;
  wxChoice *m_choiceBTDataSources, *m_choiceBaudRate, *m_choiceSerialProtocol;
  wxChoice *m_choicePriority, *m_choicePrecision, *m_choiceCANSource;
  wxScrolledWindow *m_scrollWinConnections;
  wxBoxSizer *boxSizerConnections;
  ConnectionParams *mSelectedConnection;

  bool connectionsaved;
  bool m_connection_enabled;
  bool m_bNMEAParams_shown;
  int m_btNoChangeCounter, m_btlastResultCount, m_BTscanning;
  wxArrayString m_BTscan_results;
  wxTimer m_BTScanTimer;
  wxArrayString m_choice_CANSource_choices;

  DECLARE_EVENT_TABLE()

};


class SentenceListDlg : public wxDialog {
public:
  explicit SentenceListDlg(wxWindow *parent, FilterDirection dir, ListType type,
                           const wxArrayString &list);
  wxString GetSentences(void);

private:
  void OnAddClick(wxCommandEvent &event);
  void OnDeleteClick(wxCommandEvent &event);
  void OnCLBSelect(wxCommandEvent &event);
  void OnCheckAllClick(wxCommandEvent &event);
  void OnClearAllClick(wxCommandEvent &event);

  void Populate(const wxArrayString &list);
  wxString GetBoxLabel(void) const;

  wxCheckListBox *m_clbSentences;
  wxButton *m_btnDel;

  ListType m_type;
  FilterDirection m_dir;
  wxArrayString m_sentences;

};


#endif    //_CONNECT_DIALOG_H
