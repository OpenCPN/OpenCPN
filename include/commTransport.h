/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#ifndef __COMMTRANSPORT_H__
#define __COMMTRANSPORT_H__

** The transport layer, a singleton. */
class Transport: public DriverListener {
public:

  void send_message(const nav_msg& message, const nav_addr_t& address);

  /**
   * Send given eventType message to handler when receiving a mmea0183 message
   * Message contains the payload as a wxString.
   */
  void listen(wxEventType, wxEvtHandler, const nmea0183_key&);

  /**
   * Send given eventType message to handler when receiving a n2k message.
   * Message contains the payload as a raw_msg_ptr;
   */
  void listen(wxEventType, wxEvtHandler, const nmea2000_key&);

  /**
   * Send given eventType message to handler when receiving a signalK message.
   * Message contains the parsed message tree root as a wxJSONValue*
   * NOTE: Current implementation will return all signalK messages, effectively
   * disregarding the key.
   */
  void listen(wxEventType, wxEvtHandler, const signalK_key&);


  /*DriverListener implementation: */
  void notify(const nav_msg& message);
  void notify(const AbstractDriver& driver);

  /* Singleton implementation. */
  static Transport* getInstance();
};

#endif    //guard
