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

/**
 * \file
 *
 * Platform specific wxCursor extension
 */

#ifndef __OCPCURSOR_H__
#define __OCPCURSOR_H__

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/cursor.h>
#include <wx/string.h>

class ocpCursor : public wxCursor {
public:
  ocpCursor(const wxString& cursorName, long type, int hotSpotX = 0,
            int hotSpotY = 0);
  ocpCursor(const char** xpm_data, long type, int hotSpotX = 0,
            int hotSpotY = 0);
};

#endif
