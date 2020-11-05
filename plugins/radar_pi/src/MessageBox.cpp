/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
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

#include "MessageBox.h"
#include "RadarFactory.h"
#include "RadarInfo.h"
#include "RadarPanel.h"

PLUGIN_BEGIN_NAMESPACE

enum {  // process ID's
  ID_MSG_CLOSE,
  ID_MSG_HIDE,
  ID_MSG_CHOOSE,
  ID_RADAR,
  ID_DATA,
  ID_HEADING,
};

//---------------------------------------------------------------------------------------
//          Radar Control Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS(MessageBox, wxDialog)

BEGIN_EVENT_TABLE(MessageBox, wxDialog)

EVT_CLOSE(MessageBox::OnClose)
EVT_BUTTON(ID_MSG_CHOOSE, MessageBox::OnMessageChooseRadarClick)
EVT_BUTTON(ID_MSG_CLOSE, MessageBox::OnMessageCloseButtonClick)
EVT_BUTTON(ID_MSG_HIDE, MessageBox::OnMessageHideRadarClick)

EVT_MOVE(MessageBox::OnMove)
EVT_SIZE(MessageBox::OnSize)

END_EVENT_TABLE()

MessageBox::MessageBox() { Init(); }

MessageBox::~MessageBox() {}

void MessageBox::Init() {
  // Initialize all members that need initialization
  m_parent = 0;
  m_top_sizer = 0;
  m_nmea_sizer = 0;
  m_info_sizer = 0;
  m_message_sizer = 0;
  CLEAR_STRUCT(m_radar_box);
  CLEAR_STRUCT(m_radar_text);
}

bool MessageBox::Create(wxWindow *parent, radar_pi *pi) {
  m_parent = parent;
  m_pi = pi;

  if (m_parent->GetParent()) {
    m_parent = m_parent->GetParent();
  }

  long wstyle = wxCLOSE_BOX | wxCAPTION | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxCLIP_CHILDREN;
#ifdef __WXMAC__
  wstyle |= wxSTAY_ON_TOP;  // FLOAT_ON_PARENT is broken on Mac, I know this is not optimal
#endif

  const wxString caption = wxT(PLUGIN_VERSION);
  if (!wxDialog::Create(parent, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize, wstyle)) {
    return false;
  }

  CreateControls();

  Fit();
  Hide();

  m_message_state = HIDE;
  m_old_radar_seen = false;
  m_allow_auto_hide = true;

  LOG_DIALOG(wxT("radar_pi: MessageBox created"));

  return true;
}

void MessageBox::CreateControls() {
  static int BORDER = 0;

  // A top-level sizer
  m_top_sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_top_sizer);

  //**************** MESSAGE BOX ******************//
  // A box sizer to contain warnings

  wxString label;

  m_message_sizer = new wxBoxSizer(wxVERTICAL);
  m_top_sizer->Add(m_message_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, BORDER);

  for (int i = 0; i < RADARS; i++) {
    m_radar_box[i] = new wxStaticBox(this, wxID_ANY, wxT(""));
    m_radar_box[i]->SetFont(m_pi->m_font);
    wxStaticBoxSizer *ipSizer = new wxStaticBoxSizer(m_radar_box[i], wxVERTICAL);
    m_message_sizer->Add(ipSizer, 0, wxEXPAND | wxALL, BORDER * 2);

    wxString presence;
    for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
      presence << m_pi->m_radar[r]->GetInfoStatus() << wxT("\n");
    }
    m_radar_text[i] = new wxStaticText(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0);
    m_radar_text[i]->SetFont(m_pi->m_font);
    ipSizer->Add(m_radar_text[i], 0, wxALL, BORDER);
    m_radar_box[i]->Hide();
    m_radar_text[i]->Hide();
  }

  wxStaticBox *optionsBox = new wxStaticBox(this, wxID_ANY, _("Required OpenCPN option"));
  optionsBox->SetFont(m_pi->m_font);
  wxStaticBoxSizer *optionsSizer = new wxStaticBoxSizer(optionsBox, wxVERTICAL);
  m_message_sizer->Add(optionsSizer, 0, wxEXPAND | wxALL, BORDER * 2);

  m_have_open_gl = new wxCheckBox(this, ID_BPOS, _("Accelerated Graphics (OpenGL)"), wxDefaultPosition, wxDefaultSize,
                                  wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  optionsSizer->Add(m_have_open_gl, 0, wxALL, BORDER);
  m_have_open_gl->SetFont(m_pi->m_font);
  m_have_open_gl->Disable();

  wxStaticBox *nmeaBox = new wxStaticBox(this, wxID_ANY, _("For radar overlay also required"));
  nmeaBox->SetFont(m_pi->m_font);

  m_nmea_sizer = new wxStaticBoxSizer(nmeaBox, wxVERTICAL);
  m_message_sizer->Add(m_nmea_sizer, 0, wxEXPAND | wxALL, BORDER * 2);
  m_message_sizer->Hide(m_nmea_sizer);

  m_have_boat_pos =
      new wxCheckBox(this, wxID_ANY, _("Boat position"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  m_nmea_sizer->Add(m_have_boat_pos, 0, wxALL, BORDER);
  m_have_boat_pos->SetFont(m_pi->m_font);
  m_have_boat_pos->Disable();

  wxStaticText *t =
      new wxStaticText(this, wxID_ANY, _("and"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  m_nmea_sizer->Add(t, 0, wxALL, 2);
  t->SetFont(m_pi->m_font);

  m_have_true_heading =
      new wxCheckBox(this, ID_HEADING, _("True Heading"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  m_nmea_sizer->Add(m_have_true_heading, 0, wxALL, BORDER);
  m_have_true_heading->SetFont(m_pi->m_font);
  m_have_true_heading->Disable();

  t = new wxStaticText(this, wxID_ANY, _("or"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  m_nmea_sizer->Add(t, 0, wxALL, 2);
  t->SetFont(m_pi->m_font);

  m_have_mag_heading = new wxCheckBox(this, ID_HEADING, _("Magnetic heading"), wxDefaultPosition, wxDefaultSize,
                                      wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  m_nmea_sizer->Add(m_have_mag_heading, 0, wxALL, BORDER);
  m_have_mag_heading->SetFont(m_pi->m_font);
  m_have_mag_heading->Disable();

  m_have_variation =
      new wxCheckBox(this, ID_HEADING, _("Variation"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  m_nmea_sizer->Add(m_have_variation, 0, wxALL, BORDER);
  m_have_variation->SetFont(m_pi->m_font);
  m_have_variation->Disable();

  wxStaticBox *infoBox = new wxStaticBox(this, wxID_ANY, _("Statistics"));
  infoBox->SetFont(m_pi->m_font);
  m_info_sizer = new wxStaticBoxSizer(infoBox, wxVERTICAL);
  m_message_sizer->Add(m_info_sizer, 0, wxEXPAND | wxALL, BORDER * 2);

  m_statistics = new wxStaticText(this, wxID_ANY, _("Statistics"), wxDefaultPosition, wxDefaultSize, 0);
  m_statistics->SetFont(m_pi->m_small_font);
  m_info_sizer->Add(m_statistics, 0, wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE, BORDER);

  // The <Choose Radar> button
  m_choose_button = new wxButton(this, ID_MSG_CHOOSE, _("Select radar types"), wxDefaultPosition, wxDefaultSize, 0);
  m_choose_button->SetFont(m_pi->m_font);
  m_message_sizer->Add(m_choose_button, 0, wxALL, BORDER);

  // The <Hide Radar> button
  m_hide_radar = new wxButton(this, ID_MSG_HIDE, _("&Hide Radar"), wxDefaultPosition, wxDefaultSize, 0);
  m_hide_radar->SetFont(m_pi->m_font);
  m_message_sizer->Add(m_hide_radar, 0, wxALL, BORDER);
  m_message_sizer->Hide(m_hide_radar);

  // The <Close> button
  m_close_button = new wxButton(this, ID_MSG_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0);
  m_close_button->SetFont(m_pi->m_font);
  m_message_sizer->Add(m_close_button, 0, wxALL, BORDER);
  m_message_sizer->Hide(m_close_button);
}

void MessageBox::OnMove(wxMoveEvent &event) { event.Skip(); }

void MessageBox::OnSize(wxSizeEvent &event) { event.Skip(); }

void MessageBox::OnClose(wxCloseEvent &event) {
  m_allow_auto_hide = true;
  m_message_state = HIDE;
  Hide();
}

bool MessageBox::IsModalDialogShown() {
  const wxWindowList children = m_parent->GetChildren();

  if (!children.IsEmpty()) {
    for (wxWindowList::const_iterator iter = children.begin(); iter != children.end(); iter++) {
      const wxWindow *win = *iter;
      if (win->IsShown() && win->GetName().IsSameAs(wxT("dialog"))) {
        wxDialog *dialog = (wxDialog *)win;
        if (dialog->IsModal()) {
          return true;
        }
      }
    }
  }
  return false;
}

bool MessageBox::Show(bool show) {
  LOG_DIALOG(wxT("radar_pi: message box show = %d"), (int)show);

  if (show) {
    CenterOnParent();
  }

  return wxDialog::Show(show);
}

bool MessageBox::UpdateMessage(bool force) {
  message_status new_message_state = HIDE;
  time_t now = time(0);

  bool haveOpenGL = m_pi->IsOpenGLEnabled();
  bool haveGPS = m_pi->IsBoatPositionValid();
  bool haveTrueHeading = !TIMED_OUT(now, m_pi->GetHeadingTrueTimeout());
  bool haveMagHeading = !TIMED_OUT(now, m_pi->GetHeadingMagTimeout());
  bool haveHeading = haveTrueHeading || haveMagHeading;
  bool haveVariation = m_pi->GetVariationSource() != VARIATION_SOURCE_NONE;
  bool radarSeen = false;
  bool haveData = false;
  bool showRadar = m_pi->m_settings.show != 0;
  bool ret = false;

  if (force) {
    m_allow_auto_hide = false;
  }

  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    int state = m_pi->m_radar[r]->m_state.GetValue();
    if (state != RADAR_OFF) {
      radarSeen = true;
    }
    if (state == RADAR_TRANSMIT) {
      haveData = true;
    }
  }

  bool radarOn = haveOpenGL && radarSeen;
  bool navOn = haveGPS && haveHeading;
  bool no_overlay = !(m_pi->m_settings.show && m_pi->HaveOverlay());
  LOG_DIALOG(wxT("radar_pi: messagebox decision: show=%d no_overlay=%d auto_hide=%d opengl=%d radarOn=%d navOn=%d"), showRadar,
             no_overlay, m_allow_auto_hide, haveOpenGL, radarOn, navOn);

  if (!m_allow_auto_hide) {
    LOG_DIALOG(wxT("radar_pi: messagebox explicit wanted: SHOW_CLOSE"));
    new_message_state = SHOW_CLOSE;
  } else if (IsModalDialogShown()) {
    LOG_DIALOG(wxT("radar_pi: messagebox modal dialog shown: HIDE"));
    new_message_state = HIDE;
  } else if (!showRadar) {
    LOG_DIALOG(wxT("radar_pi: messagebox no radar wanted: HIDE"));
    new_message_state = HIDE;
  } else if (!haveOpenGL) {
    LOG_DIALOG(wxT("radar_pi: messagebox no OpenGL: SHOW"));
    new_message_state = SHOW;
    ret = true;
  } else if (no_overlay) {
    if (radarOn) {
      LOG_DIALOG(wxT("radar_pi: messagebox radar window needs met: HIDE"));
      new_message_state = HIDE;
    } else {
      LOG_DIALOG(wxT("radar_pi: messagebox radar window needs not met: SHOW_NO_NMEA"));
      new_message_state = SHOW_NO_NMEA;
    }
  } else {  // overlay
    if (navOn && radarOn) {
      LOG_DIALOG(wxT("radar_pi: messagebox overlay needs met: HIDE"));
      new_message_state = HIDE;
    } else {
      LOG_DIALOG(wxT("radar_pi: messagebox overlay needs not met: SHOW"));
      new_message_state = SHOW;
      ret = true;
    }
  }

  m_have_open_gl->SetValue(haveOpenGL);
  m_have_boat_pos->SetValue(haveGPS);
  m_have_true_heading->SetValue(haveTrueHeading);
  m_have_mag_heading->SetValue(haveMagHeading);
  m_have_variation->SetValue(haveVariation);

  for (size_t r = 0; r < M_SETTINGS.radar_count; r++) {
    wxString info = m_pi->m_radar[r]->GetInfoStatus();
    m_radar_text[r]->SetLabel(info);
    m_radar_text[r]->Show();
    m_radar_box[r]->SetLabel(m_pi->m_radar[r]->m_name);
    m_radar_box[r]->Show();
    m_radar_box[r]->Layout();
  }
  for (size_t r = M_SETTINGS.radar_count; r < RADARS; r++) {
    m_radar_text[r]->Hide();
    m_radar_box[r]->Hide();
    m_radar_box[r]->Layout();
  }

  wxString label;

  if (m_true_heading_info.GetNewValue(&label)) {
    m_have_true_heading->SetLabel(label);
  }
  if (m_mag_heading_info.GetNewValue(&label)) {
    m_have_mag_heading->SetLabel(label);
  }
  if (m_variation_info.GetNewValue(&label)) {
    m_have_variation->SetLabel(label);
  }
  if (m_statistics_info.GetNewValue(&label)) {
    m_statistics->SetLabel(label);
  }

  if (m_message_state != new_message_state || m_old_radar_seen != radarSeen) {
    switch (new_message_state) {
      case HIDE:
        Show(false);
        break;

      case SHOW:
        Show(true);
        m_message_sizer->Show(m_nmea_sizer);
        m_message_sizer->Hide(m_info_sizer);
        m_close_button->Hide();
        m_hide_radar->Show();
        break;

      case SHOW_NO_NMEA:
        Show(true);
        m_message_sizer->Hide(m_nmea_sizer);
        m_message_sizer->Hide(m_info_sizer);
        m_close_button->Hide();
        m_hide_radar->Show();
        break;

      case SHOW_CLOSE:
        Show(true);
        m_message_sizer->Show(m_nmea_sizer);
        m_message_sizer->Show(m_info_sizer);
        m_close_button->Show();
        m_hide_radar->Hide();
        break;
    }
    LOG_DIALOG(wxT("radar_pi: messagebox case=%d"), new_message_state);
  } else {
    LOG_DIALOG(wxT("radar_pi: no change"));
  }

  m_nmea_sizer->Layout();
  m_info_sizer->Layout();
  m_message_sizer->Layout();
  m_top_sizer->Layout();
  Layout();
  Fit();

  m_old_radar_seen = radarSeen;
  m_message_state = new_message_state;

  return ret;
}

void MessageBox::OnMessageCloseButtonClick(wxCommandEvent &event) {
  m_allow_auto_hide = true;
  m_message_state = HIDE;
  Hide();
}

void MessageBox::OnMessageHideRadarClick(wxCommandEvent &event) {
  m_pi->m_settings.show = 0;
  m_allow_auto_hide = true;
  m_message_state = HIDE;
  Hide();
  m_pi->NotifyRadarWindowViz();
}

void MessageBox::OnMessageChooseRadarClick(wxCommandEvent &event) { m_pi->MakeRadarSelection(); }

void MessageBox::SetTrueHeadingInfo(wxString &msg) {
  wxString label;

  label << _("True heading") << wxT(" ") << msg;
  m_true_heading_info.Update(label);
}

void MessageBox::SetMagHeadingInfo(wxString &msg) {
  wxString label;

  label << _("Magnetic heading") << wxT(" ") << msg;
  m_mag_heading_info.Update(label);
}

void MessageBox::SetVariationInfo(wxString &msg) {
  wxString label;

  label << _("Variation") << wxT(" ") << msg;
  m_variation_info.Update(label);
}

void MessageBox::SetStatisticsInfo(wxString &msg) { m_statistics_info.Update(msg); }

PLUGIN_END_NAMESPACE
