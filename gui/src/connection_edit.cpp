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
 * Implement connection_edit.h -- Dialog and support for editing a connection
 */

#include <memory>
#include <set>
#include <string>
#include <vector>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/event.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/platinfo.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>
#include <wx/timer.h>
#include <wx/weakref.h>
#include <wx/window.h>
#include <wx/wrapsizer.h>

// wxAtoi

#if defined(__linux__) && !defined(__ANDROID__)
#include <linux/can.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "dnet.h"
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

#include "connection_edit.h"

#include "libgui/text_ctrl_w_help.h"
#include "libgui/expand_panel.h"

#include "model/conn_params.h"
#include "model/ocpn_utils.h"
#include "model/ser_ports.h"

#include "conn_params_panel.h"
#include "filter_sentence_dlg.h"
#include "ocpn_platform.h"
#include "options.h"

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
static const std::string kPortHelp =
    _("Port number (1025 - 65535, often 10110)");

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
// "LIMITED BROADCAST" address
//  Deprecated, but still useful on simple mobile networks.

static const std::vector<std::string> kBasicNetViews = {
    kTcpDevice, kUdpReceive, kUdpSend, kGpsdDevice, kSignalkDevice};

static const std::vector<std::string> kAdvancedNetViews = {
    // First items matches kBasicNetViews
    kTcpClient,      kUdpInput,        kUdpOutput,
    kGpsdClient,     kSignalkClient,   kTcpServer,
    kUdpInputOutput, kMulticastClient, kMulticastServer};

#ifdef __ANDROID__
static bool IsAndroid() { return true; }
#else
static bool IsAndroid() { return false; }
#endif

static bool IsWindows() {
  return wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_WINDOWS;
}

static bool IsApple() {
  return wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_MAC_OS;
}

static bool IsLinux() {
  return wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_UNIX_LINUX;
}

static wxString StringArrayToString(const wxArrayString& arr) {
  wxString ret = wxEmptyString;
  for (size_t i = 0; i < arr.Count(); i++) {
    if (i > 0) ret.Append(",");
    ret.Append(arr[i]);
  }
  return ret;
}

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

static bool CheckPort(wxWindow* parent, const TextCtrlWithHelp& ctrl) {
  if (ctrl.IsPristine() || ctrl.GetValue().empty()) {
    auto dlg = wxMessageDialog(parent, _("Required field port is missing"),
                               _("OpenCPN error"), wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  }
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

static bool CheckAddress(wxWindow* parent, const TextCtrlWithHelp& ctrl) {
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

/*--------------------------------------*/
namespace {

// CanConfigPanel global scope support
#if defined(__linux__) && !defined(__ANDROID__)
intf_t* intf;
std::vector<std::string> can_if_candidates;
#endif

/**
 * Interface implemented by widgets reflecting a ConnectionParams
 * parameter. The widget is kept in sync with the parameter(s) at any
 * time using a contract:
 *
 *   - When created, widget state is updated from parameters.
 *   - WHen user modifies widget state parameter(s) is updated.
 *   - When external code changes the parameter(s) it invokes Reload()
 *     which synchronizes widget state with parameters.
 *
 * @Note: This interface is used by Reload()  which traverses the window
 * tree. Thus a Reloadable normally should be a wxWindow subclass and not
 * for example a sizer.
 */
class Reloadable {
public:
  virtual ~Reloadable() = default;

  /** Reload widget value(s) from parameter(s) */
  virtual void Reload() = 0;
};

/** NMEA1083/NMEA2000 user choice widget. */
class ProtocolChoice : public wxChoice, public Reloadable {
public:
  ProtocolChoice(
      wxWindow* parent, ConnectionParams& params,
      std::function<void()> on_change = [] {})
      : wxChoice(parent, wxID_ANY),
        m_params(params),
        m_on_change(std::move(on_change)) {
    ProtocolChoice::Reload();
    Bind(wxEVT_CHOICE, [&](wxCommandEvent& e) { OnChoice(e); });
  }

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
      default:
        assert(false && "Illegal SignalK protocol");
        break;
    }
    Enable();
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
};  // ProtocolChoice

/** Garnin mode checkbox. */
class GarminCheckBox : public wxCheckBox, public Reloadable {
public:
  GarminCheckBox(wxWindow* parent, ConnectionParams& params)
      : wxCheckBox(parent, wxID_ANY, _("Use Garmin (GRMN) mode for input")),
        m_params(params) {
    GarminCheckBox::Reload();
    Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& ev) {
      m_params.is_garmin = GetValue();
      ev.Skip();
    });
  }

  void Reload() override { SetValue(m_params.is_garmin); }

private:
  ConnectionParams& m_params;
};

/** User comment text entry. */
class CommentText : public wxTextCtrl, public Reloadable {
public:
  CommentText(wxWindow* parent, wxString& comment)
      : wxTextCtrl(parent, wxID_ANY), m_comment(comment) {
    CommentText::Reload();
    Bind(wxEVT_TEXT, [&](wxCommandEvent& e) { m_comment = GetValue(); });
  }

  void Reload() override { SetValue(m_comment); }

private:
  wxString& m_comment;
};

/** Entry handling "Data port" setting. */
class PortTextCtrl : public TextCtrlWithHelp, public Reloadable {
public:
  PortTextCtrl(wxWindow* parent, ConnectionParams& params)
      : TextCtrlWithHelp(parent, kPortHelp), m_params(params) {
    PortTextCtrl::Reload();
    Bind(wxEVT_TEXT, [&](wxCommandEvent& e) { OnChange(e); });
  };

  void Reload() override {
    auto string = std::to_string(m_params.network_port);
    ChangeValue(string == "0" ? "" : string);
    if (GetValue().empty()) RestoreHelp();
  }

  void ChangeValue(const wxString& value) override {
    TextCtrlWithHelp::ChangeValue(value);
    OnChange();
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
};  // PortTextCtrl

/**
 * The radio buttons selecting serial, network, socketcan, etc.
 * connection type.
 */
class TypeButtons : public wxPanel, public Reloadable {
public:
  TypeButtons(wxWindow* parent, std::function<void(ConnectionType)> on_click,
              ConnectionParams& params)
      : wxPanel(parent, wxID_ANY),
        m_serial_btn(new wxRadioButton(this, wxID_ANY, _("Serial"))),
        m_net_btn(new wxRadioButton(this, wxID_ANY, _("Network"))),
        m_can_btn(nullptr),
        m_canable_btn(nullptr),
        m_internal_gps_btn(nullptr),
        m_internal_bt_btn(nullptr),
        m_on_click(std::move(on_click)),
        m_params(params) {

#ifdef _MSC_VER
    // Wrapsizer seems broken on win64/wxw 3.3.4
    auto wbox = new wxBoxSizer(wxHORIZONTAL);
#else
    auto wbox = new wxWrapSizer(wxHORIZONTAL);
#endif
    auto on_clicked = [&](wxCommandEvent& e) { OnClick(e); };
    m_serial_btn->Bind(wxEVT_RADIOBUTTON, on_clicked);
    wbox->Add(m_serial_btn, wxSizerFlags().Border());

    m_net_btn->Bind(wxEVT_RADIOBUTTON, on_clicked);
    wbox->Add(m_net_btn, wxSizerFlags().Border());

    if (IsLinux()) {
      m_can_btn = new wxRadioButton(this, wxID_ANY, _("socketCAN"));
      m_can_btn->Bind(wxEVT_RADIOBUTTON, on_clicked);
      wbox->Add(m_can_btn, wxSizerFlags().Border());
    }
    if (IsWindows()) {
      m_canable_btn = new wxRadioButton(this, wxID_ANY, _("Canable (SLCAN)"));
      m_canable_btn->Bind(wxEVT_RADIOBUTTON, on_clicked);
      wbox->Add(m_canable_btn, wxSizerFlags().Border());
    }
    if (OCPNPlatform::hasInternalGPS()) {
      m_internal_gps_btn = new wxRadioButton(this, wxID_ANY, _("Internal GPS"));
      m_internal_gps_btn->Bind(wxEVT_RADIOBUTTON, on_clicked);
      wbox->Add(m_internal_gps_btn, wxSizerFlags().Border());
    }
    if (OCPNPlatform::hasInternalBT()) {
      m_internal_bt_btn =
          new wxRadioButton(this, wxID_ANY, _("Internal Bluetooth"));
      m_internal_bt_btn->Bind(wxEVT_RADIOBUTTON, on_clicked);
      wbox->Add(m_internal_bt_btn, wxSizerFlags().Border());
    }
    SetSelectedType(ConnectionType::SERIAL);

    SetSizer(wbox);
    wbox->Layout();
  }

  void Reload() override { SetSelectedType(m_params.type); }

  void SetSelectedType(ConnectionType type) {
    switch (type) {
      case ConnectionType::NETWORK:
        m_net_btn->SetValue(true);
        break;
      case ConnectionType::SERIAL:
        m_serial_btn->SetValue(true);
        break;
      case ConnectionType::SOCKETCAN:
        m_can_btn->SetValue(true);
        break;
      case ConnectionType::CANABLE:
        if (m_canable_btn) m_canable_btn->SetValue(true);
        break;
      case ConnectionType::INTERNAL_GPS:
        if (m_internal_gps_btn) m_internal_gps_btn->SetValue(true);
        break;
      case ConnectionType::INTERNAL_BT:
        if (m_internal_bt_btn) m_internal_bt_btn->SetValue(true);
        break;
      default:
        assert(false && "Illegal connection type");
    }
  }

private:
  wxRadioButton* m_serial_btn;
  wxRadioButton* m_net_btn;
  wxRadioButton* m_can_btn;
  wxRadioButton* m_canable_btn;
  wxRadioButton* m_internal_gps_btn;
  wxRadioButton* m_internal_bt_btn;
  std::function<void(ConnectionType)> m_on_click;
  ConnectionParams& m_params;

  [[nodiscard]] ConnectionType GetSelectedType() const {
    if (m_serial_btn->GetValue()) return ConnectionType::SERIAL;
    if (m_net_btn->GetValue()) return ConnectionType::NETWORK;
    if (m_can_btn && m_can_btn->GetValue()) return ConnectionType::SOCKETCAN;
    if (m_canable_btn && m_canable_btn->GetValue())
      return ConnectionType::CANABLE;
    if (m_internal_gps_btn && m_internal_gps_btn->GetValue())
      return ConnectionType::INTERNAL_GPS;
    if (m_internal_bt_btn && m_internal_bt_btn->GetValue())
      return ConnectionType::INTERNAL_BT;
    assert(false && "No connection type radiobutton active(!)");
    return ConnectionType::UNKNOWN;
  }

  void OnClick(wxCommandEvent& e) {
    auto type = GetSelectedType();
    m_params.type = type;
    m_on_click(type);
    e.Skip();
  }
};

/** Configure input filters: accept or ignore list of sentences. */
class FilterConfigPanel : public wxPanel, public Reloadable {

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
      // auto sentence_box = new wxBoxSizer(wxHORIZONTAL);
      // sentence_box->Add(m_sentences_text,
      //                   wxSizerFlags().Border().CentreVertical());
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

    void OnEdit(wxCommandEvent& e) {
      static const std::string kNothingHere = _("No sentences selected");
      if (m_sentences_text->GetLabel() == kNothingHere)
        m_sentences_text->SetLabel("");
      const wxArrayString list =
          wxStringTokenize(m_sentences_text->GetLabel(), ",");
      SentenceListDlg dlg(m_parent, FILTER_INPUT, m_type, list);
      if (dlg.ShowModal() != wxID_OK) return;

      std::string text = dlg.GetSentences().ToStdString();
      m_list = wxStringTokenize(text, ",");
      if (text.empty()) text = kNothingHere;
      m_sentences_text->SetLabel(text);
      m_parent->Layout();
    }
  };  // Statbox

public:
  FilterConfigPanel(wxWindow* parent, ConnectionParams& params)
      : wxPanel(parent, wxID_ANY), m_params(params) {
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

  void Reload() override {
    m_output_filter->Reload();
    m_input_filter->Reload();
  }

private:
  Statbox* m_input_filter;
  Statbox* m_output_filter;
  ConnectionParams& m_params;
};  // FilterConfigPanel

/** Two checkboxes for enabling port input and/or output. */
class InputOutputPanel : public wxPanel {
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
  InputOutputPanel(wxWindow* parent, ConnectionParams& params)
      : wxPanel(parent), m_params(params) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Border();
    m_input_box = new InputCheckBox(this, m_params, [&] { OnChange(); });
    m_output_box = new OutputCheckBox(this, m_params, [&] { OnChange(); });
    vbox->Add(m_input_box, flags);
    vbox->Add(m_output_box, flags);
    SetSizer(vbox);
    wxWindow::Layout();
  }

  void EnableOutput(bool enabled = true) {
    m_output_box->Enable(enabled);
    OnChange();
  }

  void EnableInput(bool enabled = true) {
    m_input_box->Enable(enabled);
    OnChange();
  }

  void SetOutput(bool value) {
    m_output_box->SetValue(value);
    OnChange();
  }

  void SetInput(bool value) {
    m_input_box->SetValue(value);
    OnChange();
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
};  //  InputOutputPanel

/** Configure internal bluetooth serial connection. */
class InternalBtConfigPanel : public wxPanel, public Reloadable {
  class ScanTimer : public wxTimer {
  public:
    ScanTimer() : m_owner(nullptr) {}

    void SetOwner(InternalBtConfigPanel* owner) { m_owner = owner; }

    void Notify() override {
      if (m_owner.get()) m_owner->OnTimer();
    }

  private:
    wxWeakRef<InternalBtConfigPanel> m_owner;
  };

  class AdvancedPanel : public wxPanel {
  public:
    AdvancedPanel(wxWindow* parent, ConnectionParams& params)
        : wxPanel(parent, wxID_ANY), m_params(params) {
      auto vbox = new wxBoxSizer(wxVERTICAL);
      auto flags = wxSizerFlags();
      vbox->Add(new GarminCheckBox(this, m_params), flags);
      vbox->Add(new FilterConfigPanel(this, m_params), flags);
      SetSizer(vbox);
      wxWindowBase::Layout();
    }

  private:
    ConnectionParams& m_params;
  };

public:
  InternalBtConfigPanel(wxWindow* parent, ConnectionParams& params)
      : wxPanel(parent, wxID_ANY),
        m_params(params),
        m_bt_scanning(0),
        m_bt_no_change_counter(0),
        m_bt_last_result_count(0) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto flags = wxSizerFlags().Border();

    vbox->Add(new wxStaticText(this, wxID_ANY, _("Bluetooth Data Sources")),
              flags);

    auto src_box = new wxBoxSizer(wxHORIZONTAL);
    m_src_choice = new wxChoice(this, wxID_ANY);
    m_src_choice->Append("unscanned");
    m_src_choice->SetSelection(0);
    m_scan_btn = new wxButton(this, wxID_ANY, _("Scan"));
    m_scan_btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent& e) { OnClick(e); });
    src_box->Add(m_src_choice, flags);
    src_box->Add(m_scan_btn, flags);
    vbox->Add(src_box, flags);
    vbox->Add(new InputOutputPanel(this, m_params));

    auto advanced_panel = new AdvancedPanel(this, m_params);
    auto adv_box = new wxBoxSizer(wxHORIZONTAL);
    // if (IsWindows()) flags = flags.ReserveSpaceEvenIfHidden();
    adv_box->Add(new wxStaticText(this, wxID_ANY, _("Advanced")), flags);
    auto on_expand_click = [&] {
      Layout();
      FitInside();
    };
    adv_box->Add(new ExpandablePanel(this, advanced_panel, on_expand_click),
                 flags);
    vbox->Add(adv_box, flags);
    vbox->Add(advanced_panel, flags);

    SetSizer(vbox);
    m_scan_timer.SetOwner(this);
  }

  ~InternalBtConfigPanel() override { m_scan_timer.Stop(); }

  void Reload() override {
    if (m_params.type == ConnectionType::INTERNAL_BT) SetFixedParams();
  }

private:
  ScanTimer m_scan_timer;
  ConnectionParams& m_params;
  wxButton* m_scan_btn;
  wxChoice* m_src_choice;
  int m_bt_scanning;
  int m_bt_no_change_counter;
  wxArrayString m_bt_scan_results;
  int m_bt_last_result_count;

  /** Hardcoded values not directly used by this panel. */
  void SetFixedParams() {
    m_params.direction = PortDirection::kInOut;
    m_params.network_address = "";
    m_params.network_port = 0;
    m_params.net_protocol = PROTO_UNDEFINED;
    m_params.baudrate = 0;
  }

  void OnClick(wxCommandEvent& event) {
    if (m_bt_scanning)
      StopScan();
    else {
      m_bt_no_change_counter = 0;
      m_bt_last_result_count = 0;
      m_scan_timer.Start(1000, wxTIMER_CONTINUOUS);
      g_Platform->startBluetoothScan();
      m_bt_scanning = 1;
      if (m_scan_btn) {
        m_scan_btn->SetLabel(_("Stop Scan"));
      }
    }
  }

  void OnTimer() {
    if (!m_bt_scanning) return;
    m_bt_scanning++;
    m_bt_scan_results = g_Platform->getBluetoothScanResults();

    m_src_choice->Clear();
    m_src_choice->Append(m_bt_scan_results[0]);  // scan status

    unsigned i = 1;
    while ((i + 1) < m_bt_scan_results.GetCount()) {
      wxString item1 = m_bt_scan_results[i] + ";";
      wxString item2 = m_bt_scan_results.Item(i + 1);
      m_src_choice->Append(item1 + item2);
      i += 2;
    }

    if (m_bt_scan_results.GetCount() > 1) {
      m_src_choice->SetSelection(1);
    }

    //  Watch for changes.  When no changes occur after n seconds, stop the
    //  scan
    if (m_bt_no_change_counter > 5) StopScan();
    int count = static_cast<int>(m_bt_scan_results.GetCount());
    if (count == m_bt_last_result_count)
      m_bt_no_change_counter++;
    else
      m_bt_no_change_counter = 0;
    m_bt_last_result_count = static_cast<int>(m_bt_scan_results.GetCount());

    // Absolute fallback
    if (m_bt_scanning >= 15) {
      StopScan();
    }
  }

  void StopScan() {
    m_scan_timer.Stop();
    g_Platform->stopBluetoothScan();
    m_bt_scanning = 0;
    if (m_scan_btn) {
      m_scan_btn->SetLabel(_("BT Scan"));
      m_scan_btn->Enable();
    }
  }
};

/** Configure internal GPS usage, usually on Android */
class InternalGpsConfigPanel : public wxPanel, public Reloadable {
public:
  InternalGpsConfigPanel(wxWindow* parent, ConnectionParams& params)
      : wxPanel(parent, wxID_ANY), m_params(params) {
    auto checkbox = new wxCheckBox(this, wxID_ANY, _("Use internal GPS"));
    checkbox->SetValue(true);
    checkbox->Disable();

    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(checkbox);
    auto top_box = new wxBoxSizer(wxVERTICAL);
    top_box->Add(vbox, wxSizerFlags().Border());
    SetSizer(top_box);
    wxWindowBase::Layout();
    SetParams();
  }

  void Reload() override {
    if (m_params.type == ConnectionType ::INTERNAL_GPS) SetParams();
  }

private:
  ConnectionParams& m_params;

  void SetParams() {
    m_params.network_address = "";
    m_params.network_port = 0;
    m_params.net_protocol = PROTO_UNDEFINED;
    m_params.baudrate = 0;
    m_params.serial_port = "Internal GPS";
  }
};  // InternalGpsConfigPanel

/** Configure windows-only SLcan connection. */
class CanableConfigPanel : public wxPanel, public Reloadable {

  /** Hardcoded values not directly modified by this panel. */
  static void SetFixedParams(ConnectionParams& params) {
    params.data_protocol = PROTO_NMEA2000;
    params.direction = PortDirection::kInOut;
    params.network_address = "";
    params.network_port = 0;
    params.net_protocol = PROTO_UNDEFINED;
    params.baudrate = 0;
  }

  class PortCtrl : public wxTextCtrl, public Reloadable {
  public:
    PortCtrl(wxWindow* parent, ConnectionParams& params)
        : wxTextCtrl(parent, wxID_ANY), m_params(params) {
      wxTextCtrl::SetValue(m_params.serial_port);
      Bind(wxEVT_TEXT, [&](wxCommandEvent& e) {
        m_params.serial_port = GetValue();
        e.Skip();
      });
    }

    void Reload() override {
      if (m_params.type == ConnectionType::CANABLE)
        SetValue(m_params.serial_port);
    }

  private:
    ConnectionParams& m_params;
  };

public:
  CanableConfigPanel(wxWindow* parent, ConnectionParams& params)
      : wxPanel(parent, wxID_ANY), m_params(params) {
    auto* grid_sizer = new wxFlexGridSizer(2, wxWindow::GetCharHeight() / 2, 0);
    auto label_flags = wxSizerFlags().Border();
#if wxCHECK_VERSION(3, 2, 0)
    label_flags = label_flags.CentreVertical();
#endif
    auto ctrl_flags = wxSizerFlags(1).Expand();
    grid_sizer->Add(new wxStaticText(this, wxID_ANY, _("Data source")),
                    label_flags);
    auto port_text_ctrl = new PortCtrl(this, m_params);
    port_text_ctrl->SetMinSize(wxSize(50 * wxWindow::GetCharWidth(), -1));
    grid_sizer->Add(port_text_ctrl, ctrl_flags);
    grid_sizer->Add(new wxStaticText(this, wxID_ANY, "User comment"),
                    label_flags);
    grid_sizer->Add(new CommentText(this, m_params.user_comment), ctrl_flags);

    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(grid_sizer, wxSizerFlags().Border());
    SetSizer(vbox);

    wxWindowBase::Layout();
  }

  void Reload() override {
    if (m_params.type == ConnectionType::CANABLE) SetFixedParams(m_params);
  }

private:
  ConnectionParams& m_params;

};  // CanableConfigPanel

/** Configure serial connection port, baudrate, etc. */
class SerialConfigPanel : public wxPanel, public Reloadable {
  /** Select serial port combobox. */
  class PortCombo : public wxComboBox, public Reloadable {
  public:
    PortCombo(wxWindow* parent, wxString& port)
        : wxComboBox(parent, wxID_ANY), m_port(port) {
      PortCombo::LoadSerialPorts();
      Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& e) {
        m_port = GetValue();
        e.Skip();
      });
    }

    void Reload() override { SetValue(m_port); }

  private:
    wxString& m_port;

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
  };  // PortCombo

  /** Select baudrate  combobox. */
  class BaudChoice : public wxComboBox, public Reloadable {
  public:
    BaudChoice(wxWindow* parent, int& baudrate)
        : wxComboBox(parent, wxID_ANY), m_baudrate(baudrate) {
      BaudChoice::Reload();
      Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& e) { OnChange(e); });
    };

    void Reload() override { LoadBaudrates(); }

  private:
    int& m_baudrate;

    void LoadBaudrates() {
      static const std::vector<std::string> speeds = {
          "150",   "300",   "600",   "1200",   "2400",   "4800",   "9600",
          "19200", "38400", "57600", "115200", "230400", "460800", "921600"};
      Clear();
      for (const auto& speed : speeds) Append(speed);
      auto found = FindString(std::to_string(m_baudrate));
      if (found != wxNOT_FOUND)
        SetSelection(found);
      else
        SetSelection(5);  // 4800
    }

    void OnChange(wxCommandEvent& e) {
      std::string speed = GetStringSelection().ToStdString();
      try {
        m_baudrate = std::stoi(speed);
      } catch (std::logic_error&) {
        assert(false && "Cannot parse preconfigured baudrate");
      }
      e.Skip();
    }
  };  // BaudChoice

  class SerialParamsGrid : public wxFlexGridSizer {
  public:
    SerialParamsGrid(wxWindow* parent, ConnectionParams& params)
        : wxFlexGridSizer(2), m_params(params) {
      AddGrowableCol(1);
      auto flags = wxSizerFlags().Border();
      auto text_flags = wxSizerFlags().Border();
#if wxCHECK_VERSION(3, 2, 0)
      text_flags = text_flags.CentreVertical();
#endif
      Add(new wxStaticText(parent, wxID_ANY, _("Data port")), text_flags);
      Add(new PortCombo(parent, m_params.serial_port), flags);
      Add(new wxStaticText(parent, wxID_ANY, _("Baudrate")), text_flags);
      Add(new BaudChoice(parent, m_params.baudrate), flags);
      Add(new wxStaticText(parent, wxID_ANY, _("Protocol")), text_flags);
      Add(new ProtocolChoice(parent, m_params), flags);
      Add(new wxStaticText(parent, wxID_ANY, _("User Comment")), text_flags);
      Add(new CommentText(parent, m_params.user_comment),
          wxSizerFlags(1).Border().Expand());
    }

  private:
    ConnectionParams& m_params;
  };  // SerialParamsGrid

  class GridPanel : public wxPanel {
  public:
    GridPanel(wxWindow* parent, ConnectionParams& params)
        : wxPanel(parent, wxID_ANY),
          m_params(params),
          m_grid(new SerialParamsGrid(this, m_params)) {
      SetSizer(m_grid);
      wxWindow::Layout();
    }

  private:
    ConnectionParams& m_params;
    SerialParamsGrid* m_grid;
  };

  /** Global Advanced stuff: Use Garmin checkbox, filter configuration */
  class AdvancedPanel : public wxPanel {
  public:
    AdvancedPanel(wxWindow* parent, ConnectionParams& params)
        : wxPanel(parent, wxID_ANY), m_params(params) {
      auto vbox = new wxBoxSizer(wxVERTICAL);
      auto flags = wxSizerFlags().Border();
      vbox->Add(new GarminCheckBox(this, m_params), flags);
      vbox->Add(new FilterConfigPanel(this, m_params), flags);
      SetSizer(vbox);
      wxWindowBase::Layout();
    }

  private:
    ConnectionParams& m_params;
  };

public:
  SerialConfigPanel(wxWindow* parent, ConnectionParams& params)
      : wxPanel(parent), m_params(params) {
    auto flags = wxSizerFlags().Border();
    m_grid = new GridPanel(this, params);

    m_advanced_panel = new AdvancedPanel(this, params);
    auto advanced_ctrl = new ExpandablePanel(this, m_advanced_panel,
                                             [&] { GetParent()->Layout(); });
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    // if (IsWindows()) flags = flags.ReserveSpaceEvenIfHidden();
    hbox->Add(new wxStaticText(this, wxID_ANY, _("Advanced")), flags);
    hbox->Add(advanced_ctrl, flags);

    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(m_grid, wxSizerFlags().Border().Expand());
    vbox->Add(new InputOutputPanel(this, params), flags);
    vbox->Add(hbox, flags);
    vbox->Add(m_advanced_panel, flags);
    SetSizer(vbox);
    wxWindow::Layout();
    wxWindow::Show();
  }

  void Reload() override {
    if (m_params.type != ConnectionType::SERIAL) return;
    if (m_params.net_protocol != NetworkProtocol::PROTO_UNDEFINED) return;

    m_params.net_protocol = NetworkProtocol::TCP;
    m_params.data_protocol = PROTO_NMEA0183;
    m_params.direction = PortDirection::kInput;
    m_params.network_address = "";
    m_params.network_port = 0;
  }

private:
  ConnectionParams& m_params;
  wxWindow* m_advanced_panel;
  GridPanel* m_grid;
};  // SerialConfigPanel

/** Network configuration: TCP/UDP, client/server, etc. */
class NetConfigPanel : public wxPanel {
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

  /** Overall grid layout for various attribute widgets */
  class NetGrid : public wxFlexGridSizer, public Reloadable {
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
      m_connection_type_choice =
          new ConnectionTypeChoice(parent, [&] { OnConnectionTypeChange(); });
      Add(m_connection_type_label, label_flags);
      Add(m_connection_type_choice, flags);

      m_protocol_label = new wxStaticText(parent, wxID_ANY, _("Data Protocol"));
      m_protocol_choice = new ProtocolChoice(parent, m_params, [&] {
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
      m_port_textctrl = new PortTextCtrl(parent, m_params);
      m_port_textctrl->SetMinSize(wxSize(kHelpSize.x, -1));
      Add(m_port_label, label_flags);
      Add(m_port_textctrl, wxSizerFlags().Border());

      m_comment_label = new wxStaticText(parent, wxID_ANY, _("User comment"));
      m_comment_textctrl = new CommentTextCtrl(parent, m_params);
      Add(m_comment_label, label_flags);
      Add(m_comment_textctrl, wxSizerFlags(1).Expand().Border());
      OnExpertModeChange();
      ConfigureControlsForView(kTcpDevice);
      wxSizer::Layout();
    }

    [[nodiscard]] bool CheckStatus() const {
      bool ok = CheckAddress(m_parent, *m_address_textctrl);
      if (ok) ok = CheckPort(m_parent, *m_port_textctrl);
      return ok;
    }

    /** Update GUI elements from m_params values. */
    void Reload() override {
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
      int selection = m_connection_type_choice->GetSelection();
      if (selection == wxNOT_FOUND) return;
      std::string view =
          m_connection_type_choice->GetStringSelection().ToStdString();
      if (view == kTcpDevice || view == kTcpClient || view == kTcpServer ||
          view == kGpsdClient || view == kGpsdDevice ||
          view == kSignalkClient || view == kSignalkDevice)
        m_params.net_protocol = NetworkProtocol::TCP;
      else if (view == kUdpReceive || view == kUdpInput || view == kUdpSend ||
               view == kUdpOutput || view == kMulticastClient ||
               view == kMulticastServer || view == kUdpInputOutput)
        m_params.net_protocol = NetworkProtocol::UDP;
      else
        assert(false && "Cannot handle view");
      if (view == kTcpServer || view == kUdpReceive || view == kUdpInput ||
          view == kMulticastServer)
        m_params.network_address = "0.0.0.0";
      ConfigureControlsForView(view);
      OnExpertModeChange();
    }

    /**
     * Return net view name for given ConnectionParams. For ambiguous cases
     * like kTcpDevice (basic view)/kTcpClient (advanced view) return
     * the basic view.
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
        m_in_out_panel->EnableInput();
        m_in_out_panel->EnableOutput();
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
        m_in_out_panel->EnableInput();
        m_in_out_panel->EnableOutput();
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
          m_in_out_panel->EnableOutput();
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
    NetGridPanel(wxWindow* parent, ConnectionParams& params,
                 InputOutputPanel* in_out_panel,
                 std::function<void(DataProtocol)> on_protocol_change)
        : wxPanel(parent),
          m_net_grid(new NetGrid(this, params, in_out_panel,
                                 std::move(on_protocol_change))) {
      SetSizer(m_net_grid);
      wxWindowBase::Layout();
    }

    bool CheckStatus() { return m_net_grid->CheckStatus(); }

    void Reload() override { m_net_grid->Reload(); }

  private:
    NetGrid* m_net_grid;
  };

public:
  NetConfigPanel(wxWindow* parent, ConnectionParams& params)
      : wxPanel(parent), m_params(params), m_grid_panel(nullptr) {
    auto flags = wxSizerFlags().Border();
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto input_output_panel = new InputOutputPanel(this, m_params);
    m_filter_panel = new FilterConfigPanel(this, m_params);
    m_advanced_panel = new ExpandablePanel(this, m_filter_panel, [&] {
      GetParent()->Layout();
      FitInside();
    });
    auto on_proto_change = [&](DataProtocol p) {
      m_advanced_panel->EnableChild(p == DataProtocol::PROTO_NMEA0183);
    };
    m_grid_panel =
        new NetGridPanel(this, m_params, input_output_panel, on_proto_change);
    vbox->Add(m_grid_panel, flags.Expand());
    vbox->Add(input_output_panel, flags);

    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(this, wxID_ANY, _("Advanced")));
    hbox->Add(m_advanced_panel);
    // if (IsWindows()) flags = flags.ReserveSpaceEvenIfHidden();
    vbox->Add(hbox, flags.Expand());
    vbox->Add(m_filter_panel, flags.Expand());

    SetSizer(vbox);
    wxWindow::Layout();
    wxWindow::Show();
  }

  /**
   * Check that user data is sane, possibly run dialogs for missing or
   * bad input.
   * @return true if user data is ok to be submitted.
   */
  [[nodiscard]] bool CheckStatus() const {
    if (m_params.type != ConnectionType::NETWORK) return true;
    return m_grid_panel->CheckStatus();
  }

private:
  ConnectionParams& m_params;
  NetGridPanel* m_grid_panel;
  FilterConfigPanel* m_filter_panel;
  ExpandablePanel* m_advanced_panel;
};  // NetConfigPanel

/** Configure Linux socketcan connection */
class CanConfigPanel : public wxPanel {
  class CanChoice : public wxChoice, public Reloadable {
  public:
    CanChoice(wxWindow* parent, ConnectionParams& params)
        : wxChoice(parent, wxID_ANY), m_params(params) {
      CanChoice::Reload();
      Bind(wxEVT_CHOICE, [&](wxCommandEvent& e) {
        m_params.socket_can_port = GetStringSelection();
        e.Skip();
      });
    };

    void Reload() override {
      Clear();
      Enable();
      for (const auto& port : GetAvailableSocketCanInterfaces()) Append(port);
      if (GetCount() == 0) {
        Append("No source available");
        SetSelection(0);
        Disable();
      } else {
        int found = FindString(m_params.socket_can_port);
        SetSelection(found != wxNOT_FOUND ? found : 0);
      }
    }

  private:
    ConnectionParams& m_params;

    // Check available SocketCAN interfaces, empty list if not linux
#if !defined(__linux__) || defined(__ANDROID__)
    std::vector<std::string> GetAvailableSocketCanInterfaces() {
      return {{}};
    }
#else
    static int print_intf(const struct intf_entry* entry, void* arg) {
      std::string iface = entry->intf_name;
      if (entry->intf_type == 1 && iface.find("can") != std::string::npos) {
        can_if_candidates.emplace_back(entry->intf_name);
      }
      return 0;
    }

    static std::vector<std::string> GetAvailableSocketCanInterfaces() {
      std::vector<std::string> rv;
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
        if (sock < 0) continue;

        // Get the interface index
        ifreq if_request = {{0}};
        strcpy(if_request.ifr_name, iface.c_str());
        if (ioctl(sock, SIOCGIFINDEX, &if_request) < 0) continue;

        // Check if interface is UP.
        sockaddr_can can_address = {0};
        can_address.can_family = AF_CAN;
        can_address.can_ifindex = if_request.ifr_ifindex;
        if (ioctl(sock, SIOCGIFFLAGS, &if_request) < 0) continue;

        if (if_request.ifr_flags & IFF_UP) rv.push_back(iface);
      }
      return rv;
    }
#endif  // !defined(__linux__) || defined(__ANDROID__)
  };

public:
  CanConfigPanel(wxWindow* parent, ConnectionParams& params)
      : wxPanel(parent), m_params(params) {
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    auto flags = wxSizerFlags().Border();
    hbox->Add(new wxStaticText(this, wxID_ANY, "Socketcan source"), flags);
    m_choice = new CanChoice(this, m_params);
    m_choice->Append("foobar");
    hbox->Add(m_choice, wxSizerFlags().Border());
    SetSizer(hbox);
    wxWindow::Layout();
    wxWindow::Show();
  }

private:
  ConnectionParams& m_params;
  wxChoice* m_choice;
};  // CanConfigPanel

class ConfigPanel : public wxScrolledWindow {
public:
  ConfigPanel(wxWindow* parent, ConnectionParams& params)
      : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                         wxVSCROLL | wxHSCROLL),
        m_params(params) {
    auto flags = wxSizerFlags(0).Expand();
    auto vbox = new wxBoxSizer(wxVERTICAL);
    m_serial_config_panel = new SerialConfigPanel(this, m_params);
    vbox->Add(m_serial_config_panel, flags);
    m_net_config_panel = new NetConfigPanel(this, m_params);
    vbox->Add(m_net_config_panel, flags);
    m_can_config_panel = new CanConfigPanel(this, m_params);
    vbox->Add(m_can_config_panel, flags);
    m_canable_config_panel = new CanableConfigPanel(this, m_params);
    vbox->Add(m_canable_config_panel, flags);
    m_internal_gps_config_panel = new InternalGpsConfigPanel(this, m_params);
    vbox->Add(m_internal_gps_config_panel, flags);
    m_internal_bt_config_panel = new InternalBtConfigPanel(this, m_params);
    vbox->Add(m_internal_bt_config_panel, flags);

    vbox->Show(m_serial_config_panel, false);
    vbox->Show(m_net_config_panel, false);
    vbox->Show(m_can_config_panel, false);
    vbox->Show(m_canable_config_panel, false);
    vbox->Show(m_internal_gps_config_panel, false);
    vbox->Show(m_internal_bt_config_panel, false);

    SetSizer(vbox);
    SetScrollRate(10, 10);
    SetActivePanel(ConnectionType::SERIAL);
    wxScrolled<wxPanel>::Layout();
    wxWindowBase::FitInside();
  }

  ~ConfigPanel() override = default;

  void SetActivePanel(ConnectionType type) {
    m_net_config_panel->Show(type == ConnectionType::NETWORK);
    m_can_config_panel->Show(type == ConnectionType::SOCKETCAN);
    m_serial_config_panel->Show(type == ConnectionType::SERIAL);
    m_canable_config_panel->Show(type == ConnectionType::CANABLE);
    m_internal_gps_config_panel->Show(type == ConnectionType::INTERNAL_GPS);
    m_internal_bt_config_panel->Show(type == ConnectionType::INTERNAL_BT);
    Layout();
    FitInside();
  }

  bool CheckStatus() {
    if (!m_net_config_panel->IsShown()) return true;
    return m_net_config_panel->CheckStatus();
  }

private:
  ConnectionParams& m_params;
  NetConfigPanel* m_net_config_panel;
  CanConfigPanel* m_can_config_panel;
  CanableConfigPanel* m_canable_config_panel;
  InternalGpsConfigPanel* m_internal_gps_config_panel;
  InternalBtConfigPanel* m_internal_bt_config_panel;
  SerialConfigPanel* m_serial_config_panel;
};

}  // Anonymous namespace

class ConnectionEditDlg : public AbstractConnectionEditDlg {
  class ButtonSizer : public wxStdDialogButtonSizer {
  public:
    ButtonSizer(wxWindow* parent, std::function<void(wxCommandEvent&)> on_ok,
                std::function<void(wxCommandEvent&)> on_cancel)
        : wxStdDialogButtonSizer(),
          m_on_ok(std::move(on_ok)),
          m_on_cancel(std::move(on_cancel)) {
      auto ok_btn = new wxButton(parent, wxID_OK);
      ok_btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent& ev) { m_on_ok(ev); });
      AddButton(ok_btn);
      auto cancel_btn = new wxButton(parent, wxID_CANCEL);
      cancel_btn->Bind(wxEVT_BUTTON,
                       [&](wxCommandEvent& ev) { m_on_cancel(ev); });
      AddButton(cancel_btn);
      SetAffirmativeButton(ok_btn);
      Realize();
    }

  private:
    std::function<void(wxCommandEvent&)> m_on_ok;
    std::function<void(wxCommandEvent&)> m_on_cancel;
  };

public:
  explicit ConnectionEditDlg(wxWindow* parent, std::function<void()> on_cancel,
                             ConnEditCallback on_ok)
      : AbstractConnectionEditDlg(parent),
        m_on_ok(std::move(on_ok)),
        m_on_cancel(std::move(on_cancel)),
        m_params(GetDefaultParams()),
        m_is_new(true) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto statbox = new wxStaticBoxSizer(wxVERTICAL, this, _("Edit connection"));
    auto on_type_btn_click = [&](ConnectionType ct) { OnTypeBtnClick(ct); };
    auto flags = wxSizerFlags().Border();
    m_type_buttons = new TypeButtons(this, on_type_btn_click, m_params);
    statbox->Add(m_type_buttons, wxSizerFlags().Border());
    statbox->Add(new wxStaticLine(this), wxSizerFlags().Expand());
    m_config_panel = new ConfigPanel(this, m_params);
    statbox->Add(m_config_panel, wxSizerFlags(1).Expand());
    vbox->Add(statbox, wxSizerFlags(1).Expand());

    auto on_ok_ = [&](wxCommandEvent& ev) { OnOkCancel(ev, true); };
    auto on_cancel_ = [&](wxCommandEvent& ev) { OnOkCancel(ev, false); };
    auto button_sizer = new ButtonSizer(this, on_ok_, on_cancel_);
    vbox->Add(button_sizer, flags.Expand());
    m_type_buttons->SetSelectedType(ConnectionType::NETWORK);
    OnTypeBtnClick(ConnectionType::NETWORK);

    SetSizer(vbox);
    wxWindow::Layout();
  }

  void Scroll(int hscroll, int vscroll) override {
    m_config_panel->Scroll(hscroll, vscroll);
  }

  void SetParams(std::unique_ptr<ConnectionParams> p) override {
    m_is_new = !static_cast<bool>(p);
    m_params = p ? *p : GetDefaultParams();
    OnTypeBtnClick(m_params.type);
    Reload();
  }

  ConnectionParams GetEditedParams() override { return m_params; }

private:
  ConnEditCallback m_on_ok;
  std::function<void()> m_on_cancel;
  ConnectionParams m_params;

  TypeButtons* m_type_buttons;
  ConfigPanel* m_config_panel;
  bool m_is_new;

  static void DoReload(wxWindow* root) {
    for (wxWindow* child : root->GetChildren()) {
      if (auto widget = dynamic_cast<Reloadable*>(child)) widget->Reload();
      DoReload(child);
    }
  }

  void Reload() { DoReload(this); }

  void OnTypeBtnClick(ConnectionType ct) {
    m_params.type = ct;
    m_config_panel->SetActivePanel(ct);
    Reload();
    Layout();
  }

  void OnOkCancel(wxCommandEvent&, bool ok) {
    if (!ok) {
      m_on_cancel();
    } else if (m_config_panel->CheckStatus()) {
      m_on_ok(std::make_unique<ConnectionParams>(m_params), m_is_new);
    }
  }

  static ConnectionParams GetDefaultParams() {
    ConnectionParams cp;
    cp.type = ConnectionType::NETWORK;
    return cp;
  }
};

AbstractConnectionEditDlg* AbstractConnectionEditDlg::Create(
    wxWindow* parent, std::function<void()> on_cancel, ConnEditCallback on_ok) {
  return new ConnectionEditDlg(parent, std::move(on_cancel), std::move(on_ok));
}
