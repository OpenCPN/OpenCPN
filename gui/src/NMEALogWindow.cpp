/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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
 ***************************************************************************
 */

#include "NMEALogWindow.h"
#include "TTYWindow.h"
#include "OCPNPlatform.h"

#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#endif

extern OCPNPlatform *g_Platform;

NMEALogWindow *NMEALogWindow::instance = NULL;

NMEALogWindow &NMEALogWindow::GetInstance() {
  if (instance == NULL) {
    instance = new NMEALogWindow;
  }
  return *instance;
}

NMEALogWindow::NMEALogWindow()
    : m_window(NULL), m_width(0), m_height(0), m_pos_x(0), m_pos_y(0) {}

void NMEALogWindow::Shutdown() {
  if (instance) {
    delete instance;
    instance = NULL;
  }
}

bool NMEALogWindow::Active() const { return m_window != NULL; }

void NMEALogWindow::Create(wxWindow *parent, int num_lines) {
  if (m_window == NULL) {
    m_window = new TTYWindow(parent, num_lines, this);
    m_window->SetTitle(_("NMEA Debug Window"));

    // Make sure the window is well on the screen
    m_pos_x = wxMax(m_pos_x, 40);
    m_pos_y = wxMax(m_pos_y, 40);

    m_window->SetSize(m_pos_x, m_pos_y, m_width, m_height);
  }
  m_window->Show();
}

void NMEALogWindow::Add(const wxString &s) {
  if (m_window) m_window->Add(s);
}

void NMEALogWindow::Refresh(bool do_refresh) {
  if (m_window) m_window->Refresh(do_refresh);
}

void NMEALogWindow::SetSize(const wxSize &size) {
  m_width = size.GetWidth();
  m_width = wxMax(m_width, 400 * g_Platform->GetDisplayDensityFactor());
  m_width = wxMin(m_width, g_Platform->getDisplaySize().x - 20);
  m_height = size.GetHeight();
  m_height = wxMax(m_height, 300 * g_Platform->GetDisplayDensityFactor());
  m_height = wxMin(m_height, g_Platform->getDisplaySize().y - 20);
}

void NMEALogWindow::SetPos(const wxPoint &pos) {
  m_pos_x = pos.x;
  m_pos_y = pos.y;
}

int NMEALogWindow::GetSizeW() {
  UpdateGeometry();
  return m_width;
}

int NMEALogWindow::GetSizeH() {
  UpdateGeometry();
  return m_height;
}

int NMEALogWindow::GetPosX() {
  UpdateGeometry();
  return m_pos_x;
}

int NMEALogWindow::GetPosY() {
  UpdateGeometry();
  return m_pos_y;
}

void NMEALogWindow::SetSize(int w, int h) {
  m_width = w;
  m_width = wxMax(m_width, 400 * g_Platform->GetDisplayDensityFactor());
  m_width = wxMin(m_width, g_Platform->getDisplaySize().x - 20);

  m_height = h;
  m_height = wxMax(m_height, 300 * g_Platform->GetDisplayDensityFactor());
  m_height = wxMin(m_height, g_Platform->getDisplaySize().y - 20);
  //    qDebug() << w << h << width << height;
}

void NMEALogWindow::SetPos(int x, int y) {
  m_pos_x = x;
  m_pos_y = y;
}

void NMEALogWindow::CheckPos(int display_width, int display_height) {
  if ((m_pos_x < 0) || (m_pos_x > display_width)) m_pos_x = 5;
  if ((m_pos_y < 0) || (m_pos_y > display_height)) m_pos_y = 5;
}

void NMEALogWindow::DestroyWindow() {
  if (m_window) {
    UpdateGeometry();
    m_window->Destroy();
    m_window = NULL;
  }
}

void NMEALogWindow::Move() {
  if (m_window) {
    m_window->Move(m_pos_x, m_pos_y);
    m_window->Raise();
  }
}

/**
 * Update of cached geometry values. This is necessary because
 * the configuration file will store geometry information, which will
 * be the cached values (size, position).
 * Using this mechanism prevents to cache values on every move/resize
 * of the window.
 */
void NMEALogWindow::UpdateGeometry() {
  if (m_window) {
    SetSize(m_window->GetSize());
    SetPos(m_window->GetPosition());
  }
}
