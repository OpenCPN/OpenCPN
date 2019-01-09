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

#ifndef __HYPERLINK_H__
#define __HYPERLINK_H__

#include <wx/string.h>
#include <wx/list.h>

class Hyperlink // toh, 2009.02.14
{
      public:
            wxString DescrText;
            wxString Link;
            wxString LType;
};

WX_DECLARE_LIST(Hyperlink, HyperlinkList);// establish class as list member

#endif
