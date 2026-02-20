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
 * Implement filter_dlg.h
 */

#include <functional>
#include <memory>
#include <string>

#include <wx/app.h>
#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/choicdlg.h>
#include <wx/frame.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textdlg.h>
#include <wx/wrapsizer.h>

#ifndef ocpnUSE_wxBitmapBundle
#include <wx/sstream.h>
#include <wxSVG/svg.h>
#endif

#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/filters_on_disk.h"
#include "filter_dlg.h"
#include "std_filesystem.h"
#include "svg_icons.h"
#include "edit_button.h"

// Make _() return const char* instead of wxString;
#undef _
#if wxCHECK_VERSION(3, 2, 0)
#define _(s) wxGetTranslation(wxASCII_STR(s)).ToStdString().c_str()
#else
#define _(s) wxGetTranslation((s)).ToStdString().c_str()
#endif

static const std::string kEditFilterFrameName("EditFilterFrame::");

static const char* const kFilterExists =
    R"( Filter already exists. Please use Edit to update
or Remove to delete it prior to create.)";

static const std::unordered_map<NavmsgStatus::Direction, std::string>
    kStringByDirection = {{NavmsgStatus::Direction::kInput, "Input"},
                          {NavmsgStatus::Direction::kHandled, "Handled"},
                          {NavmsgStatus::Direction::kOutput, "Output"},
                          {NavmsgStatus::Direction::kInternal, "Internal"}};

/** Return window with given id (which must exist) cast to T*. */
template <typename T>
T* GetWindowById(int id) {
  return dynamic_cast<T*>(wxWindow::FindWindowById(id));
};

static NavmsgStatus::Direction StringToDirection(const std::string& s) {
  for (const auto& it : kStringByDirection)
    if (it.second == s) return it.first;
  return NavmsgStatus::Direction::kNone;
}

static wxArrayString GetUserFilters() {
  auto std_filters = filters_on_disk::List(false);
  wxArrayString wx_filters;
  for (auto& f : std_filters) wx_filters.Add(f);
  return wx_filters;
}

/** Remove the bus:: prefix like nmea0183:: from the messages. */
static std::vector<std::string> GetActiveMessages() {
  auto v = NavMsgBus::GetInstance().GetActiveMessages();
  std::vector<std::string> result;
  for (const auto& m : NavMsgBus::GetInstance().GetActiveMessages()) {
    size_t pos = m.find("::");
    if (pos == std::string::npos) {
      result.push_back(m);
    } else {
      result.push_back(m.substr(pos + 2));
    }
  }
  return result;
}

class NewFilterDlg : public wxTextEntryDialog {
public:
  NewFilterDlg(wxWindow*)
      : wxTextEntryDialog(wxTheApp->GetTopWindow(), _("New filter name")) {}
};

class DeleteFilterDlg : public wxSingleChoiceDialog {
public:
  DeleteFilterDlg(wxWindow*)
      : wxSingleChoiceDialog(wxTheApp->GetTopWindow(),
                             _("Remove filter (name):"), _("Remove filter"),
                             GetUserFilters()) {}
};

class SelectFilterDlg : public wxSingleChoiceDialog {
public:
  SelectFilterDlg(wxWindow*)
      : wxSingleChoiceDialog(wxTheApp->GetTopWindow(), _("Edit filter (name):"),
                             _("Edit filter"), GetUserFilters()) {}
};

class RenameFilterChoiceDlg : public wxSingleChoiceDialog {
public:
  RenameFilterChoiceDlg(wxWindow*)
      : wxSingleChoiceDialog(wxTheApp->GetTopWindow(),
                             _("Rename filter (name):"), _("Rename filter"),
                             GetUserFilters()) {}
};

class BadFilterNameDlg : public wxMessageDialog {
public:
  BadFilterNameDlg(wxWindow* parent)
      : wxMessageDialog(parent, _(kFilterExists)) {}
};

/**
 *  Message type filter setup GUI bound to filter.include_msg and
 *  filter.exclude_msg.
 */
class MsgTypePanel : public wxPanel {
public:
  MsgTypePanel(wxWindow* parent, NavmsgFilter& filter,
               std::function<void()> on_update)
      : wxPanel(parent, wxID_ANY),
        m_filter(filter),
        m_on_update(std::move(on_update)),
        kIncludeBtnId(wxWindow::NewControlId()),
        kExcludeBtnId(wxWindow::NewControlId()),
        kEditBtnId(wxWindow::NewControlId()),
        kListboxId(wxWindow::NewControlId()),
        kSummaryId(wxWindow::NewControlId()),
        kSetBtnId(wxWindow::NewControlId()),
        kClearBtnId(wxWindow::NewControlId()) {
    auto flags = wxSizerFlags(0).Border();
    auto vbox = new wxStaticBoxSizer(wxVERTICAL, this, _("Message Types"));
    auto hbox = new wxBoxSizer(wxHORIZONTAL);

    auto radiobox = new wxBoxSizer(wxVERTICAL);
    radiobox->Add(new wxRadioButton(this, kIncludeBtnId, _("Include messages"),
                                    wxDefaultPosition, wxDefaultSize,
                                    wxRB_GROUP));
    radiobox->Add(
        new wxRadioButton(this, kExcludeBtnId, _("Exclude messages")));
    radiobox->Add(1, 1, 1);  // Expanding space, align buttons to bottom
    radiobox->Add(new wxButton(this, kSetBtnId, _("Check all")), flags);
    radiobox->Add(new wxButton(this, kClearBtnId, _("Clear all")), flags);
    hbox->Add(radiobox, flags.Expand());

    hbox->Add(1, 1, 1);  // Expanding space, keep button/summary right aligned
    wxArrayString choices;
    auto msg_types = GetActiveMessages();
    for (const auto& msg_type : msg_types) choices.Add(msg_type);
    auto listbox = new wxCheckListBox(this, kListboxId, wxDefaultPosition,
                                      wxDefaultSize, choices, 1);
    hbox->Add(new wxStaticText(this, kSummaryId, ""), flags);
    hbox->Add(listbox, flags);

    auto edit_box = new wxBoxSizer(wxVERTICAL);
    auto edit_btn = new EditButton(this, kEditBtnId, [&] { OnEditClick(); });
    edit_box->Add(edit_btn, wxSizerFlags());
    hbox->Add(edit_box, flags.Expand());

    wxWindow* btn = GetWindowById<wxWindow>(kSetBtnId);
    btn->Hide();
    btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { SetAllItems(true); });
    btn = GetWindowById<wxWindow>(kClearBtnId);
    btn->Hide();
    btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { SetAllItems(false); });

    vbox->Add(hbox, flags.Expand());
    SetSizer(vbox);
    ImportFromFilter();
    listbox->Hide();
    GetWindowById<wxStaticText>(kSummaryId)->SetLabel(GetSummary());
    Fit();

    listbox->Bind(wxEVT_CHECKLISTBOX,
                  [&](wxCommandEvent& ev) { OnItemCheck(ev.GetInt()); });
  }

private:
  std::string GetListboxItem(wxCheckListBox* listbox, unsigned i) const {
    return listbox->GetString(i).ToStdString();
  }

  void SetAllItems(bool value) {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    for (unsigned i = 0; i < listbox->GetCount(); i += 1) {
      listbox->Check(i, value);
    }
    ExportToFilter();
  }

  std::string GetSummary() {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);

    if (m_filter.exclude_msg.empty() && m_filter.include_msg.empty())
      return _("All");
    bool excluded = !m_filter.exclude_msg.empty();
    size_t checked =
        excluded ? m_filter.exclude_msg.size() : m_filter.include_msg.size();
    size_t all = listbox->GetCount();
    if (all == checked) return _("All");

    std::stringstream ss;
    ss << "[" << checked << _(" of ") << all << "]";
    return ss.str();
  }

  /**
   * User clicked the Edit/done button. Toggle button state and update
   * all related stuff.
   */
  void OnEditClick() {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    auto summary = GetWindowById<wxStaticText>(kSummaryId);
    auto edit_button = GetWindowById<EditButton>(kEditBtnId);
    bool is_editing = listbox->IsShown();
    listbox->Show(!is_editing);
    summary->Show(is_editing);
    summary->SetLabel(GetSummary());
    edit_button->SetIcon(!is_editing);
    GetWindowById<wxWindow>(kSetBtnId)->Show(!is_editing);
    GetWindowById<wxWindow>(kClearBtnId)->Show(!is_editing);
    GetParent()->Fit();
    ExportToFilter();
  }

  /**
   * User checked/unchecked an item. If the list of active filters becomes
   * empty revert the update. Otherwise update m_set from data in GUI.
   */
  void OnItemCheck(int ix) {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    int checked = 0;
    for (unsigned i = 0; i < listbox->GetCount(); i += 1)
      if (listbox->IsChecked(i)) checked += 1;
    if (checked == 0) {
      // Refuse to create a filter with no interfaces.
      listbox->Check(ix);
      return;
    }
    ExportToFilter();
  }

  /** Update GUI from data in filter. */
  void ImportFromFilter() {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    auto exclude_btn = GetWindowById<wxRadioButton>(kExcludeBtnId);
    auto include_btn = GetWindowById<wxRadioButton>(kIncludeBtnId);

    exclude_btn->SetValue(m_filter.exclude_msg.size() > 0);
    if (include_btn->GetValue()) {
      if (m_filter.include_msg.empty()) {
        for (unsigned i = 0; i < listbox->GetCount(); i++) listbox->Check(i);
      } else {
        for (unsigned i = 0; i < listbox->GetCount(); i += 1) {
          std::string item = GetListboxItem(listbox, i);
          if (m_filter.include_msg.count(item)) listbox->Check(i);
        }
      }
    } else {
      if (m_filter.exclude_msg.empty()) {
        for (unsigned i = 0; i < listbox->GetCount(); i++) listbox->Check(i);
      } else {
        for (unsigned i = 0; i < listbox->GetCount(); i += 1) {
          std::string item = GetListboxItem(listbox, i);
          if (m_filter.exclude_msg.count(item)) listbox->Check(i);
        }
      }
    }
  }

  /** Update filter from GUI data. */
  void ExportToFilter() {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    auto include_btn = GetWindowById<wxRadioButton>(kIncludeBtnId);
    m_filter.include_msg.clear();
    m_filter.exclude_msg.clear();
    auto& the_set =
        include_btn->GetValue() ? m_filter.include_msg : m_filter.exclude_msg;
    for (unsigned i = 0; i < listbox->GetCount(); i += 1) {
      if (!listbox->IsChecked(i)) continue;
      std::string item = GetListboxItem(listbox, i);
      the_set.insert(item);
    }
    if (the_set.size() == listbox->GetCount()) {
      m_filter.include_msg.clear();
      m_filter.exclude_msg.clear();
    }
    m_on_update();
  }

  NavmsgFilter& m_filter;
  std::function<void()> m_on_update;
  const int kIncludeBtnId;
  const int kExcludeBtnId;
  const int kEditBtnId;
  const int kListboxId;
  const int kSummaryId;
  const int kSetBtnId;
  const int kClearBtnId;
};

/**
 * Common base for displaying a filter set bound to the set ctor parameter
 *   - A heading like "Interfaces"
 *   - A label like "Use interfaces: "
 *   - A checkbox labeled  "All"  which checks all set items
 *   - A ChecklistBox where user can check items. Only visible if
 *     the checkbox isn't checked.
 */
template <typename T>
class SetPanel : public wxPanel {
public:
  SetPanel(wxWindow* parent, std::set<T>& set, std::function<void()> on_update,
           const std::string& label, std::function<wxArrayString()> get_choices)
      : wxPanel(),
        m_set(set),
        m_on_update(std::move(on_update)),
        kEditBtnId(wxWindow::NewControlId()),
        kListboxId(wxWindow::NewControlId()),
        kListLabelId(wxWindow::NewControlId()) {
    wxPanel::Create(parent, wxID_ANY);
    auto flags = wxSizerFlags(0).Border();
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(this, wxID_ANY, label), flags);

    // Pre-compute required size of wxCheckListBox
    // to avoid sizer errors on MacOS
    wxArrayString array = get_choices();
    int max_char = 0;
    for (auto string : array) max_char = wxMax(max_char, string.Length());
    int hsize = (max_char + 8) * wxWindow::GetCharWidth();

    hbox->Add(1, 1, 1);
    auto listbox = new wxCheckListBox(this, kListboxId, wxDefaultPosition,
                                      wxSize(hsize, -1), get_choices());
    hbox->Add(listbox, flags);
    auto list_label = new wxStaticText(this, kListLabelId, "");
    hbox->Add(list_label, flags);
    auto edit_btn = new EditButton(this, kEditBtnId, [&] { OnEditClick(); });
    hbox->Add(edit_btn, flags);
    vbox->Add(hbox, flags.Expand());
    SetSizer(vbox);

    ImportFromFilter();
    list_label->SetLabel(GetListLabel());

    listbox->Hide();
    list_label->Show();

    // Execute a round-trip of OnEditClick() to force MacOS sizers to populate
    // the listbox.
    OnEditClick();
    Layout();
    OnEditClick();

    listbox->Bind(wxEVT_CHECKLISTBOX,
                  [&](wxCommandEvent& ev) { OnItemCheck(ev.GetInt()); });
  }

private:
  /** Return typed item in listbox corresponding to given index.*/
  T GetListboxItem(wxCheckListBox* listbox, unsigned i) const {
    if constexpr (std::is_same<T, std::string>::value)
      return listbox->GetString(i).ToStdString();
    else if constexpr (std::is_same<T, NavAddr::Bus>::value)
      return NavAddr::StringToBus(listbox->GetString(i).ToStdString());
    else if constexpr (std::is_same<T, NavmsgStatus::Direction>::value)
      return StringToDirection(listbox->GetString(i).ToStdString());
    else if constexpr (std::is_same<T, NavmsgStatus::Accepted>::value)
      return NavmsgStatus::StringToAccepted(
          listbox->GetString(i).ToStdString());
    else
      assert(false && "bad type...");
  }

  /** Get label index, return descriptiomn if defined falling back to name */
  std::string GetListLabel() {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    if (m_set.empty() || m_set.size() == listbox->GetCount()) return _("All");
    std::stringstream ss;
    ss << "[" << m_set.size() << _(" of ") << listbox->GetCount() << "]";
    return ss.str();
  }

  /**
   * User clicked the Edit/done button. Toggle button state and update
   * all related stuff.
   */
  void OnEditClick() {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    listbox->Layout();
    auto list_label = GetWindowById<wxStaticText>(kListLabelId);
    auto edit_button = GetWindowById<EditButton>(kEditBtnId);
    bool is_editing = listbox->IsShown();
    listbox->Show(!is_editing);
    list_label->Show(is_editing);
    list_label->SetLabel(GetListLabel());
    edit_button->SetIcon(!is_editing);
    GetParent()->Fit();
  }

  /**
   * User checked/unchecked an item. If the list of active filters becomes
   * empty revert the update. Otherwise update m_set from data in GUI.
   */
  void OnItemCheck(int ix) {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    auto list_label = GetWindowById<wxStaticText>(kListLabelId);
    int checked = 0;
    for (unsigned i = 0; i < listbox->GetCount(); i += 1)
      if (listbox->IsChecked(i)) checked += 1;
    if (checked == 0) {
      listbox->Check(ix);
      return;
    }
    list_label->SetLabel(GetListLabel());
    ExportToFilter();
  }

  /** Update GUI from data in m_set. */
  void ImportFromFilter() {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    if (m_set.empty()) {
      for (unsigned i = 0; i < listbox->GetCount(); i++) listbox->Check(i);
    } else {
      for (unsigned i = 0; i < listbox->GetCount(); i += 1) {
        T item = GetListboxItem(listbox, i);
        if (m_set.count(item) > 0) listbox->Check(i);
      }
    }
  }

  /** Update m_set from GUI data. */
  void ExportToFilter() {
    auto listbox = GetWindowById<wxCheckListBox>(kListboxId);
    m_set.clear();
    for (unsigned i = 0; i < listbox->GetCount(); i += 1) {
      if (!listbox->IsChecked(i)) continue;
      T item = GetListboxItem(listbox, i);
      m_set.insert(item);
    }
    if (m_set.size() == listbox->GetCount()) m_set.clear();
    m_on_update();
  }

  std::set<T>& m_set;
  std::function<void()> m_on_update;
  const int kEditBtnId;
  const int kListboxId;
  const int kListLabelId;
};

/** Interfaces filtering panel, bound to filter.interfaces */
class IfacePanel : public SetPanel<std::string> {
public:
  IfacePanel(wxWindow* parent, NavmsgFilter& filter,
             std::function<void()> on_update)
      : SetPanel<std::string>(parent, filter.interfaces, std::move(on_update),
                              _("Use interfaces"),
                              [&]() { return IfacePanel::GetChoices(); }) {}

private:
  wxArrayString GetChoices() const {
    wxArrayString choices;
    for (auto& driver : CommDriverRegistry::GetInstance().GetDrivers())
      choices.Add(driver->iface);
    choices.Add("Internal");
    return choices;
  }
};

/**
 * The bus panel, filter w r t message types (nmea2000, nmea1083, etc.).
 * Bound to filter.buses.
 */
class BusPanel : public SetPanel<NavAddr::Bus> {
public:
  BusPanel(wxWindow* parent, NavmsgFilter& filter,
           std::function<void()> on_update)
      : SetPanel<NavAddr::Bus>(parent, filter.buses, std::move(on_update),
                               _("Use message buses"),
                               [&]() { return BusPanel::GetChoices(); }) {}

private:
  wxArrayString GetChoices() const {
    static const char* choices[] = {"nmea0183", "nmea2000", "SignalK", "Onenet",
                                    "Plugin"};
    return {5, choices};
  }
};

/**
 * Direction panel, set up filter w r t direction (input, output, etc..)
 * Bound to filter.directions
 */
class DirectionPanel : public SetPanel<NavmsgStatus::Direction> {
public:
  DirectionPanel(wxWindow* parent, NavmsgFilter& filter,
                 std::function<void()> on_update)
      : SetPanel<NavmsgStatus::Direction>(
            parent, filter.directions, std::move(on_update),
            _("Use directions"), [&] { return DirectionPanel::GetChoices(); }) {
  }

private:
  wxArrayString GetChoices() const {
    static const char* choices[] = {"Input", "Handled", "Output", "Internal"};
    return {4, choices};
  }
};

class AcceptedPanel : public SetPanel<NavmsgStatus::Accepted> {
public:
  AcceptedPanel(wxWindow* parent, NavmsgFilter& filter,
                std::function<void()> on_update)
      : SetPanel<NavmsgStatus::Accepted>(
            parent, filter.accepted, std::move(on_update),
            _("Use Accepted states"),
            [&]() { return AcceptedPanel::GetChoices(); }) {}

private:
  wxArrayString GetChoices() const {
    static const char* choices[] = {"Ok", "FilteredNoOutput",
                                    "FilteredDropped"};
    return {3, choices};
  }
};

class EditFilterFrame : public wxFrame {
  class Buttons : public wxPanel {
  public:
    Buttons(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
      auto vbox = new wxBoxSizer(wxVERTICAL);
      auto buttons = new wxStdDialogButtonSizer();
      buttons->AddButton(new wxButton(this, wxID_CLOSE));
      buttons->AddButton(new wxButton(this, wxID_APPLY));
      vbox->Add(buttons, wxSizerFlags().Expand());
      buttons->Realize();
      SetSizer(vbox);
      Layout();
    }
  };

public:
  EditFilterFrame(wxWindow* parent, const std::string& name,
                  std::function<void(const std::string&)> on_update,
                  std::function<void(const std::string&)> on_apply)
      : wxFrame(parent, wxID_ANY,
                [name] { return _("Edit filter: ") + name; }()),
        m_on_update(std::move(on_update)),
        m_on_apply(std::move(on_apply)),
        m_name(name) {
    SetName(kEditFilterFrameName + name);
    m_filter = filters_on_disk::Read(m_name);

    auto flags = wxSizerFlags().Border().Expand();
    auto vbox = new wxBoxSizer(wxVERTICAL);
    SetSizer(vbox);
    vbox->Add(new IfacePanel(this, m_filter, [&] { Update(); }), flags);
    vbox->Add(new wxStaticLine(this), flags.Expand());
    auto buspanel = new BusPanel(this, m_filter, [&] { Update(); });
    vbox->Add(buspanel, flags);
    vbox->Add(new wxStaticLine(this), flags.Expand());
    vbox->Add(new DirectionPanel(this, m_filter, [&] { Update(); }), flags);
    vbox->Add(new wxStaticLine(this), flags.Expand());
    vbox->Add(new AcceptedPanel(this, m_filter, [&] { Update(); }), flags);
    vbox->Add(new MsgTypePanel(this, m_filter, [&] { Update(); }), flags);
    vbox->Add(new Buttons(this), flags);
    Fit();
    Hide();

    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent&) { Destroy(); });
    Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) { OnButtonEvent(evt); });
  }

private:
  void Update() {
    filters_on_disk::Write(m_filter, m_name);
    m_on_update(m_name);
  }

  void OnButtonEvent(wxCommandEvent& evt) {
    if (evt.GetId() == wxID_CLOSE) {
      Destroy();
      evt.Skip();
    } else if (evt.GetId() == wxID_APPLY) {
      m_on_apply(m_name);
      evt.Skip();
    }
  }

  std::function<void(const std::string&)> m_on_update;
  std::function<void(const std::string&)> m_on_apply;
  std::string m_name;
  NavmsgFilter m_filter;
};

void CreateFilterDlg(wxWindow* parent) {
  NewFilterDlg dlg(parent);
  dlg.ShowModal();
  auto name = dlg.GetValue().ToStdString();
  if (name.empty()) {
    wxMessageDialog msg_dlg(wxTheApp->GetTopWindow(), _("Illegal name"));
    msg_dlg.ShowModal();
    return;
  }
  if (filters_on_disk::Exists(name)) {
    BadFilterNameDlg(wxTheApp->GetTopWindow()).ShowModal();
    return;
  }
  NavmsgFilter filter;
  filter.m_name = name;
  filters_on_disk::Write(filter, name);
  FilterEvents::GetInstance().filter_list_change.Notify();
}

void RemoveFilterDlg(wxWindow* parent) {
  if (GetUserFilters().empty()) {
    wxMessageDialog dlg(wxTheApp->GetTopWindow(), _("No filters created"));
    dlg.ShowModal();
    return;
  }
  DeleteFilterDlg dlg(parent);
  int sts = dlg.ShowModal();
  if (sts != wxID_OK) return;

  fs::path path(dlg.GetStringSelection().ToStdString());
  if (filters_on_disk::Remove(path.stem().string())) {
    FilterEvents::GetInstance().filter_list_change.Notify();
    wxMessageDialog msg_dlg(wxTheApp->GetTopWindow(), _("Filter removed"));
    msg_dlg.ShowModal();
  } else {
    wxMessageDialog msg_dlg(wxTheApp->GetTopWindow(),
                            _("Cannot remove filter"));
    msg_dlg.ShowModal();
  }
}

void RenameFilterDlg(wxWindow* parent) {
  if (GetUserFilters().empty()) {
    wxMessageDialog dlg(wxTheApp->GetTopWindow(), _("No filters to rename"));
    dlg.ShowModal();
    return;
  }
  RenameFilterChoiceDlg dlg(parent);
  int sts = dlg.ShowModal();
  if (sts != wxID_OK) return;

  fs::path old_name(dlg.GetStringSelection().ToStdString());
  wxString caption = wxString(_("Renaming ")) + old_name.string();
  auto new_name_dlg = new wxTextEntryDialog(parent, _("New name:"), caption);
  int result = new_name_dlg->ShowModal();
  if (result != wxID_OK) return;
  filters_on_disk::Rename(old_name.string(),
                          new_name_dlg->GetValue().ToStdString());
}

void EditOneFilterDlg(wxWindow* parent, const std::string& filter) {
  std::string window_name = kEditFilterFrameName + filter;
  wxWindow* frame = wxWindow::FindWindowByName(window_name);
  if (frame) {
    frame->Raise();
    return;
  }
  auto on_update = [](const std::string& _name) {
    FilterEvents::GetInstance().filter_update.Notify(_name);
  };
  auto on_apply = [](const std::string& _name) {
    FilterEvents::GetInstance().filter_apply.Notify(_name);
  };
  new EditFilterFrame(parent, filter, on_update, on_apply);
  frame = wxWindow::FindWindowByName(window_name);
  assert(frame && "Cannot create EditFilter frame");
  frame->Show();
}

void EditFilterDlg(wxWindow* parent) {
  if (GetUserFilters().empty()) {
    wxMessageDialog dlg(wxTheApp->GetTopWindow(), _("No filters created"));
    dlg.ShowModal();
    return;
  }
  SelectFilterDlg dlg(parent);
  int sts = dlg.ShowModal();
  if (sts != wxID_OK) return;

  EditOneFilterDlg(parent, dlg.GetStringSelection().ToStdString());
};
