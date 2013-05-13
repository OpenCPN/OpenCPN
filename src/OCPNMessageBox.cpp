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
 **************************************************************************/

#include "OCPNMessageBox.h"

int OCPNMessageBox( wxWindow *parent, const wxString& message, const wxString& caption, int style,
        int x, int y )
{

#ifdef __WXOSX__
    if(g_FloatingToolbarDialog)
        g_FloatingToolbarDialog->Hide();

    if( g_FloatingCompassDialog )
        g_FloatingCompassDialog->Hide();

    if( stats )
        stats->Hide();
#endif
    wxMessageDialog dlg( parent, message, caption, style | wxSTAY_ON_TOP, wxPoint( x, y ) );
    int ret = dlg.ShowModal();

#ifdef __WXOSX__
    if(gFrame)
        gFrame->SurfaceToolbar();

    if( g_FloatingCompassDialog )
        g_FloatingCompassDialog->Show();

    if( stats )
        stats->Show();

    if(parent)
        parent->Raise();
#endif

    return ret;
}

