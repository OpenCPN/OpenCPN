/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

/**
 * \file
 *
 * GPX files UUID support
 */

#include <wx/string.h>

/** Utility to manage unique GUID strings. */
class GpxDocument {
public:
  /** Return a unique RFC4122 version 4 compliant GUID string. */
  static wxString GetUUID(void);

  /** Seed the random generator used by GetUUID(). */
  static void SeedRandom();

private:
  static int GetRandomNumber(int min, int max);
};
