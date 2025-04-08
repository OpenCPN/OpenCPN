#include <chrono>
#include <iostream>  // debug junk
#include <fstream>
#include <sstream>

#include "data_monitor.h"
#include "data_monitor_src.h"

#include <wx/app.h>
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

#include "model/filters_on_disk.h"
#include "model/navmsg_filter.h"
#include "model/nmea_log.h"
#include "model/gui.h"

#include "data_monitor_src.h"
#include "svg_icons.h"
#include "tty_scroll.h"
#include "filter_dlg.h"

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

static const char* const kFilterChoiceName = "FilterChoiceWindow";

// clang-format: off
static const std::unordered_map<NavAddr::Bus, std::string> kSourceByBus = {
    {NavAddr::Bus::N0183, "NMEA0183"},
    {NavAddr::Bus::N2000, "NMEA2000"},
    {NavAddr::Bus::Signalk, "SignalK"}};
// clang-format: on

/** Return true if given filter is defined by user. */
static bool IsUserFilter(const std::string& filter_name) {
  std::vector<std::string> filters = filters_on_disk::List();
  auto found = std::find(filters.begin(), filters.end(), filter_name);
  if (found != filters.end()) return true;
  for (auto& f : filters) {
    NavmsgFilter nf = filters_on_disk::Read(f);
    if (nf.m_description == filter_name) return true;
  }
  return false;
};

static std::string TimeStamp(const NavmsgTimePoint& when) {
  using namespace std::chrono;
  using namespace std;

  auto duration = when.time_since_epoch();
  std::stringstream ss;
  auto hrs = duration_cast<hours>(duration) % 24;
  duration -= duration_cast<hours>(duration) / 24;
  auto mins = duration_cast<minutes>(duration) % 60;
  duration -= duration_cast<minutes>(duration) / 60;
  auto secs = duration_cast<seconds>(duration) % 60;
  duration -= duration_cast<seconds>(duration) / 60;
  auto msecs = duration_cast<milliseconds>(duration);
  ss << setw(2) << setfill('0') << hrs.count() % 24 << ":" << setw(2)
     << mins.count() << ":" << setw(2) << secs.count() << "." << setw(3)
     << msecs.count();
  return ss.str();
}
/**
 * Quote arg string as required by VDR plugin, see
 * https://opencpn-manuals.github.io/main/vdr/log_format.html
 * @param arg unquoted string
 * @return Possibly quoted string handling double quotes in arg.
 */
static std::string VdrQuote(const std::string& arg) {
  auto static const npos = std::string::npos;
  if (arg.find(',') == npos && arg.find('"') == npos) return arg;
  std::string s;
  for (auto c : arg) {
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
  auto now = steady_clock::now();
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
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
static void AddStdLogline(const Logline& ll, std::ostream& stream, char fs) {
  if (!ll.navmsg) return;
  wxString ws;
  ws << TimeStamp(ll.navmsg->created_at) << fs;
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
    ws << (!ll.error_msg.empty() ? ll.error_msg : "Unknown  errror");
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
        m_lines(lines),
        m_on_right_click([] {}) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    m_tty_scroll = new TtyScroll(this, m_lines);
    m_tty_scroll->Bind(wxEVT_RIGHT_UP,
                       [&](wxMouseEvent&) { m_on_right_click(); });
    vbox->Add(m_tty_scroll, wxSizerFlags(1).Expand().Border());
    m_filter.Hide();
    SetSizer(vbox);
    Fit();
  }

  void Add(const Logline& ll) override { m_tty_scroll->Add(ll); }

  bool IsActive() const override { return IsShownOnScreen(); }

  void OnStop(bool stop) {
    m_tty_scroll->Pause(stop);
    if (stop)
      m_tty_scroll->ShowScrollbars(wxSHOW_SB_DEFAULT, wxSHOW_SB_DEFAULT);
    else
      m_tty_scroll->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
  }

  void SetFilter(const NavmsgFilter& f) { m_tty_scroll->SetFilter(f); };

  void SetQuickFilter(const std::string& filter) {
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
    return wxSize(-1, m_lines * GetCharHeight());
  }

private:
  TtyScroll* m_tty_scroll;
  wxTextCtrl m_filter;
  size_t m_lines;
  std::function<void()> m_on_right_click;
};

class QuickFilterPanel : public wxPanel {
public:
  QuickFilterPanel(wxWindow* parent, std::function<void()> on_text_evt)
      : wxPanel(parent),
        m_text_ctrl(new wxTextCtrl(this, wxID_ANY)),
        m_on_text_evt(std::move(on_text_evt)) {
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

  bool Show(bool show = true) override {
    if (!show) m_text_ctrl->SetValue("");
    return wxWindow::Show(show);
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
    SetName(kFilterChoiceName);
    Bind(wxEVT_CHOICE, [&](wxCommandEvent&) { OnChoice(); });
    OnFilterListChange();
    int ix = FindString("Default settings");
    if (ix != wxNOT_FOUND) SetSelection(ix);
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
    std::string name;
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
    return NavmsgFilter();
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

/** The monitor popup menu. */
class TheMenu : public wxMenu {
public:
  enum class Id : char {
    kNewFilter = 1,  // MacOS does not want ids to be 0.
    kEditFilter,
    kDeleteFilter,
    kEditActiveFilter,
    kLogFile,
    kLogFormatDefault,
    kLogFormatCsv,
    kLogFormatVdr,
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
    AppendSubMenu(filters, _("Filters..."));
    if (IsUserFilter(m_filter))
      Append(static_cast<int>(Id::kEditActiveFilter), _("Edit active filter"));
    auto logging = new wxMenu("");
    AppendId(logging, Id::kLogFile, _("Log file..."));
    AppendRadioId(logging, Id::kLogFormatDefault, _("Log format: standard"));
    AppendRadioId(logging, Id::kLogFormatCsv, _("Log format: CSV"));
    AppendRadioId(logging, Id::kLogFormatVdr, _("Log format: VDR"));
    AppendSubMenu(logging, _("Logging..."));

    auto view = new wxMenu("");
    AppendCheckId(view, Id::kViewStdColors, _("Use colors"));
    AppendId(view, Id::kViewCopy, _("Copy messages to clipboard"));
    AppendSubMenu(view, _("View..."));

    Bind(wxEVT_MENU, [&](wxCommandEvent& ev) {
      switch (static_cast<Id>(ev.GetId())) {
        case Id::kLogFormatDefault:
          SetLogFormat(DataLogger::Format::kDefault, _("Log format: default"));
          break;

        case Id::kLogFormatVdr:
          SetLogFormat(DataLogger::Format::kVdr, _("Log format: VDR"));
          break;

        case Id::kLogFormatCsv:
          SetLogFormat(DataLogger::Format::kCsv, _("Log format: csv"));
          break;

        case Id::kLogFile:
          SetLogfile();
          break;

        case Id::kViewStdColors:
          SetColor(static_cast<int>(Id::kViewStdColors));
          break;

        case Id::kViewCopy:
          CopyToClipboard();
          break;

        case Id::kNewFilter:
          CreateFilterDlg(parent);
          break;

        case Id::kEditFilter:
          EditFilterDlg(wxTheApp->GetTopWindow());
          break;

        case Id::kEditActiveFilter:
          EditOneFilterDlg(wxTheApp->GetTopWindow(), m_filter);
          break;

        case Id::kDeleteFilter:
          RemoveFilterDlg(parent);
          break;

        default:
          std::cout << "Menu id: " << ev.GetId() << "\n";
          break;
      }
    });
  }

  void SetFilterName(const std::string& filter) {
    int id = static_cast<int>(Id::kEditActiveFilter);
    if (FindItem(id)) Delete(id);
    if (IsUserFilter(filter)) Append(id, _("Edit active filter"));
    m_filter = filter;
  }

private:
  wxWindow* m_parent;
  wxWindow* m_log_button;
  DataLogger& m_logger;
  wxStaticText* m_log_label;
  std::string m_filter;

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
    m_log_button->Disable();
    m_parent->Layout();
  }

  void SetLogfile() {
    wxFileDialog dlg(m_parent, _("Select logfile"), "",
                     m_logger.GetLogfile().string(), _("Log Files (*.log)"),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    m_logger.SetLogfile(fs::path(dlg.GetPath().ToStdString()));
    m_log_button->Enable();
  }

  void SetColor(int id) {
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
  MenuButton(wxWindow* parent, TheMenu& menu,
             std::function<std::string()> get_current_filter)
      : wxButton(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
                 wxDefaultSize, wxBU_EXACTFIT),
        m_menu(menu),
        m_get_current_filter(std::move(get_current_filter)) {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { OnClick(); });
    SetLabel(kUtfIdenticalTo);
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
             std::function<void(bool)> on_stop, DataLogger& logger)
      : wxPanel(parent),
        m_is_resized(false),
        m_log_button(new LogButton(this, logger)),
        m_log_label(new wxStaticText(this, wxID_ANY, _("Logging: Default"))),
        m_filter_choice(new FilterChoice(this, tty_panel)),
        m_menu(this, m_log_label, logger, m_log_button) {
    // Add a containing sizer for labels, so they can be aligned vertically
    auto log_label_box = new wxBoxSizer(wxVERTICAL);
    log_label_box->Add(m_log_label);
    auto filter_label_box = new wxBoxSizer(wxVERTICAL);
    filter_label_box->Add(new wxStaticText(this, wxID_ANY, _("View")));

    auto flags = wxSizerFlags(0).Border();
    auto wbox = new wxWrapSizer(wxHORIZONTAL);
    wbox->Add(log_label_box, flags.Align(wxALIGN_CENTER_VERTICAL));
    wbox->Add(m_log_button, flags);
    wbox->Add(GetCharWidth() * 2, 0, 1);  // Stretching horizontal space
    wbox->Add(filter_label_box, flags.Align(wxALIGN_CENTER_VERTICAL));
    wbox->Add(m_filter_choice, flags);
    wbox->Add(new PauseResumeButton(this, std::move(on_stop)), flags);
    wbox->Add(new FilterButton(this, quick_filter), flags);
    auto get_current_filter = [&] {
      return m_filter_choice->GetStringSelection().ToStdString();
    };
    wbox->Add(new MenuButton(this, m_menu, get_current_filter), flags);
    SetSizer(wbox);
    Layout();
    Show();

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
  wxSize DoGetBestClientSize() const override {
    if (m_is_resized)
      return wxSize(-1, -1);
    else
      return wxSize(85 * GetCharWidth(), 2.5 * GetCharHeight());
  }

private:
  bool m_is_resized;
  wxButton* m_log_button;
  wxStaticText* m_log_label;
  wxChoice* m_filter_choice;
  TheMenu m_menu;
};

DataLogger::DataLogger(wxWindow* parent, const fs::path& path)
    : m_parent(parent),
      m_path(path),
      m_stream(path, std::ios_base::app),
      m_is_logging(false),
      m_format(Format::kDefault) {}

DataLogger::DataLogger(wxWindow* parent)
    : DataLogger(parent, DefaultLogfile()) {}

void DataLogger::SetLogging(bool logging) { m_is_logging = logging; }

void DataLogger::SetLogfile(const fs::path& path) {
  m_stream = std::ofstream(path);
  m_stream << "# timestamp_format: EPOCH_MILLIS\n";
  m_stream << "received_at,protocol,msg_type,source,raw_data\n";
  m_stream << std::flush;
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
  if (m_format == Format::kVdr && ll.navmsg->to_vdr().empty()) return;
  if (m_format == DataLogger::Format::kVdr)
    AddVdrLogline(ll, m_stream);
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
  auto status_line =
      new StatusLine(this, m_quick_filter, tty_panel, on_stop, m_logger);
  vbox->Add(status_line, wxSizerFlags().Expand());
  SetSizer(vbox);
  Fit();
  Show();

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
  m_filter_update_lstnr.Init(
      FilterEvents::GetInstance().filter_update,
      [&](ObservedEvt& ev) { OnFilterUpdate(ev.GetString().ToStdString()); });

  m_filter_apply_lstnr.Init(
      FilterEvents::GetInstance().filter_apply,
      [&](ObservedEvt& ev) { OnFilterApply(ev.GetString().ToStdString()); });
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

void DataMonitor::OnFilterListChange() {
  wxWindow* w = wxWindow::FindWindowByName(kFilterChoiceName);
  if (!w) return;
  auto filter_choice = dynamic_cast<FilterChoice*>(w);
  assert(filter_choice && "Wrong FilterChoice type (!)");
  filter_choice->OnFilterListChange();
}

void DataMonitor::OnFilterUpdate(const std::string& name) {
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
  filter_choice->OnApply(name);
}

#pragma clang diagnostic pop
