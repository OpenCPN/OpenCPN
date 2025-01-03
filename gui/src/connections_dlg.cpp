#include <array>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <wx/bitmap.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#include "model/base_platform.h"
#include "model/comm_drv_factory.h"
#include "model/comm_drv_registry.h"
#include "model/comm_util.h"
#include "model/conn_params.h"
#include "model/conn_states.h"

#include "connections_dlg.h"

#include "connection_edit.h"
#include "conn_params_panel.h"
#include "gui_lib.h"
#include "navutil.h"
#include "std_filesystem.h"
#include "svg_utils.h"

extern MyConfig* pConfig;

static const auto kUtfArrowDown = wxString::FromUTF8(u8"\u25bc");
static const auto kUtfArrowRight = wxString::FromUTF8(u8"\u25ba");
static const auto kUtfCheckmark = wxString::FromUTF8(u8"\u2713");
static const auto kUtfCircle = wxString::FromUTF8(u8"\u3007");
static const auto kUtfCrossmark = wxString::FromUTF8(u8"\u274c");
static const auto kUtfDegrees = wxString::FromUTF8(u8"\u00B0");
static const auto kUtfExclamationMark = wxString::FromUTF8("!");
static const auto kUtfFilledCircle = wxString::FromUTF8(u8"\u2b24");
static const auto kUtfFisheye = wxString::FromUTF8(u8"\u25c9");
static const auto kUtfGear = wxString::FromUTF8(u8"\u2699");
static const auto kUtfMultiplyX = wxString::FromUTF8(u8"\u2715");
static const auto kUtfTrashbin = wxString::FromUTF8(u8"\U0001f5d1");

/** The "Add new connection" button */
class AddConnectionButton : public wxButton {
public:
  AddConnectionButton(wxWindow* parent, EventVar& evt_add_connection)
      : wxButton(parent, wxID_ANY, _("Add new connection...")),
        m_evt_add_connection(evt_add_connection) {
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,
         [&](wxCommandEvent& ev) { OnAddConnection(); });
  }

private:
  void OnAddConnection() {
    ConnectionEditDialog dialog(this);
    dialog.SetPropsLabel(_("Configure new connection"));
    dialog.SetDefaultConnectionParams();
    auto rv = dialog.ShowModal();
    if (rv == wxID_OK) {
      ConnectionParams* cp = dialog.GetParamsFromControls();
      if (cp) {
        cp->b_IsSetup = false;  // Trigger new stream
        TheConnectionParams().push_back(cp);
      }
      UpdateDatastreams();
      m_evt_add_connection.Notify();
    }
#ifdef __ANDROID__
    androidEnableRotation();
#endif
  }

  EventVar& m_evt_add_connection;
};

/** Grid with existing connections: type, port, status, etc. */
class Connections : public wxGrid {
public:
  Connections(wxWindow* parent,
              const std::vector<ConnectionParams*>& connections,
              EventVar& on_conn_delete)
      : wxGrid(parent, wxID_ANY),
        m_connections(connections),
        m_on_conn_delete(on_conn_delete) {
    auto set_enabled = [&](int row, bool b) { HandleEnable(row, b); };
    SetTable(new StringTable(set_enabled), false);
    GetTable()->AppendCols(7);
    static const std::array<wxString, 7> headers = {
        "", _("Protocol"), _("Direction"), _("Port"), _("Status"), "", ""};
    for (auto hdr = headers.begin(); hdr != headers.end(); hdr++)
      SetColLabelValue(static_cast<int>(hdr - headers.begin()), *hdr);
    HideRowLabels();
    SetColAttributes(parent);
    ReloadGrid(connections);
    DisableDragColSize();
    DisableDragRowSize();
    GetGridWindow()->Bind(wxEVT_MOTION, [&](wxMouseEvent& ev) {
      OnMouseMove(ev);
      ev.Skip();
    });
    Bind(wxEVT_GRID_SELECT_CELL,
         [&](wxGridEvent& ev) { OnSelectCell(ev.GetRow(), ev.GetCol()); });
    conn_change_lstnr.Init(
        m_conn_states.evt_conn_status_change,
        [&](ObservedEvt&) { OnConnectionChange(m_connections); });
  }

  /** Reload grid using data from given list of connections. */
  void ReloadGrid(const std::vector<ConnectionParams*>& connections) {
    ClearGrid();
    for (auto it = connections.begin(); it != connections.end(); it++) {
      auto row = static_cast<int>(it - connections.begin());
      EnsureRows(row);
      SetCellValue(row, 0, (*it)->bEnabled ? "1" : "");
      if ((*it)->bEnabled)
        m_tooltips[row][0] = _("Enabled, click to disable");
      else
        m_tooltips[row][0] = _("Disabled, click to enable");
      std::string protocol = NavAddr::BusToString((*it)->GetCommProtocol());
      SetCellValue(row, 1, protocol);
      SetCellValue(row, 2, (*it)->GetIOTypeValueStr());
      SetCellValue(row, 3, (*it)->GetStrippedDSPort());
      m_tooltips[row][3] = (*it)->UserComment;
      SetCellValue(row, 5, kUtfGear);  // ⚙
      m_tooltips[row][5] = _("Edit connection");
      SetCellValue(row, 6, kUtfTrashbin);  // 🗑
      m_tooltips[row][6] = _("Delete connection");
    }
    OnConnectionChange(m_connections);
    AutoSize();
  }

  wxSize GetMaxSize() {
    return wxSize(GetCharWidth() * 60,
                  (GetNumberRows() + 1) * GetCharHeight() * 2);
  }

  wxSize GetMinSize() {
    return wxSize(GetCharWidth() * 60,
                  std::max(GetNumberRows() + 1, 4) * GetCharHeight() * 2);
  }

private:
  /** Overrides SetCellValue in order to act on changes. */
  class StringTable : public wxGridStringTable {
  public:
    explicit StringTable(std::function<void(int, bool)> set_enabled)
        : wxGridStringTable(), m_set_enabled(std::move(set_enabled)) {}

    void SetValue(int row, int col, const wxString& value) override {
      wxGridStringTable::SetValue(row, col, value);
      if (col == 0) m_set_enabled(row, !value.empty());
    }

  private:
    std::function<void(int, bool)> m_set_enabled;
  };

  /** Return pointer to parameters related to row. */
  ConnectionParams* FindRowConnection(int row) {
    auto iface = GetCellValue(row, 3);
    auto bus = NavAddr::StringToBus(GetCellValue(row, 1).ToStdString());
    auto found = find_if(
        m_connections.begin(), m_connections.end(), [&](ConnectionParams* p) {
          return bus == p->GetCommProtocol() && iface == p->GetStrippedDSPort();
        });
    return found != m_connections.end() ? *found : nullptr;
  }

  /** Find index in m_connections for given pointer. */
  int FindConnectionIndex(ConnectionParams* cp) {
    using namespace std;
    auto found = find(m_connections.begin(), m_connections.end(), cp);
    if (found == m_connections.end()) return -1;
    return static_cast<int>(found - m_connections.begin());
  }

  /**
   *  Ensure that grid and tooltips has allocated at least given amount of
   *  rows.
   */
  void EnsureRows(size_t rows) {
    while (m_tooltips.size() <= rows)
      m_tooltips.push_back(std::vector<std::string>(7));
    while (GetNumberRows() <= static_cast<int>(rows)) AppendRows(1, false);
  }

  /** Set up column attributes: alignment, font size, read-only, etc. */
  void SetColAttributes(wxWindow* parent) {
    auto enable_attr = new wxGridCellAttr();
    enable_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    // enable_attr->SetFont(parent->GetFont().Scale(1.3));
    // enable_attr->SetReadOnly(true);
    enable_attr->SetRenderer(new wxGridCellBoolRenderer());
    enable_attr->SetEditor(new wxGridCellBoolEditor());
    SetColAttr(0, enable_attr);

    auto protocol_attr = new wxGridCellAttr();
    protocol_attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
    protocol_attr->SetReadOnly(true);
    SetColAttr(1, protocol_attr);

    auto in_out_attr = new wxGridCellAttr();
    in_out_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    in_out_attr->SetReadOnly(true);
    SetColAttr(2, in_out_attr);

    auto port_attr = new wxGridCellAttr();
    port_attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
    port_attr->SetReadOnly(true);
    SetColAttr(3, port_attr);

    auto status_attr = new wxGridCellAttr();
    status_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    status_attr->SetReadOnly(true);
    status_attr->SetFont(parent->GetFont().Scale(1.3));
    SetColAttr(4, status_attr);

    auto edit_attr = new wxGridCellAttr();
    edit_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    edit_attr->SetFont(parent->GetFont().Scale(1.3));
    edit_attr->SetReadOnly(true);
    SetColAttr(5, edit_attr);

    auto delete_attr = new wxGridCellAttr();
    delete_attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
    delete_attr->SetFont(parent->GetFont().Scale(1.3));
    delete_attr->SetTextColour(*wxRED);
    delete_attr->SetReadOnly(true);
    SetColAttr(6, delete_attr);
  }

  /** User mouse click on grid dispatcher. */
  void OnSelectCell(int row, int col) {
    if (col == 5) {
      HandleEdit(row);
    } else if (col == 6) {
      HandleDelete(row);
    }
  }
  /** Handle mouse movements i. e., the tooltips. */
  void OnMouseMove(wxMouseEvent& ev) {
    wxPoint pt = ev.GetPosition();
    int row = YToRow(pt.y);
    int col = XToCol(pt.x);
    if (col >= 0 && col < 7 && row >= 0 && row < GetNumberRows()) {
      auto& new_tooltip = m_tooltips[row][col];
      if (new_tooltip != GetGridWindow()->GetToolTipText())
        GetGridWindow()->SetToolTip(new_tooltip);
    }
  }

  /** Handle connections driver statistics status changes event. */
  void OnConnectionChange(const std::vector<ConnectionParams*>& connections) {
    for (auto it = connections.begin(); it != connections.end(); it++) {
      ConnState state = m_conn_states.GetDriverState(
          (*it)->GetCommProtocol(), (*it)->GetStrippedDSPort());
      if (!(*it)->bEnabled) state = ConnState::Disabled;
      auto row = static_cast<int>(it - connections.begin());
      EnsureRows(row);
      switch (state) {
        case ConnState::Disabled:
          SetCellValue(row, 4, kUtfFilledCircle);
          m_tooltips[row][4] = _("Disabled");
          break;
        case ConnState::NoStats:
          SetCellValue(row, 4, kUtfCircle);
          m_tooltips[row][4] = _("No driver statistics available");
          break;
        case ConnState::NoData:
          SetCellValue(row, 4, kUtfExclamationMark);
          m_tooltips[row][4] = _("No data flowing through connection");
          break;
        case ConnState::Unavailable:
          SetCellValue(row, 4, kUtfMultiplyX);
          m_tooltips[row][4] = _("The device is unavailable");
          break;
        case ConnState::Ok:
          SetCellValue(row, 4, kUtfCheckmark);
          m_tooltips[row][4] = _("Data is flowing");
          break;
      }
    }
  }

  /** Handle user click on the enable/disable checkbox. */
  void HandleEnable(int row, bool enabled) {
    ConnectionParams* cp = FindRowConnection(row);
    if (!cp) return;
    cp->bEnabled = enabled;
    cp->b_IsSetup = FALSE;  // trigger a rebuild/takedown of the connection
    if (enabled)
      m_tooltips[row][0] = _("Enabled, click to disable");
    else
      m_tooltips[row][0] = _("Disabled, click to enable");
    DriverStats stats;
    stats.driver_iface = cp->GetStrippedDSPort();
    stats.driver_bus = cp->GetCommProtocol();
    m_conn_states.HandleDriverStats(stats);
    pConfig->UpdateSettings();
    StopAndRemoveCommDriver(cp->GetStrippedDSPort(), cp->GetCommProtocol());
    if (cp->bEnabled) MakeCommDriver(cp);
    cp->b_IsSetup = true;
  }

  /** Handle user click on Edit gear symbol. */
  void HandleEdit(int row) {
    ConnectionParams* cp = FindRowConnection(row);
    if (cp) {
      ConnectionEditDialog dialog(m_parent);
      dialog.SetPropsLabel(_("Edit Selected Connection"));
      dialog.PreloadControls(cp);
      auto rv = dialog.ShowModal();
      if (rv == wxID_OK) {
        ConnectionParams* cp_edited = dialog.GetParamsFromControls();
        delete cp->m_optionsPanel;
        StopAndRemoveCommDriver(cp->GetStrippedDSPort(), cp->GetCommProtocol());
        int index = FindConnectionIndex(cp);
        assert(index != -1 && "Cannot look up connection index");
        TheConnectionParams()[index] = cp_edited;
        cp_edited->b_IsSetup = false;  // Trigger new stream
        ReloadGrid(m_connections);
        UpdateDatastreams();
      }
    }
  }

  /** Handle user click on Delete trashbin symbol. */
  void HandleDelete(int row) {
    ConnectionParams* cp = FindRowConnection(row);
    auto found = std::find(m_connections.begin(), m_connections.end(), cp);
    if (found != m_connections.end()) {
      std::stringstream ss;
      ss << _("Ok to delete connection on ") << (*found)->GetStrippedDSPort();
      int rcode = OCPNMessageBox(this, ss.str(), _("Delete connection?"),
                                 wxYES_NO | wxCANCEL);
      if (rcode != wxID_OK && rcode != wxID_YES) return;
      delete (*found)->m_optionsPanel;
      StopAndRemoveCommDriver((*found)->GetStrippedDSPort(),
                              (*found)->GetCommProtocol());
      TheConnectionParams().erase(found);
      m_on_conn_delete.Notify();
      if (GetNumberRows() > static_cast<int>(m_connections.size()))
        DeleteRows(GetNumberRows() - 1);
    }
  }

  ObsListener conn_change_lstnr;
  std::vector<std::vector<std::string>> m_tooltips;
  ConnStates m_conn_states;
  const std::vector<ConnectionParams*>& m_connections;
  EventVar& m_on_conn_delete;
};

class GeneralPanel : public wxPanel {
public:
  explicit GeneralPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    auto sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("General"));
    SetSizer(sizer);

    auto standard_panel = new StandardPanel(this);
    sizer->Add(standard_panel, wxSizerFlags().Expand());
  }

private:
  class GarminCheckbox : public wxCheckBox {
  public:
    explicit GarminCheckbox(wxWindow* parent)
        : wxCheckBox(parent, wxID_ANY,
                     _("Use Garmin GRMN (Host) mode for uploads")) {}
  };
  class FurunoCheckbox : public wxCheckBox {
  public:
    explicit FurunoCheckbox(wxWindow* parent)
        : wxCheckBox(parent, wxID_ANY, _("Format uploads for Furuno GP4X")) {}
  };

  class StandardPanel : public wxPanel {
  public:
    explicit StandardPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
      auto sizer = new wxBoxSizer(wxVERTICAL);
      auto flags = wxSizerFlags().Border();
      sizer->Add(new GarminCheckbox(this), flags);
      sizer->Add(new FurunoCheckbox(this), flags);
      SetSizer(sizer);
    }
  };
};

/** The "Show advanced" text + right/down triangle and handler. */
class ShowAdvanced : public wxStaticText {
public:
  ShowAdvanced(wxWindow* parent, std::function<void(bool)> on_toggle)
      : wxStaticText(parent, wxID_ANY, ""),
        m_on_toggle(on_toggle),
        m_show_advanced(true) {
    Toggle();
    Bind(wxEVT_LEFT_DOWN, [&](wxMouseEvent& ev) { Toggle(); });
  }

private:
  wxString m_label = _("Advanced ");
  std::function<void(bool)> m_on_toggle;
  bool m_show_advanced;

  void Toggle() {
    m_show_advanced = !m_show_advanced;
    m_label = _("Advanced ");
    m_label += (m_show_advanced ? kUtfArrowDown : kUtfArrowRight);
    SetLabelText(m_label);
    m_on_toggle(m_show_advanced);
  }
};

class AdvancedPanel : public wxPanel {
public:
  explicit AdvancedPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    auto sizer = new wxStaticBoxSizer(wxVERTICAL, this, "");
    sizer->Add(new BearingsCheckbox(this), wxSizerFlags().Expand());
    sizer->Add(new NmeaFilterRow(this), wxSizerFlags().Expand());
    sizer->Add(new TalkerIdRow(this), wxSizerFlags().Expand());
    sizer->Add(new FilterButton(this), wxSizerFlags().Border());
    SetSizer(sizer);
  }

private:
  class BearingsCheckbox : public wxCheckBox {
  public:
    BearingsCheckbox(wxWindow* parent)
        : wxCheckBox(parent, wxID_ANY,
                     _("Use magnetic bearing in output sentence APB")) {}
  };

  class NmeaFilterRow : public wxPanel {
  public:
    NmeaFilterRow(wxWindow* parent) : wxPanel(parent) {
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      auto checkbox = new wxCheckBox(
          this, wxID_ANY,
          _("Filter NMEA course and speed data. Filter period: "));
      hbox->Add(checkbox, wxSizerFlags().Align(wxALIGN_CENTRE));
      auto filter_period = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition,
                                          wxSize(50, 3 * GetCharWidth()), 0);
      hbox->Add(filter_period, wxSizerFlags().Border());
      SetSizer(hbox);
    }
  };

  class TalkerIdRow : public wxPanel {
  public:
    TalkerIdRow(wxWindow* parent) : wxPanel(parent) {
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      hbox->Add(new wxStaticText(this, wxID_ANY, _("NMEA 0183 Talker Id: ")),
                wxSizerFlags().Align(wxALIGN_CENTRE_VERTICAL).Border());
      auto talker_id = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition,
                                      wxSize(50, 3 * GetCharWidth()));
      talker_id->SetLabel("EC");
      hbox->Add(talker_id, wxSizerFlags().Border());
      SetSizer(hbox);
    }
  };

  class FilterButton : public wxButton {
  public:
    FilterButton(wxWindow* parent)
        : wxButton(parent, wxID_ANY, _("Adjust communication priorities...")) {}
  };
};

/** Scrollable window wrapping the client i. e., the grid. */
class ScrolledWindow : public wxScrolledWindow {
public:
  ScrolledWindow(wxWindow* parent)
      : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                         wxVSCROLL) {}

  /** Set contents and size limits for scrollable area. */
  void AddClient(wxWindow* client, wxSize max_size, wxSize min_size) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(client);
    SetSizer(vbox);
    vbox->Layout();
    SetScrollRate(0, 10);
    SetMinSize(min_size);
    SetMaxSize(max_size);
  }
};

/** Main window: connections grid, "Add new connection", general options. */
ConnectionsDlg::ConnectionsDlg(
    wxWindow* parent, const std::vector<ConnectionParams*>& connections,
    std::function<void()> on_exit)
    : wxFrame(parent, wxID_ANY, _("Connections")),
      m_connections(connections),
      m_on_exit(std::move(on_exit)) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto scrolled_window = new ScrolledWindow(this);
  auto conn_grid =
      new Connections(scrolled_window, m_connections, m_evt_add_connection);
  scrolled_window->AddClient(conn_grid, conn_grid->GetMaxSize(),
                             conn_grid->GetMinSize());
  vbox->Add(scrolled_window, wxSizerFlags(5).Expand().Border());
  vbox->Add(new AddConnectionButton(this, m_evt_add_connection),
            wxSizerFlags().Border());
  vbox->Add(0, 0, 1);  // Expanding spacer
  vbox->Add(new GeneralPanel(this),
            wxSizerFlags().Border(wxLEFT | wxDOWN | wxRIGHT).Expand());
  auto advanced_panel = new AdvancedPanel(this);
  auto on_toggle = [&, advanced_panel, vbox](bool show) {
    advanced_panel->Show(show);
    vbox->SetSizeHints(this);
    vbox->Fit(this);
  };
  vbox->Add(new ShowAdvanced(this, on_toggle),
            wxSizerFlags().Border(wxLEFT | wxDOWN).Expand());
  vbox->Add(advanced_panel,
            wxSizerFlags().Border(wxLEFT | wxDOWN | wxRIGHT).Expand());
  SetSizer(vbox);
  SetAutoLayout(true);

  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent&) {
    m_on_exit();
    Destroy();
  });
  auto on_evt_add_connection = [&, conn_grid](ObservedEvt&) {
    conn_grid->ReloadGrid(TheConnectionParams());
    Layout();
  };
  m_add_connection_lstnr.Init(m_evt_add_connection, on_evt_add_connection);
};
