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
 *
 * Singleton.
 */
class NMEALogWindow
{
    public:
        static NMEALogWindow & Get();
        bool Active();
        void Create(wxWindow * parent, int num_lines = 35);
        void Destroy();
        void Add(const wxString & s);
        void Refresh(bool do_refresh = false);
        int GetSizeW() const;
        int GetSizeH() const;
        int GetPosX() const;
        int GetPosY() const;
        void SetSize(int w, int h);
        void SetSize(const wxSize & size);
        void SetPos(int x, int y);
        void SetPos(const wxPoint & pos);
        void CheckPos(int display_width, int display_height);
    private: // prevent class from being copied, needed by singleton
        NMEALogWindow();
        NMEALogWindow(const NMEALogWindow &) {}
        ~NMEALogWindow() {};
        NMEALogWindow & operator=(const NMEALogWindow &) { return *this; }
    private:
        static NMEALogWindow * instance;
        TTYWindow * window;
        int width;
        int height;
        int pos_x;
        int pos_y;
};

#endif
