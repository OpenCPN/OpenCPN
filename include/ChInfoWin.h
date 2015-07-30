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

#ifndef __CHINFOWIN_H__
#define __CHINFOWIN_H__

#include <wx/window.h>
#include <wx/stattext.h>

class ChInfoWin: public wxWindow
{
public:
    ChInfoWin( wxWindow *parent );
    ~ChInfoWin();

    void SetString(const wxString &s){ m_string = s; }
    const wxString& GetString(void) { return m_string; }
    void MouseEvent( wxMouseEvent& event );
    
    void SetPosition( wxPoint pt )
    {
        m_position = pt;
    }
    void SetWinSize( wxSize sz )
    {
        m_size = sz;
    }
    void SetBitmap( void );
    void FitToChars( int char_width, int char_height );
    wxSize GetWinSize( void )
    {
        return m_size;
    }
    void OnPaint( wxPaintEvent& event );
    void OnEraseBackground( wxEraseEvent& event );

    wxStaticText *m_pInfoTextCtl;
    int dbIndex;
private:

    wxString m_string;
    wxSize m_size;
    wxPoint m_position;

    DECLARE_EVENT_TABLE()
};


#endif
