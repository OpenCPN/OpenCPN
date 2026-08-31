/**************************************************************************
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
 **************************************************************************/

/**
 * \file
 *
 * Implement connection_edit.h -- Dialog and support for editing a connection
 */

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "config.h"

#include <wx/tokenzr.h>
#include <wx/regex.h>

#if defined(__linux__) && !defined(__ANDROID__)
#include <linux/can.h>
#include <net/if.h>
#include <serial/serial.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "dnet.h"
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

#include "connection_edit.h"

#include "text_ctrl_w_help.h"
#include "model/comm_drv_factory.h"
#include "model/config_vars.h"
#include "model/ocpn_utils.h"
#include "model/ser_ports.h"
#include "model/sys_events.h"

#include "conn_params_panel.h"
#include "gui_lib.h"
#include "nmea0183.h"
#include "ocpn_platform.h"
#include "ocpn_plugin.h"  // FIXME for GetOCPNScaledFont_PlugIn
#include "options.h"
#include "priority_gui.h"
#include "udev_rule_mgr.h"

// Make _() return std::string instead of wxString;
#undef _
#if wxCHECK_VERSION(3, 2, 0)
#define _(s) wxGetTranslation(wxASCII_STR(s)).ToStdString()
#else
#define _(s) wxGetTranslation((s)).ToStdString()
#endif

static const std::string kAddressDefaultHelp =
    _("Enter IP address or hostname");
static const std::string kAddressUdpHelp =
    _("IP address or hostname, often 255.255.255.255");
static const std::string kAddressMcastHelp =
    _("Group address, usually 224.0.2.0 - 224.0.255.255");
static const std::string kTcpDevice = _("Device using TCP");
static const std::string kUdpReceive = _("Device sending UDP");
static const std::string kUdpOutput = _("UDP send");
static const std::string kGpsdDevice = _("Gpsd server");
static const std::string kSignalkDevice = _("SignalK server");
static const std::string kTcpClient = _("TCP client");
static const std::string kUdpSend = _("Devices receiving UDP");
static const std::string kGpsdClient = _("Gpsd client");
static const std::string kSignalkClient = _("SignalK client");
static const std::string kTcpServer = _("TCP Server");
static const std::string kUdpInput = _("UDP Receive");
static const std::string kMulticastServer = _("UDP Multicast Receive and Send");
static const std::string kMulticastClient = _("UDP Multicast Send");

static const std::vector<std::string> kBasicNetViews = {
    kTcpDevice, kUdpReceive, kUdpSend, kGpsdDevice, kSignalkDevice};

static const std::vector<std::string> kAdvancedNetViews = {
    // First items matches kBasicNetViews
    kTcpClient,     kUdpInput,  kUdpOutput,       kGpsdClient,
    kSignalkClient, kTcpServer, kMulticastClient, kMulticastServer};

static wxString StringArrayToString(const wxArrayString& arr) {
  wxString ret = wxEmptyString;
  for (size_t i = 0; i < arr.Count(); i++) {
    if (i > 0) ret.Append(",");
    ret.Append(arr[i]);
  }
  return ret;
}

// Check available SocketCAN interfaces
#if defined(__linux__) && !defined(__ANDROID__)
static intf_t* intf;
std::vector<std::string> can_if_candidates;
static int print_intf(const struct intf_entry* entry, void* arg) {
  std::string iface = entry->intf_name;
  if (entry->intf_type == 1 && iface.find("can") != std::string::npos) {
    can_if_candidates.push_back(entry->intf_name);
  }
  return 0;
}
#endif

static bool IsAddressMultiCast(const wxString& ip) {
  wxArrayString bytes = wxSplit(ip, '.');
  if (bytes.size() != 4) {
    return false;
  }
  unsigned long ipNum = (wxAtoi(bytes[0]) << 24) + (wxAtoi(bytes[1]) << 16) +
                        (wxAtoi(bytes[2]) << 8) + wxAtoi(bytes[3]);
  unsigned long multicastStart = (224 << 24);
  unsigned long multicastEnd = (239 << 24) + (255 << 16) + (255 << 8) + 255;
  return ipNum >= multicastStart && ipNum <= multicastEnd;
}

static bool IsAddressListener(const std::string& address) {
  return address.empty() || address == "0.0.0.0";
}

/** Return  selected option from a wxChoice widget */
static std::string GetChoiceSelection(const wxChoice* choice) {
  int selected = choice->GetSelection();
  return choice->GetString(selected).ToStdString();
}

/**
 * Return net view name for given ConnectionParams. For ambigious cases
 * like kTcpDevice (basic view)/kTcpClient (Advanced view) return
 * the basic view.
 */
static std::string NetViewByConnection(const ConnectionParams* cp) {
  bool is_server = IsAddressListener(cp->NetworkAddress.ToStdString());
  if (IsAddressMultiCast(cp->NetworkAddress))
    return is_server ? kMulticastServer : kMulticastClient;
  switch (cp->NetProtocol) {
    case NetworkProtocol::GPSD:
      return kGpsdDevice;
    case NetworkProtocol::SIGNALK:
      return kSignalkDevice;
    case NetworkProtocol::UDP:
      return is_server ? kUdpReceive : kUdpSend;
    case NetworkProtocol::TCP:
      return is_server ? kTcpServer : kTcpDevice;
    default:
      wxLogWarning("Cannot deduce connection params view type");
      return "";
  }
  return "";  // for the compiler
}

static wxArrayString GetAvailableSocketCANInterfaces() {
  wxArrayString rv;

#if defined(__linux__) && !defined(__ANDROID__)
  can_if_candidates.clear();

  if ((intf = intf_open()) == nullptr) {
    wxLogWarning("Error opening interface list");
    return rv;
  }

  if (intf_loop(intf, print_intf, nullptr) < 0) {
    wxLogWarning("Error looping over interface list");
  }
  intf_close(intf);

  for (const auto& iface : can_if_candidates) {
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
      continue;
    }

    // Get the interface index
    struct ifreq if_request = {{0}};
    strcpy(if_request.ifr_name, iface.c_str());
    if (ioctl(sock, SIOCGIFINDEX, &if_request) < 0) {
      continue;
    }

    // Check if interface is UP
    struct sockaddr_can can_address = {0};
    can_address.can_family = AF_CAN;
    can_address.can_ifindex = if_request.ifr_ifindex;
    if (ioctl(sock, SIOCGIFFLAGS, &if_request) < 0) {
      continue;
    }
    if (if_request.ifr_flags & IFF_UP) {
      rv.Add(iface);
    } else {
      continue;
    }
  }
#endif
  return rv;
}

static void LoadSerialPorts(wxComboBox* box) {
  /** Sort all links to same device as equals. */
  class PortSorter {
  private:
    [[nodiscard]] static std::string GetKey(const std::string& s) {
      if (s.find("->") == std::string::npos) return s;
      return ocpn::trim(ocpn::split(s, "->")[1]) + " link";
    }

  public:
    bool operator()(const std::string& lhs, const std::string& rhs) const {
      return GetKey(lhs) < GetKey(rhs);
    }
  } port_sorter;

  std::set<std::string, PortSorter> sorted_ports(port_sorter);
  std::unique_ptr<wxArrayString> ports(EnumerateSerialPorts());
  for (size_t i = 0; i < ports->GetCount(); i++)
    sorted_ports.insert((*ports)[i].ToStdString());

  auto value = box->GetValue();
  box->Clear();
  for (auto& p : sorted_ports) box->Append(p);
  if (!value.empty()) box->SetValue(value);
}

static bool CheckPort(wxWindow* parent, TextCtrlWithHelp& ctrl) {
  if (ctrl.IsPristine() || ctrl.GetValue().empty()) {
    auto dlg = wxMessageDialog(parent, _("Required field port is missing"),
                               _("OpenCPN error"), wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  };
  int port = 0;
  try {
    port = std::stoi(ctrl.GetValue().ToStdString());
  } catch (std::logic_error&) {
    auto dlg = wxMessageDialog(parent, _("Invalid port number"),
                               _("OpenCPN error"), wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  }
  if (port < 1024) {
    static const std::string kMsg =
        _(R"(Port numbers smaller than 1024 are reserved for use by the
operating system and should normally not be used by OpenCPN)");
    auto dlg = wxMessageDialog(parent, kMsg, _("OpenCPN warning"),
                               wxOK | wxICON_WARNING);
    dlg.ShowModal();
    return true;
  }
  return true;
}

bool CheckAddress(wxWindow* parent, TextCtrlWithHelp& ctrl) {
  if (ctrl.IsPristine() || ctrl.GetValue().empty()) {
    auto dlg = wxMessageDialog(parent, _("Required field address is missing"),
                               _("OpenCPN error"), wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  };
  // Checking the address requires using gethostbyname() or so since it
  // could be a hostname. Not worthwhile in this context.
  return true;
}

/** Initiate the nmea protocol 0183/2000 choide */
static void SetupProtocolChoice(wxChoice* choice) {
  choice->Clear();
  choice->Append("NMEA 0183");
  choice->Append("NMEA 2000");
  choice->SetSelection(0);
  choice->Enable();
}
//------------------------------------------------------------------------------
//          ConnectionEditDialog Implementation
//------------------------------------------------------------------------------

// Define constructors
ConnectionEditDialog::ConnectionEditDialog()
    : ConnectionEditDialog(nullptr, nullptr) {}

ConnectionEditDialog::ConnectionEditDialog(
    wxWindow* parent,
    std::function<void(ConnectionParams* p, bool editing, bool ok_cancel)>
        _on_edit_click)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), 0,
              "conn_edit"),
      m_scroll_win_connections(nullptr),
      m_on_edit_click(std::move(_on_edit_click)),
      m_selected_conn_params(nullptr),
      m_cp_original(nullptr),
      m_is_conn_saved(false),
      m_is_editing(false),
      m_is_nmea_params_shown(false),
      m_new_mode(false),
      m_advanced(false),
      m_conn_enabled(false),
      m_accept_radiobtn(nullptr),
      m_add_btn(nullptr),
      m_advanced_chkbox(nullptr),
      m_aps_magnetic_chkbox(nullptr),
      m_auth_token_tctrl(nullptr),
      m_auth_token_text(nullptr),
      m_baud_rate_choice(nullptr),
      m_bt_data_sources_choice(nullptr),
      m_bt_last_result_count(0),
      m_bt_no_change_counter(0),
      m_bt_pairs_text(nullptr),
      m_bt_scanning(0),
      m_bt_scan_timer(nullptr),
      m_can_props_sizer(nullptr),
      m_can_source_choice(nullptr),
      m_can_source_text(nullptr),
      m_collapse_box(nullptr),
      m_connection_props_sizer(nullptr),
      m_connections_sizer(nullptr),
      m_conn_edit_statbox(nullptr),
      m_dlg_buttons_apply_btn(nullptr),
      m_dlg_buttons_cancel_btn(nullptr),
      m_dlg_buttons_ok_btn(nullptr),
      m_dlg_cancel_btn(nullptr),
      m_dlg_ok_btn(nullptr),
      m_filter_cog_sog_chkbox(nullptr),
      m_filter_sec_tctrl(nullptr),
      m_filter_sec_text(nullptr),
      m_furuno_gp3x_chkbox(nullptr),
      m_garmin_host_chkbox(nullptr),
      m_garmin_upload_host_chkbox(nullptr),
      m_ignore_radiobtn(nullptr),
      m_in_filter_sizer(nullptr),
      m_input_chkbox(nullptr),
      m_input_stc_list_btn(nullptr),
      m_input_stc_tctrl(nullptr),
      m_net_address_tctrl(nullptr),
      m_net_addr_text(nullptr),
      m_net_comment_tctrl(nullptr),
      m_net_comment_text(nullptr),
      m_net_data_protocol_choice(nullptr),
      m_net_data_protocol_text(nullptr),
      m_net_expert_box_text(nullptr),
      m_net_expert_chkbox(nullptr),
      m_net_port_tctrl(nullptr),
      m_net_port_text(nullptr),
      m_net_props_sizer(nullptr),
      m_net_view_choice(nullptr),
      m_net_type_choice_text(nullptr),
      m_o_accept_radiobtn(nullptr),
      m_o_ignore_radiobtn(nullptr),
      m_out_filter_sizer(nullptr),
      m_output_chkbox(nullptr),
      m_output_stc_list_btn(nullptr),
      m_output_stc_tctrl(nullptr),
      m_parent(parent),
      m_port_combo(nullptr),
      m_precision_choice(nullptr),
      m_precision_text(nullptr),
      m_priority_choice(nullptr),
      m_priority_dialog_btn(nullptr),
      m_remove_btn(nullptr),
      m_scan_bt_btn(nullptr),
      m_ser_baudrate_text(nullptr),
      m_ser_comment_text(nullptr),
      m_serial_comment_tctrl(nullptr),
      m_serial_protocol_choice(nullptr),
      m_ser_port_text(nullptr),
      m_ser_props_sizer(nullptr),
      m_ser_protocol_text(nullptr),
      m_sizer_box_btn(nullptr),
      m_sk_check_discover_chkbox(nullptr),
      m_sk_discover_btn(nullptr),
      m_sk_server_status_text(nullptr),
      m_std_dialog_btn_sizer(nullptr),
      m_talker_id_text(nullptr),
      m_type_can_radiobtn(nullptr),
      m_type_internal_bt_radiobtn(nullptr),
      m_type_internal_gps_radiobtn(nullptr),
      m_type_net_radiobtn(nullptr),
      m_type_serial_radiobtn(nullptr) {
  Init();
}

ConnectionEditDialog::~ConnectionEditDialog() = default;

void ConnectionEditDialog::AddOKCancelButtons() {
#ifndef ANDROID
  if (!m_std_dialog_btn_sizer) {
    m_std_dialog_btn_sizer = new wxStdDialogButtonSizer();
    m_dlg_ok_btn = new wxButton(this, wxID_OK);
    m_dlg_cancel_btn = new wxButton(this, wxID_CANCEL, _("Cancel"));
    m_std_dialog_btn_sizer->AddButton(m_dlg_ok_btn);
    m_std_dialog_btn_sizer->AddButton(m_dlg_cancel_btn);
    m_std_dialog_btn_sizer->Realize();
    GetSizer()->Add(m_std_dialog_btn_sizer, 0, wxALL | wxEXPAND, 5);
    m_std_dialog_btn_sizer->Show(true);
  }
#else
  if (!m_std_dialog_btn_sizer) {
    m_std_dialog_btn_sizer = new wxStdDialogButtonSizer();
    m_dlg_ok_btn = new wxButton(this, wxID_OK);
    m_dlg_cancel_btn = new wxButton(this, wxID_CANCEL, _("Cancel"));
    m_std_dialog_btn_sizer->AddSpacer(wxWindow::GetCharWidth());
    m_std_dialog_btn_sizer->Add(m_dlg_ok_btn, 0, wxALL, 5);
    m_std_dialog_btn_sizer->Add(m_dlg_cancel_btn, 0, wxALL, 5);
    GetSizer()->Add(m_std_dialog_btn_sizer, 0, wxALL | wxEXPAND, 5);
  }
#endif

  m_dlg_ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                     [&](wxCommandEvent& ev) { OnOKClick(); });
  m_dlg_cancel_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                         [&](wxCommandEvent& ev) { OnCancelClick(); });
}
void ConnectionEditDialog::InitiateNewConnection() {
  m_net_view_choice->Clear();
  for (const auto& view : kBasicNetViews) m_net_view_choice->Append(view);
  m_net_view_choice->SetSelection(0);
  m_net_expert_chkbox->SetValue(false);
  m_net_comment_tctrl->Hide();
  m_net_comment_text->Hide();
  auto port_ctrl = dynamic_cast<TextCtrlWithHelp*>(m_net_port_tctrl);
  if (port_ctrl) port_ctrl->RestoreHelp();
  auto addr_ctrl = dynamic_cast<TextCtrlWithHelp*>(m_net_address_tctrl);
  if (addr_ctrl) addr_ctrl->SetHelp(kAddressDefaultHelp);
  SetupProtocolChoice(m_net_data_protocol_choice);
}

void ConnectionEditDialog::OnConnectionTypeChange() {
  int selection = m_net_view_choice->GetSelection();
  if (selection == wxNOT_FOUND) return;
  std::string view = GetChoiceSelection(m_net_view_choice);
  ConfigureControlsForView(view);
  OnExpertModeChange();
}

void ConnectionEditDialog::ConfigureControlsForView(const std::string& view) {
  if (!m_type_net_radiobtn->GetValue()) return;
  auto found = std::find(kBasicNetViews.begin(), kBasicNetViews.end(), view);
  int selection = m_net_view_choice->GetSelection();
  assert(selection != wxNOT_FOUND);
  m_net_expert_chkbox->SetValue(found == kBasicNetViews.end());
  m_net_address_tctrl->Enable();
  auto net_addr_w_help = dynamic_cast<TextCtrlWithHelp*>(m_net_address_tctrl);
  assert(net_addr_w_help);
  auto net_port_w_help = dynamic_cast<TextCtrlWithHelp*>(m_net_port_tctrl);
  assert(net_port_w_help);
  m_net_addr_text->SetLabel(_("Server address"));
  m_output_chkbox->Disable();
  m_input_chkbox->Disable();
  m_net_addr_text->Show();
  m_net_address_tctrl->Show();
  auto port = m_net_port_tctrl->GetValue();
  if (port == kDefaultGpsdPort || port == kDefaultSignalkPort || port.empty())
    net_port_w_help->RestoreHelp();
  if (view == kTcpDevice || view == kTcpClient) {
    m_input_chkbox->SetValue(true);
    m_output_chkbox->SetValue(false);
    m_net_addr_text->Show();
    m_net_address_tctrl->Show();
    m_input_chkbox->Enable();
    m_output_chkbox->Enable();
  } else if (view == kUdpReceive || view == kUdpInput) {
    m_net_addr_text->Hide();
    m_net_address_tctrl->ChangeValue("0.0.0.0");
    m_net_address_tctrl->Disable();
    m_net_address_tctrl->Hide();
    m_input_chkbox->SetValue(true);
    m_output_chkbox->SetValue(false);
  } else if (view == kGpsdClient || view == kGpsdDevice) {
    m_net_data_protocol_choice->Clear();
    m_net_data_protocol_choice->Append("gpsd");
    m_net_data_protocol_choice->SetSelection(0);
    m_net_data_protocol_choice->Disable();
    m_output_chkbox->SetValue(false);
    m_input_chkbox->SetValue(true);
    if (net_port_w_help->IsPristine())
      net_port_w_help->ChangeValue(kDefaultGpsdPort);
  } else if (view == kSignalkClient || view == kSignalkDevice) {
    m_net_data_protocol_choice->Clear();
    m_net_data_protocol_choice->Append("SignalK");
    m_net_data_protocol_choice->SetSelection(0);
    m_net_data_protocol_choice->Disable();
    m_input_chkbox->SetValue(true);
    m_output_chkbox->SetValue(false);
    if (net_port_w_help->IsPristine())
      net_port_w_help->ChangeValue(kDefaultSignalkPort);
  } else if (view == kTcpServer) {
    m_net_addr_text->SetLabel(_("Interface"));
    m_net_address_tctrl->ChangeValue("0.0.0.0");
    m_net_address_tctrl->Disable();
    m_input_chkbox->SetValue(true);
    m_output_chkbox->Enable();
    m_input_chkbox->Enable();
  } else if (view == kMulticastClient || view == kMulticastServer) {
    if (m_net_view_choice->GetCount() != 2)
      SetupProtocolChoice(m_net_data_protocol_choice);
    if (net_addr_w_help->GetValue().empty()) net_addr_w_help->RestoreHelp();
    m_net_addr_text->SetLabel(_("Multicast group"));
    if (net_port_w_help->IsPristine())
      net_port_w_help->SetHelp("Port number, usually 49152..65535");
  }

  if (view == kMulticastClient || view == kUdpSend || view == kUdpOutput) {
    m_input_chkbox->SetValue(false);
    m_output_chkbox->SetValue(true);
  } else if (view == kMulticastServer) {
    m_input_chkbox->SetValue(true);
    m_output_chkbox->SetValue(false);
    m_output_chkbox->Enable();
    m_input_chkbox->Disable();
  }

  if (view == kTcpClient || view == kTcpDevice || view == kUdpInput ||
      view == kUdpReceive) {
    if (net_port_w_help->IsPristine())
      net_port_w_help->SetHelp(_("Port number (1025..65535, often 10110)"));
  }
  if (view != kGpsdClient && view != kGpsdDevice && view != kSignalkClient &&
      view != kSignalkDevice) {
    if (m_net_view_choice->GetCount() != 2)
      SetupProtocolChoice(m_net_data_protocol_choice);
  }
  if (view != kTcpServer && view != kUdpReceive && view != kUdpInput &&
      view != kMulticastServer) {
    if (m_net_address_tctrl->GetValue() == "0.0.0.0")
      net_addr_w_help->RestoreHelp();
  }
  if (net_addr_w_help->IsPristine()) {
    if (view == kUdpSend)
      net_addr_w_help->SetHelp(kAddressUdpHelp);
    else if (view == kMulticastClient || view == kMulticastServer)
      net_addr_w_help->SetHelp(kAddressMcastHelp);
    else
      net_addr_w_help->SetHelp(kAddressDefaultHelp);
  }
  RefreshAdvancedDetails();
}

void ConnectionEditDialog::RefreshAdvancedDetails() {
  if (!m_type_net_radiobtn->GetValue()) return;
  if (m_garmin_host_chkbox) m_garmin_host_chkbox->Hide();
  if (m_garmin_upload_host_chkbox) m_garmin_upload_host_chkbox->Hide();
  const std::string view = GetChoiceSelection(m_net_view_choice);
  bool show_auth = view == kSignalkDevice || view == kSignalkClient;
  m_auth_token_tctrl->Show(show_auth && m_advanced);
  m_auth_token_text->Show(show_auth && m_advanced);
  bool show_apb_precision = m_output_chkbox->IsChecked();
  m_precision_text->Show(show_apb_precision && m_advanced);
  m_precision_choice->Show(show_apb_precision && m_advanced);
  Layout();
}

void ConnectionEditDialog::Init() {
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  // Setup some initial values

  m_bt_scan_timer.SetOwner(this, ID_BT_SCANTIMER);
  m_bt_scanning = 0;
  wxSize displaySize = wxGetDisplaySize();

  // Create the UI

  auto* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  wxFont* dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
  double font_size = dFont->GetPointSize() * 17 / 16;
  wxFont* bFont = wxTheFontList->FindOrCreateFont(
      static_cast<int>(font_size), dFont->GetFamily(), dFont->GetStyle(),
      wxFONTWEIGHT_BOLD);

  //  Connections Properties
  m_conn_edit_statbox =
      new wxStaticBox(this, wxID_ANY, _("Edit Selected Connection"));
  m_conn_edit_statbox->SetFont(*bFont);

  m_connection_props_sizer =
      new wxStaticBoxSizer(m_conn_edit_statbox, wxVERTICAL);
  GetSizer()->Add(m_connection_props_sizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizer15;
  bSizer15 = new wxBoxSizer(wxHORIZONTAL);

  m_connection_props_sizer->Add(bSizer15, 0, wxTOP | wxEXPAND, 5);

  m_type_serial_radiobtn =
      new wxRadioButton(this, wxID_ANY, _("Serial"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_type_serial_radiobtn->SetValue(true);
  bSizer15->Add(m_type_serial_radiobtn, 0, wxALL, 5);

  m_type_net_radiobtn = new wxRadioButton(this, wxID_ANY, _("Network"),
                                          wxDefaultPosition, wxDefaultSize, 0);
  bSizer15->Add(m_type_net_radiobtn, 0, wxALL, 5);

  m_type_can_radiobtn = new wxRadioButton(this, wxID_ANY, "socketCAN",
                                          wxDefaultPosition, wxDefaultSize, 0);
#if defined(__linux__) && !defined(__ANDROID__) && !defined(__WXOSX__)
  bSizer15->Add(m_type_can_radiobtn, 0, wxALL, 5);
#else
  m_type_can_radiobtn->Hide();
#endif

  auto* bSizer15a = new wxBoxSizer(wxHORIZONTAL);
  m_connection_props_sizer->Add(bSizer15a, 0, wxEXPAND, 5);

  if (OCPNPlatform::hasInternalGPS()) {
    m_type_internal_gps_radiobtn = new wxRadioButton(
        this, wxID_ANY, _("Built-in GPS"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer15a->Add(m_type_internal_gps_radiobtn, 0, wxALL, 5);
  } else
    m_type_internal_gps_radiobtn = nullptr;

  // has built-in Bluetooth
  if (OCPNPlatform::hasInternalBT()) {
    m_type_internal_bt_radiobtn =
        new wxRadioButton(this, wxID_ANY, _("Built-in Bluetooth SPP"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15a->Add(m_type_internal_bt_radiobtn, 0, wxALL, 5);

    m_scan_bt_btn = new wxButton(this, wxID_ANY, _("BT Scan") + "    ",
                                 wxDefaultPosition, wxDefaultSize);
    m_scan_bt_btn->Hide();

    m_connection_props_sizer->Add(m_scan_bt_btn, 0, wxALL, 25);

    m_bt_pairs_text =
        new wxStaticText(this, wxID_ANY, _("Bluetooth Data Sources"),
                         wxDefaultPosition, wxDefaultSize, 0);
    m_bt_pairs_text->Wrap(-1);
    m_bt_pairs_text->Hide();
    m_connection_props_sizer->Add(m_bt_pairs_text, 0, wxALL, 5);

    wxArrayString mt;
    mt.Add("unscanned");

    int ref_size = this->GetCharWidth();
    m_bt_data_sources_choice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition,
                     wxSize(40 * ref_size, 2 * ref_size), mt);
    m_bt_data_sources_choice->SetSelection(0);
    m_bt_data_sources_choice->Hide();
    m_connection_props_sizer->Add(m_bt_data_sources_choice, 1, wxEXPAND | wxTOP,
                                  25);

  } else {
    m_type_internal_bt_radiobtn = nullptr;
  }

  m_net_props_sizer = new wxFlexGridSizer(0, 2, 0, 0);

  m_connection_props_sizer->Add(m_net_props_sizer, 0, wxEXPAND, 5);

  // Optimize for Portrait mode handheld devices
  if (displaySize.x < displaySize.y) {
    wxBoxSizer* bSizer16a;
    bSizer16a = new wxBoxSizer(wxHORIZONTAL);
    m_net_props_sizer->AddSpacer(1);
    m_net_props_sizer->Add(bSizer16a, 1, wxEXPAND, 5);
    m_net_props_sizer->AddSpacer(1);
    m_net_props_sizer->AddSpacer(1);
  }
  m_net_expert_box_text = new wxStaticText(this, wxID_ANY, _("Expert mode"));
  m_net_props_sizer->Add(m_net_expert_box_text, 0, wxALL, 5);
  m_net_expert_chkbox = new wxCheckBox(this, wxID_ANY, "");
  m_net_props_sizer->Add(m_net_expert_chkbox, 0, wxALL, 5);
  m_net_expert_chkbox->Bind(
      wxEVT_CHECKBOX, [&](const wxCommandEvent&) { OnExpertModeChange(); });

  m_net_type_choice_text =
      new wxStaticText(this, wxID_ANY, _("Connection type"));
  m_net_props_sizer->Add(m_net_type_choice_text, 0, wxALL, 5);
  m_net_view_choice = new wxChoice(this, wxID_ANY);
  m_net_view_choice->Append(kBasicNetViews[0]);
  m_net_view_choice->SetSelection(0);  // until OnConnectionTypeChanged()
  m_net_view_choice->Bind(wxEVT_CHOICE,
                          [&](wxCommandEvent&) { OnConnectionTypeChange(); });
  m_net_props_sizer->Add(m_net_view_choice, 0, wxTOP, 5);
  m_net_data_protocol_text =
      new wxStaticText(this, wxID_ANY, _("Data Protocol"));
  m_net_data_protocol_text->Wrap(-1);
  m_net_props_sizer->Add(m_net_data_protocol_text, 0, wxALL, 5);

  m_net_data_protocol_choice = new wxChoice(this, wxID_ANY);
  SetupProtocolChoice(m_net_data_protocol_choice);
  m_net_props_sizer->Add(m_net_data_protocol_choice, 1, wxTOP, 5);
  m_net_props_sizer->AddSpacer(1);
  m_net_props_sizer->AddSpacer(1);

  m_net_addr_text = new wxStaticText(this, wxID_ANY, _("Address"));
  m_net_addr_text->Wrap(-1);
  int column1width = 15 * GetCharWidth();
  m_net_addr_text->SetMinSize(wxSize(column1width, -1));
  m_net_props_sizer->Add(m_net_addr_text, 0, wxALL, 5);
  m_net_address_tctrl = new TextCtrlWithHelp(this, kAddressDefaultHelp);
  int column2width = 60 * GetCharWidth();
  m_net_address_tctrl->SetMaxSize(wxSize(column2width, -1));
  m_net_address_tctrl->SetMinSize(wxSize(column2width, -1));
  m_net_address_tctrl->Bind(wxEVT_KILL_FOCUS,
                            [&](wxFocusEvent& ev) { OnAddressChange(ev); });

  m_net_props_sizer->Add(m_net_address_tctrl, 0, wxEXPAND | wxTOP, 5);
  m_net_props_sizer->AddSpacer(1);
  m_net_props_sizer->AddSpacer(1);

  m_net_port_text = new wxStaticText(this, wxID_ANY, _("Data Port"));
  m_net_port_text->Wrap(-1);
  m_net_props_sizer->Add(m_net_port_text, 0, wxALL, 5);

  m_net_port_tctrl = new TextCtrlWithHelp(this, "Enter data source port");
  m_net_port_tctrl->SetMaxSize(wxSize(column2width, -1));
  m_net_port_tctrl->SetMinSize(wxSize(column2width, -1));
  m_net_props_sizer->Add(m_net_port_tctrl, 1, wxEXPAND | wxTOP, 5);
  m_net_port_tctrl->SetMaxSize(wxSize(column2width, -1));
  m_net_port_tctrl->SetMinSize(wxSize(column2width, -1));

  m_net_comment_text = new wxStaticText(this, wxID_ANY, _("User Comment"));
  m_net_comment_text->Wrap(-1);
  m_net_comment_text->SetMinSize({column1width, -1});
  m_net_props_sizer->Add(m_net_comment_text, 0, wxALL, 5);
  m_net_comment_text->Hide();

  m_net_comment_tctrl = new wxTextCtrl(this, wxID_ANY);
  m_net_comment_tctrl->SetMaxSize({column2width, -1});
  m_net_comment_tctrl->SetMinSize({column2width, -1});
  m_net_props_sizer->Add(m_net_comment_tctrl, 1, wxEXPAND | wxTOP, 5);
  m_net_comment_tctrl->Hide();

  m_net_props_sizer->AddSpacer(1);
  m_net_props_sizer->AddSpacer(1);

  m_can_props_sizer = new wxGridSizer(0, 1, 0, 0);
  wxFlexGridSizer* fgSizer1C;
  fgSizer1C = new wxFlexGridSizer(0, 2, 0, 0);

  m_can_source_text = new wxStaticText(this, wxID_ANY, _("socketCAN Source"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_can_source_text->Wrap(-1);
  m_can_source_text->SetMinSize(wxSize(column1width, -1));
  fgSizer1C->Add(m_can_source_text, 0, wxALL, 5);

  wxArrayString choices = GetAvailableSocketCANInterfaces();
  m_can_source_choice =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);

  m_can_source_choice->SetSelection(0);
  m_can_source_choice->Enable(!choices.empty());
  m_can_source_choice->SetMaxSize(wxSize(column2width, -1));
  m_can_source_choice->SetMinSize(wxSize(column2width, -1));
  fgSizer1C->Add(m_can_source_choice, 1, wxEXPAND | wxTOP, 5);

  m_can_props_sizer->Add(fgSizer1C, 0, wxEXPAND, 5);

  m_connection_props_sizer->Add(m_can_props_sizer, 0, wxEXPAND, 5);

  m_ser_props_sizer = new wxGridSizer(0, 1, 0, 0);
  m_connection_props_sizer->Add(m_ser_props_sizer, 0, wxEXPAND, 5);

  wxFlexGridSizer* fgSizer1;
  fgSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
  fgSizer1->SetFlexibleDirection(wxBOTH);
  fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_ser_port_text =
      new wxStaticText(this, wxID_ANY, _("Data port"), wxDefaultPosition,
                       wxDefaultSize, wxST_ELLIPSIZE_END);
  m_ser_port_text->SetMinSize(wxSize(column1width, -1));
  m_ser_port_text->Wrap(-1);

  fgSizer1->Add(m_ser_port_text, 0, wxALL, 5);

  m_port_combo =
      new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, 0, nullptr, 0);

  m_port_combo->SetMaxSize(wxSize(column2width, -1));
  m_port_combo->SetMinSize(wxSize(column2width, -1));

  fgSizer1->Add(m_port_combo, 0, wxEXPAND | wxTOP, 5);

  m_ser_baudrate_text = new wxStaticText(this, wxID_ANY, _("Baudrate"),
                                         wxDefaultPosition, wxDefaultSize, 0);
  m_ser_baudrate_text->Wrap(-1);
  fgSizer1->AddSpacer(1);
  fgSizer1->AddSpacer(1);
  fgSizer1->Add(m_ser_baudrate_text, 0, wxALL, 5);

  wxString m_choiceBaudRateChoices[] = {
      _("150"),    _("300"),    _("600"),    _("1200"),  _("2400"),
      _("4800"),   _("9600"),   _("19200"),  _("38400"), _("57600"),
      _("115200"), _("230400"), _("460800"), _("921600")};
  int m_choiceBaudRateNChoices =
      sizeof(m_choiceBaudRateChoices) / sizeof(wxString);
  m_baud_rate_choice =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceBaudRateNChoices, m_choiceBaudRateChoices, 0);
  // m_choiceBaudRate->Bind(wxEVT_MOUSEWHEEL,
  // &ConnectionEditDialog::OnWheelChoice, this);

  m_baud_rate_choice->SetSelection(0);

  fgSizer1->Add(m_baud_rate_choice, 1, wxEXPAND | wxTOP, 5);
  fgSizer1->AddSpacer(1);
  fgSizer1->AddSpacer(1);

  m_ser_protocol_text = new wxStaticText(this, wxID_ANY, _("Protocol"));
  m_ser_protocol_text->Wrap(-1);
  fgSizer1->Add(m_ser_protocol_text, 0, wxALL, 5);

  wxString m_choiceSerialProtocolChoices[] = {_("NMEA 0183"), _("NMEA 2000")};
  int m_choiceSerialProtocolNChoices =
      sizeof(m_choiceSerialProtocolChoices) / sizeof(wxString);
  m_serial_protocol_choice = new wxChoice(
      this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      m_choiceSerialProtocolNChoices, m_choiceSerialProtocolChoices, 0);
  // m_choiceSerialProtocol->Bind(wxEVT_MOUSEWHEEL,
  // &ConnectionEditDialog::OnWheelChoice, this);

  m_serial_protocol_choice->SetSelection(0);
  m_serial_protocol_choice->Enable(true);
  fgSizer1->Add(m_serial_protocol_choice, 1, wxEXPAND | wxTOP, 5);

  m_ser_props_sizer->Add(fgSizer1, 0, wxEXPAND, 5);

  //  User Comments

  auto* commentSizer = new wxFlexGridSizer(0, 2, 0, 0);
  // sbSizerConnectionProps->Add(commentSizer, 0, wxEXPAND, 5);

  //  Serial User Comments
  m_ser_comment_text = new wxStaticText(this, wxID_ANY, _("User Comment"));
  m_ser_comment_text->Wrap(-1);
  m_ser_comment_text->SetMinSize(wxSize(column1width, -1));
  commentSizer->Add(m_ser_comment_text, 0, wxALL, 5);

  m_serial_comment_tctrl = new wxTextCtrl(this, wxID_ANY);
  m_serial_comment_tctrl->SetMaxSize(wxSize(column2width, -1));
  m_serial_comment_tctrl->SetMinSize(wxSize(column2width, -1));

  commentSizer->Add(m_serial_comment_tctrl, 1, wxTOP, 5);

  m_connection_props_sizer->Add(commentSizer, 0, wxALL, 5);

  wxFlexGridSizer* fgSizer5;
  fgSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer5->SetFlexibleDirection(wxBOTH);
  fgSizer5->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
  m_connection_props_sizer->Add(fgSizer5, 0, wxEXPAND, 5);

  m_input_chkbox =
      new wxCheckBox(this, wxID_ANY, _("Receive Input on this Port"));
  fgSizer5->Add(m_input_chkbox, 0, wxALL, 2);
  fgSizer5->AddSpacer(1);

  m_output_chkbox =
      new wxCheckBox(this, wxID_ANY,
                     wxString::Format("%s (%s)", _("Output on this port"),
                                      _("as autopilot or NMEA repeater")));
  fgSizer5->Add(m_output_chkbox, 0, wxALL, 2);
  fgSizer5->AddSpacer(1);

  // Authentication token

  auto flags = wxSizerFlags().Border();
  m_collapse_box = new wxBoxSizer(wxHORIZONTAL);

  m_collapse_box->Add(new wxStaticText(this, wxID_ANY, _("Advanced: ")), flags);
  m_collapse_box->Add(
      new ExpandableIcon(this,
                         [&](bool collapsed) { OnCollapsedToggle(collapsed); }),
      flags);
  fgSizer5->Add(m_collapse_box, wxSizerFlags());
  fgSizer5->Add(new wxStaticText(this, wxID_ANY, ""));

#ifndef USE_GARMINHOST
  m_cbGarminHost->Hide();
#endif

  m_auth_token_text = new wxStaticText(this, wxID_ANY, _("Auth Token"));
  m_auth_token_text->SetMinSize(wxSize(column1width, -1));
  m_auth_token_text->Wrap(-1);
  m_auth_token_text->SetMinSize(wxSize(column1width, -1));
  fgSizer5->Add(m_auth_token_text, 0, wxALL, 5);
  m_auth_token_text->Hide();

  m_auth_token_tctrl = new wxTextCtrl(this, wxID_ANY, "");
  m_auth_token_tctrl->SetMinSize(wxSize(column2width, -1));
  fgSizer5->Add(m_auth_token_tctrl, 1, wxEXPAND | wxTOP, 5);
  m_auth_token_tctrl->SetValue("orvar");
  m_auth_token_tctrl->Hide();

  fgSizer5->AddSpacer(1);
  fgSizer5->Add(new wxStaticText(this, wxID_ANY, ""), 0, wxALL, 2);

  m_precision_text =
      new wxStaticText(this, wxID_ANY, _("APB bearing precision"));
  m_precision_text->Wrap(-1);
  m_precision_text->SetMinSize(wxSize(column1width, -1));
  fgSizer5->Add(m_precision_text, 0, wxALL, 2);
  m_precision_text->Hide();

  wxString m_choicePrecisionChoices[] = {_("x"), _("x.x"), _("x.xx"),
                                         _("x.xxx"), _("x.xxxx")};
  int m_choicePrecisionNChoices =
      sizeof(m_choicePrecisionChoices) / sizeof(wxString);
  m_precision_choice =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choicePrecisionNChoices, m_choicePrecisionChoices, 0);
  // m_choicePrecision->Bind(wxEVT_MOUSEWHEEL,
  // &ConnectionEditDialog::OnWheelChoice, this);

  m_precision_choice->SetSelection(g_NMEAAPBPrecision);
  fgSizer5->Add(m_precision_choice, 0, wxALL, 2);
  m_precision_choice->Hide();
  OnExpertModeChange();

  m_garmin_host_chkbox =
      new wxCheckBox(this, wxID_ANY, _("Use Garmin (GRMN) mode for input"));
  m_garmin_host_chkbox->SetValue(false);
  fgSizer5->Add(m_garmin_host_chkbox, 0, wxALL, 2);
  fgSizer5->AddSpacer(1);

  // signalK discovery enable
  m_sk_check_discover_chkbox =
      new wxCheckBox(this, wxID_ANY, _("Automatic server discovery"));
  m_sk_check_discover_chkbox->SetValue(true);
  m_sk_check_discover_chkbox->SetToolTip(
      _("If checked, signal K server will be discovered automatically"));

  fgSizer5->Add(m_sk_check_discover_chkbox, 0, wxALL, 2);

  // signal K "Discover now" button
  m_sk_discover_btn = new wxButton(this, wxID_ANY, _("Discover now..."));
  m_sk_discover_btn->Hide();
  fgSizer5->Add(m_sk_discover_btn, 0, wxALL, 2);

  // signalK Server Status
  m_sk_server_status_text = new wxStaticText(this, wxID_ANY, "");
  fgSizer5->Add(m_sk_server_status_text, 0, wxALL, 2);

  m_in_filter_sizer = new wxStaticBoxSizer(
      new wxStaticBox(this, wxID_ANY, _("Input filtering")), wxVERTICAL);
  m_connection_props_sizer->Add(m_in_filter_sizer,
                                wxSizerFlags().Expand().Border());

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxHORIZONTAL);

  m_accept_radiobtn =
      new wxRadioButton(this, wxID_ANY, _("Accept only sentences"));
  bSizer9->Add(m_accept_radiobtn, 0, wxALL, 5);

  m_ignore_radiobtn = new wxRadioButton(this, wxID_ANY, _("Ignore sentences"));
  bSizer9->Add(m_ignore_radiobtn, 0, wxALL, 5);

  m_in_filter_sizer->Add(bSizer9, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);
  m_in_filter_sizer->Add(bSizer11, 0, wxEXPAND, 5);

  m_input_stc_tctrl =
      new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizer11->Add(m_input_stc_tctrl, 1, wxALL | wxEXPAND, 5);

  m_input_stc_list_btn = new wxButton(this, wxID_ANY, "...", wxDefaultPosition,
                                      wxDefaultSize, wxBU_EXACTFIT);
  bSizer11->Add(m_input_stc_list_btn, 0, wxALL, 5);

  bSizer11->AddSpacer(GetCharWidth() * 5);

  m_out_filter_sizer = new wxStaticBoxSizer(
      new wxStaticBox(this, wxID_ANY, _("Output filtering")), wxVERTICAL);
  m_connection_props_sizer->Add(m_out_filter_sizer, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxHORIZONTAL);

  m_o_accept_radiobtn =
      new wxRadioButton(this, wxID_ANY, _("Transmit sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer10->Add(m_o_accept_radiobtn, 0, wxALL, 5);

  m_o_ignore_radiobtn = new wxRadioButton(this, wxID_ANY, _("Drop sentences"),
                                          wxDefaultPosition, wxDefaultSize, 0);
  bSizer10->Add(m_o_ignore_radiobtn, 0, wxALL, 5);

  m_out_filter_sizer->Add(bSizer10, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer12;
  bSizer12 = new wxBoxSizer(wxHORIZONTAL);
  m_out_filter_sizer->Add(bSizer12, 0, wxEXPAND, 5);

  m_output_stc_tctrl =
      new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizer12->Add(m_output_stc_tctrl, 1, wxALL | wxEXPAND, 5);

  m_output_stc_list_btn = new wxButton(this, wxID_ANY, "...", wxDefaultPosition,
                                       wxDefaultSize, wxBU_EXACTFIT);
  bSizer12->Add(m_output_stc_list_btn, 0, wxALL, 5);

  bSizer12->AddSpacer(GetCharWidth() * 5);

  m_connection_props_sizer->AddSpacer(20);

  ConnectControls();

  SetInitialSettings();

  ShowTypeCommon();

  ShowNMEACommon(true);
  ShowNMEASerial(true);
  ShowNMEANet(false);
  ShowNMEACAN(false);
  OnConnectionTypeChange();
  m_is_conn_saved = true;

  GetSizer()->Fit(this);

  m_new_device_listener.Init(
      SystemEvents::GetInstance().evt_dev_change,
      [&](ObservedEvt&) { LoadSerialPorts(m_port_combo); });
}

void ConnectionEditDialog::OnAddressChange(wxFocusEvent& ev) {
  int selection = m_net_view_choice->GetSelection();
  if (selection != wxNOT_FOUND) {
    std::string type = m_net_view_choice->GetString(selection).ToStdString();
    if (type == kMulticastClient || type == kMulticastServer) {
      auto address = m_net_address_tctrl->GetValue().ToStdString();
      if (!IsAddressMultiCast(address)) {
        auto dlg = wxMessageDialog(this, _("Illegal multicast address"),
                                   _("OpenCPN warning"), wxOK | wxICON_WARNING);
        dlg.ShowModal();
      }
    }
  }
  ev.Skip();
}

void ConnectionEditDialog::OnExpertModeChange() {
  bool advanced = m_net_expert_chkbox->GetValue();
  int view = m_net_view_choice->GetSelection();
  m_net_view_choice->Clear();
  if (advanced) {
    for (const auto& choice : kAdvancedNetViews)
      m_net_view_choice->Append(choice);
    m_net_type_choice_text->SetLabel(_("Connection type"));
    if (m_net_comment_text) m_net_comment_text->Show();
    if (m_net_comment_tctrl) m_net_comment_tctrl->Show();
  } else {
    if (view != wxNOT_FOUND) {
      if (static_cast<size_t>(view) >= kBasicNetViews.size()) view = 0;
    }
    for (const auto& choice : kBasicNetViews) m_net_view_choice->Append(choice);
    m_net_type_choice_text->SetLabel(_("Connect to"));
    if (m_net_comment_text) m_net_comment_text->Hide();
    if (m_net_comment_tctrl) m_net_comment_tctrl->Hide();
  }
  m_net_view_choice->SetSelection(view);
  auto view_str = m_net_view_choice->GetStringSelection().ToStdString();
  if (!view_str.empty()) ConfigureControlsForView(view_str);
  Layout();
}

void ConnectionEditDialog::OnCancelClick() {
  m_on_edit_click(nullptr, false, false);
}

void ConnectionEditDialog::OnOKClick() {
  if (m_cp_original) {
    int selection = m_net_view_choice->GetSelection();
    if (selection != wxNOT_FOUND) {
      std::string selected =
          m_net_view_choice->GetString(selection).ToStdString();
    }
  }
  bool ok = true;
  if (m_net_address_tctrl->IsEnabled() &&
      m_net_address_tctrl->IsShownOnScreen()) {
    auto net_address = dynamic_cast<TextCtrlWithHelp*>(m_net_address_tctrl);
    if (net_address) ok = CheckAddress(this, *net_address);
  }
  if (m_net_port_tctrl->IsEnabled() && m_net_port_tctrl->IsShownOnScreen()) {
    auto net_port = dynamic_cast<TextCtrlWithHelp*>(m_net_port_tctrl);
    if (net_port) ok = ok && CheckPort(this, *net_port);
  }
  if (ok) m_on_edit_click(m_cp_original, m_new_mode, true);
}

void ConnectionEditDialog::SetInitialSettings() {
  LoadSerialPorts(m_port_combo);
}

// void ConnectionEditDialog::OnWheelChoice(wxMouseEvent& event) {
//   return;
// }

void ConnectionEditDialog::SetSelectedConnectionPanel(
    ConnectionParamsPanel* panel) {
  //  Only one panel can be selected at any time
  //  Clear any selections

  if (m_selected_conn_params && m_selected_conn_params->m_optionsPanel)
    m_selected_conn_params->m_optionsPanel->SetSelected(false);

  if (panel) {
    m_selected_conn_params = panel->m_pConnectionParams;
    panel->SetSelected(true);
    SetConnectionParams(m_selected_conn_params);
    m_remove_btn->Enable();
    m_remove_btn->Show();
    m_add_btn->Disable();
    m_conn_edit_statbox->SetLabel(_("Edit Selected Connection"));

  } else {
    m_selected_conn_params = nullptr;
    m_remove_btn->Disable();
    m_add_btn->Enable();
    m_add_btn->Show();
    m_conn_edit_statbox->SetLabel("");
    ClearNMEAForm();
  }

  //  Scroll the panel to allow the user to see more of the NMEA parameter
  //  settings area
  // wxPoint buttonPosition = m_buttonAdd->GetPosition();
  // this->Scroll(-1, buttonPosition.y / m_parent->GetScrollRate());
}

void ConnectionEditDialog::SetPropsLabel(const wxString& label) {
  m_conn_edit_statbox->SetLabel(label);
}

void ConnectionEditDialog::EnableConnection(ConnectionParams* conn,
                                            bool value) {
  if (conn) {
    // conn->bEnabled = value;
    conn->b_IsSetup = false;  // trigger a rebuild/takedown of the connection
    m_conn_enabled = conn->bEnabled;
  }
}

void ConnectionEditDialog::OnValChange(wxCommandEvent& event) { event.Skip(); }

void ConnectionEditDialog::OnScanBtClick(wxCommandEvent& event) {
  if (m_bt_scanning)
    StopBtScan();
  else {
    m_bt_no_change_counter = 0;
    m_bt_last_result_count = 0;

    Bind(wxEVT_TIMER, &ConnectionEditDialog::OnBtScanTimer, this,
         ID_BT_SCANTIMER);
    m_bt_scan_timer.Start(1000, wxTIMER_CONTINUOUS);
    g_Platform->startBluetoothScan();
    m_bt_scanning = 1;
    if (m_scan_bt_btn) {
      m_scan_bt_btn->SetLabel(_("Stop Scan"));
    }
  }
}

void ConnectionEditDialog::OnBtScanTimer(wxTimerEvent& event) {
  if (m_bt_scanning) {
    m_bt_scanning++;

    m_bt_scan_results = g_Platform->getBluetoothScanResults();

    m_bt_data_sources_choice->Clear();
    m_bt_data_sources_choice->Append(m_bt_scan_results[0]);  // scan status

    unsigned int i = 1;
    while ((i + 1) < m_bt_scan_results.GetCount()) {
      wxString item1 = m_bt_scan_results[i] + ";";
      wxString item2 = m_bt_scan_results.Item(i + 1);
      m_bt_data_sources_choice->Append(item1 + item2);

      i += 2;
    }

    if (m_bt_scan_results.GetCount() > 1) {
      m_bt_data_sources_choice->SetSelection(1);
    }

    //  Watch for changes.  When no changes occur after n seconds, stop the
    //  scan
    if (m_bt_no_change_counter > 5) StopBtScan();

    if ((int)m_bt_scan_results.GetCount() == m_bt_last_result_count)
      m_bt_no_change_counter++;
    else
      m_bt_no_change_counter = 0;

    m_bt_last_result_count = static_cast<int>(m_bt_scan_results.GetCount());

    // Absolute fallback
    if (m_bt_scanning >= 15) {
      StopBtScan();
    }
  } else {
  }
}

void ConnectionEditDialog::StopBtScan() {
  m_bt_scan_timer.Stop();

  g_Platform->stopBluetoothScan();

  m_bt_scanning = 0;

  if (m_scan_bt_btn) {
    m_scan_bt_btn->SetLabel(_("BT Scan"));
    m_scan_bt_btn->Enable();
  }
}

void ConnectionEditDialog::OnConnValChange(wxCommandEvent& event) {
  m_is_conn_saved = false;
  event.Skip();
}

void ConnectionEditDialog::OnTypeSerialSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToSerial();
}

void ConnectionEditDialog::OnTypeNetSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToNet();
}

void ConnectionEditDialog::OnTypeCANSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToCAN();
}

void ConnectionEditDialog::OnTypeGPSSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToGPS();
}

void ConnectionEditDialog::OnTypeBTSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToBT();
}

void ConnectionEditDialog::OnUploadFormatChange(wxCommandEvent& event) {
  if (event.GetEventObject() == m_garmin_upload_host_chkbox &&
      event.IsChecked())
    m_furuno_gp3x_chkbox->SetValue(false);
  else if (event.GetEventObject() == m_furuno_gp3x_chkbox && event.IsChecked())
    m_garmin_upload_host_chkbox->SetValue(false);

  OnConnValChange(event);
  event.Skip();
}

void ConnectionEditDialog::ShowTypeCommon(bool visible) {
  m_type_serial_radiobtn->Show(visible);
  m_type_net_radiobtn->Show(visible);
#if defined(__linux__) && !defined(__ANDROID__) && !defined(__WXOSX__)
  m_type_can_radiobtn->Show(visible);
#endif
  if (m_type_internal_gps_radiobtn) m_type_internal_gps_radiobtn->Show(visible);
  if (m_type_internal_bt_radiobtn) m_type_internal_bt_radiobtn->Show(visible);
}

void ConnectionEditDialog::ShowNMEACommon(bool visible) {
  bool advanced = m_advanced;
  m_input_chkbox->Show(visible);
  m_output_chkbox->Show(visible);
  if (!visible) {
    m_out_filter_sizer->SetDimension(0, 0, 0, 0);
    m_in_filter_sizer->SetDimension(0, 0, 0, 0);
    m_connection_props_sizer->SetDimension(0, 0, 0, 0);
    m_conn_edit_statbox->SetLabel("");
  }

  m_sk_check_discover_chkbox->Hide();  // Provisional
  m_sk_discover_btn->Hide();

  const bool bin_enable = (m_input_chkbox->IsChecked() && advanced);
  ShowInFilter(visible && bin_enable);
  const bool bout_enable = (m_output_chkbox->IsChecked() && advanced);
  ShowOutFilter(visible && bout_enable);

  m_is_nmea_params_shown = visible;
}

void ConnectionEditDialog::ShowNMEANet(bool visible) {
  if (m_dlg_ok_btn) m_dlg_ok_btn->Enable();

  m_net_addr_text->Show(visible);
  m_net_address_tctrl->Show(visible);
  m_net_data_protocol_text->Show(visible);
  m_net_port_text->Show(visible);
  m_net_data_protocol_choice->Show(visible);
  m_net_port_tctrl->Show(visible);
  m_net_expert_chkbox->Show(visible);
  if (m_net_expert_chkbox->GetValue()) {
    m_net_comment_text->Show(visible);
    m_net_comment_tctrl->Show(visible);
  }
  m_net_expert_box_text->Show(visible);
  m_net_type_choice_text->Show(visible);
  m_net_view_choice->Show(visible);
  m_garmin_host_chkbox->Hide();
  OnConnectionTypeChange();
}

void ConnectionEditDialog::ShowNMEASerial(bool visible) {
  bool advanced = m_advanced;
  if (m_dlg_ok_btn) m_dlg_ok_btn->Enable();

  m_ser_baudrate_text->Show(visible);
  m_baud_rate_choice->Show(visible);
  m_ser_port_text->Show(visible);
  m_port_combo->Show(visible);
  m_ser_protocol_text->Show(visible);
  m_serial_protocol_choice->Show(visible);
  m_garmin_host_chkbox->Show(visible && advanced);
  m_ser_comment_text->Show(visible);
  m_serial_comment_tctrl->Show(visible);
}

void ConnectionEditDialog::ShowNMEAGPS(bool visible) {
  if (m_dlg_ok_btn) m_dlg_ok_btn->Enable();

  m_sk_check_discover_chkbox->Hide();
  m_sk_discover_btn->Hide();
  m_output_chkbox->Hide();
}

void ConnectionEditDialog::ShowNMEACAN(bool visible) {
  if (m_dlg_ok_btn) m_dlg_ok_btn->Enable();
  m_can_source_text->Show(visible);
  m_can_source_choice->Show(visible);
  if (visible && m_dlg_ok_btn && m_can_source_choice->IsEmpty())
    m_dlg_ok_btn->Enable(false);
}

void ConnectionEditDialog::ShowNMEABT(bool visible) {
  if (m_dlg_ok_btn) m_dlg_ok_btn->Enable();

  if (visible) {
    if (m_scan_bt_btn) m_scan_bt_btn->Show();
    if (m_bt_pairs_text) m_bt_pairs_text->Show();
    if (m_bt_data_sources_choice) {
      if (m_bt_data_sources_choice->GetCount() > 1)
        m_bt_data_sources_choice->SetSelection(1);
      m_bt_data_sources_choice->Show();
    }
  } else {
    if (m_scan_bt_btn) m_scan_bt_btn->Hide();
    if (m_bt_pairs_text) m_bt_pairs_text->Hide();
    if (m_bt_data_sources_choice) m_bt_data_sources_choice->Hide();
  }
  m_sk_check_discover_chkbox->Hide();
  m_sk_check_discover_chkbox->Hide();  // Provisional
  m_sk_discover_btn->Hide();
  m_output_stc_tctrl->Show(visible);
  m_output_stc_list_btn->Show(visible);
  m_output_chkbox->Show(visible);
}

void ConnectionEditDialog::SetNMEAFormToSerial() {
  if (m_dlg_ok_btn) m_dlg_ok_btn->Enable();

  ShowNMEACommon(true);
  ShowNMEANet(false);
  ShowNMEAGPS(false);
  ShowNMEABT(false);
  ShowNMEASerial(true);
  ShowNMEACAN(false);
  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormToNet() {
  if (m_dlg_ok_btn) m_dlg_ok_btn->Enable();

  ShowNMEACommon(true);
  ShowNMEANet(true);
  ShowNMEAGPS(false);
  ShowNMEABT(false);
  ShowNMEASerial(false);
  ShowNMEACAN(false);
  SetDSFormRWStates();

  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormToCAN() {
  if (m_dlg_ok_btn) m_dlg_ok_btn->Enable();

  ShowNMEACommon(false);
  ShowNMEANet(false);
  ShowNMEAGPS(false);
  ShowNMEABT(false);
  ShowNMEASerial(false);
  ShowNMEACAN(true);
  m_in_filter_sizer->Show(false);
  m_out_filter_sizer->Show(false);
  SetDSFormRWStates();

  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormToGPS() {
  ShowNMEACommon(true);
  ShowNMEANet(false);
  ShowNMEAGPS(true);
  ShowNMEABT(false);
  ShowNMEASerial(false);
  ShowNMEACAN(false);

  // m_container->FitInside();
  //  Fit();
  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormToBT() {
  ShowNMEACommon(true);
  ShowNMEANet(false);
  ShowNMEAGPS(false);
  ShowNMEABT(true);
  ShowNMEASerial(false);
  ShowNMEACAN(false);

  // m_container->FitInside();
  //  Fit();
  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::ClearNMEAForm() {
  ShowNMEACommon(false);
  ShowNMEANet(false);
  ShowNMEAGPS(false);
  ShowNMEABT(false);
  ShowNMEASerial(false);
  ShowNMEACAN(false);

  // m_container->FitInside();
  //  Fit();
}

/*
 * Transitional: The network view is handled by  OnConnectionTypeChange()
 * and  RefreshAdvancedDetails(), remaining is handled here
 */
void ConnectionEditDialog::SetDSFormOptionVizStates() {
  bool advanced = m_advanced;
  m_collapse_box->ShowItems(true);
  m_input_chkbox->Show();
  m_output_chkbox->Show();

  ShowInFilter(advanced);
  ShowOutFilter(advanced);
  // Discovery hidden until it works.
  // m_cbCheckSKDiscover->Show();
  // m_ButtonSKDiscover->Show();
  m_sk_server_status_text->Show(advanced);

  if (m_type_serial_radiobtn->GetValue()) {
    m_sk_check_discover_chkbox->Hide();
    m_sk_discover_btn->Hide();
    m_sk_server_status_text->Hide();
    bool n0183ctlenabled =
        (DataProtocol)m_serial_protocol_choice->GetSelection() ==
        DataProtocol::PROTO_NMEA0183;
    bool n2kctlenabled =
        (DataProtocol)m_serial_protocol_choice->GetSelection() ==
        DataProtocol::PROTO_NMEA2000;
    if (!n0183ctlenabled) {
      if (n2kctlenabled) {
        m_input_chkbox->Show();
        m_output_chkbox->Show();
      } else {
        m_input_chkbox->Hide();
        m_output_chkbox->Hide();
      }
      ShowOutFilter(false);
      ShowInFilter(false);
      m_net_data_protocol_text->Hide();
      m_net_data_protocol_choice->Hide();
      m_net_expert_chkbox->Hide();
      m_net_type_choice_text->Hide();
      m_net_view_choice->Hide();
      m_net_expert_chkbox->Hide();
      m_net_type_choice_text->Hide();
      m_net_expert_box_text->Hide();
      m_net_view_choice->Hide();
    } else {
      m_input_chkbox->Show();
      m_input_chkbox->Enable();

      ShowInFilter(m_input_chkbox->IsChecked() && advanced);
      ShowOutFilter(m_output_chkbox->IsChecked() && advanced);

      m_garmin_host_chkbox->Show(m_input_chkbox->IsChecked() && advanced);
    }
  }

  if (m_type_internal_gps_radiobtn &&
      m_type_internal_gps_radiobtn->GetValue()) {
    m_sk_check_discover_chkbox->Hide();
    m_sk_discover_btn->Hide();
    m_sk_server_status_text->Hide();
    m_output_chkbox->Hide();
    m_input_chkbox->Hide();
    ShowOutFilter(false);
    ShowInFilter(false);
    m_garmin_host_chkbox->Hide();
    m_collapse_box->ShowItems(false);
  }

  if (m_type_internal_bt_radiobtn && m_type_internal_bt_radiobtn->GetValue()) {
    m_sk_check_discover_chkbox->Hide();
    m_sk_discover_btn->Hide();
    m_sk_server_status_text->Hide();

    ShowInFilter(m_input_chkbox->IsChecked() && advanced);
    ShowOutFilter(m_output_chkbox->IsChecked() && advanced);
  }

  if (m_type_can_radiobtn->GetValue()) {
    m_sk_check_discover_chkbox->Hide();
    m_sk_discover_btn->Hide();
    m_sk_server_status_text->Hide();
    m_garmin_host_chkbox->Hide();
    m_input_chkbox->Hide();
    m_output_chkbox->Hide();

    ShowInFilter(false);
    ShowOutFilter(false);

    m_net_data_protocol_text->Hide();
    m_net_data_protocol_choice->Hide();
    m_net_expert_chkbox->Hide();
    m_net_type_choice_text->Hide();
    m_net_expert_box_text->Hide();
    m_net_view_choice->Hide();
    m_collapse_box->Show(false);
  }

  if (m_type_net_radiobtn->GetValue()) {
    if ((DataProtocol)m_net_data_protocol_choice->GetSelection() ==
        DataProtocol::PROTO_NMEA2000) {
      ShowInFilter(false);
      ShowOutFilter(false);
    }
    if ((DataProtocol)m_net_data_protocol_choice->GetSelection() ==
        DataProtocol::PROTO_NMEA0183) {
      ShowInFilter(m_input_chkbox->IsChecked() && advanced);
      ShowOutFilter(m_output_chkbox->IsChecked() && advanced);
    }
  }
}

/*
 * Transitional: The network view is handled by  OnConnectionTypeChange()
 * and  RefreshAdvancedDetails(), remaining is handled here
 */
void ConnectionEditDialog::SetDSFormRWStates() {
  if (m_type_serial_radiobtn->GetValue()) {
    m_input_chkbox->Enable(true);
    m_output_chkbox->Enable(true);
    ShowInFilter();
    ShowOutFilter(m_output_chkbox->IsChecked());
  } else {
    m_o_accept_radiobtn->Enable(true);
    m_o_ignore_radiobtn->Enable(true);
    m_output_stc_list_btn->Enable(true);
  }
  SetDSFormOptionVizStates();
}

void ConnectionEditDialog::ShowInFilter(bool bshow) {
  m_in_filter_sizer->GetStaticBox()->Show(bshow);
  m_accept_radiobtn->Show(bshow);
  m_ignore_radiobtn->Show(bshow);
  m_input_stc_tctrl->Show(bshow);
  m_input_stc_list_btn->Show(bshow);
}

void ConnectionEditDialog::ShowOutFilter(bool bshow) {
  m_out_filter_sizer->GetStaticBox()->Show(bshow);
  m_o_accept_radiobtn->Show(bshow);
  m_o_ignore_radiobtn->Show(bshow);
  m_output_stc_tctrl->Show(bshow);
  m_output_stc_list_btn->Show(bshow);
}

void ConnectionEditDialog::PreloadControls(ConnectionParams* cp) {
  m_cp_original = cp;
  SetConnectionParams(cp);
}

void ConnectionEditDialog::SetConnectionParams(ConnectionParams* cp) {
  const std::string view = NetViewByConnection(cp);
  auto found = std::find(kBasicNetViews.begin(), kBasicNetViews.end(), view);
  m_net_expert_chkbox->SetValue(found == kBasicNetViews.end());
  m_net_view_choice->Clear();
  if (found == kBasicNetViews.end())
    for (const auto& v : kAdvancedNetViews) m_net_view_choice->Append(v);
  else
    for (const auto& v : kBasicNetViews) m_net_view_choice->Append(v);
  std::vector<std::string> all_views = kBasicNetViews;
  for (const auto& v : kAdvancedNetViews) all_views.push_back(v);
  found = std::find(all_views.begin(), all_views.end(), view);
  if (found != all_views.end()) {
    int select_ix = m_net_view_choice->FindString(*found);
    if (select_ix != wxNOT_FOUND) m_net_view_choice->SetSelection(select_ix);
  }
  if (wxNOT_FOUND == m_port_combo->FindString(cp->Port))
    m_port_combo->Append(cp->Port);

  m_port_combo->Select(m_port_combo->FindString(cp->Port));

  m_garmin_host_chkbox->SetValue(cp->Garmin);
  m_sk_check_discover_chkbox->SetValue(cp->AutoSKDiscover);
  if (view == kUdpReceive || view == kUdpInput) {
    m_input_chkbox->SetValue(true);
    m_input_chkbox->Disable();
    m_output_chkbox->SetValue(false);
    m_output_chkbox->Disable();
  } else {
    m_input_chkbox->SetValue(cp->IOSelect != DS_TYPE_OUTPUT);
    m_output_chkbox->SetValue(cp->IOSelect != DS_TYPE_INPUT);
  }

  if (cp->InputSentenceListType == WHITELIST)
    m_accept_radiobtn->SetValue(true);
  else
    m_ignore_radiobtn->SetValue(true);
  if (cp->OutputSentenceListType == WHITELIST)
    m_o_accept_radiobtn->SetValue(true);
  else
    m_o_ignore_radiobtn->SetValue(true);
  m_input_stc_tctrl->SetValue(StringArrayToString(cp->InputSentenceList));
  m_output_stc_tctrl->SetValue(StringArrayToString(cp->OutputSentenceList));
  m_baud_rate_choice->Select(
      m_baud_rate_choice->FindString(wxString::Format("%d", cp->Baudrate)));
  m_serial_protocol_choice->Select(cp->Protocol);  // TODO
  auto net_address = dynamic_cast<TextCtrlWithHelp*>(m_net_address_tctrl);
  if (net_address) m_net_address_tctrl->ChangeValue(cp->NetworkAddress);

  m_net_data_protocol_choice->Select(cp->Protocol);  // TODO

  if (cp->NetworkPort == 0)
    m_net_port_tctrl->ChangeValue("");
  else
    m_net_port_tctrl->ChangeValue(std::to_string(cp->NetworkPort));

  if (cp->Type == SERIAL) {
    m_type_serial_radiobtn->SetValue(true);
    SetNMEAFormToSerial();
    SetNMEAFormForSerialProtocol();
  } else if (cp->Type == NETWORK) {
    m_type_net_radiobtn->SetValue(true);
    SetNMEAFormToNet();
  } else if (cp->Type == SOCKETCAN) {
    m_type_can_radiobtn->SetValue(true);
    SetNMEAFormToCAN();
  } else if (cp->Type == INTERNAL_GPS) {
    if (m_type_internal_gps_radiobtn)
      m_type_internal_gps_radiobtn->SetValue(true);
    SetNMEAFormToGPS();
  } else if (cp->Type == INTERNAL_BT) {
    if (m_type_internal_bt_radiobtn)
      m_type_internal_bt_radiobtn->SetValue(true);
    SetNMEAFormToBT();

    // Preset the source selector
    wxString bts = cp->NetworkAddress + ";" + cp->GetPortStr();
    m_bt_data_sources_choice->Clear();
    m_bt_data_sources_choice->Append(bts);
    m_bt_data_sources_choice->SetSelection(0);
  } else {
    ClearNMEAForm();
  }

  if (cp->Type == SERIAL) {
    m_serial_comment_tctrl->SetValue(cp->UserComment);
  } else if (cp->Type == NETWORK) {
    m_net_comment_tctrl->SetValue(cp->UserComment);
    ConfigureControlsForView(view);
    OnExpertModeChange();
  }

  m_auth_token_tctrl->SetValue(cp->AuthToken);

  m_conn_enabled = cp->bEnabled;

  // Reset touch flag
  m_is_conn_saved = true;
}

void ConnectionEditDialog::SetDefaultConnectionParams() {
  if (m_port_combo && !m_port_combo->IsListEmpty()) {
    m_port_combo->Select(0);
    m_port_combo->SetValue(wxEmptyString);  // These two broke it
  }
  m_garmin_host_chkbox->SetValue(false);
  m_input_chkbox->SetValue(true);
  m_output_chkbox->SetValue(false);
  m_accept_radiobtn->SetValue(true);
  m_o_accept_radiobtn->SetValue(true);
  m_input_stc_tctrl->SetValue(wxEmptyString);
  m_output_stc_tctrl->SetValue(wxEmptyString);
  m_baud_rate_choice->Select(m_baud_rate_choice->FindString("4800"));
  //    m_choiceSerialProtocol->Select( cp->Protocol ); // TODO

  m_net_comment_tctrl->SetValue(wxEmptyString);
  m_serial_comment_tctrl->SetValue(wxEmptyString);
  m_auth_token_tctrl->SetValue(wxEmptyString);
  auto net_address = dynamic_cast<TextCtrlWithHelp*>(m_net_address_tctrl);
  if (net_address) net_address->RestoreHelp();
  auto net_port = dynamic_cast<TextCtrlWithHelp*>(m_net_port_tctrl);
  if (net_port) net_port->RestoreHelp();
  bool bserial = true;
#ifdef __WXGTK__
  bserial = false;
#endif

#ifdef __WXOSX__
  bserial = false;
#endif

#ifdef __ANDROID__
  if (m_type_internal_gps_radiobtn) {
    m_type_internal_gps_radiobtn->SetValue(true);
    SetNMEAFormToGPS();
  } else {
    m_type_net_radiobtn->SetValue(true);
    SetNMEAFormToNet();
  }
#else
  m_type_serial_radiobtn->SetValue(bserial);
  m_type_net_radiobtn->SetValue(!bserial);
  bserial ? SetNMEAFormToSerial() : SetNMEAFormToNet();
  m_type_can_radiobtn->SetValue(false);
#endif

  m_conn_enabled = true;

  // Reset touch flag
  m_is_conn_saved = false;
}

void ConnectionEditDialog::LayoutDialog() {
  m_net_props_sizer->Layout();
  m_ser_props_sizer->Layout();
  this->Layout();
  this->Fit();
  GetSizer()->Layout();
}

void ConnectionEditDialog::UpdateSourceList(bool bResort) {
  for (auto* cp : TheConnectionParams()) {
    ConnectionParamsPanel* panel = cp->m_optionsPanel;
    if (panel) panel->Update(cp);
  }

  m_scroll_win_connections->Layout();
}

void ConnectionEditDialog::OnSelectDatasource(wxListEvent& event) {
  SetConnectionParams(TheConnectionParams()[event.GetData()]);
  m_remove_btn->Enable();
  m_remove_btn->Show();
  event.Skip();
}

void ConnectionEditDialog::OnDiscoverButton(wxCommandEvent& event) {
#if 0  // FIXME (dave)
    wxString ip;
    int port;
    std::string serviceIdent =
        std::string("_signalk-ws._tcp.local.");  // Works for node.js server

    g_Platform->ShowBusySpinner();

    if (SignalKDataStream::DiscoverSKServer(serviceIdent, ip, port,
                                            1))  // 1 second scan
    {
      m_tNetAddress->SetValue(ip);
      m_tNetPort->SetValue(wxString::Format("%i", port));
      UpdateDiscoverStatus(_("Signal K server available."));
    } else {
      UpdateDiscoverStatus(_("Signal K server not found."));
    }
    g_Platform->HideBusySpinner();
#endif
  event.Skip();
}

void ConnectionEditDialog::UpdateDiscoverStatus(const wxString& stat) {
  m_sk_server_status_text->SetLabel(stat);
}

void ConnectionEditDialog::OnBtnIStcs(wxCommandEvent& event) {
  const ListType type = m_accept_radiobtn->GetValue() ? WHITELIST : BLACKLIST;
  const wxArrayString list =
      wxStringTokenize(m_input_stc_tctrl->GetValue(), ",");
  SentenceListDlg dlg(m_parent, FILTER_INPUT, type, list);

  if (dlg.ShowModal() == wxID_OK)
    m_input_stc_tctrl->SetValue(dlg.GetSentences());
}

void ConnectionEditDialog::OnBtnOStcs(wxCommandEvent& event) {
  const ListType type = m_o_accept_radiobtn->GetValue() ? WHITELIST : BLACKLIST;
  const wxArrayString list =
      wxStringTokenize(m_output_stc_tctrl->GetValue(), ",");
  SentenceListDlg dlg(m_parent, FILTER_OUTPUT, type, list);

  if (dlg.ShowModal() == wxID_OK)
    m_output_stc_tctrl->SetValue(dlg.GetSentences());
}

void ConnectionEditDialog::OnNetProtocolSelected(wxCommandEvent& event) {
  SetDSFormRWStates();
  LayoutDialog();
  OnConnValChange(event);
}

void ConnectionEditDialog::OnRbAcceptInput(wxCommandEvent& event) {
  OnConnValChange(event);
}
void ConnectionEditDialog::OnRbIgnoreInput(wxCommandEvent& event) {
  OnConnValChange(event);
}

void ConnectionEditDialog::OnRbOutput(wxCommandEvent& event) {
  OnConnValChange(event);
}

void ConnectionEditDialog::OnCbInput(wxCommandEvent& event) {
  const bool checked = m_input_chkbox->IsChecked();
  ShowInFilter(checked);
  SetDSFormRWStates();
  LayoutDialog();
  OnConnValChange(event);
}

void ConnectionEditDialog::OnCbOutput(wxCommandEvent& event) {
  OnConnValChange(event);
  const bool is_output_enabled = m_output_chkbox->IsChecked();
  ShowOutFilter(is_output_enabled);

  int selection = m_net_view_choice->GetSelection();
  std::string view;
  if (selection != wxNOT_FOUND)
    view = m_net_view_choice->GetString(selection).ToStdString();
  if (view == kUdpInput || view == kMulticastServer) {
    if (is_output_enabled) {
      // Check for a UDP input connection on the same port
      NetworkProtocol proto = UDP;
      for (auto* cp : TheConnectionParams()) {
        if (cp->NetProtocol == proto &&
            cp->NetworkPort == wxAtoi(m_net_port_tctrl->GetValue()) &&
            cp->IOSelect == DS_TYPE_INPUT) {
          wxString mes;
          bool warn = false;
          if (cp->bEnabled) {
            mes =
                _("There is an enabled UDP input connection that uses the "
                  "same data port.");
            mes << "\n"
                << _("Please apply a filter on both connections to avoid a "
                     "feedback loop.");
            warn = true;
          } else {
            mes =
                _("There is a disabled UDP Input connection that uses the "
                  "same Dataport.");
            mes << "\n"
                << _("If you enable that input please apply a filter on both "
                     "connections to avoid a  feedback loop.");
          }
          mes << "\n"
              << _("Or consider using a different data port for one of them");
          if (warn)
            OCPNMessageBox(this, mes, _("OpenCPN Warning"),
                           wxOK | wxICON_EXCLAMATION, 60);
          else
            OCPNMessageBox(this, mes, _("OpenCPN info"),
                           wxOK | wxICON_INFORMATION, 60);
          break;
        }
      }
    }
  }
  if (view == kUdpReceive || view == kUdpInput) {
    m_net_address_tctrl->Hide();
    m_net_address_tctrl->Disable();
    m_net_addr_text->Hide();
    m_net_addr_text->Disable();
  }
  SetDSFormRWStates();
  RefreshAdvancedDetails();
  LayoutDialog();
}

void ConnectionEditDialog::OnCollapsedToggle(bool collapsed) {
  m_advanced = !collapsed;
  if (m_type_net_radiobtn->GetValue())
    SetNMEAFormForNetProtocol();
  else
    SetNMEAFormForSerialProtocol();
  RefreshAdvancedDetails();
  LayoutDialog();
}

void ConnectionEditDialog::OnCbAdvanced(wxCommandEvent& event) {
  if (m_type_net_radiobtn->GetValue())
    SetNMEAFormForNetProtocol();
  else
    SetNMEAFormForSerialProtocol();
  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormForSerialProtocol() {
  bool n0183ctlenabled =
      (DataProtocol)m_serial_protocol_choice->GetSelection() ==
      DataProtocol::PROTO_NMEA0183;
  bool advanced = m_advanced;
  ShowNMEACommon(n0183ctlenabled && advanced);
  m_garmin_host_chkbox->Show(n0183ctlenabled && advanced);

  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormForNetProtocol() {
  bool n0183ctlenabled =
      (DataProtocol)m_net_data_protocol_choice->GetSelection() ==
      DataProtocol::PROTO_NMEA0183;
  bool advanced = m_advanced;
  ShowNMEACommon(n0183ctlenabled && advanced);
  m_garmin_host_chkbox->Show(n0183ctlenabled && advanced);

  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::OnProtocolChoice(wxCommandEvent& event) {
  if (m_type_net_radiobtn->GetValue())
    SetNMEAFormForNetProtocol();
  else
    SetNMEAFormForSerialProtocol();
  OnConnValChange(event);
}

ConnectionParams* ConnectionEditDialog::GetParamsFromControls() {
  auto* pConnectionParams = new ConnectionParams();
  UpdateConnectionParamsFromControls(pConnectionParams);
  return pConnectionParams;
}

ConnectionParams* ConnectionEditDialog::UpdateConnectionParamsFromControls(
    ConnectionParams* pConnectionParams) {
  pConnectionParams->Valid = true;
  int selection = m_net_view_choice->GetSelection();
  if (selection != wxNOT_FOUND) {
    std::string s = m_net_view_choice->GetString(selection).ToStdString();
  }
  if (m_type_serial_radiobtn->GetValue())
    pConnectionParams->Type = SERIAL;
  else if (m_type_net_radiobtn->GetValue())
    pConnectionParams->Type = NETWORK;
  else if (m_type_internal_gps_radiobtn &&
           m_type_internal_gps_radiobtn->GetValue())
    pConnectionParams->Type = INTERNAL_GPS;
  else if (m_type_internal_bt_radiobtn &&
           m_type_internal_bt_radiobtn->GetValue())
    pConnectionParams->Type = INTERNAL_BT;
  else if (m_type_can_radiobtn && m_type_can_radiobtn->GetValue())
    pConnectionParams->Type = SOCKETCAN;

  if (m_type_net_radiobtn->GetValue()) {
    //  Save the existing addr/port to allow closing of existing port
    pConnectionParams->LastNetworkAddress = pConnectionParams->NetworkAddress;
    pConnectionParams->LastNetworkPort = pConnectionParams->NetworkPort;
    pConnectionParams->LastNetProtocol = pConnectionParams->NetProtocol;
    pConnectionParams->LastDataProtocol = pConnectionParams->Protocol;

    pConnectionParams->NetworkAddress =
        m_net_address_tctrl->GetValue().Trim(false).Trim(true);
    pConnectionParams->NetworkPort =
        wxAtoi(m_net_port_tctrl->GetValue().Trim(false).Trim(true));
    int net_select = m_net_view_choice->GetSelection();
    std::string net_type;
    if (net_select != wxNOT_FOUND)
      net_type = m_net_view_choice->GetString(net_select).ToStdString();
    if (net_type == kTcpClient || net_type == kTcpServer ||
        net_type == kTcpDevice) {
      pConnectionParams->NetProtocol = TCP;
      pConnectionParams->Protocol =
          static_cast<DataProtocol>(m_net_data_protocol_choice->GetSelection());
    } else if (net_type == kUdpSend || net_type == kUdpInput ||
               net_type == kUdpReceive || net_type == kMulticastClient ||
               net_type == kMulticastServer) {
      pConnectionParams->NetProtocol = UDP;
      pConnectionParams->Protocol =
          static_cast<DataProtocol>(m_net_data_protocol_choice->GetSelection());
    } else if (net_type == kGpsdClient || net_type == kGpsdDevice) {
      pConnectionParams->NetProtocol = GPSD;
    } else if (net_type == kSignalkClient || net_type == kSignalkDevice) {
      pConnectionParams->NetProtocol = SIGNALK;
    } else {
      pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    };
    pConnectionParams->is_server =
        net_type == kTcpServer || net_type == kUdpInput ||
        net_type == kUdpReceive || net_type == kMulticastServer;
  }
  if (m_type_serial_radiobtn->GetValue())
    pConnectionParams->Protocol =
        (DataProtocol)m_serial_protocol_choice->GetSelection();
  else if (m_type_net_radiobtn->GetValue())
    pConnectionParams->Protocol =
        (DataProtocol)m_net_data_protocol_choice->GetSelection();

  pConnectionParams->Baudrate =
      wxAtoi(m_baud_rate_choice->GetStringSelection());
  pConnectionParams->ChecksumCheck = true;
  pConnectionParams->AutoSKDiscover = m_sk_check_discover_chkbox->GetValue();
  pConnectionParams->Garmin = m_garmin_host_chkbox->GetValue();
  pConnectionParams->InputSentenceList =
      wxStringTokenize(m_input_stc_tctrl->GetValue(), ",");
  if (m_accept_radiobtn->GetValue())
    pConnectionParams->InputSentenceListType = WHITELIST;
  else
    pConnectionParams->InputSentenceListType = BLACKLIST;
  if (m_input_chkbox->GetValue()) {
    if (m_output_chkbox->GetValue()) {
      pConnectionParams->IOSelect = DS_TYPE_INPUT_OUTPUT;
    } else {
      pConnectionParams->IOSelect = DS_TYPE_INPUT;
    }
  } else
    pConnectionParams->IOSelect = DS_TYPE_OUTPUT;

  pConnectionParams->OutputSentenceList =
      wxStringTokenize(m_output_stc_tctrl->GetValue(), ",");
  if (m_o_accept_radiobtn->GetValue())
    pConnectionParams->OutputSentenceListType = WHITELIST;
  else
    pConnectionParams->OutputSentenceListType = BLACKLIST;
  pConnectionParams->Port = m_port_combo->GetValue().BeforeFirst(' ');
#if defined(__linux__) && !defined(__ANDROID__)
  if (pConnectionParams->Type == SERIAL)
    CheckSerialAccess(m_parent, pConnectionParams->Port.ToStdString());
#endif

  if (m_type_can_radiobtn && m_type_can_radiobtn->GetValue())
    pConnectionParams->Protocol = PROTO_NMEA2000;

  pConnectionParams->bEnabled = m_conn_enabled;
  pConnectionParams->b_IsSetup = false;

  if (pConnectionParams->Type == INTERNAL_GPS) {
    pConnectionParams->NetworkAddress = "";
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
    pConnectionParams->Port = "Internal GPS";
  }

  if (pConnectionParams->Type == INTERNAL_BT) {
    wxString parms = m_bt_data_sources_choice->GetStringSelection();
    wxStringTokenizer tkz(parms, ";");
    wxString name = tkz.GetNextToken();
    wxString mac = tkz.GetNextToken();

    pConnectionParams->NetworkAddress = name;
    pConnectionParams->Port = mac;
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
    //        pConnectionParams->SetAuxParameterStr(m_choiceBTDataSources->GetStringSelection());
  }

  if (pConnectionParams->Type == SOCKETCAN) {
    pConnectionParams->NetworkAddress = "";
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
    pConnectionParams->socketCAN_port =
        m_can_source_choice->GetString(m_can_source_choice->GetSelection());
  }
  if (pConnectionParams->Type == SERIAL) {
    pConnectionParams->UserComment = m_serial_comment_tctrl->GetValue();
  } else if (pConnectionParams->Type == NETWORK) {
    pConnectionParams->UserComment = m_net_comment_tctrl->GetValue();
  }
  pConnectionParams->AuthToken = m_auth_token_tctrl->GetValue();

  return pConnectionParams;
}

void ConnectionEditDialog::OnPriorityDialog(wxCommandEvent& event) {
  auto* pdlg = new PriorityDlg(m_parent);
  pdlg->ShowModal();
  delete pdlg;
}
void ConnectionEditDialog::ConnectControls() {
  // Connect controls
  m_type_serial_radiobtn->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnTypeSerialSelected),
      nullptr, this);
  m_type_net_radiobtn->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnTypeNetSelected), nullptr,
      this);
  m_type_can_radiobtn->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnTypeCANSelected), nullptr,
      this);
  if (m_type_internal_gps_radiobtn)
    m_type_internal_gps_radiobtn->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(ConnectionEditDialog::OnTypeGPSSelected), nullptr,
        this);
  if (m_type_internal_bt_radiobtn)
    m_type_internal_bt_radiobtn->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(ConnectionEditDialog::OnTypeBTSelected), nullptr,
        this);

  m_net_data_protocol_choice->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnProtocolChoice), nullptr,
      this);
  m_serial_protocol_choice->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnProtocolChoice), nullptr,
      this);

  // input/output control
  m_input_chkbox->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnCbInput), nullptr, this);
  m_output_chkbox->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnCbOutput), nullptr, this);

  if (m_scan_bt_btn)
    m_scan_bt_btn->Connect(
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ConnectionEditDialog::OnScanBtClick), nullptr,
        this);

  m_input_stc_list_btn->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnBtnIStcs), nullptr, this);

  // output filtering
  m_output_stc_list_btn->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnBtnOStcs), nullptr, this);
}

SentenceListDlg::SentenceListDlg(wxWindow* parent, FilterDirection dir,
                                 ListType type, const wxArrayString& list)
    : wxDialog(parent, wxID_ANY, _("Sentence Filter"), wxDefaultPosition,
               wxSize(280, 420)),
      m_type(type),
      m_dir(dir),
      m_sentences(NMEA0183().GetRecognizedArray()) {
  m_sentences.Sort();
  auto* mainSizer = new wxBoxSizer(wxVERTICAL);
  auto* secondSizer = new wxBoxSizer(wxHORIZONTAL);
  auto* pclbBox = new wxStaticBox(this, wxID_ANY, GetBoxLabel());
  auto* stcSizer = new wxStaticBoxSizer(pclbBox, wxVERTICAL);
  m_sentences_clb = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition,
                                       wxDefaultSize, m_sentences);
  auto* btnEntrySizer = new wxBoxSizer(wxVERTICAL);
  auto* btnCheckAll = new wxButton(this, wxID_ANY, _("Select All"));
  auto* btnClearAll = new wxButton(this, wxID_ANY, _("Clear All"));
  auto* btnAdd = new wxButton(this, wxID_ANY, _("Add"));
  m_del_btn = new wxButton(this, wxID_ANY, _("Delete"));
  m_del_btn->Disable();
  auto* btnSizer = new wxStdDialogButtonSizer();
  auto* btnOK = new wxButton(this, wxID_OK);
  auto* btnCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));

  secondSizer->Add(stcSizer, 1, wxALL | wxEXPAND, 5);
  stcSizer->Add(m_sentences_clb, 1, wxALL | wxEXPAND, 5);
  btnEntrySizer->Add(btnCheckAll, 0, wxALL, 5);
  btnEntrySizer->Add(btnClearAll, 0, wxALL, 5);
  btnEntrySizer->AddSpacer(1);
  btnEntrySizer->Add(btnAdd, 0, wxALL, 5);
  btnEntrySizer->Add(m_del_btn, 0, wxALL, 5);
  secondSizer->Add(btnEntrySizer, 0, wxALL | wxEXPAND, 5);
  mainSizer->Add(secondSizer, 1, wxEXPAND, 5);
  btnSizer->AddButton(btnOK);
  btnSizer->AddButton(btnCancel);
  btnSizer->Realize();
  mainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

  SetSizer(mainSizer);
  mainSizer->SetSizeHints(this);
  Centre();

  btnAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                  wxCommandEventHandler(SentenceListDlg::OnAddClick), nullptr,
                  this);
  m_del_btn->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(SentenceListDlg::OnDeleteClick),
                     nullptr, this);
  m_sentences_clb->Connect(wxEVT_COMMAND_LISTBOX_SELECTED,
                           wxCommandEventHandler(SentenceListDlg::OnCLBSelect),
                           nullptr, this);
  btnCheckAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(SentenceListDlg::OnCheckAllClick),
                       nullptr, this);
  btnClearAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(SentenceListDlg::OnClearAllClick),
                       nullptr, this);
  Populate(list);
}

wxString SentenceListDlg::GetBoxLabel() const {
  if (m_dir == FILTER_OUTPUT)
    return m_type == WHITELIST ? _("Transmit sentences") : _("Drop sentences");
  else
    return m_type == WHITELIST ? _("Accept only sentences")
                               : _("Ignore sentences");
}

void SentenceListDlg::Populate(const wxArrayString& list) {
  if (m_dir == FILTER_OUTPUT) {
    wxString s;
    m_sentences.Add(g_TalkerIdText + wxString("RMB"));
    m_sentences.Add(g_TalkerIdText + wxString("RMC"));
    m_sentences.Add(g_TalkerIdText + wxString("APB"));
    m_sentences.Add(g_TalkerIdText + wxString("XTE"));
  }
  m_sentences.Add("AIVDM");
  m_sentences.Add("AIVDO");
  m_sentences.Add("FRPOS");
  m_sentences.Add(g_TalkerIdText);
  m_sentences.Add("CD");
  m_sentences.Sort();
  m_sentences_clb->Clear();
  m_sentences_clb->InsertItems(m_sentences, 0);

  wxArrayString new_strings;
  if (list.Count() == 0) {
    for (size_t i = 0; i < m_sentences_clb->GetCount(); ++i)
      m_sentences_clb->Check(i, m_type == WHITELIST);
  } else {
    for (size_t i = 0; i < list.Count(); ++i) {
      int item = m_sentences_clb->FindString(list[i]);
      if (item != wxNOT_FOUND)
        m_sentences_clb->Check(item);
      else
        new_strings.Add(list[i]);
    }
    if (new_strings.GetCount()) {
      m_sentences_clb->InsertItems(new_strings, m_sentences_clb->GetCount());
      for (size_t i = 0; i < new_strings.GetCount(); ++i) {
        int item = m_sentences_clb->FindString(new_strings[i]);
        if (item != wxNOT_FOUND) m_sentences_clb->Check(item);
      }
    }
  }
}

wxString SentenceListDlg::GetSentences() {
  wxArrayString retString;
  for (size_t i = 0; i < m_sentences_clb->GetCount(); i++) {
    if (m_sentences_clb->IsChecked(i))
      retString.Add(m_sentences_clb->GetString(i));
  }
  return StringArrayToString(retString);
}

void SentenceListDlg::OnCLBSelect(wxCommandEvent& e) {
  // Only activate the "Delete" button if the selection is not in the standard
  // list
  m_del_btn->Enable(m_sentences.Index(e.GetString()) == wxNOT_FOUND);
}

void SentenceListDlg::OnAddClick(wxCommandEvent& event) {
#ifdef __ANDROID__
  androidDisableRotation();
#endif

  wxTextEntryDialog textdlg(
      this,
      _("Enter the NMEA sentence (2, 3 or 5 characters)\n  or a valid REGEX "
        "expression (6 characters or longer)"),
      _("Enter the NMEA sentence"));

  textdlg.SetTextValidator(wxFILTER_ASCII);
  int result = textdlg.ShowModal();

#ifdef __ANDROID__
  androidEnableRotation();
#endif

  if (result == wxID_CANCEL) return;
  wxString stc = textdlg.GetValue();

  if (stc.Length() == 2 || stc.Length() == 3 || stc.Length() == 5) {
    m_sentences_clb->Append(stc);
    m_sentences_clb->Check(m_sentences_clb->FindString(stc));
    return;
  } else if (stc.Length() < 2) {
    OCPNMessageBox(
        this,
        _("An NMEA sentence is generally 3 characters long (like RMC, GGA etc.)\n \
            It can also have a two letter prefix identifying the source, or TALKER, of the message.\n \
            The whole sentences then looks like GPGGA or AITXT.\n \
            You may filter out all the sentences with certain TALKER prefix (like GP, AI etc.).\n \
            The filter also accepts Regular Expressions (REGEX) with 6 or more characters. \n\n"),
        _("OpenCPN Info"));
    return;
  }

  else {
    // Verify that a longer text entry is a valid RegEx
    wxRegEx r(stc);
    if (r.IsValid()) {
      m_sentences_clb->Append(stc);
      m_sentences_clb->Check(m_sentences_clb->FindString(stc));
      return;
    } else {
      OCPNMessageBox(this, _("REGEX syntax error: \n") + stc,
                     _("OpenCPN Info"));
      return;
    }
  }
}

void SentenceListDlg::OnDeleteClick(wxCommandEvent& event) {
  m_sentences_clb->Delete(m_sentences_clb->GetSelection());
}

void SentenceListDlg::OnClearAllClick(wxCommandEvent& event) {
  for (size_t i = 0; i < m_sentences_clb->GetCount(); i++)
    m_sentences_clb->Check(i, false);
}

void SentenceListDlg::OnCheckAllClick(wxCommandEvent& event) {
  for (size_t i = 0; i < m_sentences_clb->GetCount(); i++)
    m_sentences_clb->Check(i, true);
}
