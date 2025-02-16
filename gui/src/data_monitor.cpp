#include <chrono>
#include <iostream>  // debug junk
#include <fstream>
#include <sstream>

#include "data_monitor.h"
#include "data_monitor_src.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/platinfo.h>
#include <wx/sizer.h>
#include <wx/sstream.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/translation.h>
#include <wx/wrapsizer.h>

#ifndef ocpnUSE_wxBitmapBundle
#include <wxSVG/svg.h>
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#include "model/navmsg_filter.h"
#include "model/nmea_log.h"
#include "model/gui.h"

#include "data_monitor_src.h"
#include "svg_icons.h"
#include "tty_scroll.h"

#include "std_filesystem.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"

// Make _() return std::string instead of wxString;
#undef _
#if wxCHECK_VERSION(3, 2, 0)
#define _(s) wxGetTranslation(wxASCII_STR(s)).ToStdString()
#else
#define _(s) wxGetTranslation((s)).ToStdString()
#endif

/** Write a line in the log using the standard format. */
static void AddCandumpLogline(const Logline& ll, std::ostream& stream) {
  using namespace std::chrono;
  auto now = steady_clock::now();
  auto us = duration_cast<microseconds>(now.time_since_epoch()).count();
  stream << "[" << us / 1000000 << "." << us % 1000000 << "] ";
  stream << (ll.navmsg ? ll.navmsg->source->iface : "-") << " ";
  stream << ll.navmsg->to_candump();
  stream << "\n";
}

/** Write a line in the log using the standard format. */
static void AddStdLogline(const Logline& ll, std::ostream& stream, char fs) {
  wxString ws;
#ifndef __WXQT__  //  Date/Time on Qt are broken, at least for android
  ws << wxDateTime::Now().FormatISOTime() << fs;
#else
  ws << "- ";
#endif
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

  ws << (ll.navmsg ? ll.navmsg->source->iface : ".") << fs;
  ws << (ll.navmsg ? NavAddr::BusToString(ll.navmsg->bus) : "-") << fs;
  if (ll.state.status != NavmsgStatus::State::kOk)
    ws << (ll.error_msg.size() > 0 ? ll.error_msg : "Unknown  errror");
  else
    ws << "ok";
  ws << fs << ll.message << "\n";
  stream << ws;
}

/** Main window, a rolling log of messages. */
class TtyPanel : public wxPanel, public NmeaLog {
public:
  TtyPanel(wxWindow* parent, size_t lines)
      : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                wxTAB_TRAVERSAL, "TtyPanel"),
        m_tty_scroll(nullptr),
        m_filter(this, wxID_ANY),
        m_lines(lines) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    m_tty_scroll = new TtyScroll(this, m_lines);
    vbox->Add(m_tty_scroll, wxSizerFlags(1).Expand().Border());
    m_filter.Hide();
    SetSizer(vbox);
    Fit();
  }

  void Add(const Logline& ll) override { m_tty_scroll->Add(ll); }

  bool IsActive() const override { return IsShownOnScreen(); }

  void OnStop(bool stop) { m_tty_scroll->Pause(stop); }

  void SetFilter(const NavmsgFilter& f) { m_tty_scroll->SetFilter(f); };

  void SetQuickFilter(const std::string& filter) {
    m_tty_scroll->SetQuickFilter(filter);
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
    return wxSize(-1, m_lines * GetCharHeight());
  }

private:
  TtyScroll* m_tty_scroll;
  wxTextCtrl m_filter;
  size_t m_lines;
};

class QuickFilterPanel : public wxPanel {
public:
  QuickFilterPanel(wxWindow* parent, std::function<void()> on_text_evt)
      : wxPanel(parent),
        m_text_ctrl(new wxTextCtrl(this, wxID_ANY)),
        m_on_text_evt(on_text_evt) {
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    auto flags = wxSizerFlags(0).Border();
    auto label_box = new wxBoxSizer(wxVERTICAL);
    label_box->Add(new wxStaticText(this, wxID_ANY, _("Quick filter:")));
    hbox->Add(label_box, flags.Align(wxALIGN_CENTER_VERTICAL));
    hbox->Add(m_text_ctrl, flags);
    SetSizer(hbox);
    Fit();
    Show();
    m_text_ctrl->Bind(wxEVT_TEXT, [&](wxCommandEvent&) { m_on_text_evt(); });
  }

  std::string GetValue() { return m_text_ctrl->GetValue().ToStdString(); }

private:
  wxTextCtrl* m_text_ctrl;
  std::function<void()> m_on_text_evt;
};

/** Button to start/stop logging. */
class LogButton : public wxButton {
public:
  LogButton(wxWindow* parent, DataLogger& logger)
      : wxButton(parent, wxID_ANY), is_logging(true), m_logger(logger) {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    OnClick();
    Disable();
    Enable(false);
    UpdateTooltip();
  }

  void UpdateTooltip() {
    if (!IsThisEnabled())
      SetToolTip(_("Set log file using menu to enable"));
    else if (is_logging)
      SetToolTip(_("Click to stop logging"));
    else
      SetToolTip(_("Click to start logging"));
  }

  bool Enable(bool enable) override {
    bool result = wxWindow::Enable(enable);
    UpdateTooltip();
    return result;
  }

private:
  bool is_logging;
  DataLogger& m_logger;

  void OnClick() {
    is_logging = !is_logging;
    SetLabel(is_logging ? _("Stop") : _("Start"));
    UpdateTooltip();
    m_logger.SetLogging(is_logging);
  }
};

/** Offer user to select current filter. */
class FilterChoice : public wxChoice {
public:
  FilterChoice(wxWindow* parent, TtyPanel* tty_panel)
      : wxChoice(parent, wxID_ANY), m_tty_panel(tty_panel) {
    m_filters = NavmsgFilter::GetSystemFilters();
    Bind(wxEVT_CHOICE, [&](wxCommandEvent&) { OnChoice(); });
    OnFilterListChange();
    int ix = FindString(_("All data"));
    if (ix != wxNOT_FOUND) SetSelection(ix);
  }

private:
  // Translated labels for system filters by filter name. If not
  // found the untranslated json description is used.
  const std::unordered_map<std::string, std::string> kLabels = {
      {"all-data", _("All data")},
      {"all-nmea", _("All NMEA data")},
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

  void OnFilterListChange() {
    Clear();
    for (auto& filter : m_filters) {
      try {
        Append(kLabels.at(filter.m_name));
      } catch (std::out_of_range&) {
        Append(filter.m_description);
      }
    }
    if (!m_filters.empty()) SetSelection(0);
  }

  NavmsgFilter FilterByLabel(const std::string label) {
    std::string name;
    for (auto kv : kLabels) {
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
    return NavmsgFilter();
  }
};

/** Button to stop/resume messages in main log window. */
class StopResumeButton : public wxButton {
public:
  StopResumeButton(wxWindow* parent, std::function<void(bool)> on_stop)
      : wxButton(parent, wxID_ANY), is_stopped(true), m_on_stop(on_stop) {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    OnClick();
  }

private:
  bool is_stopped;
  std::function<void(bool)> m_on_stop;

  void OnClick() {
    is_stopped = !is_stopped;
    m_on_stop(is_stopped);
    SetLabel(is_stopped ? _("Resume") : _("Stop"));
  }
};

/** The monitor popup menu. */
class TheMenu : public wxMenu {
public:
  enum class Id {
    kNewFilter,
    kEditFilter,
    kDeleteFilter,
    kLogFile,
    kLogFormatDefault,
    kLogFormatCsv,
    kLogFormatCandump,
    kViewStdColors,
    kViewNoColors,
    kViewTimestamps,
    kViewSource,
    kViewCopy
  };

  TheMenu(wxWindow* parent, wxStaticText* log_label, DataLogger& logger,
          wxWindow* log_button)
      : m_parent(parent),
        m_log_button(log_button),
        m_logger(logger),
        m_log_label(log_label) {
    auto filters = new wxMenu("");
    AppendId(filters, Id::kNewFilter, _("Create new..."));
    AppendId(filters, Id::kEditFilter, _("Edit..."));
    AppendId(filters, Id::kDeleteFilter, _("Delete..."));
    // AppendSubMenu(filters, _("Filters..."));    FIXME: leamas: Implement

    auto logging = new wxMenu("");
    AppendId(logging, Id::kLogFile, _("Log file..."));
    AppendRadioId(logging, Id::kLogFormatDefault, _("Log format: pretty"));
    AppendRadioId(logging, Id::kLogFormatCsv, _("Log format: CSV"));
    AppendRadioId(logging, Id::kLogFormatCandump, _("Log format: candump"));
    AppendSubMenu(logging, _("Logging..."));

    auto view = new wxMenu("");
    AppendRadioId(view, Id::kViewStdColors, _("Standard Colors"));
    AppendRadioId(view, Id::kViewNoColors, _("No Colors"));
    // AppendCheckId(view, Id::kViewSource, _("Add message source"));
    // AppendCheckId(view, Id::kViewTimestamps, _("Add timestamps"));
    AppendId(view, Id::kViewCopy, _("Copy messages to clipboard"));
    AppendSubMenu(view, _("View..."));

    Bind(wxEVT_MENU, [&](wxCommandEvent& ev) {
      switch (static_cast<Id>(ev.GetId())) {
        case Id::kLogFormatDefault:
          SetLogFormat(DataLogger::Format::kDefault, _("Log format: default"));
          break;

        case Id::kLogFormatCandump:
          SetLogFormat(DataLogger::Format::kCandump, _("Log format: candump"));
          break;

        case Id::kLogFormatCsv:
          SetLogFormat(DataLogger::Format::kCsv, _("Log format: csv"));
          break;

        case Id::kLogFile:
          SetLogfile();
          break;

        case Id::kViewStdColors:
          [[fallthrough]];
        case Id::kViewNoColors:
          SetColor(ev.GetId());
          break;

        case Id::kViewCopy:
          CopyToClipboard();
          break;

        default:
          std::cout << "Menu id: " << ev.GetId() << "\n";
          break;
      }
    });
  }

private:
  wxWindow* m_parent;
  wxWindow* m_log_button;
  DataLogger& m_logger;
  wxStaticText* m_log_label;

  wxMenuItem* AppendId(wxMenu* root, Id id, const wxString& label) {
    return root->Append(static_cast<int>(id), label);
  }

  void AppendRadioId(wxMenu* root, Id id, const wxString& label) {
    root->AppendRadioItem(static_cast<int>(id), label);
  }

  void AppendCheckId(wxMenu* root, Id id, const wxString& label) {
    root->AppendCheckItem(static_cast<int>(id), label);
  }

  void SetLogFormat(DataLogger::Format format, const std::string& label) {
    m_log_label->SetLabel(label);
    m_logger.SetFormat(format);
    m_parent->Layout();
  }

  void SetLogfile() {
    wxFileDialog dlg(m_parent, _("Select logfile"), "",
                     m_logger.GetLogfile().string(), _("Log Files (*.log)"),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    m_logger = DataLogger(m_parent, fs::path(dlg.GetPath().ToStdString()));
    m_logger.SetLogging(true);
    m_log_button->Enable();
  }

  void SetColor(int id) {
    auto* w = wxWindow::FindWindowByName("TtyScroll");
    auto tty_scroll = dynamic_cast<TtyScroll*>(w);
    if (!tty_scroll) return;

    if (id == static_cast<int>(Id::kViewStdColors))
      tty_scroll->SetColors(std::make_unique<StdColorsByState>());
    else if (id == static_cast<int>(Id::kViewNoColors))
      tty_scroll->SetColors(
          std::make_unique<NoColorsByState>(tty_scroll->GetForegroundColour()));
    else
      assert(false && "Illegal color type");
  }

  void CopyToClipboard() {
    auto* tty_scroll =
        dynamic_cast<TtyScroll*>(wxWindow::FindWindowByName("TtyScroll"));
    if (!tty_scroll) return;
    tty_scroll->CopyToClipboard();
  }
};

/** Button opening the filter dialog */
class FilterButton : public wxButton {
public:
  FilterButton(wxWindow* parent, wxWindow* quick_filter)
      : wxButton(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
                 wxDefaultSize, wxBU_EXACTFIT | wxBU_BOTTOM),
        m_quick_filter(quick_filter),
        m_show_filter(true) {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    OnClick();
  }

private:
  wxWindow* m_quick_filter;
  bool m_show_filter;

  void OnClick() {
    m_show_filter = !m_show_filter;
    char buffer[2048];
    strcpy(buffer, m_show_filter ? kNoFunnelSvg : kFunnelSvg);
#ifdef ocpnUSE_wxBitmapBundle
    auto icon_size = wxSize(2 * GetCharWidth(), GetCharHeight());
    auto bundle = wxBitmapBundle::FromSVG(buffer, icon_size);
    SetBitmap(bundle);
#else
    wxStringInputStream wis(buffer);
    wxSVGDocument svg_doc(wis);
    wxImage image = svg_doc.Render(GetCharHeight(), GetCharHeight());
    SetBitmap(wxBitmap(image));
#endif
    m_quick_filter->Show(m_show_filter);
    SetToolTip(m_show_filter ? _("Close quick filter")
                             : _("Open quick filter"));
    GetGrandParent()->Layout();
  }
};

/** Button invoking the popup menu. */
class MenuButton : public wxButton {
public:
  MenuButton(wxWindow* parent, wxStaticText* log_label, DataLogger& logger,
             wxWindow* log_button)
      : wxButton(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
                 wxDefaultSize, wxBU_EXACTFIT),
        m_logger(logger),
        m_log_button(log_button),
        m_menu(parent, log_label, m_logger, m_log_button),
        m_log_label(log_label) {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    SetLabel(kUtfIdenticalTo);
    SetToolTip(_("Open menu"));
  }

private:
  DataLogger& m_logger;
  wxWindow* m_log_button;
  TheMenu m_menu;
  wxStaticText* m_log_label;

  void OnClick() { PopupMenu(&m_menu); }
};

/** Overall bottom status line. */
class StatusLine : public wxPanel {
public:
  StatusLine(wxWindow* parent, wxWindow* quick_filter, TtyPanel* tty_panel,
             std::function<void(bool)> on_stop, DataLogger& logger)
      : wxPanel(parent), m_is_resized(false), m_logger(logger) {
    // Add a containing sizer for labels, so they can be aligned vertically
    auto log_label_box = new wxBoxSizer(wxVERTICAL);
    m_log_label = new wxStaticText(this, wxID_ANY, _("Logging: Default"));
    log_label_box->Add(m_log_label);
    auto filter_label_box = new wxBoxSizer(wxVERTICAL);
    filter_label_box->Add(new wxStaticText(this, wxID_ANY, _("View")));

    auto flags = wxSizerFlags(0).Border();
    auto wbox = new wxWrapSizer(wxHORIZONTAL);
    wbox->Add(log_label_box, flags.Align(wxALIGN_CENTER_VERTICAL));
    m_log_button = new LogButton(this, m_logger);
    wbox->Add(m_log_button, flags);
    wbox->Add(GetCharWidth() * 2, 0, 1);  // Stretching horizontal space
    wbox->Add(filter_label_box, flags.Align(wxALIGN_CENTER_VERTICAL));
    wbox->Add(new FilterChoice(this, tty_panel), flags);
    wbox->Add(new StopResumeButton(this, on_stop), flags);
    wbox->Add(new FilterButton(this, quick_filter), flags);

    wbox->Add(new MenuButton(this, m_log_label, m_logger, m_log_button), flags);
    SetSizer(wbox);
    Layout();
    Show();
    Bind(wxEVT_SIZE, [&](wxSizeEvent& ev) {
      m_is_resized = true;
      ev.Skip();
    });
  }

protected:
  // Make sure the initial size is sane, don't meddle when user resizes
  // dialog
  wxSize DoGetBestClientSize() const override {
    if (m_is_resized)
      return wxSize(-1, -1);
    else
      return wxSize(85 * GetCharWidth(), 2.5 * GetCharHeight());
  }

private:
  bool m_is_resized;
  wxStaticText* m_log_label;
  wxButton* m_log_button;
  DataLogger& m_logger;
};

DataLogger::DataLogger(wxWindow* parent, fs::path path)
    : m_parent(parent),
      m_path(path),
      m_stream(path, std::ios_base::app),
      m_is_logging(false),
      m_format(Format::kDefault) {}

DataLogger::DataLogger(wxWindow* parent)
    : DataLogger(parent, DefaultLogfile()) {}

void DataLogger::SetLogging(bool logging) {
  // TODO: dialog if no logfile i. e. m_path == DefaultLogfile()
  m_is_logging = logging;
}

void DataLogger::SetLogfile(fs::path path) {
  m_stream = std::ofstream(path, std::ios_base::app);
}

void DataLogger::SetFormat(DataLogger::Format format) { m_format = format; }

fs::path DataLogger::DefaultLogfile() {
  if (wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_WINDOWS)
    return "NUL:";
  else
    return "/dev/null";
}

void DataLogger::Add(const Logline& ll) {
  if (!m_is_logging || !ll.navmsg) return;
  if (m_format == Format::kCandump && ll.navmsg->to_candump().empty()) return;
  if (m_format == DataLogger::Format::kCandump)
    AddCandumpLogline(ll, m_stream);
  else
    AddStdLogline(ll, m_stream,
                  m_format == DataLogger::Format::kCsv ? '|' : ' ');
}

DataMonitor::DataMonitor(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "Data Monitor", wxDefaultPosition,
              wxDefaultSize, wxDEFAULT_FRAME_STYLE, kDataMonitorWindowName),
      m_monitor_src([&](const std::shared_ptr<const NavMsg>& navmsg) {
        auto msg = std::dynamic_pointer_cast<const Nmea0183Msg>(navmsg);
        TtyPanel::AddIfExists(navmsg);
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
  m_quick_filter = new QuickFilterPanel(this, on_quick_filter_evt);
  vbox->Add(m_quick_filter, wxSizerFlags());

  auto on_stop = [&, tty_panel](bool stop) { tty_panel->OnStop(stop); };
  vbox->Add(new StatusLine(this, m_quick_filter, tty_panel, on_stop, m_logger),
            wxSizerFlags().Expand());
  SetSizer(vbox);

  m_quick_filter->Bind(wxEVT_TEXT, [&, tty_panel](wxCommandEvent&) {
    tty_panel->SetQuickFilter(GetLabel().ToStdString());
  });
  m_quick_filter->Hide();

  Fit();
  Show();

  Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& ev) { Hide(); });
}

void DataMonitor::Add(const Logline& ll) {
  TtyPanel::AddIfExists(ll);
  m_logger.Add(ll);
}

bool DataMonitor::IsActive() const {
  wxWindow* w = wxWindow::FindWindowByName("TtyPanel");
  assert(w && "No TtyPanel found");
  return w->IsShownOnScreen();
}

#pragma clang diagnostic pop
