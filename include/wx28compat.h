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
#define wxPENSTYLE_DOT_DASH wxDOT_DASH

#define wxBRUSHSTYLE_TRANSPARENT wxTRANSPARENT
#define wxBRUSHSTYLE_SOLID wxSOLID
#define wxBRUSHSTYLE_FDIAGONAL_HATCH wxFDIAGONAL_HATCH
#define wxBRUSHSTYLE_CROSSDIAG_HATCH wxCROSSDIAG_HATCH

#define wxEVT_MENU wxEVT_COMMAND_MENU_SELECTED

/* Key codes */
enum wxKeyCode_compat
{
    WXK_NONE    =    0,
    
    WXK_CONTROL_A = 1,
    WXK_CONTROL_B,
    WXK_CONTROL_C,
    WXK_CONTROL_D,
    WXK_CONTROL_E,
    WXK_CONTROL_F,
    WXK_CONTROL_G,
    WXK_CONTROL_H,
    WXK_CONTROL_I,
    WXK_CONTROL_J,
    WXK_CONTROL_K,
    WXK_CONTROL_L,
    WXK_CONTROL_M,
    WXK_CONTROL_N,
    WXK_CONTROL_O,
    WXK_CONTROL_P,
    WXK_CONTROL_Q,
    WXK_CONTROL_R,
    WXK_CONTROL_S,
    WXK_CONTROL_T,
    WXK_CONTROL_U,
    WXK_CONTROL_V,
    WXK_CONTROL_W,
    WXK_CONTROL_X,
    WXK_CONTROL_Y,
    WXK_CONTROL_Z,
    
};

#endif

#endif
