/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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

/** \file win_watch_daemon.cpp Implement win_watch_daemon.h */

#include <windows.h>
#include <Dbt.h>

#include <wx/frame.h>
#include <wx/log.h>
#include <wx/window.h>

#include "model/gui.h"
#include "model/sys_events.h"
#include "model/win_usb_watch.h"

class UsbListenFrame : public wxFrame {
public:
  UsbListenFrame() : wxFrame(GetTopWindow(), wxID_ANY, "") {}

  virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam,
                                  WXLPARAM lParam) {
    if (nMsg == WM_DEVICECHANGE) {
      wxLogDebug("WM_DEVICECHANGE %x %x", wParam, lParam);
      if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE) {
        SystemEvents::GetInstance().evt_dev_change.Notify();
      }
    }
    return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
  }
};

void WinUsbWatchDaemon::Start() {
  if (m_frame) return;
  m_frame = new UsbListenFrame();
  m_frame->Hide();
}

void WinUsbWatchDaemon::Stop() {
  if (!m_frame) return;
  delete m_frame;
  m_frame = 0;
}
