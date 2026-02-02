/***************************************************************************
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
 * String positions parsing
 */

#ifndef positionparser_h_
#define positionparser_h_

#include <wx/string.h>

class PositionParser {
public:
  PositionParser(const wxString& src);
  const wxString& GetSeparator() const { return separator; }
  const wxString& GetLatitudeString() const { return latitudeString; }
  const wxString& GetLongitudeString() const { return longitudeString; }
  double GetLatitude() const { return latitude; }
  double GetLongitude() const { return longitude; }
  bool FindSeparator(const wxString& src);
  bool IsOk() const { return parsedOk; }

private:
  wxString source;
  wxString separator;
  wxString latitudeString;
  wxString longitudeString;
  double latitude;
  double longitude;
  bool parsedOk;
};

#endif
