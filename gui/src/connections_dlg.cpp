#include <array>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <wx/arrstr.h>
#include <wx/bitmap.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#include "model/base_platform.h"
#include "model/comm_drv_factory.h"
#include "model/comm_drv_registry.h"
#include "model/comm_util.h"
#include "model/config_vars.h"
#include "model/conn_params.h"
#include "model/conn_states.h"
#include "model/notification_manager.h"

#include "connections_dlg.h"

#include "color_handler.h"
#include "connection_edit.h"
#include "conn_params_panel.h"
#include "gui_lib.h"
#include "navutil.h"
#include "OCPNPlatform.h"
#include "priority_gui.h"
#include "std_filesystem.h"
#include "svg_utils.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

extern OCPNPlatform* g_Platform;

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

static const char* const TopScrollWindowName = "TopScroll";

static inline bool IsWindows() {
  return wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_WINDOWS;
}

/** Standard icons bitmaps: settings gear, trash bin, etc. */
class StdIcons {
private:
  const double m_size;
  const fs::path m_svg_dir;

  /** Return platform dependent icon size. */
  double GetSize(wxWindow* parent) {
    double size = parent->GetCharHeight() * (IsWindows() ? 1.3 : 1.0);
#if wxCHECK_VERSION(3, 1, 2)
    // Apply scale factor, mostly for Windows. Other platforms
    // does this in the toolkits, ToDIP() is aware of this.
    size *= static_cast<double>(parent->ToDIP(100)) / 100.;
#endif
    // Force minimum physical size for touch screens
    if (g_btouch) {
      double pixel_per_mm =
          wxGetDisplaySize().x / g_Platform->GetDisplaySizeMM();
      size = std::max(size, 7.0 * pixel_per_mm);
    }
    return size;
  }

  wxBitmap LoadIcon(const std::string filename) {
    fs::path path = m_svg_dir / filename;
    return LoadSVG(path.string(), m_size, m_size);
  }

public:
  StdIcons(wxWindow* parent)
      : m_size(GetSize(parent)),
        m_svg_dir(fs::path(g_Platform->GetSharedDataDir().ToStdString()) /
                  "uidata" / "MUI_flat"),
        trashbin(LoadIcon("trash_bin.svg")),
        settings(LoadIcon("setting_gear.svg")),
        filled_circle(LoadIcon("circle-on.svg")),
        open_circle(LoadIcon("circle-off.svg")),
        exclaim_mark(LoadIcon("exclaim_mark.svg")),
        x_mult(LoadIcon("X_mult.svg")),
        check_mark(LoadIcon("check_mark.svg")) {}

  const wxBitmap trashbin;
  const wxBitmap settings;
  const wxBitmap filled_circle;
  const wxBitmap open_circle;
  const wxBitmap exclaim_mark;
  const wxBitmap x_mult;
  const wxBitmap check_mark;
};

/** Custom renderer class for rendering bitmap in a grid cell */
class BitmapCellRenderer : public wxGridCellRenderer {
public:
  BitmapCellRenderer(const wxBitmap& bitmap)
      : status(ConnState::Disabled), m_bitmap(bitmap) {}

  // Update the bitmap dynamically
  void SetBitmap(const wxBitmap& bitmap) { m_bitmap = bitmap; }

  void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect,
            int row, int col, bool isSelected) {
    if (IsWindows()) {
      dc.SetBrush(wxBrush(GetGlobalColor("DILG1")));
      dc.DrawRectangle(rect);
    }
    // Draw the bitmap centered in the cell
    dc.DrawBitmap(m_bitmap, rect.x + (rect.width - m_bitmap.GetWidth()) / 2,
                  rect.y + (rect.height - m_bitmap.GetHeight()) / 2, true);
  }

  wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row,
                     int col) {
    // Return the size of the bitmap as the best size for the cell
    return wxSize(m_bitmap.GetWidth(), m_bitmap.GetHeight());
  }

  BitmapCellRenderer* Clone() const { return new BitmapCellRenderer(m_bitmap); }
  ConnState status;

private:
  wxBitmap m_bitmap;
};

/** std::sort support: Compare two ConnectionParams w r t given column */
class ConnCompare {
public:
  ConnCompare(int col) : m_col(col) {}

  bool operator()(ConnectionParams* p1, ConnectionParams* p2) {
    switch (m_col) {
      case 0:
        return int(p1->bEnabled) > int(p2->bEnabled);
      case 1:
        return p1->GetCommProtocol() < p2->GetCommProtocol();
      case 2:
        return p1->GetIOTypeValueStr() < p2->GetIOTypeValueStr();
      case 3:
        return p1->GetStrippedDSPort() < p2->GetStrippedDSPort();
      default:
        return false;
    }
  }

private:
  const int m_col;
};

/**
 * Interface implemented by widgets supporting Apply and Cancel.
 */
class ApplyCancel {
public:
  /** Make values set by user actually being used. */
  virtual void Apply() = 0;

  /**
   * Restore values modified by user to their pristine state, often in a global.
   */
  virtual void Cancel() = 0;
};

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
    wxWindow* options = wxWindow::FindWindowByName("Options");
    assert(options && "Null Options window!");
    dialog.SetSize(wxSize(options->GetSize().x, options->GetSize().y * 8 / 10));
    auto rv = dialog.ShowModal();
    if (rv == wxID_OK) {
      ConnectionParams* cp = dialog.GetParamsFromControls();
      if (cp) {
        if (cp->GetValidPort()) {
          cp->b_IsSetup = false;  // Trigger new stream
          TheConnectionParams().push_back(cp);
        } else {
          wxString msg =
              _("Unable to create a connection as configured. "
                "Connected port or address was missing.");
          auto& noteman = NotificationManager::GetInstance();
          noteman.AddNotification(NotificationSeverity::kWarning,
                                  msg.ToStdString(), 60);
        }
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
              EventVar& on_conn_update)
      : wxGrid(parent, wxID_ANY),
        m_connections(connections),
        m_on_conn_delete(on_conn_update),
        m_last_tooltip_cell(100),
        m_icons(parent) {
    ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
    SetTable(new wxGridStringTable(), false);
    GetTable()->AppendCols(8);
    HideCol(7);
    static const std::array<wxString, 7> headers = {
        "", _("Protocol"), _("In/Out"), _("Data port"), _("Status"), "", ""};
    for (auto hdr = headers.begin(); hdr != headers.end(); hdr++)
      SetColLabelValue(static_cast<int>(hdr - headers.begin()), *hdr);
    if (IsWindows()) {
      SetLabelBackgroundColour(GetGlobalColor("DILG1"));
      SetLabelTextColour(GetGlobalColor("DILG3"));
    }
    HideRowLabels();
    SetColAttributes(parent);

    ReloadGrid(connections);
    DisableDragColSize();
    DisableDragRowSize();
    wxWindow::Show(GetNumberRows() > 0);

    GetGridWindow()->Bind(wxEVT_MOTION, [&](wxMouseEvent& ev) {
      OnMouseMove(ev);
      ev.Skip();
    });
    GetGridWindow()->Bind(wxEVT_MOUSEWHEEL,
                          [&](wxMouseEvent& ev) { OnWheel(ev); });
    Bind(wxEVT_GRID_LABEL_LEFT_CLICK,
         [&](wxGridEvent& ev) { HandleSort(ev.GetCol()); });
    Bind(wxEVT_GRID_CELL_LEFT_CLICK,
         [&](wxGridEvent& ev) { OnClickCell(ev.GetRow(), ev.GetCol()); });
    Bind(wxEVT_PAINT, [&](wxPaintEvent& ev) {
      SetColAttributes(static_cast<wxWindow*>(ev.GetEventObject()));
      ev.Skip();
    });
    conn_change_lstnr.Init(
        m_conn_states.evt_conn_status_change,
        [&](ObservedEvt&) { OnConnectionChange(m_connections); });
  }

  /** Mouse wheel: scroll the TopScroll window */
  void OnWheel(wxMouseEvent& ev) {
    auto w = static_cast<wxScrolledWindow*>(
        wxWindow::FindWindowByName(TopScrollWindowName));
    assert(w && "No TopScroll window found");
    int xpos;
    int ypos;
    w->GetViewStart(&xpos, &ypos);
    int x;
    int y;
    w->GetScrollPixelsPerUnit(&x, &y);
    // Not sure where the factor "4" comes from...
    int dir = ev.GetWheelRotation();
    w->Scroll(-1, ypos - dir / y / 4);
  }

  /** Reload grid using data from given list of connections. */
  void ReloadGrid(const std::vector<ConnectionParams*>& connections) {
    ClearGrid();
    m_renderer_status_vector.clear();

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
      SetCellRenderer(row, 5, new BitmapCellRenderer(m_icons.settings));
      m_tooltips[row][5] = _("Edit connection");
      SetCellRenderer(row, 6, new BitmapCellRenderer(m_icons.trashbin));
      m_tooltips[row][6] = _("Delete connection");
      SetCellValue(row, 7, (*it)->GetKey());

      auto stat_renderer = new BitmapCellRenderer(m_icons.filled_circle);
      stat_renderer->status = ConnState::Disabled;
      m_renderer_status_vector.push_back(stat_renderer);
      SetCellRenderer(row, 4, stat_renderer);
    }
    OnConnectionChange(m_connections);
    AutoSize();
  }

  wxSize GetGridMaxSize() const {
    return wxSize(GetCharWidth() * 120,
                  std::min(GetNumberRows() + 3, 10) * 2 * GetCharHeight());
  }

  wxSize GetGridMinSize() const {
    return wxSize(GetCharWidth() * 80,
                  std::min(GetNumberRows() + 3, 6) * 2 * GetCharHeight());
  }

  /** std::sort support: Compare two ConnectionParams w r t state. */
  class ConnStateCompare {
  public:
    ConnStateCompare(Connections* connections) : m_conns(connections) {}
    bool operator()(ConnectionParams* p1, ConnectionParams* p2) {
      int row1 = m_conns->FindConnectionIndex(p1);
      int row2 = m_conns->FindConnectionIndex(p2);
      if (row1 == -1 && row2 == -1) return false;
      if (row1 == -1) return false;
      if (row2 == -1) return true;
      int v1 = static_cast<int>(m_conns->GetCellValue(row1, 4)[0]);
      int v2 = static_cast<int>(m_conns->GetCellValue(row2, 4)[0]);
      return v1 < v2;
    }
    Connections* m_conns;
  };

private:
  /**
   *  Return pointer to parameters related to row.
   *  @return valid pointer if found, else nullptr.
   */
  ConnectionParams* FindRowConnection(int row) {
    auto found = find_if(m_connections.begin(), m_connections.end(),
                         [&](ConnectionParams* p) {
                           return GetCellValue(row, 7) == p->GetKey();
                         });
    return found != m_connections.end() ? *found : nullptr;
  }

  /**
   * Find index in m_connections for given pointer.
   * @return positive index if found, else -1;
   * */
  int FindConnectionIndex(ConnectionParams* cp) {
    using namespace std;
    auto key = cp->GetKey();
    auto found =
        find_if(m_connections.begin(), m_connections.end(),
                [key](ConnectionParams* cp) { return cp->GetKey() == key; });
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
    if (IsWindows()) {
      // Set all cells to global color scheme
      SetDefaultCellBackgroundColour(GetGlobalColor("DILG1"));
    }
    auto enable_attr = new wxGridCellAttr();
    enable_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    enable_attr->SetRenderer(new wxGridCellBoolRenderer());
    enable_attr->SetEditor(new wxGridCellBoolEditor());
    if (IsWindows()) enable_attr->SetBackgroundColour(GetGlobalColor("DILG1"));
    SetColAttr(0, enable_attr);

    auto protocol_attr = new wxGridCellAttr();
    protocol_attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
    protocol_attr->SetReadOnly(true);
    if (IsWindows())
      protocol_attr->SetBackgroundColour(GetGlobalColor("DILG1"));
    SetColAttr(1, protocol_attr);

    auto in_out_attr = new wxGridCellAttr();
    in_out_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    in_out_attr->SetReadOnly(true);
    if (IsWindows()) in_out_attr->SetBackgroundColour(GetGlobalColor("DILG1"));
    SetColAttr(2, in_out_attr);

    auto port_attr = new wxGridCellAttr();
    port_attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
    port_attr->SetReadOnly(true);
    if (IsWindows()) port_attr->SetBackgroundColour(GetGlobalColor("DILG1"));
    SetColAttr(3, port_attr);

    auto status_attr = new wxGridCellAttr();
    status_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    status_attr->SetReadOnly(true);
    status_attr->SetFont(parent->GetFont().Scale(1.3));
    if (IsWindows()) status_attr->SetBackgroundColour(GetGlobalColor("DILG1"));
    SetColAttr(4, status_attr);

    auto edit_attr = new wxGridCellAttr();
    edit_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    edit_attr->SetFont(parent->GetFont().Scale(1.3));
    edit_attr->SetReadOnly(true);
    if (IsWindows()) edit_attr->SetBackgroundColour(GetGlobalColor("DILG1"));
    SetColAttr(5, edit_attr);

    auto delete_attr = new wxGridCellAttr();
    delete_attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
    delete_attr->SetFont(parent->GetFont().Scale(1.3));
    delete_attr->SetTextColour(*wxRED);
    delete_attr->SetReadOnly(true);
    if (IsWindows()) delete_attr->SetBackgroundColour(GetGlobalColor("DILG1"));
    SetColAttr(6, delete_attr);
  }

  /** User mouse click on grid dispatcher. */
  void OnClickCell(int row, int col) {
    if (col == 0)
      HandleEnable(row);
    else if (col == 5) {
      HandleEdit(row);
    } else if (col == 6) {
      HandleDelete(row);
    }
  }

  /** Handle mouse movements i.e., the tooltips. */
  void OnMouseMove(wxMouseEvent& ev) {
    wxPoint pt = ev.GetPosition();
    int row = YToRow(pt.y);
    int col = XToCol(pt.x);
    if (col < 0 || col >= 7 || row < 0 || row >= GetNumberRows()) return;
    if (row * 7 + col == m_last_tooltip_cell) return;
    m_last_tooltip_cell = row * 7 + col;
    GetGridWindow()->SetToolTip(m_tooltips[row][col]);
  }

  /** Handle connections driver statistics status changes event. */
  void OnConnectionChange(const std::vector<ConnectionParams*>& connections) {
    bool refresh_needed = false;
    for (auto it = connections.begin(); it != connections.end(); it++) {
      ConnState state = m_conn_states.GetDriverState(
          (*it)->GetCommProtocol(), (*it)->GetStrippedDSPort());
      if (!(*it)->bEnabled) state = ConnState::Disabled;
      auto row = static_cast<int>(it - connections.begin());
      EnsureRows(row);
      if (m_renderer_status_vector.size() < (size_t)(row + 1)) continue;
      switch (state) {
        case ConnState::Disabled:
          if (m_renderer_status_vector[row]->status != ConnState::Disabled) {
            m_renderer_status_vector[row]->SetBitmap(m_icons.filled_circle);
            m_renderer_status_vector[row]->status = ConnState::Disabled;
            refresh_needed = true;
          }
          m_tooltips[row][4] = _("Disabled");
          break;
        case ConnState::NoStats:
          if (m_renderer_status_vector[row]->status != ConnState::NoStats) {
            m_renderer_status_vector[row]->SetBitmap(m_icons.open_circle);
            m_renderer_status_vector[row]->status = ConnState::NoStats;
            refresh_needed = true;
          }
          m_tooltips[row][4] = _("No driver statistics available");
          break;
        case ConnState::NoData:
          if (m_renderer_status_vector[row]->status != ConnState::NoData) {
            m_renderer_status_vector[row]->SetBitmap(m_icons.exclaim_mark);
            m_renderer_status_vector[row]->status = ConnState::NoData;
            refresh_needed = true;
          }
          m_tooltips[row][4] = _("No data flowing through connection");
          break;
        case ConnState::Unavailable:
          if (m_renderer_status_vector[row]->status != ConnState::Unavailable) {
            m_renderer_status_vector[row]->SetBitmap(m_icons.x_mult);
            m_renderer_status_vector[row]->status = ConnState::Unavailable;
            refresh_needed = true;
          }
          m_tooltips[row][4] = _("The device is unavailable");
          break;
        case ConnState::Ok:
          if (m_renderer_status_vector[row]->status != ConnState::Ok) {
            m_renderer_status_vector[row]->SetBitmap(m_icons.check_mark);
            m_renderer_status_vector[row]->status = ConnState::Ok;
            refresh_needed = true;
          }
          m_tooltips[row][4] = _("Data is flowing");
          break;
      }
    }
    if (refresh_needed) ForceRefresh();
  }

  /** HandleSort() helper: change column used to sort. */
  void SetSortingColumn(int col) {
    if (GetSortingColumn() != wxNOT_FOUND) {
      int old_col = GetSortingColumn();
      auto label = GetColLabelValue(old_col);
      if (label[0] == kUtfArrowDown[0])
        SetColLabelValue(old_col, label.substr(2));
    }
    auto label = GetColLabelValue(col);
    if (label[0] != kUtfArrowDown[0])
      SetColLabelValue(col, kUtfArrowDown + " " + label);
    wxGrid::SetSortingColumn(col);
    Fit();
  }

  /** Handle user click on column header */
  void HandleSort(int col) {
    if (col > 4) return;
    auto& params = TheConnectionParams();
    if (col < 4)
      std::sort(params.begin(), params.end(), ConnCompare(col));
    else  // col == 4
      std::sort(params.begin(), params.end(), ConnStateCompare(this));
    ReloadGrid(TheConnectionParams());
    SetSortingColumn(col);
  }

  /** Handle user click on the enable/disable checkbox. */
  void HandleEnable(int row) {
    ConnectionParams* cp = FindRowConnection(row);
    if (!cp) return;
    cp->bEnabled = !cp->bEnabled;
    cp->b_IsSetup = FALSE;  // trigger a rebuild/takedown of the connection
    SetCellValue(row, 0, cp->bEnabled ? "1" : "");
    if (cp->bEnabled)
      m_tooltips[row][0] = _("Enabled, click to disable");
    else
      m_tooltips[row][0] = _("Disabled, click to enable");
    DriverStats stats;
    stats.driver_iface = cp->GetStrippedDSPort();
    stats.driver_bus = cp->GetCommProtocol();
    m_conn_states.HandleDriverStats(stats);
    StopAndRemoveCommDriver(cp->GetStrippedDSPort(), cp->GetCommProtocol());
    if (cp->bEnabled) MakeCommDriver(cp);
    cp->b_IsSetup = true;
    if (!cp->bEnabled) SetCellValue(row, 4, kUtfFilledCircle);
  }

  /** Handle user click on Edit gear symbol. */
  void HandleEdit(int row) {
    ConnectionParams* cp = FindRowConnection(row);
    if (cp) {
      ConnectionEditDialog dialog(this);
      DimeControl(&dialog);
      dialog.SetPropsLabel(_("Edit Selected Connection"));
      dialog.PreloadControls(cp);
      wxWindow* options = wxWindow::FindWindowByName("Options");
      assert(options && "Null Options window!");
      dialog.SetSize(
          wxSize(options->GetSize().x, options->GetSize().y * 8 / 10));
      Show(GetNumberRows() > 0);

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
                                 wxOK | wxCANCEL);
      if (rcode != wxID_OK && rcode != wxID_YES) return;
      delete (*found)->m_optionsPanel;
      StopAndRemoveCommDriver((*found)->GetStrippedDSPort(),
                              (*found)->GetCommProtocol());
      TheConnectionParams().erase(found);
      if (GetNumberRows() > static_cast<int>(m_connections.size()))
        DeleteRows(GetNumberRows() - 1);
      m_on_conn_delete.Notify();
    }
  }

  ObsListener conn_change_lstnr;
  std::vector<std::vector<std::string>> m_tooltips;
  ConnStates m_conn_states;
  const std::vector<ConnectionParams*>& m_connections;
  EventVar& m_on_conn_delete;
  int m_last_tooltip_cell;
  StdIcons m_icons;
  std::vector<BitmapCellRenderer*> m_renderer_status_vector;
};

/** Indeed: the General  panel. */
class GeneralPanel : public wxPanel {
public:
  explicit GeneralPanel(wxWindow* parent, wxSize max_size)
      : wxPanel(parent, wxID_ANY) {
    auto sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("General"));
    SetSizer(sizer);
    auto flags = wxSizerFlags().Border();
    sizer->Add(new UploadOptionsChoice(this), flags);
    sizer->Add(new PrioritiesBtn(this), flags);
    SetMaxSize(max_size);
  }

private:
  /** Button invokes "Adjust communication priorities" GUI. */
  class PrioritiesBtn : public wxButton {
  public:
    PrioritiesBtn(wxWindow* parent)
        : wxButton(parent, wxID_ANY, _("Adjust Nav data priorities...")) {
      Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent&) {
        PriorityDlg dialog(this);
        dialog.ShowModal();
      });
    }
  };

  /** The select Generic, Garmin or Furuno upload options choice */
  class UploadOptionsChoice : public wxChoice, public ApplyCancel {
  public:
    explicit UploadOptionsChoice(wxWindow* parent) : wxChoice() {
      wxArrayString wx_choices;
      for (auto& c : choices) wx_choices.Add(c);
      Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wx_choices);
      Cancel();
    }

    void Cancel() {
      if (g_bGarminHostUpload)
        SetSelection(1);
      else if (g_GPS_Ident == "FurunoGP3X")
        SetSelection(2);
      else
        SetSelection(0);
    }

    void Apply() {
      switch (GetSelection()) {
        case 0:
          g_bGarminHostUpload = false;
          g_GPS_Ident = "Generic";
          break;
        case 1:
          g_bGarminHostUpload = true;
          g_GPS_Ident = "Generic";
          break;
        case 2:
          g_bGarminHostUpload = false;
          g_GPS_Ident = "FurunoGP3X";
          break;
      }
    }

    const std::array<wxString, 3> choices = {
        _("Use generic Nmea 0183 format for uploads"),
        _("Use Garmin GRMN (Host) mode for uploads"),
        _("Format uploads for Furuno GP4X")};
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

/** Indeed: The "Advanced" panel. */
class AdvancedPanel : public wxPanel {
public:
  explicit AdvancedPanel(wxWindow* parent, wxSize max_size)
      : wxPanel(parent, wxID_ANY) {
    auto sizer = new wxStaticBoxSizer(wxVERTICAL, this, "");
    sizer->Add(new BearingsCheckbox(this), wxSizerFlags().Expand());
    sizer->Add(new NmeaFilterRow(this), wxSizerFlags().Expand());
    sizer->Add(new TalkerIdRow(this), wxSizerFlags().Expand());
    sizer->Add(new NetmaskRow(this), wxSizerFlags().Expand());
    SetSizer(sizer);
    SetMaxSize(max_size);
  }

private:
  /** "Use magnetic bearing..." checkbox bound to g_bMagneticAPB. */
  class BearingsCheckbox : public wxCheckBox, public ApplyCancel {
  public:
    BearingsCheckbox(wxWindow* parent)
        : wxCheckBox(parent, wxID_ANY,
                     _("Use magnetic bearing in output sentence APB")) {
      SetValue(g_bMagneticAPB);
      wxCheckBox::SetValue(g_bMagneticAPB);
    }

    void Apply() override { g_bMagneticAPB = GetValue(); }
    void Cancel() override { SetValue(g_bMagneticAPB); }
  };

  /** NMEA filter setup bound to g_bfilter_cogsog and g_COGFilterSec. */
  class NmeaFilterRow : public wxPanel, public ApplyCancel {
    wxCheckBox* checkbox;
    wxTextCtrl* filter_period;

  public:
    NmeaFilterRow(wxWindow* parent) : wxPanel(parent) {
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      checkbox = new wxCheckBox(
          this, wxID_ANY,
          _("Filter NMEA course and speed data. Filter period: "));
      checkbox->SetValue(g_bfilter_cogsog);
      hbox->Add(checkbox, wxSizerFlags().Align(wxALIGN_CENTRE));
      filter_period =
          new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition,
                         wxSize(50, 3 * wxWindow::GetCharWidth()), 0);
      filter_period->SetValue(std::to_string(g_COGFilterSec));
      hbox->Add(filter_period, wxSizerFlags().Border());
      SetSizer(hbox);
      Cancel();
    }

    void Apply() override {
      std::stringstream ss;
      ss << filter_period->GetValue();
      ss >> g_COGFilterSec;
      g_bfilter_cogsog = checkbox->GetValue();
    }

    void Cancel() override {
      std::stringstream ss;
      ss << g_COGFilterSec;
      filter_period->SetValue(ss.str());
      checkbox->SetValue(g_bfilter_cogsog);
    }
  };

  /** NMEA Talker ID configuration, bound to g_TalkerIdText. */
  class TalkerIdRow : public wxPanel, public ApplyCancel {
    wxTextCtrl* text_ctrl;

  public:
    TalkerIdRow(wxWindow* parent) : wxPanel(parent) {
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      hbox->Add(new wxStaticText(this, wxID_ANY, _("NMEA 0183 Talker Id: ")),
                wxSizerFlags().Align(wxALIGN_CENTRE_VERTICAL).Border());
      text_ctrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition,
                                 wxSize(50, 3 * wxWindow::GetCharWidth()));
      text_ctrl->SetToolTip(
          _("Enter a two-letter talker ID to override the default ID in NMEA "
            "sentences generated by OpenCPN (e.g., GP, HC, EC). This affects "
            "only sentences created by OpenCPN, not those forwarded from other "
            "devices."));
      text_ctrl->SetValue(g_TalkerIdText);
      hbox->Add(text_ctrl, wxSizerFlags().Border());
      SetSizer(hbox);
      Cancel();
    }

    void Apply() override { g_TalkerIdText = text_ctrl->GetValue(); }
    void Cancel() override { text_ctrl->SetValue(g_TalkerIdText); }
  };

  /** Global netmask configuration bound to g_netmask_bits. */
  class NetmaskRow : public wxPanel, public ApplyCancel {
  public:
    NetmaskRow(wxWindow* parent)
        : wxPanel(parent),
          m_spin_ctrl(new wxSpinCtrl(this, wxID_ANY)),
          m_text(new wxStaticText(this, wxID_ANY, "")) {
      m_spin_ctrl->SetRange(8, 32);
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      auto flags = wxSizerFlags().Align(wxALIGN_CENTRE_VERTICAL).Border();
      hbox->Add(new wxStaticText(this, wxID_ANY, _("Netmask: ")), flags);
      hbox->Add(m_text, flags);
      hbox->Add(new wxStaticText(this, wxID_ANY, _("length (bits): ")), flags);
      hbox->Add(m_spin_ctrl, flags);
      SetSizer(hbox);
      Cancel();

      Bind(wxEVT_SPINCTRL, [&](wxSpinEvent& ev) {
        m_text->SetLabel(BitsToDottedMask(m_spin_ctrl->GetValue()));
        Layout();
      });
    }

    void Apply() override { g_netmask_bits = m_spin_ctrl->GetValue(); }

    void Cancel() override {
      m_spin_ctrl->SetValue(g_netmask_bits);
      m_text->SetLabel(BitsToDottedMask(m_spin_ctrl->GetValue()));
      Layout();
    }

  private:
    wxSpinCtrl* m_spin_ctrl;
    wxStaticText* m_text;

    std::string BitsToDottedMask(unsigned bits) {
      uint32_t mask = 0xffffffff << (32 - bits);
      std::stringstream ss;
      ss << ((mask & 0xff000000) >> 24) << ".";
      ss << ((mask & 0x00ff0000) >> 16) << ".";
      ss << ((mask & 0x0000ff00) >> 8) << ".";
      ss << (mask & 0x000000ff);
      return ss.str();
    }
  };
};

/** Top panel: connections grid, "Add new connection", general options. */
class TopPanel : public wxPanel {
public:
  TopPanel(wxWindow* parent, const std::vector<ConnectionParams*>& connections,
           EventVar& evt_add_connection)
      : wxPanel(parent, wxID_ANY),
        m_connections(connections),
        m_evt_add_connection(evt_add_connection) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto conn_grid = new Connections(this, m_connections, m_evt_add_connection);
    wxSize panel_max_size(conn_grid->GetSize().x, -1);
    vbox->Add(conn_grid, wxSizerFlags().Border());
    vbox->Add(new AddConnectionButton(this, m_evt_add_connection),
              wxSizerFlags().Border());
    vbox->Add(0, wxWindow::GetCharHeight());  // Expanding spacer
    auto panel_flags =
        wxSizerFlags().Border(wxLEFT | wxDOWN | wxRIGHT).Expand();
    vbox->Add(new GeneralPanel(this, panel_max_size), panel_flags);

    auto advanced_panel = new AdvancedPanel(this, panel_max_size);
    auto on_toggle = [&, advanced_panel, vbox](bool show) {
      advanced_panel->Show(show);
      vbox->SetSizeHints(this);
      vbox->Fit(this);
    };
    vbox->Add(new ShowAdvanced(this, on_toggle), panel_flags);
    vbox->Add(advanced_panel, panel_flags.ReserveSpaceEvenIfHidden());

    SetSizer(vbox);
    vbox->SetSizeHints(this);
    vbox->Fit(this);
    wxWindow::Fit();
    Show();

    auto on_evt_update_connections = [&, conn_grid](ObservedEvt&) {
      conn_grid->ReloadGrid(TheConnectionParams());
      conn_grid->Show(conn_grid->GetNumberRows() > 0);
      Layout();
    };
    m_add_connection_lstnr.Init(m_evt_add_connection,
                                on_evt_update_connections);
  }

private:
  const std::vector<ConnectionParams*>& m_connections;
  EventVar& m_evt_add_connection;
  ObsListener m_add_connection_lstnr;
};

/** Top scroll window, adds scrollbars to TopPanel. */
class TopScroll : public wxScrolledWindow {
public:
  TopScroll(wxWindow* parent, const std::vector<ConnectionParams*>& connections,
            EventVar& evt_add_connection)
      : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                         wxVSCROLL | wxALWAYS_SHOW_SB, TopScrollWindowName) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(new TopPanel(this, connections, evt_add_connection),
              wxSizerFlags(1).Expand());
    SetSizer(vbox);
    SetScrollRate(0, 10);
  }
};

/** Main window: Panel with a single TopScroll child. */
ConnectionsDlg::ConnectionsDlg(
    wxWindow* parent, const std::vector<ConnectionParams*>& connections)
    : wxPanel(parent, wxID_ANY), m_connections(connections) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(new TopScroll(this, connections, m_evt_add_connection),
            wxSizerFlags(1).Expand());
  SetSizer(vbox);
  wxWindow::Fit();
  Show();
};

void ConnectionsDlg::OnResize(const wxSize& size) {
  auto w = wxWindow::FindWindowByName(TopScrollWindowName);
  if (!w) return;
  w->SetMinSize(size);
  Fit();
}

void ConnectionsDlg::DoApply(wxWindow* root) {
  for (wxWindow* child : root->GetChildren()) {
    auto widget = dynamic_cast<ApplyCancel*>(child);
    if (widget) widget->Apply();
    DoApply(child);
  }
}

void ConnectionsDlg::DoCancel(wxWindow* root) {
  for (wxWindow* child : root->GetChildren()) {
    auto widget = dynamic_cast<ApplyCancel*>(child);
    if (widget) widget->Cancel();
    DoCancel(child);
  }
}

void ConnectionsDlg::ApplySettings() { DoApply(this); }

void ConnectionsDlg::CancelSettings() { DoCancel(this); }
