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

/* API handling raw partially decoded messages. */

#include <memory>
#include <vector>

#include <wx/event.h>
#include <wx/jsonreader.h>

#include "datastream.h"

#include "commdriverBase.h"

#ifndef _TRANSPORT_H
#define _TRANSPORT_H

/** The transport layer, a singleton. */
class Transport : public DriverListener {
public:
  void send_message(const NavMsg& message, const NavAddr& address);

  /**
   * Return a listening object which generates wxEventType events sent to
   * wxEvtHandler when a message with given key is received. The events
   * contains a shared_ptr<NavMsg>, use get_navmsg_ptr(event) to retrieve it.
   */
  ObservedVarListener get_listener(wxEventType et, wxEvtHandler* eh,
                                   const std::string& key);

  /* DriverListener implementation: */
  void notify(const NavMsg& message);
  void notify(const AbstractCommDriver& driver);

  /* Singleton implementation. */
  static Transport* getInstance();
  Transport& operator=(Transport&) = delete;
  Transport(const Transport&) = delete;

private:
  Transport() = default;
};

#endif  // TRANSPORT_H
