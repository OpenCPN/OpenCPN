/******************************************************************************
 * $Id:   $
 *
 * Project:  OpenCPN
 * Purpose:  Clickable canvas to draw upon
 * Author:   Johan van der Sman
 *
 ***************************************************************************
 *   Copyright (C) 2015 Johan van der Sman                                 *
 *   johan.sman@gmail.com                                                  *
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
 *
 */

#ifndef _CANVAS_H_
#define _CANVAS_H_

#include "aisradar.h"
 
class Canvas : public wxPanel {
private:
	Canvas() {};  // forbidden
public:
    Canvas(wxWindow *parent, RadarFrame *view, wxWindowID id, const wxPoint& pos, const wxSize& size); 
    void mouseMoved(wxMouseEvent& event);
    void mouseDown(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent& event);
    void paintEvent(wxPaintEvent& event);
    DECLARE_EVENT_TABLE()

private:
    RadarFrame    *pv;
    bool         MouseDown;
    RadarFrame  *Parent;
};

#endif
