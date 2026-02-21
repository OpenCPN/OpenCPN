/***************************************************************************
 *   Copyright (C) 2025  Alec Leamas                                       *
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
 * Implement data_monitor.h -- new NMEA Debugger successor main window.
 */

#include <chrono>
#include <fstream>
#include <sstream>

#include <wx/app.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/translation.h>
#include <wx/wrapsizer.h>

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#include "model/base_platform.h"
#include "model/config_vars.h"
#include "model/data_monitor_src.h"
#include "model/filters_on_disk.h"
#include "model/gui.h"
#include "model/navmsg_filter.h"
#include "model/nmea_log.h"
#include "model/svg_utils.h"

#include "data_monitor.h"
#include "std_filesystem.h"
#include "svg_button.h"
#include "svg_icons.h"
#include "tty_scroll.h"
#include "user_colors_dlg.h"
#include "filter_dlg.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"

// Make _() return std::string instead of wxString;
#undef _
#if wxCHECK_VERSION(3, 2, 0)
#define _(s) wxGetTranslation(wxASCII_STR(s)).ToStdString()
#else
#define _(s) wxGetTranslation((s)).ToStdString()
#endif

using SetFormatFunc = std::function<void(DataLogger::Format, std::string)>;

/** Return window with given id (which must exist) cast to T*. */
template <typename T>
T* GetWindowById(int id) {
  return dynamic_cast<T*>(wxWindow::FindWindowById(id));
};

static const char* const kFilterChoiceName = "FilterChoiceWindow";

// clang-format: off
static const std::unordered_map<NavAddr::Bus, std::string> kSourceByBus = {
    {NavAddr::Bus::N0183, "NMEA0183"},
    {NavAddr::Bus::N2000, "NMEA2000"},
    {NavAddr::Bus::Signalk, "SignalK"}};  // clang-format: on

/** Return true if given filter is defined by user. */
static bool IsUserFilter(const std::string& filter_name) {
  std::vector<std::string> filters = filters_on_disk::List();
  auto found = std::find(filters.begin(), filters.end(), filter_name);
  if (found != filters.end()) return true;
  return std::any_of(
      filters.begin(), filters.end(),
      [filter_name](const std::string& f) { return f == filter_name; });
}

/** Return logging milliseconds timestamp. */
static std::string TimeStamp(const NavmsgTimePoint& when,
                             const NavmsgTimePoint& since) {
  using namespace std::chrono;
  using namespace std;

  auto duration = when - since;
  std::stringstream ss;
  auto hrs = duration_cast<hours>(duration) % 24;
  duration -= duration_cast<hours>(duration) / 24;
  auto mins = duration_cast<minutes>(duration) % 60;
  duration -= duration_cast<minutes>(duration) / 60;
  auto secs = duration_cast<seconds>(duration) % 60;
  duration -= duration_cast<seconds>(duration) / 60;
  const auto msecs = duration_cast<milliseconds>(duration);
  ss << setw(2) << setfill('0') << hrs.count() << ":" << setw(2) << mins.count()
     << ":" << setw(2) << secs.count() << "." << setw(3)
     << msecs.count() % 1000;
  return ss.str();
}

static fs::path NullLogfile() {
  if (wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_WINDOWS)
    return "NUL:";
  return "/dev/null";
}

/**
 * Quote arg string as required by VDR plugin, see
 * https://opencpn-manuals.github.io/main/vdr/log_format.html
 * @param arg unquoted string
 * @return Possibly quoted string handling double quotes in arg.
 */
static std::string VdrQuote(const std::string& arg) {
  auto static constexpr npos = std::string::npos;
  if (arg.find(',') == npos && arg.find('"') == npos) return arg;
  std::string s;
  for (const auto c : arg) {
    if (c == '"')
      s += "\"\"";
    else
      s += c;
  }
  return "\"" + s + "\"";
}

/**
 * Write a line in the log using the VDR plugin format as defined in
 * https://opencpn-manuals.github.io/main/vdr/log_format.html
 */
static void AddVdrLogline(const Logline& ll, std::ostream& stream) {
  if (kSourceByBus.find(ll.navmsg->bus) == kSourceByBus.end()) return;

  using namespace std::chrono;
  const auto now = system_clock::now();
  const auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
  stream << ms << ",";

  stream << kSourceByBus.at(ll.navmsg->bus) << ",";
  stream << ll.navmsg->source->iface << ",";
  switch (ll.navmsg->bus) {
    case NavAddr::Bus::N0183: {
      auto msg0183 = std::dynamic_pointer_cast<const Nmea0183Msg>(ll.navmsg);
      stream << msg0183->talker << msg0183->type << ",";
    } break;
    case NavAddr::Bus::N2000: {
      auto msg2000 = std::dynamic_pointer_cast<const Nmea2000Msg>(ll.navmsg);
      stream << msg2000->PGN.to_string() << ",";
    } break;
    case NavAddr::Bus::Signalk: {
      auto msgSignalK = std::dynamic_pointer_cast<const SignalkMsg>(ll.navmsg);
      stream << "\"" << msgSignalK->context_self << "\",";
    } break;
    default:
      assert(false && "Illegal message type");
  };
  stream << VdrQuote(ll.navmsg->to_vdr()) << "\n";
}

/** Write a line in the log using the standard format. */
static void AddStdLogline(const Logline& ll, std::ostream& stream, char fs,
                          const NavmsgTimePoint log_start) {
  if (!ll.navmsg) return;
  wxString ws;
  ws << TimeStamp(ll.navmsg->created_at, log_start) << fs;
  if (ll.state.direction == NavmsgStatus::Direction::kOutput)
    ws << kUtfRightArrow << fs;
  else if (ll.state.direction == NavmsgStatus::Direction::kInput)
    ws << kUtfLeftwardsArrowToBar << fs;
  else if (ll.state.direction == NavmsgStatus::Direction::kInternal)
    ws << kUtfLeftRightArrow << fs;
  else
    ws << kUtfLeftArrow << fs;
  if (ll.state.status != NavmsgStatus::State::kOk)
    ws << kUtfMultiplicationX << fs;
  else if (ll.state.accepted == NavmsgStatus::Accepted::kFilteredNoOutput)
    ws << kUtfFallingDiagonal << fs;
  else if (ll.state.accepted == NavmsgStatus::Accepted::kFilteredDropped)
    ws << kUtfCircledDivisionSlash << fs;
  else
    ws << kUtfCheckMark << fs;

  ws << ll.navmsg->source->iface << fs;
  ws << NavAddr::BusToString(ll.navmsg->bus) << fs;
  if (ll.state.status != NavmsgStatus::State::kOk)
    ws << (!ll.error_msg.empty() ? ll.error_msg : "Unknown  error");
  else
    ws << "ok";
  ws << fs << ll.message << "\n";
  stream << ws;
}

/** Clickable crossmark icon window*/
class CrossIconWindow : public wxWindow {
public:
  CrossIconWindow(wxWindow* parent, std::function<void()> on_click)
      : wxWindow(parent, wxID_ANY), m_on_click(on_click) {
    fs::path icon_path(g_BasePlatform->GetSharedDataDir().ToStdString());
    icon_path /= fs::path("uidata") / "MUI_flat" / "cross-small-symbolic.svg";
    int size = parent->GetTextExtent("X").y;
    m_bitmap = LoadSVG(icon_path.string(), size, size);
    assert(m_bitmap.IsOk());
    SetInitialSize({size, size});

    Bind(wxEVT_LEFT_DOWN, [&](wxMouseEvent&) { m_on_click(); });
    Bind(wxEVT_PAINT, [&](wxPaintEvent& ev) { OnPaint(ev); });
  }

private:
  void OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    PrepareDC(dc);
    dc.DrawBitmap(m_bitmap, 0, 0, true);
  }

  wxBitmap m_bitmap;
  std::function<void()> m_on_click;
};

/** Main window, a rolling log of messages. */
class TtyPanel : public wxPanel, public NmeaLog {
public:
  TtyPanel(wxWindow* parent, size_t lines)
      : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                wxTAB_TRAVERSAL, "TtyPanel"),
        m_tty_scroll(nullptr),
        m_filter(this, wxID_ANY),
        m_lines(lines),
        m_on_right_click([] {}) {
    const auto vbox = new wxBoxSizer(wxVERTICAL);
    m_tty_scroll = new TtyScroll(this, static_cast<int>(m_lines));
    m_tty_scroll->Bind(wxEVT_RIGHT_UP,
                       [&](wxMouseEvent&) { m_on_right_click(); });
    vbox->Add(m_tty_scroll, wxSizerFlags(1).Expand().Border());
    m_filter.Hide();
    SetSizer(vbox);
    wxWindow::Fit();
  }

  void Add(const Logline& ll) override { m_tty_scroll->Add(ll); }

  bool IsVisible() const override { return IsShownOnScreen(); }

  void OnStop(bool stop) const {
    m_tty_scroll->Pause(stop);
    if (stop)
      m_tty_scroll->ShowScrollbars(wxSHOW_SB_DEFAULT, wxSHOW_SB_DEFAULT);
    else
      m_tty_scroll->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
  }

  void SetFilter(const NavmsgFilter& f) const { m_tty_scroll->SetFilter(f); };

  void SetQuickFilter(const std::string& filter) const {
    m_tty_scroll->SetQuickFilter(filter);
  }

  void SetOnRightClick(std::function<void()> f) {
    m_on_right_click = std::move(f);
  }

  /** Invoke Add(s) for possibly existing instance. */
  static void AddIfExists(const Logline& ll) {
    auto window = wxWindow::FindWindowByName("TtyPanel");
    if (!window) return;
    auto tty_panel = dynamic_cast<TtyPanel*>(window);
    if (tty_panel) tty_panel->Add(ll);
  }

protected:
  wxSize DoGetBestClientSize() const override {
    return {1, static_cast<int>(m_lines * GetCharHeight())};
  }

private:
  TtyScroll* m_tty_scroll;
  wxTextCtrl m_filter;
  size_t m_lines;
  std::function<void()> m_on_right_click;
};

/** The quick filter above the status line, invoked by funnel button. */
class QuickFilterPanel : public wxPanel {
public:
  QuickFilterPanel(wxWindow* parent, std::function<void()> on_text_evt,
                   std::function<void()> on_close)
      : wxPanel(parent),
        m_text_ctrl(new wxTextCtrl(this, wxID_ANY)),
        m_on_text_evt(std::move(on_text_evt)),
        m_on_close(std::move(on_close)) {
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    auto flags = wxSizerFlags(0).Border();
    auto label_box = new wxBoxSizer(wxVERTICAL);
    label_box->Add(new wxStaticText(this, wxID_ANY, _("Quick filter:")));
    hbox->Add(label_box, flags.Align(wxALIGN_CENTER_VERTICAL));
    hbox->Add(m_text_ctrl, flags);
    hbox->AddStretchSpacer();
    hbox->Add(new CrossIconWindow(this, [&] { m_on_close(); }), flags);
    SetSizer(hbox);
    wxWindow::Fit();
    wxWindow::Show();
    m_text_ctrl->Bind(wxEVT_TEXT, [&](wxCommandEvent&) { m_on_text_evt(); });
  }

  bool Show(bool show) override {
    if (!show) m_text_ctrl->SetValue("");
    return wxWindow::Show(show);
  }

  [[nodiscard]] std::string GetValue() const {
    return m_text_ctrl->GetValue().ToStdString();
  }

private:
  wxTextCtrl* m_text_ctrl;
  std::function<void()> m_on_text_evt;
  std::function<void()> m_on_close;
};

/** Offer user to select current filter. */
class FilterChoice : public wxChoice {
public:
  FilterChoice(wxWindow* parent, TtyPanel* tty_panel)
      : wxChoice(parent, wxID_ANY), m_tty_panel(tty_panel) {
    wxWindow::SetName(kFilterChoiceName);
    Bind(wxEVT_CHOICE, [&](wxCommandEvent&) { OnChoice(); });
    OnFilterListChange();
    const int ix = wxChoice::FindString(kLabels.at("default"));
    if (ix != wxNOT_FOUND) wxChoice::SetSelection(ix);
    NavmsgFilter filter = filters_on_disk::Read("default.filter");
    m_tty_panel->SetFilter(filter);
  }

  void OnFilterListChange() {
    m_filters = NavmsgFilter::GetAllFilters();
    int select_ix = GetSelection();
    std::string selected;
    if (select_ix != wxNOT_FOUND) selected = GetString(select_ix).ToStdString();
    Clear();
    for (auto& filter : m_filters) {
      try {
        Append(kLabels.at(filter.m_name));
      } catch (std::out_of_range&) {
        if (filter.m_description.empty())
          Append(filter.m_name);
        else
          Append(filter.m_description);
      }
    }
    if (!selected.empty()) {
      int ix = FindString(selected);
      SetSelection(ix == wxNOT_FOUND ? 0 : ix);
    }
  }

  void OnFilterUpdate(const std::string& name) {
    m_filters = NavmsgFilter::GetAllFilters();
    int select_ix = GetSelection();
    if (select_ix == wxNOT_FOUND) return;

    std::string selected = GetString(select_ix).ToStdString();
    if (selected != name) return;

    NavmsgFilter filter = filters_on_disk::Read(name);
    m_tty_panel->SetFilter(filter);
  }

  void OnApply(const std::string& name) {
    int found = FindString(name);
    if (found == wxNOT_FOUND) {
      for (auto& filter : m_filters) {
        if (filter.m_name == name) {
          found = FindString(filter.m_description);
          break;
        }
      }
    }
    if (found == wxNOT_FOUND) return;

    SetSelection(found);
    OnFilterUpdate(name);
  }

private:
  // Translated labels for system filters by filter name. If not
  // found the untranslated json description is used.
  const std::unordered_map<std::string, std::string> kLabels = {
      {"all-data", _("All data")},
      {"all-nmea", _("All NMEA data")},
      {"default", _("Default settings")},
      {"malformed", _("Malformed messages")},
      {"nmea-input", _("NMEA input data")},
      {"nmea-output", _("NMEA output data")},
      {"plugins", _("Messages to plugins")},
  };

  std::vector<NavmsgFilter> m_filters;
  TtyPanel* m_tty_panel;

  void OnChoice() {
    wxString label = GetString(GetSelection());
    NavmsgFilter filter = FilterByLabel(label.ToStdString());
    m_tty_panel->SetFilter(filter);
  }

  NavmsgFilter FilterByLabel(const std::string& label) {
    std::string name = label;
    for (const auto& kv : kLabels) {
      if (kv.second == label) {
        name = kv.first;
        break;
      }
    }
    if (!name.empty()) {
      for (auto& f : m_filters)
        if (f.m_name == name) return f;
    } else {
      for (auto& f : m_filters)
        if (f.m_description == label) return f;
    }
    return {};
  }
};

/** Button to stop/resume messages in main log window. */
class PauseResumeButton : public wxButton {
public:
  PauseResumeButton(wxWindow* parent, std::function<void(bool)> on_stop)
      : wxButton(parent, wxID_ANY),
        is_paused(true),
        m_on_stop(std::move(on_stop)) {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    OnClick();
  }

private:
  bool is_paused;
  std::function<void(bool)> m_on_stop;

  void OnClick() {
    is_paused = !is_paused;
    m_on_stop(is_paused);
    SetLabel(is_paused ? _("Resume") : _("Pause"));
  }
};

/** Button to hide data monitor, used only on Android. */
class CloseButton : public wxButton {
public:
  CloseButton(wxWindow* parent, std::function<void()> on_close)
      : wxButton(parent, wxID_ANY), m_on_close(std::move(on_close)) {
    wxButton::SetLabel(_("Close"));
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    OnClick();
  }

private:
  std::function<void()> m_on_close;

  void OnClick() const { m_on_close(); }
};

/** Log setup window invoked from menu "Logging" item. */
class LoggingSetup : public wxDialog {
public:
  /** Top part above buttons */
  class ThePanel : public wxPanel {
  public:
    ThePanel(wxWindow* parent, SetFormatFunc set_logtype, DataLogger& logger)
        : wxPanel(parent),
          m_overwrite(false),
          m_set_logtype(std::move(set_logtype)),
          m_logger(logger),
          kFilenameLabelId(wxWindow::NewControlId()) {
      auto flags = wxSizerFlags(0).Border();

      /* left column: Select log format. */
      auto vdr_btn = new wxRadioButton(this, wxID_ANY, "VDR");
      vdr_btn->Bind(wxEVT_RADIOBUTTON, [&](const wxCommandEvent& e) {
        m_set_logtype(DataLogger::Format::kVdr, "VDR");
      });
      auto default_btn = new wxRadioButton(this, wxID_ANY, "Default");
      default_btn->Bind(wxEVT_RADIOBUTTON, [&](const wxCommandEvent& e) {
        m_set_logtype(DataLogger::Format::kDefault, _("Default"));
      });
      default_btn->SetValue(true);
      auto csv_btn = new wxRadioButton(this, wxID_ANY, "CSV");
      csv_btn->Bind(wxEVT_RADIOBUTTON, [&](const wxCommandEvent& e) {
        m_set_logtype(DataLogger::Format::kCsv, "CSV");
      });
      auto left_vbox = new wxStaticBoxSizer(wxVERTICAL, this, _("Log format"));
      left_vbox->Add(default_btn, flags.DoubleBorder());
      left_vbox->Add(vdr_btn, flags);
      left_vbox->Add(csv_btn, flags);

      /* Right column: log file */
      m_logger.SetLogfile(m_logger.GetDefaultLogfile());
      auto label = new wxStaticText(this, kFilenameLabelId,
                                    m_logger.GetDefaultLogfile().string());
      auto path_btn = new wxButton(this, wxID_ANY, _("Change..."));
      path_btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnFileDialog(); });
      auto force_box =
          new wxCheckBox(this, wxID_ANY, _("Overwrite existing file"));
      force_box->Bind(wxEVT_CHECKBOX, [&](const wxCommandEvent& e) {
        m_overwrite = e.IsChecked();
      });
      auto right_vbox = new wxStaticBoxSizer(wxVERTICAL, this, _("Log file"));
      right_vbox->Add(label, flags);
      right_vbox->Add(path_btn, flags);
      right_vbox->Add(force_box, flags);

      /* Top part above buttons */
      auto hbox = new wxBoxSizer(wxHORIZONTAL);
      hbox->Add(left_vbox, flags);
      hbox->Add(wxWindow::GetCharWidth() * 10, 0, 1);
      hbox->Add(right_vbox, flags);
      SetSizer(hbox);
      wxWindow::Layout();
      wxWindow::Show();

      FilenameLstnr.Init(logger.OnNewLogfile, [&](const ObservedEvt& ev) {
        GetWindowById<wxStaticText>(kFilenameLabelId)->SetLabel(ev.GetString());
        g_dm_logfile = ev.GetString();
      });
    }

    void OnFileDialog() const {
      long options = wxFD_SAVE;
      if (!m_overwrite) options |= wxFD_OVERWRITE_PROMPT;
      wxFileDialog dlg(m_parent, _("Select logfile"),
                       m_logger.GetDefaultLogfile().parent_path().string(),
                       m_logger.GetDefaultLogfile().stem().string(),
                       m_logger.GetFileDlgTypes(), options);
      if (dlg.ShowModal() == wxID_CANCEL) return;
      m_logger.SetLogfile(fs::path(dlg.GetPath().ToStdString()));
      auto file_label = GetWindowById<wxStaticText>(kFilenameLabelId);
      file_label->SetLabel(dlg.GetPath());
    }

    bool m_overwrite;
    SetFormatFunc m_set_logtype;
    DataLogger& m_logger;
    const int kFilenameLabelId;
    ObsListener FilenameLstnr;
  };  // ThePanel

  LoggingSetup(wxWindow* parent, SetFormatFunc set_logtype, DataLogger& logger)
      : wxDialog(parent, wxID_ANY, _("Logging setup"), wxDefaultPosition,
                 wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
    auto flags = wxSizerFlags(0).Border();

    /* Buttons at bottom */
    auto buttons = new wxStdDialogButtonSizer();
    auto close_btn = new wxButton(this, wxID_CLOSE);
    close_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                    [&](wxCommandEvent& ev) { EndModal(0); });
    buttons->AddButton(close_btn);
    buttons->Realize();
    buttons->Fit(parent);

    /* Overall vbox setup */
    auto panel = new ThePanel(this, std::move(set_logtype), logger);
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(panel, flags.Expand());
    vbox->Add(new wxStaticLine(this, wxID_ANY), flags.Expand());
    vbox->Add(buttons, flags.Expand());
    SetSizer(vbox);
    wxWindow::Fit();
    wxDialog::Show();
  }
  ObsListener FilenameLstnr;
};

/** The monitor popup menu. */
class TheMenu : public wxMenu {
public:
  enum class Id : char {
    kNewFilter = 1,  // MacOS does not want ids to be 0.
    kEditFilter,
    kDeleteFilter,
    kRenameFilter,
    kEditActiveFilter,
    kLogSetup,
    kViewStdColors,
    kUserColors,
    kClear
  };

  TheMenu(wxWindow* parent, DataLogger& logger)
      : m_parent(parent), m_logger(logger), m_is_logging_configured(false) {
    AppendCheckItem(static_cast<int>(Id::kViewStdColors), _("Use colors"));
    Append(static_cast<int>(Id::kUserColors), _("Colors..."));
    Append(static_cast<int>(Id::kClear), _("Clear..."));
    Append(static_cast<int>(Id::kLogSetup), _("Logging..."));
    auto filters = new wxMenu("");
    AppendId(filters, Id::kNewFilter, _("Create new..."));
    AppendId(filters, Id::kEditFilter, _("Edit..."));
    AppendId(filters, Id::kDeleteFilter, _("Delete..."));
    AppendId(filters, Id::kRenameFilter, _("Rename..."));
    AppendSubMenu(filters, _("Filters..."));
    if (IsUserFilter(m_filter))
      Append(static_cast<int>(Id::kEditActiveFilter), _("Edit active filter"));

    Bind(wxEVT_MENU, [&](const wxCommandEvent& ev) {
      switch (static_cast<Id>(ev.GetId())) {
        case Id::kLogSetup:
          ConfigureLogging();
          break;

        case Id::kViewStdColors:
          SetColor(static_cast<int>(Id::kViewStdColors));
          break;

        case Id::kNewFilter:
          CreateFilterDlg(parent);
          break;

        case Id::kEditFilter:
          EditFilterDlg(wxTheApp->GetTopWindow());
          break;

        case Id::kRenameFilter:
          RenameFilterDlg(wxTheApp->GetTopWindow());
          break;

        case Id::kEditActiveFilter:
          EditOneFilterDlg(wxTheApp->GetTopWindow(), m_filter);
          break;

        case Id::kDeleteFilter:
          RemoveFilterDlg(parent);
          break;

        case Id::kUserColors:
          UserColorsDlg(wxTheApp->GetTopWindow());
          break;

        case Id::kClear:
          ClearLogWindow();
          break;
      }
    });
    Check(static_cast<int>(Id::kViewStdColors), true);
  }

  void ClearLogWindow() {
    auto* w = wxWindow::FindWindowByName("TtyScroll");
    auto tty_scroll = dynamic_cast<TtyScroll*>(w);
    if (tty_scroll) tty_scroll->Clear();
  }

  void SetFilterName(const std::string& filter) {
    int id = static_cast<int>(Id::kEditActiveFilter);
    if (FindItem(id)) Delete(id);
    if (IsUserFilter(filter)) Append(id, _("Edit active filter"));
    m_filter = filter;
  }

  void ConfigureLogging() {
    LoggingSetup dlg(
        m_parent,
        [&](DataLogger::Format f, const std::string& s) { SetLogFormat(f, s); },
        m_logger);
    dlg.ShowModal();
    m_is_logging_configured = true;
    auto monitor = wxWindow::FindWindowByName(kDataMonitorWindowName);
    assert(monitor);
    monitor->Layout();
  }

  bool IsLoggingConfigured() const { return m_is_logging_configured; }

private:
  static wxMenuItem* AppendId(wxMenu* root, Id id, const wxString& label) {
    return root->Append(static_cast<int>(id), label);
  }

  void SetLogFormat(DataLogger::Format format, const std::string& label) const {
    m_logger.SetFormat(format);
    std::string extension =
        format == DataLogger::Format::kDefault ? ".log" : ".csv";
    fs::path path = m_logger.GetLogfile();
    path = path.parent_path() / (path.stem().string() + extension);
    m_logger.SetLogfile(path);
  }

  void SetColor(int id) const {
    auto* w = wxWindow::FindWindowByName("TtyScroll");
    auto tty_scroll = dynamic_cast<TtyScroll*>(w);
    if (!tty_scroll) return;

    wxMenuItem* item = FindItem(id);
    if (!item) return;
    if (item->IsCheck() && item->IsChecked())
      tty_scroll->SetColors(std::make_unique<StdColorsByState>());
    else
      tty_scroll->SetColors(
          std::make_unique<NoColorsByState>(tty_scroll->GetForegroundColour()));
  }

  wxWindow* m_parent;
  DataLogger& m_logger;
  std::string m_filter;
  bool m_is_logging_configured;
};

/** Button to start/stop logging. */
class LogButton : public wxButton {
public:
  LogButton(wxWindow* parent, DataLogger& logger, TheMenu& menu)
      : wxButton(parent, wxID_ANY),
        is_logging(true),
        m_is_inited(false),
        m_logger(logger),
        m_menu(menu) {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    OnClick(true);
    UpdateTooltip();
  }

  void UpdateTooltip() {
    if (is_logging)
      SetToolTip(_("Click to stop logging"));
    else
      SetToolTip(_("Click to start logging"));
  }

private:
  bool is_logging;
  bool m_is_inited;
  DataLogger& m_logger;
  TheMenu& m_menu;

  void OnClick(bool ctor = false) {
    if (!m_is_inited && !ctor && !m_menu.IsLoggingConfigured()) {
      m_menu.ConfigureLogging();
      m_is_inited = true;
    }
    is_logging = !is_logging;
    SetLabel(is_logging ? _("Stop logging") : _("Start logging"));
    UpdateTooltip();
    m_logger.SetLogging(is_logging);
  }
};

/** Copy to clipboard button */
class CopyClipboardButton : public SvgButton {
public:
  explicit CopyClipboardButton(wxWindow* parent) : SvgButton(parent) {
    LoadIcon(kCopyIconSvg);
    SetToolTip(_("Copy to clipboard"));
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
      auto* tty_scroll =
          dynamic_cast<TtyScroll*>(wxWindow::FindWindowByName("TtyScroll"));
      if (tty_scroll) tty_scroll->CopyToClipboard();
    });
  }
};

/** Button opening the filter dialog */
class FilterButton : public SvgButton {
public:
  FilterButton(wxWindow* parent, wxWindow* quick_filter)
      : SvgButton(parent), m_quick_filter(quick_filter), m_show_filter(true) {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    LoadIcon(kFunnelSvg);
    OnClick();
  }

private:
  wxWindow* m_quick_filter;
  bool m_show_filter;

  void OnClick() {
    m_quick_filter->Show(m_show_filter);
    SetToolTip(_("Open quick filter"));
    GetGrandParent()->Layout();
  }
};

/** Button invoking the popup menu. */
class MenuButton : public SvgButton {
public:
  MenuButton(wxWindow* parent, TheMenu& menu,
             std::function<std::string()> get_current_filter)
      : SvgButton(parent),
        m_menu(menu),
        m_get_current_filter(std::move(get_current_filter)) {
    LoadIcon(kMenuSvg);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    SetToolTip(_("Open menu"));
  }

private:
  TheMenu& m_menu;
  std::function<std::string()> m_get_current_filter;

  void OnClick() {
    m_menu.SetFilterName(m_get_current_filter());
    PopupMenu(&m_menu);
  }
};

/** Overall bottom status line. */
class StatusLine : public wxPanel {
public:
  StatusLine(wxWindow* parent, wxWindow* quick_filter, TtyPanel* tty_panel,
             std::function<void(bool)> on_stop,
             const std::function<void()>& on_hide, DataLogger& logger)
      : wxPanel(parent),
        m_is_resized(false),
        m_filter_choice(new FilterChoice(this, tty_panel)),
        m_menu(this, logger),
        m_log_button(new LogButton(this, logger, m_menu)) {
    // Add a containing sizer for labels, so they can be aligned vertically
    auto filter_label_box = new wxBoxSizer(wxVERTICAL);
    filter_label_box->Add(new wxStaticText(this, wxID_ANY, _("Filter")));

    auto flags = wxSizerFlags(0).Border();
    auto wbox = new wxWrapSizer(wxHORIZONTAL);
    wbox->Add(m_log_button, flags);
    // Stretching horizontal space. Does not work with a WrapSizer, known
    // wx bug. Left in place if it becomes fixed.
    wbox->Add(wxWindow::GetCharWidth() * 5, 0, 1);
    wbox->Add(filter_label_box, flags.Align(wxALIGN_CENTER_VERTICAL));
    wbox->Add(m_filter_choice, flags);
    wbox->Add(new PauseResumeButton(this, std::move(on_stop)), flags);
    wbox->Add(new FilterButton(this, quick_filter), flags);
    auto get_current_filter = [&] {
      return m_filter_choice->GetStringSelection().ToStdString();
    };
    wbox->Add(new CopyClipboardButton(this), flags);
    wbox->Add(new MenuButton(this, m_menu, get_current_filter), flags);
#ifdef ANDROID
    wbox->Add(new CloseButton(this, std::move(on_hide)), flags);
#endif
    SetSizer(wbox);
    wxWindow::Layout();
    wxWindow::Show();

    Bind(wxEVT_SIZE, [&](wxSizeEvent& ev) {
      m_is_resized = true;
      ev.Skip();
    });
    Bind(wxEVT_RIGHT_UP, [&](wxMouseEvent& ev) {
      m_menu.SetFilterName(m_filter_choice->GetStringSelection().ToStdString());
      PopupMenu(&m_menu);
    });
  }

  void OnContextClick() {
    m_menu.SetFilterName(m_filter_choice->GetStringSelection().ToStdString());
    PopupMenu(&m_menu);
  }

protected:
  // Make sure the initial size is sane, don't meddle when user resizes
  // dialog
  [[nodiscard]] wxSize DoGetBestClientSize() const override {
    if (m_is_resized)
      return {-1, -1};
    else
      return {85 * GetCharWidth(), 5 * GetCharHeight() / 2};
  }

private:
  bool m_is_resized;
  wxChoice* m_filter_choice;
  TheMenu m_menu;
  wxButton* m_log_button;
};

DataLogger::DataLogger(wxWindow* parent, const fs::path& path)
    : m_parent(parent),
      m_path(path),
      m_stream(path, std::ios_base::app),
      m_is_logging(false),
      m_format(Format::kDefault),
      m_log_start(NavmsgClock::now()) {}

DataLogger::DataLogger(wxWindow* parent) : DataLogger(parent, NullLogfile()) {}

void DataLogger::SetLogging(bool logging) { m_is_logging = logging; }

void DataLogger::SetLogfile(const fs::path& path) {
  m_stream = std::ofstream(path);
  m_stream << "# timestamp_format: EPOCH_MILLIS\n";
  const auto now = std::chrono::system_clock::now();
  const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
  m_stream << "# Created at: " << std::ctime(&t_c) << " \n";
  m_stream << "received_at,protocol,source,msg_type,raw_data\n";
  m_stream << std::flush;
  m_path = path;
  OnNewLogfile.Notify(path.string());
}

void DataLogger::SetFormat(DataLogger::Format format) { m_format = format; }

fs::path DataLogger::GetDefaultLogfile() {
  if (!g_dm_logfile.empty()) return g_dm_logfile.ToStdString();
  if (m_path.stem() != NullLogfile().stem()) return m_path;
  fs::path path(g_BasePlatform->GetHomeDir().ToStdString());
  path /= "monitor";
  path += (m_format == Format::kDefault ? ".log" : ".csv");
  return path;
}

std::string DataLogger::GetFileDlgTypes() const {
  if (m_format == Format::kDefault)
    return _("Log file (*.log)|*.log");
  else
    return _("Spreadsheet csv file(*.csv)|*.csv");
}

void DataLogger::Add(const Logline& ll) {
  if (!m_is_logging || !ll.navmsg) return;
  if (m_format == Format::kVdr && ll.navmsg->to_vdr().empty()) return;
  if (m_format == DataLogger::Format::kVdr)
    AddVdrLogline(ll, m_stream);
  else
    AddStdLogline(ll, m_stream,
                  m_format == DataLogger::Format::kCsv ? '|' : ' ',
                  m_log_start);
}

DataMonitor::DataMonitor(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, _("Data Monitor"), wxPoint(0, 0), wxDefaultSize,
              wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT,
              kDataMonitorWindowName),
      m_monitor_src([&](const std::shared_ptr<const NavMsg>& navmsg) {
        TtyPanel::AddIfExists(Logline(navmsg));
      }),
      m_quick_filter(nullptr),
      m_logger(parent) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto tty_panel = new TtyPanel(this, 12);
  vbox->Add(tty_panel, wxSizerFlags(1).Expand().Border());
  vbox->Add(new wxStaticLine(this), wxSizerFlags().Expand().Border());

  auto on_quick_filter_evt = [&, tty_panel] {
    auto* quick_filter = dynamic_cast<QuickFilterPanel*>(m_quick_filter);
    assert(quick_filter);
    std::string value = quick_filter->GetValue();
    tty_panel->SetQuickFilter(value);
  };
  auto on_dismiss = [&] {
    m_quick_filter->Hide();
    Layout();
  };
  m_quick_filter = new QuickFilterPanel(this, on_quick_filter_evt, on_dismiss);
  vbox->Add(m_quick_filter, wxSizerFlags().Expand());

  auto on_stop = [&, tty_panel](bool stop) { tty_panel->OnStop(stop); };
  auto on_close = [&, this]() { this->OnHide(); };
  auto status_line = new StatusLine(this, m_quick_filter, tty_panel, on_stop,
                                    on_close, m_logger);
  vbox->Add(status_line, wxSizerFlags().Expand());
  SetSizer(vbox);
  wxWindow::Fit();
  wxWindow::Hide();

  m_quick_filter->Bind(wxEVT_TEXT, [&, tty_panel](wxCommandEvent&) {
    tty_panel->SetQuickFilter(GetLabel().ToStdString());
  });
  m_quick_filter->Hide();
  tty_panel->SetOnRightClick(
      [&, status_line] { status_line->OnContextClick(); });

  Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& ev) { Hide(); });
  Bind(wxEVT_RIGHT_UP, [status_line](wxMouseEvent& ev) {
    status_line->OnContextClick();
    ev.Skip();
  });
  m_filter_list_lstnr.Init(FilterEvents::GetInstance().filter_list_change,
                           [&](ObservedEvt&) { OnFilterListChange(); });
  m_filter_update_lstnr.Init(FilterEvents::GetInstance().filter_update,
                             [&](const ObservedEvt& ev) {
                               OnFilterUpdate(ev.GetString().ToStdString());
                             });

  m_filter_apply_lstnr.Init(FilterEvents::GetInstance().filter_apply,
                            [&](const ObservedEvt& ev) {
                              OnFilterApply(ev.GetString().ToStdString());
                            });
}

void DataMonitor::Add(const Logline& ll) {
  TtyPanel::AddIfExists(ll);
  m_logger.Add(ll);
}

bool DataMonitor::IsVisible() const {
  wxWindow* w = wxWindow::FindWindowByName("TtyPanel");
  assert(w && "No TtyPanel found");
  return w->IsShownOnScreen();
}

void DataMonitor::OnFilterListChange() {
  wxWindow* w = wxWindow::FindWindowByName(kFilterChoiceName);
  if (!w) return;
  auto filter_choice = dynamic_cast<FilterChoice*>(w);
  assert(filter_choice && "Wrong FilterChoice type (!)");
  filter_choice->OnFilterListChange();
}

void DataMonitor::OnFilterUpdate(const std::string& name) const {
  if (name != m_current_filter) return;
  wxWindow* w = wxWindow::FindWindowByName("TtyScroll");
  if (!w) return;
  auto tty_scroll = dynamic_cast<TtyScroll*>(w);
  assert(tty_scroll && "Wrong TtyScroll type (!)");
  tty_scroll->SetFilter(filters_on_disk::Read(name));
}

void DataMonitor::OnFilterApply(const std::string& name) {
  wxWindow* w = wxWindow::FindWindowByName(kFilterChoiceName);
  if (!w) return;
  auto filter_choice = dynamic_cast<FilterChoice*>(w);
  assert(filter_choice && "Wrong FilterChoice type (!)");
  m_current_filter = name;
  filter_choice->OnApply(name);
}

void DataMonitor::OnHide() { Hide(); }

#pragma clang diagnostic pop
