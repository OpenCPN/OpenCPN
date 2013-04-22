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

#ifndef __TTYSCROLL_H__
#define __TTYSCROLL_H__

#include <wx/scrolwin.h>

//    Scrolled TTY-like window for logging, etc....
class TTYScroll : public wxScrolledWindow
{
    public:
        TTYScroll(wxWindow *parent, int n_lines);
        virtual ~TTYScroll();
        virtual void OnDraw(wxDC& dc);
        virtual void Add(const wxString &line);
        void OnSize(wxSizeEvent& event);
        void Pause(bool pause) { bpause = pause; }

    protected:

        wxCoord m_hLine;  // the height of one line on screen
        size_t m_nLines;  // the number of lines we draw

        wxArrayString *m_plineArray;
        bool           bpause;
};

#endif
