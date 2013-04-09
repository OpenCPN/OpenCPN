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

#include "NMEALogWindow.h"
#include "TTYWindow.h"

TTYWindow * NMEALogWindow::window = NULL;
int NMEALogWindow::width = 0;
int NMEALogWindow::height = 0;
int NMEALogWindow::pos_x = 0;
int NMEALogWindow::pos_y = 0;

bool NMEALogWindow::Active()
{
    return window != NULL;
}

void NMEALogWindow::Create(wxWindow * parent, int num_lines)
{
    if (window == NULL) {
        window = new TTYWindow(parent, num_lines);
        window->SetTitle(_("NMEA Debug Window"));

        // Make sure the window is well on the screen
        pos_x = wxMax(pos_x, 40);
        pos_y = wxMax(pos_y, 40);

        window->SetSize(pos_x, pos_y, width, height);
    }
    window->Show();
}

void NMEALogWindow::Destroy()
{
    if (window) {
        window->Destroy();
        window = NULL;
    }
}

void NMEALogWindow::Add(const wxString & s)
{
    if (window)
        window->Add(s);
}

void NMEALogWindow::Refresh(bool do_refresh)
{
    if (window)
        window->Refresh(do_refresh);
}

void NMEALogWindow::SetSize(const wxSize & size)
{
    width = size.GetWidth();
    height = size.GetHeight();
}

void NMEALogWindow::SetPos(const wxPoint & pos)
{
    pos_x = pos.x;
    pos_y = pos.y;
}

int NMEALogWindow::GetSizeW()
{
    return width;
}

int NMEALogWindow::GetSizeH()
{
    return height;
}

int NMEALogWindow::GetPosX()
{
    return pos_x;
}

int NMEALogWindow::GetPosY()
{
    return pos_y;
}

void NMEALogWindow::SetSize(int w, int h)
{
    width = w;
    height = h;
}

void NMEALogWindow::SetPos(int x, int y)
{
    pos_x = x;
    pos_y = y;
}

void NMEALogWindow::CheckPos(int display_width, int display_height)
{
    if ((pos_x < 0) || (pos_x > display_width))
        pos_x = 5;
    if ((pos_y < 0) || (pos_y > display_height))
        pos_y = 5;
}

