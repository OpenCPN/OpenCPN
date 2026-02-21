/***************************************************************************
 *   Copyright (C) 2025 Alec Leamas                                        *
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
 * Implement connections_dlg.h --  %Options | %Connections GUI
 */

#include <array>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "gl_headers.h"  // Must be included before anything using GL stuff

#include <wx/arrstr.h>
#include <wx/bitmap.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#include "model/base_platform.h"
#include "model/comm_drv_factory.h"
#include "model/comm_util.h"
#include "model/config_vars.h"
#include "model/conn_params.h"
#include "model/conn_states.h"
#include "model/gui_events.h"
#include "model/notification_manager.h"

#include "connections_dlg.h"

#include "color_handler.h"
#include "color_types.h"
#include "connection_edit.h"
#include "conn_params_panel.h"
#include "gui_lib.h"
#include "navutil.h"
#include "ocpn_platform.h"
#include "options.h"
#include "priority_gui.h"
#include "std_filesystem.h"
#include "model/svg_utils.h"

static wxString UtfArrowDown() { return wxString::FromUTF8(u8"\u25bc"); }
static wxString UtfArrowRight() { return wxString::FromUTF8(u8"\u25ba"); }
static wxString UtfFilledCircle() { return wxString::FromUTF8(u8"\u2b24"); }

static const auto TopScrollWindowName = "TopScroll";

static const char* kInfoHeader = _("OpenCPN help").c_str();
static const char* kInfo = _(R"---(
Normally OpenCPN sends the RMB (distance and heading to waypoint
etc) and RMC (position, heading etc.) sentences when there is an
active waypoint. The talker ID is by default EC.

In certain cases these messages (ECRMB and ECRMC) are required
also when there is no active waypoint. This option enables this.

Obviously, this option carries risks for vessels which have an
autopilot which is not engaged manually -- the "fake" RMB messages
could possibly initiate the autopilot in dangerous ways.

Devices needing this includes the NASA Clipper GPS Repeater. In this
case the output must also be filtered so that ECRMB and ECRMC are
the only transmitted messages.
)---")
                               .c_str();

static bool IsWindows() {
  return wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_WINDOWS;
}

static bool IsAndroid() {
#ifdef ANDROID
  return true;
#else
  return false;
#endif
}

static std::string BitsToDottedMask(const unsigned bits) {
  const uint32_t mask = 0xffffffff << (32 - bits);
  std::stringstream ss;
  ss << ((mask & 0xff000000) >> 24) << ".";
  ss << ((mask & 0x00ff0000) >> 16) << ".";
  ss << ((mask & 0x0000ff00) >> 8) << ".";
  ss << (mask & 0x000000ff);
  return ss.str();
}

/** Standard icons bitmaps: settings gear, trash bin, etc. */
class StdIcons {
private:
  const unsigned m_size;
  const fs::path m_svg_dir;
  ColorScheme m_cs;

  /** Return platform dependent icon size. */
  static unsigned GetSize(const wxWindow* parent) {
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
    return static_cast<unsigned>(size);
  }

  [[nodiscard]] wxBitmap LoadIcon(const std::string& filename) const {
    fs::path path = m_svg_dir / filename;
    return LoadSVG(path.string(), m_size, m_size);
  }

  [[nodiscard]] wxBitmap IconApplyColorScheme(const wxBitmap& proto) const {
    if (!proto.IsOk()) return wxNullBitmap;
    if ((m_cs != GLOBAL_COLOR_SCHEME_DAY) &&
        (m_cs != GLOBAL_COLOR_SCHEME_RGB)) {
      // Assume the bitmap is monochrome, so simply invert the colors.
      const wxImage image = proto.ConvertToImage();
      unsigned char* data = image.GetData();
      unsigned char* p_idata = data;
      for (int i = 0; i < image.GetSize().y; i++) {
        for (int j = 0; j < image.GetSize().x; j++) {
          unsigned char v = *p_idata;
          v = 255 - v;
          *p_idata++ = v;
          v = *p_idata;
          v = 255 - v;
          *p_idata++ = v;
          v = *p_idata;
          v = 255 - v;
          *p_idata++ = v;
        }
      }
      return {image};
    }
    return proto;
  }

public:
  explicit StdIcons(const wxWindow* parent)
      : m_size(GetSize(parent)),
        m_svg_dir(fs::path(g_Platform->GetSharedDataDir().ToStdString()) /
                  "uidata" / "MUI_flat"),
        m_cs(GLOBAL_COLOR_SCHEME_RGB),
        trash_bin_proto(LoadIcon("trash_bin.svg")),
        settings_proto(LoadIcon("setting_gear.svg")),
        filled_circle_proto(LoadIcon("circle-on.svg")),
        open_circle_proto(LoadIcon("circle-off.svg")),
        exclaim_mark_proto(LoadIcon("exclaim_mark.svg")),
        x_mult_proto(LoadIcon("X_mult.svg")),
        check_mark_proto(LoadIcon("check_mark.svg")) {
    trash_bin = trash_bin_proto;
    settings = settings_proto;
    filled_circle = filled_circle_proto;
    open_circle = open_circle_proto;
    exclaim_mark = exclaim_mark_proto;
    x_mult = x_mult_proto;
    check_mark = check_mark_proto;
  }

  void SetColorScheme(const ColorScheme cs) {
    if (m_cs != cs) {
      m_cs = cs;
      trash_bin = IconApplyColorScheme(trash_bin_proto);
      settings = IconApplyColorScheme(settings_proto);
      filled_circle = IconApplyColorScheme(filled_circle_proto);
      open_circle = IconApplyColorScheme(open_circle_proto);
      exclaim_mark = IconApplyColorScheme(exclaim_mark_proto);
      x_mult = IconApplyColorScheme(x_mult_proto);
      check_mark = IconApplyColorScheme(check_mark_proto);
    }
  }

  const wxBitmap trash_bin_proto;
  const wxBitmap settings_proto;
  const wxBitmap filled_circle_proto;
  const wxBitmap open_circle_proto;
  const wxBitmap exclaim_mark_proto;
  const wxBitmap x_mult_proto;
  const wxBitmap check_mark_proto;

  wxBitmap trash_bin;
  wxBitmap settings;
  wxBitmap filled_circle;
  wxBitmap open_circle;
  wxBitmap exclaim_mark;
  wxBitmap x_mult;
  wxBitmap check_mark;
};

/** Custom renderer class for rendering bitmap in a grid cell */
class BitmapCellRenderer : public wxGridCellRenderer {
public:
  BitmapCellRenderer(const wxBitmap& bitmap, ColorScheme cs)
      : status(ConnState::Disabled), m_bitmap(bitmap), m_cs(cs) {}

  // Update the bitmap dynamically
  void SetBitmap(const wxBitmap& bitmap) { m_bitmap = bitmap; }

  void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect,
            int row, int col, bool isSelected) override {
    dc.SetBrush(wxBrush(GetGlobalColor("DILG1")));
    if ((m_cs != GLOBAL_COLOR_SCHEME_DAY) && m_cs != GLOBAL_COLOR_SCHEME_RGB)
      dc.SetBrush(wxBrush(GetDialogColor(DLG_BACKGROUND)));
    if (IsWindows()) dc.SetBrush(wxBrush(GetGlobalColor("DILG1")));
    dc.DrawRectangle(rect);

    // Draw the bitmap centered in the cell
    dc.DrawBitmap(m_bitmap, rect.x + (rect.width - m_bitmap.GetWidth()) / 2,
                  rect.y + (rect.height - m_bitmap.GetHeight()) / 2, true);
  }

  wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row,
                     int col) override {
    // Return the size of the bitmap as the best size for the cell
    return {m_bitmap.GetWidth(), m_bitmap.GetHeight()};
  }

  [[nodiscard]] BitmapCellRenderer* Clone() const override {
    return new BitmapCellRenderer(m_bitmap, m_cs);
  }
  ConnState status;

private:
  wxBitmap m_bitmap;
  ColorScheme m_cs;
};

/**
 * Custom renderer class for rendering ENABLE in a grid cell.
 * Allows setting of rendered Icon size
 */
class BitmapEnableCellRenderer : public wxGridCellBoolRenderer {
public:
  BitmapEnableCellRenderer(int _size, ColorScheme cs) : size(_size), m_cs(cs) {}

  [[nodiscard]] BitmapEnableCellRenderer* Clone() const override {
    return new BitmapEnableCellRenderer(size, m_cs);
  }

  wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row,
                     int col) override {
    return {size, size};
  }

private:
  int size;
  ColorScheme m_cs;
};

/** std::sort support: Compare two ConnectionParams w r t given column */
class ConnCompare {
public:
  explicit ConnCompare(int col) : m_col(col) {}

  bool operator()(const ConnectionParams* p1,
                  const ConnectionParams* p2) const {
    switch (m_col) {
      case 0:
        return static_cast<int>(p1->bEnabled) > static_cast<int>(p2->bEnabled);
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
  /**
   * Destroy the Apply Cancel object.
   */
  virtual ~ApplyCancel() = default;

  /** Make values set by user actually being used. */
  virtual void Apply() = 0;

  /**
   * Restore values modified by user to their pristine state, often in a global.
   */
  virtual void Cancel() = 0;
};

/** The "Add new connection" button */
class AddConnectionButton final : public wxButton {
public:
  AddConnectionButton(wxWindow* parent, EventVar& evt_add_connection,
                      const std::function<void(ConnectionParams* p,
                                               bool editing)>& _start_edit_conn)
      : wxButton(parent, wxID_ANY, _("Add new connection...")),
        m_evt_add_connection(evt_add_connection),
        m_start_edit_conn(_start_edit_conn) {
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,
         [&](wxCommandEvent&) { OnAddConnection(); });
  }

private:
  void OnAddConnection() const { m_start_edit_conn(nullptr, false); }

  EventVar& m_evt_add_connection;
  std::function<void(ConnectionParams* p, bool editing)> m_start_edit_conn;
};

/** Grid with existing connections: type, port, status, etc. */
class Connections final : public wxGrid {
public:
  Connections(wxWindow* parent,
              const std::vector<ConnectionParams*>& connections,
              EventVar& on_conn_update,
              const std::function<void(ConnectionParams* p, bool editing)>&
                  on_edit_conn)
      : wxGrid(parent, wxID_ANY),
        m_connections(connections),
        m_header_column_widths({0, 0, 0, 0, 0, 0, 0}),
        m_last_tooltip_cell(100),
        m_cs(GLOBAL_COLOR_SCHEME_DAY),
        m_on_conn_delete(on_conn_update),
        m_icons(parent),
        m_on_edit_conn(on_edit_conn) {
    ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
    SetTable(new wxGridStringTable(), true);
    GetTable()->AppendCols(8);
    HideCol(7);
    if (IsAndroid()) {
      SetDefaultRowSize(wxWindow::GetCharHeight() * 2);
      SetColLabelSize(wxWindow::GetCharHeight() * 2);
    }
    static const std::array<wxString, 7> headers = {
        "", _("Protocol") + "  ", _("In/Out"), _("Data port"), _("Status"), "",
        ""};
    int ic = 0;
    for (auto hdr = headers.begin(); hdr != headers.end(); hdr++, ic++) {
      SetColLabelValue(static_cast<int>(hdr - headers.begin()), *hdr);
      unsigned col_width = hdr->Length() * wxWindow::GetCharWidth();
      col_width = wxMax(col_width, 6 * wxWindow::GetCharWidth());
      m_header_column_widths[ic] = static_cast<int>(col_width);
      SetColSize(ic, static_cast<int>(col_width));
    }

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
                          [&](const wxMouseEvent& ev) { OnWheel(ev); });
    // wxGridEvent.GetCol() and GetRow() are not const until wxWidgets 3.2
    Bind(wxEVT_GRID_LABEL_LEFT_CLICK,
         [&](wxGridEvent& ev) { HandleSort(ev.GetCol()); });
    Bind(wxEVT_GRID_CELL_LEFT_CLICK,
         [&](wxGridEvent& ev) { OnClickCell(ev.GetRow(), ev.GetCol()); });
    Bind(wxEVT_PAINT, [&](wxPaintEvent& ev) {
      SetColAttributes(dynamic_cast<const wxWindow*>(ev.GetEventObject()));
      ev.Skip();
    });
    conn_change_lstnr.Init(
        m_conn_states.evt_conn_status_change,
        [&](ObservedEvt&) { OnConnectionChange(m_connections); });
  }
  void SetColorScheme(const ColorScheme cs) {
    m_cs = cs;
    m_icons.SetColorScheme(cs);
    ReloadGrid(m_connections);
  }

  wxSize GetEstimatedSize() const {
    unsigned rs = 0;
    for (auto s : m_header_column_widths) rs += s;
    return {static_cast<int>(rs), -1};
  }

  /** Mouse wheel: scroll the TopScroll window */
  static void OnWheel(const wxMouseEvent& ev) {
    auto w =
        dynamic_cast<wxScrolledWindow*>(FindWindowByName(TopScrollWindowName));
    assert(w && "No TopScroll window found");
    int xpos;
    int ypos;
    w->GetViewStart(&xpos, &ypos);
    int x;
    int y;
    w->GetScrollPixelsPerUnit(&x, &y);
    // Not sure where the factor "4" comes from...
    const int dir = ev.GetWheelRotation();
    w->Scroll(-1, ypos - dir / y / 4);
  }

  /** Reload grid using data from given list of connections. */
  void ReloadGrid(const std::vector<ConnectionParams*>& connections) {
    ClearGrid();
    m_renderer_status_vector.clear();

    for (auto it = connections.begin(); it != connections.end(); ++it) {
      const auto row = static_cast<int>(it - connections.begin());
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
      SetCellRenderer(row, 5, new BitmapCellRenderer(m_icons.settings, m_cs));
      m_tooltips[row][5] = _("Edit connection");
      SetCellRenderer(row, 6, new BitmapCellRenderer(m_icons.trash_bin, m_cs));
      m_tooltips[row][6] = _("Delete connection");
      SetCellValue(row, 7, (*it)->GetKey());

      auto stat_renderer = new BitmapCellRenderer(m_icons.filled_circle, m_cs);
      stat_renderer->status = ConnState::Disabled;
      m_renderer_status_vector.push_back(stat_renderer);
      SetCellRenderer(row, 4, stat_renderer);
      if (IsAndroid()) {
        wxString sp(protocol);
        unsigned size = sp.Length() * wxWindow::GetCharWidth();
        m_header_column_widths[1] = std::max(m_header_column_widths[1], size);
        size = (*it)->GetIOTypeValueStr().Length() * wxWindow::GetCharWidth();
        m_header_column_widths[2] = std::max(m_header_column_widths[2], size);
        sp = wxString((*it)->GetStrippedDSPort());
        size = sp.Length() * wxWindow::GetCharWidth();
        m_header_column_widths[3] = std::max(m_header_column_widths[3], size);
      }
    }
    OnConnectionChange(m_connections);

    if (IsAndroid()) {
      int ic = 0;
      for (auto val : m_header_column_widths) {
        SetColSize(ic, static_cast<int>(val));
        ic++;
      }
    } else
      AutoSize();
  }

  /** std::sort support: Compare two ConnectionParams w r t state. */
  class ConnStateCompare {
  public:
    explicit ConnStateCompare(Connections* connections)
        : m_conns(connections) {}
    bool operator()(const ConnectionParams* p1,
                    const ConnectionParams* p2) const {
      int row1 = m_conns->FindConnectionIndex(p1);
      int row2 = m_conns->FindConnectionIndex(p2);
      if (row1 == -1 && row2 == -1) return false;
      if (row1 == -1) return false;
      if (row2 == -1) return true;
      const int v1 = m_conns->GetCellValue(row1, 4)[0];
      const int v2 = m_conns->GetCellValue(row2, 4)[0];
      return v1 < v2;
    }
    Connections* m_conns;
  };

  /**
   * Find index in m_connections for given pointer.
   * @return positive index if found, else -1;
   * */
  int FindConnectionIndex(const ConnectionParams* cp) const {
    using namespace std;
    auto key = cp->GetKey();
    auto found = find_if(
        m_connections.begin(), m_connections.end(),
        [key](const ConnectionParams* cp) { return cp->GetKey() == key; });
    if (found == m_connections.end()) return -1;
    return static_cast<int>(found - m_connections.begin());
  }

private:
  /**
   *  Return pointer to parameters related to row.
   *  @return valid pointer if found, else nullptr.
   */
  ConnectionParams* FindRowConnection(const int row) const {
    auto found = find_if(m_connections.begin(), m_connections.end(),
                         [&](const ConnectionParams* p) {
                           return GetCellValue(row, 7) == p->GetKey();
                         });
    return found != m_connections.end() ? *found : nullptr;
  }

  /**
   *  Ensure that grid and tooltips has allocated at least given amount of
   *  rows.
   */
  void EnsureRows(size_t rows) {
    for (unsigned i = m_tooltips.size(); i <= rows; i++)
      m_tooltips.emplace_back(std::vector<std::string>(7));
    for (unsigned i = GetNumberRows(); i <= rows; i++) AppendRows(1, false);
  }

  /** Set up column attributes: alignment, font size, read-only, etc. */
  void SetColAttributes(const wxWindow* parent) {
    if (IsWindows()) {
      // Set all cells to global color scheme
      SetDefaultCellBackgroundColour(GetGlobalColor("DILG1"));
    }
    auto enable_attr = new wxGridCellAttr();
    enable_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);

    if (IsAndroid())
      enable_attr->SetRenderer(
          new BitmapEnableCellRenderer(wxWindow::GetCharWidth() * 3 / 2, m_cs));
    else
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
  void OnMouseMove(const wxMouseEvent& ev) {
    const wxPoint pt = ev.GetPosition();
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
    for (auto it = connections.begin(); it != connections.end(); ++it) {
      ConnState state = m_conn_states.GetDriverState(
          (*it)->GetCommProtocol(), (*it)->GetStrippedDSPort());
      if (!(*it)->bEnabled) state = ConnState::Disabled;
      auto row = static_cast<int>(it - connections.begin());
      EnsureRows(row);
      if (static_cast<int>(m_renderer_status_vector.size()) < row + 1) continue;
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
      if (label[0] == UtfArrowDown()[0])
        SetColLabelValue(old_col, label.substr(2));
    }
    auto label = GetColLabelValue(col);
    if (label[0] != UtfArrowDown()[0])
      SetColLabelValue(col, UtfArrowDown() + " " + label);
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
    if (!cp->bEnabled) {
      SetCellValue(row, 4, UtfFilledCircle());
      // ForceRefresh() apparently broken, see #4648
      ReloadGrid(TheConnectionParams());
    }
  }

  /** Handle user click on Edit gear symbol. */
  void HandleEdit(int row) {
    if (ConnectionParams* cp = FindRowConnection(row)) {
      Show(GetNumberRows() > 0);
      m_on_edit_conn(cp, true);
    }
  }

  /** Handle user click on Delete trash bin symbol. */
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
  std::array<unsigned, 7> m_header_column_widths;
  int m_last_tooltip_cell;
  ColorScheme m_cs;
  EventVar& m_on_conn_delete;
  StdIcons m_icons;
  std::vector<BitmapCellRenderer*> m_renderer_status_vector;
  std::function<void(ConnectionParams* p, bool editing)> m_on_edit_conn;
};

/** Indeed: the General  panel. */
class GeneralPanel : public wxPanel {
public:
  explicit GeneralPanel(wxWindow* parent, wxSize max_size)
      : wxPanel(parent, wxID_ANY) {
    auto sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("General"));
    SetSizer(sizer);
    auto flags = wxSizerFlags().Border();
    m_upload_options = new UploadOptionsChoice(this);
    sizer->Add(m_upload_options, flags);
    sizer->Add(new PrioritiesBtn(this), flags);
    wxWindow::SetMaxSize(max_size);
  }
  void SetColorScheme(ColorScheme) {
    DimeControl(m_upload_options);
    Refresh();
  }

private:
  /** Button invokes "Adjust communication priorities" GUI. */
  class PrioritiesBtn final : public wxButton {
  public:
    explicit PrioritiesBtn(wxWindow* parent)
        : wxButton(parent, wxID_ANY, _("Adjust Nav data priorities...")) {
      Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent&) {
        PriorityDlg dialog(this->GetParent());
        dialog.ShowModal();
      });
    }
  };

  /** The select Generic, Garmin or Furuno upload options choice */
  class UploadOptionsChoice final : public wxRadioBox, public ApplyCancel {
  public:
    explicit UploadOptionsChoice(wxWindow* parent) : wxRadioBox() {
      wxArrayString wx_choices;
      for (auto& c : choices) wx_choices.Add(c);
      Create(parent, wxID_ANY, _("Upload Format"), wxDefaultPosition,
             wxDefaultSize, wx_choices, 0, wxRA_SPECIFY_ROWS);
      DimeControl(this);
      UploadOptionsChoice::Cancel();
    }

    void Cancel() override {
      if (g_bGarminHostUpload)
        SetSelection(1);
      else if (g_GPS_Ident == "FurunoGP3X")
        SetSelection(2);
      else
        SetSelection(0);
    }

    void Apply() override {
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
        default:
          assert(false && "Invalid upload case option");
      }
    }

    const std::array<wxString, 3> choices = {
        _("Generic NMEA 0183"), _("Garmin Host mode"), _("Furuno GP4X")};
  };

  UploadOptionsChoice* m_upload_options;
};

/** The "Show advanced" text + right/down triangle and handler. */
class ShowAdvanced : public wxStaticText {
public:
  ShowAdvanced(wxWindow* parent, std::function<void(bool)> on_toggle)
      : wxStaticText(parent, wxID_ANY, ""),
        m_on_toggle(std::move(on_toggle)),
        m_show_advanced(true) {
    Toggle();
    Bind(wxEVT_LEFT_DOWN, [&](const wxMouseEvent& ev) { Toggle(); });
  }

private:
  wxString m_label = _("Advanced ");
  std::function<void(bool)> m_on_toggle;
  bool m_show_advanced;

  void Toggle() {
    m_show_advanced = !m_show_advanced;
    m_label = _("Advanced ");
    m_label += m_show_advanced ? UtfArrowDown() : UtfArrowRight();
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
    sizer->Add(new ExtraRmbRmcLine(this), wxSizerFlags().Expand());
    sizer->Add(new NmeaFilterRow(this), wxSizerFlags().Expand());
    sizer->Add(new TalkerIdRow(this), wxSizerFlags().Expand());
    sizer->Add(new NetmaskRow(this), wxSizerFlags().Expand());
    SetSizer(sizer);
    wxWindow::SetMaxSize(max_size);
  }

private:
  /** "Use magnetic bearing..." checkbox bound to g_bMagneticAPB. */
  class BearingsCheckbox : public wxCheckBox, public ApplyCancel {
  public:
    explicit BearingsCheckbox(wxWindow* parent)
        : wxCheckBox(parent, wxID_ANY,
                     _("Use magnetic bearing in output sentence APB")) {
      wxCheckBox::SetValue(g_bMagneticAPB);
    }

    void Apply() override { g_bMagneticAPB = GetValue(); }
    void Cancel() override { SetValue(g_bMagneticAPB); }
  };

  class ExtraRmbRmcLine : public wxPanel {
    class RmbRmcCheckbox : public wxCheckBox, public ApplyCancel {
    public:
      explicit RmbRmcCheckbox(wxWindow* parent)
          : wxCheckBox(parent, wxID_ANY,
                       _("Always send RMB and RMC NMEA0183 data")) {
        wxCheckBox::SetValue(g_always_send_rmb_rmc);
      }

      void Apply() override { g_always_send_rmb_rmc = GetValue(); }
      void Cancel() override { SetValue(g_always_send_rmb_rmc); }
    };

  public:
    explicit ExtraRmbRmcLine(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      hbox->Add(new RmbRmcCheckbox(this), wxSizerFlags().Expand());
      hbox->Add(1, 1, 1, wxEXPAND);
      hbox->Add(new InfoButton(this, g_btouch, kInfoHeader, kInfo),
                wxSizerFlags().Border());
      SetSizer(hbox);
      wxWindow::Layout();
      wxWindow::Show();
    }
  };
  class ExtraRmbRmcCheckbox final : public wxCheckBox, public ApplyCancel {
  public:
    explicit ExtraRmbRmcCheckbox(wxWindow* parent)
        : wxCheckBox(parent, wxID_ANY,
                     _("Always send RMB and RMC NMEA0183 data")) {
      wxCheckBox::SetValue(g_always_send_rmb_rmc);
    }

    void Apply() override { g_always_send_rmb_rmc = GetValue(); }
    void Cancel() override { SetValue(g_always_send_rmb_rmc); }
  };

  /** NMEA filter setup bound to g_bfilter_cogsog and g_COGFilterSec. */
  class NmeaFilterRow : public wxPanel, public ApplyCancel {
    wxCheckBox* checkbox;
    wxTextCtrl* filter_period;

  public:
    explicit NmeaFilterRow(wxWindow* parent) : wxPanel(parent) {
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
      NmeaFilterRow::Cancel();
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
    explicit TalkerIdRow(wxWindow* parent) : wxPanel(parent) {
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      hbox->Add(new wxStaticText(this, wxID_ANY, _("NMEA 0183 Talker Id: ")),
                wxSizerFlags().Align(wxALIGN_CENTRE_VERTICAL).Border());
      text_ctrl = new wxTextCtrl(
          this, wxID_ANY, "", wxDefaultPosition,
          wxSize(4 * wxWindow::GetCharWidth(), 3 * wxWindow::GetCharWidth()));
      text_ctrl->SetToolTip(
          _("Enter a two-letter talker ID to override the default ID in NMEA "
            "sentences generated by OpenCPN (e.g., GP, HC, EC). This affects "
            "only sentences created by OpenCPN, not those forwarded from other "
            "devices."));
      text_ctrl->SetValue(g_TalkerIdText);
      hbox->Add(text_ctrl, wxSizerFlags().Border());
      SetSizer(hbox);
      TalkerIdRow::Cancel();
    }

    void Apply() override { g_TalkerIdText = text_ctrl->GetValue(); }
    void Cancel() override { text_ctrl->SetValue(g_TalkerIdText); }
  };

  /** Global netmask configuration bound to g_netmask_bits. */
  class NetmaskRow : public wxPanel, public ApplyCancel {
  public:
    explicit NetmaskRow(wxWindow* parent)
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
      NetmaskRow::Cancel();

      Bind(wxEVT_SPINCTRL, [&](wxSpinEvent&) {
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
  };
};

/** Top panel: connections grid, "Add new connection", general options. */
class TopPanel : public wxPanel {
public:
  TopPanel(wxWindow* parent, const std::vector<ConnectionParams*>& connections,
           EventVar& evt_add_connection,
           const std::function<void(ConnectionParams* p, bool editing)>&
               on_edit_conn)
      : wxPanel(parent, wxID_ANY),
        m_evt_add_connection(evt_add_connection),
        m_connections(connections) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto conn_grid = new Connections(this, m_connections, m_evt_add_connection,
                                     on_edit_conn);
    wxSize panel_max_size(conn_grid->GetEstimatedSize());
    vbox->AddSpacer(wxWindow::GetCharHeight());
    auto conn_flags = wxSizerFlags().Border();
    if (IsAndroid()) conn_flags = wxSizerFlags().Border().Expand();
    vbox->Add(conn_grid, conn_flags);
    vbox->Add(new AddConnectionButton(this, m_evt_add_connection, on_edit_conn),
              wxSizerFlags().Border());
    vbox->Add(0, wxWindow::GetCharHeight());  // Expanding spacer
    auto panel_flags =
        wxSizerFlags().Border(wxLEFT | wxDOWN | wxRIGHT).Expand();
    m_general_panel = new GeneralPanel(this, panel_max_size);
    vbox->Add(m_general_panel, panel_flags);

    auto advanced_panel = new AdvancedPanel(this, panel_max_size);
    m_advanced_panel = advanced_panel;
    auto on_toggle = [&, advanced_panel, vbox](bool show) {
      // FIXME advanced_panel->SetMaxSize({conn_grid->GetSize().x, -1});
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
    wxWindow::Show();

    auto on_evt_update_connections = [&, conn_grid](ObservedEvt&) {
      conn_grid->ReloadGrid(TheConnectionParams());
      conn_grid->Show(conn_grid->GetNumberRows() > 0);
      Layout();
    };
    m_add_connection_lstnr.Init(m_evt_add_connection,
                                on_evt_update_connections);
    m_conn_grid = conn_grid;
  }

  void SetColorScheme(const ColorScheme cs) const {
    m_conn_grid->SetColorScheme(cs);
    m_general_panel->SetColorScheme(cs);
  }
  [[nodiscard]] Connections* GetConnectionsGrid() const { return m_conn_grid; }

  EventVar& m_evt_add_connection;

private:
  const std::vector<ConnectionParams*>& m_connections;
  ObsListener m_add_connection_lstnr;
  Connections* m_conn_grid;
  GeneralPanel* m_general_panel;
  AdvancedPanel* m_advanced_panel;
};

/** Top scroll window, adds scrollbars to TopPanel. */
class TopScroll : public wxScrolledWindow {
public:
  TopScroll(wxWindow* parent, const std::vector<ConnectionParams*>& connections,
            EventVar& evt_add_connection)
      : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                         wxVSCROLL | wxHSCROLL, TopScrollWindowName) {
    ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_ALWAYS);
    auto vbox = new wxBoxSizer(wxVERTICAL);
    SetSizer(vbox);

    auto on_edit_connection = [&](ConnectionParams* p, bool editing) {
      if (editing)
        HandleEdit(p);
      else
        HandleNew(p);
    };

    top_panel =
        new TopPanel(this, connections, evt_add_connection, on_edit_connection);
    vbox->Add(top_panel, wxSizerFlags(1).Expand());

    auto on_edit_click = [&](const ConnectionParams* p, bool new_mode,
                             bool ok_cancel) {
      HandleEditFinish(p, new_mode, ok_cancel);
    };

    m_edit_panel = new ConnectionEditDialog(this, on_edit_click);
    m_edit_panel->SetPropsLabel(_("Edit Selected Connection"));
    wxWindow* options = FindWindowByName("Options");
    assert(options && "Null Options window!");
    int fraction = 9;
#ifdef ANDROID
    fraction = 10;
#endif
    m_edit_panel->SetSize(
        wxSize(options->GetSize().x, options->GetSize().y * fraction / 10));
    vbox->Add(m_edit_panel, wxSizerFlags(0).Expand());
    m_edit_panel->Hide();

    SetScrollRate(0, 10);
    if (IsAndroid()) SetScrollRate(1, 1);

    SetScrollRate(0, 10);
    if (IsAndroid()) SetScrollRate(1, 1);
  }

  void SetColorScheme(ColorScheme cs) const {
    if (top_panel) top_panel->SetColorScheme(cs);
  }

  void HandleEdit(ConnectionParams* p) {
    m_edit_panel->SetPropsLabel(_("Edit Selected Connection"));
    m_edit_panel->SetNewMode(false);
    m_edit_panel->PreloadControls(p);
    m_edit_panel->AddOKCancelButtons();
    SwitchToEditor();
  }

  void HandleNew(ConnectionParams*) {
    m_edit_panel->SetPropsLabel(_("Configure new connection"));
    m_edit_panel->SetDefaultConnectionParams();
    m_edit_panel->SetNewMode(true);
    m_edit_panel->AddOKCancelButtons();
    SwitchToEditor();
  }

  void SwitchToEditor() {
    top_panel->Hide();  // TopPanel
    Scroll(0, 0);
    DimeControl(m_edit_panel);
    m_edit_panel->Show();
    g_options->ShowOKButtons(false);

    Layout();
  }

  void SwitchToGrid() {
    g_options->ShowOKButtons(true);
    m_edit_panel->Hide();
    top_panel->Show();
    top_panel->GetConnectionsGrid()->ReloadGrid(TheConnectionParams());
    Layout();
    Scroll(0, 0);
  }

  void HandleEditFinish(const ConnectionParams* cp_orig, bool new_mode,
                        bool ok_cancel) {
    if (!ok_cancel) {
      SwitchToGrid();
      return;
    }
    // OK from EDIT mode
    if (!new_mode) {
      ConnectionParams* cp_edited = m_edit_panel->GetParamsFromControls();
      delete cp_orig->m_optionsPanel;
      StopAndRemoveCommDriver(cp_orig->GetStrippedDSPort(),
                              cp_orig->GetCommProtocol());
      int index = top_panel->GetConnectionsGrid()->FindConnectionIndex(cp_orig);
      assert(index != -1 && "Cannot look up connection index");
      TheConnectionParams()[index] = cp_edited;
      cp_edited->b_IsSetup = false;  // Trigger new stream
    }
    //  OK from NEW mode
    else {
      if (ConnectionParams* cp = m_edit_panel->GetParamsFromControls()) {
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
      top_panel->m_evt_add_connection.Notify();
    }

    SwitchToGrid();
    UpdateDatastreams();
  }

private:
  TopPanel* top_panel;
  ConnectionEditDialog* m_edit_panel;
};

/** Main window: Panel with a single TopScroll child. */
ConnectionsDlg::ConnectionsDlg(
    wxWindow* parent, const std::vector<ConnectionParams*>& connections)
    : wxPanel(parent, wxID_ANY), m_connections(connections) {
  const auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(new TopScroll(this, connections, m_evt_add_connection),
            wxSizerFlags(1).Expand());
  SetSizer(vbox);
  wxWindow::Fit();
  wxWindow::Show();
}

void ConnectionsDlg::OnResize(const wxSize& size) {
  auto w = FindWindowByName(TopScrollWindowName);
  if (!w) return;
  w->SetMinSize(size);
  Fit();
}

void ConnectionsDlg::DoApply(wxWindow* root) {
  for (wxWindow* child : root->GetChildren()) {
    if (auto widget = dynamic_cast<ApplyCancel*>(child)) widget->Apply();
    DoApply(child);
  }
}

void ConnectionsDlg::DoCancel(wxWindow* root) {
  for (wxWindow* child : root->GetChildren()) {
    if (auto widget = dynamic_cast<ApplyCancel*>(child)) widget->Cancel();
    DoCancel(child);
  }
}

void ConnectionsDlg::ApplySettings() { DoApply(this); }

void ConnectionsDlg::CancelSettings() { DoCancel(this); }

void ConnectionsDlg::SetColorScheme(ColorScheme cs) {
  auto w = dynamic_cast<TopScroll*>(FindWindowByName(TopScrollWindowName));
  if (w) w->SetColorScheme(cs);
}
