/***************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/

/**
 * \file
 *
 * Dialog and support code for editing a connection
 */

#ifndef COnnect_EDIT_H
#define COnnect_EDIT_H

#include <functional>

#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/timer.h>

#include "observable/observable.h"

#include "model/conn_params.h"

#include "expand_icon.h"
#include "options.h"
#include "conn_params_panel.h"

/**
 * Dialog for editing connection parameters. Provides an interface for
 * creating new connections or editing existing connection parameters in detail.
 */
class ConnectionEditDialog : public wxPanel {
public:
  const std::string kDefaultTcpPort = "10110";
  const std::string kDefaultUdpPort = "10110";
  const std::string kDefaultGpsdPort = "2947";
  const std::string kDefaultSignalkPort = "3000";
  const std::string kDefaultIpAddress = "localhost";
  const std::string kDefaultMulticastAddr = "224.0.2.21";
  // "LIMITED BROADCAST" address
  //  Deprecated, but still useful on simple mobile networks.
  const wxString DEFAULT_UDP_OUT_ADDRESS = "255.255.255.255";

  ConnectionEditDialog();
  ConnectionEditDialog(
      wxWindow *parent,
      std::function<void(ConnectionParams *p, bool editing, bool ok_cancel)>
          on_edit_click);

  ~ConnectionEditDialog() override;
  void SetPropsLabel(const wxString &label);
  void SetDefaultConnectionParams();
  void SetNewMode(bool mode) { new_mode = mode; }
  void AddOKCancelButtons();
  /** Return parameters instance populated from UI elements owned by caller */
  ConnectionParams *GetParamsFromControls();
  void PreloadControls(ConnectionParams *cp);

private:
  void Init();
  void SetInitialSettings();

  void ApplySettings();
  ConnectionParams *CreateConnectionParamsFromSelectedItem();
  ConnectionParams *UpdateConnectionParamsFromControls(
      ConnectionParams *pConnectionParams);
  void SetSelectedConnectionPanel(ConnectionParamsPanel *panel);
  void OnSelectDatasource(wxListEvent &event);
  // void OnAddDatasourceClick(wxCommandEvent &event);
  // void OnRemoveDatasourceClick(wxCommandEvent &event);

  void OnTypeSerialSelected(wxCommandEvent &event);
  void OnTypeNetSelected(wxCommandEvent &event);
  void OnTypeGPSSelected(wxCommandEvent &event);
  void OnTypeBTSelected(wxCommandEvent &event);
  void OnTypeCANSelected(wxCommandEvent &event);

  void OnNetProtocolSelected(wxCommandEvent &event);
  void OnBaudrateChoice(wxCommandEvent &event) { OnConnValChange(event); }
  void OnProtocolChoice(wxCommandEvent &event);
  void OnRbAcceptInput(wxCommandEvent &event);
  void OnRbIgnoreInput(wxCommandEvent &event);
  void OnBtnIStcs(wxCommandEvent &event);
  void OnCbInput(wxCommandEvent &event);
  void OnCbOutput(wxCommandEvent &event);
  void OnCbAdvanced(wxCommandEvent &event);
  // void OnClickMore(wxMouseEvent &event);
  void OnRbOutput(wxCommandEvent &event);
  void OnBtnOStcs(wxCommandEvent &event);
  void OnConnValChange(wxCommandEvent &event);
  void OnValChange(wxCommandEvent &event);
  void OnUploadFormatChange(wxCommandEvent &event);
  void OnCollapsedToggle(bool collapsed);
  void OnShowGpsWindowCheckboxClick(wxCommandEvent &event);
  void EnableConnection(ConnectionParams *conn, bool value);
  void OnDiscoverButton(wxCommandEvent &event);
  void UpdateDiscoverStatus(const wxString &stat);
  void OnPriorityDialog(wxCommandEvent &event);

  void SetConnectionParams(ConnectionParams *cp);
  void SetDSFormRWStates();
  void SetDSFormOptionVizStates();
  // void FillSourceList();
  void UpdateSourceList(bool bResort);

  void ClearNMEAForm();
  void SetNMEAFormToSerial();
  void SetNMEAFormToNet();
  void SetNMEAFormToGPS();
  void SetNMEAFormToBT();
  void SetNMEAFormToCAN();
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
  void StopBTScan();

  // void OnWheelChoice(wxMouseEvent &event);

  void ShowInFilter(bool bshow = true);
  void ShowOutFilter(bool bshow = true);
  void LayoutDialog();

  // void CreateControls();
  void ConnectControls();

  wxStdDialogButtonSizer *m_btnSizer;
  wxBoxSizer *m_btnSizerBox;

  wxButton *m_btnOK;
  wxButton *m_btnCancel;
  bool new_mode;

  void OnOKClick();
  void OnCancelClick();

  // private:
  wxWindow *m_parent;
  // wxScrolledWindow *m_scrolledwin;

  wxGridSizer *gSizerNetProps;
  wxGridSizer *gSizerSerProps;
  wxGridSizer *gSizerCanProps;
  wxTextCtrl *m_tNetAddress;
  wxTextCtrl *m_tNetPort;
  wxTextCtrl *m_tFilterSec;
  wxTextCtrl *m_tcInputStc;
  wxTextCtrl *m_tcOutputStc;
  wxCheckBox *m_advanced_net_box;
  wxCheckBox *m_cbGarminHost;
  wxCheckBox *m_cbGarminUploadHost;
  wxCheckBox *m_cbCheckSKDiscover;
  wxCheckBox *m_cbFurunoGP3X;
  wxCheckBox *m_cbNMEADebug;
  wxCheckBox *m_cbFilterSogCog;
  wxCheckBox *m_cbInput;
  wxCheckBox *m_cbAdvanced;
  wxCheckBox *m_cbOutput;
  wxCheckBox *m_cbAPBMagnetic;
  wxComboBox *m_comboPort;
  wxStdDialogButtonSizer *m_sdbSizerDlgButtons;
  wxButton *m_ButtonSKDiscover;
  wxButton *m_ButtonPriorityDialog;
  wxStaticText *m_StaticTextSKServerStatus;

  wxButton *m_buttonAdd;
  wxButton *m_buttonRemove;
  wxButton *m_buttonScanBT;
  wxButton *m_btnInputStcList;
  wxButton *m_btnOutputStcList;
  wxButton *m_sdbSizerDlgButtonsOK;
  wxButton *m_sdbSizerDlgButtonsApply;
  wxButton *m_sdbSizerDlgButtonsCancel;
  wxStaticBoxSizer *sbSizerConnectionProps;
  wxStaticBoxSizer *sbSizerInFilter;
  wxStaticBoxSizer *sbSizerOutFilter;
  wxRadioButton *m_rbTypeSerial;
  wxRadioButton *m_rbTypeNet;
  wxRadioButton *m_rbTypeInternalGPS;
  wxRadioButton *m_rbTypeInternalBT;
  wxRadioButton *m_rbIAccept;
  wxRadioButton *m_rbIIgnore;
  wxRadioButton *m_rbOAccept;
  wxRadioButton *m_rbOIgnore, *m_rbTypeCAN;
  wxStaticText *m_stBTPairs;
  wxStaticText *m_net_type_choice_label;
  wxStaticText *m_stNetAddr;
  wxStaticText *m_stNetPort;
  wxStaticText *m_stSerPort;
  wxStaticText *m_stSerBaudrate;
  wxStaticText *m_stSerProtocol;
  wxStaticText *m_stFilterSec;
  wxStaticText *m_stPrecision;
  wxStaticText *m_stTalkerIdText;
  wxStaticText *m_stNetComment;
  wxStaticText *m_stSerialComment;
  wxStaticText *m_stCANSource;
  wxStaticText *m_stAuthToken;
  wxStaticText *m_net_adv_box_label;
  wxTextCtrl *m_tNetComment;
  wxTextCtrl *m_tSerialComment;
  wxTextCtrl *m_tAuthToken;
  wxStaticBox *m_sbConnEdit;
  wxChoice *m_choiceBTDataSources;
  wxChoice *m_choiceBaudRate;
  wxChoice *m_choiceSerialProtocol;
  wxChoice *m_choicePriority;
  wxChoice *m_choicePrecision;
  wxChoice *m_choiceCANSource;
  wxChoice *m_choiceNetDataProtocol;
  wxChoice *m_net_type_choice;

  wxBoxSizer *boxSizerConnections;
  wxStaticText *m_stNetDataProtocol;
  ConnectionParams *mSelectedConnection;

#ifdef __ANDROID__
  wxPanel *m_scrollWinConnections;
#else
  wxScrolledWindow *m_scrollWinConnections;
#endif

  bool connectionsaved;
  bool m_advanced = false;
  bool m_connection_enabled;
  bool m_bNMEAParams_shown;
  int m_btNoChangeCounter;
  int m_btlastResultCount;
  int m_BTscanning;
  wxArrayString m_BTscan_results;
  wxTimer m_BTScanTimer;
  wxArrayString m_choice_CANSource_choices;

  obs::Listener new_device_listener;
  ConnectionParams *m_cp_original;

  std::function<void(ConnectionParams *, bool, bool)> m_on_edit_click;

  wxSizer *m_collapse_box;
  void OnConnectionTypeChange();
  void OnAdvancedModeChange();
  void OnAddressChange(wxFocusEvent &ev);
};

class SentenceListDlg : public wxDialog {
public:
  explicit SentenceListDlg(wxWindow *parent, FilterDirection dir, ListType type,
                           const wxArrayString &list);
  wxString GetSentences();

private:
  void OnAddClick(wxCommandEvent &event);
  void OnDeleteClick(wxCommandEvent &event);
  void OnCLBSelect(wxCommandEvent &event);
  void OnCheckAllClick(wxCommandEvent &event);
  void OnClearAllClick(wxCommandEvent &event);

  void Populate(const wxArrayString &list);
  [[nodiscard]] wxString GetBoxLabel() const;

  wxCheckListBox *m_clbSentences;
  wxButton *m_btnDel;

  ListType m_type;
  FilterDirection m_dir;
  wxArrayString m_sentences;
};

#endif  // COnnect_EDIT_H
