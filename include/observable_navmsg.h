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

class ObservableMsg : public Observable {
public:
  ObservableMsg(const std::string key) : Observable(key){};

  /* Send message to all listeners. */
  void Notify(std::shared_ptr<const NavMsg> msg) {
    Observable::Notify(std::dynamic_pointer_cast<const void>(msg));
  }
};

#endif  // OBSERVABLE_MSG_H
