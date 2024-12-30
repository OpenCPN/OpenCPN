#include <array>
#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <wx/bitmap.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
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

static const auto kUtfCheckmark = wxString::Format("%c", 0x2713);
static const auto kUtfCircle = wxString::Format("%c", 0x3007);
static const auto kUtfCrossmark = wxString::Format("%c", 0x2715);
static const auto kUtfDegrees = wxString::Format("%c", 0x00B0);
static const auto kUtfExclamationMark = wxString::Format("!");
static const auto kUtfFilledCircle = wxString::Format("%c", 0x2b24);
static const auto kUtfFisheye = wxString::Format("%c", 0x25c9);
static const auto kUtfGear = wxString::Format("%c", 0x2699);
static const auto kUtfTrashbin = wxString::Format("%lc", 0x1f5d1);

/** The "Add new connection" plus sign */
class AddConnectionPanel : public wxPanel {
public:
  AddConnectionPanel(wxWindow* parent, EventVar& evt_add_connection)
      : wxPanel(parent), m_evt_add_connection(evt_add_connection) {
    int h;
    int w;
    parent->GetTextExtent("+", &w, &h);

    auto datadir = g_BasePlatform->GetSharedDataDir().ToStdString();
    auto path = fs::path(datadir) / "uidata" / "plus.svg";
    wxBitmap bitmap = LoadSVG(path.c_str(), h * 3, w * 3);

    auto static_bitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);
    static_bitmap->Bind(wxEVT_LEFT_DOWN, [&](wxMouseEvent& ev) {
      OnAddConnection();
      ev.Skip();
    });

    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    auto flags = wxSizerFlags().Expand().Border();
    sizer->Add(static_bitmap, flags);
    SetSizer(sizer);
    Fit();
    Show();
    SetToolTip("Add new connection");
  }

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
    AssignTable(new StringTable(set_enabled));
    GetTable()->AppendCols(7);
    static const std::array<std::string, 7> headers = {
        "", "Protocol", "Direction", "Port", "Status", "", ""};
    for (auto hdr = headers.begin(); hdr != headers.end(); hdr++)
      SetColLabelValue(hdr - headers.begin(), *hdr);
    HideRowLabels();
    SetColAttributes(parent);
    ReloadGrid(connections);
    DisableDragColSize();
    DisableDragRowSize();
    GetGridWindow()->Bind(wxEVT_MOTION, [&](wxMouseEvent ev) {
      wxPoint pt = ev.GetPosition();
      EnsureRows(YToRow(pt.y));
      SetToolTip(m_tooltips[YToRow(pt.y)][XToCol(pt.x)]);
      ev.Skip();
    });

    Bind(wxEVT_GRID_SELECT_CELL,
         [&](wxGridEvent& ev) { OnSelectCell(ev.GetRow(), ev.GetCol()); });
    conn_change_lstnr.Init(
        m_conn_states.evt_conn_status_change,
        [&](ObservedEvt) { OnConnectionChange(m_connections); });
  }

  /** Reload grid using data from given list of connections. */
  void ReloadGrid(const std::vector<ConnectionParams*>& connections) {
    ClearGrid();
    for (auto it = connections.begin(); it != connections.end(); it++) {
      size_t row = it - connections.begin();
      EnsureRows(row);
      SetCellValue(row, 0, (*it)->bEnabled ? "1" : "");
      std::string protocol = NavAddr::BusToString((*it)->GetCommProtocol());
      SetCellValue(row, 1, protocol);
      SetCellValue(row, 2, (*it)->GetIOTypeValueStr());
      SetCellValue(row, 3, (*it)->GetStrippedDSPort());
      SetCellValue(row, 5, kUtfGear);  // ⚙
      m_tooltips[row][5] = "Edit connection";
      SetCellValue(row, 6, kUtfTrashbin);  // 🗑
      m_tooltips[row][6] = "Delete connection";
    }
    OnConnectionChange(m_connections);
    AutoSize();
  }

private:
  /** Overrides SetCellValue in order to act on changes. */
  class StringTable : public wxGridStringTable {
  public:
    StringTable(std::function<void(int, bool)> set_enabled)
        : wxGridStringTable(), m_set_enabled(set_enabled) {}

    void SetValue(int row, int col, const wxString& value) {
      wxGridStringTable::SetValue(row, col, value);
      if (col == 0) m_set_enabled(row, value != "");
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
    return found != m_connections.end() ? found - m_connections.begin() : -1;
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

  /** Handle connections driver statistics status changes event. */
  void OnConnectionChange(const std::vector<ConnectionParams*>& connections) {
    for (auto it = connections.begin(); it != connections.end(); it++) {
      ConnState state = m_conn_states.GetDriverState(
          (*it)->GetCommProtocol(), (*it)->GetStrippedDSPort());
      if (!(*it)->bEnabled) state = ConnState::Disabled;
      size_t row = it - connections.begin();
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
          SetCellValue(row, 4, kUtfCrossmark);
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
      int rcode =
          OCPNMessageBox(this, ss.str(),
                         _("Delete connection?"), wxYES_NO | wxCANCEL);
      delete (*found)->m_optionsPanel;
      TheConnectionParams().erase(found);
      StopAndRemoveCommDriver((*found)->GetStrippedDSPort(),
                              (*found)->GetCommProtocol());
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

/** Main window: connections grid + "Add new connection" plus sign. */
ConnectionsDlg::ConnectionsDlg(
    wxWindow* parent, const std::vector<ConnectionParams*>& connections,
    std::function<void()> on_exit)
    : wxFrame(parent, wxID_ANY, _("Connections")),
      m_connections(connections),
      m_on_exit(on_exit) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto flags = wxSizerFlags().Expand().DoubleBorder();
  auto conn_grid = new Connections(this, m_connections, m_evt_add_connection);
  vbox->Add(conn_grid, flags);
  vbox->Add(new AddConnectionPanel(this, m_evt_add_connection), flags);
  SetSizer(vbox);
  Fit();
  Show();

  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent&) {
    m_on_exit();
    Destroy();
  });
  auto on_evt_add_connection = [&, conn_grid](ObservedEvt) {
    conn_grid->ReloadGrid(TheConnectionParams());
    Layout();
    Fit();
    Refresh();
  };
  m_add_connection_lstnr.Init(m_evt_add_connection, on_evt_add_connection);
};
