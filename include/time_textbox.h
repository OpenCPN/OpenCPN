/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Time textbox to replace broken wxTimePickerCtrl on wxGTK
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2019 by David S. Register                               *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef time_textbox_h
#define time_textbox_h

#pragma once

#include <wx/datetime.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include <wx/dateevt.h>

#define NO_TIME _T("00:00")
#define TIME_FORMAT _T("%H:%M")

class TimeCtrl : public wxTextCtrl {
public:
  TimeCtrl(wxWindow *parent, wxWindowID id,
           const wxDateTime &value = wxDefaultDateTime,
           const wxPoint &pos = wxDefaultPosition,
           const wxSize &size = wxDefaultSize, long style = 0,
           const wxValidator &validator = wxDefaultValidator,
           const wxString &name = wxTextCtrlNameStr)
      : wxTextCtrl(parent, id,
                   value.IsValid() ? value.Format(TIME_FORMAT) : NO_TIME, pos,
                   size, style, validator, name) {
    Bind(wxEVT_KEY_UP, &TimeCtrl::OnChar, this);
    Bind(wxEVT_KILL_FOCUS, &TimeCtrl::OnKillFocus, this);
  };

  void SetValue(const wxDateTime val) {
    if (val.IsValid()) {
      wxTextCtrl::SetValue(val.Format(TIME_FORMAT));
    } else {
      wxTextCtrl::SetValue(NO_TIME);
    }
  };

  wxDateTime GetValue() {
    wxDateTime dt;
    wxString str = wxTextCtrl::GetValue();
    wxString::const_iterator end;
    if (!dt.ParseTime(str, &end)) {
      return wxInvalidDateTime;
    } else if (end == str.end()) {
      return dt;
    } else {
      return dt;
    }
  };

  void OnChar(wxKeyEvent &event) {
    if (GetValue().IsValid()) {
      wxDateEvent evt(this, GetValue(), wxEVT_TIME_CHANGED);
      HandleWindowEvent(evt);
    }
  };

  void OnKillFocus(wxFocusEvent &event) {
    wxTextCtrl::SetValue(GetValue().Format(TIME_FORMAT));
  };

  bool GetTime(int *hour, int *min, int *sec) {
    const wxDateTime::Tm tm = GetValue().GetTm();
    *hour = tm.hour;
    *min = tm.min;
    *sec = tm.sec;

    return true;
  }
};

#endif /* time_textbox_h */
