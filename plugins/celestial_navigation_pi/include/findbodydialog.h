/******************************************************************************
 * $Id: findbodydialog.h,v 1.10 2010/05/15 03:55:35 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Celestial Navigation Support
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Sean D'Epagnier   *
 *   $EMAIL$   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 */

#ifndef _FINDBODYDIALOG_H_
#define _FINDBODYDIALOG_H_

/*!
 * Includes
 */
#include "wx/listctrl.h"
#include "wx/calctrl.h"
#include "scrollingdialog.h"

/*!
 * Forward declarations
 */

class   wxListCtrl;
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_FINDBODYDIALOG 7000
#define SYMBOL_FINDBODYDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FINDBODYDIALOG_TITLE _("Find sky location for a given Body")
#define SYMBOL_FINDBODYDIALOG_IDNAME ID_FINDBODYDIALOG
#define SYMBOL_FINDBODYDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_FINDBODYDIALOG_POSITION wxDefaultPosition

#define ID_FINDBODYDIALOG_UPDATE_TEXT 7005
#define ID_FINDBODYDIALOG_UPDATE_CHECK 7006
#define ID_FINDBODYDIALOG_DONE 7007

#define ID_MARKPROP 8000
#define SYMBOL_MARKPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MARKPROP_TITLE _("Mark Properties")
#define SYMBOL_MARKPROP_IDNAME ID_MARKPROP
#define SYMBOL_MARKPROP_SIZE wxSize(200, 300)
#define SYMBOL_MARKPROP_POSITION wxDefaultPosition
#define ID_MARKPROP_CANCEL 8001
#define ID_MARKPROP_OK 8002
#define ID_ICONCTRL 8003
#define ID_LATCTRL 8004
#define ID_LONCTRL 8005
#define ID_SHOWNAMECHECKBOX1 8006

// toh, 2009.02.08
#define ID_MARKINFO 8007
#define SYMBOL_MARKINFO_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MARKINFO_TITLE _("Mark Information")
#define SYMBOL_MARKINFO_IDNAME ID_MARKINFO
#define SYMBOL_MARKINFO_SIZE wxSize(200, 300)
#define SYMBOL_MARKINFO_POSITION wxDefaultPosition
#define ID_MARKINFO_CANCEL 8008
#define ID_MARKINFO_OK 8009


////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * FindBodyDialog class declaration
 */

class Sight;

class FindBodyDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( FindBodyDialog )
    DECLARE_EVENT_TABLE()

public:

    /// Constructors
    FindBodyDialog();
    FindBodyDialog(wxWindow* parent, Sight *sight, wxWindowID id = SYMBOL_FINDBODYDIALOG_IDNAME,
        const wxString& caption = SYMBOL_FINDBODYDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_FINDBODYDIALOG_POSITION,
        const wxSize& size = SYMBOL_FINDBODYDIALOG_SIZE,
        long style = SYMBOL_FINDBODYDIALOG_STYLE );
    ~FindBodyDialog( );

    bool Create( wxWindow* parent, Sight* sight, wxWindowID id,
                 const wxString& caption, const wxPoint& pos,
                 const wxSize& size, long style );

    void CreateControls();

    void SetDialogTitle(wxString title);

    void OnUpdate( wxCommandEvent& event );
    void Update();
    void OnDoneClick( wxCommandEvent& event );
    void OnClose(wxCloseEvent& event);

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxTextCtrl*   m_Latitude;
    wxTextCtrl*   m_Longitude;
    wxCheckBox*   m_azimuthMagneticNorth;

    wxStaticText* m_Altitude;
    wxStaticText* m_Azimuth;

    wxButton*     m_DoneButton;

    Sight*        m_Sight;
};



#endif
    // _FINDBODYDIALOG_H_
