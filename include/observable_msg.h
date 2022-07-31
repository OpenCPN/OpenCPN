/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  notify()/listen() interface class for raw NavMsg messages.
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


#ifndef _OBSERVABLE_MSG_H
#define _OBSERVABLE_MSG_H

#include <memory>

#include <wx/event.h>
#include "comm_navmsg_bus.h"


/** Return the message pointer available in wxCommandEvent.GetClientData() */
std::shared_ptr<const NavMsg> get_navmsg_ptr(wxCommandEvent ev);

class ObservableMsg : public ObservedVar {
public:
  ObservableMsg(const std::string key) : ObservedVar(key){};

  /* Send message to all listeners. */
  void notify(std::shared_ptr<const NavMsg> msg);

protected:
  void notify(NavMsg* msg);
};

#endif  // OBSERVABLE_MSG_H
