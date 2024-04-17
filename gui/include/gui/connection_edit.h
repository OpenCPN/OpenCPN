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

#ifndef _CONNECT_EDIT_H
#define _CONNECT_EDIT_H

#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/timer.h>


#include "model/conn_params.h"
#include "model/comm_util.h"

#include "observable.h"


class options;
class ConnectionParamsPanel;

//----------------------------------------------------------------------------
// ConnectionEditDialog
//----------------------------------------------------------------------------
class ConnectionEditDialog : public wxDialog {
public:
  const wxString DefaultTCPPort = "10110";
  const wxString DefaultUDPPort = "10110";
  const wxString DefaultGPSDPort = "2947";
  const wxString DefaultSignalKPort = "3000";
  const wxString DefaultIPAddress = "0.0.0.0";

  ConnectionEditDialog();
 // ConnectionEditDialog(wxScrolledWindow *container, options *parent);
  ConnectionEditDialog(options *parent, ConnectionsDialog *client);

  ~ConnectionEditDialog();

  void Init(void);
  void SetInitialSettings(void);
  void PreloadControls(ConnectionParams* cp);
  ConnectionParams* GetParamsFromControls();
  void SetPropsLabel(wxString label);

  void ApplySettings();
  ConnectionParams *CreateConnectionParamsFromSelectedItem();
  ConnectionParams *UpdateConnectionParamsFromControls(
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
  void OnProtocolChoice(wxCommandEvent &event);
  void OnCrcCheck(wxCommandEvent &event) { OnConnValChange(event); }
  void OnRbAcceptInput(wxCommandEvent &event);
  void OnRbIgnoreInput(wxCommandEvent &event);
  void OnBtnIStcs(wxCommandEvent &event);
  void OnCbInput(wxCommandEvent &event);
  void OnCbOutput(wxCommandEvent &event);
  void OnCbMultiCast(wxCommandEvent &event);
  void OnCbAdvanced(wxCommandEvent &event);
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
  void SetUDPNetAddressVisiblity(void);
  bool IsAddressMultiCast(wxString &ip);
  bool IsAddressBroadcast(wxString &ip);
  bool IsDefaultPort(wxString address);

  void ClearNMEAForm(void);
  void SetNMEAFormToSerial(void);
  void SetNMEAFormToNet(void);
  void SetNMEAFormToGPS(void);
  void SetNMEAFormToBT(void);
  void SetNMEAFormToCAN(void);
  void SetNMEAFormForSerialProtocol();
  void SetNMEAFormForNetProtocol();

  void ShowTypeCommon(bool visible = true);
  void ShowNMEACommon(bool visible);
  void ShowNMEASerial(bool visible);
  void ShowNMEANet(bool visible);
  void ShowNMEAGPS(bool visible);
  void ShowNMEABT(bool visible);
  void ShowNMEACAN(bool visible);

  void OnScanBTClick(wxCommandEvent &event);
  void onBTScanTimer(wxTimerEvent &event);
  void StopBTScan(void);

  void OnWheelChoice(wxMouseEvent& event);

  void ShowInFilter(bool bshow = true);
  void ShowOutFilter(bool bshow = true);
  void LayoutDialog();


  void CreateControls();
  void ConnectControls();

//private:
  options *m_parent;
  wxScrolledWindow *m_scrolledwin;

  
  wxGridSizer *gSizerNetProps, *gSizerSerProps, *gSizerCanProps;
  wxTextCtrl *m_tNetAddress, *m_tNetPort, *m_tFilterSec, *m_tcInputStc;
  wxTextCtrl *m_tcOutputStc, *m_TalkerIdText;
  wxCheckBox *m_cbCheckCRC, *m_cbGarminHost, *m_cbGarminUploadHost,
      *m_cbCheckSKDiscover;
  wxCheckBox *m_cbFurunoGP3X, *m_cbNMEADebug, *m_cbFilterSogCog, *m_cbInput;
  wxCheckBox *m_cbMultiCast, *m_cbAdvanced;
  wxCheckBox *m_cbOutput, *m_cbAPBMagnetic;
  wxComboBox *m_comboPort;
  wxStdDialogButtonSizer *m_sdbSizerDlgButtons;
  wxButton  *m_ButtonSKDiscover, *m_ButtonPriorityDialog;
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
  wxStaticText *m_stNetComment, *m_stSerialComment, *m_stCANSource, *m_stAuthToken;
  wxTextCtrl *m_tNetComment, *m_tSerialComment, *m_tAuthToken;
  wxStaticBox *m_sbConnEdit;
  wxChoice *m_choiceBTDataSources, *m_choiceBaudRate, *m_choiceSerialProtocol;
  wxChoice *m_choicePriority, *m_choicePrecision, *m_choiceCANSource;
  wxChoice *m_choiceNetDataProtocol;

  wxBoxSizer *boxSizerConnections;
  wxStaticText *m_stNetDataProtocol;

  ConnectionParams *mSelectedConnection;

#ifdef __ANDROID__
  wxPanel *m_scrollWinConnections;
#else
  wxScrolledWindow *m_scrollWinConnections;
#endif

  bool connectionsaved;
  bool m_connection_enabled;
  bool m_bNMEAParams_shown;
  int m_btNoChangeCounter, m_btlastResultCount, m_BTscanning;
  wxArrayString m_BTscan_results;
  wxTimer m_BTScanTimer;
  wxArrayString m_choice_CANSource_choices;

  ObsListener new_device_listener;


  //DECLARE_EVENT_TABLE()

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
