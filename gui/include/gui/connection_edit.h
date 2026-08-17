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
      wxWindow* parent,
      std::function<void(ConnectionParams* p, bool editing, bool ok_cancel)>
          on_edit_click);

  ~ConnectionEditDialog() override;
  void SetPropsLabel(const wxString& label);
  void SetDefaultConnectionParams();
  void SetNewMode(bool mode) { m_new_mode = mode; }
  void AddOKCancelButtons();
  /** Return parameters instance populated from UI elements owned by caller */
  ConnectionParams* GetParamsFromControls();
  void PreloadControls(ConnectionParams* cp);
  void InitiateNewConnection();

  /** Initiate a network connection view with new data */
  void OnConnectionTypeChange();

  /** Refresh visible states in a network connection view */
  void RefreshAdvancedDetails();

private:
#ifdef __ANDROID__
  wxPanel* m_scroll_win_connections;
#else
  wxScrolledWindow* m_scroll_win_connections;
#endif
  std::function<void(ConnectionParams*, bool, bool)> m_on_edit_click;
  ConnectionParams* m_selected_conn_params;
  ConnectionParams* m_cp_original;
  obs::Listener m_new_device_listener;

  bool m_is_conn_saved;
  bool m_is_editing;
  bool m_is_nmea_params_shown;
  bool m_new_mode;
  bool m_advanced = false;
  bool m_conn_enabled;

  wxRadioButton* m_accept_radiobtn;
  wxButton* m_add_btn;
  wxCheckBox* m_advanced_cb;
  wxCheckBox* m_aps_magnetic_cb;
  wxTextCtrl* m_auth_token_tc;
  wxStaticText* m_auth_token_text;
  wxChoice* m_baud_rate_choice;
  wxChoice* m_bt_data_sources_choice;
  int m_bt_last_result_count;
  int m_bt_no_change_counter;
  wxStaticText* m_bt_pairs_text;
  int m_bt_scanning;
  wxArrayString m_bt_scan_results;
  wxTimer m_bt_scan_timer;
  wxGridSizer* m_can_props_sizer;
  wxChoice* m_can_source_choice;
  wxArrayString m_can_source_choice2;
  wxStaticText* m_can_source_text;
  wxSizer* m_collapse_box;
  wxStaticBoxSizer* m_connection_props_sizer;
  wxBoxSizer* m_connections_sizer;
  wxStaticBox* m_conn_edit_statbox;
  wxButton* m_dlg_buttons_apply_btn;
  wxButton* m_dlg_buttons_cancel_btn;
  wxButton* m_dlg_buttons_ok_btn;
  wxButton* m_dlg_cancel_btn;
  wxButton* m_dlg_ok_btn;
  wxCheckBox* m_filter_cog_sog_cb;
  wxTextCtrl* m_filter_sec_tc;
  wxStaticText* m_filter_sec_text;
  wxCheckBox* m_furuno_gp3x_cb;
  wxCheckBox* m_garmin_host_cb;
  wxCheckBox* m_garmin_upload_host_cb;
  wxRadioButton* m_ignore_radiobtn;
  wxStaticBoxSizer* m_in_filter_sizer;
  wxCheckBox* m_input_cb;
  wxButton* m_input_stc_list_btn;
  wxTextCtrl* m_input_stc_tc;
  wxTextCtrl* m_net_address_tc;
  wxStaticText* m_net_addr_text;
  wxTextCtrl* m_net_comment_tc;
  wxStaticText* m_net_comment_text;
  wxChoice* m_net_data_protocol_choice;
  wxStaticText* m_net_data_protocol_text;
  wxStaticText* m_net_expert_box_text;
  wxCheckBox* m_net_expert_cb;
  wxTextCtrl* m_net_port_tc;
  wxStaticText* m_net_port_text;
  wxGridSizer* m_net_props_sizer;
  wxChoice* m_net_type_choice;
  wxStaticText* m_net_type_choice_text;
  wxRadioButton* m_o_accept_radiobtn;
  wxRadioButton* m_o_ignore_radiobtn;
  wxStaticBoxSizer* m_out_filter_sizer;
  wxCheckBox* m_output_cb;
  wxButton* m_output_stc_list_btn;
  wxTextCtrl* m_output_stc_tc;
  wxWindow* m_parent;
  wxComboBox* m_port_combo;
  wxChoice* m_precision_choice;
  wxStaticText* m_precision_text;
  wxChoice* m_priority_choice;
  wxButton* m_priority_dialog_btn;
  wxButton* m_remove_btn;
  wxButton* m_scan_bt_btn;
  wxStaticText* m_ser_baudrate_text;
  wxStaticText* m_ser_comment_text;
  wxTextCtrl* m_serial_comment_tc;
  wxChoice* m_serial_protocol_choice;
  wxStaticText* m_ser_port_text;
  wxGridSizer* m_ser_props_sizer;
  wxStaticText* m_ser_protocol_text;
  wxBoxSizer* m_sizer_box_btn;
  wxCheckBox* m_sk_check_discover_cb;
  wxButton* m_sk_discover_btn;
  wxStaticText* m_sk_server_status_text;
  wxStdDialogButtonSizer* m_std_dialog_btn_sizer;
  wxStaticText* m_talker_id_text;
  wxRadioButton* m_type_can_radiobtn;
  wxRadioButton* m_type_internal_bt_radiobtn;
  wxRadioButton* m_type_internal_gps_radiobtn;
  wxRadioButton* m_type_net_radiobtn;
  wxRadioButton* m_type_serial_radiobtn;

  ConnectionParams* UpdateConnectionParamsFromControls(ConnectionParams* cp);

  void ClearNMEAForm();
  void ConnectControls();
  void EnableConnection(ConnectionParams* conn, bool value);
  void Init();
  void LayoutDialog();
  void OnAddressChange(wxFocusEvent& ev);
  void OnAdvancedModeChange();
  void OnBaudrateChoice(wxCommandEvent& event) { OnConnValChange(event); }
  void OnBtnIStcs(wxCommandEvent& event);
  void OnBtnOStcs(wxCommandEvent& event);
  void OnBtScanTimer(wxTimerEvent& event);
  void OnCancelClick();
  void OnCbAdvanced(wxCommandEvent& event);
  void OnCbInput(wxCommandEvent& event);
  void OnCbOutput(wxCommandEvent& event);
  void OnCollapsedToggle(bool collapsed);
  void OnConnValChange(wxCommandEvent& event);
  void OnDiscoverButton(wxCommandEvent& event);
  void OnNetProtocolSelected(wxCommandEvent& event);
  void OnOKClick();
  void OnPriorityDialog(wxCommandEvent& event);
  void OnProtocolChoice(wxCommandEvent& event);
  void OnRbAcceptInput(wxCommandEvent& event);
  void OnRbIgnoreInput(wxCommandEvent& event);
  void OnRbOutput(wxCommandEvent& event);
  void OnScanBtClick(wxCommandEvent& event);
  void OnSelectDatasource(wxListEvent& event);
  void OnTypeBTSelected(wxCommandEvent& event);
  void OnTypeCANSelected(wxCommandEvent& event);
  void OnTypeGPSSelected(wxCommandEvent& event);
  void OnTypeNetSelected(wxCommandEvent& event);
  void OnTypeSerialSelected(wxCommandEvent& event);
  void OnUploadFormatChange(wxCommandEvent& event);
  void OnValChange(wxCommandEvent& event);
  void SetConnectionParams(ConnectionParams* cp);
  void SetDSFormOptionVizStates();
  void SetDSFormRWStates();
  void SetInitialSettings();
  void SetNMEAFormForNetProtocol();
  void SetNMEAFormForSerialProtocol();
  void SetNMEAFormToBT();
  void SetNMEAFormToCAN();
  void SetNMEAFormToGPS();
  void SetNMEAFormToNet();
  void SetNMEAFormToSerial();
  void SetSelectedConnectionPanel(ConnectionParamsPanel* panel);
  void ShowInFilter(bool bshow = true);
  void ShowNMEABT(bool visible);
  void ShowNMEACAN(bool visible);
  void ShowNMEACommon(bool visible);
  void ShowNMEAGPS(bool visible);
  void ShowNMEANet(bool visible);
  void ShowNMEASerial(bool visible);
  void ShowOutFilter(bool bshow = true);
  void ShowTypeCommon(bool visible = true);
  void StopBtScan();
  void UpdateDiscoverStatus(const wxString& stat);
  void UpdateSourceList(bool bResort);
};

class SentenceListDlg : public wxDialog {
public:
  explicit SentenceListDlg(wxWindow* parent, FilterDirection dir, ListType type,
                           const wxArrayString& list);
  wxString GetSentences();

private:
  void OnAddClick(wxCommandEvent& event);
  void OnDeleteClick(wxCommandEvent& event);
  void OnCLBSelect(wxCommandEvent& event);
  void OnCheckAllClick(wxCommandEvent& event);
  void OnClearAllClick(wxCommandEvent& event);

  void Populate(const wxArrayString& list);
  [[nodiscard]] wxString GetBoxLabel() const;

  wxCheckListBox* m_sentences_clb;
  wxButton* m_del_btn;

  ListType m_type;
  FilterDirection m_dir;
  wxArrayString m_sentences;
};

#endif  // COnnect_EDIT_H
