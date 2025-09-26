/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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

/**
 * \file
 *
 * Routeman drawing stuff
 */

#ifndef _ROUTEMAN_GUI_H
#define _ROUTEMAN_GUI_H

#include "model/routeman.h"

class RoutemanGui {
public:
  RoutemanGui(Routeman& routeman) : m_routeman(routeman) {}

  static RoutemanDlgCtx GetDlgCtx();

  void DeleteAllTracks();
  void DeleteTrack(Track* pTrack);
  bool UpdateProgress();

private:
  void DoAdvance();

  Routeman& m_routeman;
};

#endif  // _ROUTEMAN_GUI_H
