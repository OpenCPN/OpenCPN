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

#ifndef __FONTDESC_H__
#define __FONTDESC_H__

#include <wx/string.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/list.h>

class MyFontDesc
{
public:

      MyFontDesc(wxString DialogString, wxString ConfigString, wxFont *pFont, wxColour color);
      ~MyFontDesc();

      wxString    m_dialogstring;
      wxString    m_configstring;
      wxString    m_nativeInfo;
      wxFont      *m_font;
      wxColour    m_color;
};


WX_DECLARE_LIST(MyFontDesc, FontList);

#endif
