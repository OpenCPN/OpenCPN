/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Notification Manager GUI
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2025 by David S. Register                               *
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
#ifndef _NOTIFICATION_MANAGER_GUI_H__
#define _NOTIFICATION_MANAGER_GUI_H__

#include <memory>

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "model/notification.h"
#include "color_types.h"

class ocpnDC;
class ChartCanvas;
class NotificationListPanel;

class NotificationPanel : public wxPanel {
  DECLARE_EVENT_TABLE()

public:
  /** Construct an entry for a notification. */
  NotificationPanel(wxPanel *parent, wxWindowID id, const wxPoint &pos,
                    const wxSize &size,
                    std::shared_ptr<Notification> _notification,
                    int _repeat_count);

  ~NotificationPanel();
  void OnAckButton(wxCommandEvent &event);
  void OnPaint(wxPaintEvent &event);
  std::shared_ptr<Notification> GetNotification() { return notification; }

private:
  std::shared_ptr<Notification> notification;
  NotificationListPanel *Notification_list;
  wxBitmap notification_bitmap;
  wxStaticBitmap *m_itemStaticBitmap;
  wxButton *m_ack_button;
  int repeat_count;
};

class NotificationListPanel : public wxScrolledWindow {
public:
  NotificationListPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                        const wxSize &size);
  ~NotificationListPanel();

  void AddNotificationPanel(NotificationPanel *_panel);
  void ReloadNotificationPanels();
  std::vector<NotificationPanel *> GetPanels() { return panels; }

private:
  std::vector<NotificationPanel *> panels;
};

class NotificationsList : public wxDialog {
public:
  NotificationsList(wxWindow *parent);

  void OnClose(wxCloseEvent &event);
  void ReloadNotificationList();
  void OnAckAllButton(wxCommandEvent &event);
  void SetColorScheme();
  void RecalculateSize();

private:
  wxScrolledWindow *m_sWindow;
  NotificationListPanel *m_notifications_list_panel;
  wxButton *m_ackall_button;
  DECLARE_EVENT_TABLE()
};

class NotificationButton {
public:
  NotificationButton(ChartCanvas *parent);
  ~NotificationButton();

  void SetIconName(wxString &name) { m_NoteIconName = name; }
  void SetIconSeverity(NotificationSeverity _severity);

  bool IsShown() const { return m_shown; }
  void Show(bool show) { m_shown = show; }
  void Paint(ocpnDC &dc);

  bool UpdateStatus(bool newColorScheme = false);

  void SetColorScheme(ColorScheme cs);
  float GetScaleFactor() { return m_scale; }
  void SetScaleFactor(float factor);

  void Move(const wxPoint &pt) { m_rect.SetPosition(pt); }
  /**
   * Return the coordinates of the widget, in physical pixels relative
   * to the canvas window. Beware when comparing with data returned from
   * wxWidgets APIs, which return logical pixels.
   */
  wxRect GetRect(void) const { return m_rect; }
  /**
   * Return the coordinates of the widget, in logical pixels.
   * This can be compared with data returned from wxWidgets APIs.
   */
  wxRect GetLogicalRect(void) const;

private:
  void CreateBmp(bool bnew = false);
  void CreateTexture();
  void UpdateTexture();

  ChartCanvas *m_parent;
  wxBitmap m_StatBmp;
  wxBitmap m_MaskBmp;
  wxStaticBitmap *m_pStatBoxToolStaticBmp;

  wxString m_lastNoteIconName;
  wxString m_NoteIconName;

  wxBitmap _img_gpsRed;
  int m_xoffset;
  int m_yoffset;
  float m_scale;

  /**
   * The coordinates of the compass widget, in physical pixels relative to the
   * canvas window.
   */
  wxRect m_rect;
  bool m_shown;
  ColorScheme m_cs;
  bool m_texOK;

#ifdef ocpnUSE_GL
  unsigned int m_texobj;
  int m_tex_w, m_tex_h;
  int m_image_width, m_image_height;
#endif
};

#endif
