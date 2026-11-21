/**************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
 *   Copyright (C) 2062 Alec Leamas                                        *
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
 *                                                                         e
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * connection_edit.cpp internal support.
 */

#ifndef CoNN_eDiT_h
#define CoNN_eDiT_h

#include <functional>
#include <string>
#include <type_traits>

#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/panel.h>

#include "libgui/text_ctrl_w_help.h"

#include "model/conn_params.h"

// Make _() return std::string instead of wxString;
#undef _
#if wxCHECK_VERSION(3, 2, 0)
#define _(s) wxGetTranslation(wxASCII_STR(s)).ToStdString()
#else
#define _(s) wxGetTranslation((s)).ToStdString()
#endif

namespace conn_edit {

/** Return first wxWindow parent of given type or nullptr. */
template <typename T>
T* FindParentOfType(wxWindow* w) {
  for (; w; w = w->GetParent()) {
    if (T* candidate = dynamic_cast<T*>(w)) return candidate;
  }
  return nullptr;
}

static const std::string kPortHelp =
    _("Port number (1025 - 65535, often 10110)");

/**
 * Interface implemented by widgets reflecting a ConnectionParams
 * parameter. The widget is kept in sync with the parameter(s) at any
 * time using a contract:
 *
 *   - When created, widget state is updated from parameters.
 *   - WHen user modifies widget state parameter(s) is updated.
 *   - When external code changes the parameter(s) it invokes Reload()
 *     which synchronizes widget state with parameters.
 *
 * @Note: This interface is used by Reload()  which traverses the window
 * tree. Thus a Reloadable  should be a wxWindow subclass and not
 * for example a sizer; this is checked at compile time.
 */

template <typename T>
class ReloadableTmpl {
  static_assert(std::is_base_of_v<wxWindow, T>,
                "Reloadable must inherit from wxWindow");

public:
  virtual ~ReloadableTmpl() = default;

  /** Synchronize widget value(s) with parameter(s) */
  virtual void Reload() = 0;
};

using Reloadable = ReloadableTmpl<wxWindow>;

/** NMEA2000/NMEA0183 user choice. */
class ProtocolChoice : public wxChoice, public Reloadable {
public:
  static ProtocolChoice* Create(
      wxWindow* parent, ConnectionParams& params,
      std::function<void()> on_change = [] {});

protected:
  explicit ProtocolChoice(wxWindow* parent) : wxChoice(parent, wxID_ANY) {}
};

/** Entry handling "Data port" setting. */
class PortTextCtrl : public TextCtrlWithHelp, public Reloadable {
public:
  static PortTextCtrl* Create(wxWindow* parent, ConnectionParams& params);

protected:
  explicit PortTextCtrl(wxWindow* parent)
      : TextCtrlWithHelp(parent, kPortHelp) {}
};

/** Configure n0183 input and output filters. */
class FilterConfigPanel : public wxPanel, public Reloadable {
public:
  static FilterConfigPanel* Create(wxWindow* parent, ConnectionParams& params);

protected:
  explicit FilterConfigPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {}
};

/** Two checkboxes for enabling port input and/or output. */
class InputOutputPanel : public wxPanel {
public:
  static InputOutputPanel* Create(wxWindow* parent, ConnectionParams& params);

  virtual void EnableOutput(bool enable) = 0;

  virtual void EnableInput(bool enable) = 0;

  virtual void SetOutput(bool value) = 0;

  virtual void SetInput(bool value) = 0;

protected:
  explicit InputOutputPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {}
};

/** Select serial port combobox. */
class PortCombo : public wxComboBox, public Reloadable {
public:
  static PortCombo* Create(wxWindow* parent, wxString& port);

protected:
  explicit PortCombo(wxWindow* parent) : wxComboBox(parent, wxID_ANY) {}
};

/** Main network configuration: TCP/UDP, client/server, etc. */
class NetConfigPanel : public wxPanel {
public:
  static NetConfigPanel* Create(wxWindow* parent, ConnectionParams& params);

  ~NetConfigPanel() override = default;

  [[nodiscard]] virtual bool CheckStatus() const = 0;

protected:
  explicit NetConfigPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {}
};

}  // namespace conn_edit

#endif  // CoNN_eDiT_h
