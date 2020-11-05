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

#include "SelectDialog.h"
#include "RadarFactory.h"
#include "RadarInfo.h"

PLUGIN_BEGIN_NAMESPACE

SelectDialog::SelectDialog(wxWindow *parent, radar_pi *pi)
    : wxDialog(parent, wxID_ANY, _("Radar Selection"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE) {
  m_parent = parent;
  m_pi = pi;

  int font_size_y, font_descent, font_lead;
  GetTextExtent(_T("0"), NULL, &font_size_y, &font_descent, &font_lead);
  wxSize small_button_size(-1, (int)(1.4 * (font_size_y + font_descent + font_lead)));

  int border_size = 4;
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(topSizer);

  // Menu options

  wxStaticBox *selectBox = new wxStaticBox(this, wxID_ANY, wxString::Format(_("Select (max) %d radar scanner types"), RADARS));
  wxStaticBoxSizer *selectSizer = new wxStaticBoxSizer(selectBox, wxVERTICAL);

  wxArrayString names;
  RadarFactory::GetRadarTypes(names);

  for (int i = 0; i < RT_MAX; i++) {
    m_selected[i] = new wxCheckBox(this, wxID_ANY, names[i], wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    selectSizer->Add(m_selected[i], 0, wxALL, border_size);

    m_selected[i]->SetValue(false);
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      if (m_pi->m_radar[r]->m_radar_type == i) {
        m_selected[i]->SetValue(true);
      }
    }
  }

  // Accept/Reject button
  wxStdDialogButtonSizer *DialogButtonSizer = wxDialog::CreateStdDialogButtonSizer(wxOK | wxCANCEL);

  topSizer->Add(selectSizer);
  topSizer->Add(DialogButtonSizer, 0, wxALIGN_RIGHT | wxALL, border_size);

  DimeWindow(this);

  Fit();
  SetLayoutAdaptationMode(wxDIALOG_ADAPTATION_MODE_ENABLED);
}

PLUGIN_END_NAMESPACE
