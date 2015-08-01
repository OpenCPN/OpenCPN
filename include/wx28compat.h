/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  wx 2.8 compatibility
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *
 *
 */




#ifndef __WX28COMPAT_H__
#define __WX28COMPAT_H__

#if !wxCHECK_VERSION(3,0,0)

typedef int wxPenStyle;

#define wxPENSTYLE_INVALID -1
#define wxPENSTYLE_USER_DASH wxUSER_DASH
#define wxPENSTYLE_SOLID wxSOLID
#define wxPENSTYLE_SHORT_DASH wxSHORT_DASH
#define wxPENSTYLE_LONG_DASH wxLONG_DASH
#define wxPENSTYLE_DOT wxDOT

#define wxBRUSHSTYLE_TRANSPARENT wxTRANSPARENT
#define wxBRUSHSTYLE_SOLID wxSOLID
#define wxBRUSHSTYLE_FDIAGONAL_HATCH wxFDIAGONAL_HATCH
#define wxBRUSHSTYLE_CROSSDIAG_HATCH wxCROSSDIAG_HATCH

#endif

#endif
