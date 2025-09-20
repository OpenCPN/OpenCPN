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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

#ifndef TIDE_TIME_H_
#define TIDE_TIME_H_

/** High and low tide search constants */
enum {
  FORWARD_ONE_HOUR_STEP = 3600,
  FORWARD_TEN_MINUTES_STEP = 600,
  FORWARD_ONE_MINUTES_STEP = 60,
  BACKWARD_ONE_HOUR_STEP = -3600,
  BACKWARD_TEN_MINUTES_STEP = -600,
  BACKWARD_ONE_MINUTES_STEP = -60
};

#endif
