/**************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
 *   Copyright (C) 2062 Alec Leamas                                        *
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
 * Implement conn_edit_commons.h -- Internal support for connection_edit.cpp
 */
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "conn_edit_commons.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>

#include "libgui/expand_panel.h"

#include "model/ocpn_utils.h"
#include "model/ser_ports.h"
#include "model/sys_events.h"

#include "filter_sentence_dlg.h"

// Make _() return std::string instead of wxString;
#undef _
#if wxCHECK_VERSION(3, 2, 0)
#define _(s) wxGetTranslation(wxASCII_STR(s)).ToStdString()
#else
#define _(s) wxGetTranslation((s)).ToStdString()
#endif

namespace conn_edit {

static const std::string kAddressDefaultHelp =
    _("Enter IP address or hostname");
static const std::string kAddressUdpHelp =
    _("IP address or hostname, often 255.255.255.255");
static const std::string kAddressMcastHelp =
    _("Group address, usually 224.0.2.0 - 224.0.255.255");
static const std::string kMcastPortHelp =
    _("Port number, usually 49152 - 65535");
static const std::string kTcpDevice = _("Device using TCP");
static const std::string kUdpReceive = _("Device sending UDP");
static const std::string kUdpOutput = _("UDP send");
static const std::string kGpsdDevice = _("Gpsd server");
static const std::string kSignalkDevice = _("SignalK server");
static const std::string kTcpClient = _("TCP client");
static const std::string kUdpSend = _("Device(s) receiving UDP");
static const std::string kGpsdClient = _("Gpsd client");
static const std::string kSignalkClient = _("SignalK client");
static const std::string kTcpServer = _("TCP Server");
static const std::string kUdpInput = _("UDP Receive");
static const std::string kUdpInputOutput = _("UDP Receive and Send");
static const std::string kMulticastServer = _("UDP Multicast Receive and Send");
static const std::string kMulticastClient = _("UDP Multicast Send");
static const std::string kDefaultGpsdPort = "2947";
static const std::string kDefaultSignalkPort = "3000";

const std::vector<std::string> kBasicNetViews = {
    kTcpDevice, kUdpReceive, kUdpSend, kGpsdDevice, kSignalkDevice};

const std::vector<std::string> kAdvancedNetViews = {
    // First items matches kBasicNetViews
    kTcpClient,      kUdpInput,        kUdpOutput,
    kGpsdClient,     kSignalkClient,   kTcpServer,
    kUdpInputOutput, kMulticastClient, kMulticastServer};

void LoadSerialPorts(wxComboBox* box) {
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

bool CheckPort(wxWindow* parent, const TextCtrlWithHelp& ctrl) {
  if (ctrl.IsPristine() || ctrl.GetValue().empty()) {
    auto dlg = wxMessageDialog(parent, _("Required field port is missing"),
                               _("OpenCPN error"), wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  }
  int port;
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

bool CheckAddress(wxWindow* parent, const TextCtrlWithHelp& ctrl) {
  if (ctrl.IsPristine() || ctrl.GetValue().empty()) {
    auto dlg = wxMessageDialog(parent, _("Required field address is missing"),
                               _("OpenCPN error"), wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  }
  // Checking the address requires using gethostbyname() or so since it
  // could be a hostname. Not worthwhile in this context.
  return true;
}

bool IsAddressMultiCast(const wxString& ip) {
  wxArrayString bytes = wxSplit(ip, '.');
  if (bytes.size() != 4) {
    return false;
  }
  unsigned long ip_num = (wxAtoi(bytes[0]) << 24) + (wxAtoi(bytes[1]) << 16) +
                         (wxAtoi(bytes[2]) << 8) + wxAtoi(bytes[3]);
  unsigned long multicast_start = (224 << 24);
  unsigned long multicast_end = (239 << 24) + (255 << 16) + (255 << 8) + 255;
  return ip_num >= multicast_start && ip_num <= multicast_end;
}

static wxString StringArrayToString(const wxArrayString& arr) {
  wxString ret;
  for (size_t i = 0; i < arr.Count(); i++) {
    if (i > 0) ret.Append(",");
    ret.Append(arr[i]);
  }
  return ret;
}

class ProtocolChoiceImpl : public ProtocolChoice {
  friend ProtocolChoice* ProtocolChoice::Create(wxWindow*, ConnectionParams&,
                                                std::function<void()>);

public:
  void Reload() override {
    Clear();
    Append("NMEA 0183");
    Append("NMEA 2000");
    switch (m_params.data_protocol) {
      case PROTO_NMEA0183:
        SetSelection(0);
        break;
      case PROTO_NMEA2000:
        SetSelection(1);
        break;
      case PROTO_SIGNALK:
        Clear();
        Append("SignalK");
        SetSelection(0);
        break;
      case PROTO_GPSD:
        Clear();
        Append("Gpsd");
        SetSelection(0);
        break;
      default:
        assert(false && "Illegal data  protocol");
        break;
    }
    Enable();
  }

protected:
  ProtocolChoiceImpl(
      wxWindow* parent, ConnectionParams& params,
      std::function<void()> on_change = [] {})
      : ProtocolChoice(parent),
        m_params(params),
        m_on_change(std::move(on_change)) {
    ProtocolChoiceImpl::Reload();
    Bind(wxEVT_CHOICE, [&](wxCommandEvent& e) { OnChoice(e); });
  }

private:
  ConnectionParams& m_params;
  std::function<void()> m_on_change;

  void OnChoice(wxCommandEvent& e) {
    switch (GetSelection()) {
      case 0:
        m_params.data_protocol = PROTO_NMEA0183;
        break;
      case 1:
        m_params.data_protocol = PROTO_NMEA2000;
        break;
      default:
        assert(false && "Illegal protocol combobox value");
    }
    m_on_change();
    e.Skip();
  }
};  // ProtocolChoiceImpl

ProtocolChoice* ProtocolChoice::Create(wxWindow* parent,
                                       ConnectionParams& params,
                                       std::function<void()> on_change) {
  return new ProtocolChoiceImpl(parent, params, std::move(on_change));
}

class PortTextCtrlImpl : public PortTextCtrl {
  friend PortTextCtrl* PortTextCtrl::Create(wxWindow*, ConnectionParams&);

public:
  void Reload() override {
    auto string = std::to_string(m_params.network_port);
    ChangeValue(string == "0" ? "" : string);
    if (GetValue().empty()) RestoreHelp();
  }

  void ChangeValue(const wxString& value) override {
    TextCtrlWithHelp::ChangeValue(value);
    OnChange();
  }

protected:
  PortTextCtrlImpl(wxWindow* parent, ConnectionParams& params)
      : PortTextCtrl(parent), m_params(params) {
    PortTextCtrlImpl::Reload();
    Bind(wxEVT_TEXT, [&](wxCommandEvent& e) { OnChange(e); });
  }

private:
  ConnectionParams& m_params;

  void OnChange() {
    if (GetValue().empty()) return;
    try {
      m_params.network_port = std::stoi(GetValue().ToStdString());
    } catch (std::logic_error& e) {
      wxLogWarning("Cannot parse configuration network port: %s", e.what());
      m_params.network_port = 99999;  // Illegal for ipv4
    }
  }

  void OnChange(wxCommandEvent& event) {
    OnChange();
    event.Skip();
  }
};  // PortTextCtrlImpl

PortTextCtrl* PortTextCtrl::Create(wxWindow* parent, ConnectionParams& params) {
  return new PortTextCtrlImpl(parent, params);
}

class InputOutputPanelImpl : public InputOutputPanel {
  friend InputOutputPanel* InputOutputPanel::Create(wxWindow*,
                                                    ConnectionParams&);

  class InputCheckBox : public wxCheckBox, public Reloadable {
  public:
    InputCheckBox(wxWindow* parent, ConnectionParams& params,
                  std::function<void()> on_change)
        : wxCheckBox(parent, wxID_ANY, _("Receive Input on this Port")),
          m_params(params),
          m_on_change(std::move(on_change)) {
      InputCheckBox::Reload();
      Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& ev) {
        m_on_change();
        ev.Skip();
      });
    }

    void Reload() override {
      SetValue(m_params.direction != PortDirection::kOutput);
    }

  private:
    ConnectionParams& m_params;
    std::function<void()> m_on_change;
  };

  class OutputCheckBox : public wxCheckBox, public Reloadable {
  public:
    OutputCheckBox(wxWindow* parent, ConnectionParams& params,
                   std::function<void()> on_change)
        : wxCheckBox(parent, wxID_ANY,
                     _("Output on this port as autopilot or NMEA repeater")),
          m_params(params),
          m_on_change(std::move(on_change)) {
      OutputCheckBox::Reload();
      Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& ev) {
        m_on_change();
        ev.Skip();
      });
    };

    void Reload() override {
      SetValue(m_params.direction != PortDirection::kInput);
    }

  private:
    ConnectionParams& m_params;
    std::function<void()> m_on_change;
  };

public:
  void EnableOutput(bool enabled) override {
    m_output_box->Enable(enabled);
    OnChange();
  }

  void EnableInput(bool enabled) override {
    m_input_box->Enable(enabled);
    OnChange();
  }

  void SetOutput(bool value) override {
    m_output_box->SetValue(value);
    OnChange();
  }

  void SetInput(bool value) override {
    m_input_box->SetValue(value);
    OnChange();
  }

protected:
  InputOutputPanelImpl(wxWindow* parent, ConnectionParams& params)
      : InputOutputPanel(parent), m_params(params) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Border();
    m_input_box = new InputCheckBox(this, m_params, [&] { OnChange(); });
    m_output_box = new OutputCheckBox(this, m_params, [&] { OnChange(); });
    vbox->Add(m_input_box, flags);
    vbox->Add(m_output_box, flags);
    SetSizer(vbox);
    wxWindow::Layout();
  }

private:
  ConnectionParams& m_params;
  wxCheckBox* m_input_box;
  wxCheckBox* m_output_box;

  void OnChange() {
    if (m_input_box->GetValue() && m_output_box->GetValue())
      m_params.direction = PortDirection::kInOut;
    else if (m_input_box->GetValue())
      m_params.direction = PortDirection::kInput;
    else if (m_output_box->GetValue())
      m_params.direction = PortDirection::kOutput;
    else
      wxLogWarning("Inconsistent GUI direction state");
  }
};  //  InputOutputPanelImpl

InputOutputPanel* InputOutputPanel::Create(wxWindow* parent,
                                           ConnectionParams& params) {
  return new InputOutputPanelImpl(parent, params);
}

/** Configure input filters: accept or ignore list of sentences. */
class FilterConfigPanelImpl : public FilterConfigPanel {
  friend FilterConfigPanel* FilterConfigPanel::Create(wxWindow*,
                                                      ConnectionParams&);

  /** Accept/ignore buttons and  filtered sentences container. */
  class Statbox : public wxStaticBoxSizer {
  public:
    Statbox(wxWindow* parent, ListType& type, wxArrayString& list,
            const std::string& label)
        : wxStaticBoxSizer(wxVERTICAL, parent, label),
          m_parent(parent),
          m_type(type),
          m_list(list) {
      auto radio_btn_box = new wxBoxSizer(wxHORIZONTAL);
      m_accept_btn =
          new wxRadioButton(parent, wxID_ANY, _("Accept only sentences"),
                            wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
      m_ignore_btn = new wxRadioButton(parent, wxID_ANY, _("Ignore sentences"));
      radio_btn_box->Add(m_accept_btn, wxSizerFlags().Border());
      radio_btn_box->Add(m_ignore_btn, wxSizerFlags().Border());

      auto setup_box = new wxBoxSizer(wxHORIZONTAL);
      auto flags1 = wxSizerFlags(1);
      auto flags0 = wxSizerFlags(0);
      m_sentences_text =
          new wxStaticText(parent, wxID_ANY, "", wxDefaultPosition,
                           wxDefaultSize, wxST_ELLIPSIZE_END);
      m_edit_btn = new wxButton(parent, wxID_ANY, "...");
      m_edit_btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent& e) { OnEdit(e); });
      m_edit_btn->SetMaxSize(wxSize(parent->GetCharWidth() * 4, -1));
      setup_box->Add(m_sentences_text, flags1.Expand());
      setup_box->Add(m_edit_btn, flags0.Border(wxDOWN | wxRIGHT | wxLEFT));

      auto vbox = new wxBoxSizer(wxVERTICAL);
      vbox->Add(radio_btn_box, wxSizerFlags());
      vbox->Add(setup_box, flags1.DoubleBorder(wxLEFT | wxRIGHT | wxDOWN));
      Add(vbox);
      wxSizer::Layout();

      Statbox::Reload();
      m_ignore_btn->Bind(wxEVT_RADIOBUTTON,
                         [&](wxCommandEvent& e) { OnDirectionChange(e); });
      m_accept_btn->Bind(wxEVT_RADIOBUTTON,
                         [&](wxCommandEvent& e) { OnDirectionChange(e); });
    }

    void Reload() {
      m_accept_btn->SetValue(m_type == ListType::WHITELIST);
      m_ignore_btn->SetValue(m_type == ListType::BLACKLIST);
      auto text = StringArrayToString(m_list);
      text = text.empty() ? _("No sentences selected") : text;
      m_sentences_text->SetLabel(text);
      m_parent->GetGrandParent()->Layout();
    }

  private:
    wxWindow* m_parent;
    ListType& m_type;
    wxArrayString& m_list;
    wxRadioButton* m_accept_btn;
    wxRadioButton* m_ignore_btn;
    wxStaticText* m_sentences_text;
    wxButton* m_edit_btn;

    void OnDirectionChange(wxCommandEvent& e) {
      if (m_accept_btn->GetValue()) m_type = ListType::WHITELIST;
      if (m_ignore_btn->GetValue()) m_type = ListType::BLACKLIST;
      e.Skip();
    }

    void OnEdit(wxCommandEvent&) {
      static const std::string kNothingHere = _("No sentences selected");
      if (m_sentences_text->GetLabel() == kNothingHere)
        m_sentences_text->SetLabel("");
      const wxArrayString list =
          wxStringTokenize(m_sentences_text->GetLabel(), ",");
      FilterSentenceDlg dlg(m_parent, FILTER_INPUT, m_type, list);
      if (dlg.ShowModal() != wxID_OK) return;

      std::string text = dlg.GetSentences().ToStdString();
      m_list = wxStringTokenize(text, ",");
      if (text.empty()) text = kNothingHere;
      m_sentences_text->SetLabel(text);
      m_parent->Layout();
    }
  };  // Statbox

public:
  void Reload() override {
    m_output_filter->Reload();
    m_input_filter->Reload();
  }

protected:
  FilterConfigPanelImpl(wxWindow* parent, ConnectionParams& params)
      : FilterConfigPanel(parent), m_params(params) {
    auto* vbox = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Border();
    m_input_filter =
        new Statbox(this, m_params.input_sentence_list_type,
                    m_params.input_sentence_list, _("Input filtering"));
    vbox->Add(m_input_filter, flags);
    m_output_filter =
        new Statbox(this, m_params.output_sentence_list_type,
                    m_params.output_sentence_list, _("Output filtering"));
    vbox->Add(m_output_filter, flags);
    SetSizer(vbox);
    wxWindowBase::Layout();
  }

private:
  Statbox* m_input_filter;
  Statbox* m_output_filter;
  ConnectionParams& m_params;
};  // FilterConfigPanelImpl

FilterConfigPanel* FilterConfigPanel::Create(wxWindow* parent,
                                             ConnectionParams& params) {
  return new FilterConfigPanelImpl(parent, params);
}

class PortComboImpl : public PortCombo {
  friend PortCombo* PortCombo::Create(wxWindow*, wxString&);

public:
  void Reload() override { SetValue(m_port); }

protected:
  PortComboImpl(wxWindow* parent, wxString& port)
      : PortCombo(parent), m_port(port) {
    PortComboImpl::LoadSerialPorts();
    Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& e) {
      std::string value = GetValue().ToStdString();
      size_t pos = value.find(' ');  // Strip away informational info
      m_port = pos == std::string::npos ? value : value.substr(0, pos);
      e.Skip();
    });
    m_device_list_listener.Init(
        SystemEvents::GetInstance().evt_dev_change,
        [&](ObservedEvt&) { conn_edit::LoadSerialPorts(this); });
  }

private:
  wxString& m_port;
  obs::Listener m_device_list_listener;

  virtual void LoadSerialPorts() {
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

    auto value = GetValue();
    Clear();
    for (auto& p : sorted_ports) Append(p);
    if (!value.empty())
      SetValue(value);
    else if (FindString(m_port) != wxNOT_FOUND)
      Select(FindString(m_port));
    else if (!sorted_ports.empty())
      SetSelection(0);
  }
};  // PortComboImpl

PortCombo* PortCombo::Create(wxWindow* parent, wxString& port) {
  return new PortComboImpl(parent, port);
}

class NetConfigPanelImpl : public NetConfigPanel {
  friend NetConfigPanel* NetConfigPanel::Create(wxWindow*, ConnectionParams&);

  class ExpertModeCheckBox : public wxCheckBox {
  public:
    explicit ExpertModeCheckBox(wxWindow* parent)
        : wxCheckBox(parent, wxID_ANY, "") {}
  };

  /** Type of connection: TCP client, TCP server, UDP client, ... */
  class ConnectionTypeChoice : public wxChoice {
  public:
    ConnectionTypeChoice(wxWindow* parent, std::function<void()> on_choice)
        : wxChoice(parent, wxID_ANY), m_on_choice(std::move(on_choice)) {
      for (const auto& view : kBasicNetViews) Append(view);  // FIXME
      Append(kBasicNetViews[0]);
      wxChoice::SetSelection(0);  // until OnConnectionTypeChanged()
      Bind(wxEVT_CHOICE, [&](wxCommandEvent e) {
        m_on_choice();
        e.Skip();
      });
    }

  private:
    std::function<void()> m_on_choice;
  };

  /** Network address text entry. */
  class AddressTextCtrl : public TextCtrlWithHelp, public Reloadable {
  public:
    AddressTextCtrl(wxWindow* parent, ConnectionParams& params)
        : TextCtrlWithHelp(parent,
                           "IP address or hostname, often 255.255.255.255"),
          m_params(params) {
      AddressTextCtrl::Reload();
      Bind(wxEVT_TEXT, [&](wxCommandEvent& e) {
        m_params.network_address = GetValue();
        e.Skip();
      });
      wxWindowBase::SetMinSize(wxSize(60 * wxWindow::GetCharWidth(), -1));
    }

    void Reload() override {
      TextCtrlWithHelp::ChangeValue(m_params.network_address);
      if (GetValue().empty()) RestoreHelp();
    }

    void ChangeValue(const wxString& text) override {
      TextCtrlWithHelp::ChangeValue(text);
      m_params.network_address = text;
    }

    void SetClientMode() {
      if (GetValue() == "0.0.0.0") ChangeValue("");
      if (IsPristine() || GetValue().empty()) SetHelp(kAddressDefaultHelp);
      Enable();
    }

    void SetServerMode() {
      ChangeValue("0.0.0.0");
      m_params.network_address = "0.0.0.0";
      Disable();
    }

    void SetClientMode(const std::string& help) {
      SetClientMode();
      SetHelp(help);
      if (IsPristine()) RestoreHelp();
    }

  private:
    ConnectionParams& m_params;
  };

  /** User comments text entry. */
  class CommentTextCtrl : public wxTextCtrl, public Reloadable {
  public:
    CommentTextCtrl(wxWindow* parent, ConnectionParams& params)
        : wxTextCtrl(parent, wxID_ANY), m_params(params) {
      CommentTextCtrl::Reload();
      Bind(wxEVT_TEXT, [&](wxCommandEvent& e) {
        m_params.user_comment = GetValue();
        e.Skip();
      });
    }

    void Reload() override { ChangeValue(m_params.user_comment); }

  private:
    ConnectionParams& m_params;
  };

  /** Overall grid layout for network address, port, etc. */
  class NetGrid : public wxFlexGridSizer {
  public:
    NetGrid(wxWindow* parent, ConnectionParams& params,
            InputOutputPanel* in_out_panel,
            std::function<void(DataProtocol)> on_protocol_change)
        : wxFlexGridSizer(2),
          m_parent(parent),
          m_params(params),
          m_in_out_panel(in_out_panel),
          m_on_protocol_change(std::move(on_protocol_change)) {

      // Longest help text in address and port:
      static const wxSize kHelpSize = parent->GetTextExtent(kAddressMcastHelp);
      AddGrowableCol(1);
      auto flags = wxSizerFlags().Border();
      auto label_flags = wxSizerFlags().Border();
#if wxCHECK_VERSION(3, 2, 0)
      label_flags = label_flags.CentreVertical();
#endif
      m_expert_mode_checkbox = new ExpertModeCheckBox(parent);
      m_expert_mode_checkbox->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e) {
        OnExpertModeChange();
        e.Skip();
      });
      m_expert_mode_label =
          new wxStaticText(parent, wxID_ANY, _("Expert mode"));
      Add(m_expert_mode_label, label_flags);
      Add(m_expert_mode_checkbox, flags);

      m_connection_type_label =
          new wxStaticText(parent, wxID_ANY, _("Connection type"));
      m_connection_type_choice = new ConnectionTypeChoice(parent, [&] {
        OnConnectionTypeChange();
        m_on_protocol_change(m_params.data_protocol);
      });
      Add(m_connection_type_label, label_flags);
      Add(m_connection_type_choice, flags);

      m_protocol_label = new wxStaticText(parent, wxID_ANY, _("Data Protocol"));
      m_protocol_choice = ProtocolChoice::Create(parent, m_params, [&] {
        m_on_protocol_change(m_params.data_protocol);
      });
      Add(m_protocol_label, label_flags);
      Add(m_protocol_choice, flags);

      m_address_label = new wxStaticText(parent, wxID_ANY, _("Server address"));
      m_address_textctrl = new AddressTextCtrl(parent, m_params);
      m_address_textctrl->SetMinSize(wxSize(kHelpSize.x, -1));
      m_address_textctrl->Bind(wxEVT_KILL_FOCUS,
                               [&](wxFocusEvent& ev) { OnAddressChange(ev); });
      Add(m_address_label, label_flags);
      Add(m_address_textctrl, wxSizerFlags().Border());

      m_port_label = new wxStaticText(parent, wxID_ANY, _("Data Port"));
      m_port_textctrl = conn_edit::PortTextCtrl::Create(parent, m_params);
      m_port_textctrl->SetMinSize(wxSize(kHelpSize.x, -1));
      Add(m_port_label, label_flags);
      Add(m_port_textctrl, wxSizerFlags().Border());

      m_comment_label = new wxStaticText(parent, wxID_ANY, _("User comment"));
      m_comment_textctrl = new CommentTextCtrl(parent, m_params);
      Add(m_comment_label, label_flags);
      Add(m_comment_textctrl, wxSizerFlags(1).Expand().Border());
      OnExpertModeChange();
      std::string view = NetViewByConnection(&params);
      ConfigureControlsForView(view);
      wxSizer::Layout();
    }

    [[nodiscard]] bool CheckStatus() const {
      bool ok = CheckAddress(m_parent, *m_address_textctrl);
      if (ok) ok = CheckPort(m_parent, *m_port_textctrl);
      return ok;
    }

    /** Update GUI elements from m_params values. */
    void Reload() {
      if (m_params.type != ConnectionType::NETWORK) return;

      std::string view;
      bool is_expert;
      if (IsDefaultParams(&m_params)) {
        view = kTcpDevice;
        is_expert = false;
      } else {
        view = NetViewByConnection(&m_params);
        auto found =
            std::find(kBasicNetViews.begin(), kBasicNetViews.end(), view);
        is_expert = found == kBasicNetViews.end();
      }
      m_expert_mode_checkbox->SetValue(is_expert);
      OnExpertModeChange();

      auto item = m_connection_type_choice->FindString(view);
      if (item != wxNOT_FOUND) m_connection_type_choice->SetSelection(item);
      OnConnectionTypeChange();
    }

  private:
    wxWindow* m_parent;
    ConnectionParams& m_params;
    InputOutputPanel* m_in_out_panel;
    std::function<void(DataProtocol)> m_on_protocol_change;
    wxStaticText* m_expert_mode_label;
    ExpertModeCheckBox* m_expert_mode_checkbox;
    wxStaticText* m_connection_type_label;
    wxChoice* m_connection_type_choice;
    wxStaticText* m_protocol_label;
    ProtocolChoice* m_protocol_choice;
    wxStaticText* m_address_label;
    AddressTextCtrl* m_address_textctrl;
    wxStaticText* m_port_label;
    PortTextCtrl* m_port_textctrl;
    wxStaticText* m_comment_label;
    wxTextCtrl* m_comment_textctrl;

    void OnExpertModeChange() {
      bool is_expert = m_expert_mode_checkbox->GetValue();
      int view = m_connection_type_choice->GetSelection();
      m_connection_type_choice->Clear();
      if (is_expert) {
        for (const auto& choice : kAdvancedNetViews)
          m_connection_type_choice->Append(choice);
        m_connection_type_label->SetLabel(_("Connection type"));
        if (m_comment_label) m_comment_label->Show();
        if (m_comment_textctrl) m_comment_textctrl->Show();
      } else {
        if (view != wxNOT_FOUND) {
          if (static_cast<size_t>(view) >= kBasicNetViews.size()) view = 0;
        }
        for (const auto& choice : kBasicNetViews)
          m_connection_type_choice->Append(choice);
        m_connection_type_label->SetLabel(_("Connect to"));
        if (m_comment_label) m_comment_label->Hide();
        if (m_comment_textctrl) m_comment_textctrl->Hide();
      }
      m_connection_type_choice->SetSelection(view);
      auto view_str =
          m_connection_type_choice->GetStringSelection().ToStdString();
      if (!view_str.empty()) ConfigureControlsForView(view_str);
      m_parent->GetParent()->Layout();
      m_parent->FitInside();
    }

    void OnAddressChange(wxFocusEvent& ev) const {
      int selection = m_connection_type_choice->GetSelection();
      if (selection != wxNOT_FOUND) {
        std::string type =
            m_connection_type_choice->GetString(selection).ToStdString();
        if (type == kMulticastClient || type == kMulticastServer) {
          auto address = m_address_textctrl->GetValue().ToStdString();
          if (!IsAddressMultiCast(address)) {
            auto dlg =
                wxMessageDialog(m_parent, _("Illegal multicast address"),
                                _("OpenCPN warning"), wxOK | wxICON_WARNING);
            dlg.ShowModal();
          }
        }
      }
      ev.Skip();
    }

    void OnConnectionTypeChange() {
      if (m_params.type != ConnectionType::NETWORK) return;
      auto p = m_params.data_protocol;
      if (p == DataProtocol::PROTO_SIGNALK || p == DataProtocol::PROTO_GPSD)
        m_params.data_protocol = PROTO_NMEA0183;
      int selection = m_connection_type_choice->GetSelection();
      if (selection == wxNOT_FOUND) return;
      std::string view =
          m_connection_type_choice->GetStringSelection().ToStdString();
      if (view == kTcpDevice || view == kTcpClient || view == kTcpServer) {
        m_params.net_protocol = NetworkProtocol::TCP;
      } else if (view == kSignalkClient || view == kSignalkDevice) {
        m_params.net_protocol = NetworkProtocol::SIGNALK;
        m_params.data_protocol = DataProtocol::PROTO_SIGNALK;
      } else if (view == kGpsdClient || view == kGpsdDevice) {
        m_params.net_protocol = NetworkProtocol::GPSD;
        m_params.data_protocol = DataProtocol::PROTO_GPSD;
      } else if (view == kUdpReceive || view == kUdpInput || view == kUdpSend ||
                 view == kUdpOutput || view == kMulticastClient ||
                 view == kMulticastServer || view == kUdpInputOutput) {
        m_params.net_protocol = NetworkProtocol::UDP;
      } else {
        assert(false && "Cannot handle view");
      }
      if (view == kTcpServer || view == kUdpReceive || view == kUdpInput ||
          view == kMulticastServer)
        m_params.network_address = "0.0.0.0";
      ConfigureControlsForView(view);
      OnExpertModeChange();
      m_on_protocol_change(m_params.data_protocol);
    }

    /**
     * Return net view name for given ConnectionParams. For ambiguous cases
     * like kTcpDevice (basic view)/kTcpClient (advanced view) return
     * the basic view item.
     */
    static std::string NetViewByConnection(const ConnectionParams* cp) {
      auto address = cp->network_address.ToStdString();
      bool is_server = address.empty() || address == "0.0.0.0";
      if (IsAddressMultiCast(cp->network_address))
        return is_server ? kMulticastServer : kMulticastClient;
      switch (cp->net_protocol) {
        case NetworkProtocol::GPSD:
          return kGpsdDevice;
        case NetworkProtocol::SIGNALK:
          return kSignalkDevice;
        case NetworkProtocol::UDP:
          if (cp->direction == PortDirection::kInOut) return kUdpInputOutput;
          if (!is_server) return kUdpSend;
          return kUdpReceive;
        case NetworkProtocol::TCP:
          return is_server ? kTcpServer : kTcpDevice;
        default:
          wxLogWarning("Cannot deduce connection params view type");
          return "";
      }
      return "";  // for the compiler
    }

    void ConfigureControlsForView(const std::string& view) {
      if (m_params.type != ConnectionType::NETWORK) {
        m_connection_type_choice->SetSelection(0);
        return;
      }
      int selection = m_connection_type_choice->GetSelection();
      assert(selection != wxNOT_FOUND);
      auto found =
          std::find(kBasicNetViews.begin(), kBasicNetViews.end(), view);
      m_expert_mode_checkbox->SetValue(found == kBasicNetViews.end());
      m_address_textctrl->Reload();
      m_address_textctrl->SetClientMode();
      m_address_textctrl->Show();
      m_address_label->SetLabel(_("Server address"));
      m_address_label->Show();
      m_in_out_panel->EnableOutput(false);
      m_in_out_panel->EnableInput(false);
      m_port_textctrl->Reload();
      auto port = m_port_textctrl->GetValue();
      if (port == kDefaultGpsdPort || port == kDefaultSignalkPort ||
          port.empty()) {
        m_port_textctrl->RestoreHelp();
      }
      m_protocol_choice->Reload();
      m_protocol_choice->Enable();
      if (view == kTcpDevice || view == kTcpClient) {
        m_address_label->Show();
        m_address_textctrl->SetClientMode();
        if (m_port_textctrl->IsPristine()) m_port_textctrl->SetHelp(kPortHelp);
        m_in_out_panel->EnableInput(true);
        m_in_out_panel->EnableOutput(true);
      } else if (view == kUdpReceive || view == kUdpInput ||
                 view == kUdpInputOutput) {
        m_address_label->Show(view == kUdpInputOutput);
        if (view != kUdpInputOutput) m_address_textctrl->SetServerMode();
        m_address_textctrl->Show(view == kUdpInputOutput);
        if (m_port_textctrl->IsPristine()) m_port_textctrl->SetHelp(kPortHelp);
        m_in_out_panel->SetOutput(view == kUdpInputOutput);
        m_in_out_panel->SetInput(true);
      } else if (view == kUdpSend || view == kUdpOutput) {
        m_address_textctrl->SetClientMode(kAddressUdpHelp);
        m_in_out_panel->SetInput(false);
        m_in_out_panel->SetOutput(true);
      } else if (view == kGpsdClient || view == kGpsdDevice) {
        m_protocol_choice->Clear();
        m_protocol_choice->Append("gpsd");
        m_protocol_choice->SetSelection(0);
        m_protocol_choice->Disable();
        m_address_textctrl->SetClientMode();
        if (m_port_textctrl->IsPristine())
          m_port_textctrl->ChangeValue(kDefaultGpsdPort);
        m_in_out_panel->SetInput(true);
        m_in_out_panel->SetOutput(false);
      } else if (view == kSignalkClient || view == kSignalkDevice) {
        m_protocol_choice->Clear();
        m_protocol_choice->Append("SignalK");
        m_protocol_choice->SetSelection(0);
        m_protocol_choice->Disable();
        m_address_textctrl->SetClientMode(kAddressUdpHelp);
        if (m_port_textctrl->IsPristine())
          m_port_textctrl->ChangeValue(kDefaultSignalkPort);
        m_in_out_panel->SetInput(true);
        m_in_out_panel->SetOutput(false);
      } else if (view == kTcpServer) {
        m_address_label->SetLabel(_("Interface"));
        m_address_textctrl->SetServerMode();
        m_in_out_panel->EnableInput(true);
        m_in_out_panel->EnableOutput(true);
      } else if (view == kMulticastClient || view == kMulticastServer) {
        auto address = m_address_textctrl->GetValue();
        if (address.empty() || address == "0.0.0.0")
          m_address_textctrl->SetHelp(kAddressMcastHelp);
        m_address_label->SetLabel(_("Multicast group"));
        if (m_port_textctrl->IsPristine())
          m_port_textctrl->SetHelp(kMcastPortHelp);
        if (m_address_textctrl->IsPristine())
          m_address_textctrl->SetHelp(kAddressMcastHelp);
        if (view == kMulticastClient) {
          m_in_out_panel->SetInput(false);
          m_in_out_panel->SetOutput(true);
        } else {
          m_in_out_panel->SetInput(true);
          m_in_out_panel->EnableOutput(true);
        }
      } else {
        wxLogError("Illegal view: %s", view.c_str());
        assert(false && "Illegal view");
      }
    }

    static bool IsDefaultParams(const ConnectionParams* params) {
      static const ConnectionParams default_params;
      return ConnectionType::NETWORK == params->type &&
             default_params.net_protocol == params->net_protocol &&
             default_params.network_port == params->network_port &&
             default_params.network_address == params->network_address;
    }
  };  // NetGrid

  class NetGridPanel : public wxPanel, public Reloadable {
  public:
    /**
     * @note:on_protocol_change is invoked if connection type (a k a view)
     * or data protocol (NMEA0183, NMEA2000, SignalK) is updated
     */
    NetGridPanel(wxWindow* parent, ConnectionParams& params,
                 InputOutputPanel* in_out_panel,
                 std::function<void(DataProtocol)> on_protocol_change)
        : wxPanel(parent),
          m_net_grid(new NetGrid(this, params, in_out_panel,
                                 std::move(on_protocol_change))) {
      SetSizer(m_net_grid);
      wxWindowBase::Layout();
    }

    ~NetGridPanel() override = default;

    bool CheckStatus() { return m_net_grid->CheckStatus(); }

    void Reload() override { m_net_grid->Reload(); }

  private:
    NetGrid* m_net_grid;
  };

  class AuthTokenPanel : public wxPanel, public Reloadable {
  public:
    AuthTokenPanel(wxWindow* parent, ConnectionParams& params)
        : wxPanel(parent), m_params(params) {
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      auto flags = wxSizerFlags().Border();
#if wxCHECK_VERSION(3, 2, 0)
      flags = flags.CentreVertical();
#endif
      hbox->Add(new wxStaticText(this, wxID_ANY, _("Authentication token ")),
                flags);
      m_token = new wxTextCtrl(this, wxID_ANY);
      hbox->Add(m_token, wxSizerFlags().Border());
      SetSizer(hbox);
      wxWindow::Layout();
      Bind(wxEVT_TEXT, [&](wxCommandEvent& e) {
        m_params.auth_token = m_token->GetValue();
        e.Skip();
      });
    }

    void Reload() override { m_token->SetValue(m_params.auth_token); }

  private:
    ConnectionParams& m_params;
    wxTextCtrl* m_token;
  };

  class AdvancedPanel : public wxPanel {
  public:
    AdvancedPanel(wxWindow* parent, ConnectionParams& params)
        : wxPanel(parent),
          m_filter_panel(nullptr),
          m_auth_token_panel(nullptr) {
      m_filter_panel = FilterConfigPanel::Create(this, params);
      m_auth_token_panel = new AuthTokenPanel(this, params);
      auto vbox = new wxBoxSizer(wxVERTICAL);
      vbox->Add(m_filter_panel);
      vbox->Add(m_auth_token_panel);
      SetSizer(vbox);
      SetProtocol(params.data_protocol);
    }

    void SetProtocol(DataProtocol protocol) {
      switch (protocol) {
        case DataProtocol::PROTO_SIGNALK:
          m_filter_panel->Hide();
          m_auth_token_panel->Show();
          break;
        case DataProtocol::PROTO_NMEA0183:
          m_filter_panel->Show();
          m_auth_token_panel->Hide();
          break;
        default:
          m_filter_panel->Hide();
          m_auth_token_panel->Hide();
      }
    }

  private:
    FilterConfigPanel* m_filter_panel;
    AuthTokenPanel* m_auth_token_panel;
  };

public:
  /**
   * Check that user data is sane, possibly run dialogs for missing or
   * bad input.
   * @return true if user data is ok to be submitted.
   */
  [[nodiscard]] bool CheckStatus() const override {
    if (m_params.type != ConnectionType::NETWORK) return true;
    return m_grid_panel->CheckStatus();
  }

protected:
  NetConfigPanelImpl(wxWindow* parent, ConnectionParams& params)
      : NetConfigPanel(parent),
        m_params(params),
        m_grid_panel(nullptr),
        m_advanced_panel(nullptr) {
    auto flags = wxSizerFlags().Border();
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto input_output_panel = InputOutputPanel::Create(this, m_params);

    m_advanced_panel = new AdvancedPanel(this, m_params);
    m_expand_advanced_panel = new ExpandablePanel(this, m_advanced_panel, [&] {
      if (auto parent = GetParent()) parent->Layout();
      if (auto* scrollwin = FindParentOfType<wxScrolledWindow>(this))
        scrollwin->FitInside();
    });
    auto on_proto_change = [&](DataProtocol p) { OnProtocolChange(p); };
    m_grid_panel =
        new NetGridPanel(this, m_params, input_output_panel, on_proto_change);
    vbox->Add(m_grid_panel, flags.Expand());
    vbox->Add(input_output_panel, flags);

    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(this, wxID_ANY, _("Advanced")));
    hbox->Add(m_expand_advanced_panel);
    vbox->Add(hbox, flags.Expand());
    vbox->Add(m_advanced_panel, flags.Expand());

    SetSizer(vbox);
    wxWindow::Layout();
    wxWindow::Show();
  }

private:
  ConnectionParams& m_params;
  NetGridPanel* m_grid_panel;
  AdvancedPanel* m_advanced_panel;
  ExpandablePanel* m_expand_advanced_panel;

  void OnProtocolChange(DataProtocol p) {
    m_expand_advanced_panel->Collapse();
    m_advanced_panel->SetProtocol(p);
    m_expand_advanced_panel->EnableChild(p == DataProtocol::PROTO_NMEA0183 ||
                                         p == DataProtocol::PROTO_SIGNALK);
  }
};

NetConfigPanel* NetConfigPanel::Create(wxWindow* parent,
                                       ConnectionParams& params) {
  return new NetConfigPanelImpl(parent, params);
}

}  // namespace conn_edit
