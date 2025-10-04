/***************************************************************************
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

/**
 * \file
 *
 * Generic hyperlink data type.
 */

#ifndef HYPERLINK_H_
#define HYPERLINK_H_

#include <vector>

#include <wx/string.h>

class Hyperlink {
public:
  Hyperlink();
  virtual ~Hyperlink();
  wxString DescrText;
  wxString Link;
  wxString LType;
  std::string GUID;
};

using HyperlinkList = std::vector<Hyperlink*>;

#endif
