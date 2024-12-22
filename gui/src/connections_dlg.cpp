#include <array>

#include <wx/bitmap.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/window.h>

#include "model/base_platform.h"
#include "model/conn_params.h"
#include "model/conn_states.h"
#include "connections_dlg.h"
#include "connection_edit.h"
#include "conn_params_panel.h"

#include "std_filesystem.h"
#include "svg_utils.h"
#include "model/comm_drv_registry.h"
#include "model/comm_util.h"

static const char* kCheckmark = "\u2713";
static const char* kExclamationMark = "!";
static const char* kFisheye = "\u25c9";
static const char* kCircle = "\u3007";
static const char* kTrashbin = "\U0001f5d1";
static const char* kGear = "\u2699";

class AddConnectionPanel : public wxPanel {
public:
  AddConnectionPanel(wxWindow* parent) : wxPanel(parent) {
    int h;
    int w;
    parent->GetTextExtent("+", &w, &h);

    auto datadir = g_BasePlatform->GetSharedDataDir().ToStdString();
    auto path = fs::path(datadir) / "uidata" / "plus.svg";
    wxBitmap bitmap = LoadSVG(path.c_str(), h * 3, w * 3);

    auto static_bitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);
    static_bitmap->Bind(wxEVT_LEFT_DOWN, [](wxMouseEvent& ev) {
      std::cout << "Add connection\n";
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
};

class Connections : public wxGrid {
public:
  void SetColAttributes(wxWindow* parent) {
    auto enable_attr = new wxGridCellAttr();
    enable_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    enable_attr->SetFont(parent->GetFont().Scale(1.3));
    enable_attr->SetReadOnly(true);
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

  Connections(wxWindow* parent,
              const std::vector<ConnectionParams*>& connections)
      : wxGrid(parent, wxID_ANY), m_connections(connections) {
    CreateGrid(0, 7);
    static const std::array<std::string, 7> headers = {
        "", "Protocol", "Direction", "Port", "Status", "", ""};
    for (auto hdr = headers.begin(); hdr != headers.end(); hdr++) {
      SetColLabelValue(hdr - headers.begin(), *hdr);
    }
    HideRowLabels();
    SetColAttributes(parent);
    ReloadGrid(connections);
    DisableDragColSize();
    DisableDragRowSize();

    GetGridWindow()->Bind(wxEVT_MOTION, [&](wxMouseEvent ev) {
      wxPoint pt = ev.GetPosition();
      int row = YToRow(pt.y);
      int col = XToCol(pt.x);
      SetToolTip(m_tooltips[row][col]);
      ev.Skip();
    });

    Bind(wxEVT_GRID_SELECT_CELL,
         [&](wxGridEvent& ev) { OnSelectCell(ev.GetRow(), ev.GetCol()); });
    conn_change_lstnr.Init(
        m_conn_states.evt_conn_status_change,
        [&](ObservedEvt) { OnConnectionChange(m_connections); });
  }

  ConnectionParams* FindRowConnection(int row) {
    auto iface = GetCellValue(row, 3);
    auto bus = NavAddr::StringToBus(GetCellValue(row, 1).ToStdString());
    auto found = find_if(
        m_connections.begin(), m_connections.end(), [&](ConnectionParams* p) {
          return bus == p->GetCommProtocol() && iface == p->GetStrippedDSPort();
        });
    return found != m_connections.end() ? *found : nullptr;
  }

  int FindConnectionIndex(ConnectionParams* cp) {
    using namespace std;
    auto found = find(m_connections.begin(), m_connections.end(), cp);
    return found != m_connections.end() ? found - m_connections.begin() : -1;
  }

  void HandleEnable(int row) {
    ConnectionParams* cp = FindRowConnection(row);
    if (!cp) return;
    cp->bEnabled = !cp->bEnabled;
    SetEnabled(row, cp->bEnabled);
    DriverStats stats;
    stats.driver_iface = cp->GetStrippedDSPort();
    stats.driver_bus = cp->GetCommProtocol();
    m_conn_states.HandleDriverStats(stats);
    ConnState state = m_conn_states.GetDriverState(cp->GetCommProtocol(),
                                                   cp->GetStrippedDSPort());
  }

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

  void OnSelectCell(int row, int col) {
    if (col == 0) {
      HandleEnable(row);
    } else if (col == 5) {
      HandleEdit(row);
    }
  }

  void EnsureRows(size_t row) {
    while (m_tooltips.size() <= row)
      m_tooltips.push_back(std::vector<std::string>(7));
    while (GetNumberRows() <= static_cast<int>(row)) AppendRows();
  }

  void SetEnabled(int row, bool enabled) {
    if (enabled) {
      SetCellValue(row, 0, "☑");
      m_tooltips[row][0] = "Enabled, click to disable";
    } else {
      SetCellValue(row, 0, "☐");
      m_tooltips[row][0] = "Disabled, click to enable";
    }
    ForceRefresh();
  }

  void ReloadGrid(const std::vector<ConnectionParams*>& connections) {
    for (auto it = connections.begin(); it != connections.end(); it++) {
      size_t row = it - connections.begin();
      ConnState state = m_conn_states.GetDriverState(
          (*it)->GetCommProtocol(), (*it)->GetStrippedDSPort());
      EnsureRows(row);
      SetEnabled(row, state != ConnState::Disabled);
      std::string protocol = NavAddr::BusToString((*it)->GetCommProtocol());
      SetCellValue(row, 1, protocol);
      SetCellValue(row, 2, (*it)->GetIOTypeValueStr());
      SetCellValue(row, 3, (*it)->GetStrippedDSPort());
      SetCellValue(row, 5, kGear);  // ⚙
      m_tooltips[row][5] = "Edit connection";
      SetCellValue(row, 6, kTrashbin);  // 🗑
      m_tooltips[row][6] = "Delete connection";
      OnConnectionChange(TheConnectionParams());
      AutoSize();
    }
  }

  void OnConnectionChange(const std::vector<ConnectionParams*>& connections) {
    static const char* kCrossmark = "\u2715";
    for (auto it = connections.begin(); it != connections.end(); it++) {
      ConnState state = m_conn_states.GetDriverState(
          (*it)->GetCommProtocol(), (*it)->GetStrippedDSPort());
      size_t row = it - connections.begin();
      EnsureRows(row);
      switch (state) {
        case ConnState::Disabled:
          SetCellValue(row, 4, kFisheye);
          m_tooltips[row][4] = _("Disabled");
          break;
        case ConnState::NoStats:
          SetCellValue(row, 4, kCircle);
          m_tooltips[row][4] = _("No driver statistics available");
          break;
        case ConnState::NoData:
          SetCellValue(row, 4, kExclamationMark);
          m_tooltips[row][4] = _("No data flowing through connection");
          break;
        case ConnState::Unavailable:
          SetCellValue(row, 4, kCrossmark);
          m_tooltips[row][4] = _("The device is unavailable");
          break;
        case ConnState::Ok:
          SetCellValue(row, 4, kCheckmark);
          m_tooltips[row][4] = _("Data is flowing");
          break;
      }
    }
  }

  ObsListener conn_change_lstnr;
  std::vector<std::vector<std::string>> m_tooltips;
  ConnStates m_conn_states;
  const std::vector<ConnectionParams*>& m_connections;
};

ConnectionsDlg::ConnectionsDlg(
    wxWindow* parent, const std::vector<ConnectionParams*>& connections,
    std::function<void()> on_exit)
    : wxFrame(parent, wxID_ANY, _("Connections")),
      m_connections(connections),
      m_on_exit(on_exit) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto flags = wxSizerFlags().Expand().DoubleBorder();
  vbox->Add(new Connections(this, m_connections), flags);
  vbox->Add(new AddConnectionPanel(this), flags);
  SetSizer(vbox);
  Fit();
  Show();
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& ev) {
    m_on_exit();
    Destroy();
  });
};
