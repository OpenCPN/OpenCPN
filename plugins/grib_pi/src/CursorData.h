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

#ifndef __GRIBUICData_H__
#define __GRIBUICData_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "GribUIDialogBase.h"
#include "GribSettingsDialog.h"
#include "GrabberWin.h"

class GRIBUICtrlBar;
class GribGrabberWin;
class GribSpacerWin;
//----------------------------------------------------------------------------------------------------------
//    GRIB CtrlBar Specification
//----------------------------------------------------------------------------------------------------------
class CursorData: public CursorDataBase
{
public:

    CursorData( wxWindow *window, GRIBUICtrlBar &parent );
	~CursorData() {}

    void OnCursorTrackTimer( wxTimerEvent & event) {UpdateTrackingControls();}
	void PopulateTrackingControls( bool vertical );
    void UpdateTrackingControls();
    void ResolveDisplayConflicts( int Id );
    void OnMouseEvent( wxMouseEvent &event );

    wxTimer m_tCursorTrackTimer;
private:
	void AddTrackingControl( wxControl *ctrl1,  wxControl *ctrl2,  wxControl *ctrl3, wxControl *ctrl4, bool show,
            bool vertical, int wictrl2, int wictrl3 = 0 );
    void MenuAppend( wxMenu *menu, int id, wxString label, int setting);
	void OnCBAny( wxCommandEvent& event );
	void OnMenuCallBack( wxMouseEvent& event );

	GRIBUICtrlBar &m_gparent;
	bool          m_bLeftDown;
};

#endif
