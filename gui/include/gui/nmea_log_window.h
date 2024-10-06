
/**************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef NMEALOGWINDOW_H_
#define NMEALOGWINDOW_H_

#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/window.h>

#include "model/nmea_log.h"

#include "nmea_window.h"
#include "observable_evtvar.h"

/**
 * Access to the NMEA log/debug window.
 *
 * Provides everything needed to use the single NMEA log window.
 *
 * Singleton.
 *
 * Reading geometry information from the window will cache them
 * inside this class. This is used to store them permanently in
 * the configuration file.
 */
class NmeaLogWindow : public NmeaLog {
public:
  static NmeaLogWindow &GetInstance();
  NmeaLogWindow(const NmeaLogWindow &) = delete;
  NmeaLogWindow &operator=(const NmeaLogWindow &) = delete;

  /** If required , create the NMEA log window and show it. */
  static void Show();

  /** If visible, hide the NMEA log window. */
  static void Hide();

  static void Shutdown();

  /** Notified with a bool reflecting if window is visible or not. */
  EventVar OnHideChange;

  bool Active() const;
  void Create(wxWindow *parent, int num_lines = 35);
  void Add(const wxString &s);
  void Refresh(bool do_refresh = false);
  int GetSizeW();
  int GetSizeH();
  int GetPosX();
  int GetPosY();
  void SetSize(int w, int h);
  void SetSize(const wxSize &size);
  void SetPos(int x, int y);
  void SetPos(const wxPoint &pos);
  void CheckPos(int display_width, int display_height);
  void Move();

  virtual void DestroyWindow();
  wxWindow *GetTTYWindow(void) { return static_cast<wxWindow *>(m_window); }

private:
  NmeaLogWindow();
  virtual ~NmeaLogWindow() {};
  void UpdateGeometry();

  static NmeaLogWindow *instance;
  NmeaWindow *m_window;
  int m_width;
  int m_height;
  int m_pos_x;
  int m_pos_y;
};

#endif
