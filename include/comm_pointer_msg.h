/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Container  which carries a shared_ptr through wxWidgets event
 *           handling which only allows simple objects which can be represented
 *           by a void*.
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

#ifndef _COMM_POINTER_MSG_H__
#define _COMM_POINTER_MSG_H__

#include <memory>

#include "wx/event.h"

template <typename T>
class PointerMsg {
public:
  PointerMsg(std::shared_ptr<T> p) : ptr(p){};

  /** Retrieve the pointer contained in an instance and delete instance. */
  static std::shared_ptr<T> get_pointer(wxCommandEvent ev) {
    auto msg = static_cast<PointerMsg<T>*>(ev.GetClientData());
    auto ptr = std::move(msg->ptr);
    delete msg;
    return ptr;
  };

private:
  PointerMsg() = delete;
  std::shared_ptr<T> operator=(std::shared_ptr<T>&) = delete;
  std::shared_ptr<T> ptr;
};

#endif  // _COMM_POINTER_MSG_H__
