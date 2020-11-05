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

#ifndef _MESSAGEBOX_H_
#define _MESSAGEBOX_H_

#include "radar_pi.h"

PLUGIN_BEGIN_NAMESPACE

enum message_status { HIDE, SHOW, SHOW_NO_NMEA, SHOW_CLOSE };

class radar_info_item {
 public:
  void Update(wxString &v) {
    wxCriticalSectionLocker lock(m_exclusive);

    mod = true;
    value = v;
  };

  bool GetNewValue(wxString *str) {
    if (mod) {
      wxCriticalSectionLocker lock(m_exclusive);

      mod = false;
      *str = value;
      return true;
    }
    return false;
  }

  void GetValue(wxString *str) {
    wxCriticalSectionLocker lock(m_exclusive);

    *str = value;
  }

  radar_info_item() { mod = false; }

 private:
  wxCriticalSection m_exclusive;
  wxString value;
  bool mod;
};

class MessageBox : public wxDialog {
  DECLARE_CLASS(MessageBox)
  DECLARE_EVENT_TABLE()

 public:
  MessageBox();

  ~MessageBox();
  void Init();
  bool Show(bool show = true);

  bool Create(wxWindow *parent, radar_pi *pi);
  void CreateControls();
  bool UpdateMessage(bool force);  // Check whether message box needs to be visible, return true if shown
  void SetTrueHeadingInfo(wxString &msg);
  void SetMagHeadingInfo(wxString &msg);
  void SetVariationInfo(wxString &msg);
  void SetStatisticsInfo(wxString &msg);

  void OnClose(wxCloseEvent &event);

 private:
  void OnIdOKClick(wxCommandEvent &event);
  void OnMove(wxMoveEvent &event);
  void OnSize(wxSizeEvent &event);

  void OnMessageCloseButtonClick(wxCommandEvent &event);
  void OnMessageHideRadarClick(wxCommandEvent &event);
  void OnMessageChooseRadarClick(wxCommandEvent &event);

  bool IsModalDialogShown();

  wxWindow *m_parent;
  radar_pi *m_pi;

  radar_info_item m_true_heading_info;
  radar_info_item m_mag_heading_info;
  radar_info_item m_variation_info;
  radar_info_item m_statistics_info;

  message_status m_message_state;
  bool m_old_radar_seen;
  bool m_allow_auto_hide;

  wxBoxSizer *m_top_sizer;
  wxBoxSizer *m_nmea_sizer;
  wxBoxSizer *m_info_sizer;

  wxBoxSizer *m_message_sizer;  // Contains NO HDG and/or NO GPS

  // For each radar we have a text box
  wxStaticBox *m_radar_box[4];
  wxStaticText *m_radar_text[4];

  // MessageBox
  wxButton *m_choose_button;
  wxButton *m_hide_radar;
  wxButton *m_close_button;
  wxCheckBox *m_have_open_gl;
  wxCheckBox *m_have_boat_pos;
  wxCheckBox *m_have_true_heading;
  wxCheckBox *m_have_mag_heading;
  wxCheckBox *m_have_variation;
  wxStaticText *m_statistics;
};

PLUGIN_END_NAMESPACE

#endif
