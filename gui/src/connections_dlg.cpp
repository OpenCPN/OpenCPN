#include <array>

#include <wx/bitmap.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/window.h>

#include "model/base_platform.h"

#include "connections_dlg.h"
#include "std_filesystem.h"
#include "svg_utils.h"

static void ReloadGrid(wxGrid& grid) {
  static const std::array<std::array<std::string, 7>, 4> contents = {
      {{"☑", "Nmea 0183", "In", "ttyS0", "🟢", "⚙", "🗑"},
       {"☐", "Nmea 0183", "In", "ttyUSB0", "⚫", "⚙", "🗑"},
       {"☑", "Nmea 2000", "In", "can0", "🔴", "⚙", "🗑"},
       {"☑", "Nmea 0183/tcp", "In", "wlx982567", "🟡", "⚙", "🗑"}}};
  grid.ClearGrid();
  for (auto row = contents.begin(); row < contents.end(); row++) {
    grid.AppendRows(1);
    for (auto col = row->begin(); col != row->end(); col++) {
      grid.SetCellValue(row - contents.begin(), col - row->begin(), *col);
    }
  }
}

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
    auto flags = wxSizerFlags().Expand();
    sizer->Add(static_bitmap, flags);
    SetSizer(sizer);
    Fit();
    Show();
    SetToolTip("Add new connection");
  }
};

class Connections : public wxGrid {
public:
  Connections(wxWindow* parent) : wxGrid(parent, wxID_ANY) {
    CreateGrid(0, 7);
    static const std::array<std::string, 7> headers = {
        "", "Protocol", "Direction", "Port", "Status", "", ""};
    for (auto hdr = headers.begin(); hdr != headers.end(); hdr++) {
      SetColLabelValue(hdr - headers.begin(), *hdr);
    }
    HideRowLabels();
    auto enable_attr = new wxGridCellAttr();

    enable_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    enable_attr->SetFont(parent->GetFont().Scale(1.3));
    SetColAttr(0, enable_attr);

    auto in_out_attr = new wxGridCellAttr();
    in_out_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    SetColAttr(2, in_out_attr);

    auto status_attr = new wxGridCellAttr();
    status_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    SetColAttr(4, status_attr);

    auto edit_attr = new wxGridCellAttr();
    edit_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    edit_attr->SetFont(parent->GetFont().Scale(1.3));
    SetColAttr(5, edit_attr);

    auto delete_attr = new wxGridCellAttr();
    delete_attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    delete_attr->SetFont(parent->GetFont().Scale(1.3));
    delete_attr->SetTextColour(*wxRED);
    SetColAttr(6, delete_attr);
    ReloadGrid(*this);

    GetGridWindow()->Bind(wxEVT_MOTION, [&](wxMouseEvent ev) {
      wxPoint pt = ev.GetPosition();
      SetTooltip(YToRow(pt.y), XToCol(pt.x));
      ev.Skip();
    });
    Bind(wxEVT_GRID_SELECT_CELL, [&](wxGridEvent& ev) {
      std::cout << "OnSelectCell, row: " << ev.GetRow()
                << ", col: " << ev.GetCol() << "\n";
    });
  }

  void SetTooltip(int row, int col) {
    std::string tooltip = "";
    switch (col) {
      case 0:
        if (row == 1)
          tooltip = "Disabled, click to enable";
        else
          tooltip = "Enabled, click to disable";
        break;
      case 5:
        tooltip = "Edit connection";
        break;
      case 6:
        tooltip = "Delete connection";
        break;
      case 4:
        if (row == 0)
          tooltip = "Active, data is flowing";
        else if (row == 1)
          tooltip = "Disabled";
        else if (row == 2)
          tooltip = "Device unavailable";
        else
          tooltip = "No data detected";
        break;
      default:
        break;
    }
    GetGridWindow()->SetToolTip(tooltip);
  }
};

ConnectionsDlg::ConnectionsDlg(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, _("Connections")) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto flags = wxSizerFlags().Expand().DoubleBorder();
  vbox->Add(new Connections(this), flags);
  vbox->Add(new AddConnectionPanel(this), flags);
  SetSizer(vbox);
  Fit();
  Show();
}
