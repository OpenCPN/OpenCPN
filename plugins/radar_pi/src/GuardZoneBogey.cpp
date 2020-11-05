/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "GuardZoneBogey.h"
#include "radar_pi.h"

PLUGIN_BEGIN_NAMESPACE

enum {  // process ID's
  ID_CONFIRM,
  ID_CLOSE
};

IMPLEMENT_CLASS(GuardZoneBogey, wxDialog)

BEGIN_EVENT_TABLE(GuardZoneBogey, wxDialog)

EVT_CLOSE(GuardZoneBogey::OnClose)
EVT_BUTTON(ID_CONFIRM, GuardZoneBogey::OnConfirmClick)
EVT_BUTTON(ID_CLOSE, GuardZoneBogey::OnCloseClick)

END_EVENT_TABLE()

GuardZoneBogey::GuardZoneBogey() {}

GuardZoneBogey::~GuardZoneBogey() {
  if (IsShown()) {
    m_pi->m_settings.alarm_pos = GetPosition();
  }
}

bool GuardZoneBogey::Create(wxWindow *parent, radar_pi *pi, wxWindowID id, const wxString &m_caption, const wxPoint &pos,
                            const wxSize &size, long style) {
  m_parent = parent;
  m_pi = pi;

#ifdef __WXMSW__
  long wstyle = wxSYSTEM_MENU | wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN;
#endif
#ifdef __WXMAC__
  long wstyle = wxCLOSE_BOX | wxCAPTION;
  // long wstyle = wxCLOSE_BOX | wxCAPTION | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP;

  wstyle |= wxSTAY_ON_TOP;  // Radar AUI windows, when float, are already FLOAT_ON_PARENT and we don't seem to be on top of those.
  wstyle |= wxFRAME_FLOAT_ON_PARENT;  // Float on our parent
  wstyle |= wxFRAME_TOOL_WINDOW;      // This causes window to hide when OpenCPN is not activated, but this doesn't wo
#endif
#ifdef __WXGTK__
  long wstyle = wxCLOSE_BOX | wxCAPTION | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR;
#endif

  wxSize size_min = size;

  if (!wxDialog::Create(parent, id, m_caption, m_pi->m_settings.alarm_pos, size_min, wstyle)) {
    return false;
  }

  const int border = 5;

  wxBoxSizer *GuardZoneBogeySizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(GuardZoneBogeySizer);

  m_bogey_text = new wxStaticText(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0);
  GuardZoneBogeySizer->Add(m_bogey_text, 0, wxALIGN_LEFT | wxALL, border);

  m_confirm_button = new wxButton(this, ID_CONFIRM, _("Confirm"), wxDefaultPosition, wxDefaultSize, 0);
  GuardZoneBogeySizer->Add(m_confirm_button, 0, wxALL, border);

  wxButton *bClose = new wxButton(this, ID_CLOSE, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
  GuardZoneBogeySizer->Add(bClose, 0, wxALL, border);

  DimeWindow(this);

  Fit();
  SetMinSize(GetBestSize());
  Show();

  return true;
}

//*********************************************************************************************************************

void GuardZoneBogey::ShowBogeys(wxString &text, bool bogeys, bool confirmed) {
  m_bogey_text->SetLabel(text);
  if (confirmed) {
    m_confirm_button->Disable();
  } else {
    m_confirm_button->Enable();
  }
  Fit();
  if (bogeys && !confirmed) {
    Show();
  }
}

void GuardZoneBogey::OnClose(wxCloseEvent &event) {
  m_pi->ConfirmGuardZoneBogeys();
  m_pi->m_settings.alarm_pos = GetPosition();
  Hide();
  event.Skip();
}

void GuardZoneBogey::OnConfirmClick(wxCommandEvent &event) {
  m_pi->ConfirmGuardZoneBogeys();
  event.Skip();
}

void GuardZoneBogey::OnCloseClick(wxCommandEvent &event) {
  m_pi->ConfirmGuardZoneBogeys();
  m_pi->m_settings.alarm_pos = GetPosition();
  Hide();
  event.Skip();
}

PLUGIN_END_NAMESPACE
