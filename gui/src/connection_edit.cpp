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

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
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
#include <wx/panel.h>
#include <wx/platinfo.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <wx/weakref.h>
#include <wx/window.h>
#include <wx/wrapsizer.h>

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
#include "conn_edit_commons.h"

#include "libgui/expand_panel.h"

#include "model/conn_params.h"

#include "conn_params_panel.h"
#include "filter_sentence_dlg.h"
#include "ocpn_platform.h"
#include "options.h"
#include "udev_rule_mgr.h"

// Make _() return std::string instead of wxString;
#undef _
#if wxCHECK_VERSION(3, 2, 0)
#define _(s) wxGetTranslation(wxASCII_STR(s)).ToStdString()
#else
#define _(s) wxGetTranslation((s)).ToStdString()
#endif

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

/*--------------------------------------*/
namespace conn_edit {

// CanConfigPanel global scope support
#if defined(__linux__) && !defined(__ANDROID__)
intf_t* intf;
std::vector<std::string> can_if_candidates;
#endif

/** Garnin mode checkbox. */
class GarminCheckBoxImpl : public wxCheckBox, public Reloadable {
public:
  GarminCheckBoxImpl(wxWindow* parent, ConnectionParams& params)
      : wxCheckBox(parent, wxID_ANY, _("Use Garmin (GRMN) mode for input")),
        m_params(params) {
    GarminCheckBoxImpl::Reload();
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
      vbox->Add(new GarminCheckBoxImpl(this, m_params), flags);
      vbox->Add(FilterConfigPanel::Create(this, m_params), flags);
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
    vbox->Add(conn_edit::InputOutputPanel::Create(this, m_params));

    auto advanced_panel = new AdvancedPanel(this, m_params);
    auto adv_box = new wxBoxSizer(wxHORIZONTAL);
    adv_box->Add(new wxStaticText(this, wxID_ANY, _("Advanced")), flags);
    auto on_expand_click = [&] {
      if (auto parent = GetParent()) parent->Layout();
      if (auto* scrollwin = FindParentOfType<wxScrolledWindow>(this))
        scrollwin->FitInside();
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

  void OnClick(wxCommandEvent&) {
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
    auto port_text_ctrl = PortCombo::Create(this, m_params.serial_port);
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
      Add(PortCombo::Create(parent, m_params.serial_port), flags);
      Add(new wxStaticText(parent, wxID_ANY, _("Baudrate")), text_flags);
      Add(new BaudChoice(parent, m_params.baudrate), flags);
      Add(new wxStaticText(parent, wxID_ANY, _("Protocol")), text_flags);
      Add(ProtocolChoice::Create(parent, m_params), flags);
      Add(new wxStaticText(parent, wxID_ANY, _("User Comment")), text_flags);
      Add(new CommentText(parent, m_params.user_comment),
          wxSizerFlags(1).Border().Expand());
    }

  private:
    ConnectionParams& m_params;
  };  // SerialParamsGrid

  class GridPanel : public wxPanel, public Reloadable {
  public:
    GridPanel(wxWindow* parent, ConnectionParams& params)
        : wxPanel(parent, wxID_ANY),
          m_params(params),
          m_grid(new SerialParamsGrid(this, m_params)) {
      SetSizer(m_grid);
      wxWindow::Layout();
    }

    void Reload() override {
      // Serial connections are n0182 or n2000, nothing else.
      if (m_params.type != ConnectionType::SERIAL) return;
      if (m_params.data_protocol == DataProtocol::PROTO_NMEA0183 ||
          m_params.data_protocol == DataProtocol::PROTO_NMEA2000)
        return;
      m_params.data_protocol = DataProtocol::PROTO_NMEA0183;
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
      vbox->Add(new GarminCheckBoxImpl(this, m_params), flags);
      vbox->Add(FilterConfigPanel::Create(this, m_params), flags);
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
    auto advanced_ctrl = new ExpandablePanel(this, m_advanced_panel, [&] {
      if (auto parent = GetParent()) parent->Layout();
      if (auto* scrollwin = FindParentOfType<wxScrolledWindow>(this))
        scrollwin->FitInside();
    });
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(this, wxID_ANY, _("Advanced")), flags);
    hbox->Add(advanced_ctrl, flags);

    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(m_grid, wxSizerFlags().Border().Expand());
    vbox->Add(InputOutputPanel::Create(this, params), flags);
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

  /**
   * Check that user data is sane, possibly run dialogs for missing or
   * bad input.
   * @return true if user data is ok to be submitted.
   */
  [[nodiscard]] bool CheckStatus() {
    return CheckSerialAccess(this, m_params.serial_port.ToStdString());
  }

private:
  ConnectionParams& m_params;
  wxWindow* m_advanced_panel;
  GridPanel* m_grid;
};  // SerialConfigPanel

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
    static int print_intf(const struct intf_entry* entry, void*) {
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
    m_net_config_panel = conn_edit::NetConfigPanel::Create(this, m_params);
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
    if (m_serial_config_panel->IsShown())
      return m_serial_config_panel->CheckStatus();
    if (m_net_config_panel->IsShown()) return m_net_config_panel->CheckStatus();
    return true;
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

}  // namespace conn_edit

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
    m_type_buttons =
        new conn_edit::TypeButtons(this, on_type_btn_click, m_params);
    statbox->Add(m_type_buttons, wxSizerFlags().Border());
    statbox->Add(new wxStaticLine(this), wxSizerFlags().Expand());
    m_config_panel = new conn_edit::ConfigPanel(this, m_params);
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

  void EnableScroll(bool enable) override {
    if (enable) {
      m_config_panel->Layout();
      m_config_panel->FitInside();
      m_config_panel->Scroll(0, 0);
      m_config_panel->ShowScrollbars(wxSHOW_SB_DEFAULT, wxSHOW_SB_DEFAULT);
    } else {
      m_config_panel->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
    }
  }

  /** Synchronize widgets and parameters. */
  void Reload() override { DoReload(this); }

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

  conn_edit::TypeButtons* m_type_buttons;
  conn_edit::ConfigPanel* m_config_panel;
  bool m_is_new;

  static void DoReload(wxWindow* root) {
    for (wxWindow* child : root->GetChildren()) {
      if (auto widget = dynamic_cast<conn_edit::Reloadable*>(child))
        widget->Reload();
      DoReload(child);
    }
  }

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
