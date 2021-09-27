/******************************************************************************
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

#include "TCDataFactory.h"

const unit TCDataFactory::known_units[NUMUNITS] = {
    {"feet", "ft", LENGTH, 0.3048},
    {"meters", "m", LENGTH, 1.0},
    {"knots", "kt", VELOCITY, 1.0},
    {"knots^2", "kt^2", BOGUS, 1.0},
};

/* Find a unit; returns -1 if not found. */
int TCDataFactory::findunit(const char *unit) {
  for (int a = 0; a < NUMUNITS; a++) {
    if (!strcmp(unit, known_units[a].name) ||
        !strcmp(unit, known_units[a].abbrv))
      return a;
  }
  return -1;
}
