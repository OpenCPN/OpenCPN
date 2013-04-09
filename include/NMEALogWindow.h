/***************************************************************************
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

#ifndef __NMEALOGWINDOW_H__
#define __NMEALOGWINDOW_H__

class wxWindow;
class wxString;
class wxSize;
class wxPoint;
class TTYWindow;

/**
 * This class provides access to the NMEA log/debug window.
 *
 * This provides everything needed to use the single NMEA log window.
 */
class NMEALogWindow
{
    public:
        static bool Active();
        static void Create(wxWindow * parent, int num_lines = 35);
        static void Destroy();
        static void Add(const wxString & s);
        static void Refresh(bool do_refresh = false);
        static int GetSizeW();
        static int GetSizeH();
        static int GetPosX();
        static int GetPosY();
        static void SetSize(int w, int h);
        static void SetSize(const wxSize & size);
        static void SetPos(int x, int y);
        static void SetPos(const wxPoint & pos);
        static void CheckPos(int display_width, int display_height);
    private: // prevent class from being copied
        NMEALogWindow(const NMEALogWindow &) {}
        ~NMEALogWindow() {}
        NMEALogWindow & operator=(const NMEALogWindow &) { return *this; }
    private:
        static TTYWindow * window;
        static int width;
        static int height;
        static int pos_x;
        static int pos_y;
};

#endif
