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

#ifndef _EMULATORRECEIVE_H_
#define _EMULATORRECEIVE_H_

#include "RadarReceive.h"
#include "socketutil.h"

PLUGIN_BEGIN_NAMESPACE

//
// An intermediary class that implements the common parts of any Emulator radar.
//

class EmulatorReceive : public RadarReceive {
 public:
  EmulatorReceive(radar_pi *pi, RadarInfo *ri) : RadarReceive(pi, ri) {
    m_shutdown = false;
    m_next_spoke = 0;
    m_next_rotation = 0;
    m_receive_socket = GetLocalhostServerTCPSocket();
    m_send_socket = GetLocalhostSendTCPSocket(m_receive_socket);
    LOG_RECEIVE(wxT("radar_pi: %s receive thread created"), m_ri->m_name.c_str());
  };

  ~EmulatorReceive() {
    closesocket(m_receive_socket);
    closesocket(m_send_socket);
  }

  void *Entry(void);
  void Shutdown(void);
  wxString GetInfoStatus();

 private:
  void EmulateFakeBuffer(void);

  volatile bool m_shutdown;

  int m_next_spoke;     // emulator next spoke
  int m_next_rotation;  // slowly rotate emulator

  SOCKET m_receive_socket;  // Where we listen for message from m_send_socket
  SOCKET m_send_socket;     // A message to this socket will interrupt select() and allow immediate shutdown
};

PLUGIN_END_NAMESPACE

#endif /* _EMULATORRECEIVE_H_ */
