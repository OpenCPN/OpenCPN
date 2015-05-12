/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Friends
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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

#ifndef __GRIBGRABBERWIN_H__
#define __GRIBGRABBERWIN_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "GribUIDialog.h"

class GriUICtrlBar;
class GribGrabberWin;
class GribSpacerWin;
//----------------------------------------------------------------------------
// GrabberWindow Definition
//----------------------------------------------------------------------------

class GribGrabberWin: public wxPanel
{
public:
    GribGrabberWin( wxWindow *parent );
    void OnPaint( wxPaintEvent& event );
    int Size( int height );
private:
    void OnMouseEvent( wxMouseEvent& event );

    wxBitmap m_bitmap;
    bool m_bLeftDown;

DECLARE_EVENT_TABLE()
};

#endif
