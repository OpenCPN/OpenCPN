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

#ifndef _GUARDZONEBOGEY_H_
#define _GUARDZONEBOGEY_H_

#include "radar_pi.h"

PLUGIN_BEGIN_NAMESPACE

/*
 =======================================================================================================================
 Guard Zone Bogey Dialog Specification ;
 =======================================================================================================================
 */
class GuardZoneBogey : public wxDialog {
  DECLARE_CLASS(GuardZoneBogey)
  DECLARE_EVENT_TABLE()

 public:
  GuardZoneBogey();

  ~GuardZoneBogey();

  bool Create(wxWindow *parent, radar_pi *pi, wxWindowID id = wxID_ANY, const wxString &m_caption = _("Alarm"),
              const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
              long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  void ShowBogeys(wxString &text, bool bogeys, bool confirmed);

 private:
  void OnClose(wxCloseEvent &event);
  void OnConfirmClick(wxCommandEvent &event);
  void OnCloseClick(wxCommandEvent &event);

  wxWindow *m_parent;
  radar_pi *m_pi;

  /* Controls */
  wxStaticText *m_bogey_text;
  wxButton *m_confirm_button;
};

PLUGIN_END_NAMESPACE

#endif /* _GUARDZONE_H_ */
